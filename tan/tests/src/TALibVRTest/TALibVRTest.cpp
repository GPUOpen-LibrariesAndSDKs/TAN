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
// TALibVRTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "tanlibrary/include/TrueAudioNext.h"
#include "samples/src/TrueAudioVR/TrueAudioVR.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"

#include "FileUtility.h"
#include "wav.h"

#include <vector>
#include <iostream>
#include <cstring>

#include <memory.h>
#include <math.h>
#include <assert.h>

#if !defined(__APPLE__) && !defined(__MACOSX)
  #include <omp.h>
#endif

#ifdef _WIN32
  #include <AclAPI.h>
  #include <process.h>
#endif

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

#define DBTODAMP(dB) powf(10.0,float(-dB/20.0))
#define DAMPTODB(d) float(-20.0*log10(d))


#ifdef RTQ_ENABLED
	#define CL_DEVICE_MAX_REAL_TIME_COMPUTE_QUEUES_AMD  0x404D
	#define CL_DEVICE_MAX_REAL_TIME_COMPUTE_UNITS_AMD   0x404E
	#define CL_QUEUE_REAL_TIME_COMPUTE_UNITS_AMD        0x404f
#endif
cl_command_queue createQueue(int cu_ = 0, bool rtQueue = false) //2 //4
{
    cl_command_queue cmdQueue = NULL;
    cl_context context;
    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;
    int status;

    fprintf(stdout, "Creating %s Queue for %d compute units\n", rtQueue ? "RT" : "general", cu_);
    /*
    * Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return NULL;
    }
    if (0 < numPlatforms)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
            return NULL;
        }

        // ToDo: we need to extend this logic to read CL_PLATFORM_NAME, etc.
        // and add logic to decide which of several AMD GPUs to use.
        for (unsigned i = 0; i < numPlatforms; ++i)
        {
            char vendor[100];
            char name[100];
            status = clGetPlatformInfo(platforms[i],
                CL_PLATFORM_VENDOR,
                sizeof(vendor),
                vendor,
                NULL);

            if (status != CL_SUCCESS) {
                fprintf(stdout, "clGetPlatformInfo returned error: %d\n", status);
                return NULL;
            }

            status = clGetPlatformInfo(platforms[i],
                CL_PLATFORM_NAME,
                sizeof(name),
                name,
                NULL);


            if (!(strcmp(vendor, "Advanced Micro Devices, Inc.") == 0))
            {
                continue;
            }

            platform = platforms[i];
        }
        delete[] platforms;
    }

    if (platform == NULL) {
        fprintf(stdout, "No suitable platform found!\n");
        return NULL;
    }

    cl_context_properties cps[3] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0
    };


    // enumerate devices
    {
        // To Do: handle multi-GPU case, pick appropriate GPU/APU
        char driverVersion[100];

            // Retrieve device
        cl_uint numDevices = 0;
        clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices );
        assert( numDevices > 0 );
        if ( numDevices == 0 )
        {
            fprintf(stdout, "Cannot find any GPU devices!\n");
            return NULL;
        }
        cl_device_id* devices = new cl_device_id[ numDevices ];
        clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, numDevices, devices, &numDevices );
        clGetDeviceInfo( devices[0], CL_DRIVER_VERSION, 100, driverVersion, NULL );

        fprintf(stdout,"Driver version: %s\n", driverVersion);
        fprintf(stdout, "%d devices found:\n", numDevices);
        for (unsigned int n = 0; n < numDevices; n++) {
            char deviceName[100];
            clGetDeviceInfo(devices[n], CL_DEVICE_NAME, 100, deviceName, NULL);
            fprintf(stdout, "   GPU device %s\n", deviceName);
        }
    }

    // create the OpenCL context on a GPU device
    context = clCreateContextFromType(cps, clDeviceType, NULL, NULL, &status);
    if (context == (cl_context)0){
        fprintf(stdout, "clCreateContextFromType returned error: %d\n", status);
        return NULL;
    }

    // get the list of GPU devices associated with context
    size_t cb;
    cl_device_id *devices = 0;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
    devices = (cl_device_id*)malloc(cb);
    clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, devices, NULL);

#ifdef _WIN32
    SetEnvironmentVariableA("GPU_USE_SHARED_CONTEXT", "0");
#else
    putenv("GPU_USE_SHARED_CONTEXT=0");
#endif

    // Allocate RT-Queues
    int deviceId = 0;
    //int cu_ = 0; //2 //4
    cl_int error = 0;


    // Create a real time queue
#ifdef RTQ_ENABLED
    if (rtQueue)
    {
        const cl_queue_properties cprops[] = {
            CL_QUEUE_PROPERTIES,
            0,
            CL_QUEUE_REAL_TIME_COMPUTE_UNITS_AMD,
            static_cast<cl_queue_properties>(unsigned long long(cu_)),
			0};
        cmdQueue = clCreateCommandQueueWithProperties(context, devices[deviceId], cprops, &error);
    }
    else {
        const cl_queue_properties cprops[1] = {0};
        cmdQueue = clCreateCommandQueueWithProperties(context, devices[deviceId], cprops, &error);
    }
#else

    #if CL_TARGET_OPENCL_VERSION >= 200
	  const cl_queue_properties cprops[1] = { 0 };
	  cmdQueue = clCreateCommandQueueWithProperties(context, devices[deviceId], cprops, &error);
    #else
      cmdQueue = clCreateCommandQueue(context, devices[deviceId], NULL, &error);
    #endif

#endif
    printf("Queue created %llX\r\n", cmdQueue);

    clReleaseDevice(devices[deviceId]);
    clReleaseContext(context);


    if (error != 0)
        fprintf(stdout, "clCreateCommandQueueWithProperties returned error: %d\n", error);

    if (cmdQueue != NULL) {
        fprintf(stdout, "RT Queue created.\n");
    }
    else {
        fprintf(stdout, "RT Queue NOT created.\n");
    }

    printf("Queue created %llX\r\n", cmdQueue);

    return cmdQueue;
}

/*
Very simple XML parser
bool findElement(char **start, char **end, char *name)
finds one XML element between start and end
*/
bool findElement(char **start, char **end, char *name)
{
    bool found = false;
    char *p = *start;
    while (p < *end){
        if (*p == '<')
        {
            if (strncmp(p + 1, name, strlen(name)) == 0){
                *start = p++;
                int nestcount = 0;
                while (p < *end){
                    if (p[0] == '<')
                    {
                        if (p[1] == '/')
                        {
                            if (strncmp(p + 2, name, strlen(name)) == 0)
                            {
                                while (p < *end)
                                {
                                    if (*p++ == '>')
                                        break;
                                }
                                *end = p;
                                found = true;
                            }
                            ++p;
                        }
                        ++nestcount;
                    }
                    else if (p[0] == '/' && p[1] == '>' && nestcount == 0){
                        p += 2;
                        *end = p;
                        found = true;
                    }
                    ++p;
                }
            }
            else {
                while (p < *end && *p != '>') {
                    ++p;
                }
            }
        }
        else {
            ++p;
        }
    }
    return found;
}

/*
Very simple XML parser
bool parseElement(char *start, char *end, struct element *elem)
Parses [recursively] one XML element.

*/

bool parseElement(char *start, char *end, struct element *elem)
{
    bool ok = false;
    start += strlen(elem->name) + 1;

    // parse attributes
    for (int j = 0; j < elem->nAttribs; j++){
        char *pName = elem->attriblist[j].name;
        int len = static_cast<int>(strlen(pName));
        char *p = start;
        while (p++ < end){
            if (strncmp(p, pName, len) == 0){
                p += len;
                while (p < end){
                    if (*p++ == '=')
                        break;
                }
                while (p < end){
                    if (*p++ == '\"')
                        break;
                }
                switch (elem->attriblist[j].fmt) {
                case 'f':
                    sscanf_s(p, "%f", (float *)elem->attriblist[j].value);
                    break;
                case 'i':
                    sscanf_s(p, "%d", (int *)elem->attriblist[j].value);
                    break;
                case 's':
                {
                            char *sv = (char *)elem->attriblist[j].value;
                            while (p < end && *p != '\"'){
                                *sv++ = *p++;
                            }
                            *sv = '\0';
                }
                    break;
                }
            }
        }
    }

    //parse included elements
    for (int i = 0; i < elem->nElements; i++){
        char *s, *e;
        s = start;
        e = end;
        if (findElement(&s, &e, elem->elemList[i].name)){
            ok = parseElement(s, e, &elem->elemList[i]);
        }
    }

    return ok;
}


RoomDefinition room;
StereoListener listener;
MonoSource source;

int convolutionLength;
int bufferSize;
int useGPU;


void LoadParameters(char * xmlFileName)
{
    room.mLeft.damp = room.mRight.damp = room.mFront.damp = room.mBack.damp = room.mTop.damp = room.mBottom.damp = 0.0;

    struct attribute srcPosAttribs[3] = { { "X", &source.speakerX, 'f' }, { "Y", &source.speakerY, 'f' }, { "Z", &source.speakerZ, 'f' } };
    struct attribute headPosAttribs[6] = { { "X", &listener.headX, 'f' }, { "Y", &listener.headY, 'f' }, { "Z", &listener.headZ, 'f' },
    { "yaw", &listener.yaw, 'f' }, { "pitch", &listener.pitch, 'f' }, { "roll", &listener.roll, 'f' } };
    struct attribute roomDimAttribs[3] = { { "width", &room.width, 'f' }, { "height", &room.height, 'f' }, { "length", &room.length, 'f' } };
    struct attribute roomDampAttribs[6] =
    { { "left", &room.mLeft.damp, 'f' }, { "right", &room.mRight.damp, 'f' },
    { "front", &room.mFront.damp, 'f' }, { "back", &room.mBack.damp, 'f' },
    { "top", &room.mTop.damp, 'f' }, { "bottom", &room.mBottom.damp, 'f' } };

    struct attribute earAttribs[1] = { { "S", &listener.earSpacing, 'f' } };
    struct attribute convCfgAttribs[3] = {
        { "length", &convolutionLength, 'i' },
        { "buffersize", &bufferSize, 'i' },
        { "useGPU", &useGPU, 'i' } };

    struct element src1[1] = {
        { "position1", 3, srcPosAttribs, 0, NULL }
    };

    struct element headPos[2] = {
        { "position3", 6, headPosAttribs, 0, NULL },
        { "earSpacing", 1, earAttribs, 0, NULL },
    };

    struct element roomElems[2] = {
        { "dimensions", 3, roomDimAttribs, NULL },
        { "damping", 6, roomDampAttribs, NULL }
    };

    struct element convElems[1] = {
        { "configuration", 3, convCfgAttribs, 0, NULL }
    };

    struct element RAelementList[4] =
    {
        { "Source", 0, NULL, sizeof(src1) / sizeof(element), src1 },
        { "Listener", 0, NULL, sizeof(headPos) / sizeof(element), headPos },
        { "Room", 0, NULL, sizeof(roomElems) / sizeof(element), roomElems },
        { "Convolution", 0, NULL, sizeof(convElems) / sizeof(element), convElems }
    };

    struct element RoomAcoustics = { "RoomAcoustics", 0, NULL, sizeof(RAelementList) / sizeof(element), RAelementList };

    // read xml file
    FILE *fpLoadFile = NULL;

    fopen_s(&fpLoadFile,xmlFileName, "r");

    if (fpLoadFile == NULL){
        return;
    }

    fseek(fpLoadFile, 0, SEEK_END);
    int fLen = ftell(fpLoadFile);
    fseek(fpLoadFile, 0, SEEK_SET);

    char *xmlBuf = (char *)calloc(fLen, 1);
    fread(xmlBuf, 1, fLen, fpLoadFile);

    char *start, *end;
    start = xmlBuf;
    end = start + fLen;
    parseElement(start, end, &RoomAcoustics);

    room.mLeft.damp = DBTODAMP(room.mLeft.damp);
    room.mRight.damp = DBTODAMP(room.mRight.damp);
    room.mFront.damp = DBTODAMP(room.mFront.damp);
    room.mBack.damp = DBTODAMP(room.mBack.damp);
    room.mTop.damp = DBTODAMP(room.mTop.damp);
    room.mBottom.damp = DBTODAMP(room.mBottom.damp);

}

/* Store the difference between two channels in result
*/
float * getChannelDiff(float * cpuResponse, float * gpuResponse, float * result, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        result[i] = cpuResponse[i] - gpuResponse[i];
		float c = cpuResponse[i];
		float g = gpuResponse[i];
		float diff = c - g;
    }
    return result;
}

