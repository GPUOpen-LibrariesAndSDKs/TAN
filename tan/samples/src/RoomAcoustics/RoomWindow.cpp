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
#include "stdafx.h"
#include <math.h>
#include "RoomWindow.h"
#include "resource.h"

int RoomWin::nWnds;
HWND RoomWin::hWndList[MAXWINDOWS];
RoomWin * RoomWin::pWndList[MAXWINDOWS];

XFORM RoomWin::xFormHeadPos;
XFORM RoomWin::xFormSourcesPos[MAX_SOURCES];
XFORM RoomWin::xFormHeadPosPrev;
XFORM RoomWin::xFormSourcesPosPrev[MAX_SOURCES];

//XFORM RoomWin::xFormSource2Pos;
float RoomWin::headAngle;
int RoomWin::m_height;
int RoomWin::m_width;
int RoomWin::m_heightOrg;
int RoomWin::m_widthOrg;
//bool RoomWin::stop = false;
bool RoomWin::running = false;
//bool RoomWin::updating = false;
int RoomWin::enableSrc[MAX_SOURCES];
int RoomWin::srcTrackHead[MAX_SOURCES];
int RoomWin::activeSource=1;
int RoomWin::nSources = 1;
int RoomWin::timerPeriodMS = 0;

RoomWin *g_RoomSimWindow;


