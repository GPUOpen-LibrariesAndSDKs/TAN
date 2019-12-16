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

#include "IWavPlayer.h"
#include "fifo.h"

#include <iostream>

#include <portaudio.h>

class               PortPlayer:
    public          IWavPlayer
{
protected:
    PaStream *      mStream;

    uint8_t         mChannelsCount;
    uint16_t        mBitsPerSample;
    uint32_t        mSamplesPerSecond;

    Fifo            mFifoPlayBuffer;
    Fifo            mFifoRecordBuffer;

    static int      StreamCallback
    (
        const void *input,
        void *      output,
        unsigned long
                    frameCount,
        const PaStreamCallbackTimeInfo *
                    timeInfo,
        PaStreamCallbackFlags
                    statusFlags,
        void *      userData
    );
    int             StreamCallbackImplementation
    (
        const void *input,
        void *      output,
        unsigned long
                    frameCount,
        const PaStreamCallbackTimeInfo *
                    timeInfo,
        PaStreamCallbackFlags
                    statusFlags
    );

public:
    PortPlayer();
    virtual ~PortPlayer();

    std::string     GetPlayerName() const override 
	{
		return "PortAudio";
	}

	uint16_t		GetSampleSizeInBytes() const override
	{
		return mChannelsCount * mBitsPerSample / 8;
	}

    PlayerError     Init
    (
        uint16_t    channelsCount,
        uint16_t    bitsPerSample,
        uint32_t    samplesPerSecond,
        bool        play,
        bool        record
    ) override;
    void            Close() override;

    uint32_t        Record(uint8_t * buffer, uint32_t size) override;
    uint32_t        Play(uint8_t * buffer, uint32_t sizeInBytes, bool mute) override;
};