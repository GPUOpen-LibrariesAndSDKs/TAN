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
#include "SimpleVRaudio.h"

#include "../TrueAudioVR/TrueAudioVR.h"
#include "GpuUtils.h"
#include "Utilities.h"
#include "cpucaps.h"

#include <time.h>
#include <stdio.h>
#include <CL/cl.h>

#if defined(_WIN32)
    #include "../common/WASAPIPlayer.h"
#else
	#if !defined(__MACOSX) && !defined(__APPLE__)
		#include "../common/AlsaPlayer.h"
	#endif
#endif

#ifdef ENABLE_PORTAUDIO
	#include "../common/PortPlayer.h"
#endif

#include <immintrin.h>
#include <cmath>
#include <cstring>
#include <chrono>
#include <algorithm>

//to format debug outputs
/**/
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
/**/

//const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
bool Audio3D::useIntrinsics = true; // InstructionSet::AVX() && InstructionSet::FMA();

#ifndef ERROR_MESSAGE

#ifdef _WIN32
#define ERROR_MESSAGE(message) ::MessageBoxA(0, #message, "Error", MB_OK)
#else
#define ERROR_MESSAGE(message) __asm__("int3"); std::cerr << "Error: " << message << std::endl
#endif

#endif

#define RETURN_IF_FAILED(x) \
{ \
    AMF_RESULT tmp = (x); \
    if (tmp != AMF_OK) { \
        ERROR_MESSAGE(int(x)); \
        return -1; \
    } \
}

#define RETURN_IF_FALSE(x) \
{ \
    bool tmp = (x); \
    if (!tmp) { \
        ERROR_MESSAGE(int(x)); \
        return -1; \
    } \
}

#define SAFE_DELETE_ARR(x) if (x) { delete[] x; x = nullptr; }

transRotMtx::transRotMtx(){
    memset(m, 0, sizeof(m));
    m[0][0] = 1.0;
    m[1][1] = 1.0;
    m[2][2] = 1.0;
}

void transRotMtx::setAngles(float yaw, float pitch, float roll)
{
    float sinY = std::sin(yaw * (float)PI / 180);
    float cosY = std::cos(yaw * (float)PI / 180);
    float sinP = std::sin(pitch * (float)PI / 180);
    float cosP = std::cos(pitch * (float)PI / 180);
    float sinR = std::sin(roll * (float)PI / 180);
    float cosR = std::cos(roll * (float)PI / 180);

    m[0][0] = cosR*cosY - sinR*sinP*sinY;
    m[0][1] = -sinR*cosP;
    m[0][2] = cosR*sinY + sinR*sinP*cosY;
    m[1][0] = sinR*cosY + cosR*sinP*sinY;
    m[1][1] = cosR*cosP;
    m[1][2] = sinR*sinY - cosR*sinP*cosY;
    m[2][0] = -cosP*sinY;
    m[2][1] = sinP;
    m[2][2] = cosP*cosY;
}

void transRotMtx::setOffset(float x, float y, float z){
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}

void transRotMtx::transform(float &X, float &Y, float &Z)
{
    float x = X;
    float y = Y;
    float z = Z;
    X = x*m[0][0] + y*m[0][1] + z*m[0][2] + m[0][3];
    Y = x*m[1][0] + y*m[1][1] + z*m[1][2] + m[1][3];
    Z = x*m[2][0] + y*m[2][1] + z*m[2][2] + m[2][3];
}


unsigned Audio3D::processThreadProc(void * ptr)
{
    Audio3D *pAudio3D = static_cast<Audio3D*>(ptr);

    return pAudio3D->ProcessProc();
}

unsigned Audio3D::updateThreadProc(void * ptr)
{
    Audio3D *pAudio3D = static_cast<Audio3D*>(ptr);

    return pAudio3D->UpdateProc();
}

Audio3D::Audio3D():
    m_pTAVR(nullptr),
    mProcessThread(true),
    mStop(false),
    m_headingOffset(0.),
    m_headingCCW(true)
{
}

Audio3D::~Audio3D()
{
    //Close();
}

int Audio3D::Close()
{
    mRunning = false;

    if(mPlayer)
    {
        mPlayer->Close();
        mPlayer.reset();
    }

    // destroy dumb pointer:
    if (m_pTAVR != NULL) {
        delete m_pTAVR;
        m_pTAVR = nullptr;
    }

    for (int i = 0; i < MAX_SOURCES*2; i++)
    {
        if (mOCLResponses[i] == NULL) continue;
        clReleaseMemObject(mOCLResponses[i]);
        mOCLResponses[i] = NULL;
    }

    for (int i = 0; i < mWavFiles.size() * 2; i++)
    {
        if (mOutputCLBufs[i] == NULL) continue;
        clReleaseMemObject(mOutputCLBufs[i]);
        mOutputCLBufs[i] = NULL;
    }

    if (mOutputMainCLbuf != NULL)
    {
        clReleaseMemObject(mOutputMainCLbuf);
        mOutputMainCLbuf = NULL;
    }
    for (int i = 0; i < 2; i++)
    {
        if (mOutputMixCLBufs[i] == NULL) continue;
        clReleaseMemObject(mOutputMixCLBufs[i]);
        mOutputMixCLBufs[i] = NULL;
    }
    if (mOutputShortBuf)
    {
        clReleaseMemObject(mOutputShortBuf);
        mOutputShortBuf = NULL;
    }
    mUseClMemBufs = false;

    // release smart pointers:
    m_spFft.Release();
    m_spConvolution.Release();
    m_spMixer.Release();
    m_spConverter.Release();
    mTANRoomContext.Release();
    mTANConvolutionContext.Release();

    
    // Retain/release counts must match. 
	// Each component that is initialized with a clQueue should increment the ref count (clRetain) in init, and release on terminate.
	// This component (SimpleVR creates the queues (ref = 1 on create) so does not need a retain, but still needs to release one close.

	/************** WARNING MIS-MATCHED clRetain / clRelease CAN CAUSE KERNEL MODE HANG for RTQs ****************************
	 *                                                                                                                      *
	 *  When using AMD Real Time Queues (RTQ) There can be only one of each type of queue RTQ1, RTQ2.                       *
	 *  If these are not released, it will be impossible to create another RTQ.                                             *
	 *  The kernel mode driver (KMD) will HANG  waiting for the existing RTQ queue to be released,                          *
	 *  before it will create a new one.                                                                                    *
	 *                                                                                                                      *
	 *  [All queues are released when the process exits, however.]                                                          *
	 ************************************************************************************************************************/

    if (mCmdQueue1 != NULL){
		DBG_CLRELEASE(mCmdQueue1,"mCmdQueue1");
    }
    if (mCmdQueue2 != NULL){
		DBG_CLRELEASE(mCmdQueue2,"mCmdQueue2");
	}
    if (mCmdQueue3 != NULL ){
		DBG_CLRELEASE(mCmdQueue3,"mCmdQueue3");
	}
    

    mCmdQueue1 = NULL;
    mCmdQueue2 = NULL;
    mCmdQueue3 = NULL;

    mWavFiles.resize(0);

    return 0;
}

