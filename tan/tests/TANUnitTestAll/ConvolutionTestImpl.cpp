#include "ConvolutionTest.h"
#include "AudioBuffer.h"

DEFINE_BASIC_UNIT_TEST(Convolution_Native_Buffer, L"Run Convolution On Native Buffer");
bool Convolution_Native_Buffer::OnRun(UnitTestParamBlob* const params)
{
	ConvolutionTestSuitesParam* real_parameters = (ConvolutionTestSuitesParam*)params;

	amf_uint32* flagMasks = new amf_uint32[real_parameters->m_iNumberOfChannel];
	for (amf_uint i = 0; i < real_parameters->m_iNumberOfChannel; i++)
	{
		flagMasks[i] = amf::TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM;
	}


	Audiobuffer inputbuffer(eFloat, real_parameters->m_ibufferSizeInSamples, 1, real_parameters->m_iNumberOfChannel);
	Audiobuffer responsebuffer(eFloat, real_parameters->m_iresponseLengthInSamples, 1, real_parameters->m_iNumberOfChannel);
	Audiobuffer outputbuffer(eFloat, real_parameters->m_ibufferSizeInSamples, 1, real_parameters->m_iNumberOfChannel);
	
	inputbuffer.fillbufferforConvolution();
	responsebuffer.fillbufferforResponse();

	RETURN_FALSE_IF_FAILED(real_parameters->m_pConvolution->UpdateResponseTD(responsebuffer.getFloatTypeBuffer(),
		responsebuffer.m_iSize, flagMasks, 0));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pConvolution->Process(inputbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer(),
		real_parameters->m_ibufferSizeInSamples, flagMasks, NULL));
	Audiobuffer* direct_result = inputbuffer.convolutewith(&responsebuffer);
	RETURN_FALSE_IF_FAILED(direct_result->verifyBufferConvolutionResult(outputbuffer, 8000));
	delete direct_result;
	return true;
}

DEFINE_BASIC_UNIT_TEST(Convolution_OpenCL_Buffer, L"Run Convolution On OpenCL Buffer");
bool Convolution_OpenCL_Buffer::OnRun(UnitTestParamBlob* const params)
{
	ConvolutionTestSuitesParam* real_parameters = (ConvolutionTestSuitesParam*)params;

	Audiobuffer inputbuffer(eFloat, real_parameters->m_ibufferSizeInSamples, 1, real_parameters->m_iNumberOfChannel);
	Audiobuffer responsebuffer(eFloat, real_parameters->m_iresponseLengthInSamples, 1, real_parameters->m_iNumberOfChannel);
	Audiobuffer outputbuffer(eFloat, real_parameters->m_ibufferSizeInSamples, 1, real_parameters->m_iNumberOfChannel);
	
	amf_uint32* flagMasks = new amf_uint32[real_parameters->m_iNumberOfChannel];
	for (amf_uint i = 0; i < real_parameters->m_iNumberOfChannel; i++)
	{
		flagMasks[i] = amf::TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM;
	}


	inputbuffer.fillbufferforConvolution();
	responsebuffer.fillbufferforResponse();
	
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	responsebuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	
	RETURN_FALSE_IF_FAILED(real_parameters->m_pConvolution->UpdateResponseTD(responsebuffer.getCLBuffer(),
		responsebuffer.m_iSize, nullptr, 0));
	amf_size NumOfSamplesProcessed = 0;
	RETURN_FALSE_IF_FAILED(real_parameters->m_pConvolution->Process(inputbuffer.getCLBuffer(), outputbuffer.getCLBuffer(),
		real_parameters->m_ibufferSizeInSamples, NULL, NULL));
	
	//outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	//Audiobuffer* direct_result = inputbuffer.convolutewith(&responsebuffer);
	//RETURN_FALSE_IF_FAILED(direct_result->verifyBufferConvolutionResult(outputbuffer, 1500));
	//delete direct_result;
	return true;
}


ConvolutionTestSuites::ConvolutionTestSuites(const std::wstring& suiteName, enum eTestMode mode) : UnitTestSuite(suiteName)
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = mode;
	this->TrapSegFault(true);
	RegisterTests();
}

ConvolutionTestSuites::ConvolutionTestSuites(const std::wstring& suiteName, ConvolutionTestSuitesParam* param) : UnitTestSuite(suiteName)
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = param->m_eTestMode;
	this->TrapSegFault(true);
	RegisterTests();
	this->m_pParam = param;
}

UnitTestParamBlob* ConvolutionTestSuites::OnPrepare()
{
	ConvolutionTestSuitesParam* param;
	if (m_pParam == nullptr)
	{
		param = new ConvolutionTestSuitesParam(m_testmode);
	}
	else
	{
		param = m_pParam;
	}
	param->InitializeConvolution(param->m_eConvolutionMethod, param->m_iresponseLengthInSamples, param->m_ibufferSizeInSamples, param->m_iNumberOfChannel);
	return param;
}

void ConvolutionTestSuites::RegisterTests()
{
	REGISTER_UNIT_TEST((*this), Convolution_Native_Buffer, false);
	if (this->m_testmode == eGPU)
	{

		// REGISTER_UNIT_TEST((*this), Convolution_OpenCL_Buffer, false); 
		// Disabled due to graal bug
	}
}

void ConvolutionTestSuites::ParseCommandLine(int argc, wchar_t* argv[])
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->processCommandArgs(argc, argv);
}
