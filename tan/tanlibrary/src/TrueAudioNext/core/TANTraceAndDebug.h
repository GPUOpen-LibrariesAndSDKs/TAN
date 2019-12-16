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
///-------------------------------------------------------------------------
///  @file   TANTraceAdapter.h
///  @brief  TAN AMFTrace interface
///-------------------------------------------------------------------------
#pragma once

#include "public/include/core/Trace.h"
#include "public/include/core/Debug.h"

namespace amf
{
    class TANTrace : public AMFTrace
    {
    public:
        static TANTrace &shared_instance() { static TANTrace s_tanTrace; return s_tanTrace; }
    public:
        void               AMF_STD_CALL TraceW(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope, amf_int32 countArgs, const wchar_t* format, ...);
        void               AMF_STD_CALL Trace(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope, const wchar_t* message, va_list* pArglist);

        amf_int32           AMF_STD_CALL SetGlobalLevel(amf_int32 level);
        amf_int32           AMF_STD_CALL GetGlobalLevel();

        bool                AMF_STD_CALL EnableWriter(const wchar_t* writerID, bool enable);
        bool                AMF_STD_CALL WriterEnabled(const wchar_t* writerID);
        AMF_RESULT          AMF_STD_CALL TraceEnableAsync(bool enable);
        AMF_RESULT          AMF_STD_CALL TraceFlush();
        AMF_RESULT          AMF_STD_CALL SetPath(const wchar_t* path);
        AMF_RESULT          AMF_STD_CALL GetPath(wchar_t* path, amf_size* pSize);
        amf_int32           AMF_STD_CALL SetWriterLevel(const wchar_t* writerID, amf_int32 level);
        amf_int32           AMF_STD_CALL GetWriterLevel(const wchar_t* writerID);
        amf_int32           AMF_STD_CALL SetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope, amf_int32 level);
        amf_int32           AMF_STD_CALL GetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope);

        amf_int32           AMF_STD_CALL GetIndentation();
        void                AMF_STD_CALL Indent(amf_int32 addIndent);

        void                AMF_STD_CALL RegisterWriter(const wchar_t* writerID, AMFTraceWriter* pWriter, bool enable);
        void                AMF_STD_CALL UnregisterWriter(const wchar_t* writerID);

        const wchar_t*      AMF_STD_CALL GetResultText(AMF_RESULT res);
        const wchar_t*      AMF_STD_CALL SurfaceGetFormatName(const AMF_SURFACE_FORMAT eSurfaceFormat);
        AMF_SURFACE_FORMAT  AMF_STD_CALL SurfaceGetFormatByName(const wchar_t* name);

        const wchar_t* const AMF_STD_CALL GetMemoryTypeName(const AMF_MEMORY_TYPE memoryType);
        AMF_MEMORY_TYPE     AMF_STD_CALL GetMemoryTypeByName(const wchar_t* name);

        const wchar_t* const AMF_STD_CALL GetSampleFormatName(const AMF_AUDIO_FORMAT eFormat);
        AMF_AUDIO_FORMAT    AMF_STD_CALL GetSampleFormatByName(const wchar_t* name);

    private:
        // Make constructor private. Only shared_instance() method will create an instance.
        TANTrace() {}
    };

    AMFTrace* GetTANTracer();

    class TANDebug : public AMFDebug
    {
    public:
        static TANDebug &shared_instance() { static TANDebug s_tanDebug; return s_tanDebug; }
    public:
        void               AMF_STD_CALL EnablePerformanceMonitor(bool enable);
        bool               AMF_STD_CALL PerformanceMonitorEnabled();
        void               AMF_STD_CALL AssertsEnable(bool enable);
        bool               AMF_STD_CALL AssertsEnabled();
    private:
        // Make constructor private. Only shared_instance() method will create an instance.
        TANDebug() {}
    };

    AMFDebug* GetTANDebugger();

} // namespace amf
