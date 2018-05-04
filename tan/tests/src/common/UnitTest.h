#pragma once

#include <string>
#include <list>
#include <memory>
#include "anyoption.h"

/*
    Unit Test Framework

    Usage:

      Each unit test is supposed to test a specific function/class/interface. Each unit test is represented by a class derived from UnitTest. 
      Class UnitTest cannot be instantiated directly and must be derived from.

      Individual unit tests are combined into a suite. While not required, it is recommended to have one suite per test executable. A test suite
      is represented by an instance of the UnitTestSuite class. The UnitTestSuite class can be instantiated directly.

      Each unit test consists of three phases: Prepare, Run and Shutdown. The Run phase is mandatory, Prepare and Shutdown are optional. In most 
      cases everything can be done in the Run phase, but sometimes it is convenient to separate the preparation and the clean-up tasks from the
      actual test code. To implement each of the phases, override the OnPrepare(), OnRun() and OnShutdown() methods in your unit test class derived
      from UnitTest. These methods must return true in case of a pass and false in case of a failure. Default implementations of OnPrepare and OnShutdown
      do nothing and return true.

      You can use the DEFINE_BASIC_UNIT_TEST(className, testReadableName) macro to define a basic unit test class with only the Run phase implemented as
      follows:

      DEFINE_BASIC_UNIT_TEST(MyUnitTest, L"My Unit Test #1");
      bool MyUnitTest::OnRun(UnitTestParamBlob* const params)
      {
          //    Write your test code here

          return true;  //  Return true if test passed, otherwise return false
      }

      A simple test suite can be created by instantiating the UnitTestSuite class and adding unit tests to it using the REGISTER_UNIT_TEST macro as follows:

      UnitTestSuite suite(L"My Test Suite");
      REGISTER_UNIT_TEST(suite, MyUnitTest, true);

      The third parameter of the REGISTER_UNIT_TEST macro indicates whether this test relies on successful passing of all previous tests. When set to false,
      this test will run regardless of the result of any previous test, when true, the test will be skipped if any of the previous tests has failed.

      To execute all tests in the suite just call the UnitTestSuite::Execute() method. The return value is the number of failed unit tests. This value should
      typically be used as return value for main(). UnitTestSuite::Execute() will print all of the pass/fail statistics.

      The parameter passed to Execute indicates whether the suite should continue to run tests after a test failure. When set to false, the suite will abort after 
      the first failure.

      UnitTestSuite::Execute() traps all of the system level exceptions, such as access violations, division by zero and so on. It also will catch all C++ exceptions
      with catch (...) and report all of these conditions as failures, preventing the test application from crashing.

      UnitTestSuite also has the optional Prepare and Shutdown phases. The default implementations do nothing. To implement them, override the OnPrepare and OnShutdown
      methods in the derived class.

      OnPrepare allows to allocate a parameters blob which will be passed to every unit test in the suite. This blob may contain anything that is required for the test.
      To create a parameter blob, derive a class from UnitTestParamBlob, implement whatever you need there and simply instantiate it with the new operator in 
      UnitTestSuite::OnPrepare(), returning a naked C++ pointer to it. This pointer will be passed to OnPrepare(), OnRun and OnShutdown() methods of every unit test, as well 
      as to UnitTestSuite::OnShutdown(). DO NOT delete this object - it will be done by the framework, but its destructor is resonsible for cleaning up whatever the object 
      itself has allocated.
*/

/*
    This is a base class for optional parameter blob that is shared across all of the unit tests in a suite
*/

class UnitTestParamBlob
{
public:
    typedef std::unique_ptr<UnitTestParamBlob>  Ptr;
protected:
    UnitTestParamBlob() {}

public:
    virtual ~UnitTestParamBlob() {}
};

class UnitTestSuite;
class UnitTest
{
    friend class UnitTestSuite;
public:
    typedef std::unique_ptr<UnitTest>   Ptr;

protected:
    UnitTest(const std::wstring& testName);

    virtual bool OnPrepare(UnitTestParamBlob* const params);
    virtual bool OnRun(UnitTestParamBlob* const params) = 0;
    virtual bool OnShutdown(UnitTestParamBlob* const params);

public:
    virtual ~UnitTest();

    const std::wstring& GetName() const;
	void setName(const wchar_t* _str)
	{
		m_TestName = std::wstring(_str);
	}
    static long long GetHighPrecisionTime();
private:
    std::wstring    m_TestName;
};

class UnitTestSuite
{
public:
    UnitTestSuite(const std::wstring& suiteName);
    virtual ~UnitTestSuite();

    void AddTest(UnitTest::Ptr&& test, bool skipIfPrevFailed = false);

    int Execute(bool continueExecution = true);

    inline void TrapSegFault(bool trap) { m_TrapSegFaults = trap; }
    inline void TrapIllegalInstructions(bool trap) { m_TrapIllegalInstructions = trap; }
    inline void TrapFloatingPointExceptions(bool trap) { m_TrapFloatingPoint = trap; }
    
    typedef std::shared_ptr<AnyOption> AnyOptionsPtr;
    AnyOptionsPtr& getCmdOptions(){ return m_AnyOptions; };

protected:
    virtual UnitTestParamBlob* OnPrepare();
    virtual bool OnShutdown(UnitTestParamBlob* const params);

    inline void FailPrepare() { m_PrepareSuccess = false; }

private:
    bool Prepare();
    bool Shutdown();

    std::wstring GetFullTestName(const UnitTest& unitTest) const;
private:
    AnyOptionsPtr           m_AnyOptions;

    typedef std::pair<UnitTest::Ptr, bool>  TestWithFlag;
    typedef std::list<TestWithFlag>    TestList;
    TestList                m_Tests;
    std::wstring            m_Name;
    UnitTestParamBlob::Ptr  m_Params;
    bool                    m_TrapSegFaults;
    bool                    m_TrapIllegalInstructions;
    bool                    m_TrapFloatingPoint;
    bool                    m_TrapAbort;
    bool                    m_PrepareSuccess;
};

#define DEFINE_BASIC_UNIT_TEST(className, testReadableName) class className : public UnitTest \
{ \
public: \
    className() : UnitTest(testReadableName) {} \
    virtual bool OnRun(UnitTestParamBlob* const params); \
}

#define REGISTER_UNIT_TEST(suite, test, dependencies) (suite.AddTest(std::move(test::Ptr(new test)), dependencies))