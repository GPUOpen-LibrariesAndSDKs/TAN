#include "tanlibrary/include/TrueAudioNext.h"

#define QUEUE_NORMAL_QUEUE 0

#ifdef RTQ_ENABLED
#define QUEUE_MEDIUM_PRIORITY                   0x00010000
#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
#endif
#define RETURN_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return tmp; } }
#define RETURN_FALSE_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return false; } }
enum eTestMode
{
	eGPU,
	eCPU
};

enum eCommandQueueType
{
#ifdef RTQ_ENABLED
	eMediumPriorityQueue = 2,
	eRealTimeQueue = 1,
#endif
	eNormalQueue = 0
};

#ifdef RTQ_ENABLED
enum eCommandQueueType
{
	eMediumPriorityQueue,
	eRealTimeQueue
};
#endif RTQ_ENABLED


enum eOutputFlag
{
	eInput = 1,
	eReferenceOutput = 2,
	eTestOutput = 4,
	eDiff = 8,
	eResponse = 16
};

amf_uint random(int maxVal);

float random_float(float maxVal);