#include "common.h"
#include "tanlibrary/include/TrueAudioNext.h"
#include <ctime>

amf_uint random(int maxVal)
{
	// Return 'random' variables initialized with a seed
	srand((unsigned int)time(NULL));
	return rand() / ((RAND_MAX + maxVal + 1) / (maxVal + 1));
}

float random_float(float maxVal)
{
	return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxVal));
}