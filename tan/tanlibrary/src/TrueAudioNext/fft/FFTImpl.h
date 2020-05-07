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
///-------------------------------------------------------------------------
///  @file   TANFFTImpl.h
///  @brief  TANFFT interface implementation
///-------------------------------------------------------------------------
#pragma once
#ifdef USE_IPP
#include <ipp.h>
//#include "ippcore_tl.h"
#include <ippcore.h>
#include <ippversion.h>
//#include "tbb/task.h"
//#include "tbb/task_scheduler_init.h"
//#include "tbb/tick_count.h"
//#include "tbb/blocked_range.h"
//#include "tbb/concurrent_vector.h"
//#include "tbb/concurrent_queue.h"
//#include "tbb/concurrent_hash_map.h"
//#include "tbb/parallel_while.h"
//#include "tbb/parallel_for.h"
//#include "tbb/parallel_reduce.h"
//#include "tbb/parallel_scan.h"
//#include "tbb/pipeline.h"
//#include "tbb/atomic.h"
//#include "tbb/mutex.h"
//#include "tbb/spin_mutex.h"
//#include "tbb/queuing_mutex.h"
//#include "tbb/tbb_thread.h"
#endif
#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Context.h"        //AMF
#include "public/include/components/Component.h"//AMF
#include "public/common/PropertyStorageExImpl.h"
#include <unordered_map>
#ifdef _WIN32
#include "tanlibrary/src/fftw-3.3.5-dll64/fftw3.h"
#else
#include "tanlibrary/src/fftw-3.3.5/api/fftw3.h"
#endif

#ifndef _WIN32
typedef unsigned int DWORD;
#endif

#define MAX_CACHE_POWER 20

namespace amf
{
    class TANFFTImpl
        : public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANFFT> >
    {
    public:
        static bool useIntrinsics;

        typedef AMFInterfacePtr_T<TANFFTImpl> Ptr;

        TANFFTImpl(TANContext *pContextTAN, bool useConvQueue);
        virtual ~TANFFTImpl(void);

// interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANFFT> >)
        AMF_END_INTERFACE_MAP

//TANFFT interface
        AMF_RESULT	AMF_STD_CALL Init()         override;
        AMF_RESULT  AMF_STD_CALL Terminate()    override;
        TANContext* AMF_STD_CALL GetContext()   override    { return m_pContextTAN; }
        
        // log2len - arrays' length should be power of 2, the true lenght is expected to be
        //           (2 ^ log2len) * (2 * sizeof(float)) (due to complex numbers).
        AMF_RESULT  AMF_STD_CALL Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                           amf_uint32 log2len,
                                           amf_uint32 channels,
                                           float* ppBufferInput[],
                                           float* ppBufferOutput[]) override;

        AMF_RESULT  AMF_STD_CALL Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                           amf_uint32 log2len,
                                           amf_uint32 channels,
                                           cl_mem pBufferInput[],
                                           cl_mem pBufferOutput[]) override;

		AMF_RESULT  AMF_STD_CALL TransformBatchGPU(TAN_FFT_TRANSFORM_DIRECTION direction,
											amf_uint32 log2len,
											amf_uint32 channels,
											cl_mem pBufferInput,
											cl_mem pBufferOutput,
											int dataSpacing) override;

    private:
		//first 32 bit-> log2length, second 32 bit -> num of channel
		std::unordered_map<amf_uint64, size_t> m_pCLFFTHandleMap;
        //std::unordered_map<amf_uint, void *> m_pFFTWHandleMap;

#ifdef USE_IPP
		IppsFFTSpec_R_32f **m_IppFFTspecsR32F[MAX_CACHE_POWER];
		IppsFFTSpec_C_32fc **m_IppFFTspecsC32F[MAX_CACHE_POWER];
		Ipp8u **m_IppFFTworkBuf[MAX_CACHE_POWER];
