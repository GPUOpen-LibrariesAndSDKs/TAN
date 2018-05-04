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
#include "CL/CL.h"
#include "simpleVRaudio.h"
#include "gpuutils.h"
#include "wav.h"
#include "cpucaps.h"
#include <immintrin.h>


const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
bool Audio3D::useIntrinsics = InstructionSet::AVX() && InstructionSet::FMA();
amf::TAN_CONVOLUTION_METHOD Audio3D::m_convMethod = amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;


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
    ZeroMemory(outputMixFloatBufs, sizeof(outputMixFloatBufs));
    ZeroMemory(m_samplePos, sizeof(m_samplePos));

    for (int i = 0; i < MAX_SOURCES * 2; i++){
        oclResponses[i] = NULL;
        outputCLBufs[i] = NULL;
    }
    outputMainCLbuf = NULL;
    m_useClMemBufs = false;
    for (int i = 0; i < 2; i++)
    {
        outputMixCLBufs[i] = NULL;
        outputShortBuf = NULL;
    }
    updated = false;

}

Audio3D::~Audio3D()
{
    finit();
   
}

int Audio3D::finit(){
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

        if (oclResponses[i] == NULL) continue;
        clReleaseMemObject(oclResponses[i]);
        oclResponses[i] = NULL;
    }
    for (int i = 0; i < m_nFiles * 2; i++)
    {
        if (outputCLBufs[i] == NULL) continue;
        clReleaseMemObject(outputCLBufs[i]);
        outputCLBufs[i] = NULL;
    }
    
    if (outputMainCLbuf != NULL)
    {
        clReleaseMemObject(outputMainCLbuf);
        outputMainCLbuf = NULL;
    }
    for (int i = 0; i < 2; i++)
    {
        if (outputMixCLBufs[i] == NULL) continue;
        clReleaseMemObject(outputMixCLBufs[i]);
        outputMixCLBufs[i] = NULL;
    }
    if (outputShortBuf)
    {
        clReleaseMemObject(outputShortBuf);
        outputShortBuf = NULL;
    }
    m_useClMemBufs = false;

    // release smart pointers:
    m_spFft.Release();
    m_spConvolution.Release();
    m_spConverter.Release();
    m_spMixer.Release();
    m_spTANContext2.Release();
    m_spTANContext1.Release();

    if (cmdQueue1 != NULL){
        clReleaseCommandQueue(cmdQueue1);
    }
    if (cmdQueue2 != NULL){
        clReleaseCommandQueue(cmdQueue2);
    }
    if (cmdQueue3 != NULL && cmdQueue3 != cmdQueue2){
        clReleaseCommandQueue(cmdQueue3);
    }
    cmdQueue1 = NULL;
    cmdQueue2 = NULL;
    cmdQueue3 = NULL;

    return 0;
}

//int Audio3D::init(  RoomDefinition roomDef, int nFiles, char **inFiles, int fftLen, int bufSize, 
//                    bool useGPU_Conv)

