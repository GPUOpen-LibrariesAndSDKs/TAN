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

#include <CL/cl.h>

#include "public/include/core/Interface.h"
#include "public/include/core/Data.h"
#include "public/include/core/PropertyStorageEx.h"

#ifdef RTQ_ENABLED
///This section is not related to RTQ, only listed so it can be strip away
///TAN_VERSION_MAJOR and TAN_VERSION_MINOR is use for version tracking
///TAN_VERSION_RELEASE == 1 (GPU Open)
///TAN_VERSION_RELEASE == 2 (General NDA)
///TAN_VERSION_RELEASE == 3...10 (specific NDA)
///TAN_VERSION_BUILD      build CL#, or Build time, or ...
#endif

#define TAN_VERSION_MAJOR          1
#define TAN_VERSION_MINOR          3
#define TAN_VERSION_RELEASE        1
#define TAN_VERSION_BUILD          20

#define TAN_FULL_VERSION ( (uint64_t(TAN_VERSION_MAJOR) << 48ull) |   \
                           (uint64_t(TAN_VERSION_MINOR) << 32ull) |   \
                           (uint64_t(TAN_VERSION_RELEASE) << 16ull) | \
                            uint64_t(TAN_VERSION_BUILD))

//define export declaration
#ifdef _WIN32
    #if defined(TAN_SDK_EXPORTS)
        #define TAN_SDK_LINK __declspec(dllexport)
    #else
        #define TAN_SDK_LINK __declspec(dllimport)
    #endif

    #define TAN_CDECL_CALL __cdecl
#else // #ifdef _WIN32
    #define TAN_SDK_LINK

    #define TAN_CDECL_CALL
#endif // #ifdef _WIN32


#define TAN_OUTPUT_MEMORY_TYPE         L"OutputMemoryType" // Values : AMF_MEMORY_OPENCL or AMF_MEMORY_HOST

namespace amf
{
    enum TAN_SAMPLE_TYPE
    {
        TAN_SAMPLE_TYPE_FLOAT       = 0,
        TAN_SAMPLE_TYPE_SHORT       = 1,
    };

    class TANContext;

    enum TAN_CONVOLUTION_METHOD 
    {
        TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD,             // [CPU processing] FFT overlap add algorithm. Processes bufSize samples at a time.
        TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_UNIFORM,     // [CPU processing] FFT convolution using uniform partitions. Efficiently processes bufSize samples at a time.
		TAN_CONVOLUTION_METHOD_FFT_PARTITIONED_NONUNIFORM,     // [CPU processing] FFT convolution using nonuniform partitions.
        //Graal methods
        TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED,     // Uniform Partitioned FFT algorithm. Processes bufSize samples at a time.
        TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED,     // Uniform Partitioned FHT algorithm. Processes bufSize samples at a time.
        TAN_CONVOLUTION_METHOD_FFT_UNIFORM_HEAD_TAIL,       // Uniformed, convolution performed in 2 stages head and tail
        TAN_CONVOLUTION_METHOD_FHT_UNIFORM_HEAD_TAIL,       // Uniformed, convolution performed in 2 stages head and tail

        // 
        TAN_CONVOLUTION_METHOD_TIME_DOMAIN,                 // pure time domain convolution. Processes from 1 to length samples at a time.
        TAN_CONVOLUTION_METHOD_FHT_NONUNIFORM_PARTITIONED,
        TAN_CONVOLUTION_METHOD_FFT_NONUNIFORM_PARTITIONED,  // Non-Uniform Partitioned FFT algorithm. Processes bufSize samples at a time.
		TAN_CONVOLUTION_METHOD_USE_PROCESS_FINALIZE = 0x8000, // use ProcessFinalize() optimization for HEAD_TAIL mode called from external thread
		TAN_CONVOLUTION_METHOD_USE_PROCESS_TAILTHREAD = 0xC000, // use ProcessFinalize() optimization for HEAD_TAIL mode called from internal thread
	};