int Audio3D::Init
(
	const std::string &     dllPath,
	const RoomDefinition &  roomDef,

    const std::vector<std::string> &
                            inFiles,

    bool                    useMicSource,
    bool                    trackHeadPos,

	int                     fftLen,
	int                     bufferSizeInSamples,

    bool                    useGPU_Conv,
    bool                    useGPU_ConvQueue,
    int                     devIdx_Conv,

//#ifdef RTQ_ENABLED
	bool                    useHPr_Conv,
    bool                    useRTQ_Conv,
    int                     cuRes_Conv,
//#endif

    bool                    useGPU_IRGen,
    bool                    useGPU_IRGenQueue,
    int                     devIdx_IRGen,

//#ifdef RTQ_ENABLED
	bool                    useHPr_IRGen,
    bool                    useRTQ_IRGen,
    int                     cuRes_IRGen,
//#endif

    amf::TAN_CONVOLUTION_METHOD
                            convMethod,

    const std::string &     playerType
)
{
    if(useGPU_ConvQueue && !useGPU_Conv)
    {
        std::cerr
            << "Error: GPU queues must be used only with OpenCL Convolution processing!"
            << std::endl;

        return -1;
    }

    if(useGPU_IRGenQueue && !useGPU_IRGen)
    {
        std::cerr
            << "Error: GPU queues must be used only with OpenCL Convolution processing!"
            << std::endl;

        return -1;
    }

    m_useOCLOutputPipeline = useGPU_Conv && useGPU_IRGen;
    //m_useOCLOutputPipeline = useGPU_Conv || useGPU_IRGen;
    
    mSrc1EnableMic = useMicSource;
    mSrc1TrackHeadPos = trackHeadPos;

    mBufferSizeInSamples = bufferSizeInSamples;
    mBufferSizeInBytes = mBufferSizeInSamples * STEREO_CHANNELS_COUNT * sizeof(int16_t);

    mWavFiles.reserve(inFiles.size());

    for(const auto& fileName: inFiles)
    {
        WavContent content;

        if(content.ReadWaveFile(fileName))
        {
            if(content.SamplesPerSecond != FILTER_SAMPLE_RATE)
            {
                std::cerr
                    << "Error: file " << fileName << " has an unsupported frequency! Currently only "
                    << FILTER_SAMPLE_RATE << " frequency is supported!"
                    << std::endl;

                return -1;
            }

            if(content.BitsPerSample != 16)
            {
                std::cerr
                    << "Error: file " << fileName << " has an unsupported bits per sample count. Currently only "
                    << 16 << " bits is supported!"
                    << std::endl;

                return -1;
            }

            if(content.ChannelsCount != 2)
            {
                //std::cerr
                //    << "Error: file " << fileName << " is not a stereo file. Currently only stereo files are supported!"
                //    << std::endl;

                //return -1;
				content.Convert16bMonoTo16BitStereo();//Convert2Stereo16Bit();
            }

            if(content.SamplesCount < mBufferSizeInSamples)
            {
                std::cerr
                    << "Error: file " << fileName << " are too short."
                    << std::endl;

                return -1;
            }

            //check that samples have compatible formats
            //becouse convertions are not yet implemented
            if(mWavFiles.size())
            {
                if(!mWavFiles[0].IsSameFormat(content))
                {
                    std::cerr << "Error: file " << fileName << " has a diffrent format with opened files" << std::endl;

                    return -1;
                }
            }

            mWavFiles.push_back(content);
        }
        else
        {
            std::cerr << "Error: could not load WAV data from file " << fileName << std::endl;

            return -1;
        }
    }

    if(!mWavFiles.size())
    {
        std::cerr << "Error: no files opened to play" << std::endl;

        return -1;
    }

    //initialize hardware
    mPlayer.reset(
#if defined(__MACOSX) || defined(__APPLE__)
        static_cast<IWavPlayer *>(new PortPlayer())
#else

#ifdef ENABLE_PORTAUDIO
        playerType == "PortAudio"
            ? static_cast<IWavPlayer *>(new PortPlayer())
            :
#ifdef _WIN32
                static_cast<IWavPlayer *>(new WASAPIPlayer())
#elif !defined(__MACOSX) && !defined(__APPLE__)
                static_cast<IWavPlayer *>(new AlsaPlayer())
#endif

#else

#ifdef _WIN32
        new WASAPIPlayer()
#elif !defined(__MACOSX) && !defined(__APPLE__)
        new AlsaPlayer()
#endif

#endif

#endif
        );

    //assume that all opened files has the same format
    //and we have at least one opened file
    auto openStatus = mPlayer->Init(
        mWavFiles[0].ChannelsCount,
        mWavFiles[0].BitsPerSample,
        mWavFiles[0].SamplesPerSecond,

        mWavFiles.size() > 0,
        useMicSource
        );

    if(PlayerError::OK != openStatus)
    {
        std::cerr << "Error: could not initialize player " << std::endl;

        return -1;
    }

    /* # fft buffer length must be power of 2: */
    m_fftLen = 1;
    while((int)m_fftLen < fftLen && (m_fftLen << 1) <= MAXRESPONSELENGTH)
    {
        m_fftLen <<= 1;
    }

    /*for(int i = 0; i < MAX_SOURCES; i++)
    {
        m_samplePos[i] = 0;
    }*/

    // allocate responses in one block
    // to optimize transfer to GPU
    //mResponseBuffer = new float[mWavFiles.size() * m_fftLen * STEREO_CHANNELS_COUNT];
    mResponseBuffer = mResponseBufferStorage.Allocate(mWavFiles.size() * m_fftLen * STEREO_CHANNELS_COUNT);

    //todo: use std::align(32, sizeof(__m256), out2, space)...
    for(int idx = 0; idx < mWavFiles.size() * 2; idx++)
    {
        mResponses[idx] = mResponseBuffer + idx * m_fftLen;

        mInputFloatBufs[idx] = mInputFloatBufsStorage[idx].Allocate(m_fftLen);
        mOutputFloatBufs[idx] = mOutputFloatBufsStorage[idx].Allocate(m_fftLen);
    }

    for (int i = 0; i < mWavFiles.size() * STEREO_CHANNELS_COUNT; i++)
    {
        memset(mResponses[i], 0, sizeof(float)*m_fftLen);
        memset(mInputFloatBufs[i], 0, sizeof(float)*m_fftLen);
        memset(mOutputFloatBufs[i], 0, sizeof(float)*m_fftLen);
    }

    for (int i = 0; i < STEREO_CHANNELS_COUNT; i++)//Right and left channel after mixing
    {
        mOutputMixFloatBufs[i] = mOutputMixFloatBufsStorage[i].Allocate(m_fftLen);
        memset(mOutputMixFloatBufs[i], 0, sizeof(float)*m_fftLen);
    }

    memset(&room, 0, sizeof(room));

    room = roomDef;

    for (int idx = 0; idx < mWavFiles.size(); idx++)
    {
        sources[idx].speakerX = 0.0;
        sources[idx].speakerY = 0.0;
        sources[idx].speakerZ = 0.0;
    }

    ears.earSpacing = float(0.16);
    ears.headX = 0.0;
    ears.headZ = 0.0;
    ears.headY = 1.75;
    ears.pitch = 0.0;
    ears.roll = 0.0;
    ears.yaw = 0.0;

    {
        mMaxSamplesCount = mWavFiles[0].SamplesCount;

        for(int file = 1; file < mWavFiles.size(); ++file)
        {
            mMaxSamplesCount = std::max(
                mWavFiles[file].SamplesCount,
                mMaxSamplesCount
                );
        }

        mStereoProcessedBuffer.resize(
            STEREO_CHANNELS_COUNT
            *
            mMaxSamplesCount
            );
    }

    // Allocate RT-Queues
    {
        mCmdQueue1 = mCmdQueue2 = mCmdQueue3 = nullptr;
        
        int32_t flagsQ1 = 0;
        int32_t flagsQ2 = 0;

        //CL convolution on GPU
        if(useGPU_Conv && useGPU_ConvQueue)
        {
 //   #ifdef RTQ_ENABLED

    #define QUEUE_MEDIUM_PRIORITY                   0x00010000

    #define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
            if (useHPr_Conv){
                flagsQ1 = QUEUE_MEDIUM_PRIORITY;
            }

    else if (useRTQ_Conv){
                flagsQ1 = QUEUE_REAL_TIME_COMPUTE_UNITS | cuRes_Conv;
            }

            if (useHPr_IRGen){
                flagsQ2 = QUEUE_MEDIUM_PRIORITY;
            }

            else if (useRTQ_IRGen){
                flagsQ2 = QUEUE_REAL_TIME_COMPUTE_UNITS | cuRes_IRGen;
            }
//    #endif // RTQ_ENABLED

			// Retain/release counts must match. 
			 // Each component that is initialized with a clQueue should increment the ref count (clRetain) in init, and release on terminate.
			 // This component (SimpleVR creates the queues (ref = 1 on create) so does not need a retain, but still needs to release one close.

            CreateGpuCommandQueues(devIdx_Conv, flagsQ1, &mCmdQueue1, flagsQ2, &mCmdQueue2);
			//CLQUEUE_REFCOUNT(mCmdQueue1);
			//CLQUEUE_REFCOUNT(mCmdQueue2);


			/************** WARNING MIS-MATCHED clRetain / clRelease CAN CAUSE KERNEL MODE HANG for RTQs ****************************
			 *                                                                                                                      *
			 *  When using AMD Real Time Queues (RTQ) There can be only one of each type of queue RTQ1, RTQ2.                       *
			 *  If these are not released, it will be impossible to create another RTQ.                                             *
			 *  The kernel mode driver (KMD) will HANG  waiting for the existing RTQ queue to be released,                          *
			 *  before it will create a new one.                                                                                    *
			 *                                                                                                                      *
			 *  [All queues are released when the process exits, however.]                                                          *
			 ************************************************************************************************************************/
 
            if((devIdx_Conv == devIdx_IRGen) && useGPU_IRGen)
            {
                mCmdQueue3 = mCmdQueue2;
				clRetainCommandQueue(mCmdQueue3);
				CLQUEUE_REFCOUNT(mCmdQueue3);
            }
        }

	// OpenCL on CPU is deprecated
    //    //CL convolution on CPU
    //    else if(useGPU_Conv && !useGPU_ConvQueue)
    //    {
    //#ifdef RTQ_ENABLED
    //        // For " core "reservation" on CPU" -ToDo test and enable
    //        if (cuRes_Conv > 0 && cuRes_IRGen > 0)
    //        {
    //            cl_int err = CreateCommandQueuesWithCUcount(devIdx_Conv, &mCmdQueue1, &mCmdQueue2, cuRes_Conv, cuRes_IRGen);
    //        }
    //        //else
    //        //{
    //        //    CreateCpuCommandQueues(devIdx_Conv, 0, &mCmdQueue1, 0, &mCmdQueue2);
    //        //}
    //#endif // RTQ_ENABLED

    //        //CL room on CPU
    //        if((devIdx_Conv == devIdx_IRGen) && !useGPU_IRGenQueue)
    //        {
    //            mCmdQueue3 = mCmdQueue2;
    //        }
    //    }

        //room queue not yet created
        if(!mCmdQueue3)
        {
            //CL over GPU
            if(useGPU_IRGenQueue)
            {
                CreateGpuCommandQueues(devIdx_IRGen, 0, &mCmdQueue3, 0, nullptr);
            }
            //CL over CPU
            //else
            //{
            //    CreateCpuCommandQueues(devIdx_IRGen, 0, &mCmdQueue3, 0, nullptr);
            //}
        }
    }

    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &mTANConvolutionContext));
    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &mTANRoomContext));

	//CLQUEUE_REFCOUNT(mCmdQueue1);
	//CLQUEUE_REFCOUNT(mCmdQueue2);

    //convolution over OpenCL
    if(useGPU_Conv) 
    {
        RETURN_IF_FAILED(mTANConvolutionContext->InitOpenCL(mCmdQueue1, mCmdQueue2));
    }
	CLQUEUE_REFCOUNT(mCmdQueue1);
	CLQUEUE_REFCOUNT(mCmdQueue2);

    //room processing over OpenCL
    if(useGPU_IRGen) 
    {
        RETURN_IF_FAILED(mTANRoomContext->InitOpenCL(mCmdQueue3, mCmdQueue3));
    }
	CLQUEUE_REFCOUNT(mCmdQueue1);
	CLQUEUE_REFCOUNT(mCmdQueue2);

    RETURN_IF_FAILED(TANCreateConvolution(mTANConvolutionContext, &m_spConvolution));
	CLQUEUE_REFCOUNT(mCmdQueue1);
	CLQUEUE_REFCOUNT(mCmdQueue2);

    //don't use OpenCL at all
    if(!useGPU_Conv) 
    {        
        /*
        // ensures compatible with old room acoustic
		if(convMethod != TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD)
        {
            convMethod = TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
        }
        */

        // C_Model implementation
        RETURN_IF_FAILED(
            m_spConvolution->InitCpu(
                amf::TAN_CONVOLUTION_METHOD(convMethod),  // | TAN_CONVOLUTION_METHOD_USE_PROCESS_FINALIZE), //optional to reduce latency, must call ProcessFinalize in separate thread.
                m_fftLen,
                mBufferSizeInSamples,
                mWavFiles.size() * STEREO_CHANNELS_COUNT
                )
            );
    }
    else
    {
        RETURN_IF_FAILED(
            m_spConvolution->InitGpu(
                //TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL,
				amf::TAN_CONVOLUTION_METHOD(convMethod), // | TAN_CONVOLUTION_METHOD_USE_PROCESS_FINALIZE), //optional to reduce latency, must call ProcessFinalize in separate thread.
                m_fftLen,
                mBufferSizeInSamples,
                mWavFiles.size() * STEREO_CHANNELS_COUNT
                )
            );
    }

    //RETURN_IF_FAILED(TANCreateConverter(mTANRoomContext, &m_spConverter));
	RETURN_IF_FAILED(TANCreateConverter(mTANConvolutionContext, &m_spConverter));
	RETURN_IF_FAILED(m_spConverter->Init());

	//RETURN_IF_FAILED(TANCreateMixer(mTANRoomContext, &m_spMixer));
	RETURN_IF_FAILED(TANCreateMixer(mTANConvolutionContext, &m_spMixer));
	RETURN_IF_FAILED(m_spMixer->Init(mBufferSizeInSamples, mWavFiles.size()));

    RETURN_IF_FAILED(TANCreateFFT(mTANRoomContext, &m_spFft));
    RETURN_IF_FAILED(m_spFft->Init());

    if (m_pTAVR != NULL) {
        delete(m_pTAVR);
        m_pTAVR = NULL;
    }

    //CL over GPU for both Convolution and Room processing
    if(useGPU_ConvQueue && useGPU_IRGenQueue)
    {
        //if (mCmdQueue3 == NULL){
        //    CreateGpuCommandQueues(devIdx_IRGen, 0, &mCmdQueue3, 0, NULL);
        //}

        cl_int status;
        cl_context context_IR;
        cl_context context_Conv;

        clGetCommandQueueInfo(mCmdQueue3, CL_QUEUE_CONTEXT, sizeof(cl_context), &context_IR, NULL);
        clGetCommandQueueInfo(mCmdQueue2, CL_QUEUE_CONTEXT, sizeof(cl_context), &context_Conv, NULL);
        
        if(context_IR == context_Conv) 
        {
            for(int i = 0; i < mWavFiles.size() * 2; i++)
            {
                mOCLResponses[i] = clCreateBuffer(context_IR, CL_MEM_READ_WRITE, m_fftLen * sizeof(float), NULL, &status);
            }

            //HACK out for test
            mUseClMemBufs = true;
        }

        // Initialize CL output buffers,
        //if(useGPU_Conv) already?
        {
            cl_int clErr;

            // First create a big cl_mem buffer then create small sub-buffers from it
            mOutputMainCLbuf = clCreateBuffer(
                mTANConvolutionContext->GetOpenCLContext(),
                CL_MEM_READ_WRITE,
                mBufferSizeInBytes * mWavFiles.size() * STEREO_CHANNELS_COUNT,
                nullptr,
                &clErr
                );

            if (clErr != CL_SUCCESS)
            {
                printf("Could not create OpenCL buffer\n");
                return AMF_FAIL;
            };

            for(amf_uint32 i = 0; i < mWavFiles.size() * 2; i++)
            {
                cl_buffer_region region;
                region.origin = i * mBufferSizeInBytes;
                region.size = mBufferSizeInBytes;
                mOutputCLBufs[i] = clCreateSubBuffer(
                    mOutputMainCLbuf, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &clErr);

                if (clErr != CL_SUCCESS)
                {
                    printf("Could not create OpenCL subBuffer\n");
                    return AMF_FAIL;
                }

                float zero = 0.0;
                clErr = clEnqueueFillBuffer(mCmdQueue1, mOutputCLBufs[i], &zero,sizeof(zero), 0, region.size, 0, NULL, NULL);
                if (clErr != CL_SUCCESS)
                {
                    printf("Could not fill OpenCL subBuffer\n");
                    return AMF_FAIL;
                }
            }

            for (int idx = 0; idx < 2; idx++)
            {
                mOutputMixCLBufs[idx] = clCreateBuffer(
                    mTANConvolutionContext->GetOpenCLContext(),
                    CL_MEM_READ_WRITE,
                    mBufferSizeInBytes,
                    nullptr,
                    &clErr
                    );

                if (clErr != CL_SUCCESS)
                {
                    printf("Could not create OpenCL buffer\n");
                    return AMF_FAIL;
                }

                if (clErr != CL_SUCCESS)
                {

                    printf("Could not create OpenCL buffer\n");
                    return AMF_FAIL;
                }
            }

            // The output short buffer stores the final (after mixing) left and right channels interleaved as short samples
            // The short buffer size is equal to sizeof(short)*2*m_bufSize/sizeof(float) which is equal to m_bufSize
            mOutputShortBuf = clCreateBuffer(
                mTANConvolutionContext->GetOpenCLContext(),
                CL_MEM_READ_WRITE,
                mBufferSizeInBytes,
                nullptr,
                &clErr
                );
        }
    }

	//To Do: use dlopen /dlsym / dlclose  on linux / mac
    #ifdef _WIN32
    HMODULE TanVrDll;
    TanVrDll = LoadLibraryA("TrueAudioVR.dll");
    typedef int  (WINAPI *CREATEVR)(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue, float samplesPerSecond, int convolutionLength);
    CREATEVR CreateAmdTrueAudioVR = nullptr;

    CreateAmdTrueAudioVR = (CREATEVR)GetProcAddress(TanVrDll, "CreateAmdTrueAudioVR");
    #endif

    CreateAmdTrueAudioVR(
        &m_pTAVR,
        mTANRoomContext,
        m_spFft,
        mCmdQueue3,
        48000, //todo: other frequencies?
        m_fftLen
        );

    if(useGPU_IRGenQueue)
    {
        m_pTAVR->SetExecutionMode(AmdTrueAudioVR::GPU);
    }
    else 
    {
        m_pTAVR->SetExecutionMode(AmdTrueAudioVR::CPU);
    }

    std::cout << "Room: " << room.width<< "fm W x " << room.length << "fm L x " << room.height << "fm H" << std::endl;

    // head model:
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);

    //To Do use gpu mem responses
    for (int idx = 0; idx < mWavFiles.size(); idx++){
        if (mUseClMemBufs) {
            m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, mOCLResponses[idx * 2], mOCLResponses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES | GENROOM_USE_GPU_MEM, 50);
        }
        else {
            m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, mResponses[idx * 2], mResponses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES, 50);
        }
    }

    if (mUseClMemBufs) {
        RETURN_IF_FAILED(m_spConvolution->UpdateResponseTD(mOCLResponses, m_fftLen, nullptr, IR_UPDATE_MODE));
    }
    else {
        RETURN_IF_FAILED(m_spConvolution->UpdateResponseTD(mResponses, m_fftLen, nullptr, IR_UPDATE_MODE));
    }

    mRunning = true;

    return mRunning;
}

