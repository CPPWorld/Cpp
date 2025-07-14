#pragma once

#include <list>
#include <queue>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <future>
#include <variant>
#include <optional>
#include <functional>
#include <unordered_map>
#include <condition_variable>

namespace ns_event_synchronizer{
    using event_type=std::string; // Alias for event identifiers
    enum class execution_mode{invalid=-1,async=0,sync}; // execution mode.

    // Interface for event wrappers that define execution logic
    class i_event{
    public:
        virtual bool execute(const event_type&)=0; // Execute handler for given event type
        virtual void add(const event_type&,
            std::function<bool()>)=0; // Register callback for event type
        virtual void remove(const event_type& evt)=0;
        virtual ~i_event(){}
    };

    // Generic event class holding command data and event handlers
    template<typename T>
    class event:public i_event{
    public:
        event():cmd(std::make_shared <T>()){
        }
        ~event()override=default;

        // Execute the corresponding handler based on event type
        bool execute(const event_type& evt)override{
            std::lock_guard<std::mutex> lock(_mtx);
            auto it=handlers.find(evt);
            return it!=handlers.end()?it->second():false;
        }

        std::shared_ptr<T> command(){
            return cmd;
        }

        // Add handler for event type
        void add(const event_type& evt,
            std::function<bool()> handler)override{
            std::lock_guard<std::mutex> lock(_mtx);
            handlers[evt]=std::move(handler);
        }
        
        // Remove handler for event type
        void remove(const event_type& evt)override{
            std::lock_guard<std::mutex> lock(_mtx);
            handlers.erase(evt);
        }

    private:
        std::shared_ptr<T> cmd; // Command payload object. Used during execute() call.
        std::mutex _mtx; //sync access to handlers map.
        std::unordered_map<event_type,
            std::function<bool()>> handlers; // collection of registered Event handlers.
    };

    using target_id=std::string;
    class event_registry{
    public:
        void add(const target_id& id,
            std::shared_ptr<i_event> evt){
            std::lock_guard<std::mutex> lock(_mtx);
            _registry[id]=std::move(evt);
        }

        void remove(const target_id& id){
            std::lock_guard<std::mutex> lock(_mtx);
            _registry.erase(id);
        }

        std::shared_ptr<i_event> get(const target_id& id){
            std::lock_guard<std::mutex> lock(_mtx);
            auto it=_registry.find(id);
            return it!=_registry.end()?it->second:nullptr;
        }

        std::vector<target_id> get_targets(){
            std::vector<target_id> targets;
            std::lock_guard<std::mutex> lock(_mtx);
            for (auto item:_registry){
                targets.emplace_back(item.first);
            }
            return targets;
        }

    private:
        std::unordered_map<target_id,
            std::shared_ptr<i_event>> _registry;
        std::mutex _mtx;
    };

    // Structure to package data needed for an event execution
    struct event_handler_data{
        event_type event_type;
        execution_mode execution_mode{execution_mode::invalid};
        std::optional<std::promise<void>> sync_promise; // Used for signaling in sync mode
    };

    // Interface for event executor
    class i_executor{
    public:
        virtual std::future<void> submit(event_type,execution_mode)=0;
        virtual void shutdown()=0;
        virtual bool is_empty()=0;
        virtual ~i_executor()=default;
    };

    // Concrete executor for queued event processing per target
    class executor:public i_executor{
    public:
        explicit executor(std::shared_ptr<i_event> evt){
            // One execetor/item in event registry.
            _thread=std::thread([event=std::move(evt),this](){
                if (!event){
                    return;
                }
                do{
                    event_handler_data data;
                    {
                        std::unique_lock<std::mutex> lock(_mtx);
                         // Block until an event arrives or shutdown
                        _cv.wait(lock,[this]{
                            return !_queue.empty()||_shutdown;
                        });
                        if (_shutdown){
                            break;
                        }
                        data=std::move(_queue.front());
                        _queue.pop();
                    }
                    if(!event->execute(data.event_type)){ // Run event
                        // log error
                        // or what to do?
                    }
                    if ((execution_mode::sync==data.execution_mode)&&
                        (data.sync_promise.has_value())){
                        data.sync_promise->set_value(); // Signal completion
                    }
                }while(true);
            });
        }