#endif
#ifndef __linux__
        bool bFFTWavailable;
        // FFTW declarations for dynamic load:
        typedef fftwf_plan(__cdecl* fftwf_plan_dft_1dType)(int n, fftwf_complex *in, fftwf_complex *out, int sign, unsigned flags);
        fftwf_plan_dft_1dType fftwf_plan_dft_1d = nullptr;

        typedef void(__cdecl * fftwf_destroy_planType)(fftwf_plan);
        fftwf_destroy_planType fftwf_destroy_plan = nullptr;

        typedef void(__cdecl* fftwf_execute_dftType) (fftwf_plan, fftwf_complex *, fftwf_complex *);
        fftwf_execute_dftType fftwf_execute_dft = nullptr;

		// real input fft real to complex
		typedef fftwf_plan(__cdecl* fftwf_plan_dft_r2c_1dType)(int n0, float *in, fftwf_complex *out, unsigned flags);
		fftwf_plan_dft_r2c_1dType fftwf_plan_dft_r2c_1d = nullptr;

		typedef void(__cdecl* fftwf_execute_dft_r2cType) (fftwf_plan, float *, fftwf_complex *);
		fftwf_execute_dft_r2cType fftwf_execute_dft_r2c = nullptr;

		// inverse fft complex to real
		typedef fftwf_plan(__cdecl* fftwf_plan_dft_c2r_1dType)(int n0, fftwf_complex *in, float *out, unsigned flags);
		fftwf_plan_dft_c2r_1dType fftwf_plan_dft_c2r_1d = nullptr;

		typedef void(__cdecl* fftwf_execute_dft_c2rType) (fftwf_plan, fftwf_complex *, float *);
		fftwf_execute_dft_c2rType fftwf_execute_dft_c2r = nullptr;

		// real input fft real to complex planar
		/*fftw_plan fftw_plan_guru_split_dft_r2c(	int rank, const fftw_iodim *dims,
													int howmany_rank, const fftw_iodim *howmany_dims,
													double *in, double *ro, double *io,
													unsigned flags);
													*/

		typedef fftwf_plan(__cdecl* fftwf_plan_guru_split_dft_r2cType)(int rank, const fftwf_iodim *dims, int howmany_rank, 
			const fftwf_iodim *howmany_dims, float *in, float *ro, float *io,unsigned flags);

		fftwf_plan_guru_split_dft_r2cType fftwf_plan_guru_split_dft_r2c = nullptr;

		typedef void(__cdecl* fftwf_execute_split_dft_r2cType )(	const fftwf_plan p, float *in, float *ro, float *io);
		fftwf_execute_split_dft_r2cType fftwf_execute_split_dft_r2c = nullptr;


		typedef fftwf_plan(__cdecl* fftwf_plan_guru_split_dft_c2rType)(int rank, const fftwf_iodim *dims, int howmany_rank, 
			const fftw_iodim *howmany_dims, float *rin, float *iin, float *out, unsigned flags);

		fftwf_plan_guru_split_dft_c2rType fftwf_plan_guru_split_dft_c2r = nullptr;

		typedef void(__cdecl* fftwf_execute_split_dft_c2rType )(	const fftwf_plan p, float *rin, float *iin, float *out );
		fftwf_execute_split_dft_r2cType fftwf_execute_split_dft_c2r = nullptr;

		//int fftwf_import_wisdom_from_filename(const char *filename);
		//int fftwf_import_wisdom_from_filename(const char *filename);

		typedef int(__cdecl* fftwf_export_wisdom_to_filenameType)(const char *filename);
		fftwf_export_wisdom_to_filenameType fftwf_export_wisdom_to_filename = nullptr;

		typedef int(__cdecl* fftwf_import_wisdom_from_filenameType)(const char *filename);
		fftwf_import_wisdom_from_filenameType fftwf_import_wisdom_from_filename = nullptr;

