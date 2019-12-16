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

#ifndef TAN_SDK_EXPORTS

#include "GraalConv.hpp"
#include "GraalCLUtil/GraalCLUtil.hpp"
#include "GraalConvOCL.hpp"
#if !defined(__APPLE__) && !defined(__MACOSX)
#include <malloc.h>
#endif

namespace graal
{

static CGraalConvOCL spGraalConvOCL;

CGraalConvOCL & getGraalOCL(void)
{
//	spGraalConvOCL.setupCL();
    return spGraalConvOCL;
}

CGraalConvOCL:: CGraalConvOCL( void )
#ifndef _DEBUG_PRINTF
    : queue_(0)
#endif
{
    init_counter_ = 0;
    dType_ = CL_DEVICE_TYPE_GPU;
    platform_ = 0;
    context_ = 0;
    devices_.clear();
    device_infors_.clear();
    build_prog_map_.clear();

}


CGraalConvOCL:: ~CGraalConvOCL( void )
{
    cleanup();
}




int
CGraalConvOCL::setupCL(
                    cl_context _context,
                    cl_device_id _device,
                    cl_command_queue _queue
                    )
{
    cl_int status = GRAAL_SUCCESS;

    if (++init_counter_ == 1)
    {
        if (_context != 0)
        {
            context_ = _context;
        }
        else
        {
            own_context_ = true;

            /*
             * Have a look at the available platforms and pick either
             * the AMD one if available or a reasonable default.
             */
            cl_platform_id platform_ = NULL;
            status = getPlatformL(platform_, 0, false, dType_);
            CHECK_ERROR(status, GRAAL_SUCCESS, "graal::getPlatform() failed");
            // Display available devices.
            status = displayDevices(platform_, dType_);
            CHECK_ERROR(status, GRAAL_SUCCESS, "graal::displayDevices() failed");
            /*
             * If we could find our platform, use it. Otherwise use just available platform.
             */
            cl_context_properties cps[3] =
            {
                CL_CONTEXT_PLATFORM,
                (cl_context_properties)platform_,
                0
            };
            context_ = clCreateContextFromType(cps,
                dType_,
                NULL,
                NULL,
                &status);
            CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");
        }

#ifdef _DEBUG_PRINTF
    // getting device on which to run the sample
        status = getDevices(context_, devices_, 0, false);
        CHECK_ERROR(status, 0, "sampleCommon::getDevices() failed");

        for(int i = 0; i < devices_.size(); i++)
        {
            GraalDeviceInfo * deviceInfo = new GraalDeviceInfo;
            status = deviceInfo->setDeviceInfo(devices_[i]);
            CHECK_OPENCL_ERROR(status, "deviceInfo.setDeviceInfo failed");
            device_infors_.push_back(deviceInfo);
        }
#else
        devices_.push_back(_device);
        queue_ = _queue;
#endif // _DEBUG_PRINTF
    }

    return status;
}

cl_kernel CGraalConvOCL:: getKernel(
    std::string _kernel_id,
    std::string kernel_src,
    size_t kernel_src_size,
    std::string _kernel_name,
    std::string _comp_options
)
{
    cl_kernel ret = 0;
    cl_int status = 0;
    std::string key = _kernel_id + "." + _comp_options;
    std::map<std::string,buildProgramData*>::iterator b;
    b = build_prog_map_.find(key);
    if(b == build_prog_map_.end())
    {
// did not find the program
// build it
    // create a CL program using the kernel source
        buildProgramData *buildData = new buildProgramData;
        buildData->kernelId = _kernel_id;
        buildData->kernelSrc = kernel_src;
        buildData->kernelSrcSize = kernel_src_size;

        buildData->devices = &devices_[0];
        buildData->deviceId = 0;
        buildData->flagsStr = std::string(_comp_options);

        status = buildOpenCLProgram(buildData->program, context_, *buildData);
        CHECK_OPENCL_ERROR_MSG(status, "build failed.");

        build_prog_map_[key] = buildData;

    }

    b = build_prog_map_.find(key);

    cl_program prog = b->second->program;
        // get a kernel object handle for a kernel with the given name
    ret = clCreateKernel(prog, _kernel_name.c_str(), &status);
    CHECK_OPENCL_ERROR_MSG(status, "clCreateKernel failed.");

    return ret;
}


int CGraalConvOCL:: cleanup()
{
    int status = CL_SUCCESS;
    if (--init_counter_ == 0 )
    {

        devices_.clear();

        for(std::vector<GraalDeviceInfo*>::iterator i = device_infors_.begin(); i != device_infors_.end(); i++)
        {
            delete *i;
        }
        device_infors_.clear();

        for(std::map<std::string,buildProgramData*>::iterator b = build_prog_map_.begin(); b != build_prog_map_.end(); b++ )
        {
            delete b->second;
            b->second = 0;
        }

        build_prog_map_.clear();

        if (own_context_ && context_)
        {
            status = clReleaseContext(context_);
            CHECK_OPENCL_ERROR(status, "clReleaseContext failed.");
            context_ = 0;
        }

        own_context_ = false;


    }
    return status;
}

cl_command_queue CGraalConvOCL::getClQueue(cl_command_queue_properties * _prop, int _deviceId) const
{
#ifdef _DEBUG_PRINTF
    cl_int status = CL_SUCCESS;
    cl_command_queue commandQueue = 0;
    if (device_infors_.size() > _deviceId )
    {
        int majorRev, minorRev;

        if (sscanf((*device_infors_[_deviceId]).deviceVersion, "OpenCL %d.%d", &majorRev, &minorRev) == 2)
        {
            if (majorRev < 2) {
                //OPENCL_EXPECTED_ERROR_RETURN("Unsupported device! Required CL_DEVICE_OPENCL_C_VERSION 2.0 or higher");
                return nullptr;
            }
        }

        commandQueue = clCreateCommandQueueWithProperties(context_,
                                            devices_[_deviceId],
                                            _prop,
                                            &status);
        printf("Queue created %llX\r\n", commandQueue);
        //ASSERT_CL_RETURN(status, "clCreateCommandQueue failed.");
        if (status != CL_SUCCESS)
        {
            return nullptr;
        }

    }

    return commandQueue;
#else
    return queue_;
#endif
}






#if 0


// TODO: generic type
int SetValue(ProjPlan *plan, OCLBuffer * buf, unsigned int value) {
int err;
int n_arg = 0;

    if(buf->len == 0 || !buf->mem) {
        printf("wrong data\n");
        return(-1);
    }
cl_command_queue commandQueue =  GetOCLQueue(plan);
HSA_OCL_ConfigS * config = GetOclConfig(plan);


    cl_kernel setvalue_kernel;
        if (  config->kernels[__UTIL_PROGRAM_INDEX__] ) {
            setvalue_kernel = clCreateKernel(config->program[__UTIL_PROGRAM_INDEX__],  "SetValue", &err);
        }
        else {
            printf("cannot create kernel SetValue \n");
        }

    err = -1;
    if ( setvalue_kernel ) {

        size_t local_work_size[1] = {256};
        size_t global_work_size[1] = {(buf->len / sizeof(unsigned int))};
        err = clSetKernelArg(setvalue_kernel, n_arg++, sizeof(cl_mem), &buf->mem);
        err |= clSetKernelArg(setvalue_kernel, n_arg++, sizeof(cl_int), &value);
        err |= clEnqueueNDRangeKernel(commandQueue, setvalue_kernel , 1, NULL, global_work_size, local_work_size,
                   0, NULL, NULL);

        if(err != CL_SUCCESS) {
           printf("error setting value: %d\n", err);
        }
        clReleaseKernel(setvalue_kernel);
    }


    return(err);
}



// sys buffer should have exact buf->len lenght
int CopyToDevice2(cl_command_queue commandQueue, OCLBuffer * buf, void * sys, size_t sys_len)
{
int err;

    if(buf->len == 0 || !sys || !buf->mem) {
        printf("wrong data\n");
        return(-1);
    }

    err = clEnqueueWriteBuffer(commandQueue, buf->mem, CL_TRUE,0, buf->len, sys, 0, NULL, NULL);

    if(err != CL_SUCCESS) {
        printf("error writing data to device: %d\n", err);
        return err;
    }

    return(err);
}

int CopyFromDevice(cl_command_queue commandQueue, OCLBuffer * buf)
{
  int err;

    if(buf->len == 0 || (buf->sys && buf->flags) || !buf->mem) {
        printf("wrong data\n");
        return(-1);
    }

    if ( !buf->sys ) {
        buf->sys = malloc(buf->len);

        if ( !buf->sys ) {
            err = -1;
            printf("error buf allocation\n");
            return err;
        }
    }

    err = clEnqueueReadBuffer(commandQueue, buf->mem, CL_TRUE,0, buf->len, buf->sys, 0, NULL, NULL);


    if(err != CL_SUCCESS) {
        printf("error writing data to device: %d\n", err);
        return err;
    }

    return(err);
}
#endif

}

#endif // !TAN_SDK_EXPORTS