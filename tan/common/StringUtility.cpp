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

#include "StringUtility.h"

#include <cctype>

bool compareIgnoreCase(const std::string & first, const std::string & second)
{
	if(first.size() != second.size())
	{
        return false;
	}

	for(auto firstChar = first.begin(), secondChar = second.begin(); firstChar != first.end(); ++firstChar, ++secondChar)
	{
		if(std::tolower(*firstChar) != std::tolower(*secondChar))
		{
			return false;
		}
	}

    return true;
}

std::string toString(const std::wstring & inputString)
{
	std::mbstate_t state = std::mbstate_t();

	const std::wstring::value_type *input(inputString.c_str());

    std::size_t length = std::wcsrtombs(nullptr, &input, 0, &state) + 1;
    std::vector<std::string::value_type> buffer(length);
    std::wcsrtombs(buffer.data(), &input, buffer.size(), &state);

	return buffer.data();
}

std::wstring toWideString(const std::string & inputString)
{
	std::mbstate_t state = std::mbstate_t();

	const std::string::value_type *input(inputString.c_str());

    std::size_t length = std::mbsrtowcs(nullptr, &input, 0, &state) + 1;
    std::vector<std::wstring::value_type> buffer(length);
    std::mbsrtowcs(buffer.data(), &input, buffer.size(), &state);

	return buffer.data();
}