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

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <string>
#include <list>
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

class WindowsRegister
{
public:
	WindowsRegister();
	WindowsRegister(HKEY key, std::string subkey);
	virtual ~WindowsRegister();
	void loadKey(HKEY key, std::string subkey);
	void printSubKeyInfo();
	void printValueInfo();
	DWORD getNumOfSubKeys() const { return m_iNumOfSubKeys; };
	WindowsRegister* getSubKeys();
	WindowsRegister* getSubKey(std::string subkey);
	std::string getStringValue(std::string valuename);
	bool hasSubKey(std::string subkey);
	//WindowsRegister* getSubKeys();
	//WindowsRegister* getSubkeys(char* subkeyname);
private:
	std::string m_sSubKeyName;
	bool getSubKeysNames();
	bool getValueNames();
	char m_cpName[MAX_KEY_LENGTH*2];
	char** m_cpSubKeyNames;
	DWORD m_iNumOfSubKeys = 0;
	char** m_cpValueNames;
	DWORD m_iNumOfValue = 0;
	FILETIME m_iLastWriteTime;
	HKEY m_RegisterKey;
};

#endif