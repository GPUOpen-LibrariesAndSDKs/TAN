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


#include "GraalConv.hpp"
#include "amdFHT.h"
#include "GraalCLUtil/GraalCLUtil.hpp"

//#include "OclKernels/GraalFHT.cl.h"
#include "OclKernels/CLKernel_GraalFHT.h"

#include <malloc.h>
#include "public/common/thread.h"
#include "public/common/AMFFactory.h"           //AMF
#include "../common/OCLHelper.h"

#include <CL/cl_ext.h>

#ifndef AMF_RETURN_IF_FALSE
#define AMF_RETURN_IF_FALSE(exp, ret_value, /*optional message,*/ ...)
#endif
#ifndef AMF_RETURN_IF_FAILED
#define AMF_RETURN_IF_FAILED(exp, ...) 
#endif
#ifndef AMF_ASSERT_OK
#define AMF_ASSERT_OK(exp, ... /*optional format, args*/) 
#endif

namespace graal
{

#if _MSC_VER <= 1800
    static double log2(double n)
    {
        // log(n)/log(2) is log2.  
        return log(n) / log(2.);
    }
#endif

CGraalConv:: CGraalConv( void ) 
{
    algorithm_ = ALG_UNI_HEAD_TAIL; // ALG_UNIFORMED;
    n_max_channels_ = 0;
    max_conv_sz_ = 0;
    max_proc_buffer_sz_ = 0;
    n_sets_ = 0;
    n_components_ = 1;
    n_aligned_proc_blocks_ = 0;
    aligned_proc_bufffer_sz_ = 0;
    align_padding_sz_ = 0;
    processing_log2_ = 1;
    aligned_conv_sz_ = 0;
    aligned_processing_sz_ = 0;
    uploadKernel_ = 0;
    uploadKernel2_ = 0;
    resetKernel_ = 0;
    sincos_ = 0;  // precomputeted sincos table
    bit_reverse_ = 0;  // reverse bit table

#ifndef TAN_SDK_EXPORTS
    own_queue_ = false;
    graalQ_  = 0;
    graalTailQ_ = 0;
    eop_event_ = 0;
    eoh_event_ = 0;
#endif

// kernel channel map
    kernel_channels_map_ = NULL;
// kernel set map
    kernel_sets_map_ = NULL;
// kernel lens map
    kernel_lens_map_ = NULL;
// input
    kernel_input_union_ = NULL;


// conter per each stream
    round_counters_ = NULL;
// channels map
    channels_map_ = NULL;
// sets map
    sets_map_ = NULL;
// history, input transformed data	
    history_transformed_ = NULL;
// output data
    process2_output_staging_ = NULL;

    m_headTailKernelEvent = NULL;
    m_pullKernelEvent = NULL;
    FHT_transformCPU_ = 0;
    kernel_staging_.clear();
    kernel_transformed_.clear();
    kernel_trasformed_union_ = 0;
    directTransformKernel_ = 0;
    inverseTransformKernel_ = 0;
    CMADKernels_.clear();
    convHead1_ = 0;

    verify = 0;

    conv_mem_alloc_flags_ =  0; //CL_MEM_ALLOC_HOST_PTR;
    n_upload_qs_ = 1;
    n_input_blocks_ = 2;
    n_accum_blocks_ = 8; // n blocks accumulated witha single CMAD invokation
// total round count
    round_counter_ = 0;

    n_input_qs_ = 0;
    n_stages_ = 0;
    conv_log2_ = 0;
    accum_stride_ = 0;
    state_union_ = 0;
    inputKernel_ = nullptr;
    inputStageKernel_ = nullptr;

    cmad_accum_ = 0;
    cmad_accum_xf_ = 0;
    copy_response_in_offset_ = 0;
    copy_response_out_offset_ = 0;
    host_copy_resp_out_offset = 0;
    host_copy_resp_in_offset = 0;
}


CGraalConv:: ~CGraalConv( void ) 
{
    cleanup();
}



int CGraalConv::initializeConv(
#ifdef TAN_SDK_EXPORTS
    amf::TANContextPtr &pContextTAN,
    amf::AMFComputePtr &pConvolution,
    amf::AMFComputePtr &pUpdate,
#endif
    int _n_max_channels, int _max_conv_sz, int _max_proc_buffer_sz, int _n_sets,
    int _algorithm
#ifndef TAN_SDK_EXPORTS
    ,
    cl_context _clientContext,
    cl_device_id _clientDevice,
    cl_command_queue _clientQ
#endif
)
{
    m_pContextTAN = pContextTAN;

    algorithm_ = (_algorithm == ALG_ANY) ? ALG_UNI_HEAD_TAIL : _algorithm; // ALG_UNIFORMED;
    n_max_channels_ = _n_max_channels;
    max_conv_sz_ = ((_max_conv_sz + 1023) / 1024 ) * 1024;
    max_proc_buffer_sz_ = _max_proc_buffer_sz;
    n_sets_ = _n_sets;
    processing_log2_ = static_cast<int>(ceil(log2((double)max_proc_buffer_sz_)));

    aligned_proc_bufffer_sz_ = (1 << processing_log2_);
    align_padding_sz_ = aligned_proc_bufffer_sz_ - max_proc_buffer_sz_;
    aligned_processing_sz_ = aligned_proc_bufffer_sz_  * 2;

    n_aligned_proc_blocks_ = (max_conv_sz_ + aligned_proc_bufffer_sz_ - 1) / aligned_proc_bufffer_sz_;
    aligned_conv_sz_ = (n_aligned_proc_blocks_ * aligned_processing_sz_ * n_components_);

    conv_log2_ = static_cast<int>(ceil(log2((double)aligned_conv_sz_)));

    int ret = setupCL(pConvolution, pUpdate
#ifndef TAN_SDK_EXPORTS
        _clientContext,
        _clientDevice,
        _clientQ
#endif
        );

    return ret;
}

int
CGraalConv::updateConv(
    int _n_channels,
    const int *_uploadIDs,
    const int *_convIDs,
    const float** _conv_ptrs,
    const int * _conv_lens,
    bool _synchronous)
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    for( int j = 0; j < _n_channels; j++ )
    {
// move data into staging
        int uploadId = _uploadIDs[j];
        int convId = _convIDs[j];

        CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[uploadId][convId];
        ret = stg_buf->unmap();

    }

    ret = uploadConvControlMaps(_n_channels, 
                                _uploadIDs,     // upload set IDs
                                _convIDs,       // kernel IDs
                                _conv_lens);


    ret = updateConvIntnl(_n_channels, 
                        _uploadIDs,     // upload set IDs
                        _convIDs,       // kernel IDs
                        _conv_lens,
                        _synchronous   // synchronoius call			
                        );

    return ret;

}

    /**
     * Upload kernels from arbitrary system pointers.
     * Pointers become invalid after the call.
     * 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

int 
CGraalConv::updateConvHostPtrs(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const float** _conv_ptrs,  // arbitrary host ptrs
        const int * _conv_lens,
        bool _synchronous   // synchronous call	
        )
{
    int ret = GRAAL_SUCCESS;

    ret = uploadConvControlMaps(_n_channels, 
                                _uploadIDs,     // upload set IDs
                                _convIDs,       // kernel IDs
                                _conv_lens);
    if (ret != GRAAL_SUCCESS)
    {
        return ret;
    }

    ret = uploadConvHostPtrIntnl(
                            _n_channels, 
                            _uploadIDs,     // upload set IDs
                            _convIDs,       // kernel IDs
                            _conv_ptrs,  // arbitrary host ptrs
                            _conv_lens
                            );
    if (ret != GRAAL_SUCCESS)
    {
        return ret;
    }

    ret = updateConvIntnl(_n_channels, 
                        _uploadIDs,     // upload set IDs
                        _convIDs,       // kernel IDs
                        _conv_lens,
                        _synchronous   // synchronoius call			
                        );


    return ret;
}

    /**
     * Upload kernels from a previously acquired gpu-friendly system pointers.
     * Pointers become invalid after the call.
     * 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

int
CGraalConv::updateConv(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const cl_mem* _ocl_mems,
        const int * _conv_lens,
        bool _synchronous   // synchronoius call	
        )
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLGeneralQueue();
    if ( NULL == uploadQ)
    {
        return false;
    }
    cl_context Ctxt = NULL;
    clGetCommandQueueInfo(uploadQ, CL_QUEUE_CONTEXT, sizeof(Ctxt), &Ctxt, NULL);
    if ( NULL == Ctxt)
    {
        return false;
    }

    for( int j = 0; j < _n_channels; j++ )
    {
        CABuf<float> stg_buf(Ctxt);

        ret = stg_buf.attach(_ocl_mems[j], _conv_lens[j]);
// do direct transform
        int uploadId = _uploadIDs[j];
        int convId = _convIDs[j];

        CASubBuf<float> *transf_buf = (CASubBuf<float> *)kernel_transformed_[uploadId][convId];


        ret = updateConvOCL (&stg_buf,  transf_buf, _conv_lens[j], uploadQ, uploadId, convId);

    }

    if ( _synchronous ) 
    {
        clFinish(uploadQ);
    }
    return ret;
}

int 
CGraalConv:: updateConv(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const int * _conv_lens,
        bool _synchronous   // synchronoius call			
        )
{
    int ret = GRAAL_SUCCESS;

    ret = uploadConvControlMaps(_n_channels, 
                                _uploadIDs,     // upload set IDs
                                _convIDs,       // kernel IDs
                                _conv_lens);


    ret = updateConvIntnl(_n_channels, 
                        _uploadIDs,     // upload set IDs
                        _convIDs,       // kernel IDs
                        _conv_lens,
                        _synchronous   // synchronoius call			
                        );


    return(ret);
}

    /**
     * All kernels will be ready upon the return from the call
     * 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */


int 
CGraalConv::finishUpdate(void)
{
    int ret = GRAAL_SUCCESS;
    ret = sincUpload();
    return ret;

}