   // Per-channel buffer flags.
    //
    // STOP_INPUT    - let's the channel's stream to fade out, new input isn't applied while this
    //                 flag is set.
    // FLUSH_STREAM  - flushes channel's output to zeros, but keeps applying the following inputs.
    enum TAN_CONVOLUTION_CHANNEL_FLAG
    {
        TAN_CONVOLUTION_CHANNEL_FLAG_PROCESS = 0,
        TAN_CONVOLUTION_CHANNEL_FLAG_STOP_INPUT     = 0x01,
        TAN_CONVOLUTION_CHANNEL_FLAG_FLUSH_STREAM   = 0x02,
    };

    // Flags to set the behavior of TANConvolution object.
    //
    // BLOCK_UNTIL_READY - Update() methods block CPU thread until new response is ready to be used
    //                     in the next Process() call.
    enum TAN_CONVOLUTION_OPERATION_FLAG
    {
        TAN_CONVOLUTION_OPERATION_FLAG_NONE                 = 0x00,
        TAN_CONVOLUTION_OPERATION_FLAG_BLOCK_UNTIL_READY    = 0x01,
    };

    //----------------------------------------------------------------------------------------------
    // TANConvolution interface
    //----------------------------------------------------------------------------------------------
    class TANConvolution : virtual public AMFPropertyStorageEx
    {
    public:
        AMF_DECLARE_IID(0x99d46c18, 0xc92, 0x40bd, 0x8a, 0xc7, 0x1c, 0x30, 0xf7, 0xe4, 0xeb, 0xcb)

        // Initialization function.
        //
        // Note: this method allocates internal buffers and initializes internal structures. Should
        // only be called once.
        virtual	AMF_RESULT	AMF_STD_CALL	Init(TAN_CONVOLUTION_METHOD convolutionMethod,
                                                 amf_uint32 responseLengthInSamples,
                                                 amf_uint32 bufferSizeInSamples,
                                                 amf_uint32 channels) = 0;
        // Slated to be removed
        virtual AMF_RESULT  AMF_STD_CALL    InitCpu(TAN_CONVOLUTION_METHOD convolutionMethod,
                                                    amf_uint32 responseLengthInSamples,
                                                    amf_uint32 bufferSizeInSamples,
                                                    amf_uint32 channels) = 0;
        virtual AMF_RESULT  AMF_STD_CALL    InitGpu(TAN_CONVOLUTION_METHOD convolutionMethod,
                                                    amf_uint32 responseLengthInSamples,
                                                    amf_uint32 bufferSizeInSamples,
                                                    amf_uint32 channels) = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        // Time domain float data update responce functions.
        //
        // Note: kernel is time domain data, and if shorter or longer than length specified in
        // Init(), it will be truncated or zero padded to fit.
        // Note: buffer contains 'channels' arrays of impulse response data for each channel.
        // Note: there should be as many 'states' and 'flagMasks' as channels in the buffer (set in
        // Init() method).
        virtual AMF_RESULT AMF_STD_CALL UpdateResponseTD(float* ppBuffer[],
                                                         amf_size numOfSamplesToProcess,
                                                         const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                         const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                         ) = 0;
        virtual AMF_RESULT AMF_STD_CALL UpdateResponseTD(cl_mem ppBuffer[],
                                                         amf_size numOfSamplesToProcess,
                                                         const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                         const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                         ) = 0;


        // Frequency domain float data update responce functions.
        //
        // Note: kernel is frequency domain complex float data, must be 2 * length specified in
        // Init().
        // Note: buffer contains 'channels' arrays of impulse response data for each channel.
        // Note: there should be as many 'flags' as channels in the buffer (set in Init() method).
        // Note: not currently implemented.
        virtual AMF_RESULT  AMF_STD_CALL    UpdateResponseFD(float* ppBuffer[],
                                                             amf_size numOfSamplesToProcess,
                                                             const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                             const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                             ) = 0;
        virtual AMF_RESULT  AMF_STD_CALL    UpdateResponseFD(cl_mem ppBuffer[],
                                                             amf_size numOfSamplesToProcess,
                                                             const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                             const amf_uint32 operationFlags // Mask of flags from enum TAN_CONVOLUTION_OPERATION_FLAG.
                                                             ) = 0;
 
