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

#define MAX_RESPONSE_LENGTH 131072
#define FILTER_SAMPLE_RATE 48000

#include "ReverbProcessor.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
#include "samples/src/common/GpuUtils.h"
#include "FileUtility.h"
#include "wav.h"

#if defined(_WIN32)
    #include "../common/WASAPIPlayer.h"
#else
	#if !defined(__MACOSX) && !defined(__APPLE__)
		#include "../common/AlsaPlayer.h"
	#endif
#endif

#ifdef ENABLE_PORTAUDIO
	#include "../common/PortPlayer.h"
#endif

#include <CL/cl.h>

#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>

#ifdef _WIN32
  #include <AclAPI.h>
  #include <process.h>
#endif

#define AMF_RETURN_IF_FAILED(x,y) \
{ \
    AMF_RESULT tmp = (x); \
    if (tmp != AMF_OK) { \
        printf(y); \
        return tmp; \
	    } \
}

#define OCL_RETURN_IF_FAILED(x,y) \
{ \
    cl_int tmp = (x); \
    if (tmp != CL_SUCCESS) { \
        printf(y); \
        return tmp; \
		    } \
}

#define STD_RETURN_IF_NULL(x,message, ret) \
{ \
    if (x == nullptr) { \
        printf(message); \
        return ret; \
			    } \
}

#define STD_RETURN_IF_NOT_ZERO(x,message, ret) \
{ \
    if (x != 0) { \
        printf(message); \
        return ret; \
				    } \
}

#define STD_RETURN_IF_FALSE(x,message, ret) \
{ \
    if (!x) { \
        printf(message); \
        return ret; \
					    } \
}


#define IF_NOT_NULL_DELETE(x) if(x!=nullptr) {delete x; x=nullptr;}
#define IF_NOT_NULL_DELETE_ARRAY(x) if(x!=nullptr) {delete []x; x=nullptr;}

