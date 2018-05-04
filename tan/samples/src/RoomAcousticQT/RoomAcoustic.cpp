#include "RoomAcoustic.h"
#include "AclAPI.h"
#include "io.h"
#include <time.h>
#include <string>
#include <vector>
#include <Shlwapi.h>
#include "samples/src/common/GpuUtils.h"
#include "samples/src//TrueAudioVR/TrueAudioVR.h"
#include <direct.h>


RoomAcoustic::RoomAcoustic()
{
	initialize();
}

RoomAcoustic::~RoomAcoustic()
{
	delete m_cpConfigFilePath;
	delete m_cpLogPath;
	delete m_cpTANDLLPath;
}


void RoomAcoustic::initialize()
{
	initializeEnvironment();
	initializeAudioEngine();
	initializeRoom();
	initializeListener();
	initializeDevice();
}

int RoomAcoustic::start()
{
	// Remap the sound path to accomodates the audioVR engine
	// TODO: Need to port the configuration back to the audio3D engine

	//Initialize Audio 3D engine
	portInfoToEngine();
	// Since cpu's device id is 0 in this demo, we need to decrease the device id if
	// you want to run GPU
	int convolutiondevice = m_iConvolutionDeviceID;
	int Roomdevice = m_iRoomDeviceID;
	if (m_iuseGPU4Conv)
	{
		convolutiondevice--;
	}
	if (m_iuseGPU4Room)
	{
		Roomdevice--;
	}
	int err =  m_pAudioEngine->init(m_cpTANDLLPath, m_RoomDefinition,m_iNumOfWavFileInternal,m_cpWavFileNamesInternal, m_iConvolutionLength,
		m_iBufferSize, m_iuseGPU4Conv, convolutiondevice, 
#ifdef RTQ_ENABLED
		m_iuseMPr4Conv,m_iuseRTQ4Conv, m_iConvolutionCUCount,
#endif // RTQ_ENABLED
		m_iuseGPU4Room, Roomdevice,
#ifdef RTQ_ENABLED
		m_iuseMPr4Room,m_iuseRTQ4Room, m_iRoomCUCount, 
#endif
		m_eConvolutionMethod);
	m_pAudioEngine->setWorldToRoomCoordTransform(0., 0., 0., 0., 0., true);
	m_pAudioEngine->setSrc1Options(m_isrc1EnableMic, m_isrc1TrackHeadPos);
	updateAllSoundSourcesPosition();
	updateListenerPosition();
	return m_pAudioEngine->Run();
}

void RoomAcoustic::stop()
{
	m_pAudioEngine->Stop();
}

