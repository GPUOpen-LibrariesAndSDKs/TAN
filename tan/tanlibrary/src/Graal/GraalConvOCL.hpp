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


#ifndef GRAALCONVOCL_H_
#define GRAALCONVOCL_H_

//Header Files
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "GraalCLUtil/GraalCLUtil.hpp"

//#include "OclKernels/GraalUtil.cl.h"
#include "OclKernels/CLKernel_GraalUtil.h"
#  include "tanlibrary/include/TrueAudioNext.h" //TAN

#  include "public/include/core/Compute.h"      //AMF
#  include "public/include/core/context.h"      //AMF
#  include "public/common/AMFFactory.h"         //AMF

/**
 */
namespace graal
{

#ifndef TAN_SDK_EXPORTS

class CGraalConvOCL;

CGraalConvOCL & getGraalOCL(void);

class CGraalConvOCL
{
//    streamsdk::SDKDeviceInfo deviceInfo;            /**< Structure to store device information*/
//    streamsdk::KernelWorkGroupInfo kernelInfo;      /**< Structure to store kernel related info */


    public:
    /**
     * Constructor
     * Initialize member variables
     */
     CGraalConvOCL(void);

    /**
     * Destructor
     * @param name name of sample (string)
     */

     ~CGraalConvOCL(void);
 

    /**
     * OpenCL related initialisations. 
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
    int setupCL(
        cl_context context = 0,
        cl_device_id device = 0,
        cl_command_queue queue = 0
        );


    /**
     * Cleanup memory allocations
     * @return GRAAL_SUCCESS on success and GRAAL_FAILURE on failure
     */
    int cleanup();

    /**
     * Cleanup memory allocations
     *  @param prop array of CL queue properties
     *  @param deviceId id device of the queue
     */

    cl_command_queue getClQueue(cl_command_queue_properties *prop, int deviceId) const;
    inline cl_context getClContext(void) const
    {
        return(context_);
    }

    cl_kernel getKernel(std::string kernel_id, 
                        std::string kernel_src,
                        size_t kernel_src_size,
                        std::string kernel_name, 
                        std::string comp_options);

protected:
    bool own_context_;
    int init_counter_;
    cl_device_type dType_;
    cl_platform_id platform_;
    cl_context context_;
#ifndef _DEBUG_PRINTF
    cl_command_queue queue_;
#endif
    std:: vector<cl_device_id> devices_;
    std:: vector<GraalDeviceInfo*> device_infors_;
    std::map<std::string,buildProgramData*> build_prog_map_;

};

#endif // !TAN_SDK_EXPORTS

/*---------------------------------------------------------
CABuf
----------------------------------------------------------*/


template<typename T>
class CABuf
{
public:
    CABuf(cl_context _context)
    {
        context_ = _context;
        mappingQ_ = 0;
        sys_ptr_ = 0;
        map_ptr_ = 0;
        buf_ = 0;
        len_ = 0;
        flags_ = 0;
        sys_own_ = false;
        cl_own_  = false;
        offset_ = 0;
        sys_ptr_ = nullptr;
    }

    CABuf(void)
    {
        context_ = 0;
        mappingQ_ = 0;
        sys_ptr_ = 0;
        map_ptr_ = 0;
        buf_ = 0;
        len_ = 0;
        flags_ = 0;
        sys_own_ = false;
        cl_own_  = false;
        offset_ = 0;
        sys_ptr_ = nullptr;
    }

    virtual ~CABuf(void)
    {
        release();
    }


// TO DO : correct copy costractor, operator =, clone()
    CABuf(const CABuf & _src)
    {
        context_ = _src.context_;
        mappingQ_ = 0;
        sys_ptr_ = 0;
        map_ptr_ = 0;
        buf_ = 0;
        len_ = 0;
        flags_ = 0;
        sys_own_ = false;
        cl_own_ = false;
        offset_ = 0;
        sys_ptr_ = nullptr;
    }


/*
    CABuf clone(void)
    {
        CABuf new_buf(context_);
        return(new_buf);
    }
*/