/* Determine if the difference of results is different enough to be distinguishable
 * Currently returns false if they are non-identical
*/
bool responsesMatch(float ** responseDiff, int length, float delta)
{
    float error = 0.0f;
    bool match[2] = { 1, 1 };
    int i, j;
    for (j = 0; j < 2; j++)
    {
        error = 0;
        for (i = 0; i < length; i++)
        {
            error += responseDiff[j][i] * responseDiff[j][i];
        }

        if (error / length > delta)
        {
            return false;
        }
    }
    return true;
}

#define RETURN_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) return -1; }
#define RETURN_IF_FALSE(x) { if (!(x)) return -1; }

void mangleFileName(
    char *resFileName,
    size_t resFileNameLen,
    const char *baseFileName,
    const char *suffix
)
{
    if (strchr(baseFileName, '.'))
    {
        std::strncpy(resFileName, /*resFileNameLen,*/ baseFileName, strchr(baseFileName, '.') - baseFileName);
        std::strncat(resFileName, suffix, resFileNameLen);
        std::strncat(resFileName, strchr(baseFileName, '.'), resFileNameLen);
    }
    else {
        std::strncpy(resFileName, /*resFileNameLen,*/ baseFileName, strlen(baseFileName) + 1);
        std::strncat(resFileName, suffix, resFileNameLen);
    }
}

