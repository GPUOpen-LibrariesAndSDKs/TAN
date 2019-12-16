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
#include "stdafx.h"
#include "StringUtility.h"
#include "FileUtility.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#if defined(_WIN32)
#include <direct.h>
#endif

#define MAX_SIZE_STRING  16384
#define MAX_BLOCK_CHUNK  12000

int main(int argc, char* argv[])
{
	if (argc > 3)
	{
		std::cerr << "Too many parameters" << std::endl;

		return 1;
	}

	if (argc < 3)
	{
		std::cerr << "Not enough parameters" << std::endl;

		return 1;
	}

	std::string kernelFileFullName = argv[1];
	auto kernelFileExtension = getFileExtension(kernelFileFullName);

	if(!compareIgnoreCase(kernelFileExtension, "cl"))
	{
		std::cerr << "File is not a .cl file" << std::endl;

		return 1;
	}

    std::basic_ifstream<char, std::char_traits<char> > clKernelStream(
        kernelFileFullName,
        std::ios::in | std::ios::binary
        );

	if(clKernelStream.fail())
	{
		std::cerr << "Could not open .cl file" << std::endl;

		return 1;
	}

	auto fileName = getFileNameWithExtension(kernelFileFullName);
	fileName.resize(fileName.length() - 3); //skip extension

	std::string outputName = argv[2];
	auto wideOutputName = toWideString(outputName);

	std::string outputFileName(
		outputName + ".cl.h"
		);

	//std::cout << "CURRENT: " << getCurrentDirectory() << " " << outputFileName << std::endl;
	auto path2File(getPath2File(outputFileName));

	if(path2File.length() && !checkDirectoryExist(path2File))
	{
		if(!createPath(path2File))
		{
			std::cout << "Could not create path " << path2File << std::endl;

			return 1;
		}
	}

	std::wofstream outputStream(outputFileName);

	if(outputStream.fail())
	{
		//try to create directory
		//auto separated = getFile outputFileName

		std::cerr << "Could not open output file " << outputFileName << "." << std::endl;
		std::cerr << "Verify that output directory exists." << std::endl;

		return 1;
	}

	std::cout << "Outputing: " << outputFileName << std::endl;

	// copies all data into buffer, not optimal implementation (dynamic resizes will happened)
	std::vector<char> clKernelSource(
		(std::istreambuf_iterator<char, std::char_traits<char>>(clKernelStream)),
		std::istreambuf_iterator<char, std::char_traits<char>>()
		);

	outputStream
	    << L"#pragma once" << std::endl
		<< std::endl
		<< L"const amf_uint8 " << wideOutputName << L"[] =" << std::endl
		<< L"{";

    unsigned counter(0);

	for(auto charIterator = clKernelSource.begin(); charIterator != clKernelSource.end(); )
	{
		outputStream << std::endl << L"    ";

		for(auto column(0); (column < 8) && (charIterator != clKernelSource.end()); ++column, ++charIterator, ++counter)
		{
			outputStream
			  //<< L"0x" << std::setw(2) << std::setfill(L'0') << std::hex
			  << int(*charIterator)
			  << L", ";
		}
	}

	outputStream << std::dec;

	outputStream
	    << std::endl
		<< L"};" << std::endl
		<< std::endl
		<< L"const amf_size " << wideOutputName << L"Count = " << counter/*clKernelSource.length()*/ << L";" << std::endl
		<< std::endl;

	outputStream.flush();

	return 0;
}