int Audio3D::init(
	char *dllPath, 
	RoomDefinition roomDef, 
	int nFiles, 
	char **inFiles, 
	int fftLen, 
	int bufSize,
    bool useGPU_Conv, int devIdx_Conv, 
#ifdef RTQ_ENABLED
	bool useHPr_Conv,bool useRTQ_Conv, int cuRes_Conv,
#endif // RTQ_ENABLED
    bool useGPU_IRGen, int devIdx_IRGen, 
#ifdef RTQ_ENABLED
	bool useHPr_IRGen, bool useRTQ_IRGen, int cuRes_IRGen,
#endif // RTQ_ENABLED
    amf::TAN_CONVOLUTION_METHOD &convMethod,
    bool useCPU_Conv, bool useCPU_IRGen)
{
    finit();
    // shouldn't need this, they are radio buttons:
    if (useGPU_Conv) useCPU_Conv = false;
    if (useGPU_IRGen) useCPU_IRGen = false;

    //m_useCpuConv = useCPU_Conv;
    //m_useCpuIRGen = useCPU_IRGen;
    //m_useGpuConv = useGPU_Conv;
	//m_useGpuIRGen = useGPU_IRGen;


    /* # fft buffer length must be power of 2: */
    m_fftLen = 1;
    while ((int)m_fftLen < fftLen && (m_fftLen << 1) <= MAXRESPONSELENGTH){
        m_fftLen <<= 1;
    }

    m_bufSize = bufSize*sizeof(float);

    for (int i = 0; i < MAX_SOURCES; i++) {
        m_samplePos[i] = 0;
    }

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

    for (int i = 0; i < 2; i++)//Right and left channel after mixing
    {
        outputMixFloatBufs[i] = new float[m_fftLen];
        memset(outputMixFloatBufs[i], 0, sizeof(float)*m_fftLen);
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
        HRESULT res = Player.QueueWaveFile(inFiles[idx], &nSamples[idx], &pBuffers[idx]);
        if (res == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            return res;
        }
    }


    //int nSP = nSamples[0] > nSamples[1] ? nSamples[0] : nSamples[1];
    //nSP = nSamples[2] > nSP ? nSamples[2] : nSP;

    int nSP = 0;
    for (int i = 0; i < nFiles; i++){
        if (nSamples[i] > nSP) nSP = nSamples[i];
    }
    pProcessed = new unsigned char[nSP * 4];

 
    //// Allocate RT-Queues
    //cl_command_queue cmdQueue1 = NULL;
    //cl_command_queue cmdQueue2 = NULL;
    //cl_command_queue cmdQueue3 = NULL;
    cmdQueue3 = cmdQueue2 = cmdQueue1 = NULL;

    unsigned int refCountQ1 = 0;
    unsigned int refCountQ2 = 0;

    int32_t flagsQ1 = 0;
    int32_t flagsQ2 = 0;

    //if (useGPU_Conv || useGPU_IRGen)
    if (useGPU_Conv)
    {
#ifdef RTQ_ENABLED
#define QUEUE_MEDIUM_PRIORITY                   0x00010000

#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000  
        if (useHPr_Conv){
            flagsQ1 = QUEUE_MEDIUM_PRIORITY;
        }

  else if (useRTQ_Conv){
            flagsQ1 = QUEUE_REAL_TIME_COMPUTE_UNITS | cuRes_Conv;
        }  

	    if (useHPr_IRGen){
            flagsQ2 = QUEUE_MEDIUM_PRIORITY;
        }

		else if (useRTQ_IRGen){
			flagsQ2 = QUEUE_REAL_TIME_COMPUTE_UNITS | cuRes_IRGen;
		}
#endif // RTQ_ENABLED

        CreateGpuCommandQueues(devIdx_Conv, flagsQ1, &cmdQueue1, flagsQ2, &cmdQueue2);

        if (devIdx_Conv == devIdx_IRGen && useGPU_IRGen)
        {
            cmdQueue3 = cmdQueue2;
        }
    } 

    if (useCPU_Conv)
    {
#ifdef RTQ_ENABLED
		// For " core "reservation" on CPU" -ToDo test and enable
        if (cuRes_Conv > 0 && cuRes_IRGen > 0){
            cl_int err = CreateCommandQueuesWithCUcount(devIdx_Conv, &cmdQueue1, &cmdQueue2, cuRes_Conv, cuRes_IRGen);
        }
        else {
            CreateCpuCommandQueues(devIdx_Conv, 0, &cmdQueue1, 0, &cmdQueue2);
        }
#endif // RTQ_ENABLED

        if (devIdx_Conv == devIdx_IRGen && useCPU_IRGen)
        {
            cmdQueue3 = cmdQueue2;
        }
    }

    if(cmdQueue3 == NULL)
    {
        if (useGPU_IRGen)
        {
            CreateGpuCommandQueues(devIdx_IRGen, 0, &cmdQueue3, 0, NULL);
        } 
        else if (useCPU_IRGen)
        {
            CreateCpuCommandQueues(devIdx_IRGen, 0, &cmdQueue3, 0, NULL);
        }
    }

    // Create TAN objects.
    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &m_spTANContext1));

    if (useGPU_Conv || useCPU_Conv) {
        RETURN_IF_FAILED(m_spTANContext1->InitOpenCL(cmdQueue1, cmdQueue2));
    }

    RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &m_spTANContext2));
    if (useGPU_IRGen || useCPU_IRGen) {
        RETURN_IF_FAILED(m_spTANContext2->InitOpenCL(cmdQueue3, cmdQueue3));
    }

    RETURN_IF_FAILED(TANCreateConvolution(m_spTANContext1, &m_spConvolution));
    if (!useGPU_Conv && !useCPU_Conv) // don't use OpenCL at all
    {
		if (convMethod != TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD)
			convMethod = TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;			// ensures compatible with old room acoustic
        // C_Model implementation
        RETURN_IF_FAILED(m_spConvolution->InitCpu(convMethod,
            m_fftLen, bufSize,
            nFiles * 2));
    }
    else {
        RETURN_IF_FAILED(m_spConvolution->InitGpu(convMethod,//TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL
            m_fftLen, bufSize,
            nFiles * 2));
    }

    RETURN_IF_FAILED(TANCreateConverter(m_spTANContext2, &m_spConverter));
    RETURN_IF_FAILED(m_spConverter->Init());

    RETURN_IF_FAILED(TANCreateMixer(m_spTANContext2, &m_spMixer));
	RETURN_IF_FAILED(m_spMixer->Init(bufSize, m_nFiles));

    RETURN_IF_FAILED(TANCreateFFT(m_spTANContext2, &m_spFft));
    RETURN_IF_FAILED(m_spFft->Init());

    if (m_pTAVR != NULL) {
        delete(m_pTAVR);
        m_pTAVR = NULL;
    }

    m_useOCLOutputPipeline = useGPU_Conv && useGPU_IRGen;

    if (useGPU_IRGen && useGPU_Conv) {
        //if (cmdQueue3 == NULL){
        //    CreateGpuCommandQueues(devIdx_IRGen, 0, &cmdQueue3, 0, NULL);
        //}

        cl_int status;
        cl_context context_IR;
        cl_context context_Conv;

        clGetCommandQueueInfo(cmdQueue3, CL_QUEUE_CONTEXT, sizeof(cl_context), &context_IR, NULL);
        clGetCommandQueueInfo(cmdQueue2, CL_QUEUE_CONTEXT, sizeof(cl_context), &context_Conv, NULL);
        if (context_IR == context_Conv) {
            for (int i = 0; i < nFiles * 2; i++){
                oclResponses[i] = clCreateBuffer(context_IR, CL_MEM_READ_WRITE, m_fftLen * sizeof(float), NULL, &status);
            }
            //HACK out for test 
            m_useClMemBufs = true;
        }
        // Initialize CL output buffers,
        if(useGPU_Conv)
        {
            cl_int clErr;

            // First create a big cl_mem buffer then create small sub-buffers from it
            outputMainCLbuf = clCreateBuffer(m_spTANContext1->GetOpenCLContext(), CL_MEM_READ_WRITE,
                 m_bufSize * nFiles * 2, nullptr, &clErr);

            if (clErr != CL_SUCCESS)
            {
                printf("Could not create OpenCL buffer\n");
                return AMF_FAIL;
            };

            for (amf_uint32 i = 0; i < nFiles * 2; i++){
                cl_buffer_region region;
                region.origin = i*m_bufSize;
                region.size = m_bufSize;
                outputCLBufs[i] = clCreateSubBuffer(
                    outputMainCLbuf, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &clErr);

                if (clErr != CL_SUCCESS)
                {

                    printf("Could not create OpenCL subBuffer\n");
                    return AMF_FAIL;
                };
                float zero = 0.0;
                clErr = clEnqueueFillBuffer(cmdQueue1, outputCLBufs[i], &zero,sizeof(zero), 0, region.size, 0, NULL, NULL);
                if (clErr != CL_SUCCESS)
                {
                    printf("Could not fill OpenCL subBuffer\n");
                    return AMF_FAIL;
                };
            }

            for (int idx = 0; idx < 2; idx++)
            {

                outputMixCLBufs[idx] = clCreateBuffer(
                    m_spTANContext1->GetOpenCLContext(), CL_MEM_READ_WRITE, m_bufSize, nullptr, &clErr);
                if (clErr != CL_SUCCESS)
                {

                    printf("Could not create OpenCL buffer\n");
                    return AMF_FAIL;
                };
                
                if (clErr != CL_SUCCESS)
                {

                    printf("Could not create OpenCL buffer\n");
                    return AMF_FAIL;
                };
            }
            // The output short buffer stores the final (after mixing) left and right channels interleaved as short samples 
            // The short buffer size is equal to sizeof(short)*2*m_bufSize/sizeof(float) which is equal to m_bufSize
            outputShortBuf = clCreateBuffer(
                m_spTANContext1->GetOpenCLContext(), CL_MEM_READ_WRITE, m_bufSize, nullptr, &clErr);
        }
    }


    //AmdTrueAudioVR *TrueAudioVR
    HMODULE TanVrDll;
    TanVrDll = LoadLibraryA("TrueAudioVR.dll");
    typedef int  (WINAPI *CREATEVR)(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue, float samplesPerSecond, int convolutionLength);
    CREATEVR CreateAmdTrueAudioVR = nullptr;

    CreateAmdTrueAudioVR = (CREATEVR)GetProcAddress(TanVrDll, "CreateAmdTrueAudioVR");
    CreateAmdTrueAudioVR(&m_pTAVR, m_spTANContext2, m_spFft, cmdQueue3, 48000, m_fftLen);

    if (useGPU_IRGen){
        m_pTAVR->SetExecutionMode(AmdTrueAudioVR::GPU);
    }
    else {
        m_pTAVR->SetExecutionMode(AmdTrueAudioVR::CPU);
    }

    char title[100];
    sprintf(title, "Room: %5.1lfm W x %5.1lfm L x %5.1lfm H", room.width, room.length, room.height);

 
    // head model:
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);


    //To Do use gpu mem responses

    for (int idx = 0; idx < nFiles; idx++){
        if (m_useClMemBufs) {
            m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, oclResponses[idx * 2], oclResponses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES | GENROOM_USE_GPU_MEM, 50);
        }
        else {
            m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, responses[idx * 2], responses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES, 50);
        }
    }

    if (m_useClMemBufs) {
        RETURN_IF_FAILED(m_spConvolution->UpdateResponseTD(oclResponses, m_fftLen, nullptr, IR_UPDATE_MODE));
    }
    else {
        RETURN_IF_FAILED(m_spConvolution->UpdateResponseTD(responses, m_fftLen, nullptr, IR_UPDATE_MODE));
    }

    running = false;

    return 0;
}

