// #define MFC_VERSION
#ifdef MFC_VERSION
#define _AFXDLL
#include <iostream>
#include <afxmt.h>                                            // Step 1 : Include the header file <afxmt.h>

CCriticalSection g_csLock;                                    // Step 2 : Declare the CCriticalSection variable

void PerformOperationInThreads( const int nThreadCount_i );
void CriticalFunctionality( const int& nThreadIndex_i );
DWORD WINAPI ThreadFunction( PVOID pVoid );

int main(){
    PerformOperationInThreads( 6 );  // Create and start different thread that calls ThreadFunction().
    return 0;
}

DWORD WINAPI ThreadFunction( PVOID pVoid ){
    CSingleLock lock( &g_csLock, TRUE );                      // step 3 : Automatic CCriticalSection lock.

    CriticalFunctionality( *reinterpret_cast<int*>( pVoid )); // step 4 : Execute the synchronized code section.

    return 0;
}                                                             // Step 5: Execute step 3 to 6 for all threads.
#else

#include <iostream>
#include <Windows.h>                                          // Step 1 : Include the header file <windows.h>

CRITICAL_SECTION g_csLock;                                    // Step 2 : Declare the CRITICAL_SECTION variable


void PerformOperationInThreads( const int nThreadCount_i );
void CriticalFunctionality( const int& nThreadIndex_i );
DWORD WINAPI ThreadFunction( PVOID pVoid );

int main(){
    ::InitializeCriticalSection( &g_csLock );                 // Step 3 : Initialize CRITICAL_SECTION object.
    
    PerformOperationInThreads( 6 );  // Create and start different thread that calls ThreadFunction().

    ::DeleteCriticalSection( &g_csLock );                     // Step 8: Delete the CRITICAL_SECTION
    return 0;
}

DWORD WINAPI ThreadFunction( PVOID pVoid ){
    ::EnterCriticalSection( &g_csLock );                      // step 4 : Accquire the CRITICAL_SECTION lock.

    CriticalFunctionality( *reinterpret_cast<int*>( pVoid )); // step 5 : Execute the synchronized code section.

    ::LeaveCriticalSection( &g_csLock );                      // step 6 : Release the CRITICAL_SECTION lock.
    return 0;
}                                                             // Step 7: Execute step 4 to 6 for all threads.
#endif
void CriticalFunctionality( const int& nThreadIndex_i ){
    std::cout << "Thread Index : " << nThreadIndex_i << '\n';
    Sleep( 1000 );
}

void PerformOperationInThreads( const int nThreadCount_i )
{
    const int THREAD_COUNT  = nThreadCount_i;
    HANDLE* phThreadArray   = new HANDLE[THREAD_COUNT];
    int* pnThreadIndexArray = new int[THREAD_COUNT];
    for( int nIndex = 0; nIndex < THREAD_COUNT; ++nIndex ){
        pnThreadIndexArray[nIndex] = nIndex;
        phThreadArray[nIndex] = ::CreateThread( NULL, 0,
                                                reinterpret_cast<LPTHREAD_START_ROUTINE>( ThreadFunction ),
                                                reinterpret_cast<PVOID>( pnThreadIndexArray + nIndex ),
                                                CREATE_SUSPENDED, NULL );
    }
    for( int nIndex = 0; nIndex < THREAD_COUNT; ++nIndex ){
        ::ResumeThread( phThreadArray[nIndex] );
    }
    ::WaitForMultipleObjects( THREAD_COUNT, phThreadArray, TRUE, INFINITE );
    for( int nIndex = 0; nIndex < THREAD_COUNT; ++nIndex ){
        ::CloseHandle( phThreadArray[nIndex] );
    }
    delete[] pnThreadIndexArray;
}




