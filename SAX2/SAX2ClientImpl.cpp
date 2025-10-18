/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ClientImpl.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAX2ClientImpl.h"
#include "SAX2ContentHandler.h"

#include <MsXml6.h>

#import "msxml6.dll" raw_interfaces_only, named_guids  exclude( "_FILETIME", "ISequentialStream" )

namespace SAX2XML
{

    SAX2Client::SAX2ClientImpl::SAX2ClientImpl( const std::wstring&          wstrXMLFilePath_i,
                                                const std::wstring&          wstrFilterNodeName_i,
                                                const SAX2XMLOutputCallback& SAX2XMLOutputcallback_i,
                                                const SAX2XMLErrorCallback&  SAX2XMLErrorcallback_i ,
                                                const std::any&              anyUserInfo_io ) :
                                                m_wstrXMLFile( wstrXMLFilePath_i ),
                                                m_wstrFilterNode( wstrFilterNodeName_i ),
                                                m_anyUserInfo( anyUserInfo_io )
    {
        try
        {
            HRESULT hr = m_pISAXXMLReader.CoCreateInstance( __uuidof( SAXXMLReader60 ), NULL);
            if( FAILED( hr ))
            {
                throw hr;
            }
            m_SAX2ContentHandlerUPtr = std::make_unique<SAX2ContentHandler>( wstrFilterNodeName_i,
                                                                             SAX2XMLOutputcallback_i,
                                                                             m_anyUserInfo );

            m_SAX2ErrorHandlerImplUPtr = std::make_unique<SAX2ErrorHandlerImpl>( SAX2XMLErrorcallback_i,
                                                                                 m_anyUserInfo );

            m_pISAXXMLReader->putContentHandler( m_SAX2ContentHandlerUPtr.get());
            m_pISAXXMLReader->putErrorHandler( m_SAX2ErrorHandlerImplUPtr.get());
        }
        catch( ... )
        {
            throw;
        }
    }


    [[nodiscard]] bool SAX2Client::SAX2ClientImpl::ParseXML()
    {
        return FAILED( m_pISAXXMLReader->parseURL( m_wstrXMLFile.c_str())) ? false : true;
    }
}