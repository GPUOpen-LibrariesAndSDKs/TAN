#include "Converter_Test.h"
#include "iostream"
#include <assert.h>

#include "AudioBuffer.h"

// Test case for converter with native buffer
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_Native, L"Convert float to short with native buffer");
bool Converter_Float2Short_Native::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eFloat, 1024,inStep);
	Audiobuffer outputbuffer(eShort, 1024,outStep);
	inputbuffer.fillchannelforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getFloatTypeBuffer()[0], inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getShortTypeBuffer()[0], outputbuffer.m_iStep, gain);
	
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyChannelConverterResult(outputbuffer,gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_Native, L"Convert short to float with native buffer");
bool Converter_Short2Float_Native::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eShort, 1024,inStep);
	Audiobuffer outputbuffer(eFloat, 1024,outStep);
	inputbuffer.fillchannelforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getShortTypeBuffer()[0], inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getFloatTypeBuffer()[0], outputbuffer.m_iStep, gain);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyChannelConverterResult(outputbuffer, gain));
	return true;
}

DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_Batch_Native_1CH, L"Convert float to short with native buffer (Batch 1 Channel)");
bool Converter_Float2Short_Batch_Native_1CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	
	Audiobuffer inputbuffer(eShort, 1024, inStep,1);
	Audiobuffer outputbuffer(eFloat, 1024, outStep,1);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getShortTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getFloatTypeBuffer(), outputbuffer.m_iStep, gain,1);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_Batch_Native_1CH, L"Convert short to float with native buffer (Batch 1 Channel)");
bool Converter_Short2Float_Batch_Native_1CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);

	Audiobuffer inputbuffer(eFloat, 1024, inStep, 1);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 1);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getFloatTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getShortTypeBuffer(), outputbuffer.m_iStep, gain, 1);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}

DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_Batch_Native_2CH, L"Convert float to short with native buffer (Batch 2 Channel)");
bool Converter_Float2Short_Batch_Native_2CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);

	Audiobuffer inputbuffer(eShort, 1024, inStep, 2);
	Audiobuffer outputbuffer(eFloat, 1024, outStep, 2);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getShortTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getFloatTypeBuffer(), outputbuffer.m_iStep, gain, 2);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_Batch_Native_2CH, L"Convert short to float with native buffer (Batch 2 Channel)");
bool Converter_Short2Float_Batch_Native_2CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);

	Audiobuffer inputbuffer(eFloat, 1024, inStep, 2);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 2);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getFloatTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getShortTypeBuffer(), outputbuffer.m_iStep, gain, 2);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}

DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_Batch_Native_10CH, L"Convert short to float with native buffer (Batch 10 Channel)");
bool Converter_Short2Float_Batch_Native_10CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);

	Audiobuffer inputbuffer(eFloat, 1024, inStep, 10);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 10);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getFloatTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getShortTypeBuffer(), outputbuffer.m_iStep, gain, 10);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_Batch_Native_10CH, L"Convert float to short with native buffer (Batch 10 Channel)");
bool Converter_Float2Short_Batch_Native_10CH::OnRun(UnitTestParamBlob* const params)
{
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);

	Audiobuffer inputbuffer(eFloat, 1024, inStep, 10);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 10);
	inputbuffer.fillbufferforconverter(overflow);
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getFloatTypeBuffer(), inputbuffer.m_iStep, samplesToProcess,
		outputbuffer.getShortTypeBuffer(), outputbuffer.m_iStep, gain, 10);

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}

// Test cases for converter with opencl buffer (GPU only)
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_OPENCL, L"Convert float to short with OPENCL buffer");
bool Converter_Float2Short_OPENCL::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eFloat, 1024, inStep);
	Audiobuffer outputbuffer(eShort, 1024, outStep);
	inputbuffer.fillchannelforconverter(overflow);
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	
	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer()[0], inputbuffer.m_iStep, 0, 
		amf::TAN_SAMPLE_TYPE_FLOAT,outputbuffer.getCLBuffer()[0],outputbuffer.m_iStep,0,amf::TAN_SAMPLE_TYPE_SHORT,samplesToProcess,gain);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyChannelConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_OPENCL, L"Convert short to float with OPENCL buffer");
