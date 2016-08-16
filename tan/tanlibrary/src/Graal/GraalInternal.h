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

#ifndef __AMDGRAALINTERNAL_OCL__
#define __AMDGRAALINTERNAL_OCL__

#include <CL/cl.h>
#include "graal.h"


typedef struct _wav_header {
	uint ChunkID;         // Contains the letters "RIFF" in ASCII form
                          //     (0x52494646 big-endian form).
	uint ChunkSize;       //36 + SubChunk2Size, or more precisely:
                          //     4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                          //     This is the size of the rest of the chunk 
                          //     following this number.  This is the size of the 
                          //     entire file in bytes minus 8 bytes for the
                          //     two fields not included in this count:
                          //     ChunkID and ChunkSize.
	uint Format;          //  Contains the letters "WAVE"
                          //     (0x57415645 big-endian form).

//The "WAVE" format consists of two subchunks: "fmt " and "data":
//The "fmt " subchunk describes the sound data's format:

	uint Subchunk1ID;    //   Contains the letters "fmt "
                         //      (0x666d7420 big-endian form).
	uint Subchunk1Size;   //   16 for PCM.  This is the size of the
                          //     rest of the Subchunk which follows this number.
	short AudioFormat;    //      PCM = 1 (i.e. Linear quantization)
                          //     Values other than 1 indicate some 
                          //     form of compression.
	short NumChannels;    //      Mono = 1, Stereo = 2, etc.
	uint SampleRate;      //      8000, 44100, etc.
	uint ByteRate;       //     == SampleRate * NumChannels * BitsPerSample/8
	short BlockAlign;    //     == NumChannels * BitsPerSample/8
                         //      The number of bytes for one sample including
                         //      all channels. I wonder what happens when
                         //      this number isn't an integer?
	short BitsPerSample; //    8 bits = 8, 16 bits = 16, etc.
//          2   ExtraParamSize   if PCM, then doesn't exist
//          X   ExtraParams      space for extra parameters

//The "data" subchunk contains the size of the data and the actual sound:

	uint Subchunk2ID;    //      Contains the letters "data"
                         //      (0x64617461 big-endian form).
	uint Subchunk2Size;  //    == Num
} wav_header;



__int64 ReverbOCLGetRunCounter(amdOCLRvrb rvrb);


//////////////////////////////////////////////////////////////////////////
// the same as above but data is randomly generated
///////////////////////////////////////////////////////////////////////////
int ReverbOCLSetupReverbKernelFromProc(amdOCLRvrb rvrb, const char * proc, int n_samples, int n_subchannels, int bitPersample);




///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalLoadFIRs(graalHandle rvrb, firHandle * firHandleArray, __FLOAT__ ** firPtrs, int * fir_sz, int n_firs);


///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalUnloadFIRs(graalHandle rvrb);


/*-------------------------------------------------------------------

-------------------------------------------------------------------*/
void ReverbOCLDebugOutput(amdOCLRvrb rvrb, char * msg) ;

/*------------------------------------------------------------------
-------------------------------------------------------------------*/
typedef struct _StatisticsPerRun {
	double processing_time; // in samples
} StatisticsPerRun;

StatisticsPerRun * GetStat(amdOCLRvrb rvrb);
/*-------------------------------------------------------------------
------------------------------------------------------------------*/
// if !reverb_kernel get only header
int WavGet( wav_header * header,  void * reverb_kernel, const char * wav_file_loc);
int WavPut( wav_header * header,  void * reverb_kernel, const char * wav_file_loc);

#endif