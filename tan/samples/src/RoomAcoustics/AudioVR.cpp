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

#include <time.h>
#include <process.h>
#include <AclAPI.h>
#include <stdio.h>
#include "CL/OpenCL.h"
#include "audiovr.h"
#include "gpuutils.h"

#define RETURN_IF_FAILED(x) \
{ \
    AMF_RESULT tmp = (x); \
    if (tmp != AMF_OK) { \
        ::MessageBoxA(0, #x, "Error", MB_OK); \
        return -1; \
    } \
}

#define RETURN_IF_FALSE(x) \
{ \
    bool tmp = (x); \
    if (!tmp) { \
        ::MessageBoxA(0, #x, "Error", MB_OK); \
        return -1; \
    } \
}

#define SAFE_DELETE_ARR(x) if (x) { delete[] x; x = nullptr; }


transRotMtx::transRotMtx(){
    memset(m, 0, sizeof(m));
    m[0][0] = 1.0;
    m[1][1] = 1.0;
    m[2][2] = 1.0;
}

void transRotMtx::setAngles(float yaw, float pitch, float roll)
{
    float sinY = sin(yaw * (float)PI / 180);
    float cosY = cos(yaw * (float)PI / 180);
    float sinP = sin(pitch * (float)PI / 180);
    float cosP = cos(pitch * (float)PI / 180);
    float sinR = sin(roll * (float)PI / 180);
    float cosR = cos(roll * (float)PI / 180);

    m[0][0] = cosR*cosY - sinR*sinP*sinY;
    m[0][1] = -sinR*cosP;
    m[0][2] = cosR*sinY + sinR*sinP*cosY;
    m[1][0] = sinR*cosY + cosR*sinP*sinY;
    m[1][1] = cosR*cosP;
    m[1][2] = sinR*sinY - cosR*sinP*cosY;
    m[2][0] = -cosP*sinY;
    m[2][1] = sinP;
    m[2][2] = cosP*cosY;
}

void transRotMtx::setOffset(float x, float y, float z){
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}

void transRotMtx::transform(float &X, float &Y, float &Z)
{
    float x = X;
    float y = Y;
    float z = Z;
    X = x*m[0][0] + y*m[0][1] + z*m[0][2] + m[0][3];
    Y = x*m[1][0] + y*m[1][1] + z*m[1][2] + m[1][3];
    Z = x*m[2][0] + y*m[2][1] + z*m[2][2] + m[2][3];
}


unsigned Audio3D::processThreadProc(void * ptr)
{
    Audio3D *pAudio3D = static_cast<Audio3D*>(ptr);
    return pAudio3D->processProc();
}

unsigned Audio3D::updateThreadProc(void * ptr)
{
    Audio3D *pAudio3D = static_cast<Audio3D*>(ptr);
    return pAudio3D->updateProc();
}

Audio3D::Audio3D() :
m_samplePos(0),
m_pTAVR(nullptr),
pProcessed(nullptr),
m_hProcessThread(nullptr),
m_hUpdateThread(nullptr),
running(false),
stop(false),
m_headingOffset(0.),
m_headingCCW(true)

{
	responseBuffer = NULL;
    ZeroMemory(nSamples, sizeof(nSamples));
    ZeroMemory(pBuffers, sizeof(pBuffers));
    ZeroMemory(responses, sizeof(responses));
    ZeroMemory(inputFloatBufs, sizeof(inputFloatBufs));
    ZeroMemory(outputFloatBufs, sizeof(outputFloatBufs));
}

Audio3D::~Audio3D()
{
    finit();
   
}

int Audio3D::finit(){
    // release smart pointers:
    m_spContext1 = NULL;
    m_spContext2 = NULL;
    m_spConverter = NULL;
    m_spConvolution = NULL;
    m_spFft = NULL;

    // destroy dumb pointer:
    if (m_pTAVR != NULL) {
        delete m_pTAVR;
        m_pTAVR = nullptr;
    }

    for (amf_uint id = 0; id < _countof(pBuffers); id++)
    {
        SAFE_DELETE_ARR(pBuffers[id]);
    }
    SAFE_DELETE_ARR(pProcessed);

    SAFE_DELETE_ARR(responseBuffer);
    for (int i = 0; i < MAX_SOURCES*2; i++){
        SAFE_DELETE_ARR(inputFloatBufs[i]);
        SAFE_DELETE_ARR(outputFloatBufs[i]);
    }

    return 0;
}

int Audio3D::init(  RoomDefinition roomDef, int nFiles, char **inFiles, int fftLen, int bufSize, 
                    bool useGPU_Conv, int devIdx_Conv, int useRTQ_Conv, int cuRes_Conv,
                    bool useGPU_IRGen, int devIdx_IRGen, int useRTQ_IRGen, int cuRes_IRGen)
{
    finit();

    /* # fft buffer length must be power of 2: */
    m_fftLen = 1;
    while ((int)m_fftLen < fftLen && (m_fftLen << 1) <= MAXRESPONSELENGTH){
        m_fftLen <<= 1;
    }

    m_bufSize = bufSize*4;
    m_samplePos = 0;

    if (nFiles > MAX_SOURCES)
        nFiles = MAX_SOURCES;

    m_nFiles = nFiles;

	// allocate responses in one block
	// to optimize transfer to GPU
	responseBuffer = new float[m_fftLen*nFiles*2];
    for (int idx = 0; idx < nFiles * 2; idx++){
		responses[idx] = responseBuffer + idx*m_fftLen;
        inputFloatBufs[idx] = new float[m_fftLen];
        outputFloatBufs[idx] = new float[m_fftLen];
    }

    for (int i = 0; i < nFiles * 2; i++){
        memset(responses[i], 0, sizeof(float)*m_fftLen);
        memset(inputFloatBufs[i], 0, sizeof(float)*m_fftLen);
        memset(outputFloatBufs[i], 0, sizeof(float)*m_fftLen);
    }


    memset(&room, 0, sizeof(room));

    room = roomDef;
 
    for (int idx = 0; idx < nFiles; idx++) {
        sources[idx].speakerX = 0.0;
        sources[idx].speakerY = 0.0;
        sources[idx].speakerZ = 0.0;
    }

    ears.earSpacing = float(0.16);
    ears.headX = 0.0;
    ears.headZ = 0.0;
    ears.headY = 1.75;
    ears.pitch = 0.0;
    ears.roll = 0.0;
    ears.yaw = 0.0;

    //To Do skip missing files / handle error?
    for (int idx = 0; idx < nFiles; idx++){
        Player.QueueWaveFile(inFiles[idx], &nSamples[idx], &pBuffers[idx]);
    }


    //int nSP = nSamples[0] > nSamples[1] ? nSamples[0] : nSamples[1];
    //nSP = nSamples[2] > nSP ? nSamples[2] : nSP;

    int nSP = 0;
    for (int i = 0; i < nFiles; i++){
        if (nSamples[i] > nSP) nSP = nSamples[i];
    }
    pProcessed = new unsigned char[nSP * 4];

 
    // Allocate RT-Queues
    cl_command_queue cmdQueue1 = NULL;
    cl_command_queue cmdQueue2 = NULL;
    unsigned int refCountQ1 = 0;
    unsigned int refCountQ2 = 0;

    if (useGPU_Conv) {
        cmdQueue1 = createQueue(cuRes_Conv, !!useRTQ_Conv, devIdx_Conv);

        if (cmdQueue1 == NULL){

            fprintf(stdout, "createQueue failed to create Real Time cmdQueue1\n");
            cmdQueue1 = createQueue();
            if (cmdQueue1 == NULL){
                fprintf(stdout, "createQueue failed to create General cmdQueue1\n");
            }
        }

        clGetCommandQueueInfo(cmdQueue1, CL_QUEUE_REFERENCE_COUNT, sizeof(refCountQ1), &refCountQ1, NULL);
    }

    if (useGPU_IRGen) {
        cmdQueue2 = createQueue(cuRes_IRGen, !!useRTQ_IRGen, devIdx_IRGen);

        if (cmdQueue2 == NULL){
            fprintf(stdout, "createQueue failed to create Real Time cmdQueue2\n");
            cmdQueue2 = createQueue();
            if (cmdQueue2 == NULL){
                fprintf(stdout, "createQueue failed to create General cmdQueue2\n");
            }
        }

        clGetCommandQueueInfo(cmdQueue2, CL_QUEUE_REFERENCE_COUNT, sizeof(refCountQ2), &refCountQ2, NULL);
    }

    // Create TAN objects.
    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &m_spContext1));
    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &m_spContext2));

    if (useGPU_Conv) {
        //fails in TANContextImpl:   RETURN_IF_FAILED(m_spContext1->InitOpenCL(cmdQueue2, cmdQueue1));
        RETURN_IF_FAILED(m_spContext1->InitOpenCL(cmdQueue1,cmdQueue1));
    }

    if (useGPU_IRGen) {
        RETURN_IF_FAILED(m_spContext2->InitOpenCL(cmdQueue2,cmdQueue2));
    }

    RETURN_IF_FAILED(TANCreateConverter(m_spContext1, &m_spConverter));
    RETURN_IF_FAILED(m_spConverter->Init());

    RETURN_IF_FAILED(TANCreateConvolution(m_spContext1, &m_spConvolution));
    if (!useGPU_Conv)
    {
        RETURN_IF_FAILED(m_spConvolution->InitCpu(TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD,
            m_fftLen, bufSize,
            nFiles * 2));
    }
    else {
        //RETURN_IF_FAILED(m_spConvolution->InitGpu(TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED,//TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL
        RETURN_IF_FAILED(m_spConvolution->InitGpu(TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL,
            m_fftLen, bufSize,
            nFiles * 2));
    }

    if (m_pTAVR != NULL) {
        delete(m_pTAVR);
        m_pTAVR = NULL;
    }

    RETURN_IF_FAILED(TANCreateFFT(m_spContext2, &m_spFft));
    RETURN_IF_FAILED(m_spFft->Init());


    //RETURN_IF_FAILED(TANCreateTrueAudioVR(m_spContext2, m_spFft, 48000, fftLen, &m_pTAVR));
	m_pTAVR = new AmdTrueAudioVR(m_spContext2, m_spFft, cmdQueue2, 48000, fftLen);
	//AmdTrueAudioVR(TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue,
	//	float samplesPerSecond, int convolutionLength);


    m_pTAVR->SetExecutionMode(useGPU_IRGen ? AmdTrueAudioVR::GPU : AmdTrueAudioVR::CPU);

    char title[100];
    sprintf(title, "Room: %5.1lfm W x %5.1lfm L x %5.1lfm H", room.width, room.length, room.height);

 
    // head model:
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);

    for (int idx = 0; idx < nFiles; idx++){
        m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, responses[idx*2], responses[idx*2+1]);
    }


    RETURN_IF_FAILED(m_spConvolution->UpdateResponseTD(responses, m_fftLen, nullptr, 0));

    running = false;

    return 0;
}

