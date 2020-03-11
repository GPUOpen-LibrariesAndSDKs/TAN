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

#include "RoomAcousticQT.h"
#include "samples/src/common/GpuUtils.h"
#include "samples/src//TrueAudioVR/TrueAudioVR.h"
#include "FileUtility.h"

#include <time.h>
#include <cmath>
#include <string>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include "AclAPI.h"
#include <Shlwapi.h>
#else
#include <unistd.h>
#endif

#include <QStandardPaths>

RoomAcousticQT::RoomAcousticQT()
{
	initialize();
}

RoomAcousticQT::~RoomAcousticQT()
{
}

void RoomAcousticQT::initialize()
{
	initializeEnvironment();
	initializeAudioEngine();

	initializeRoom();
	initializeListener();

	initializeDevice();
}

int RoomAcousticQT::start()
{
	// Remap the sound path to accomodates the audioVR engine
	// TODO: Need to port the configuration back to the audio3D engine

	//Initialize Audio 3D engine
	portInfoToEngine();
	// Since cpu's device id is 0 in this demo, we need to decrease the device id if
	// you want to run GPU
	int convolutionDeviceIndex = m_iConvolutionDeviceID;
	int roomDeviceIndex = m_iRoomDeviceID;

	//becouse indices must be zero based?
	if(m_iuseGPU4Conv)
	{
		convolutionDeviceIndex--;
	}

	if(m_iuseGPU4Room)
	{
		roomDeviceIndex--;
	}

	std::vector<std::string> fileNames;
	fileNames.reserve(m_iNumOfWavFileInternal);

	for(int nameIndex(0); nameIndex < m_iNumOfWavFileInternal; ++nameIndex)
	{
		fileNames.push_back(mWavFileNamesInternal[nameIndex]);
	}

	int err =  m_pAudioEngine->Init(
		mTANDLLPath,
		m_RoomDefinition,

		fileNames,

		mSrc1EnableMic,
		m_isrc1TrackHeadPos,

		m_iConvolutionLength,
		m_iBufferSize,

		m_iuseGPU4Conv,
		mCLConvolutionOverGPU,
		convolutionDeviceIndex,

#ifdef RTQ_ENABLED
		m_iuseMPr4Conv,
		m_iuseRTQ4Conv,
		m_iConvolutionCUCount,
#endif // RTQ_ENABLED

		m_iuseGPU4Room,
		mCLRoomOverGPU,
		roomDeviceIndex,

#ifdef RTQ_ENABLED
		m_iuseMPr4Room,
		m_iuseRTQ4Room,
		m_iRoomCUCount,
#endif

		m_eConvolutionMethod,

		mPlayerName
		);

	if(!err)
	{
		m_pAudioEngine->setWorldToRoomCoordTransform(0., 0., 0., 0., 0., true);

		updateAllSoundSourcesPosition();
		updateListenerPosition();

		return m_pAudioEngine->Run();
	}

	return -1;
}

void RoomAcousticQT::stop()
{
	m_pAudioEngine->Stop();
}

void RoomAcousticQT::initializeEnvironment()
{
	auto moduleFileName = getModuleFileName();
	auto path2Exe = getPath2File(moduleFileName);
	auto commandName = getFileNameWithoutExtension(moduleFileName);

	auto locations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	auto homeLocation = locations.size() ? locations[0].toStdString() : path2Exe;

	mTANDLLPath = path2Exe;
	mConfigFileName = joinPaths(homeLocation, std::string(".") + commandName + "-default.ini");
	mLogPath = joinPaths(homeLocation, std::string(".") + commandName + ".log");

	setCurrentDirectory(mTANDLLPath);

	//todo: ivm: uncomment
	/*
	FILE *fpLog = NULL;
	errno_t err = 0;
	errno = 0;

	// Redirect stdout and stderr to a log file.
	if (fopen_s(&fpLog, mLogPath.c_str(), "a+") == 0)
	{
		dup2(fileno(fpLog), fileno(stdout));
		dup2(fileno(fpLog), fileno(stderr));
	}
	//else if (fopen_s(&fpLog, "TaLibVRDemoLog.log", "a+") == 0) {
	//	dup2(fileno(fpLog), fileno(stdout));
	//	dup2(fileno(fpLog), fileno(stderr));
	//}
	*/

	//todo: ivm: return
	/*
	wchar_t logDateVerStamp[MAX_PATH * 2] = {0};
	char version[40] = {0};
	GetFileVersionAndDate(logDateVerStamp, version);
	wprintf(logDateVerStamp);
	*/
	std::cout << "Log started" << std::endl;

	for (int idx = 0; idx < MAX_SOURCES; idx++)
	{
		mWavFileNames[idx].resize(0);
	}

	for (int index = 0; index < MAX_SOURCES; ++index)
	{
		m_bSrcTrackHead[index] = false;
	}

	for (int idx = 0; idx < MAX_SOURCES; idx++) {
		m_cpWavFileNames[idx] = new char[MAX_PATH + 2];
		m_cpWavFileNames[idx][0] = '\0';
	}

}

