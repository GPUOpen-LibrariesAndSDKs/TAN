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
#include <io.h>

#include "RoomAcoustics.h"
#include "Aclapi.h"
#include "RoomAcousticsDlg.h"
#include "../common/gpuutils.h"
//#include "..\common\fifo.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTALibVRDemoApp

BEGIN_MESSAGE_MAP(CTALibVRDemoApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// The one and only CTALibVRDemoApp object
CTALibVRDemoApp theApp;

// CTALibVRDemoApp construction

CTALibVRDemoApp::CTALibVRDemoApp()
{
    // Place all significant initialization in InitInstance
    m_pAudioVR = NULL;
};

void CTALibVRDemoApp::updatePositions()
{
    static float prevX, prevY, prevA, prevW, prevH;
    static float prevSX[MAX_SOURCES], prevSY[MAX_SOURCES];
    //, prevXS, prevYS, prevXS2, prevYS2;
    float xL = RoomWin::xFormHeadPos.eDx;
    float yL = RoomWin::xFormHeadPos.eDy;
    float angle = RoomWin::headAngle;

    if (xL == prevX && yL == prevY && angle == prevA && RoomWin::m_width == prevW && RoomWin::m_height == prevH){
        bool srcChange = false;
        for (int idx = 0; idx < MAX_SOURCES; idx++){
            if (prevSX[idx] != RoomWin::xFormSourcesPos[idx].eDx ||
                prevSY[idx] != RoomWin::xFormSourcesPos[idx].eDy ) {
                srcChange = true;
            }
        }
        if (!srcChange) return;
    }

    prevX = xL;
    prevY = yL;
    prevA = angle;

    for (int idx = 0; idx < MAX_SOURCES; idx++){
        prevSX[idx] = RoomWin::xFormSourcesPos[idx].eDx;
        prevSY[idx] = RoomWin::xFormSourcesPos[idx].eDy;
    }

    prevW = (float)RoomWin::m_width;
    prevH = (float)RoomWin::m_height;

    ears.headX = float(room.width * (1.0 - xL / RoomWin::m_widthOrg));
    ears.headZ = float(room.length * (yL / RoomWin::m_heightOrg));
    //ears.headY 
    ears.yaw = angle;

    m_pAudioVR->updateHeadPosition(ears.headX, ears.headY, ears.headZ, ears.yaw, ears.pitch, ears.roll);

    for (int idx = 0; idx < MAX_SOURCES; idx++){
        float xS = RoomWin::xFormSourcesPos[idx].eDx;
        float yS = RoomWin::xFormSourcesPos[idx].eDy;
        sources[idx].speakerX = float(room.width * (1.0 - xS / RoomWin::m_widthOrg));
        sources[idx].speakerZ = float(room.length * (yS / RoomWin::m_heightOrg));

        if (srcEnables[idx]) {
            m_pAudioVR->updateSourcePosition(idx, sources[idx].speakerX, sources[idx].speakerY, sources[idx].speakerZ);
        }

    }
}

void _cdecl CTALibVRDemoApp::updatePositionsThreadProc(void * p)
{
    //RoomWin::updating = true;

    CTALibVRDemoApp *me = static_cast<CTALibVRDemoApp*>(p);
    while (RoomWin::running){
        if (me->headSpin) {
            RoomWin::headAngle += 5.0;
            RoomWin::invalidateMovedRects(me->m_RoomSimWindow->hwndMain);
        }
        me->updatePositions();
        //Sleep(50);
        Sleep(30);
    }
    //RoomWin::updating = false;
}

#define RETURN_IF_FAILED(x) \
{ \
    AMF_RESULT tmp = (x); \
    if (tmp != AMF_OK) { \
        ::MessageBoxA(0, #x, "Error", MB_OK); \
        return FALSE; \
    } \
}


void GetFileVersionAndDate( WCHAR *logMessage, char *version){
    time_t dt = time(NULL);
    struct tm *lt = localtime(&dt);
    DWORD size;
    DWORD dummy;
    WCHAR filename[1024];

    GetModuleFileNameW(NULL, filename, sizeof(filename) / sizeof(filename[0]));
    size = GetFileVersionInfoSizeW(filename, &dummy);
    WCHAR * buffer = new WCHAR[size];
    WCHAR *ver = NULL, *pStr;
    WCHAR *pSrch = L"FileVersion";
    if (buffer == NULL){ return ; }
    if (GetFileVersionInfoW(filename, 0, size, (void*)buffer)){
        pStr = buffer;
        while (pStr < buffer + size){
            if (wcsncmp(pStr, pSrch, 10) == 0){
                ver = pStr + wcslen(pStr) + 2;
                break;
            }
            else {
                pStr++;
            }
        }
    }
    
    

    wsprintfW(logMessage, L"**** %s v%s on %4d/%02d/%02d %02d:%02d:%02d ****\n", filename, ver,
            2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

    size_t lv = wcslen(ver);
    for (int i = 0; i < lv; i++){
        version[i] = char(ver[i]);
    }
    delete buffer;
}

// CTALibVRDemoApp initialization

BOOL CTALibVRDemoApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("AMD/TAN/SAMPLES/ROOMACOUSTICS"));

    char configPath[MAX_PATH + 1];
    char dllPath[MAX_PATH + 1];
    char logPath[MAX_PATH + 1];
    GetModuleFileName(NULL, dllPath, MAX_PATH);
    char *pslash = strrchr(dllPath, '\\');
    char exename[80] = "\0";
    if (pslash){
        strncpy(exename, pslash + 1, 80);
        *pslash = '\0';
    }
    char *pdot = strrchr(exename, '.');
    if (pdot)
        *pdot = '\0';


    strncpy(configPath, dllPath, MAX_PATH);
    strncpy(logPath, dllPath, MAX_PATH);

    strncat_s(configPath, "\\default.xml", MAX_PATH);
    strncat_s(logPath, "\\", MAX_PATH);
    strncat_s(logPath, exename, MAX_PATH);
    strncat_s(logPath, ".log", MAX_PATH);

    FILE *fpLog = NULL;
    errno_t err = 0;
    errno = 0;
    // Redirect stdout and stderr to a log file. 

    if (fopen_s(&fpLog, logPath, "a+") == 0) {
        dup2(fileno(fpLog), fileno(stdout));
        dup2(fileno(fpLog), fileno(stderr));
    }

    WCHAR logDateVerStamp[MAX_PATH*2];
    char version[40];
    memset(version, 0, sizeof(version));
    GetFileVersionAndDate(logDateVerStamp, version);
    wprintf(logDateVerStamp);

    CRoomAcousticsDlg dlg(NULL, NULL);
    m_pMainWnd = &dlg;

    //double GR = 1.618;
    memset(&room, 0, sizeof(room));

    room.height = 4.0;
    room.width = 6.0;
    room.length = 10.0;
    room.mFront.damp = DBTODAMP(2.0);
    room.mBack.damp = DBTODAMP(6.0);
    room.mLeft.damp = room.mRight.damp = DBTODAMP(4.0);
    room.mTop.damp = room.mBottom.damp = DBTODAMP(2.0);
    ears.earSpacing = float(0.16);
    ears.headX = float(room.width*.8);
    ears.headZ = float(room.length*.8);
    ears.headY = 1.75;
    ears.pitch = 0.0;
    ears.roll = 0.0;
    ears.yaw = 0.0;

    for (int idx = 0; idx < MAX_SOURCES; idx++) {
        dlg.srcX[idx] = sources[idx].speakerX = room.width / 2 + idx*room.width / MAX_SOURCES;
        dlg.srcY[idx] = sources[idx].speakerZ = float(room.width * 0.05);
        dlg.srcZ[idx] = sources[idx].speakerY = 1.75;
    dlg.srcEnable[idx] = 0;
    }
    dlg.srcEnable[0] = 1;
    dlg.nFiles = 1;

    dlg.roomHeight = room.height;
    dlg.roomWidth = room.width;
    dlg.roomLength = room.length;


    dlg.earSpacing = ears.earSpacing;


    dlg.headX = ears.headX;
    dlg.headY = ears.headY;
    dlg.headZ = ears.headZ;

    dlg.roomDampFront = DAMPTODB(room.mFront.damp);
    dlg.roomDampBack = DAMPTODB(room.mBack.damp);
    dlg.roomDampLeft = DAMPTODB(room.mLeft.damp);
    dlg.roomDampRight = DAMPTODB(room.mRight.damp);
    dlg.roomDampTop = DAMPTODB(room.mTop.damp);
    dlg.roomDampBottom = DAMPTODB(room.mBottom.damp);

    dlg.convolutionLength = fftLen;
    dlg.bufferSize = bufSize / 4;
    dlg.src1EnableMic = 0;
    dlg.src1MuteDirectPath = 0;
    dlg.src1TrackHeadPos = 0;

    ///reset deviceName buffers
    for (int i = 0; i < MAX_DEVICES; i++){
        dlg.deviceNames[i][0] = '\0';
    }
    SetDllDirectory(dllPath); 
    int nDevices = listGpuDeviceNames(dlg.deviceNames, MAX_DEVICES);
    SetDllDirectory(NULL);

    dlg.LoadParameters(configPath);

    if (FAILED(CoInitialize(NULL))) return false;

    while (1)
    {
        // eat any left over messages:
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0){
            ;
        }

        // run the parameter dialog:
        INT_PTR nResponse = dlg.DoModal();

        // eat any left over messages:
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0){
            ;
        }

        if (nResponse == IDOK)
        {

            // set parameters to room simulation:
            room.height = dlg.roomHeight;
            room.width = dlg.roomWidth;
            room.length = dlg.roomLength;
            

            for(int idx = 0; idx < MAX_SOURCES; idx++){
                sources[idx].speakerX = dlg.srcX[idx];
                sources[idx].speakerY = dlg.srcY[idx];
                sources[idx].speakerZ = dlg.srcZ[idx];
                srcEnables[idx] = dlg.srcEnable[idx];
                RoomWin::enableSrc[idx] = dlg.srcEnable[idx];
            }

            RoomWin::nSources = dlg.nFiles;

            ears.earSpacing = dlg.earSpacing;

            ears.headX = dlg.headX;
            ears.headY = dlg.headY;
            ears.headZ = dlg.headZ;
            ears.pitch = dlg.pitch;
            ears.roll = dlg.roll;
            ears.yaw = dlg.yaw;

            headSpin = (dlg.autoSpinHead != 0);

            room.mFront.damp = DBTODAMP(dlg.roomDampFront);
            room.mBack.damp = DBTODAMP(dlg.roomDampBack);
            room.mLeft.damp = DBTODAMP(dlg.roomDampLeft);
            room.mRight.damp = DBTODAMP(dlg.roomDampRight);
            room.mTop.damp = DBTODAMP(dlg.roomDampTop);
            room.mBottom.damp = DBTODAMP(dlg.roomDampBottom);

            // initialize audio engine:
            m_pAudioVR = new Audio3D();

            int err = m_pAudioVR->init(dllPath, room, dlg.nFiles, dlg.waveFileNames, dlg.convolutionLength, dlg.bufferSize,
                dlg.useGPU4Conv, dlg.convDevIdx, dlg.useGPU4Room, dlg.roomDevIdx);

            if (err != 0) {
                continue;
            }
            m_pAudioVR->setWorldToRoomCoordTransform(0., 0., 0., 0., 0., true);

            // scale window to fit monitor:
            POINT p = { 0, 0 };
            HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFOEX monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            BOOL res = GetMonitorInfo(hMonitor, &monitorInfo);
            int monitorWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
            int monitorHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
            int pixScale = 100;

            float X = monitorHeight * room.width / room.length;
            float Y = monitorWidth * room.length / room.width;
            if (X < monitorWidth){
                pixScale = int(X / room.width);
            }
            else {
                pixScale = int(Y / room.length);
            }

            int width = int(pixScale * room.width); //1000 / 2;
            int height = int(pixScale * room.length); //1618 / 2;

            for (int idx = 0; idx < MAX_SOURCES; idx++) {
                RoomWin::xFormSourcesPos[idx].eDx = pixScale*(room.width - sources[idx].speakerX);
                RoomWin::xFormSourcesPos[idx].eDy = pixScale*(sources[idx].speakerZ);
            }



            RoomWin::xFormHeadPos.eDx = pixScale*(room.width - ears.headX);
            RoomWin::xFormHeadPos.eDy = pixScale*(ears.headZ);
            RoomWin::headAngle = ears.yaw;
            RoomWin::srcTrackHead[0] = dlg.src1TrackHeadPos;
            m_pAudioVR->setSrc1Options(dlg.src1EnableMic, dlg.src1TrackHeadPos);


            // log run data:
            puts("Run:");
            printf(" audio file=%s\n", dlg.waveFileNames[0]);
            printf(" convolution: length=%d buffersize =%d ",
                dlg.convolutionLength, dlg.bufferSize);
            printf(" source: x=%f y=%f z=%f\n", dlg.srcX[0], dlg.srcY[0], dlg.srcZ[0]);
            printf(" source2: on=%d enableMic=%d\n", dlg.srcEnable[1], dlg.src1EnableMic);
            printf(" head: x=%f y=%f z=%f\n", dlg.headX, dlg.headY, dlg.headZ);
            printf(" earSpacing: %f\n", dlg.earSpacing);
            printf(" room: width=%f height=%f length=%f\n", dlg.roomWidth, dlg.roomHeight, dlg.roomLength);
            printf(" wall damping: left=%f right=%f front=%f back=%f top=%f bottom=%f\n\n",
                dlg.roomDampLeft, dlg.roomDampRight, dlg.roomDampFront, dlg.roomDampBack, dlg.roomDampTop, dlg.roomDampBottom);
            fflush(stdout);

            timeBeginPeriod(1);

            char title[100];
            sprintf(title, "Room: %5.1lfm W x %5.1lfm L x %5.1lfm H          code version: %s", room.width, room.length, room.height, version);
            RoomWin* pMyWindow = new RoomWin(width, height, title);
            m_RoomSimWindow = pMyWindow;

            int frNum = 0;

            m_pAudioVR->Run();

            // once processing thread is running, start position update thread:
            _beginthread(updatePositionsThreadProc, 0, this);
            pMyWindow->Update();

            m_pAudioVR->Stop();

            pMyWindow->Close();
            delete pMyWindow;
            Sleep(100);

        }
        else if (nResponse == IDCANCEL)
        {
            //  dialog is dismissed with Cancel
            break; 
        }


        //maybe changed during run:
        for (int idx = 0; idx < MAX_SOURCES; idx++){
            dlg.srcX[idx] = sources[idx].speakerX;
            dlg.srcY[idx] = sources[idx].speakerY;
            dlg.srcZ[idx] = sources[idx].speakerZ;
        }

        dlg.headX = ears.headX;
        dlg.headY = ears.headY;
        dlg.headZ = ears.headZ;
        dlg.yaw = ears.yaw;
        dlg.pitch = ears.pitch;
        dlg.roll = ears.roll;

        m_pAudioVR->finit();
        delete m_pAudioVR;
        m_pAudioVR = NULL;
    }

    fclose(stderr);
    fclose(stdout);
    fclose(fpLog);

    return FALSE;
}
