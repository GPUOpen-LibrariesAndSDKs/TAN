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

#ifndef MGIFIFOBUFFER
#define MGIFIFOBUFFER

class FifoBuffer
{
public:
    FifoBuffer(int maxSize);
    ~FifoBuffer();
    bool store(char *inputData, unsigned int length);
    bool retrieve(char *outputData, unsigned int length);
    bool putBack(unsigned int n);

    // APIs for reduced copying:
    int getNextEmptySeg(char **pSeg);
    bool storeSeg(unsigned int length);
    int getNextFullSeg(char **pSeg);
    bool retrieveSeg(unsigned int length);
    unsigned int fifoLength();
    int retrieveAll(char *outputData);
    void flush();

private:
    char *m_Buffer;
    unsigned int m_BufferSize, m_Tail, m_Head;
    unsigned int roomLeft();

#ifdef BE_THREAD_SAFE
    CCritSec m_Lock;
#endif
};

#include <cstring>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

//two-threads frendly fifo
//one thread - write
//another thread - read
class Fifo
{
    //std::atomic<size_t>
    size_t
                    mQueueSize;
    //std::atomic<size_t>
    size_t
                    mBufferInPosition;
    //std::atomic<size_t>
    size_t
                    mBufferOutPosition;
    std::vector<uint8_t>
                    mBuffer;

    /*inline size_t   GetQueueSize() const
    {
        std::lock_guard<std::mutex> lock(mLockMutex);

        return mQueueSize;
    }

    inline size_t   GetBufferInPosition() const
    {
        std::lock_guard<std::mutex> lock(mLockMutex);

        return mBufferInPosition;
    }

    inline size_t   GetBufferOutPosition() const
    {
        std::lock_guard<std::mutex> lock(mLockMutex);

        return mBufferOutPosition;
    }*/

public:
    Fifo():
        mQueueSize(0),
        mBufferInPosition(0),
        mBufferOutPosition(0)
    {
    }

    void Reset(size_t newSize);
    size_t GetQueueSize() const;

    uint32_t Write(const uint8_t *data, size_t size);
    uint32_t Read(uint8_t *outputBuffer, size_t size2Fill);
};

#endif