#else

	bool bFFTWavailable;
        // FFTW declarations for dynamic load:
        typedef fftwf_plan(* fftwf_plan_dft_1dType)(int n, fftwf_complex *in, fftwf_complex *out, int sign, unsigned flags);
        fftwf_plan_dft_1dType fftwf_plan_dft_1d = nullptr;

        typedef void( * fftwf_destroy_planType)(fftwf_plan);
        fftwf_destroy_planType fftwf_destroy_plan = nullptr;

        typedef void(* fftwf_execute_dftType) (fftwf_plan, fftwf_complex *, fftwf_complex *);
        fftwf_execute_dftType fftwf_execute_dft = nullptr;

		// real input fft real to complex
		typedef fftwf_plan(* fftwf_plan_dft_r2c_1dType)(int n0, float *in, fftwf_complex *out, unsigned flags);
		fftwf_plan_dft_r2c_1dType fftwf_plan_dft_r2c_1d = nullptr;

		typedef void(* fftwf_execute_dft_r2cType) (fftwf_plan, float *, fftwf_complex *);
		fftwf_execute_dft_r2cType fftwf_execute_dft_r2c = nullptr;

		// inverse fft complex to real
		typedef fftwf_plan(* fftwf_plan_dft_c2r_1dType)(int n0, fftwf_complex *in, float *out, unsigned flags);
		fftwf_plan_dft_c2r_1dType fftwf_plan_dft_c2r_1d = nullptr;

		typedef void(* fftwf_execute_dft_c2rType) (fftwf_plan, fftwf_complex *, float *);
		fftwf_execute_dft_c2rType fftwf_execute_dft_c2r = nullptr;

		// real input fft real to complex planar
		/*fftw_plan fftw_plan_guru_split_dft_r2c(	int rank, const fftw_iodim *dims,
													int howmany_rank, const fftw_iodim *howmany_dims,
													double *in, double *ro, double *io,
													unsigned flags);
													*/

		typedef fftwf_plan(* fftwf_plan_guru_split_dft_r2cType)(int rank, const fftwf_iodim *dims, int howmany_rank, 
			const fftwf_iodim *howmany_dims, float *in, float *ro, float *io,unsigned flags);

		fftwf_plan_guru_split_dft_r2cType fftwf_plan_guru_split_dft_r2c = nullptr;

		typedef void(* fftwf_execute_split_dft_r2cType )(	const fftwf_plan p, float *in, float *ro, float *io);
		fftwf_execute_split_dft_r2cType fftwf_execute_split_dft_r2c = nullptr;


		typedef fftwf_plan(* fftwf_plan_guru_split_dft_c2rType)(int rank, const fftwf_iodim *dims, int howmany_rank, 
			const fftw_iodim *howmany_dims, float *rin, float *iin, float *out, unsigned flags);

		fftwf_plan_guru_split_dft_c2rType fftwf_plan_guru_split_dft_c2r = nullptr;

		typedef void(* fftwf_execute_split_dft_c2rType )(	const fftwf_plan p, float *rin, float *iin, float *out );
		fftwf_execute_split_dft_r2cType fftwf_execute_split_dft_c2r = nullptr;

		//int fftwf_import_wisdom_from_filename(const char *filename);
		//int fftwf_import_wisdom_from_filename(const char *filename);

		typedef int(* fftwf_export_wisdom_to_filenameType)(const char *filename);
		fftwf_export_wisdom_to_filenameType fftwf_export_wisdom_to_filename = nullptr;

		typedef int(* fftwf_import_wisdom_from_filenameType)(const char *filename);
		fftwf_import_wisdom_from_filenameType fftwf_import_wisdom_from_filename = nullptr;
