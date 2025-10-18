#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ErrorHandlerImpl.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */
#include "SAX2XMLCallbacks.h"

#include <MsXml6.h>

namespace SAX2XML
{
    class SAX2ErrorHandlerImpl : public ISAXErrorHandler
    {
    public:

        SAX2ErrorHandlerImpl( const SAX2XMLErrorCallback& SAX2XMLErrorcallback_i,
                              std::any&            anyUserInfo_io );

        HRESULT STDMETHODCALLTYPE error( ISAXLocator* pLocator_i,
                                         const wchar_t* pwszErrorMessage_i,
                                         const HRESULT hrErrorCode_i ) override;
        
         HRESULT STDMETHODCALLTYPE fatalError( ISAXLocator* pLocator_i,
                                               const wchar_t* pwszErrorMessage_i,
                                               const HRESULT hrErrorCode_i ) override;
        
         HRESULT STDMETHODCALLTYPE ignorableWarning( ISAXLocator *pLocator_i,
                                                     const wchar_t* pwszErrorMessage_i,
                                                     const HRESULT hrErrorCode_i ) override;

        long STDMETHODCALLTYPE QueryInterface( const struct _GUID &riid_io,
                                               void ** ppvObject_io ) override;

        unsigned long STDMETHODCALLTYPE AddRef() override;

        unsigned long STDMETHODCALLTYPE Release() override;

        SAX2ErrorHandlerImpl()                                         = delete;
        SAX2ErrorHandlerImpl( const SAX2ErrorHandlerImpl&  )           = delete;
        SAX2ErrorHandlerImpl( SAX2ErrorHandlerImpl&& )                 = delete;
        SAX2ErrorHandlerImpl& operator=( const SAX2ErrorHandlerImpl& ) = delete;
        SAX2ErrorHandlerImpl& operator=( SAX2ErrorHandlerImpl&& )      = delete;

        // Nothing to handle in destructor;
        ~SAX2ErrorHandlerImpl()                                        = default;

    private:

        void InvokeErrorCallback( const wchar_t* pwszErrorMessage_i );


    private:

        SAX2XMLErrorCallback m_SAX2XMLErrorcallback;
        std::any& m_anyUserInfo;
    };
}