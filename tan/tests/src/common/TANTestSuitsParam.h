//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once
#include "UnitTest.h"
#include "tanlibrary/include/TrueAudioNext.h"

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
