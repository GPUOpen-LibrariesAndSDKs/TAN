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

        typedef struct TANSampleBuffer {
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
        AMFComputePtr               m_pDevice;

        TAN_CONVOLUTION_METHOD      m_eConvolutionMethod;
        amf_uint32                  m_iLengthInSamples;
        amf_uint32                  m_iBufferSizeInSamples;
        amf_uint32                  m_iChannels;    // Number of channels our buffers are allocated
                                                    // to, as opposed to m_MaxChannels.

        AMFComputeKernelPtr         m_pKernelCrossfade;
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

        bool                        m_doFftOnGpu;
        bool                        m_doXFadeOnGpu;


    private:
        bool m_initialized;
        int m_length;           // Next power of two for the response's length.
        int m_log2len;          // =log2(m_length)
        int m_MaxChannels;      // Current number of channels, less or equal to m_iChannels.
        float **m_OutSamples;   // Buffer to store FFT, multiplication and ^FFT for a buffer.
        TANSampleBuffer m_pXFadeSamples;  // For cross-fading purposes.
        float *m_silence;       // Array filled with zeroes, used to emulate silent signal.

        TANSampleBuffer m_internalOutBufs;
        TANSampleBuffer m_internalInBufs;
        bool *m_availableChannels;
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

#  define N_FILTER_STATES 3
        // Round-queue to store responces (from Update() method), items in this queue can be in
        // five states:
        // - FREE   - to be filled by Update() calls.
        // - ACCUM  - accumulating responses before the next Update processing to happen in a
        //            separate CPU thread.
        // - UPDATE - responses being processed before Process() calls can start using them.
        // - READY  - processed responses are waiting for the next Process() call to
        //            start using them.
        // - USED   - currently used by the Process(), can be more than one during cross-fading.
        ovlAddFilterState *m_FilterState[N_FILTER_STATES];

        AMFQueue<int> m_readyFilterStateIds;    // Queue of READY indexes.
        AMFQueue<int> m_freeFilterStateIds;     // Queue of FREE indexes.
        int m_idxFilter;                        // Currently USED current index.
        int m_idxPrevFilter;                    // Currently USED previous index (for crossfading).
        amf_int m_accumIdx;                     // ACCUM slot's index.
        amf_int m_updIdx;                       // UPDATE slot's index.

        int m_first_round_ever;

        AMFEvent m_procReadyForNewResponsesEvent;
        AMFEvent m_updateFinishedProcessing;

        class UpdateThread : public AMFThread
        {
        protected:
            TANConvolutionImpl *m_pParent;
        public:
            UpdateThread(TANConvolutionImpl *pParent) : m_pParent(pParent) {}
            void Run() override { m_pParent->UpdateThreadProc(this); }
        };
        UpdateThread m_updThread;
        void UpdateThreadProc(AMFThread *pThread);

        AMF_RESULT VectorComplexMul(float *vA, float *vB, float *out, int count);

        amf_size ovlAddProcess(void *state, float **inputData, float **outputData, amf_size length,
                               amf_uint32 n_channels);

        AMF_RESULT ProcessInternal(
            int idx, void *state,
            TANSampleBuffer pBuffersInput,
            TANSampleBuffer pBufferOutput,
            amf_size length,
            const amf_uint32 flagMasks[],
            amf_size *pNumOfSamplesProcessed,
            int prev_input = 0,
            int advance_time = 1,
            int ocl_skip_stage = 0
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
            }

            void Alloc(amf_uint32 channelCnt)
            {
                versions = new int[channelCnt];
                prevVersions = new int[channelCnt];
                channels = new int[channelCnt];
                lens = new int[channelCnt];
                responses = new float*[channelCnt];
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
                        versions[updatesCnt] = toVersion;
                        prevVersions[updatesCnt] = fromVersion;
                        channels[updatesCnt] = channelId;
                        updatesCnt++;
                    }
                }
            }

            int *versions;
            int *prevVersions;
            int *channels;
            int *lens;
            float **responses;
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
        int * m_s_versions;
        int * m_s_channels;
        int m_n_delays_onconv_switch;
        int m_onconv_switch_delay_counter;


        /*************************************************************************************
        AmdTrueAudioConvolution destructor
        deallocates all internal buffers.
        *************************************************************************************/

    };
} //amf
