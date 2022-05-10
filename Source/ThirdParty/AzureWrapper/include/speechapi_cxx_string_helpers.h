//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license201809 for the full license information.
//

#pragma once

#include <string>
#include <codecvt>
#include <locale>
#include <wchar.h>
#include <vector>

#include <azac_api_c_pal.h>
#include <speechapi_cxx_common.h>
#include <speechapi_c_error.h>
#include <speechapi_c_property_bag.h>

#if defined(SWIG) && defined(SPX_UWP)
#define SPXSTRING std::wstring
#define SPXSTRING_EMPTY std::wstring()
#else
#define SPXSTRING std::string
#define SPXSTRING_EMPTY std::string()
#endif

namespace Microsoft{
namespace CognitiveServices {
namespace Speech {
namespace Utils {

namespace Details {

    inline std::string to_string(const std::wstring& value)
    {
        const auto size = pal_wstring_to_string(nullptr, value.c_str(), 0);
        auto buffer = std::make_unique<std::string::value_type[]>(size);
        pal_wstring_to_string(buffer.get(), value.c_str(), size);
        return std::string{ buffer.get() };
    }

    inline std::wstring to_string(const std::string& value)
    {
        const auto size = pal_string_to_wstring(nullptr, value.c_str(), 0);
        auto buffer = std::make_unique<std::wstring::value_type[]>(size);
        pal_string_to_wstring(buffer.get(), value.c_str(), size);
        return std::wstring{ buffer.get() };
    }
}

#if defined(SWIG) && defined(SPX_UWP)
inline std::wstring ToSPXString(const std::string& value)
{
    return Details::to_string(value);
}

inline std::wstring ToSPXString(const char* value)
{
    if (!value)
        return L"";
    return ToSPXString(std::string(value));
}
#else
inline std::string ToSPXString(const char* value)
{
    return value == nullptr ? "" : value;
}

inline std::string ToSPXString(const std::string& value)
{
    return value;
}
#endif

inline std::string ToUTF8(const std::wstring& value)
{
    return Details::to_string(value);
}

inline std::string ToUTF8(const wchar_t* value)
{
    if (!value)
        return "";
    return ToUTF8(std::wstring(value));
}

inline std::string ToUTF8(const std::string& value)
{
    return value;
}

inline const char* ToUTF8(const char* value)
{
    return value;
}

inline static std::string CopyAndFreePropertyString(const char* value)
{
    std::string copy = (value == nullptr) ? "" : value;
    property_bag_free_string(value);
    return copy;
}

inline static std::vector<SPXSTRING> Split(const std::string& str, const char delim)
{
    std::vector<SPXSTRING> result;

    if (str.empty())
    {
        return result;
    }

    size_t start = 0;
    size_t end = str.find(delim);
    while (end != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delim, start);
    }
    result.push_back(ToSPXString(str.substr(start)));

    return result;
}

}}}}