RoomWin::RoomWin(int width, int height, char *title)
{
    WNDPROC wndProc = (WNDPROC )RoomWin::MainWndProc;
    int idxWin = RoomWin::nWnds++;
    if(idxWin >= MAXWINDOWS){
        return;
    }
    RoomWin::pWndList[idxWin] = this;

    HINSTANCE hinstance = (HINSTANCE) GetModuleHandle(NULL);
    HICON hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDR_MAINFRAME));

    //HWND hwnd = CreateWindowEx(0,winName,WS_CAPTION | WS_SYSMENU,10,10,256,256,NULL,NULL,NULL,NULL);
    WNDCLASSEX wndclass = { 
    sizeof(WNDCLASSEX),				//UINT       cbSize; 
    CS_HREDRAW | CS_VREDRAW,	//UINT       style; 
    wndProc, //WindowProc,			//WNDPROC    lpfnWndProc; 
    0,				//int        cbClsExtra; 
    0,				//int        cbWndExtra; 
    hinstance,		//HINSTANCE  hInstance; 
    hIcon,			//HICON      hIcon; 
    NULL,			//HCURSOR    hCursor; 
    (HBRUSH) GetStockObject (WHITE_BRUSH),			//HBRUSH     hbrBackground; 
    NULL,			//LPCTSTR    lpszMenuName; 
    TEXT("MainWClass"),	//LPCTSTR    lpszClassName; 
    hIcon,			//HICON      hIconSm; 
    }; 

    RegisterClassEx(&wndclass);
    int err = GetLastError();
    memset(windowTitle,0,sizeof(windowTitle));
    strncpy_s(windowTitle, title, (sizeof(windowTitle) / sizeof(windowTitle[0]))-1  );
    wTitleLen = (int) strlen(windowTitle);

    hwndMain = CreateWindowEx( 
    0,                      // no extended styles           
    TEXT("MainWClass"),           // class name                   
    windowTitle,					// window name                  
    WS_OVERLAPPEDWINDOW,    // overlapped window            
    CW_USEDEFAULT,          // default horizontal position  
    CW_USEDEFAULT,          // default vertical position    
    width,          //  width                
    height,          // default height               
    (HWND) NULL,            // no parent or owner window    
    (HMENU) NULL,           // class menu used              
    hinstance,              // instance handle              
    NULL);                  // no window creation data      
    m_heightOrg = m_height = height;
    m_widthOrg = m_width = width;
    RoomWin::hWndList[idxWin] = hwndMain;

    err = GetLastError();
    //ShowWindow(hwndMain, SW_SHOW);

    hdcMain = GetDC(hwndMain);
    hdcMem = CreateCompatibleDC(hdcMain);

    hbmp = CreateCompatibleBitmap(hdcMain,width,height);
    SelectObject(hdcMem,hbmp);
    //HPEN hpen, hpenOld;
    //hpen =CreatePen(PS_SOLID,1,RGB(0,0,0));
    bmInfo.bmiColors[0].rgbRed = 255;
    bmInfo.bmiColors[0].rgbGreen = 255;
    bmInfo.bmiColors[0].rgbBlue = 255;
    bmInfo.bmiColors[0].rgbReserved = 255;
    bmInfo.bmiColors[1].rgbRed = 0;
    bmInfo.bmiColors[1].rgbGreen = 0;
    bmInfo.bmiColors[1].rgbBlue = 0;
    bmInfo.bmiColors[1].rgbReserved = 0;
    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biWidth = width;
    bmInfo.bmiHeader.biHeight = height;
    bmInfo.bmiHeader.biPlanes = 1;
    bmInfo.bmiHeader.biBitCount = 1;
    bmInfo.bmiHeader.biCompression = 0;
    bmInfo.bmiHeader.biSizeImage = width*height/8;
    bmInfo.bmiHeader.biXPelsPerMeter = 1000;
    bmInfo.bmiHeader.biYPelsPerMeter = 1000;
    bmInfo.bmiHeader.biClrUsed = 2;
    bmInfo.bmiHeader.biClrImportant = 0;

    RECT caRect;
    GetClientRect(hwndMain,&caRect);
    width += width - caRect.right;
    height += height - caRect.bottom;
    SetWindowPos(hwndMain,HWND_TOP,0,0,width,height,SWP_SHOWWINDOW);


    //SetGraphicsMode(hdcMain,GM_ADVANCED);

    //RoomWin::xFormHeadPos.eDx = width*0.8;
    //RoomWin::xFormHeadPos.eDy = height*0.8;
    RoomWin::xFormHeadPos.eM11 = 1.0;
    RoomWin::xFormHeadPos.eM12 = 0.0;
    RoomWin::xFormHeadPos.eM21 = 0.0;
    RoomWin::xFormHeadPos.eM22 = 1.0;

    //RoomWin::xFormSourcePos.eDx = width/2;
    //RoomWin::xFormSourcePos.eDy = width/20;
    RoomWin::xFormSourcesPos[0].eM11 = 1.0;
	RoomWin::xFormSourcesPos[0].eM12 = 0.0;
	RoomWin::xFormSourcesPos[0].eM21 = 0.0;
	RoomWin::xFormSourcesPos[0].eM22 = 1.0;

    RoomWin::activeSource = 1;
    //timer 1s to force periodic updates:
    if (timerPeriodMS > 0){
        SetTimer(hwndMain, 1, timerPeriodMS, NULL);
    }

};

void RoomWin::Close()
{
    DestroyWindow(hwndMain);
}

void RoomWin::invalidateMovedRects(HWND hwnd)
{
    // invalidate moved head rect:
    float angle = RoomWin::headAngle;
    float scale = RoomWin::m_width / float(RoomWin::m_widthOrg);
    XFORM tXform = RoomWin::xFormHeadPos;
    //angle = RoomWin::headAngle;
    rotateAndScale(&tXform, angle, scale);

    if (memcmp(&RoomWin::xFormHeadPosPrev, &tXform, sizeof(XFORM)) != 0) {
        RECT rh;
        rh.left = LONG(RoomWin::xFormHeadPosPrev.eDx - 32);
        rh.bottom = LONG(RoomWin::xFormHeadPosPrev.eDy - 32);
        rh.right = rh.left + 64;
        rh.top = rh.bottom + 64;
        InvalidateRect(hwnd, &rh, TRUE);
    }
    RoomWin::xFormHeadPosPrev = tXform;

    // invalidate moved source rects:
    for (int idx = 0; idx < nSources; idx++){
        if (srcTrackHead[idx]) continue;
        tXform = RoomWin::xFormSourcesPos[idx];
        rotateAndScale(&tXform, 0.0, scale);

        if (memcmp(&tXform, &RoomWin::xFormSourcesPosPrev[idx], sizeof(XFORM)) != 0) {
            RECT rs;
            XFORM tXformP = RoomWin::xFormSourcesPosPrev[idx];
            rs.left = LONG(tXformP.eDx - 32);
            rs.bottom = LONG(tXformP.eDy - 32);
            rs.right = rs.left + 64;
            rs.top = rs.bottom + 64;
            InvalidateRect(hwnd, &rs, TRUE);
        }

        RoomWin::xFormSourcesPosPrev[idx] = tXform;
    }

}



