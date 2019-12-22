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
#define _USE_MATH_DEFINES
#include <cmath>
#include <omp.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "../../../../common/cpucaps.h"
#include <immintrin.h>

#include "tanlibrary/src/clFFT-master/src/include/clFFT.h"
#include "FFTImpl.h"
#include "../core/TANContextImpl.h"     //TAN
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <memory>

#include "../../clFFT-master/src/include/sharedLibrary.h"

#ifndef _WIN32
typedef void * HMODULE;
#endif

#ifdef _WIN32
#define _mkdir mkdir
#define _chdir chdir
#define PATH_MAX MAX_PATH
#endif

#define AMF_FACILITY L"TANFFTImpl"

//const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
bool amf::TANFFTImpl::useIntrinsics = true;// InstructionSet::AVX() && InstructionSet::FMA();

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
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateFFT(
    amf::TANContext* pContext, 
    amf::TANFFT** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANFFTImpl(pContext, false);
    (*ppComponent)->Acquire();
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
 AMF_RESULT  amf::TANCreateFFT(
    amf::TANContext* pContext,
    amf::TANFFT** ppComponent,
    bool useConvQueue
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANFFTImpl(pContext, useConvQueue);
    (*ppComponent)->Acquire();
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
TANFFTImpl::TANFFTImpl(TANContext *pContextTAN, bool useConvQueue) :
    m_pContextTAN(pContextTAN),
    m_eOutputMemoryType(AMF_MEMORY_HOST),
    m_pInputsOCL(nullptr),
    m_pOutputsOCL(nullptr),
    m_useConvQueue(false)
{
    m_useConvQueue = useConvQueue;
   // AMFPrimitivePropertyInfoMapBegin
   //     AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE ,  L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
   // AMFPrimitivePropertyInfoMapEnd

		for (int i = 0; i < MAX_CACHE_POWER; i++)
		{
			fwdPlans[i] = NULL;
			bwdPlans[i] = NULL;
			fwdRealPlans[i] = NULL;
			bwdRealPlans[i] = NULL;			
		}
}
//-------------------------------------------------------------------------------------------------
TANFFTImpl::~TANFFTImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::Init()
{
    AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    if (m_pContextTAN->GetOpenCLContext())
    {
        return InitGpu();
    }
    else
    {
        return InitCpu();
    }
}

// Report IPP version and enabled features
#ifdef USE_IPP
int reportIPPVersionAndFeatures() {
	const IppLibraryVersion *lib;

	IppStatus status;
	Ipp64u mask, emask;

	/* Get IPP library version info */
	lib = ippGetLibVersion();

	printf("%s %s\n", lib->Name, lib->Version);

	/* Get CPU features and features enabled with selected library level */
	status = ippGetCpuFeatures(&mask, 0);

	if (ippStsNoErr == status) {
		emask = ippGetEnabledCpuFeatures();
		printf("Features supported by CPU\tby IPP\n");
		printf("-----------------------------------------\n");
		printf("  ippCPUID_MMX        = ");
		printf("%c\t%c\t", (mask & ippCPUID_MMX) ? 'Y' : 'N', (emask & ippCPUID_MMX) ? 'Y' : 'N');
		printf("Intel(R) Architecture MMX technology supported\n");
		printf("  ippCPUID_SSE        = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE) ? 'Y' : 'N', (emask & ippCPUID_SSE) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions\n");
		printf("  ippCPUID_SSE2       = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE2) ? 'Y' : 'N', (emask & ippCPUID_SSE2) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 2\n");
		printf("  ippCPUID_SSE3       = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE3) ? 'Y' : 'N', (emask & ippCPUID_SSE3) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 3\n");
		printf("  ippCPUID_SSSE3      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSSE3) ? 'Y' : 'N', (emask & ippCPUID_SSSE3) ? 'Y' : 'N');
		printf("Intel(R) Supplemental Streaming SIMD Extensions 3\n");
		printf("  ippCPUID_MOVBE      = ");
		printf("%c\t%c\t", (mask & ippCPUID_MOVBE) ? 'Y' : 'N', (emask & ippCPUID_MOVBE) ? 'Y' : 'N');
		printf("The processor supports MOVBE instruction\n");
		printf("  ippCPUID_SSE41      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE41) ? 'Y' : 'N', (emask & ippCPUID_SSE41) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 4.1\n");
		printf("  ippCPUID_SSE42      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE42) ? 'Y' : 'N', (emask & ippCPUID_SSE42) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 4.2\n");
		printf("  ippCPUID_AVX        = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX) ? 'Y' : 'N', (emask & ippCPUID_AVX) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions instruction set\n");
		printf("  ippAVX_ENABLEDBYOS  = ");
		printf("%c\t%c\t", (mask & ippAVX_ENABLEDBYOS) ? 'Y' : 'N', (emask & ippAVX_ENABLEDBYOS) ? 'Y' : 'N');
		printf("The operating system supports Intel(R) AVX\n");
		printf("  ippCPUID_AES        = ");
		printf("%c\t%c\t", (mask & ippCPUID_AES) ? 'Y' : 'N', (emask & ippCPUID_AES) ? 'Y' : 'N');
		printf("Intel(R) AES instruction\n");
		printf("  ippCPUID_SHA        = ");
		printf("%c\t%c\t", (mask & ippCPUID_SHA) ? 'Y' : 'N', (emask & ippCPUID_SHA) ? 'Y' : 'N');
		printf("Intel(R) SHA new instructions\n");
		printf("  ippCPUID_CLMUL      = ");
		printf("%c\t%c\t", (mask & ippCPUID_CLMUL) ? 'Y' : 'N', (emask & ippCPUID_CLMUL) ? 'Y' : 'N');
		printf("PCLMULQDQ instruction\n");
		printf("  ippCPUID_RDRAND     = ");
		printf("%c\t%c\t", (mask & ippCPUID_RDRAND) ? 'Y' : 'N', (emask & ippCPUID_RDRAND) ? 'Y' : 'N');
		printf("Read Random Number instructions\n");
		printf("  ippCPUID_F16C       = ");
		printf("%c\t%c\t", (mask & ippCPUID_F16C) ? 'Y' : 'N', (emask & ippCPUID_F16C) ? 'Y' : 'N');
		printf("Float16 instructions\n");
		printf("  ippCPUID_AVX2       = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX2) ? 'Y' : 'N', (emask & ippCPUID_AVX2) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions 2 instruction set\n");
		printf("  ippCPUID_AVX512F    = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512F) ? 'Y' : 'N', (emask & ippCPUID_AVX512F) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions 3.1 instruction set\n");
		printf("  ippCPUID_AVX512CD   = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512CD) ? 'Y' : 'N', (emask & ippCPUID_AVX512CD) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions CD (Conflict Detection) instruction set\n");
		printf("  ippCPUID_AVX512ER   = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512ER) ? 'Y' : 'N', (emask & ippCPUID_AVX512ER) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions ER instruction set\n");
		printf("  ippCPUID_ADCOX      = ");
		printf("%c\t%c\t", (mask & ippCPUID_ADCOX) ? 'Y' : 'N', (emask & ippCPUID_ADCOX) ? 'Y' : 'N');
		printf("ADCX and ADOX instructions\n");
		printf("  ippCPUID_RDSEED     = ");
		printf("%c\t%c\t", (mask & ippCPUID_RDSEED) ? 'Y' : 'N', (emask & ippCPUID_RDSEED) ? 'Y' : 'N');
		printf("The RDSEED instruction\n");
		printf("  ippCPUID_PREFETCHW  = ");
		printf("%c\t%c\t", (mask & ippCPUID_PREFETCHW) ? 'Y' : 'N', (emask & ippCPUID_PREFETCHW) ? 'Y' : 'N');
		printf("The PREFETCHW instruction\n");
		printf("  ippCPUID_KNC        = ");
		printf("%c\t%c\t", (mask & ippCPUID_KNC) ? 'Y' : 'N', (emask & ippCPUID_KNC) ? 'Y' : 'N');
		printf("Intel(R) Xeon Phi(TM) Coprocessor instruction set\n");
	}
	return status;
}
#endif

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::InitCpu()
{
    /* AMF Initialization */

    AMFLock lock(&m_sect);

	int numThreads = 0;
	int numProcs = 1;
	numProcs = omp_get_num_procs();
	numThreads = numProcs / 4;
	if (numThreads == 0)
		numThreads = 1;
	omp_set_num_threads(numThreads);
	numThreads = omp_get_max_threads();


#ifdef USE_IPP
	/* Init IPP library */
	IppStatus err = ippInit();
	//reportIPPVersionAndFeatures();  
	//ippSetCpuFeatures(L9_FM); // L9_FM == AVX 256 // K0_FM == AVX 512

	if (err != ippStsNoErr && err != ippStsNonIntelCpu) {
		return AMF_FAIL;
	}
	ippSetNumThreads(1);

	for (int i = 0; i < MAX_CACHE_POWER; i++) {
		m_IppFFTworkBuf[i] = new Ipp8u *[numThreads];
		m_IppFFTspecsR32F[i] = new IppsFFTSpec_R_32f *[numThreads];
		m_IppFFTspecsC32F[i] = new IppsFFTSpec_C_32fc *[numThreads];
		for (int k = 0; k < numThreads; k++) {
			m_IppFFTworkBuf[i][k] = NULL;
			m_IppFFTspecsR32F[i][k] = NULL;
			m_IppFFTspecsC32F[i][k] = NULL;
		}
	}

	return AMF_OK;
#endif

    AMF_RETURN_IF_FALSE( (NULL != m_pContextTAN), AMF_WRONG_STATE,
    L"Cannot initialize after termination");

    m_doProcessingOnGpu = false;

    void * FFTWDll = NULL;
    bFFTWavailable = false;

	/*
		GetModuleFileNameA(NULL, Path, PATH_MAX);
	char *pslash = strrchr(Path, '\\');
	char exename[80] = "\0";
	if (pslash){
		strncpy(exename, pslash + 1, 80);
		*pslash = '\0';
	}

	*/

	//libfftw3f-3.dll
#ifdef _WIN32
    FFTWDll = LoadSharedLibrary("", "libfftw3f-3", true);
#else
	FFTWDll = LoadSharedLibrary("", "libfftw3f", true);
#endif

    if (NULL != FFTWDll)
    {
        fftwf_plan_dft_1d = (fftwf_plan_dft_1dType)LoadFunctionAddr(FFTWDll, "fftwf_plan_dft_1d");
        fftwf_destroy_plan = (fftwf_destroy_planType)LoadFunctionAddr(FFTWDll, "fftwf_destroy_plan");
		fftwf_execute_dft = (fftwf_execute_dftType)LoadFunctionAddr(FFTWDll, "fftwf_execute_dft");
			
		fftwf_plan_dft_r2c_1d = (fftwf_plan_dft_r2c_1dType)LoadFunctionAddr(FFTWDll, "fftwf_plan_dft_r2c_1d");
		fftwf_execute_dft_r2c = (fftwf_execute_dft_r2cType)LoadFunctionAddr(FFTWDll, "fftwf_execute_dft_r2c");

		fftwf_plan_dft_c2r_1d = (fftwf_plan_dft_c2r_1dType)LoadFunctionAddr(FFTWDll, "fftwf_plan_dft_c2r_1d");
		fftwf_execute_dft_c2r = (fftwf_execute_dft_c2rType)LoadFunctionAddr(FFTWDll, "fftwf_execute_dft_c2r");


		// try plan_guru_split_dft_r2c
		fftwf_plan_guru_split_dft_r2c = (fftwf_plan_guru_split_dft_r2cType)LoadFunctionAddr(FFTWDll, "fftwf_plan_guru_split_dft_r2c");
		fftwf_execute_split_dft_r2c = (fftwf_execute_split_dft_r2cType)LoadFunctionAddr(FFTWDll, "fftwf_execute_split_dft_r2c");

		fftwf_plan_guru_split_dft_c2r = (fftwf_plan_guru_split_dft_c2rType)LoadFunctionAddr(FFTWDll, "fftwf_plan_guru_split_dft_c2r");
		fftwf_execute_split_dft_c2r = (fftwf_execute_split_dft_c2rType)LoadFunctionAddr(FFTWDll, "fftwf_execute_split_dft_c2r");

		fftwf_export_wisdom_to_filename = (fftwf_export_wisdom_to_filenameType)LoadFunctionAddr(FFTWDll, "fftwf_export_wisdom_to_filename");
		fftwf_import_wisdom_from_filename = (fftwf_import_wisdom_from_filenameType)LoadFunctionAddr(FFTWDll, "fftwf_import_wisdom_from_filename");



        if (fftwf_plan_dft_1d != nullptr && fftwf_destroy_plan != nullptr && fftwf_execute_dft != nullptr){
            bFFTWavailable = true;
			char path[PATH_MAX + 2] = "\0";
			int len = PATH_MAX;
			GetFFTWCachePath(path, len);
			FILE *fp = NULL;
			fp = fopen(path, "r");
			if (fp == NULL) {
				cacheFFTWplans();
			}
			else {
				fclose(fp);
			}
			int result = fftwf_import_wisdom_from_filename(path);
        }
    }

	return bFFTWavailable ? AMF_OK : AMF_FAIL;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::InitGpu()
{
    /* AMF Initialization */

    AMFLock lock(&m_sect);

    AMF_RETURN_IF_FALSE(m_pInputsOCL == nullptr, AMF_ALREADY_INITIALIZED, L"Already initialized");
    AMF_RETURN_IF_FALSE( (nullptr != m_pContextTAN), AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    amf_int64 tmp = 0;
    GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
    m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;

    cl_context context = m_pContextTAN->GetOpenCLContext();
	cl_command_queue cmdQueue = nullptr;
	if (m_useConvQueue) {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
	}
	else {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
	}

    
    m_doProcessingOnGpu = (nullptr != context);

    if (m_doProcessingOnGpu){
		AdjustInternalBufferSize(13, 2);
    }
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::Terminate()
{
    AMFLock lock(&m_sect);

	//if (bFFTWavailable) {
	//	char path[PATH_MAX + 2] = "\0";
	//	int len = PATH_MAX;
	//	GetFFTWCachePath(path, len);
	//	fftwf_export_wisdom_to_filename(path);
	//}

    m_pContextTAN.Release();

    m_pKernelCopy.Release();

    //release ocl bufers
	clearInternalBuffers();
	if (m_doProcessingOnGpu)
	{
		for (std::unordered_map<amf_uint64, size_t>::iterator x = m_pCLFFTHandleMap.begin(); x != m_pCLFFTHandleMap.end(); ++x)
		{
			clfftDestroyPlan(&x->second);
		}
    }
    else {
        for (int i = 0; i < MAX_CACHE_POWER; i++)
        {
            if (fwdPlans[i] != NULL) {
                fftwf_destroy_plan(fwdPlans[i]);
                fwdPlans[i] = NULL;
            }
            if (bwdPlans[i] != NULL) {
                fftwf_destroy_plan(bwdPlans[i]);
                bwdPlans[i] = NULL;
            }
			if (fwdRealPlans[i] != NULL) {
				fftwf_destroy_plan(fwdRealPlans[i]);
				fwdRealPlans[i] = NULL;
			}
			if (bwdRealPlans[i] != NULL) {
				fftwf_destroy_plan(bwdRealPlans[i]);
				bwdRealPlans[i] = NULL;
			}
		}

    }
	

    return AMF_OK;
}

void TANFFTImpl::GetFFTWCachePath(char *path, DWORD len)
{
#ifdef _WIN32
	char* appdata = getenv("LOCALAPPDATA");
	WCHAR curDir[2 * PATH_MAX];
	curDir[0] = L'\0';

	//for Linux, use _getcwd()

	// use W version in case folders have unicode names:
	GetCurrentDirectoryW(PATH_MAX, curDir);

	strcpy(path, appdata);
	strcat(path, "\\AMD");
	if (_chdir(path) == -1) {
		_mkdir(path);
	}
	strcat(path, "\\TAN");
	if (_chdir(path) == -1) {
		_mkdir(path);
	}
	strcat(path, "\\FFTW_TAN_WISDOM.cache");
	len = strlen(path);

	// for Linux use _chdir()

	// use W version in case folders have unicode names:
	SetCurrentDirectoryW(curDir);
#endif
	return;
}


void TANFFTImpl::cacheFFTWplans()
{
	int minL2N = 15;
	int maxL2N = 16; MAX_CACHE_POWER;

	char path[PATH_MAX + 2] = "\0";
	int len = PATH_MAX;
	GetFFTWCachePath(path, len);

	if (bFFTWavailable) {
		float * ppBufferInput = new float[(1 << maxL2N)]; // _aligned_malloc(sizeof(float) * (1 << maxL2N), 32);
		float * ppBufferOutput = new float[(1 << maxL2N)];  // _aligned_malloc(sizeof(float) * (1 << maxL2N), 32);
		for (int log2len = minL2N; log2len < maxL2N; log2len++) {

			amf_uint fftLength = 1 << log2len;

			fftwf_complex * in = (fftwf_complex *)ppBufferInput;
			fftwf_complex * out = (fftwf_complex *)ppBufferOutput;

			fftw_iodim iod;
			iod.n = fftLength;
			iod.is = 1;
			iod.os = 1;

			fwdRealPlans[log2len] = fftwf_plan_dft_r2c_1d(fftLength, (float *)in, (fftwf_complex *)out, FFTW_MEASURE);//correct flag ??
			bwdRealPlans[log2len] = fftwf_plan_dft_c2r_1d(fftLength, (fftwf_complex *)in, (float *)out, FFTW_MEASURE);
			fwdRealPlanarPlans[log2len] = fftwf_plan_guru_split_dft_r2c(1, &iod, 0, NULL, (float *)in, (float *)(out), (float *)(out)+(8 + fftLength / 2), FFTW_MEASURE);//correct flag ??
			bwdRealPlanarPlans[log2len] = fftwf_plan_guru_split_dft_c2r(1, &iod, 0, NULL, (float *)in, (float *)(in)+(8 + fftLength / 2), (float *)out, FFTW_MEASURE);

			fwdPlans[log2len] = fftwf_plan_dft_1d(fftLength, in, out, FFTW_FORWARD, FFTW_MEASURE);
			bwdPlans[log2len] = fftwf_plan_dft_1d(fftLength, in, out, FFTW_BACKWARD, FFTW_MEASURE);
		}
		fftwf_export_wisdom_to_filename(path);
		delete(ppBufferInput); // _aligned_free(ppBufferInput);
		delete(ppBufferOutput); // _aligned_free(ppBufferOutput);
	}
}


//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANFFTImpl::Transform(
    TAN_FFT_TRANSFORM_DIRECTION direction, 
    amf_uint32 log2len,
    amf_uint32 channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
)
{
    AMFLock lock(&m_sect);
    AMF_RESULT res = AMF_OK;

    AMF_RETURN_IF_FALSE(ppBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
    AMF_RETURN_IF_FALSE(ppBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");
    AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(log2len > 0, AMF_INVALID_ARG, L"log2len == 0");
    AMF_RETURN_IF_FALSE(log2len < sizeof(amf_size) * 8, AMF_INVALID_ARG, L"log2len is too big");

	bool useRealFFT = (direction & 2) != 0;
	//direction = (TAN_FFT_TRANSFORM_DIRECTION)(int(direction) & 1);

    AMF_RETURN_IF_FALSE(direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD || direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD
		|| direction == TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD || direction == TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD
		|| direction == TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD || direction == TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD,
        AMF_INVALID_ARG, L"Invalid conversion type");

	const amf_size requiredChannelLengthInFloat = (1 << (log2len+1)); // Double the size for complex.
	const amf_size requiredChannelLengthInBytes = requiredChannelLengthInFloat* sizeof(float);
	const amf_size requiredBufferLengthInBytes = requiredChannelLengthInBytes * channels;
    if (m_doProcessingOnGpu) {
		//CLFFT_TRANSFORM_TYPE transfrmType = CLFFT_TRANSFORM_COMPLEX;
		//if (useRealFFT && == direction)  transfrmType = CLFFT_TRANSFORM_R2C_FORWARD;
        cl_int status;
		AdjustInternalBufferSize(log2len + 1, channels);
		cl_command_queue cmdQueue = nullptr;
		if (m_useConvQueue) {
			cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
		}
		else {
			cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
		}


		for (amf_uint32 i = 0; i < channels; i++){
			clEnqueueWriteBuffer(cmdQueue, m_pInputsOCL, CL_TRUE, i*requiredChannelLengthInBytes, requiredChannelLengthInBytes, ppBufferInput[i], 0, NULL, NULL);
        }
		res = TransformImplGPUBatched(direction, log2len, channels, m_pInputsOCL, m_pOutputsOCL,0);
		for (amf_uint32 i = 0; i < channels; i++){
            clEnqueueReadBuffer(cmdQueue, m_pOutputsOCL, CL_TRUE, i*requiredChannelLengthInBytes, requiredChannelLengthInBytes, ppBufferOutput[i],0, NULL, NULL);
        }
        return res;
    }
    // process
#ifdef USE_IPP
//hack for overlap add, uses complex fft
//	if ((direction & 1) != direction) {
		res = TransformImplIPP(direction, log2len, channels, ppBufferInput, ppBufferOutput);
		return AMF_OK;
//	}
#endif
    if (amf::TANFFTImpl::useIntrinsics){
            res = TransformImplCpuOMP(direction, log2len, channels, ppBufferInput, ppBufferOutput);
    }
    else {
        res = TransformImplCpu(direction, log2len, channels, ppBufferInput, ppBufferOutput);
    }
    AMF_RETURN_IF_FAILED(res, L"Transform() failed");

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANFFTImpl::Transform(
    TAN_FFT_TRANSFORM_DIRECTION direction, 
    amf_uint32 log2len,
    amf_uint32 channels,
    cl_mem pBufferInput[],
    cl_mem pBufferOutput[]
)
{
    AMF_RETURN_IF_FALSE(pBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
    AMF_RETURN_IF_FALSE(pBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");
    AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(log2len > 0, AMF_INVALID_ARG, L"log2len == 0");
    AMF_RETURN_IF_FALSE(log2len < sizeof(amf_size) * 8, AMF_INVALID_ARG, L"log2len is too big");
    AMF_RETURN_IF_FALSE(direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD ||
                        direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD,
                        AMF_INVALID_ARG, L"Invalid conversion type");

    if (!m_doProcessingOnGpu) {
        return AMF_INVALID_ARG;
    }
    AMFLock lock(&m_sect);
    
    AMF_RESULT res = AMF_OK;
    // process
    res = TransformImplGpu(direction, log2len, channels, pBufferInput, pBufferOutput);
    AMF_RETURN_IF_FAILED(res, L"Transform() failed");

    return AMF_OK;
}


//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANFFTImpl::TransformBatchGPU(
	TAN_FFT_TRANSFORM_DIRECTION direction,
	amf_uint32 log2len,
	amf_uint32 channels,
	cl_mem pBufferInput,
	cl_mem pBufferOutput,
	int dataSpacing
)
{
	AMF_RETURN_IF_FALSE(pBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
	AMF_RETURN_IF_FALSE(pBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
	AMF_RETURN_IF_FALSE(log2len > 0, AMF_INVALID_ARG, L"log2len == 0");
	AMF_RETURN_IF_FALSE(log2len < sizeof(amf_size) * 8, AMF_INVALID_ARG, L"log2len is too big");
	//AMF_RETURN_IF_FALSE(direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD || direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD,
	//	AMF_INVALID_ARG, L"Invalid conversion type");

	if (!m_doProcessingOnGpu) {
		return AMF_INVALID_ARG;
	}
	AMFLock lock(&m_sect);

	AMF_RESULT res = AMF_OK;
	// process
	res = TransformImplGPUBatched(direction, log2len, channels, pBufferInput, pBufferOutput, dataSpacing);

	AMF_RETURN_IF_FAILED(res, L"Transform() failed");

	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------

size_t TANFFTImpl::getclFFTPlan(int log2len, int numOfChannels, CLFFT_TRANSFORM_TYPE transformType, int interBlockDistance)
{
	amf_uint64 key = log2len;
	key <<= 16;
	key |= interBlockDistance;
	key <<= 16;
	key |= numOfChannels;
	key <<= 16;
	key |= transformType;

    if (m_doProcessingOnGpu) {

        if (m_pCLFFTHandleMap.find(key) != m_pCLFFTHandleMap.end())
        {
            return m_pCLFFTHandleMap.find(key)->second;
        }

        clfftStatus status = CLFFT_SUCCESS;
        // Create new plan
        cl_context context = static_cast<cl_context>(m_pContextTAN->GetOpenCLContext());
        cl_command_queue cmdQueue;

		//hack
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());

        //if (m_useConvQueue){
        //    cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
        //}
        //else {
        //    cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
        //}

        clfftPlanHandle plan;
        const size_t fftLength = (size_t)1 << log2len;
        status = clfftCreateDefaultPlan(&plan, context, CLFFT_1D, &fftLength);
        assert(status == CLFFT_SUCCESS);
		if (transformType == CLFFT_TRANSFORM_R2C_FORWARD) {
			// To Do, try:       
			status = clfftSetLayout(plan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);// CLFFT_COMPLEX_INTERLEAVED);
			//clfftSetPlanDistance(plan, fftLength + 2, fftLength + 2);
		} else
		if (transformType == CLFFT_TRANSFORM_C2R_BACKWARD) {
			// To Do, try:       
			status = clfftSetLayout(plan, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);// CLFFT_COMPLEX_INTERLEAVED);
			//clfftSetPlanDistance(plan, fftLength + 2, fftLength + 2);
		}
		else
		if(transformType == CLFFT_TRANSFORM_COMPLEX){
			status = clfftSetLayout(plan, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
		}

        clfftSetResultLocation(plan, CLFFT_OUTOFPLACE);
        clfftSetPlanBatchSize(plan, numOfChannels);
		//clfftSetPlanDistance(plan, dist);
		if (interBlockDistance > 0) {
			clfftSetPlanDistance(plan, interBlockDistance, interBlockDistance);
		}

        clfftBakePlan(plan, 1, &cmdQueue, NULL, NULL);
        m_pCLFFTHandleMap.insert(std::make_pair(key, plan));
        return plan;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------

/**************************************************************************************************
FftCPU, adapted from code by Stephan Bernsee

COPYRIGHT 1999-2009 Stephan M. Bernsee
The Wide Open License (WOL)
Permission to use, copy, modify, distribute and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice and this license appear in all source copies.

**************************************************************************************************/
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplCpu(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
    )
{
    const amf_size fftFrameSize = (amf_size)pow(2.0, (double)log2len);

    int sign = (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) ? -1 : 1;

	double wr, wi, arg;
    float  *p1, *p2, temp;
	double tr, ti, ur, ui;
	float *p1r, *p1i, *p2r, *p2i;
    amf_uint32 bitm, j, le, le2, k, idx;

    for (amf_size i = 0; i < channels; i++){
        if (ppBufferInput[i] != ppBufferOutput[i]) {
            memcpy(ppBufferOutput[i], ppBufferInput[i], fftFrameSize * 2 * sizeof(float));
        }
    }

    for (amf_size i = 2; i < 2 * fftFrameSize - 2; i += 2) {
        for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
            if (i & bitm) j++;
            j <<= 1;
        }
        if (i < j) {
            for (idx = 0; idx < channels; idx++){
                p1 = ppBufferOutput[idx] + i; p2 = ppBufferOutput[idx] + j;
                temp = *p1; *(p1++) = *p2;
                *(p2++) = temp; temp = *p1;
                *p1 = *p2; *p2 = temp;
            }
        }
    }
    for (k = 0, le = 2; k < (amf_size)(log((float)fftFrameSize) / log(2.) + .5); k++) {
        le <<= 1;
        le2 = le >> 1;
        ur = 1.0;
        ui = 0.0;
        arg = (double)(M_PI / (le2 >> 1));
        wr = cos(arg);
        wi = sign*sin(arg);
        for (j = 0; j < le2; j += 2) {
            for (idx = 0; idx < channels; idx++){
                p1r = ppBufferOutput[idx] + j; p1i = p1r + 1;
                p2r = p1r + le2; p2i = p2r + 1;
                for (amf_size i = j; i < 2 * fftFrameSize; i += le) {
                    tr = *p2r * ur - *p2i * ui;
                    ti = *p2r * ui + *p2i * ur;
                    *p2r = *p1r - tr; *p2i = *p1i - ti;
                    *p1r += tr; *p1i += ti;
                    p1r += le; p1i += le;
                    p2r += le; p2i += le;
                }
            }
            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }

    // Riemann sum.
    if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD)
    {
        for (idx = 0; idx < channels; idx++){
            for (int k = 0; k < 2 * fftFrameSize; k++){
                ppBufferOutput[idx][k] /= fftFrameSize;
            }
        }
    }

    return AMF_OK;
}

/**************************************************************************************************
FftCPU, adapted from code by Stephan Bernsee

COPYRIGHT 1999-2009 Stephan M. Bernsee
The Wide Open License (WOL)
Permission to use, copy, modify, distribute and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice and this license appear in all source copies.

**************************************************************************************************/
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplCpu1Chan(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    float* pBufferInput,
    float* pBufferOutput
    )
{
    const amf_size fftFrameSize = (amf_size)pow(2.0, (double)log2len);

    int sign = (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) ? -1 : 1;

    double wr, wi, arg;
    float  *p1, *p2, temp;
    double tr, ti, ur, ui;
    float *p1r, *p1i, *p2r, *p2i;
    amf_uint32 bitm, j, le, le2, k, idx;

    if (pBufferInput != pBufferOutput) {
        memcpy(pBufferOutput, pBufferInput, fftFrameSize * 2 * sizeof(float));
    }

    for (amf_size i = 2; i < 2 * fftFrameSize - 2; i += 2) {
        for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
            if (i & bitm) j++;
            j <<= 1;
        }
        if (i < j) {
            p1 = pBufferOutput + i; p2 = pBufferOutput + j;
            temp = *p1; *(p1++) = *p2;
            *(p2++) = temp; temp = *p1;
            *p1 = *p2; *p2 = temp;           
        }
    }
    for (k = 0, le = 2; k < (amf_size)(log((float)fftFrameSize) / log(2.) + .5); k++) {
        le <<= 1;
        le2 = le >> 1;
        ur = 1.0;
        ui = 0.0;
        arg = (double)(M_PI / (le2 >> 1));
        wr = cos(arg);
        wi = sign*sin(arg);
        for (j = 0; j < le2; j += 2) {
            p1r = pBufferOutput + j; p1i = p1r + 1;
            p2r = p1r + le2; p2i = p2r + 1;
            for (amf_size i = j; i < 2 * fftFrameSize; i += le) {
                tr = *p2r * ur - *p2i * ui;
                ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr; *p2i = *p1i - ti;
                *p1r += tr; *p1i += ti;
                p1r += le; p1i += le;
                p2r += le; p2i += le;
            }
            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }

    // Riemann sum.
    if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD)
    {
        for (int k = 0; k < 2 * fftFrameSize; k++){
            pBufferOutput[k] /= fftFrameSize;
        }
    }

    return AMF_OK;
}

AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplFFTW1Chan(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channel,
    float* pBufferInput[],
    float* pBufferOutput[]
    )
{
    amf_uint64 key = 0;
    key = direction;
    key |= log2len << 1;
    key |= channel << 32;

    int fftWDir = direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD ? FFTW_FORWARD : FFTW_BACKWARD;
    amf_uint fftLength = 1 << log2len;

    //static fftwf_plan plan;
 
    fftwf_complex * in = (fftwf_complex *)pBufferInput[channel];
    fftwf_complex * out = (fftwf_complex *)pBufferOutput[channel];

    if (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) {
        fftwf_execute_dft(fwdPlans[log2len], in, out);
    }
    else {
        fftwf_execute_dft(bwdPlans[log2len], in, out);
    }

    //fftwf_destroy_plan(plan);

    // Riemann sum.
    if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD)
    {
        for (int k = 0; k < 2 * fftLength; k++){
            pBufferOutput[channel][k] /= fftLength;
        }
    }

    return AMF_OK;
}

AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplFFTWReal1Chan(
	TAN_FFT_TRANSFORM_DIRECTION direction,
	amf_size log2len,
	amf_size channel,
	float* pBufferInput[],
	float* pBufferOutput[]
)
{

	int fftWDir = 0;
	bool usePlanarMode = false;
	switch (direction) {
	case TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD:
		fftWDir = FFTW_FORWARD;
		break;
	case TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD:
		fftWDir = FFTW_BACKWARD;
		break;
	case TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD:
		fftWDir = FFTW_FORWARD;
		usePlanarMode = true;
		break;
	case TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD:
		fftWDir = FFTW_BACKWARD;
		usePlanarMode = true;
		break;

	default:
		return AMF_FAIL;
	}
	amf_uint fftLength = 1 << log2len;

	static fftwf_plan plan;

	float *in = pBufferInput[channel];
	float *out = pBufferOutput[channel];

	//if (fwdRealPlans[log2len] == NULL || bwdRealPlans[log2len] == NULL) {
	//	fwdRealPlans[log2len] = fftwf_plan_dft_r2c_1d(fftLength, (float *)in, (fftwf_complex *)out, FFTW_MEASURE);//correct flag ??
	//	bwdRealPlans[log2len] = fftwf_plan_dft_c2r_1d(fftLength, (fftwf_complex *)in, (float *)out, FFTW_MEASURE);
	//}

	// To Do: real,imaginary buffers should be spaced for 32 byte alignment:

	if (fftWDir == FFTW_FORWARD) {
		if (usePlanarMode) {
			//hack fftwf_execute_split_dft_r2c(fwdRealPlanarPlans[log2len], in, out, out + 1 + fftLength);
			fftwf_execute_split_dft_r2c(fwdRealPlanarPlans[log2len], in, out, out + fftLength/2 + 8);
		}
		else {
			fftwf_execute_dft_r2c(fwdRealPlans[log2len], in, (fftwf_complex *)out);
		}
	}
	else {
		if (usePlanarMode) {
			//fftwf_execute_split_dft_c2r(bwdRealPlanarPlans[log2len], in, in + 1 + fftLength, out);
			fftwf_execute_split_dft_c2r(bwdRealPlanarPlans[log2len], in,in + fftLength/2 + 8, out);
		}
		else {
			fftwf_execute_dft_c2r(bwdRealPlans[log2len], (fftwf_complex *)in, out);
		}
	}

	// Riemann sum.
	if (fftWDir == FFTW_BACKWARD)
	{
		for (int k = 0; k < 2*fftLength + 2; k++) {
			pBufferOutput[channel][k] /= fftLength;
		}
		//if (pBufferOutput == pBufferInput) {
		//	pBufferOutput[channel][fftLength] = 0.0;
		//	pBufferOutput[channel][fftLength + 1] = 0.0;
		//}
	}

	return AMF_OK;
}


AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplFFTWReal(TAN_FFT_TRANSFORM_DIRECTION direction,
	amf_size log2len,
	float* in,
	float* out) {

	int fftWDir = 0;
	bool usePlanarMode = false;
	switch (direction) {
	case TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD:
		fftWDir = FFTW_FORWARD;
		break;
	case TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD:
		fftWDir = FFTW_BACKWARD;
		break;
	case TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD:
		fftWDir = FFTW_FORWARD;
		usePlanarMode = true;
		break;
	case TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD:
		fftWDir = FFTW_BACKWARD;
		usePlanarMode = true;
		break;

	default:
		return AMF_FAIL;
	}
	amf_uint fftLength = 1 << log2len;

	static fftwf_plan plan;

	// To Do: real,imaginary buffers should be spaced for 32 byte alignment:

	if (fftWDir == FFTW_FORWARD) {
		if (usePlanarMode) {
			fftwf_execute_split_dft_r2c(fwdRealPlanarPlans[log2len], in, out, out + fftLength / 2 + 8);
		}
		else {
			fftwf_execute_dft_r2c(fwdRealPlans[log2len], in, (fftwf_complex *)out);
		}
	}
	else {
		if (usePlanarMode) {
			fftwf_execute_split_dft_c2r(bwdRealPlanarPlans[log2len], in, in + fftLength / 2 + 8, out);
		}
		else {
			fftwf_execute_dft_c2r(bwdRealPlans[log2len], (fftwf_complex *)in, out);
		}
	}

	// Riemann sum.
	if (fftWDir == FFTW_BACKWARD)
	{
		for (int k = 0; k < 2 * fftLength + 2; k++) {
			out[k] /= fftLength;
		}
	}

	return AMF_OK;
}


AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplCpuOMP(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
    )
{
    amf_uint fftLength = 1 << log2len;
	bool useRealFFT = (direction == TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD 
					|| direction == TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD
					|| direction == TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD
					|| direction == TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD);

    int idx;
    if (bFFTWavailable){
        fftwf_complex * in = (fftwf_complex *)ppBufferInput[0];
        fftwf_complex * out = (fftwf_complex *)ppBufferOutput[0];

		if (useRealFFT) {
			fftw_iodim iod;
			iod.n = fftLength;
			iod.is = 1;
			iod.os = 1;

			if (fwdRealPlans[log2len] == NULL || bwdRealPlans[log2len] == NULL) {

				fwdRealPlans[log2len] = fftwf_plan_dft_r2c_1d(fftLength, (float *)in, (fftwf_complex *)out, FFTW_MEASURE);//correct flag ??
				bwdRealPlans[log2len] = fftwf_plan_dft_c2r_1d(fftLength, (fftwf_complex *)in, (float *)out, FFTW_MEASURE);
				fwdRealPlanarPlans[log2len] = fftwf_plan_guru_split_dft_r2c(1, &iod, 0, NULL, (float *)in, (float *)(out), (float *)(out) + (8 + fftLength / 2), FFTW_MEASURE);//correct flag ??
				bwdRealPlanarPlans[log2len] = fftwf_plan_guru_split_dft_c2r(1, &iod, 0, NULL, (float *)in, (float *)(in) + (8 + fftLength/2), (float *)out, FFTW_MEASURE);
			}

// OpenMP doesn't work well for realtime code on Windows :(
#pragma omp parallel default(none) private(idx) shared(direction, log2len,channels,ppBufferInput,ppBufferOutput)
#pragma omp for private(idx) schedule(static) // schedule(guided) nowait //schedule(static) 
//__pragma(loop(hint_parallel(8))) // requires VS compiler flag /QPar       {see Enable Parallel Code Generation }
			for (int idx = 0; idx < (int)channels; idx++) {
				TransformImplFFTWReal(direction, log2len, ppBufferInput[idx], ppBufferOutput[idx]);
			}

		}
		else {
			if (fwdPlans[log2len] == NULL || bwdPlans[log2len] == NULL){
				fwdPlans[log2len] = fftwf_plan_dft_1d(fftLength, (fftwf_complex *)in, (fftwf_complex *)out, FFTW_FORWARD, FFTW_MEASURE);
				bwdPlans[log2len] = fftwf_plan_dft_1d(fftLength, (fftwf_complex *)in, (fftwf_complex *)out, FFTW_BACKWARD, FFTW_MEASURE);
			}
#pragma omp parallel default(none) private(idx) shared(direction, log2len,channels,ppBufferInput,ppBufferOutput)
#pragma omp for 
			for (idx = 0; idx < channels; idx++) {
				TransformImplFFTW1Chan(direction, log2len, idx, ppBufferInput, ppBufferOutput);
			}
		}
    }
    else {
#pragma omp parallel default(none) private(idx) shared(direction, log2len,channels,ppBufferInput,ppBufferOutput)
#pragma omp for private(idx) 
		for (idx = 0; idx < channels; idx++){
            TransformImplCpu1Chan(direction, log2len, ppBufferInput[idx], ppBufferOutput[idx]);
        }
//  todo try this:      res = TransformImplCpu(direction, log2len, channels, ppBufferInput, ppBufferOutput);

    }

    return AMF_OK;
}

#ifdef USE_IPP
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplIPP(
	TAN_FFT_TRANSFORM_DIRECTION direction,
	amf_size log2len,
	amf_size channels,
	float* ppBufferInput[],
	float* ppBufferOutput[]
)
{
	IppStatus err = 0;
	Ipp8u *pSpec = NULL;
	Ipp8u *pSpecBuffer = NULL;

	// complex fft
	//ippsFFTFwd_CToC_32f
	//ippsFFTInv_CToC_32f
	//TAN_FFT_TRANSFORM_DIRECTION_FORWARD = 0,
	//	TAN_FFT_TRANSFORM_DIRECTION_BACKWARD = 1,
	//	TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD = 2,
	//	TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD = 3,
	//	TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD = 4,
	//	TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD = 5

	bool bRealFFT = (
		direction == TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD ||
		direction == TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD ||
		direction == TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD ||
		direction == TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD 
		);

	if (bRealFFT) {
		// using real FFT
		if (m_IppFFTspecsR32F[log2len][0] == NULL) {
			int specSize, specBufSize, workBufSize;
			err = ippsFFTGetSize_R_32f(log2len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &specSize, &specBufSize, &workBufSize);
			pSpec = new Ipp8u[specSize]; //(Ipp8u *)ippMalloc(specBufSize);// new Ipp8u[specSize];
			if (specBufSize > 0)
				pSpecBuffer = new Ipp8u[specBufSize]; //(Ipp8u *)ippMalloc(specBufSize);//new Ipp8u[specBufSize];
			if (workBufSize > 0) {
				// need a work buffer per thread:
				int numThreads = omp_get_max_threads();
				for (int i = 0; i < numThreads; i++) {
					m_IppFFTworkBuf[log2len][i] = new Ipp8u[workBufSize];// (Ipp8u *)ippMalloc(workBufSize);
					memset(m_IppFFTworkBuf[log2len][i], 0, workBufSize);
				}
			}

			// save spec
			int numThreads = omp_get_max_threads();
			for (int k = 0; k < numThreads; k++) {
				err = ippsFFTInit_R_32f(&m_IppFFTspecsR32F[log2len][k], log2len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, pSpec, pSpecBuffer);
			}
			//if (specBufSize > 0)
			//	ippFree(pSpecBuffer);
			if (specBufSize > 0)
				delete pSpecBuffer;
		}

		int chan;
		Ipp8u *pWorkBuffer;
		int threadId = 0;
		if (direction == TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD) {
#pragma omp parallel default(none) private(chan,pWorkBuffer,threadId) shared(channels,ppBufferInput, ppBufferOutput,log2len)
#pragma omp for
			for (chan = 0; chan < channels; chan++) {
				threadId = omp_get_thread_num();
				pWorkBuffer = m_IppFFTworkBuf[log2len][threadId];
				ippsFFTFwd_RToPack_32f(ppBufferInput[chan], ppBufferOutput[chan], m_IppFFTspecsR32F[log2len][threadId], pWorkBuffer);
			}
		}

		if (direction == TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD) {
#pragma omp parallel default(none) private(chan,pWorkBuffer,threadId) shared(channels,ppBufferInput, ppBufferOutput,log2len)
#pragma omp for
			for (chan = 0; chan < channels; chan++) {
				threadId = omp_get_thread_num();
				pWorkBuffer = m_IppFFTworkBuf[log2len][threadId];
				ippsFFTInv_PackToR_32f(ppBufferInput[chan], ppBufferOutput[chan], m_IppFFTspecsR32F[log2len][threadId], pWorkBuffer);
			}
		}
	}
	// using complex FFT
	else {
		if (m_IppFFTspecsC32F[log2len][0] == NULL) {
			int specSize, specBufSize, workBufSize;
			err = ippsFFTGetSize_C_32f(log2len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &specSize, &specBufSize, &workBufSize);
			pSpec = new Ipp8u[specSize]; //(Ipp8u *)ippMalloc(specBufSize);// new Ipp8u[specSize];
			if (specBufSize > 0)
				pSpecBuffer = new Ipp8u[specBufSize]; //(Ipp8u *)ippMalloc(specBufSize);//new Ipp8u[specBufSize];
			if (workBufSize > 0) {
				// need a work buffer per thread:
				int numThreads = omp_get_max_threads();
				for (int i = 0; i < numThreads; i++) {
					m_IppFFTworkBuf[log2len][i] = new Ipp8u[workBufSize];// (Ipp8u *)ippMalloc(workBufSize);
					memset(m_IppFFTworkBuf[log2len][i], 0, workBufSize);
				}
			}

			// save spec
			int numThreads = omp_get_max_threads();
			for (int k = 0; k < numThreads; k++) {
				err = ippsFFTInit_C_32fc(&m_IppFFTspecsC32F[log2len][k], log2len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, pSpec, pSpecBuffer);
			}
			//if (specBufSize > 0)
			//	ippFree(pSpecBuffer);
			if (specBufSize > 0)
				delete pSpecBuffer;
		}

		int chan;
		Ipp8u *pWorkBuffer;
		int threadId = 0;
		if (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) {
#pragma omp parallel default(none) private(chan,pWorkBuffer,threadId) shared(channels,ppBufferInput, ppBufferOutput,log2len)
#pragma omp for
			for (chan = 0; chan < channels; chan++) {
				threadId = omp_get_thread_num();
				pWorkBuffer = m_IppFFTworkBuf[log2len][threadId];
				ippsFFTFwd_CToC_32fc((Ipp32fc *)ppBufferInput[chan], (Ipp32fc *)ppBufferOutput[chan], m_IppFFTspecsC32F[log2len][threadId], pWorkBuffer);
			}
		}

		if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD) {
#pragma omp parallel default(none) private(chan,pWorkBuffer,threadId) shared(channels,ppBufferInput, ppBufferOutput,log2len)
#pragma omp for
			for (chan = 0; chan < channels; chan++) {
				threadId = omp_get_thread_num();
				pWorkBuffer = m_IppFFTworkBuf[log2len][threadId];
				ippsFFTInv_CToC_32fc((Ipp32fc *)ppBufferInput[chan], (Ipp32fc *)ppBufferOutput[chan], m_IppFFTspecsC32F[log2len][threadId], pWorkBuffer);
			}
		}
	}

	return AMF_OK;
}
#endif


#if 0
/**************************************************************************************************
FftCPU, adapted from code by Stephan Bernsee

COPYRIGHT 1999-2009 Stephan M. Bernsee
The Wide Open License (WOL)
Permission to use, copy, modify, distribute and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice and this license appear in all source copies.

**************************************************************************************************/
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplCpuSIMD(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
    )
{
    int nb_threads = omp_get_max_threads();

    const amf_size fftFrameSize = (amf_size)pow(2.0, (double)log2len);

    int sign = (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) ? -1 : 1;

    double wr, wi, arg;
    float  *p1, *p2, temp;
    double tr, ti, ur, ui;
    float *p1r, *p1i, *p2r, *p2i;
    amf_uint32 bitm, j, le, le2, k;// , idx;
    int idx;

    for (amf_size i = 0; i < channels; i++){
        if (ppBufferInput[i] != ppBufferOutput[i]) {
            memcpy(ppBufferOutput[i], ppBufferInput[i], fftFrameSize * 2 * sizeof(float));
        }
    }

    for (amf_size i = 2; i < 2 * fftFrameSize - 2; i += 2) {
        for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
            if (i & bitm) j++;
            j <<= 1;
        }
        if (i < j) {
            for (idx = 0; idx < channels; idx++){
                p1 = ppBufferOutput[idx] + i; p2 = ppBufferOutput[idx] + j;
                temp = *p1; *(p1++) = *p2;
                *(p2++) = temp; temp = *p1;
                *p1 = *p2; *p2 = temp;
            }
        }
    }
    int nThreads = 0;

    //omp_set_nested(1);
    //omp_set_num_threads(8);
    //#pragma omp master  
    //nThreads = omp_get_num_threads();
    //omp_set_dynamic(0);
    // omp_set_num_threads(nb_threads/2);


    //#pragma omp parallel    // default(none) num_threads(8)// private(idx) private(kc) private(a1rx8,a1ix8,a2rx8,a2ix8,trx8,tix8) private(a1r,a1i,a2r,a2i,i) shared(ur,ui,wr,wi,le,le2,j) num_threads(8)
    {
        int end = (amf_size)(log((float)fftFrameSize) / log(2.) + .5);

        //omp_set_num_threads(4);

//#pragma omp parallel default(none) private(k)  shared(end,sign,ppBufferOutput,channels) num_threads(4)
//#pragma omp for private(k)
    //for (k = 0; k < end; k++) {
    for (int k = 0; k < end; k++) {
        int  j, le, le2;
        double wr, wi, arg;
        //double tr, ti, ur, ui;
        double ti, ur, ui;
        //le <<= 1;
        le = pow(2,(k + 2));

        le2 = le >> 1;
        ur = 1.0;
        ui = 0.0;

        arg = (double)(M_PI / (le2 >> 1));
        wr = cos(arg);
        wi = sign*sin(arg);

//#pragma omp parallel default(none) private(k)  shared(ur,ui,wr,wi,le,le2,ppBufferOutput,channels) num_threads(4)
//#pragma omp for private(k)
        for (j = 0; j < le2; j += 2) {
            double tr;
            register __m256 urx8, uix8;
            urx8 = _mm256_set1_ps((float)ur);
            uix8 = _mm256_set1_ps((float)ui);



            //            #pragma omp master  
            //            nThreads = omp_get_num_threads();
            //omp_set_dynamic(0);
            //omp_set_num_threads(4);
            //
#pragma omp parallel default(none) private(idx)  shared(tr,ui,wr,wi,le,le2,j,urx8,uix8,ppBufferOutput,channels) num_threads(4)
#pragma omp for private(idx)
            for (int idx = 0; idx < channels; idx += 8){

                int threadId = omp_get_thread_num();

                //p1r = ppBufferOutput[idx] + j;
                //p1i = p1r + 1;
                //p2r = p1r + le2;
                //p2i = p2r + 1;
                //
                // use AVX and FMA intrinsics to process 8 samples per pass
                // __m256 _mm256_set1_ps (float a)
                // __m256 _mm256_set_ps (float e7, float e6, float e5, float e4, float e3, float e2, float e1, float e0)
                // __m256 _mm256_fmadd_ps (__m256 a, __m256 b, __m256 c)


                register __m256 *a1rx8, *a1ix8, *a2rx8, *a2ix8, trx8, tix8;
                _mm256_zeroall();

                float a1r[8], a1i[8], a2r[8], a2i[8], *pChD[8];
                for (int kc = 0; kc < 8; kc++){
                    pChD[kc] = ppBufferOutput[idx + kc] + j;
                }

                a1rx8 = (__m256 *) a1r;
                a1ix8 = (__m256 *) a1i;
                a2rx8 = (__m256 *) a2r;
                a2ix8 = (__m256 *) a2i;

                float *pd;
                for (amf_size i = j; i < 2 * fftFrameSize; i += le) {
                    for (int kc = 0; kc < 8; kc++){
                        //a1r[kc] = *(pChD[kc]);                  //[
                        //a1i[kc] = *(pChD[kc] + 1);              // 36%
                        //a2r[kc] = *(pChD[kc] + le2);            //
                        //a2i[kc] = *(pChD[kc] + le2 + 1);        //]
                        pd = pChD[kc];
                        a1r[kc] = *pd;                            //[
                        a1i[kc] = *(pd + 1);                      //  23%
                        pd += le2;                                //
                        a2r[kc] = *pd++;                          //]
                        a2i[kc] = *pd;
                    }

                    //tr = *p2r * ur - *p2i * ui;
                    //ti = *p2r * ui + *p2i * ur;
                    //*p2r = *p1r - tr;
                    //*p2i = *p1i - ti;
                    //*p1r += tr;
                    //*p1i += ti;   
                    trx8 = _mm256_fmaddsub_ps(*a2rx8, urx8, _mm256_mul_ps(*a2ix8, uix8));
                    tix8 = _mm256_fmadd_ps(*a2rx8, uix8, _mm256_mul_ps(*a2ix8, urx8));
                    *a2rx8 = _mm256_sub_ps(*a1rx8, trx8);
                    *a2ix8 = _mm256_sub_ps(*a1ix8, tix8);
                    *a1rx8 = _mm256_add_ps(*a1rx8, trx8);
                    *a1ix8 = _mm256_add_ps(*a1ix8, tix8);


                    //p1r += le; p1i += le;
                    //p2r += le; p2i += le;
                    for (int kc = 0; kc < 8; kc++){
                        //*(pChD[kc]) = a1r[kc];
                        //*(pChD[kc] + 1) = a1i[kc];
                        //*(pChD[kc] + le2) = a2r[kc];
                        //*(pChD[kc] + le2 + 1) = a2i[kc];
                        pd = pChD[kc];
                        *pd = a1r[kc];
                        pd[1] = a1i[kc];
                        pd += le2;
                        *pd++ = a2r[kc];
                        *pd = a2i[kc];

                        pChD[kc] += le;
                    }

                }
            }
            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }
}
    // Riemann sum.
    if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD)
    {
        register __m256 rdenom, *data;
        rdenom = _mm256_set1_ps((float)(1.0 / fftFrameSize));

        for (int idx = 0; idx < channels; idx++){
            for (int k = 0; k < 2 * fftFrameSize; k += 8){
                //ppBufferOutput[idx][k] /= fftFrameSize;
                data = (__m256 *) &ppBufferOutput[idx][k];
                *data = _mm256_mul_ps(*data, rdenom);
            }
        }
    }

    printf("%d",nThreads);
    return AMF_OK;
}

#endif

AMF_RESULT TANFFTImpl::TransformImplGPUBatched(
	TAN_FFT_TRANSFORM_DIRECTION direction, 
	amf_size log2len, 
	amf_size channels,
	cl_mem pBufferInput, 
	cl_mem pBufferOutput,
	int dataSpacing	)
{
	AMF_RETURN_IF_FALSE(m_doProcessingOnGpu, AMF_UNEXPECTED, L"Internal error");
	CLFFT_TRANSFORM_TYPE transformType = CLFFT_TRANSFORM_COMPLEX;
	clfftDirection clDir = CLFFT_FORWARD;
	switch (direction) {
	case TAN_FFT_TRANSFORM_DIRECTION_FORWARD:
		transformType = CLFFT_TRANSFORM_COMPLEX;
		clDir = CLFFT_FORWARD;
		break;
	case TAN_FFT_TRANSFORM_DIRECTION_BACKWARD:
		transformType = CLFFT_TRANSFORM_COMPLEX;
		clDir = CLFFT_BACKWARD;
		break;
	case TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD:
		transformType = CLFFT_TRANSFORM_R2C_FORWARD;
		clDir = CLFFT_FORWARD;
		break;
	case TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD:
		transformType = CLFFT_TRANSFORM_C2R_BACKWARD;
		clDir = CLFFT_BACKWARD;
		break;
	}

//HACK debug
	//if (channels > 2) channels = 2;

    clfftPlanHandle plan = (clfftPlanHandle)getclFFTPlan(static_cast<int>(log2len), channels, transformType, dataSpacing);
	//clfftSetPlanBatchSize(plan, channels);
	cl_command_queue cmdQueue = nullptr;
	if (m_useConvQueue) {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
	}
	else {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
	}
// HACK OUT for debug
//#if 0
	clfftStatus status = clfftEnqueueTransform(plan,
		clDir,
		1, /*num queues and out events*/
		&cmdQueue, /*command queue*/
		0, /*num wait events*/
		NULL, /*wait events*/
		NULL, /*out event*/
		&pBufferInput,/*input buf*/
		&pBufferOutput, /*output buf*/
		NULL /*tmp buffer*/);
	AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_UNEXPECTED);
//#endif

	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplGpu(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    cl_mem pBufferInput[],
    cl_mem pBufferOutput[]
    )
{
	AMF_RESULT res;
    AMF_RETURN_IF_FALSE(m_doProcessingOnGpu, AMF_UNEXPECTED, L"Internal error");
	const amf_size requiredChannelLengthInfloat = (1 << (log2len + 1)); // Double the size for complex.
	const amf_size requiredChannelLengthInBytes = requiredChannelLengthInfloat* sizeof(float);
	cl_int status;
	AdjustInternalBufferSize(log2len + 1, channels);
	cl_command_queue cmdQueue = nullptr;
	if (m_useConvQueue) {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
	}
	else {
		cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
	}


	for (amf_uint32 i = 0; i < channels; i++){
		clEnqueueCopyBuffer(cmdQueue, pBufferInput[i], m_pInputsOCL, 0, i*requiredChannelLengthInBytes,requiredChannelLengthInBytes, 0, NULL, NULL);
	}
	res = TransformImplGPUBatched(direction, log2len, channels, m_pInputsOCL, m_pOutputsOCL,0);
	for (amf_uint32 i = 0; i < channels; i++){
		//hack bug? clEnqueueCopyBuffer(cmdQueue, m_pInputsOCL, pBufferOutput[i], i*requiredChannelLengthInBytes, 0, requiredChannelLengthInBytes, 0, NULL, NULL);
        clEnqueueCopyBuffer(cmdQueue, m_pOutputsOCL, pBufferOutput[i], i*requiredChannelLengthInBytes, 0, requiredChannelLengthInBytes, 0, NULL, NULL);
    }
	return res;
}
// -----------------------------------------------------------------------------------------------
void TANFFTImpl::clearInternalBuffers()
{
	//release ocl bufers
	if (m_pInputsOCL != nullptr){
		clReleaseMemObject(m_pInputsOCL);
		m_pInputsOCL = nullptr;
	}
	if (m_pOutputsOCL != nullptr){
		clReleaseMemObject(m_pOutputsOCL);
		m_pOutputsOCL = nullptr;
	}
}

void TANFFTImpl::AdjustInternalBufferSize(size_t desireSizeInSampleLog2, size_t numOfChannels)
{
	const amf_size requiredChannelLengthInSamples = (1 << (desireSizeInSampleLog2 )); // Double the size for complex.
	const amf_size requiredChannelLengthInBytes = requiredChannelLengthInSamples* sizeof(float);
	const amf_size requiredBufferLengthInBytes = requiredChannelLengthInBytes * numOfChannels;
	if (requiredBufferLengthInBytes > m_iInternalBufferSizeInBytes)
	{
		//release old bufers
		clearInternalBuffers();
		// create new buffers
		m_iInternalBufferSizeInBytes = requiredChannelLengthInBytes;
		cl_context context = m_pContextTAN->GetOpenCLContext();
		cl_command_queue cmdQueue = nullptr;
		if (m_useConvQueue) {
			cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
		}
		else {
			cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
		}

		cl_int status;
		m_pInputsOCL = clCreateBuffer(context, CL_MEM_READ_WRITE, requiredBufferLengthInBytes, NULL, &status);
		m_pOutputsOCL = clCreateBuffer(context, CL_MEM_READ_WRITE, requiredBufferLengthInBytes, NULL, &status);
		float fill = 0.0;
		status = clEnqueueFillBuffer(cmdQueue, m_pInputsOCL, &fill, sizeof(float), 0, requiredBufferLengthInBytes, 0, NULL, NULL);
		status = clEnqueueFillBuffer(cmdQueue, m_pOutputsOCL, &fill, sizeof(float), 0, requiredBufferLengthInBytes, 0, NULL, NULL);
	}
}

//-------------------------------------------------------------------------------------------------
