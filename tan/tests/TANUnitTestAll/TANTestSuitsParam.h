#pragma once
#include "common/UnitTest.h"
#include "tanlibrary/include/TrueAudioNext.h"
#include "common.h"


class TANTestSuitsParam : public UnitTestParamBlob
{
public:
	TANTestSuitsParam(eTestMode mode);
	TANTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype, 
		int _generalCUcount = 0, int _convolutionCUcount = 0);
	amf::TANContextPtr m_pContext;
	eTestMode m_eTestMode;
};

class FFTTestSuitsParam : public TANTestSuitsParam
{
public:
	FFTTestSuitsParam(eTestMode mode) : TANTestSuitsParam(mode){ m_iAllowDiffInEpisilon = 0; }
	FFTTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype,
		int _generalCUcount = 0, int _convolutionCUcount = 0) : TANTestSuitsParam(mode, _generalqueuetype, _convolutionqueuetype,
		_generalCUcount, _convolutionCUcount){
		m_iAllowDiffInEpisilon = 0;}
	AMF_RESULT InitializeFFT();
	amf::TANFFTPtr m_pFFT;
	int m_iAllowDiffInEpisilon;
};

class ConverterTestSuitsParam : public TANTestSuitsParam
{
public:
	ConverterTestSuitsParam(eTestMode mode) : TANTestSuitsParam(mode){};
	ConverterTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype,
		int _generalCUcount = 0, int _convolutionCUcount = 0) : TANTestSuitsParam(mode, _generalqueuetype, _convolutionqueuetype,
		_generalCUcount, _convolutionCUcount){}
	AMF_RESULT InitializeConverter();
	amf::TANConverterPtr m_pConverter;

};

class ConvolutionTestSuitesParam : public TANTestSuitsParam
{
public:
	ConvolutionTestSuitesParam(
		eTestMode mode,
		eCommandQueueType _generalqueuetype = eMediumPriorityQueue,
		eCommandQueueType _convolutionqueuetype = eMediumPriorityQueue,
		amf::TAN_CONVOLUTION_METHOD _method = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD,
		amf_uint32 _responseLengthInSamples = 32768,
		amf_uint32 _bufferSizeInSamples = 128,
		amf_uint32 _channels = 1,
		int _generalCUcount = 0, 
		int _convolutionCUcount = 0) 
		: TANTestSuitsParam(mode, _generalqueuetype, _convolutionqueuetype,
		_generalCUcount, _convolutionCUcount)
	{
		m_eConvolutionMethod = _method;
		m_ibufferSizeInSamples = _bufferSizeInSamples;
		m_iresponseLengthInSamples = _responseLengthInSamples;
		m_iNumberOfChannel = _channels;
	}
	AMF_RESULT InitializeConvolution(amf::TAN_CONVOLUTION_METHOD _method, amf_uint32 _responseLengthInSamples,
		amf_uint32 _bufferSizeInSamples, amf_uint32 _channels = 1);
	amf::TAN_CONVOLUTION_METHOD m_eConvolutionMethod = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	
	amf_uint32 m_iresponseLengthInSamples = 1024 * 32;
	amf_uint32 m_ibufferSizeInSamples = 1024 / 8; 
	amf_uint32 m_iNumberOfChannel = 1;
	amf::TANConvolutionPtr m_pConvolution;
};

class MathTestSuitesParam : public TANTestSuitsParam
{
public:
	MathTestSuitesParam(eTestMode mode) : TANTestSuitsParam(mode){};
	MathTestSuitesParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype,
		int _generalCUcount = 0, int _convolutionCUcount = 0) : TANTestSuitsParam(mode, _generalqueuetype, _convolutionqueuetype,
		_generalCUcount, _convolutionCUcount){}
	AMF_RESULT InitializeMath();
	amf::TANMathPtr m_pMath;
};