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


#include "OCLHelper.h"

bool GetOclKernel(
    cl_kernel &ResultKernel,
    amf::AMFComputePtr &pDevice,
    const cl_command_queue c_queue,
    std::string kernel_id,
    std::string kernel_src,
    size_t kernel_src_size,
    std::string kernel_name,
    std::string comp_options)
{
    ResultKernel = NULL;

    ///First: Try to load kernel via AMF interface
    if (nullptr != pDevice)
    {
        amf::AMF_KERNEL_ID amfKernelId = -1;
        std::wstring wKernelId(kernel_id.begin(), kernel_id.end());

        amf::AMFPrograms* pPrograms = nullptr;
        g_AMFFactory.GetFactory()->GetPrograms(&pPrograms);
        if (nullptr != pPrograms)
        {
            if (AMF_OK == pPrograms->RegisterKernelSource(&amfKernelId,
                                                            wKernelId.c_str(),
                                                            kernel_name.c_str(),
                                                            kernel_src_size,
                                                            reinterpret_cast<const amf_uint8*>(kernel_src.c_str()),
                                                            comp_options.c_str()) )
            {
                amf::AMFComputeKernel* pAMFComputeKernel = nullptr;
                pDevice->GetKernel(amfKernelId, &pAMFComputeKernel);
                if (nullptr != pAMFComputeKernel)
                {
                    ResultKernel = (cl_kernel)pAMFComputeKernel->GetNative();
                    return true;
                }
            }
        }
    }

    //seconf: try to load kernel via OpenCL interface
    if ( NULL != c_queue )
    {
        cl_kernel ret = 0;
        cl_int status = CL_SUCCESS;
        std::string key = kernel_id + "." + comp_options;

        cl_context context = NULL;
        size_t param_value_size_ret = 0;
        clGetCommandQueueInfo(c_queue, CL_QUEUE_CONTEXT, sizeof(context), &context, &param_value_size_ret);

        const char *source = (kernel_src.c_str());

        cl_program program = clCreateProgramWithSource(context,
                                            1,
                                            &source,
                                            &kernel_src_size,
                                            &status);
        if ( CL_SUCCESS == status )
        {
            cl_device_id device_id = 0;
            status = clGetCommandQueueInfo(c_queue, CL_QUEUE_DEVICE, sizeof(device_id), &device_id, &param_value_size_ret);
            if ( CL_SUCCESS == status )
            {
                status = clBuildProgram(program, 1, &device_id, comp_options.c_str(), NULL, NULL);
                if ( CL_SUCCESS == status )
                {
                    ResultKernel = clCreateKernel(program, kernel_name.c_str(), &status);
                    if ((CL_SUCCESS == status) && (NULL < ResultKernel) )
                    {
                        return true;
                    }
                }
                else
                {
                    if (CL_BUILD_PROGRAM_FAILURE == status)
                    {
                        cl_int logStatus;
                        char *buildLog = NULL;
                        size_t buildLogSize = 0;
                        logStatus = clGetProgramBuildInfo(program,
                            device_id,
                            CL_PROGRAM_BUILD_LOG,
                            buildLogSize,
                            buildLog,
                            &buildLogSize);

                        buildLog = (char*)malloc(buildLogSize);
                        memset(buildLog, 0, buildLogSize);
                        logStatus = clGetProgramBuildInfo(program,
                            device_id,
                            CL_PROGRAM_BUILD_LOG,
                            buildLogSize,
                            buildLog,
                            &buildLogSize);
                        fprintf(stderr, buildLog);
                        free(buildLog);
                    }
                }
            }
        }
    }

    return false;
}

