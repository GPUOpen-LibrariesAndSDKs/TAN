// TANUnitTestNew.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestList.h"
#include "common/UnitTest.h"

int _tmain(int argc, wchar_t* argv[])
{
	AnyOption parsing;
	parsing.addUsage(L"True Audio Next Unit Test");
	parsing.addUsage(L"Usage: ");
	parsing.addUsage(L"-converter:		Run the converter test");	
	parsing.addUsage(L"-FFT:			Run the FFT test");
	parsing.addUsage(L"-convolution:	Run the convolution test");
	parsing.addUsage(L"-math:			Run the math test");
	parsing.addUsage(L"-a:				Run all test in the module specified(FFT,convolution, etc). ");
	parsing.addUsage(L"Example: -converter -a : Run all the test in converter test module");
	parsing.noPOSIX();
	parsing.setCommandPrefixChar('-');
	parsing.setFlag(L"converter");
	parsing.setFlag(L"FFT");
	parsing.setFlag(L"convolution");
	parsing.setFlag(L"math");
	parsing.setFlag(L"help");
	parsing.processCommandArgs(argc, argv);
	if (parsing.getFlag(L"converter"))
	{
		converter_test_suits_CPU.ParseCommandLine(argc, argv);
		converter_test_suits_GPU.ParseCommandLine(argc, argv);
		converter_test_suits_CPU.Execute();
		converter_test_suits_GPU.Execute();
	}
	if (parsing.getFlag(L"FFT"))
	{
		FFT_test_suits_CPU.ParseCommandLine(argc, argv);
		FFT_test_suits_GPU.ParseCommandLine(argc, argv);
		FFT_test_suits_CPU.Execute();
		FFT_test_suits_GPU.Execute();
	}
	if (parsing.getFlag(L"convolution"))
	{
		Convolution_testsuits_CPU_1.ParseCommandLine(argc, argv);
		Convolution_testsuits_GPU_1.ParseCommandLine(argc, argv);
		Convolution_testsuits_CPU_1.Execute();
		Convolution_testsuits_GPU_1.Execute();
	}
	if (parsing.getFlag(L"math"))
	{
		MATH_testsuits_CPU.ParseCommandLine(argc, argv);
		MATH_testsuits_GPU.ParseCommandLine(argc, argv);
		MATH_testsuits_CPU.Execute();
		MATH_testsuits_GPU.Execute();
	}
	if (parsing.getFlag(L"help"))
	{
		parsing.printUsage();
	}
}

