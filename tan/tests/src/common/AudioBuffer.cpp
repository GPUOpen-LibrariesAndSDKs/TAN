#include "AudioBuffer.h"
#include "assert.h"
#include <iostream>
#include "common.h"
#include "wav.h"
#include <algorithm>
#define M_PI       3.14159265358979323846
const size_t c_fltToShrCoversionEpsilon = size_t(((1.f - -1.f) / FLT_EPSILON) /
	(SHRT_MAX - SHRT_MIN) + 0.5f);

Audiobuffer::Audiobuffer(eBufferType _type, amf_uint _size, amf_uint _step, amf_uint _nchannel)
{
	m_eBufferType = _type;
	m_iOffset = new amf_size[_nchannel];
	m_iStep = _step;
	m_iSize = _size;
	m_iNumOfChannel = _nchannel;
	switch (_type)
	{
	case eFloat:{
		float** ptr = new float*[_nchannel];
		for (amf_uint i = 0; i < _nchannel; i++)
		{
			ptr[i] = new float[_size];
			ZeroMemory(ptr[i], sizeof(float)*_size);
			m_iOffset[i] = 0;
		}
		m_pBuffer = (void**)ptr;
		break;
	}
	case eShort:{
		short** ptr = new short*[_nchannel];
		for (amf_uint i = 0; i < _nchannel; i++)
		{
			ptr[i] = new short[_size];
			ZeroMemory(ptr[i], sizeof(short)*_size);
			m_iOffset[i] = 0;
		}
		m_pBuffer = (void**)ptr;
		break;
	}
	default:
		assert(0); // Unexpected Buffer Type;
	}
	emptybuffer();
}

Audiobuffer::~Audiobuffer()
{
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float** buffer = this->getFloatTypeBuffer();
		for (amf_uint i = 0; i < m_iNumOfChannel; i++)
		{
			delete[]buffer[i];
		}
		delete[]buffer;
		break;
	}
	case eShort:
	{
		short** buffer = this->getShortTypeBuffer();
		for (amf_uint i = 0; i < m_iNumOfChannel; i++)
		{
			delete[]buffer[i];
		}
		delete[]buffer;
		break;
	}
	}
	delete[]m_iOffset;
	if (m_pCLBuffer != NULL)
	{
		cl_int clStatus = 0;
		// Clean up the cl buffer
		for (amf_uint i = 0; i < m_iNumOfChannel; i++)
		{
			clStatus = clReleaseMemObject(m_pCLBuffer[i]);
			if (clStatus != CL_SUCCESS)
			{
				assert(0); // cannot release opencl memory
			}
		}
		delete[]m_pCLBuffer;
	}
}

float** Audiobuffer::getFloatTypeBuffer() const
{
	return (float**)m_pBuffer;
}

short** Audiobuffer::getShortTypeBuffer() const
{
	return (short**)m_pBuffer;
}

cl_mem* Audiobuffer::getCLBuffer() const
{
	return m_pCLBuffer;
}

