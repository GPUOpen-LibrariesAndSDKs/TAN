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

#include "WASAPIPlayer.h"
#include "wav.h"

#define MAXFILES 100
int muteInit = 1;
IAudioEndpointVolume *g_pEndptVol = NULL;

WASAPIPlayer::WASAPIPlayer()
{
}

WASAPIPlayer::~WASAPIPlayer()
{
    Close();
}

PlayerError WASAPIPlayer::Init
(
    uint16_t    channelsCount,
    uint16_t    bitsPerSample,
    uint32_t    samplesPerSecond,
    bool        play,
    bool        record
)
{
	mChannelsCount = channelsCount;
	mBitsPerSample = bitsPerSample;
	mSamplesPerSecond = samplesPerSecond;

    HRESULT hr;

    REFERENCE_TIME bufferDuration = (BUFFER_SIZE_8K);//(SIXTH_SEC_BUFFER_SIZE); //(MS100_BUFFER_SIZE); // (ONE_SEC_BUFFER_SIZE);

	WAVEFORMATEXTENSIBLE mixFormat = { 0 };

    /* PCM audio */
    mixFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    /* Default is 44100 */
    mixFormat.Format.nSamplesPerSec = samplesPerSecond;
    /* Default is 2 channels */
    mixFormat.Format.nChannels = (WORD)channelsCount;
    /* Default is 16 bit */
    mixFormat.Format.wBitsPerSample = (WORD)bitsPerSample;
    mixFormat.Format.cbSize = sizeof(mixFormat) - sizeof(WAVEFORMATEX);
    /* nChannels * bitsPerSample / BitsPerByte (8) = 4 */
    mixFormat.Format.nBlockAlign = (WORD)(channelsCount * bitsPerSample / 8);
    /* samples per sec * blockAllign */
    mixFormat.Format.nAvgBytesPerSec = samplesPerSecond * (channelsCount * bitsPerSample / 8);
    mixFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    mixFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    mixFormat.Samples.wValidBitsPerSample = 16;

    /* Let's see if this is supported */
    WAVEFORMATEX* format = nullptr;
    format = (WAVEFORMATEX*) &mixFormat;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **) &mDevEnum);
    if (FAILED(hr))
    {
        //"Failed getting MMDeviceEnumerator."
        return PlayerError::PCMError;
    }

    if(record)
    {
        if (mInitializedCapture)
        {
            return PlayerError::OK;
        }

        hr = mDevEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &mDevCapture);
        LOGERROR(hr, "Failed to getdefaultaudioendpoint for Capture.");

        if (mDevCapture) {
            hr = mDevCapture->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&mAudioCapClient);
            LOGERROR(hr, "Failed capture activate.");
        }
        
		if (mAudioCapClient){
            hr = mAudioCapClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST, bufferDuration, 0, format, NULL);
            if (hr != S_OK) {
                hr = mAudioCapClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferDuration, 0, format, NULL);
            }
            
            if (FAILED(hr))
            {
                //"Failed audioCapClient->Initialize"
                return PlayerError::PCMError;
            }
            
            hr = mAudioCapClient->GetService(__uuidof(IAudioCaptureClient), (void **)&mCaptureClient);
            if (FAILED(hr))
            {
                //"Failed getting captureClient"
                return PlayerError::PCMError;
            }
            
            hr = mAudioCapClient->GetBufferSize(&mBufferSize);
            if (FAILED(hr))
            {
                //"Failed getting BufferSize"
                return PlayerError::PCMError;
            }
        }
        mStartedCapture = false;
        mInitializedCapture = true;

    }
    else {
        if (mInitializedRender)
        {
            return PlayerError::OK;
        }

        hr = mDevEnum->GetDefaultAudioEndpoint(eRender, eConsole, &mDevRender);
        //FAILONERROR(hr, "Failed to getdefaultaudioendpoint for Render.");
        if (FAILED(hr))
        {
            //"Failed getting BufferSize"
            return PlayerError::PCMError;
        }

        if (mDevRender) {
            hr = mDevRender->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&mAudioClient);
            //FAILONERROR(hr, "Failed render activate.");
            if (FAILED(hr))
            {
                //"Failed getting BufferSize"
                return PlayerError::PCMError;
            }
        }
        if(!muteInit)
        {
            hr = mDevRender->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)&g_pEndptVol);
            //FAILONERROR(hr, "Failed Mute Init.");
            if (FAILED(hr))
            {
                //"Failed getting BufferSize"
                return PlayerError::PCMError;
            }

            muteInit =1;
        }

       hr = mAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST, bufferDuration, 0, format, NULL);
        if (hr != S_OK) {
            hr = mAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferDuration, 0, format, NULL);
        }
        //FAILONERROR(hr, "Failed audioClient->Initialize");
        if (FAILED(hr))
        {
            //"Failed getting BufferSize"
            return PlayerError::PCMError;
        }

        hr = mAudioClient->GetService(__uuidof(IAudioRenderClient), (void **) &mRenderClient);
        //FAILONERROR(hr, "Failed getting renderClient");
        if (FAILED(hr))
        {
            //"Failed getting BufferSize"
            return PlayerError::PCMError;
        }

        hr = mAudioClient->GetBufferSize(&mBufferSize);
        //FAILONERROR(hr, "Failed getting BufferSize");
        if (FAILED(hr))
        {
            //"Failed getting BufferSize"
            return PlayerError::PCMError;
        }

        mStartedRender = false;
        mInitializedRender = true;
    }

    return PlayerError::OK;
}

