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
#include "cl/cl.h"
#include "ConvolutionImpl.h"
#include "../core/TANContextImpl.h"
#include "public/common/AMFFactory.h"

#include <tuple>
#include "../fft\FFTImpl.h"

#include "Crossfading.cl.h"

#define AMF_FACILITY L"TANConvolutionImpl"

using namespace amf;

static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] = 
{
#if AMF_BUILD_OPENCL
    {AMF_MEMORY_OPENCL,     L"OpenCL"},
#endif
    {AMF_MEMORY_HOST,       L"CPU"},
    {AMF_MEMORY_UNKNOWN,    0}  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
#define RETURN_IF_FAILED(ret) \
    if ((ret) != AMF_OK) goto ErrorHandling;
#define RETURN_IF_FALSE(expr, retVariable, retCode) \
    if ((expr) == false) { \
        retVariable = retCode;\
        goto ErrorHandling; \
    }
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateConvolution(
    amf::TANContext* pContext,
    amf::TANConvolution** ppConvolution
    )
{
    AMF_RETURN_IF_FALSE(pContext, AMF_INVALID_ARG, L"pContext == nullptr");
    AMF_RETURN_IF_FALSE(ppConvolution, AMF_INVALID_ARG, L"ppConvolution == nullptr");

    TANContextImplPtr contextImpl(pContext);
    *ppConvolution = new TANConvolutionImpl(pContext);
    (*ppConvolution)->Acquire();
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
TANConvolutionImpl::TANConvolutionImpl(TANContext *pContextTAN) 
    :m_pContextTAN(pContextTAN)
    ,m_eConvolutionMethod(TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED)
    ,m_iLengthInSamples(0)
    ,m_iBufferSizeInSamples(0)
    ,m_iChannels(0)
    ,m_eOutputMemoryType(AMF_MEMORY_HOST)
    ,m_updThread(this)
{
    TANContextImplPtr contextImpl(pContextTAN);
    m_pUpdateContextAMF = contextImpl->GetGeneralCompute();
    m_pProcContextAMF = contextImpl->GetConvolutionCompute();

    // CPU processing case.
    if (!m_pProcContextAMF)
    {
        m_pProcContextAMF = m_pUpdateContextAMF;
    }

    AMFPrimitivePropertyInfoMapBegin
        AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE ,  L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
    AMFPrimitivePropertyInfoMapEnd

    m_initialized = false;
    m_idxFilter = -1;
    m_idxPrevFilter = -1;
    m_accumIdx = -1;
    m_updIdx = -1;
    m_graal_conv = nullptr;

    m_s_versions = NULL;
    m_s_channels = NULL;
    m_n_delays_onconv_switch = 0;
    m_onconv_switch_delay_counter = 0;

    memset(m_FilterState, 0, sizeof(m_FilterState));
    m_tailLeftOver = nullptr;
    m_availableChannels = nullptr;
    m_internalOutBufs.buffer.host = nullptr;
    m_internalInBufs.buffer.host = nullptr;
    m_silence = nullptr;
    m_OutSamples = nullptr;
    m_MaxChannels = -1;
    m_log2len = -1;
    m_length = -1;

    m_doXFadeOnGpu = false;
    m_doFftOnGpu = false;

}
//-------------------------------------------------------------------------------------------------
TANConvolutionImpl::~TANConvolutionImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::Init(
    TAN_CONVOLUTION_METHOD convolutionMethod,
    amf_uint32 responseLengthInSamples,
    amf_uint32 bufferSizeInSamples,
    amf_uint32 channels)
{
    AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    // Determine how to initialize based on context, CPU for CPU and GPU for GPU
    if (m_pContextTAN->GetOpenCLContext())
    {
        return Init(
            convolutionMethod, responseLengthInSamples, bufferSizeInSamples, channels, true);
    }
    else
    {
        return Init(
            convolutionMethod, responseLengthInSamples, bufferSizeInSamples, channels, false);
    }
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::InitCpu(
    TAN_CONVOLUTION_METHOD convolutionMethod, 
    amf_uint32 responseLengthInSamples,
    amf_uint32 bufferSizeInSamples, 
    amf_uint32 channels)
{
    AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    return Init(convolutionMethod, responseLengthInSamples, bufferSizeInSamples, channels, false);
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::InitGpu(
    TAN_CONVOLUTION_METHOD convolutionMethod,
    amf_uint32 responseLengthInSamples,
    amf_uint32 bufferSizeInSamples,
    amf_uint32 channels)
{
    AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
        L"Cannot initialize after termination");
    
    AMF_RETURN_IF_FALSE(m_pContextTAN->GetOpenCLContext() != NULL, AMF_WRONG_STATE,
        L"Cannot initialize on GPU with a CPU context");

    return Init(convolutionMethod, responseLengthInSamples, bufferSizeInSamples, channels, true);
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_updThread.RequestStop();
    m_procReadyForNewResponsesEvent.SetEvent();
    m_updThread.WaitForStop();

    m_updateFinishedProcessing.SetEvent();

    m_pContextTAN.Release();
    m_pProcContextAMF.Release();
    m_pUpdateContextAMF.Release();
    m_pDevice.Release();

    m_pKernelCrossfade.Release();

    m_pTanFft.Release();
    m_pUpdateTanFft.Release();

    m_freeFilterStateIds.Clear();
    m_readyFilterStateIds.Clear();
    m_idxFilter = -1;
    m_idxPrevFilter = -1;

    deallocateBuffers();

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::UpdateResponseTD(
    float* pBuffer[],
    amf_size numOfSamplesToProcess,
    const amf_uint32 flagMasks[],
    const amf_uint32 operationFlags
)
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    AMF_RETURN_IF_FALSE(pBuffer != NULL, AMF_INVALID_ARG, L"pBuffer == NULL");

    AMF_RESULT res = AMF_OK;

    AMFLock lock(&m_sectUpdate);

    // process

    TANSampleBuffer sampleBuffer;
    sampleBuffer.buffer.host = pBuffer;
    sampleBuffer.mType = AMF_MEMORY_HOST;

    return UpdateResponseTD(sampleBuffer, numOfSamplesToProcess, flagMasks, operationFlags);
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::UpdateResponseTD(
    cl_mem *pBuffer,
    amf_size numOfSamplesToProcess,
    const amf_uint32 flagMasks[],
    const amf_uint32 operationFlags
)
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    AMF_RETURN_IF_FALSE(pBuffer != NULL, AMF_INVALID_ARG, L"pBuffer == NULL");

    AMF_RESULT res = AMF_OK;

    AMFLock lock(&m_sectUpdate);

    bool needToUpdateInputBuf = false;
 
    // process
    TANSampleBuffer sampleBuffer;
    sampleBuffer.buffer.clmem = pBuffer;
    sampleBuffer.mType = AMF_MEMORY_OPENCL;

    return UpdateResponseTD(sampleBuffer, numOfSamplesToProcess, flagMasks, operationFlags);
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::UpdateResponseTD(
    TANSampleBuffer pBuffer,
    amf_size numOfSamplesToProcess,
    const amf_uint32 flagMasks[],
    const amf_uint32 operationFlags
)
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess <= m_iLengthInSamples, AMF_INVALID_ARG,
                        L"Inconsistent with one set in Init() call length passed");

    AMFLock lock(&m_sectUpdate);
    AMFLock lockAccum(&m_sectAccum);

    // Check if all the channels are disabled.
    if (flagMasks)
    {
        amf_uint32 actualChannelCnt = 0;
        for (amf_uint32 channelId = 0; channelId < m_iChannels; channelId++)
        {
            if (!(flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT))
            {
                actualChannelCnt++;
            }

            // If we need to flush the current stream
            if ((flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM) &&
                // ... and we have been initialized before.
                m_idxFilter != -1)
            {
                AMF_RETURN_IF_FAILED(Flush(m_idxFilter, channelId), L"Flush failed");
            }
        }
        AMF_RETURN_IF_FALSE(actualChannelCnt > 0, AMF_WRONG_STATE, L"No active channels found");
    }

    AMF_RESULT ret = AMF_OK;

    // Allocate an empty response (or wait until available).
    amf_ulong uiid;
    if (m_accumIdx == -1) {
        if (!m_freeFilterStateIds.Get(uiid, m_accumIdx, 100))
        {
            // Require the client to try again, otherwise it might miss a closing event and hang.
            // Shouldn't come here, though, anymore.
            return AMF_INPUT_FULL;
        }

        m_accumulatedArgs.Clear(m_iChannels);
    }

    const amf_uint32 prevIndex = (m_accumIdx + N_FILTER_STATES - 1) % N_FILTER_STATES;

    switch (m_eConvolutionMethod) {
        case TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD:
            if (pBuffer.mType == AMF_MEMORY_HOST)
            {
            float **filter = ((ovlAddFilterState *)m_FilterState[m_accumIdx])->m_Filter;
            float **overlap = ((ovlAddFilterState *)m_FilterState[m_accumIdx])->m_Overlap;

            for (amf_uint32 n = 0; n < m_iChannels; n++){
                if (!flagMasks || !(flagMasks[n] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT))
                {
                    memset(filter[n], 0, 2 * m_length * sizeof(float));
                    memset(overlap[n], 0, m_length * sizeof(float));

                    for (int k = 0; k < numOfSamplesToProcess; k++){
                        // copy data to real part (even samples):
                        filter[n][k << 1] = pBuffer.buffer.host[n][k];
                    }
                
                    m_accumulatedArgs.responses[n] = filter[n];
                    m_accumulatedArgs.lens[n] = static_cast<int>(numOfSamplesToProcess);
                    m_accumulatedArgs.updatesCnt++;
                    }
                }
            }
            else {
                return AMF_NOT_IMPLEMENTED;
            }
        break;

        case TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED:
        case TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED:
        case TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL:
        {
            graal::CGraalConv*graalConv = (graal::CGraalConv*)m_graal_conv;
            amf_uint32 n_channels = 0;
            if (pBuffer.mType == AMF_MEMORY_OPENCL){

                cl_mem *clResponses = new cl_mem[m_iChannels];

                for (amf_uint32 n = 0; n < m_iChannels; n++)
                {
                    if (!flagMasks || !(flagMasks[n] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT))
                    {
                        //m_uploadArgs.responses[n_channels] = pBuffer.buffer.host[n];
                        clResponses[n_channels] = pBuffer.buffer.clmem[n];
                        m_uploadArgs.versions[n_channels] = m_accumIdx;
                        m_uploadArgs.channels[n_channels] = n;
                        m_uploadArgs.lens[n_channels] = static_cast<int>(numOfSamplesToProcess);
                        n_channels++;

                        m_accumulatedArgs.versions[n] = m_accumIdx;
                        m_accumulatedArgs.channels[n] = n;
                        m_accumulatedArgs.lens[n] = static_cast<int>(numOfSamplesToProcess);
                        m_accumulatedArgs.updatesCnt++;
                    }
                }

                AMF_RETURN_IF_FALSE(graalConv->uploadConvGpuPtrs(
                    n_channels, m_uploadArgs.versions, m_uploadArgs.channels,
                    clResponses,
                    m_uploadArgs.lens, true) == GRAAL_SUCCESS,
                    AMF_UNEXPECTED, L"Internal graal's failure");

                delete clResponses;

            }
            else if (pBuffer.mType == AMF_MEMORY_HOST){
                for (amf_uint32 n = 0; n < m_iChannels; n++)
                {
                    if (!flagMasks || !(flagMasks[n] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT))
                    {
                        m_uploadArgs.responses[n_channels] = pBuffer.buffer.host[n];
                        m_uploadArgs.versions[n_channels] = m_accumIdx;
                        m_uploadArgs.channels[n_channels] = n;
                        m_uploadArgs.lens[n_channels] = static_cast<int>(numOfSamplesToProcess);
                        n_channels++;

                        m_accumulatedArgs.versions[n] = m_accumIdx;
                        m_accumulatedArgs.channels[n] = n;
                        m_accumulatedArgs.lens[n] = static_cast<int>(numOfSamplesToProcess);
                        m_accumulatedArgs.updatesCnt++;
                    }
                }

                AMF_RETURN_IF_FALSE(graalConv->uploadConvHostPtrs(
                    n_channels, m_uploadArgs.versions, m_uploadArgs.channels,
                    const_cast<const float**>(m_uploadArgs.responses),
                    m_uploadArgs.lens, true) == GRAAL_SUCCESS,
                    AMF_UNEXPECTED, L"Internal graal's failure");
            }
        }
        break;

        default:
            AMF_RETURN_IF_FAILED(AMF_NOT_IMPLEMENTED, L"Unsupported convolution method");
    }

    // Wait until finished if it's our first run, most likely user would expect it to happen.
    const bool blockUntilReady =
        (operationFlags & TAN_CONVOLUTION_OPERATION_FLAG_BLOCK_UNTIL_READY) || m_idxFilter == -1;

    // Wait until finished.
    if (blockUntilReady) {
        lockAccum.Unlock();

        while (!m_updThread.StopRequested()) {
            lockAccum.Lock();
            const bool isDone = (m_accumIdx == -1 && m_updIdx == -1);
            lockAccum.Unlock();
            if (isDone) {
                break;
            }

            m_procReadyForNewResponsesEvent.SetEvent();
            m_updateFinishedProcessing.Lock();
        }
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::UpdateResponseFD(
    float* ppBuffers[],
    amf_size numOfSamplesToProcess,
    const amf_uint32 flagMasks[],
    const amf_uint32 operationFlags
)    
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    return AMF_NOT_SUPPORTED;
}
////-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::UpdateResponseFD(
    cl_mem ppBuffers[],
    amf_size numOfSamplesToProcess,
    const amf_uint32 flagMasks[],
    const amf_uint32 operationFlags
)
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    return AMF_NOT_SUPPORTED;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::Process(
    float* ppBufferInput[],
    float* ppBufferOutput[],
    amf_size numOfSamplesToProcess,
    // Masks of flags from enum
    // TAN_CONVOLUTION_CHANNEL_FLAG.
    const amf_uint32 flagMasks[],
    amf_size *pNumOfSamplesProcessed
)                    // system memory
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);

    AMF_RETURN_IF_FALSE(ppBufferInput != NULL, AMF_INVALID_ARG, L"ppBufferInput == NULL");
    AMF_RETURN_IF_FALSE(ppBufferOutput != NULL, AMF_INVALID_ARG, L"ppBufferOutput == NULL");

    AMF_RESULT res = AMF_OK;

    // process
    TANSampleBuffer inBuf, outBuf;
    inBuf.buffer.host = ppBufferInput;
    inBuf.mType = AMF_MEMORY_HOST;
    outBuf.buffer.host = ppBufferOutput;
    outBuf.mType = AMF_MEMORY_HOST;

    return Process(inBuf, outBuf, numOfSamplesToProcess, flagMasks);
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConvolutionImpl::Process(
    cl_mem pBufferInput[],
    cl_mem pBufferOutput[],
    amf_size numOfSamplesToProcess,
    // Masks of flags from enum
    // TAN_CONVOLUTION_CHANNEL_FLAG.
    const amf_uint32 flagMasks[],
    amf_size *pNumOfSamplesProcessed
)          // cl_mem
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);

    AMF_RETURN_IF_FALSE(pBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
    AMF_RETURN_IF_FALSE(pBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");

    AMF_RESULT res = AMF_OK;

    //// process
    TANSampleBuffer inBuf, outBuf;
    inBuf.buffer.clmem = pBufferInput;
    inBuf.mType = AMF_MEMORY_HOST;
    outBuf.buffer.clmem = pBufferOutput;
    outBuf.mType = AMF_MEMORY_HOST;

    return Process(inBuf, outBuf, numOfSamplesToProcess, flagMasks);

}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConvolutionImpl::Process(
    TANSampleBuffer pBufferInput,
    TANSampleBuffer pBufferOutput,
    amf_size numOfSamplesToProcess,
    // Masks of flags from enum
    // TAN_CONVOLUTION_CHANNEL_FLAG.
    const amf_uint32 flagMasks[],
    amf_size *pNumOfSamplesProcessed
)
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);

    AMFLock lock(&m_sectProcess);

    AMF_RESULT res = AMF_OK;

    AMF_RETURN_IF_FALSE(m_idxFilter >= 0, AMF_NOT_INITIALIZED,
                        L"Update() method must be called prior any calls to Process()");

    AMF_RESULT ret = AMF_OK;
    if (pNumOfSamplesProcessed)
    {
        *pNumOfSamplesProcessed = 0;
    }

    // Check if we've finished with the latest crossfading (m_idxPrevFilte == -1) and there is no
    // new responses available (obtained in the Update() method).
    int idx = m_idxFilter;
    amf_ulong uuid = 0;
    if (m_idxPrevFilter == -1 && !m_readyFilterStateIds.Get(uuid, idx, 0))
    {
        // Run update in the background.
        if (m_accumIdx != -1) {
            m_procReadyForNewResponsesEvent.SetEvent();
        }

        ret = ProcessInternal(idx, m_FilterState[idx], pBufferInput, pBufferOutput,
                              numOfSamplesToProcess, flagMasks, pNumOfSamplesProcessed);
        AMF_RETURN_IF_FAILED(ret);
    }
    // We've switched to a new filter response, so we'll cross fade from old to new:
    else {
        // Move the new READY index to the USED indexes (if it's not already there from the
        // previous steps of crossfading).
        if (idx != m_idxFilter)
        {
            m_idxPrevFilter = m_idxFilter;
            m_idxFilter = idx;
        }

        if (m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD ||
            m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED ||
            m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED)
        {
            // last previous conv run, do not advance the internal Graal timer
            ret = ProcessInternal(m_idxPrevFilter, m_FilterState[m_idxPrevFilter], pBufferInput,
                                  pBufferOutput, numOfSamplesToProcess, flagMasks,
                                  pNumOfSamplesProcessed, 0, 0);
            RETURN_IF_FAILED(ret);

            // new conv run, previous input, advance the internal Graal timer
            ret = ProcessInternal(idx, m_FilterState[idx], pBufferInput, m_pXFadeSamples,
                                  numOfSamplesToProcess, flagMasks, pNumOfSamplesProcessed, 1);
            RETURN_IF_FAILED(ret);

            // cross fade old to new:
            AMF_RETURN_IF_FAILED(Crossfade(pBufferOutput, numOfSamplesToProcess));

            // We don't need the m_idxPrevFilter, so return it to the FREE queue. All the buffers'
            // clearing is happening in Update() methods.
            AMF_RETURN_IF_FALSE(m_freeFilterStateIds.Add(0, m_idxPrevFilter), AMF_UNEXPECTED,
                                L"Internal synchronization issue");
            m_idxPrevFilter = -1;
        }

    } // if (new response)

    if (pNumOfSamplesProcessed)
    {
        *pNumOfSamplesProcessed = numOfSamplesToProcess;
    }

    //ReleaseMutex(m_hMutex);
    return AMF_OK;

