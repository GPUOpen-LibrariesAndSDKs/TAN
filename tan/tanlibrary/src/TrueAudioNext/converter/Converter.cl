#define OVERFLOW_WARNING 1

__kernel void shortToShort(
	__global	short*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	short*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset	///< [in]
	)
{
	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = inputBuffer[(gid * inputStep) + inputOffset];
}

__kernel void floatToFloat(
	__global	float*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	float*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset	///< [in]
	)
{
	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = inputBuffer[(gid * inputStep) + inputOffset];
}

__kernel void shortToFloat(
	__global	short*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	float*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset,	///< [in]
				float	conversionGain	///< [in]
	)
{
	float scale = conversionGain / SHRT_MAX;

	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = convert_float( inputBuffer[(gid * inputStep) + inputOffset] ) * scale;
}

__kernel void floatToShort(
	__global	float*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	short*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset,	///< [in]
				float	conversionGain,	///< [in]
	__global	int*	overflowError	///< [out]
	)
{
	int gid = get_global_id(0);

	float scale = SHRT_MAX * conversionGain;
	float f = inputBuffer[(gid * inputStep) + inputOffset] * scale;

	if ( f > SHRT_MAX)
	{
		f = SHRT_MAX;
		*overflowError = OVERFLOW_WARNING;
	}

	if ( f < SHRT_MIN)
	{
		f = SHRT_MIN;
		*overflowError = OVERFLOW_WARNING;
	}

	outputBuffer[(gid * outputStep) + outputOffset] = convert_short( f );
}
