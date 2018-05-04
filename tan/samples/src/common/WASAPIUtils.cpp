//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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

#include "WASAPIUtils.h"
#include "wav.h"

#define MAXFILES 100
int muteInit = 1;
IAudioEndpointVolume *g_pEndptVol = NULL;

WASAPIUtils::WASAPIUtils(){
    startedRender = false;
    startedCapture = false;
    initializedRender = false;
    initializedCapture = false;
    devRender = devCapture = NULL;

    devRender = NULL;
    devCapture = NULL;
    devEnum = NULL;
    audioClient = NULL;
    renderClient = NULL;
    audioCapClient = NULL;
    captureClient = NULL;

}

WASAPIUtils::~WASAPIUtils(){
    wasapiRelease();
}


/**
 *******************************************************************************
 * @fn wasapiInit
 * @brief Will export stream header contents
 *
 * @param[in/out] mp3Decoder    : Points to structure which holds
 *                                elements required for MP3Decoding
 *
 * @return INT
 *         0   for success
 *         >0  for failure
 *
 *******************************************************************************
 */
int WASAPIUtils::wasapiInit(STREAMINFO *streaminfo, UINT *bufferSize, UINT *frameSize, AUDCLNT_SHAREMODE sharMode, bool capture)
{
    HRESULT hr;

    INT sampleRate = streaminfo->SamplesPerSec;
    INT numCh = streaminfo->NumOfChannels;
    INT bitsPerSample = streaminfo->bitsPerSample;

    REFERENCE_TIME bufferDuration = (BUFFER_SIZE_8K);//(SIXTH_SEC_BUFFER_SIZE); //(MS100_BUFFER_SIZE); // (ONE_SEC_BUFFER_SIZE);
 
    WAVEFORMATEXTENSIBLE mixFormat;

    /* PCM audio */
    mixFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

    /* Default is 44100 */
    mixFormat.Format.nSamplesPerSec = sampleRate;

    /* Default is 2 channels */
    mixFormat.Format.nChannels = (WORD) numCh;

    /* Default is 16 bit */
    mixFormat.Format.wBitsPerSample = (WORD) bitsPerSample;

    mixFormat.Format.cbSize = sizeof(mixFormat) - sizeof(WAVEFORMATEX);

    /* nChannels * bitsPerSample / BitsPerByte (8) = 4 */
    mixFormat.Format.nBlockAlign = (WORD)(numCh * bitsPerSample / 8);

    /* samples per sec * blockAllign */
    mixFormat.Format.nAvgBytesPerSec = sampleRate * (numCh * bitsPerSample / 8);

    mixFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    mixFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    mixFormat.Samples.wValidBitsPerSample = 16;

    /* Let's see if this is supported */ 
    WAVEFORMATEX* format = NULL;
    format = (WAVEFORMATEX*) &mixFormat;

    (*frameSize) = (format->nChannels * format->wBitsPerSample / 8);

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **) &devEnum);
    FAILONERROR(hr, "Failed getting MMDeviceEnumerator.");

    if (capture){
        if (initializedCapture){
            return 0;
        }
        devCapture = NULL;
        audioCapClient = NULL;
        hr = devEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &devCapture);
        LOGERROR(hr, "Failed to getdefaultaudioendpoint for Capture.");
        if (devCapture) {
            hr = devCapture->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioCapClient);
            LOGERROR(hr, "Failed capture activate.");
        }
        captureClient = NULL;
        if (audioCapClient){
            hr = audioCapClient->Initialize(sharMode, AUDCLNT_STREAMFLAGS_RATEADJUST, bufferDuration, 0, format, NULL);
            if (hr != S_OK) {
                hr = audioCapClient->Initialize(sharMode, 0, bufferDuration, 0, format, NULL);
            }
            FAILONERROR(hr, "Failed audioCapClient->Initialize");
            hr = audioCapClient->GetService(__uuidof(IAudioCaptureClient), (void **)&captureClient);
            FAILONERROR(hr, "Failed getting captureClient");
            hr = audioCapClient->GetBufferSize(bufferSize);
            FAILONERROR(hr, "Failed getting BufferSize");
        }
        startedCapture = false;
        initializedCapture = true;

    }
    else {
        if (initializedRender){
            return 0;
        }
        devRender = NULL;
        audioClient = NULL;
        hr = devEnum->GetDefaultAudioEndpoint(eRender, eConsole, &devRender);
        FAILONERROR(hr, "Failed to getdefaultaudioendpoint for Render.");
        if (devRender) {
            hr = devRender->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient);
            FAILONERROR(hr, "Failed render activate.");
        }
        if(!muteInit)
        {
            hr = devRender->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)&g_pEndptVol);
            FAILONERROR(hr, "Failed Mute Init.");
            muteInit =1;
        }
       hr = audioClient->Initialize(sharMode, AUDCLNT_STREAMFLAGS_RATEADJUST, bufferDuration, 0, format, NULL);
        if (hr != S_OK) {
            hr = audioClient->Initialize(sharMode, 0, bufferDuration, 0, format, NULL);
        }
        FAILONERROR(hr, "Failed audioClient->Initialize");
        hr = audioClient->GetService(__uuidof(IAudioRenderClient), (void **) &renderClient);
        FAILONERROR(hr, "Failed getting renderClient");
        hr = audioClient->GetBufferSize(bufferSize);
        FAILONERROR(hr, "Failed getting BufferSize");
        startedRender = false;
        initializedRender = true;

    }



    //AUDIO_STREAM_CATEGORY AudioCategory_ForegroundOnlyMedia
    //BOOL isOffloadCapable = false;

    return hr;
}



