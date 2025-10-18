/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2ErrorHandlerImpl.cpp
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */
#include "SAX2ErrorHandlerImpl.h"

namespace SAX2XML
{

    SAX2ErrorHandlerImpl::SAX2ErrorHandlerImpl( const SAX2XMLErrorCallback& SAX2XMLErrorcallback_i,
                                                std::any&                   anyUserInfo_io         ) :
                                                m_SAX2XMLErrorcallback( SAX2XMLErrorcallback_i ),
                                                m_anyUserInfo( anyUserInfo_io )
    {
    }


    HRESULT STDMETHODCALLTYPE SAX2ErrorHandlerImpl::error( ISAXLocator* ,
                                                   const wchar_t* pwszErrorMessage_i,
                                                   const HRESULT  )
    {
        InvokeErrorCallback( pwszErrorMessage_i );
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAX2ErrorHandlerImpl::fatalError( ISAXLocator * ,
                                                        const wchar_t * pwszErrorMessage_i,
                                                        const HRESULT  )
    {
        InvokeErrorCallback( pwszErrorMessage_i );
        return S_OK;
    }


    HRESULT STDMETHODCALLTYPE SAX2ErrorHandlerImpl::ignorableWarning( ISAXLocator * ,
                                                              const wchar_t * pwszErrorMessage_i,
                                                              const HRESULT  )
    {
        InvokeErrorCallback( pwszErrorMessage_i );
        return S_OK;
    }

    long STDMETHODCALLTYPE SAX2ErrorHandlerImpl::QueryInterface( const struct _GUID&,
                                                                 void** )
    {
        return 0;
    }


    unsigned long STDMETHODCALLTYPE SAX2ErrorHandlerImpl::AddRef()
    {
        return 0;
    }


    unsigned long STDMETHODCALLTYPE SAX2ErrorHandlerImpl::Release()
    {
        return 0;
    }


    void SAX2ErrorHandlerImpl::InvokeErrorCallback( const wchar_t * pwszErrorMessage_i )
    {
        if( m_SAX2XMLErrorcallback )
        {
            m_SAX2XMLErrorcallback( pwszErrorMessage_i, m_anyUserInfo );
        }
    }
}