#endif

        fftwf_plan fwdPlans[MAX_CACHE_POWER];
        fftwf_plan bwdPlans[MAX_CACHE_POWER];
		fftwf_plan fwdRealPlans[MAX_CACHE_POWER];
		fftwf_plan bwdRealPlans[MAX_CACHE_POWER];
		fftwf_plan fwdRealPlanarPlans[MAX_CACHE_POWER];
		fftwf_plan bwdRealPlanarPlans[MAX_CACHE_POWER];

		void GetFFTWCachePath(char *path, DWORD len);
		void cacheFFTWplans();


		enum CLFFT_TRANSFORM_TYPE
		{
			CLFFT_TRANSFORM_COMPLEX = 0,
			CLFFT_TRANSFORM_R2C_FORWARD = 1,
			CLFFT_TRANSFORM_C2R_BACKWARD = 2,
		};

        size_t getclFFTPlan(int log2len, int numOfChannels, CLFFT_TRANSFORM_TYPE transformType = CLFFT_TRANSFORM_COMPLEX, int interBlockDistance = 0);

		AMF_RESULT virtual AMF_STD_CALL InitCpu();
        AMF_RESULT virtual AMF_STD_CALL InitGpu();

    protected:
        AMF_RESULT virtual AMF_STD_CALL TransformImplCpu(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                         amf_size log2len,
                                                         amf_size channels,
                                                         float* ppBufferInput[],
                                                         float* ppBufferOutput[]);
        AMF_RESULT virtual AMF_STD_CALL TransformImplCpuOMP(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                        amf_size log2len,
                                                        amf_size channels,
                                                        float* ppBufferInput[],
                                                        float* ppBufferOutput[]);
#ifdef USE_IPP
		AMF_RESULT virtual AMF_STD_CALL TransformImplIPP(TAN_FFT_TRANSFORM_DIRECTION direction,
														amf_size log2len,
														amf_size channels,
														float* ppBufferInput[],
														float* ppBufferOutput[]);
#endif
		AMF_RESULT virtual AMF_STD_CALL TransformImplCpu1Chan(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                        amf_size log2len,
                                                        float* pBufferInput,
                                                        float* pBufferOutput);
        AMF_RESULT virtual AMF_STD_CALL TransformImplFFTW1Chan(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                        amf_size log2len,
                                                        amf_size channel,
                                                        float* pBufferInput[],
                                                        float* pBufferOutput[]);
		AMF_RESULT virtual AMF_STD_CALL TransformImplFFTWReal1Chan(TAN_FFT_TRANSFORM_DIRECTION direction,
														amf_size log2len,
														amf_size channel,
														float* pBufferInput[],
														float* pBufferOutput[]);

		AMF_RESULT virtual AMF_STD_CALL TransformImplFFTWReal(TAN_FFT_TRANSFORM_DIRECTION direction,
														amf_size log2len,
														float* pBufferInput,
														float* pBufferOutput);

		AMF_RESULT virtual AMF_STD_CALL TransformImplGPUBatched(
														TAN_FFT_TRANSFORM_DIRECTION direction,
														amf_size log2len,
														amf_size channels,
														cl_mem pBufferInput,
														cl_mem pBufferOutput,
														int dataSpacing);

        AMF_RESULT virtual AMF_STD_CALL TransformImplGpu(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                         amf_size log2len,
                                                         amf_size channels,
                                                         cl_mem pBufferInput[],
                                                         cl_mem pBufferOutput[]);

    private:
		void clearInternalBuffers();
		void AdjustInternalBufferSize(size_t desireSizeInSampleLog2, size_t numofChannel);
        TANContextPtr               m_pContextTAN;

        AMFComputeKernelPtr         m_pKernelCopy;
        AMF_MEMORY_TYPE             m_eOutputMemoryType;
        AMFCriticalSection          m_sect;

		amf_size m_iInternalBufferSizeInBytes = 0;
        cl_mem          m_pInputsOCL;
        cl_mem          m_pOutputsOCL;

        bool                        m_doProcessingOnGpu;
        bool                        m_useConvQueue;
    };

    // Internal function used only from TANConvolution class.
    AMF_RESULT TANCreateFFT(amf::TANContext *pContext,
        amf::TANFFT** ppComponent,
        bool useConvQueue);
    ////-------------------------------------------------------------------------------------------------
    //AMF_RESULT TANCreateFFT(
    //    amf::TANContextPtr pContext,
    //    amf::TANFFT **ppComponent,
    //    amf::AMFComputePtr pAmfContext
    //    );
} //amf