#ifdef TAN_SDK_EXPORTS
AMF_RESULT CGraalConv::copyResponses(
    uint channelsCnt,
    const uint pFromUploadIds[],
    const uint pToUploadIds[],
    const uint pConvIds[],
    const bool synchronous
    )
{
    for (uint channelId = 0; channelId < channelsCnt; channelId++)
    {
        AMF_RETURN_IF_FALSE(pFromUploadIds[channelId] < (uint)n_sets_, AMF_INVALID_ARG,
                            L"pFromUploadIds[channelId] out of range");
        AMF_RETURN_IF_FALSE(pToUploadIds[channelId] < (uint)n_sets_, AMF_INVALID_ARG,
                            L"pToUploadIds[channelId] out of range");
        AMF_RETURN_IF_FALSE(pConvIds[channelId] < (uint)n_max_channels_, AMF_INVALID_ARG,
                            L"pConvIds[channelId] out of range");

        /*const uint inOffset =*/
        host_copy_resp_in_offset[channelId] = (pFromUploadIds[channelId] * n_max_channels_ + pConvIds[channelId]) * aligned_conv_sz_;
        /*const uint outOffset =*/
        host_copy_resp_out_offset[channelId] = (pToUploadIds[channelId] * n_max_channels_ + pConvIds[channelId]) * aligned_conv_sz_;

    }
    if(channelsCnt>=1) {
        CASubBuf<int> *pCopyRespInOffset = static_cast<CASubBuf<int>*>(copy_response_in_offset_);
        CASubBuf<int> *pCopyRespOutOffset = static_cast<CASubBuf<int>*>(copy_response_out_offset_);
        clEnqueueWriteBuffer(this->m_pContextTAN->GetOpenCLGeneralQueue(), pCopyRespInOffset->getCLMem(), CL_FALSE, 0, channelsCnt*sizeof(int), host_copy_resp_in_offset,0, NULL,NULL);
        clEnqueueWriteBuffer(this->m_pContextTAN->GetOpenCLGeneralQueue(), pCopyRespOutOffset->getCLMem(), CL_FALSE, 0, channelsCnt*sizeof(int), host_copy_resp_out_offset, 0, NULL, NULL);
        cl_int ret = CL_SUCCESS;
        amf_size n_arg = 0;
        CASubBuf<float> *pTransfBuf = static_cast<CASubBuf<float>*>(kernel_trasformed_union_);
        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgBufferNative(n_arg++, pTransfBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(cl_mem), &pTransfBuf->getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: pTransfBuf->" )
        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgBufferNative(n_arg++, pTransfBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(cl_mem), &pTransfBuf->getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: pTransfBuf->" )

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, inOffset)); //in offset
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(cl_mem), &pCopyRespInOffset->getCLMem()); //in offset
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: inOffset" )

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, static_cast<amf_int32>(pTransfBuf->getLen()))); //in length
        int in_length = pTransfBuf->getLen();
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), &in_length); //in offset
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: pTransfBuf->getLen()" )

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, outOffset)); //out offset
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(cl_mem), &pCopyRespOutOffset->getCLMem()); //out offset
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: outOffset" )

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, static_cast<amf_int32>(pTransfBuf->getLen())));//out length
        int out_length = pTransfBuf->getLen();
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), &out_length ); //out length
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out length" )

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, aligned_conv_sz_));//block length
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), &aligned_conv_sz_ );//block length
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: aligned_conv_sz_")

        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, 0));//pad length
        cl_int tmp = 0;
        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), &tmp );//pad length
        //ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), 0);//pad length
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: 0")

        ret = clSetKernelArg(m_copyWithPaddingKernel, n_arg++, sizeof(int), &channelsCnt);//channelCount
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: 0")

        size_t l_wk[3] = { size_t(min(aligned_conv_sz_, 256)), 1, 1 };
        size_t g_wk[3] = { size_t(aligned_conv_sz_), channelsCnt, 1 }; //divide by 4 as we process a vec4
        ////////AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->Enqueue((size_t)1, NULL, g_wk, l_wk), L"Enqueue() failed");
        ret = clEnqueueNDRangeKernel(this->m_pContextTAN->GetOpenCLGeneralQueue(), m_copyWithPaddingKernel, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel m_copyWithPaddingKernel failed: " )
    }

    if (synchronous)
    {
        clFinish(this->m_pContextTAN->GetOpenCLGeneralQueue());
        ////////AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
    }

    return AMF_OK;
}
#endif // TAN_SDK_EXPORTS


int
CGraalConv:: getConvBuffers(int _n_channels, int *_uploadIDs, int *_convIDs, float** _conv_ptrs)
{
    int ret = GRAAL_SUCCESS;
    int i = 0;

    cl_command_queue graalQ_ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    // first asynchronous
    for( i = _n_channels - 1; i >= 0; i-- )
    {
        CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[_uploadIDs[i]][_convIDs[i]];
        
        _conv_ptrs[i] = stg_buf->mapA(graalQ_, CL_MAP_WRITE_INVALIDATE_REGION);
    }

    return(ret);
}


int
CGraalConv:: getConvBuffers(
        int _n_channels,
        int * _uploadIDs,             // upload set IDs per kernel
        int * _convIDs,               // kernel IDs
        cl_mem* _ocl_mems           // gpu-frendly system pointers
        )
{
    int ret = GRAAL_SUCCESS;

    for(int i = 0; i < _n_channels; i++) 
    {
        CABuf<float> &stg_buf = *(CABuf<float> *)kernel_staging_[_uploadIDs[i]][_convIDs[i]];
        _ocl_mems[i] = stg_buf.getCLMem();
    }

    return(ret);
}

#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
bool CGraalConv::checkForContiguousBuffers(
    int count,
    const float** _conv_ptrs,  
    const int * _conv_lens
    ) {
   if (count <= 1) //  don't need to do anything
        return false;
    bool contiguous = true;
    const float *start, *end;
    start = _conv_ptrs[0];
    for (int i = 1; i < count; i++){
        end = start + _conv_lens[i-1];
        start = _conv_ptrs[i];
        if (end != start){
            contiguous = false;
            break;
        }
    }
    return contiguous;
}
#endif

int 
CGraalConv::uploadConvHostPtrs(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const float** _conv_ptrs,  // arbitrary host ptrs
        const int * _conv_lens,
        bool _synchronous   // synchronous call	
        )
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    // If response buffers happen to be allocated contiguously, we can improve copy performance
    // by using a single copy.
#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
    if (checkForContiguousBuffers(_n_channels, _conv_ptrs, _conv_lens))
    {
        CABuf<float> *stg_bufAll = (CABuf<float> *)kernel_input_store_[_uploadIDs[0]]; //UploadIDs [index of current Set ]

        const float * conv_ptr = _conv_ptrs[0];
        int len = _conv_lens[0] * _n_channels;
        ret = stg_bufAll->copyToDeviceNonBlocking(uploadQ, CL_MEM_READ_ONLY, conv_ptr, len);

        CHECK_OPENCL_ERROR(ret, "upload failed.");
    }
    else 
#endif
    {

        for (int j = 0; j < _n_channels; j++)
        {
            // move data into staging
            CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[_uploadIDs[j]][_convIDs[j]];

            ret = stg_buf->copyToDeviceNonBlocking(uploadQ, CL_MEM_READ_ONLY, _conv_ptrs[j],
                _conv_lens[j]);
            CHECK_OPENCL_ERROR(ret, "upload failed.");
        }
    }

    if ( _synchronous ) 
    {
        ret = sincUpload( );
    }
    return ret;
}


int
CGraalConv::uploadConvGpuPtrs(
int _n_channels,
const int *_uploadIDs,     // upload set IDs
const int *_convIDs,       // kernel IDs
const cl_mem * _conv_ptrs,  // arbitrary cl_mem ptrs
const int * _conv_lens,
bool _synchronous   // synchronous call	
)
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    for (int j = 0; j < _n_channels; j++)
    {
        // move data into staging
        CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[_uploadIDs[j]][_convIDs[j]];

        ret = stg_buf->copyCLmem(_conv_ptrs[j], uploadQ, _conv_lens[j]*sizeof(float));

        CHECK_OPENCL_ERROR(ret, "upload failed.");
        }

    if (_synchronous)
    {
        ret = sincUpload();
    }
    return ret;
}


int
CGraalConv::process(
        int _n_channels, 
        const int *_uploadID,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        float** _inputs,
        float** _outputs,
        int _prev_input,
        int _advance_time,
        int _skip_stage,
        int _crossfade_state
        )
{
    int ret = GRAAL_SUCCESS;
    GraalSampleBuffer outBuf;
    outBuf.buffer.host = _outputs;
    outBuf.mType = GRAAL_MEMORY_HOST;
    ret = processIntrnl(
            _n_channels,
            _uploadID,     // upload set IDs
            _convIDs,       // kernel IDs
            _inputs,
            outBuf,
            _prev_input,
            _advance_time,
            _skip_stage,
            _crossfade_state
            );
    return ret;
}

int 
CGraalConv::process(
		int _n_channels,
		const int *_uploadID,     // upload set IDs
		const int *_convIDs,       // kernel IDs
		float** _inputs,
		cl_mem* _output,
		int _prev_input ,
		int _advance_time ,
		int _skip_stage,
        int _crossfade_state
		)
{
    int ret = GRAAL_SUCCESS;
    GraalSampleBuffer outBuf;
    outBuf.buffer.clmem = _output;
    outBuf.mType = GRAAL_MEMORY_OPENCL;
    ret = processIntrnl(
        _n_channels,
        _uploadID,     // upload set IDs
        _convIDs,       // kernel IDs
        _inputs,
        outBuf,
        _prev_input,
        _advance_time,
        _skip_stage,
        _crossfade_state
    );
    return ret;
}

int CGraalConv::flush(amf_uint channelId, const bool synchronous)
{
    AMF_RETURN_IF_FALSE(channelId < static_cast<amf_uint>(n_max_channels_), AMF_INVALID_ARG,
                        L"channelId out of range");

    int ret = GRAAL_SUCCESS;

    switch (algorithm_)
    {
        case ALG_UNIFORMED:
        case ALG_UNI_HEAD_TAIL:
        {
            CABuf<float> *pHistoryBuf = static_cast<CABuf<float>*>(history_transformed_);
            CABuf<float> *pInputBuf = &m_process_input_staging_;
            CABuf<float> *pAccumBuf = static_cast<CABuf<float>*>(cmad_accum_);
            CABuf<float> *pAccumBufxf = static_cast<CABuf<float>*>(cmad_accum_xf_);
            AMF_RETURN_IF_FAILED(zeroMemory(pHistoryBuf, channelId * aligned_conv_sz_,
                                            aligned_conv_sz_));
            AMF_RETURN_IF_FAILED(zeroMemory(pInputBuf,
                                            channelId * n_input_blocks_ * aligned_proc_bufffer_sz_,
                                            n_input_blocks_ * aligned_proc_bufffer_sz_));
            for (uint setId = 0; setId < static_cast<uint>(n_sets_); setId++) {
                AMF_RETURN_IF_FAILED(zeroMemory(pAccumBuf,
                                                setId * n_max_channels_ * accum_stride_ +
                                                channelId * accum_stride_,
                                                accum_stride_));
                AMF_RETURN_IF_FAILED(zeroMemory(pAccumBufxf,
                                                setId * n_max_channels_ * accum_stride_ +
                                                channelId * accum_stride_,
                                                accum_stride_));
            }
        }
        break;

        default:
            AMF_RETURN_IF_FAILED(AMF_NOT_IMPLEMENTED);
    }

    if (synchronous)
    {
        clFinish(this->m_pContextTAN->GetOpenCLGeneralQueue());
        ////////AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
    }

    return ret;
}

    /**
     * Upload kernels from a previously acquired gpu-friendly system pointers.
     * Pointers become invalid after the call.
     * 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

int 
CGraalConv::getDevInputPtrs(
        int _n_channels, 
        int _uploadID,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        float** _inputs
        )
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    CABuf<float> &inp_buf = *(CABuf<float> *)host_input_staging_[_uploadID];
    float * inp_buf_ptr = inp_buf.map(inQ, CL_MAP_WRITE_INVALIDATE_REGION);

    for (int c = 0; c < _n_channels; c++ ) 
    {
        int convId = _convIDs[c];
        _inputs[c] = inp_buf_ptr + convId * aligned_proc_bufffer_sz_;
    }

    return(ret);
}

    /**
     * Upload kernels from a previously acquired gpu-friendly system pointers.
     * Pointers become invalid after the call.
     * 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

int 
CGraalConv::processDevPtrs(
        int _n_channels, 
        int _uploadID,     // upload set ID
        const int *_convIDs,       // kernel IDs
        float** _inputs,
        float** _outputs
        )
{
    int ret = GRAAL_NOT_IMPLEMENTED;

#ifdef _DEBUG_PRINTF
    printf("NOT IMPLEMENTED\n");
#endif
    AMF_ASSERT_OK(AMF_NOT_IMPLEMENTED, L"CGraalConv::processDevPtrs not implemented");

    return(ret);
}




/*---------------------------------------------------------------------------------------------
Internals
-----------------------------------------------------------------------------------------------*/




/**
* Upload kernels from a previously acquired gpu-friendly system pointers.
* Pointers become invalid after the call.
*
* @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
*/

