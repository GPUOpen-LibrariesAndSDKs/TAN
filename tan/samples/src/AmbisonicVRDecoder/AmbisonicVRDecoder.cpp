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
#include "../../../tanlibrary/include/TrueAudioNext.h"
#include "../TrueAudioVR/TrueAudioVR.h"
#include "wav.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>



enum Ambi2SteroMethod {
    AMBI2STEREO_SIMPLE = 0,
    AMBI2STEREO_AMD_HRTF = 1
};

#define PI 3.1415926535897932384626433

#define ICO_HRTF_LEN 512
#define ICO_NVERTICES 20
//#define EAR_FWD_AMBI_ANGLE 45.0
#define EAR_FWD_AMBI_ANGLE 45

class Ambi2Stereo {

private:
    float *LeftResponseW, *LeftResponseX, *LeftResponseY, *LeftResponseZ;
    float *RightResponseW, *RightResponseX, *RightResponseY, *RightResponseZ;

    unsigned int responseLength;
    unsigned int method;
    float *theta, *phi;
    TANFFT *pFFT;

    void buildCompositeHRTFs(TANContext *pTANContext);

public:
    Ambi2Stereo(TANContext *pTANContext, Ambi2SteroMethod  method);
    ~Ambi2Stereo();

    void getResponses(float theta, float phi,
                      int channel,
                      unsigned int length,
                      float *Wresponse, float *Xresponse, float *Yresponse, float *Zresponse);

    int getLength()  { return(responseLength); };

    typedef struct _vertex { double x, y, z; } vertex;
    static vertex icoVertices[20];
    float *vSpkrNresponse_L[20];
    float *vSpkrNresponse_R[20];

};



// vertices of 1 m diameter icosohedron:
Ambi2Stereo::vertex Ambi2Stereo::icoVertices[20] = {
    { 0.356820546,  0,            0.934172949 },
    { 0.577350269, -0.577350269,  0.577350269 },
    {-0.577350269,  0.577350269,  0.577350269 },
    { 0,            0.934172949,  0.356820546 },
    { 0.577350269,  0.577350269,  0.577350269 },
    { 0,           -0.934172949,  0.356820546 },
    {-0.356820546,  0,            0.934172949 },
    {-0.934172949,  0.356820546,            0 },
    { 0.934172949,  0.356820546,            0 },
    {-0.577350269,  0.577350269, -0.577350269 },
    { 0,            0.934172949, -0.356820546 },
    { 0.934172949, -0.356820546,            0 },
    {-0.934172949, -0.356820546,            0 },
    { 0.356820546,  0,           -0.934172949 },
    {-0.356820546,  0,           -0.934172949 },
    { 0.577350269, -0.577350269, -0.577350269 },
    { 0,           -0.934172949, -0.356820546 },
    { 0.577350269,  0.577350269, -0.577350269 },
    {-0.577350269, -0.577350269,  0.577350269 },
    {-0.577350269, -0.577350269, -0.577350269 }
};