bool RoomWin::Update()
{
	RoomWin::running = true;
    InvalidateRect(hwndMain,NULL,TRUE);
    //strncpy(&windowTitle[wTitleLen],progress,sizeof(windowTitle)- wTitleLen - 1);
    SetWindowText(hwndMain,windowTitle);
    UpdateWindow(hwndMain);

    MSG msg;
    BOOL bRet;
    //while( (bRet = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0)
    //while ((bRet = GetMessage(&msg, NULL, 0, WM_KEYLAST)) != 0)
    MSG lastMouseMove;
    lastMouseMove.message = 0;
    while (RoomWin::running)
    { 
        bRet = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msg.message == WM_MOUSEMOVE && bRet != 0){
            lastMouseMove = msg;
            continue;
        }
        else if (lastMouseMove.message == WM_MOUSEMOVE){
            TranslateMessage(&lastMouseMove);
            DispatchMessage(&lastMouseMove);

            lastMouseMove.message = 0;
        }

        if (bRet != -1 )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
        }
        if(msg.message == WM_QUIT){
			RoomWin::running = false;
            return true;
        }
        if (bRet == 0){
            Sleep(5);
        }
    }
    return false;
};


VOID CALLBACK updateTimerProc(HWND hwd, UINT p1, UINT p2, DWORD p3)
{
    g_RoomSimWindow->Update();
}

int RoomWin::WaitForClose()
{
    MSG msg;
    BOOL bRet;

    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    return 0;
};

RoomWin::~RoomWin()
{
};

void RoomWin::drawSource(HDC hdcWin, int num)
{
    //MoveToEx(hdcWin,5,5,NULL);
    //LineTo(hdcWin,5,-5);
    //LineTo(hdcWin,-5,-5);
    //LineTo(hdcWin,-5,5);
    //LineTo(hdcWin,5,5);
    // speaker shape
    MoveToEx(hdcWin, 15, -10, NULL);
    LineTo(hdcWin, -15, -10);
    LineTo(hdcWin, -15, 0);
    LineTo(hdcWin, 15, 0);
    LineTo(hdcWin, 15, -10);
    MoveToEx(hdcWin, -5, 0, NULL);
    LineTo(hdcWin, -15, 10);
    LineTo(hdcWin, 15, 10);
    LineTo(hdcWin, 5, 0);
    Arc(hdcWin, -15, 18, 15, -10, -10, 16, 10, 16);
    RECT tr = {-15,10,15,-10};

    // Retrieve a handle to the variable stock font.  
    //HFONT hFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
    //HFONT hOldFont = (HFONT)SelectObject(hdcWin, hFont);
    //SetTextColor(hdcWin, RGB(0, 0, 0));
    char buf[20];
    sprintf_s(buf, "%d", num);
    TextOut(hdcWin, 20, -10, buf, 2);
}

