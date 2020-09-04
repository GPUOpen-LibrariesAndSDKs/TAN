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

#include <cstdint>

#include "tanlibrary/include/TrueAudioNext.h"       //TAN
#include "../TrueAudioVR/TrueAudioVR.h"

#include "fifo.h"
#include "maxlimits.h"
#include "threads.h"
#include "IWavPlayer.h"
#include "wav.h"
#include "Timer.h"
#include "Allocators.h"

#include <memory>
#include <string>
#include <vector>
#include <array>

#ifdef USE_ASIO
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h" 
#endif

// rotation, translation matrix
class transRotMtx{
private:
	float m[3][4];
public:
	transRotMtx();
	void setAngles(float yaw, float pitch, float roll);
	void setOffset(float x, float y, float z);
	void transform(float &X, float &Y, float &Z);
};

enum class ProcessingType
{
    ProcessingType_CPU = 1 << 0,
    ProcessingType_GPU = 1 << 1
};

// Simple VR audio engine using True Audio Next GPU acceleration
class Audio3D
{
public:
    Audio3D();
    Audio3D(Audio3D const &) = delete;
    virtual ~Audio3D();

    int Init
    (
        const std::string &     dllPath,
        const RoomDefinition &  roomDef,
        const std::vector<std::string> &
                                fileNames2Open,

        // Source 1 can optionally be captured audio from default microphone:
        bool                    useMicSource,
        bool                    trackHeadPos,

        int                     fftLen,
        int                     bufSize,

        bool                    useGPU_Conv,
        bool                    useGPU_ConvQueue,
        int                     devIdx_Conv,

//#ifdef RTQ_ENABLED
		bool                    useHPr_Conv,
        bool                    useRTQ_Conv,
        int                     cuRes_Conv,
//#endif // RTQ_ENABLED

        bool                    useGPU_IRGen,
        bool                    useCPU_IRGenQueue,
        int                     devIdx_IRGen,

//#ifdef RTQ_ENABLED
		bool                    useHPr_IRGen,
        bool                    useRTQ_IRGen,
        int                     cuRes_IRGen,
//#endif

        amf::TAN_CONVOLUTION_METHOD
                                convMethod,

        const std::string &     playerType
        );

	// finalize, deallocate resources, close files, etc.
	int Close();

	// start audio engine:
    int Run();

	// Stop audio engine:
    bool Stop();
	int ProcessNextBlock(int16_t **pOut, int16_t **pRec, uint32_t sampleCount);
	//int ProcessNextBlock1ch(int16_t *pOut, int16_t *pRec, uint32_t sampleCount);

protected:
    static bool useIntrinsics;
    static const int IR_UPDATE_MODE = 1; // 0: Non-Blocking 1: Blocking
    static unsigned processThreadProc(void *ptr);
    static unsigned updateThreadProc(void *ptr);

    PrioritizedThread mProcessThread;
    PrioritizedThread mUpdateThread;

    int Process(int16_t * pOut, int16_t * pChan[MAX_SOURCES], uint32_t sampleCount);

    int ProcessProc();
    int UpdateProc();

    bool mRunning = false;
    bool mUpdated = false;
    bool mStop = false;
    bool mUpdateParams = true;
    bool m_useOCLOutputPipeline;

	int16_t *pWaves[MAX_SOURCES] = { nullptr };
	int16_t *pWaveStarts[MAX_SOURCES] = { nullptr };

	uint32_t waveSizesInBytes[MAX_SOURCES] = { 0 };
	uint32_t waveBytesPlayed[MAX_SOURCES] = { 0 };


#ifdef USE_ASIO
	FifoBuffer *outFifos[2];
	FifoBuffer *inFifos[2];
	short *outsamplesLeft, *outsamplesRight;

#endif

	std::string      mplayerType;
    std::unique_ptr<IWavPlayer> mPlayer; //todo: dynamic creation of choosen player
	std::vector<WavContent>     mWavFiles;

    //Timer                       mRealtimeTimer;

    uint32_t                    mMaxSamplesCount = 0;
    std::vector<int16_t>        mStereoProcessedBuffer;
	int16_t *mProcessedStereo;

	TANContextPtr mTANConvolutionContext;
	TANContextPtr mTANRoomContext;

	TANConvolutionPtr m_spConvolution;
	TANConverterPtr m_spConverter;
    TANMixerPtr m_spMixer;
	TANFFTPtr m_spFft;
	AmdTrueAudioVR *m_pTAVR = NULL;

    RoomDefinition room;
    MonoSource sources[MAX_SOURCES];
	StereoListener ears;