void Ambi2Stereo::buildCompositeHRTFs(TANContext *pTANContext)
{

    TANCreateFFT(pTANContext, &pFFT);


    responseLength = ICO_HRTF_LEN;
    float *hrtf = new float[responseLength];

    LeftResponseW = new float[responseLength];
    LeftResponseX = new float[responseLength];
    LeftResponseY = new float[responseLength];
    LeftResponseZ = new float[responseLength];
   RightResponseW = new float[responseLength];
   RightResponseX = new float[responseLength];
   RightResponseY = new float[responseLength];
   RightResponseZ = new float[responseLength];


   theta = new float[ICO_NVERTICES];
   phi = new float[ICO_NVERTICES];

   for (int n = 0; n < ICO_NVERTICES; n++){
       double x = icoVertices[n].x;
       double y = icoVertices[n].y;
       double z = icoVertices[n].z;
       theta[n] = atan2(y, x)*180.0/PI;
       phi[n] = atan(z / sqrt(x*x + y*y))*180.0/PI;

       vSpkrNresponse_L[n] = new float[responseLength];
       vSpkrNresponse_R[n] = new float[responseLength];
       memset(vSpkrNresponse_L[n], 0, responseLength*sizeof(float));
       memset(vSpkrNresponse_R[n], 0, responseLength*sizeof(float));

   }

   float p = 0.5; // Cardiod
   float W0 = p*sqrt(2.0);

#ifdef _WIN32
    HMODULE TanVrDll;
    TanVrDll = LoadLibraryA("TrueAudioVR.dll");
    typedef int  (WINAPI *CREATEVR)(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue, float samplesPerSecond, int convolutionLength);
    CREATEVR CreateAmdTrueAudioVR = nullptr;
    CreateAmdTrueAudioVR = (CREATEVR)GetProcAddress(TanVrDll, "CreateAmdTrueAudioVR");
#endif

   AmdTrueAudioVR *tanVR;
   CreateAmdTrueAudioVR(&tanVR, pTANContext, pFFT, NULL, 44100, ICO_HRTF_LEN);

    RoomDefinition room;
    MonoSource source;
    StereoListener ear;
    int inSampRate = 48000;
    void *responseL;
    void *responseR;
    int firstNZ, lastNZ;
    room.height = 1.0;
    room.length = 1.0;
    room.width = 1.0;
    room.mBack = { 1.0 };
    room.mBottom = { 1.0 };
    room.mFront = { 1.0 };
    room.mLeft = { 1.0 };
    room.mRight = { 1.0 };
    room.mTop = { 1.0 };

    ear.earSpacing = 0.16;
    ear.headX = 0.0;
    ear.headY = 0.0;
    ear.headZ = 0.0;
    ear.pitch = 0.0; // 90 ??
    ear.roll = 0.0; // 90 ??
    ear.yaw = 90.0; //180 ???
    tanVR->SetExecutionMode(AmdTrueAudioVR::VRExecutionMode::CPU);

    tanVR->generateSimpleHeadRelatedTransform(&ear.hrtf, ear.earSpacing);


   for (int i = 0; i < ICO_NVERTICES; i++){
       // note coordinate transform: ambisonic is Z up, TAN Y up.
       //source.speakerX = icoVertices[i].x;
       //source.speakerY = icoVertices[i].z;
       //source.speakerZ = -icoVertices[i].y;

       source.speakerX = cos(theta[i] * PI / 180.0);
       source.speakerZ = -sin(theta[i] * PI / 180.0);
       source.speakerY = sin(phi[i] * PI / 180.0);

       // left & right responses
       tanVR->generateDirectResponse(room, source, ear, inSampRate, responseLength, vSpkrNresponse_L[i], vSpkrNresponse_R[i], &firstNZ, &lastNZ);

   }

}

Ambi2Stereo::Ambi2Stereo(TANContext *pTANContext, Ambi2SteroMethod  decodemethod){

    method = decodemethod;
    for (int n = 0; n < ICO_NVERTICES; n++){
        vSpkrNresponse_L[n] = NULL;
        vSpkrNresponse_R[n] = NULL;
    }

    switch (method){
    case Ambi2SteroMethod::AMBI2STEREO_SIMPLE:
        responseLength = 1;
        LeftResponseW = new float[responseLength];
        LeftResponseX = new float[responseLength];
        LeftResponseY = new float[responseLength];
        LeftResponseZ = new float[responseLength];
        RightResponseW = new float[responseLength];
        RightResponseX = new float[responseLength];
        RightResponseY = new float[responseLength];
        RightResponseZ = new float[responseLength];

        break;
    case Ambi2SteroMethod::AMBI2STEREO_AMD_HRTF:

        buildCompositeHRTFs(pTANContext );

        break;
    default:
        break;
    };


}

