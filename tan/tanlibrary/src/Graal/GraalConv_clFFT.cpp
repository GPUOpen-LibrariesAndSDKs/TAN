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



#include "GraalConv_clFFT.hpp"
#include "GraalCLUtil/GraalUtil.hpp"
#include "GraalConvOCL.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include "OclKernels\amdFFT_conv_kernels.cl.h"

#ifndef AMF_RETURN_IF_FALSE
#define AMF_RETURN_IF_FALSE(exp, ret_value, /*optional message,*/ ...)
#endif

const int debug = 0;
const int debug_octave = 0;
const char testName[] = "test1";
//The clFFT supports the Hermitian data format which stores a transformed real FFT in
//about half the space (N/2 + 1) as there duplicate, but I ran into accuracty issues
const bool use_hermitian = false;

namespace graal
{

#if _MSC_VER <= 1800
    static double log2(double n)
    {
        // log(n)/log(2) is log2.  
        return log(n) / log(2.);
    }
#endif

CGraalConv_clFFT::CGraalConv_clFFT()
{
}

CGraalConv_clFFT::~CGraalConv_clFFT()
{
    cleanup();
}

void CGraalConv_clFFT::cleanup()
{
    if (clSignalHistBaseBuf)
        delete clSignalHistBaseBuf;

    if (clInputBaseBuf)
        delete clInputBaseBuf;

    if (clInputBlockBaseBuf)
        delete clInputBlockBaseBuf;

    if (clOutputBaseBuf)
        delete clOutputBaseBuf;

    if (clOutputComplexBaseBuf)
        delete clOutputComplexBaseBuf;

    if (clChannelMap)
        delete clChannelMap;

    if (clSetMap)
        delete clSetMap;

    if (clCurrInputMap)
        delete clCurrInputMap;

    if (clPrevInputMap)
        delete clPrevInputMap;

    if (clBlockNumberMap)
        delete clBlockNumberMap;

    for (uint ch = 0; ch < n_max_channels_; ch++)
    {
        if (clIRInputBuf[ch])
            delete clIRInputBuf[ch];
        
        if (clIRInputPaddedBuf[ch])
            delete clIRInputPaddedBuf[ch];

        for (uint set = 0; set < n_sets_; set++)
        {
            if (clIRBlocksBuf[ch][set])
                delete clIRBlocksBuf[ch][set];
        }
    }

    //delete the base buffer after we delete the sub-buffers
    if (clIRBlocksBaseBuf)
        delete clIRBlocksBaseBuf;


    // There is a bug in clFFT library due to which it doesn't properly release the plans and
    // crashes during repeated tearing down (after re-initialization, of course).
    clfftDestroyPlan(&clfftPlanBackAllChannels);
    clfftDestroyPlan(&clfftPlanFwdAllChannels);
    clfftDestroyPlan(&clfftPlanFwdIR);

#ifdef _DEBUG_PRINTF
    std::cout << "CGraalConv_clFFT cleanup finished\n";
#endif
}

int CGraalConv_clFFT::initializeConv(
#ifdef TAN_SDK_EXPORTS
    amf::TANContextPtr &pContextTAN,
    amf::AMFComputePtr &pConvolution,
    amf::AMFComputePtr &pUpdate,
#endif
    int _n_max_channels,
    int _max_conv_sz,
    int _max_proc_buffer_sz,
    int _n_sets,
    int algorithm
#ifndef TAN_SDK_EXPORTS
    ,
    cl_context _clientContext,
    cl_device_id _clientDevice,
    cl_command_queue _clientQ
#endif
)
{
#ifdef TAN_SDK_EXPORTS
    m_pContextTAN = pContextTAN;
    m_pComputeEngineConvolution = pConvolution;
    m_pComputeEngineUpdate = pUpdate;

    AMF_RETURN_IF_INVALID_POINTER(m_pComputeEngineConvolution);
    AMF_RETURN_IF_INVALID_POINTER(m_pComputeEngineUpdate);
#endif

    n_max_channels_ = _n_max_channels;
    n_sets_ = _n_sets;
    n_input_blocks_ = 2;
    block_sz_ = 1 << static_cast<uint>(ceil(log2((double)_max_proc_buffer_sz)));
    align_padding_sz_ = static_cast<int>(block_sz_) - _max_proc_buffer_sz;
    double_block_sz_ = 2 * block_sz_;
    num_blocks_ = static_cast<int>((_max_conv_sz + block_sz_ - 1) / block_sz_);
    round_counter_ = 0;
    max_conv_sz_ = num_blocks_ * block_sz_;
    aligned_proc_bufffer_sz_ = static_cast<int>(max_proc_buffer_sz_ = _max_proc_buffer_sz);

    //Setup the data formats
    if (use_hermitian)
        freq_block_num_elements_ = double_block_sz_ / 2 + 1;  //hermitian stores N/2 + 1 values as N/2-1 are duplicates
    else
        freq_block_num_elements_ = double_block_sz_;

    freq_block_sz_ = freq_block_num_elements_ * 2;  //size in number of floats
    max_conv_sz_freq_ = num_blocks_ * freq_block_sz_;
    const size_t one = 1;
    max_conv_sz_freq_aligned_ = one << static_cast<uint>(ceil(log2((double)max_conv_sz_freq_)));
    
    int ret = setupCL(
#ifndef TAN_SDK_EXPORTS
        _clientContext, _clientDevice, _clientQ
#endif
        );
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Internal error: graal's initialization failed");
    
    ret = setupCLFFT();
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Internal error: CLFFT's initialization failed");
    
    return 0;
}


int
CGraalConv_clFFT::setupCL(
#ifndef TAN_SDK_EXPORTS
    cl_context _clientContext,
    cl_device_id _clientDevice,
    cl_command_queue _clientQ
#endif
)
{
    int ret = GRAAL_SUCCESS;


#ifndef TAN_SDK_EXPORTS
    // need to be the first call to set the device, context in CGraalConvOCL
    graal::getGraalOCL().setupCL(_clientContext, _clientDevice, _clientQ);

    clientContext_ = graal::getGraalOCL().getClContext();
    cl_queue_properties prop[] = { 0 };
    clientQ_ = graal::getGraalOCL().getClQueue(prop, 0);
#endif

    clIRInputBuf.resize(n_max_channels_);
    clIRInputPaddedBuf.resize(n_max_channels_);
    clIRBlocksBuf.resize(n_max_channels_);
    roundCounter_.resize(n_max_channels_);

#ifdef TAN_SDK_EXPORTS
    cl_context clientContext_ =
        static_cast<cl_context>(m_pComputeEngineConvolution->GetNativeContext());
    cl_command_queue clientQ_ =
        static_cast<cl_command_queue>(m_pComputeEngineConvolution->GetNativeCommandQueue());
#  define CABufArgs clientContext_, m_pComputeEngineConvolution
#  define CAUpdBufArgs clientContext_, m_pComputeEngineUpdate
#else
#  define CABufArgs clientContext_
#  define CAUpdBufArgs clientContext_
#endif

    //allocate the giant 'base' buffers
    clIRBlocksBaseBuf = new CABuf<float>(CABufArgs);
    ret = clIRBlocksBaseBuf->create(max_conv_sz_freq_ * n_max_channels_ * n_sets_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clSignalHistBaseBuf = new CABuf<float>(CABufArgs);
    ret = clSignalHistBaseBuf->create(max_conv_sz_freq_ * n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    //This holds multiple versions of the input (n_input_blocks_ worth) as we need previous values for the convolution
    clInputBaseBuf = new CABuf<float>(CABufArgs);
    ret = clInputBaseBuf->create(block_sz_ * n_max_channels_ * n_input_blocks_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    //The input in frequency domain 
    clInputBlockBaseBuf = new CABuf<float>(CABufArgs);
    ret = clInputBlockBaseBuf->create(freq_block_sz_ * n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clOutputBaseBuf = new CABuf<float>(CABufArgs);
    ret = clOutputBaseBuf->create(freq_block_sz_ * n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clOutputComplexBaseBuf = new CABuf<float>(CABufArgs);
    ret = clOutputComplexBaseBuf->create(freq_block_sz_ * n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    // the maps are used for control in the kernel
    clChannelMap = new CABuf<int>(CABufArgs);
    ret = clChannelMap->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clSetMap = new CABuf<int>(CABufArgs);
    ret = clSetMap->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clCurrInputMap = new CABuf<int>(CABufArgs);
    ret = clCurrInputMap->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clPrevInputMap = new CABuf<int>(CABufArgs);
    ret = clPrevInputMap->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    clBlockNumberMap = new CABuf<int>(CABufArgs);
    ret = clBlockNumberMap->create(n_max_channels_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

    //used to track rounds
    CABuf<float>* round_counters_buf = new CABuf<float>(CABufArgs);
    ret = round_counters_buf->create(n_max_channels_ * n_sets_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");
    ret = round_counters_buf->setValue2(clientQ_, 0);
    AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer zero-filling failed");
    round_counters_ = round_counters_buf;

    for (uint ch = 0; ch < n_max_channels_; ch++)
    {
        clIRInputBuf[ch] = new CABuf<float>(CAUpdBufArgs);
        ret = clIRInputBuf[ch]->create(max_conv_sz_, 0);
        AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");

        clIRInputPaddedBuf[ch] = new CABuf<float>(CAUpdBufArgs);
        ret = clIRInputPaddedBuf[ch]->create(num_blocks_ * double_block_sz_, 0);
        AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");
        
        clIRBlocksBuf[ch].resize(n_sets_);
        roundCounter_[ch].resize(n_sets_);
        for (uint set = 0; set < n_sets_; set++)
        {
            clIRBlocksBuf[ch][set] = new CASubBuf<float>(*clIRBlocksBaseBuf);
            ret = clIRBlocksBuf[ch][set]->create(max_conv_sz_freq_*(set + ch*n_sets_),
                                                 max_conv_sz_freq_, 0); // double the size for padding, double again for complex numbers
            AMF_RETURN_IF_FALSE(ret == GRAAL_SUCCESS, ret, L"Buffer creation failed");
            
            roundCounter_[ch][set] = 0;
        }
    }

    //setup kernels
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(getOclKernel(padKernel_, m_pComputeEngineUpdate, "FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                      amdFFT_conv_kernelsCount, "amdPadFFTBlock", ""));
    m_copyWithPaddingKernel = padKernel_;
    AMF_RETURN_IF_FAILED(getOclKernel(interleaveKernel_, m_pComputeEngineUpdate, "FFT_conv_kernels",
                                      (const char*)amdFFT_conv_kernels, amdFFT_conv_kernelsCount,
                                      "amdInterleaveFFTBlock", ""));
    AMF_RETURN_IF_FAILED(getOclKernel(deinterleaveKernel_, m_pComputeEngineConvolution, "FFT_conv_kernels",
                                      (const char*)amdFFT_conv_kernels, amdFFT_conv_kernelsCount, 
                                      "amdDeinterleaveFFTBlock", ""));

    //These MuliChan kernels handle multiple channels at a time
    AMF_RETURN_IF_FAILED(getOclKernel(interleaveMultiChanKernel_, m_pComputeEngineConvolution, "FFT_conv_kernels",
                                      (const char*)amdFFT_conv_kernels, amdFFT_conv_kernelsCount, 
                                      "amdInterleaveFFTBlockMultiChan", ""));
    AMF_RETURN_IF_FAILED(getOclKernel(madaccMultiChanKernel_, m_pComputeEngineConvolution, "FFT_conv_kernels",
                                      (const char*)amdFFT_conv_kernels, amdFFT_conv_kernelsCount, 
                                      "amdMADAccBlocksMultiChan", ""));
    AMF_RETURN_IF_FAILED(getOclKernel(sigHistInsertMultiChanKernel_, m_pComputeEngineConvolution, "FFT_conv_kernels",
                                      (const char*)amdFFT_conv_kernels, amdFFT_conv_kernelsCount, 
                                      "amdSigHistoryInsertMultiChan", ""));
#else
    //padKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdPadFFTBlock", "");
    padKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels, 
                                                amdFFT_conv_kernelsCount, "amdPadFFTBlock", "");
    assert(padKernel_);
    //interleaveKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdInterleaveFFTBlock", "");
    interleaveKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                                       amdFFT_conv_kernelsCount, "amdInterleaveFFTBlock", "");
    assert(interleaveKernel_);
    //deinterleaveKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdDeinterleaveFFTBlock", "");
    deinterleaveKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                                         amdFFT_conv_kernelsCount, "amdDeinterleaveFFTBlock", "");
    assert(deinterleaveKernel_);

    //These MuliChan kernels handle multiple channels at a time
    //interleaveMultiChanKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdInterleaveFFTBlockMultiChan", "");
    interleaveMultiChanKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                                                amdFFT_conv_kernelsCount, "amdInterleaveFFTBlockMultiChan", "");
    assert(interleaveMultiChanKernel_);
    //madaccMultiChanKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdMADAccBlocksMultiChan", "");
    madaccMultiChanKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                                            amdFFT_conv_kernelsCount, "amdMADAccBlocksMultiChan", "");
    assert(madaccMultiChanKernel_);
    //sigHistInsertMultiChanKernel_ = graal::getGraalOCL().getKernel("amdFFT_conv_kernels.cl", "amdSigHistoryInsertMultiChan", "");
    sigHistInsertMultiChanKernel_ = graal::getGraalOCL().getKernel("FFT_conv_kernels", (const char*)amdFFT_conv_kernels,
                                                                   amdFFT_conv_kernelsCount, "amdSigHistoryInsertMultiChan", "");
    assert(sigHistInsertMultiChanKernel_);
#endif
    
    return 0;
}


int 
CGraalConv_clFFT::setupCLFFT()
{
    clfftStatus status;
    size_t double_block_sz_64 = double_block_sz_;

#ifdef TAN_SDK_EXPORTS
    status = clfftCreateDefaultPlan(
        &clfftPlanFwdIR, static_cast<cl_context>(m_pComputeEngineUpdate->GetNativeContext()),
                CLFFT_1D, &double_block_sz_64);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");

    status = clfftCreateDefaultPlan(
                &clfftPlanFwdAllChannels,
                static_cast<cl_context>(m_pComputeEngineConvolution->GetNativeContext()),
                CLFFT_1D, &double_block_sz_64);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");

    status = clfftCreateDefaultPlan(
                &clfftPlanBackAllChannels, 
                static_cast<cl_context>(m_pComputeEngineConvolution->GetNativeContext()),
                CLFFT_1D, &double_block_sz_64);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");
#else
    status = clfftCreateDefaultPlan(&clfftPlanFwdIR, clientContext_, CLFFT_1D, &double_block_sz_64);
    assert(status == CLFFT_SUCCESS);
    status = clfftCreateDefaultPlan(&clfftPlanFwdAllChannels, clientContext_, CLFFT_1D, &double_block_sz_64);
    assert(status == CLFFT_SUCCESS);
    status = clfftCreateDefaultPlan(&clfftPlanBackAllChannels, clientContext_, CLFFT_1D, &double_block_sz_64);
    assert(status == CLFFT_SUCCESS);
#endif

    if (use_hermitian)
    {
        clfftSetLayout(clfftPlanFwdIR, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
        clfftSetLayout(clfftPlanFwdAllChannels, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
        clfftSetLayout(clfftPlanBackAllChannels, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
    }
    else
    {
        clfftSetLayout(clfftPlanFwdIR, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        clfftSetLayout(clfftPlanFwdAllChannels, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        clfftSetLayout(clfftPlanBackAllChannels, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        //This is for the output and it is out-of-place just to use a second buffer to de-interleave the results
        clfftSetResultLocation(clfftPlanBackAllChannels, CLFFT_OUTOFPLACE);
    }

    //setup distance (how big a block is in memory) to do batching
    if (use_hermitian)
    {
        clfftSetPlanDistance(clfftPlanFwdIR, freq_block_num_elements_ * 2, freq_block_num_elements_); //the distance for real is block_sz_ + padding =  2*freq_block_num_elements (double because complex take twise the space)
    }
    else
    {
        clfftSetPlanDistance(clfftPlanFwdIR, freq_block_num_elements_, freq_block_num_elements_); //the distance for real is block_sz_ + padding =  2*freq_block_num_elements (double because complex take twise the space)
        clfftSetPlanDistance(clfftPlanFwdAllChannels, freq_block_num_elements_, freq_block_num_elements_); //the distance for real is block_sz_ + padding =  2*freq_block_num_elements (double because complex take twise the space)
        clfftSetPlanDistance(clfftPlanBackAllChannels, freq_block_num_elements_, freq_block_num_elements_); //the distance for real is block_sz_ + padding =  2*freq_block_num_elements (double because complex take twise the space)
    }
    clfftSetPlanBatchSize(clfftPlanFwdIR, num_blocks_);
    clfftSetPlanBatchSize(clfftPlanFwdAllChannels, n_max_channels_);
    clfftSetPlanBatchSize(clfftPlanBackAllChannels, n_max_channels_);

#ifdef TAN_SDK_EXPORTS
    cl_command_queue updContextArr[1] = {
        static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue())
    };
    cl_command_queue prcContextArr[1] = {
        static_cast<cl_command_queue>(m_pComputeEngineConvolution->GetNativeCommandQueue())
    };
    status = clfftBakePlan(clfftPlanFwdIR, _countof(updContextArr), updContextArr, NULL, NULL);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");

    status = clfftBakePlan(clfftPlanFwdAllChannels, _countof(prcContextArr), prcContextArr, NULL, NULL);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");

    status = clfftBakePlan(clfftPlanBackAllChannels, _countof(prcContextArr), prcContextArr, NULL, NULL);
    AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_OPENCL_FAILED, L"CLFFT failure");
#else
    status = clfftBakePlan(clfftPlanFwdIR, 1, &clientQ_, NULL, NULL);
    assert(status == CLFFT_SUCCESS);
    status = clfftBakePlan(clfftPlanFwdAllChannels, 1, &clientQ_, NULL, NULL);
    assert(status == CLFFT_SUCCESS);
    status = clfftBakePlan(clfftPlanBackAllChannels, 1, &clientQ_, NULL, NULL);
    assert(status == CLFFT_SUCCESS);
#endif

    return 0;
}

//print the first 'count' floats of a block
void CGraalConv_clFFT::printBlock(CABuf<float>* buf, std::string name, int blockLength, int offset, int count)
{
#ifdef _DEBUG_PRINTF
    if (!debug)
        return;
    float* data = buf->map(clientQ_, CL_MAP_READ);
    std::cout << "The values of " << name << " : ";
    for (int i = 0; i < count; i++)
    {
        std::cout << data[i+offset] << " ";
    }
    std::cout << "\n\tlast before mid ";
    for (int i = count; i > 0; i--)
    {
        std::cout << data[offset + blockLength/2 - i] << " ";
    }
    std::cout << "\n\tmid and after ";
    for (int i = 0; i < count; i++)
    {
        std::cout << data[offset + blockLength/2 + i] << " ";
    }
    std::cout << "\n\tlast members ";
    for (int i = count; i > 0; i--)
    {
        std::cout << data[offset + blockLength - i] << " ";
    }
    std::cout << std::endl;
    buf->unmap();
#endif
}

void CGraalConv_clFFT::printAllBlocks(CABuf<float>* buf, std::string name, int blockLength, int blockCount, int count)
{
#ifdef _DEBUG_PRINTF
    if (!debug)
        return;
    std::cout << "The values of " << name << std::endl;
    for (int block = 0; block < blockCount; block++)
    { 
        char str[64];
        sprintf(str, "block %d", block);
        printBlock(buf, str, blockLength, block*blockLength, count);
    }
#endif
}


void CGraalConv_clFFT::writeComplexToOctaveFile(CABuf<float>* buf, std::string filename, int blockLength)
{
    cl_command_queue updQueue =
        static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue());
    float* data = buf->map(updQueue, CL_MAP_READ);
    std::ofstream file;
    file.open(filename);
    for (int i = 0; i < blockLength; i += 2)
    {
        if (i != 0)
            file << ",";
        file << data[i] << " " << data[i+1] << "i";
    }	
    buf->unmap();
    file.close();
}


void CGraalConv_clFFT::writeToOctaveFile(CABuf<float>* buf, std::string filename, int blockLength)
{
    cl_command_queue updQueue =
        static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue());
    float* data = buf->map(updQueue, CL_MAP_READ);
    std::ofstream file;
    file.open(filename);
    for (int i = 0; i < blockLength; i += 1)
    {
        if (i != 0)
            file << ",";
        file << data[i];
    }
    buf->unmap();
    file.close();
}


int CGraalConv_clFFT::getConvBuffers(
    int n_channels,
    int *uploadIDs,             // upload set IDs per kernel
    int *convIDs,               // kernel IDs
    float** conv_ptrs           // gpu-frendly system pointers
    )
{ 
    cl_command_queue updQueue =
        static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue());

    for (int n = 0; n < n_channels; n++)
    {
        int set = uploadIDs[n];
        int ch = convIDs[n];

        conv_ptrs[n] = clIRInputBuf[ch]->mapA(updQueue, CL_MAP_WRITE_INVALIDATE_REGION);
    }
    
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
#else
    clFinish(clientQ_);
#endif

    return 0;
}


int CGraalConv_clFFT::getConvBuffers(
    int n_channels,
    int *uploadIDs,
    int *convIDs,
    cl_mem* ocl_bufffers
    )
{
    for (int n = 0; n < n_channels; n++)
    {
        int set = uploadIDs[n];
        int ch = convIDs[n];

        ocl_bufffers[n] = clIRInputBuf[ch]->getCLMem();
    }
    return 0;
}


int CGraalConv_clFFT::uploadConvHostPtrs(
    int n_channels,
    const int *uploadIDs,     // upload set IDs
    const int *convIDs,       // kernel IDs
    const float** conv_ptrs,  // arbitrary host ptrs
    const int * conv_lens,
    bool synchronous   // synchronous call	
    )
{
    cl_int ret;

    cl_command_queue updQueue =
        static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue());

    for (int n = 0; n < n_channels; n++)
    {
        int set = uploadIDs[n];
        int ch = convIDs[n];

        ret = clIRInputBuf[ch]->copyToDeviceNonBlocking(updQueue, CL_MEM_READ_ONLY, conv_ptrs[n],
                                                        conv_lens[n]);
        CHECK_OPENCL_ERROR(ret, "upload failed.");
    }

    if (synchronous)
    {
#ifdef TAN_SDK_EXPORTS
        AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
#else
        clFinish(clientQ_);
#endif
    }
    
    return 0;
}


int CGraalConv_clFFT::updateConv(
    int n_channels,
    const int *uploadIDs,
    const int *convIDs,
    const float** conv_ptrs,
    const int * conv_lens,
    bool synchronous
    )
{
    //same as updateConvHostPtrs except we need to unmap the inputs at some point and this is a good time
    for (int n = 0; n < n_channels; n++)
    {
        int set = uploadIDs[n];
        int ch = convIDs[n];
        clIRInputBuf[ch]->unmap();
    }

    return updateConvHostPtrs(n_channels, uploadIDs, convIDs, conv_ptrs, conv_lens, synchronous);
};


int CGraalConv_clFFT::updateConvHostPtrs(
    int n_channels,
    const int *uploadIDs,     // upload set IDs
    const int *convIDs,       // kernel IDs
    const float** conv_ptrs,  // arbitrary host ptrs
    const int * conv_lens,
    bool synchronous   // synchronous call	
    )
{ 
    processLock.Lock();

    // first copy IR from system to GPU [clIRInputBuf]
    uploadConvHostPtrs(n_channels, uploadIDs, convIDs, conv_ptrs, conv_lens, false);

    std::vector<cl_mem> ocl_mems;
    for (int n = 0; n < n_channels; n++)
    {
        int ch = convIDs[n];
        int set = uploadIDs[n];
        ocl_mems.push_back(clIRInputBuf[ch]->getCLMem());
    }

    updateConv(n_channels, uploadIDs, convIDs, &ocl_mems[0], conv_lens, synchronous);

    
    processLock.Unlock();
    return 0;
}


//using internal ocl_mems
int CGraalConv_clFFT::updateConv(
    int n_channels,
    const int *uploadIDs,     // upload set IDs
    const int *convIDs,       // kernel IDs
    const int * conv_lens,
    bool synchronous   // synchronous call	
    )
{
    processLock.Lock();

    std::vector<cl_mem> ocl_mems;
    for (int n = 0; n < n_channels; n++)
    {
        int ch = convIDs[n];
        int set = uploadIDs[n];
        ocl_mems.push_back(clIRInputBuf[ch]->getCLMem());
    }

    updateConv(n_channels, uploadIDs, convIDs, &ocl_mems[0], conv_lens, synchronous);

    processLock.Unlock();
    return 0;
}


int CGraalConv_clFFT::updateConv(
    int n_channels,
    const int *uploadIDs,     // upload set IDs
    const int *convIDs,       // kernel IDs
    const cl_mem* ocl_mems,
    const int * conv_lens,
    bool synchronous   // synchronoius call	
    )
{ 
    processLock.Lock();

    cl_int ret = CL_SUCCESS;
    clfftStatus fftstatus = CLFFT_SUCCESS;

    for (int n = 0; n < n_channels; n++)
    {
        int ch = convIDs[n];
        int set = uploadIDs[n];

        //1) copy data in with proper padding
        uint blockOffset = 0;
        uint padLength = 0;
        uint inputBufSize = 0;
        if (use_hermitian) {
            padLength = static_cast<uint>(freq_block_sz_ - block_sz_);
            inputBufSize = static_cast<uint>(clIRBlocksBuf[ch][set]->getLen());
        }
        else {
            //We pad in real values
            padLength = static_cast<uint>(double_block_sz_ - block_sz_);
            inputBufSize = static_cast<uint>(clIRInputPaddedBuf[ch]->getLen());
        }

        int n_arg = 0;
#ifdef TAN_SDK_EXPORTS
        AMF_RETURN_IF_FAILED(padKernel_->SetArgBufferNative(n_arg++, ocl_mems[n], amf::AMF_ARGUMENT_ACCESS_READWRITE));
        if (use_hermitian) {
            AMF_RETURN_IF_FAILED(padKernel_->SetArgBufferNative(n_arg++, clIRBlocksBuf[ch][set]->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        }
        else {
            AMF_RETURN_IF_FAILED(padKernel_->SetArgBufferNative(n_arg++, clIRInputPaddedBuf[ch]->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        }
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, blockOffset)); //in offset
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, max_conv_sz_)); //in length
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, blockOffset)); //out offset
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, inputBufSize));//out length
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, block_sz_));//block length
        AMF_RETURN_IF_FAILED(padKernel_->SetArgInt32(n_arg++, padLength));//pad length

        size_t l_wk[3] = { min(block_sz_, 256), 1, 1 };
        size_t g_wk[3] = { inputBufSize / 1, 1, 1 }; //divide by 4 as we process a vec4
        if (!use_hermitian) {
            g_wk[0] = num_blocks_ * double_block_sz_;
        }
        AMF_RETURN_IF_FAILED(padKernel_->Enqueue(1, NULL, g_wk, l_wk), L"Enqueue() failed");
#else
        ret = clSetKernelArg(padKernel_, n_arg++, sizeof(cl_mem), &ocl_mems[n]);
        if (use_hermitian)
            ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(cl_mem), &clIRBlocksBuf[ch][set]->getCLMem());
        else
            ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(cl_mem), &clIRInputPaddedBuf[ch]->getCLMem());
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &blockOffset); //in offset
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &max_conv_sz_); //in length
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &blockOffset); //out offset
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &max_conv_sz_freq_);//out length
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &block_sz_);//block length
        ret |= clSetKernelArg(padKernel_, n_arg++, sizeof(int), &padLength);//pad length
        CHECK_OPENCL_ERROR(ret, "parmeters failed.");

        size_t l_wk[3] = { min(block_sz_, 256), 1, 1 };
        size_t g_wk[3] = { max_conv_sz_freq_ / 1, 1, 1 }; //divide by 4 as we process a vec4
        if (!use_hermitian)
            g_wk[0] = num_blocks_ * double_block_sz_;
        ret = clEnqueueNDRangeKernel(clientQ_, padKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
        CHECK_OPENCL_ERROR(ret, "Running padKernel failed.");
#endif

        //We need to interleave the real data into complex format when we aren't using the Hermitian
        if (!use_hermitian)
        {
            n_arg = 0;
#ifdef TAN_SDK_EXPORTS
            AMF_RETURN_IF_FAILED(interleaveKernel_->SetArgBufferNative(n_arg++, clIRInputPaddedBuf[ch]->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
            AMF_RETURN_IF_FAILED(interleaveKernel_->SetArgBufferNative(n_arg++, clIRBlocksBuf[ch][set]->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
            g_wk[0] = num_blocks_ * double_block_sz_;

            AMF_RETURN_IF_FAILED(interleaveKernel_->Enqueue(1, NULL, g_wk, l_wk),
                                 L"Enqueue() failed");
#else
            ret = clSetKernelArg(interleaveKernel_, n_arg++, sizeof(cl_mem), &clIRInputPaddedBuf[ch]->getCLMem());
            ret |= clSetKernelArg(interleaveKernel_, n_arg++, sizeof(cl_mem), &clIRBlocksBuf[ch][set]->getCLMem());
            CHECK_OPENCL_ERROR(ret, "parmeters failed.");
            g_wk[0] = num_blocks_ * double_block_sz_;
            ret = clEnqueueNDRangeKernel(clientQ_, interleaveKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
            CHECK_OPENCL_ERROR(ret, "Running padKernel failed.");
#endif
        }
        printAllBlocks(clIRBlocksBuf[ch][set], "IR padded", freq_block_sz_);

        //2) Transform from time to frequency domain
        cl_command_queue updQueueArr[1] = {
            static_cast<cl_command_queue>(m_pComputeEngineUpdate->GetNativeCommandQueue())
        };
        cl_mem inBuf = clIRBlocksBuf[ch][set]->getCLMem();
        fftstatus = clfftEnqueueTransform(clfftPlanFwdIR,
            CLFFT_FORWARD,
            _countof(updQueueArr), /*num queues and out events*/
            updQueueArr, /*command queue*/
            0, /*num wait events*/
            NULL, /*wait events*/
            NULL, /*out event*/
            &inBuf,/*input buf*/
            NULL, /*output buf*/
            NULL /*tmp buffer*/);
        assert(fftstatus == CLFFT_SUCCESS);

        printAllBlocks(clIRBlocksBuf[ch][set], "IR in freq", freq_block_sz_);
    }
    
    if (synchronous)
    {
#ifdef TAN_SDK_EXPORTS
        AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
#else
        clFinish(clientQ_);
#endif
    }

    processLock.Unlock();

    return 0;
}


int CGraalConv_clFFT::finishUpdate(void)
{ return 0;}


#ifdef TAN_SDK_EXPORTS
/**
* Responce copying utility function.
*
* @return AMF_OK on success.
*/
AMF_RESULT CGraalConv_clFFT::copyResponses(
    uint n_channels,
    const uint pFromUploadIds[],
    const uint pToUploadIds[],
    const uint pConvIds[],
    const bool synchronous
    )
{
    for (amf_uint32 channelId = 0; channelId < n_channels; channelId++)
    {
        AMF_RETURN_IF_FALSE(pFromUploadIds[channelId] < n_sets_, AMF_INVALID_ARG,
                            L"pFromUploadIds[channelId] out of range");
        AMF_RETURN_IF_FALSE(pToUploadIds[channelId] < n_sets_, AMF_INVALID_ARG,
                            L"pToUploadIds[channelId] out of range");
        AMF_RETURN_IF_FALSE(pConvIds[channelId] < n_max_channels_, AMF_INVALID_ARG,
                            L"pConvIds[channelId] out of range");

        const amf_uint32 inOffset =
            (pConvIds[channelId] * n_sets_ + pFromUploadIds[channelId]) * max_conv_sz_freq_;
        const amf_uint32 outOffset =
            (pConvIds[channelId] * n_sets_ + pToUploadIds[channelId]) * max_conv_sz_freq_;
        const amf_uint32 len = static_cast<amf_uint32>(clIRBlocksBaseBuf->getLen());

        amf_size n_arg = 0;
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgBufferNative(n_arg++,
            clIRBlocksBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgBufferNative(n_arg++,
            clIRBlocksBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, inOffset)); //in offset
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, len)); //in length
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, outOffset)); //out offset
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, len));//out length
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, max_conv_sz_freq_));//block length
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->SetArgInt32(n_arg++, 0));//pad length

        size_t l_wk[3] = { min(max_conv_sz_freq_, 256), 1, 1 };
        size_t g_wk[3] = { max_conv_sz_freq_, 1, 1 }; //divide by 4 as we process a vec4
        AMF_RETURN_IF_FAILED(m_copyWithPaddingKernel->Enqueue(1, NULL, g_wk, l_wk),
            L"Enqueue() failed");
    }

    if (synchronous)
    {
        AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FinishQueue(), L"FinishQueue() failed");
    }

    return AMF_OK;
}
#endif // TAN_SDK_EXPORTS


