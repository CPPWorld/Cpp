#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2Client.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */
#include "SAX2XMLCallbacks.h"

#include <string> // std::wstring
#include <memory> // std::unique_ptr

namespace SAX2XML
{
    class SAX2Client
    {
    public:

        SAX2Client( const std::wstring&          wstrXMLFilePath_i,
                    const std::wstring&          wstrFilterNodeName_i,
                    const SAX2XMLOutputCallback& SAX2XMLOutputcallback_i,
                    const SAX2XMLErrorCallback&  SAX2XMLErrorcallback_i,
                    const std::any&              anyUserInfo_io );

        [[nodiscard]] bool ParseXML();


        //Rule of Five
        SAX2Client( const SAX2Client& )            = delete;
        SAX2Client( SAX2Client&& )                 = delete;
        SAX2Client& operator=( const SAX2Client& ) = delete;
        SAX2Client& operator=( SAX2Client&& )      = delete;
        SAX2Client()                               = delete;

        ~SAX2Client();

    private:

        // Opaque SAX2ClientImpl
        class SAX2ClientImpl;
        std::unique_ptr<SAX2ClientImpl>m_SAX2ClientImplUPtr;
    };
}