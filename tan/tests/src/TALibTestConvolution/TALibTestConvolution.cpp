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

// TALibTestConvolution.cpp : Defines the entry point for the console application.
//

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#ifdef _WIN32
  #include <SDKDDKVer.h>
#endif

#include <memory.h>
#include <math.h>

#if !defined(__APPLE__) && !defined(__MACOSX)
    #include <omp.h>
#endif

#if defined(_WIN32)
    #include <process.h>
#else
    #include <pthread.h>
#endif

#if defined(METRO_APP)
    #include <ppl.h>
    #include <ppltasks.h>
#endif

#include "tanlibrary/include/TrueAudioNext.h"
using namespace amf;

#include "FileUtility.h"
#include "wav.h"

#include "samples/src/common/utilities.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
const uint32_t c_modesCnt = 2;

int main(int argc, char* argv[])
{
	//comment out unused code:
	//errno_t errno;
	//FILE *fpLog = NULL;
	//char logfname[] = "TALibTestConvolutionLog.html";
	//errno = fopen_s(&fpLog, logfname, "w+");

	struct {
		char abrev[20];
		enum TAN_CONVOLUTION_METHOD method;
		bool gpu;
	} methodTable[6] = { 
		{"CPU-OV",TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD, false },
		{"CPU-UN",TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_UNIFORM, false },
		{"CPU-NU",TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_NONUNIFORM, false },
		{"GPU-OV",TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD, true },
		{"GPU-UN",TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED, true },
		{"GPU-NU",TAN_CONVOLUTION_METHOD_FHT_NONUNIFORM_PARTITIONED, true },
	};

	getchar();

	char *inFileName, *outFileName, *responseFileName, *responseFileName2;
	//std::vector<std::string> outFileNm(2);
	int n_samples = 128;



	uint32_t SamplesPerSec, resSamplesPerSec;
	uint16_t BitsPerSample, resBitsPerSample;
	uint16_t NChannels, NResChannels;
	uint32_t NSamples, NResSamples;
	unsigned char *pSamples;
	unsigned char *pResponse;
	float **pfSamples = NULL;
	float **pfResponse = NULL;
	float **pfResponse2 = NULL;
	bool gpu = 0;
	enum TAN_CONVOLUTION_METHOD method = TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	char * abrv = NULL;

	if (argc < 5){
		puts("syntax:\n");
		puts("TALibTestConvolution method inFile outFile responseFile1 [responseFile2]\n");
		puts("method = [CPU-OV | CPU-UN | CPU-NU | GPU-OV | GPU-UN | GPU-NU]\n"); 
		puts("Where: OV = Overlap-Add, UN = Uniform Partitioned, NU = Non-Uniform Partitioned");
		return(0);
	}
	
	for (int i = 0; i < sizeof(methodTable) / sizeof(methodTable[0]); i++) {
		if (strncmp(argv[1], methodTable[i].abrev, 20) == 0) {
			method = methodTable[i].method;
			gpu = methodTable[i].gpu;
			abrv = methodTable[i].abrev;
		}
	}

	inFileName = argv[2];
	outFileName = argv[3];

	responseFileName = argv[4];
	responseFileName2 = argv[5];

	printf("inFile: %s outFile: %s responseFile: %s \n", inFileName, outFileName, responseFileName);

	ReadWaveFile(responseFileName, resSamplesPerSec, resBitsPerSample, NResChannels, NResSamples, &pResponse, &pfResponse);
	ReadWaveFile(inFileName, SamplesPerSec, BitsPerSample, NChannels, NSamples, &pSamples, &pfSamples);

	if(responseFileName2)
	{
		ReadWaveFile(responseFileName2, resSamplesPerSec, resBitsPerSample, NResChannels, NResSamples, &pResponse, &pfResponse2);
	}

	int n_blocks = (NSamples + n_samples - 1) / n_samples;
	//std::vector<float*> inp_hist(NChannels);
	//std::vector<float*> out_c(NChannels);
	float **pfOutSamples = new float*[NChannels];
	float **pfOutTemp = new float*[NChannels];
	float **pfInTemp = new float*[NChannels];
	for (int n = 0; n < NChannels; n++) {


		//inp_hist[n] = new float[(n_blocks + 1) * n_samples];
		//memset(inp_hist[n], 0, (n_blocks + 1) * n_samples * sizeof(float));
		//out_c[n] = new float[n_samples];
		pfOutSamples[n] = new float[NSamples];
		memset(pfOutSamples[n], 0, NSamples*sizeof(float));
		pfOutTemp[n] = pfOutSamples[n];
		pfInTemp[n] = pfSamples[n];

	}

	if (SamplesPerSec != resSamplesPerSec || BitsPerSample != resBitsPerSample || NChannels != NResChannels) {
		printf("format of source [%dHz:%dbits:%dchans] and response [%dHz:%dbits:%dchans] don't match!\n", SamplesPerSec, BitsPerSample, NChannels, resSamplesPerSec, resBitsPerSample, NResChannels);
		return(0);
	}

	TANContextPtr taGPU;
	TANContextPtr taCPU = NULL;

	if (gpu)
	{
		TANCreateContext(TAN_FULL_VERSION, &taGPU);
		cl_context gpu_context;
		cl_device_id device_id;
		getDeviceAndContext(0, &gpu_context, &device_id);
		taGPU->InitOpenCL(createQueue(gpu_context, device_id), createQueue(gpu_context, device_id));
	}
	else {
		TANCreateContext(TAN_FULL_VERSION, &taCPU);
	}

	amf_uint32 flags[] = { 0, 0 };
	TANConvolutionPtr convCPU = NULL;
	TANConvolutionPtr convGPU = NULL;
	if (!gpu)
	{
		TANCreateConvolution(taCPU, &convCPU);
		convCPU->Init(method, NResSamples, n_samples, NChannels);

	}
	if (gpu)
	{
		TANCreateConvolution(taGPU, &convGPU);
		convGPU->Init(method, NResSamples, n_samples, NChannels);
	}




	int n_IR_updates = 10;
	int update_interv = n_blocks / n_IR_updates;
	int mismatch = 0;
	int b = 0;
	int ns = 0;
	float ** pfResp = NULL;

	for (ns = 0; ns < NSamples && !mismatch; ns += n_samples, b++)
	{
		if (b % update_interv == 0)
		{
			pfResp = ((b & 1) == 0 || !pfResponse2) ? pfResponse : pfResponse2;
			if (!gpu)
			{
				convCPU->UpdateResponseTD(pfResp, NResSamples,NULL,NULL);
				printf("UPLOAD: %d\n", ns);
			}
			if (gpu)
			{
				convGPU->UpdateResponseTD(pfResp, NResSamples,NULL,NULL);
				printf("UPLOAD: %d\n", ns);
			}


		}

		amf_size actual_samples = (NSamples - ns >= n_samples) ? n_samples : NSamples - ns;
		if (actual_samples < n_samples)
		{
			static int oh = 0;
			oh++;
		}

		if (!gpu)
		{
			convCPU->Process(pfInTemp, pfOutTemp, actual_samples, flags,NULL);

		}

		if (gpu)
		{
			convGPU->Process(pfInTemp, pfOutTemp, actual_samples, flags,NULL);
		}

		for (int nch = 0; nch < NChannels && !mismatch; nch++){

			if (!gpu)
			{
				pfInTemp[nch] += n_samples;
				pfOutTemp[nch] += n_samples;
			}

			if (gpu)
			{
				pfInTemp[nch] += n_samples;
				pfOutTemp[nch] += n_samples;
			}


		}


	}

	if (taGPU)
	{
		taGPU->Terminate();
	}

	if (taCPU)
	{
		taCPU->Terminate();
	}


	WriteWaveFileF(outFileName, SamplesPerSec, NChannels, BitsPerSample, NSamples, pfOutSamples);


	delete[] pResponse;
	for (int n = 0; n < NChannels; n++){
		delete[] pfResponse[n];
	}
	delete pfResponse;

	delete[] pSamples;
	for (int n = 0; n < NChannels; n++){
		delete[] pfSamples[n];
		//delete[] inp_hist[n];
		//delete[] out_c[n];

	}
	delete pfSamples;

	for (int n = 0; n < NChannels; n++){
		delete[] pfOutSamples[n];

	}
	delete[]pfOutSamples;
	delete[] pfOutTemp;
	delete[] pfInTemp;


	//_spawnlp(_P_NOWAIT, "C:/Program Files/Internet Explorer/iexplore.exe", &logfname[0], &logfname[0], NULL);

	return 0;
}

