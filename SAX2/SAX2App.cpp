/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2App.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAX2Client.h" // SAX2Client

#include <any>          // std::any
#include <vector>       // std::vector
#include <iostream>     // std::wcout

// To end XML file parsing, return false from callback.
bool SAX2XMLOutputCallbackFunc( wchar_t* const pwszOutput_i, // Internal Memory, clients should not delete this. Scope only with in callback.
                                std::any& anyUserInfo_i );   // User Info for client specific useages.

bool SAX2XMLErrorCallbackFunc( const wchar_t* const pwszError_i, // Internal Memory, clients should not delete this. Scope only with in callback.
                               std::any& anyUserInfo_i );        // User Info for client specific useages.


// Sample user info
struct USER_INFO_st
{
    int nID = 0;

    USER_INFO_st() = default;
    explicit USER_INFO_st( int nID_i ) : nID( nID_i ){};
};


int main(int argc, char** pvArg)
{
    //if (argc < 2)
    //{
    //    return;
    //}
    std::any UserInfo = std::make_any<USER_INFO_st>( 1 );
    return ( SAX2XML::SAX2Client( L"E:\\Roy\\Official\\Office\\SID\\study\\SAX2\\Books.xml",
                                  L"price",
                                  SAX2XMLOutputCallbackFunc,
                                  SAX2XMLErrorCallbackFunc,
                                  UserInfo ).ParseXML()) ? 0 : 1;
}

std::vector<std::wstring>Vec;

// Return False to abort the furthe XML File parsing.
bool SAX2XMLOutputCallbackFunc( wchar_t* const wstrOutput_i,
                                std::any& anyUserInfo_i )
{
    static size_t uCount = 0u;
    if( anyUserInfo_i.has_value() )
    {
        // User Info for tracking purpose/custom implementation.
        const USER_INFO_st* pUserInfo =  std::any_cast<USER_INFO_st>( &anyUserInfo_i );
        if( nullptr != pUserInfo )
        {
            // Use it
        }
    }

    ++uCount;
    //if( 0 == uCount%10000 )
    //{
    //    std::wcout<< uCount << L"\n";
    //}
    std::wstring wstrOP = wstrOutput_i;
    std::wcout << wstrOutput_i << "\n";
    return true;
}


bool SAX2XMLErrorCallbackFunc( const wchar_t* const pwszError_i,
                               std::any& anyUserInfo_i )
{
    if( anyUserInfo_i.has_value() )
    {
        // User Info for tracking purpose/custom implementation.
        try
        {
            USER_INFO_st userInfo = std::any_cast<USER_INFO_st>( anyUserInfo_i );
        }
        catch( const std::bad_any_cast& eBadAnyCast )
        {
            std::wcout << eBadAnyCast.what() << "\n";
        }
    }
    std::wcout << pwszError_i;
    return true;
}