int Audio3D::updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
    // world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);
    yaw = m_headingOffset + (m_headingCCW ? yaw : -yaw);

    if (x == ears.headX && y == ears.headY && z == ears.headZ //) {
        && yaw == ears.yaw && pitch == ears.pitch && roll == ears.roll) {
            return 0;
    }

    ears.headX = x;
    ears.headY = y;
    ears.headZ = z;

    ears.yaw = yaw;
    ears.pitch = pitch;
    ears.roll = roll;

    updateParams = true;
    return 0;
}

int Audio3D::updateSourcePosition(int srcNumber, float x, float y, float z)
{
    if (srcNumber >= m_nFiles){
        return -1;
    }
    // world to room coordinates transform:
    m_mtxWorldToRoomCoords.transform(x, y, z);

    sources[srcNumber].speakerX = x;// +room.width / 2.0f;
    sources[srcNumber].speakerY = y;
    sources[srcNumber].speakerZ = z;// +room.length / 2.0f;

    updateParams = true;
    return 0;
}

int Audio3D::updateRoomDimension(float _width, float _height, float _length)
{
	room.width = _width;
	room.height = _height;
	room.length = _length;

	updateParams = true;
	return 0;
}

int Audio3D::updateRoomDamping(float _left, float _right, float _top, float _buttom, float _front, float _back)
{
	room.mTop.damp = _top;
	room.mBottom.damp = _buttom;
	room.mLeft.damp = _left;
	room.mRight.damp = _right;
	room.mFront.damp = _front;
	room.mBack.damp = _back;
	
	updateParams = true;
	return 0;
}

