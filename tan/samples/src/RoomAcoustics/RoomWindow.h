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

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "../common/maxlimits.h"

#define MAXWINDOWS 100

class RoomWin
{
private:
	static int nWnds;
	static HWND hWndList[MAXWINDOWS];
	static RoomWin * pWndList[MAXWINDOWS];

	static LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam);    // second message parameter

	HDC hdcMem;
	HBITMAP hbmp;
	struct {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[2];	
	} bmInfo;

public:
	HWND hwndMain;
	HDC hdcMain;
    char windowTitle[256];

	int wTitleLen;
	//static bool stop;
	static bool running;
	//static bool updating;

    RoomWin(int width, int height, char *title);
	~RoomWin();
	bool Update();
    static void invalidateMovedRects(HWND hwnd);
	static int m_height, m_width;
	static int m_heightOrg, m_widthOrg;

	//void startPeriodicUpdates();
	//void __stdcall updateTimerProc(HWND hwd, UINT p1, UINT p2, DWORD p3);

	int WaitForClose();
	void Close();
	static void drawSource(HDC hdcWin, int num);
	static void drawHead(HDC hdcWin, int num);
	static bool visible;

	static XFORM xFormHeadPos;
    static XFORM xFormHeadPosPrev;
    static float headAngle;
	//static XFORM xFormSourcePos;
	static XFORM xFormSourcesPos[MAX_SOURCES];
    static XFORM xFormSourcesPosPrev[MAX_SOURCES];
    //static XFORM xFormSource2Pos;
	static void rotateAndScale(XFORM *pXForm, float angle, float scale);
	static int enableSrc[MAX_SOURCES];
	static int srcTrackHead[MAX_SOURCES];
	static int activeSource;
	static int nSources;
    static int timerPeriodMS;
};
