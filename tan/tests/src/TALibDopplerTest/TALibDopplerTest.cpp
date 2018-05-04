// TALibVRTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory.h>
#include <process.h>
#include <math.h>

#include <vector>
#include <omp.h>

#include "tanlibrary/include/TrueAudioNext.h"
#include "samples/src/common/wav.h"
#include "samples/src/TrueAudioVR/TrueAudioVR.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
#include <iostream>

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


#define MAX_SOURCES 10
float srcX[MAX_SOURCES], srcY[MAX_SOURCES], srcZ[MAX_SOURCES];
float srcVX[MAX_SOURCES], srcVY[MAX_SOURCES], srcVZ[MAX_SOURCES];
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
        int len = (int)strlen(pName);
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

/*
void LoadParameters(char * xmlFileName)
Load parameters using Very Simple XML parser.

Notes:  Does not support duplicate elements. Attribute names must not be subsets of other names in same list.
Each tag has unique name tied to a single C++ variable.

*/
void LoadParameters(char * xmlFileName)
{
    useGPU4Conv = 0;
    useGPU4Room = 0;

    struct attribute src1PosAttribs[3] = { { "X", &srcX[0], 'f' }, { "Y", &srcY[0], 'f' }, { "Z", &srcZ[0], 'f' } };
    struct attribute src2PosAttribs[3] = { { "X", &srcX[1], 'f' }, { "Y", &srcY[1], 'f' }, { "Z", &srcZ[1], 'f' } };
    struct attribute src3PosAttribs[3] = { { "X", &srcX[2], 'f' }, { "Y", &srcY[2], 'f' }, { "Z", &srcZ[2], 'f' } };
    struct attribute src4PosAttribs[3] = { { "X", &srcX[3], 'f' }, { "Y", &srcY[3], 'f' }, { "Z", &srcZ[3], 'f' } };
    struct attribute src5PosAttribs[3] = { { "X", &srcX[4], 'f' }, { "Y", &srcY[4], 'f' }, { "Z", &srcZ[4], 'f' } };
    struct attribute src6PosAttribs[3] = { { "X", &srcX[5], 'f' }, { "Y", &srcY[5], 'f' }, { "Z", &srcZ[5], 'f' } };
    struct attribute src7PosAttribs[3] = { { "X", &srcX[6], 'f' }, { "Y", &srcY[6], 'f' }, { "Z", &srcZ[6], 'f' } };
    struct attribute src8PosAttribs[3] = { { "X", &srcX[7], 'f' }, { "Y", &srcY[7], 'f' }, { "Z", &srcZ[7], 'f' } };
    struct attribute src9PosAttribs[3] = { { "X", &srcX[8], 'f' }, { "Y", &srcY[8], 'f' }, { "Z", &srcZ[8], 'f' } };
    struct attribute src10PosAttribs[3] = { { "X", &srcX[9], 'f' }, { "Y", &srcY[9], 'f' }, { "Z", &srcZ[9], 'f' } };
 
    struct attribute src1VelAttribs[3] = { { "X", &srcVX[0], 'f' }, { "Y", &srcVY[0], 'f' }, { "Z", &srcVZ[0], 'f' } };
    struct attribute src2VelAttribs[3] = { { "X", &srcVX[1], 'f' }, { "Y", &srcVY[1], 'f' }, { "Z", &srcVZ[1], 'f' } };
    struct attribute src3VelAttribs[3] = { { "X", &srcVX[2], 'f' }, { "Y", &srcVY[2], 'f' }, { "Z", &srcVZ[2], 'f' } };
    struct attribute src4VelAttribs[3] = { { "X", &srcVX[3], 'f' }, { "Y", &srcVY[3], 'f' }, { "Z", &srcVZ[3], 'f' } };
    struct attribute src5VelAttribs[3] = { { "X", &srcVX[4], 'f' }, { "Y", &srcVY[4], 'f' }, { "Z", &srcVZ[4], 'f' } };
    struct attribute src6VelAttribs[3] = { { "X", &srcVX[5], 'f' }, { "Y", &srcVY[5], 'f' }, { "Z", &srcVZ[5], 'f' } };
    struct attribute src7VelAttribs[3] = { { "X", &srcVX[6], 'f' }, { "Y", &srcVY[6], 'f' }, { "Z", &srcVZ[6], 'f' } };
    struct attribute src8VelAttribs[3] = { { "X", &srcVX[7], 'f' }, { "Y", &srcVY[7], 'f' }, { "Z", &srcVZ[7], 'f' } };
    struct attribute src9VelAttribs[3] = { { "X", &srcVX[8], 'f' }, { "Y", &srcVY[8], 'f' }, { "Z", &srcVZ[8], 'f' } };
    struct attribute src10VelAttribs[3] = {{ "X", &srcVX[9], 'f' }, { "Y", &srcVY[9], 'f' }, { "Z", &srcVZ[9], 'f' } };


    struct attribute src1MicAttribs[3] = { { "enableMic", &src1EnableMic, 'i' }, { "trackHeadPos", &src1TrackHeadPos, 'i' }, { "muteDirectPath", &src1MuteDirectPath, 'i' } };
    struct attribute headPosAttribs[6] = { { "X", &headX, 'f' }, { "Y", &headY, 'f' }, { "Z", &headZ, 'f' },
    { "yaw", &yaw, 'f' }, { "pitch", &pitch, 'f' }, { "roll", &roll, 'f' } };
    struct attribute roomDimAttribs[3] = { { "width", &roomWidth, 'f' }, { "height", &roomHeight, 'f' }, { "length", &roomLength, 'f' } };
    struct attribute roomDampAttribs[6] =
    { { "left", &roomDampLeft, 'f' }, { "right", &roomDampRight, 'f' },
    { "front", &roomDampFront, 'f' }, { "back", &roomDampBack, 'f' },
    { "top", &roomDampTop, 'f' }, { "bottom", &roomDampBottom, 'f' } };

    struct attribute roomRenderAttribs[2] = { { "nSources", &nFiles, 'i' }, { "withGPU", &useGPU4Room, 'i' } };

    struct attribute earAttribs[1] = { { "S", &earSpacing, 'f' } };
    struct attribute stream1Attribs[1] = { { "file", &waveFileNames[0][0], 's' } };
    struct attribute stream2Attribs[2] = { { "on", &srcEnable[1], 'i' }, { "file", &waveFileNames[1][0], 's' } };
    struct attribute stream3Attribs[2] = { { "on", &srcEnable[2], 'i' }, { "file", &waveFileNames[2][0], 's' } };
    struct attribute stream4Attribs[2] = { { "on", &srcEnable[3], 'i' }, { "file", &waveFileNames[3][0], 's' } };
    struct attribute stream5Attribs[2] = { { "on", &srcEnable[4], 'i' }, { "file", &waveFileNames[4][0], 's' } };
    struct attribute stream6Attribs[2] = { { "on", &srcEnable[5], 'i' }, { "file", &waveFileNames[5][0], 's' } };
    struct attribute stream7Attribs[2] = { { "on", &srcEnable[6], 'i' }, { "file", &waveFileNames[6][0], 's' } };
    struct attribute stream8Attribs[2] = { { "on", &srcEnable[7], 'i' }, { "file", &waveFileNames[7][0], 's' } };
    struct attribute stream9Attribs[2] = { { "on", &srcEnable[8], 'i' }, { "file", &waveFileNames[8][0], 's' } };
    struct attribute stream10Attribs[2] = { { "on", &srcEnable[9], 'i' }, { "file", &waveFileNames[9][0], 's' } };


    struct attribute convCfgAttribs[3] = {
        { "length", &convolutionLength, 'i' },
        { "buffersize", &bufferSize, 'i' },
        { "useGPU", &useGPU4Conv, 'i' },
    };


    struct element src1[4] = {
        { "streamS1", 1, stream1Attribs, 0, NULL },
        { "microphone", 3, src1MicAttribs, 0, NULL },
        { "positionS1", 3, src1PosAttribs, 0, NULL },
        { "velocityS1", 3, src1VelAttribs, 0, NULL }
    };
    struct element src2[3] = {
        { "streamS2", 2, stream2Attribs, 0, NULL },
        { "positionS2", 3, src2PosAttribs, 0, NULL },
        { "velocityS2", 3, src2VelAttribs, 0, NULL }
    };

    struct element src3[3] = {
        { "streamS3", 2, stream3Attribs, 0, NULL },
        { "positionS3", 3, src3PosAttribs, 0, NULL },
        { "velocityS3", 3, src3VelAttribs, 0, NULL }
    };
    struct element src4[3] = {
        { "streamS4", 2, stream4Attribs, 0, NULL },
        { "positionS4", 3, src4PosAttribs, 0, NULL },
        { "velocityS4", 3, src4VelAttribs, 0, NULL }
    };
    struct element src5[3] = {
        { "streamS5", 2, stream5Attribs, 0, NULL },
        { "positionS5", 3, src5PosAttribs, 0, NULL },
        { "velocityS5", 3, src5VelAttribs, 0, NULL }
    };
    struct element src6[3] = {
        { "streamS6", 2, stream6Attribs, 0, NULL },
        { "positionS6", 3, src6PosAttribs, 0, NULL },
        { "velocityS6", 3, src6VelAttribs, 0, NULL }
    };
    struct element src7[3] = {
        { "streamS7", 2, stream7Attribs, 0, NULL },
        { "positionS7", 3, src7PosAttribs, 0, NULL },
        { "velocityS7", 3, src7VelAttribs, 0, NULL }
    };
    struct element src8[3] = {
        { "streamS8", 2, stream8Attribs, 0, NULL },
        { "positionS8", 3, src8PosAttribs, 0, NULL },
        { "velocityS8", 3, src8VelAttribs, 0, NULL }
    };
    struct element src9[3] = {
        { "streamS9", 2, stream9Attribs, 0, NULL },
        { "positionS9", 3, src9PosAttribs, 0, NULL },
        { "velocityS9", 3, src9VelAttribs, 0, NULL }
    };
    struct element src10[10] = {
        { "streamS10", 2, stream10Attribs, 0, NULL },
        { "positionS10", 3, src10PosAttribs, 0, NULL },
        { "velocityS10", 3, src10VelAttribs, 0, NULL }
    };


    struct element head[2] = {
        { "positionL1", 6, headPosAttribs, 0, NULL },
        { "earSpacing", 1, earAttribs, 0, NULL },
    };

    struct element roomElems[3] = {
        { "dimensions", 3, roomDimAttribs, NULL },
        { "damping", 6, roomDampAttribs, NULL },
        { "rendering", 2, roomRenderAttribs, NULL }
    };

    struct element convElems[1] = {
        { "configuration", 3, convCfgAttribs, 0, NULL }
    };

    struct element RAelementList[13] =
    {
        { "Source1", 0, NULL, sizeof(src1) / sizeof(element), src1 },
        { "Source2", 0, NULL, sizeof(src2) / sizeof(element), src2 },
        { "Source3", 0, NULL, sizeof(src3) / sizeof(element), src3 },
        { "Source4", 0, NULL, sizeof(src4) / sizeof(element), src4 },
        { "Source5", 0, NULL, sizeof(src5) / sizeof(element), src5 },
        { "Source6", 0, NULL, sizeof(src6) / sizeof(element), src6 },
        { "Source7", 0, NULL, sizeof(src7) / sizeof(element), src7 },
        { "Source8", 0, NULL, sizeof(src8) / sizeof(element), src8 },
        { "Source9", 0, NULL, sizeof(src9) / sizeof(element), src9 },
        { "Source10", 0, NULL, sizeof(src10) / sizeof(element), src10 },


        { "Listener", 0, NULL, sizeof(head) / sizeof(element), head },
        { "Room", 0, NULL, sizeof(roomElems) / sizeof(element), roomElems },
        { "Convolution", 0, NULL, sizeof(convElems) / sizeof(element), convElems }
    };

    struct element RoomAcoustics = { "RoomAcoustics", 0, NULL, sizeof(RAelementList) / sizeof(element), RAelementList };

    // read xml file
    FILE *fpLoadFile = NULL;

    fopen_s(&fpLoadFile, xmlFileName, "r+");

    if (fpLoadFile == NULL){
        return;
    }

    fseek(fpLoadFile, 0, SEEK_END);
    int fLen = ftell(fpLoadFile);
    fseek(fpLoadFile, 0, SEEK_SET);

    char *xmlBuf = (char *)calloc(fLen, 1);
    if (!xmlBuf) return;

    fread(xmlBuf, 1, fLen, fpLoadFile);

    char *start, *end;
    start = xmlBuf;
    end = start + fLen;
    parseElement(start, end, &RoomAcoustics);

    fclose(fpLoadFile);

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

        strncpy_s(resFileName, resFileNameLen, baseFileName, strchr(baseFileName, '.') - baseFileName);
        strcat_s(resFileName, resFileNameLen, suffix);
        strcat_s(resFileName, resFileNameLen, strchr(baseFileName, '.'));
    }
    else {
        strncpy_s(resFileName, resFileNameLen, baseFileName, strlen(baseFileName) + 1);
        strcat_s(resFileName, resFileNameLen, suffix);
    }
}