void RoomAcousticQT::initializeAudioEngine()
{
	m_pAudioEngine.reset(new Audio3D());
}

void RoomAcousticQT::initializeRoom()
{
	m_RoomDefinition.height = 4.0;
	m_RoomDefinition.width = 6.0;
	m_RoomDefinition.length = 10.0;

	m_RoomDefinition.mFront.damp = DBTODAMP(2.0);
	m_RoomDefinition.mBack.damp = DBTODAMP(6.0);

	m_RoomDefinition.mLeft.damp = m_RoomDefinition.mRight.damp = DBTODAMP(4.0);
	m_RoomDefinition.mTop.damp = m_RoomDefinition.mBottom.damp = DBTODAMP(2.0);
}

void RoomAcousticQT::initializeListener()
{
	m_Listener.earSpacing = float(0.16);
	m_Listener.headX = float(m_RoomDefinition.width * .8);
	m_Listener.headZ = float(m_RoomDefinition.length * .8);
	m_Listener.headY = 1.75;
	m_Listener.pitch = 0.0;
	m_Listener.roll = 0.0;
	m_Listener.yaw = 0.0;
}

void RoomAcousticQT::initializeAudioPosition(int index)
{

	float radius = m_RoomDefinition.width / 2;

	//for(int idx = 0; idx < MAX_SOURCES; idx++)
	{
		//original:
		/*m_SoundSources[idx].speakerX = m_RoomDefinition.width / 2 + idx * m_RoomDefinition.width / MAX_SOURCES;
		m_SoundSources[idx].speakerZ = float(m_RoomDefinition.width * 0.05);*/
		m_SoundSources[index].speakerX = m_Listener.headX + radius * std::sin(float(index));
		m_SoundSources[index].speakerZ = m_Listener.headZ + radius * std::cos(float(index));
		m_SoundSources[index].speakerY = 1.75;

		//mWavFileNamesInternal[idx].resize(0);
	}

	for (int i = 0; i < MAX_SOURCES; i++)
	{
		m_iSoundSourceMap[i] = 0;
	}
}

void RoomAcousticQT::initializeDevice()
{
	m_iConvolutionLength = 32768;
	m_iBufferSize = 1024;
	for (int i = 0; i < MAX_DEVICES; i++){
		m_cpDeviceName[i] = new char[MAX_PATH + 2];
		memset(m_cpDeviceName[i], 0, (MAX_PATH + 2));
	}
	m_iDeviceCount = listGpuDeviceNamesWrapper(m_cpDeviceName, MAX_DEVICES);
}

