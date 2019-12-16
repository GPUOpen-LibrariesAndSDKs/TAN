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

#include <vector>

//in c++17 aligned allocation are easy
//in c++<17 we must control aligment by hands

//Wasteful alligned allocator
//will eat up to Alignment bytes of not demanded memory
template<typename Type, size_t Alignment>
class AllignedAllocator
{
protected:
    std::vector<uint8_t> mMemory;
    Type *mPointer = nullptr;

public:
    AllignedAllocator()
    {
        //test power of two
        if(!Alignment || (Alignment & (Alignment - 1)))
        {
            throw std::runtime_error("Error: alignment is not a power of 2!");
        }
    }

    Type * GetAlignedAddress() const
    {
        return static_cast<Type *>(mPointer);
    }

    /*const Type * GetAlignedAddress() const
    {
        return static_cast<Type *>(mPointer);
    }*/

    explicit operator Type *() const
    {
        return static_cast<Type *>(mPointer);
    }

    explicit operator const Type *() const
    {
        return static_cast<const Type *>(mPointer);
    }

    /*operator const void *() const
    {
        return GetAlignedAddress();
    }

    operator void *() const
    {
        return GetAlignedAddress();
    }*/

    Type * Allocate(size_t count)
    {
        mMemory.resize(count * sizeof(Type) + Alignment);

        //c-style, not wastful, but need smartpointer
        //posix_memalign(outPointer, Alignment, sizeof(Type) * count);

        //c++11:
        void *pointer(&mMemory.front());
        size_t capacity(mMemory.size());

        return mPointer = static_cast<Type *>(std::align(Alignment, sizeof(Type), pointer, capacity));
    }
};