int Audio3D::updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
    //[ToDo] world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);
    yaw = m_headingOffset + (m_headingCCW ? yaw : -yaw);

    //Occulus yaw = 180 - yaw*180.f / (float)PI;
    //pitch = pitch*180.f / (float)PI;
    //roll =  roll*180.f / (float)PI;

    if (x == ears.headX && y == ears.headY && z == ears.headZ //) {
        && yaw == ears.yaw && pitch == ears.pitch && roll == ears.roll) {
            return 0;
    }

    ears.headX = x;
    ears.headY = y;
    ears.headZ = z;

    //source.speakerY = ears.headY; //hack

    ears.yaw = yaw;
    ears.pitch = pitch;
    ears.roll = roll;

    // Since head moved, upate all responses

    return 0;
}

int Audio3D::updateSourcePosition(int srcNumber, float x, float y, float z)
{
    if (srcNumber >= m_nFiles){
        return -1;
    }
    //[ToDo] world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);

    sources[srcNumber].speakerX = x;// +room.width / 2.0f;
    sources[srcNumber].speakerY = y;
    sources[srcNumber].speakerZ = z;// +room.length / 2.0f;

    //no head motion, only one source moved, so just regenerate it's response:
    //m_pTAVR->generateRoomResponse(room, sources[srcNumber], ears, FILTER_SAMPLE_RATE, m_fftLen, responses[srcNumber * 2], responses[srcNumber * 2 + 1]);

    // ToDo mask unchanged responses:

    return 0;
}

