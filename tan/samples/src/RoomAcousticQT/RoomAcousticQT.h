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

#pragma once

#include "../common/SimpleVRaudio.h"
#include <string>

#define MAX_DEVICES 10

//struct attribute {
//	std::string name;
//	void *value;
//	char fmt; // f, i, s
//};

struct attribute {
	char *name;
	void *value;
	char fmt; // f, i, s
};

//struct element {
//	std::string name;
//	int nAttribs;
//	struct attribute *attriblist;
//	int nElements;
//	struct element *elemList;
//};

struct element {
	char *name;
	int nAttribs;
	struct attribute *attriblist;
	int nElements;
	struct element *elemList;
};

enum execMode {
	C_MODEL = 0,
	OCL_GPU = 0x04,
	OCL_GPU_MPQ = 0x05,
	OCL_GPU_RTQ1 = 0x06,
	OCL_GPU_RTQ2 = 0x07,
	OCL_CPU = 0x10
};

class RoomAcousticQT
{
public:
	RoomAcousticQT();
	virtual ~RoomAcousticQT();

	void initialize();															// Initialize the Room Acoustic
	int start();																// Start the demo
	void stop();												// Stop the demo

	void loadConfiguration(const std::string& xmlfilename);						// Load the configuration from the xml file
	void saveConfiguraiton(const std::string& xmlfilename);						// Save all the configruation in xml file named by the parameter

	int addSoundSource(const std::string& sourcename);							// Add a sound source in the audio engine
	bool replaceSoundSource(const std::string& sourcename, int id);				// Replace a sound source in the audio engine given its ID. Return true if success
	bool removeSoundSource(const std::string& sourcename);						// Remove a sound source in the audio engine, return true if success
	bool removeSoundSource(int id);
	int  findSoundSource(const std::string& sourcename);						// Given the sound source name, find the corresponding soundsource ID,

	bool isInsideRoom(float x, float y, float z);								// determine if a point is inside the room or not.
																				// return -1 if not found

	float getReverbTime(float final_db, int* nreflections);						// Based on the room definition and given db, estimate the reverb time.
																				// Number of reflection will be returned in parameters

	float getConvolutionTime();													// Based on the convolution length, calculate the convoltion time
	float getBufferTime();														// Based on the buffer length, calculate the buffer tiem
	void getCPUConvMethod(std::string** _out, int* _num);						// Get the name of the supported CPU convolution method
	void getGPUConvMethod(std::string** _out, int* _num);						// Get the name of the supported GPU convolution method
	amf::TAN_CONVOLUTION_METHOD getConvMethodFlag(const std::string& _name);	// Convert a convolution method's name in to internal flag that can be used in runtime
	/*Run time - these function should be used only when engine is running*/
	void updateAllSoundSourcesPosition();										// update all the sound source position
	void updateSoundSourcePosition(int index);									// update the sound source position
	void updateListenerPosition();												// update the listener position
	void updateRoomDimention();
	void updateRoomDamping();
	AmdTrueAudioVR* getAMDTrueAudioVR();
	TANConverterPtr getTANConverter();

	void UpdateSoundSourcesPositions();

private:
	void initializeEnvironment();												// Initialize TAN DLL
	void initializeAudioEngine();												// Initialize TAN Audio3D Engine

	void initializeRoom();														// Initialize TAN Room definition
	void initializeListener();													// Initialize TAN listener profile
	void initializeAudioPosition(int index);

	void initializeDevice();													// Initialize TAN device (Convolution, FFT, etc.)
	bool parseElement(char *start, char *end, struct element *elem);			// Function used to parse XML file. Used in load configruation
	bool findElement(char **start, char **end, char *name);						// Function used to parse XML file
	void portInfoToEngine();													// Port all the configuration to engine

//todo: make accessors
public:
	std::string mTANDLLPath;
	std::string mLogPath;
	std::string mConfigFileName;

	std::string mWavFileNames[MAX_SOURCES];
	char*  m_cpWavFileNames[MAX_SOURCES];
	int m_iNumOfWavFile = 0;

	std::unique_ptr<Audio3D> m_pAudioEngine;							// Pointer to the main audio3d engine
	RoomDefinition m_RoomDefinition;									// Roombox definition, contains damping and dimension
	StereoListener m_Listener;											// Listener configuration
	int m_iHeadAutoSpin = 0;

	/*Sound Source*/
	MonoSource m_SoundSources[MAX_SOURCES];								// All of the sound sources
	bool mSoundSourceEnable[MAX_SOURCES];								// sound sources' enable
	int m_iSoundSourceMap[MAX_SOURCES];
	int m_bSrcTrackHead[MAX_SOURCES];
	bool mSrc1EnableMic = false;
	int m_isrc1MuteDirectPath = 0;
	int m_isrc1TrackHeadPos = 0;
	/*Device*/
	char* m_cpDeviceName[MAX_DEVICES];									// Device names
	int m_iDeviceCount = 0;												// Device count
	/*Convolution*/
	amf::TAN_CONVOLUTION_METHOD m_eConvolutionMethod =					// TAN Convolution method
		amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	int m_iConvolutionLength = 0;
	int m_iBufferSize = 0;
	int m_iConvolutionDeviceID = 0;

	int m_iuseGPU4Conv = 0;
	int m_iuseMPr4Conv = 0;
#ifdef RTQ_ENABLED
	int m_iConvolutionCUCount = 0;
	int m_iuseRTQ4Conv = 0;
#endif // RTQ_ENABLED

	/*Room*/

	int m_iuseGPU4Room = 0;
	int m_iRoomDeviceID = 0;											// the device that the room generator is running on
	int m_iuseMPr4Room = 0;
#ifdef RTQ_ENABLED
	int m_iRoomCUCount = 0;
	int m_iuseRTQ4Room = 0;
#endif // RTQ_ENABLED

	std::string mPlayerName;

	bool mCLRoomOverGPU = false;
	bool mCLConvolutionOverGPU = false;

private:
	std::string mWavFileNamesInternal[MAX_SOURCES];							// Internal wav file name, used to pass valid source file into Audio3D engine
	int m_iNumOfWavFileInternal = 0;
};