int Audio3D::updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
    // world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);
    yaw = m_headingOffset + (m_headingCCW ? yaw : -yaw);

    if (x == ears.headX && y == ears.headY && z == ears.headZ //) {
        && yaw == ears.yaw && pitch == ears.pitch && roll == ears.roll) {
            return 0;
    }

    ears.headX = x;
    ears.headY = y;
    ears.headZ = z;

    ears.yaw = yaw;
    ears.pitch = pitch;
    ears.roll = roll;

    mUpdateParams = true;
    return 0;
}

int Audio3D::updateSourcePosition(int srcNumber, float x, float y, float z)
{
    if (srcNumber >= mWavFiles.size()){
        return -1;
    }
    // world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);

    sources[srcNumber].speakerX = x;// +room.width / 2.0f;
    sources[srcNumber].speakerY = y;
    sources[srcNumber].speakerZ = z;// +room.length / 2.0f;

    mUpdateParams = true;
    return 0;
}

int Audio3D::updateRoomDimension(float _width, float _height, float _length)
{
	room.width = _width;
	room.height = _height;
	room.length = _length;

	mUpdateParams = true;
	return 0;
}

int Audio3D::updateRoomDamping(float _left, float _right, float _top, float _buttom, float _front, float _back)
{
	room.mTop.damp = _top;
	room.mBottom.damp = _buttom;
	room.mLeft.damp = _left;
	room.mRight.damp = _right;
	room.mFront.damp = _front;
	room.mBack.damp = _back;

	mUpdateParams = true;
	return 0;
}