int CGraalConv_clFFT::process(
    int n_channels,
    const int *uploadID,     // upload set IDs
    const int *convIDs,       // kernel IDs
    float** inputs,
    float** outputs,
    int prev_input,
    int advance_time,
    int skip_stage
    )
{
    processLock.Lock();
    
    clfftStatus fftstatus;
    int inputset = 0;//for the roundCounter as the input is not set dependent
    int n_arg;
    size_t l_wk[3] = { min(block_sz_, 256), 1, 1 };
    size_t g_wk[3] = { 1, 1, 1 };

    int inspectCh = 1;

    //setup maps used to control kernels
#ifdef TAN_SDK_EXPORTS
    cl_command_queue clientQ_ =
        static_cast<cl_command_queue>(m_pComputeEngineConvolution->GetNativeCommandQueue());
#endif

    int* ChannelMap = clChannelMap->map(clientQ_, CL_MAP_WRITE);
    int* SetMap = clSetMap->map(clientQ_, CL_MAP_WRITE);
    int* CurrInputMap = clCurrInputMap->map(clientQ_, CL_MAP_WRITE);
    int* PrevInputMap = clPrevInputMap->map(clientQ_, CL_MAP_WRITE);
    int* BlockNumberMap = clBlockNumberMap->map(clientQ_, CL_MAP_WRITE);
    for (int n = 0; n < n_channels; n++)
    {
        int ch = convIDs[n];
        int set = uploadID[n];
        int input_index_prev = (getRoundCounter(inputset, ch) + n_input_blocks_ - 1) % n_input_blocks_;
        int input_index_curr = getRoundCounter(inputset, ch) % n_input_blocks_;
        int blockNumber = getRoundCounter(inputset, ch) % num_blocks_;

        ChannelMap[n] = ch;
        SetMap[n] = set;
        CurrInputMap[n] = input_index_curr;
        PrevInputMap[n] = input_index_prev;
        BlockNumberMap[n] = blockNumber;
    }
    clChannelMap->unmap();
    clSetMap->unmap();
    clCurrInputMap->unmap();
    clPrevInputMap->unmap();
    clBlockNumberMap->unmap();


    //1 copy all the inputs onto the GPU together, place in the 'current' slot in the clInputBaseBuf
    float *input_buf_ptr = clInputBaseBuf->map(clientQ_, CL_MAP_WRITE);
    for (int n = 0; n < n_channels; n++)
    {
        int ch = convIDs[n];
        int input_index_curr = getRoundCounter(inputset, ch) % n_input_blocks_;
        int input_index_prev = (getRoundCounter(inputset, ch) + n_input_blocks_ - 1) % n_input_blocks_;

        float *in_ptr = inputs[n];
        float *upload_ptr = input_buf_ptr + (ch * n_input_blocks_ + input_index_curr) * block_sz_;
        memcpy(upload_ptr, in_ptr, max_proc_buffer_sz_ * sizeof(float));
        memset(upload_ptr + max_proc_buffer_sz_, 0, align_padding_sz_ * sizeof(float));

    }
    clInputBaseBuf->unmap();

    printAllBlocks(clInputBaseBuf, "input with prev/curr", double_block_sz_, n_channels);

    //2 Create a new block in InputBlockBase where the first half is the previous input and second half is current
    n_arg = 0;
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgBufferNative(n_arg++, clInputBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgBufferNative(n_arg++, clInputBlockBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgBufferNative(n_arg++, clChannelMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgBufferNative(n_arg++, clCurrInputMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgBufferNative(n_arg++, clPrevInputMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgInt32(n_arg++, freq_block_sz_));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgInt32(n_arg++, block_sz_));
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->SetArgInt32(n_arg++, n_input_blocks_));

    g_wk[0] = freq_block_sz_ * n_channels / 2;
    AMF_RETURN_IF_FAILED(interleaveMultiChanKernel_->Enqueue(1, NULL, g_wk, l_wk),
                         L"Enqueue() failed");
#else
    cl_int ret;
    ret = clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(cl_mem), &clInputBaseBuf->getCLMem());
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(cl_mem), &clInputBlockBaseBuf->getCLMem());
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(cl_mem), &clChannelMap->getCLMem());
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(cl_mem), &clCurrInputMap->getCLMem());
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(cl_mem), &clPrevInputMap->getCLMem());
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(int), &freq_block_sz_);
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(int), &block_sz_);
    ret |= clSetKernelArg(interleaveMultiChanKernel_, n_arg++, sizeof(int), &n_input_blocks_);
    CHECK_OPENCL_ERROR(ret, "parmeters failed.");
    g_wk[0] = freq_block_sz_ * n_channels / 2;
    ret = clEnqueueNDRangeKernel(clientQ_, interleaveMultiChanKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(ret, "Running padKernel failed.");

    printAllBlocks(clInputBlockBaseBuf, "input interleaved ready for FFT", freq_block_sz_, n_channels);
#endif

    //3 FFT the whole InputBlockBase
    cl_mem inBuf = clInputBlockBaseBuf->getCLMem();
    fftstatus = clfftEnqueueTransform(clfftPlanFwdAllChannels,
        CLFFT_FORWARD,
        1, /*num queues and out events*/
        &clientQ_, /*command queue*/
        0, /*num wait events*/
        NULL, /*wait events*/
        NULL, /*out event*/
        &inBuf,
        NULL, /*output buf*/
        NULL /*tmp buffer?*/);
    assert(fftstatus == CLFFT_SUCCESS);

    printAllBlocks(clInputBlockBaseBuf, "input after FFT", freq_block_sz_, n_channels);

    //4 copy the FFTed input to the correct place in the Signal History
    //store new block into history block chain in increasing block number based on roundCounter
    n_arg = 0;
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgBufferNative(n_arg++, clInputBlockBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgBufferNative(n_arg++, clSignalHistBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgBufferNative(n_arg++, clChannelMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgBufferNative(n_arg++, clBlockNumberMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgInt32(n_arg++, freq_block_sz_));
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->SetArgInt32(n_arg++, num_blocks_));

    g_wk[0] = freq_block_sz_;
    g_wk[1] = n_channels;
    AMF_RETURN_IF_FAILED(sigHistInsertMultiChanKernel_->Enqueue(2, NULL, g_wk, l_wk),
                         L"Enqueue() failed");
#else
    ret = clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(cl_mem), &clInputBlockBaseBuf->getCLMem());
    ret |= clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(cl_mem), &clSignalHistBaseBuf->getCLMem());
    ret |= clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(cl_mem), &clChannelMap->getCLMem());
    ret |= clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(cl_mem), &clBlockNumberMap->getCLMem());
    ret |= clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(int), &freq_block_sz_);
    ret |= clSetKernelArg(sigHistInsertMultiChanKernel_, n_arg++, sizeof(int), &num_blocks_);
    CHECK_OPENCL_ERROR(ret, "sigHistInsertMultiChanKernel_ parmeters failed.");

    g_wk[0] = freq_block_sz_;
    g_wk[1] = n_channels;
    ret = clEnqueueNDRangeKernel(clientQ_, sigHistInsertMultiChanKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(ret, "Running sigHistInsertMultiChanKernel_ failed.");
#endif

    //5 Convolve by multiplying and accumulating the the Signal History with the IR
    n_arg = 0;
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clSignalHistBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clIRBlocksBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clOutputBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clChannelMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clSetMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgBufferNative(n_arg++, clBlockNumberMap->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgInt32(n_arg++, freq_block_num_elements_)); //blockLength
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgInt32(n_arg++, num_blocks_)); //maxBlockNumber
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->SetArgInt32(n_arg++, n_sets_));

    g_wk[0] = freq_block_num_elements_;
    g_wk[1] = n_channels;
    AMF_RETURN_IF_FAILED(madaccMultiChanKernel_->Enqueue(2, NULL, g_wk, l_wk), L"Enqueue() failed");
