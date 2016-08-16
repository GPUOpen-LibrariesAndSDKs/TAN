// TALibTestConvolution.cpp : Defines the entry point for the console application.
//

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#include <memory.h>
#include <process.h>
#include <math.h>
#include <vector>
#include <omp.h>
#include <memory>

#include "tanlibrary/include/TrueAudioNext.h"
using namespace amf;

#include "samples/src/common/wav.h"
#include "samples/src/common/utilities.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
const UINT c_modesCnt = 2;

static
int DirectConv(float * out, float * in, int index, int block_sz, float * kernel, int kernel_sz, int n_blocks)
{
	int ret = 0;
	int b, j, c;
	int in_sz = n_blocks* block_sz;
#pragma omp parallel for private(j,c)
	for (b = 0; b < block_sz; b++) {
		out[b] = 0;
		double o = 0;
		// limited by real kernel length
		for (j = index * block_sz + b, c = 0; c < kernel_sz; c++, j--) {
			j = (j < 0) ? in_sz - 1 : j;

			o += in[j] * kernel[c];

		}

		out[b] = (float)o;
	}

	return ret;
}


int main(int argc, char* argv[])
{
	errno_t errno;
	FILE *fpLog = NULL;
	char logfname[] = "TALibTestConvolutionLog.html";
	//errno = fopen_s(&fpLog, logfname, "w+");
	char *inFileName, *outFileName, *responseFileName, *responseFileName2;
	std::vector<std::string> outFileNm(2);
	int n_samples = 128;
	int SamplesPerSec, resSamplesPerSec;
	int BitsPerSample, resBitsPerSample;
	int NChannels, NResChannels;
	long NSamples, NResSamples;
	unsigned char *pSamples;
	unsigned char *pResponse;
	float **pfSamples = NULL;
	float **pfResponse = NULL;
	float **pfResponse2 = NULL;
	int gpu = 0;

	if (argc < 5){
		puts("syntax:\n");
		puts("TALibTestConvolution <gpu | cpu> inFile outFile responseFile1 [responseFile2]\n");
		return(0);
	}
	if (!(!strcmp(argv[1], "gpu") || !strcmp(argv[1], "cpu") || !strcmp(argv[1], "both")))
	{
		puts("syntax:\n");
		puts("TALibTestConvolution <gpu | cpu | both> inFile outFile responseFile1 [responseFile2]\n");
		return(0);
	}

	gpu = (!strcmp(argv[1], "gpu")) ? 1 : (!strcmp(argv[1], "both")) ? 2 : 0;
	if (gpu == 0)
	{
		printf("CPU ONLY\n");
	}
	else if (gpu == 1)
	{
		printf("GPU ONLY\n");
	}
	else
	{
		printf("CPU/GPU cross-verification\n");
	}

	inFileName = argv[2];
	outFileName = argv[3];
	size_t point_pos = std::string(argv[3]).find_last_of('.');
	outFileNm[0] = std::string(argv[3]).substr(0, point_pos);
	outFileNm[1] = outFileNm[0];
	std::string ext = std::string(argv[3]).substr(point_pos);
	outFileNm[0] += std::string("CPU") + ext;
	outFileNm[1] += std::string("GPU") + ext;

	responseFileName = argv[4];
	responseFileName2 = argv[5];

	ReadWaveFile(responseFileName, &resSamplesPerSec, &resBitsPerSample, &NResChannels, &NResSamples, &pResponse, &pfResponse);
	ReadWaveFile(inFileName, &SamplesPerSec, &BitsPerSample, &NChannels, &NSamples, &pSamples, &pfSamples);
	if (responseFileName2){
		ReadWaveFile(responseFileName2, &resSamplesPerSec, &resBitsPerSample, &NResChannels, &NResSamples, &pResponse, &pfResponse2);
	}

	int n_blocks = (NSamples + n_samples - 1) / n_samples;
	std::vector<float*> inp_hist(NChannels);
	std::vector<float*> out_c(NChannels);
	float **pfOutSamples = new float*[NChannels];
	float **pfOutTemp = new float*[NChannels];
	float **pfInTemp = new float*[NChannels];
	float **pfOutSamplesCPU = new float*[NChannels];
	float **pfOutTempCPU = new float*[NChannels];
	float **pfInTempCPU = new float*[NChannels];
	for (int n = 0; n < NChannels; n++) {


		inp_hist[n] = new float[(n_blocks + 1) * n_samples];
		memset(inp_hist[n], 0, (n_blocks + 1) * n_samples * sizeof(float));
		out_c[n] = new float[n_samples];
		pfOutSamples[n] = new float[NSamples];
		memset(pfOutSamples[n], 0, sizeof(float));
		pfOutTemp[n] = pfOutSamples[n];
		pfInTemp[n] = pfSamples[n];

		pfOutSamplesCPU[n] = new float[NSamples];
		memset(pfOutSamplesCPU[n], 0, sizeof(float));
		pfOutTempCPU[n] = pfOutSamplesCPU[n];
		pfInTempCPU[n] = pfSamples[n];

	}

	if (SamplesPerSec != resSamplesPerSec || BitsPerSample != resBitsPerSample || NChannels != NResChannels) {
		printf("format of source [%dHz:%dbits:%dchans] and response [%dHz:%dbits:%dchans] don't match!\n", SamplesPerSec, BitsPerSample, NChannels, resSamplesPerSec, resBitsPerSample, NResChannels);
		return(0);
	}

	TANContextPtr taGPU;
	TANContextPtr taCPU = NULL;

	if (gpu == 0 || gpu == 2)
	{
		TANCreateContext(TAN_FULL_VERSION, &taCPU);
	}
	if (gpu == 1 || gpu == 2)
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
	if (gpu == 0 || gpu == 2)
	{
		TANCreateConvolution(taCPU, &convCPU);
		convCPU->Init(TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD, NResSamples, n_samples, NChannels);

	}
	if (gpu == 1 || gpu == 2)
	{
		TANCreateConvolution(taGPU, &convGPU);
		convGPU->Init(TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED, NResSamples, n_samples, NChannels);
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
			if (gpu == 0 || gpu == 2)
			{
				convCPU->UpdateResponseTD(pfResp, NResSamples,NULL,NULL);
				if (gpu == 0)
				{
					printf("UPLOAD: %d\n", ns);
				}
			}
			if (gpu == 1 || gpu == 2)
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

		if (gpu == 0 || gpu == 2)
		{
			convCPU->Process(pfInTempCPU, pfOutTempCPU, actual_samples, flags,NULL);

		}

		if (gpu == 1 || gpu == 2)
		{
			convGPU->Process(pfInTemp, pfOutTemp, actual_samples, flags,NULL);
		}

		for (int nch = 0; nch < NChannels && !mismatch; nch++){

			if (gpu == 2)
			{
				double diff2 = 0;
				double cpu2 = 0;
				int index = b % (n_blocks + 1);
				memset(inp_hist[nch] + index * n_samples, 0, n_samples * sizeof(float));
				memcpy(inp_hist[nch] + index * n_samples, pfInTempCPU[nch], actual_samples * sizeof(float));

				DirectConv(out_c[nch], inp_hist[nch], index, n_samples, pfResp[nch], NResSamples, (n_blocks + 1));

				for (int s = 0; s < actual_samples && !mismatch; s++)
				{
#if 0
					if (abs(out_c[nch][s] - pfOutTempCPU[nch][s]) > 0.001)
					{
						printf("CPU-em/CPU mismatch c=%d s=%d cev=%f cv=%f\n", nch, ns + s, out_c[nch][s], pfOutTempCPU[nch][s]);
						//						mismatch = 1;
						//						break;
					}
					//#else
					if (abs(out_c[nch][s] - pfOutTemp[nch][s]) > 0.01)
					{
						printf("CPU_em/GPU mismatch c=%d s=%d cev=%f gv=%f\n", nch, ns + s, out_c[nch][s], pfOutTemp[nch][s]);
						//						mismatch = 1;
						//						break;
					}


#endif
					diff2 += (pfOutTemp[nch][s] - pfOutTempCPU[nch][s]) * (pfOutTemp[nch][s] - pfOutTempCPU[nch][s]);
					cpu2 += pfOutTempCPU[nch][s] * pfOutTempCPU[nch][s];
				}

#if 1
				//				double err = (cpu2 != 0) ? diff2 / cpu2 : 0;
				if (sqrt(diff2) > 0.01)
				{
					for (int s = 0; s < actual_samples && !mismatch; s++)
					{

						if (abs(pfOutTemp[nch][s] - pfOutTempCPU[nch][s]) > 0.001)
						{
							printf("CPU/GPU mismatch c=%d s=%d cv=%f gv=%f\n", nch, ns + s, pfOutTempCPU[nch][s], pfOutTemp[nch][s]);
							break;
						}

					}
					mismatch = 1;
				}
#endif
			}

			if (gpu == 0 || gpu == 2)
			{
				pfInTempCPU[nch] += n_samples;
				pfOutTempCPU[nch] += n_samples;
			}

			if (gpu == 1 || gpu == 2)
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


	if (gpu == 0 || gpu == 2)
	{
		WriteWaveFileF((char*)outFileNm[0].c_str(), SamplesPerSec, NChannels, BitsPerSample, NSamples, pfOutSamplesCPU);
	}

	if (gpu == 1 || gpu == 2)
	{
		WriteWaveFileF((char*)outFileNm[1].c_str(), SamplesPerSec, NChannels, BitsPerSample, NSamples, pfOutSamples);
	}


	delete[] pResponse;
	for (int n = 0; n < NChannels; n++){
		delete[] pfResponse[n];
	}
	delete pfResponse;

	delete[] pSamples;
	for (int n = 0; n < NChannels; n++){
		delete[] pfSamples[n];
		delete[] inp_hist[n];
		delete[] out_c[n];

	}
	delete pfSamples;

	for (int n = 0; n < NChannels; n++){
		delete[] pfOutSamples[n];

	}
	delete[]pfOutSamples;
	delete[] pfOutTemp;
	delete[] pfInTemp;
	delete[] pfOutSamplesCPU;
	delete[] pfOutTempCPU;
	delete[] pfInTempCPU;


	//_spawnlp(_P_NOWAIT, "C:/Program Files/Internet Explorer/iexplore.exe", &logfname[0], &logfname[0], NULL);

	return 0;
}

