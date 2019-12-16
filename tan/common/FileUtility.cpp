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

#include "FileUtility.h"

#include <string>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else

#if defined(__APPLE__) || defined(__MACOSX)
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#include <unistd.h>
//#include <sys/types.h>
#include <sys/stat.h>

#endif

#include <sys/types.h>
#include <sys/stat.h>

std::vector<std::string> getDirectorySeparatorVariants()
{
	return {
#ifdef _WIN32
		   "\\",
#endif
			"/"
		};
}

size_t getDefaultSeparatorIndex()
{
	return 0;
}

size_t getLastSeparatorPosition(const std::string & path, std::string & foundSeparator)
{
	for(const auto & variant : getDirectorySeparatorVariants())
	{
		auto lastPosition = path.rfind(variant);

		if(lastPosition != std::string::npos)
		{
			foundSeparator = variant;

			return lastPosition;
		}
	}

	return std::string::npos;
}

std::string getCurrentDirectory()
{
    char currentDirectory[MAX_PATH] = { 0 };

#ifdef _WIN32
    GetCurrentDirectoryA(MAX_PATH, currentDirectory);
#else
	getcwd(currentDirectory, MAX_PATH);
#endif

	return currentDirectory;
}

bool setCurrentDirectory(const std::string& directoryName)
{
	return -1 !=
#ifdef _WIN32
		_chdir(directoryName.c_str());
#else
		chdir(directoryName.c_str());
#endif
		;
}

std::string joinPaths(const std::string & left, const std::string & right)
{
	std::string foundSeparator;
	auto leftSeparatorPosition(getLastSeparatorPosition(left, foundSeparator));

	bool separatorAtEnd(leftSeparatorPosition != std::string::npos && leftSeparatorPosition == (left.size() - foundSeparator.length()));

	return left + (separatorAtEnd ? right : getDirectorySeparatorVariants()[getDefaultSeparatorIndex()] + right);
}

FileVersion getFileVersion(const std::string& filepath)
{
	FileVersion ret;
	std::memset(&ret, 0, sizeof(FileVersion));

	//todo: port it
	/*DWORD verHandle = NULL;
	DWORD versioninfosize = GetFileVersionInfoSize(filepath.c_str(), &verHandle);
	char* verData = new char[versioninfosize];
	if(GetFileVersionInfo(filepath.c_str(),verHandle,versioninfosize,verData))
	{
		LPVOID lpbuffer = NULL;
		UINT size = 0;
		if (VerQueryValue(verData, "\\", &lpbuffer, &size))
		{
			if(size)
			{
				VS_FIXEDFILEINFO* verInfo = static_cast<VS_FIXEDFILEINFO*>(lpbuffer);
				if(verInfo->dwSignature == 0xfeef04bd)
				{
					ret.m_MajorVersion = (verInfo->dwFileVersionMS >> 16) & 0xffff;
					ret.m_MinorVersion = (verInfo->dwFileVersionMS >> 0) & 0xffff;
					ret.m_BuildVersion = (verInfo->dwFileVersionLS >> 16) & 0xffff;
					ret.m_RevisionVersion = (verInfo->dwFileVersionLS >> 0) & 0xffff;
				}
			}
		}
	}*/
	return ret;
}

std::string getFileVersionString(const std::string& filepath)
{
	FileVersion ret = getFileVersion(filepath);

	return std::to_string(ret.m_MajorVersion) + '.' +
		std::to_string(ret.m_MinorVersion) + '.' +
		std::to_string(ret.m_BuildVersion) + '.' +
		std::to_string(ret.m_RevisionVersion);
}

/*
void getFileVersionAndDate(wchar_t *logMessage, char *version, size_t maxLength))
{
#ifdef _WIN32
	time_t dt = time(NULL);
	tm *lt = localtime(&dt);

	auto moduleFileName = getModuleFileName();

	DWORD dummy = 0;
	DWORD size = GetFileVersionInfoSizeW(filename, &dummy);
	std::wstring buffer(size + 1);

	WCHAR * buffer = new WCHAR[size];
	if (buffer == NULL){ return; }
	WCHAR *ver = NULL, *pStr;
	WCHAR *pSrch = L"FileVersion";
	if (GetFileVersionInfoW(filename, 0, size, (void*)buffer)){
		pStr = buffer;
		while (pStr < buffer + size){
			if (wcsncmp(pStr, pSrch, 10) == 0){
				ver = pStr + wcslen(pStr) + 2;
				break;
			}
			else {
				pStr++;
			}
		}
	}

	wsprintfW(logMessage, L"**** %s v%s on %4d/%02d/%02d %02d:%02d:%02d ****\n", filename, ver,
		2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

	int lv = wcslen(ver);
	for (int i = 0; i < lv; i++){
		version[i] = char(ver[i]);
	}
	delete []buffer;
#else
    //todo: implement
	std::swprintf(logMessage, 256, L"Error! Not implemented");
	std::snprintf(version, 256, "Error! Not implemented");
#endif
}
*/

std::string getPath2File(const std::string& fileNameWithPath)
{
	std::string foundSeparator;
	auto separatorPosition(getLastSeparatorPosition(fileNameWithPath, foundSeparator));

	auto path2File(
		(separatorPosition != std::string::npos)
			? fileNameWithPath.substr(0, separatorPosition/*  + 1 */)
			: ""
		);

	//std::cout << "path to file [" << fileNameWithPath << "] is [" << path2File << "]" << std::endl;

	return path2File;
}

