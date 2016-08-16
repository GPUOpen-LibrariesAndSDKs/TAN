#include "AudioBuffer.h"
#include "assert.h"
#include <iostream>
#include "common.h"
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
	if (m_pCLBuffer)
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
	float frequency = float(random(10) + 1);
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float *dataIn = getFloatTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize / 2; i += 1)
		{
			dataIn[i * 2] = cosf(i * 3.14f / frequency);
			dataIn[i * 2 + 1] = 0;
		}
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

AMF_RESULT Audiobuffer::fillchannelforConvolution(amf_uint _channel) const
{
	//assert(m_eBufferType == eFloat); // For convolution, the buffer type must be float;
	if (m_eBufferType != eFloat)
	{
		return AMF_UNEXPECTED;
	}
	float* buffer = getFloatTypeBuffer()[_channel];
	for (amf_uint i = 0; i < m_iSize; i++)
	{
		buffer[i] = sinf(i*3.14f / 16.f);
		//buffer[i] = 1.0f;
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillbufferforConvolution() const
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(fillchannelforConvolution(i));
	}
	return AMF_OK;
}

AMF_RESULT Audiobuffer::fillchannelforResponse(amf_uint _channel) const
{
	if (m_eBufferType != eFloat)
	{
		return AMF_UNEXPECTED;
	}
	else
	{
		float *dataIn = getFloatTypeBuffer()[_channel];
		/*for (amf_uint i = 0; i < m_iSize; i ++)
		{
			//dataIn[i] = sinf((i+1) * 3.14f / 32.f) / (i + 1);
			dataIn[i] = 1.0f;
		}*/
		return AMF_OK;
		dataIn[0] = 1.0f;
	}
}

AMF_RESULT Audiobuffer::fillbufferforResponse()
{
	for (amf_uint i = 0; i < m_iNumOfChannel; i++)
	{
		RETURN_IF_FAILED(fillchannelforResponse(i));
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
					printf("Error in item = %d.-- Output: %f , Should be: %f", i, dataOut[i], output);
					failed = true;
				}
				input_index += this->m_iStep;
			}
			else if (dataOut[i] != 0) {
				printf("Error in item = %d\n", i);
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
					printf("Error in item = %d.-- Output: %d , Should be: %d", i, dataOut[i], output);
					failed = true;
				}
				input_index += this->m_iStep;
			}
			else if (dataOut[i] != 0) {
				// Data should be zeroed-out otherwise	
				printf("Error in item = %d\n", i);
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
		if (fabs(dataIn[i] - dataOut[i]) / SHRT_MAX / FLT_EPSILON >
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
	for (amf_uint s = 0; s < m_iSize; s++)
	{
		size_t Num_of_epsilon = size_t(fabs(datain[s] - dataout[s]) / FLT_EPSILON);
		if (Num_of_epsilon> allowrance)
		{
			printf(": mismatch at i=%d input=%f, output=%f , Num of Epsilon: %llu\n", s,
				datain[s], dataout[s], Num_of_epsilon);
		}
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
		if (m_eBufferType == eFloat)
		{
			clStatus = clEnqueueReadBuffer(_queue, m_pCLBuffer[i], CL_TRUE, 0, cl_buffer_size, (float*)m_pBuffer[i], NULL, NULL, NULL);
		}
		else if (m_eBufferType == eShort)
		{
			clStatus = clEnqueueReadBuffer(_queue, m_pCLBuffer[i], CL_TRUE, 0, cl_buffer_size, (short*)m_pBuffer[i], NULL, NULL, NULL);
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

void Audiobuffer::printbuffer(amf_uint _channel) const
{
	switch (m_eBufferType)
	{
	case eFloat:
	{
		float* data = getFloatTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize; i++)
		{
			std::cout << i << " : " << data[i] << " ";
		}
		std::cout << std::endl;
		break;
	}
	case eShort:
	{
		short* data = getShortTypeBuffer()[_channel];
		for (amf_uint i = 0; i < m_iSize; i++)
		{
			std::cout << i << " : " << data[i] << " ";
		}
		std::cout << std::endl;
		break;
	}
	}
}