        // Convolution process functions.
        //
        // ppBufferInput            - pointer to a channels long array of arrays of floats to be processed
        // ppBufferOutput            - pointer to a channels long array of arrays of floats to take output
        // numOfSamplesToProcess    - number of samples, from each array, of input samples to process 
        // pNumOfSamplesProcessed    - number of samples, from each array, actually processed.
        // 
        // On success:  returns AMF_OK and pNumOfSamplesProcessed will contain number of samples actually processed. This 
        // will be numOfSamplesToProcess, rounded down to next lower integral number of bufSize samples. Also see
        // Init().
        // On failure: returns appropriate AMF_RESULT value. 
        // Process system memory buffers:
        virtual AMF_RESULT  AMF_STD_CALL    Process(float* ppBufferInput[],
                                                    float* ppBufferOutput[],
                                                    amf_size numOfSamplesToProcess,
                                                    const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                    amf_size *pNumOfSamplesProcessed // Can be NULL.
                                                    ) = 0; 
        // Process OpenCL cl_mem buffers at output, host memory buffers at input:
        virtual AMF_RESULT  AMF_STD_CALL    Process(float* pBufferInput[],
                                                    cl_mem pBufferOutput[],
                                                    amf_size numOfSamplesToProcess,
                                                    const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                    amf_size *pNumOfSamplesProcessed // Can be NULL.
                                                    ) = 0;

        // Process OpenCL cl_mem buffers:
        virtual AMF_RESULT  AMF_STD_CALL    Process(cl_mem pBufferInput[],
                                                    cl_mem pBufferOutput[],
                                                    amf_size numOfSamplesToProcess,
                                                    const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                    amf_size *pNumOfSamplesProcessed // Can be NULL.
                                                    ) = 0; 

        // Process direct (no update required), system memory buffers:
        virtual AMF_RESULT  AMF_STD_CALL    ProcessDirect(
                                                    float* ppImpulseResponse[],
                                                    float* ppBufferInput[],
                                                    float* ppBufferOutput[],
                                                    amf_size numOfSamplesToProcess,
                                                    amf_size *pNumOfSamplesProcessed,
                                                    int *nzFirstLast = NULL
                                                    ) = 0;

		virtual AMF_RESULT  AMF_STD_CALL    ProcessFinalize(void) = 0;

        // Process direct (no update required),  OpenCL cl_mem  buffers:
        virtual AMF_RESULT  AMF_STD_CALL    ProcessDirect(
                                                cl_mem* ppImpulseResponse[],
                                                cl_mem* ppBufferInput[],
                                                cl_mem* ppBufferOutput[],
                                                amf_size numOfSamplesToProcess,
                                                amf_size *pNumOfSamplesProcessed,
                                                int *nzFirstLast = NULL
                                                ) = 0;


