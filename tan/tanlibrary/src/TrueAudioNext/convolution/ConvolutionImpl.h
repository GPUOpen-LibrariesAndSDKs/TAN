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
///-------------------------------------------------------------------------
///  @file   ConvolutionImpl.h
///  @brief  TANConvolutionImpl interface implementation
///-------------------------------------------------------------------------
#pragma once
#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Context.h"        //AMF
#include "public/include/components/Component.h"//AMF
#include "public/common/PropertyStorageExImpl.h"//AMF

#include "tanlibrary/src/Graal/GraalConv.hpp"
#include "tanlibrary/src/Graal/GraalConv_clFFT.hpp"
#ifdef AMF_FACILITY
#  undef AMF_FACILITY
#endif

#define SAFE_DELETE(x) { delete x; (x) = nullptr; }
#define SAFE_ARR_DELETE(x) { delete[] x; (x) = nullptr; }

namespace amf
{
    class TANConvolutionImpl
        : public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANConvolution> >
    {
    public:
        typedef AMFInterfacePtr_T<TANConvolutionImpl> Ptr;

        TANConvolutionImpl(TANContext *pContextTAN);
        virtual ~TANConvolutionImpl(void);

// interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANConvolution> >)
        AMF_END_INTERFACE_MAP

//TANConvolution interface

		AMF_RESULT	AMF_STD_CALL	Init(TAN_CONVOLUTION_METHOD convolutionMethod,
										 amf_uint32 responseLengthInSamples,
										 amf_uint32 bufferSizeInSamples,
										 amf_uint32 channels) override;
        AMF_RESULT  AMF_STD_CALL InitCpu(TAN_CONVOLUTION_METHOD convolutionMethod,
                                         amf_uint32 responseLengthInSamples,
                                         amf_uint32 bufferSizeInSamples,
                                         amf_uint32 channels) override;
        AMF_RESULT  AMF_STD_CALL InitGpu(TAN_CONVOLUTION_METHOD convolutionMethod,
                                         amf_uint32 responseLengthInSamples,
                                         amf_uint32 bufferSizeInSamples,
                                         amf_uint32 channels) override;
        AMF_RESULT  AMF_STD_CALL Terminate() override;

        AMF_RESULT  AMF_STD_CALL UpdateResponseTD(float* ppBuffer[],
                                                  amf_size numOfSamplesToProcess,
                                                  const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                  const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                  ) override;
        AMF_RESULT  AMF_STD_CALL UpdateResponseTD(cl_mem ppBuffer[],
                                                  amf_size numOfSamplesToProcess,
                                                  const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                  const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                  ) override;
 
        AMF_RESULT  AMF_STD_CALL    UpdateResponseFD(float* ppBuffer[],
                                                     amf_size numOfSamplesToProcess,
                                                     const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                     const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                     ) override;
        AMF_RESULT  AMF_STD_CALL    UpdateResponseFD(cl_mem ppBuffer[],
                                                     amf_size numOfSamplesToProcess,
                                                     const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                     const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                     ) override;
 
        AMF_RESULT  AMF_STD_CALL    Process(float* ppBufferInput[],
                                            float* ppBufferOutput[],
                                            amf_size numOfSamplesToProcess,
                                            // Masks of flags from enum
                                            // TAN_CONVOLUTION_CHANNEL_FLAG.
                                            const amf_uint32 flagMasks[],
                                            amf_size *pNumOfSamplesProcessed = nullptr) override; // system memory
        AMF_RESULT  AMF_STD_CALL    Process(cl_mem pBufferInput[],
                                            cl_mem pBufferOutput[],
                                            amf_size numOfSamplesToProcess,
                                            // Masks of flags from enum
                                            // TAN_CONVOLUTION_CHANNEL_FLAG.
                                            const amf_uint32 flagMasks[],
                                            amf_size *pNumOfSamplesProcessed = nullptr) override; // cl_mem
        AMF_RESULT  AMF_STD_CALL    Process(float* pBufferInput[],
                                            cl_mem pBufferOutput[],
                                            amf_size numOfSamplesToProcess,
                                            // Masks of flags from enum
                                            // TAN_CONVOLUTION_CHANNEL_FLAG.
                                            const amf_uint32 flagMasks[],
                                            amf_size *pNumOfSamplesProcessed = nullptr) override; // cl_mem


        // Process direct (no update required), system memory buffers:
        AMF_RESULT  AMF_STD_CALL    ProcessDirect(
                                            float* ppImpulseResponse[],
                                            float* ppBufferInput[],
                                            float* ppBufferOutput[],
                                            amf_size numOfSamplesToProcess,
                                            amf_size *pNumOfSamplesProcessed,
                                            int *nzFirstLast = NULL
                                            ) override;

        // Process direct (no update required),  OpenCL cl_mem  buffers:
        AMF_RESULT  AMF_STD_CALL    ProcessDirect(
                                            cl_mem* ppImpulseResponse[],
                                            cl_mem* ppBufferInput[],
                                            cl_mem* ppBufferOutput[],
                                            amf_size numOfSamplesToProcess,
                                            amf_size *pNumOfSamplesProcessed,
                                            int *nzFirstLast = NULL
                                            ) override;


        AMF_RESULT AMF_STD_CALL GetNextFreeChannel(amf_uint32 *pChannelIndex,
                                                   const amf_uint32 flagMasks[] // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG.
                                                   ) override;

        virtual TANContext* AMF_STD_CALL GetContext(){return m_pContextTAN;}

    protected:
        virtual AMF_RESULT  Init(TAN_CONVOLUTION_METHOD convolutionMethod,
                                 amf_uint32 responseLengthInSamples,
                                 amf_uint32 bufferSizeInSamples,
                                 amf_uint32 channels,
                                 bool doProcessingOnGpu);

        virtual AMF_RESULT Flush(amf_uint32 filterStateId, amf_uint32 channelId);

        struct TANSampleBuffer {
            union {
                float ** host;
                cl_mem *clmem;
            } buffer;
            AMF_MEMORY_TYPE mType;
        };

        AMF_RESULT  AMF_STD_CALL UpdateResponseTD(
            TANSampleBuffer pBuffer,
            amf_size numOfSamplesToProcess,
            const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
            const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
            );

        AMF_RESULT  AMF_STD_CALL    Process(TANSampleBuffer pBufferInput,
                                            TANSampleBuffer pBufferOutput,
                                            amf_size numOfSamplesToProcess,
                                            // Masks of flags from enum
                                            // TAN_CONVOLUTION_CHANNEL_FLAG.
                                            const amf_uint32 flagMasks[],
                                            amf_size *pNumOfSamplesProcessed = nullptr); // TAN Audio buffers

        TANContextPtr               m_pContextTAN;
        AMFComputePtr               m_pProcContextAMF;
        AMFComputePtr               m_pUpdateContextAMF;

        TAN_CONVOLUTION_METHOD      m_eConvolutionMethod;
        amf_uint32                  m_iLengthInSamples;
        amf_uint32                  m_iBufferSizeInSamples;
        amf_uint32                  m_iChannels;    // Number of channels our buffers are allocated
                                                    // to, as opposed to m_MaxChannels.

		cl_kernel					m_pKernelCrossfade;
        AMF_MEMORY_TYPE             m_eOutputMemoryType;

        AMF_KERNEL_ID               m_KernelIdCrossfade;

        AMFCriticalSection          m_sect;

        // In the current implementation we can run neither Update() nor Process() in parallel to
        // itself (as we use many different preallocated buffers).
        AMFCriticalSection          m_sectAccum;
        AMFCriticalSection          m_sectUpdate;
        AMFCriticalSection          m_sectProcess;

        //cl_mem           m_pUpdateInputOCL;
        //cl_mem           m_pInputsOCL;
        //cl_mem           m_pOutputsOCL;

        TANFFTPtr                   m_pTanFft;
        TANFFTPtr                   m_pUpdateTanFft;

        bool                        m_doProcessOnGpu;


    private:
        bool m_initialized;

        //cl_program m_TimeDomainProgram;
        cl_kernel m_TimeDomainKernel;

        int m_length;           // Next power of two for the response's length.
        int m_log2len;          // =log2(m_length)
        int m_MaxChannels;      // Current number of channels, less or equal to m_iChannels.
        float **m_OutSamples;   // Buffer to store FFT, multiplication and ^FFT for a buffer.
        float** m_ovlAddLocalInBuffs;
        float** m_ovlAddLocalOutBuffs;
        TANSampleBuffer m_pCLXFadeSubBuf[2];  // For cross-fading on GPU is created as subfolder of m_pCLXFadeMasterBuf[] memory objects
        cl_mem m_pCLXFadeMasterBuf[2];
        TANSampleBuffer m_pXFadeSamples;  // For cross-fading on CPU 
        float *m_silence;       // Array filled with zeroes, used to emulate silent signal.

        TANSampleBuffer m_internalOutBufs;
        TANSampleBuffer m_internalInBufs;
        bool *m_availableChannels;
        bool *m_flushedChannels;// if a channel has just been flushed no need to flush it repeatedly
        int *m_tailLeftOver;

        AMF_RESULT allocateBuffers();
        AMF_RESULT deallocateBuffers();
        AMF_RESULT AMF_FAST_CALL Crossfade(
            TANSampleBuffer pBufferOutput, amf_size numOfSamplesToProcess);

        typedef struct _ovlAddFilterState {
            float **m_Filter;
            float **m_Overlap;
            float **m_internalFilter;
            float **m_internalOverlap;
        } ovlAddFilterState;

        typedef struct _tdFilterState {
            float **m_Filter;
            cl_mem *m_clFilter;
            cl_mem *m_clTemp;
            int *firstNz;
            int *lastNz;
            float **m_SampleHistory;
            cl_mem *m_clSampleHistory;
            int *m_sampHistPos;
        }tdFilterState;

#  define N_FILTER_STATES 3
        ovlAddFilterState *m_FilterState[N_FILTER_STATES];
        tdFilterState *m_tdFilterState[N_FILTER_STATES];
        tdFilterState *m_tdInternalFilterState[N_FILTER_STATES];
        int m_idxFilter;                        // Currently USED current index.
        int m_idxPrevFilter;                    // Currently USED previous index (for crossfading).
        int m_idxUpdateFilter;                  // Next FREE  index.
        int m_idxUpdateFilterLatest;
        int m_first_round_ever;

        AMFEvent m_procReadyForNewResponsesEvent;
        AMFEvent m_updateFinishedProcessing;
        AMFEvent m_xFadeStarted;

        class UpdateThread : public AMFThread
        {
        protected:
            TANConvolutionImpl *m_pParent;
        public:
            UpdateThread(TANConvolutionImpl *pParent) : m_pParent(pParent) {}
            void Run() override { m_pParent->UpdateThreadProc(this); }
        };
        UpdateThread m_updThread;

        // HACK Windows specific
        HANDLE m_updThreadHandle;

        void UpdateThreadProc(AMFThread *pThread);

        AMF_RESULT VectorComplexMul(float *vA, float *vB, float *out, int count);

        amf_size ovlAddProcess(ovlAddFilterState *state, TANSampleBuffer inputData, TANSampleBuffer outputData, amf_size length,
                               amf_uint32 n_channels, bool advanceOverlap = true);

        amf_size ovlTDProcess(tdFilterState *state, float **inputData, float **outputData, amf_size length,
            amf_uint32 n_channels);

        void ovlTimeDomainCPU(float *resp, amf_uint32 firstNonZero, amf_uint32 lastNonZero,
            float *in, float *out, float *histBuf, amf_uint32 bufPos,
            amf_size datalength, amf_size convlength);

        void ovlTimeDomain(tdFilterState *state, int chIdx, float *resp, amf_uint32 firstNonZero, amf_uint32 lastNonZero,
            float *in, float *out, amf_uint32 bufPos,
            amf_size datalength, amf_size convlength);

        void ovlTimeDomainGPU(cl_mem resp, amf_uint32 firstNonZero, amf_uint32 lastNonZero,
             cl_mem out, cl_mem histBuf, amf_uint32 bufPos,
            amf_size datalength, amf_size convlength);

        AMF_RESULT ProcessInternal(
            int idx,
            TANSampleBuffer pBuffersInput,
            TANSampleBuffer pBufferOutput,
            amf_size length,
            const amf_uint32 flagMasks[],
            amf_size *pNumOfSamplesProcessed,
            int prev_input = 0,
            int advance_time = 1,
            int ocl_skip_stage = 0,
            int ocl_crossfade_state = 0
            );

        /*********************************************************************************
        OCL convolution library
        **********************************************************************************/
        /****************************************************************************************
        Graal convolution library adapter interface
        *****************************************************************************************/
        void * m_graal_conv;

        struct GraalArgs
        {
            GraalArgs() :
                versions(nullptr),
                prevVersions(nullptr),
                channels(nullptr),
                lens(nullptr),
                responses(nullptr),
                negateCnt(0),
                updatesCnt(0)
            {
            }

            ~GraalArgs()
            {
                SAFE_ARR_DELETE(versions);
                SAFE_ARR_DELETE(prevVersions);
                SAFE_ARR_DELETE(channels);
                SAFE_ARR_DELETE(lens);
                SAFE_ARR_DELETE(responses);
                SAFE_ARR_DELETE(negateCnt);
            }

            void Alloc(amf_uint32 channelCnt)
            {
                versions = new int[channelCnt];
                prevVersions = new int[channelCnt];
                channels = new int[channelCnt];
                lens = new int[channelCnt];
                responses = new float*[channelCnt];
                negateCnt = new int[channelCnt];
                ZeroMemory(negateCnt, sizeof(int) * channelCnt);
                Clear(channelCnt);
            }

            void Clear(amf_uint32 channelCnt)
            {
                ZeroMemory(versions, sizeof(int) * channelCnt);
                ZeroMemory(prevVersions, sizeof(int) * channelCnt);
                ZeroMemory(channels, sizeof(int) * channelCnt);
                ZeroMemory(lens, sizeof(int) * channelCnt);
                ZeroMemory(responses, sizeof(float*) * channelCnt);
                updatesCnt = 0;
            }

            void Pack(const GraalArgs &from, amf_uint32 channelCnt)
            {
                Clear(channelCnt);

                for (amf_uint32 channelId = 0; channelId < channelCnt; channelId++)
                {
                    if (from.lens[channelId] > 0) {
                        versions[updatesCnt] = from.versions[channelId];
                        channels[updatesCnt] = from.channels[channelId];
                        lens[updatesCnt] = from.lens[channelId];
                        responses[updatesCnt] = from.responses[channelId];
                        updatesCnt++;
                    }
                }
            }

            void Negate(
                const GraalArgs &from, 
                amf_uint32 channelCnt, 
                amf_uint32 fromVersion, 
                amf_uint32 toVersion)
            {
                Clear(channelCnt);

                amf_uint32 lastChannelId = 0;
                for (amf_uint32 channelId = 0; channelId < channelCnt; channelId++)
                {
                    if (from.lens[channelId] == 0) {
                        // We keep copying a stopped channel from curr slot to update slot up
                        // until its propagated through out all the IR buffers (N_FILTER_STATES)
                        if (negateCnt[channelId] >= N_FILTER_STATES-1) { continue; }
                        versions[updatesCnt] = toVersion;
                        prevVersions[updatesCnt] = fromVersion;
                        channels[updatesCnt] = channelId;
                        negateCnt[channelId]++;
                        updatesCnt++;
                    } else {
                        negateCnt[channelId] = 0;// reset the counter
                    }
                }
            }

            int *versions;
            int *prevVersions;
            int *channels;
            int *lens;
            float **responses;
            int *negateCnt; // Counts how many times a stopped channel has been copied from curr to update slot
            amf_uint32 updatesCnt;
        };

        // response upload parameters (to pass to upload facilities).
        GraalArgs m_uploadArgs;
        // response accumulated parameters (to use in update facility).
        GraalArgs m_accumulatedArgs;
        // response copying parameters (to pass to update facilities).
        GraalArgs m_copyArgs;
        // response update parmeters (to pass to update facilities).
        GraalArgs m_updateArgs;
        // audio stream parameters (to pass to process facilities).
        // Two buffers are used for storing the IR indices; so that back to back calls 
        // to the ProcessInternal() during the crossfade will not corrupt the already stored data 
        const static int PARAM_BUF_COUNT = 2;
        int * m_s_versions[PARAM_BUF_COUNT];
        int * m_s_channels;
        int m_n_delays_onconv_switch;
        int m_onconv_switch_delay_counter;
        bool m_doHeadTailXfade;
        // m_param_buf_idx is used to flip flop between alternate m_s_versions buffers,
        int m_param_buf_idx;
        /*************************************************************************************
        AmdTrueAudioConvolution destructor
        deallocates all internal buffers.
        *************************************************************************************/

    };
} //amf
