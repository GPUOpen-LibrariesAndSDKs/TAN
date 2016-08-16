#include "tanlibrary/include/TrueAudioNext.h"
#define QUEUE_MEDIUM_PRIORITY                   0x00010000
#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
#define RETURN_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return tmp; } }
#define RETURN_FALSE_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return false; } }
enum eTestMode
{
	eGPU,
	eCPU
};

enum eCommandQueueType
{
	eMediumPriorityQueue,
	eRealTimeQueue
};


amf_uint random(int maxVal);

float random_float(float maxVal);