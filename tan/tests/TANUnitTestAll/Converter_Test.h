#include "common/UnitTest.h"
#include "TANTestSuitsParam.h"
class ConverterTestSuites : public UnitTestSuite
{
public:
	ConverterTestSuites(const std::wstring& suiteName, eTestMode mode);
	ConverterTestSuites(const std::wstring& suiteName, ConverterTestSuitsParam* param);
	void RegisterTests();
	UnitTestParamBlob* OnPrepare();
	void ParseCommandLine(int argc, wchar_t* argv[]);
private:
	eTestMode m_testmode;
	ConverterTestSuitsParam* m_pParam = nullptr;
};


