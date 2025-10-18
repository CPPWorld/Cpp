/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ContentHandler.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAX2ContentHandler.h"

namespace SAX2XML
{

    SAX2ContentHandler::SAX2ContentHandler( const std::wstring&          wstrFilterNode_i,
                                            const SAX2XMLOutputCallback& SAX2XMLOutputCallback_i,
                                            const std::any&              anyUserInfo_io ) :
                                                    m_wstrFilterNode( wstrFilterNode_i ),
                                                    m_SAX2XMLOutputCallback( SAX2XMLOutputCallback_i ),
                                                    m_anyUserInfo( anyUserInfo_io )
    {
        try
        {
            HRESULT hr = m_pWriter.CoCreateInstance( __uuidof( MXXMLWriter60 ), NULL );
            if( FAILED( hr ))
            {
                throw hr;
            }
            m_pWriter->put_standalone( TRUE );
            m_pWriter->put_omitXMLDeclaration( TRUE );
            // m_pWriter->put_indent( TRUE );

            // Attach the writer interface to content handler.
            // This allows creating the O/P from SAXXMLReader Interface events
            m_pISAXContentHandler = m_pWriter;

            const auto MAX_NODE_CHAR_LEN = 1024u;
            m_wszStartElement = std::make_unique<wchar_t[]>( MAX_NODE_CHAR_LEN + 1 );
            m_wszEndElement   = std::make_unique<wchar_t[]>( MAX_NODE_CHAR_LEN + 1 );
        }
        catch( ... )
        {
            throw;
        }
    }


    HRESULT STDMETHODCALLTYPE SAX2ContentHandler::startElement( const wchar_t*  pwszNamespaceUri_i,
                                                                const int       nNamespaceUri_i,
                                                                const wchar_t*  pwszLocalName_i,
                                                                const int       nLocalName_i,
                                                                const wchar_t*  pwszRawName_i,
                                                                const int       nRawName_i,
                                                                ISAXAttributes* pAttributes_i )
    {
        // Check whether to stop further processing.
        // Decided based on the return value of Callback method from client side.
        if( m_bEndXMLParse )
        {
            return E_ABORT;
        }

        if( m_stElementFilter.isEnded() && !m_wstrFilterNode.empty())
        {
            if( 0 == wmemcmp( m_wstrFilterNode.c_str(), pwszLocalName_i, nLocalName_i ))
            {
                // For each found node, Reset the output varaible and avoid the previously read content.
                m_ccomOutpPutVar.Clear();
                m_pWriter->put_output( m_ccomOutpPutVar );

                HRESULT hr = m_pISAXContentHandler->startDocument();
                if( FAILED( hr ))
                {
                    return hr;
                }
                m_stElementFilter.Start();
            }

            // Filter condition
            if( !m_stElementFilter.isStarted())
            {
                return S_OK;
            }
        }

        return m_pISAXContentHandler->startElement( pwszNamespaceUri_i,
                                                    nNamespaceUri_i,
                                                    pwszLocalName_i,
                                                    nLocalName_i,
                                                    pwszRawName_i,
                                                    nRawName_i,
                                                    pAttributes_i );
    }


    HRESULT STDMETHODCALLTYPE SAX2ContentHandler::characters( const wchar_t* pwszChars_i,
                                                              int            nChars_i )
    {
        if( !m_stElementFilter.isStarted())
        {
            return S_OK;
        }
        return m_pISAXContentHandler->characters( pwszChars_i, nChars_i );
    }


    HRESULT STDMETHODCALLTYPE SAX2ContentHandler::endElement( const wchar_t* pwszNamespaceUri_i,
                                                              const int      nNamespaceUri_i,
                                                              const wchar_t* pwszLocalName_i,
                                                              const int      nLocalName_i,
                                                              const wchar_t* pwszRawName_i,
                                                              const int      nRawName_i )
    {
        if( !m_stElementFilter.isStarted())
        {
            return S_OK;
        }
        HRESULT hr = m_pISAXContentHandler->endElement( pwszNamespaceUri_i,
                                                        nNamespaceUri_i,
                                                        pwszLocalName_i,
                                                        nLocalName_i,
                                                        pwszRawName_i,
                                                        nRawName_i );
        if( FAILED( hr ))
        {
            return hr;
        }

        // If there is no filter node specifed, the full XML is to be read as a single O/P.
        if( m_wstrFilterNode.empty())
        {
            return S_OK;
        }
        if( 0 == wmemcmp( m_wstrFilterNode.c_str(), pwszLocalName_i, nLocalName_i ))
        {
            m_stElementFilter.End();
        }

        if( m_stElementFilter.isEnded())
        {
            hr = m_pISAXContentHandler->endDocument();
            if( FAILED( hr ))
            {
                return hr;
            }
            ReadOutput();
        }
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAX2ContentHandler::startDocument()
    {
        if( !m_wstrFilterNode.empty())
        {
            return S_OK;
        }
        m_stElementFilter.Start();
        return m_pISAXContentHandler->startDocument();
    }


    HRESULT STDMETHODCALLTYPE SAX2ContentHandler::endDocument()
    {
        if( !m_wstrFilterNode.empty())
        {
            return S_OK;
        }

        m_stElementFilter.End();
        HRESULT hr = m_pISAXContentHandler->endDocument();
        if( !FAILED( hr ))
        {
            ReadOutput();
        }
        return hr;
    }


    void SAX2ContentHandler::ReadOutput()
    {
        // Get the output from IMXXMLWriter Interface
        m_pWriter->flush();
        m_pWriter->get_output( &m_ccomOutpPutVar );

        // Support for client callback to deliver the data to client
        if( m_SAX2XMLOutputCallback )
        {
            m_bEndXMLParse = !m_SAX2XMLOutputCallback( m_ccomOutpPutVar.bstrVal, m_anyUserInfo );
        }
    }
}