ErrorHandling:
    AMF_RETURN_IF_FALSE(m_freeFilterStateIds.Add(0, m_idxPrevFilter), AMF_UNEXPECTED);
    m_idxPrevFilter = -1;
    AMF_RETURN_IF_FAILED(ret);
    return ret;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANConvolutionImpl::GetNextFreeChannel(
    amf_uint32 *pChannelIndex,
    const amf_uint32 flagMasks[]
    )
{
    AMF_RETURN_IF_FALSE(m_initialized, AMF_NOT_INITIALIZED);
    AMF_RETURN_IF_FALSE(pChannelIndex != NULL, AMF_INVALID_ARG, L"pChannelIndex == NULL");

    *pChannelIndex = -1;

    if (!flagMasks)
    {
        // No stopped channels!
        return AMF_NOT_FOUND;
    }

    for (amf_uint32 channelId = 0; channelId < m_iChannels; channelId++){
        if (flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT) {
            if (m_tailLeftOver[channelId] <= 0) {
                *pChannelIndex = channelId;
                return AMF_OK;
            }
        }
    }

    return AMF_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  TANConvolutionImpl::Init(
    TAN_CONVOLUTION_METHOD convolutionMethod,
    amf_uint32 responseLengthInSamples,
    amf_uint32 bufferSizeInSamples,
    amf_uint32 channels,
    bool doProcessingOnGpu
)
{
    if ((doProcessingOnGpu && !m_pContextTAN->GetOpenCLContext()) ||
        (!doProcessingOnGpu && m_pContextTAN->GetOpenCLContext()))
    {
        // Attempting to do processing in a state not initialized with the context
        return AMF_WRONG_STATE;
    }

    AMFLock lock(&m_sect);

    AMF_RETURN_IF_FALSE(responseLengthInSamples > 0, AMF_INVALID_ARG, L"lengthInSamples == 0");
    AMF_RETURN_IF_FALSE(bufferSizeInSamples > 0, AMF_INVALID_ARG, L"bufferSizeInSamples == 0");
    AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
#if defined (_M_X64)
    AMF_RETURN_IF_FALSE(!doProcessingOnGpu || __popcnt64(bufferSizeInSamples) == 1, AMF_INVALID_ARG,
                        L"bufferSizeInSamples must be power of 2");
#else
    AMF_RETURN_IF_FALSE(!doProcessingOnGpu || __popcnt(bufferSizeInSamples) == 1, AMF_INVALID_ARG,
        L"bufferSizeInSamples must be power of 2");
#endif
    AMF_RETURN_IF_FALSE(!m_initialized, AMF_ALREADY_INITIALIZED, L"Already initialized");
    AMF_RETURN_IF_FALSE((NULL != m_pContextTAN), AMF_WRONG_STATE,
                        L"Cannot initialize after termination");
    AMF_RETURN_IF_FALSE(convolutionMethod < TAN_CONVOLUTION_METHOD_TIME_DOMAIN, AMF_NOT_SUPPORTED,
                        L"convolutionMethod isn't supported");

    m_pDevice = m_pProcContextAMF;
    AMF_RETURN_IF_FALSE(!doProcessingOnGpu || m_pDevice != nullptr, AMF_OPENCL_FAILED,
                        L"Cannot create OpenCL device for CPU-only context.");
    AMF_RETURN_IF_FALSE(convolutionMethod == TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD ||
                        m_pDevice != nullptr, AMF_OPENCL_FAILED, L"Fail to create OpenCL device");

    AMF_RETURN_IF_FALSE(m_readyFilterStateIds.SetQueueSize(N_FILTER_STATES), AMF_UNEXPECTED,
                        L"Cannot create a semaphore");
    AMF_RETURN_IF_FALSE(m_freeFilterStateIds.SetQueueSize(N_FILTER_STATES), AMF_UNEXPECTED,
                        L"Cannot create a semaphore");
    m_freeFilterStateIds.Add(0, 1);
    m_freeFilterStateIds.Add(0, 2);
    m_freeFilterStateIds.Add(0, 0);

    m_doFftOnGpu = doProcessingOnGpu;
    m_doXFadeOnGpu = false;


    AMF_RESULT res = AMF_OK;

    m_eConvolutionMethod = convolutionMethod;
    m_iLengthInSamples = responseLengthInSamples;
    m_iBufferSizeInSamples = bufferSizeInSamples;
    m_iChannels = channels;

    if (doProcessingOnGpu && m_pDevice)
    {
        AMFPrograms* pPrograms = NULL;
        g_AMFFactory.GetFactory()->GetPrograms(&pPrograms);
        if (pPrograms)
        {
            // Load crossfade kernel
            AMF_RETURN_IF_FAILED(pPrograms->RegisterKernelSource(
                &m_KernelIdCrossfade,
                L"crossfade", "crossfade",
                CrossfadingCount, Crossfading,
                0));
        }
        AMF_RETURN_IF_FAILED(m_pDevice->GetKernel(m_KernelIdCrossfade, &m_pKernelCrossfade),
            L"GetKernel() failed");
    }

    amf_int64 tmp = 0;
    GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
    m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;

    // Initialize TAN FFT objects.
    if (convolutionMethod == TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD)
    {
        AMF_RETURN_IF_FAILED(TANCreateFFT(m_pContextTAN, &m_pUpdateTanFft, false));
        AMF_RETURN_IF_FAILED(m_pUpdateTanFft->Init());
        if (m_pProcContextAMF == m_pUpdateContextAMF)
        {
            m_pTanFft = m_pUpdateTanFft;
        }
        else {
            AMF_RETURN_IF_FAILED(TANCreateFFT(m_pContextTAN, &m_pTanFft , true));
            AMF_RETURN_IF_FAILED(m_pTanFft->Init());
        }
    }

    amf_uint32 len = 1;
    int log2len = 0;
    // use next bigger power of 2:
    while (len < responseLengthInSamples){
        len <<= 1;
        ++log2len;
    }

    if (m_initialized){
        deallocateBuffers();
    }

    m_MaxChannels = m_iChannels;
    m_length = len;
    m_log2len = log2len;

    res = allocateBuffers();
    if (res == 0) {
        m_initialized = true;
    }

    m_updThread.Init();
    m_updThread.Start();

    return res;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT amf::TANConvolutionImpl::Flush(amf_uint32 filterStateId, amf_uint32 channelId)
{
    auto pFilterState = static_cast<ovlAddFilterState*>(m_FilterState[filterStateId]);

    // If we flush the current set, we need to synchronize with process().
    if (filterStateId == m_idxFilter)
    {
        m_sectProcess.Lock();
        if (m_eConvolutionMethod != TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD) {
            graal::CGraalConv* pGraalConv = (graal::CGraalConv*)m_graal_conv;
            AMF_RETURN_IF_FAILED(pGraalConv->finishProcess(), L"Sync failed");
        }
    }

    if (m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD)
    {
        float **overlap = pFilterState->m_Overlap;
        memset(overlap[channelId], 0, m_length * sizeof(float));
    }
    else {
        // Flushing of Graal objects.
        graal::CGraalConv* pGraalConv = (graal::CGraalConv*)m_graal_conv;
        AMF_RETURN_IF_FALSE(pGraalConv->flush(channelId) == GRAAL_SUCCESS, AMF_UNEXPECTED,
                            L"Flushing failed");
    }

    // If we flush the current set, we need to synchronize with process().
    if (filterStateId == m_idxFilter)
    {
        m_sectProcess.Unlock();
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_FAST_CALL TANConvolutionImpl::Crossfade(
    TANSampleBuffer pBufferOutput,
    amf_size numOfSamplesToProcess
)
{
    // Move buffers to appropriate locations
    if (pBufferOutput.mType == AMF_MEMORY_OPENCL)
    {
        //// GPU Implementation
        //// Create as many work-items as there are samples needed to be crossfaded
        amf_size global[3] = { numOfSamplesToProcess, 0, 0 };
        amf_size local[3] = { 0, 0, 0 };

        for (int n = 0; n < m_MaxChannels; n++)
        {
            if (!m_availableChannels[n])
            { // !available == running
                // Set kernel arguments and execute; m_pXFadeSamples should be in GPU memory already
                int index = 0;
                AMF_RETURN_IF_FAILED(m_pKernelCrossfade->SetArgBufferNative(index++,
                    m_pXFadeSamples.buffer.clmem[n], amf::AMF_ARGUMENT_ACCESS_READWRITE),
                    L"Failed to set OpenCL kernel argument");
                AMF_RETURN_IF_FAILED(m_pKernelCrossfade->SetArgBufferNative(index++,
                    pBufferOutput.buffer.clmem[n], amf::AMF_ARGUMENT_ACCESS_READWRITE),
                    L"Failed to set OpenCL kernel argument");
                AMF_RETURN_IF_FAILED(m_pKernelCrossfade->Enqueue(1, NULL, global, local),
                    L"Enqueue() failed");
            }
        }
        ;
    }
    else
    {
        // CPU Implementation
        for (int n = 0; n < m_MaxChannels; n++){
            if (!m_availableChannels[n]){ // !available == running
                float *pFltOut = pBufferOutput.buffer.host[n];
                float *pFltFade = m_pXFadeSamples.buffer.host[n];
                for (amf_size i = 0; i < numOfSamplesToProcess; i++){
                    pFltOut[i] = (pFltFade[i] * i +
                        pFltOut[i] * (numOfSamplesToProcess - i)) /
                        float(numOfSamplesToProcess);
                }
            }
        }
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANConvolutionImpl::allocateBuffers()
{
    //allocate stuff:
    //...
    m_OutSamples = new float *[m_iChannels];

    // internal pointer arrays used to shuffle buffer order
    // no need to allocate actual data buffers here:
    if (m_doXFadeOnGpu) {
        m_internalInBufs.buffer.clmem = new cl_mem [m_iChannels];
        m_internalInBufs.mType = AMF_MEMORY_OPENCL;
        m_internalOutBufs.buffer.clmem = new cl_mem [m_iChannels];
        m_internalOutBufs.mType = AMF_MEMORY_OPENCL;
    }
    else {
        m_internalInBufs.buffer.host = new float *[m_iChannels];
        m_internalInBufs.mType = AMF_MEMORY_HOST;
        m_internalOutBufs.buffer.host = new float *[m_iChannels];
        m_internalOutBufs.mType = AMF_MEMORY_HOST;
    }

    m_availableChannels = new bool[m_iChannels];
    for (amf_uint32 i = 0; i < m_iChannels; i++) m_availableChannels[i] = false;

    m_tailLeftOver = new int[m_iChannels];
    memset(m_tailLeftOver, 0, sizeof(int)*m_iChannels);
    m_silence = new float[m_length];
    memset(m_silence, 0, sizeof(float)*m_length);

    for (amf_uint32 n = 0; n < m_iChannels; n++){
        m_OutSamples[n] = new float[2 * m_length];
        memset(m_OutSamples[n], 0, (2 * m_length) * sizeof(float));
        m_internalInBufs.buffer.host[n] = NULL;
        m_internalOutBufs.buffer.host[n] = NULL;
    }

    m_updateArgs.Alloc(m_iChannels);
    m_uploadArgs.Alloc(m_iChannels);
    m_copyArgs.Alloc(m_iChannels);
    m_accumulatedArgs.Alloc(m_iChannels);

    switch (m_eConvolutionMethod) {
    case TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD:
        // allocate state data for ovlAddProcess:
        for (int i = 0; i < N_FILTER_STATES; i++){
            m_FilterState[i] = new ovlAddFilterState;
            ((ovlAddFilterState *)m_FilterState[i])->m_Filter = new float *[m_iChannels];
            ((ovlAddFilterState *)m_FilterState[i])->m_Overlap = new float *[m_iChannels];
            ((ovlAddFilterState *)m_FilterState[i])->m_internalFilter = new float *[m_iChannels];
            ((ovlAddFilterState *)m_FilterState[i])->m_internalOverlap = new float *[m_iChannels];
        }
        for (amf_uint32 n = 0; n < m_iChannels; n++){
            for (int i = 0; i < N_FILTER_STATES; i++){
                ((ovlAddFilterState *)m_FilterState[i])->m_Filter[n] = new float[2 * m_length];
                memset(((ovlAddFilterState *)m_FilterState[i])->m_Filter[n], 0, 2 * m_length*sizeof(float));
                ((ovlAddFilterState *)m_FilterState[i])->m_Overlap[n] = new float[m_length];
                memset(((ovlAddFilterState *)m_FilterState[i])->m_Overlap[n], 0, m_length*sizeof(float));
            }
        }
        break;

    case TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL:
    {
        graal::CGraalConv*graalConv = NULL;


        if (m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED)
        {
            graalConv = new graal::CGraalConv_clFFT;
        }
        else
        {
            graalConv = new graal::CGraalConv;
        }

        bool isPartitionedMethod =
            m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED ||
            m_eConvolutionMethod == TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED;

        int ret = graalConv->initializeConv(
            m_pContextTAN, m_pProcContextAMF, m_pUpdateContextAMF,
            m_iChannels,
            (int)m_length,
            (int)m_iBufferSizeInSamples,
            N_FILTER_STATES,
            isPartitionedMethod ? graal::ALG_UNIFORMED : graal::ALG_UNI_HEAD_TAIL);
        if (ret == GRAAL_EXPECTED_FAILURE) {
            return AMF_OPENCL_FAILED;
        }
        else if (ret == GRAAL_NOT_ENOUGH_GPU_MEM)
        {
            return AMF_OUT_OF_MEMORY;
        }
        else if (ret != GRAAL_SUCCESS) {
            return AMF_UNEXPECTED;
        }

        m_s_versions = new int[m_iChannels];
        m_s_channels = new int[m_iChannels];

        m_n_delays_onconv_switch = isPartitionedMethod ? 0 : 2;
        m_graal_conv = graalConv;
    }
    break;

    default:
        AMF_RETURN_IF_FALSE(false, AMF_NOT_SUPPORTED, L"Convolution method not supported");
    }

    if (m_doXFadeOnGpu) {
        // allocate crossfade buffers on GPU memory
        m_pXFadeSamples.mType = AMF_MEMORY_OPENCL;
        m_pXFadeSamples.buffer.clmem = new cl_mem[m_iChannels];
        cl_int clErr;
        for (amf_uint32 i = 0; i < m_iChannels; i++){
            m_pXFadeSamples.buffer.clmem[i] = clCreateBuffer(
                m_pContextTAN->GetOpenCLContext(), CL_MEM_READ_WRITE, sizeof(amf_int32), nullptr, &clErr);
            if (clErr != CL_SUCCESS)
            {
                printf("Could not create OpenCL buffer\n");
                return AMF_FAIL;
            };
        }

    }
    else {
        // allocate crossfade buffers on host memory
        m_pXFadeSamples.mType = AMF_MEMORY_HOST;
        m_pXFadeSamples.buffer.host = new float *[m_iChannels];
        for (amf_uint32 i = 0; i < m_iChannels; i++){
            m_pXFadeSamples.buffer.host[i] = new float[4 * m_length + 2];
            memset(m_pXFadeSamples.buffer.host[i], 0, sizeof(float) * (4 * m_length + 2));
        }
    }

    return AMF_OK;
}

AMF_RESULT TANConvolutionImpl::deallocateBuffers()
{
    for (amf_uint32 n = 0; m_OutSamples && n < m_iChannels; n++){
        SAFE_ARR_DELETE(m_OutSamples[n]);

    }
    SAFE_ARR_DELETE(m_OutSamples);

    switch (m_internalOutBufs.mType) {
    case AMF_MEMORY_HOST:
        delete m_internalOutBufs.buffer.host;
        m_internalOutBufs.buffer.host = nullptr;
        break;
    case AMF_MEMORY_OPENCL:
        delete m_internalOutBufs.buffer.clmem;
        m_internalOutBufs.buffer.clmem = nullptr;
        break;
    default:
        ;
    }
    m_internalOutBufs.mType = AMF_MEMORY_UNKNOWN;

    switch (m_internalInBufs.mType) {
    case AMF_MEMORY_HOST:
        delete m_internalInBufs.buffer.host;
        m_internalInBufs.buffer.host = nullptr;
        break;
    case AMF_MEMORY_OPENCL:
        delete m_internalInBufs.buffer.clmem;
        m_internalInBufs.buffer.clmem = nullptr;
        break;
    default:
        ;
    }
    m_internalInBufs.mType = AMF_MEMORY_UNKNOWN;

    switch (m_pXFadeSamples.mType) {
    case AMF_MEMORY_HOST:
        for (amf_uint32 n = 0; n < m_iChannels; n++){
            delete m_pXFadeSamples.buffer.host[n];
        }
        delete m_pXFadeSamples.buffer.host;
        m_pXFadeSamples.buffer.host = nullptr;
        break;
    case AMF_MEMORY_OPENCL:
        for (amf_uint32 n = 0; n < m_iChannels; n++){
             clReleaseMemObject(m_pXFadeSamples.buffer.clmem[n]);
        }
        delete m_pXFadeSamples.buffer.clmem;
        m_pXFadeSamples.buffer.clmem = nullptr;
        break;
    default:
        ;
    }
    m_internalInBufs.mType = AMF_MEMORY_UNKNOWN;


    SAFE_ARR_DELETE(m_availableChannels);
    SAFE_ARR_DELETE(m_tailLeftOver);
    SAFE_ARR_DELETE(m_silence);

    switch (m_eConvolutionMethod) {
    case TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD:
        // deallocate state data for ovlAddProcess:
        for (amf_uint32 n = 0; n < m_iChannels; n++){
            for (int i = 0; i < N_FILTER_STATES; i++) {
                if (m_FilterState[i] && ((ovlAddFilterState *)m_FilterState[i])->m_Filter[n]) {
                    SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_Filter[n]);
                    SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_Overlap[n]);
                }
            }
        }
        for (int i = 0; m_FilterState[i] && i < N_FILTER_STATES; i++){
            SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_Filter);
            SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_Overlap);
            SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_internalFilter);
            SAFE_ARR_DELETE(((ovlAddFilterState *)m_FilterState[i])->m_internalOverlap);
            SAFE_ARR_DELETE(m_FilterState[i]);
        }
        break;

    case TAN_CONVOLUTION_METHOD_TIME_DOMAIN:
        break;

    case TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL:
    {
        if (m_graal_conv)
        {
            delete (graal::CGraalConv *)m_graal_conv;
            m_graal_conv = nullptr;

            SAFE_ARR_DELETE(m_s_versions);
            SAFE_ARR_DELETE(m_s_channels);

            m_graal_conv = 0;
        }

    }
    break;

    default:
        break;
    }

    m_onconv_switch_delay_counter = 0;
    return AMF_OK;
}

void TANConvolutionImpl::UpdateThreadProc(AMFThread *pThread)
{
    do {
        // Wait for the time to start processing.
        m_procReadyForNewResponsesEvent.Lock();
        if (pThread->StopRequested()) {
            break;
        }

        // Do processing if there is something ready.
        if (m_accumIdx == -1 || m_accumulatedArgs.updatesCnt == 0) {
            m_updateFinishedProcessing.SetEvent();
            continue;
        }

        // Restart accumulation.
        {
            AMFLock lock(&m_sectAccum);
            m_updIdx = m_accumIdx;
            m_accumIdx = -1;

            const amf_uint32 prevIndex = (m_updIdx + N_FILTER_STATES - 1) % N_FILTER_STATES;
            m_updateArgs.Pack(m_accumulatedArgs, m_iChannels);
            m_copyArgs.Negate(m_accumulatedArgs, m_iChannels, prevIndex, m_updIdx);
            m_accumulatedArgs.Clear(m_iChannels);
        }

        // Start processing.
        const amf_uint32 prevIndex = (m_updIdx + N_FILTER_STATES - 1) % N_FILTER_STATES;
        AMF_RESULT ret = AMF_OK;
        switch (m_eConvolutionMethod) {
            case TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD:
            {
                float **filter = ((ovlAddFilterState *)m_FilterState[m_updIdx])->m_Filter;
                float **overlap = ((ovlAddFilterState *)m_FilterState[m_updIdx])->m_Overlap;

                RETURN_IF_FAILED(ret = m_pUpdateTanFft->Transform(
                                            TAN_FFT_TRANSFORM_DIRECTION_FORWARD,
                                            m_log2len, m_updateArgs.updatesCnt,
                                            m_updateArgs.responses, m_updateArgs.responses));

                // Copy data to the new slot, as this channel can be still processed (user doesn't
                // pass Stop flag to Process() method) and we may start doing cross-fading.
                float **const ppOldFilter =
                    ((ovlAddFilterState *)m_FilterState[prevIndex])->m_Filter;
                float **const ppOldOverlap =
                    ((ovlAddFilterState *)m_FilterState[prevIndex])->m_Overlap;

                for (amf_uint32 argId = 0; argId < m_copyArgs.updatesCnt; argId++) {
                    const amf_uint32 channelId = m_copyArgs.channels[argId];
                    memcpy(filter[channelId], ppOldFilter[channelId], 2 * m_length * sizeof(float));
                    memcpy(overlap[channelId], ppOldOverlap[channelId], m_length * sizeof(float));
                }

            }
            break;

            case TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED:
            case TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED:
            case TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL:
            {
                graal::CGraalConv*graalConv = (graal::CGraalConv*)m_graal_conv;
                RETURN_IF_FALSE(graalConv->updateConv(m_updateArgs.updatesCnt,
                                                      m_updateArgs.versions,
                                                      m_updateArgs.channels,
                                                      m_updateArgs.lens, true) == GRAAL_SUCCESS,
                                ret, AMF_UNEXPECTED);

                // Copy data to the new slot, as this channel can be still processed (user doesn't
                // pass Stop flag to Process() method) and we may start doing cross-fading.
                RETURN_IF_FAILED(ret = graalConv->copyResponses(m_copyArgs.updatesCnt,
                                                                (uint*)m_copyArgs.prevVersions,
                                                                (uint*)m_copyArgs.versions,
                                                                (uint*)m_copyArgs.channels));
            }
            break;

            default:
                RETURN_IF_FALSE(false, ret, AMF_NOT_IMPLEMENTED);
        }

        // Submit the result for the Process() method to use next time it runs. Note that if there is
        // no current buffer we don't want to do crossfading and shift the index directly to the USED
        // state.
        if (m_idxFilter != -1)
        {
            if (!m_readyFilterStateIds.Add(0, m_updIdx, 0, 100))
            {
                // Should never happen (unless our queues erroneously have different lengths).
                AMF_ASSERT(false, L"Internal error! Impossible to continue");
                return;
            }
        }
        else {
            m_idxFilter = m_updIdx;
        }

        m_updIdx = -1;
        m_updateFinishedProcessing.SetEvent();

        continue;

ErrorHandling:
        if (!m_freeFilterStateIds.Add(0, m_updIdx))
        {
            AMF_ASSERT(false, L"Internal error! Impossible to continue");
            return;
        }
        m_updateFinishedProcessing.SetEvent();

    } while (!pThread->StopRequested());
}

AMF_RESULT TANConvolutionImpl::VectorComplexMul(float *vA, float *vB, float *out, int count){

    for (int i = 0; i < count; i++){
        int j;
        float ar, ai, br, bi;
        j = (i << 1);
        ar = vA[j];
        ai = vA[j + 1];
        br = vB[j];
        bi = vB[j + 1];
        out[j] = ar*br - ai*bi;
        out[j + 1] = ar*bi + ai*br;
    }

    return AMF_OK;
}

amf_size TANConvolutionImpl::ovlAddProcess(
    void *state, 
    float **inputData, 
    float **outputData, 
    amf_size nSamples,
    amf_uint32 n_channels
)
{
    float **filter = ((ovlAddFilterState *)state)->m_internalFilter;
    float **overlap = ((ovlAddFilterState *)state)->m_internalOverlap;

    // we process in bufSize blocks
    if (nSamples < m_iBufferSizeInSamples)
        return 0;
    // use fixed overlap size:
    nSamples = m_iBufferSizeInSamples;

    // Convert to complex numbers.
    for (amf_uint32 iChan = 0; iChan < n_channels; iChan++){
        // get next block of data, expand into real part of complex values:
        for (int k = 0; k < nSamples; k++){
            m_OutSamples[iChan][2 * k] = inputData[iChan][k];
            m_OutSamples[iChan][2 * k + 1] = 0.0;
        }
        // zero pad:
        ZeroMemory(&m_OutSamples[iChan][2 * nSamples],
                   2 * (m_length - nSamples) * sizeof(m_OutSamples[0][0]));
    }

    AMF_RETURN_IF_FAILED(m_pTanFft->Transform(TAN_FFT_TRANSFORM_DIRECTION_FORWARD, m_log2len, m_iChannels,
                                              m_OutSamples, m_OutSamples));

    for (amf_uint32 iChan = 0; iChan < n_channels; iChan++){
        VectorComplexMul(m_OutSamples[iChan], filter[iChan], m_OutSamples[iChan], m_length);
    }

    AMF_RETURN_IF_FAILED(m_pTanFft->Transform(TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, m_log2len, m_iChannels,
                                              m_OutSamples, m_OutSamples));

    for (amf_uint32 iChan = 0; iChan < n_channels; iChan++){
        for (amf_size id = 0; id < m_length; id++)
        {
            overlap[iChan][id] = m_OutSamples[iChan][id * 2] + 
                                 (id + nSamples >= m_length ? 0 : overlap[iChan][id + nSamples]);
        }
        memcpy(outputData[iChan], overlap[iChan], nSamples * sizeof(float));
    }

    return nSamples;
}

AMF_RESULT TANConvolutionImpl::ProcessInternal(
    int idx, 
    void *state, 
    TANSampleBuffer pInputData,
    TANSampleBuffer pOutputData,
    amf_size nSamples, 
    // Masks of flags from enum
    // TAN_CONVOLUTION_CHANNEL_FLAG.
    const amf_uint32 flagMasks[],
    amf_size *pNumOfSamplesProcessed,
    int ocl_prev_input,
    int ocl_advance_time,
    int ocl_skip_stage
)
{
    //ToDo handle audio buffers in GPU memory:
    if (pInputData.mType != AMF_MEMORY_HOST){
        return AMF_NOT_IMPLEMENTED;
    }

    int ret = GRAAL_SUCCESS;

    if (pNumOfSamplesProcessed)
    {
        *pNumOfSamplesProcessed = 0;
    }

    // update available channel list
    for (amf_uint32 channelId = 0; channelId < static_cast<amf_uint32>(m_MaxChannels); channelId++){
        if (!flagMasks) {
            m_availableChannels[channelId] = false;
            m_tailLeftOver[channelId] = m_length;
        }
        else if (flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT) {
            if (flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM)
            {
                m_availableChannels[channelId] = true;
                m_tailLeftOver[channelId] = 0;
            }
            else if (m_tailLeftOver[channelId] > 0) {
                m_availableChannels[channelId] = false;
                if (ocl_advance_time) {
                    m_tailLeftOver[channelId] -= static_cast<int>(nSamples);
                }
                if (m_tailLeftOver[channelId] <= 0) {
                    // Stopping a stream here we need to flush the convolutor.
                    AMF_RETURN_IF_FAILED(Flush(idx, channelId));
                }
            }
            else {
                m_availableChannels[channelId] = true;
                m_tailLeftOver[channelId] = 0;
            }
        }
        else {
            m_availableChannels[channelId] = false;
            m_tailLeftOver[channelId] = m_length;
        }

        if (flagMasks && flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM) {
            AMF_RETURN_IF_FAILED(Flush(idx, channelId));
        }
    }
    // copy valid channel buffer pointers to internal list:
    int idxInt = 0;
    for (amf_uint32 channelId = 0; channelId < static_cast<amf_uint32>(m_MaxChannels); channelId++){
        if (!m_availableChannels[channelId]) { // !available == running
            m_internalOutBufs.buffer.host[idxInt] = pOutputData.buffer.host[channelId];
            if (flagMasks && flagMasks[channelId] & TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT) {
                m_internalInBufs.buffer.host[idxInt] = m_silence;
            }
            else {
                m_internalInBufs.buffer.host[idxInt] = pInputData.buffer.host[channelId];
            }
            ++idxInt;
        }
    }

    int n_channels = idxInt;
    AMF_RETURN_IF_FALSE(n_channels > 0, AMF_WRONG_STATE, L"No active channels found");

    switch (m_eConvolutionMethod) {
    case TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD:
    {
        float **filter = ((ovlAddFilterState *)state)->m_Filter;
        float **overlap = ((ovlAddFilterState *)state)->m_Overlap;
        float **ifilter = ((ovlAddFilterState *)state)->m_internalFilter;
        float **ioverlap = ((ovlAddFilterState *)state)->m_internalOverlap;
        // copy valid state pointers to internal list:
        for (amf_uint32 channelId = 0, idxInt = 0;
             channelId < static_cast<amf_uint32>(m_MaxChannels); channelId++)
        {
            if (!m_availableChannels[channelId]) { // !available == running
                ifilter[idxInt] = filter[channelId];
                ioverlap[idxInt] = overlap[channelId];
                ++idxInt;
            }
        }

        amf_size numOfSamplesProcessed = 
            ovlAddProcess(state, m_internalInBufs.buffer.host, m_internalOutBufs.buffer.host, static_cast<int>(nSamples),
                          n_channels);
        if (pNumOfSamplesProcessed)
        {
            *pNumOfSamplesProcessed = numOfSamplesProcessed;
        }
        return AMF_OK;
    }

    case TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED:
    case TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL:
    {
        graal::CGraalConv*graalConv = (graal::CGraalConv*)m_graal_conv;
        int idxInt = 0;
        for (amf_uint32 channelId = 0; channelId < static_cast<amf_uint32>(m_MaxChannels);
             channelId++)
        {
            if (!m_availableChannels[channelId]) { // !available == running
                m_s_versions[idxInt] = idx;
                m_s_channels[idxInt] = channelId;
                idxInt++;
            }
        }
        int n_channels = idxInt;
        if (nSamples < m_iBufferSizeInSamples)
        {
            float **inp = (float**)malloc(2 * (sizeof(float*) + m_length * sizeof(float)) * n_channels);
            float **out = (float**)((char*)inp + (sizeof(float*) + m_length * sizeof(float)) * n_channels);
            memset(inp, 0, 2 * (sizeof(float*) + m_length * sizeof(float)) * n_channels);
            for (int n = 0; n < n_channels; n++)
            {
                inp[n] = (float*)(inp + n_channels) + n*m_length;
                out[n] = (float*)(out + n_channels) + n*m_length;
                memcpy(inp[n], m_internalInBufs.buffer.host[n], nSamples * sizeof(float));

            }
            ret = graalConv->process(n_channels,
                m_s_versions,
                m_s_channels,
                inp,
                out,
                ocl_prev_input,
                ocl_advance_time,
                ocl_skip_stage
                );
            if (ret != GRAAL_SUCCESS)
            {
                return AMF_UNEXPECTED;
            }

            for (int n = 0; n < n_channels; n++)
            {
                memcpy(m_internalOutBufs.buffer.host[n], out[n], nSamples * sizeof(float));
            }
            free(inp);

        }
        else
        {
            ret = graalConv->process(n_channels,
                m_s_versions,
                m_s_channels,
                m_internalInBufs.buffer.host,
                m_internalOutBufs.buffer.host,
                ocl_prev_input,
                ocl_advance_time,
                ocl_skip_stage
                );
            if (ret != GRAAL_SUCCESS)
            {
                return AMF_UNEXPECTED;
            }
        }
    }
    break;

    default:
        AMF_RETURN_IF_FALSE(false, AMF_NOT_IMPLEMENTED);
    }

    if (pNumOfSamplesProcessed)
    {
        *pNumOfSamplesProcessed = nSamples;
    }

    return AMF_OK;
}

