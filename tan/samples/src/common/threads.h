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

#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>
//#include <mutex>
//#include <condition_variable>

#ifdef _WIN32

#include <Windows.h>
#include <AclAPI.h>

#else

#include <pthread.h>

#endif

class PrioritizedThread:
    public std::thread
{
protected:
    bool mRealtime;
    //std::mutex mLockMutex;
    //std::condition_variable mQuitCondition;

#ifdef _WIN32
#else
    int mPolicy;
    int mPriority;
    sched_param mSheduleParams;
#endif

public:
    PrioritizedThread(bool realTime = false):
        mRealtime(realTime)
#ifdef  _WIN32
#else
        , mPolicy(
            realTime
                ? SCHED_RR
                : SCHED_FIFO
          )
        , mPriority(
            realTime
                ? 2
                : 10
            )
#endif
    {
#ifdef _WIN32
#else
        mSheduleParams.sched_priority = mPriority;
#endif
    }

    virtual ~PrioritizedThread()
    {
    }

    PrioritizedThread& operator=(std::thread&& other) noexcept
    {
        swap(other);

#ifdef _WIN32
        if(mRealtime)
        {
            SetSecurityInfo(native_handle(), SE_WINDOW_OBJECT, PROCESS_SET_INFORMATION, 0, 0, 0, 0);
            SetPriorityClass(native_handle(), REALTIME_PRIORITY_CLASS);
            SetThreadPriority(native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
        }
#else
        if(pthread_setschedparam(
            native_handle(),
            mPolicy,
            &mSheduleParams
            ))
        {
            std::cerr << "Error: failed to set thread priority (" << std::strerror(errno) << ")" << std::endl;
        }
#endif

        //??? SetSecurityInfo(GetCurrentProcess(), SE_WINDOW_OBJECT, PROCESS_SET_INFORMATION, 0, 0, 0, 0);
        //done - SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        //done - SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        return *this;
    }

    virtual void WaitCloseInfinite()
    {
        join();

        /*
        // wait for the detached thread
        std::unique_lock<std::mutex> lock(mLockMutex);
        mQuitCondition.wait(lock, []{ return ready; });
        */
    }
};