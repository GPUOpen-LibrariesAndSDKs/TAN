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

#include "fifo.h"
#include "wasapiutils.h"
#include "tanlibrary/include/TrueAudioNext.h"       //TAN
#include "../TrueAudioVR/trueaudiovr.h"
#include "maxlimits.h"
#include <Windows.h>

// rotation, translation matrix
class transRotMtx{
private:
	float m[3][4];
public:
	transRotMtx();
	void setAngles(float yaw, float pitch, float roll);
	void setOffset(float x, float y, float z);
	void transform(float &X, float &Y, float &Z);
};

// Simple VR audio engine using True Audio Next GPU acceleration
class  Audio3D {
public:
    int processProc();
    int updateProc();

    Audio3D();
    Audio3D(Audio3D const&) = delete;
    virtual ~Audio3D();

private:
    static bool useIntrinsics;
    static const int IR_UPDATE_MODE = 1; // 0: Non-Blocking 1: Blocking
    static unsigned _stdcall processThreadProc(void *ptr);
    static unsigned _stdcall updateThreadProc(void *ptr);
    HANDLE m_hProcessThread;
    HANDLE m_hUpdateThread;

    int process(short *pOut, short *pChan[MAX_SOURCES], int sampleCount);

    bool running = false;
    bool updated = false;
    bool stop = false;
    bool updateParams = true;
    bool m_useOCLOutputPipeline;
    // Microsoft WASAPI based audio player:
    WASAPIUtils Player;

	int m_nFiles;
    long nSamples[MAX_SOURCES];
	unsigned char *pBuffers[MAX_SOURCES];
    unsigned char *pProcessed;
  
	TANContextPtr m_spTANContext1;
	TANContextPtr m_spTANContext2;
	TANConvolutionPtr m_spConvolution;
	TANConverterPtr m_spConverter;
    TANMixerPtr m_spMixer;
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
    cl_mem oclResponses[MAX_SOURCES * 2];
    bool m_useClMemBufs;

    float *inputFloatBufs[MAX_SOURCES*2];
	float *outputFloatBufs[MAX_SOURCES * 2];
    float* outputMixFloatBufs[2];
	cl_mem outputCLBufs[MAX_SOURCES * 2];
	cl_mem outputMainCLbuf;
	cl_mem outputMixCLBufs[2];
	cl_mem outputShortBuf;

	// current position in each stream:
	__int64 m_samplePos[MAX_SOURCES];

	// fft length must be power of 2
	// 65536/48000 = reverb time of 1.37 seconds
    int m_fftLen = 65536;  //default fft length

	// buffer length 4096 / 48000 = 85 ms update rate:
	int m_bufSize = 4096 * 4; // default buffer length


	// World To Room coordinate transform:
	transRotMtx m_mtxWorldToRoomCoords;
	float m_headingOffset;
	bool m_headingCCW;

    // RT-Queues
    cl_command_queue cmdQueue1 = NULL;
    cl_command_queue cmdQueue2 = NULL;
    cl_command_queue cmdQueue3 = NULL;


public:
    /*
    m_pAudioVR->init(room, dlg.nFiles, dlg.waveFileNames, dlg.convolutionLength, dlg.bufferSize,
    dlg.useGPU4Conv, dlg.convDevIdx, dlg.useRTQ4Conv, dlg.cuCountConv,
    dlg.useGPU4Room, dlg.roomDevIdx, dlg.useRTQ4Room, dlg.cuCountRoom);
    */
	// Initialize room acoustics model, WASAPI audio, and TAN convolution:
    //int init(RoomDefinition roomDef, int nFiles, char **inFiles, int fftLen, int bufSize, bool useGPU_Conv = true);

    static amf::TAN_CONVOLUTION_METHOD m_convMethod;// = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;

    int init(char * dllPath, RoomDefinition roomDef, int nFiles, char **inFiles, int fftLen, int bufSize,
        bool useGPU_Conv = true, int devIdx_Conv=0, 
#ifdef RTQ_ENABLED
		bool useHPr_Conv = false, bool useRTQ_Conv = false, int cuRes_Conv = 0,
#endif // RTQ_ENABLED
        bool useGPU_IRGen = true, int devIdx_IRGen=0,  
#ifdef RTQ_ENABLED
		bool useHPr_IRGen = false, bool useRTQ_IRGen = false, int cuRes_IRGen = 0,
#endif
        amf::TAN_CONVOLUTION_METHOD &convMethod = m_convMethod, //amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD,
       bool useCPU_Conv = false, bool useCPU_IRGen = false);

	// Set transform to translate game coordinates to room audio coordinates:
	int setWorldToRoomCoordTransform(float translationX, float translationY, float translationZ, 
									 float rotationY, float headingOffset, bool headingCCW);

	// Source 1 can optionally be captured audio from default microphone:
	int setSrc1Options(bool useMicSource, bool trackHeadPos);

	// finalize, deallocate resources, close files, etc.
	int finit();

	// start audio engine: 
    int Run();

	// Stop audio engine:
    int Stop();

	// get's the current playback position in a stream:
    __int64 getCurrentPosition(int stream);

	// update the head (listener) position: 
    int updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll);

	//update a source position:
    int updateSourcePosition(int srcNumber, float x, float y, float z);

	//update a room's dimension:
	int updateRoomDimension(float _width, float _height, float _length);

	//update a room's damping factor
	int updateRoomDamping(float _left, float _right, float _top, float _buttom, float _front, float _back);

    // export impulse response for source  + current listener and room:
    int exportImpulseResponse(int srcNumber, char * fname);
	AmdTrueAudioVR* getAMDTrueAudioVR();
	TANConverterPtr getTANConverter();
};