int
CGraalConv::processIntrnl(
            int _n_channels,
            const int *_uploadIDs,     // upload set IDs
            const int *_convIDs,       // kernel IDs
            float** _inputs,
            GraalSampleBuffer& _output,
            int _prev_input,
            int _advance_time,
            int _skip_stage,
            int _crossfade_state
)
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLConvQueue();

    {
        // Copying the control and input data needed for the convolution engine, 
        // Non-blocking calls has replaced the previous blocking mapping.

        // upload channel map
        CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
        // upload set map
        CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
        // upload input data
        CABuf<float> &inp_buf = m_process_input_staging_;

        if (!_prev_input && _skip_stage != 1)
        {
            // Copying the channel mapping and the input data only if an output is going to be gnerated
            chnl_map_buf.copyToDeviceNonBlocking(inQ, 0, _convIDs, _n_channels);
            for (int i = 0; i < _n_channels; i++)
            {
                int convId = _convIDs[i];
                int input_index = getRoundCounter(0, convId) % n_input_blocks_;
                float *in_ptr = _inputs[i];
                inp_buf.copyToDeviceNonBlocking(inQ, 0, in_ptr, max_proc_buffer_sz_, (convId * n_input_blocks_ + input_index) * aligned_proc_bufffer_sz_);
                if(align_padding_sz_!= 0) inp_buf.setValue2(inQ, 0, align_padding_sz_, max_proc_buffer_sz_);
            }
        }
        // Copying the IR index
        set_map_buf.copyToDeviceNonBlocking(inQ, 0, _uploadIDs, _n_channels);
    }


    CABuf<float> & out_buf = *(CABuf<float>*)process2_output_staging_;
    float * d_out_ptr;

    cl_command_queue outQ = this->m_pContextTAN->GetOpenCLConvQueue();
    cl_command_queue generalQ = this->m_pContextTAN->GetOpenCLGeneralQueue();
    switch (algorithm_)
    {
    case ALG_UNIFORMED:
    default:



            // push data into the pipeline
        processPush(
            _n_channels,
            _uploadIDs,     // upload set IDs
            _convIDs,       // kernel IDs
            _prev_input
            );



        // accumulate CMAD
        processAccum(_n_channels);

        // push data out of pipeline
        processPull(
            _n_channels,
            _uploadIDs,     // upload set IDs
            _convIDs,       // kernel IDs
            _advance_time
            );



        // move data upstream
        if (_output.mType == GRAAL_MEMORY_HOST)
        {
            for (int c = 0; c < _n_channels; c++)
            {
                int uploadId = _uploadIDs[c];
                int convId = _convIDs[c];
                int status = clEnqueueReadBuffer(
                    outQ,
                    out_buf.getCLMem(),
                    (c == (_n_channels - 1)) ? CL_TRUE : CL_FALSE,
                    (uploadId * n_max_channels_ + convId) * aligned_proc_bufffer_sz_ * sizeof(float),
                    max_proc_buffer_sz_ * sizeof(float),
                    _output.buffer.host[c],
                    0,
                    NULL,
                    NULL);
                CHECK_OPENCL_ERROR(status, "copy failed.");
            }
        }
        else if (_output.mType == GRAAL_MEMORY_OPENCL)
        {
            for (int c = 0; c < _n_channels; c++)
            {
                int uploadId = _uploadIDs[c];
                int convId = _convIDs[c];
                int status = clEnqueueCopyBuffer(
                    generalQ,
                    out_buf.getCLMem(),
                    _output.buffer.clmem[c],
                    (uploadId * n_max_channels_ + convId) * aligned_proc_bufffer_sz_* sizeof(float),
                    0,
                    max_proc_buffer_sz_ * sizeof(float),
                    (c == 0) ? 1 : 0,
                    (c == 0) ? &m_pullKernelEvent : NULL,//Set the event for the first copy command only, the rest will line up since all in the same queue
                    NULL);
                CHECK_OPENCL_ERROR(status, "copy failed.");
            }
        }


        break;
    case ALG_UNI_HEAD_TAIL:
        
        if (_skip_stage != 1)
        {

        // direct FHT, MAD + taiMAD, inverse FHT, advance time

            processHead1(
                _n_channels,
                _uploadIDs,     // upload set IDs
                _convIDs,       // kernel IDs
                _prev_input,	// use previous input data
                _advance_time, // update counters: 1, not: 0
                (_crossfade_state == 2)
            );


        // move data upstream
            if (_output.mType == GRAAL_MEMORY_HOST)
            {
                for (int c = 0; c < _n_channels; c++)
                {
                    int uploadId = _uploadIDs[c];
                    int convId = _convIDs[c];
                    int status = clEnqueueReadBuffer(
                        outQ,
                        out_buf.getCLMem(),
                        (c == (_n_channels-1)) ? CL_TRUE: CL_FALSE,
                        (uploadId * n_max_channels_ + convId) * aligned_proc_bufffer_sz_ * sizeof(float),
                        max_proc_buffer_sz_ * sizeof(float),
                        _output.buffer.host[c],
                        0,
                        NULL,
                        NULL);
                    CHECK_OPENCL_ERROR(status, "copy failed.");
                }
            } 
            else if (_output.mType == GRAAL_MEMORY_OPENCL)
            {
                for (int c = 0; c < _n_channels; c++)
                {
                    int convId = _convIDs[c];
                    int uploadId = _uploadIDs[c];
                    int status = clEnqueueCopyBuffer(
                        generalQ,
                        out_buf.getCLMem(),
                        _output.buffer.clmem[c],
                        (uploadId * n_max_channels_ + convId) * aligned_proc_bufffer_sz_* sizeof(float),
                        0,
                        max_proc_buffer_sz_ * sizeof(float),
                        (c == 0) ? 1 : 0,
                        (c == 0) ? &m_headTailKernelEvent:NULL,//Set the event for the first copy command only, the rest will line up since all in the same queue
                        NULL);
                    CHECK_OPENCL_ERROR(status, "copy failed.");
                }
            }
        }


        if (_skip_stage != 2)
        {
            // accumulate the tail CMAD
            // for the next time step
            int str_shift = -1;
            processAccum(_n_channels, 1, str_shift, (_crossfade_state == 1));
        }
#if 0
        // update counters
        resetConvState(
            _n_channels,
            _uploadIDs,
            _convIDs,
            1);
#endif
        //	exit(0);
        break;
    }

    // at this point GPU-side counters has been updated already


    if (_advance_time)
    {
        // update CPU-side counters
        for (int i = 0; i < _n_channels; i++)
        {
            int convId = _convIDs[i];

            incRoundCounter(0, convId);

        }
        incRoundCounter();
    }

    return(ret);
}



int 
CGraalConv:: uploadConvControlMaps(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const int * _conv_lens
        )
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

// upload channel map
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)kernel_channels_map_;
// upload set map
    CABuf<int> &set_map_buf = *(CABuf<int> *)kernel_sets_map_;
// upload input data
    CABuf<int> &len_map_buf = *(CABuf<int> *)kernel_lens_map_;
// update only sent blocks area
    chnl_map_buf.copyToDeviceNonBlocking(inQ, 0, _convIDs, _n_channels, 0);
    set_map_buf.copyToDeviceNonBlocking(inQ, 0, _uploadIDs, _n_channels, 0);
    for (int i = 0; i < _n_channels; i++)
    {
        int convId = _convIDs[i];
        int uploadId = _uploadIDs[i];
        len_map_buf.copyToDeviceNonBlocking(inQ, 0, _conv_lens + i, 1, uploadId * n_max_channels_ + convId);
    }

    return(ret);
}


int 
CGraalConv:: updateConvIntnl(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const int * _conv_lens,
        bool _synchronous   // synchronoius call			
        )
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

// upload channel map
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)kernel_channels_map_;
// upload set map
    CABuf<int> &set_map_buf = *(CABuf<int> *)kernel_sets_map_;
// upload input data
    CABuf<int> &len_map_buf = *(CABuf<int> *)kernel_lens_map_;

    CABuf<float> & stg_buf = *(CABuf<float>*)kernel_input_union_;
    CASubBuf<float> & transf_buf = *(CASubBuf<float> *)kernel_trasformed_union_;
    CABuf<float> & sincos = *(CABuf<float> *)sincos_;
    CABuf<short> & bit_reverse = *(CABuf<short> * )bit_reverse_;
    CABuf<int> & round_counters = *(CABuf<int>*)round_counters_;
    CABuf<float> & data_hist = *(CABuf<float>*)history_transformed_;


    uint in_version_stride = n_max_channels_ * max_conv_sz_;
    uint in_chnl_stride = max_conv_sz_;
    uint out_version_stride = n_max_channels_ * aligned_conv_sz_;
    uint out_chnl_stride = aligned_conv_sz_;
    uint version_stride = n_max_channels_;
    uint data_version_stride = n_max_channels_ * aligned_conv_sz_;
    uint data_channel_stride = aligned_conv_sz_;


    size_t l_wk[3] = { min(aligned_processing_sz_ >> 1, size_t(256)), (size_t)1, (size_t)1 };

    size_t g_wk[3] = {1,1,1};

    g_wk[0] = n_aligned_proc_blocks_* l_wk[0];
    g_wk[1] = _n_channels;
// run direct FHT for the 1st stream
    int n_arg = 0;
// direct FHT

#ifdef TAN_SDK_EXPORTS
    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, stg_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &stg_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: stg_buf" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, transf_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &transf_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: transf_buf" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, bit_reverse.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: bit_reverse" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, sincos.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sincos" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: set_map_buf" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, len_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &len_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: len_map_buf" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, round_counters.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &round_counters.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: round_counters" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgBufferNative(n_arg++, data_hist.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &data_hist.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: data_hist" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, in_version_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &in_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_version_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, in_chnl_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &in_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, out_version_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &out_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_version_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, out_chnl_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &out_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, version_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: version_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, data_version_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &data_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: data_version_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->SetArgInt32(n_arg++, data_channel_stride));
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &data_channel_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: data_channel_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel_->Enqueue(2, NULL, g_wk, l_wk), L"Enqueue() failed");
    ret = clEnqueueNDRangeKernel(inQ, uploadKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel uploadKernel_ failed" )

    if (_synchronous) {
        clFinish(inQ);
        ////////AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
    }
#else
    ret = clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &stg_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &transf_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &len_map_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &round_counters.getCLMem());
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(cl_mem), &data_hist.getCLMem());

    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &in_version_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &in_chnl_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &out_version_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &out_chnl_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &version_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &data_version_stride);
    ret |= clSetKernelArg(uploadKernel_, n_arg++, sizeof(int), &data_channel_stride);

    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    ret = clEnqueueNDRangeKernel(inQ, uploadKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);

    CHECK_OPENCL_ERROR(ret, "kernel direct transform  failed.");
    if ( _synchronous )
    {
        clFinish(inQ);
    }
#endif

    if ( verify  ==1 || verify == 3) 
    {
        transf_buf.copyToHost(inQ);
        stg_buf.copyToHost(inQ);
        size_t len = aligned_conv_sz_; 
        float *ext_stg = new float[len];
        float * src_ptr = stg_buf.getSysMem();
        float * tgt_ptr = transf_buf.getSysMem();
        float * src_buf_ptr, * ext_buf_ptr, *tgt_buf_ptr;
        for(int j = 0; j < _n_channels; j++)
        {
            int n_test_loops = (_conv_lens[j] + aligned_proc_bufffer_sz_ - 1)/aligned_proc_bufffer_sz_;
            int err = -1; //GRAAL_SUCCESS;
            int convId = _convIDs[j];
            int uploadId = _uploadIDs[j];


            src_buf_ptr = src_ptr + (uploadId * n_max_channels_ + convId) * in_chnl_stride;
            ext_buf_ptr = ext_stg; 
            tgt_buf_ptr = tgt_ptr + (uploadId * n_max_channels_ + convId) * out_chnl_stride;
            memset(ext_stg, 0, len * sizeof(float));

            // src moves every block, second half padded with 0s
            for( int i = 0, k = 0, l = 0; i < n_test_loops; i++, l +=aligned_processing_sz_, k+=aligned_proc_bufffer_sz_)
            {
                for ( int m = 0; m < aligned_proc_bufffer_sz_ && k + m < _conv_lens[j]; m++) 
                {
                    ext_buf_ptr[l + m] = src_buf_ptr[k + m];
                }
            }
 
            for (int i = 0; i < n_test_loops; i++)
            {
                err = FHT_verify((const __FLOAT__ *)ext_buf_ptr + aligned_processing_sz_ *i, (const __FLOAT__ *)tgt_buf_ptr + aligned_processing_sz_ *i,
                    aligned_processing_sz_, 0, aligned_processing_sz_, (__FLOAT__)1. );
                if ( err >= 0 ) {
#ifdef _DEBUG_PRINTF
                    printf("Kernel update mismatch: at block %d\n", i);
#endif
                    AMF_ASSERT_OK(AMF_UNEXPECTED, L"Kernel update mismatch: at block %d\n", i);
                    break;
                }

            }

#ifdef _DEBUG_PRINTF
#ifdef TAN_SDK_EXPORTS
            AMF_ASSERT(err == -1, L"Kernel update verified: u=%d c=%d len=%d",
                uploadId, convId, _conv_lens[j]);
#else
            if ( err == -1 )  {
                std::cout << "Kernel update verified: u=" << uploadId << " c=" << convId << " len=" <<_conv_lens[j] << "\n";
            }
#endif
#endif
        }

        delete [] ext_stg;
    }




    return(ret);
}



