#pragma once
#include "common/UnitTest.h"
#include "TANTestSuitsParam.h"
class ConvolutionTestSuites :
	public UnitTestSuite
{
public:
	ConvolutionTestSuites(const std::wstring& suiteName, eTestMode mode);
	ConvolutionTestSuites(const std::wstring& suiteName, ConvolutionTestSuitesParam* param);
	UnitTestParamBlob* OnPrepare() override;
	void RegisterTests();
	void ParseCommandLine(int argc, wchar_t* argv[]);
private:
	eTestMode m_testmode;
	ConvolutionTestSuitesParam* m_pParam = nullptr;
};

