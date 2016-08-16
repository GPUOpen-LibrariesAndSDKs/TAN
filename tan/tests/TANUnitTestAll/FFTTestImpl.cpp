#include "FFTTest.h"
#include "AudioBuffer.h"

DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_256_1CH, L"Run FFT On Native Buffer: Length 256 (1CH)");
bool FFT_Native_Buffer_256_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 8;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_256_2CH, L"Run FFT On Native Buffer: Length 256 (2CH)");
bool FFT_Native_Buffer_256_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 8;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_512_1CH, L"Run FFT On Native Buffer: Length 512 (1CH)");
bool FFT_Native_Buffer_512_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 9;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_512_2CH, L"Run FFT On Native Buffer: Length 512 (2CH)");
bool FFT_Native_Buffer_512_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 9;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_1024_1CH, L"Run FFT On Native Buffer: Length 1024 (1CH)");
bool FFT_Native_Buffer_1024_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 10;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_1024_2CH, L"Run FFT On Native Buffer: Length 1024 (2CH)");
bool FFT_Native_Buffer_1024_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 10;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_2048_1CH, L"Run FFT On Native Buffer: Length 2048 (1CH)");
bool FFT_Native_Buffer_2048_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 11;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_2048_2CH, L"Run FFT On Native Buffer: Length 2048 (2CH)");
bool FFT_Native_Buffer_2048_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 11;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_4096_1CH, L"Run FFT On Native Buffer: Length 4096 (1CH)");
bool FFT_Native_Buffer_4096_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 12;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_Native_Buffer_4096_2CH, L"Run FFT On Native Buffer: Length 4096 (2CH)");
bool FFT_Native_Buffer_4096_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 12;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getFloatTypeBuffer(), medianbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getFloatTypeBuffer(), outputbuffer.getFloatTypeBuffer()));
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}

DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_256_1CH, L"Run FFT On OpenCL Buffer: Length 256 (1CH)");
bool FFT_OPENCL_Buffer_256_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 8;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_256_2CH, L"Run FFT On OpenCL Buffer: Length 256 (2CH)");
bool FFT_OPENCL_Buffer_256_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 8;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_512_1CH, L"Run FFT On OpenCL Buffer: Length 512 (1CH)");
bool FFT_OPENCL_Buffer_512_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 9;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_512_2CH, L"Run FFT On OpenCL Buffer: Length 512 (2CH)");
bool FFT_OPENCL_Buffer_512_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 9;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_1024_1CH, L"Run FFT On OpenCL Buffer: Length 1024 (1CH)");
bool FFT_OPENCL_Buffer_1024_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 10;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_1024_2CH, L"Run FFT On OpenCL Buffer: Length 1024 (2CH)");
bool FFT_OPENCL_Buffer_1024_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 10;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_2048_1CH, L"Run FFT On OpenCL Buffer: Length 2048 (1CH)");
bool FFT_OPENCL_Buffer_2048_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 11;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_2048_2CH, L"Run FFT On OpenCL Buffer: Length 2048 (2CH)");
bool FFT_OPENCL_Buffer_2048_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 11;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_4096_1CH, L"Run FFT On OpenCL Buffer: Length 4096 (1CH)");
bool FFT_OPENCL_Buffer_4096_1CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 12;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 1);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 1);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}
DEFINE_BASIC_UNIT_TEST(FFT_OPENCL_Buffer_4096_2CH, L"Run FFT On OpenCL Buffer: Length 4096 (2CH)");
bool FFT_OPENCL_Buffer_4096_2CH::OnRun(UnitTestParamBlob* const params)
{
	unsigned int log2lenth = 12;
	unsigned int length = 1 << log2lenth;
	FFTTestSuitsParam* real_parameters = (FFTTestSuitsParam*)params;
	Audiobuffer inputbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer medianbuffer(eFloat, length * 2, 1, 2);
	Audiobuffer outputbuffer(eFloat, length * 2, 1, 2);
	inputbuffer.fillbufferforFFT();
	inputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	medianbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	outputbuffer.copytoCLBuffer(real_parameters->m_pContext->GetOpenCLContext());
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_FORWARD, log2lenth, 1,
		inputbuffer.getCLBuffer(), medianbuffer.getCLBuffer()));
	RETURN_FALSE_IF_FAILED(real_parameters->m_pFFT->Transform(amf::TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, log2lenth, 1,
		medianbuffer.getCLBuffer(), outputbuffer.getCLBuffer()));
	outputbuffer.copyfromCLBuffer(real_parameters->m_pContext->GetOpenCLGeneralQueue());
	RETURN_FALSE_IF_FAILED(inputbuffer.verifyBufferFFTResult(outputbuffer, real_parameters->m_iAllowDiffInEpisilon));
	return true;
}

FFTTestSuites::FFTTestSuites(const std::wstring& suiteName, eTestMode mode) : UnitTestSuite(suiteName)
{
	
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = mode;
	this->TrapSegFault(false);
	RegisterTests();
}

FFTTestSuites::FFTTestSuites(const std::wstring& suiteName, FFTTestSuitsParam* param) : UnitTestSuite(suiteName)
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = param->m_eTestMode;
	RegisterTests();

}

UnitTestParamBlob* FFTTestSuites::OnPrepare()
{
	// Initialize TAN Context and FFT

	FFTTestSuitsParam* param;
	if (m_pParam == nullptr)
	{
		param = new FFTTestSuitsParam(m_testmode);
	}
	else
	{
		param = m_pParam;
	}
	AMF_RESULT result = param->InitializeFFT();
	if (m_testmode == eGPU)
	{
		param->m_iAllowDiffInEpisilon = 20;
	}
	else
	{
		param->m_iAllowDiffInEpisilon = 3;
	}
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

void FFTTestSuites::RegisterTests()
{
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_256_1CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_256_2CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_512_1CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_512_2CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_1024_1CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_1024_2CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_2048_1CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_2048_2CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_4096_1CH, false);
	REGISTER_UNIT_TEST((*this), FFT_Native_Buffer_4096_2CH, false);
	if (m_testmode == eGPU)
	{
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_256_1CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_256_2CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_512_1CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_512_2CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_1024_1CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_1024_2CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_2048_1CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_2048_2CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_4096_1CH, false);
		REGISTER_UNIT_TEST((*this), FFT_OPENCL_Buffer_4096_2CH, false);
	}
}

void FFTTestSuites::ParseCommandLine(int argc, wchar_t* argv[])
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->processCommandArgs(argc, argv);
}