int main(int argc, char* argv[])
{
	errno_t errno;
	FILE *fpLog = NULL;
	char logfname[] = "TALibDopplerLog.html";

	char *xmlFileName, *inFileName, *outFileName;

    waveFileNames[0] = new char[MAX_PATH + 4];

	int SamplesPerSec = 48000;
	int BitsPerSample = 16;
	int NChannels = 2;
    long NSamples = 0;
    unsigned char *pSamples = NULL;
    float **pfSamples = NULL;

	float *pfResponses[2];

    unsigned int n_samples = 16;
    amf_size nSamplesProcessed = 0;
    unsigned int nSamplesToProcess = 64;

    // #include <crtdbg.h>
    //_CrtDumpMemoryLeaks();

    if (argc < 4){
		puts("syntax:\n");
		puts("TALibDopplerTest roomDef inWavFile outWavFile [maxbounces] [CPU | GPU]\n");
        puts("where: \n");
        puts("\n");
        puts("roomDef is an xml file defining the source and listener positions, room dimensions and acoustic properties.\n");
        puts("inWavFile, outWavFile are .wav format input and output files\n");
        puts("maxbounces controls max number of echos. N Echos = 6 ^ maxbounces. Default is zero.\n");
        puts("CPU | GPU - select CPU or GPU for processing. Default: CPU\n");
        return(0);
	}

	xmlFileName = argv[1];
    inFileName = argv[2];
	outFileName = argv[3];
    int maxbounces = 0;
    bool gpu = false; 

    if (argc >= 5){
        sscanf(argv[4], "%d", &maxbounces);
    }

    if (argc >= 6){
        if (strnicmp(argv[5], "GPU",3) == 0){
            gpu = true;
        }
    }

	LoadParameters(xmlFileName);

    RoomDefinition room;
    memset(&room, 0, sizeof(room));
    room.length = roomLength;
    room.width = roomWidth;
    room.height = roomHeight;
    room.mFront.damp = DBTODAMP(roomDampFront);
    room.mBack.damp = DBTODAMP(roomDampBack);
    room.mLeft.damp = DBTODAMP(roomDampLeft);
    room.mRight.damp = DBTODAMP(roomDampRight);
    room.mTop.damp = DBTODAMP(roomDampTop);
    room.mBottom.damp = DBTODAMP(roomDampBottom);

    MonoSource src;
    src.speakerX = srcX[0];
    src.speakerY = srcY[0];
    src.speakerZ = srcZ[0];
    StereoListener ears;
    ears.earSpacing = earSpacing;
    ears.headX = headX;
    ears.headY = headY;
    ears.headZ = headZ;
    ears.pitch = pitch;
    ears.roll = roll;
    ears.yaw = yaw;


    ReadWaveFile(inFileName, &SamplesPerSec, &BitsPerSample, &NChannels, &NSamples, &pSamples, &pfSamples);

    float *pfOutput[2];
    pfOutput[0] = new float[NSamples];
    pfOutput[1] = new float[NSamples];
    memset(pfOutput[0], 0, NSamples*sizeof(float));
    memset(pfOutput[1], 0, NSamples*sizeof(float));



	TANContextPtr pContext;
	TANFFTPtr pFft, pFft2;

	RETURN_IF_FAILED(TANCreateContext(TAN_FULL_VERSION, &pContext));
    cl_command_queue cmdQueue = NULL;
    
    if (gpu)
	{
		cl_context cl_context1;
		cl_device_id cl_device_id1;
		cl_uint status = getDeviceAndContext(0, &cl_context1, &cl_device_id1);
        cmdQueue = createQueue(cl_context1, cl_device_id1);
		
        RETURN_IF_FALSE(!!cmdQueue);
        RETURN_IF_FAILED(pContext->InitOpenCL(cmdQueue, cmdQueue));
	}
    

	RETURN_IF_FAILED(TANCreateFFT(pContext, &pFft));

	cl_int status;
	float* zerobuffer = new float[convolutionLength];
	memset(zerobuffer, 0, convolutionLength*sizeof(float));

	float *pfResponseL = nullptr;
	float *pfResponseR = nullptr;
    /*
    if (gpu)
	{
		pfResponseL = (float *)clCreateBuffer(pContext->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);
		pfResponseR = (float *)clCreateBuffer(pContext->GetOpenCLContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, convolutionLength * sizeof(float), zerobuffer, &status);
		pfResponses[0] = new float[convolutionLength];
		pfResponses[1] = new float[convolutionLength];
	}
	else
*/
    {
		pfResponseL = new float[convolutionLength];
		pfResponseR = new float[convolutionLength];
		memset(pfResponseL, 0, convolutionLength*sizeof(float));
		memset(pfResponseR, 0, convolutionLength*sizeof(float));
	}

	
	RETURN_IF_FAILED(pFft->Init());
	// Open TrueAudioVR DLL:
	AmdTrueAudioVR *taVR = NULL;
	AmdTrueAudioVR *taTVR = NULL;
	HMODULE TanVrDll;
	TanVrDll = LoadLibraryA("TrueAudioVR.dll");
	typedef int  (WINAPI *CREATEVR)(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue, float samplesPerSecond, int convolutionLength);
	CREATEVR CreateAmdTrueAudioVR = nullptr;
	CreateAmdTrueAudioVR = (CREATEVR)GetProcAddress(TanVrDll, "CreateAmdTrueAudioVR");

	CreateAmdTrueAudioVR(&taVR, pContext, pFft, cmdQueue, static_cast<float>(SamplesPerSec), convolutionLength);

	taVR->SetExecutionMode(gpu ? AmdTrueAudioVR::GPU : AmdTrueAudioVR::CPU);

	taVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);
	//taVR->generateRoomResponse(room, src, ears, SamplesPerSec, convolutionLength, pfResponseL, pfResponseR);

	//if (gpu)
	//{
	//	clEnqueueReadBuffer(cmdQueue, (cl_mem)pfResponseL, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponses[0], 0, NULL, NULL);
	//	clEnqueueReadBuffer(cmdQueue, (cl_mem)pfResponseR, CL_TRUE, 0, convolutionLength * sizeof(float), pfResponses[1], 0, NULL, NULL);
	//}
	//else
	{
		pfResponses[0] = pfResponseL;
		pfResponses[1] = pfResponseR;
	}


    TANConvolutionPtr convCPU = NULL;
    TANConvolutionPtr convGPU = NULL;


    TANCreateConvolution(pContext, &convCPU);
    TANCreateConvolution(pContext, &convGPU);

    convCPU->Init(TAN_CONVOLUTION_METHOD_TIME_DOMAIN, convolutionLength, n_samples, NChannels);

    int nzFL[4] = { 0, convolutionLength, 0, convolutionLength };

    //hack
    if (NSamples > 5 * SamplesPerSec){
        NSamples = 5 * SamplesPerSec;
    }
    //src.speakerX = 101.0; // 50.0;

    unsigned int nIter = NSamples / nSamplesToProcess;

    float *ppOut[2];
    ppOut[0] = pfOutput[0];
    ppOut[1] = pfOutput[1];

    // mono input
    pfSamples[1] = pfSamples[0];



    //float speed = 100.0; // km/h
    //float dx = (speed * 1000 / 3600)*((float)nSamplesToProcess/(float)SamplesPerSec);

    float dx = srcVX[0] * ((float)nSamplesToProcess / (float)SamplesPerSec);
    int firstNonZero = 0;
    int lastNonZero = 0;
    memset(pfResponseL, 0, convolutionLength*sizeof(float));
    memset(pfResponseR, 0, convolutionLength*sizeof(float));

    for (unsigned int idx = 0; idx < nIter; idx++) {
        //memset(pfResponseL, 0, convolutionLength*sizeof(float));
        //memset(pfResponseR, 0, convolutionLength*sizeof(float));
        memset(pfResponseL + firstNonZero, 0, (lastNonZero - firstNonZero)*sizeof(float));
        memset(pfResponseR + firstNonZero, 0, (lastNonZero - firstNonZero)*sizeof(float));
        firstNonZero = convolutionLength;
        lastNonZero = 0;
        if (maxbounces > 0){
            taVR->generateRoomResponse(room, src, ears, SamplesPerSec, convolutionLength, pfResponseL, pfResponseR, GENROOM_LIMIT_BOUNCES, maxbounces);
            //find first and last non zero response values 
            for (int k = 0; k < convolutionLength; k++){
                if (pfResponseL[k] != 0.0) {
                    firstNonZero = k;
                    break;
                }
            }
            for (int k = convolutionLength-1; k  >= 0; k--){
                if (pfResponseL[k] != 0.0) {
                    lastNonZero = k;
                    break;
                }
            }
            for (int k = 0; k < convolutionLength; k++){
                if (pfResponseR[k] != 0.0) {
                    firstNonZero = k;
                    break;
                }
            }
            for (int k = convolutionLength - 1; k >= 0; k--){
                if (pfResponseR[k] != 0.0) {
                    lastNonZero = k;
                    break;
                }
            }
            nzFL[0] = nzFL[2] = firstNonZero;
            nzFL[1] = nzFL[3] = lastNonZero;
        }
        else {
            taVR->generateDirectResponse(room, src, ears, SamplesPerSec, convolutionLength, pfResponseL, pfResponseR, &firstNonZero, &lastNonZero);
            nzFL[0] = nzFL[2] = firstNonZero;
            nzFL[1] = nzFL[3] = lastNonZero;
        }
        if (idx % 10 == 0) printf(".");


        src.speakerX += dx;
        convCPU->ProcessDirect(pfResponses, pfSamples, ppOut, (amf_size)nSamplesToProcess, (amf_size *)&nSamplesProcessed, &nzFL[0]);
 
        pfSamples[0] += nSamplesProcessed;
        pfSamples[1] += nSamplesProcessed;
        ppOut[0] += nSamplesProcessed;
        ppOut[1] += nSamplesProcessed;
    }

    {
        WriteWaveFileF(outFileName, SamplesPerSec, 2, BitsPerSample, NSamples, pfOutput);
    }

    if (taVR)
    {
        delete taVR;
    }

    if (taTVR)
    {
        delete taTVR;
    }


	if (!gpu)
	{
		delete pfResponseL;
		delete pfResponseR;
	}
    pfResponseL = pfResponseR = NULL;
    
    return 0;
}

