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

#include "utilities.h"

#include "samples/src/GPUUtilities/GpuUtilities.h"
const uint32_t c_modesCnt = 2;

int main(int argc, char* argv[])
{
	//comment out unused code:
	//errno_t errno;
	//FILE *fpLog = NULL;
	//char logfname[] = "TALibTestConvolutionLog.html";
	//errno = fopen_s(&fpLog, logfname, "w+");

    char *infilename = NULL;// = "bip5chan.wav";
    char *respName = NULL;// = "rev5chan.wav";
    char *outName = NULL;// = "out5chan.wav";
    char *configName = NULL;
    FILE *fpConfig = NULL;

	int blockLength = 256;

	uint32_t SamplesPerSec, resSamplesPerSec;
	uint16_t BitsPerSample, resBitsPerSample;
	uint16_t NChannels, NResChannels;
	uint32_t NSamples, NResSamples;
	unsigned char *pSamples;
	unsigned char *pResponse;
	float **pfSamples = NULL;
	float **pfResponse = NULL;

	bool gpu = 0;

    if (argc < 5 || !(!strcmp(argv[1], "gpu") || !strcmp(argv[1], "cpu")))
	{
		puts("syntax:\n");
        puts("TALibTestConvolution <gpu | cpu> infile.wav response.wav output.wav [updflagsched.txt]\n");
		return(0);
	}

    infilename = argv[2];
    respName = argv[3];
    outName = argv[4];

    if (argc >= 6){
        configName = argv[5];
        fpConfig = fopen(configName, "r");
        if (!fpConfig)
        {
            puts("Error cannot open this config file, try providing absolute path:\n");
            puts(configName);
            return 0;
        }
    }

	gpu = !strcmp(argv[1], "gpu");
	if (!gpu)
	{
		printf("CPU \n");
	}
	else
	{
		printf("GPU \n");
	}

    ReadWaveFile(infilename, SamplesPerSec, BitsPerSample, NChannels, NSamples, &pSamples, &pfSamples);
    ReadWaveFile(respName, resSamplesPerSec, resBitsPerSample, NResChannels, NResSamples, &pResponse, &pfResponse);

	if (SamplesPerSec != resSamplesPerSec || BitsPerSample != resBitsPerSample || NChannels != NResChannels) {
		printf("format of source [%dHz:%dbits:%dchans] and response [%dHz:%dbits:%dchans] don't match!\n", SamplesPerSec, BitsPerSample, NChannels, resSamplesPerSec, resBitsPerSample, NResChannels);
		return(0);
	}

    // 10 repeats:
    const int NReps = 3;
    float **inStreams = new float *[NChannels];
    float **outStreams = new float *[NChannels];
    float **tmpIn = new float *[NChannels];
    float **tmpOut = new float *[NChannels];
    float updTime = 0;
    amf_uint32 *updFlags = new amf_uint32[NChannels];
    memset(updFlags, 0, NChannels*sizeof(int));

    for (int i = 0; i < NChannels; i++){
        float *pIn;
        inStreams[i] = tmpIn[i] = pIn = new float[NSamples * NReps + blockLength];
        outStreams[i] = tmpOut[i] = new float[NSamples * NReps + blockLength];

        memset(tmpIn[i], 0, (NSamples * NReps + blockLength)*sizeof(float));
        memset(tmpOut[i], 0, (NSamples * NReps + blockLength)*sizeof(float));


        for (int j = 0; j < NReps; j++){
            memcpy(pIn, pfSamples[i], NSamples*sizeof(float));
            pIn += NSamples;
        }
    }

    NSamples *= NReps;

	TANContextPtr taGPU;
	TANContextPtr taCPU = NULL;

	if (!gpu)
	{
		TANCreateContext(TAN_FULL_VERSION, &taCPU);
	}
	if (gpu)
	{
		TANCreateContext(TAN_FULL_VERSION, &taGPU);
		cl_context gpu_context;
		cl_device_id device_id;
		getDeviceAndContext(0, &gpu_context, &device_id);
		taGPU->InitOpenCL(createQueue(gpu_context, device_id), createQueue(gpu_context, device_id));
	}

	amf_uint32 flags[] = { 0, 0 };
	TANConvolutionPtr convCPU = NULL;
	TANConvolutionPtr convGPU = NULL;
	if (gpu)
	{
		TANCreateConvolution(taGPU, &convGPU);
        convGPU->Init(TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL, NResSamples, blockLength, NChannels);
	}else
	{
		TANCreateConvolution(taCPU, &convCPU);
        convCPU->Init(TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD, NResSamples, blockLength, NChannels);

	}



    fscanf(fpConfig, "%f ", &updTime);// it should be zero, its ignored anyway
    updTime = 0;
    bool update = true;
    bool end_of_config_file = false;
    for (int ns = 0; ns < NSamples; ns += blockLength)
	{
        update = false;
        float curTime = (float)ns / (float)SamplesPerSec;
        if (curTime > updTime && !end_of_config_file){
            for (int i = 0; i < NChannels; i++){
                fscanf(fpConfig, "%d ", &updFlags[i]);
            }
            if (fscanf(fpConfig, "%f ", &updTime) == EOF)// read the next
            {
                end_of_config_file = true;
            }
            update = true;
        }
        if (update)
        {
            if (gpu)
            {
                convGPU->UpdateResponseTD(pfResponse, NResSamples, updFlags, NULL);
                //convGPU->UpdateResponseTD(pfResponse, NResSamples, NULL, NULL);
                printf("UPLOAD: %d\n", ns);
            }
            else
            {
                convCPU->UpdateResponseTD(pfResponse, NResSamples, updFlags, NULL);
                printf("UPLOAD: %d\n", ns);
            }

        }


		if (gpu)
		{
            convGPU->Process(tmpIn, tmpOut, blockLength, updFlags, NULL);
		} else
        {
            convCPU->Process(tmpIn, tmpOut, blockLength, updFlags, NULL);

		}


		for (int nch = 0; nch < NChannels; nch++)
        {
            tmpIn[nch] += blockLength;
            tmpOut[nch] += blockLength;
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


    WriteWaveFileF(outName, SamplesPerSec, NChannels, BitsPerSample, NSamples, outStreams);


	delete[] pResponse;
	for (int n = 0; n < NChannels; n++){
		delete[] pfResponse[n];
	}
	delete pfResponse;

	delete[] pSamples;
	for (int n = 0; n < NChannels; n++){
		delete[] pfSamples[n];
	}
	delete pfSamples;

	for (int n = 0; n < NChannels; n++){
        delete[] outStreams[n];
        delete[] inStreams[n];

	}
    delete[] outStreams;
    delete[] inStreams;

	//_spawnlp(_P_NOWAIT, "C:/Program Files/Internet Explorer/iexplore.exe", &logfname[0], &logfname[0], NULL);

	return 0;
}

