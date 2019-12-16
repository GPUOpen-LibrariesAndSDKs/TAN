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
#include "Graal.h"
#  include "tanlibrary/include/TrueAudioNext.h"
#include "public/common/AMFFactory.h"    
#define __MAX_VST_BLOCKS__  2048
#define __DEFAULT_STREAM_DIR__ ".\\"

typedef struct _ProjPlan ProjPlan;



class GraalWrapper {
    friend class UpdateThread;
public:
    GraalWrapper() { m_handle = 0;  }
    ~GraalWrapper();
	void terminate();
    /*
    * Wraps the ReverbDriverInit graalHandle* new_plan, int sample_rate,int block_size, int n_channels, int subchannels, int verification
    */
    int Init(
        amf::TANContextPtr& pContextTAN,
        amf::AMFComputePtr& pConvolution,
        amf::AMFComputePtr& pUpdate,
        int n_max_channels,
        int max_conv_kernel_size,
        int buffer_size,
        int number_ir_buffer_sets);

    /*
    * Wraps the graalReverbSetupReverbKernelFromFile
    */
    bool ready4IrUpdate();
    int UploadOCLIRs( int _n_channels, const cl_mem * _ir_buf_ptrs,  const int*  _ir_lens, int* channel_ids, unsigned int _ir_version);
    int UploadHostIRs( int _n_channels,  float ** _ir_buf_ptrs, const int * _ir_lens, int* _channel_ids, unsigned int _ir_version);
    /*
    * wraps graalReverbProcessing
    */
    int process(__FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int xfade_state);
    int updateConv(int numChans, unsigned int _ir_version, int* _channel_ids);
    int copyResponses(int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids);
    int flush(int channelId);
private:
    int ReverbDriverInit(graalHandle * new_plan, int block_size, int n_channels, int subchannels, int verification);

    graalHandle m_handle;
    amf::TANContextPtr m_pContextTAN;
    amf::AMFComputePtr m_amfComputeConv;
    amf::AMFComputePtr m_amfComputeUpdate;
    class OCLIRBuff {
    public:
		OCLIRBuff() { m_initialized = false; }
        int init(graalHandle handle, int init_flags, cl_context  context, int numChaneels, int kernelSize);
        int copy(int _n_channels,
                  const cl_mem * _conv_ptrs,  // arbitrary cl_mem ptrs
                  const int*  _conv_lens, 
                  int* channel_ids);
        ~OCLIRBuff();
        int clearBuffer();
        int getNumChannels() { return n_active_channels; }
        cl_mem* getClMemBuffs() { return conv_ptrs; }
        cl_mem getUnifiedClMemBuff() { return unified_conv_ptr; }
        int* getConvLens() { return conv_lens; }
        void sync() { clFinish(queue); }
        int getMaxIRLength() { return max_kernel_length; }
        int getChannelSampleSize() { return singleBufferSize/sizeof(cl_float); }
    private:
		bool m_initialized;
        int n_active_channels;
        int max_num_channels;
        cl_mem * conv_ptrs;
        cl_mem unified_conv_ptr;
        int*  conv_lens;
        int max_kernel_length;
        int singleBufferSize;
        int unifiedBufferSize;
        const static int TEMP_BUFF_SZ = 8;
        float tempBuff[TEMP_BUFF_SZ];
        cl_command_queue queue;
    } m_oclIrBuff;

    int n_kernel_samples; 

    // The flags
    int FHT_2streams = __INIT_FLAG_2STREAMS__; // To go uniform just change to 0
    int heterogen = 0; //__INIT_FLAG_HETEROGEN__;
    int fht = __INIT_FLAG_FHT__;
    int fft = 0; //__INIT_FLAG_FFT__
    int fir = 0; // __INIT_FLAG_FIR__
    int verification = 0; //__INIT_FLAG_VER_TRANSFORM1__; // | __INIT_FLAG_VER_TRANSFORM2__;
};

