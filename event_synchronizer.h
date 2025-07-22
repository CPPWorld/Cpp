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
        event():cmd(std::make_shared <T>()){}
        event(std::initializer_list<std::pair<event_type,
            std::function<bool()>>> evts)
            :cmd(std::make_shared <T>()){
            for (const auto& [type, handler] : evts) {
                handlers.emplace(type,handler);
            }
        }
        ~event()override=default;

        // Execute the corresponding handler based on event type
        bool execute(const event_type& evt)override{
            std::lock_guard<std::mutex> lock(_mtx);
            auto it=handlers.find(evt);
            return it!=handlers.end()?it->second():false;
        }

        inline std::shared_ptr<T> command(){ return cmd; }

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
        std::atomic_bool _shutdown{false};
        std::queue<event_handler_data> _queue;
        std::mutex _mtx;
        std::condition_variable _cv;
    };

    struct registry_entry {
        target_id id;
        std::shared_ptr<ns_event_synchronizer::i_event> handler;
    };

    // Manages all event handlers and dispatching logic
    class event_synchronizer{
    struct event_data{ // Struct for dispatching events across targets
        target_id _target;
        event_type _type;
        execution_mode _execution_mode{execution_mode::invalid};
    };
    public:
        event_synchronizer(std::initializer_list<registry_entry> entries){
            for (const auto& [id, handler]:entries){
                _executor_map.emplace(id,
                    std::make_unique<executor>(handler));
            }
            _worker_thread=std::thread([this](){
                while (true){
                    auto get_event_data=[this]()->std::optional<event_data>{
                        std::unique_lock<std::mutex> lock(_mtx);
                        _cv.wait(lock,[this]{
                            return !_event_queue.empty()||_shutdown.load();
                        });
                        if (_shutdown.load()){ return std::nullopt; }
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
                        future.wait(); // Ensure sync behavior
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

        void addEvent(registry_entry entry){
            {
                std::lock_guard<std::mutex>lock(_executor_map_mtx);
                _executor_map.emplace(entry.id,
                    std::make_unique<executor>(entry.handler));
            }
        }

        void removeEntry(target_id id){
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
        void wait(){ // TODO : avoid polling mechanism with another approach
            bool exit{false};
            while (!exit){
                std::unique_lock<std::mutex> lock(_wait_mtx);
                _wait_cv.wait_for(lock,std::chrono::microseconds(100),
                    [this,&exit]{ // polling mechanism to identify any events available
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
            _cv.notify_all(); // Unblock all wait
        }

    private:
        std::mutex _executor_map_mtx;
        std::unordered_map<target_id,
            std::shared_ptr<i_executor>> _executor_map; // One Executor/target

        std::thread _worker_thread; // for gracefull shutdown
        std::atomic_bool _shutdown{false};

        std::mutex _mtx;
        std::condition_variable _cv;
        std::queue<event_data> _event_queue;

        std::mutex _wait_mtx;
        std::condition_variable _wait_cv;
    };
}