bool createPath(const std::string & path)
{
	//std::cout << "create path: " << path << std::endl;
	
	std::vector<std::string> directories;

	auto component = getFileNameWithExtension(path);
	auto otherPath = getPath2File(path);

	if(!component.length() && otherPath.length())
	{
		component = otherPath;
	}

	//std::cout << "INCOME: [" << path << "] [" << component << "] [" << otherPath << "]" << std::endl;

	for( ; component.length(); )
	{
		//std::cout << "PUSH:" << component << std::endl;

		directories.push_back(component);

		if(otherPath.length())
		{
			component = getFileNameWithExtension(otherPath);
			otherPath = getPath2File(otherPath);

			//if(!component.length() && otherPath.length() && component != otherPath)
			//{
			//	component = otherPath;
			//}

			//std::cout << "comp:" << component << " other:" << otherPath << std::endl;
		}
		else
		{
			component.resize(0);
		}
	}

	for
	(
		auto componentIterator(directories.rbegin());
		componentIterator != directories.rend();
		++componentIterator
	)
	{
		auto directory(*componentIterator);

		//std::cout << "check directory exists: " << directory << std::endl;

		if(!checkDirectoryExist(directory))
		{
			//std::cout << "directory does not exist: " << directory << std::endl;

#ifndef _WIN32

			auto result(
				mkdir(
					directory.c_str(), 0777
					)
				);

			//if (!result)
			//{
			//	break;
			//}

#else

			auto result(
				_mkdir(
					directory.c_str()
				)
				);

			//if (!result)
			//{
			//	break;
			//}
#endif
			//std::cout << "Result: " << result << std::endl;
		}
    }

	return checkDirectoryExist(path);
}

std::string getFileNameWithExtension(const std::string& filepath)
{
	std::string foundSeparator;
	auto lastSeparatorPosition = getLastSeparatorPosition(filepath, foundSeparator);

	if(std::string::npos != lastSeparatorPosition)
	{
		return filepath.substr(
			lastSeparatorPosition + foundSeparator.length(),
			std::string::npos
			);
	}

	return filepath;
}

std::string getFileNameWithoutExtension(const std::string& filepath)
{
	std::string foundSeparator;
	auto lastSeparatorPosition = getLastSeparatorPosition(filepath, foundSeparator);

	size_t dotPosition = filepath.rfind('.');

	if((std::string::npos != dotPosition) && ((std::string::npos == lastSeparatorPosition) || (dotPosition > lastSeparatorPosition)))
	{
		return std::string::npos == lastSeparatorPosition
			? filepath.substr(0, dotPosition - 1)
			: filepath.substr(lastSeparatorPosition + foundSeparator.length(), dotPosition - lastSeparatorPosition - foundSeparator.length())
			;
	}

	return getFileNameWithExtension(filepath);
}

std::string getFileExtension(const std::string & fileName)
{
    auto lastDot = fileName.rfind('.');

    if(std::string::npos != lastDot)
    {
        return fileName.substr(lastDot + 1);
    }

    return "";
}

bool checkFileExist(const std::string& filename)
{
	if (FILE *file = fopen(filename.c_str(), "r"))
	{
		fclose(file);

		return true;
	}
	else
	{
		return false;
	}
}

bool checkDirectoryExist(const std::string& path)
{
	struct stat statInfo = { 0 };

	auto pathStat = stat(path.c_str(), &statInfo);
	
	if(pathStat)
	{
		return false;
	}

	return statInfo.st_mode & S_IFDIR;
}

/*bool isDirExist(const std::string& path)
{
#if defined(_WIN32)
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}*/

std::string getModuleFileName()
{
    char buffer[MAX_PATH] = {};

#ifdef _WIN32
    if(!GetModuleFileNameA(NULL, (LPSTR)buffer, sizeof(buffer)))
    {
        throw std::string("GetModuleFileName() failed!");
    }
#else

#if defined(__APPLE__) || defined(__MACOSX)
	uint32_t bufsize = MAX_PATH;

	//0 means success
	if(!!_NSGetExecutablePath(buffer, &bufsize))
	{
		throw std::string("_NSGetExecutablePath failed!");
	}

	/*
	if (!module) { return std::string(); }
	uint32_t count = _dyld_image_count();
	for (uint32_t i = 0; i < count; ++i) {
		const mach_header *header = _dyld_get_image_header(i);
		if (!header) { break; }
		char *code_ptr = NULL;
		if ((header->magic & MH_MAGIC_64) == MH_MAGIC_64) {
			uint64_t size;
			code_ptr = getsectdatafromheader_64((const mach_header_64 *)header, SEG_TEXT, SECT_TEXT, &size);
		} else {
			uint32_t size;
			code_ptr = getsectdatafromheader(header, SEG_TEXT, SECT_TEXT, &size);
		}
		if (!code_ptr) { continue; }
		const uintptr_t slide = _dyld_get_image_vmaddr_slide(i);
		const uintptr_t start = (const uintptr_t)code_ptr + slide;
		Dl_info info;
		if (dladdr((const void *)start, &info)) {
			if (dlopen(info.dli_fname, RTLD_NOW) == module) {
				return std::string(info.dli_fname);
			}
		}
	}
	return std::string();
	*/
#else

    if(-1 == readlink("/proc/self/exe", buffer, MAX_PATH))
    {
		throw std::string("Error: readlink() failed!");
    }

#endif

#endif

	return buffer;

	//todo: ivm: ivestigate why?
	/*
	std::string str(buffer);

	auto separatorPosition = str.find_last_of(getDirectorySeparator());

	return (separatorPosition != std::string::npos) && (separatorPosition > 0)
		? str.substr(0, separatorPosition + 1)
		: str;
	*/
}

#ifndef _WIN32

// Linux equivalent function of fopen_s
errno_t fopen_s(FILE **file, const char *filename, const char *mode)
{
    if (NULL == file)
        return EINVAL;

    (*file) = fopen(filename, mode);

    if (NULL != *file)
        return 0;
    else
        return errno;
}

#endif