AMF_RESULT Audiobuffer::emptybuffer()
{
	switch (m_eBufferType)
	{
	case(eFloat) :
	{
		float** buffer = (float**)m_pBuffer;
		for (amf_uint i = 0; i < m_iNumOfChannel; i++)
		{
			ZeroMemory(buffer[i], sizeof(float)*m_iSize);
		}
		break;
	}
	case(eShort) :
	{
		short** buffer = (short**)m_pBuffer;
		for (amf_uint i = 0; i < m_iNumOfChannel; i++)
		{
			ZeroMemory(buffer[i], sizeof(short)*m_iSize);
		}
		break;
	}
	default:
		return AMF_UNEXPECTED;
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillchannelforconverter(eOverFlowType _type, amf_uint _channel)
{
	assert(m_pBuffer != NULL); // Buffer Not Initialized
	this->m_eOverFlowType = _type;
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float* data = ((float**)m_pBuffer)[_channel];
		for (amf_size i = m_iOffset[_channel]; i < m_iSize; i++)
		{
			// Include the presence of overflowing values if necessary otherwise as normal
			// Values are close to the legal range of [-1, 1]
			if (_type == ePositive)
			{
				data[i] = (float)1 + ((float)i / SHRT_MAX);
			}
			else if (_type == eNegative)
			{
				data[i] = (float)-1 - ((float)(i + 1) / SHRT_MAX);
			}
			else
			{
				data[i] = (float)i / SHRT_MAX;
			}
		}
		break;
	}
	case eShort:
	{
		short *dataIn = (short *)m_pBuffer[_channel];
		for (amf_size i = m_iOffset[_channel]; i < m_iSize; i++)
		{
			dataIn[i] = short(i);
		}
		break;
	}
	default:
		return AMF_UNEXPECTED;
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillbufferforconverter(eOverFlowType _type)
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		fillchannelforconverter(_type, i);
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillchannelforFFT(amf_uint _channel) const
{
	float frequency = float(5);
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float *dataIn = getFloatTypeBuffer()[_channel];
		dataIn[0] = 1.0f;
//		for (amf_uint i = 0; i < m_iSize/2 ; i += 1)
//		{
//			dataIn[i << 1] = (float)i;
//			dataIn[(i << 1) + 1] = 0.0f;
//		}
		break;
	}

	case eShort:
	{
		short *dataIn = getShortTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize / 2; i += 1)
		{
			dataIn[i * 2] = short(cosf(i * 3.14f / frequency) * SHRT_MAX);
			dataIn[i * 2 + 1] = 0;
		}
		break;
	}

	default:
		return AMF_UNEXPECTED;
	}

	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillbufferforFFT() const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		fillchannelforFFT(i);
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillchannelforConvolution(amf_uint _channel, amf_uint pulseInterval) const
{
	//assert(m_eBufferType == eFloat); // For convolution, the buffer type must be float;
	if (m_eBufferType != eFloat)
	{
		return AMF_UNEXPECTED;
	}
	float* buffer = getFloatTypeBuffer()[_channel];
	int interval = pulseInterval;
	int pulse_width = interval >> 2;
	int pulse_duration = pulse_width >> 2;
	float frequency = 1.0f / pulse_duration;
	for (int i = 0; i < m_iSize; i++)
	{
		//buffer[i] = sinf(i);
		if (i % interval < pulse_width)
		{
			float a = 0.5 * sinf((float)i * 2.0f * M_PI*frequency);
			buffer[i] = a;
		}
		else
			buffer[i] = 0;
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillbufferforConvolution(amf_uint pulseInterval) const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(fillchannelforConvolution(i,pulseInterval));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillchannelforResponse(amf_uint _channel, amf_uint pulseInterval) const
{
	if (m_eBufferType != eFloat)
	{
		return AMF_UNEXPECTED;
	}
	else
	{
		float *dataIn = getFloatTypeBuffer()[_channel];
		amf_uint subInterval = pulseInterval >> 2;
		dataIn[0] = 1.0f;
		dataIn[subInterval] = 0.75f;
		dataIn[subInterval << 1] = 0.5f;
		return AMF_OK;
	}
}

AMF_RESULT Audiobuffer::fillbufferforResponse(amf_uint pulseInterval)
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(fillchannelforResponse(i, pulseInterval));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillfloatinchannel(int _channel, float _input)
{
	float* buffer = getFloatTypeBuffer()[_channel];
	for (amf_uint i = 0; i < m_iSize; i++)
	{
		buffer[i] = _input;
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillfloatinbuffer(float _input)
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		fillfloatinchannel(i, _input);
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillSineInBuffer(size_t _step, size_t _channel, float _frequency)
{
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float *dataIn = getFloatTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize; i += _step)
		{
			dataIn[i] = sinf((float)i * 2.0f * M_PI*_frequency);
		}
	}

	case eShort:
	{
		short *dataIn = getShortTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize; i += _step)
		{
			dataIn[i] = sin((float)i * 2.0f * M_PI*_frequency);
		}
	}
	default:
		return AMF_UNEXPECTED;
	}

}

