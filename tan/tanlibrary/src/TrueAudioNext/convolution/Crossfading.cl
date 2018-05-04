
__kernel void crossfade(
	__global	float*	fadeBuffer,		///< [in]
	__global	float*	outputBuffer,   ///< [in/out]
	int channelStride
	)
{
	int sampleId = get_global_id(0);
	int numSamples = get_global_size(0);
	int chId = get_global_id(1);
	int sampleOffset = sampleId + chId*channelStride;
	outputBuffer[sampleOffset] = (outputBuffer[sampleOffset] * sampleId + fadeBuffer[sampleOffset] * (numSamples - sampleId)) / (float)numSamples;
}
