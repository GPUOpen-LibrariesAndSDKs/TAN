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

// Ambisonic1stOrderDecoder.cpp : Defines the entry point for the console application.
//
#include "wav.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>

int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf("Syntax:\n");
        printf("%s: infile.wav outfile.wav  theta1 phi1 theta2 phi2 ... ", argv[0]);
        puts("where:");
        puts(" thetaN, phiN are horizontal and vertical angle of speakerN");
        puts(" angles are measured from zero at forward direction");
        puts("standard 5.1 speaker mappings:");
        puts("5.1 WAV  FL , FR , FC , LFE, SL , SR  ");
        puts("5.1 AC3  FL , FC , FR , SL , SR , LFE ");
        puts("5.1 DTS  FC , FL , FR , SL , SR , LFE ");
        puts("5.1 AAC  FC , FL , FR , SL , SR , LFE ");
        puts("5.1 AIFF  FL , SL , FC , FR , SR , LFE");
        return -1;
    }

    char *infile = argv[1];
    char *outfile = argv[2];

    int nSpeakers = (argc - 3) / 2;

    float *theta = new float[nSpeakers];
    float *phi = new float[nSpeakers];

    for (int i = 0; i < nSpeakers; i++){
        sscanf(argv[3 + i * 2], "%f", &theta[i]);
        sscanf(argv[4 + i * 2], "%f", &phi[i]);

    }

    for (int n = 0; n < nSpeakers; n++){
        theta[n] *= 3.1415926535 / 180.0;
        phi[n] *= 3.1415926535 / 180.0;
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

    // standard speaker channel mappingsL
    //5.1 WAV  FL , FR , FC , LFE, SL , SR
    //5.1 AC3  FL , FC , FR , SL , SR , LFE
    //5.1 DTS  FC , FL , FR , SL , SR , LFE
    //5.1 AAC  FC , FL , FR , SL , SR , LFE
    //5.1 AIFF  FL , SL , FC , FR , SR , LFE

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

    float **spkStreams = new float *[nSpeakers];
    for (int n = 0; n < nSpeakers; n++){
        spkStreams[n] = new float[NSamples];
        memset(spkStreams[n], 0, NSamples*sizeof(float));
    }

    float p = 0.5; // Cardiod
    for (int n = 0; n < nSpeakers; n++){
        float Xcoeff = cos(theta[n]) * cos(phi[n]);
        float Ycoeff = sin(theta[n]) * cos(phi[n]);
        float Zcoeff = sin(phi[n]);

        for (int i = 0; i < NSamples; i++){
            spkStreams[n][i] = p*sqrt(2.0)*W[i] + (1 - p)*(X[i] * Xcoeff + Y[i] * Ycoeff + Z[i] * Zcoeff);
        }
    }

    WriteWaveFileF(outfile, SamplesPerSec, nSpeakers, BitsPerSample, NSamples, spkStreams);


	return 0;
}