AMF_RESULT Audiobuffer::verifyChannelConverterResult(Audiobuffer& _output, float _conversion_gain, amf_uint _channel) const
{
	assert(this->m_eBufferType != _output.m_eBufferType); // Buffers have same type
	assert(this->m_iSize == _output.m_iSize); // Buffer have different size;
	float float_to_short_scale = SHRT_MAX * _conversion_gain;
	float short_to_float_scale = _conversion_gain / SHRT_MAX;
	bool failed = false;
	amf_uint input_index = 0;
	amf_size output_offset = _output.m_iOffset[_channel];
	amf_size input_offset = this->m_iOffset[_channel];
	for (amf_uint i = 0; i < this->m_iSize; i++)
	{
		// Valid data only exists in this region
		switch (_output.m_eBufferType)
		{
		case eFloat:
		{
			float *dataOut = _output.getFloatTypeBuffer()[_channel];
			short* dataIn = this->getShortTypeBuffer()[_channel];
			if (i >= output_offset &&
				i - output_offset <= ((_output.m_iSize - input_offset - 1) / this->m_iStep) * _output.m_iStep &&
				(i - output_offset) % _output.m_iStep == 0)
			{
				float output = dataIn[input_index] * short_to_float_scale;
				if (fabs(dataOut[i] - output) > FLT_EPSILON)
				{
					//this->printbuffer(_channel);
					//_output.printbuffer(_channel);
					printf("Error in item = %d.-- Output: %f , Should be: %f \n", i, dataOut[i], output);
					failed = true;
				}
				input_index += this->m_iStep;
			}
			else if (dataOut[i] != 0) {
				printf("Error in item = %d \n", i);
				failed = true;

			}
			break;

		}
		case eShort:
		{
			// Overflows guaranteed to occur here; clipping required
			short output = 0;
			short *dataOut = _output.getShortTypeBuffer()[_channel];
			float* dataIn = this->getFloatTypeBuffer()[_channel];

			if (i >= output_offset &&
				i - output_offset <= ((_output.m_iSize - input_offset - 1) / this->m_iStep) * _output.m_iStep &&
				(i - output_offset) % _output.m_iStep == 0)
			{
				if (this->m_eOverFlowType == ePositive)
				{
					output = short(SHRT_MAX);
				}
				else if (this->m_eOverFlowType == eNegative)
				{
					output = short(SHRT_MIN);
				}
				else
				{
					// No overflow indicates regular data to be converted
					output = short((dataIn[input_index] * float_to_short_scale));
				}
				if (dataOut[i] != output)
				{
					printf("Error in item = %d.-- Output: %d , Should be: %d \n", i, dataOut[i], output);
					failed = true;
				}
				input_index += this->m_iStep;
			}
			else if (dataOut[i] != 0) {
				// Data should be zeroed-out otherwise	
				printf("Error in item = %d \n", i);
				failed = true;
			}
			break;
		}

		}
	}
	if (failed)
	{
		return AMF_FAIL;
	}
	else
	{
		return AMF_OK;
	}

}