int main(int argc, char* argv[])
{
	errno_t errno;
	FILE *fpLog = NULL;
	char logfname[] = "TALibVRTestLog.html";

	char *xmlFileName, *outFileName;

	int SamplesPerSec = 48000;
	int BitsPerSample = 16;
	int NChannels = 2;

	bool gpu = 0;
	bool test = 0;
	float *pfResponses[2];
	// Created for when 'test' is called
	float *pfResponsesT[2];
	float *pfResponsesDiff[2];
	if (argc < 3){
		puts("syntax:\n");
		puts("TALibVRTest <gpu | cpu | test> xmlfile outWavFile \n");
		return(0);
	}
	if ((strcmp(argv[1], "gpu") != 0) && (strcmp(argv[1], "cpu") != 0) && (strcmp(argv[1], "test") != 0))
	{
		puts("syntax:\n");
		puts("TALibVRTest <gpu | cpu | test> \n");
		return(0);
	}

	gpu = (strcmp(argv[1], "gpu") == 0);
	test = (strcmp(argv[1], "test") == 0);
	if (gpu)
	{
		printf("GPU \n");
	}
	else if (test)
	{
		printf("TEST \n");
	}
	else {
		printf("CPU \n");
	}

	xmlFileName = argv[2];
	outFileName = argv[3];

	LoadParameters(xmlFileName);
	long NSamples = convolutionLength;


	TANContextPtr pContext;
	TANFFTPtr pFft, pFft2;

	RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &pContext));
	cl_command_queue cmdQueue = NULL;
	if (gpu)
	{
		cl_context cl_context1;
		cl_device_id cl_device_id1;
		cl_uint status = getDeviceAndContext(0, &cl_context1, &cl_device_id1);
		cl_command_queue command_queue1 = createQueue(cl_context1, cl_device_id1);

		RETURN_IF_FALSE(!!command_queue1);
		RETURN_IF_FAILED(pContext->InitOpenCL(command_queue1, command_queue1));
	}


	RETURN_IF_FAILED(TANCreateFFT(pContext, &pFft));

	cl_int status;
	float* zerobuffer = new float[convolutionLength];
	memset(zerobuffer, 0, convolutionLength*sizeof(float));

	float *pfResponseL = nullptr;
	float *pfResponseR = nullptr;
	float *pfResponseLT = nullptr;
	float *pfResponseRT = nullptr;
	if (gpu)
	{
		pfResponseL = (float *)clCreateBuffer(pContext->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);
		pfResponseR = (float *)clCreateBuffer(pContext->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);
		pfResponses[0] = new float[convolutionLength];
		pfResponses[1] = new float[convolutionLength];
	}
	else
	{
		pfResponseL = new float[convolutionLength];
		pfResponseR = new float[convolutionLength];
		memset(pfResponseL, 0, convolutionLength*sizeof(float));
		memset(pfResponseR, 0, convolutionLength*sizeof(float));
	}



	// Created for when 'test' arg is called: create comparable elements

	pfResponsesDiff[0] = new float[convolutionLength];
	pfResponsesDiff[1] = new float[convolutionLength];
	memset(pfResponsesDiff[0], 0, convolutionLength*sizeof(float));
	memset(pfResponsesDiff[1], 0, convolutionLength*sizeof(float));
	// Using hard-coded max-name length
	char outName[255];
	char outNameGPU[255];
	char outNameCPU[255];

	RETURN_IF_FAILED(pFft->Init());
	// Open TrueAudioVR DLL:
	AmdTrueAudioVR *taVR = NULL;
	AmdTrueAudioVR *taTVR = NULL;

