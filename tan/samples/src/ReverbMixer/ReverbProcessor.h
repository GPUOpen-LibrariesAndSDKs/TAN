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

#include "tanlibrary/include/TrueAudioNext.h"
#include "IWavPlayer.h"

#include <CL/cl.h>


#include <vector>
#include <thread>

#define SAVE_DELETE_BUFFER(buffer, NumOfChannel){\
	if(buffer!=nullptr){\
		for (size_t i = 0; i < NumOfChannel; i++)\
		{\
			if (buffer != nullptr)\
				delete[]buffer[i];\
		}\
		delete[]buffer;\
	}\
}

enum eOperationMode
{
	eUnset,
	eCPU,
	eGPU,
};


class ReverbProcessor
{
public:
	ReverbProcessor(){};
	AMF_RESULT init(
		eOperationMode mode = eCPU,
		int deviceIDx = 0,
		size_t fftLengthInSamples = 2048,
		size_t bufferLengthInSamples = 2048,
		size_t numOfChannel = 2
	);
	~ReverbProcessor()
	{
		m_bIsPlaying = false;
		if (m_threadProcessing!=nullptr)
			m_threadProcessing->join();
		SAVE_DELETE_BUFFER(m_pALLPassBuffer, m_iNumOfChannels);
		SAVE_DELETE_BUFFER(m_pInternalProcessedFilterTDBuffer, m_iNumOfChannels);
		SAVE_DELETE_BUFFER(m_pInternalProcessedFilterFDBuffer, m_iNumOfChannels);
		SAVE_DELETE_BUFFER(m_pfConvolutionInputBufferFloat, m_iNumOfChannels);
		SAVE_DELETE_BUFFER(m_pfConvolutionOutputBuffer, m_iNumOfChannels);		
		
		if(m_WASAPIPlayer)
		{
			m_WASAPIPlayer->Close();
		}
	};
	/**
	 * \brief Play input file with filter applied
	 * \param wavFileName Path to the wav file
	 * \return AMF_OK if no errr occurs
	 */
	AMF_RESULT playerPlay(char* wavFileName);
	/**
	 * \brief write the processed output to a WAV file (All of the specs of the output will be inhereited from input)
	 * \param inWAVFile Input WAV File Path
	 * \param outWAVFile Output WAV File Path
	 * \return AMF_OK if success
	 */
	AMF_RESULT playerProcessToWAV(char* inWAVFile, char* outWAVFile);
	/**
	 * \brief Stop the current player;
	 */
	void playerStop();
	/**
	 * \brief add a frequncy domain filter into the processor
	 * \param filter float array for filter, must equal to numOfChannel(declared in init()) * fftlengthInSamples
	 * \param AMFErr error code, return AMF_OK if everything is fine
	 * \return index in the filter list
	 */
	int addFilterFD(float** filter, AMF_RESULT* AMFErr);
	/**
	 * \brief Intialize the recorder, default to 16 bit / 2 Channel
	 * \param SamplePerSec Sampling rate of the recorder
	 * \return AMF_OK if success
	 */
	AMF_RESULT recorderInit(size_t SamplePerSec);
	/**
	 * \brief Start recording and save the output
	 * \param outputWAVName output file path
	 * \return AMF_OK if success
	 */
	AMF_RESULT recorderStart(char* outputWAVName);
	/**
	 * \brief Stop Recording
	 * \return AMF_OK if success
	 */
	AMF_RESULT recorderStop();
	/**
	 * \brief add a time domain filter into the processor from WAV file
	 * \param FilePath path to the wav file
	 * \param AMFErr 
	 * \return AMF_OK if no error occurs
	 */
	int addFilterTDFromWAV(char* FilePath, AMF_RESULT* AMFErr);
	/**
	 * \brief Update a frequency domain filter with filter position
	 * \param position : index of the filter
	 * \param filter : frequency domain data of the fitler
	 * \return 
	 */
	AMF_RESULT updateFilterFD(int position, float** filter);
	AMF_RESULT writeToWAV(void* in_pBuffer, int in_ChannelCount, int in_iSampleRate, int in_iBitsPerSample, int in_iNumOfSamples, const char* FilePath);
	AMF_RESULT writeFilterToWAV(size_t in_iFilterIndex, int in_ChannelCount, int in_iSampleRate, int in_iBitsPerSample, int in_iNumOfSamples, char* FilePath);
	/**
	 * \brief process the filters in the filter chains and update the combined filter response
	 * \return 
	 */
	AMF_RESULT processFilter();
	AMF_RESULT deleteFilter(int filterIndex);
	bool	isPlayerPlaying();
	bool	isRecorderRunning();
	bool	isFilterValid(int index);
	int						getDeviceNames(char*** _deviceName, int count);
	AMF_RESULT				generateAllPassFilterFD(float*** output, size_t sizeInFloat, size_t numOfChannel);
	AMF_RESULT				generate10BandEQFilterFD(float in[10], int sampleRate, float*** output, size_t sizeInComplexLog2, size_t numOfChannel);
	AMF_RESULT				generate10BandEQFilterTD(float in[10], int sampleRate, float*** output, size_t sizeInLog2, size_t numOfChannel);
	AMF_RESULT				fillAllPassFilterFD(float** input, size_t sizeInFloatPerChannel, size_t numOfChannel);
	AMF_RESULT				fill10BandEQFilterFD(float in[10], int sampleRate, float** output, size_t sizeInComplexLog2, size_t numOfChannel);
	AMF_RESULT				getWAVFileInfo(const char* FilePath, uint32_t & samplesPerSec, uint16_t & bitsPerSample, uint16_t & nChannels, uint32_t & nSamples);
private:
	int						playerPlayInternal();
	AMF_RESULT				recorderStartInternel();
	int						loadWAVFile(char* FilePath);
	unsigned int			getNextPowOf2_32Bit(unsigned int a);
	float					db2Mag(float db);
	AMF_RESULT				processInput(short* in, short* out, size_t size);
	void					adjustInternalFilterBuffer(size_t sizeInComplex, size_t numOfChannels);
	void					adjustInternalInputBuffer(size_t sizeInFloat, size_t numOfChannels);
	void					zeroInternelInOutBuffer();
	void					deallocateInternalBuffer();
	void					deallocateBuffer(float** buffer, size_t size);
	AMF_RESULT				resetDiskBuffer();
	// Operating parameters
	eOperationMode				m_eOperationMode = eUnset;
	int							m_iDeviceIDx = 0;
	bool						m_bIsPlaying = false;
	bool						m_bIsRecording = false;
	
