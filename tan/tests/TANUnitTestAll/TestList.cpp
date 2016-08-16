#include "TestList.h"
#include "TANTestSuitsParam.h"
ConverterTestSuites converter_test_suits_CPU(L"Converter CPU Test Suites", eCPU);
ConverterTestSuites converter_test_suits_GPU(L"Converter GPU Test Suites", eGPU);
FFTTestSuites FFT_test_suits_CPU(L"FFT CPU Test Suites", eCPU);
FFTTestSuites FFT_test_suits_GPU(L"FFT GPU Test Suites", eGPU);
MATHTestSuites MATH_testsuits_CPU(L"MATH CPU Test Suites", eCPU);
MATHTestSuites MATH_testsuits_GPU(L"MATH GPU Test Suites", eGPU);
ConvolutionTestSuites Convolution_testsuits_CPU_1(L"Convolution Test Suites CPU", new ConvolutionTestSuitesParam(
	eCPU,
	eMediumPriorityQueue,
	eMediumPriorityQueue,
	amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD,
	1024,
	1024
	));
ConvolutionTestSuites Convolution_testsuits_GPU_1(L"Convolution Test Suites GPU", new ConvolutionTestSuitesParam(
	eGPU,
	eMediumPriorityQueue,
	eMediumPriorityQueue,
	amf::TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED,
	1024,
	1024
	));
