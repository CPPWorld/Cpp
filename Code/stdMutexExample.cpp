/* std::mutex example
   -------------------
   Purpose      : Print TWO different specific character, N times continuously from TWO threads
   Condition    : Characters SHOULD NOT GET MIXED UP. But Order of lines may vary
   Expected O/P :
   **************************************************
   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

   Wrong O/P    :
   $*$*$*$*$*$$*$$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$*$$*$$*$*$*$*$*$$*$*$*$*$*$*$******

*/
#include <iostream>
#include<thread>
#include<mutex>

std::mutex mtx;

auto PrintFunction( int nCount, char szCharacterToPrint )
{
    mtx.lock();
    for( auto i = 0; i < nCount ; ++i )
    {
        std::cout << szCharacterToPrint;
    }
    std::cout<< '\n';
    mtx.unlock();
}

int main(){

    std::thread Thread1( PrintFunction, 20, '*' );
    std::thread Thread2( PrintFunction, 20, '&' );

    Thread1.join();
    Thread2.join();

    return 0;
}