#else
    ret = clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clSignalHistBaseBuf->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clIRBlocksBaseBuf->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clOutputBaseBuf->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clChannelMap->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clSetMap->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(cl_mem), &clBlockNumberMap->getCLMem());
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(int), &freq_block_num_elements_); //blockLength
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(int), &num_blocks_); //maxBlockNumber
    ret |= clSetKernelArg(madaccMultiChanKernel_, n_arg++, sizeof(int), &n_sets_);
    CHECK_OPENCL_ERROR(ret, "parmeters failed.");

    g_wk[0] = freq_block_num_elements_;
    g_wk[1] = n_channels;
    ret = clEnqueueNDRangeKernel(clientQ_, madaccMultiChanKernel_, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(ret, "Running padKernel failed.");
#endif

#ifndef TAN_SDK_EXPORTS
        clFinish(clientQ_);

#  ifdef _DEBUG_PRINTF
        printAllBlocks(clSignalHistBaseBuf, "Signal history for ch 0", freq_block_sz_, 3);
        printAllBlocks(clIRBlocksBaseBuf, "IR for ch 0", freq_block_sz_, 3);
        printf("\nFor channel %d\n", inspectCh);
        printBlock(clOutputBaseBuf, "Filtered in freq", freq_block_sz_, inspectCh * freq_block_sz_);
#  endif
    }
