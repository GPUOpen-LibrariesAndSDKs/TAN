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
	bool start();																// Start the demo
	void stop();																// Stop the demo
	
	std::string getLastError() const 
	{
		return mLastError;
	}															

	void loadConfiguration(const std::string& xmlfilename);						// Load the configuration from the xml file
	void saveConfiguraiton(const std::string& xmlfilename);						// Save all the configruation in xml file named by the parameter

	int addSoundSource(const std::string& sourcename);							// Add a sound source in the audio engine
	bool removeSoundSource(int id);
	int  findSoundSource(const std::string& sourcename);						// Given the sound source name, find the corresponding soundsource ID,

	bool isInsideRoom(float x, float y, float z);								// determine if a point is inside the room or not.
																				// return -1 if not found

	float getReverbTime(float final_db, int* nreflections);						// Based on the room definition and given db, estimate the reverb time.
																				// Number of reflection will be returned in parameters

	float getConvolutionTime();													// Based on the convolution length, calculate the convoltion time
	float getBufferTime();														// Based on the buffer length, calculate the buffer tiem
	std::vector<std::string> getCPUConvMethod() const;							// Get the name of the supported CPU convolution method
	std::vector<std::string> getGPUConvMethod() const;							// Get the name of the supported GPU convolution method
	amf::TAN_CONVOLUTION_METHOD getConvMethodFlag(const std::string& _name);	// Convert a convolution method's name in to internal flag that can be used in runtime
	int getConvMethodIndex(amf::TAN_CONVOLUTION_METHOD method, bool bGPU);	    // Convert a convolution method's enum to menu index
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
	void initializeConvolution();
	void initializeListener();													// Initialize TAN listener profile
	void initializeAudioPosition(int index);

	//void initializeDevice();													// Initialize TAN device (Convolution, FFT, etc.)
	bool parseElement(char *start, char *end, struct element *elem);			// Function used to parse XML file. Used in load configruation
	bool findElement(char **start, char **end, char *name);						// Function used to parse XML file
	//void portInfoToEngine();													// Port all the configuration to engine

	void enumDevices();															// Initialize TAN device (Convolution, FFT, etc.)

//todo: make accessors
public:
	std::string mTANDLLPath;
	std::string mLogPath;
	std::string mConfigFileName;

	std::string mWavFileNames[MAX_SOURCES];
	char*  m_cpWavFileNames[MAX_SOURCES];

	int m_iNumOfWavFile = 0;

	std::unique_ptr<Audio3D> m_pAudioEngine;											// Pointer to the main audio3d engine
	RoomDefinition m_RoomDefinition;									// Roombox definition, contains damping and dimension
	StereoListener m_Listener;											// Listener configuration
	int m_iHeadAutoSpin = 0;
	
	/*Sound Source*/
	MonoSource m_SoundSources[MAX_SOURCES];								// All of the sound sources
	bool mSoundSourceEnable[MAX_SOURCES];								// sound sources' enable
	bool mSrcTrackHead[MAX_SOURCES];
	bool mSrc1EnableMic = false;
	int m_isrc1MuteDirectPath = 0;
	int m_isrc1TrackHeadPos = 0;

	/*Device*/
	char* m_cpDeviceName[MAX_DEVICES];									// Device names
	int mCPUDevicesCount = 0;												// Device count
	int mGPUDevicesCount = 0;												// Device count
	std::string mCPUDevicesNames[MAX_DEVICES];								// Device names
	std::string mGPUDevicesNames[MAX_DEVICES];
	
	/*Convolution*/
	amf::TAN_CONVOLUTION_METHOD m_eConvolutionMethod =					// TAN Convolution method
		amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	int m_iConvolutionLength = 0;
	int m_iBufferSize = 0;

	bool mConvolutionOverCL = false;
	bool mConvolutionOverGPU = false;
	int mConvolutionDeviceIndex = 0;
	int mConvolutionPriority = 0;
	int mConvolutionCUCount = 0;
	int m_exModeConv = 0;

	/*Room*/
	bool mRoomOverCL = false;
	bool mRoomOverGPU = false;
	int mRoomDeviceIndex = 0;
	int mRoomPriority = 0;
	int mRoomCUCount = 0;
	int m_exModeRoom = 0;

	//Mix down
	int m_exModeMix = 0;

	std::string mPlayerName;

	std::string mLastError;
};