int
CGraalConv:: resetConvState(
            size_t _n_channels,
            const int *_uploadIDs,
            const int *_convIDs,
            int _time_step
            )
{
    int ret = GRAAL_SUCCESS;
// uload maps

    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLConvQueue();

// upload channel map
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)kernel_channels_map_;

    CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;

    
    uint data_version_stride = aligned_conv_sz_ * n_max_channels_;
    uint data_channel_stride = aligned_conv_sz_;
    uint version_stride = n_max_channels_;


    int n_arg = 0;

#ifdef TAN_SDK_EXPORTS
    ////////AMF_RETURN_IF_FAILED(resetKernel_->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(resetKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf." )
    ////////AMF_RETURN_IF_FAILED(resetKernel_->SetArgBufferNative(n_arg++, count_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(resetKernel_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: count_buf." )
    ////////AMF_RETURN_IF_FAILED(resetKernel_->SetArgInt32(n_arg++, _time_step));
        ret = clSetKernelArg(resetKernel_, n_arg++, sizeof(int), &_time_step);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: _time_step" )

    size_t l_wk[3] = {256,1,1};
    size_t g_wk[3] = { _n_channels, 1, 1 };
    ////////AMF_RETURN_IF_FAILED(resetKernel_->Enqueue(1, NULL, g_wk, l_wk), L"Enqueue() failed");
        ret = clEnqueueNDRangeKernel(uploadQ, resetKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel resetKernel_ failed: " )
#else
    ret = clSetKernelArg(resetKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(resetKernel_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    ret |= clSetKernelArg(resetKernel_, n_arg++, sizeof(int), &_time_step);
    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    size_t l_wk[3] = {256,1,1};

    size_t g_wk[3] = { _n_channels, 1, 1 };

    ret = clEnqueueNDRangeKernel(uploadQ, resetKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);

    CHECK_OPENCL_ERROR(ret, "kernel direct transform  failed.");
#endif

    return(ret);
}

int 
CGraalConv:: getRoundCounter(int _uploadId, int _chnl_id)
{
        int ret = (int)round_counter_;
        if ( _uploadId > -1 ) 
        {
            CABuf<uint> &count_buf = *(CABuf<uint>*)round_counters_;
            ret = count_buf.getSysMem()[_uploadId * n_max_channels_ + _chnl_id];
        }
        return(ret);
}

void 
CGraalConv:: incRoundCounter(int _uploadId, int _chnl_id)
{
        if ( _uploadId > -1 ) 
        {
            CABuf<uint> &count_buf = *(CABuf<uint>*)round_counters_;
            uint curr_round = count_buf.getSysMem()[_uploadId * n_max_channels_ + _chnl_id];
            curr_round++;
            count_buf.getSysMem()[_uploadId * n_max_channels_ + _chnl_id] = curr_round;

        }
        else
        {
            round_counter_++;
        }
}

/*---------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------*/
int
CGraalConv::processHead1(
    int _n_channels,
    const int *_uploadIDs,     // upload set IDs
    const int *_convIDs,       // kernel IDs
    int _prev_input,           // uses the dat inserted into the pipeline on the previous round
    int _advance_time,         // advance time pointer (counter) - 1, not advance - 0
    bool _use_xf_buff
)
{
    int ret = GRAAL_SUCCESS;

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLConvQueue();

    CABuf<float> &inp_buf = m_process_input_staging_;
    CABuf<float> &sincos = *(CABuf<float> *)sincos_;
    CABuf<short> &bit_reverse = *(CABuf<short> *)bit_reverse_;
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
    CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
    CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;
    CABuf<float> &kern_store_buf = *(CABuf<float> *)kernel_trasformed_union_;
    CABuf<float> &data_store_buf = *(CABuf<float> *)history_transformed_;
    CABuf<float> &accum_buf = _use_xf_buff ? *(CABuf<float>*)cmad_accum_xf_:*(CABuf<float>*)cmad_accum_;
    CABuf<float> &out_buf = *(CABuf<float>*)process2_output_staging_;



    uint n_in_blocks = n_input_blocks_;   // # of blocks kept in input staging
    uint n_conv_blocks = n_aligned_proc_blocks_;  // # of conv blocks (total)
    float scale = (float)0.5 / (float)aligned_processing_sz_;        // inverse conv scale
    uint in_version_stride = aligned_proc_bufffer_sz_ * n_max_channels_ * n_input_blocks_;
    uint in_chnl_stride = aligned_proc_bufffer_sz_* n_input_blocks_;
    uint hist_version_stride = aligned_conv_sz_ * n_max_channels_;
    uint hist_chnl_stride = aligned_conv_sz_;
    uint IR_version_stride = aligned_conv_sz_ * n_max_channels_;
    uint IR_chnl_stride = aligned_conv_sz_;
    uint accum_version_stride = accum_stride_ * n_max_channels_;
    uint accum_chnl_stride = accum_stride_;
    uint counter_version_stride = n_max_channels_;
    uint out_version_stride = aligned_proc_bufffer_sz_ * n_max_channels_;
    uint out_chnl_stride = aligned_proc_bufffer_sz_;

#ifdef TAN_SDK_EXPORTS
    int n_arg = 0;

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, inp_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &inp_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: inp_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, kern_store_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &kern_store_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: kern_store_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, accum_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, data_store_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &data_store_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: data_store_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, out_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &out_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, bit_reverse.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: bit_reverse" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, sincos.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sincos" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, n_in_blocks));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &n_in_blocks);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: n_in_blocks" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, n_conv_blocks));  // # of conv blocks (total)
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &n_conv_blocks);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: n_conv_blocks" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgFloat(n_arg++, scale));        // inverse conv scale
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(float), &scale);        // inverse conv scale
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: scale" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, _prev_input));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &_prev_input);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: _prev_input" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, _advance_time));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &_advance_time);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: _advance_time" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, in_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &in_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, in_chnl_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &in_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, hist_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &hist_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: hist_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, hist_chnl_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &hist_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: hist_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, IR_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &IR_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: IR_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, IR_chnl_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &IR_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: IR_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, accum_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &accum_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, accum_chnl_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &accum_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, counter_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &counter_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: counter_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, out_version_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &out_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_version_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgInt32(n_arg++, out_chnl_stride));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(int), &out_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: set_map_buf" )

    ////////AMF_RETURN_IF_FAILED(convHead1_->SetArgBufferNative(n_arg++, count_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(convHead1_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: count_buf" )


    // direct ransform
    size_t l_wk[3] = { (size_t)min(aligned_processing_sz_ / 2, 256), (size_t)1, (size_t)1 };
    size_t g_wk[3] = { 1, 1, 1 };
    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;

    ////////AMF_RETURN_IF_FAILED(convHead1_->Enqueue(2, NULL, g_wk, l_wk), L"Enqueue() failed");
    if (m_headTailKernelEvent) { clReleaseEvent(m_headTailKernelEvent); m_headTailKernelEvent = NULL; }
    ret = clEnqueueNDRangeKernel(inQ, convHead1_, 2, NULL, g_wk, l_wk, 0, NULL, &m_headTailKernelEvent);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel convHead1_ failed: " )
#else
    cl_kernel convHead = convHead1_;

    int n_arg = 0;

    ret = clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &inp_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &kern_store_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &data_store_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &out_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &n_in_blocks);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &n_conv_blocks);  // # of conv blocks (total)
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(float), &scale);        // inverse conv scale
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &_prev_input);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &_advance_time);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &in_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &in_chnl_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &hist_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &hist_chnl_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &IR_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &IR_chnl_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &accum_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &accum_chnl_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &counter_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &out_version_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(int), &out_chnl_stride);
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    ret |= clSetKernelArg(convHead, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());

    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    // direct ransform


    size_t l_wk[3] = { min(aligned_processing_sz_ / 2, 256), 1, 1 };

    size_t g_wk[3] = { 1, 1, 1 };

    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;

    // THIS IS NOT implemented: running on the same Q
    int n_wait_events = 0;
    cl_event * p_wait_event = NULL;
    cl_event * p_set_event = NULL;
    if (eop_event_ != NULL)
    {
        n_wait_events = 1;
        p_wait_event = &eop_event_;
    }

    eoh_event_ = NULL;
    p_set_event = NULL; // &eoh_event_;


    ret = clEnqueueNDRangeKernel(inQ, convHead, 2, NULL, g_wk, l_wk, n_wait_events, p_wait_event, p_set_event);

    CHECK_OPENCL_ERROR(ret, "kernel one shot convolution  failed.");

    if (eop_event_ != NULL)
    {
        clReleaseEvent(eop_event_);
        eop_event_ = 0;
    }
#endif

    return(ret);
}


/*---------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------*/

int 
CGraalConv::processPush(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        int _prev_input
        )
{
    int ret = GRAAL_SUCCESS;
// if previous input is uaed do nothing
    if (_prev_input)
    {
        return(ret);
    }

    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLConvQueue();

    CABuf<float> &inp_buf = m_process_input_staging_;
    CABuf<float> &dir_fht_buf = *(CABuf<float> *)history_transformed_;
    CABuf<float> &sincos = *(CABuf<float> *)sincos_;
    CABuf<short> &bit_reverse = *(CABuf<short> * )bit_reverse_;
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
    CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
    CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;


    int in_version_stride = aligned_proc_bufffer_sz_ * n_max_channels_ * n_input_blocks_;
    int in_chnl_stride = aligned_proc_bufffer_sz_* n_input_blocks_;
    int out_version_stride = aligned_conv_sz_ * n_max_channels_;
    int out_chnl_stride = aligned_conv_sz_;
    int version_stride = n_max_channels_;



// run direct FHT for the 1st stream
    int n_arg = 0;
    // direct FHT

#ifdef TAN_SDK_EXPORTS
    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, inp_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &inp_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: inp_buf" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, dir_fht_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &dir_fht_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: dir_fht_buf" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, bit_reverse.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: bit_reverse" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, sincos.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sincos" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, n_input_blocks_));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &n_input_blocks_);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: n_input_blocks_" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, in_version_stride));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &in_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_version_stride" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, in_chnl_stride));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &in_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, out_version_stride));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &out_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_version_stride" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, out_chnl_stride));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &out_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, n_aligned_proc_blocks_));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &n_aligned_proc_blocks_);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: n_aligned_proc_blocks_" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, version_stride));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: version_stride" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: set_map_buf" )

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgBufferNative(n_arg++, count_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: count_buf" )

    // TEST::
    int input_index = getRoundCounter();
    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->SetArgInt32(n_arg++, input_index));
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &input_index);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: input_index" )

    // direct ransform
    size_t l_wk[3] = { size_t(min(aligned_processing_sz_ / 2, 256)), size_t(1), size_t(1) };
    size_t g_wk[3] = {1,1,1};
    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;

    ////////AMF_RETURN_IF_FAILED(directTransformKernel_->Enqueue(2, NULL, g_wk, l_wk), L"Enqueue() failed");
    ret = clEnqueueNDRangeKernel(inQ, directTransformKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel directTransformKernel_ failed: " )
#else
    ret = clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &inp_buf.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &dir_fht_buf.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &n_input_blocks_);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &in_version_stride);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &in_chnl_stride);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &out_version_stride);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &out_chnl_stride);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &n_aligned_proc_blocks_);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &version_stride);
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
// TEST::
    int input_index = getRoundCounter();
    ret |= clSetKernelArg(directTransformKernel_, n_arg++, sizeof(int), &input_index);

    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

        // direct ransform

    size_t l_wk[3] = { min(aligned_processing_sz_ / 2, 256), 1, 1 };

    size_t g_wk[3] = {1,1,1};

    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;



    ret = clEnqueueNDRangeKernel(inQ, directTransformKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);

    CHECK_OPENCL_ERROR(ret, "kernel direct transform  failed.");
