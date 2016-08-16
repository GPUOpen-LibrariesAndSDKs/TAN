#include "MathTest.h"
#include <iostream>
#include "AudioBuffer.h"
DEFINE_BASIC_UNIT_TEST(MATH_Native_Buffer_Multiplication, L"Run Multiplication on Native Buffer");
bool MATH_Native_Buffer_Multiplication::OnRun(UnitTestParamBlob* const params)
{
	MathTestSuitesParam* real_params = (MathTestSuitesParam*)params;
	Audiobuffer input1(eFloat, 1024);
	Audiobuffer input2(eFloat, 1024);
	Audiobuffer output(eFloat, 1024);
	float input1float = random_float(5.0);
	float input2float = random_float(5.0);
	float expectresult = 2 * input1float*input2float;
	input1.fillfloatinbuffer(input1float);
	input2.fillfloatinbuffer(input2float);
	int error_count = 0;
	RETURN_IF_FAILED(real_params->m_pMath->ComplexMultiplication(input1.getFloatTypeBuffer(), input2.getFloatTypeBuffer(),
		output.getFloatTypeBuffer(), input1.m_iNumOfChannel, input1.m_iSize));
	for (amf_uint i = 0; i < output.m_iNumOfChannel; i++)
	{
		float* datain = output.getFloatTypeBuffer()[i];
		for (amf_uint j = 0; j < output.m_iSize; j++)	
		{
			if ((j % 2 == 1 && fabs(datain[j] - expectresult)>FLT_EPSILON) || (j % 2 == 0 && datain[j] != 0))
			{
				std::cout << "Error in item " << j << ", channel " << i << ", Output: " << datain[i] << ", expected: " << expectresult << std::endl;
				error_count++;
			}
		}
	}
	if (!error_count)
		return true;
	else
	{
		return false;
	}
}
DEFINE_BASIC_UNIT_TEST(MATH_OpenCL_Buffer_Multiplication, L"Run Multiplication on Native Buffer");
bool MATH_OpenCL_Buffer_Multiplication::OnRun(UnitTestParamBlob* const params)
{
	MathTestSuitesParam* real_params = (MathTestSuitesParam*)params;
	Audiobuffer input1(eFloat, 1024);
	Audiobuffer input2(eFloat, 1024);
	Audiobuffer output(eFloat, 1024);
	float input1float = random_float(5.0);
	float input2float = random_float(5.0);
	float expectresult = 2 * input1float*input2float;
	input1.fillfloatinbuffer(input1float);
	input2.fillfloatinbuffer(input2float);
	
	input1.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());
	input2.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());
	output.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());
	
	int error_count = 0;
	RETURN_IF_FAILED(real_params->m_pMath->ComplexMultiplication(input1.getCLBuffer(), input1.m_iOffset,input2.getCLBuffer(),input2.m_iOffset,
		output.getCLBuffer(),output.m_iOffset,input1.m_iNumOfChannel, input1.m_iSize));
	output.copyfromCLBuffer(real_params->m_pContext->GetOpenCLGeneralQueue());
	for (amf_uint i = 0; i < output.m_iNumOfChannel; i++)
	{
		float* datain = output.getFloatTypeBuffer()[i];
		for (amf_uint j = 0; j < output.m_iSize; j++)
		{
			if ((j % 2 == 1 && fabs(datain[j] - expectresult)>FLT_EPSILON) || (j % 2 == 0 && datain[j] != 0))
			{
				std::cout << "Error in item " << j << ", channel " << i << ", Output: " << datain[i] << ", expected: " << expectresult << std::endl;
				error_count++;
			}
		}
	}
	if (!error_count)
		return true;
	else
	{
		return false;
	}
}
DEFINE_BASIC_UNIT_TEST(MATH_Native_Buffer_Division, L"Run Division on OpenCL Buffer");
bool MATH_Native_Buffer_Division::OnRun(UnitTestParamBlob* const params)
{
	MathTestSuitesParam* real_params = (MathTestSuitesParam*)params;
	Audiobuffer input1(eFloat, 1024);
	Audiobuffer input2(eFloat, 1024);
	Audiobuffer output(eFloat, 1024);
	float input1float = random_float(5.0);
	float input2float = random_float(5.0);
	float expectresult = 2 * input1float*input2float / (2 * input2float*input2float);
	input1.fillfloatinbuffer(input1float);
	input2.fillfloatinbuffer(input2float);
	int error_count = 0;
	RETURN_IF_FAILED(real_params->m_pMath->ComplexDivision(input1.getFloatTypeBuffer(), input2.getFloatTypeBuffer(),
		output.getFloatTypeBuffer(), input1.m_iNumOfChannel, input1.m_iSize));
	for (amf_uint i = 0; i < output.m_iNumOfChannel; i++)
	{
		float* datain = output.getFloatTypeBuffer()[i];
		for (amf_uint j = 0; j < output.m_iSize; j++)
		{
			if ((j % 2 == 0 && fabs(datain[j] - expectresult)> 4 * FLT_EPSILON) || (j % 2 == 1 && datain[j] != 0))
			{
				std::cout << "Error in item " << j << ", channel " << i << ", Output: " << datain[i] << ", expected: " << expectresult << std::endl;
				error_count++;
			}
		}
	}
	if (!error_count)
		return true;
	else
	{
		return false;
	}
}
DEFINE_BASIC_UNIT_TEST(MATH_OpenCL_Buffer_Division, L"Run Division on OpenCL Buffer");
bool MATH_OpenCL_Buffer_Division::OnRun(UnitTestParamBlob* const params)
{
	MathTestSuitesParam* real_params = (MathTestSuitesParam*)params;
	Audiobuffer input1(eFloat, 1024);
	Audiobuffer input2(eFloat, 1024);
	Audiobuffer output(eFloat, 1024);
	float input1float = random_float(5.0);
	float input2float = random_float(5.0);
	float expectresult = 2 * input1float*input2float / (2 * input2float*input2float);
	input1.fillfloatinbuffer(input1float);
	input2.fillfloatinbuffer(input2float);

	input1.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());
	input2.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());
	output.copytoCLBuffer(real_params->m_pContext->GetOpenCLContext());

	int error_count = 0;
	RETURN_IF_FAILED(real_params->m_pMath->ComplexDivision(input1.getCLBuffer(), input1.m_iOffset, input2.getCLBuffer(), input2.m_iOffset,
		output.getCLBuffer(), output.m_iOffset, input1.m_iNumOfChannel, input1.m_iSize));
	output.copyfromCLBuffer(real_params->m_pContext->GetOpenCLGeneralQueue());
	for (amf_uint i = 0; i < output.m_iNumOfChannel; i++)
	{
		float* datain = output.getFloatTypeBuffer()[i];
		for (amf_uint j = 0; j < output.m_iSize; j++)
		{
			if ((j % 2 == 0 && fabs(datain[j] - expectresult)> 4 * FLT_EPSILON) || (j % 2 == 1 && datain[j] != 0))
			{
				std::cout << "Error in item " << j << ", channel " << i << ", Output: " << datain[i] << ", expected: " << expectresult << std::endl;
				error_count++;
			}
		}
	}
	if (!error_count)
		return true;
	else
	{
		return false;
	}
}

