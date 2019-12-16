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


#ifndef GRAALCONV_H_
#define GRAALCONV_H_

//Header Files
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <map>

#ifdef WIN32

#include <io.h>
#include <windows.h>
#include <BaseTsd.h>

#include "public/common/thread.h"

#  include "tanlibrary/include/TrueAudioNext.h"

#  include "public/include/core/Compute.h"
#  include "public/include/core/Context.h"

//#  include "public/common/TraceAdapter.h"
//#  define AMF_FACILITY L"GraalConv"


typedef unsigned int uint;

static
double mach_absolute_time()
{
    double ret = 0;
    int64_t frec;
    int64_t clocks;
    QueryPerformanceFrequency((LARGE_INTEGER *)&frec);
    QueryPerformanceCounter((LARGE_INTEGER *)&clocks);
    ret = (double)clocks * 1000. / (double)frec;
    return(ret);
}

#else

#if defined(__APPLE__) || defined(__MACOSX)
#include <mach/mach_time.h>
#endif

#include<strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>

#endif

#include "public/common/TraceAdapter.h"
#define AMF_FACILITY L"GraalConv"

#include "GraalConvOCL.hpp"

#define __FLOAT__ float
typedef unsigned int uint;

static double subtractTimes(double endTime, double startTime)
{
    double difference = endTime - startTime;
    static double conversion = 0.0;

    if (conversion == 0.0)
    {
#if __APPLE__
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info(&info);

        //Convert the timebase into seconds
        if (err == 0)
            conversion = 1e-9 * (double)info.numer / (double)info.denom;
#endif
#ifdef _WIN32
        conversion = 1.;
#endif
    }

    return conversion * (double)difference;
}


/**
 */
namespace graal
{
// implemented pipelines
    enum GRAAL_ALG {
// selected by Graal
        ALG_ANY,
// uniform "classical" pipeline:
// direct transform ->MAD->inverse transform
        ALG_UNIFORMED,
// uniform, spltted into 2 stages: head and tail
// 1st stage done in a curret round and returns the filtered block
// direct transform-> MAD (with the 0th conv block) + tail sum->inverse transform
// 2nd stage is done in background, bulids the "tail" sum.
// MAD (with all conv blocks except 0th)
        ALG_UNI_HEAD_TAIL,
		ALG_USE_PROCESS_FINALIZE = 0x8000 // use ProcessFinalize() optimization for HEAD_TAIL mode   
	};

class CGraalConv
{

    public:
    /**
     * Constructor
     * Initialize member variables
     */
     CGraalConv(void);

    /**
     * Destructor
     * @param name name of sample (string)
     */

