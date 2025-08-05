#pragma once
#include <queue>
#include <thread>
#include <memory>
#include <atomic>
#include <string>
#include <future>
#include <optional>
#include <functional>
#include <shared_mutex>
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
        virtual void add(const event_id&,std::function<bool()>)=0;
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
        bool execute(const event_id& evt) override{
            std::lock_guard<std::mutex> lock(_event_umap_mtx);
            auto it=_event_umap.find(evt);
            return it!=_event_umap.end()?it->second():false;
        }

        template<typename U = T>
        std::enable_if_t<!std::is_void<U>::value, std::shared_ptr<U>> command() const{
            return cmd;
        }

        // Add handler for event type
        void add(const event_id& evt,std::function<bool()> func)override{
            std::lock_guard<std::mutex> lock(_event_umap_mtx);
            _event_umap[evt]=std::move(func);
        }
        
        // Remove handler for event type
        void remove(const event_id& evt)override{
            std::lock_guard<std::mutex> lock(_event_umap_mtx);
            _event_umap.erase(evt);
        }

    private:
         // Command payload object. Used during execute() call.
        using Cmd = std::conditional_t<std::is_void<T>::value,
            std::nullptr_t, std::shared_ptr<T>>;
        Cmd cmd;

        std::mutex _event_umap_mtx;
        std::unordered_map<event_id,std::function<bool()>> _event_umap;
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
        virtual void shutdown()noexcept=0;
        virtual bool is_empty()noexcept=0;
        virtual ~i_executor()=default;
    };

    // Concrete executor for queued event processing per target
    class executor:public i_executor{
    public:
        explicit executor(std::shared_ptr<i_event> evt){
            // One execetor/item in event registry.
            _executor_thread=std::thread([event=std::move(evt),this](){
                if (!event){
                    return;
                }
                do{
                    event_handler_data data;
                    {
                        std::unique_lock<std::mutex> lock(_evt_hndlr_mtx);
                         // Block until an event arrives or shutdown
                        _cv.wait(lock,[this]{
                            return !_evt_hndlr_q.empty()||_shutdown;
                        });
                        if (_shutdown.load()){
                            break;
                        }
                        data=std::move(_evt_hndlr_q.front());
                        _evt_hndlr_q.pop();
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
            if (_executor_thread.joinable()){
                _executor_thread.join();
            }
        }

        // Posts a new event to this handler's queue
        std::future<void> submit(event_id event_id,
            execution_mode execution_mode) override{
            std::optional<std::promise<void>> promise;
            std::future<void> future;
            if (execution_mode::sync==execution_mode){
                promise.emplace();
                // Capture future for sync execution
                future=promise->get_future();
            }
            {
                std::lock_guard<std::mutex> lock(_evt_hndlr_mtx);
                _evt_hndlr_q.push({event_id,execution_mode,
                    std::move(promise)});
            }
            // Wake thread
            _cv.notify_one();
            return future;
        }

        bool is_empty()noexcept override{
            std::unique_lock<std::mutex> lock(_evt_hndlr_mtx);
            return _evt_hndlr_q.empty();
        }

        void shutdown()noexcept override{
            _shutdown=true;
            _cv.notify_all();
        }

    private:
        std::condition_variable _cv;
        std::queue<event_handler_data> _evt_hndlr_q;
        std::mutex _evt_hndlr_mtx;
        std::atomic_bool _shutdown{false};
        std::thread _executor_thread;
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
                _executor_umap.emplace(id,std::make_shared<executor>(handler));
            }
            _synchronizer_thread=std::thread([this](){
                while (true){
                    auto get_event_data=[this]()->std::optional<event_data>{
                        std::unique_lock<std::mutex> lock(_event_queue_mtx);
                        _event_cv.wait(lock,[this]{
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
                        std::shared_lock lock(_executor_umap_mtx);
                        if (const auto it=_executor_umap.find(target);
                            it!=_executor_umap.end()){
                            executor = it->second;
                        }
                        else{
                            continue; // Unknown target, skip it
                        }
                    }
                    if (auto future=executor->submit(command,execution_mode);
                        execution_mode::sync==execution_mode) {
                        // Ensure sync behavior
                        future.wait();
                    }
                }
                // Shutting down
                for (std::shared_lock lock(_executor_umap_mtx);
                    auto& executor:_executor_umap){
                    executor.second->shutdown();
                }
            });
        }

        void addEvent(event_registry entry){
            if (is_alive())
            {
                std::unique_lock lock(_executor_umap_mtx);
                _executor_umap.emplace(entry.id,
                    std::make_shared<executor>(entry.handler));
            }
        }

        void removeEvent(target id){
            if (is_alive())
            {
                std::unique_lock lock(_executor_umap_mtx);
                if( auto it = _executor_umap.find(id);
                    !(it == _executor_umap.end())){
                    it->second->shutdown();
                    _executor_umap.erase(id);
                }
            }
        }

        ~event_synchronizer(){
            shutdown();
            if (_synchronizer_thread.joinable()){
                _synchronizer_thread.join();
            }
        }

        // Post a new event into the dispatcher queue
        void post(const event_data& cmd){
            if (is_alive())
            {
                std::lock_guard<std::mutex> lock(_event_queue_mtx);
                _event_queue.push(cmd);
            }
            _event_cv.notify_one();
        }

        // Blocks until all events are fully processed
        // TODO : avoid polling mechanism with another approach
        void wait(){
            bool exit{false};
            while (!exit){
                std::unique_lock<std::mutex> lock(_wait_mtx);
                // polling mechanism to identify any events available
                _wait_cv.wait_for(lock,std::chrono::milliseconds(100),
                    [this,&exit]{
                    if (std::lock_guard<std::mutex> lock(_event_queue_mtx);
                        !_event_queue.empty()){
                        return false;
                    }
                    // check if all the active executor queue is empty.
                    for (std::shared_lock lock(_executor_umap_mtx);
                        auto& executor:_executor_umap){
                        if (!executor.second->is_empty()){
                            return false;
                        }
                    }
                    return exit=true;
                });
            }
        }

        void shutdown() noexcept{
            _shutdown.store( true );
            // Unblock all wait
            _event_cv.notify_all();
            _wait_cv.notify_all();
        }

        inline bool is_alive() const noexcept{
            return !_shutdown.load();
        }

    private:
        // One Executor/target
        std::unordered_map<target,std::shared_ptr<i_executor>> _executor_umap;
        // std::shared_mutex to allow parallel read access in wait() & _synchronizer_thread
        std::shared_mutex _executor_umap_mtx;

        // for gracefull shutdown
        std::atomic_bool _shutdown{false};
        std::thread _synchronizer_thread;

        std::queue<event_data> _event_queue;
        std::mutex _event_queue_mtx;
        std::condition_variable _event_cv;

        std::mutex _wait_mtx;
        std::condition_variable _wait_cv;
    };
}