int Audio3D::setWorldToRoomCoordTransform(float translationX=0., float translationY=0., float translationZ=0.,
float rotationY=0., float headingOffset=0., bool headingCCW=true){

    m_mtxWorldToRoomCoords.setOffset(translationX, translationY, translationZ);
    m_mtxWorldToRoomCoords.setAngles(rotationY, 0.0, 0.0);
    m_headingOffset = headingOffset;
    m_headingCCW = headingCCW;

    return 0;
}


int Audio3D::Run()
{
    stop = false;
    // start main processing thread:
    m_hProcessThread = (HANDLE)_beginthreadex(0, 10000000, processThreadProc, this, 0, 0);
    RETURN_IF_FALSE(m_hProcessThread != (HANDLE)-1);

    // wait for processing thread to start:
    while (!running) {
        Sleep(100);
    }

    // start update processing thread:
    m_hUpdateThread = (HANDLE)_beginthreadex(0, 10000000, updateThreadProc, this, 0, 0);
    RETURN_IF_FALSE(m_hUpdateThread != (HANDLE)-1);

    return 0;
}

int Audio3D::Stop()
{
    stop = true;
    WaitForSingleObject(m_hProcessThread, INFINITE);
    WaitForSingleObject(m_hUpdateThread, INFINITE);

    CloseHandle(m_hProcessThread);
    CloseHandle(m_hUpdateThread);

    m_hProcessThread = nullptr;
    m_hUpdateThread = nullptr;

    return 0;
}