     virtual ~CGraalConv(void);
    /**
     * Allocate and initialize convolution class
     *
     * @param n_max_channels		max number of channels tp be processed
     * @param max_conv_sz			max number of samples of IR
     * @param max_proc_buffer_sz	max size of the input buffers to be proccessed
     * @param n_upload_sets			max number of IR versions per 1 channnel (for double buffeering)
     * @param algorithm				algorthm to be used (see enum above)
     * @param clientContext			user defined context (if this parameter is not 0, next 2 should be non-0 also)
     * @param clientDevice			user defined device
     * @param clientQ				user defined queue
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
     virtual int initializeConv(
#ifdef TAN_SDK_EXPORTS
         amf::TANContextPtr &pContextTAN,
         amf::AMFComputePtr &pConvolution,
         amf::AMFComputePtr &pUpdate,
#endif
         int n_max_channels,
         int max_conv_sz,
         int max_proc_buffer_sz,
         int n_upload_sets = 2,         // number of shadow buffers for double buffering
         int algorithm = ALG_ANY
#ifndef TAN_SDK_EXPORTS
         ,
         cl_context clientContext = 0,
         cl_device_id clientDevice = 0,
         cl_command_queue clientQ = 0
#endif
         );

    /**
     * Returns a set of gpu_friendly system pointers.
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param conv_ptrs					gpu-frendly system pointers
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int getConvBuffers(
        int n_channels,				// number of channels
        int *uploadIDs,             // upload set IDs per kernel
        int *convIDs,               // kernel IDs
        float** conv_ptrs           // gpu-frendly system pointers
        );

    /**
     * Returns a array of library-managed OCL buffers
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param ocl_bufffers				Graal library-managed ocl-buffers
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int getConvBuffers(
        int n_channels,				// number of channels
        int *uploadIDs,             // upload set IDs per kernel
        int *convIDs,               // kernel IDs
        cl_mem* ocl_bufffers           // library-managed OCL buffers
        );


    /**
     * Upload kernels from a previously acquired gpu-friendly system pointers.
     * Pointers become invalid after the call.
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param conv_ptrs					gpu-frendly system pointers
     * @param conv_lens					length of the IR per channel
     * @param synchronous				if 0, the call is asynchronous and returns immideatly,
     *									if 1, it's an asynchronous call and it returns only after the IR is ready for use.
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int updateConv(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        const float** conv_ptrs,
        const int * conv_lens,
        bool synchronous = false   // synchronoius call
        );

    /**
     * Upload kernels from arbitrary system pointers.
     * It's the slowest upload.
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param conv_ptrs					arbitrary system pointers
     * @param conv_lens					length of the IR per channel
     * @param synchronous				if 0, the call is asynchronous and returns immideatly,
     *									if 1, it's an asynchronous call and it returns only after the IR is ready for use.
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int updateConvHostPtrs(
        int n_channels,
        const int *uploadIDs,
        const int *convIDs,
        const float** conv_ptrs,
        const int * conv_lens,
        bool synchronous = false
        );


    /**
     * Upload kernels from client-managed OCL buffers.
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param ocl_buffers				client managed OCL buffers
     * @param conv_lens					length of the IR per channel
     * @param synchronous				if 0, the call is asynchronous and returns immideatly,
     *									if 1, it's an asynchronous call and it returns only after the IR is ready for use.
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int updateConv(
        int n_channels,
        const int *uploadIDs,
        const int *convIDs,
        const cl_mem* ocl_buffers,
        const int * conv_lens,
        bool synchronous = false
        );


    /**
     * Upload kernels from library-managed OCL buffers.
     * this is the fastest upload.
     * ocl buffers has to be obtain with the getConvBuffers interface.
     *
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param conv_lens					length of the IR per channel
     * @param synchronous				if 0, the call is asynchronous and returns immideatly,
     *									if 1, it's an asynchronous call and it returns only after the IR is ready for use.
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int updateConv(
        int _n_channels,
        const int *_uploadIDs,
        const int *_convIDs,
        const int * _conv_lens,
        bool synchronous = false
        );

    /**
     * All previously uploaded IRs will be ready for use upon the return from the call
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
     virtual int finishUpdate(void);

#ifdef TAN_SDK_EXPORTS
     /**
      * Blocks until all the currently submitted processing tasks are done.
      *
      * @return AMF_OK on success.
      */
     virtual AMF_RESULT finishProcess(void)
     {
        cl_int status = CL_SUCCESS;
        status = clFlush(m_pContextTAN->GetOpenCLConvQueue());
        AMF_RETURN_IF_CL_FAILED(status, L"failed: finishProcess clFlush" );
        status = clFinish(m_pContextTAN->GetOpenCLConvQueue());
        AMF_RETURN_IF_CL_FAILED(status, L"failed: finishProcess clFinish" );
        return AMF_OK;
     }

     /**
      * Response copying utility function.
      *
      * @return AMF_OK on success.
      */
     virtual AMF_RESULT copyResponses(
         uint n_channels,
         const uint pFromUploadIds[],
         const uint pToUploadIds[],
         const uint pConvIds[],
         const bool synchronous = true
         );
#endif

// more efficient dma copy
#define COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK 1

#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
     virtual bool checkForContiguousBuffers(
         int count,
         const float** _conv_ptrs,
         const int * _conv_lens
         );
#endif

