/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAXContentHandlerImpl.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */

#include "SAXContentHandlerImpl.h"

namespace SAX2XML
{
    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::putDocumentLocator( ISAXLocator* )
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startDocument()
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::endDocument()
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startPrefixMapping( const wchar_t*,
                                                                         const int,
                                                                         const wchar_t*,
                                                                         const int)
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::endPrefixMapping( const wchar_t*,
                                                                       const int)
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startElement( const wchar_t*,
                                                                   const int,
                                                                   const wchar_t*,
                                                                   const int,
                                                                   const wchar_t*,
                                                                   const int,
                                                                   ISAXAttributes* )
    {
        return S_OK;
    }


    HRESULT SAXContentHandlerImpl::endElement( const wchar_t*,
                                               const int,
                                               const wchar_t*,
                                               const int,
                                               const wchar_t*,
                                               const int )
    {
        return S_OK;
    }


    HRESULT SAXContentHandlerImpl::characters( const wchar_t*,
                                               const int )
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::ignorableWhitespace( const wchar_t*,
                                                                          const int )
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::processingInstruction(  const wchar_t*,
                                                                             const int,
                                                                             const wchar_t*,
                                                                             const int )
    {
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::skippedEntity( const wchar_t*,
                                                                    const int )
    {
        return S_OK;
    }


    long STDMETHODCALLTYPE SAXContentHandlerImpl::QueryInterface( const struct _GUID& ,
                                                                  void** )
    {
        return 0;
    }


    unsigned long STDMETHODCALLTYPE SAXContentHandlerImpl::AddRef()
    {
        return 0;
    }


    unsigned long STDMETHODCALLTYPE SAXContentHandlerImpl::Release()
    {
        return 0;
    }
}