#endif

    if ( verify > 1)
    {
#ifndef TAN_SDK_EXPORTS
        cl_command_queue inQ =  graalQ_;
#endif
        CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;
        CABuf<float> &inp_buf = m_process_input_staging_;
        CABuf<float> &dir_fht_buf = *(CABuf<float> *)history_transformed_;
        inp_buf.copyToHost(inQ);
        dir_fht_buf.copyToHost(inQ);
        float * inp_buf_ptr = inp_buf.getSysMem();
        float * tgt_ptr = dir_fht_buf.getSysMem();
        float * in_b = new float[aligned_processing_sz_];
        int in_version_stride = aligned_proc_bufffer_sz_ * n_max_channels_ * n_input_blocks_;
        int in_chnl_stride = aligned_proc_bufffer_sz_* n_input_blocks_;
        int out_version_stride = aligned_conv_sz_ * n_max_channels_;
        int out_chnl_stride = aligned_conv_sz_;
        int err = -1; //GRAAL_SUCCESS;
        for(int i = 0; i < _n_channels; i++ )
        {
        // current + previous input
            int convId = _convIDs[i];
            int  uploadId = _uploadIDs[i];
            int input_index_curr = getRoundCounter(0, convId) % n_input_blocks_;
            int input_index_prev = std::abs(getRoundCounter(0, convId)-1) % n_input_blocks_;

            float *in_proc_ptr = inp_buf_ptr + uploadId * in_version_stride + convId  * in_chnl_stride + input_index_curr * aligned_proc_bufffer_sz_;
            memcpy(in_b, in_proc_ptr, aligned_proc_bufffer_sz_ * sizeof(float));
            in_proc_ptr = inp_buf_ptr + uploadId * in_version_stride + convId  * in_chnl_stride + input_index_prev * aligned_proc_bufffer_sz_;
            memcpy(in_b + aligned_proc_bufffer_sz_, in_proc_ptr, aligned_proc_bufffer_sz_ * sizeof(float));
            int output_index = getRoundCounter(0, convId) % n_aligned_proc_blocks_;
            float * tg_b = tgt_ptr + out_chnl_stride * convId + aligned_processing_sz_ * output_index;
            err = FHT_verify((const __FLOAT__ *)in_b, (const __FLOAT__ *)tg_b, aligned_processing_sz_, 0, aligned_processing_sz_, (__FLOAT__)1. );
            if ( err >= 0 ) {
#ifdef _DEBUG_PRINTF
                std::cout <<  "Process direct tarnsform mismatch: round " << (int)getRoundCounter(uploadId, convId) << " channel " << i << "\n";
#endif
                AMF_ASSERT_OK(AMF_UNEXPECTED, L"Process direct tarnsform mismatch: "
                                              L"round %d channel %d", 
                              (int)getRoundCounter(uploadId, convId), i);
                break;
            }
        }
        delete [] in_b;

#ifdef _DEBUG_PRINTF
#ifdef TAN_SDK_EXPORTS
        AMF_ASSERT(err == -1, L"Process direct tarnsform verified : %d", (int)getRoundCounter(0, 0));
#else
        if ( err == -1 )  {
            std::cout << "Process direct tarnsform verified : " << (int)getRoundCounter(0, 0) << "\n";
        }
#endif
#endif
    }

    return(ret);
}



/*---------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------*/
int 
CGraalConv::processAccum(int _n_channels,
                        int _IR_bin_shift,
                        int _STR_bin_shift,
                        bool _use_xf_buff
#ifndef TAN_SDK_EXPORTS
                        cl_command_queue _graalQ
#endif
                        )
{
    int ret = GRAAL_SUCCESS;
// starting convolution from bin 0 - uniform, single stage  pipeline, 1 - head tail
    int IR_bin_shift = _IR_bin_shift;//
    int STR_bin_shift = _STR_bin_shift; // time shift 0, uniform -1, head tail

    int total_n_bins = n_aligned_proc_blocks_ - IR_bin_shift;
    int headRun = ((total_n_bins + n_accum_blocks_ - 1) / n_accum_blocks_);

// head
    cl_command_queue inQ = this->m_pContextTAN->GetOpenCLConvQueue();
    cl_kernel headAccumKernel = CMADKernels_[0];

    CASubBuf<float> &kern_store_buf = *(CASubBuf<float> *)kernel_trasformed_union_;
    CABuf<float> &data_store_buf = *(CABuf<float> *)history_transformed_;
    CABuf<float> &accum_buf = _use_xf_buff ? *(CABuf<float>*)cmad_accum_xf_ : *(CABuf<float>*)cmad_accum_;
    CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
    CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
    CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;

    uint store_version_stride = aligned_conv_sz_ * n_max_channels_;
    uint accum_version_stride = accum_stride_ * n_max_channels_;
    int n_arg = 0;
    
#ifdef TAN_SDK_EXPORTS
    if (headRun > 0)
    {
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, kern_store_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &kern_store_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, data_store_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &data_store_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, accum_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, accum_version_stride));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &accum_version_stride);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, accum_stride_));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &accum_stride_);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, store_version_stride));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &store_version_stride);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, aligned_conv_sz_));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &aligned_conv_sz_);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, IR_bin_shift));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &IR_bin_shift);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, n_aligned_proc_blocks_));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_aligned_proc_blocks_);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, n_accum_blocks_));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_accum_blocks_);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, n_max_channels_));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_max_channels_);
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgBufferNative(n_arg++, count_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->SetArgInt32(n_arg++, STR_bin_shift));
        ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &STR_bin_shift);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: STR_bin_shift" )

        size_t l_wk[3] = { size_t(min(aligned_processing_sz_ / 2, 256)), size_t(1), size_t(1) };
        size_t g_wk[3] = { 1, 1, 1 };
        g_wk[0] = aligned_processing_sz_ / 2;
        g_wk[1] = headRun;
        g_wk[2] = _n_channels;

        ////////AMF_RETURN_IF_FAILED(CMADKernels_[0]->Enqueue(3, NULL, g_wk, l_wk), L"Enqueue() failed");
        ret = clEnqueueNDRangeKernel(inQ, headAccumKernel, 3, NULL, g_wk, l_wk, 0, NULL, NULL);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel CMADKernels_[0] failed: " )
    }
#else
    ret = clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &kern_store_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &data_store_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &accum_version_stride);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &accum_stride_);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &store_version_stride);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &aligned_conv_sz_);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &IR_bin_shift);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_aligned_proc_blocks_);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_accum_blocks_);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &n_max_channels_);
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    ret |= clSetKernelArg(headAccumKernel, n_arg++, sizeof(int), &STR_bin_shift);



    CHECK_OPENCL_ERROR(ret, "parmeters failed.");


    size_t l_wk[3] = {min(aligned_processing_sz_/2, 256),1,1};

    size_t g_wk[3] = {1,1,1};

    g_wk[0] = aligned_processing_sz_/2;
    g_wk[1] = headRun;
    g_wk[2] = _n_channels;


    //	eoh_event_ is set only in the head stage of the head -tail algorithm
    // the classic algorithms runs with a single queue and does not use events at all

    int n_wait_events = 0;
    cl_event * p_wait_event = NULL;
    cl_event * p_set_event = NULL;

    eop_event_ = NULL;

    // THIS IS NOT implemented: running on the same Q
    // if event has been setup in the 1st tsage wiat for it
    if (eoh_event_ != NULL)
    {
        n_wait_events = 1;
        p_wait_event = &eoh_event_;
    }


    // THIS IS NOT implemented: running on the same Q
    // setup an even the next raound will wait for
    if (eoh_event_ != NULL && headRun == 1)
    {
        p_set_event = &eop_event_;
    }


    if (headRun > 0)
    {
        ret = clEnqueueNDRangeKernel(inQ, headAccumKernel, 3, NULL, g_wk, l_wk, n_wait_events, p_wait_event, p_set_event);
    }

    CHECK_OPENCL_ERROR(ret, "kernel accumulator head failed.");
#endif


    total_n_bins = headRun;
    int last_tail = 0;

// tail
    if (total_n_bins > 1)
    {

        int tailRun = headRun;

        int n_arg = 0, last_arg;

        cl_command_queue inQ = this->m_pContextTAN->GetOpenCLConvQueue();
        cl_kernel tailAccumKernel = CMADKernels_[1];

        CABuf<float> &accum_buf = _use_xf_buff ? *(CABuf<float>*)cmad_accum_xf_ : *(CABuf<float>*)cmad_accum_;
        CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
        CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
        uint accum_version_stride = accum_stride_ * n_max_channels_;

#ifdef TAN_SDK_EXPORTS
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgBufferNative(n_arg++, accum_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_buf" )
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: set_map_buf" )
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf" )
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgInt32(n_arg++, accum_version_stride));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &accum_version_stride);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_version_stride" )
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgInt32(n_arg++, accum_stride_));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &accum_stride_);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: accum_stride_" )
        ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgInt32(n_arg++, n_accum_blocks_));
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &n_accum_blocks_);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: n_accum_blocks_" )

        do {
            last_arg = n_arg;
            total_n_bins = tailRun;

            ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->SetArgInt32(last_arg++, total_n_bins));
            ret = clSetKernelArg(tailAccumKernel, last_arg++, sizeof(int), &total_n_bins);
            AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: total_n_bins" )

            tailRun = (total_n_bins + n_accum_blocks_ - 1) / n_accum_blocks_;
            size_t l_wk[3] = {size_t(min(aligned_processing_sz_, 256)),1,1};
            size_t g_wk[3] = {1,1,1};

            g_wk[0] = aligned_processing_sz_;
            g_wk[1] = tailRun;
            g_wk[2] = _n_channels;

            last_tail = (int)(tailRun == 1);

            ////////AMF_RETURN_IF_FAILED(CMADKernels_[1]->Enqueue(3, NULL, g_wk, l_wk), L"Enqueue() failed");
            ret = clEnqueueNDRangeKernel(inQ, CMADKernels_[1], 3, NULL, g_wk, l_wk, 0, NULL, NULL);
            AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"CMADKernels_[1] clEnqueueNDRangeKernel failed: " )
        } while (!last_tail);
        // Pushing the jobs to be flushed from command buffer and be submitted to the device in a non-blocking way 
        clEnqueueReadBuffer(inQ, accum_buf.getCLMem(), CL_FALSE,0, 8 * sizeof(float), m_dataBuff, 0 , NULL, NULL);
        
    }
#else
        ret = clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &accum_buf.getCLMem());
        ret |= clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
        ret |= clSetKernelArg(tailAccumKernel, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
        ret |= clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &accum_version_stride);
        ret |= clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &accum_stride_);
        ret |= clSetKernelArg(tailAccumKernel, n_arg++, sizeof(int), &n_accum_blocks_);


        do {

            last_arg = n_arg;
            total_n_bins = tailRun;

            ret |= clSetKernelArg(tailAccumKernel, last_arg++, sizeof(int), &total_n_bins);
            CHECK_OPENCL_ERROR(ret, "parmeters failed.");


            tailRun = (total_n_bins + n_accum_blocks_ - 1) / n_accum_blocks_;
            size_t l_wk[3] = {min(aligned_processing_sz_, 256),1,1};

            size_t g_wk[3] = {1,1,1};

            g_wk[0] = aligned_processing_sz_;
            g_wk[1] = tailRun;
            g_wk[2] = _n_channels;

            last_tail = (int)(tailRun == 1);

            // THIS IS NOT implemented: running on the same Q
            // setup an even the next round will wait for
            if (eoh_event_ != NULL && last_tail)
            {
                p_set_event = &eop_event_;
            }

            ret = clEnqueueNDRangeKernel(inQ, tailAccumKernel, 3, NULL, g_wk, l_wk, 0, NULL, p_set_event);

            CHECK_OPENCL_ERROR(ret, "kernel accumulator tail failed.");

    

        } while (!last_tail);
    }

    if (eoh_event_ != NULL)
    {
        clReleaseEvent(eoh_event_);
        eoh_event_ = NULL;
    }