void RoomWin::drawHead(HDC hdcWin, int num)
{
    //MoveToEx(hdcWin,0,-25,NULL);
    //LineTo(hdcWin,25,30);
    //LineTo(hdcWin,-25,30);
    //LineTo(hdcWin,0,-25);

    // back of head
    Arc(hdcWin,-20,25,20,-20,-20, 0, 20,0 );
    // forehead
    Arc(hdcWin, -18, 15, 18, -25, 18, -10, -18, -10);
    // left ear
    MoveToEx(hdcWin, -20, 0, NULL);
    LineTo(hdcWin, -22, 4);
    LineTo(hdcWin, -24, 4);
    LineTo(hdcWin, -25, 1);
    LineTo(hdcWin, -24, -1);
    LineTo(hdcWin, -25, -2);
    LineTo(hdcWin, -22, -8);
    LineTo(hdcWin, -21, -8);
    LineTo(hdcWin, -19, -5);
    LineTo(hdcWin, -18, -11);
    // right ear
    MoveToEx(hdcWin, 20, 0, NULL);
    LineTo(hdcWin, 22, 4);
    LineTo(hdcWin, 24, 4);
    LineTo(hdcWin, 25, 1);
    LineTo(hdcWin, 24, -1);
    LineTo(hdcWin, 25, -2);
    LineTo(hdcWin, 22, -8);
    LineTo(hdcWin, 21, -8);
    LineTo(hdcWin, 19, -5);
    LineTo(hdcWin, 18, -11);
    LineTo(hdcWin, 18, -10);
    // nose
    Arc(hdcWin, -4, -20, 4, -30, 5, -25, -5, -25);

    if (num > 0) {
        char buf[20];
        sprintf_s(buf, "%d", num);
        TextOut(hdcWin, 20, -10, buf, 1);
    }
}

void RoomWin::rotateAndScale(XFORM *pXForm, float angle, float scale)
{
    float sinA = sinf(angle*3.14159265f/180);
    float cosA = cosf(angle*3.14159265f/180);
    pXForm->eDx *= scale;
    pXForm->eDy *= scale;
    pXForm->eM11 = cosA * scale;
    pXForm->eM12 = sinA * scale;
    pXForm->eM21 = -sinA * scale;
    pXForm->eM22 = cosA * scale;
}

void updatePositions();