bool RoomAcousticQT::parseElement(char* start, char* end, element* elem)
{
	bool ok = false;
	start += strlen(elem->name) + 1;

	// parse attributes
	for (int j = 0; j < elem->nAttribs; j++) {
		char *pName = elem->attriblist[j].name;
		int len = (int)strlen(pName);
		char *p = start;
		while (p++ < end) {
			if (strncmp(p, pName, len) == 0) {
				p += len;
				while (p < end) {
					if (*p++ == '=')
						break;
				}
				while (p < end) {
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
					while (p < end && *p != '\"') {
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
	for (int i = 0; i < elem->nElements; i++) {
		char *s, *e;
		s = start;
		e = end;
		if (findElement(&s, &e, elem->elemList[i].name)) {
			ok = parseElement(s, e, &elem->elemList[i]);
		}
	}

	return ok;
}

bool RoomAcousticQT::findElement(char** start, char** end, char* name)
{
	bool found = false;
	char *p = *start;
	while (p < *end) {
		if (*p == '<')
		{
			if (strncmp(p + 1, name, strlen(name)) == 0) {
				*start = p++;
				int nestcount = 0;
				while (p < *end) {
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
					else if (p[0] == '/' && p[1] == '>' && nestcount == 0) {
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


void RoomAcousticQT::portInfoToEngine()
{
	// Remap the source name so that it maps the audio3D engine
	int soundnameindex = 0;
	m_iNumOfWavFileInternal = 0;

	for (int i = 0; i < MAX_SOURCES; i++)
	{
		mWavFileNamesInternal[soundnameindex].resize(0);

		if(mWavFileNames[i].length() && mSoundSourceEnable[i])
		{
			mWavFileNamesInternal[soundnameindex] = mWavFileNames[i];
			m_iSoundSourceMap[i] = soundnameindex;
			soundnameindex++;
			m_iNumOfWavFileInternal++;
		}
	}
}

void RoomAcousticQT::loadConfiguration(const std::string& xmlfilename)
{
	initializeEnvironment();

	// Creating internal structre and prepare for xml loading
	// attribute src1PosAttribs[3] = { { "X", &m_SoundSources->speakerX, 'f' }, { "Y", &srcY[0], 'f' }, { "Z", &srcZ[0], 'f' } };
	element RAelementList[MAX_SOURCES + 4];
	std::vector<attribute*> attributes_list;
	std::vector<element*> elements_list;
	int temp = 0;
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		attribute* srcPosAttribs = new attribute[3];
		srcPosAttribs[0].name = "X";
		srcPosAttribs[0].value = &(m_SoundSources[i].speakerX);
		srcPosAttribs[0].fmt = 'f';
		srcPosAttribs[1].name = "Y";
		srcPosAttribs[1].value = &(m_SoundSources[i].speakerY);
		srcPosAttribs[1].fmt = 'f';
		srcPosAttribs[2].name = "Z";
		srcPosAttribs[2].value = &(m_SoundSources[i].speakerZ);
		srcPosAttribs[2].fmt = 'f';
		attributes_list.push_back(srcPosAttribs);

		RAelementList[i].name = new char[MAX_PATH];
		attribute *streamAttribs = new attribute[2];
		attributes_list.push_back(streamAttribs);
		streamAttribs[0].name = "on";
		streamAttribs[0].value = &mSoundSourceEnable[i];
		streamAttribs[0].fmt = 'i';

		streamAttribs[1].name = "file";
		streamAttribs[1].value = &m_cpWavFileNames[i][0];
		streamAttribs[1].fmt = 's';
		element *src = nullptr;

		if (i == 0) {
			src = new element[3];
			elements_list.push_back(src);
			attribute* src1MicAttribs = new attribute[3];
			attributes_list.push_back(src1MicAttribs);
			src1MicAttribs[0].name = "enableMic";
			src1MicAttribs[0].value = &temp;//&m_isrc1EnableMic;
			src1MicAttribs[0].fmt = 'i';
			src1MicAttribs[1].name = "trackHeadPos";
			src1MicAttribs[1].value = &m_isrc1TrackHeadPos;
			src1MicAttribs[1].fmt = 'i';
			src1MicAttribs[2].name = "muteDirectPath";
			src1MicAttribs[2].value = &m_isrc1TrackHeadPos;
			src1MicAttribs[2].fmt = 'i';

			//attribute src1MicAttribs[3] = { { "enableMic", &m_isrc1EnableMic, 'i' }, { "trackHeadPos", &m_isrc1TrackHeadPos, 'i' }, { "muteDirectPath", &m_isrc1TrackHeadPos, 'i' } };
			//attribute streamAttribs[1] = { { "file", &m_cpWavFileNames[i][0], 's' } };
			src[0] = { "streamS1", 2, streamAttribs, 0, NULL };
			src[1] = { "microphone", 3, src1MicAttribs, 0, NULL };
			src[2] = { "positionS1", 3, srcPosAttribs, 0, NULL };
			RAelementList[i].nElements = 3;
			RAelementList[i].elemList = src;
		}
		else
		{
			src = new element[2];
			elements_list.push_back(src);


			std::string _stream = "streamS";
			std::string _position = "positionS";
			_stream += std::to_string(i + 1);
			_position += std::to_string(i + 1);
			src[0].name = new char[MAX_PATH];
			src[1].name = new char[MAX_PATH];
			strncpy(src[0].name, _stream.c_str(), MAX_PATH);
			strncpy(src[1].name, _position.c_str(), MAX_PATH);
			src[0].nAttribs = 2;
			src[1].nAttribs = 3;
			src[0].attriblist = streamAttribs;
			src[1].attriblist = srcPosAttribs;
			src[0].nElements = src[1].nElements = 0;
			src[0].elemList = src[1].elemList = NULL;
			RAelementList[i].nElements = 2;
			RAelementList[i].elemList = src;
		}
		std::string _Source = "Source";
		_Source += std::to_string(i + 1);

		strncpy(RAelementList[i].name, _Source.c_str(), MAX_PATH);
		RAelementList[i].nAttribs = 0;
		RAelementList[i].attriblist = NULL;


	}

	// Initialize Listener Attributes
	attribute headPosAttribs[6] = { { "X", &m_Listener.headX, 'f' }, { "Y", &m_Listener.headY, 'f' }, { "Z", &m_Listener.headZ, 'f' },
	{ "yaw", &m_Listener.yaw, 'f' }, { "pitch", &m_Listener.pitch, 'f' }, { "roll", &m_Listener.roll, 'f' } };
	attribute earAttribs[1] = { { "S", &m_Listener.earSpacing, 'f' } };
	attribute spinAttribs[1] = { { "AS", &m_iHeadAutoSpin, 'i' } };
	struct element head[3] = {
		{ "positionL1", 6, headPosAttribs, 0, NULL },
		{ "earSpacing", 1, earAttribs, 0, NULL },
		{ "autoSpin", 1, spinAttribs, 0, NULL }
	};

	// Initialize Room Attributes
	attribute roomDimAttribs[3] = { { "width", &m_RoomDefinition.width, 'f' }, { "height", &m_RoomDefinition.height, 'f' }, { "length", &m_RoomDefinition.length, 'f' } };
	attribute roomDampAttribs[6] =
	{ { "left", &m_RoomDefinition.mLeft.damp, 'f' }, { "right", &m_RoomDefinition.mRight.damp, 'f' },
	{ "front", &m_RoomDefinition.mFront.damp, 'f' }, { "back", &m_RoomDefinition.mBack.damp, 'f' },
	{ "top", &m_RoomDefinition.mTop.damp, 'f' }, { "bottom", &m_RoomDefinition.mBottom.damp, 'f' } };

	int useCMODEL_4Room(0), useOCLGPU_4Room(0), useMPr_4Room(0), useRTQ1_4Room(0), useRTQ2_4Room(0), useOCLCPU_4Room(0);

	attribute roomRenderAttribs[8] = { { "nSources", &m_iNumOfWavFile, 'i' },
										{ "withCMODEL", &useCMODEL_4Room, 'i' },
										{ "withOCLGPU", &useOCLGPU_4Room, 'i' },
										{ "withMPr", &useMPr_4Room, 'i' },
										{ "withRTQ1", &useRTQ1_4Room, 'i' },
										{ "withRTQ2", &useRTQ2_4Room, 'i' },
										{ "withOCLCPU", &useOCLCPU_4Room, 'i' },
										{ "withCus", &temp, 'i' }
										//{ "withCus", &m_iRoomCUCount, 'i' }
	};

	element roomElems[3] = {
		{ "dimensions", 3, roomDimAttribs, NULL },
		{ "damping", 6, roomDampAttribs, NULL },
		{ "rendering", 8, roomRenderAttribs, NULL }
	};

	// Initialize Convolution attribute
	int useCMODEL_4Conv(0), useOCLGPU_4Conv(0), useMPr_4Conv(0), useRTQ1_4Conv(0), useRTQ2_4Conv(0), useOCLCPU_4Conv(0);

	struct attribute convCfgAttribs[10] = {
		{ "length", &m_iConvolutionLength, 'i' },
		{ "buffersize", &m_iBufferSize, 'i' },
		{ "useCMODEL", &useCMODEL_4Conv, 'i' },
		{ "useOCLGPU", &useOCLGPU_4Conv, 'i' },
		{ "useMPr", &useMPr_4Conv, 'i' },
		{ "useRTQ1", &useRTQ1_4Conv, 'i' },
		{ "useRTQ2", &useRTQ2_4Conv, 'i' },
		{ "useOCLCPU", &useOCLCPU_4Conv, 'i' },
		//{ "cuCount", &m_iConvolutionCUCount, 'i' },
		{ "cuCount", &temp, 'i' },
		{ "method", &m_eConvolutionMethod, 'i' }
	};
	struct element convElems[1] = {
		{ "configuration", 10, convCfgAttribs, 0, NULL }
	};

	// Initialize Mix Down attribute
	int useCPU_4Mix(0), useGPU_4Mix(0);

	struct attribute mixAttribs[2] = {
		{ "useCPUMix", &useCPU_4Mix, 'i' },
		{ "useGPUMix", &useGPU_4Mix, 'i' }
	};
	struct element mixElems[1] = {
		{ "configuration", 2, mixAttribs, 0, NULL }
	};

	RAelementList[MAX_SOURCES].name = "Listener";
	RAelementList[MAX_SOURCES].nElements = sizeof(head) / sizeof(element);
	RAelementList[MAX_SOURCES].elemList = head;

	RAelementList[MAX_SOURCES + 1].name = "Room";
	RAelementList[MAX_SOURCES + 1].nElements = sizeof(roomElems) / sizeof(element);
	RAelementList[MAX_SOURCES + 1].elemList = roomElems;


	RAelementList[MAX_SOURCES + 2].name = "Convolution";
	RAelementList[MAX_SOURCES + 2].nElements = sizeof(convElems) / sizeof(element);
	RAelementList[MAX_SOURCES + 2].elemList = convElems;

	RAelementList[MAX_SOURCES + 3].name = "Mix";
	RAelementList[MAX_SOURCES + 3].nElements = sizeof(mixElems) / sizeof(element);
	RAelementList[MAX_SOURCES + 3].elemList = mixElems;

	RAelementList[MAX_SOURCES].nAttribs = RAelementList[MAX_SOURCES + 1].nAttribs = RAelementList[MAX_SOURCES + 2].nAttribs = RAelementList[MAX_SOURCES + 3].nAttribs = 0;
	RAelementList[MAX_SOURCES].attriblist = RAelementList[MAX_SOURCES + 1].attriblist = RAelementList[MAX_SOURCES + 2].attriblist = RAelementList[MAX_SOURCES + 3].attriblist = NULL;

	struct element RoomAcoustics = { "RoomAcoustics", 0, NULL, sizeof(RAelementList) / sizeof(element), RAelementList };

	FILE *fpLoadFile = NULL;

	fopen_s(&fpLoadFile, xmlfilename.c_str(), "r+");

	if (fpLoadFile == NULL) {
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

	//// Apply Parsed Room config to instance
	//if (useCMODEL_4Room) m_exModeRoom = C_MODEL;
	//if (useOCLGPU_4Room) m_exModeRoom = OCL_GPU;
	//if (useMPr_4Room) m_exModeRoom = OCL_GPU_MPQ;
	//if (useRTQ1_4Room) m_exModeRoom = OCL_GPU_RTQ1;
	//if (useRTQ2_4Room) m_exModeRoom = OCL_GPU_RTQ2;
	//if (useOCLCPU_4Room) m_exModeRoom = OCL_CPU;

	//// Apply Parsed Convolution config to instance
	//if (useCMODEL_4Conv) m_exModeConv = C_MODEL;
	//if (useOCLGPU_4Conv) m_exModeConv = OCL_GPU;
	//if (useMPr_4Conv) m_exModeConv = OCL_GPU_MPQ;
	//if (useRTQ1_4Conv) m_exModeConv = OCL_GPU_RTQ1;
	//if (useRTQ2_4Conv) m_exModeConv = OCL_GPU_RTQ2;
	//if (useOCLCPU_4Conv) m_exModeConv = OCL_CPU;

	//// Apply Parsed Mix Down config to instance
	//if (useCPU_4Mix) m_exModeMix = C_MODEL;
	//if (useGPU_4Mix) m_exModeMix = OCL_GPU;

	for (unsigned int i = 0; i < attributes_list.size(); i++)
	{
		delete[]attributes_list[i];
	}
	for (unsigned int i = 0; i < elements_list.size(); i++)
	{
		if (i == 0)
		{
			delete[]elements_list[i];
		}
		else
		{
			delete[] elements_list[i]->name;
			delete elements_list[i];
		}
	}
	// Correct the damping factor to the correct unit
	this->m_RoomDefinition.mLeft.damp = DBTODAMP(this->m_RoomDefinition.mLeft.damp);
	this->m_RoomDefinition.mRight.damp = DBTODAMP(this->m_RoomDefinition.mRight.damp);
	this->m_RoomDefinition.mTop.damp = DBTODAMP(this->m_RoomDefinition.mTop.damp);
	this->m_RoomDefinition.mBottom.damp = DBTODAMP(this->m_RoomDefinition.mBottom.damp);
	this->m_RoomDefinition.mFront.damp = DBTODAMP(this->m_RoomDefinition.mFront.damp);
	this->m_RoomDefinition.mBack.damp = DBTODAMP(this->m_RoomDefinition.mBack.damp);

	for (int idx = 0; idx < MAX_SOURCES; idx++)
	{
		mWavFileNames[idx] = m_cpWavFileNames[idx];
	}

}

/* Save Room acoustic configuration in xml file*/
void RoomAcousticQT::saveConfiguraiton(const std::string& xmlfilename)
{
	time_t dt = time(NULL);
	struct tm *lt = localtime(&dt);
	FILE *fpSaveFile = NULL;

	fopen_s(&fpSaveFile, xmlfilename.c_str(), "w+");

	if (fpSaveFile == NULL) {
		return;
	}

	fputs("<?xml version='1.0' encoding='UTF-8'?>\n", fpSaveFile);
	fprintf(fpSaveFile,
		"<!-- This document was created by AMD RoomAcousticsDemo v1.0.2 on %4d/%02d/%02d %02d:%02d:%02d -->\n",
		2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
	fputs("<RoomAcoustics>\n", fpSaveFile);
	fputs("<!-- All dimensions in meters, damping in decibels -->\n", fpSaveFile);

	fputs(" <Source1>\n", fpSaveFile);
	fprintf(fpSaveFile, " <streamS1  on=\"%d\" file=\"%s\" />\n", mSoundSourceEnable[0], mWavFileNames[0].c_str());
	fprintf(fpSaveFile, "  <microphone enableMic=\"%d\" trackHeadPos=\"%d\" muteDirectPath=\"%d\" />\n",
		0,
		//m_isrc1EnableMic,
		m_isrc1TrackHeadPos, m_isrc1MuteDirectPath);
	fprintf(fpSaveFile, "  <positionS1 X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", m_SoundSources[0].speakerX, m_SoundSources[0].speakerY, m_SoundSources[0].speakerZ);
	fputs(" </Source1>\n", fpSaveFile);

	for (int i = 1; i < MAX_SOURCES; i++) {
		fprintf(fpSaveFile, " <Source%d>\n", i + 1);
		fprintf(fpSaveFile, " <streamS%d  on=\"%d\" file=\"%s\" />\n", i + 1, mSoundSourceEnable[i], mWavFileNames[i].c_str());
		fprintf(fpSaveFile, "  <positionS%d X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", i + 1, m_SoundSources[i].speakerX, m_SoundSources[i].speakerY, m_SoundSources[i].speakerZ);
		fprintf(fpSaveFile, " </Source%d>\n", i + 1);
	}

	fputs(" <Listener>\n", fpSaveFile);
	fprintf(fpSaveFile, "  <positionL1 X=\"%f\" Y=\"%f\" Z=\"%f\" yaw=\"%f\" pitch=\"%f\" roll=\"%f\" />\n",
		m_Listener.headX, m_Listener.headY, m_Listener.headZ, m_Listener.yaw, m_Listener.pitch, m_Listener.roll);
	fprintf(fpSaveFile, "  <earSpacing S=\"%f\"/>\n", m_Listener.earSpacing);
	fprintf(fpSaveFile, "  <autoSpin AS=\"%d\"/>\n", m_iHeadAutoSpin);

	fputs(" </Listener>\n", fpSaveFile);
	fputs(" <Room>\n", fpSaveFile);
	fprintf(fpSaveFile, "  <dimensions width=\"%f\" height=\"%f\" length=\"%f\" />\n", m_RoomDefinition.width, m_RoomDefinition.height, m_RoomDefinition.length);
	fprintf(fpSaveFile, "  <damping left=\"%f\" right=\"%f\" front=\"%f\" back=\"%f\" top=\"%f\" bottom=\"%f\"/>\n",
		DAMPTODB(m_RoomDefinition.mLeft.damp), DAMPTODB(m_RoomDefinition.mRight.damp), DAMPTODB(m_RoomDefinition.mFront.damp), DAMPTODB(m_RoomDefinition.mBack.damp),
		DAMPTODB(m_RoomDefinition.mTop.damp), DAMPTODB(m_RoomDefinition.mBottom.damp));

	fprintf(fpSaveFile, " <rendering nSources=\"%d\" withCMODEL=\"%d\" withOCLGPU=\"%d\" withMPr=\"%d\" withRTQ1=\"%d\" withRTQ2=\"%d\" withOCLCPU=\"%d\" withCus=\"%d\"/>\n",
		m_iNumOfWavFile,
		0, //(m_exModeRoom == C_MODEL),
		0, //(m_exModeRoom == OCL_GPU),
		0, //(m_exModeRoom == OCL_GPU_MPQ),
		0, //(m_exModeRoom == OCL_GPU_RTQ1),
		0, //(m_exModeRoom == OCL_GPU_RTQ2),
		0, //(m_exModeRoom == OCL_CPU),
		0//m_iRoomCUCount
	);

	fputs(" </Room>\n", fpSaveFile);
	fputs("<Convolution>\n", fpSaveFile);

	fprintf(fpSaveFile, " <configuration length=\"%d\" buffersize =\"%d\" useCMODEL=\"%d\" useOCLGPU=\"%d\" useMPr=\"%d\" useRTQ1=\"%d\" useRTQ2=\"%d\" useOCLCPU=\"%d\" cuCount=\"%d\" method=\"%d\"/>\n",
		m_iConvolutionLength,
		m_iBufferSize,
		0,//(m_exModeConv == C_MODEL),
		0,//(m_exModeConv == OCL_GPU),
		0,//(m_exModeConv == OCL_GPU_MPQ),
		0, //(m_exModeConv == OCL_GPU_RTQ1),
		0, //(m_exModeConv == OCL_GPU_RTQ2),
		0, //(m_exModeConv == OCL_CPU),
		0,//m_iConvolutionCUCount,
		m_eConvolutionMethod
	);

	fputs("</Convolution>\n", fpSaveFile);
	fputs("<Mix>\n", fpSaveFile);
	fprintf(fpSaveFile, " <configuration useCPUMix=\"%d\" useGPUMix=\"%d\"/>\n",
		0,//(m_exModeMix == C_MODEL),
		0//(m_exModeMix == OCL_GPU)
	);
	fputs("</Mix>\n", fpSaveFile);
	fputs("</RoomAcoustics>\n", fpSaveFile);
	fclose(fpSaveFile);
}


int RoomAcousticQT::addSoundSource(const std::string& sourcename)
{
	// Find a empty sound source slots and assign to it.
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if(!mWavFileNames[i].length())
		{
			mWavFileNames[i] = sourcename;
			mSoundSourceEnable[i] = true;
			m_bSrcTrackHead[i] = false;
			if(!i)
			{
				mSrc1EnableMic = false;
			}
			initializeAudioPosition(i);
			mWavFileNames[i] = sourcename;
			m_iNumOfWavFile++;

			return i;
		}
	}
}

bool RoomAcousticQT::replaceSoundSource(const std::string& sourcename, int id)
{
	if (id < 0 && id >= MAX_PATH)
	{
		return false;
	}
	else
	{
		m_SoundSources[id].speakerY = 0.0f;
		m_SoundSources[id].speakerX = 0.0f;
		m_SoundSources[id].speakerZ = 0.0f;

		mSoundSourceEnable[id] = true;

		if (id == 0)
		{
			m_isrc1TrackHeadPos = 0;
			mSrc1EnableMic = false;
			m_isrc1MuteDirectPath = 0;
		}

		mWavFileNames[id] = sourcename;

		return true;
	}
}

bool RoomAcousticQT::removeSoundSource(const std::string& sourcename)
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (!strcmp(sourcename.c_str(), mWavFileNames[i].c_str()))
		{
			removeSoundSource(i);
		}
	}
	return true;
}

bool RoomAcousticQT::removeSoundSource(int id)
{
	// Check if the id is valid
	if (id >= 0 && id < MAX_SOURCES)
	{
		// if the sound source exist
		if (mWavFileNames[id].length())
		{
			m_SoundSources[id].speakerY = 0.0f;
			m_SoundSources[id].speakerX = 0.0f;
			m_SoundSources[id].speakerZ = 0.0f;
			mSoundSourceEnable[id] = true;

			if (id == 0)
			{
				m_isrc1TrackHeadPos = 0;
				mSrc1EnableMic = false;
				m_isrc1MuteDirectPath = 0;
			}

			// Clean file name
			mWavFileNames[id].resize(0);
			m_iNumOfWavFile--;

			return true;
		}
	}

	return false;
}

/*TODO: Need to rework this function for potential epislon comparison*/
bool RoomAcousticQT::isInsideRoom(float x, float y, float z)
{
	return (x >= 0.0f && x <= m_RoomDefinition.width) &&
		(y >= 0.0f && y <= m_RoomDefinition.length) &&
		(z >= 0.0f && z <= m_RoomDefinition.height);
}

int RoomAcousticQT::findSoundSource(const std::string& sourcename)
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if(!strcmp(sourcename.c_str(), mWavFileNames[i].c_str()))
		{
			return i;
		}
	}
	return -1;
}

float RoomAcousticQT::getReverbTime(float final_db, int* nreflections)
{
	return estimateReverbTime(this->m_RoomDefinition, final_db, nreflections);
}

float RoomAcousticQT::getConvolutionTime()
{
	return m_iConvolutionLength / 48000.0f;
}

float RoomAcousticQT::getBufferTime()
{
	return m_iBufferSize / 48000.0f;
}

void RoomAcousticQT::getCPUConvMethod(std::string** _out, int* _num)
{
	int numberOfMethod = 3;
	std::string* output = new std::string[numberOfMethod];
	output[0] = "OVERLAP ADD";
	output[1] = "UNIFORM PARTITIONED";
	output[2] = "NONUNIFORM PARTITIONED";
	*_out = output;
	*_num = numberOfMethod;
}

void RoomAcousticQT::getGPUConvMethod(std::string** _out, int* _num)
{
	int numberOfMethod = 3;
	std::string* output = new std::string[numberOfMethod];
	output[0] = "OVERLAP ADD";
	output[1] = "UNIFORM PARTITIONED GPU";
	output[2] = "NONUNIFORM PARTITIONED GPU";
	*_out = output;
	*_num = numberOfMethod;
}

amf::TAN_CONVOLUTION_METHOD RoomAcousticQT::getConvMethodFlag(const std::string& _name)
{
	if (_name == "OVERLAP ADD")
		return TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	if (_name == "UNIFORM PARTITIONED")
		return TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_UNIFORM;
	if (_name == "NONUNIFORM PARTITIONED")
		return TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_NONUNIFORM;
	if (_name == "UNIFORM PARTITIONED GPU")
		return  TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL;
	if (_name == "NONUNIFORM PARTITIONED GPU")
		return TAN_CONVOLUTION_METHOD_FHT_NONUNIFORM_PARTITIONED;

	return TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
}

void RoomAcousticQT::updateAllSoundSourcesPosition()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if(mWavFileNames[i].length() && mSoundSourceEnable[i])
		{
			updateSoundSourcePosition(i);
		}
	}
}

void RoomAcousticQT::updateSoundSourcePosition(int index)
{
	int i = m_iSoundSourceMap[index];
	m_pAudioEngine->updateSourcePosition(
		i,
		m_SoundSources[index].speakerX,
		m_SoundSources[index].speakerY,
		m_SoundSources[index].speakerZ
		);
}

void RoomAcousticQT::UpdateSoundSourcesPositions()
{
	return;
	for(int index = 0; index < MAX_SOURCES; index++)
	{
		if(mWavFileNames[index].length() && mSoundSourceEnable[index])
		{
			float deltaX = m_Listener.headX - m_SoundSources[index].speakerX;
			float deltaZ = m_Listener.headZ - m_SoundSources[index].speakerZ;

			float radius = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);

			m_SoundSources[index].speakerX += 0.5;
			m_SoundSources[index].speakerZ += 0.5;

			m_pAudioEngine->updateSourcePosition(
				index,
				m_SoundSources[index].speakerX,
				m_SoundSources[index].speakerY,
				m_SoundSources[index].speakerZ
				);
		}
	}
}

void RoomAcousticQT::updateListenerPosition()
{
	m_pAudioEngine->updateHeadPosition(m_Listener.headX, m_Listener.headY, m_Listener.headZ,
		m_Listener.yaw, m_Listener.pitch, m_Listener.roll);
}

void RoomAcousticQT::updateRoomDimention()
{
	m_pAudioEngine->updateRoomDimension(m_RoomDefinition.width, m_RoomDefinition.height, m_RoomDefinition.length);
}

void RoomAcousticQT::updateRoomDamping()
{
	m_pAudioEngine->updateRoomDamping(m_RoomDefinition.mLeft.damp, m_RoomDefinition.mRight.damp, m_RoomDefinition.mTop.damp,
		m_RoomDefinition.mBottom.damp, m_RoomDefinition.mFront.damp, m_RoomDefinition.mBack.damp);
}

AmdTrueAudioVR* RoomAcousticQT::getAMDTrueAudioVR()
{
	return m_pAudioEngine->getAMDTrueAudioVR();
}


TANConverterPtr RoomAcousticQT::getTANConverter()
{
	return m_pAudioEngine->getTANConverter();
}
