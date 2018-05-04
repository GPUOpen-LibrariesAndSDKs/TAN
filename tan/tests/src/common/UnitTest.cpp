#include "UnitTest.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <sstream>
#include <ios>

#ifdef _WIN32

static long long GetHighPrecisionClock()
{
    static int state = 0;
    static LARGE_INTEGER Frequency;
    static LARGE_INTEGER StartCount;
    if (state == 0)
    {
        if (QueryPerformanceFrequency(&Frequency))
        {
            state = 1;
            QueryPerformanceCounter(&StartCount);
        }
        else
        {
            state = 2;
        }
    }
    if (state == 1)
    {
        LARGE_INTEGER PerformanceCount;
        if (QueryPerformanceCounter(&PerformanceCount))
        {
            return static_cast<long long>((PerformanceCount.QuadPart - StartCount.QuadPart) * 10000000LL / Frequency.QuadPart);
        }
    }
#if defined(METRO_APP)
    return GetTickCount64() * 10;

#else
    return GetTickCount() * 10;

#endif
}
#else   //  Linux implementation should go here
static long long GetHighPrecisionClock()
{
    return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum LogType
{
    LogTypeInfo,
    LogTypeSuccess,
    LogTypeError
};

static void ChangeTextColor(LogType type)
{
#if !defined(METRO_APP)
    HANDLE hCmd = GetStdHandle(STD_OUTPUT_HANDLE);

    switch (type)
    {
    case LogTypeInfo:
        SetConsoleTextAttribute(hCmd, FOREGROUND_INTENSITY);
        break;
    case LogTypeSuccess:
        SetConsoleTextAttribute(hCmd, FOREGROUND_GREEN);
        break;
    case LogTypeError:
        SetConsoleTextAttribute(hCmd, FOREGROUND_RED);
        break;
    }
#endif
}


static void WriteLog(const wchar_t* message, LogType type)
{

    ChangeTextColor(type);
    wprintf(message);
    ChangeTextColor(LogTypeInfo);
}

#define LOG_WRITE(a, type)\
    do \
    { \
        std::wstringstream messageStream12345;\
        messageStream12345 << a;\
        WriteLog(messageStream12345.str().c_str(), type);\
    } while(0)

#define LOG_INFO(a) LOG_WRITE(a << std::endl, LogTypeInfo)
#define LOG_SUCCESS(a) LOG_WRITE(a << std::endl, LogTypeSuccess)
#define LOG_ERROR(a) LOG_WRITE(a << std::endl, LogTypeError)

#ifdef _DEBUG
#define LOG_DEBUG(a)     LOG_INFO(a)
#else
#define LOG_DEBUG(a)
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

UnitTest::UnitTest(const std::wstring& testName) :
    m_TestName(testName)
{

}

UnitTest::~UnitTest()
{
}

const std::wstring& UnitTest::GetName() const
{
    return m_TestName;
}


bool UnitTest::OnPrepare(UnitTestParamBlob* const params)
{
    return true;
}

bool UnitTest::OnShutdown(UnitTestParamBlob* const params)
{
    return true;
}

long long UnitTest::GetHighPrecisionTime()
{
    return GetHighPrecisionClock();
}

//////////////////////////////////////////////////////////////////////////////////////////////
UnitTestSuite::UnitTestSuite(const std::wstring& suiteName) :
    m_Name(suiteName),
    m_TrapSegFaults(true),
    m_TrapIllegalInstructions(true),
    m_TrapFloatingPoint(true),
    m_TrapAbort(true),
    m_PrepareSuccess(false)
{
    m_AnyOptions = AnyOptionsPtr(new AnyOption());
}

UnitTestSuite::~UnitTestSuite()
{
}

void UnitTestSuite::AddTest(UnitTest::Ptr&& test, bool skipIfPrevFailed)
{
    // Add all available test case names to command line parser.
    const wchar_t* testName = test->GetName().c_str();
    m_AnyOptions->setFlag(testName);

    // Add all test cases in test suite.
    m_Tests.push_back(TestWithFlag(std::move(test), skipIfPrevFailed));
}

typedef void(*SignalHandlerPointer)(int);
static volatile int lastSignal = 0;
static jmp_buf jmpMark;
static void SignalHandler(int sig)
{
    lastSignal = sig;
    signal(sig, SignalHandler); //  Reset itself
    longjmp(jmpMark, -1);
}

enum Stage
{
    STAGE_PREPARE,
    STAGE_RUN,
    STAGE_SHUTDOWN
};

std::wstring GetStageName(Stage stage)
{
    std::wstring stgStr;
    switch (stage)
    {
    case STAGE_PREPARE:
        stgStr = L"Prepare";
        break;
    case STAGE_RUN:
        stgStr = L"Run";
        break;
    case STAGE_SHUTDOWN:
        stgStr = L"Shutdown";
        break;
    }
    return stgStr;
}

std::wstring GetSignalDescription(int sig)
{
    std::wstring sigStr;
    switch (sig)
    {
    case SIGABRT:
        sigStr += L"SIGABRT (Abnormal termination)";
        break;
    case SIGFPE:
        sigStr += L"SIGFPE (Floating point exception)";
        break;
    case SIGILL:
        sigStr += L"SIGILL (Illegal instruction)";
        break;
    case SIGSEGV:
        sigStr += L"SIGSEGV (Segmentation fault)";
        break;
    case SIGINT:
        sigStr += L"SIGINT (Ctrl+C was pressed)";
        break;
    case SIGTERM:
        sigStr += L"SIGTERM (Termination request)";
        break;
    }

    return sigStr;
}


std::wstring UnitTestSuite::GetFullTestName(const UnitTest& unitTest) const
{
    std::wstring fullName;
    fullName = m_Name + L"::" + unitTest.GetName();
    return fullName;
}

int UnitTestSuite::Execute(bool continueExecution)
{
    bool testResult = false;
    int passCount = 0;
    int skippedCount = 0;
    SignalHandlerPointer sigFpe = SIG_DFL, sigIll = SIG_DFL, sigSegV = SIG_DFL, sigAbrt = SIG_DFL;
    if (m_TrapFloatingPoint == true)
    {
        sigFpe = signal(SIGFPE, SignalHandler);
    }
    if (m_TrapIllegalInstructions == true)
    {
        sigIll = signal(SIGILL, SignalHandler);
    }
    if (m_TrapSegFaults == true)
    {
        sigSegV = signal(SIGSEGV, SignalHandler);
    }
    if (m_TrapAbort)
    {
        sigAbrt = signal(SIGABRT, SignalHandler);
    }
    if (Prepare() == true)
    {
        bool prevTestSucceeded = true;
        testResult = true;
        for (TestList::iterator it = m_Tests.begin(); it != m_Tests.end() && (testResult == true || continueExecution == true); ++it)
        {
            UnitTest* curTest = it->first.get();
            if (curTest != nullptr)
            {
                std::wstring msg;
                std::wstring testName = curTest->GetName();
                std::wstring fullTestName = GetFullTestName(*curTest);
                if (prevTestSucceeded == false && it->second == true)
                {
                    msg += L"Skipping test \'" + fullTestName + L"\' because one of the previous tests it depends on failed.";
                    LOG_INFO(msg);
                    ++skippedCount;
                }
                // Ignore this case if user wants to run all cases.
                // If not running all cases, then check if user chose to run this test.
                else if (m_AnyOptions->getFlag(L'a') == false && m_AnyOptions->getFlag(testName.c_str()) == false)
                {
                    //msg += L"Skipping test \'" + fullTestName + L"\' because this test was not specified for testing.";
                    //LOG_DEBUG(msg);
                    ++skippedCount;
                }
                // Execute test only if the test is specified by user
                // Or no specified tests implies that running all tests.
                else 
                {
                    msg += L"Running test \'" + fullTestName + L"\'... ";
                    LOG_INFO(msg);

                    Stage stage;

                    testResult = true;
                    int jmpRet = setjmp(jmpMark);
                    if (jmpRet == 0)    //  We will return here if a signal has been trapped, but jmpRet will be set to -1
                    {
                        long long startTime;
                        long long endTime;
                        try
                        {
                            stage = STAGE_PREPARE;
                            bool prepared = false;
                            if ((prepared = curTest->OnPrepare(m_Params.get())) != true)
                            {
                                LOG_INFO(L"FAILED (in Prepare)");
                                testResult = false;
                            }
                            stage = STAGE_RUN;
#ifdef _DEBUG
                            _CrtMemState    memStateBefore;
                            _CrtMemCheckpoint(&memStateBefore);
#endif
                            startTime = GetHighPrecisionClock();
                            if (prepared == true && curTest->OnRun(m_Params.get()) != true)
                            {
                                LOG_ERROR(L"FAILED");
                                testResult = false;
                                prevTestSucceeded = false;
                            }
                            else
                            {
                                endTime = GetHighPrecisionClock();
                            }
#ifdef _DEBUG
                            _CrtMemState    memStateAfter;
                            _CrtMemCheckpoint(&memStateAfter);
                            _CrtMemState    memDifference;
                            if (_CrtMemDifference(&memDifference, &memStateBefore, &memStateAfter) == TRUE)
                            {
                                LOG_INFO(L"FAILED (memory leaks detected)");
                                testResult = false;
                            }
#endif
                            stage = STAGE_SHUTDOWN;
                            if (prepared == true && curTest->OnShutdown(m_Params.get()) != true)
                            {
                                LOG_INFO(L"FAILED (in Shutdown)");
                                testResult = false;
                            }
                        }
                        catch (...)
                        {
                            std::wstring msg;
                            msg += L"FAILED - uncaught exception during the " + GetStageName(stage) + L" stage.";
                            LOG_INFO(msg);
                            testResult = false;
                        }
                        if (testResult == true)
                        {
                            ++passCount;
                            testResult = true;
                            std::wstringstream str;
                            str << L"PASSED, execution time is " << static_cast<double>(endTime - startTime) / 10000 << L"ms";
                            LOG_SUCCESS(str.str());
                        }
                    }
                    else    //  Signal trapped
                    {
                        std::wstring msg;
                        msg += L"FAILED - triggered an uncaught signal " +
                            GetSignalDescription(lastSignal) + L" during the " + GetStageName(stage) + L" stage.";
                        LOG_INFO(msg);
                        testResult = false;
                    }
                }
            }
            else    //  Invalid test case - curTest is nullptr
            {
                std::wstring msg;
                msg += L"Invalid test in suite \'" + m_Name + L"\' FAILED";
                LOG_INFO(msg);
                testResult = false;
            }
        }
        Shutdown();
        m_Params = nullptr;
    }

    if (m_TrapFloatingPoint == true)
    {
        sigFpe = signal(SIGFPE, sigFpe);
    }
    if (m_TrapIllegalInstructions == true)
    {
        sigIll = signal(SIGILL, sigIll);
    }
    if (m_TrapSegFaults == true)
    {
        sigSegV = signal(SIGSEGV, sigSegV);
    }
    if (m_TrapAbort == true)
    {
        sigAbrt = signal(SIGABRT, sigAbrt);
    }

    if (passCount + skippedCount == m_Tests.size())
    {
        std::wstringstream msg;
        msg << L"Test suite \'" << m_Name << L"\' PASSED";
        LOG_SUCCESS(msg.str());
    }
    else
    {
        if (continueExecution == false)
        {
            skippedCount += (int)m_Tests.size() - passCount - 1;
        }
        std::wstringstream msg;
        msg << L"Test suite \'" << m_Name << L"\' FAILED (passed " << passCount << L", failed " << m_Tests.size() - passCount - skippedCount << L", skipped " << skippedCount << L" of total " << m_Tests.size() << L" tests)";
        LOG_ERROR(msg.str());
    }
    return passCount - static_cast<int>(m_Tests.size());
}

UnitTestParamBlob* UnitTestSuite::OnPrepare()
{
    return nullptr;
}

bool UnitTestSuite::OnShutdown(UnitTestParamBlob* const params)
{
    return true;
}

bool UnitTestSuite::Prepare()
{
    bool result = true;
    int jmpRet = setjmp(jmpMark);
    if (jmpRet == 0)
    {
        m_PrepareSuccess = true;
        m_Params = UnitTestParamBlob::Ptr(OnPrepare());
        result = m_PrepareSuccess;
        if (result != true)
        {
            std::wstring msg;
            msg += L"Test suite \'" + m_Name + L"\' FAILED because OnPrepare() signalled a failure";
            LOG_ERROR(msg);
        }
    }
    else
    {
        std::wstring msg;
        msg += L"Test suite \'" + m_Name + L"\' FAILED because it has triggered an uncaught signal " +
            GetSignalDescription(lastSignal) + L" in Prepare stage.";
        LOG_ERROR(msg);
        result = false;
    }
    return result;
}

bool UnitTestSuite::Shutdown()
{
    bool result = true;
    int jmpRet = setjmp(jmpMark);
    if (jmpRet == 0)
    {
        OnShutdown(m_Params.get());
    }
    else
    {
        std::wstring msg;
        msg += L"Test suite \'" + m_Name + L"\' FAILED because it has triggered an uncaught signal " +
            GetSignalDescription(lastSignal) + L" in Shutdown stage.";
        LOG_ERROR(msg);
        result = false;
    }
    return result;
}