/**
 *******************************************************************************
 * @fn wasapiRelease
 * @brief Play output using Wasapi Application
 *
 * @param[in/out] mp3Decoder    : Points to structure which holds
 *                                elements required for MP3Decoding
 *
 * @return INT
 *         0   for success
 *         >0  for failure
 *
 *******************************************************************************
 */


void WASAPIUtils::wasapiRelease()
{

    // Reset system timer
    //timeEndPeriod(1);
    if (audioClient)
        audioClient->Stop();
    SAFE_RELEASE(renderClient);
    SAFE_RELEASE(audioClient);
    SAFE_RELEASE(devRender);
    SAFE_RELEASE(devCapture);
    SAFE_RELEASE(devEnum);
	initializedRender = false;
	initializedCapture = false;
}

int WASAPIUtils::QueueWaveFile(char *inFile,long *pNsamples, unsigned char **ppOutBuffer)
{
    STREAMINFO          streaminfo;

    int samplesPerSec = 0;
    int bitsPerSample = 0;
    int nChannels = 0;
    float **pSamples;
    unsigned char *pOutBuffer;

    if (!ReadWaveFile(inFile, &samplesPerSec, &bitsPerSample, &nChannels, pNsamples, &pOutBuffer, &pSamples)){
        strncat_s(inFile, MAX_PATH, " >>>>ERROR: failed to load!", MAX_PATH - strlen(inFile));
        FAILONERROR(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), "Failed to read wave file %s");
    }

    if (nChannels != 2 || bitsPerSample != 16) {
        free(pOutBuffer);
        pOutBuffer = (unsigned char *)calloc(*pNsamples, 2 * sizeof(short));
        if (!pOutBuffer) return -1;
        
        short *pSBuf = (short *)pOutBuffer;
        for (int i = 0; i < *pNsamples; i++)
        {
            pSBuf[2 * i + 1] = pSBuf[2 * i] = (short)(32767 * pSamples[0][i]);
            if (nChannels == 2){
                pSBuf[2 * i + 1] = (short)(32767 * pSamples[1][i]);
            }
        }
    }
    //don't need floats;
    for (int i = 0; i < nChannels; i++){
        delete pSamples[i];
    }
    delete pSamples;

    *ppOutBuffer = pOutBuffer;

    memset(&streaminfo, 0, sizeof(STREAMINFO));
    streaminfo.bitsPerSample = 16;
    streaminfo.NumOfChannels = 2;
    streaminfo.SamplesPerSec = samplesPerSec;// 48000;
    int result = wasapiInit( &streaminfo,  &bufferSize, &frameSize, AUDCLNT_SHAREMODE_SHARED);
    return result;
}