    int create(const T *_buf, size_t _sz, uint _flags)
    {
        int ret = GRAAL_SUCCESS;
        if (_sz == 0 )
        {
            ret = GRAAL_FAILURE;
 
           return ret;
        }

        buf_ = clCreateBuffer(context_, _flags, _sz * sizeof(T), _buf, &ret);
        if(ret != CL_SUCCESS)
        {
#ifdef _DEBUG_PRINTF
            printf("error creating buffer: %d\n", ret);
#endif
            AMF_ASSERT(ret == CL_SUCCESS, L"error creating buffer: %d", ret);
            return ret == CL_INVALID_BUFFER_SIZE ? GRAAL_NOT_ENOUGH_GPU_MEM : GRAAL_FAILURE;
        }

        len_ = _sz;
        cl_own_ = true;
        flags_ = _flags;
        sys_ptr_ = _buf;

        return(ret);
    }

    int create(size_t _sz, uint _flags)
    {
        int ret = GRAAL_SUCCESS;
        if (_sz == 0 )
        {
            ret = GRAAL_FAILURE;
 
           return ret;
        }
        buf_ = clCreateBuffer(context_, _flags, _sz*sizeof(T), NULL, &ret);
        if(ret != CL_SUCCESS)
        {
#ifdef _DEBUG_PRINTF
            printf("error creating buffer: %d\n", ret);
#endif
            AMF_ASSERT(ret == CL_SUCCESS, L"error creating OpenCL buffer: %d", ret);
            return ret == CL_INVALID_BUFFER_SIZE ? GRAAL_NOT_ENOUGH_GPU_MEM : GRAAL_FAILURE;
        }

        len_ = _sz;

        cl_own_ = true;
        flags_ = _flags;
    
        return(ret);
    }
// TO DO :: CORECT 
    int attach(const T *_buf, size_t _sz)
    {
        int ret = GRAAL_SUCCESS;
        uint flags = flags_;
        bool old_sys_own = sys_own_;
        T * old_ptr = sys_ptr_;

        if ( _sz > len_ ) {
            release();
            create(_sz, flags);
        }

        if ( sys_own_ && sys_ptr_  && old_ptr != _buf)
        {
            delete [] sys_ptr_;
            sys_ptr_ = 0;
        }

        sys_ptr_ = (T*)_buf;
        len_ = _sz;
        sys_own_ = (old_ptr != _buf) ? false : old_sys_own;

        return(ret);
    }

// TO DO : CORRECT 


    int attach(cl_mem _buf, size_t _sz)
    {
        int ret = GRAAL_SUCCESS;

        if ( _buf != buf_ || _sz > len_ )
        {
            release();
            buf_ = _buf;
            len_ = _sz;
            cl_own_ = false;

        }

        
        return(ret);
    }

    T*  map(cl_command_queue _mappingQ, uint _flags)
    {
        T* ret = 0;
        int status = 0;

        if ( buf_ && !map_ptr_ ) {
            mappingQ_ = _mappingQ;
    
                ret = map_ptr_ = (T *)clEnqueueMapBuffer(mappingQ_,
                    buf_,
                    CL_TRUE,
                    _flags, //CL_MAP_WRITE_INVALIDATE_REGION,
                    0,
                    len_*sizeof(T),
                    0,
                    NULL,
                    NULL,
                    &status);
        }

        return(ret);
    }

    T*  mapA(cl_command_queue _mappingQ, uint _flags, cl_event *_wait_event = NULL, cl_event *_set_event = NULL )
    {
        T* ret = 0;
        int status = 0;
        if ( buf_ && !map_ptr_ ) {

            int n_wait_events = 0;
            cl_event * p_set_event = _set_event;
            cl_event * p_wait_event = _wait_event;
            if (_wait_event != NULL)
            {
                n_wait_events = 1;
            }


            mappingQ_ = _mappingQ;
    
            ret = map_ptr_ = (T *)clEnqueueMapBuffer (mappingQ_,
                                                buf_,
                                                CL_FALSE,
                                                _flags, //CL_MAP_WRITE_INVALIDATE_REGION,
                                                0,
                                                len_*sizeof(T),
                                                n_wait_events,
                                                p_wait_event,
                                                p_set_event,
                                                &status);
            if (_wait_event != NULL)
            {
                clReleaseEvent(*_wait_event);
                *_wait_event = NULL;
            }


        }
        return(ret);
    }

