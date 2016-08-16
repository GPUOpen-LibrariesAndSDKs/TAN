#include "tanlibrary/include/TrueAudioNext.h"

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
	~Audiobuffer();
	float** getFloatTypeBuffer() const;
	short** getShortTypeBuffer() const;
	cl_mem* getCLBuffer() const;
	AMF_RESULT emptybuffer();
	AMF_RESULT fillchannelforconverter(eOverFlowType _type = eNone, amf_uint _channel = 0); // Fill one of the channel in the buffer
	AMF_RESULT fillbufferforconverter(eOverFlowType _type = eNone); // Fill the whole buffer
	AMF_RESULT fillchannelforFFT(amf_uint _channel = 0) const;
	AMF_RESULT fillbufferforFFT() const;
	AMF_RESULT fillchannelforConvolution(amf_uint _channel = 0) const;
	AMF_RESULT fillbufferforConvolution() const;
	AMF_RESULT fillchannelforResponse(amf_uint _channel = 0) const;
	AMF_RESULT fillbufferforResponse();
	AMF_RESULT fillfloatinchannel(int _channel, float _input);
	AMF_RESULT fillfloatinbuffer(float _input);
	AMF_RESULT verifyChannelConverterResult(Audiobuffer& _output, float _conversion_gain, amf_uint _channel = 0) const;
	AMF_RESULT verifyBufferConverterResult(Audiobuffer& _output, float _conversion_gain) const;
	AMF_RESULT verifyChannelFFTResult(Audiobuffer& _output, const size_t allowrance, amf_uint _channel) const;
	AMF_RESULT verifyBufferFFTResult(Audiobuffer& _output, const size_t allowrance) const;
	AMF_RESULT verifyChannelConvolutionResult(Audiobuffer& _output, const size_t allowrance, amf_uint _channel) const;
	AMF_RESULT verifyBufferConvolutionResult(Audiobuffer& _output, const size_t allowrance) const;
	Audiobuffer* convolutewith(Audiobuffer* _input) const; // Do direct convolution.
	AMF_RESULT copytoCLBuffer(cl_context _context);
	AMF_RESULT copyfromCLBuffer(cl_command_queue _queue);
	amf_uint getChannelOffset(int _channel) const;
	AMF_RESULT setChannelOffset(amf_uint _offset, int _channel) const;
	void printbuffer(amf_uint _channel) const;

	amf_uint m_iSize;
	amf_uint m_iStep;
	amf_uint m_iNumOfChannel;
	eBufferType m_eBufferType;
	eOverFlowType m_eOverFlowType;
	amf_size* m_iOffset;
private:
	
	void** m_pBuffer = nullptr;
	cl_mem* m_pCLBuffer = nullptr;
};