#ifdef _WIN32
	HMODULE TanVrDll;
	TanVrDll = LoadLibraryA("TrueAudioVR.dll");
	typedef int  (WINAPI *CREATEVR)(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue, float samplesPerSecond, int convolutionLength);
	CREATEVR CreateAmdTrueAudioVR = nullptr;
	CreateAmdTrueAudioVR = (CREATEVR)GetProcAddress(TanVrDll, "CreateAmdTrueAudioVR");
#endif

	//taVR = new AmdTrueAudioVR(pContext, pFft, cmdQueue, static_cast<float>(SamplesPerSec), convolutionLength);
	CreateAmdTrueAudioVR(&taVR, pContext, pFft, cmdQueue, static_cast<float>(SamplesPerSec), convolutionLength);

	taVR->SetExecutionMode(gpu ? AmdTrueAudioVR::GPU : AmdTrueAudioVR::CPU);

	taVR->generateSimpleHeadRelatedTransform(&listener.hrtf, listener.earSpacing);
	taVR->generateRoomResponse(room, source, listener, SamplesPerSec, convolutionLength, pfResponseL, pfResponseR);
	if (gpu)
	{
		clEnqueueReadBuffer(cmdQueue, (cl_mem)pfResponseL, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponses[0], 0, NULL, NULL);
		clEnqueueReadBuffer(cmdQueue, (cl_mem)pfResponseR, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponses[1], 0, NULL, NULL);
	}
	else
	{
		pfResponses[0] = pfResponseL;
		pfResponses[1] = pfResponseR;
	}

	//for (int i = 0; i < convolutionLength; i++)
	//{
	//	std::cout << "Left: " << pfResponseL[i] << std::endl;
	//	std::cout << "Right: " << pfResponseR[i] << std::endl;


    if (test)
    {
        mangleFileName(outNameCPU, sizeof(outNameCPU), outFileName, "CPU");
        mangleFileName(outNameGPU, sizeof(outNameGPU), outFileName, "GPU");

		TANContextPtr pContext2;

		cl_context cl_context2;
		cl_device_id cl_device_id2;
		status = getDeviceAndContext(0, &cl_context2, &cl_device_id2);
		cl_command_queue command_queue2 = createQueue(cl_context2, cl_device_id2);

		RETURN_IF_FALSE(!!command_queue2);
		RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &pContext2));
		RETURN_IF_FAILED(pContext2->InitOpenCL(command_queue2, command_queue2));

		RETURN_IF_FAILED(TANCreateFFT(pContext2, &pFft2));
		RETURN_IF_FAILED(pFft2->Init());


		pfResponseLT = (float *)clCreateBuffer(pContext2->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);
		pfResponseRT = (float *)clCreateBuffer(pContext2->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);

		pfResponsesT[0] = new float[convolutionLength];
		pfResponsesT[1] = new float[convolutionLength];



			//taTVR = new AmdTrueAudioVR(pContext, pFft, cmdQueue, static_cast<float>(SamplesPerSec),convolutionLength);
        CreateAmdTrueAudioVR(&taTVR, pContext2, pFft2, cmdQueue, static_cast<float>(SamplesPerSec), convolutionLength);


        taTVR->SetExecutionMode(AmdTrueAudioVR::GPU);

        taTVR->generateSimpleHeadRelatedTransform(&listener.hrtf, listener.earSpacing);
        taTVR->generateRoomResponse(room, source, listener, SamplesPerSec, convolutionLength,
                                    pfResponseLT, pfResponseRT);

		clEnqueueReadBuffer(command_queue2, (cl_mem)pfResponseLT, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponsesT[0], 0, NULL, NULL);
		clEnqueueReadBuffer(command_queue2, (cl_mem)pfResponseRT, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponsesT[1], 0, NULL, NULL);


        getChannelDiff(pfResponses[0], pfResponsesT[0], pfResponsesDiff[0], convolutionLength);
        getChannelDiff(pfResponses[1], pfResponsesT[1], pfResponsesDiff[1], convolutionLength);
        bool responseMatch = responsesMatch(pfResponsesDiff, convolutionLength, 0.1f);

        // Set up for writing difference information
        if (responseMatch)
        {
            mangleFileName(outName, sizeof(outName), outFileName, "PASS");
            printf("PASS\n");
        }
        else
        {
            mangleFileName(outName, sizeof(outName), outFileName, "FAIL");
        }
    }

    if (test)
    {
        WriteWaveFileF(outName, SamplesPerSec, NChannels, BitsPerSample, NSamples, pfResponsesDiff);
        WriteWaveFileF(outNameCPU, SamplesPerSec, NChannels, BitsPerSample, NSamples, pfResponses);
        WriteWaveFileF(outNameGPU, SamplesPerSec, NChannels, BitsPerSample, NSamples, pfResponsesT);
    }
    else
    {
        WriteWaveFileF(outFileName, SamplesPerSec, NChannels, BitsPerSample, NSamples, pfResponses);
    }

    if (taVR)
    {
        delete taVR;
    }

    if (taTVR)
    {
        delete taTVR;
    }

	if (test)
	{
		clReleaseMemObject((cl_mem)pfResponseLT);
		clReleaseMemObject((cl_mem)pfResponseRT);
	}
	if (!gpu)
	{
		delete pfResponseL;
		delete pfResponseR;
	}
    pfResponseL = pfResponseR = NULL;
    pfResponseLT = pfResponseRT = NULL;

    return 0;
}

