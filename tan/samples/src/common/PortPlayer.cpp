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
#include "PortPlayer.h"
#include "wav.h"

#include <string>
#include <vector>
#include <cstring>

PortPlayer::PortPlayer():
    mStream(nullptr),

    mChannelsCount(0),
    mBitsPerSample(0),
    mSamplesPerSecond(0)

    //, mBufferInPosition(0)
    //, mBufferOutPosition(0)
{
}

PortPlayer::~PortPlayer()
{
    Close();
}

/**
 *******************************************************************************
 * @fn Init
 * @brief Will init alsa
 *******************************************************************************
 */
PlayerError         PortPlayer::Init
(
    uint16_t        channelsCount,
    uint16_t        bitsPerSample,
    uint32_t        samplesPerSecond,
    bool            play,
    bool            record
    )
{
    if(!play && !record)
    {
        std::cerr << "Error: player must has play or record flags set!" << std::endl;

        return PlayerError::InvalidParameters;
    }

    //todo: support paFloat32
    //paFloat32, paInt16, paInt32, paInt24, paInt8 and aUInt8
    PaSampleFormat sampleFormat = 0;

    switch(bitsPerSample)
    {
    case 8:
        sampleFormat = paInt8;
        break;

    case 16:
        sampleFormat = paInt16;
        break;

    case 32:
        sampleFormat = paInt32;
        break;

    default:
        std::cerr << "Error: unsupported sample format: " << bitsPerSample << std::endl;

        return PlayerError::PCMError;
    }

    mChannelsCount = channelsCount;
    mBitsPerSample = bitsPerSample;
    mSamplesPerSecond = samplesPerSecond;

    mFifoPlayBuffer.Reset(mBitsPerSample / 8 * mSamplesPerSecond * mChannelsCount);
    mFifoRecordBuffer.Reset(mBitsPerSample / 8 * mSamplesPerSecond * mChannelsCount);

    auto returnCode(Pa_Initialize());
    if(returnCode != paNoError)
    {
        std::cerr << "Error: Can't initialize PortAudio, " << Pa_GetErrorText(returnCode) << std::endl;

        return PlayerError::PCMError;
    }

    //setup output device
    PaDeviceIndex outputDevice(Pa_GetDefaultOutputDevice());
    if(paNoDevice == outputDevice)
    {
        std::cerr << "Error: Can't get default output device" << std::endl;

        return PlayerError::PCMError;
    }

    auto outputDeviceInfo(Pa_GetDeviceInfo(outputDevice));
    if(!outputDeviceInfo)
    {
        std::cerr << "Error: Can't get default output device information" << std::endl;

        return PlayerError::PCMError;
    }

    PaStreamParameters outputStreamParameters = {};
    outputStreamParameters.device = outputDevice;
    outputStreamParameters.channelCount = channelsCount;
    outputStreamParameters.sampleFormat = sampleFormat;
    outputStreamParameters.suggestedLatency = outputDeviceInfo->defaultHighOutputLatency;

    //setup input device
    PaDeviceIndex inputDevice(paNoDevice);
    PaStreamParameters inputStreamParameters = {};

    if(record)
    {
        if(paNoDevice == (inputDevice = Pa_GetDefaultInputDevice()))
        {
            std::cerr << "Error: Can't get default input device" << std::endl;

            return PlayerError::PCMError;
        }

        auto inputDeviceInfo(Pa_GetDeviceInfo(inputDevice));
        if(!inputDeviceInfo)
        {
            std::cerr << "Error: Can't get default input device information" << std::endl;

            return PlayerError::PCMError;
        }

        inputStreamParameters.device = inputDevice;
        inputStreamParameters.channelCount = channelsCount;
        inputStreamParameters.sampleFormat = sampleFormat;
        inputStreamParameters.suggestedLatency = inputDeviceInfo->defaultHighInputLatency;
    }

	returnCode = Pa_OpenStream(
		&mStream,
        record ? &inputStreamParameters : nullptr,
		play ? &outputStreamParameters : nullptr,
		samplesPerSecond,
		paFramesPerBufferUnspecified, // framesPerBuffer
		0, // flags
		&StreamCallback,
		this //userData
		);

	if(returnCode != paNoError)
    {
        std::cerr << "Error: Can't open stream, " << Pa_GetErrorText(returnCode) << std::endl;

        return PlayerError::PCMError;
	}

	returnCode = Pa_StartStream(mStream);
    if(returnCode != paNoError)
    {
        std::cerr << "Error: Can't start stream, " << Pa_GetErrorText(returnCode) << std::endl;

        return PlayerError::PCMError;
	}

    return PlayerError::OK;
}

/**
 *******************************************************************************
 * @fn Release
 *******************************************************************************
 */
void                PortPlayer::Close()
{
    if(mStream)
    {
        Pa_StopStream(mStream);
        Pa_CloseStream(mStream);

        mStream = nullptr;
    }

    mFifoPlayBuffer.Reset(0);
    mFifoRecordBuffer.Reset(0);

    auto returnCode(Pa_Terminate());
    if(returnCode != paNoError)
    {
        std::cerr << "Error: Can't terminate PortAudio, " << Pa_GetErrorText(returnCode) << std::endl;
    }
}

int                 PortPlayer::StreamCallback
(
    const void *    input,
    void *          output,
    unsigned long   frameCount,
    const PaStreamCallbackTimeInfo *
                    timeInfo,
    PaStreamCallbackFlags
                    statusFlags,
    void *          userData
)
{
    auto thisPointer = static_cast<PortPlayer *>(userData);

    return thisPointer->StreamCallbackImplementation(
        input,
        output,
        frameCount,
        timeInfo,
        statusFlags
        );
}

int                 PortPlayer::StreamCallbackImplementation
(
    const void *    input,
    void *          output,
    unsigned long   frameCount,
    const PaStreamCallbackTimeInfo *
                    timeInfo,
    PaStreamCallbackFlags
                    statusFlags
)
{
    //std::memcpy(output, input, frameCount * mChannelsCount * (mBitsPerSample / 8));
    //return paContinue;

    auto dataSize(frameCount * mChannelsCount * (mBitsPerSample / 8));

    //play
    auto sizePlayed(mFifoPlayBuffer.Read(static_cast<uint8_t *>(output), dataSize));

    if(sizePlayed < dataSize)
    {
        std::memset(static_cast<uint8_t *>(output) + sizePlayed, 0, dataSize - sizePlayed);
    }

    //record
    if(input)
    {
        auto sizeRecorded(
            mFifoRecordBuffer.Write(
                static_cast<const uint8_t *>(input), dataSize
                )
            );
    }

	return paContinue;
}

/**
 *******************************************************************************
 * @fn Play
 * @brief Play output
 *******************************************************************************
 */
uint32_t PortPlayer::Play(uint8_t * outputBuffer, uint32_t size, bool mute)
{
    return mFifoPlayBuffer.Write(outputBuffer, size);
}

/**
*******************************************************************************
* @fn Record
*******************************************************************************
*/
uint32_t PortPlayer::Record(uint8_t * outputBuffer, uint32_t size)
{
    return mFifoRecordBuffer.Read(outputBuffer, size);
}