#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAXContentHandlerImpl.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include <MsXml6.h>

namespace SAX2XML
{
    class SAXContentHandlerImpl : public ISAXContentHandler
    {
    public:

        SAXContentHandlerImpl()  = default;
        ~SAXContentHandlerImpl() = default;

        HRESULT STDMETHODCALLTYPE putDocumentLocator( ISAXLocator* ) override;

        HRESULT STDMETHODCALLTYPE startDocument() override;

        HRESULT STDMETHODCALLTYPE endDocument() override;
 
        HRESULT STDMETHODCALLTYPE startPrefixMapping( const wchar_t*,
                                                      const int,
                                                      const wchar_t*,
                                                      const int ) override;

        HRESULT STDMETHODCALLTYPE endPrefixMapping( const wchar_t*,
                                                    const int) override;

        HRESULT STDMETHODCALLTYPE startElement( const wchar_t*,
                                                const int,
                                                const wchar_t*,
                                                const int,
                                                const wchar_t*,
                                                const int,
                                                ISAXAttributes* ) override;

        HRESULT STDMETHODCALLTYPE endElement( const wchar_t*,
                                              const int,
                                              const wchar_t*,
                                              const int,
                                              const wchar_t*,
                                              const int ) override;

        HRESULT STDMETHODCALLTYPE characters( const wchar_t*,
                                              const int ) override;

        HRESULT STDMETHODCALLTYPE ignorableWhitespace( const wchar_t*,
                                                       const int ) override;

        HRESULT STDMETHODCALLTYPE processingInstruction( const wchar_t*,
                                                         const int,
                                                         const wchar_t*,
                                                         const int ) override;

        HRESULT STDMETHODCALLTYPE skippedEntity( const wchar_t*,
                                                 const int ) override;

        long STDMETHODCALLTYPE QueryInterface( const struct _GUID &,
                                               void ** ) override;

        unsigned long STDMETHODCALLTYPE AddRef() override;

        unsigned long STDMETHODCALLTYPE Release() override;
    };
}