        ~executor(){
            if (_thread.joinable()){
                _thread.join(); // Ensure thread shutdown
            }
        }

        // Posts a new event to this handler's queue
        std::future<void> submit(event_type event_type,
            execution_mode execution_mode)override{
            std::optional<std::promise<void>> promise;
            std::future<void> future;
            if (execution_mode::sync==execution_mode){
                promise.emplace();
                future=promise->get_future(); // Capture future for sync execution
            }
            {
                std::lock_guard<std::mutex> lock(_mtx);
                _queue.push({event_type,execution_mode,
                    std::move(promise)});
            }
            _cv.notify_one(); // Wake thread
            return future;
        }

        bool is_empty()override{
            std::unique_lock<std::mutex> lock(_mtx);
            return _queue.empty();
        }

        void shutdown()override{
            _shutdown=true;
            _cv.notify_all();
        }

    private:
        std::thread _thread;
        std::atomic<bool> _shutdown{false};
        std::queue<event_handler_data> _queue;
        std::mutex _mtx;
        std::condition_variable _cv;
    };

    // Manages all event handlers and dispatching logic
    class event_synchronizer{
    struct event_data{ // Struct for dispatching events across targets
        target_id _target;
        event_type _type;
        execution_mode _execution_mode{execution_mode::invalid};
    };
    public:
        event_synchronizer(std::shared_ptr<event_registry>&& evt_reg){
            _worker_thread=std::thread([evt_registry=std::move(evt_reg),this](){
                for (auto& target:evt_registry->get_targets()){
                    _executor_map.emplace(target,
                        std::make_unique<executor>(
                            evt_registry->get(target))); // Spawn handler per target
                }
                while (true){
                    auto get_event_data=[this]()->std::optional<event_data>{
                        std::unique_lock<std::mutex> lock(_mtx);
                        _cv.wait(lock,[this]{
                            return !_event_queue.empty()||_shutdown;
                        });
                        if (_shutdown){
                            return std::nullopt;
                        }
                        event_data evt_data=_event_queue.front();
                        _event_queue.pop();
                        return evt_data;
                    }();
                    if (!get_event_data){
                        break;
                    }
                    auto& [target, command, execution_mode]=*get_event_data;
                    const auto it=_executor_map.find(target);
                    if (it==_executor_map.end()){
                        continue; // Unknown target, skip it
                    }
                    auto future=it->second->submit(command,execution_mode);
                    if (execution_mode::sync==execution_mode) {
                        future.wait(); // Ensure sync behavior
                    }
                }

                for (auto& slave:_executor_map){
                    slave.second->shutdown();
                }
            });
        }

        ~event_synchronizer(){
            shutdown();
            if (_worker_thread.joinable()){
                _worker_thread.join();
            }
        }

        // Post a new event into the dispatcher queue
        void post(const event_data& cmd){
            if (_shutdown){
                return;
            }
            {
                std::lock_guard<std::mutex> lock(_mtx);
                _event_queue.push(cmd);
            }
            _cv.notify_one();
        }

        // Blocks until all events are fully processed
        // TODO : avoid polling mechanism with another approach
        void wait(){
            bool exit{false};
            while (!exit){
                std::unique_lock<std::mutex> lock(_wait_mtx);
                _wait_cv.wait_for(lock,std::chrono::microseconds(100),
                    [this,&exit]{ //polling mechanism to identify any events available
                    if (!_event_queue.empty()){
                        return false;
                    }
                    // check if all the active executor queue is empty.
                    for (auto& event_hndlr:_executor_map){
                        if (!event_hndlr.second->is_empty()){
                            return false;
                        }
                    }
                    return exit=true;
                });
            }
        }

        void shutdown(){
            _shutdown=true;
            _cv.notify_all(); // Unblock all wait
        }

    private:
        std::unordered_map<target_id,
             std::unique_ptr<i_executor>> _executor_map;

        std::thread _worker_thread;
        std::atomic<bool> _shutdown{false};

        std::mutex _mtx;
        std::condition_variable _cv;
        std::queue<event_data> _event_queue;

        std::mutex _wait_mtx;
        std::condition_variable _wait_cv;
    };
}