bool Converter_Short2Float_OPENCL::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eShort, 1024, inStep);
	Audiobuffer outputbuffer(eFloat, 1024, outStep);
	inputbuffer.fillchannelforconverter(overflow);
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer()[0], inputbuffer.m_iStep, 0,
		amf::TAN_SAMPLE_TYPE_SHORT, outputbuffer.getCLBuffer()[0], outputbuffer.m_iStep, 0, amf::TAN_SAMPLE_TYPE_FLOAT, samplesToProcess, gain);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());

	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyChannelConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_OPENCL_1CH, L"Convert float to short with OPENCL buffer (Batch 1 Channel)");
bool Converter_Float2Short_OPENCL_1CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eFloat, 1024, inStep,1);
	Audiobuffer outputbuffer(eShort, 1024, outStep,1);
	inputbuffer.fillbufferforconverter(overflow);
	
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_FLOAT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_SHORT, samplesToProcess, gain,inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_OPENCL_1CH, L"Convert short to float with OPENCL buffer (Batch 1 Channel)");
bool Converter_Short2Float_OPENCL_1CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eShort, 1024, inStep, 1);
	Audiobuffer outputbuffer(eFloat, 1024, outStep, 1);
	inputbuffer.fillbufferforconverter(overflow);

	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_SHORT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_FLOAT, samplesToProcess, gain, inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_OPENCL_2CH, L"Convert float to short with OPENCL buffer (Batch 2 Channel)");
bool Converter_Float2Short_OPENCL_2CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eFloat, 1024, inStep, 2);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 2);
	inputbuffer.fillbufferforconverter(overflow);

	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_FLOAT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_SHORT, samplesToProcess, gain, inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_OPENCL_2CH, L"Convert short to float with OPENCL buffer (Batch 2 Channel)");
bool Converter_Short2Float_OPENCL_2CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eShort, 1024, inStep, 2);
	Audiobuffer outputbuffer(eFloat, 1024, outStep, 2);
	inputbuffer.fillbufferforconverter(overflow);

	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_SHORT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_FLOAT, samplesToProcess, gain, inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Float2Short_OPENCL_10CH, L"Convert float to short with OPENCL buffer (Batch 10 Channel)");
bool Converter_Float2Short_OPENCL_10CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eFloat, 1024, inStep, 10);
	Audiobuffer outputbuffer(eShort, 1024, outStep, 10);
	inputbuffer.fillbufferforconverter(overflow);

	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_FLOAT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_SHORT, samplesToProcess, gain, inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}
DEFINE_BASIC_UNIT_TEST(Converter_Short2Float_OPENCL_10CH, L"Convert short to float with OPENCL buffer (Batch 10 Channel)");
bool Converter_Short2Float_OPENCL_10CH::OnRun(UnitTestParamBlob* const params)
{
	ConverterTestSuitsParam* real_parameters = (ConverterTestSuitsParam*)params;
	bool bError = false;
	amf_uint overflowTemp = random(2);
	const eOverFlowType overflow =
		(overflowTemp == 0) ? eNone :
		(overflowTemp == 1 ? ePositive : eNegative);
	const float gain =
		(overflow == ePositive || overflow == eNegative) ? 1 :
		(1 + random(9)) / 100.f;
	const amf_uint outStep = 1 + random(5);
	const amf_uint inStep = outStep + random(5);
	const amf_uint samplesToProcess = (((1024 - 1) / inStep) + 1);
	Audiobuffer inputbuffer(eShort, 1024, inStep, 10);
	Audiobuffer outputbuffer(eFloat, 1024, outStep, 10);
	inputbuffer.fillbufferforconverter(overflow);

	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());

	AMF_RESULT res = real_parameters->m_pConverter->Convert(inputbuffer.getCLBuffer(), inputbuffer.m_iStep, inputbuffer.m_iOffset,
		amf::TAN_SAMPLE_TYPE_SHORT, outputbuffer.getCLBuffer(), outputbuffer.m_iStep, outputbuffer.m_iOffset, amf::TAN_SAMPLE_TYPE_FLOAT, samplesToProcess, gain, inputbuffer.m_iNumOfChannel);

	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	if (res == AMF_TAN_CLIPPING_WAS_REQUIRED && !overflow)
	{
		// Inaccurate value but conversion ran to completion
		bError = true;
	}
	// Reset to AMF_OK if clipping occurred to continue with test
	res = (res == AMF_TAN_CLIPPING_WAS_REQUIRED) ? AMF_OK : res;
	RETURN_FALSE_IF_FAILED(res);
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferConverterResult(outputbuffer, gain));
	return true;
}