Ambi2Stereo::~Ambi2Stereo()
{
    for (int n = 0; n < ICO_NVERTICES; n++){
        if (vSpkrNresponse_L[n] != NULL){
            delete vSpkrNresponse_L[n];
        }
        vSpkrNresponse_L[n] = NULL;
        if (vSpkrNresponse_R[n] != NULL){
            delete vSpkrNresponse_R[n];
        }
        vSpkrNresponse_R[n] = NULL;
    }
    delete LeftResponseW;
    delete LeftResponseX;
    delete LeftResponseY;
    delete LeftResponseZ;
    delete RightResponseW;
    delete RightResponseX;
    delete RightResponseY;
    delete RightResponseZ;


}

    //virtual mic
    //2D:
    // M(a,p) = p sqr(2)W +(1-p)(cos(a)X + sin(a)Y)
    //3D: ???
    // M3d(theta,phi,p) = p*sqr(2)*W +( 1 - p)*(cos(theta)cos(phi)*X + sin(theta)cos(phi)*Y + sin(phi)*Z)
    // where:
    // p = 0 => figure 8
    // p = .5 => Cardiod
    // p = 1.0 => Omnidirectional

    // The coordinate system used in Ambisonics follows the right hand rule convention with positive X pointing forwards,
    // positive Y pointing to the left and positive Z pointing upwards. Horizontal angles run anticlockwise
    // from due front and vertical angles are positive above the horizontal, negative below.


void Ambi2Stereo::getResponses(float thetaHead, float phiHead,
    int channel,
    unsigned int length,
    float *Wresponse, float *Xresponse, float *Yresponse, float *Zresponse)
{
    float earAngle = channel == 0 ? EAR_FWD_AMBI_ANGLE : -EAR_FWD_AMBI_ANGLE;

    float Xcoeff = cos((thetaHead + earAngle)*PI / 180.0) * cos(phiHead*PI / 180.0);
    float Ycoeff = sin((thetaHead + earAngle)*PI / 180.0) * cos(phiHead*PI / 180.0);
    float Zcoeff = sin(phiHead*PI / 180.0);

    float p = 0.5; // Cardiod
    //area radius 1 cardiod = (2.0 / 3.0)*PI
    //area radius 1 circle = PI
    //ratio circle / cardiod = 3 / 2
    const float scale = (3.0 / 2.0) / 20.0;

    float W0 = p*sqrt(2.0);
    float X0 = (1 - p)*Xcoeff;
    float Y0 = (1 - p)*Ycoeff;
    float Z0 = (1 - p)*Zcoeff;

    switch (method){
    case AMBI2STEREO_SIMPLE:
        Wresponse[0] = W0;
        Xresponse[0] = X0;
        Yresponse[0] = Y0;
        Zresponse[0] = Z0;
        break;
    case AMBI2STEREO_AMD_HRTF:

        W0 = p*sqrt(2.0);

        memset(Wresponse, 0, sizeof(float)*responseLength);
        memset(Xresponse, 0, sizeof(float)*responseLength);
        memset(Yresponse, 0, sizeof(float)*responseLength);
        memset(Zresponse, 0, sizeof(float)*responseLength);

        for (int n = 0; n < 20; n++){
        //for (int n = 0; n < 1; n++){
            X0 = (1 - p)*cos((thetaHead - theta[n] + earAngle)*PI / 180.0) * cos((phiHead - phi[n])*PI / 180.0);
            Y0 = (1 - p)*sin((thetaHead - theta[n] + earAngle)*PI / 180.0) * cos((phiHead - phi[n])*PI / 180.0);
            Z0 = (1 - p)*sin((phiHead - phi[n])*PI / 180.0);

            switch (channel){
            case 0:
                 for (int i = 0; i < responseLength; i++){
                     Wresponse[i] += W0*vSpkrNresponse_L[n][i] * scale;
                     Xresponse[i] += X0*vSpkrNresponse_L[n][i] * scale;
                     Yresponse[i] += Y0*vSpkrNresponse_L[n][i] * scale;
                     Zresponse[i] += Z0*vSpkrNresponse_L[n][i] * scale;
                 }
                 break;
            case 1:
                for (int i = 0; i < responseLength; i++){
                    Wresponse[i] += W0*vSpkrNresponse_R[n][i] * scale;
                    Xresponse[i] += X0*vSpkrNresponse_R[n][i] * scale;
                    Yresponse[i] += Y0*vSpkrNresponse_R[n][i] * scale;
                    Zresponse[i] += Z0*vSpkrNresponse_R[n][i] * scale;
                }
                break;
            default:
                break;
            }

            //Wresponse[0] += W0;
            //Xresponse[0] += X0;
            //Yresponse[0] += Y0;
            //Zresponse[0] += Z0;
        }

        break;
    default:
        break;
    }
}



