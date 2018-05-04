#pragma once
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