#endif

    return(ret);
}




/*---------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------*/
int 
CGraalConv::processPull(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        int _advance_time
        )
{
    int ret = GRAAL_SUCCESS;
#ifndef TAN_SDK_EXPORTS
    cl_kernel outFhtKernel = inverseTransformKernel_;
#endif

    uint out_version_stride = aligned_proc_bufffer_sz_ * n_max_channels_;
    uint counter_version_stride = n_max_channels_;

    int in_version_stride = accum_stride_ * n_max_channels_;
    int in_chnl_stride = accum_stride_;
    int out_chnl_stride = aligned_proc_bufffer_sz_;
    float scale = (float)0.5 / (float)aligned_processing_sz_;

    cl_command_queue outQ = this->m_pContextTAN->GetOpenCLConvQueue();

    CABuf<float> & sum_buf = *(CABuf<float>*)cmad_accum_;
    CABuf<float> & out_buf = *(CABuf<float>*)process2_output_staging_;
    CABuf<float> & sincos = *(CABuf<float> *)sincos_;
    CABuf<short> & bit_reverse = *(CABuf<short> *)bit_reverse_;

    CABuf<int> &chnl_map_buf = *(CABuf<int> *)channels_map_;
    CABuf<int> &set_map_buf = *(CABuf<int> *)sets_map_;
    CABuf<uint> &count_buf = *(CABuf<uint> *)round_counters_;


    int n_arg = 0;
    // inverse FHT

#ifdef TAN_SDK_EXPORTS
    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, sum_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &sum_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sum_buf" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, out_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &out_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_buf" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, bit_reverse.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: bit_reverse" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, sincos.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sincos" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgInt32(n_arg++, in_version_stride));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(int), &in_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_version_stride" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgInt32(n_arg++, in_chnl_stride));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(int), &in_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgInt32(n_arg++, out_version_stride));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(int), &out_version_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_version_stride" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgInt32(n_arg++, out_chnl_stride));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(int), &out_chnl_stride);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgFloat(n_arg++, scale));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(float), &scale);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )
    //@todo: why are we passing uint as a float??
#pragma message("why are we passing uint as a float??") 
    ////////AMF_RETURN_IF_FAILED(
    ////////    inverseTransformKernel_->SetArgFloat(n_arg++, *reinterpret_cast<float*>(&counter_version_stride)));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(float), &counter_version_stride);        // inverse conv scale
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: counter_version_stride" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, chnl_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: chnl_map_buf" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, set_map_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: set_map_buf" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgBufferNative(n_arg++, count_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: count_buf" )

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->SetArgInt32(n_arg++, _advance_time));
    ret = clSetKernelArg(inverseTransformKernel_, n_arg++, sizeof(int), &_advance_time);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: _advance_time" )


    size_t l_wk[3] = { size_t(min(aligned_processing_sz_ / 2, 256)), 1, 1 };
    size_t g_wk[3] = { 1, 1, 1 };

    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;

    ////////AMF_RETURN_IF_FAILED(inverseTransformKernel_->Enqueue(2, NULL, g_wk, l_wk), L"Enqueue() failed");
    if (m_pullKernelEvent) { clReleaseEvent(m_pullKernelEvent); m_pullKernelEvent = NULL; }
	ret = clEnqueueNDRangeKernel(outQ, inverseTransformKernel_, 2, NULL, g_wk, l_wk, 0, NULL, &m_pullKernelEvent);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel inverseTransformKernel_ failed: " )

#else
    ret = clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &sum_buf.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &out_buf.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(int), &in_version_stride);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(int), &in_chnl_stride);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(int), &out_version_stride);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(int), &out_chnl_stride);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(float), &scale);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(float), &counter_version_stride);
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &chnl_map_buf.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &set_map_buf.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(cl_mem), &count_buf.getCLMem());
    ret |= clSetKernelArg(outFhtKernel, n_arg++, sizeof(int), &_advance_time);


    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    size_t l_wk[3] = { min(aligned_processing_sz_ / 2, 256), 1, 1 };

    size_t g_wk[3] = { 1, 1, 1 };

    g_wk[0] = l_wk[0];
    g_wk[1] = _n_channels;

    ret = clEnqueueNDRangeKernel(outQ, outFhtKernel, 2, NULL, g_wk, l_wk, 0, NULL, NULL);

    CHECK_OPENCL_ERROR(ret, "kernel accumulator tail failed.");
#endif

    if (verify > 1)
    {

        int in_version_stride = accum_stride_ * n_max_channels_;
        int in_chnl_stride = accum_stride_;
        int out_chnl_stride = aligned_proc_bufffer_sz_;
        float scale = (float)0.5 / (float)aligned_processing_sz_;

        cl_command_queue outQ = this->m_pContextTAN->GetOpenCLConvQueue();

        CABuf<float> & sum_buf = *(CABuf<float>*)cmad_accum_;
        CABuf<float> & out_buf = *(CABuf<float>*)process2_output_staging_;

        sum_buf.copyToHost(outQ);
        float * tgt_ptr = out_buf.map(outQ, CL_MAP_READ);
        float * src_ptr = sum_buf.getSysMem();
        int err = -1;
        for (int i = 0; i < _n_channels; i++)
        {
            int convId = _convIDs[i];
            int uploadId = _uploadIDs[i];
            err = FHT_verify((const __FLOAT__ *)src_ptr + uploadId * in_version_stride + convId * accum_stride_, (const __FLOAT__ *)tgt_ptr + (uploadId * n_max_channels_ + convId) * aligned_proc_bufffer_sz_, aligned_processing_sz_, 1, aligned_proc_bufffer_sz_, 0.5f);
            if (err >= 0) {
#ifdef _DEBUG_PRINTF
                std::cout << "Process invert transform mismatch: round " << (int)getRoundCounter(uploadId, convId) << " channel " << i << "\n";
#endif
                AMF_ASSERT_OK(AMF_UNEXPECTED, L"Process invert transform mismatch: "
                                              L"round %d channel %d",
                              (int)getRoundCounter(uploadId, convId), i);
                break;
            }

        }


        out_buf.unmap();

#ifdef _DEBUG_PRINTF
        if (err == -1)  {
            std::cout << "Process inverse tarnsform verified : " << (int)(int)getRoundCounter(0, 0) << "\n";
        }
#endif
        AMF_ASSERT(err == -1, L"Process inverse tarnsform verified : %d",
                   (int)(int)getRoundCounter(0, 0));
    }

        return(ret);
}


/*---------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------*/
int
CGraalConv::updateConvOCL(	 void* _stg_buf,  void *_transf_buf, int _conv_len, cl_command_queue _uploadQ, int _uploadID, int _convID)
{
    int ret = GRAAL_SUCCESS;

    CABuf<float> & stg_buf = *(CABuf<float>*)_stg_buf;
    CASubBuf<float> & transf_buf = *(CASubBuf<float> *)_transf_buf;
    CABuf<float> & sincos = *(CABuf<float> *)sincos_;
    CABuf<short> & bit_reverse = *(CABuf<short> * )bit_reverse_;

    size_t l_wk[3] = { size_t(min(aligned_processing_sz_ / 2, 256)), 1, 1 };

    size_t g_wk[3] = {1,1,1};

    g_wk[0] = n_aligned_proc_blocks_* l_wk[0];

// run direct FHT for the 1st stream
    int n_arg = 0;
// direct FHT
    
    int in_chnl_stride = max_conv_sz_;
    int out_chnl_stride = aligned_conv_sz_;

#if TAN_SDK_EXPORTS
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgBufferNative(n_arg++, stg_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &stg_buf.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: stg_buf." )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgBufferNative(n_arg++, transf_buf.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &transf_buf.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: transf_buf." )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgBufferNative(n_arg++, bit_reverse.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: bit_reverse." )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgBufferNative(n_arg++, sincos.getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &sincos.getCLMem() );
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: sincos." )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgInt32(n_arg++, _conv_len));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &_conv_len);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: _conv_len" )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgInt32(n_arg++, in_chnl_stride));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &in_chnl_stride);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: in_chnl_stride" )
    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->SetArgInt32(n_arg++, out_chnl_stride));
        ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &out_chnl_stride);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clSetKernelArg failed: out_chnl_stride" )

    ////////AMF_RETURN_IF_FAILED(uploadKernel2_->Enqueue(1, NULL, g_wk, l_wk), L"Enqueue() failed");
        ret = clEnqueueNDRangeKernel(_uploadQ, uploadKernel2_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
        AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueNDRangeKernel uploadKernel2_ failed: " )
#else
    ret = clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &stg_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &transf_buf.getCLMem());
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &bit_reverse.getCLMem());
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(cl_mem), &sincos.getCLMem());
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &_conv_len);
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &in_chnl_stride);
    ret |= clSetKernelArg(uploadKernel2_, n_arg++, sizeof(int), &out_chnl_stride);
    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    ret = clEnqueueNDRangeKernel(_uploadQ, uploadKernel2_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);

    CHECK_OPENCL_ERROR(ret, "kernel direct transform  failed.");
//	clFinish(clUploadQ_);
#endif

    if ( verify  == 1 || verify == 3) 
    {
        transf_buf.copyToHost(_uploadQ);
        stg_buf.copyToHost(_uploadQ);
        size_t len = transf_buf.getLen(); 
        float *ext_stg = new float[len];
        memset(ext_stg, 0, len * sizeof(float));
        float * src_ptr = stg_buf.getSysMem();
        float * tgt_ptr = transf_buf.getSysMem();
        float * src_buf_ptr, * ext_buf_ptr, *tgt_buf_ptr;

        int n_test_loops = (_conv_len + aligned_proc_bufffer_sz_ - 1)/aligned_proc_bufffer_sz_;
        int err = -1; //GRAAL_SUCCESS;
        src_buf_ptr = src_ptr; // + j * in_chnl_stride;
        ext_buf_ptr = ext_stg; // + j * out_chnl_stride;
        tgt_buf_ptr = tgt_ptr;

            // src moves every block, second half padded with 0s
            for( int i = 0, k = 0, l = 0; i < n_test_loops; i++, l +=aligned_processing_sz_, k+=aligned_proc_bufffer_sz_)
            {
                for ( int j = 0; j < aligned_proc_bufffer_sz_ && k + j < _conv_len; j++) 
                {
                    ext_buf_ptr[l + j] = src_buf_ptr[k + j];
                }
            }
 
            for (int i = 0; i < n_test_loops; i++)
            {
                err = FHT_verify((const __FLOAT__ *)ext_buf_ptr + aligned_processing_sz_ *i, (const __FLOAT__ *)tgt_buf_ptr + aligned_processing_sz_ *i,
                    aligned_processing_sz_, 0, aligned_processing_sz_, (__FLOAT__)1. );
                if ( err >= 0 ) {
#ifdef _DEBUG_PRINTF
                    printf("Kernel update mismatch: at block %d\n", i);
#endif
                    AMF_ASSERT_OK(AMF_UNEXPECTED, L"Kernel update mismatch: at block %d", i);
                    break;
                }

            }

    
#ifdef _DEBUG_PRINTF
        if ( err == -1 )  {
            std::cout << "Kernel update verified: u=" << _uploadID << " c=" << _convID << " len=" <<_conv_len << "\n";
        }
#endif
        AMF_ASSERT(err == -1, L"Kernel update verified: u=%d c=%d len=%d",
                   _uploadID, _convID, _conv_len);
    }
    return ret;
}