    bool mSrc1EnableMic = false;
    bool mSrc1TrackHeadPos = false;
    bool mSrc1MuteDirectPath = false;

    AllignedAllocator<float, 32>mResponseBufferStorage;
	float *mResponseBuffer = nullptr;
    
    float *mResponses[MAX_SOURCES * 2] = {nullptr};
    cl_mem mOCLResponses[MAX_SOURCES * 2] = {nullptr};
    bool   mUseClMemBufs = false;

    //attention:
    //the following buffers must be 32-bit aligned to use AVX/SSE instructions

    AllignedAllocator<float, 32>mInputFloatBufsStorage[MAX_SOURCES * 2];
    float *mInputFloatBufs[MAX_SOURCES * 2] = {nullptr};

    AllignedAllocator<float, 32>mOutputFloatBufsStorage[MAX_SOURCES * 2];
    float *mOutputFloatBufs[MAX_SOURCES * 2] = {nullptr};

    AllignedAllocator<float, 32>mOutputMixFloatBufsStorage[STEREO_CHANNELS_COUNT];
    float *mOutputMixFloatBufs[2] = {nullptr};

	cl_mem mOutputCLBufs[MAX_SOURCES * 2] = {nullptr};
	cl_mem mOutputMainCLbuf = nullptr;
	cl_mem mOutputMixCLBufs[2] = {nullptr};
	cl_mem mOutputShortBuf = nullptr;

	// current position in each stream:
	//int64_t m_samplePos[MAX_SOURCES];

	// fft length must be power of 2
	// 65536/48000 = reverb time of 1.37 seconds
    int m_fftLen = 65536;  //default fft length

	// buffer length 4096 / 48000 = 85 ms update rate:
	//int m_bufSize = 4096 * 4; // default buffer length
    uint32_t mBufferSizeInSamples = 0;
    uint32_t mBufferSizeInBytes = 0;

	// World To Room coordinate transform:
	transRotMtx m_mtxWorldToRoomCoords;
	float m_headingOffset;
	bool m_headingCCW;

    // RT-Queues
    cl_command_queue mCmdQueue1 = nullptr;
    cl_command_queue mCmdQueue2 = nullptr;
    cl_command_queue mCmdQueue3 = nullptr;

public:
    /*
    m_pAudioVR->init(room, dlg.nFiles, dlg.waveFileNames, dlg.convolutionLength, dlg.bufferSize,
    dlg.useGPU4Conv, dlg.convDevIdx, dlg.useRTQ4Conv, dlg.cuCountConv,
    dlg.useGPU4Room, dlg.roomDevIdx, dlg.useRTQ4Room, dlg.cuCountRoom);
    */
	// Initialize room acoustics model, WASAPI audio, and TAN convolution:
    //int init(RoomDefinition roomDef, int nFiles, char **inFiles, int fftLen, int bufSize, bool useGPU_Conv = true);

    //static amf::TAN_CONVOLUTION_METHOD m_convMethod;// = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;

    //todo: move to Init with help of struct
    // Set transform to translate game coordinates to room audio coordinates:
	int setWorldToRoomCoordTransform(
        float translationX,
        float translationY,
        float translationZ,
        float rotationY,
        float headingOffset,
        bool headingCCW
        );

	// get's the current playback position in a stream:
    //int64_t getCurrentPosition(int stream);

	// update the head (listener) position:
    int updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll);

	//update a source position:
    int updateSourcePosition(int srcNumber, float x, float y, float z);

	//update a room's dimension:
	int updateRoomDimension(float _width, float _height, float _length);

	//update a room's damping factor
	int updateRoomDamping(float _left, float _right, float _top, float _buttom, float _front, float _back);

    // export impulse response for source  + current listener and room:
    int exportImpulseResponse(int srcNumber, char * fname);
	AmdTrueAudioVR* getAMDTrueAudioVR();
	TANConverterPtr getTANConverter();
};

#ifndef CLQUEUE_REFCOUNT
#ifdef _DEBUG
#define CLQUEUE_REFCOUNT( clqueue ) { \
		cl_uint refcount = 0; \
		clGetCommandQueueInfo(clqueue, CL_QUEUE_REFERENCE_COUNT, sizeof(refcount), &refcount, NULL); \
		printf("\nFILE:%s line:%d Queue %llX ref count: %d\r\n", __FILE__ , __LINE__, clqueue, refcount); \
}
#else
#define CLQUEUE_REFCOUNT( clqueue ) { \
}
#endif
#endif