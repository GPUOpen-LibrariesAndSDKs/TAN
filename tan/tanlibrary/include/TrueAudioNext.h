//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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

#include "cl/cl.h"

#include "public/include/core/Interface.h"
#include "public/include/core/Data.h"
#include "public/include/core/PropertyStorageEx.h"

#define TAN_VERSION_MAJOR          1
#define TAN_VERSION_MINOR          1
#define TAN_VERSION_RELEASE        1
#define TAN_VERSION_BUILD          7

#define TAN_FULL_VERSION ( (unsigned __int64(TAN_VERSION_MAJOR) << 48ull) |   \
                           (unsigned __int64(TAN_VERSION_MINOR) << 32ull) |   \
                           (unsigned __int64(TAN_VERSION_RELEASE) << 16ull) | \
                            unsigned __int64(TAN_VERSION_BUILD))

//define export declaration
#ifdef _WIN32
        #if defined(TAN_SDK_EXPORTS)
            #define TAN_SDK_LINK __declspec(dllexport)
        #else
            #define TAN_SDK_LINK __declspec(dllimport)
        #endif
#else // #ifdef _WIN32
    #define TAN_SDK_LINK
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

        TAN_CONVOLUTION_METHOD_FFT_UNIFORM_PARTITIONED,     // Uniform Partitioned FFT algorithm. Processes bufSize samples at a time.
        TAN_CONVOLUTION_METHOD_FHT_UNIFORM_PARTITIONED,     // Uniform Partitioned FHT algorithm. Processes bufSize samples at a time.
        TAN_CONVOLUTION_METHOD_FHT_UINFORM_HEAD_TAIL,       // Uniformed, convolution performed in 2 stages head and tail

        // Note: currently not supported:
        TAN_CONVOLUTION_METHOD_TIME_DOMAIN,                 // pure time domain convolution. Processes from 1 to length samples at a time.
        TAN_CONVOLUTION_METHOD_FFT_UINFORM_HEAD_TAIL,       // Uniformed, convolution performed in 2 stages head and tail
        TAN_CONVOLUTION_METHOD_FFT_NONUNIFORM_PARTITIONED,  // Non-Uniform Partitioned FFT algorithm. Processes bufSize samples at a time.
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
        // Process OpenCL cl_mem buffers:
        virtual AMF_RESULT  AMF_STD_CALL    Process(cl_mem pBufferInput[],
                                                    cl_mem pBufferOutput[],
                                                    amf_size numOfSamplesToProcess,
                                                    const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_CONVOLUTION_CHANNEL_FLAG, can be NULL.
                                                    amf_size *pNumOfSamplesProcessed // Can be NULL.
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
                                                    float conversionGain) = 0;

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
                                                    int channels) = 0;

        virtual AMF_RESULT  AMF_STD_CALL    Convert(cl_mem inputBuffer,
                                                    amf_size inputStep,
                                                    amf_size inputOffset,
                                                    TAN_SAMPLE_TYPE inputType, 
                                                    cl_mem outputBuffer,
                                                    amf_size outputStep,
                                                    amf_size outputOffset,
                                                    TAN_SAMPLE_TYPE outputType,                                                
                                                    amf_size numOfSamplesToProcess,
                                                    float conversionGain) = 0;
        
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
                                                    int count) = 0;

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

        virtual AMF_RESULT ComplexMultiplication(const float* const inputBuffers1[],
                                                 const float* const inputBuffers2[],
                                                 float *outputBuffers[],
                                                 amf_uint32 channels,
                                                 amf_size numOfSamplesToProcess) = 0;
        virtual AMF_RESULT ComplexMultiplication(const cl_mem inputBuffers1[],
                                                 const amf_size buffers1OffsetInSamples[],
                                                 const cl_mem inputBuffers2[],
                                                 const amf_size buffers2OffsetInSamples[],
												 cl_mem outputBuffers[],
                                                 const amf_size outputBuffersOffsetInSamples[],
                                                 amf_uint32 channels,
                                                 amf_size numOfSamplesToProcess) = 0;
 
        virtual AMF_RESULT ComplexDivision(const float* const inputBuffers1[],
                                           const float* const inputBuffers2[],
                                           float *outputBuffers[],
                                           amf_uint32 channels,
                                           amf_size numOfSamplesToProcess) = 0;
        virtual AMF_RESULT ComplexDivision(const cl_mem inputBuffers1[],
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
    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANFFT> TANFFTPtr;

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

        virtual cl_context   AMF_STD_CALL   GetOpenCLContext() = 0;
        virtual	cl_command_queue	AMF_STD_CALL	GetOpenCLGeneralQueue() = 0;
        virtual	cl_command_queue	AMF_STD_CALL	GetOpenCLConvQueue() = 0;
    };

    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<TANContext> TANContextPtr;
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
    //Create an TANFFT object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateFFT(
                                                        amf::TANContext* pContext,
                                                        amf::TANFFT** ppFFT);
    // Create a TANMath object:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANCreateMath(
                                                        amf::TANContext* pContext,
                                                        amf::TANMath** ppMath);
    // Set folder to cache compiled OpenCL kernels:
    TAN_SDK_LINK AMF_RESULT         AMF_CDECL_CALL TANSetCacheFolder(const wchar_t* path);
    TAN_SDK_LINK const wchar_t*     AMF_CDECL_CALL TANGetCacheFolder();
}

