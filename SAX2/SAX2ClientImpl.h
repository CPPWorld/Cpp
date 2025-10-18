#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ClientImpl.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAX2Client.h"
#include "SAX2ContentHandler.h"
#include "SAX2ErrorHandlerImpl.h"

#include <atlcomcli.h> // CComQIPtr
#include <memory>      // std::unique_ptr

namespace SAX2XML
{
    class SAX2Client :: SAX2ClientImpl
    {

    public:

        SAX2ClientImpl( const std::wstring&          wstrXMLFilePath_i,
                        const std::wstring&          wstrFilterNodeName_i,
                        const SAX2XMLOutputCallback& SAX2XMLOutputcallback_i,
                        const SAX2XMLErrorCallback&  SAX2XMLErrorcallback_i,
                        const std::any&              anyUserInfo_io );

        [[nodiscard]] bool ParseXML();

    private:

        std::unique_ptr<SAX2ContentHandler> m_SAX2ContentHandlerUPtr;
        std::unique_ptr<SAX2ErrorHandlerImpl> m_SAX2ErrorHandlerImplUPtr;

        CComQIPtr<ISAXXMLReader> m_pISAXXMLReader;

        // Copy of user Info is created.
        std::any m_anyUserInfo;

        // User Inputs
        std::wstring m_wstrFilterNode;
        std::wstring m_wstrXMLFile;
    };
}