/**
 *******************************************************************************
 * @fn wasapiPlay
 * @brief Play output using Wasapi Application
 *
 * @param[in/out] mp3Decoder    : Points to structure which holds
 *                                elements required for MP3Decoding
 *
 * @return INT
 *         0   for success
 *         >0  for failure
 *
 *******************************************************************************
 */
INT WASAPIUtils::wasapiPlay(unsigned char *pOutputBuffer, unsigned int size, bool mute)
{
    if (audioClient == NULL || renderClient==NULL)
        return 0;

    HRESULT hr;
    UINT padding = 0;
    UINT availableFreeBufferSize = 0;
    UINT frames;
    CHAR *buffer = NULL;

    hr = audioClient->GetCurrentPadding(&padding);
    FAILONERROR(hr, "Failed getCurrentPadding");

    availableFreeBufferSize = bufferSize - padding;

    frames = min(availableFreeBufferSize/frameSize, size/frameSize);

    hr = renderClient->GetBuffer(frames, (BYTE **) &buffer);
    FAILONERROR(hr, "Failed getBuffer");

    if (mute)
        memset(buffer, 0, (frames*frameSize));
    else
        memcpy(buffer, pOutputBuffer, (frames*frameSize));

    hr = renderClient->ReleaseBuffer(frames, NULL);
    FAILONERROR(hr, "Failed releaseBuffer");

    if (!startedRender)
    {
        startedRender = TRUE;
        audioClient->Start();
    }
    
    return  (frames*frameSize);
}

/**
*******************************************************************************
* @fn wasapiRecord
* @brief Play output using Wasapi Application
*
* @param[in/out]     : Points to structure 
*                  
*
* @return INT
*         0   for success
*         >0  for failure
*
*******************************************************************************
*/
INT WASAPIUtils::wasapiRecord( unsigned char *pOutputBuffer, unsigned int size)
{
    if (captureClient == NULL)
        return 0;

    HRESULT hr;
    UINT32 frames;
    CHAR *buffer = NULL;

    if (!startedCapture)
    {
        startedCapture = TRUE;
        hr = audioCapClient->Start();
    }

    //hr = audioClient->GetCurrentPadding(&padding);
    hr = captureClient->GetNextPacketSize(&frames);
    FAILONERROR(hr, "Failed GetNextPacketSize");
    if (frames == 0) {
        return frames;
    }

    frames = min(frames, size / frameSize);


    DWORD flags;
    //UINT64 DevPosition;
    //UINT64 QPCPosition;
    hr = captureClient->GetBuffer( (BYTE **)&buffer, &frames, &flags, NULL, NULL );
    FAILONERROR(hr, "Failed getBuffer");

    memcpy(pOutputBuffer, buffer, (frames*frameSize));

    hr = captureClient->ReleaseBuffer(frames);
    FAILONERROR(hr, "Failed releaseBuffer");

    return  (frames*frameSize);
}

bool WASAPIUtils::PlayQueuedStreamChunk(bool init, long sampleCount, unsigned char *pOutBuffer )
{
    static int bytesPlayed;
    static int bytesRecorded;
    static bool done = false;
    static unsigned char *pData;
    static int size;

    if(init) {
            pData = pOutBuffer;
            size = sampleCount * frameSize;
        return false;
    }

    done = true;
    if (size > 0){
        bytesPlayed = wasapiPlay( pData, size, false);
        pData += bytesPlayed;
        size -= bytesPlayed;
        done = false;
        //printf("stream%d: %d bytes played\n",i,bytesPlayed);
        Sleep(0);
    }
    Sleep(5);

    return done;
}



