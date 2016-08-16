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

// RoomAcousticsDlg.h : header file
//
#include "../common/maxlimits.h"
#pragma once

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

#define MAX_DEVICES 10
#define MAX_DEVNAME_LEN 100

struct attribute {
	char *name;
	void *value;
	char fmt; // f, i, s
};

struct element {
	char *name;
	int nAttribs;
	struct attribute *attriblist;
	int nElements;
	struct element *elemList;
};


// CRoomAcousticsDlg dialog
class CRoomAcousticsDlg : public CDialog
{
// Construction
public:
	CRoomAcousticsDlg(AmdTrueAudioVR *pTAVR, TANConverterPtr pConverter, CWnd* pParent = NULL);
	~CRoomAcousticsDlg();
// Dialog Data
	enum { IDD = IDD_RoomAcoustics_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	void updateRoomInfo();
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	//AmdTrueAudio *m_pTA;
	AmdTrueAudioVR *m_pTAVR;
    TANConverterPtr m_spConverter;

public:
    void LoadParameters(char * xmlFileName);
	void updateAllFields();
	bool findElement(char **start, char **end, char *name);
	bool parseElement(char *start, char *end, struct element *elem);

	float srcX[MAX_SOURCES], srcY[MAX_SOURCES], srcZ[MAX_SOURCES];
	int srcEnable[MAX_SOURCES];

	int nFiles;

	int src1EnableMic;
	int src1TrackHeadPos;
	int src1MuteDirectPath;
	float headX, headY, headZ;
	float yaw, pitch, roll;
	float earSpacing;
	int autoSpinHead;
	float roomWidth, roomHeight, roomLength;
	float roomDampLeft, roomDampRight, roomDampFront, roomDampBack, roomDampTop, roomDampBottom;
	int convolutionLength;
	int bufferSize;
    int useGPU4Conv;
    int useGPU4Room;

    char *waveFileNames[MAX_SOURCES];

	char *deviceNames[MAX_DEVICES];
	int roomDevIdx;
	int convDevIdx;
	int sourceIdx;

	afx_msg void OnBnClickedButtonopen();
	afx_msg void OnEnKillfocusEditfile();
	afx_msg void OnEnChangeEditSrc1X();
	afx_msg void OnEnChangeEditSrc1Y();
	afx_msg void OnEnChangeEditSrc1Z();
	afx_msg void OnEnChangeEditHeadX();
	afx_msg void OnEnChangeEditHeadY();
	afx_msg void OnEnChangeEditHeadZ();
	afx_msg void OnEnChangeEditEarspacing();
	afx_msg void OnEnChangeEditRoomWidth();
	afx_msg void OnEnChangeEditRoomHeight();
	afx_msg void OnEnChangeEditRoomLength();
	afx_msg void OnEnChangeEditRoomDampLeft();
	afx_msg void OnEnChangeEditRoomDampFront();
	afx_msg void OnEnChangeEditRoomDampTop();
	afx_msg void OnEnChangeEditRoomDampRight();
	afx_msg void OnEnChangeEditRoomDampBack();
	afx_msg void OnEnChangeEditRoomDampBottom();
	afx_msg void OnEnChangeEditConvLength();
	afx_msg void OnEnChangeEditBufferSize();
	afx_msg void OnBnClickedRadioGpu();
	afx_msg void OnBnClickedRadioCpu();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedExport();

	afx_msg void OnBnClickedCheckEnSrc();
	afx_msg void OnEnChangeEditSrcX();
	afx_msg void OnEnChangeEditSrcY();
	afx_msg void OnEnChangeEditSrcZ();
	afx_msg void OnEnChangeEditfile2();
	afx_msg void OnBnClickedCheckSrc1TrackHead();
	afx_msg void OnBnClickedCheckSrc1MuteDirect();
	afx_msg void OnEnKillfocusEditfile2();
	afx_msg void OnBnClickedCheckEnSrc1mic();
	afx_msg void OnBnClickedButtonopen2();
    afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioRgCpu();
	afx_msg void OnBnClickedRadioRgGpu();
	afx_msg void OnBnClickedHeadspin();
	afx_msg void OnLbnSelchangeListDevicesRoom();
	afx_msg void OnLbnSelchangeListDevicesConv();
	afx_msg void OnLbnSelchangeListSources();
	afx_msg void OnEnChangeEditNfiles();
};
