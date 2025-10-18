/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2Client.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAX2ClientImpl.h"
#include "SAX2ContentHandler.h"

namespace SAX2XML
{
    SAX2Client::SAX2Client( const std::wstring&          wstrXMLFilePath_i,
                            const std::wstring&          wstrFilterNodeName_i,
                            const SAX2XMLOutputCallback& SAX2XMLOutputcallback_i,
                            const SAX2XMLErrorCallback&  SAX2XMLErrorcallback_i,
                            const std::any&              anyUserInfo_io )
    {
        ::CoInitialize( NULL );
        try
        {
            m_SAX2ClientImplUPtr = std::make_unique<SAX2ClientImpl>( wstrXMLFilePath_i,
                                                                     wstrFilterNodeName_i,
                                                                     SAX2XMLOutputcallback_i ,
                                                                     SAX2XMLErrorcallback_i,
                                                                     anyUserInfo_io );
        }
        catch( ... )
        {
            ::CoUninitialize();
            throw;
        }
    }


    [[nodiscard]] bool SAX2Client::ParseXML()
    {
        return m_SAX2ClientImplUPtr->ParseXML();
    }


    SAX2Client::~SAX2Client()
    {
        // Get rid of the COM smart pointers before the CoUninitialize;
        m_SAX2ClientImplUPtr.reset();

        ::CoUninitialize();
    }
}