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

#ifdef _WIN32

#include "RegisterBrowser.h"
#include <stdio.h>
#include <tchar.h>

WindowsRegister::WindowsRegister()
{
}

WindowsRegister::WindowsRegister(HKEY key, std::string subkey)
{
	loadKey(key, subkey);
}

void WindowsRegister::loadKey(HKEY key, std::string subkey)
{
	m_sSubKeyName = subkey;
	RegOpenKeyEx(key,
		subkey.c_str(),
		0,
		KEY_READ,
		&m_RegisterKey);


	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
								   // Get the class name and the value count. 
	DWORD retcode = RegQueryInfoKey(
		m_RegisterKey,           // key handle 
		m_cpName,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&m_iNumOfSubKeys,        // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&m_iNumOfValue,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&m_iLastWriteTime);       // last write time

								  // Getting the subkeys name
	m_cpSubKeyNames = new char*[m_iNumOfSubKeys];
	for(int i =0 ; i < m_iNumOfSubKeys; i++)
	{
		m_cpSubKeyNames[i] = new char[MAX_KEY_LENGTH+1];
		m_cpSubKeyNames[i][0] = '\0';
	}
	m_cpValueNames = new char*[m_iNumOfValue];
	for(int i = 0; i < m_iNumOfValue; i++)
	{
		m_cpValueNames[i] = new char[MAX_VALUE_NAME+1];
		m_cpValueNames[i][0] = '\0';
	}
	getSubKeysNames();
	getValueNames();
}

WindowsRegister::~WindowsRegister()
{
	for(int i =0 ; i < m_iNumOfSubKeys;i++)
	{
		delete[]m_cpSubKeyNames[i];
	}
	delete[] m_cpSubKeyNames;

	for (int i = 0; i < m_iNumOfValue;i++)
	{
		delete[]m_cpValueNames[i];
	}
	delete[] m_cpValueNames;
	RegCloseKey(m_RegisterKey);
}

bool WindowsRegister::getSubKeysNames()
{
	
	FILETIME ftLastWriteTime;
	
	for (int i = 0; i<m_iNumOfSubKeys; i++)
	{
		DWORD cbName = MAX_KEY_LENGTH;
		DWORD retCode = RegEnumKeyEx(m_RegisterKey, i,
			m_cpSubKeyNames[i],
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);
		if (retCode != ERROR_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

bool WindowsRegister::getValueNames()
{
	for (int i = 0, retCode = ERROR_SUCCESS; i<m_iNumOfValue; i++)
	{
		DWORD cchValue = MAX_VALUE_NAME;
		retCode = RegEnumValue(m_RegisterKey, i,
			m_cpValueNames[i],
			&cchValue,
			NULL,
			NULL,
			NULL,
			NULL);

		if (retCode != ERROR_SUCCESS)
		{
			return false;
		}
	}

	return true;
}

void WindowsRegister::printSubKeyInfo()
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	FILETIME ftLastWriteTime;      // last write time 
	printf("\nNumber of subkeys: %d\n", m_iNumOfSubKeys);

	for (int i = 0; i<m_iNumOfSubKeys; i++)
	{
		cbName = MAX_KEY_LENGTH;
		DWORD retCode = RegEnumKeyEx(m_RegisterKey, i,
			achKey,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);
		if (retCode == ERROR_SUCCESS)
		{
			_tprintf(TEXT("(%d) %s\n"), i + 1, achKey);
		}
	}
}

void WindowsRegister::printValueInfo()
{
	printf("\nNumber of values: %d\n", m_iNumOfValue);


	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	for (int i = 0, retCode = ERROR_SUCCESS; i<m_iNumOfValue; i++)
	{
		cchValue = MAX_VALUE_NAME;
		achValue[0] = '\0';
		retCode = RegEnumValue(m_RegisterKey, i,
			achValue,
			&cchValue,
			NULL,
			NULL,
			NULL,
			NULL);

		if (retCode == ERROR_SUCCESS)
		{
			_tprintf(TEXT("(%d) %s\n"), i + 1, achValue);
		}
	}
}

WindowsRegister* WindowsRegister::getSubKeys()
{
	//WindowsRegister
	WindowsRegister* return_registers = new WindowsRegister[m_iNumOfSubKeys];
	for(int i = 0; i<m_iNumOfSubKeys; i++)
	{
		return_registers[i].loadKey(this->m_RegisterKey, m_cpSubKeyNames[i]);
	}
	return return_registers;
}

WindowsRegister* WindowsRegister::getSubKey(std::string subkey)
{
	for (int i = 0; i < m_iNumOfSubKeys; i++)
	{
		if (!strcmp(m_cpSubKeyNames[i], subkey.c_str()))
		{
			return new WindowsRegister(m_RegisterKey, subkey);
		}
	}
	return nullptr;
}

std::string WindowsRegister::getStringValue(std::string valuename)
{	
	for(int i = 0; i < m_iNumOfValue; i++)
	{
		if(!strcmp(m_cpValueNames[i],valuename.c_str()))
		{
			TCHAR  achValue[MAX_VALUE_NAME];
			DWORD cchValue = MAX_VALUE_NAME;
			DWORD size;
			DWORD type;
			RegEnumValue(m_RegisterKey, i,
				achValue,
				&cchValue,
				NULL,
				&type,
				NULL,
				&size);

			if(type == REG_SZ)
			{
				char* ret = new char[size];
				RegGetValue(m_RegisterKey, NULL, valuename.c_str(), RRF_RT_REG_SZ, NULL, ret, &size);

				return std::string(ret);
			}

			break;
		}
	}

	return std::string();
}

bool WindowsRegister::hasSubKey(std::string subkey)
{
	for(int i = 0 ; i < m_iNumOfSubKeys; i++)
	{
		if(!strcmp(m_cpSubKeyNames[i],subkey.c_str()))
		{
			return true;
		}
	}
	return false;
}

#endif