int
CGraalConv::uploadConvHostPtrIntnl(
        int _n_channels, 
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const float** _conv_ptrs,  // arbitrary host ptrs
        const int * _conv_lens
        )
{
    int ret = GRAAL_SUCCESS;

// move data into staging
    cl_command_queue uploadQ = this->m_pContextTAN->GetOpenCLGeneralQueue();

    for(int j = 0; j < _n_channels; j++)
    {

        int convId = _convIDs[j];
        int uploadId = _uploadIDs[j];
        int conv_len = _conv_lens[j];
        const float * conv_ptr = _conv_ptrs[j];

// move data into staging
        CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[uploadId][convId];

//	ret = stg_buf->attach(_conv_ptr, _conv_len);
        ret = stg_buf->copyToDeviceNonBlocking(uploadQ, CL_MEM_READ_ONLY, conv_ptr, conv_len);
        CHECK_OPENCL_ERROR(ret, "upload failed.");
// do direct transform
    }

    return ret;

}


int
CGraalConv::sincUpload( void )
{
    int ret = GRAAL_SUCCESS;
    clFinish(this->m_pContextTAN->GetOpenCLGeneralQueue() );
    return(ret);
}

//-cl-fp32-correctly-rounded-divide-sqrt
void
CGraalConv::selectOptions(std::string & _kernel_file, std::string & _comp_options)
{
    int group_sz = min(aligned_processing_sz_ / 2, 256);
    int log2_group_sz = static_cast<int>(ceil(log2((double)group_sz)));
    _comp_options = std::string("-cl-fp32-correctly-rounded-divide-sqrt ") + std::string("-D _K0_GROUP_SZ=") + std::to_string((_Longlong)group_sz) + std::string(" -D _K0_LOG2_GROUP_SZ=") + std::to_string((_Longlong)log2_group_sz) +
        std::string(" -D _K0_LOG2_N=") + std::to_string((_Longlong)(processing_log2_ + 1)) + std::string(" -D _K0_N=") + std::to_string((_Longlong)aligned_processing_sz_);
 
}
int
CGraalConv::selectUploadOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTConvIn";

//	_comp_options = "-x clc++ -D _K0_GROUP_SZ=256 -D _K0_N=2048 -D _K0_LOG2_N=11";
    selectOptions(_kernel_file, _comp_options);
    return(ret);
}


int
CGraalConv::selectUpload2Options(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTUploadConv";

    selectOptions(_kernel_file, _comp_options);
    return(ret);
}


int
CGraalConv::selectResetOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTAdvanceTime";
    selectOptions(_kernel_file, _comp_options);
    return(ret);
}



int
CGraalConv::selectDirectFHTOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTPushIn";
    selectOptions(_kernel_file, _comp_options);
    return(ret);
}

int
CGraalConv::selectFHT_CMADOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::vector<std::string> & _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name.resize(2);
    _kernel_name[0] = "FHTMultAddHead2";
    _kernel_name[1] = "FHTMultAddTail";

    selectOptions(_kernel_file, _comp_options);
    return(ret);
}

int
CGraalConv::selectInverseFHTOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTPushOut";
    selectOptions(_kernel_file, _comp_options);

    return(ret);
}


int 
CGraalConv::selectConvHead1Options(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options)
{
    int ret = GRAAL_SUCCESS;
    _kernel_file = "GraalFHT.cl";
    kernel_src = (char*)GraalFHT;
    kernel_src_size = GraalFHTCount;
    _kernel_name = "amdFHTConvHead1";
    selectOptions(_kernel_file, _comp_options);
    return(ret);
}


#ifdef TAN_SDK_EXPORTS

AMF_RESULT CGraalConv::zeroMemory(CABuf<float> *pBuf, amf_uint offset, amf_uint amount)
{
    float pattern = 0;
    cl_int ret = clEnqueueFillBuffer(this->m_pContextTAN->GetOpenCLGeneralQueue(), pBuf->getCLMem(), &pattern, sizeof(float), offset * sizeof(float), amount* sizeof(float), 0, NULL,NULL);
    AMF_RETURN_IF_FALSE(CL_SUCCESS == ret, AMF_UNEXPECTED, L"clEnqueueFillBuffer failed: ")
    return AMF_OK;
}
#endif // TAN_SDK_EXPORTS

int
CGraalConv::setupCL(amf::AMFComputePtr pComputeConvolution, amf::AMFComputePtr pComputeUpdate 
#ifndef TAN_SDK_EXPORTS
    cl_context _clientContext,
    cl_device_id _clientDevice,
    cl_command_queue _clientQ
#endif
)
{
    int ret = GRAAL_SUCCESS;
    cl_queue_properties prop[] = { 0 };


    const cl_context Ctxt = static_cast<cl_context>(m_pContextTAN->GetOpenCLContext());
    cl_command_queue graalQ_ = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue() );

#  define CABufArgs Ctxt
#  define CAUpdBufArgs Ctxt

    CABuf<float> *new_inp_union_buf = new CABuf<float>(CAUpdBufArgs);
    assert(new_inp_union_buf);
    ret = new_inp_union_buf->create(n_sets_* n_max_channels_ * max_conv_sz_, 0 );
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(new_inp_union_buf, graalQ_);
    kernel_input_union_ = new_inp_union_buf;

    CABuf<float> *new_transf_union_buf = new CABuf<float>(CABufArgs);
    assert(new_transf_union_buf);
    ret = new_transf_union_buf->create(n_sets_* n_max_channels_ *aligned_conv_sz_, 0 );
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(new_transf_union_buf, graalQ_);
    kernel_trasformed_union_ = new_transf_union_buf;
#ifdef _DEBUG_PRINTF
    printf("Kernel storage size = %6.2fMB\n", (float)(new_transf_union_buf->getLen() * sizeof(float)) / (float)1000000);
#endif
    AMFTraceInfo(AMF_FACILITY, L"Kernel storage size = %6.2fMB\n",
                 (float)(new_transf_union_buf->getLen() * sizeof(float)) / 1024 / 1024);

// kernel channels map
    CABuf<int> *krnl_chnls_map_buf = new CABuf<int>(CAUpdBufArgs);
    assert(krnl_chnls_map_buf);
    ret = krnl_chnls_map_buf->create(n_max_channels_,  CL_MEM_USE_PERSISTENT_MEM_AMD);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(krnl_chnls_map_buf, graalQ_);
    kernel_channels_map_ = krnl_chnls_map_buf;

// kernel sets map
    CABuf<int> *krnl_sets_map_buf = new CABuf<int>(CAUpdBufArgs);
    assert(krnl_sets_map_buf);
	ret = krnl_sets_map_buf->create(n_max_channels_,  CL_MEM_USE_PERSISTENT_MEM_AMD);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(krnl_sets_map_buf, graalQ_);
    kernel_sets_map_ = krnl_sets_map_buf;

// kernel sets map
    CABuf<int> *krnl_lens_map_buf = new CABuf<int>(CAUpdBufArgs);
    assert(krnl_lens_map_buf);
	ret =  krnl_lens_map_buf->create(n_sets_ * n_max_channels_,  CL_MEM_USE_PERSISTENT_MEM_AMD);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(krnl_lens_map_buf, graalQ_);
    kernel_lens_map_ = krnl_lens_map_buf;

    kernel_transformed_.resize(n_sets_);
    kernel_staging_.resize(n_sets_);
    kernel_transformed_store_.resize(n_sets_);
#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
    kernel_input_store_.resize(n_sets_);