    int unmap(cl_event *_wait_event = NULL, cl_event *_set_event = NULL)
    {
        int ret = GRAAL_SUCCESS;
        if ( buf_ && map_ptr_ && mappingQ_) {

            int n_wait_events = 0;
            cl_event * p_set_event = _set_event;
            cl_event * p_wait_event = _wait_event;
            if (_wait_event != NULL)
            {
                n_wait_events = 1;
            }

            ret = clEnqueueUnmapMemObject(mappingQ_,
                buf_,
                map_ptr_,
                n_wait_events,
                p_wait_event,
                p_set_event
                );

            if (_wait_event != NULL)
            {
                clReleaseEvent(*_wait_event);
                *_wait_event = NULL;
            }

            map_ptr_ = 0;
            mappingQ_ = 0;
        }
        return(ret);
    }

    int copyToDevice(cl_command_queue _commandQueue, uint _flags, const T* _data = NULL, size_t _len = -1, size_t _offset = 0)
    {
        int err = GRAAL_SUCCESS;

        if (_len != -1 && (_len > len_ || !_data)) {
#ifdef _DEBUG_PRINTF
            printf("wrong data\n");
#endif
            AMF_ASSERT(false, L"copyToDevice: wrong data");
            return(-1);
        }   


        if ( !buf_ )
        {
            flags_ = _flags;
            buf_ = clCreateBuffer(context_, _flags, len_*sizeof(T), NULL, &err);
            if(err != CL_SUCCESS)
            {
#ifdef _DEBUG_PRINTF
                printf("error creating buffer: %d\n", err);
#endif
                AMF_ASSERT(false, L"error creating buffer: %d", err);
                return err;
            }
            cl_own_ = true;

        }

        size_t len = (_len != -1 )? _len : len_; 
        const T * sys_ptr = (_len != -1) ? _data : sys_ptr_;
        AMF_RETURN_IF_INVALID_POINTER(sys_ptr,
                            L"Internal error: buffer hasn't been preallocated");
        err = clEnqueueWriteBuffer(_commandQueue, buf_, CL_TRUE, _offset * sizeof(T), len * sizeof(T), sys_ptr, 0, NULL, NULL);

        if(err != CL_SUCCESS) {
#ifdef _DEBUG_PRINTF
            printf("error writing data to device: %d\n", err);
#endif
            AMF_ASSERT(false, L"error writing data to device: %d\n", err);
            return err;
        }

        return(err);
    }

    int copyToDeviceNonBlocking(cl_command_queue _commandQueue, uint _flags, const T* _data = NULL, size_t _len = -1, size_t _offset = 0)
    {
        int err = GRAAL_SUCCESS;

        if (_len!=-1 && (_len > len_ || !_data)) {
#ifdef _DEBUG_PRINTF
            printf("wrong data\n");
#endif
            AMF_ASSERT(false, L"copyToDeviceA: wrong data");
            return(-1);
        }   


        if ( !buf_ )
        {
            flags_ = _flags;
            buf_ = clCreateBuffer(context_, _flags, len_*sizeof(T), NULL, &err);
            if(err != CL_SUCCESS)
            {
#ifdef _DEBUG_PRINTF
                printf("error creating buffer: %d\n", err);
#endif
                AMF_ASSERT(false, L"error creating buffer: %d\n", err);
                return err;
            }
            cl_own_ = true;

        }

        size_t len = (_len!=-1 )? _len : len_; 
        const T * sys_ptr = (_len!=-1) ? _data : sys_ptr_;
        AMF_RETURN_IF_INVALID_POINTER(sys_ptr,
                            L"Internal error: buffer hasn't been preallocated");
        err = clEnqueueWriteBuffer(_commandQueue, buf_, CL_FALSE, _offset * sizeof(T), len * sizeof(T), sys_ptr, 0, NULL, NULL);

        if(err != CL_SUCCESS) {
#ifdef _DEBUG_PRINTF
            printf("error writing data to device: %d\n", err);
#endif
            AMF_ASSERT(false, L"error writing data to device: %d\n", err);
            return err;
        }

        return(err);
    }