AMF_RESULT Audiobuffer::verifyBufferConverterResult(Audiobuffer& _output, float _conversion_gain) const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(verifyChannelConverterResult(_output, _conversion_gain, i));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::verifyChannelFFTResult(Audiobuffer& _output, const size_t _allowrance, amf_uint _channel) const
{
	if ((_output.m_eBufferType != this->m_eBufferType)||(this->m_eBufferType!=eFloat||_output.m_eBufferType!=eFloat))
	{
		return AMF_UNEXPECTED; // Unmatch buffer type
	}
	size_t shrAllowedEps = _allowrance + c_fltToShrCoversionEpsilon;
	float *dataIn = this->getFloatTypeBuffer()[_channel];
	float *dataOut = _output.getFloatTypeBuffer()[_channel];
	for (amf_uint i = 0; i < m_iSize; i++)
	{
		if (fabs(dataIn[i] - dataOut[i]) / FLT_EPSILON >
			shrAllowedEps)
		{
 			printf("Error in item = %d, Input: %f, Output %f \n", i,dataIn[i],dataOut[i]);
			return AMF_FAIL;
		}
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::verifyBufferFFTResult(Audiobuffer& _output, const size_t allowrance) const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(verifyChannelFFTResult(_output, allowrance, i));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::verifyChannelConvolutionResult(Audiobuffer& _output, const size_t allowrance, amf_uint _channel) const
{
	float* datain = getFloatTypeBuffer()[_channel];
	float* dataout = _output.getFloatTypeBuffer()[_channel];
	size_t max_eps = 0;
	size_t avg_eps = 0;
	size_t total_eps = 0;
	size_t max_eps_pos = 0;
	float reference_value = 0.0f;
	float output_value = 0.0f;
	for (amf_uint s = 0; s < m_iSize; s++)
	{
		size_t Num_of_epsilon = size_t(fabs(datain[s] - dataout[s]) / FLT_EPSILON);
		if (Num_of_epsilon > max_eps)
		{
			max_eps = Num_of_epsilon;
			max_eps_pos = s;
			reference_value = datain[s];
			output_value = dataout[s];
		}
		total_eps += Num_of_epsilon;

	}
	avg_eps = total_eps / m_iSize;
	if (max_eps> allowrance)
	{
		printf("Difference is greater than allowance. MAX.Epsilon: at i=%llu output value=%f, reference_value=%f allowance=%llu. Avg.Eps: %llu Max.Eps: %llu \n", max_eps_pos,
			output_value, reference_value, allowrance, avg_eps, max_eps);
		return AMF_FAIL;
	}
	else
	{
		printf("Difference is smaller than allowance. MAX.Epsilon: at i=%llu output value=%f, reference_value=%f allowance=%llu. Avg.Eps: %llu Max.Eps: %llu \n", max_eps_pos,
			output_value, reference_value, allowrance, avg_eps, max_eps);
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::verifyBufferConvolutionResult(Audiobuffer& _output, const size_t allowrance) const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(verifyChannelConvolutionResult(_output, allowrance, i));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::compareWith(Audiobuffer& ref, Audiobuffer** diff)
{
	if ((ref.m_eBufferType != m_eBufferType)||
		(ref.m_iSize != m_iSize)||
		(ref.m_iNumOfChannel != m_iNumOfChannel))
	{
		return AMF_FAIL;
	}
	Audiobuffer* ret = nullptr;

	//Audiobuffer* ret = new Audiobuffer(m_eBufferType, m_iSize, m_iStep, m_iNumOfChannel);
	if (ref.m_eBufferType == eFloat)
	{
		

		float** thisbuffer = getFloatTypeBuffer();
		float** refbuffer = ref.getFloatTypeBuffer();
		float diff = 0.0f;
		for (amf_uint i = 0; i < ref.m_iNumOfChannel; i++)
		{
			for (amf_uint j = 0; j < ref.m_iSize; j++)
			{
				diff = abs(thisbuffer[i][j] - refbuffer[i][j]);
				if ( diff > FLT_EPSILON)
				{
					// Difference found, put in the ret array
					if (ret == nullptr)
					{
						ret = new Audiobuffer(m_eBufferType, m_iSize, m_iStep, m_iNumOfChannel);
					}
					ret->getFloatTypeBuffer()[i][j] = diff;
				}
			}
		}
	}
	if (ref.m_eBufferType == eShort)
	{
		
	}

	return AMF_OK;
}

AMF_RESULT Audiobuffer::compareWith(float* ref, size_t numfInSample, size_t channelNum, float allowance)
{
	float* channel = getFloatTypeBuffer()[channelNum];
	for (size_t i = 0; i < numfInSample; i++)
	{
		if (abs(channel[i] - ref[i]) > allowance)
		{
			return AMF_FAIL;
		}
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::compareWith(Audiobuffer&ref, size_t numfInSample, size_t channelNum, size_t refChannelNum, float allowance)
{
	return compareWith(ref.getFloatTypeBuffer()[refChannelNum], numfInSample, channelNum, allowance);
}

Audiobuffer* Audiobuffer::convolutewith(Audiobuffer* _input) const
{
    amf_uint i, j, i1;
	float tmp;

	//allocated convolution buffer

	Audiobuffer* output = new Audiobuffer(eFloat, this->m_iSize, 1,this->m_iNumOfChannel);
	//convolution process
	for (amf_uint channel_index = 0; channel_index < m_iNumOfChannel; channel_index++)
	{
		float* A = getFloatTypeBuffer()[channel_index];
		float* B = _input->getFloatTypeBuffer()[channel_index];
		float* C = output->getFloatTypeBuffer()[channel_index];
		for (i = 0; i<output->m_iSize; i++)
		{
			i1 = i;
			tmp = 0.0;
			for (j = 0; j<_input->m_iSize; j++)
			{
				if (i1 >= 0 && i1<this->m_iSize)
					tmp = tmp + (A[i1] * B[j]);
				i1 = i1 - 1;
				
			}
			C[i] = tmp;
		}
	}
	return output;
}

Audiobuffer* Audiobuffer::diff(Audiobuffer* _input) const
{
	amf_size max_size = std::max(this->m_iSize, _input->m_iSize);
	amf_size max_channel = std::max(this->m_iNumOfChannel, _input->m_iNumOfChannel);
	if (this->m_eBufferType != _input->m_eBufferType)
	{
		// mistype diff temporarilly not supported
		return nullptr;
	}
	Audiobuffer* output = new Audiobuffer(this->m_eBufferType, max_size, 1, max_channel);
	for (int i = 0; i < max_channel; i++)
	{
		if (i >= m_iNumOfChannel || i >= _input->m_iNumOfChannel)
		{
			output->fillfloatinchannel(i, 1.0f);
		}
		else
		{
			void* buffer = output->m_pBuffer[i];
			void* bufferA = this->m_pBuffer[i];
			void* bufferB = _input->m_pBuffer[i];
			for (int j = 0; j < max_size; j++)
			{
				if (j >= m_iSize || j >= _input->m_iSize)
				{
					if (m_eBufferType == eShort)
					{
						((short*)buffer)[i] = 1;
					}
					if (m_eBufferType == eFloat)
					{
						((float*)buffer)[i] = 1.0f;
					}
				}
				else
				{
					if (m_eBufferType == eShort)
					{
						((short*)buffer)[i] = ((short*)bufferA)[i] - ((short*)bufferB)[i];
					}
					if (m_eBufferType == eFloat)
					{
						((float*)buffer)[i] = ((float*)bufferA)[i] - ((float*)bufferB)[i];
					}
				}
			}
		}
	}
	return output;
}

AMF_RESULT Audiobuffer::copytoCLBuffer(cl_context _context)
{
	// This function copys the content in the current native buffer and allocate a new OpenCL buffer for it.
	// cl_mem instance would be clean if this function has called.
	m_pCLBuffer = new cl_mem[m_iNumOfChannel];
	size_t cl_buffer_size = m_iSize;
	switch (m_eBufferType)
	{
	case eFloat:{
		cl_buffer_size *= sizeof(float);
		break;
	}
	case eShort:{
		cl_buffer_size *= sizeof(short);
	}
	}
	cl_int status;
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		m_pCLBuffer[i] = clCreateBuffer(_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, cl_buffer_size,
			m_pBuffer[i], &status);
		if (status != CL_SUCCESS)
		{
			return AMF_FAIL; // Cannot create OpenCL buffer;
		}
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::copyfromCLBuffer(cl_command_queue _queue)
{
	cl_int clStatus;
	size_t cl_buffer_size = m_iSize;
	switch (m_eBufferType)
	{
	case eFloat:{
		cl_buffer_size *= sizeof(float);
		break;
	}
	case eShort:{
		cl_buffer_size *= sizeof(short);
	}
	}
	if (m_pCLBuffer == nullptr)
	{
		return AMF_FAIL; // CL buffer doesn't exist
	}
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		cl_event copy_event = nullptr;
		if (m_eBufferType == eFloat)
		{
			clStatus = clEnqueueReadBuffer(_queue, m_pCLBuffer[i], CL_TRUE, 0, cl_buffer_size, (float*)m_pBuffer[i], 0, NULL, NULL);
		}
		else if (m_eBufferType == eShort)
		{
			clStatus = clEnqueueReadBuffer(_queue, m_pCLBuffer[i], CL_TRUE, 0, cl_buffer_size, (short*)m_pBuffer[i], 0, NULL, NULL);
		}
		else
		{
			return AMF_FAIL; //Not supported buffer type;
		}
		if (clStatus != CL_SUCCESS)
		{
			return AMF_FAIL; //Fail to retrieve buffer from opencl
		}
	}
	return AMF_OK;
}

amf_uint Audiobuffer::getChannelOffset(int _channel) const
{
	assert(_channel >= 0 && _channel < int(m_iNumOfChannel)); // Invalid channel number;
	return amf_uint(m_iOffset[_channel]);
}

AMF_RESULT Audiobuffer::setChannelOffset(amf_uint _offset, int _channel) const
{
	assert(_channel >= 0 && _channel < int(m_iNumOfChannel)); // Invalid channel number;
	m_iOffset[_channel] = _offset;
	return AMF_OK;
}

void Audiobuffer::printbuffer(amf_uint _channel, amf_size from, amf_size to) const
{
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float* data = getFloatTypeBuffer()[_channel];
		for (amf_size i = from; i < to; i++)
		{
			std::cout << i << " : " << data[i] << " " << std::endl;;
		}
		std::cout << std::endl;
		break;
	}
	case eShort:
	{
		short* data = getShortTypeBuffer()[_channel];
		for (amf_size i = from; i < to; i++)
		{
			std::cout << i << " : " << data[i] << " " << std::endl;
		}
		std::cout << std::endl;
		break;
	}
	}
}

void Audiobuffer::WriteToWave(char* filename, int samplesPersec)
{
	int numOfBits;
	m_eBufferType == eFloat ? numOfBits = sizeof(float) : numOfBits = sizeof(short);
	numOfBits *= 4;
	//printbuffer(0);
	if (m_eBufferType == eFloat)
		WriteWaveFileF(filename, samplesPersec, m_iNumOfChannel, numOfBits, m_iSize, getFloatTypeBuffer());
	else
	{
		printf("Unsupported buffer type to be written to WAV. ");
	}
}

void Audiobuffer::ReadFromWave(char* filename)
{
	unsigned char* buffer_c = nullptr;
	int samplepersec = 0;
	int bitsPersample = 0;
	int numofchannel = 0;
	long nSamples = 0;
	bool a = ReadWaveFile(filename, &samplepersec, &bitsPersample, &numofchannel, &nSamples, &buffer_c, (float***)&m_pBuffer);
	m_iNumOfChannel = numofchannel;
	m_iSamplePerSec = samplepersec;
	m_iBitsPerSample = bitsPersample;
	m_iSize = nSamples;
}