int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf("Syntax:\n");
        printf("%s: mode infile.wav outfile.wav  HrotateSpeed VrotateSpeed  ", argv[0]);
        puts("where:");
        puts("mode = SIMPLE | ADVANCED");
        puts("HrotateSpeed, VrotateSpeed are horizontal and vertical rotation speed in degrees/second");

        return -1;
    }
    int aIdx = 1;
    Ambi2SteroMethod mode = AMBI2STEREO_SIMPLE;
    if (strncmp(argv[aIdx++], "ADVANCED", 8) == 0){
        mode = AMBI2STEREO_AMD_HRTF;
    }
    char *infile = argv[aIdx++];
    char *outfile = argv[aIdx++];
    float hRotationSpeed = 0.0;
    float vRotationSpeed = 0.0;
    float theta = 0.0;
    float phi = 0.0;
    sscanf(argv[aIdx++], "%f", &hRotationSpeed);
    sscanf(argv[aIdx++], "%f", &vRotationSpeed);

    uint16_t BitsPerSample, NChannels;
    uint32_t SamplesPerSec, NSamples;
    unsigned char *pSsamples;
    float **Samples;

    ReadWaveFile(infile, SamplesPerSec, BitsPerSample, NChannels, NSamples, &pSsamples, &Samples);

    if (NChannels != 4){
        puts("input must be 4 channel 1st order ambisonic stream of (W,X,Y,Z) \n");
        return -1;
    }

    float *W = Samples[0];
    float *X = Samples[1];
    float *Y = Samples[2];
    float *Z = Samples[3];
    float *Data[8];
    Data[0] = W;
    Data[1] = X;
    Data[2] = Y;
    Data[3] = Z;
    Data[4] = W;
    Data[5] = X;
    Data[6] = Y;
    Data[7] = Z;

    float **stereoOut = new float *[2];
    for (int n = 0; n < 2; n++){
        stereoOut[n] = new float[NSamples];
        memset(stereoOut[n], 0, NSamples*sizeof(float));
    }

    // Create TAN objects.
    TANContext *pTANContext;
    AMF_RESULT res = TANCreateContext(TAN_FULL_VERSION, &pTANContext);

    Ambi2Stereo *ambi2S = new Ambi2Stereo(pTANContext, mode);
    int length = ambi2S->getLength();


    TANConvolution *pConvolution;
    TANCreateConvolution(pTANContext, &pConvolution);
    TAN_CONVOLUTION_METHOD convMethod = TAN_CONVOLUTION_METHOD_TIME_DOMAIN; //TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
    int bufSize = length / 4;
    float *OutData[8];
    OutData[0] = new float[bufSize];
    OutData[1] = new float[bufSize];
    OutData[2] = new float[bufSize];
    OutData[3] = new float[bufSize];
    OutData[4] = new float[bufSize];
    OutData[5] = new float[bufSize];
    OutData[6] = new float[bufSize];
    OutData[7] = new float[bufSize];

    pConvolution->InitCpu(convMethod, length, bufSize, 8);


    float *leftRespW = new float[length];
    float *leftRespX = new float[length];
    float *leftRespY = new float[length];
    float *leftRespZ = new float[length];

    float *rightRespW = new float[length];
    float *rightRespX = new float[length];
    float *rightRespY = new float[length];
    float *rightRespZ = new float[length];

    ambi2S->getResponses(theta, phi, 0, length, leftRespW, leftRespX, leftRespY, leftRespZ);
    ambi2S->getResponses(theta, phi, 1, length, rightRespW, rightRespX, rightRespY, rightRespZ);

    float *Responses[8];
    Responses[0] = leftRespW;
    Responses[1] = leftRespX;
    Responses[2] = leftRespY;
    Responses[3] = leftRespZ;
    Responses[4] = rightRespW;
    Responses[5] = rightRespX;
    Responses[6] = rightRespY;
    Responses[7] = rightRespZ;

    pConvolution->UpdateResponseTD(Responses, length, nullptr, 0);

    float updTime = 0.0;
    if (length == 1){
        for (long i = 0; i < NSamples; i++){
            // convolve streams with left and right responses:
            stereoOut[0][i] = W[i] * leftRespW[0] + X[i] * leftRespX[0] + Y[i] * leftRespY[0] + Z[i] * leftRespZ[0];
            stereoOut[1][i] = W[i] * rightRespW[0] + X[i] * rightRespX[0] + Y[i] * rightRespY[0] + Z[i] * rightRespZ[0];
            float seconds = float(i) / SamplesPerSec;
            theta += hRotationSpeed*(seconds - updTime);
            phi += vRotationSpeed*(seconds - updTime);
            ambi2S->getResponses(theta, phi, 0, length, leftRespW, leftRespX, leftRespY, leftRespZ);
            ambi2S->getResponses(theta, phi, 1, length, rightRespW, rightRespX, rightRespY, rightRespZ);
            updTime = seconds;
        }
    }
    else {
        for (long i = 0; i < NSamples; i += bufSize){
            amf_size nProcessed;

            nProcessed = bufSize;
            memset(OutData[0], 0, sizeof(float)*bufSize);
            memset(OutData[1], 0, sizeof(float)*bufSize);
            memset(OutData[2], 0, sizeof(float)*bufSize);
            memset(OutData[3], 0, sizeof(float)*bufSize);
            memset(OutData[4], 0, sizeof(float)*bufSize);
            memset(OutData[5], 0, sizeof(float)*bufSize);
            memset(OutData[6], 0, sizeof(float)*bufSize);
            memset(OutData[7], 0, sizeof(float)*bufSize);

            // convolve streams with left and right responses:
            //pConvolution->Process(Data, OutData, bufSize, 0, &nProcessed);
            pConvolution->ProcessDirect(Responses, Data, OutData, bufSize, &nProcessed);


            //mix down to stereo:
            for (int j = 0; j < bufSize; j++){
                stereoOut[0][i + j] = OutData[0][j] + OutData[1][j] + OutData[2][j] + OutData[3][j];
                stereoOut[1][i + j] = OutData[4][j] + OutData[5][j] + OutData[6][j] + OutData[7][j];
                //stereoOut[0][i + j] = leftRespW[0] * Data[0][j] + leftRespX[0] * Data[1][j] + leftRespY[0] * Data[2][j] + leftRespZ[0]*Data[3][j];
                //stereoOut[1][i + j] = rightRespW[0] * Data[4][j] + rightRespX[0] * Data[5][j] + rightRespY[0] * Data[6][j] + rightRespZ[0]*Data[7][j];
            }

            float seconds = float(i) / SamplesPerSec;
            theta += hRotationSpeed*(seconds - updTime);
            phi += vRotationSpeed*(seconds - updTime);
            ambi2S->getResponses(theta, phi, 0, length, leftRespW, leftRespX, leftRespY, leftRespZ);
            ambi2S->getResponses(theta, phi, 1, length, rightRespW, rightRespX, rightRespY, rightRespZ);
            updTime = seconds;
            //pConvolution->UpdateResponseTD(Responses, length, nullptr, 0);

            for (int k = 0; k < 8; k++){
                Data[k] += nProcessed;
            }
        }
    }


    WriteWaveFileF(outfile, SamplesPerSec, 2, BitsPerSample, NSamples, stereoOut);


	return 0;
}