void GetFileVersionAndDate(WCHAR *logMessage, char *version){
	time_t dt = time(NULL);
	struct tm *lt = localtime(&dt);
	DWORD size;
	DWORD dummy;
	WCHAR filename[1024];
	unsigned int len;
	//filename = 0x00000075cfd2e470 L"C:\\lomo\\zli2_TAN_Mark1\\TAN_amir\\build\\solution\\..\\..\\bin\\vs2013x64Debug\\RoomAcousticsFull64.exe"
	//filename = 0x0000009340dce720 L"C:\\lomo\\zli2_TAN_Mark1\\TAN_amir\\build\\solution\\..\\..\\bin\\vs2013x64Debug\\RoomAccousticNew64.exe"
	GetModuleFileNameW(NULL, filename, sizeof(filename) / sizeof(filename[0]));
	size = GetFileVersionInfoSizeW(filename, &dummy);
	DWORD a = GetLastError();
	WCHAR * buffer = new WCHAR[size];
	WCHAR *ver = NULL, *pStr;
	WCHAR *pSrch = L"FileVersion";
	if (buffer == NULL){ return; }
	if (GetFileVersionInfoW(filename, 0, size, (void*)buffer)){
		pStr = buffer;
		while (pStr < buffer + size){
			if (wcsncmp(pStr, pSrch, 10) == 0){
				ver = pStr + wcslen(pStr) + 2;
				break;
			}
			else {
				pStr++;
			}
		}
	}



	wsprintfW(logMessage, L"**** %s v%s on %4d/%02d/%02d %02d:%02d:%02d ****\n", filename, ver,
		2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

	int lv = wcslen(ver);
	for (int i = 0; i < lv; i++){
		version[i] = char(ver[i]);
	}
	delete []buffer;
}


void RoomAcoustic::initializeEnvironment()
{
	this->m_cpConfigFilePath = new char[MAX_PATH + 1];
	this->m_cpTANDLLPath = new char[MAX_PATH + 1];
	this->m_cpLogPath = new char[MAX_PATH + 1];
	GetModuleFileNameA(NULL, this->m_cpTANDLLPath, MAX_PATH);
	char *pslash = strrchr(this->m_cpTANDLLPath, '\\');
	char exename[80] = "\0";
	if (pslash){
		strncpy(exename, pslash + 1, 80);
		*pslash = '\0';
	}
	char *pdot = strrchr(exename, '.');
	if (pdot)
		*pdot = '\0';


	//strncpy(dllPath, "c:\\TANSDK\\testapp",80); //hack
	strncpy(this->m_cpConfigFilePath, this->m_cpTANDLLPath, MAX_PATH);
	// Change current working directory to executable directory
	_chdir(this->m_cpConfigFilePath);
	strncpy(this->m_cpLogPath, this->m_cpTANDLLPath, MAX_PATH);

	strncat(this->m_cpConfigFilePath, "\\default.xml", MAX_PATH);
	//strncat_s(logPath, "\\TaLibVRDemoLog.log", MAX_PATH);
	strncat(this->m_cpLogPath, "\\", MAX_PATH);
	strncat(this->m_cpLogPath, exename, MAX_PATH);
	strncat(this->m_cpLogPath, ".log", MAX_PATH);

	FILE *fpLog = NULL;
	errno_t err = 0;
	errno = 0;
	// Redirect stdout and stderr to a log file. 

	if (fopen_s(&fpLog, this->m_cpLogPath, "a+") == 0) {
		dup2(fileno(fpLog), fileno(stdout));
		dup2(fileno(fpLog), fileno(stderr));
	}
	//else if (fopen_s(&fpLog, "TaLibVRDemoLog.log", "a+") == 0) {
	//	dup2(fileno(fpLog), fileno(stdout));
	//	dup2(fileno(fpLog), fileno(stderr));
	//}

	WCHAR logDateVerStamp[MAX_PATH * 2];
	char version[40];
	memset(version, 0, sizeof(version));
	GetFileVersionAndDate(logDateVerStamp, version);
	wprintf(logDateVerStamp);

	for (int idx = 0; idx < MAX_SOURCES; idx++){
		m_cpWavFileNames[idx] = new char[MAX_PATH + 2];
		m_cpWavFileNames[idx][0] = '\0';
	}

	for (int index = 0; index < MAX_SOURCES; ++index)
	{
		m_bSrcTrackHead[index] = false;
	}
}

void RoomAcoustic::initializeAudioEngine()
{
	m_pAudioEngine = new Audio3D();

}

void RoomAcoustic::initializeRoom()
{
	m_RoomDefinition.height = 4.0;
	m_RoomDefinition.width = 6.0;
	m_RoomDefinition.length = 10.0;
	m_RoomDefinition.mFront.damp = DBTODAMP(2.0);
	m_RoomDefinition.mBack.damp = DBTODAMP(6.0);
	m_RoomDefinition.mLeft.damp = m_RoomDefinition.mRight.damp = DBTODAMP(4.0);
	m_RoomDefinition.mTop.damp = m_RoomDefinition.mBottom.damp = DBTODAMP(2.0);

	for (int idx = 0; idx < MAX_SOURCES; idx++) {
		m_SoundSources[idx].speakerX = m_RoomDefinition.width / 2 + idx*m_RoomDefinition.width / MAX_SOURCES;
		m_SoundSources[idx].speakerZ = float(m_RoomDefinition.width * 0.05);
		m_SoundSources[idx].speakerY = 1.75;
		m_iSoundSourceEnable[idx] = 0;
		m_cpWavFileNamesInternal[idx] = nullptr;
	}

	for (int i = 0; i < MAX_SOURCES; i++)
	{
		m_iSoundSourceMap[i] = 0;
	}
}

void RoomAcoustic::initializeListener()
{
	m_Listener.earSpacing = float(0.16);
	m_Listener.headX = float(m_RoomDefinition.width*.8);
	m_Listener.headZ = float(m_RoomDefinition.length*.8);
	m_Listener.headY = 1.75;
	m_Listener.pitch = 0.0;
	m_Listener.roll = 0.0;
	m_Listener.yaw = 0.0;
}

void RoomAcoustic::initializeDevice()
{
	m_iConvolutionLength = 32768;
	m_iBufferSize = 2048;
	for (int i = 0; i < MAX_DEVICES; i++){
		m_cpDeviceName[i] = new char[MAX_PATH + 2];
		memset(m_cpDeviceName[i], 0, (MAX_PATH + 2));
	}
	m_iDeviceCount = listGpuDeviceNames(m_cpDeviceName, MAX_DEVICES);
}

bool RoomAcoustic::parseElement(char* start, char* end, element* elem)
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

bool RoomAcoustic::findElement(char** start, char** end, char* name)
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

void RoomAcoustic::portInfoToEngine()
{
	// Remap the source name so that it maps the audio3D engine
	int soundnameindex = 0;
	m_iNumOfWavFileInternal = 0;
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (m_cpWavFileNamesInternal[i] != NULL)
		{
			delete[] m_cpWavFileNamesInternal[i];
			m_cpWavFileNamesInternal[i] = nullptr;
		}
		m_cpWavFileNamesInternal[soundnameindex] = new char[MAX_PATH];
		if (m_cpWavFileNames[i][0] != '\0' && m_iSoundSourceEnable[i])
		{
			strcpy(m_cpWavFileNamesInternal[soundnameindex], m_cpWavFileNames[i]);
			m_iSoundSourceMap[i] = soundnameindex;
			soundnameindex++;
			m_iNumOfWavFileInternal++;
		}
	}
}