        // Returns index for a stopped channel which's stopped fading out (or was flushed), if
        // available; if no such channel can be found AMF_NOT_FOUND is returned.
        virtual AMF_RESULT AMF_STD_CALL GetNextFreeChannel(amf_uint32 *pChannelIndex,
                                                           const amf_uint32 flagMasks[] // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                           ) = 0;
    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANConvolution> TANConvolutionPtr;


    //----------------------------------------------------------------------------------------------
    // TANIIRfilter interface
    //----------------------------------------------------------------------------------------------
    class TANIIRfilter : virtual public AMFPropertyStorageEx
    {
    public:
        // {0B50B639-709D-476E-AADA-FAECB3E854FA}
        AMF_DECLARE_IID(0xb50b639, 0x709d, 0x476e, 0xaa, 0xda, 0xfa, 0xec, 0xb3, 0xe8, 0x54, 0xfa)

            // Initialization function.
            //
            // Note: this method allocates internal buffers and initializes internal structures. Should
            // only be called once.
            virtual	AMF_RESULT	AMF_STD_CALL	Init(
            amf_uint32 numInputTaps, 
            amf_uint32 numOutputTaps,
			amf_uint32 bufferSizeInSamples,
            amf_uint32 channels) = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        virtual AMF_RESULT AMF_STD_CALL UpdateIIRResponses(
            float* ppInputResponse[],
            float* ppOutputResponse[],
            amf_size inResponseSz, amf_size outResponseSz,
            const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
            const amf_uint32 operationFlags // Mask of flags from enum TAN_IIR_OPERATION_FLAG.
            ) = 0;

       virtual AMF_RESULT  AMF_STD_CALL    Process(
           float* ppBufferInput[],
            float* ppBufferOutput[],
            amf_size numOfSamplesToProcess,
            const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
            amf_size *pNumOfSamplesProcessed // Can be NULL.
            ) = 0;

	   virtual AMF_RESULT  AMF_STD_CALL    ProcessDirect(
		   float* ppBufferInput[],
		   float* ppBufferOutput[],
		   amf_size numOfSamplesToProcess,
		   const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
		   amf_size *pNumOfSamplesProcessed // Can be NULL.
	   ) = 0;

       virtual AMF_RESULT  AMF_STD_CALL    Process(
           cl_mem ppBufferInput[],
           cl_mem ppBufferOutput[],
           amf_size numOfSamplesToProcess,
           const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
           amf_size *pNumOfSamplesProcessed // Can be NULL.
           ) = 0;

    };

	//----------------------------------------------------------------------------------------------
	// smart pointer
	//----------------------------------------------------------------------------------------------
	typedef AMFInterfacePtr_T<TANIIRfilter> TANIIRfilterPtr;

    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANConvolution> TANConvolutionPtr;






    //----------------------------------------------------------------------------------------------
    // TANConverter interface
    //
    // Provides conversion between normalized FLOAT and SHORT representations.
    //
    // Converts an array of floats int the range - 1.0 -> + 1.0
    //    to or from
    //    an array of shorts int the range - 32767 -> + 32767
    //
    //    inputStep	interleave step size for inputBuffer.
    //    outputStep	interleave step size for outputBuffer.
    //
    //    conversionGain = 1.0 gives standard - 1.0 -> + 1.0 to / from - 32768 -> + 32768
    //
    //NOTE: to interleave or deinterleave data : Use step = 1 for mono data, 2 for stereo, etc.
    //----------------------------------------------------------------------------------------------
    class TANConverter : virtual public AMFPropertyStorageEx
    {
    public:
        AMF_DECLARE_IID(0x2c3e6c65, 0x1788, 0x4bbf, 0xb0, 0x70, 0xa1, 0x35, 0x8c, 0xbe, 0xe8, 0x9e)

        virtual AMF_RESULT  AMF_STD_CALL    Init() = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Convert(short* inputBuffer, amf_size inputStep, 
                                                    amf_size numOfSamplesToProcess, 
                                                    float* outputBuffer, amf_size outputStep,
                                                    float conversionGain) = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Convert(float* inputBuffer, amf_size inputStep,
                                                    amf_size numOfSamplesToProcess,
                                                    short* outputBuffer, amf_size outputStep, 
                                                    float conversionGain, bool* outputClipped = NULL) = 0;

        // Method for batch processing
        virtual AMF_RESULT  AMF_STD_CALL    Convert(short** inputBuffers, amf_size inputStep,
                                                    amf_size numOfSamplesToProcess,
                                                    float** outputBuffers, amf_size outputStep,
                                                    float conversionGain, 
                                                    int channels) = 0;
        // Method for batch processing
        virtual AMF_RESULT  AMF_STD_CALL    Convert(float** inputBuffers, amf_size inputStep,
                                                    amf_size numOfSamplesToProcess,
                                                    short** outputBuffers, amf_size outputStep,
                                                    float conversionGain, 
                                                    int channels, bool* outputClipped = NULL) = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Convert(cl_mem inputBuffer,
                                                    amf_size inputStep,
                                                    amf_size inputOffset,
                                                    TAN_SAMPLE_TYPE inputType, 
                                                    cl_mem outputBuffer,
                                                    amf_size outputStep,
                                                    amf_size outputOffset,
                                                    TAN_SAMPLE_TYPE outputType,                                                
                                                    amf_size numOfSamplesToProcess,
                                                    float conversionGain, bool* outputClipped = NULL) = 0;
        
        // Method for batch processing
        virtual AMF_RESULT  AMF_STD_CALL    Convert(cl_mem* inputBuffers,
													amf_size inputStep,
                                                    amf_size* inputOffsets,
                                                    TAN_SAMPLE_TYPE inputType,
                                                    cl_mem* outputBuffers,
                                                    amf_size outputStep,
													amf_size* outputOffsets,
                                                    TAN_SAMPLE_TYPE outputType,
                                                    amf_size numOfSamplesToProcess,
                                                    float conversionGain,                                                   
                                                    int count, bool* outputClipped = NULL) = 0;

    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANConverter> TANConverterPtr;

    //----------------------------------------------------------------------------------------------
    // TANMath interface
    //
    // Provides mathematical utility functions.
    //
    // buffers are arrays of channels pointers to floats, each at least numOfSamplesToProcess long.
    //----------------------------------------------------------------------------------------------
    class TANMath : virtual public AMFPropertyStorageEx
    {
    public:
        // {4B20F249-7286-4121-A153-12575011380D}
        AMF_DECLARE_IID(0x4b20f249, 0x7286, 0x4121, 0xa1, 0x53, 0x12, 0x57, 0x50, 0x11, 0x38, 0xd);

        virtual AMF_RESULT  AMF_STD_CALL    Init() = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        virtual AMF_RESULT ComplexMultiplication(		const float* const inputBuffers1[],
														const float* const inputBuffers2[],
														float *outputBuffers[],
														amf_uint32 channels,
														amf_size numOfSamplesToProcess) = 0;

        virtual AMF_RESULT ComplexMultiplication(		const cl_mem inputBuffers1[],
														const amf_size buffers1OffsetInSamples[],
														const cl_mem inputBuffers2[],
														const amf_size buffers2OffsetInSamples[],
														cl_mem outputBuffers[],
														const amf_size outputBuffersOffsetInSamples[],
														amf_uint32 channels,
														amf_size numOfSamplesToProcess) = 0;

		virtual AMF_RESULT ComplexMultiplyAccumulate(	const float* const inputBuffers1[],
														const float* const inputBuffers2[],
														float *accumbuffers[],
														amf_uint32 channels,
														amf_size numOfSamplesToProcess) = 0;

		virtual AMF_RESULT PlanarComplexMultiplyAccumulate(const float* const inputBuffers1[],
			const float* const inputBuffers2[],
			float *accumbuffers[],
			amf_uint32 channels,
			amf_size numOfSamplesToProcess,
			amf_uint riPlaneSpacing) = 0;
#ifdef USE_IPP
		virtual AMF_RESULT IPPComplexMultiplyAccumulate(const float* const inputBuffers1[],
			const float* const inputBuffers2[],
			float *accumbuffers[],
			float *workbuffer[],
			amf_uint32 channels,
			amf_size numOfSamplesToProcess) = 0;
#endif

		virtual AMF_RESULT ComplexMultiplyAccumulate(const cl_mem inputBuffers1,
			const cl_mem inputBuffers2,
			cl_mem accumBuffers,
			amf_uint32 channels,
			amf_size numOfSamplesToProcess,
			amf_size inputOffset1,
			amf_size inputOffset2,
			amf_size inputStride1,
			amf_size inputStride2) = 0;


        virtual AMF_RESULT ComplexDivision(				const float* const inputBuffers1[],
														const float* const inputBuffers2[],
														float *outputBuffers[],
														amf_uint32 channels,
														amf_size numOfSamplesToProcess) = 0;

        virtual AMF_RESULT ComplexDivision(				const cl_mem inputBuffers1[],
													   const amf_size buffers1OffsetInSamples[],
													   const cl_mem inputBuffers2[],
													   const amf_size buffers2OffsetInSamples[],
													   cl_mem outputBuffers[],
													   const amf_size outputBuffersOffsetInSamples[],
													   amf_uint32 channels,
													   amf_size numOfSamplesToProcess) = 0;
    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANMath> TANMathPtr;

    //----------------------------------------------------------------------------------------------
    // TANFFT interface
    //
    //----------------------------------------------------------------------------------------------
    enum TAN_FFT_TRANSFORM_DIRECTION
    {
        TAN_FFT_TRANSFORM_DIRECTION_FORWARD = 0,
        TAN_FFT_TRANSFORM_DIRECTION_BACKWARD = 1,
		TAN_FFT_R2C_TRANSFORM_DIRECTION_FORWARD = 2,
		TAN_FFT_C2R_TRANSFORM_DIRECTION_BACKWARD = 3,
		TAN_FFT_R2C_PLANAR_TRANSFORM_DIRECTION_FORWARD = 4,
		TAN_FFT_C2R_PLANAR_TRANSFORM_DIRECTION_BACKWARD = 5
	};

    class TANFFT : virtual public AMFPropertyStorageEx
    {
    public:
        AMF_DECLARE_IID(0x5f3965a1, 0x46c2, 0x4987, 0xb7, 0x1f, 0x3f, 0xf8, 0x1a, 0xe7, 0x5b, 0x6f)


        virtual	AMF_RESULT	AMF_STD_CALL	Init() = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        // FFT functions.
        //
        // Note: input and output arrays consist of pairs (real, imag).
        // Note: 'log2len' sets the length of the FFT's data, which is 2 ^ log2len * 2 (complex).
        // Note: CPU implementation currently returns unscaled results for backward transformation
        //       (multiplyed by 2 ^ log2len).
        // Note: Position and count functionality of TANAudioBuffer isn't supported.
        // pBufferInput     pointer to channels input vectors of floats, (complex R, I pairs), to be converted
        // pBufferOutput    pointer to channels output vectors of floats, (complex R, I pairs), result
        virtual AMF_RESULT  AMF_STD_CALL    Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                      amf_uint32 log2len,
                                                      amf_uint32 channels,
                                                      float* pBufferInput[],
                                                      float* pBufferOutput[]) = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                      amf_uint32 log2len,
                                                      amf_uint32 channels,
                                                      cl_mem pBufferInput[],
                                                      cl_mem pBufferOutput[]) = 0;

		virtual AMF_RESULT  AMF_STD_CALL    TransformBatchGPU(TAN_FFT_TRANSFORM_DIRECTION direction,
			amf_uint32 log2len,
			amf_uint32 channels,
			cl_mem pBufferInput,
			cl_mem pBufferOutput,
			int dataSpacing) = 0;

	};
	//----------------------------------------------------------------------------------------------
	// smart pointer
	//----------------------------------------------------------------------------------------------
	typedef AMFInterfacePtr_T<TANFFT> TANFFTPtr;

    class TANFilter : virtual public AMFPropertyStorageEx
    {
    public:
        // {7A6E4BBD-03F4-4AAB-9824-ED6935327E92}
        AMF_DECLARE_IID(0x7a6e4bbd, 0x03f4, 0x4aab, 0x98, 0x24, 0xed, 0x69, 0x35, 0x32, 0x7e, 0x92)

        virtual	AMF_RESULT	AMF_STD_CALL	Init() = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        // Standard 10 band octave equalizer
        // center frequencies: 31,62,125,250,500,1000,2000,4000,8000,16000
        virtual AMF_RESULT  AMF_STD_CALL    generate10BandEQ(amf_uint32 log2len,
                                                             float sampleRate,
                                                             float *impulseResponse,
                                                             float dbLevels[10]) = 0;
     };

    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANFilter> TANFilterPtr;

    //----------------------------------------------------------------------------------------------
    // TANContext interface:
    // TANContext may be initialized for OpenCL using either a cl_context, or one or two 
    // cl_command_queues.
    // the general queue may be shared by application kernels, the convolution queue is meant to be
    // dedicated for a convolution object.
    //
    // NOTE: If TANContext::InitOpenCL is not called, objects initialized with the context will 
    // use CPU processing only.
    //----------------------------------------------------------------------------------------------
    class TANContext : virtual public AMFPropertyStorage
    {
    public:
        AMF_DECLARE_IID(0x601a0b1e, 0x7659, 0x41f2, 0xb2, 0x6d, 0x6a, 0x16, 0x5f, 0x37, 0xa4, 0xa2);

        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;

        virtual AMF_RESULT  AMF_STD_CALL    InitOpenCL(
                                                cl_context pContext) = 0;
        virtual AMF_RESULT  AMF_STD_CALL    InitOpenCL(
                                                cl_command_queue pGeneralQueue = nullptr,
                                                cl_command_queue pConvolutionQueue = nullptr) = 0;
		virtual AMF_RESULT  AMF_STD_CALL    InitOpenMP(int nThreads) = 0;

        virtual cl_context   AMF_STD_CALL   GetOpenCLContext() = 0;
        virtual	cl_command_queue	AMF_STD_CALL	GetOpenCLGeneralQueue() = 0;
        virtual	cl_command_queue	AMF_STD_CALL	GetOpenCLConvQueue() = 0;
    };

    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANContext> TANContextPtr;

    //----------------------------------------------------------------------------------------------
    // TANMixer interface
    //
    // Mixes the input audio channels 
    //
    // Mixes a set of floating point arrays each representing one channel's audio samples
    //----------------------------------------------------------------------------------------------
    class TANMixer : virtual public AMFPropertyStorageEx
    {
    public:
        AMF_DECLARE_IID(0xd2432f7f, 0xc646, 0x4764, 0x9d, 0x61, 0x60, 0xd2, 0x7c, 0x7e, 0x20, 0xc7)

        virtual AMF_RESULT  AMF_STD_CALL    Init(amf_size buffer_size, int num_channels) = 0;
        virtual AMF_RESULT  AMF_STD_CALL    Terminate() = 0;
        virtual TANContext* AMF_STD_CALL    GetContext() = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Mix(float* ppBufferInput[],
                                                float* ppBufferOutput
                                                ) = 0;
        
        // For disjoint cl_mem input buffers, 
        virtual AMF_RESULT  AMF_STD_CALL    Mix(cl_mem pBufferInput[],
                                                cl_mem pBufferOutput
                                                ) = 0;

        // For contigous cl_mem input buffers. Each channel's sample offset is found using the inputStride and channel index.
        virtual AMF_RESULT  AMF_STD_CALL    Mix(cl_mem pBufferInput,
                                                cl_mem pBufferOutput,
                                                amf_size inputStride
                                                ) = 0;

    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANMixer> TANMixerPtr;
}

// TAN objects creation functions.
extern "C"
{
    // Creates a True Audio Next context. After the Context is initialized, it can be passed to creation functions
    // for Convolution, Converter, FFT, and Math objects.
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateContext(amf_uint64 version, amf::TANContext** ppContext);

    // Create a TANConvolution object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateConvolution(
                                                        amf::TANContext* pContext,
                                                        amf::TANConvolution** ppConvolution);
    // Create a TANConverter object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateConverter(
                                                        amf::TANContext* pContext,
                                                        amf::TANConverter** ppConverter);
    // Create a TANMixer object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateMixer(
                                                        amf::TANContext* pContext,
                                                        amf::TANMixer** ppMixer);
    //Create an TANFFT object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateFFT(
                                                        amf::TANContext* pContext,
                                                        amf::TANFFT** ppFFT);
    //Create an TANFilter object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateFilter(
                                                        amf::TANContext* pContext,
                                                        amf::TANFilter** ppFilter);
    // Create a TANMath object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateMath(
                                                        amf::TANContext* pContext,
                                                        amf::TANMath** ppMath);
    // Create a TANIIRfilter object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateIIRfilter(
                                                        amf::TANContext* pContext,
                                                        amf::TANIIRfilter** ppIIRfilter);

    // Set folder to cache compiled OpenCL kernels:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANSetCacheFolder(const wchar_t* path);
    TAN_SDK_LINK const wchar_t*     AMF_CDECL_CALL TANGetCacheFolder();
}

