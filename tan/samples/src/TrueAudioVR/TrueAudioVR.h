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

#include <stdio.h>

#include "tanlibrary/include/TrueAudioNext.h"       //TAN
using namespace amf;

#include <cl/cl.h>

#ifndef AMD_TA_VR
#define AMD_TA_VR
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define PI 3.141592653589793

#define ATAL_FFT_FORWARD -1
#define ATAL_FFT_INVERSE  1
//#define FFT_LENGTH 32768

#define SPEED_OF_SOUND 343.0 // m/s
#define FILTER_SAMPLE_RATE 48000

#define DBTODAMP(dB) powf(10.0,float(-dB/20.0))
#define DAMPTODB(d) float(-20.0*log10(d))

//flags for generateRoomResponse
#define GENROOM_NONE 0
#define GENROOM_LIMIT_BOUNCES 0x40  // stop after maxBounces reflections on any axis
#define GENROOM_SUPPRESS_DIRECT 0x80  // suppress direct sound path, render only echos
#define GENROOM_USE_GPU_MEM     0x800  // create impulse responses in GPU buffer 

#define SIMPLEHRTF_FFTLEN 64

// Head Related Transfer Function
struct HeadModel {
    int filterLength;
    // head blocks wavelengths shorter the it's diameter:
    float lowPass[SIMPLEHRTF_FFTLEN];
    float highPass[SIMPLEHRTF_FFTLEN];
};

struct MonoSource {
    //speaker position:
    float speakerX, speakerY, speakerZ;
};



struct StereoListener {
    float headX, headY, headZ;
    float earSpacing;
    float yaw, pitch, roll;
    struct HeadModel hrtf;
};

struct MaterialProperty {
    // surface acoustic absorption: [required]
    float damp;			     // acoustic absorption factor [0 < damp < 1.0]  

    // Frequency response: [optional], set irLen = 0 to disable
    //int irLen;               // length of impulse response
    //float *iresponse;        // impulse response, 48kHz, normalized (will be scaled by dampdB)

    // Surface roughness:  [optional], set nHostLevels = 0 to disable
    //int nHistLevels;		 // roughnessHistogram is array representing 
    //float histHeight;		 //	fractions of surface area having heights 	
    //float *roughnessHistogram;	// in each of nHistLevels bands between 0 and histHeight 
                                             
};

// RoomDefinition:
// data structure to define dimensions and acoustic properties of a
// rectangular room. All dimensions in meters.
struct RoomDefinition {
    // room dimensions:
    float width, height, length;
    // wall material properties:
    //left wall, right wall, cieling, floor, front wall, back wall
    MaterialProperty mLeft, mRight, mTop, mBottom, mFront, mBack;
};

#define DOOR_VS_SPACING 0.01; // 1cm = 1/2 wavelength at 17 kHz

struct Door {
    float r1cX, r1cY, r1cZ;    //  center
    float r1blX, r1blY, r1blZ; //  bottom left corner
    float r1brX, r1brY, r1brZ; //  bottom right corner
    // tr = bl + 2*(c - bl)
    // tl = br + 2*(c - br)
    float r2cX, r2cY, r2cZ;    //  center
    float r2blX, r2blY, r2blZ; //  bottom left corner
    float r2brX, r2brY, r2brZ; //  bottom right corner
};

class __declspec(dllexport) AmdTrueAudioVR 
{
public:
    enum VRExecutionMode
    {
        CPU,
        GPU
    };

private:

protected:
    AmdTrueAudioVR() { };
public:

    static const float S;

    static const int localX = 4;
    static const int localY = 4;
    static const int localZ = 4;

    static const int localSizeFill = 256;

    static const int HeadFilterSize = 64;


    //AmdTrueAudioVR(TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue,
     //              float samplesPerSecond, int convolutionLength);
    virtual ~AmdTrueAudioVR() { };
    static bool useIntrinsics;

    virtual void generateRoomResponse(RoomDefinition room, MonoSource source, StereoListener ear,
        int inSampRate, int responseLength, void *responseL, void *responseR,
        int flags = 0, int maxBounces = 0) = 0;

    virtual void generateDirectResponse(RoomDefinition room, MonoSource source, StereoListener ear,
        int inSampRate, int responseLength, void *responseL, void *responseR, int *pFirstNonZero, int *pLastNonZero) = 0;

#ifdef DOORWAY_TRANSFORM
    virtual void generateDoorwayResponse(RoomDefinition room1, RoomDefinition room2,
        MonoSource source, Door door, StereoListener ear, int inSampRate, int responseLength, float *responseLeft, float *responseRight, int flags, int maxBounces) = 0;
#endif

    virtual void SetExecutionMode(VRExecutionMode executionMode) = 0;
    //{
    //    m_executionMode = executionMode;
    //}

    virtual VRExecutionMode GetExecutionMode() = 0;
    //{
    //    return m_executionMode;
    //}

    /*void SetLogFile(char* pflogname, FILE* pFile)
    {
        m_pfNmae = pflogname;
        m_fpLog = pFile;
    }*/

    /**************************************************************************************************
    AmdTrueAudio::generateSimpleHeadRelatedTransform:

    Generates a simple head related transfer function (HRTF) for acoustic shadowing as heard by
    a human ear on a human head, as a function of angle to a sound source.

    This function models the head as a sphere of diameter earSpacing * 1.10, and generates a table of
    180 impulse response curves for 1 degree increments from the direction the ear points.

    **************************************************************************************************/
    virtual void generateSimpleHeadRelatedTransform(HeadModel * pHead, float earSpacing) = 0;

    virtual void applyHRTF(HeadModel * pHead, float scale, float *response, int length, float earVX, float earVY, float earVZ, float srcVX, float srcVY, float srcZ) = 0;
    virtual void applyHRTFoptCPU(HeadModel * pHead, float scale, float *response, int length, float earVX, float earVY, float earVZ, float srcVX, float srcVY, float srcZ) = 0;

};


// TAN objects creation functions.
extern "C"
{

    __declspec(dllexport) AMF_RESULT __cdecl  CreateAmdTrueAudioVR(AmdTrueAudioVR **taVR, TANContextPtr pContext, TANFFTPtr pFft, cl_command_queue cmdQueue,
                                                                 float samplesPerSecond, int convolutionLength);

    __declspec(dllexport) float estimateReverbTime(RoomDefinition room, float finaldB, int *nReflections);


}

#endif // #ifndef AMD_TA_VR