#endif

    //6 inverse FFT the results of the convolution back to the time domain
    inBuf = clOutputBaseBuf->getCLMem();
    cl_mem outBuf = clOutputComplexBaseBuf->getCLMem();
    fftstatus = clfftEnqueueTransform(clfftPlanBackAllChannels,
        CLFFT_BACKWARD,
        1, /*num queues and out events*/
        &clientQ_, /*command queue*/
        0, /*num wait events*/
        NULL, /*wait events*/
        NULL, /*out event*/
        &inBuf,
        use_hermitian ? NULL : &outBuf, /* hermitian does it in place*/
        NULL /*tmp buffer?*/);
    assert(fftstatus == CLFFT_SUCCESS);

    printBlock(clOutputComplexBaseBuf, "Filtered in time", block_sz_, inspectCh * block_sz_);

    //7 de-interleave the complex
    n_arg = 0;
#ifdef TAN_SDK_EXPORTS
    AMF_RETURN_IF_FAILED(deinterleaveKernel_->SetArgBufferNative(n_arg++, clOutputComplexBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));
    AMF_RETURN_IF_FAILED(deinterleaveKernel_->SetArgBufferNative(n_arg++, clOutputBaseBuf->getCLMem(), amf::AMF_ARGUMENT_ACCESS_READWRITE));

    g_wk[0] = double_block_sz_ * n_channels;
    g_wk[1] = 1;
    AMF_RETURN_IF_FAILED(deinterleaveKernel_->Enqueue(1, NULL, g_wk, l_wk), L"Enqueue() failed");
