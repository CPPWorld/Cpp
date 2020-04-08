#include <thread>
#include <mutex>
#include <iostream>
 
int nThreadSharedVaraible = 0;
std::mutex g_mutexObject;
 
void safe_increment() {
    const std::lock_guard<std::mutex> lockGaurdObject( g_mutexObject );
    for( auto i = 0; i < 1000; ++i ){
        ++nThreadSharedVaraible;
        std::cout << nThreadSharedVaraible << '\n';
    }
} 

int main() {
    std::cout << "main: " << nThreadSharedVaraible << '\n'; 
    std::thread Thread1( safe_increment );
    std::thread Thread2( safe_increment ); 
    Thread1.join();
    Thread2.join(); 
    std::cout << "main: " << nThreadSharedVaraible << '\n';
    return 0;
}

