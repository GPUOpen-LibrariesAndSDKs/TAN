#include "TANTestSuitsParam.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
#include "assert.h"
TANTestSuitsParam::TANTestSuitsParam(eTestMode mode)
{
	TANCreateContext(TAN_FULL_VERSION, &m_pContext);
	this->m_eTestMode = mode;
	if (mode == eGPU)
	{
		cl_device_id id;
		cl_context context;
		getDeviceAndContext(0, &context, &id);
		cl_command_queue queue = createQueue(context, id);
		m_pContext->InitOpenCL(queue, queue);
	}
	
}

TANTestSuitsParam::TANTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype, int _generalCUcount, int _convolutionCUcount)
{
	TANCreateContext(TAN_FULL_VERSION, &m_pContext);
	if (mode == eGPU)
	{
		cl_device_id id;
		cl_context context;
		getDeviceAndContext(0, &context, &id);
		int general_queue_flag = 0;
		int convolution_queue_flag = 0;
		
		cl_command_queue general_queue;
		if (_generalqueuetype == eMediumPriorityQueue)
		{
			general_queue_flag = QUEUE_MEDIUM_PRIORITY;
			general_queue = createQueue(context, id,general_queue_flag);
		}
		else
		{
			general_queue_flag = QUEUE_REAL_TIME_COMPUTE_UNITS;
			assert(_convolutionCUcount != 0);
			general_queue = createQueue(context, id, general_queue_flag, _generalCUcount);
		}
		
		cl_command_queue convolution_queue;
		
		if (_convolutionqueuetype == eMediumPriorityQueue)
		{
			convolution_queue_flag = QUEUE_MEDIUM_PRIORITY;
			convolution_queue = createQueue(context, id, convolution_queue_flag);
		}
		else
		{
			convolution_queue_flag = QUEUE_REAL_TIME_COMPUTE_UNITS;
			assert(_convolutionCUcount != 0);
			convolution_queue = createQueue(context, id, convolution_queue_flag, _convolutionCUcount);
		}

		m_pContext->InitOpenCL(general_queue, convolution_queue);
	}
	this->m_eTestMode = mode;
}

AMF_RESULT ConverterTestSuitsParam::InitializeConverter()
{

	RETURN_IF_FAILED(TANCreateConverter(m_pContext, &m_pConverter));
	RETURN_IF_FAILED(m_pConverter->Init());

	return AMF_OK;
}

AMF_RESULT FFTTestSuitsParam::InitializeFFT()
{
	RETURN_IF_FAILED(TANCreateFFT(m_pContext, &m_pFFT));
	RETURN_IF_FAILED(m_pFFT->Init());

	return AMF_OK;
}

AMF_RESULT ConvolutionTestSuitesParam::InitializeConvolution(amf::TAN_CONVOLUTION_METHOD _method, amf_uint32 _responseLengthInSamples,
	amf_uint32 _bufferSizeInSamples, amf_uint32 _channels)
{
	RETURN_IF_FAILED(TANCreateConvolution(m_pContext,&m_pConvolution));
	RETURN_IF_FAILED(m_pConvolution->Init(_method, _responseLengthInSamples, _bufferSizeInSamples, _channels));
	return AMF_OK;
}

AMF_RESULT MathTestSuitesParam::InitializeMath()
{
	RETURN_IF_FAILED(TANCreateMath(m_pContext,&m_pMath));
	RETURN_IF_FAILED(m_pMath->Init());
	return AMF_OK;
}
