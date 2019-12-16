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

#include <Windows.h>
//#include <VersionHelpers.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <mmsystem.h>
#include <Audioclient.h>
#include <endpointvolume.h>

#define ONE_SEC_BUFFER_SIZE   (10000000)
#define MS500_BUFFER_SIZE      (5000000)
#define MS100_BUFFER_SIZE      (1000000)
#define FIFTY_MS_BUFFER_SIZE    (500000)
#define SIXTH_SEC_BUFFER_SIZE  (10000000/6) //@48000/6 = 8000 samples
#define BUFFER_SIZE_8K	       (1706667)
#define ONE_SEC               (1000)
#define CHECKHR(hr, msg) { if (hr) {std::cerr << msg; return hr;} }
#define SAFE_RELEASE(pointer) if(pointer){pointer->Release(); pointer = NULL;}

/*Structure which has all elements related to stream */
typedef struct streaminfo
{
    /* Number of channels 1=Mono 2=Stereo   */
    WORD NumOfChannels;
    /* Sampling Frequency in Hz   */
    DWORD SamplesPerSec;
    /*Num of bytes Decoded */
    DWORD DecodedBytes;
    /*Num of bits per sample for all channels*/
    INT bitsPerSample;
} STREAMINFO;


/*Structure which hold elements required for wasapi playback */
typedef struct wasapi
{
    IMMDevice *dev;
    IMMDeviceEnumerator *devEnum;
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
	IAudioClient *audioCapClient;
	IAudioCaptureClient *captureClient;
	UINT frameSize;
	UINT bufferSize;
    bool started;
	UINT init;
//	UINT play;
} WASAPISTRUCT;

int wasapiInit(WASAPISTRUCT *wasapi, STREAMINFO *streaminfo, int playBack, UINT *bufferSize, UINT *frameSize, AUDCLNT_SHAREMODE sharMode, BOOL offload, BOOL loopBack);
INT wasapiPlay(WASAPISTRUCT *wasapi,unsigned char *pOutputBuffer, unsigned int size, bool mute);
INT wasapiRecord(WASAPISTRUCT *wasapi, unsigned char *pOutputBuffer, unsigned int size);
void wasapiRelease(WASAPISTRUCT *wasapi, int playBack);
void wasapiSetMute(BOOL mute);
int ReadWaveFile(const char *inFile, WASAPISTRUCT *pWasapiHandle, long *pNsamples, unsigned char **ppOutBuffer);
int PlayQueuedSamples(WASAPISTRUCT *pWasapiHandle, int nSamples, unsigned char *pOutBuffer);
int PlayQueuedStreams(int nStreams, WASAPISTRUCT *pWasapiHandles, long *sampleCounts, unsigned char **pOutBuffers,
				       WASAPISTRUCT *pWasapiLoopback, long lbSampleCount, unsigned char *lbBuffer);
int PlayWaveFile(const char *inFile, BOOL offload);
bool PlayQueuedStreamChunk(bool init, int nStreams, WASAPISTRUCT *pWasapiHandles, long *sampleCounts, unsigned char **pOutBuffers,
				       WASAPISTRUCT *pWasapiLoopback, long lbSampleCount, unsigned char *lbBuffer);
#endif /* _WASAPI_H_ */

/*End Of File */