void RoomAcoustic::loadConfiguration(char* xmlfilename)
{
	// Creating internal structre and prepare for xml loading
	// attribute src1PosAttribs[3] = { { "X", &m_SoundSources->speakerX, 'f' }, { "Y", &srcY[0], 'f' }, { "Z", &srcZ[0], 'f' } };
	element RAelementList[MAX_SOURCES + 3];
	std::vector<attribute*> attributes_list;
	std::vector<element*> elements_list;
	for (int i = 0; i < MAX_SOURCES; i++)
	{	
		attribute* srcPosAttribs = new attribute[3];
		attributes_list.push_back(srcPosAttribs);
		srcPosAttribs[0].name = "X";
		srcPosAttribs[0].value = &(m_SoundSources[i].speakerX);
		srcPosAttribs[0].fmt = 'f';
		srcPosAttribs[1].name = "Y";
		srcPosAttribs[1].value = &(m_SoundSources[i].speakerY);
		srcPosAttribs[1].fmt = 'f';
		srcPosAttribs[2].name = "Z";
		srcPosAttribs[2].value = &(m_SoundSources[i].speakerZ);
		srcPosAttribs[2].fmt = 'f';

		RAelementList[i].name = new char[MAX_PATH];
		attribute *streamAttribs = new attribute[2];
		attributes_list.push_back(streamAttribs);
		streamAttribs[0].name = "on";
		streamAttribs[0].value = &m_iSoundSourceEnable[i];
		streamAttribs[0].fmt = 'i';

		streamAttribs[1].name = "file";
		streamAttribs[1].value = &m_cpWavFileNames[i][0];
		streamAttribs[1].fmt = 's';
		element *src = nullptr;
		
		if (i == 0){
			src = new element[3];
			elements_list.push_back(src);
			attribute* src1MicAttribs = new attribute[3];
			attributes_list.push_back(src1MicAttribs);
			src1MicAttribs[0].name = "enableMic";
			src1MicAttribs[0].value = &m_isrc1EnableMic;
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
			_stream += std::to_string(i+1);
			_position += std::to_string(i+1);
			src[0].name = new char[MAX_PATH];
			src[1].name = new char[MAX_PATH];
			strncpy_s(src[0].name, MAX_PATH, _stream.c_str(), MAX_PATH);
			strncpy_s(src[1].name, MAX_PATH, _position.c_str(), MAX_PATH);
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
		_Source += std::to_string(i+1);
		
		strncpy_s(RAelementList[i].name, MAX_PATH, _Source.c_str(), MAX_PATH);
		RAelementList[i].nAttribs = 0;
		RAelementList[i].attriblist = NULL;
		
		
	}

	// Initialize Listener Attributes
	attribute headPosAttribs[6] = { { "X", &m_Listener.headX, 'f' }, { "Y", &m_Listener.headY, 'f' }, { "Z", &m_Listener.headZ, 'f' },
	{ "yaw", &m_Listener.yaw, 'f' }, { "pitch", &m_Listener.pitch, 'f' }, { "roll", &m_Listener.roll, 'f' } };
	attribute earAttribs[1] = { { "S", &m_Listener.earSpacing, 'f' } };
	struct element head[2] = {
		{ "positionL1", 6, headPosAttribs, 0, NULL },
		{ "earSpacing", 1, earAttribs, 0, NULL },
	};

	// Initialize Room Attributes
	attribute roomDimAttribs[3] = { { "width", &m_RoomDefinition.width, 'f' }, { "height", &m_RoomDefinition.height, 'f' }, { "length", &m_RoomDefinition.length, 'f' } };
	attribute roomDampAttribs[6] =
	{ { "left", &m_RoomDefinition.mLeft.damp, 'f' }, { "right", &m_RoomDefinition.mRight.damp, 'f' },
	{ "front", &m_RoomDefinition.mFront.damp, 'f' }, { "back", &m_RoomDefinition.mBack.damp, 'f' },
	{ "top", &m_RoomDefinition.mTop.damp, 'f' }, { "bottom", &m_RoomDefinition.mBottom.damp, 'f' } };
#ifdef RTQ_ENABLED
	attribute roomRenderAttribs[5] = { { "nSources", &m_iNumOfWavFile, 'i' }, { "withGPU", &m_iuseGPU4Room, 'i' }, { "withRTQ", &m_iuseRTQ4Room, 'i' }, { "withMPr", &m_iuseMPr4Room, 'i' }, { "withCus", &m_iRoomCUCount, 'i' } };
#else
	attribute roomRenderAttribs[3] = { { "nSources", &m_iNumOfWavFile, 'i' }, { "withGPU", &m_iuseGPU4Room, 'i' }, { "withMPr", &m_iuseMPr4Room, 'i' }};
#endif // RTQ_ENABLED
	element roomElems[3] = {
		{ "dimensions", 3, roomDimAttribs, NULL },
		{ "damping", 6, roomDampAttribs, NULL },

#ifdef RTQ_ENABLED
		{ "rendering", 5, roomRenderAttribs, NULL }
#else
		{ "rendering", 3, roomRenderAttribs, NULL }
#endif // RTQ_ENABLED

	};

	// Initialize Convolution attribute

#ifdef RTQ_ENABLED
	struct attribute convCfgAttribs[7] = {
		{ "length", &m_iConvolutionLength, 'i' },
		{ "buffersize", &m_iBufferSize, 'i' },
		{ "useGPU", &m_iuseGPU4Conv, 'i' },
		{ "useRTQ", &m_iuseRTQ4Conv, 'i' },
		{ "useMpr", &m_iuseMPr4Conv, 'i' },
		{ "cuCount", &m_iConvolutionCUCount, 'i' },
		{ "method", &m_eConvolutionMethod, 'i' }
	};
	struct element convElems[1] = {
		{ "configuration", 7, convCfgAttribs, 0, NULL }
	};
#else
	struct attribute convCfgAttribs[5] = {
		{ "length", &m_iConvolutionLength, 'i' },
		{ "buffersize", &m_iBufferSize, 'i' },
		{ "useGPU", &m_iuseGPU4Conv, 'i' },
		{ "useMpr", &m_iuseMPr4Conv, 'i' },
		{ "method", &m_eConvolutionMethod, 'i' }
	};
	struct element convElems[1] = {
		{ "configuration", 5, convCfgAttribs, 0, NULL }
	};
#endif // RTQ_ENABLED
	RAelementList[MAX_SOURCES].name = "Listener";
	RAelementList[MAX_SOURCES].nElements = sizeof(head) / sizeof(element);
	RAelementList[MAX_SOURCES].elemList = head;
	
	RAelementList[MAX_SOURCES + 1].name = "Room";
	RAelementList[MAX_SOURCES + 1].nElements = sizeof(roomElems) / sizeof(element);
	RAelementList[MAX_SOURCES + 1].elemList = roomElems;


	RAelementList[MAX_SOURCES + 2].name = "Convolution";
	RAelementList[MAX_SOURCES + 2].nElements = sizeof(convElems) / sizeof(element);
	RAelementList[MAX_SOURCES + 2].elemList = convElems;

	RAelementList[MAX_SOURCES].nAttribs = RAelementList[MAX_SOURCES + 1].nAttribs = RAelementList[MAX_SOURCES + 2].nAttribs = 0;
	RAelementList[MAX_SOURCES].attriblist = RAelementList[MAX_SOURCES + 1].attriblist = RAelementList[MAX_SOURCES + 2].attriblist = NULL;

	struct element RoomAcoustics = { "RoomAcoustics", 0, NULL, sizeof(RAelementList) / sizeof(element), RAelementList };

	FILE *fpLoadFile = NULL;

	fopen_s(&fpLoadFile, xmlfilename, "r+");

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

}

/* Save Room acoustic configuration in xml file*/
void RoomAcoustic::saveConfiguraiton(char* xmlfilename)
{
	time_t dt = time(NULL);
	struct tm *lt = localtime(&dt);
	FILE *fpSaveFile = NULL;

	fopen_s(&fpSaveFile, xmlfilename, "w+");

	if (fpSaveFile == NULL){
		return;
	}

	fputs("<?xml version='1.0' encoding='UTF-8'?>\n", fpSaveFile);
	fprintf(fpSaveFile,
		"<!-- This document was created by AMD RoomAcousticsDemo v1.0.2 on %4d/%02d/%02d %02d:%02d:%02d -->\n",
		2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
	fputs("<RoomAcoustics>\n", fpSaveFile);
	fputs("<!-- All dimensions in meters, damping in decibels -->\n", fpSaveFile);

	fputs(" <Source1>\n", fpSaveFile);
	fprintf(fpSaveFile, " <streamS1  on=\"%d\" file=\"%s\" />\n", m_iSoundSourceEnable[0], m_cpWavFileNames[0]);
	fprintf(fpSaveFile, "  <microphone enableMic=\"%d\" trackHeadPos=\"%d\" muteDirectPath=\"%d\" />\n",
		m_isrc1EnableMic, m_isrc1TrackHeadPos, m_isrc1MuteDirectPath);
	fprintf(fpSaveFile, "  <positionS1 X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", m_SoundSources[0].speakerX, m_SoundSources[0].speakerY, m_SoundSources[0].speakerZ);
	fputs(" </Source1>\n", fpSaveFile);

	for (int i = 1; i < MAX_SOURCES; i++) {
		fprintf(fpSaveFile, " <Source%d>\n", i + 1);
		fprintf(fpSaveFile, " <streamS%d  on=\"%d\" file=\"%s\" />\n", i + 1, m_iSoundSourceEnable[i], m_cpWavFileNames[i]);
		fprintf(fpSaveFile, "  <positionS%d X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", i + 1, m_SoundSources[i].speakerX, m_SoundSources[i].speakerY, m_SoundSources[i].speakerZ);
		fprintf(fpSaveFile, " </Source%d>\n", i + 1);
	}

	fputs(" <Listener>\n", fpSaveFile);
	fprintf(fpSaveFile, "  <positionL1 X=\"%f\" Y=\"%f\" Z=\"%f\" yaw=\"%f\" pitch=\"%f\" roll=\"%f\" />\n",
		m_Listener.headX, m_Listener.headY, m_Listener.headZ, m_Listener.yaw, m_Listener.pitch, m_Listener.roll);
	fprintf(fpSaveFile, "  <earSpacing S=\"%f\"/>\n", m_Listener.earSpacing);

	fputs(" </Listener>\n", fpSaveFile);
	fputs(" <Room>\n", fpSaveFile);
	fprintf(fpSaveFile, "  <dimensions width=\"%f\" height=\"%f\" length=\"%f\" />\n", m_RoomDefinition.width, m_RoomDefinition.height, m_RoomDefinition.length);
	fprintf(fpSaveFile, "  <damping left=\"%f\" right=\"%f\" front=\"%f\" back=\"%f\" top=\"%f\" bottom=\"%f\"/>\n",
		DAMPTODB(m_RoomDefinition.mLeft.damp), DAMPTODB(m_RoomDefinition.mRight.damp), DAMPTODB(m_RoomDefinition.mFront.damp), DAMPTODB(m_RoomDefinition.mBack.damp), 
		DAMPTODB(m_RoomDefinition.mTop.damp), DAMPTODB(m_RoomDefinition.mBottom.damp));
#ifdef RTQ_ENABLED
	fprintf(fpSaveFile, " <rendering nSources=\"%d\" withGPU=\"%d\" withRTQ=\"%d\" withMPr=\"%d\" withCus=\"%d\"/>\n", m_iNumOfWavFile,
		m_iuseGPU4Room, m_iuseRTQ4Room, m_iuseMPr4Room, m_iRoomCUCount);
#else
	fprintf(fpSaveFile, " <rendering nSources=\"%d\" withGPU=\"%d\" withMPr=\"%d\"/>\n", m_iNumOfWavFile,
		m_iuseGPU4Room, m_iuseMPr4Room);
#endif // RTQ_ENABLED
	fputs(" </Room>\n", fpSaveFile);
	fputs("<Convolution>\n", fpSaveFile);
#ifdef RTQ_ENABLED
	fprintf(fpSaveFile, " <configuration length=\"%d\" buffersize =\"%d\" useGPU=\"%d\" useRTQ=\"%d\" useMPr=\"%d\" cuCount=\"%d\" method=\"%d\"/>\n",
		m_iConvolutionLength, m_iBufferSize, m_iuseGPU4Conv, m_iuseRTQ4Conv, m_iuseMPr4Conv, m_iConvolutionCUCount, m_eConvolutionMethod);
#else
	fprintf(fpSaveFile, " <configuration length=\"%d\" buffersize =\"%d\" useGPU=\"%d\" useMPr=\"%d\" method=\"%d\"/>\n",
		m_iConvolutionLength, m_iBufferSize, m_iuseGPU4Conv, m_iuseMPr4Conv, m_eConvolutionMethod);
#endif
	fputs("</Convolution>\n", fpSaveFile);
	fputs("</RoomAcoustics>\n", fpSaveFile);
	fclose(fpSaveFile);
}

int RoomAcoustic::addSoundSource(char* sourcename)
{
	char* extention = PathFindExtension(sourcename);
	if (strcmp(extention, ".wav"))
	{
		// Not a supported format, return
		return false;
	}
	else
	{
		// Find a empty sound source slots and assign to it.
		for (int i = 0; i < MAX_SOURCES; i++)
		{
			if (m_cpWavFileNames[i][0] == '\0')
			{
				m_cpWavFileNames[i] = strdup(sourcename);
				m_iNumOfWavFile++;
				return i;
			}
		}

		// Reach the source limit, exiting
		return -1;
	}
}

bool RoomAcoustic::replaceSoundSource(char* sourcename, int id)
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
		m_iSoundSourceEnable[id] = 0;
		if (id == 0)
		{
			m_isrc1TrackHeadPos = 0;
			m_isrc1EnableMic = 0;
			m_isrc1MuteDirectPath = 0;
		}
		delete[]m_cpWavFileNames[id];
		m_cpWavFileNames[id] = strdup(sourcename);
		return true;
	}
}