AMF_RESULT ReverbProcessor::init(eOperationMode mode, int deviceIDx, size_t fftLengthInSamples, size_t bufferLengthInSamples, size_t numOfChannel)
{
	if (mode != m_eOperationMode || m_iDeviceIDx != deviceIDx || bufferLengthInSamples != m_iConvolutionLengthInSample || m_iFilterLengthInComplex != fftLengthInSamples || m_iNumOfChannels != numOfChannel)
		TANteardown();
	else
		return AMF_OK;
	AMF_RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &m_pTANContext), "Failed to create tan context\n");
	amf::TAN_CONVOLUTION_METHOD eConvolutionMethod;
	m_eOperationMode = mode;
	m_iDeviceIDx = deviceIDx;
	mode == eCPU ? eConvolutionMethod = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD : eConvolutionMethod = amf::TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED;
	if (mode == eGPU)
	{
		// Initialize TAN Context
		cl_context GPUContext;
		cl_device_id GPUID;
		cl_command_queue GPUGeneralQueue;
		getDeviceAndContext(deviceIDx, &GPUContext, &GPUID);
		GPUGeneralQueue = createQueue(GPUContext, GPUID);
		AMF_RETURN_IF_FAILED(m_pTANContext->InitOpenCL(GPUGeneralQueue, GPUGeneralQueue), "Failed to initialize OpenCL for tan context");
	}
	AMF_RETURN_IF_FAILED(TANCreateConvolution(m_pTANContext, &m_pTANConvolution), "Failed to create tan convolution\n");
	AMF_RETURN_IF_FAILED(TANCreateFFT(m_pTANContext, &m_pTANFFT), "Failed to create tan FFT\n");
	AMF_RETURN_IF_FAILED(TANCreateMath(m_pTANContext, &m_pTANMath), "Failed to create tan math\n");
	AMF_RETURN_IF_FAILED(TANCreateConverter(m_pTANContext, &m_pTANConverter), "Failed to create tan converter\n");
	AMF_RETURN_IF_FAILED(TANCreateFilter(m_pTANContext, &m_pTANFilter), "Failed to create TAN Filter\n");
	AMF_RETURN_IF_FAILED(m_pTANConvolution->Init(eConvolutionMethod, fftLengthInSamples, bufferLengthInSamples, numOfChannel), "TAN convolution->init() Failed\n");
	AMF_RETURN_IF_FAILED(m_pTANFFT->Init(), "FFT Init() Failed\n");
	AMF_RETURN_IF_FAILED(m_pTANMath->Init(), "Math Init() Failed\n");
	AMF_RETURN_IF_FAILED(m_pTANConverter->Init(), "Converter Init() Failed\n");
	AMF_RETURN_IF_FAILED(m_pTANFilter->Init(), "Filter Init() Failed\n");
	m_iConvolutionLengthInSample = bufferLengthInSamples;
	m_iNumOfChannels = numOfChannel;
	adjustInternalFilterBuffer(fftLengthInSamples, numOfChannel);
	// Insert one all pass filter into the system so that when no filter passed in, system will still function well

	for (int i = 0; i < m_vFDFilterList.size(); i++)
	{
		if (m_vFDFilterList[i] != nullptr)
		{
			deleteFilter(i);
		}
	}
	float** allpass = nullptr;
	AMF_RETURN_IF_FAILED(generateAllPassFilterFD(&allpass, m_iFilterLengthInFloat, m_iNumOfChannels), "Failed to generate all pass filter\n");
	AMF_RESULT AMF_ERR;
	addFilterFD(allpass, &AMF_ERR);
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::playerPlay(char* wavFileName)
{
	if (!m_bIsPlaying)
	{
		loadWAVFile(wavFileName);
		size_t requireSamplePerChannel = m_iInputSizeInBytesPerChannel / sizeof(short);
		adjustInternalInputBuffer(requireSamplePerChannel, m_iNumOfChannels);
		processFilter();
		m_bIsPlaying = true;
		m_threadProcessing = new std::thread(&ReverbProcessor::playerPlayInternal, this);
//		std::thread thread();
//		//thread.join();
	}
	else
	{
		AMF_RETURN_IF_FAILED(AMF_FAIL, "Player Already Started");
	}
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::playerProcessToWAV(char* inWAVFile, char* outWAVFile)
{
	loadWAVFile(inWAVFile);
	size_t requireSamplePerChannel = m_iInputSizeInBytesPerChannel / sizeof(float);
	adjustInternalInputBuffer(requireSamplePerChannel, m_iNumOfChannels);
	processFilter();
	short *pOut;
	short *pWaves;
	size_t totalNumOfBytes = 0;
	
	uint32_t samplePerSec = 0;
	uint16_t bitPerSample = 0;
	uint16_t numOfChannel = 0;
	uint32_t nOfSample = 0;
	float** pfile;
	ReadWaveFile(inWAVFile, samplePerSec, bitPerSample, numOfChannel, nOfSample, &m_pInputRawBuffer, &pfile);

	totalNumOfBytes = nOfSample * sizeof(short) * numOfChannel; // stereo short samples
	size_t totalNumOfShort = nOfSample * numOfChannel;
	pWaves = (short *)(m_pInputRawBuffer);
	pOut = new short[totalNumOfShort];
	short* outStart = pOut;
	memset(pOut, 0, totalNumOfBytes);

	size_t processBufferSizeInBytes = m_iConvolutionLengthInSample * m_iNumOfChannels * sizeof(short);
	size_t processBufferSizeInShort = processBufferSizeInBytes / sizeof(short);
	size_t processedBytes = 0;

	while (processedBytes != totalNumOfBytes)
	{
		if (totalNumOfBytes - processedBytes >= processBufferSizeInBytes)
		{
			processBufferSizeInBytes = processBufferSizeInBytes;
		}
		else
		{
			processBufferSizeInBytes = totalNumOfBytes - processedBytes;
		}
		processBufferSizeInShort = processBufferSizeInBytes / sizeof(short);
		processInput(pWaves, pOut, processBufferSizeInBytes);
		pWaves += processBufferSizeInShort;
		pOut += processBufferSizeInShort;
		processedBytes += processBufferSizeInBytes;
	}
	WriteWaveFileS(outWAVFile, samplePerSec, numOfChannel, bitPerSample, nOfSample, outStart);
	return AMF_OK;
}

void ReverbProcessor::playerStop()
{
	m_bIsPlaying = false;
	if (m_threadProcessing != nullptr)
	{
		m_threadProcessing->join();
		delete m_threadProcessing;
		m_threadProcessing = nullptr;
	}

}

int ReverbProcessor::addFilterFD(float** filter, AMF_RESULT* AMFErr)
{
	for (int i = 0; i < m_vFDFilterList.size(); i++)
	{
		if (m_vFDFilterList[i] == nullptr)
		{
			m_vFDFilterList[i] = filter;
			*AMFErr = AMF_OK;
			return i;
		}
	}
	m_vFDFilterList.push_back(filter);
	*AMFErr = AMF_OK;
	return m_vFDFilterList.size() - 1;
}

AMF_RESULT ReverbProcessor::recorderInit(size_t SamplesPerSec)
{
	//STREAMINFO          streaminfo;
	//memset(&streaminfo, 0, sizeof(STREAMINFO));
	//streaminfo.bitsPerSample = 16;
	//streaminfo.NumOfChannels = 2;
	//streaminfo.SamplesPerSec = SamplesPerSec;// 48000;
	//UINT bufferSize, frameSize;

	m_WASAPIRecorder.reset(
#if defined(__MACOSX) || defined(__APPLE__)
        static_cast<IWavPlayer *>(new PortPlayer())
#else

  #ifdef ENABLE_PORTAUDIO
        static_cast<IWavPlayer *>(new PortPlayer())
  #else
            
    #ifdef _WIN32
	    static_cast<IWavPlayer *>(new WASAPIPlayer())
    #elif !defined(__MACOSX) && !defined(__APPLE__)
        static_cast<IWavPlayer *>(new AlsaPlayer())
	#else
	    nullptr //no unsopported players
	#endif

  #endif

#endif
		);
	
	//STD_RETURN_IF_NOT_ZERO(m_WASAPIRecorder.wasapiInit(&streaminfo, &m_WASAPIRecorder.bufferSize, &m_WASAPIRecorder.frameSize, AUDCLNT_SHAREMODE_SHARED, true), "Failed to initialize recorder", AMF_FAIL);
	if(PlayerError::OK != m_WASAPIRecorder->Init(2, 16, SamplesPerSec, false, true))
	{
		return AMF_FAIL;
	}

	return AMF_OK;
}

AMF_RESULT ReverbProcessor::recorderStart(char* outputWAVName)
{
	if (!m_bIsRecording)
	{
		m_bIsRecording = true;
		m_cpRecordWAVFileName = outputWAVName;
		resetDiskBuffer();
		if (m_pDiskBuffer == nullptr){
			STD_RETURN_IF_NOT_ZERO(fopen_s(&m_pDiskBuffer, "tmp.bin", "wb+"), "Failed to open temporary file on disk", AMF_FAIL);
		}
		if (m_threadRecord)
		{
			delete m_threadRecord;
			m_threadRecord = nullptr;
		}

		if (!m_WASAPIRecorder)
		{
			STD_RETURN_IF_NOT_ZERO(recorderInit(48000), "Failed to intialize record device", AMF_FAIL);
		}

		m_threadRecord = new std::thread(&ReverbProcessor::recorderStartInternel, this);
	}
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::recorderStop()
{
	if (m_bIsRecording && m_threadRecord != nullptr)
	{
		m_bIsRecording = false;
		m_threadRecord->join();
		size_t outputBufferSizeInSample = m_iNumOfValidBytesInDiskBuffer / 2 / sizeof(short);
		size_t outputBufferSizeInBytes = m_iNumOfValidBytesInDiskBuffer;
		size_t sampleSizeInBits = sizeof(short) * 8;
		short* outputBuffer = new short[outputBufferSizeInSample * 2];

		//RtlSecureZeroMemory(outputBuffer, outputBufferSizeInBytes);
		std::memset(outputBuffer, 0, outputBufferSizeInBytes);
		
		fseek(m_pDiskBuffer, 0, SEEK_SET);
		STD_RETURN_IF_FALSE(fread(outputBuffer, 1, outputBufferSizeInBytes, m_pDiskBuffer) == outputBufferSizeInBytes,
			"Failed to retrived buffer from disk", AMF_FAIL);
		STD_RETURN_IF_FALSE(WriteWaveFileS(m_cpRecordWAVFileName, 48000, 2, sampleSizeInBits,outputBufferSizeInSample, outputBuffer),
			"Failed to write to wav", AMF_FAIL);
		IF_NOT_NULL_DELETE(m_threadRecord);

		resetDiskBuffer();
	}
	return AMF_OK;
}

int ReverbProcessor::addFilterTDFromWAV(char* FilePath, AMF_RESULT* AMFErr)
{
	// Load the WAV File;
	long bytesPerChannel = 0;
	unsigned char* output_char = nullptr;
	float** output_float_temp = nullptr;

	uint32_t samplePerSec = 0;
	uint16_t bitPerSample = 0;
	uint16_t channelcount = 0;
	uint32_t sampleCount = 0;

	if(ReadWaveFile(FilePath, samplePerSec, bitPerSample, channelcount, sampleCount, &output_char, &output_float_temp))
	{
		float** bufferTD = new float*[channelcount];
		float** bufferFD = new float*[channelcount];
		for (size_t i = 0; i < channelcount; i++)
		{
			bufferFD[i] = new float[m_iFilterLengthInFloat];
			bufferTD[i] = new float[m_iFilterLengthInFloat];
			//RtlZeroMemory(bufferFD[i], sizeof(float)*m_iFilterLengthInFloat);
			std::memset(bufferFD[i], 0, sizeof(float)*m_iFilterLengthInFloat);
			//RtlZeroMemory(bufferTD[i], sizeof(float)*m_iFilterLengthInFloat);
			std::memset(bufferTD[i], 0, sizeof(float)*m_iFilterLengthInFloat);
		}
		if(bitPerSample == 16)
		{
			short* output_short = (short*)output_char;

			int a = std::min(3, 4);

			for (int chan = 0; chan < channelcount; chan++) 
			{
				AMF_RETURN_IF_FAILED(m_pTANConverter->Convert(
					output_short + chan, 
					channelcount, 
					(std::min)((uint32_t)m_iFilterLengthInFloat / 2, sampleCount), 
					bufferTD[chan], 2, 1.f), 
					"Failed to execute tan convert\n"
					);
			}
		}
		else if(bitPerSample == 32)
		{
			float* output_float = (float*)output_char;
			size_t target_size = (std::min)(m_iFilterLengthInFloat/2, (size_t)channelcount*sampleCount);
			for (size_t i = 0; i < target_size; i++)
			{
				bufferTD[i%channelcount][i / channelcount * 2] = output_float[i];
			}
		}
		else
		{
			return -1;
		}

		AMF_RETURN_IF_FAILED(m_pTANFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, m_iFilterLengthInComplexLog2, channelcount, bufferTD, bufferFD), "tan FFT Transform Failed\n");

		if(m_iNumOfChannels != channelcount)
		{
			// need to remap the buffer to support
			float** outputFD = new float*[m_iNumOfChannels];
			for (size_t i = 0; i < m_iNumOfChannels; i++)
			{
				outputFD[i] = new float[m_iFilterLengthInFloat];

#ifdef _WIN32
				RtlZeroMemory(outputFD[i], sizeof(float)*m_iFilterLengthInFloat);
#else
				std::memset(outputFD[i], 0, sizeof(float)*m_iFilterLengthInFloat);
#endif
				
				outputFD[i][0] = 1.0f;
			}
			if(channelcount == 1)
			{
				for (size_t i = 0; i < m_iNumOfChannels; i++)
				{
					memcpy(outputFD[i], bufferFD[0], sizeof(float)*m_iFilterLengthInFloat);
				}
			}
			else
			{
				size_t min_channel = (std::min)(m_iNumOfChannels,(size_t)channelcount);
				for (size_t i = 0; i < min_channel; i++)
				{
					memcpy(outputFD[i], bufferFD[i], sizeof(float)*m_iFilterLengthInFloat);
				}
			}
			for (size_t i = 0; i < channelcount; i++)
			{
				delete[] bufferTD[i];
				delete[] bufferFD[i];
				delete[] output_float_temp[i];
			}
			delete[]output_char;
			delete[]output_float_temp;
			delete[]bufferTD;
			delete[]bufferFD;
			return addFilterFD(outputFD, AMFErr);
		}
		else
		{
			// Channel already mapped, not need to remap, pass right through
			for (size_t i = 0; i < channelcount; i++)
			{
				delete[] bufferTD[i];
				delete[] output_float_temp[i];
			}
			delete[]output_char;
			delete[]output_float_temp;
			delete[]bufferTD;
			return addFilterFD(bufferFD, AMFErr);
		}
	}
	return -1;
}

AMF_RESULT ReverbProcessor::deleteFilter(int filterIndex)
{
	if (filterIndex < m_vFDFilterList.size())
	{
		m_vFDFilterList.erase(m_vFDFilterList.begin()+filterIndex);
		return AMF_OK;
	}
	return AMF_FAIL;
}

bool ReverbProcessor::isPlayerPlaying()
{
	return m_bIsPlaying;
}

bool ReverbProcessor::isRecorderRunning()
{
	return m_bIsRecording;
}

bool ReverbProcessor::isFilterValid(int index)
{
	if (index < m_vFDFilterList.size())
	{
		if (m_vFDFilterList[index] != nullptr)
			return true;
	}
	return false;
}

AMF_RESULT ReverbProcessor::updateFilterFD(int position, float** filter)
{
	if (position < m_vFDFilterList.size())
	{
		deleteFilter(position);
		m_vFDFilterList[position] = filter;
		return AMF_OK;
	}
	return AMF_FAIL;
}

AMF_RESULT ReverbProcessor::writeToWAV(void* in_pBuffer, int in_ChannelCount, int in_iSampleRate, int in_iBitsPerSample, int in_iNumOfSamples, const char* FilePath)
{
	if(in_pBuffer!=nullptr)
	{
		if(in_iBitsPerSample == 32)
			WriteWaveFileF(FilePath, in_iSampleRate, in_ChannelCount, in_iBitsPerSample, in_iNumOfSamples, (float**)in_pBuffer);
		if(in_iBitsPerSample == 16)
			WriteWaveFileS(FilePath, in_iSampleRate, in_ChannelCount, in_iBitsPerSample, in_iNumOfSamples, (short*)in_pBuffer);
		return AMF_OK;
	}
	return AMF_FAIL;
}

AMF_RESULT ReverbProcessor::writeFilterToWAV(size_t in_iFilterIndex, int in_ChannelCount, int in_iSampleRate, int in_iBitsPerSample, int in_iNumOfSamples,char* FilePath)
{
	if(in_iFilterIndex < m_vFDFilterList.size())
	{
		WriteWaveFileF(FilePath, in_iSampleRate, in_ChannelCount, in_iBitsPerSample, in_iNumOfSamples, m_vFDFilterList[in_iFilterIndex]);
		return AMF_OK;
	}
	return AMF_FAIL;
}

AMF_RESULT ReverbProcessor::processFilter()
{
	AMF_RESULT AMFErr;
	for (int i = 0; i < m_iNumOfChannels; i++)
	{
		//RtlSecureZeroMemory(m_pInternalProcessedFilterFDBuffer[i],m_iFilterLengthInFloat * sizeof(float));
		std::memset(m_pInternalProcessedFilterFDBuffer[i], 0, m_iFilterLengthInFloat * sizeof(float));
		//RtlSecureZeroMemory(m_pInternalProcessedFilterTDBuffer[i], m_iFilterLengthInFloat * sizeof(float));
		std::memset(m_pInternalProcessedFilterTDBuffer[i], 0, m_iFilterLengthInFloat * sizeof(float));
	}

	if (m_vFDFilterList.size() == 1)
	{
		for (int i = 0; i < m_iNumOfChannels; i++)
		{
			memcpy(m_pInternalProcessedFilterFDBuffer[i], m_vFDFilterList[0][i], m_iFilterLengthInFloat * sizeof(float));
		}
	}
	else
	{
		int dumbfilterIndex = addFilterFD(m_pALLPassBuffer, &AMFErr);
		for (int i = 1; i < m_vFDFilterList.size() - 1; i++)
		{
			if (m_vFDFilterList[i] != nullptr)
			{
				AMF_RETURN_IF_FAILED(m_pTANMath->ComplexMultiplication(m_vFDFilterList[i], m_vFDFilterList[dumbfilterIndex], m_pInternalProcessedFilterFDBuffer, m_iNumOfChannels, m_iFilterLengthInComplex),
					"ComplexMultiplication() Failed");
				for (int j = 0; j < m_iNumOfChannels; j++)
				{
					memcpy(m_vFDFilterList[dumbfilterIndex][j], m_pInternalProcessedFilterFDBuffer[j], m_iFilterLengthInFloat * sizeof(float));
				}
			}
		}
		for (int i = 0; i < m_iNumOfChannels; i++)
		{
			memcpy(m_pInternalProcessedFilterFDBuffer[i], m_vFDFilterList[dumbfilterIndex][i], m_iFilterLengthInFloat * sizeof(float));
		}
		m_vFDFilterList.pop_back();
		AMF_RETURN_IF_FAILED(fillAllPassFilterFD(m_pALLPassBuffer, m_iFilterLengthInFloat, m_iNumOfChannels), "Failed to reset all pass filter");
	}
	for (int i = 0; i < 64; i++)
	{
		printf("%f,", m_pInternalProcessedFilterFDBuffer[0][i]);
	}
	// Transform filter to TD and wait for update
	AMF_RETURN_IF_FAILED(m_pTANFFT->Transform(
		amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, m_iFilterLengthInComplexLog2, m_iNumOfChannels, m_pInternalProcessedFilterFDBuffer, m_pInternalProcessedFilterTDBuffer),
		"Transform() Failed");
	//Deinterleaved the buffer
	for (int i = 0; i < m_iNumOfChannels; i++)
	{
		for (int j = 0; j < m_iFilterLengthInFloat / 2; j++)
		{
			m_pInternalProcessedFilterTDBuffer[i][j] = m_pInternalProcessedFilterTDBuffer[i][j << 1];
		}
	}
//	// Update the filter to the convolution engine
	AMF_RETURN_IF_FAILED(m_pTANConvolution->UpdateResponseTD(m_pInternalProcessedFilterTDBuffer, m_iFilterLengthInFloat / 2, NULL, 0), "UpdateresponseTD() Failed");
	return AMF_OK;
}

int ReverbProcessor::getDeviceNames(char*** _deviceName, int count)
{
	return listGpuDeviceNamesWrapper(*_deviceName, count);
}

int ReverbProcessor::playerPlayInternal()
{
	int bytesPlayed;
	short *pOut;
	short *pWaves;
	short *pWaveStarts;

	pWaveStarts= pWaves= (short *)(m_pInputRawBuffer);
	int totalNumOfBytes = m_iInputSizeInBytesPerChannel* m_iNumOfChannels; // stereo short samples

	pOut = new short[FILTER_SAMPLE_RATE];
	memset(pOut, 0, FILTER_SAMPLE_RATE * sizeof(short));

	#ifdef _WIN32

	// upgrade our windows process and thread priorities:
	SetSecurityInfo(GetCurrentProcess(), SE_WINDOW_OBJECT, PROCESS_SET_INFORMATION, 0, 0, 0, 0);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	#endif

	size_t chunkSizeInBytes = m_iConvolutionLengthInSample * m_iNumOfChannels * sizeof(short);

	while (m_bIsPlaying) {
		processInput(pWaves, pOut, chunkSizeInBytes);
		std::this_thread::sleep_for(std::chrono::milliseconds(0));
		int bytes2Play = chunkSizeInBytes;
		unsigned char *pData;
		pData = (unsigned char *)pOut;

		while(bytes2Play > 0) 
		{
			//bytesPlayed = m_WASAPIPlayer.Play(pData, bytes2Play, false);
			bytesPlayed = m_WASAPIPlayer->Play(pData, bytes2Play, false);

			bytes2Play -= bytesPlayed;
			pData += bytesPlayed;
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}

		bytesPlayed = chunkSizeInBytes;
		pWaves += bytesPlayed / m_iNumOfChannels;
		if (pWaves - pWaveStarts + chunkSizeInBytes / sizeof(short) > totalNumOfBytes){
			pWaves = pWaveStarts;
		}
	}
	delete[]pOut;

	m_WASAPIPlayer->Close();

	return 0;
}

AMF_RESULT ReverbProcessor::recorderStartInternel()
{
	size_t tempBufferSize = 48000;
	size_t recordedBytes = 0;
	unsigned char* tempBuffer = new unsigned char[tempBufferSize];

	//RtlSecureZeroMemory(tempBuffer, tempBufferSize);
	std::memset(tempBuffer, 0, tempBufferSize);
	
	while (m_bIsRecording)
	{
		//recordedBytes = m_WASAPIRecorder.Record(tempBuffer, tempBufferSize);
		recordedBytes = m_WASAPIRecorder->Record(tempBuffer, tempBufferSize);

		STD_RETURN_IF_FALSE(fwrite(tempBuffer, 1, recordedBytes, m_pDiskBuffer) == recordedBytes, "Failed to write to disk", AMF_FAIL);
		m_iNumOfValidBytesInDiskBuffer += recordedBytes;
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	return AMF_OK;
}

int ReverbProcessor::loadWAVFile(char* FilePath)
{
	if (m_pInputRawBuffer != nullptr)
		delete m_pInputRawBuffer;

	uint16_t BitsPerSample, NChannels;
	uint32_t SamplesPerSec, NSamples;
	unsigned char *pSsamples;
	float **Samples;

	if(!ReadWaveFile(FilePath, SamplesPerSec, BitsPerSample, NChannels, NSamples, &pSsamples, &Samples))
	{
		return -1;
	}
    
	//WavError queueErrors = m_WASAPIPlayer.ReadWaveFile(FilePath, &m_iInputSizeInBytesPerChannel, &m_pInputRawBuffer);
	m_iInputSizeInBytesPerChannel = (BitsPerSample / 8) * NSamples;
	m_iInputSizeInFloatPerChannel = m_iInputSizeInBytesPerChannel / sizeof(float);
	
	return 0;
}

unsigned ReverbProcessor::getNextPowOf2_32Bit(unsigned int a)
{
	a--;
	a |= a >> 1;
	a |= a >> 2;
	a |= a >> 4;
	a |= a >> 8;
	a |= a >> 16;
	a++;
	return a;
}

float ReverbProcessor::db2Mag(float db)
{
	return std::pow(10.0f, db / 10.0f);
}

AMF_RESULT ReverbProcessor::processInput(short* in, short* out, size_t bufferSizeInBytes)
{
	size_t samplePerchannel = bufferSizeInBytes / (sizeof(short) * m_iNumOfChannels); // Stereo data
	if (samplePerchannel < m_iConvolutionLengthInSample)
	{
		// If the sample Per channel is less than the convolution size, we need to zero the convolution buffer
		zeroInternelInOutBuffer();
	}
	for (int chan = 0; chan < m_iNumOfChannels; chan++){
		// de-interleaved data and convert short to float
		m_pTANConverter->Convert(in + chan, m_iNumOfChannels, samplePerchannel, m_pfConvolutionInputBufferFloat[chan], 1, 1.f);
	}
	AMF_RETURN_IF_FAILED(m_pTANConvolution->Process(m_pfConvolutionInputBufferFloat, m_pfConvolutionOutputBuffer, m_iConvolutionLengthInSample, nullptr, nullptr), "Failed to Process Convolution()\n");
	for (int chan = 0; chan < m_iNumOfChannels; chan++)
	{
		m_pTANConverter->Convert(m_pfConvolutionOutputBuffer[chan], 1, samplePerchannel, out+chan, m_iNumOfChannels, 1.f);
	}
	return AMF_OK;
}

void ReverbProcessor::adjustInternalFilterBuffer(size_t sizeInComplex, size_t numOfChannels)
{
	size_t requireSizeInFloat = sizeInComplex * 2;
	if (m_iNumOfFilterBufferChannels < numOfChannels || m_iFilterLengthInFloat < requireSizeInFloat)
	{
		deallocateBuffer(m_pInternalProcessedFilterTDBuffer, m_iNumOfFilterBufferChannels);
		deallocateBuffer(m_pInternalProcessedFilterFDBuffer, m_iNumOfFilterBufferChannels);
		deallocateBuffer(m_pALLPassBuffer, m_iNumOfFilterBufferChannels);


		m_pInternalProcessedFilterFDBuffer = new float*[numOfChannels];
		m_pInternalProcessedFilterTDBuffer = new float*[numOfChannels];
		for (int channelID = 0; channelID < numOfChannels; channelID++)
		{
			m_pInternalProcessedFilterFDBuffer[channelID] = new float[requireSizeInFloat];
			m_pInternalProcessedFilterTDBuffer[channelID] = new float[requireSizeInFloat];
			
			//RtlSecureZeroMemory(m_pInternalProcessedFilterFDBuffer[channelID], requireSizeInFloat*sizeof(float));
			std::memset(m_pInternalProcessedFilterFDBuffer[channelID], 0, requireSizeInFloat*sizeof(float));

			//RtlSecureZeroMemory(m_pInternalProcessedFilterTDBuffer[channelID], requireSizeInFloat*sizeof(float));
			std::memset(m_pInternalProcessedFilterTDBuffer[channelID], 0, requireSizeInFloat*sizeof(float));
		}

		generateAllPassFilterFD(&m_pALLPassBuffer, requireSizeInFloat, numOfChannels);
	}

	m_iNumOfFilterBufferChannels = numOfChannels;
	m_iFilterLengthInFloat = requireSizeInFloat;
	m_iFilterLengthInComplexLog2 = 0;
	m_iFilterLengthInComplex = sizeInComplex;
	
	size_t temp = 1;
	while (temp < sizeInComplex)
	{
		m_iFilterLengthInComplexLog2 += 1;
		temp <<= 1;
	}

}

void ReverbProcessor::adjustInternalInputBuffer(size_t sizeInFloat, size_t numOfChannels)
{
	if (sizeInFloat < m_iConvolutionLengthInSample)
		sizeInFloat = m_iConvolutionLengthInSample;
	if (m_iNumOfConvBufferChannels < numOfChannels || m_pInternalInOutBufferSizeInfloat < sizeInFloat)
	{
		deallocateBuffer(m_pfConvolutionInputBufferFloat, m_iNumOfConvBufferChannels);
		deallocateBuffer(m_pfConvolutionOutputBuffer, m_iNumOfConvBufferChannels);
		m_pInternalInOutBufferSizeInfloat = sizeInFloat;

		m_pfConvolutionInputBufferFloat = new float*[numOfChannels];
		m_pfConvolutionOutputBuffer = new float*[numOfChannels];

		for (int channelID = 0; channelID < numOfChannels; channelID++)
		{
			m_pfConvolutionInputBufferFloat[channelID] = new float[sizeInFloat];
			m_pfConvolutionOutputBuffer[channelID] = new float[sizeInFloat];
			
			//RtlSecureZeroMemory(m_pfConvolutionInputBufferFloat[channelID], sizeInFloat * sizeof(float));
			std::memset(m_pfConvolutionInputBufferFloat[channelID], 0, sizeInFloat * sizeof(float));

			//RtlSecureZeroMemory(m_pfConvolutionOutputBuffer[channelID], sizeInFloat * sizeof(float));
			std::memset(m_pfConvolutionOutputBuffer[channelID], 0, sizeInFloat * sizeof(float));
		}
		m_iNumOfConvBufferChannels = numOfChannels;
	}
}

void ReverbProcessor::zeroInternelInOutBuffer()
{
	for (size_t i = 0; i < m_iNumOfChannels; i++)
	{
		//RtlSecureZeroMemory(m_pfConvolutionInputBufferFloat[i], m_pInternalInOutBufferSizeInfloat * sizeof(float));
		std::memset(m_pfConvolutionInputBufferFloat[i], 0, m_pInternalInOutBufferSizeInfloat * sizeof(float));
		
		//RtlSecureZeroMemory(m_pfConvolutionOutputBuffer[i], m_pInternalInOutBufferSizeInfloat * sizeof(float));
		std::memset(m_pfConvolutionOutputBuffer[i], 0, m_pInternalInOutBufferSizeInfloat * sizeof(float));
	}
}

void ReverbProcessor::deallocateInternalBuffer()
{
	for (int i = 0; i < m_iNumOfChannels; i++)
	{
		delete m_pInternalProcessedFilterTDBuffer[i];
		delete m_pInternalProcessedFilterFDBuffer[i];
		delete m_pALLPassBuffer[i];
		delete m_pfConvolutionInputBufferFloat[i];
		delete m_pfConvolutionOutputBuffer[i];
	}
	delete[]m_pfConvolutionInputBufferFloat;
	delete[]m_pfConvolutionOutputBuffer;
	delete[]m_pInternalProcessedFilterFDBuffer;
	delete[]m_pInternalProcessedFilterTDBuffer;
	delete[]m_pALLPassBuffer;
}

void ReverbProcessor::deallocateBuffer(float** buffer, size_t size)
{
	if (buffer != nullptr)
	{
		for (size_t i = 0; i < size; i++)
		{
			delete buffer[i];
		}
		delete[]buffer;
	}
}

AMF_RESULT ReverbProcessor::resetDiskBuffer()
{
	m_iNumOfValidBytesInDiskBuffer = 0;
	if (m_pDiskBuffer != nullptr)
	{
		STD_RETURN_IF_NOT_ZERO(fseek(m_pDiskBuffer, 0, SEEK_SET), "Failed to reset file the pointer", AMF_FAIL);
	}
}

AMF_RESULT ReverbProcessor::generateAllPassFilterFD(float*** output, size_t sizeInFloat, size_t numOfChannel)
{
	if (output == NULL) return AMF_FAIL;
	if (sizeInFloat == 0 || numOfChannel == 0) return AMF_FAIL;
	float** filter = new float*[numOfChannel];
	for (int i =0; i < numOfChannel; i++)
	{
		filter[i] = new float[sizeInFloat];
		for (int j = 0; j < sizeInFloat ; j++)
		{
			(j & 1) == 0 ? filter[i][j] = 1.0f : filter[i][j] = 0.0f;
		}
	}
	*output = filter;
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::fillAllPassFilterFD(float** input, size_t sizeInFloatPerChannel, size_t numOfChannel)
{
	if (input == NULL) return AMF_FAIL;
	if (sizeInFloatPerChannel == 0 || numOfChannel == 0) return AMF_FAIL;
	for (int i = 0; i < numOfChannel; i++)
	{
		for (int j = 0; j < sizeInFloatPerChannel; j++)
		{
			(j & 1) == 0 ? input[i][j] = 1.0f : input[i][j] = 0.0f;
		}
	}
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::fill10BandEQFilterFD(float in[10], int sampleRate, float** output, size_t sizeInComplexLog2, size_t numOfChannel)
{
	printf("decibels: ");
	for (int i = 0; i < 10; i++)
	{
		printf("%f,", in[i]);
	}
	printf("\n");
	if (in == NULL) return AMF_FAIL;
	if (sampleRate == 0 || numOfChannel == 0) return AMF_FAIL;
	size_t sizeInfloats = 1 << (sizeInComplexLog2 + 1);
	float* FilterTD = new float[sizeInfloats/2];
	memset(FilterTD, 0, sizeInfloats*sizeof(float)/2);
	AMF_RETURN_IF_FAILED(m_pTANFilter->generate10BandEQ(sizeInComplexLog2-1, sampleRate, FilterTD, in), "generate10BandEQ() Failed");

	float** FilterTDC = new float*[numOfChannel];
	for (int i = 0; i < numOfChannel; i++)
	{
		FilterTDC[i] = new float[sizeInfloats];
		memset(FilterTDC[i], 0, sizeInfloats*sizeof(float));
		for (int k = 0; k < sizeInfloats / 2; k++)
		{
			FilterTDC[i][k << 1] = FilterTD[k];
		}
	}
	AMF_RETURN_IF_FAILED(m_pTANFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, sizeInComplexLog2, numOfChannel, FilterTDC, output), "TAN FFT() Failed");
	delete[]FilterTD;
	for (int i =0; i < numOfChannel; i++)
	{
		delete[]FilterTDC[i];
	}
	delete[]FilterTDC;
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::generate10BandEQFilterFD(float in[10], int sampleRate, float*** output, size_t sizeInComplexLog2, size_t numOfChannel)
{
	printf("decibels: ");
	for (int i = 0; i < 10; i++)
	{
		printf("%f,",in[i]);
	}
	printf("\n");
	if (in == NULL) return AMF_FAIL;
	if (sampleRate == 0 || numOfChannel == 0) return AMF_FAIL;
	size_t sizeInfloats = 1 << (sizeInComplexLog2 + 1);
	float* FilterTD = new float[sizeInfloats];
	memset(FilterTD, 0, sizeInfloats*sizeof(float));
	AMF_RETURN_IF_FAILED(m_pTANFilter->generate10BandEQ(sizeInComplexLog2, sampleRate, FilterTD, in), "generate10BandEQ() Failed");

	float** FilterTDC = new float*[numOfChannel];
	float** FilterFD = new float*[numOfChannel];
	for (int i = 0; i < numOfChannel; i++)
	{
		FilterTDC[i] = new float[sizeInfloats];
		FilterFD[i] = new float[sizeInfloats];
        memset(FilterTDC[i], 0, sizeInfloats*sizeof(float));
        memset(FilterFD[i], 0, sizeInfloats*sizeof(float));
		for (int k = 0; k < sizeInfloats / 2; k++)
		{
			FilterTDC[i][k << 1] = FilterTD[k];
		}
	}
	AMF_RETURN_IF_FAILED(m_pTANFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD,sizeInComplexLog2,numOfChannel, FilterTDC, FilterFD), "TAN FFT() Failed");
	delete[]FilterTD;
	*output = FilterFD;
	for (int i = 0; i < numOfChannel; i++)
	{
		delete[]FilterTDC[i];
	}
	delete[]FilterTDC;
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::generate10BandEQFilterTD(float in[10], int sampleRate, float*** output, size_t sizeInLog2, size_t numOfChannel)
{
	if (in == NULL) return AMF_FAIL;
	if (sampleRate == 0 || numOfChannel == 0) return AMF_FAIL;
	size_t sizeInfloats = 1 << (sizeInLog2);
	float* FilterTD = new float[sizeInfloats];
	memset(FilterTD, 0, sizeof(float)*sizeInfloats);
	AMF_RETURN_IF_FAILED(m_pTANFilter->generate10BandEQ(sizeInLog2, sampleRate, FilterTD, in), "generate10BandEQ() Failed");
	// Scale down the response
	float** FilterTDC = new float*[numOfChannel];
	for (int i = 0; i < numOfChannel; i++)
	{
		FilterTDC[i] = new float[sizeInfloats];
		
		//RtlSecureZeroMemory(FilterTDC[i], sizeInfloats * sizeof(float));
		std::memset(FilterTDC[i], 0, sizeInfloats * sizeof(float));

		memcpy(FilterTDC[i], FilterTD, sizeInfloats * sizeof(float));
	}
	delete[]FilterTD;
	*output = FilterTDC;
	return AMF_OK;
}

AMF_RESULT ReverbProcessor::getWAVFileInfo(const char* FilePath, uint32_t & samplesPerSec, uint16_t & bitsPerSample, uint16_t & nChannels, uint32_t & nSamples)
{
	if (FilePath == nullptr)
	{
		return AMF_FAIL;
	}

	unsigned char* file;
	float** pfile;

	ReadWaveFile(FilePath, samplesPerSec, bitsPerSample, nChannels, nSamples, &file, &pfile);

	for (int i = 0; i < nChannels; i++)
	{
		delete pfile[i];
	}

	delete[]pfile;
	delete[]file;

	return AMF_OK;
}

//unsigned ReverbProcessor::processThreadStub(void* ptr)
//{
//	ReverbProcessor *p = static_cast<ReverbProcessor*> (ptr);
//	return p->playerPlayInternal();
//}

void ReverbProcessor::TANteardown()
{
}
