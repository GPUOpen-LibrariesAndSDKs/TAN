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

#ifndef GRAALFILE_HPP_
#define GRAALFILE_HPP_

#include "FileUtility.h"

#include <vector>
#include <string>
#include <fstream>
#if !defined(__APPLE__) && !defined(__MACOSX)
#include <malloc.h>
#endif
#include <cstring>

/**
 * namespace appsdk
 */
namespace graal
{

/**
 * class SDKFile
 * for the opencl program file processing
 */
class GraalFile
{
    public:
        /**
         *Default constructor
         */
        GraalFile(): source_("") {}

        /**
         * Destructor
         */
        ~GraalFile() {};

        /**
         * Opens the CL program file
         * @return true if success else false
         */
        bool open(const char* fileName)
        {
            size_t      size;
            char*       str;
            // Open file stream
            std::fstream f(fileName, (std::fstream::in | std::fstream::binary));
            // Check if we have opened file stream
            if (f.is_open())
            {
                size_t  sizeFile;
                // Find the stream size
                f.seekg(0, std::fstream::end);
                size = sizeFile = (size_t)f.tellg();
                f.seekg(0, std::fstream::beg);
                str = new char[size + 1];
                if (!str)
                {
                    f.close();
                    return  false;
                }
                // Read file
                f.read(str, sizeFile);
                f.close();
                str[size] = '\0';
                source_  = str;
                delete[] str;
                return true;
            }
            return false;
        }

        /**
         * writeBinaryToFile
         * @param fileName Name of the file
         * @param binary char binary array
         * @param numBytes number of bytes
         * @return true if success else false
         */
        int writeBinaryToFile(const char* fileName, const char* binary, size_t numBytes)
        {
            FILE *output = NULL;

	        if(fopen_s(&output, fileName, "wb"))
            {
                return GRAAL_FAILURE;
            }
            fwrite(binary, sizeof(char), numBytes, output);
            fclose(output);
            return GRAAL_SUCCESS;
        }


        /**
         * readBinaryToFile
         * @param fileName name of file
         * @return true if success else false
         */
        int readBinaryFromFile(const char* fileName)
        {
            FILE * input = NULL;
            size_t size = 0,val;
            char* binary = NULL;

            if(fopen_s(&input, fileName, "rb"))
            {
                return GRAAL_FAILURE;
            }
            fseek(input, 0L, SEEK_END);
            size = ftell(input);
            rewind(input);
            binary = (char*)malloc(size);
            if(binary == NULL)
            {
                return GRAAL_FAILURE;
            }
            val=fread(binary, sizeof(char), size, input);
            fclose(input);
            source_.assign(binary, size);
            free(binary);
            return GRAAL_SUCCESS;
        }


        /**
         * Replaces Newline with spaces
         */
        void replaceNewlineWithSpaces()
        {
            size_t pos = source_.find_first_of('\n', 0);
            while(pos != -1)
            {
                source_.replace(pos, 1, " ");
                pos = source_.find_first_of('\n', pos + 1);
            }
            pos = source_.find_first_of('\r', 0);
            while(pos != -1)
            {
                source_.replace(pos, 1, " ");
                pos = source_.find_first_of('\r', pos + 1);
            }
        }

        /**
         * source
         * Returns a pointer to the string object with the source code
         */
        const std::string&  source() const
        {
            return source_;
        }

    private:
        /**
         * Disable copy constructor
         */
        GraalFile(const GraalFile&);

        /**
         * Disable operator=
         */
        GraalFile& operator=(const GraalFile&);

        std::string     source_;    //!< source code of the CL program
};

} // namespace appsdk

#endif  // SDKFile_HPP_