AmdTrueAudioVR* Audio3D::getAMDTrueAudioVR()
{
	return m_pTAVR;
}

TANConverterPtr Audio3D::getTANConverter()
{
	return m_spConverter;
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

    updateParams = true;
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

int Audio3D::process(short *pOut, short *pChan[MAX_SOURCES], int sampleCountBytes)
{
    int sampleCount = sampleCountBytes / (sizeof(short) * 2);

    
    // Read from the files
    for (int idx = 0; idx < m_nFiles; idx++) {
        for (int chan = 0; chan < 2; chan++){
            // The way sources in inputFloatBufs are ordered is: Even indexed elements for left channels, odd indexed ones for right,
            // this ordering matches with the way impulse responses are generated and indexed to be convolved with the sources.
            RETURN_IF_FAILED(m_spConverter->Convert(pChan[idx] + chan, 2, sampleCount,
                inputFloatBufs[idx * 2 + chan], 1, 1.f));
        }
    }





    if (m_useOCLOutputPipeline) 
    {   // OCL device memory objects are passed to the TANConvolution->Process method.
        // Mixing and short conversion is done on GPU.

        RETURN_IF_FAILED(m_spConvolution->Process(inputFloatBufs, outputCLBufs, sampleCount, nullptr, nullptr));

        cl_mem outputCLBufLeft[MAX_SOURCES];
        cl_mem outputCLBufRight[MAX_SOURCES];
        for (int src = 0; src < MAX_SOURCES; src++)
        {
            outputCLBufLeft[src] = outputCLBufs[src*2 ];// Even indexed channels for left ear input
            outputCLBufRight[src] = outputCLBufs[src*2+1];// Odd indexed channels for right ear input
        }
        AMF_RESULT ret = m_spMixer->Mix(outputCLBufLeft, outputMixCLBufs[0]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spMixer->Mix(outputCLBufRight, outputMixCLBufs[1]);
        RETURN_IF_FALSE(ret == AMF_OK);
        ret = m_spConverter->Convert(outputMixCLBufs[0], 1, 0, TAN_SAMPLE_TYPE_FLOAT, 
            outputShortBuf, 2, 0, TAN_SAMPLE_TYPE_SHORT, sampleCount, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED); 

        ret = m_spConverter->Convert(outputMixCLBufs[1], 1, 0, TAN_SAMPLE_TYPE_FLOAT, 
            outputShortBuf, 2, 1, TAN_SAMPLE_TYPE_SHORT, sampleCount, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

        cl_int clErr = clEnqueueReadBuffer(m_spTANContext1->GetOpenCLGeneralQueue(), outputShortBuf, CL_TRUE,
             0, sampleCountBytes, pOut, NULL, NULL, NULL);
        RETURN_IF_FALSE(clErr == CL_SUCCESS);

    }
    else
    {   // Host memory pointers are passed to the TANConvolution->Process method
        // Mixing and short conversion are still performed on CPU.

        RETURN_IF_FAILED(m_spConvolution->Process(inputFloatBufs, outputFloatBufs, sampleCount,
            nullptr, nullptr));

        float * outputFloatBufLeft[MAX_SOURCES];
        float * outputFloatBufRight[MAX_SOURCES];
        for (int src = 0; src < MAX_SOURCES; src++)
        {
            outputFloatBufLeft[src] = outputFloatBufs[src * 2];// Even indexed channels for left ear input
            outputFloatBufRight[src] = outputFloatBufs[src * 2 + 1];// Odd indexed channels for right ear input
        }
        AMF_RESULT ret = m_spMixer->Mix(outputFloatBufLeft, outputMixFloatBufs[0]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spMixer->Mix(outputFloatBufRight, outputMixFloatBufs[1]);
        RETURN_IF_FALSE(ret == AMF_OK);

        ret = m_spConverter->Convert(outputMixFloatBufs[0], 1, sampleCount, pOut, 2, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

        ret = m_spConverter->Convert(outputMixFloatBufs[1], 1, sampleCount, pOut + 1, 2, 1.f);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);
    }


#if 0// Old code: Crossfade, Mixing and Conversion on CPU

    for (int idx = 0; idx < m_nFiles; idx++) {
        for (int chan = 0; chan < 2; chan++){
            RETURN_IF_FAILED(m_spConverter->Convert(pChan[idx] + chan, 2, sampleCount,
                inputFloatBufs[idx*2 + chan], 1, 1.f));
        }
    }

    RETURN_IF_FAILED(m_spConvolution->Process(inputFloatBufs, outputFloatBufs, sampleCount,
                                             nullptr, nullptr));

    // ToDo:  use limiter...

 
    for (int idx = 2; idx < 2 * m_nFiles; idx += 2) {
        int k = 0;
        int n = sampleCount;
        while (n >= 8 && useIntrinsics){
            register __m256 *outL, *outR, *inL, *inR;
            outL = (__m256 *)&outputFloatBufs[0][k];
            outR = (__m256 *)&outputFloatBufs[1][k];
            inL = (__m256 *)&outputFloatBufs[idx][k];
            inR = (__m256 *)&outputFloatBufs[idx + 1][k];

            *outL = _mm256_add_ps(*outL, *inL);
            *outR = _mm256_add_ps(*outR, *inR);
            k += 8;
            n -= 8;
        }
        while(n > 0) {
            outputFloatBufs[0][k] += outputFloatBufs[idx][k];
            outputFloatBufs[1][k] += outputFloatBufs[idx + 1][k];
            k++;
            n--;
        }
    }
    

    AMF_RESULT ret = m_spConverter->Convert(outputFloatBufs[0], 1, sampleCount, pOut, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);

    ret = m_spConverter->Convert(outputFloatBufs[1], 1, sampleCount, pOut + 1, 2, 1.f);
    RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_TAN_CLIPPING_WAS_REQUIRED);
#endif
    return 0;
}


int Audio3D::processProc()
{
    int bytesPlayed;
    int bytesRecorded;
 
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

    pRec = new  short[FILTER_SAMPLE_RATE*2];
    memset(pRec, 0, FILTER_SAMPLE_RATE * sizeof(short) * 2);

    pOut = new  short[FILTER_SAMPLE_RATE*2];
    memset(pOut, 0, FILTER_SAMPLE_RATE * sizeof(short) * 2);

    // upgrade our windows process and thread priorities:
    SetSecurityInfo(GetCurrentProcess(), SE_WINDOW_OBJECT, PROCESS_SET_INFORMATION, 0, 0, 0, 0);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    unsigned char *pProc = pProcessed;
    running = true;

    while (!updated){
        Sleep(5);
    }

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
        memcpy(pProc, pData, m_bufSize);

        while (bytes2Play > 0) {
            bytesPlayed = Player.wasapiPlay(pData, bytes2Play, false);
            bytes2Play -= bytesPlayed;
            pData += bytesPlayed;
            Sleep(2);
        }
        bytesPlayed = m_bufSize;

        for (int idx = 0; idx<m_nFiles; idx++){
            pWaves[idx] += bytesPlayed / 2;
            if (pWaves[idx] - pWaveStarts[idx] + m_bufSize / 2 > sizes[idx]){
                pWaves[idx] = pWaveStarts[idx];
            }
        }

        pProc += m_bufSize;
        if (pProc - pProcessed + m_bufSize > nSP){
            pProc = pProcessed;
        }

        ///compute current sample position for each stream:
        for (int i = 0; i < m_nFiles; i++) {
            m_samplePos[i] = (pWaves[i] - pWaveStarts[i]) / sizeof(short);
        }

        Sleep(1);
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

__int64 Audio3D::getCurrentPosition(int streamIdx)
{
    return m_samplePos[streamIdx];
}


int Audio3D::updateProc(){

    while (running && !stop) {
        while (!updateParams  && running && !stop){
            Sleep(10);
        }

        if (src1TrackHeadPos) {
            sources[0].speakerX = ears.headX;
            sources[0].speakerY = ears.headY;
            sources[0].speakerZ = ears.headZ;
        }

        for (int idx = 0; idx < m_nFiles; idx++){
             if (m_useClMemBufs) {
                m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, oclResponses[idx * 2], oclResponses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES | GENROOM_USE_GPU_MEM, 50);
            }
            else {
                memset(responses[idx*2], 0, sizeof(float)*m_fftLen);
                memset(responses[idx*2 + 1], 0, sizeof(float)*m_fftLen);
                m_pTAVR->generateRoomResponse(room, sources[idx], ears, FILTER_SAMPLE_RATE, m_fftLen, responses[idx * 2], responses[idx * 2 + 1], GENROOM_LIMIT_BOUNCES, 50);
            }
        }

        AMF_RESULT ret = AMF_OK;
        do {
            if (m_useClMemBufs) {
                ret = m_spConvolution->UpdateResponseTD(oclResponses, m_fftLen, nullptr, IR_UPDATE_MODE);
            }
            else {
                ret = m_spConvolution->UpdateResponseTD(responses, m_fftLen, nullptr, IR_UPDATE_MODE);
            }

        } while (ret == AMF_INPUT_FULL && running && !stop);
        RETURN_IF_FALSE(ret == AMF_OK || ret == AMF_BUSY);


        updated = true;
        //Sleep(20);
        updateParams = false;
    }

    return 0;
}

int Audio3D::exportImpulseResponse(int srcNumber, char * fileName)
{

    int convolutionLength = this->m_fftLen;
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);

    float *leftResponse = responses[0];
    float *rightResponse = responses[1];
    short *sSamples = new short[2 * convolutionLength];
     memset(sSamples, 0, 2 * convolutionLength*sizeof(short));
        
    (void)m_spConverter->Convert(leftResponse, 1, convolutionLength, sSamples, 2, 1.f);

    (void)m_spConverter->Convert(rightResponse, 1, convolutionLength, sSamples + 1, 2, 1.f);

    WriteWaveFileS(fileName, 48000, 2, 16, convolutionLength, sSamples);
    delete[] sSamples;
    return 0;
}