#pragma once
#include "UnitTest.h"
#include "tanlibrary/include/TrueAudioNext.h"
#include "common.h"


class TANTestSuitsParam : public UnitTestParamBlob
{
public:
	TANTestSuitsParam(eTestMode mode);
#ifdef RTQ_ENABLED
	TANTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype, 
		int _generalCUcount = 0, int _convolutionCUcount = 0, int outputflag = 0);
#else
	TANTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype,
		int outputflag = 0);
#endif
	virtual void PrintTestSuitesParam();
	virtual std::wstring getTestSuiteParamWStr();
	amf::TANContextPtr m_pContext;
	eTestMode m_eTestMode;
	eCommandQueueType m_eGeneralQueueType;
	eCommandQueueType m_eConvolutuionQueueType;
#ifdef RTQ_ENABLED
	int m_iGeneralQueueCUCount = 0;
	int m_iConvolutionQueueCUCount = 0;
#endif // RTQ_ENABLED

	int m_iOutputFlag = 0;
};
