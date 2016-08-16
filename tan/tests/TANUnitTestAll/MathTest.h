#include "common/UnitTest.h"
#include "TANTestSuitsParam.h"
class MATHTestSuites : public UnitTestSuite
{
public:
	MATHTestSuites(const std::wstring& suiteName, eTestMode mode);
	MATHTestSuites(const std::wstring& suiteName, MathTestSuitesParam* param);
	UnitTestParamBlob* OnPrepare() override;
	void RegisterTests();
	void ParseCommandLine(int argc, wchar_t* argv[]);
private:
	eTestMode m_testmode;
	MathTestSuitesParam* m_pParam = nullptr;
};