void WASAPIPlayer::Close()
{
    // Reset system timer
    //timeEndPeriod(1);
    if (mAudioClient)
        mAudioClient->Stop();

    SAFE_RELEASE(mRenderClient);
    SAFE_RELEASE(mAudioClient);
    SAFE_RELEASE(mDevRender);
    SAFE_RELEASE(mDevCapture);
    SAFE_RELEASE(mDevEnum);

	mInitializedRender = false;
	mInitializedCapture = false;
}

uint32_t WASAPIPlayer::Play(uint8_t * buffer2Play, uint32_t sizeInBytes, bool mute)
{
	if(!mAudioClient || !mRenderClient)
        return 0;

	HRESULT hr;
    
	UINT paddingFrames = 0;
    hr = mAudioClient->GetCurrentPadding(&paddingFrames);
	if(FAILED(hr))
	{
		return 0;
	}

	UINT availableFreeBufferSizeInFrames = sizeInBytes / GetSampleSizeInBytes() - paddingFrames;

	UINT frames2Copy = min(availableFreeBufferSizeInFrames, sizeInBytes / GetSampleSizeInBytes());

	BYTE *buffer = nullptr;
	hr = mRenderClient->GetBuffer(frames2Copy, &buffer);
	if(FAILED(hr))
	{
		return 0;
	}

	if(mute)
	{
		memset(buffer, 0, (frames2Copy * GetSampleSizeInBytes()));
	}
	else
	{
		memcpy(buffer, buffer2Play, (frames2Copy * GetSampleSizeInBytes()));
	}

    hr = mRenderClient->ReleaseBuffer(frames2Copy, NULL);
	if(FAILED(hr))
	{
		return 0;
	}

    if(!mStartedRender)
    {
        mStartedRender = TRUE;
        mAudioClient->Start();
    }

    return (frames2Copy * GetSampleSizeInBytes());
}

uint32_t WASAPIPlayer::Record(uint8_t * buffer, uint32_t size)
{
    if(!mCaptureClient)
        return 0;

    HRESULT hr;
    
    if(!mStartedCapture)
    {
        mStartedCapture = TRUE;
        hr = mAudioCapClient->Start();
    }

    //hr = audioClient->GetCurrentPadding(&padding);
	UINT32 frames(0);
	hr = mCaptureClient->GetNextPacketSize(&frames);
    if(FAILED(hr) || !frames)
	{
		return 0;
	}

	frames = min(frames, size / GetSampleSizeInBytes());

	BYTE *data = NULL;
	DWORD flags(0);
    //UINT64 DevPosition;
    //UINT64 QPCPosition;
    hr = mCaptureClient->GetBuffer((BYTE **)&data, &frames, &flags, NULL, NULL );
    if(FAILED(hr))
	{
		return 0;
	}

    memcpy(buffer, data, frames * GetSampleSizeInBytes());

    hr = mCaptureClient->ReleaseBuffer(frames);
	if(FAILED(hr))
	{
		return 0;
	}

	return frames * GetSampleSizeInBytes();
}