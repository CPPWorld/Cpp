#pragma once
/**
 * Copyright(c) 2025 Roy Mathew, All Rights Reserved.
 *
 * SAX2XMLCallbacks.h
 *
 * @author  : Roy Mathew
 * @version : 1.0            Date:  2025-10-18
 * @info    : (Base version)
 */
#include <any>        // std::any
#include <functional> // std::function

namespace SAX2XML
{
    using SAX2XMLOutputCallback = std::function<bool( wchar_t* const,
                                                      std::any& )>;
    using SAX2XMLErrorCallback  = std::function<bool( const wchar_t* const,
                                                      std::any& )>;
}