
__kernel void crossfade(
	__global	float*	fadeBuffer,		///< [in]
	__global	float*	outputBuffer	///< [in/out]
	)
{
	int gid = get_global_id(0);
	int gsize = get_global_size(0);
	outputBuffer[gid] = (fadeBuffer[gid] * gid + outputBuffer[gid] * (gsize - gid)) / (float) gsize;
}
