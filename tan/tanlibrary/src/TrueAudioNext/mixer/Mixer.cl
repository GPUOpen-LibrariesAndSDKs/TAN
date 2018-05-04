// For contiguous input buffers
__kernel void Mixer(
    __global	float*	inputBuffer,	///< [in]
    __global	float*	outputBuffer,	///< [out]
    int inputStride,
    int numOfChannels
    )
{
    int sampId = get_global_id(0);

    float sum = 0;
    for (int i = 0; i < numOfChannels; i++)
    {
        sum += inputBuffer[i*inputStride + sampId];
    }

    outputBuffer[sampId] = sum;
}