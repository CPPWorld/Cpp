#include "event_synchronizer.h"
#include <iostream>
#include <sstream>
#include <thread>
using namespace ns_event_synchronizer;

// Interface for subjects that respond to event triggers
class i_base {
public:
    virtual void start()=0;
    virtual void execute()=0;
    virtual void exit()=0;
    virtual ~i_base(){}
};

class derrived_a:public i_base{
public:
    derrived_a(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    ~derrived_a(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    void start()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
    void execute()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        std::cout << name_ << "[" << std::this_thread::get_id() << "] timeout" << __func__ << std::endl;
    };
    void postprocess(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void exit()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
private:
    std::string name_{"[derrived_a]"};
};

class derrived_b:public i_base{
public:
    derrived_b(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    ~derrived_b(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    }
    void start()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
    void execute()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void postprocess(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << name_ << "[" << std::this_thread::get_id() << "] timeout " << __func__ << std::endl;
    };
    void exit()override{
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__ << std::endl;
    };
private:
    std::string name_{"[derrived_b]"};
};
int g_number{};
class random_class{
public:

    void increment(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__  << " result=" << ++g_number << std::endl;
    }
private:
    std::string name_{"[random_class]"};
};

class random_class_2{
public:

    void decrement(){
        std::cout << name_ << "[" << std::this_thread::get_id() << "]" << __func__  << " result=" << --g_number << std::endl;
    }
private:
    std::string name_{"[random_class]"};

};

template<typename T>
std::shared_ptr<event<T>>  register_events(){
    auto evt=std::make_shared<event<T>>();
    evt->add("start",[cmd=evt->command()]{cmd->start();return true;});
    evt->add("execute",[cmd=evt->command()]{cmd->execute();return true;});
    evt->add("exit",[cmd=evt->command()]{cmd->exit();return true;});
    evt->add("postprocess",[cmd=evt->command()]{cmd->postprocess();return true;});

    evt->add("echo",[]{std::cout<<"-------------------Echo------------------\n";return true;});
    return evt;
}

std::shared_ptr<event<random_class>> register_random_events() {
    auto evt = std::make_shared<event<random_class>>();
    evt->add( "increment", [cmd = evt->command()](){ cmd->increment();return true;});
    return evt;
}

std::shared_ptr<event<random_class_2>> register_random_2_events() {
    auto evt = std::make_shared<event<random_class_2>>();
    evt->add( "decrement", [cmd = evt->command()](){ cmd->decrement();return true;});
    return evt;
}
class Test {
public:
    int Random(){
        {
            auto random_=register_random_events();
            auto random_2=register_random_2_events();

            auto evt_reg=std::make_shared<event_registry>();
            evt_reg->add("random_class", random_);
            evt_reg->add("random_class_2", random_2);

            ns_event_synchronizer::event_synchronizer synchronizer(std::move(evt_reg));

            for (int i=0;i<100;++i){
                synchronizer.post({ "random_class",  "increment",    execution_mode::sync });
                synchronizer.post({ "random_class_2",  "decrement",   execution_mode::sync });
            }
            for (int i=0;i<100;++i){
                synchronizer.post({ "random_class",  "increment",    execution_mode::sync });
            }

            for (int i=0;i<100;++i){
                synchronizer.post({ "random_class_2",  "decrement",   execution_mode::sync });
            }

            synchronizer.wait();

            synchronizer.shutdown();
        }
        return 0;
    };
    int derriveds(){
        auto der_a=register_events<derrived_a>();
        auto der_b=register_events<derrived_b>();

        auto evt_reg=std::make_shared<event_registry>();
        evt_reg->add("derrived_a",der_a);
        evt_reg->add("derrived_b",der_b);

        ns_event_synchronizer::event_synchronizer synchronizer(std::move(evt_reg));

        synchronizer.post({ "derrived_a", "echo",    execution_mode::sync });
        synchronizer.post({ "derrived_b", "echo",    execution_mode::sync });

        synchronizer.post({ "derrived_a", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "postprocess", execution_mode::sync });

        synchronizer.wait();

        synchronizer.post({ "derrived_a", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_a", "exit",    execution_mode::sync });
        synchronizer.wait();

        synchronizer.post({ "derrived_b",  "echo",   execution_mode::sync });

        //der_a->remove("execute");
        //der_b->remove("execute");

        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_a", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_a", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_a", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "start",   execution_mode::sync });
        synchronizer.post({ "derrived_b", "execute", execution_mode::sync });
        synchronizer.post({ "derrived_b", "echo",    execution_mode::sync });

        synchronizer.wait();

        synchronizer.shutdown();
        return 0;
    }
};

std::shared_ptr<event<Test>> register_random(){
    auto evt=std::make_shared<event<Test>>();
    evt->add("Random",[cmd=evt->command()](){cmd->Random();return true;});
    return evt;
}

std::shared_ptr<event<Test>> register_derriveds(){
    auto evt=std::make_shared<event<Test>>();
    evt->add("derriveds",[cmd = evt->command()](){cmd->derriveds();return true;});
    return evt;
}

int main(){
        auto der_a=register_random();
        auto der_b=register_derriveds();

        auto evt_reg=std::make_shared<event_registry>();
        evt_reg->add("Random",der_a);
        evt_reg->add("derriveds",der_b);

        ns_event_synchronizer::event_synchronizer synchronizer(std::move(evt_reg));
        synchronizer.post({"derriveds","derriveds",execution_mode::async});
        synchronizer.post({"Random","Random",execution_mode::async});
        synchronizer.wait();

        synchronizer.shutdown();
    return 0;
}