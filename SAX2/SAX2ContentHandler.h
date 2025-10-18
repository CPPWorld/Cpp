#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ContentHandler.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAXContentHandlerImpl.h"
#include "SAX2XMLCallbacks.h"

#include <string>      // std::wstring
#include <atlcomcli.h> // CComQIPtr
#include <memory>

namespace SAX2XML
{
    class SAX2ContentHandler : public SAXContentHandlerImpl
    {
    public:

        SAX2ContentHandler( const std::wstring&          wstrFilterNode_i,
                            const SAX2XMLOutputCallback& callback_i,
                            const std::any&              anyUserInfo_io );

        HRESULT STDMETHODCALLTYPE startElement( const wchar_t*  pwszNamespaceUri_i,
                                                const int       nNamespaceUri_i,
                                                const wchar_t*  pwszLocalName_i,
                                                const int       nLocalName_i,
                                                const wchar_t*  pwszRawName_i,
                                                const int       nRawName_i,
                                                ISAXAttributes* pAttributes_i ) override;

        HRESULT STDMETHODCALLTYPE characters( const wchar_t* pwszChars_i,
                                              const int      nChars_i ) override;

        HRESULT STDMETHODCALLTYPE endElement( const wchar_t* pwszNamespaceUri,
                                              const int      nNamespaceUri_i,
                                              const wchar_t* pwszLocalName,
                                              const int      nLocalName_i,
                                              const wchar_t* pwszRawName,
                                              const int      nRawName_i ) override;

        HRESULT STDMETHODCALLTYPE startDocument() override;

        HRESULT STDMETHODCALLTYPE endDocument() override;

        SAX2ContentHandler() = delete;

        // Rule of Five
        SAX2ContentHandler( const SAX2ContentHandler& )             = delete;
        SAX2ContentHandler( SAX2ContentHandler&& )                  = delete;
        SAX2ContentHandler& operator=( const SAX2ContentHandler& )  = delete;
        SAX2ContentHandler& operator=( SAX2ContentHandler&& )       = delete;

        // Everything is wrapped by smart classes, so nothing to do in dtor.
        ~SAX2ContentHandler() = default;

    private:

        void ReadOutput();

    private:

        CComQIPtr<IMXWriter>          m_pWriter;
        CComQIPtr<ISAXContentHandler> m_pISAXContentHandler;

        SAX2XMLOutputCallback m_SAX2XMLOutputCallback;

        std::any m_anyUserInfo;

        // Holds the start node to be used for filter the XML
        std::wstring m_wstrFilterNode;

        // Used for reusing the memory for checking the Start and End Elements.
        std::unique_ptr<wchar_t[]> m_wszStartElement;
        std::unique_ptr<wchar_t[]> m_wszEndElement;

        // Used to end the XML parsing based on the return of callback from client
        bool m_bEndXMLParse = false;

    private:

        struct ELEMENT_FILTER_st
        {
            void Start()
            {
                m_bStart = true;
                m_bEnd   = false;
            }
            void End()
            {
                m_bStart = false;
                m_bEnd   = true;
            }
            bool isStarted() const { return m_bStart; }
            bool isEnded()   const { return m_bEnd;   }

        private:

            bool m_bStart = false;
            bool m_bEnd   = true;
        };
        ELEMENT_FILTER_st m_stElementFilter;
        CComVariant       m_ccomOutpPutVar;
    };
}