MATHTestSuites::MATHTestSuites(const std::wstring& suiteName, eTestMode mode) : UnitTestSuite(suiteName)
{

	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = mode;
	this->TrapSegFault(false);
	RegisterTests();
}

MATHTestSuites::MATHTestSuites(const std::wstring& suiteName, MathTestSuitesParam* param) : UnitTestSuite(suiteName)
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->setFlag(L'a');
	m_testmode = param->m_eTestMode;
	RegisterTests();
	this->TrapSegFault(false);

}

UnitTestParamBlob* MATHTestSuites::OnPrepare()
{
	MathTestSuitesParam* param;
	if (m_pParam == nullptr)
	{
		param = new MathTestSuitesParam(m_testmode);
	}
	else
	{
		param = m_pParam;
	}
	AMF_RESULT result = param->InitializeMath();
	if (result == AMF_OK)
	{
		return param;
	}
	else
	{
		std::cout << "Failed to initialize TANMATH" << std::endl;
		FailPrepare();
		return nullptr;
	}
}

void MATHTestSuites::RegisterTests()
{
	REGISTER_UNIT_TEST((*this),MATH_Native_Buffer_Multiplication,false);
	REGISTER_UNIT_TEST((*this), MATH_Native_Buffer_Division, false);
	if (m_testmode == eGPU)
	{
		REGISTER_UNIT_TEST((*this), MATH_OpenCL_Buffer_Multiplication, false);
		REGISTER_UNIT_TEST((*this), MATH_OpenCL_Buffer_Division, false);
	}
}


void MATHTestSuites::ParseCommandLine(int argc, wchar_t* argv[])
{
	AnyOptionsPtr ptr = this->getCmdOptions();
	ptr->processCommandArgs(argc, argv);
}