bool RoomAcoustic::removeSoundSource(const char* sourcename)
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (!strcmp(sourcename, m_cpWavFileNames[i]))
		{
			removeSoundSource(i);
		}
	}
	return true;
}

bool RoomAcoustic::removeSoundSource(int id)
{
	// Check if the id is valid
	if (id >= 0 && id < MAX_SOURCES)
	{
		// if the sound source exist
		if (m_cpWavFileNames[id][0] != '\0')
		{
			m_SoundSources[id].speakerY = 0.0f;
			m_SoundSources[id].speakerX = 0.0f;
			m_SoundSources[id].speakerZ = 0.0f;
			m_iSoundSourceEnable[id] = 0;
			if (id == 0)
			{
				m_isrc1TrackHeadPos = 0;
				m_isrc1EnableMic = 0;
				m_isrc1MuteDirectPath = 0;
			}
			// Clean file name
			m_cpWavFileNames[id][0] = '\0';
			m_iNumOfWavFile--;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

/*TODO: Need to rework this function for potential epislon comparison*/
bool RoomAcoustic::isInsideRoom(float x, float y, float z)
{
	return (x >= 0.0f && x <= m_RoomDefinition.width) &&
		(y >= 0.0f && y <= m_RoomDefinition.length) &&
		(z >= 0.0f && z <= m_RoomDefinition.height);
}

int RoomAcoustic::findSoundSource(const char* sourcename)
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (!strcmp(sourcename, m_cpWavFileNames[i]))
		{
			return i;
		}
	}
	return -1;
}

float RoomAcoustic::getReverbTime(float final_db, int* nreflections)
{
	return estimateReverbTime(this->m_RoomDefinition, final_db, nreflections);
}

float RoomAcoustic::getConvolutionTime()
{
	return m_iConvolutionLength / 48000.0f;
}

float RoomAcoustic::getBufferTime()
{
	return m_iBufferSize / 48000.0f;
}

void RoomAcoustic::getCPUConvMethod(std::string** _out, int* _num)
{
	int numberOfMethod = 1;
	std::string* output = new std::string[numberOfMethod];
	output[0] = "FFT OVERLAP ADD";
	*_out = output;
	*_num = numberOfMethod;
}

void RoomAcoustic::getGPUConvMethod(std::string** _out, int* _num)
{
	int numberOfMethod = 4;
	std::string* output = new std::string[numberOfMethod];
	output[0] = "FFT OVERLAP ADD";
	output[1] = "FFT UNIFORM PARTITIONED";
	output[2] = "FHT UNIFORM PARTITIONED";
	output[3] = "FHT UINFORM HEAD TAIL";
	*_out = output;
	*_num = numberOfMethod;
}

amf::TAN_CONVOLUTION_METHOD RoomAcoustic::getConvMethodFlag(const std::string& _name)
{
	if (_name == "FFT OVERLAP ADD")
		return TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	if (_name == "FFT UNIFORM PARTITIONED")
		return TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED;
	if (_name == "FHT UNIFORM PARTITIONED")
		return TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED;
	if (_name == "FHT UINFORM HEAD TAIL")
		return TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL;
}

void RoomAcoustic::updateAllSoundSourcesPosition()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (m_cpWavFileNames[i][0] != '\0' && m_iSoundSourceEnable[i] )
		{
			updateSoundSourcePosition(i);
		}
	}
}

