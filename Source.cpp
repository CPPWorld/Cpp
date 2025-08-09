#include "event_synchronizer.h"
#include <iostream>
#include <syncstream>
#include <sstream>
#include <thread>
using namespace roymathew::ns_event_synchronizer;

// Interface for subjects that respond to event triggers
class i_base {
public:
    virtual void start()=0;
    virtual void execute()=0;
    virtual void exit()=0;
    virtual ~i_base(){}
};

#define sync_cout std::osyncstream( std::cout)

class Test_A:public i_base{
public:
    Test_A(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    ~Test_A(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    void start()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
    void execute()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        sync_cout << name_ << "[" << std::this_thread::get_id() << "] timeout" << __func__ << std::endl;
    };
    void postprocess(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        sync_cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void exit()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
private:
    std::string name_{"[Test_A]"};
};

class Test_B:public i_base{
public:
    Test_B(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    ~Test_B(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    void start()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
    void execute()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        sync_cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void postprocess(){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        sync_cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void exit()override{
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
private:
    std::string name_{"[Test_B]"};
};

int g_number{};
class Test_C{
public:

    void increment(int v1, int v2){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__  << " result=" << (g_number+=(v1+v2)) << std::endl;
    }
private:
    std::string name_{"[Test_C]"};
};

class Test_D{
public:

    void decrement(int decrement_by){
        sync_cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__  << " result=" << (g_number-=decrement_by) << std::endl;
    }
private:
    std::string name_{"[Test_D]"};

};

class Tests {
    template<typename T>
    std::shared_ptr<event<T>> register_events(){
        auto evt=std::make_shared<event<T>>();
        evt->add("start",[cmd=evt->command()]{cmd->start();return true;});
        evt->add("execute",[cmd=evt->command()]{cmd->execute();return true;});
        evt->add("exit",[cmd=evt->command()]{cmd->exit();return true;});
        evt->add("postprocess",[cmd=evt->command()]{cmd->postprocess();return true;});

        evt->add("echo",[]{sync_cout<<"-------------------Echo------------------\n";return true;});
        return evt;
    }
    std::shared_ptr<event<Test_C>> register_Test_C() {
        auto evt = std::make_shared<event<Test_C>>();
        evt->add( "Case_1:increment", [cmd = evt->command()](){ cmd->increment(1,0);return true;});
        return evt;
    }

    std::shared_ptr<event<Test_D>> register_Test_D() {
        auto evt = std::make_shared<event<Test_D>>();
        evt->add( "Case_1:decrement", [cmd = evt->command()](){ cmd->decrement(1);return true;});
        return evt;
    }
public:

    int Test_1(){
        auto der_a=register_events<Test_A>();
        auto der_b=register_events<Test_B>();

        event_synchronizer synchronizer({
            {"Test_A",der_a},
            {"Test_B",der_b}
        });

        synchronizer.post({ "Test_A", "echo",    execution_mode::sync });
        synchronizer.post({ "Test_B", "echo",    execution_mode::sync });
        synchronizer.wait();

        synchronizer.post({ "Test_A", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "postprocess", execution_mode::sync });

        synchronizer.wait();

        synchronizer.post({ "Test_A", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_A", "exit",    execution_mode::sync });
        synchronizer.wait();
        synchronizer.removeEvent("Test_A");


        synchronizer.post({ "Test_B",  "echo",   execution_mode::sync });

        der_a->remove("execute");
        der_b->remove("execute");

        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_A", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });

        synchronizer.wait();
        synchronizer.removeEvent("Test_B");

        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_A", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_A", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "start",   execution_mode::sync });
        synchronizer.post({ "Test_B", "execute", execution_mode::sync });
        synchronizer.post({ "Test_B", "echo",    execution_mode::sync });
        synchronizer.post({ "Test_A", "exit", execution_mode::sync });
        synchronizer.wait();

        synchronizer.shutdown();
        return 0;
    }
    int Test_2(){
        {
            auto test_C=register_Test_C();
            auto test_D=register_Test_D();

            event_synchronizer synchronizer({
                {"Test_C", test_C},
                {"Test_D", test_D}
            });

            for (int i=0;i<100;++i){
                synchronizer.post({ "Test_C",  "Case_1:increment",    execution_mode::sync });
                synchronizer.post({ "Test_D",  "Case_1:decrement",   execution_mode::sync });


            }

            for (int i=0;i<100;++i){
                synchronizer.post({ "Test_D",  "Case_1:decrement",   execution_mode::sync });

            }
            synchronizer.wait();
            synchronizer.shutdown();
        }
        return 0;
    };
};

std::shared_ptr<event<Tests>> register_Test_1(){
    auto evt=std::make_shared<event<Tests>>();
    evt->add("Case_1",[cmd=evt->command()](){cmd->Test_1();return true;});
    return evt;
}

std::shared_ptr<event<Tests>> register_Test_2(){
    auto evt=std::make_shared<event<Tests>>();
    evt->add("Case_1",[cmd = evt->command()](){cmd->Test_2();return true;});
    return evt;
}


bool handler_A() {
    std::cout << "[handler_A] Executed.\n";
    return true;
}

bool handler_B() {
    std::cout << "[handler_B] Executed.\n";
    return true;
}

int main(){
    {
        auto test_1=register_Test_1();
        auto test_2=register_Test_2();

        event_synchronizer synchronizer({{"Test_1",test_1}, {"Test_2",test_2}});
        synchronizer.post({"Test_1","Case_1",execution_mode::async});
        synchronizer.post({"Test_2","Case_1",execution_mode::async});
        synchronizer.wait();

        synchronizer.shutdown();
    }
    {
        // Step 1: Create event handlers
        auto eventA = std::make_shared<event<>>();
        auto eventB = std::make_shared<event<>>();

        eventA->add("evt1", handler_A);
        eventB->add("evt2", handler_B);

        // Step 2: Register events
        event_registry entryA{ "targetA", eventA };
        event_registry entryB{ "targetB", eventB };

        event_synchronizer synchronizer{ entryA, entryB };

        // Step 3: Post async events
        synchronizer.post({ "targetA", "evt1", execution_mode::sync });
        synchronizer.post({ "targetB", "evt2", execution_mode::sync });

        // Step 4: Post sync event
        synchronizer.post({ "targetA", "evt1", execution_mode::sync });

        // Step 5: Wait until all processing completes
        synchronizer.wait();

        // Step 6: Remove and test ignored event
        synchronizer.removeEvent("targetA");
        synchronizer.post({ "targetA", "evt1", execution_mode::async });

        // Wait and shutdown
        synchronizer.wait();
        synchronizer.shutdown();
    }
    std::cout << "All tests completed.\n";

    return 0;
}