    int copyToHost(cl_command_queue _commandQueue)
    {
        int err = GRAAL_SUCCESS;

        if(len_ == 0 || !buf_) {
#ifdef _DEBUG_PRINTF
            printf("wrong data\n");
#endif
            AMF_ASSERT(false, L"copyToHost: wrong data");
            return(-1);
        }

        if ( !sys_ptr_ )
        {
            sys_ptr_ = new T[len_];
            if(!sys_ptr_ )
            {
                err = GRAAL_FAILURE;
#ifdef _DEBUG_PRINTF
                printf("error creating buffer: %d\n", err);
#endif
                AMF_ASSERT(false, L"error creating buffer: %d\n", err);
                return err;
            }
            sys_own_ = true;
        }

        err = clEnqueueReadBuffer(_commandQueue, buf_, CL_TRUE,0, len_ * sizeof(T), sys_ptr_, 0, NULL, NULL);
        if(err != CL_SUCCESS) {
#ifdef _DEBUG_PRINTF
            printf("error writing data to device: %d\n", err);
#endif
            AMF_ASSERT(false, L"error writing data to device: %d\n", err);
            return err;
        }

        return(err);
    }

    int copy(CABuf<T> & _src, cl_command_queue _commandQueue, size_t _src_offset = 0, size_t _dst_offset = 0)
    {
        int status;
        status = clEnqueueCopyBuffer (	_commandQueue,
                                        _src.getCLMem(),
                                        getCLMem(),
                                        _src_offset,
                                        _dst_offset,
                                        len_ * sizeof(T),
                                        0,
                                        NULL,
                                        NULL);
        return (status);
    }


    int copyCLmem(cl_mem _src, cl_command_queue _commandQueue, size_t len)
    {
        int status;
        status = clEnqueueCopyBuffer(_commandQueue,
            _src,
            getCLMem(),
            0,
            0,
            len,
            0,
            NULL,
            NULL);
        return (status);
    }

    int setValue(cl_command_queue _commandQueue, T _val)
    {
        int err = GRAAL_SUCCESS;

        T * map_ptr = map(_commandQueue, CL_MAP_WRITE_INVALIDATE_REGION);
        for( int i = 0; i < len_; i++)
        {
            map_ptr[i] = _val;
            if ( sys_ptr_ )
            {
                sys_ptr_[i] = _val;
            }
        }
        unmap();
        return(err);
    }

    int setValue2(cl_command_queue _commandQueue, T _val, size_t _len = 0, size_t _offset = 0)
    {
        int err = GRAAL_SUCCESS;
        int len = (_len != 0) ? _len: len_;
        if (NULL != _commandQueue)
        {
           err = clEnqueueFillBuffer(_commandQueue, buf_, (const void *)&_val, sizeof(_val), _offset* sizeof(T), len * sizeof(T) , 0, nullptr, nullptr);
        }
        for (int i = _offset; sys_ptr_ && i < len; i++)
        {
            sys_ptr_[i] = _val;
        }
        return(err);
    }


    int release(void)
    {
        int ret = GRAAL_SUCCESS;
        if ( sys_own_ && sys_ptr_) 
        {
            delete [] sys_ptr_;
            sys_ptr_ = 0;

        }
        sys_own_  = false;

        if ( cl_own_ ) 
        {
            unmap();
            if ( buf_ )
            {
                if (clReleaseMemObject(buf_) != CL_SUCCESS) {
                    ret = GRAAL_OPENCL_FAILURE;
#ifdef TAN_SDK_EXPORTS
                    AMF_ASSERT(false, L"clReleaseMemObject() failed");
#endif
                }
            }
            buf_ = 0;

        }

        cl_own_ = false;
        len_ = 0;

        return(ret);
    }

