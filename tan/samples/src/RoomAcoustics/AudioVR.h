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

#include "wav.h"
#include "fifo.h"
//#include "wasapiutils.h"
#  include "tanlibrary/include/TrueAudioNext.h" //TAN
#include "..\TrueAudioVR\trueaudiovr.h"
#include "maxlimits.h"
#include "IWavPlayer.h"

#include <Windows.h>

//#define MAXFILES 3
//#define MAXRESPONSELENGTH 131072

class transRotMtx{
private:
	float m[3][4];
public:
	transRotMtx();
	void setAngles(float yaw, float pitch, float roll);
	void setOffset(float x, float y, float z);
	void transform(float &X, float &Y, float &Z);
};

class  Audio3D {
public:
    int processProc();
    int updateProc();

    Audio3D();
    Audio3D(Audio3D const&) = delete;
    virtual ~Audio3D();

private:
    static unsigned _stdcall processThreadProc(void *ptr);
    static unsigned _stdcall updateThreadProc(void *ptr);
    HANDLE m_hProcessThread;
    HANDLE m_hUpdateThread;

    //void operator=(Audio3D const&);
    int process(short *pOut, short *pChan[MAX_SOURCES], int sampleCount);

    bool running = false;
    bool stop = false;

	std::unique_ptr<IWavPlayer> mPlayer;

	int m_nFiles;
    long nSamples[MAX_SOURCES];
	unsigned char *pBuffers[MAX_SOURCES];
    unsigned char *pProcessed;
  
	TANContextPtr m_spContext1;
	TANContextPtr m_spContext2;
	TANConvolutionPtr m_spConvolution;
	TANConverterPtr m_spConverter;
	TANFFTPtr m_spFft;
	AmdTrueAudioVR *m_pTAVR = NULL;


    RoomDefinition room;
    MonoSource sources[MAX_SOURCES];  
	StereoListener ears;


    int src1EnableMic = 0;
    int src1TrackHeadPos = 0;
    int src1MuteDirectPath = 0;

	float *responseBuffer;
    float *responses[MAX_SOURCES*2];
    float *inputFloatBufs[MAX_SOURCES*2];
	float *outputFloatBufs[MAX_SOURCES * 2];

    int m_fftLen = 65536; // 2048;//  8192;
    int m_bufSize = 2048 * 4;//4096 * 4; 
    int64_t m_samplePos;

	// World To Room coordinate transform:
	transRotMtx m_mtxWorldToRoomCoords;
	float m_headingOffset;
	bool m_headingCCW;

public:

	int init(RoomDefinition room, int nFiles, char **inFiles, int fftLen, int bufSize, 
		bool useGPU_Conv, int devIdx_Conv, int useRTQ_Conv, int cuRes_Conv, 
		bool useGPU_IRGen, int devIdx_IRGen, int useRTQ_IRGen, int cuRes_IRGen);

	//To Do implement this:
	int setWorldToRoomCoordTransform(float translationX, float translationY, float translationZ, 
									 float rotationY, float headingOffset, bool headingCCW);

	int setSrc1Options(bool useMicSource, bool trackHeadPos);

	int finit();

    int Run();
    int Stop();
    int64_t getCurrentPosition();

    int updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll);
    int updateSourcePosition(int srcNumber, float x, float y, float z);

};
