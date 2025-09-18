# Cpp
C++ Event Synchronization System
Overview
This repository contains a robust and modern C++ event synchronization and dispatching system. It is designed to manage event-driven tasks across multiple threads, ensuring data consistency and thread safety. The core of its design is a thread-per-target architecture, where each logical "target" (e.g., a specific database connection or a file stream) is assigned a dedicated worker thread to process all its events sequentially.

This project is built entirely on the C++20 Standard Library, without any third-party dependencies.

Key Features
Asynchronous & Synchronous Event Execution: Supports both fire-and-forget asynchronous tasks and synchronous tasks that block until a result is available.

Thread-per-Target Model: Guarantees sequential execution of events for the same target, naturally preventing race conditions and simplifying event handlers.

Graceful Shutdown: All threads are cooperatively joined on program exit, ensuring a clean and safe shutdown process.

Robust Exception Handling: Events are executed within try-catch blocks, preventing a single event's failure from crashing the entire worker thread.

Resource Management: Leverages std::jthread for automatic thread joining (RAII), eliminating a common source of bugs.

Efficient Synchronization: Uses std::shared_mutex for concurrent read access to the executor map and std::condition_variable to avoid busy-waiting.

Architecture
The system consists of two primary components: a central dispatcher and a pool of executors.

event_synchronizer: This is the main dispatcher. It receives all incoming events and queues them. It then dispatches each event to the correct executor thread based on the event's unique target ID.

executor: A dedicated worker thread that continuously processes events from its own private queue. Each executor is responsible for handling all events for a single target, ensuring sequential execution.

# How to Use
# 1. Define an Event Handler
First, create an event handler for your specific task. In this example, we'll create a simple event handler for file operations.

## C++

#include "event_synchronizer.h" // Assuming the code is in this header

#include "event_synchronizer.h" 
#include <iostream>
#include <syncstream>
#include <sstream>
#define sync_cout std::osyncstream( std::cout)

/// The main event handler class for our target

    class FileHandler : public roymathew::ns_event_synchronizer::eventIF {
    public:
        FileHandler() = default;

    bool execute(const roymathew::ns_event_synchronizer::index id,
                 const roymathew::ns_event_synchronizer::event_id& evt_id) override {
        // Here, you would put the actual business logic for a file event
        if (evt_id == "OPEN_FILE") {
            // Logic to open a file
            sync_cout << "Thread " << std::this_thread::get_id() << ": Opening file..." << std::endl;
        } else if (evt_id == "WRITE_DATA") {
            // Logic to write data
            sync_cout << "Thread " << std::this_thread::get_id() << ": Writing data to file..." << std::endl;
        }else if (evt_id == "READ_DATA") {
            // Logic to read data
            sync_cout << "Thread " << std::this_thread::get_id() << ": Reading data from file..." << std::endl;
        } else if (evt_id == "CLOSE_FILE") {
            // Logic to close data
            sync_cout << "Thread " << std::this_thread::get_id() << ": Closing file..." << std::endl;
        } else {
            return false;
        }
        return true;
    }

    void add(const roymathew::ns_event_synchronizer::event_id&, roymathew::ns_event_synchronizer::callback) override {}
    void remove(const roymathew::ns_event_synchronizer::event_id& evt) override {}
};

# 2. Initialize the Synchronizer and Post Events
In your main function, you initialize the event_synchronizer with your event handlers and then post events.

C++


    int main() {
    using namespace roymathew::ns_event_synchronizer;

    // Initialize the synchronizer with an event handler for the "file_target"
    auto fileHandler =  std::make_shared<FileHandler>();
    event_synchronizer es({
        {"Open",fileHandler},
        {"Write", fileHandler},
        {"Read1", fileHandler}, // Reader 1
        {"Read2", fileHandler}, // Reader 2
        {"Read3", fileHandler}, // Reader 3
        {"Close", fileHandler}

    });

    // --- Synchronous Events ---
    es.post({"Open", "OPEN_FILE",   execution_mode::sync});
    es.post({"Write", "WRITE_DATA", execution_mode::sync});
    es.post({"Read1", "READ_DATA",   execution_mode::sync});
    es.post({"Write", "WRITE_DATA", execution_mode::sync});
    es.post({"Read2", "READ_DATA",   execution_mode::sync});
    es.post({"Write", "WRITE_DATA", execution_mode::sync});

     // --- Asynchronous Events ---
    es.post({"Read1", "READ_DATA",   execution_mode::async});
    es.post({"Read2", "READ_DATA",   execution_mode::async});
    es.post({"Read3", "READ_DATA",   execution_mode::async});
 
    // --- Synchronous Events ---
    es.post({"Read1", "READ_DATA",   execution_mode::sync});
    es.post({"Close", "CLOSE_FILE", execution_mode::sync});

    // Graceful shutdown on application exit
    es.wait();
    es.shutdown();

    return 0;
}
# Dependencies
This project uses only the C++ Standard Library (C++20). A C++20-compliant compiler (like GCC 10+, Clang 11+, or MSVC 19.29+) is required to compile the code.

# License
This code is provided under the MIT License. You are free to use, modify, and distribute it for personal and commercial projects.