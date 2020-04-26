#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <map>
// #include <Windows.h>

struct IntType{
    int m_Count = 0;
};

template<typename T>
class ThreadSafeDataProxy {
public:
    ThreadSafeDataProxy( std::mutex* pMutexLock_io, T* pIntCounter_io ) : m_pMutexLock{ pMutexLock_io },
                                                                          m_pData{ pIntCounter_io } {
        m_pMutexLock->lock();
        std::cout << "Locked" << " TID:" << std::this_thread::get_id() << "\t Lock Object : " << m_pMutexLock;
        // MessageBox( NULL, L"Locked", L"Locked", MB_OK );
    }
    T* operator->() {
        std::cout << "\tData Access" << " TID:" << std::this_thread::get_id() << "";
        return m_pData;
    }
    ~ThreadSafeDataProxy() {
        std::cout << "\tUnLocked" << " TID:" << std::this_thread::get_id() << "\t Lock Object : " << m_pMutexLock <<"\n";
        //m_pMutexLock->unlock();
    }

private:
    T* m_pData               = nullptr;
    std::mutex* m_pMutexLock = nullptr;
};

template<typename T>
class ThreadSafeData {
public:
    ThreadSafeData(){}
    ThreadSafeDataProxy<T> operator->() {
        return ThreadSafeDataProxy<T>( &m_mutexLock, &m_Data );
    }
    ~ThreadSafeData(){}

    ThreadSafeData( const ThreadSafeData& ThreadSafeData_i )
    {
        this->m_Data = ThreadSafeData_i.m_Data;
    }
    ThreadSafeData& operator=( const ThreadSafeData& ThreadSafeData_i )
    {
        if( this != &ThreadSafeData_i ){
            this->m_Data = ThreadSafeData_i.m_Data;
        }
        return *this;
    }

private:
    T m_Data;
    std::mutex m_mutexLock;
};

static ThreadSafeData<IntType>            g_SharedIntData;
static ThreadSafeData<std::vector<int>>   g_ThreadSafeVector;
static ThreadSafeData<std::map<int,int>>  g_ThreadSafeMap;

void DoSomethingAsThread( const int nTID_i, const int nSleepTime_i )
{
    for( auto nIndex = nTID_i; nIndex <= nSleepTime_i; ++nIndex ) {
       // ++g_SharedIntData->m_Count;                    // integer variable
       // g_ThreadSafeMap->emplace( nIndex, nIndex*10 ); // std::map
       g_ThreadSafeVector->push_back( nIndex );          // std::vector
    }
}

int main() {
    std::thread Thread1( DoSomethingAsThread, 1, 10 );
    std::thread Thread2( DoSomethingAsThread, 2, 50 );
    Thread1.join();
    Thread2.join();
    return 0;
}