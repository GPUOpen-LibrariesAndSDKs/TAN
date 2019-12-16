//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// stdafx.cpp : source file that includes just the standard includes
// CLKernelPreprocessor.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if !defined(_WIN32) && defined( __GNUC__ )

#include <cassert>
#include <cerrno>
#include <cerrno>
#include <cwchar>
#include <cstdlib>

/*inline std::string narrow(std::wstring const& text)
{
    std::locale const loc("");
    wchar_t const* from = text.c_str();
    std::size_t const len = text.size();
    std::vector<char> buffer(len + 1);
    std::use_facet<std::ctype<wchar_t> >(loc).narrow(from, from + len, '_', &buffer[0]);
    return std::string(&buffer[0], &buffer[len]);
}

std::string w2s(const std::wstring &var)
{
   static std::locale loc("");
   auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
   return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
}

std::wstring s2w(const std::string &var)
{
   static std::locale loc("");
   auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
   return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
}*/

#endif