AmdTrueAudioVR* Audio3D::getAMDTrueAudioVR()
{
	return m_pTAVR;
}

TANConverterPtr Audio3D::getTANConverter()
{
	return m_spConverter;
}

int Audio3D::setWorldToRoomCoordTransform(
    float translationX=0.,
    float translationY=0.,
    float translationZ=0.,

    float rotationY=0.,

    float headingOffset=0.,
    bool headingCCW=true
    )
{
    m_mtxWorldToRoomCoords.setOffset(translationX, translationY, translationZ);
    m_mtxWorldToRoomCoords.setAngles(rotationY, 0.0, 0.0);
    m_headingOffset = headingOffset;
    m_headingCCW = headingCCW;

    return 0;
}

int Audio3D::Run()
{
    mStop = false;
    // start main processing thread:
    //m_hProcessThread = (HANDLE)_beginthreadex(0, 10000000, processThreadProc, this, 0, 0);
    //RETURN_IF_FALSE(m_hProcessThread != (HANDLE)-1);
    mProcessThread = std::thread(processThreadProc, this);

    // wait for processing thread to start:
    while (!mRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // start update processing thread:
    //m_hUpdateThread = (HANDLE)_beginthreadex(0, 10000000, updateThreadProc, this, 0, 0);
    //RETURN_IF_FALSE(m_hUpdateThread != (HANDLE)-1);
    mUpdateThread = std::thread(updateThreadProc, this);

    mUpdateParams = true;
    return true;
}

bool Audio3D::Stop()
{
    mStop = true;

    mProcessThread.WaitCloseInfinite();
    mUpdateThread.WaitCloseInfinite();

    Close();

    return true;
}

int Audio3D::Process(int16_t *pOut, int16_t *pChan[MAX_SOURCES], uint32_t sampleCountBytes)
{
    uint32_t sampleCount = sampleCountBytes / (sizeof(int16_t) * STEREO_CHANNELS_COUNT);

    // Read from the files
    for (int idx = 0; idx < mWavFiles.size(); idx++) {
        for (int chan = 0; chan < 2; chan++){
            // The way sources in inputFloatBufs are ordered is: Even indexed elements for left channels, odd indexed ones for right,
            // this ordering matches with the way impulse responses are generated and indexed to be convolved with the sources.
            RETURN_IF_FAILED(
                m_spConverter->Convert(
                    pChan[idx] + chan,
                    2,
                    sampleCount,
                    mInputFloatBufs[idx * 2 + chan],
                    1,
                    1.f
                    )
                );
        }
    }

    if (m_useOCLOutputPipeline)
    {
        /**/
        // OCL device memory objects are passed to the TANConvolution->Process method.
        // Mixing and short conversion is done on GPU.

        RETURN_IF_FAILED(m_spConvolution->Process(mInputFloatBufs, mOutputCLBufs, sampleCount, nullptr, nullptr));

        cl_mem outputCLBufLeft[MAX_SOURCES];
        cl_mem outputCLBufRight[MAX_SOURCES];

        for (int src = 0; src < MAX_SOURCES; src++)
        {
            outputCLBufLeft[src] = mOutputCLBufs[src*2 ];// Even indexed channels for left ear input
            outputCLBufRight[src] = mOutputCLBufs[src*2+1];// Odd indexed channels for right ear input
        }

        AMF_RESULT ret = m_spMixer->Mix(outputCLBufLeft, mOutputMixCLBufs[0]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spMixer->Mix(outputCLBufRight, mOutputMixCLBufs[1]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spConverter->Convert(mOutputMixCLBufs[0], 1, 0, TAN_SAMPLE_TYPE_FLOAT,
            mOutputShortBuf, 2, 0, TAN_SAMPLE_TYPE_SHORT, sampleCount, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

        ret = m_spConverter->Convert(mOutputMixCLBufs[1], 1, 0, TAN_SAMPLE_TYPE_FLOAT,
            mOutputShortBuf, 2, 1, TAN_SAMPLE_TYPE_SHORT, sampleCount, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

        cl_int clErr = clEnqueueReadBuffer(mTANConvolutionContext->GetOpenCLConvQueue(), mOutputShortBuf, CL_TRUE,
             0, sampleCountBytes, pOut, NULL, NULL, NULL);

		// Call the processFinalize() method after all the post processing is enqueued on the convolution queue
		//if method & TAN_CONVOLUTION_METHOD_USE_PROCESS_FINALIZE
		// m_spConvolution->ProcessFinalize();
		RETURN_IF_FALSE(clErr == CL_SUCCESS);
        /**/
    }
    else
    {   // Host memory pointers are passed to the TANConvolution->Process method
        // Mixing and short conversion are still performed on CPU.

        RETURN_IF_FAILED(m_spConvolution->Process(mInputFloatBufs, mOutputFloatBufs, sampleCount,
            nullptr, nullptr));

        float * outputFloatBufLeft[MAX_SOURCES];
        float * outputFloatBufRight[MAX_SOURCES];

        for (int src = 0; src < MAX_SOURCES; src++)
        {
            outputFloatBufLeft[src] = mOutputFloatBufs[src * 2];// Even indexed channels for left ear input
            outputFloatBufRight[src] = mOutputFloatBufs[src * 2 + 1];// Odd indexed channels for right ear input
        }

        AMF_RESULT ret(AMF_OK);

        ret = m_spMixer->Mix(outputFloatBufLeft, mOutputMixFloatBufs[0]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spMixer->Mix(outputFloatBufRight, mOutputMixFloatBufs[1]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spConverter->Convert(mOutputMixFloatBufs[0], 1, sampleCount, pOut, 2, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

        ret = m_spConverter->Convert(mOutputMixFloatBufs[1], 1, sampleCount, pOut + 1, 2, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);
    }

	// Call the processFinalize() method after all the post processing is enqueued on the convolution queue
	//if method & TAN_CONVOLUTION_METHOD_USE_PROCESS_FINALIZE
	// m_spConvolution->ProcessFinalize();

#if 0// Old code: Crossfade, Mixing and Conversion on CPU

    for (int idx = 0; idx < mWavFiles.size(); idx++) {
        for (int chan = 0; chan < 2; chan++){
            RETURN_IF_FAILED(m_spConverter->Convert(pChan[idx] + chan, 2, sampleCount,
                inputFloatBufs[idx*2 + chan], 1, 1.f));
        }
    }

    RETURN_IF_FAILED(m_spConvolution->Process(inputFloatBufs, outputFloatBufs, sampleCount,
                                             nullptr, nullptr));

    // ToDo:  use limiter...


    for (int idx = 2; idx < 2 * mWavFiles.size(); idx += 2) {
        int k = 0;
        int n = sampleCount;
        while (n >= 8 && useIntrinsics){
            register __m256 *outL, *outR, *inL, *inR;
            outL = (__m256 *)&outputFloatBufs[0][k];
            outR = (__m256 *)&outputFloatBufs[1][k];
            inL = (__m256 *)&outputFloatBufs[idx][k];
            inR = (__m256 *)&outputFloatBufs[idx + 1][k];

            *outL = _mm256_add_ps(*outL, *inL);
            *outR = _mm256_add_ps(*outR, *inR);
            k += 8;
            n -= 8;
        }
        while(n > 0) {
            outputFloatBufs[0][k] += outputFloatBufs[idx][k];
            outputFloatBufs[1][k] += outputFloatBufs[idx + 1][k];
            k++;
            n--;
        }
    }


    AMF_RESULT ret = m_spConverter->Convert(outputFloatBufs[0], 1, sampleCount, pOut, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

    ret = m_spConverter->Convert(outputFloatBufs[1], 1, sampleCount, pOut + 1, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);
#endif
    return 0;
}

int Audio3D::ProcessProc()
{
    //uint32_t bytesRecorded(0);

    std::array<uint8_t, STEREO_CHANNELS_COUNT * FILTER_SAMPLE_RATE * sizeof(int16_t)> recordBuffer;
    std::array<uint8_t, STEREO_CHANNELS_COUNT * FILTER_SAMPLE_RATE * sizeof(int16_t)> extractBuffer;
    std::array<int16_t, STEREO_CHANNELS_COUNT * FILTER_SAMPLE_RATE> outputBuffer;

    //FifoBuffer recFifo(STEREO_CHANNELS_COUNT * FILTER_SAMPLE_RATE * sizeof(int16_t));
	// for mic input mode
    // Fifo recordFifo;
    //recordFifo.Reset(STEREO_CHANNELS_COUNT * FILTER_SAMPLE_RATE * mWavFiles[0].GetSampleSizeInBytes());

    int16_t *pWaves[MAX_SOURCES] = {nullptr};
    int16_t *pWaveStarts[MAX_SOURCES] = {nullptr};

    uint32_t waveSizesInBytes[MAX_SOURCES] = {0};
    uint32_t waveBytesPlayed[MAX_SOURCES] = {0};

    auto buffers2Play = mMaxSamplesCount / mBufferSizeInSamples; //at least 1 block
    uint32_t buffersPlayed(0);
    uint32_t buffersPerSecond = FILTER_SAMPLE_RATE / mBufferSizeInSamples;
    uint32_t deltaTimeInMs = 1000 / buffersPerSecond;

    for(int file = 0; file < mWavFiles.size(); ++file)
    {
        int16_t *data16Bit((int16_t *)&(mWavFiles[file].Data[0]));

        pWaveStarts[file] = pWaves[file] = data16Bit;
        waveSizesInBytes[file] = mWavFiles[file].ChannelsCount * mWavFiles[file].SamplesCount * (mWavFiles[file].BitsPerSample / 8); // stereo short samples
    }

    while(!mUpdated && !mStop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    auto *processed = &mStereoProcessedBuffer.front();

    double previousTimerValue(0.0);
    bool firstFrame(true);

    while(!mStop)
    {
        uint32_t bytes2Play(0);

#if 0 // To Do fix mic input mode
        if(mSrc1EnableMic)
        {
            while
            (
                !mStop
                &&
                (recordFifo.GetQueueSize() < mBufferSizeInBytes)
            )
            {
                // get some more:
                uint32_t recordedBytes = mPlayer->Record(
                    &recordBuffer.front(),
                    recordBuffer.size()
                    );

                recordFifo.Write(
                    &recordBuffer.front(),
                    recordedBytes
                    );

                //Sleep(5);
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                //std::this_thread::sleep_for(std::chrono::milliseconds(0));
            }

            if(!mStop)
            {
                bytes2Play = uint32_t(std::min(recordFifo.GetQueueSize(), size_t(mBufferSizeInBytes)));

                recordFifo.Read(
                    &extractBuffer.front(),
                    bytes2Play
                    );

                // [temporarily] duck the wave audio for mic audio:
                int16_t *wavPtrTemp = pWaves[0];
                pWaves[0] = (int16_t*)&extractBuffer.front();
                Process(&outputBuffer.front(), pWaves, bytes2Play);
                pWaves[0] = wavPtrTemp;

                //to test
                /** /
                mPlayer->Play(
                    &extractBuffer.front(),
                    bytes2Extract,
                    false
                    );
                /**/
            }
        }
        else
#endif
        {
            //todo: this is not correct for common case, add size calculation
            bytes2Play = mBufferSizeInBytes;

            Process(&outputBuffer.front(), pWaves, mBufferSizeInBytes);
        }

        //continue;
        std::this_thread::sleep_for(std::chrono::milliseconds(0));

        //memcpy(processed, &outputBuffer.front(), bytes2Play);

        uint32_t bytesTotalPlayed(0);
        uint8_t *outputBufferData = (uint8_t *)&outputBuffer.front();

        while(bytes2Play && !mStop)
        {
            //if(!mRealtimeTimer.IsStarted())
            {
                //mRealtimeTimer.Start();
                //firstFrame = true;
            }

            //double timerValue(firstFrame ? 0.0 : mRealtimeTimer.Sample());

            if
            (
                firstFrame
                ||
                true
                //((timerValue - previousTimerValue) * 1000 > 0.7 * deltaTimeInMs)
            )
            {
                auto bytesPlayed = mPlayer->Play(outputBufferData, bytes2Play, false);
                bytesTotalPlayed += bytesPlayed;

                outputBufferData += bytesPlayed;
                bytes2Play -= bytesPlayed;

                //previousTimerValue = timerValue;
                firstFrame = false;
            }
            else
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(2));
                std::this_thread::sleep_for(std::chrono::milliseconds(0));
            }
        }

        for(int fileIndex = 0; fileIndex < mWavFiles.size(); ++fileIndex)
        {
            waveBytesPlayed[fileIndex] += bytesTotalPlayed;

            //todo: eliminate size gaps
            //size of wav are not exactly rounded to mBufferSizeInBytes
            if(waveBytesPlayed[fileIndex] + mBufferSizeInBytes < waveSizesInBytes[fileIndex])
            {
                pWaves[fileIndex] += (mBufferSizeInBytes / sizeof(int16_t));
                /*std::cout
                  << "pl " << bytesTotalPlayed
                  << " fr " << mWavFiles[fileIndex].Data.size()
                  << std::endl;*/
            }
            else
            {
                //play wav again
                pWaves[fileIndex] = pWaveStarts[fileIndex];
                waveBytesPlayed[fileIndex] = 0;
            }
        }

        if(processed - &mStereoProcessedBuffer.front() + (mBufferSizeInBytes / sizeof(int16_t)) > mMaxSamplesCount)
        {
            processed = &mStereoProcessedBuffer.front();
        }
        else
        {
            processed += (mBufferSizeInBytes / sizeof(int16_t));
        }

        /*///compute current sample position for each stream:
        for (int i = 0; i < mWavFiles.size(); i++)
        {
            m_samplePos[i] = (pWaves[i] - pWaveStarts[i]) / sizeof(short);
        }*/

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /*
    if(!WriteWaveFileS(
        "RoomAcousticsRun.wav",
        FILTER_SAMPLE_RATE,
        2,
        16,
        //mWavFiles[0].SamplesCount, //why not a max?
        mMaxSamplesCount,
        &mStereoProcessedBuffer.front()
        ))
    {
        puts("unable to write RoomAcousticsRun.wav");
    }
    else
    {
        puts("wrote output to RoomAcousticsRun.wav");
    }*/

    mRunning = false;

    return 0;
}

/*int64_t Audio3D::getCurrentPosition(int streamIdx)
{
    return m_samplePos[streamIdx];
}*/

int Audio3D::UpdateProc()
{
    while(mRunning && !mStop)
    {
        while(!mUpdateParams && mRunning && !mStop)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if(!mStop)
        {
            if(mSrc1TrackHeadPos)
            {
                sources[0].speakerX = ears.headX;
                sources[0].speakerY = ears.headY;
                sources[0].speakerZ = ears.headZ;
            }
        }

        for(int idx = 0; !mStop && (idx < mWavFiles.size()); idx++)
        {
            if(mUseClMemBufs)
            {
                m_pTAVR->generateRoomResponse(
                    room,
                    sources[idx],
                    ears,
                    FILTER_SAMPLE_RATE,
                    m_fftLen,
                    mOCLResponses[idx * 2],
                    mOCLResponses[idx * 2 + 1],
                    GENROOM_LIMIT_BOUNCES | GENROOM_USE_GPU_MEM,
                    50
                    );
            }
            else
            {
                memset(mResponses[idx * 2], 0, sizeof(float )* m_fftLen);
                memset(mResponses[idx * 2 + 1], 0, sizeof(float) * m_fftLen);

                m_pTAVR->generateRoomResponse(
                    room,
                    sources[idx],
                    ears,
                    FILTER_SAMPLE_RATE,
                    m_fftLen,
                    mResponses[idx * 2],
                    mResponses[idx * 2 + 1],
                    GENROOM_LIMIT_BOUNCES,
                    50
                    );
            }
        }

        AMF_RESULT ret = AMF_OK;

        while(mRunning && !mStop)
        {
            if(mUseClMemBufs)
            {
                ret = m_spConvolution->UpdateResponseTD(mOCLResponses, m_fftLen, nullptr, IR_UPDATE_MODE);
            }
            else
            {
                ret = m_spConvolution->UpdateResponseTD(mResponses, m_fftLen, nullptr, IR_UPDATE_MODE);
            }

            if(ret != AMF_INPUT_FULL)
            {
                break;
            }
        }

        //todo: investigate about AMF_BUSY
        RETURN_IF_FALSE(ret == AMF_OK /*|| ret == AMF_BUSY*/);

        mUpdated = true;

        //Sleep(20);
        mUpdateParams = false;
    }

    return 0;
}

int Audio3D::exportImpulseResponse(int srcNumber, char * fileName)
{
    int convolutionLength = this->m_fftLen;
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);

    float *leftResponse = mResponses[0];
    float *rightResponse = mResponses[1];
    short *sSamples = new short[2 * convolutionLength];
     memset(sSamples, 0, 2 * convolutionLength*sizeof(short));

    (void)m_spConverter->Convert(leftResponse, 1, convolutionLength, sSamples, 2, 1.f);

    (void)m_spConverter->Convert(rightResponse, 1, convolutionLength, sSamples + 1, 2, 1.f);

    WriteWaveFileS(fileName, 48000, 2, 16, convolutionLength, sSamples);
    delete[] sSamples;
    return 0;
}