    /**
     * Process audio blocks from arbitrary system pointers.
     *
     * @param n_channels				number of channels in the request
     * @param *uploadIDs				version id
     * @param *convIDs					channel id
     * @param inputs					input pointers (size of buffers must be eaqual to max_proc_buffer_sz)
     * @param outputs					output pointer (size of buffers must be eaqual to max_proc_buffer_sz)
     * @param prev_input				1 - reuse previous input, 0 - process curent input (used in "mix")
     * @param advance_time				1 - advance internal round counter, 0 - do not advance (used in "mix")
     * @param skip_stage				1 - skip the 1st stager, 2 - skipo 2nd stage, 0 - full algorithm (used in "mix")
     *
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */

     virtual int process(
        int n_channels,
        const int *uploadID,     // upload set IDs
        const int *convIDs,       // kernel IDs
        float** inputs,
        float** outputs,
        int prev_input = 0,
        int advance_time = 1,
        int skip_stage = 0,
        int _crossfade_state = 0
        );

    /**
    *
    * @param n_channels				number of channels in the request
    * @param *uploadIDs				version id
    * @param *convIDs				channel id
    * @param inputs					input pointers (size of buffers must be eaqual to max_proc_buffer_sz)
    * @param outputBuf				pointers to the client-managed OCL buffers.
    * @param prev_input				1 - reuse previous input, 0 - process curent input (used in "mix")
    * @param advance_time			1 - advance internal round counter, 0 - do not advance (used in "mix")
    * @param skip_stage				1 - skip the 1st stager, 2 - skipo 2nd stage, 0 - full algorithm (used in "mix")
    *
    * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
    */
    virtual int process(
        int n_channels,
        const int *uploadID,     // upload set IDs
        const int *convIDs,       // kernel IDs
        float** inputs,
        cl_mem* outputBuf,
        int prev_input = 0,
        int advance_time = 1,
        int skip_stage = 0,
        int _crossfade_state = 0
        );

	virtual AMF_RESULT processFinalize();

     /**
      * Flushes history.
      */
     virtual int flush(amf_uint channelId, const bool synchronous = true);


    /*************************************************************************************************
    * emulation/verification helper interfaces
    *************************************************************************************************/


    virtual int getRoundCounter(int _uploadId = -1, int _chnl_id = -1);



    virtual int uploadConvHostPtrs(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        const float** conv_ptrs,  // arbitrary host ptrs
        const int * conv_lens,
        bool synchronous = false   // synchronous call
        );

    virtual int uploadConvGpuPtrs(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        const cl_mem * conv_ptrs,  // arbitrary host ptrs
        const int * conv_lens,
        bool synchronous = false   // synchronous call
        );



    inline int getInputBlockSz(void)
    {
        return(aligned_proc_bufffer_sz_);
    }


    /*************************************************************************************************
    * not implemented
    *************************************************************************************************/

    /**
    * Acquires gpu-friendly system pointers.
    * Pointers become invalid after the call.
    *
    * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
    */

    int getDevInputPtrs(
        int n_channels,				// # of channels
        int uploadID,				// upload set ID
        const int *convIDs,       // kernel IDs
        float** inputs			// array of gpu-frendly system pointers
        );

    /**
    * Process audio blocks from gpu_friendly pointers.
    *
    * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
    */

    int processDevPtrs(
        int n_channels,
        int uploadID,     // upload set ID
        const int *convIDs,       // kernel IDs
        float** inputs,
        float** outputs
        );


protected:
#ifdef TAN_SDK_EXPORTS
    amf::TANContextPtr          m_pContextTAN;

    virtual AMF_RESULT zeroMemory(CABuf<float> *pBuf, amf_uint offset, amf_uint amount);
#endif

    void incRoundCounter(int _uploadId = -1, int _chnl_id = -1);