#else
    ret = clSetKernelArg(deinterleaveKernel_, n_arg++, sizeof(cl_mem), &clOutputComplexBaseBuf->getCLMem());
    ret |= clSetKernelArg(deinterleaveKernel_, n_arg++, sizeof(cl_mem), &clOutputBaseBuf->getCLMem());
    CHECK_OPENCL_ERROR(ret, "parmeters failed.");
    g_wk[0] = double_block_sz_ * n_channels;
    g_wk[1] = 1;
    ret = clEnqueueNDRangeKernel(clientQ_, deinterleaveKernel_, 1, NULL, g_wk, l_wk, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(ret, "Running padKernel failed.");

    printBlock(clOutputBaseBuf, "Filtered in time deinterleaved", double_block_sz_, inspectCh * double_block_sz_);
#endif

    //8 copy from the GPU back to the sytem ouputs
    float * clOutMem = clOutputBaseBuf->map(clientQ_, CL_MAP_READ);
    for (uint n = 0; n < static_cast<uint>(n_channels); n++)
    {
        const uint ch = convIDs[n];
        const uint set = uploadID[n];
    
        float * sysOutMem = outputs[n];
        memcpy(sysOutMem, clOutMem + n * double_block_sz_ + block_sz_,
               max_proc_buffer_sz_ * sizeof(float));

        // NEXT ROUND !!!
        if (advance_time)
            incRoundCounter(inputset, ch);
    }
    clOutputBaseBuf->unmap();

    processLock.Unlock();

    return 0;
}

int CGraalConv_clFFT::flush(amf_uint channelId, const bool synchronous)
{
    AMF_RETURN_IF_FALSE(channelId < static_cast<amf_uint>(n_max_channels_), AMF_INVALID_ARG,
                        L"channelId out of range");

    int ret = GRAAL_SUCCESS;

    AMF_RETURN_IF_FAILED(zeroMemory(clSignalHistBaseBuf, channelId * max_conv_sz_freq_,
                                    max_conv_sz_freq_));
    AMF_RETURN_IF_FAILED(zeroMemory(clInputBaseBuf, channelId * block_sz_ * n_input_blocks_,
                                    block_sz_ * n_input_blocks_));

    if (synchronous)
    {
        AMF_RETURN_IF_FAILED(m_pComputeEngineUpdate->FlushQueue(), L"FlushQueue() failed");
    }

    return ret;
}

int CGraalConv_clFFT::processHostPtrs(
    int n_channels,
    int uploadIDs,     // upload set IDs
    const int *convIDs,       // kernel IDs
    float** inputs,
    float** outputs
    )
{ return 0;}
}