    int set_value(T val)
    {
        int ret = GRAAL_SUCCESS;
        return(ret);
    }

#ifndef TAN_SDK_EXPORTS
    inline void setContext(cl_context _context)
    {
        context_ = _context;
    }
#endif

    inline cl_context getContext(void)
    {
        return(context_);
    }

    inline const cl_mem & getCLMem(void) 
    {
        return(buf_);
    }

    inline virtual T * & getSysMem(void)
    {
        if (!sys_ptr_)
        {
            sys_ptr_ = new T[len_];

            if (!sys_ptr_)
            {
#ifdef _DEBUG_PRINTF
#ifdef TAN_SDK_EXPORTS
                AMF_ASSERT(false, L"Cannot allocate memory: %lu", len_ * sizeof(T));
#else
                printf("error creating bufffer: %d\n", ret);
#endif
#endif
                return sys_ptr_;
            }

            cl_own_ = true;
        }

        return(sys_ptr_);
    }

    inline T * & getMappedMem(void)
    {
        return(map_ptr_);
    }


    inline void setSysOwnership(bool own )
    {
        sys_own_ = own;
    }


    inline bool getSysOwnership(void)
    {
        return(sys_own_);
    }

    // DENGEROUS
    inline void setLen(size_t _len)
    {
        len_ = _len;
    }

    inline size_t getLen(void)
    {
        return(len_);
    }

protected:
    cl_context context_;
    cl_command_queue mappingQ_;

    cl_kernel setUintValue2Kernel_;
    cl_kernel setFloatValue2Kernel_;

    T * sys_ptr_;
    T * map_ptr_;
    cl_mem buf_;
    size_t len_;
    uint flags_;
    bool sys_own_;
    bool cl_own_;
    size_t offset_;


};


template<typename T>
class CASubBuf : public CABuf<T>
{
public:
    CASubBuf(CABuf<T> & _base) : CABuf(_base), m_base(_base)
    {
        base_buf_ = _base.getCLMem();
        sys_ptr_ = nullptr;

        assert(base_buf_);

    }
    int create(size_t _offset, size_t _sz, uint _flags)
    {
        int ret = GRAAL_SUCCESS;
        if (_sz == 0 )
        {
            ret = GRAAL_FAILURE;
 
           return ret;
        }

        m_offset = _offset;

        cl_buffer_region sub_buf;

        sub_buf.origin = _offset* sizeof(T);
        sub_buf.size = _sz * sizeof(T);

        buf_ = clCreateSubBuffer (	base_buf_,
                                    _flags,
                                    CL_BUFFER_CREATE_TYPE_REGION,
                                    &sub_buf,
                                    &ret);
        
        
        if(ret != CL_SUCCESS)
        {
#ifdef _DEBUG_PRINTF
            printf("error creating buffer: %d\n", ret);
#endif
            AMF_ASSERT(false, L"error creating buffer: %d\n", ret);
            return ret;
        }

        len_ = _sz;
        cl_own_ = true;
        flags_ = _flags;
    
        return(ret);
    }

    int create(size_t _sz, uint _flags)
    {
        create(0, _sz, _flags);
    }

    T * & getSysMem(void) override
    {
        if (!sys_ptr_)
        {
            sys_ptr_ = m_base.getSysMem();
            AMF_RETURN_IF_FALSE(sys_ptr_, sys_ptr_, L"Internal error: subbuffer's parent failed to allocate system memory");
            sys_ptr_ += m_offset;
        }

        return sys_ptr_;
    }
protected:
    cl_mem base_buf_;
    size_t m_offset;
    CABuf<T> &m_base;
};

};


#endif