    /**
     * OpenCL related initialisations.
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
    int setupCL( amf::AMFComputePtr pComputeConvolution,  amf::AMFComputePtr pComputeUpdate );




    /**
     * Cleanup
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
    int cleanup();

    /*
        kernel source files, kernel names, build options
    */
    void selectOptions(std::string & _kernel_file, std::string & _comp_options);
    int selectUploadOptions( std:: string & kernel_file, std::string &kernel_src, size_t &kernel_src_size, std:: string& kernel_name, std::string & comp_options);
    int selectUpload2Options(std::string & kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& kernel_name, std::string & comp_options);
    int selectDirectFHTOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options);
    int selectFHT_CMADOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::vector<std::string> & _kernel_name, std::string & _comp_options);
    int selectInverseFHTOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options);
    int selectResetOptions(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options);
    int selectConvHead1Options(std::string & _kernel_file, std::string &kernel_src, size_t &kernel_src_size, std::string& _kernel_name, std::string & _comp_options);



    /*
     advance round counterd
    */

    int resetConvState(
                size_t n_channels,
                const int *uploadIDs,
                const int *convIDs,
                int time_step
            );
    /*
        upload control maps
    */
    int uploadConvControlMaps(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        const int * conv_lens
        );


    /**
        upload in 1 shot, Graal managed OCL buffers
    */
    int updateConvOCL(void * stg_buf, void * transf_buf, int conv_len, cl_command_queue uploadQ, int uploadID, int convID);

    /*
        upload in a loop
    */
    int updateConvIntnl(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        const int *conv_lens,
        bool _synchronous   // synchronoius call
        );

    /*
        uitility, upload time domain data
    */
    int uploadConvHostPtrIntnl(
        int _n_channels,
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        const float** _conv_ptrs,  // arbitrary host ptrs
        const int * _conv_lens
        );

    enum GRAAL_MEMORY_TYPE
    {
        GRAAL_MEMORY_UNKNOWN = 0,
        GRAAL_MEMORY_HOST = 1,
        GRAAL_MEMORY_OPENCL = 2
    };

    struct GraalSampleBuffer {
        union {
            float ** host;
            cl_mem* clmem;
        } buffer;
        GRAAL_MEMORY_TYPE mType;
    };

    /**
    * internal processing routine
    *
    * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
    */

    int processIntrnl(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        float** inputs,
        GraalSampleBuffer& output,
        int prev_input = 0,
        int advance_time = 1,
        int skip_stage = 0,
        int _crossfade_state = 0
        );

    /*
        head-tail, head stage
    */

    int processHead1(
        int _n_channels,
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        int prev_input = 0,
        int advance_time = 1,
        bool _use_xf_buff = false
        );

	struct ProcessParams {
		int prev_input;
		int advance_time;
		int skip_stage;
		int n_channels;
		ProcessParams() {
			n_channels = 0;
		}
		void set(
			int _prev_input,
			int _advance_time,
			int _skip_stage,
			int _n_channels
		) {
			prev_input = _prev_input;
			advance_time = _advance_time;
			skip_stage = _skip_stage;
			n_channels = _n_channels;
		}
	} m_processParams, m_processParams_xf;

    /*
        classic
    */
    /*
        push input into the pipeline
    */
    int processPush(
        int n_channels,
        const int *uploadIDs,     // upload set IDs
        const int *convIDs,       // kernel IDs
        int prev_input
        );


    /*
        MAD. classic and the second stage of the head
        in head-tail case, the round counter is shifted 1 step since it's alread advance in the head stage
        also IR shist set to 1, since the 2nd stage convolve with all blocks except 0th
    */
    int processAccum(int n_channels,
        int IR_bin_shift = 0,
        int _STR_bin_shift = 0,
        bool _use_xf_buff = false
#ifndef TAN_SDK_EXPORTS
        ,
        cl_command_queue graalQ = NULL
#endif
        );

    int processPull(
        int _n_channels,
        const int *_uploadIDs,     // upload set IDs
        const int *_convIDs,       // kernel IDs
        int advance_time);


    int sincUpload( void );


    template<typename T>
    void initBuffer(CABuf<T> *buf, cl_command_queue queue)
    {
        buf->setValue2(queue, 0);
        return;
    }
    //   alg config
    int n_sets_;             // number of kerenl sets to implement double-buffering
    int n_components_;      // real = 1, complex = 2
    int n_input_blocks_;  // # of block to keep in staging buffer
    int conv_mem_alloc_flags_; // not use
    int n_upload_qs_;   // n IR upload queues 1
    int n_input_qs_;    // n process queues 2
    int n_accum_blocks_; // n blocks accumulated at one CMAD invokation
    int n_stages_;    // classic  = 1, head tail 2
	int m_useProcessFinalize;