void RoomAcoustic::updateSoundSourcePosition(int index)
{
	int i = m_iSoundSourceMap[index];
	m_pAudioEngine->updateSourcePosition(i, m_SoundSources[index].speakerX,
		m_SoundSources[index].speakerY, m_SoundSources[index].speakerZ);
}

void RoomAcoustic::updateListenerPosition()
{
	m_pAudioEngine->updateHeadPosition(m_Listener.headX, m_Listener.headY, m_Listener.headZ,
		m_Listener.yaw, m_Listener.pitch, m_Listener.roll);
}

void RoomAcoustic::updateRoomDimention()
{
	m_pAudioEngine->updateRoomDimension(m_RoomDefinition.width, m_RoomDefinition.height, m_RoomDefinition.length);
}

void RoomAcoustic::updateRoomDamping()
{
	m_pAudioEngine->updateRoomDamping(m_RoomDefinition.mLeft.damp, m_RoomDefinition.mRight.damp, m_RoomDefinition.mTop.damp,
		m_RoomDefinition.mBottom.damp, m_RoomDefinition.mFront.damp, m_RoomDefinition.mBack.damp);
}

AmdTrueAudioVR* RoomAcoustic::getAMDTrueAudioVR()
{
	return m_pAudioEngine->getAMDTrueAudioVR();
}


TANConverterPtr RoomAcoustic::getTANConverter()
{
	return m_pAudioEngine->getTANConverter();
}
