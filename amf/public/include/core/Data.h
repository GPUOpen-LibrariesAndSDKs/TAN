// 
// Notice Regarding Standards.  AMD does not provide a license or sublicense to
// any Intellectual Property Rights relating to any standards, including but not
// limited to any audio and/or video codec technologies such as MPEG-2, MPEG-4;
// AVC/H.264; HEVC/H.265; AAC decode/FFMPEG; AAC encode/FFMPEG; VC-1; and MP3
// (collectively, the "Media Technologies"). For clarity, you will pay any
// royalties due for such third party technologies, which may include the Media
// Technologies that are owed as a result of AMD providing the Software to you.
// 
// MIT license 
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

#ifndef __AMFData_h__
#define __AMFData_h__
#pragma once

#include "PropertyStorage.h"

#if defined(__cplusplus)
namespace amf
{
#endif
    //----------------------------------------------------------------------------------------------
    typedef enum AMF_DATA_TYPE
    {
        AMF_DATA_BUFFER             = 0,
        AMF_DATA_SURFACE            = 1,
        AMF_DATA_AUDIO_BUFFER       = 2,
        AMF_DATA_USER               = 1000,
        // all extensions will be AMF_DATA_USER+i
    } AMF_DATA_TYPE;
    //----------------------------------------------------------------------------------------------
    typedef enum AMF_MEMORY_TYPE
    {
        AMF_MEMORY_UNKNOWN          = 0,
        AMF_MEMORY_HOST             = 1,
        AMF_MEMORY_DX9              = 2,
        AMF_MEMORY_DX11             = 3,
        AMF_MEMORY_OPENCL           = 4,
        AMF_MEMORY_OPENGL           = 5,
        AMF_MEMORY_XV               = 6,
        AMF_MEMORY_GRALLOC          = 7,
        AMF_MEMORY_COMPUTE_FOR_DX9  = 8,
        AMF_MEMORY_COMPUTE_FOR_DX11 = 9,
    } AMF_MEMORY_TYPE;

    //----------------------------------------------------------------------------------------------
    typedef enum AMF_DX_VERSION
    {
        AMF_DX9                     = 90,
        AMF_DX9_EX                  = 91,
        AMF_DX11_0                  = 110,
        AMF_DX11_1                  = 111 
    } AMF_DX_VERSION;
    //----------------------------------------------------------------------------------------------
    // AMFData interface
    //----------------------------------------------------------------------------------------------
#if defined(__cplusplus)
    class AMF_NO_VTABLE AMFData : public AMFPropertyStorage
    {
    public:
        AMF_DECLARE_IID(0xa1159bf6, 0x9104, 0x4107, 0x8e, 0xaa, 0xc5, 0x3d, 0x5d, 0xba, 0xc5, 0x11)

        virtual AMF_MEMORY_TYPE     AMF_STD_CALL GetMemoryType() = 0;

        virtual AMF_RESULT          AMF_STD_CALL Duplicate(AMF_MEMORY_TYPE type, AMFData** ppData) = 0;
        virtual AMF_RESULT          AMF_STD_CALL Convert(AMF_MEMORY_TYPE type) = 0; // optimal interop if possilble. Copy through host memory if needed
        virtual AMF_RESULT          AMF_STD_CALL Interop(AMF_MEMORY_TYPE type) = 0; // only optimal interop if possilble. No copy through host memory for GPU objects

        virtual AMF_DATA_TYPE       AMF_STD_CALL GetDataType() = 0;

        virtual amf_bool            AMF_STD_CALL IsReusable() = 0;

