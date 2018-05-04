//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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

// TALibVRDemo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif


#include "tanlibrary/include/TrueAudioNext.h"       //TAN

#include "../TrueAudioVR/trueaudiovr.h"
#include "../common/WasApiUtils.h"
#include "RoomWindow.h"
#include "../common/simplevraudio.h"

#include "resource.h"		// main symbols

#define MAXFILES 3
#define MAXRESPONSELENGTH 131072

// CTALibVRDemoApp:
// See TALibVRDemo.cpp for the implementation of this class
//

enum execMode {
    C_MODEL = 0,
    OCL_GPU = 0x04,
    OCL_GPU_MPQ = 0x05,
    OCL_GPU_RTQ = 0x06,
    OCL_CPU = 0x10
};

class CTALibVRDemoApp : public CWinApp
{
public:
	CTALibVRDemoApp();

// Overrides
public:
	BOOL InitInstance() override;

protected:
    void updatePositions();
    static void _cdecl updatePositionsThreadProc(void * p);

protected:
    WASAPIUtils Player;

    long nSamples[MAXFILES];
    unsigned char *pBuffers[MAXFILES];
    unsigned char *pProcessed;

    RoomDefinition room;

    MonoSource sources[MAX_SOURCES];
	StereoListener ears;
    int srcEnables[MAX_SOURCES];
	bool headSpin = false;

    int fftLen = 32768;
    int bufSize = 2048 * 4; 

    RoomWin *m_RoomSimWindow;

	Audio3D *m_pAudioVR;

	DECLARE_MESSAGE_MAP()
};