	// Filters
	float**						m_pInternalProcessedFilterFDBuffer = nullptr;
	float**						m_pInternalProcessedFilterTDBuffer = nullptr;
	float**						m_pALLPassBuffer = nullptr;
	size_t						m_iFilterLengthInFloat = 0;
	size_t						m_iFilterLengthInComplexLog2 = 0;
	size_t						m_iFilterLengthInComplex = 0;
	size_t						m_iNumOfFilterBufferChannels = 0;
	
	std::vector<float**>		m_vFDFilterList;
	// Inputs
	unsigned char*				m_pInputRawBuffer = nullptr;
	float**						m_pfConvolutionInputBufferFloat = nullptr;		// Handle interleaved data
	float**						m_pfConvolutionOutputBuffer = nullptr;
	size_t						m_pInternalInOutBufferSizeInfloat = 0;
	long						m_iInputSizeInBytesPerChannel = 0;
	size_t						m_iInputSizeInFloatPerChannel = 0;
	size_t						m_iNumOfConvBufferChannels = 0;
	
	// Output
	unsigned char*				m_pOutputBuffer = nullptr;
	size_t						m_iInternalInputBufferLengthInSample = 0;
	size_t						m_iConvolutionLengthInSample = 0;
	size_t						m_iNumOfChannels = 0;

	//WASAPIPlayer				m_WASAPIPlayer;
	//WASAPIPlayer				m_WASAPIRecorder;
	std::unique_ptr<IWavPlayer> m_WASAPIPlayer;
	std::unique_ptr<IWavPlayer> m_WASAPIRecorder;

	std::thread*				m_threadProcessing = nullptr;
	std::thread*				m_threadRecord = nullptr;
	FILE*						m_pDiskBuffer = nullptr;
	size_t						m_iNumOfValidBytesInDiskBuffer = 0;
	char*						m_cpRecordWAVFileName = nullptr;

	//static unsigned _stdcall processThreadStub(void * ptr);
private: //TAN modules
	void TANteardown();
	amf::TANContextPtr m_pTANContext;
	amf::TANFFTPtr  m_pTANFFT;
	amf::TANConvolutionPtr  m_pTANConvolution;
	amf::TANMathPtr  m_pTANMath;
	amf::TANConverterPtr m_pTANConverter;
	amf::TANFilterPtr m_pTANFilter;
};