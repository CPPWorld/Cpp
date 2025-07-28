#pragma once
#include <queue>
#include <thread>
#include <memory>
#include <atomic>
#include <string>
#include <future>
#include <optional>
#include <functional>
#include <unordered_map>
#include <condition_variable>

namespace ns_event_synchronizer{
    // Alias for event identifiers
    using event_id=std::string;

    // execution mode.
    enum class execution_mode{invalid=-1,async=0,sync};

    // Interface for event wrappers that define execution logic
    class i_event{
    public:
        // Execute handler for given event type
        virtual bool execute(const event_id&)=0;

        // Register callback for event type
        virtual void add(const event_id&,
            std::function<bool()>)=0;
        virtual void remove(const event_id& evt)=0;
        virtual ~i_event(){}
    };

    // Generic event class holding command data and event handlers
    template<typename T = void>
    class event:public i_event{
    public:
        event(){
            if constexpr (!std::is_void<T>::value) {
                cmd = std::make_shared<T>();
            }
        }
        ~event()override=default;

        // Execute the corresponding handler based on event type
        bool execute(const event_id& evt)override{
            std::lock_guard<std::mutex> lock(_mtx);
            auto it=handlers.find(evt);
            return it!=handlers.end()?it->second():false;
        }

        template<typename U = T>
        std::enable_if_t<!std::is_void<U>::value, std::shared_ptr<U>> command() {
            return cmd;
        }

        // Add handler for event type
        void add(const event_id& evt,
            std::function<bool()> handler)override{
            std::lock_guard<std::mutex> lock(_mtx);
            handlers[evt]=std::move(handler);
        }
        
        // Remove handler for event type
        void remove(const event_id& evt)override{
            std::lock_guard<std::mutex> lock(_mtx);
            handlers.erase(evt);
        }

    private:
         // Command payload object. Used during execute() call.
        using Cmd = std::conditional_t<std::is_void<T>::value, std::nullptr_t, std::shared_ptr<T>>;
        Cmd cmd;


        std::mutex _mtx;
        std::unordered_map<event_id,std::function<bool()>> handlers;
    };

    using target=std::string;

    // Structure to package data needed for an event execution
    struct event_handler_data{
        event_id evt_id;
        execution_mode execution_mode{execution_mode::invalid};

         // Used for signaling in sync mode
        std::optional<std::promise<void>> sync_promise;
    };

    // Interface for event executor
    class i_executor{
    public:
        virtual std::future<void> submit(event_id,execution_mode)=0;
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
                    // Run event
                    if(!event->execute(data.evt_id)){
                        // log error
                        // or what to do?
                    }
                    if ((execution_mode::sync==data.execution_mode)&&
                        (data.sync_promise.has_value())){
                        data.sync_promise->set_value();
                    }
                }while(true);
            });
        }

        ~executor(){
            if (_thread.joinable()){
                _thread.join();
            }
        }

        // Posts a new event to this handler's queue
        std::future<void> submit(event_id event_id,
            execution_mode execution_mode)override{
            std::optional<std::promise<void>> promise;
            std::future<void> future;
            if (execution_mode::sync==execution_mode){
                promise.emplace();
                // Capture future for sync execution
                future=promise->get_future();
            }
            {
                std::lock_guard<std::mutex> lock(_mtx);
                _queue.push({event_id,execution_mode,
                    std::move(promise)});
            }
            // Wake thread
            _cv.notify_one();
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
        std::atomic_bool _shutdown{false};
        std::queue<event_handler_data> _queue;
        std::mutex _mtx;
        std::condition_variable _cv;
    };

    struct event_registry{
        target id;
        std::shared_ptr<ns_event_synchronizer::i_event> handler;
    };

    // Manages all event handlers and dispatching logic
    class event_synchronizer{
    // Struct for dispatching events across targets
    struct event_data{
        target _target;
        event_id _type;
        execution_mode _execution_mode{execution_mode::invalid};
    };
    public:
        event_synchronizer(std::initializer_list<event_registry> entries){
            for (const auto& [id, handler]:entries){
                _executor_map.emplace(id,
                    std::make_shared<executor>(handler));
            }
            _worker_thread=std::thread([this](){
                while (true){
                    auto get_event_data=[this]()->std::optional<event_data>{
                        std::unique_lock<std::mutex> lock(_mtx);
                        _cv.wait(lock,[this]{
                            return !_event_queue.empty()||_shutdown.load();
                        });
                        if (_shutdown.load()){return std::nullopt;}
                        event_data evt_data=_event_queue.front();
                        _event_queue.pop();
                        return evt_data;
                    }();
                    if (!get_event_data){
                        break;
                    }
                    std::shared_ptr<i_executor> executor;
                    auto& [target, command, execution_mode]=*get_event_data;
                    {
                        std::lock_guard<std::mutex>lock(_executor_map_mtx);
                        const auto it=_executor_map.find(target);
                        if (it==_executor_map.end()){
                            continue; // Unknown target, skip it
                        }
                        executor = it->second;
                    }
                    auto future=executor->submit(command,execution_mode);
                    if (execution_mode::sync==execution_mode) {
                        // Ensure sync behavior
                        future.wait();
                    }
                }
                {
                    std::lock_guard<std::mutex>lock(_executor_map_mtx);
                    for (auto& executor:_executor_map){
                        executor.second->shutdown();
                    }
                }
            });
        }

        void addEvent(event_registry entry){
            {
                std::lock_guard<std::mutex>lock(_executor_map_mtx);
                _executor_map.emplace(entry.id,
                    std::make_unique<executor>(entry.handler));
            }
        }

        void removeEvent(target id){
            {
                std::lock_guard<std::mutex>lock(_executor_map_mtx);
                if( auto it = _executor_map.find(id);!(it == _executor_map.end())){
                    it->second->shutdown();
                    _executor_map.erase(id);
                }
            }
        }

        ~event_synchronizer(){
            shutdown();
            if (_worker_thread.joinable()){
                _worker_thread.join();
            }
        }

        // Post a new event into the dispatcher queue
        void post(const event_data& cmd){
            if (_shutdown.load()){
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
                 // polling mechanism to identify any events available
                _wait_cv.wait_for(lock,std::chrono::microseconds(100),
                    [this,&exit]{
                    if (!_event_queue.empty()){
                        return false;
                    }
                    {   // check if all the active executor queue is empty.
                        std::lock_guard<std::mutex>lock(_executor_map_mtx);
                        for (auto& event_hndlr:_executor_map){
                            if (!event_hndlr.second->is_empty()){
                                return false;
                            }
                        }
                    }
                    return exit=true;
                });
            }
        }

        void shutdown(){
            _shutdown.store( true );

             // Unblock all wait
            _cv.notify_all();
        }

    private:
         // One Executor/target
        std::mutex _executor_map_mtx;
        std::unordered_map<target,
            std::shared_ptr<i_executor>> _executor_map;

        // for gracefull shutdown
        std::thread _worker_thread;
        std::atomic_bool _shutdown{false};

        std::mutex _mtx;
        std::condition_variable _cv;
        std::queue<event_data> _event_queue;

        std::mutex _wait_mtx;
        std::condition_variable _wait_cv;
    };
}