LRESULT CALLBACK RoomWin::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
 
    static float angle;
    PAINTSTRUCT ps;
    int idxWin = 0;
    int width, height;
    BOOL ret = FALSE;
    //HDC hdcWin = GetDC(hwnd);
    static int timerID = 0;
    //static int activeSource = 1;

    angle = RoomWin::headAngle;
    static float shake = 0.00001;

    for(idxWin= 0; idxWin < MAXWINDOWS; idxWin++)
    {
        if(hwnd == RoomWin::hWndList[idxWin])
            break;
    }
    switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window. 
            return 0; 
 
        case WM_PAINT:
        if (GetUpdateRect(hwnd, NULL, FALSE)) 
        {
            // Paint the window's client area. 
            BeginPaint(hwnd,&ps);
            HDC hdcWin = GetDC(hwnd);

            XFORM tXform;

            SetGraphicsMode(hdcWin, GM_ADVANCED);
            width = ps.rcPaint.right - ps.rcPaint.left;
            height = ps.rcPaint.bottom - ps.rcPaint.top;
            float scale =  RoomWin::m_width / float(RoomWin::m_widthOrg);

            // draw rotated scaled sources:
			for (int idx = 0; idx < nSources; idx++){
				if (srcTrackHead[idx]) continue;
				tXform = RoomWin::xFormSourcesPos[idx];
                rotateAndScale(&tXform, 0.0, scale);
                ret = SetWorldTransform(hdcWin, &tXform);

                drawSource(hdcWin, idx + 1);
                RoomWin::xFormSourcesPosPrev[idx] = tXform;
		    }

            // draw rotated scaled head:
            tXform = RoomWin::xFormHeadPos;
            rotateAndScale(&tXform, angle, scale);          
            ret = SetWorldTransform(hdcWin, &tXform);

            if (RoomWin::srcTrackHead[0]) {
                drawHead(hdcWin, 1);
            }
            else {
                drawHead(hdcWin, 0);
            }

            ReleaseDC(hwnd, hdcWin);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:

            switch(wParam){
            case VK_UP:
                RoomWin::xFormHeadPos.eDy -= 5*cosf(angle*3.14159265f/180);
                RoomWin::xFormHeadPos.eDx += 5*sinf(angle*3.14159265f/180);

                if (RoomWin::xFormHeadPos.eDx < 0.0) RoomWin::xFormHeadPos.eDx = 0.0;
                if (RoomWin::xFormHeadPos.eDy < 0.0) RoomWin::xFormHeadPos.eDy = 0.0;
                if (RoomWin::xFormHeadPos.eDx > RoomWin::m_widthOrg) 
                    RoomWin::xFormHeadPos.eDx = (float)RoomWin::m_widthOrg;
                if (RoomWin::xFormHeadPos.eDy > RoomWin::m_heightOrg) 
                    RoomWin::xFormHeadPos.eDy = (float)RoomWin::m_heightOrg;

                break;
            case VK_DOWN:
                RoomWin::xFormHeadPos.eDy += 5*cosf(angle*3.14159265f/180);
                RoomWin::xFormHeadPos.eDx -= 5*sinf(angle*3.14159265f/180);
                break;
            case VK_LEFT:
                angle -= 5.0;
                break;
            case VK_RIGHT:
                angle += 5.0;
                break;
            case VK_NUMPAD1:
            case VK_F1:
                activeSource = 1;
                break;
			case VK_F2:
				activeSource = 2;
				break;
			case VK_F3:
				activeSource = 3;
				break;
			case VK_F4:
				activeSource = 4;
				break;
			case VK_F5:
				activeSource = 5;
				break;
			case VK_F6:
				activeSource = 6;
				break;
			case VK_F7:
				activeSource = 7;
				break;
			case VK_F8:
				activeSource = 8;
				break;
			case VK_F9:
				activeSource = 9;
				break;
			case VK_F10:
				activeSource = 10;
				break;
			case VK_F11:
				activeSource = 11;
				break;
			case VK_F12:
				activeSource = 12;
				break;
            case VK_F13:
                activeSource = 13;
                break;
            case VK_F14:
                activeSource = 14;
                break;
            case VK_F15:
                activeSource = 15;
                break;
            case VK_F16:
                activeSource = 16;
                break;
            case VK_F17:
                activeSource = 17;
                break;
            case VK_F18:
                activeSource = 18;
                break;
            case VK_F19:
                activeSource = 19;
                break;
            case VK_F20:
                activeSource = 20;
                break;
            case VK_F21:
                activeSource = 21;
                break;
            case VK_F22:
                activeSource = 22;
                break;
            case VK_F23:
                activeSource = 23;
                break;
            case VK_F24:
                activeSource = 24;
                break;

			default:
                break;
            }
            RoomWin::headAngle = angle;
            //updatePositions();
            return 0;
            break;
        case WM_SIZE:
            break;
        case WM_SIZING:
            {
                // Set the size and position of the window. 
                //int w = LOWORD(lParam);
                //int h = HIWORD(lParam);
                RECT *pwinRect = (RECT *)lParam;
                RECT caRect, wRect;
                GetClientRect(hwnd, &caRect);
                GetWindowRect(hwnd, &wRect);

                POINT p = { 0, 0 };
                HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                BOOL res = GetMonitorInfo(hMonitor, &monitorInfo);
                int monitorWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                int monitorHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

                int x = pwinRect->left;
                int y = pwinRect->top;
                int w = pwinRect->right - pwinRect->left;
                int h = pwinRect->bottom - pwinRect->top;
                int bw = wRect.right - caRect.right;
                int bh = wRect.bottom - caRect.bottom;
                w -= bw;
                h -= bh;

                if (w > monitorWidth) {
                    w = monitorWidth;
                    h = (m_heightOrg * w) / m_widthOrg;
                }

                if(h > monitorHeight) {
                    h = monitorHeight;
                    w = (m_widthOrg * h) / m_heightOrg;
                }

                if (w != m_width){
                    h = (m_heightOrg * w) / m_widthOrg;
                }
                else {
                    w = (m_widthOrg * h) / m_heightOrg;
                }

                m_width = w;
                m_height = h;


                pwinRect->right = pwinRect->left + m_width + bw;
                pwinRect->bottom = pwinRect->top + m_height + bh;

                //SetWindowPos(hwnd, HWND_TOP, 0, 0, m_width, m_height, SWP_NOMOVE | SWP_NOSENDCHANGING);
            }
            return 0; 

        case WM_MOVE:
            return 0;
            break;
        case WM_TIMER:
            shake = -shake;
            RoomWin::headAngle += shake;
            break;
        case WM_MOUSEMOVE:
            if (wParam & MK_LBUTTON){
                int xPos, yPos;
                xPos = GET_X_LPARAM(lParam);
                yPos = GET_Y_LPARAM(lParam);
                
                RoomWin::xFormHeadPos.eDx = float(xPos*m_widthOrg / m_width);
                RoomWin::xFormHeadPos.eDy = float(yPos*m_heightOrg / m_height);
            }
            if (wParam & MK_RBUTTON){
                int xPos, yPos;
                xPos = GET_X_LPARAM(lParam);
                yPos = GET_Y_LPARAM(lParam);

				int idx = activeSource - 1;
				if (idx < nSources) {
					RoomWin::xFormSourcesPos[idx].eDx = float(xPos*m_widthOrg / m_width);
					RoomWin::xFormSourcesPos[idx].eDy = float(yPos*m_heightOrg / m_height);
				}

            }
            break;
        case WM_LBUTTONDOWN:
            {
                int xPos, yPos;
                xPos = GET_X_LPARAM(lParam);
                yPos = GET_Y_LPARAM(lParam);

                RoomWin::xFormHeadPos.eDx = float(xPos*m_widthOrg / m_width);
                RoomWin::xFormHeadPos.eDy = float(yPos*m_heightOrg / m_height);
            }
            break;
            
        case WM_RBUTTONDOWN:
            {
                int xPos, yPos;
                xPos = GET_X_LPARAM(lParam);
                yPos = GET_Y_LPARAM(lParam);
				int idx = activeSource - 1;
				if (idx < nSources) {
					RoomWin::xFormSourcesPos[idx].eDx = float(xPos*m_widthOrg / m_width);
					RoomWin::xFormSourcesPos[idx].eDy = float(yPos*m_heightOrg / m_height);
				}

            }
            break;
            
        case WM_MOUSEWHEEL:
            {
                short d = HIWORD(wParam);
                angle += 5 * float(d) / float(WHEEL_DELTA);
                RoomWin::headAngle = angle;
            }
            break;
        //case WM_LBUTTONUP:
        //	break;
        //case WM_LBUTTONDBLCLK:
        //case WM_RBUTTONDOWN:
        //	break;
        //case WM_RBUTTONUP:
        //	break;
        //case WM_RBUTTONDBLCLK:
        //case WM_MBUTTONUP:
        //case WM_MBUTTONDBLCLK:
        //	break;
        //case WM_INITDIALOG:
        //case WM_ACTIVATE:
        //case WM_ACTIVATEAPP:
        //case WM_SHOWWINDOW:
        //case WM_SETFOCUS:
        //		return DefWindowProc(hwnd, uMsg, wParam, lParam);
        //		break;
        case WM_SHOWWINDOW:
            break;
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
            PostMessage(hwnd,WM_QUIT,0,0);
            return -1; 
 
        // 
        // Process other messages. 
        // 
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    } 

    RoomWin::invalidateMovedRects(hwnd);

    return 0; 
} 