#endif
    for( int i = 0; i < n_sets_; i++ )
    {

        CASubBuf<float> *new_store_buf = new CASubBuf<float>(*new_transf_union_buf);
        assert(new_store_buf);
		ret = new_store_buf->create(n_max_channels_ *aligned_conv_sz_ *i, n_max_channels_ *aligned_conv_sz_, 0 );
		AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
        initBuffer(new_store_buf, graalQ_);
        kernel_transformed_store_[i] = new_store_buf;

#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
        CASubBuf<float> *new_inputstore_buf = new CASubBuf<float>(*new_inp_union_buf);
        assert(new_inputstore_buf);
		ret = new_inputstore_buf->create((i*n_max_channels_) *max_conv_sz_, max_conv_sz_*n_max_channels_, conv_mem_alloc_flags_);
		AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
        initBuffer(new_inputstore_buf, graalQ_);
        kernel_input_store_[i] = new_inputstore_buf;
#endif
        kernel_staging_[i].resize(n_max_channels_);
        kernel_transformed_[i].resize(n_max_channels_);
        for(int j = 0; j < n_max_channels_; j++) 
        {
            CASubBuf<float> *new_stg_buf = new CASubBuf<float>(*new_inp_union_buf);
            assert(new_stg_buf);
			ret = new_stg_buf->create((i*n_max_channels_ + j) *max_conv_sz_, max_conv_sz_, conv_mem_alloc_flags_);
			AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
            initBuffer(new_stg_buf, graalQ_);
            kernel_staging_[i][j] = new_stg_buf;


            CASubBuf<float> *new_transf_buf = new CASubBuf<float>(*new_transf_union_buf);
            assert(new_transf_buf);
			ret = new_transf_buf->create((i*n_max_channels_ + j) * aligned_conv_sz_, aligned_conv_sz_, 0 );
			AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
            initBuffer(new_transf_buf, graalQ_);
            kernel_transformed_[i][j] = new_transf_buf;
        }
    }

    // get fht lookup tables
    CABuf<float> * sincos = new CABuf<float>(CABufArgs);
    CABuf<short> * bit_reverse = new CABuf<short>(CABufArgs);
    
    FHTInit(&(sincos->getSysMem()), &(bit_reverse->getSysMem()), (FHT_FUNC *)&FHT_transformCPU_, aligned_processing_sz_); 
    sincos->setLen(aligned_processing_sz_);
    bit_reverse->setLen(aligned_processing_sz_);

    sincos_ = sincos;
    bit_reverse_ = bit_reverse;
    sincos->copyToDevice(graalQ_, 0);
    bit_reverse->copyToDevice(graalQ_, 0);

    std:: string kernel_file, kernel_src, kernel_name, comp_options;
    size_t kernel_src_size = 0;

    bool goit = false;

    selectUploadOptions(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(uploadKernel_, pComputeUpdate, this->m_pContextTAN->GetOpenCLGeneralQueue(),
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

    selectUpload2Options(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(uploadKernel2_, pComputeUpdate, this->m_pContextTAN->GetOpenCLGeneralQueue(),
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

    selectResetOptions(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(resetKernel_, pComputeConvolution, graalQ_,
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

    goit = GetOclKernel(m_copyWithPaddingKernel, pComputeUpdate, this->m_pContextTAN->GetOpenCLGeneralQueue(),
                                        "GraalFHT.cl", (const char*)GraalFHT, GraalFHTCount, "amdPadFFTBlock", comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", "amdPadFFTBlock");

    cl_command_queue iniQ = graalQ_;
    // per stream counters
    CABuf<uint> *count_buf = new CABuf<uint>(CABufArgs);
    assert(count_buf);

	ret = count_buf->create(n_max_channels_ * n_sets_, 0);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    count_buf->setValue2(graalQ_, 0);
    (void)count_buf->getSysMem();   // allocate backing system memory block.
    initBuffer(count_buf, graalQ_);
    round_counters_ = count_buf;

// channels map
    CABuf<int> *chnls_map_buf = new CABuf<int>(CABufArgs);
    assert(chnls_map_buf);
	ret = chnls_map_buf->create(n_sets_ * n_max_channels_,  CL_MEM_USE_PERSISTENT_MEM_AMD);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(chnls_map_buf, graalQ_);
    channels_map_ = chnls_map_buf;
// sets map
    CABuf<int> *sets_map_buf = new CABuf<int>(CABufArgs);
    assert(sets_map_buf);
	ret = sets_map_buf->create(n_sets_ * n_max_channels_,  CL_MEM_USE_PERSISTENT_MEM_AMD);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(sets_map_buf, graalQ_);
    sets_map_ = sets_map_buf;

// process input
// fill input and keep previous inputs in a proper  places
    m_process_input_staging_ = CABuf<float>(CABufArgs);
	ret = m_process_input_staging_.create(aligned_proc_bufffer_sz_ * n_input_blocks_ * n_max_channels_,
                                    CL_MEM_USE_PERSISTENT_MEM_AMD );
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(&m_process_input_staging_, graalQ_);

// history, transformed input data cyclic array
    CABuf<float> *history_transform_buf = new CABuf<float>(CABufArgs);
    assert(history_transform_buf);
	ret = history_transform_buf->create(n_max_channels_ *aligned_conv_sz_/* n_sets_*/,  0);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(history_transform_buf, graalQ_);
    history_transformed_ = history_transform_buf;
#ifdef _DEBUG_PRINTF
    printf("Hist storage size = %6.2fMB\n", (float)(history_transform_buf->getLen() * sizeof(float)) / (float)1000000);
#endif
    AMFTraceInfo(AMF_FACILITY, L"Hist storage size = %6.2fMB\n",
                 (float)(history_transform_buf->getLen() * sizeof(float)) / 1024 / 1024);

// output data
// inverse transform output
    CABuf<float> * out_buf = new CABuf<float>(CABufArgs);
    assert(out_buf);
	ret = out_buf->create(aligned_proc_bufffer_sz_ * n_max_channels_ * n_sets_, CL_MEM_ALLOC_HOST_PTR);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(out_buf, graalQ_);
    process2_output_staging_ = out_buf;

    // accumulator

    accum_stride_ = ((n_aligned_proc_blocks_ + n_accum_blocks_ - 1) / n_accum_blocks_) * aligned_processing_sz_;

    size_t accum_len = accum_stride_ * n_max_channels_ * n_sets_;
    CABuf<float> *accum_buf = new CABuf<float>(CABufArgs);
	ret = accum_buf->create(accum_len, 0);
	AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(accum_buf, graalQ_);
    cmad_accum_ = accum_buf;
    assert(cmad_accum_);

    CABuf<float> *accum_buf_cf = new CABuf<float>(CABufArgs);
    ret = accum_buf_cf->create(accum_len, 0);
    AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(accum_buf_cf, graalQ_);
    cmad_accum_xf_ = accum_buf_cf;
    assert(cmad_accum_xf_);

    CABuf<int> *copy_response_in_offset = new CABuf<int>(CABufArgs);
    ret = copy_response_in_offset->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(copy_response_in_offset, graalQ_);
    copy_response_in_offset_ = copy_response_in_offset;
    assert(copy_response_in_offset_);

    CABuf<int> *copy_response_out_offset = new CABuf<int>(CABufArgs);
    ret = copy_response_out_offset->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
    initBuffer(copy_response_out_offset, graalQ_);
    copy_response_out_offset_ = copy_response_out_offset;
    assert(copy_response_out_offset_);

    host_copy_resp_in_offset = new int[n_max_channels_];
    host_copy_resp_out_offset = new int[n_max_channels_];
    host_input_staging_.resize(n_sets_);
//	input_transformed_.resize(n_sets_);
//	cmad_accum_.resize(n_sets_);
    for(int i = 0; i < n_sets_; i++)
    {
// queued input in cyclic array
        //AMD_PERSISTENT
        CABuf<float> *host_inp_buf = new CABuf<float>(CAUpdBufArgs);
		ret = host_inp_buf->create(aligned_proc_bufffer_sz_ * n_max_channels_, CL_MEM_ALLOC_HOST_PTR);
		AMF_RETURN_IF_FALSE(GRAAL_SUCCESS == ret, ret, L"Failed to create buffer: %d", ret);
        initBuffer(host_inp_buf, graalQ_);
        host_input_staging_[i] = host_inp_buf;

        assert(host_input_staging_[i]);
    }

    selectDirectFHTOptions(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(directTransformKernel_, pComputeConvolution, graalQ_,
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

//	CMADKernels_
    std::vector<std::string> kernel_names;
    selectFHT_CMADOptions(kernel_file, kernel_src, kernel_src_size, kernel_names, comp_options);
    CMADKernels_.resize(kernel_names.size());
    for(int i = 0; i < CMADKernels_.size(); i++)
    {
        goit = GetOclKernel(CMADKernels_[i], pComputeConvolution, graalQ_,
                                            kernel_file, kernel_src, kernel_src_size, kernel_names[i], comp_options);
        AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_names[i]);
    }

    selectInverseFHTOptions(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(inverseTransformKernel_, pComputeConvolution, graalQ_,
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

    selectConvHead1Options(kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    goit = GetOclKernel(convHead1_, pComputeConvolution, graalQ_,
                                        kernel_file, kernel_src, kernel_src_size, kernel_name, comp_options);
    AMF_RETURN_IF_FALSE(true == goit, goit, L"failed: GetOclKernel %s", kernel_name);

    clFinish(m_pContextTAN->GetOpenCLConvQueue());
    clFinish(m_pContextTAN->GetOpenCLGeneralQueue());
    return ret;
}


int
CGraalConv::cleanup()
{
    int ret = GRAAL_SUCCESS;
    int status = CL_SUCCESS;

    if (this->m_pContextTAN->GetOpenCLConvQueue() )
    {
        //clFinish(this->m_pContextTAN->GetOpenCLConvQueue());
        clFlush(this->m_pContextTAN->GetOpenCLConvQueue());
    }
    if (this->m_pContextTAN->GetOpenCLGeneralQueue() )
    {
        //clFinish(this->m_pContextTAN->GetOpenCLGeneralQueue());
        clFlush(this->m_pContextTAN->GetOpenCLGeneralQueue());
    }

    if (round_counters_)
    {
        delete (CABuf<uint> *)round_counters_;
        round_counters_ = 0;
    }
    if (channels_map_)
    {
        delete (CABuf<uint> *)channels_map_;
        channels_map_ = NULL;
    }
    if (sets_map_)
    {
        delete (CABuf<uint> *)sets_map_;
        sets_map_ = NULL;
    }

    if (history_transformed_)
    {
        delete (CABuf<float> *)history_transformed_;
        history_transformed_ = NULL;
    }


    if (process2_output_staging_)
    {
        delete (CABuf<float> *)process2_output_staging_;
        process2_output_staging_ = NULL;
    }

    for (int i = 0; i < n_sets_; i++)
    {

        delete (CABuf<float> *)kernel_transformed_store_[i];
#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
        delete (CABuf<float> *)kernel_input_store_[i];
#endif
        for (int j = 0; j < n_max_channels_; j++)
        {
            CABuf<float> *stg_buf = (CABuf<float> *)kernel_staging_[i][j];
            if (stg_buf) {
                delete stg_buf;
            }

            CASubBuf<float> *transf_buf = (CASubBuf<float> *)kernel_transformed_[i][j];
            if (transf_buf) {
                delete transf_buf;
            }
        }

        delete (CABuf<float> *)host_input_staging_[i];
//		delete (CABuf<float> *)cmad_accum_[i];

    }

    if (m_headTailKernelEvent) { 
        clReleaseEvent(m_headTailKernelEvent);
        m_headTailKernelEvent = NULL; 
    }
    if (m_pullKernelEvent) { 
        clReleaseEvent(m_pullKernelEvent);
        m_pullKernelEvent = NULL;
    }
    kernel_staging_.clear();
    kernel_transformed_.clear();
    kernel_transformed_store_.clear();
#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
    kernel_input_store_.clear();
#endif
    host_input_staging_.clear();


    if (cmad_accum_)
    {
        delete (CABuf<float> *)cmad_accum_;
        cmad_accum_ = NULL;
    }
    if (cmad_accum_xf_)
    {
        delete (CABuf<float> *)cmad_accum_xf_;
        cmad_accum_xf_ = NULL;
    }
    if (kernel_channels_map_)
    {
        delete (CABuf<uint> *)kernel_channels_map_;
        kernel_channels_map_ = NULL;
    }

    if (kernel_sets_map_)
    {
        delete (CABuf<uint> *)kernel_sets_map_;
        kernel_sets_map_ = NULL;
    }
    if (kernel_lens_map_)
    {
        delete (CABuf<uint> *)kernel_lens_map_;
        kernel_lens_map_ = NULL;
    }

    if (kernel_input_union_)
    {
        delete (CABuf<float> *)kernel_input_union_;
        kernel_input_union_ = NULL;
    }
    if (kernel_trasformed_union_) {
        delete (CABuf<float> *)kernel_trasformed_union_;
        kernel_trasformed_union_ = 0;
    }

    if (sincos_) {
        delete ((CABuf<float>*)sincos_);
        sincos_ = 0;  // precomputeted sincos table
    }

    if (bit_reverse_) {
        delete ((CABuf<short>*)bit_reverse_);
        bit_reverse_ = 0;  // reverse bit table
    }
    if (copy_response_out_offset_) {
        delete ((CABuf<short>*)copy_response_out_offset_);
        copy_response_out_offset_ = 0;
    }
    if (copy_response_in_offset_) {
        delete ((CABuf<short>*)copy_response_in_offset_);
        copy_response_in_offset_ = 0;
    }
    if (host_copy_resp_in_offset) {
        delete[] host_copy_resp_in_offset;
        host_copy_resp_in_offset = 0;
    }
    if (host_copy_resp_out_offset) {
        delete[] host_copy_resp_out_offset;
        host_copy_resp_out_offset = 0;
    }
    
#ifndef TAN_SDK_EXPORTS
    if (uploadKernel_)
    {
        clReleaseKernel(uploadKernel_);
        uploadKernel_ = 0;
    }

    if (uploadKernel2_)
    {
        clReleaseKernel(uploadKernel2_);
        uploadKernel2_ = 0;
    }

    if (resetKernel_)
    {
        clReleaseKernel(resetKernel_);
        resetKernel_ = 0;
    }

    if (directTransformKernel_)
    {
        clReleaseKernel(directTransformKernel_);
        directTransformKernel_ = 0;
    }

    for (int i = 0; i < CMADKernels_.size(); i++)
    {
        clReleaseKernel(CMADKernels_[i]);
    }
    CMADKernels_.clear();

    if (inverseTransformKernel_)
    {
        clReleaseKernel(inverseTransformKernel_);
        inverseTransformKernel_ = 0;
    }
    if (convHead1_)
    {
        clReleaseKernel(convHead1_);
        convHead1_ = 0;
    }

    if (graalTailQ_)
    {
        clReleaseCommandQueue(graalTailQ_);
    }
    graalTailQ_ = NULL;

    if (own_queue_ && graalQ_)
    {
        clReleaseCommandQueue(graalQ_);
    }

    graalQ_ = 0;
    own_queue_ = false;

    ret = graal::getGraalOCL().cleanup();
#endif // !TAN_SDK_EXPORTS

    return(ret);
}



}




#if 0


/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/
void fftCPU(short int dir,long m,cl_float *x,cl_float *y)
{
   long n, i, i1, j, k, i2, l, l1, l2;
   double c1, c2, tx, ty, t1, t2, u1, u2, z;

   // Calculate the number of points 
   n = 1;
   for (i = 0;i < m;i++) 
      n *= 2;

   // Do the bit reversal 
   i2 = n >> 1;
   j = 0;
   for (i = 0;i < n - 1;i++) 
   {
      if (i < j) 
      {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = (cl_float)tx;
         y[j] = (cl_float)ty;
      }
      k = i2;
      while (k <= j)
      {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   // Compute the FFT 
   c1 = -1.0; 
   c2 = 0.0;
   l2 = 1;
   for (l = 0;l < m;l++) 
   {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0; 
      u2 = 0.0;
      for (j = 0;j < l1;j++)
      {
         for (i = j;i < n;i += l2) 
         {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = (cl_float)(x[i] - t1); 
            y[i1] = (cl_float)(y[i] - t2);
            x[i] += (cl_float)t1;
            y[i] += (cl_float)t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1) 
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   /*if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }*/
   
}

#endif