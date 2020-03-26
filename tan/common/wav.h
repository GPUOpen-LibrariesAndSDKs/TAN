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
#pragma once
#pragma pack(push,1)

#include <cstdint>

/*\
|*|----====< ".WAV" file definition >====----
|*|
|*|     4 bytes 'RIFF'
|*|     4 bytes <length>
|*|     4 bytes 'WAVE'
|*|     4 bytes 'fmt '
|*| 	4 bytes  <length>	; 10h - length of 'data' block
|*|     2 bytes  01 	    ; format tag
|*|     2 bytes  01 	    ; channels (1=mono, 2=stereo)
|*|     4 bytes  xxxx	    ; samples per second
|*|     4 bytes  xxxx	    ; average samples per second
|*| 	2 bytes  01/02/04	; block alignment
|*|     2 bytes  08/16	    ; bits per sample
|*|     4 bytes 'data'
|*|     4 bytes <length>
|*|       bytes <sample data>
|*|
\*/

/* Wave format control block					*/

typedef struct
{
	uint16_t formatTag;		/* format category		*/
	uint16_t nChannels;		/* stereo/mono			*/
	uint32_t nSamplesPerSec;	/* sample rate			*/
	uint32_t nAvgBytesPerSec;	/* stereo * sample rate 	*/
	uint16_t nBlockAlign;		/* block alignment (1=byte)	*/
	uint16_t nBitsPerSample;	/* # byte bits per sample	*/
} WaveInfo;

typedef struct
{
	char name[4];
	uint32_t length;
	WaveInfo info;
} WaveFormat;

/* Data header which follows a WaveFormat Block			*/
typedef struct
{
	char name[4];
	uint32_t length;
} DataHeader;

/* Total Wave Header data in a wave file				*/
typedef struct
{
	char name[4];
	WaveFormat fmt;
	DataHeader data;
} WaveHeader;

/* Riff wrapper around the WaveFormat Block (optional)		*/
typedef struct
{
	char name[4];
	uint32_t length;
} RiffHeader;

/* Riff wrapped WaveFormat Block					*/
typedef struct
{
	RiffHeader riff;
	WaveHeader wave;
} RiffWave;

bool ReadWaveFile
(
	const char*	fileName,
	uint32_t & 	samplesPerSec,
	uint16_t &	bitsPerSample,
	uint16_t &	channelsCount,
	uint32_t & 	samplesCount,
	uint8_t **	pSamples,
	float ***	pfSamples
);
bool WriteWaveFileF(const char *fileName, int samplesPerSec, int nChannels, int bitsPerSample, long nSamples, float **pSamples);
bool WriteWaveFileS
(
	const char * fileName,
	uint32_t samplesPerSec,
	uint16_t channelsCount,
	uint16_t bitsPerSample,
	uint32_t samplesCount,
	int16_t * pSamples
);

#ifdef __cplusplus

#include <vector>
#include <string>
#include <chrono>

#define STEREO_CHANNELS_COUNT 2

struct WavContent
{
	uint16_t	ChannelsCount;
	uint32_t 	SamplesCount;
	uint16_t 	BitsPerSample;
	uint32_t 	SamplesPerSecond;
	std::vector<uint8_t>
				Data;

	inline std::chrono::milliseconds
				GetDuration() const
	{
		return std::chrono::milliseconds(int(1000.f * float(SamplesCount) / float(SamplesPerSecond)));
	}
	inline size_t
				GetSampleSizeInBytes() const
	{
		return BitsPerSample / 8;
	}

	inline void Reset()
	{
		ChannelsCount = 0;
		SamplesCount = 0;
		BitsPerSample = 0;
		SamplesPerSecond = 0;
		Data.resize(0);
	}

	inline bool IsValid() const
	{
		return Data.size() && Data.size() == ChannelsCount * SamplesCount * BitsPerSample / 8;
	}

	bool ReadWaveFile(const std::string & fileName);
	bool Convert2Stereo16Bit();
	bool Convert16bMonoTo16BitStereo();
	bool JoinChannels();

	inline bool IsSameFormat(const WavContent & other)
	{
		return
		    other.ChannelsCount == ChannelsCount
			&&
		    other.BitsPerSample == BitsPerSample
			&&
			other.SamplesPerSecond == SamplesPerSecond
			;
	}
};

#endif

#pragma pack(pop)