int Audio3D::process(short *pOut, short *pChan[MAX_SOURCES], int sampleCount)
{
    //hack
    //memcpy(pOut, pChan3, sampleCount);
    //return 0;

    sampleCount = sampleCount / (sizeof(short)* 2);

    for (int idx = 0; idx < m_nFiles; idx++) {
        for (int chan = 0; chan < 2; chan++){
            RETURN_IF_FAILED(m_spConverter->Convert(pChan[idx] + chan, 2, sampleCount,
                inputFloatBufs[idx*2 + chan], 1, 1.f));
        }
    }

    RETURN_IF_FAILED(m_spConvolution->Process(inputFloatBufs, outputFloatBufs, sampleCount,
                                             nullptr, nullptr));

        // ToDo:  use limiter...

    for (int idx = 2; idx < 2*m_nFiles; idx += 2) {
        for (int n = 0; n < sampleCount; n++) {
            outputFloatBufs[0][n] += outputFloatBufs[idx][n];
            outputFloatBufs[1][n] += outputFloatBufs[idx+1][n];
        }
    }

    AMF_RESULT ret = m_spConverter->Convert(outputFloatBufs[0], 1, sampleCount, pOut, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

    ret = m_spConverter->Convert(outputFloatBufs[1], 1, sampleCount, pOut + 1, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

    return 0;
}


int Audio3D::processProc()
{
    int bytesPlayed;
    int bytesRecorded;
    //bool done = false;
    //short *pWave, *pWave2, *pWave3, *pWaveStart, *pWave2Start, *pWave3Start, *pRec, *pOut;
    //int size, size2, size3;

    short *pRec, *pOut;
    short *pWaves[MAX_SOURCES];
    short *pWaveStarts[MAX_SOURCES]; 
    int sizes[MAX_SOURCES];

    int nSP = 0;

    for (int idx = 0; idx < m_nFiles; idx++){
        pWaveStarts[idx] = pWaves[idx] = (short *)(pBuffers[idx]);
        sizes[idx] = nSamples[idx] * 2; // stereo short samples
        if (nSamples[idx] > nSP) nSP = nSamples[idx];
    }

    FifoBuffer recFifo(FILTER_SAMPLE_RATE * 4);

    pRec = new  short[FILTER_SAMPLE_RATE];
    memset(pRec, 0, FILTER_SAMPLE_RATE * sizeof(short));

    pOut = new  short[FILTER_SAMPLE_RATE];
    memset(pOut, 0, FILTER_SAMPLE_RATE * 2);

    // upgrade our windows process and thread priorities:
    //SetSecurityInfo(GetCurrentProcess(), SE_WINDOW_OBJECT, PROCESS_SET_INFORMATION, 0, 0, 0, 0);
    //SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    unsigned char *pProc = pProcessed;
    running = true;

    while (!stop) { 

        if (src1EnableMic) {
            bytesRecorded = 0;
            int nRec;
            while ((nRec = recFifo.fifoLength()) < m_bufSize){
                // get some more:
                nRec = Player.wasapiRecord((unsigned char *)pRec, m_bufSize - nRec);
                recFifo.store((char *)pRec, nRec);
                //Sleep(5);
                Sleep(2);
            }

            recFifo.retrieve((char *)pRec, m_bufSize);
            // [temporarily] duck the wave audio for mic audio:
            short * wavPtr = pWaves[0];
            pWaves[0] = pRec;
            process(pOut, pWaves, m_bufSize);
            pWaves[0] = wavPtr;
        }
        else {
            process(pOut, pWaves, m_bufSize);
        }

        Sleep(0);
        int bytes2Play = m_bufSize;
        unsigned char *pData;
        pData = (unsigned char *)pOut;
        while (bytes2Play > 0) {
            bytesPlayed = Player.wasapiPlay(pData, bytes2Play, false);
            bytes2Play -= bytesPlayed;
            pData += bytesPlayed;
            //m_samplePos += bytesPlayed/4;
            Sleep(2);
        }
        bytesPlayed = m_bufSize;

        for (int idx = 0; idx<m_nFiles; idx++){
            pWaves[idx] += bytesPlayed / 2;
            if (pWaves[idx] - pWaveStarts[idx] + m_bufSize / 2 > sizes[idx]){
                pWaves[idx] = pWaveStarts[idx];
            }
        }

        //wrap out buffer to length of longest input file:
        //memcpy(pProc, pOut, bytesPlayed);
        //pProc += bytesPlayed;

        //if (pProc - pProcessed > nSP * 4)
        //    pProc = pProcessed;

        Sleep(10);
    }
    Sleep(100);


    if (!WriteWaveFileS("RoomAcousticsRun.wav", FILTER_SAMPLE_RATE, 2, 16, nSamples[0], (short *)(pProcessed))){
        puts("unable to write RoomAcousticsRun.wav");
    }
    else {
        puts("wrote output to RoomAcousticsRun.wav");
    }

    SAFE_DELETE_ARR(pOut);
    SAFE_DELETE_ARR(pRec);

    running = false;
    return 0;
}

int Audio3D::setSrc1Options(bool useMicSource, bool trackHeadPos){
    src1EnableMic = useMicSource;
    src1TrackHeadPos = trackHeadPos;
    return 0;
}

__int64 Audio3D::getCurrentPosition()
{
    return m_samplePos;
}


int Audio3D::updateProc(){

    while (running && !stop) {

        if (src1TrackHeadPos) {
            sources[0].speakerX = ears.headX;
            sources[0].speakerY = ears.headY;
            sources[0].speakerZ = ears.headZ;
        }
		// ToDo skip unchanged responses:
        for (int idx = 0; idx < m_nFiles; idx++){
                memset(responses[idx*2], 0, sizeof(float)*m_fftLen);
                memset(responses[idx*2 + 1], 0, sizeof(float)*m_fftLen);
                m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, responses[idx*2], responses[idx*2 + 1]);
        }

        AMF_RESULT ret = AMF_OK;
        do {
			// ToDo mask unchanged responses:
            ret = m_spConvolution->UpdateResponseTD(responses, m_fftLen, nullptr, 0);
        } while (ret == AMF_INPUT_FULL && running && !stop);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_INPUT_FULL);

        Sleep(20);
    }

    return 0;
}
