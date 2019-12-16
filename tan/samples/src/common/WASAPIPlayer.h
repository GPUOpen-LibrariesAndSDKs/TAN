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

#ifndef _WASAPI_H_
#define _WASAPI_H_

#include "IWavPlayer.h"

#include <Windows.h>
//#include <VersionHelpers.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <mmsystem.h>
#include <Audioclient.h>
#include <endpointvolume.h>

class WASAPIPlayer:
  public IWavPlayer
{
protected:
    uint16_t mChannelsCount = 0;
    uint16_t mBitsPerSample = 0;
    uint32_t mSamplesPerSecond = 0;

    IMMDevice *mDevRender = nullptr;
    IMMDevice *mDevCapture = nullptr;
    IMMDeviceEnumerator *mDevEnum = nullptr;
    IAudioClient *mAudioClient = nullptr;
    IAudioRenderClient *mRenderClient = nullptr;
    IAudioClient *mAudioCapClient = nullptr;
    IAudioCaptureClient *mCaptureClient = nullptr;
	uint32_t mBufferSize = 0;

    bool mStartedRender = false;
    bool mStartedCapture = false;
    bool mInitializedRender = false;
    bool mInitializedCapture = false;

public:
    WASAPIPlayer();
    virtual ~WASAPIPlayer();

    std::string GetPlayerName() const override { return "WASAPI"; }

	uint16_t GetSampleSizeInBytes() const override
	{
		return mChannelsCount * mBitsPerSample / 8;
	}

    virtual PlayerError Init(
        uint16_t    channelsCount,
        uint16_t    bitsPerSample,
        uint32_t    samplesPerSecond,
        bool        play,
        bool        record
    ) override;
    virtual void Close() override;

    virtual uint32_t Play(uint8_t * buffer, uint32_t sizeInBytes, bool mute) override;
    virtual uint32_t Record(uint8_t * buffer, uint32_t size) override;
};

#endif