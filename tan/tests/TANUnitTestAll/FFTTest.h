#include "common/UnitTest.h"
#include "TANTestSuitsParam.h"
class FFTTestSuites : public UnitTestSuite
{
public:
	FFTTestSuites(const std::wstring& suiteName, eTestMode mode);
	FFTTestSuites(const std::wstring& suiteName, FFTTestSuitsParam* param);
	UnitTestParamBlob* OnPrepare();
	void RegisterTests();
	void ParseCommandLine(int argc, wchar_t* argv[]);
private:
	eTestMode m_testmode;
	FFTTestSuitsParam* m_pParam = nullptr;
};