        virtual void                AMF_STD_CALL SetPts(amf_pts pts) = 0;
        virtual amf_pts             AMF_STD_CALL GetPts() = 0;
        virtual void                AMF_STD_CALL SetDuration(amf_pts duration) = 0;
        virtual amf_pts             AMF_STD_CALL GetDuration() = 0;
    };
    //----------------------------------------------------------------------------------------------
    // smart pointer
    //----------------------------------------------------------------------------------------------
    typedef AMFInterfacePtr_T<AMFData> AMFDataPtr;
    //----------------------------------------------------------------------------------------------

#else // #if defined(__cplusplus)
    typedef struct AMFData AMFData;
    AMF_DECLARE_IID(AMFData, 0xa1159bf6, 0x9104, 0x4107, 0x8e, 0xaa, 0xc5, 0x3d, 0x5d, 0xba, 0xc5, 0x11)

    typedef struct AMFDataVtbl
    {
        // AMFInterface interface
        amf_long            (AMF_STD_CALL *Acquire)(AMFData* pThis);
        amf_long            (AMF_STD_CALL *Release)(AMFData* pThis);
        enum AMF_RESULT     (AMF_STD_CALL *QueryInterface)(AMFData* pThis, const struct AMFGuid *interfaceID, void** ppInterface);

        // AMFPropertyStorage interface
        AMF_RESULT          (AMF_STD_CALL *SetProperty)(AMFData* pThis, const wchar_t* name, AMFVariantStruct value);
        AMF_RESULT          (AMF_STD_CALL *GetProperty)(AMFData* pThis, const wchar_t* name, AMFVariantStruct* pValue);
        amf_bool            (AMF_STD_CALL *HasProperty)(AMFData* pThis, const wchar_t* name);
        amf_size            (AMF_STD_CALL *GetPropertyCount)(AMFData* pThis);
        AMF_RESULT          (AMF_STD_CALL *GetPropertyAt)(AMFData* pThis, amf_size index, wchar_t* name, amf_size nameSize, AMFVariantStruct* pValue);
        AMF_RESULT          (AMF_STD_CALL *Clear)(AMFData* pThis);
        AMF_RESULT          (AMF_STD_CALL *AddTo)(AMFData* pThis, AMFPropertyStorage* pDest, amf_bool overwrite, amf_bool deep);
        AMF_RESULT          (AMF_STD_CALL *CopyTo)(AMFData* pThis, AMFPropertyStorage* pDest, amf_bool deep);
        void                (AMF_STD_CALL *AddObserver)(AMFData* pThis, AMFPropertyStorageObserver* pObserver);
        void                (AMF_STD_CALL *RemoveObserver)(AMFData* pThis, AMFPropertyStorageObserver* pObserver);

        // AMFData interface

        AMF_MEMORY_TYPE     (AMF_STD_CALL *GetMemoryType)(AMFData* pThis);

        AMF_RESULT          (AMF_STD_CALL *Duplicate)(AMFData* pThis, AMF_MEMORY_TYPE type, AMFData** ppData);
        AMF_RESULT          (AMF_STD_CALL *Convert)(AMFData* pThis, AMF_MEMORY_TYPE type); // optimal interop if possilble. Copy through host memory if needed
        AMF_RESULT          (AMF_STD_CALL *Interop)(AMFData* pThis, AMF_MEMORY_TYPE type); // only optimal interop if possilble. No copy through host memory for GPU objects

        AMF_DATA_TYPE       (AMF_STD_CALL *GetDataType)(AMFData* pThis);

        amf_bool            (AMF_STD_CALL *IsReusable)(AMFData* pThis);

        void                (AMF_STD_CALL *SetPts)(AMFData* pThis, amf_pts pts);
        amf_pts             (AMF_STD_CALL *GetPts)(AMFData* pThis);
        void                (AMF_STD_CALL *SetDuration)(AMFData* pThis, amf_pts duration);
        amf_pts             (AMF_STD_CALL *GetDuration)(AMFData* pThis);

    } AMFDataVtbl;

    struct AMFData
    {
        const AMFDataVtbl *pVtbl;
    };


#endif // #if defined(__cplusplus)

#if defined(__cplusplus)
} // namespace
#endif

#endif //#ifndef __AMFData_h__
