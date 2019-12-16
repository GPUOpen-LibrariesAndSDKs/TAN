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

#include "tanlibrary/include/TrueAudioNext.h"

#define MAJOR_RATIO_12 1.25
#define MAJOR_RATIO_23 1.2
#define MAJOR_RATIO_13 1.5

#define NODE_REQUENCY_C0 16.35


enum eBufferType
{
	eFloat,
	eShort
};

enum eOverFlowType
{
	eNone,
	ePositive,
	eNegative
};

class Audiobuffer
{
	// This is the audio buffer that used for testing
public:
	Audiobuffer(eBufferType _type, amf_uint _size, amf_uint _step = 1, amf_uint _nchannel = 1);
	Audiobuffer(){}
	~Audiobuffer();
	float** getFloatTypeBuffer() const;
	short** getShortTypeBuffer() const;
	cl_mem* getCLBuffer() const;
	AMF_RESULT emptybuffer();

	AMF_RESULT fillchannelforconverter(eOverFlowType _type = eNone, amf_uint _channel = 0); // Fill one of the channel in the buffer
	AMF_RESULT fillbufferforconverter(eOverFlowType _type = eNone); // Fill the whole buffer
	AMF_RESULT fillchannelforFFT(amf_uint _channel = 0) const;
	AMF_RESULT fillbufferforFFT() const;
	AMF_RESULT fillchannelforConvolution(amf_uint _channel = 0, amf_uint pulseInterval = 1024) const;
	AMF_RESULT fillbufferforConvolution(amf_uint pulseInterval = 1024) const;
	AMF_RESULT fillchannelforResponse(amf_uint _channel = 0, amf_uint pulseInterval = 1024) const;
	AMF_RESULT fillbufferforResponse(amf_uint pulseInterval = 1024);
	AMF_RESULT fillfloatinchannel(int _channel, float _input);
	AMF_RESULT fillfloatinbuffer(float _input);
	AMF_RESULT fillSineInBuffer(size_t step, size_t _channel, float frequency);
	AMF_RESULT verifyChannelConverterResult(Audiobuffer& _output, float _conversion_gain, amf_uint _channel = 0) const;
	AMF_RESULT verifyBufferConverterResult(Audiobuffer& _output, float _conversion_gain) const;
	AMF_RESULT verifyChannelFFTResult(Audiobuffer& _output, const size_t allowrance, amf_uint _channel) const;
	AMF_RESULT verifyBufferFFTResult(Audiobuffer& _output, const size_t allowrance) const;
	AMF_RESULT verifyChannelConvolutionResult(Audiobuffer& _output, const size_t allowrance, amf_uint _channel) const;
	AMF_RESULT verifyBufferConvolutionResult(Audiobuffer& _output, const size_t allowrance) const;
	AMF_RESULT compareWith(Audiobuffer& ref, Audiobuffer** diff);
	AMF_RESULT compareWith(float* ref, size_t numfInSample, size_t channelNum, float allowance);
	AMF_RESULT compareWith(Audiobuffer&ref, size_t numfInSample, size_t channelNum, size_t refChannelNum, float allowance);
	
	Audiobuffer* convolutewith(Audiobuffer* _input) const; // Do direct convolution.
	Audiobuffer* diff(Audiobuffer* _input) const;
	AMF_RESULT copytoCLBuffer(cl_context _context);
	AMF_RESULT copyfromCLBuffer(cl_command_queue _queue);
	amf_uint getChannelOffset(int _channel) const;
	AMF_RESULT setChannelOffset(amf_uint _offset, int _channel) const;
	void printbuffer(amf_uint _channel, amf_size from = 0, amf_size to = 0) const;
	void WriteToWave(char* filename, int samplesPersec);
	void ReadFromWave(char* filename);
	amf_uint m_iSize = 0;
	amf_uint m_iStep = 1;
	amf_uint m_iNumOfChannel = 1;
	eBufferType m_eBufferType = eFloat;
	eOverFlowType m_eOverFlowType = eNone;
	amf_size* m_iOffset = nullptr;
	amf_uint m_iBufferNumber = 0; // Use for debug purpose;
	amf_uint32 m_iSamplePerSec = 48000;
	amf_uint32 m_iBitsPerSample = 32;
private:
	void** m_pBuffer = nullptr;
	cl_mem* m_pCLBuffer = nullptr;
};