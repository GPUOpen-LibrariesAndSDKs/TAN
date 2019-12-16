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

struct BasicTimer
{
    bool Started;

    BasicTimer():
        Started(false)
    {
    }

    void Start()
    {
        Started = true;
    }

    void Stop()
    {
        Started = false;
    }

    inline bool IsStarted() const
    {
        return Started;
    }
};

#ifdef WIN32

struct Timer:
    public BasicTimer
{
    LARGE_INTEGER start, stop, freq;

public:
    Timer() { QueryPerformanceFrequency( &freq ); }

    void Start()
    {
        BasicTimer::Start();

        QueryPerformanceCounter(&start);
    }

    double Sample()
    {
        QueryPerformanceCounter  ( &stop );
        double time = (double)(stop.QuadPart-start.QuadPart) / (double)(freq.QuadPart);
        return time;
    }
};

#elif defined(__APPLE__) || defined(__MACOSX)

#include <mach/clock.h>
#include <mach/mach.h>

struct Timer:
    public BasicTimer
{
    clock_serv_t clock;
    mach_timespec_t start, end;

public:
    Timer() { host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock); }
    ~Timer() { mach_port_deallocate(mach_task_self(), clock); }

    void Start()
    {
        BasicTimer::Start();

        clock_get_time(clock, &start);
    }

    double Sample()
    {
        clock_get_time(clock, &end);
        double time = 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        return time * 1E-9;
    }
};

#else

#include <time.h>
#include <math.h>

struct Timer:
    public BasicTimer
{
    struct timespec start, end;

public:
    void Start()
    {
        BasicTimer::Start();

        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    double Sample()
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        return time * 1E-9;
    }
};

#endif