ConverterTestSuites::ConverterTestSuites(const std::wstring& suiteName, eTestMode mode) : UnitTestSuite(suiteName)
{

		
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = mode;
	this->TrapSegFault(false);
	RegisterTests();
}

ConverterTestSuites::ConverterTestSuites(const std::wstring& suiteName, ConverterTestSuitsParam* param) : UnitTestSuite(suiteName)
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = param->m_eTestMode;
	this->TrapSegFault(false);
	m_pParam = param;
}

void ConverterTestSuites::RegisterTests()
{
	REGISTER_UNIT_TEST((*this), Converter_Float2Short_Native, false);
	REGISTER_UNIT_TEST((*this), Converter_Short2Float_Native, false);
	REGISTER_UNIT_TEST((*this), Converter_Float2Short_Batch_Native_1CH, false);
	REGISTER_UNIT_TEST((*this), Converter_Short2Float_Batch_Native_1CH, false);
	REGISTER_UNIT_TEST((*this), Converter_Float2Short_Batch_Native_2CH, false);
	REGISTER_UNIT_TEST((*this), Converter_Short2Float_Batch_Native_2CH, false);
	REGISTER_UNIT_TEST((*this), Converter_Float2Short_Batch_Native_10CH, false);
	REGISTER_UNIT_TEST((*this), Converter_Short2Float_Batch_Native_10CH, false);
	if (m_testmode == eGPU)
	{
		REGISTER_UNIT_TEST((*this), Converter_Float2Short_OPENCL, false);
		REGISTER_UNIT_TEST((*this), Converter_Short2Float_OPENCL, false);
		REGISTER_UNIT_TEST((*this), Converter_Float2Short_OPENCL_1CH, false);
		REGISTER_UNIT_TEST((*this), Converter_Short2Float_OPENCL_1CH, false);
		REGISTER_UNIT_TEST((*this), Converter_Float2Short_OPENCL_2CH, false);
		REGISTER_UNIT_TEST((*this), Converter_Short2Float_OPENCL_2CH, false);
		REGISTER_UNIT_TEST((*this), Converter_Float2Short_OPENCL_10CH, false);
		REGISTER_UNIT_TEST((*this), Converter_Short2Float_OPENCL_10CH, false);
	}
}

UnitTestParamBlob* ConverterTestSuites::OnPrepare()
{
	// Initialize TAN Context and Converter
	ConverterTestSuitsParam* param;
	if (m_pParam == nullptr)
	{
		param = new ConverterTestSuitsParam(m_testmode);
	}
	else
	{
		param = m_pParam;
	}
	AMF_RESULT result = param->InitializeConverter();
	if (result == AMF_OK)
	{
		return param;
	}
	else
	{
		FailPrepare();
        return NULL;
	}
}

void ConverterTestSuites::ParseCommandLine(int argc, wchar_t* argv[])
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->processCommandArgs(argc, argv);
}