// internal state
    int algorithm_;
    int n_max_channels_;
    int max_conv_sz_;
    int conv_log2_;
    int aligned_conv_sz_;  // size of convolution buffer with 0 padding if neded
    int max_proc_buffer_sz_;
    int processing_log2_;
    int aligned_proc_bufffer_sz_; // size of aligned input block
    int align_padding_sz_; // size of the padding or dif(aligned_proc_bufffer_sz_, max_proc_buffer_sz_)
    int n_aligned_proc_blocks_; // number of aligned blocks in aligned kernel buffer
    int aligned_processing_sz_;  // size of freq domain block
    int accum_stride_;  // stride of accum buffer per channel

// FHT
    void* sincos_;  // precomputeted sincos table
    void* bit_reverse_;  // reverse bit table

    // single Graal OCL Q ???
#ifndef TAN_SDK_EXPORTS
    bool own_queue_;
    cl_command_queue graalQ_;
    cl_command_queue graalTailQ_;
    // end of pipeline event
    cl_event eop_event_;
    // end of head event
    cl_event eoh_event_;
#endif

    std::vector<std::vector<void*>> kernel_staging_;
    std::vector<std::vector<void*>> kernel_transformed_; // per channel
    std::vector<void*> kernel_transformed_store_; // per set
#ifdef COPY_CONTIGUOUS_IRS_IN_ONE_BLOCK
    std::vector<void*> kernel_input_store_; // per set
#endif
    // kernel channel map
    void* kernel_channels_map_;
// kernel set map
    void* kernel_sets_map_;
// kernel length map
    void* kernel_lens_map_;
// kernel input
    void * kernel_input_union_;  // base union store
// kernel storage
    void * kernel_trasformed_union_;  // base union store

private:
// upload in a single run
    cl_kernel uploadKernel_;
// upload per stream
    cl_kernel uploadKernel2_;

    cl_kernel resetKernel_;

protected:
    cl_kernel m_copyWithPaddingKernel;

    int64_t round_counter_;

    std::vector<void*> host_input_staging_;
// combined state buffer channel/version/roound counter by number of stages
    void * state_union_;

// round counter per channel and set
    void * round_counters_;
// channel map
    void* channels_map_;
// set map
    void* sets_map_;
	void* sets_map_xf;
// input data
    CABuf<float> m_process_input_staging_;
// input transformed history
    void* history_transformed_;
// output data
    void* process2_output_staging_;

// cmad accumulator
    void* cmad_accum_;
    void* cmad_accum_xf_;// used to store data needed for crossfade
    void * copy_response_in_offset_;
    void * copy_response_out_offset_;
    int* host_copy_resp_in_offset;
    int* host_copy_resp_out_offset;

    cl_kernel inputKernel_;
    cl_kernel inputStageKernel_;
    cl_kernel directTransformKernel_;
    cl_kernel inverseTransformKernel_;
    std::vector<cl_kernel> CMADKernels_;
    cl_kernel convHead1_;
    cl_event m_headTailKernelEvent;
    cl_event m_pullKernelEvent;
    void * FHT_transformCPU_;
    float m_dataBuff[32];
// verification/log
    int verify;
};

};


#endif
