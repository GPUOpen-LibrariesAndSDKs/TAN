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

#include "TANTraceAndDebug.h"

#include <cwchar>

#pragma warning(disable: 4251)
#pragma warning(disable: 4996)

using namespace amf;

amf::AMFTrace* amf::GetTANTracer()
{
    return &TANTrace::shared_instance();
}

void                TANTrace::TraceW(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope, amf_int32 countArgs, const wchar_t* format, ...)
{
    //todo: think about, check security reasons, may be use std::cerr?
    //fwprintf_s(stderr, format);
    std::fwprintf(stderr, format);
}
void                TANTrace::Trace(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope, const wchar_t* message, va_list* pArglist)
{
    //todo: look above
    //fwprintf_s(stderr, message);
    std::fwprintf(stderr, message);
}

amf_int32           TANTrace::SetGlobalLevel(amf_int32 level)                           { return 0; }
amf_int32           TANTrace::GetGlobalLevel()                                          { return 0; }

bool                TANTrace::EnableWriter(const wchar_t* writerID, bool enable)        { return false; }
bool                TANTrace::WriterEnabled(const wchar_t* writerID)                    { return false; }
AMF_RESULT          TANTrace::TraceEnableAsync(bool enable)                             { return AMF_NOT_IMPLEMENTED; }
AMF_RESULT          TANTrace::TraceFlush()                                              { return AMF_NOT_IMPLEMENTED; }
AMF_RESULT          TANTrace::SetPath(const wchar_t* path)                              { return AMF_NOT_IMPLEMENTED; }
AMF_RESULT          TANTrace::GetPath(wchar_t* path, amf_size* pSize)                   { return AMF_NOT_IMPLEMENTED; }
amf_int32           TANTrace::SetWriterLevel(const wchar_t* writerID, amf_int32 level)  { return 0; }
amf_int32           TANTrace::GetWriterLevel(const wchar_t* writerID)                   { return 0; }
amf_int32           TANTrace::SetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope, amf_int32 level){ return 0; }
amf_int32           TANTrace::GetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope) { return 0; }

amf_int32           TANTrace::GetIndentation()                                          { return 4; }
void                TANTrace::Indent(amf_int32 addIndent)                               { return; }

void                TANTrace::RegisterWriter(const wchar_t* writerID, AMFTraceWriter* pWriter, bool enable){ return; }
void                TANTrace::UnregisterWriter(const wchar_t* writerID)                 { return; }

const wchar_t*      TANTrace::GetResultText(AMF_RESULT res)                             { return NULL; }
const wchar_t*      TANTrace::SurfaceGetFormatName(const AMF_SURFACE_FORMAT eSurfaceFormat){ return NULL; }
AMF_SURFACE_FORMAT  TANTrace::SurfaceGetFormatByName(const wchar_t* name)               { return AMF_SURFACE_UNKNOWN; }

const wchar_t* const TANTrace::GetMemoryTypeName(const AMF_MEMORY_TYPE memoryType)      { return NULL; }
AMF_MEMORY_TYPE     TANTrace::GetMemoryTypeByName(const wchar_t* name)                  { return AMF_MEMORY_UNKNOWN; }

const wchar_t* const TANTrace::GetSampleFormatName(const AMF_AUDIO_FORMAT eFormat)      { return NULL; }
AMF_AUDIO_FORMAT    TANTrace::GetSampleFormatByName(const wchar_t* name)                { return AMFAF_UNKNOWN; }

amf::AMFDebug* amf::GetTANDebugger()
{
    return &TANDebug::shared_instance();
}

void               TANDebug::EnablePerformanceMonitor(bool enable)  { return; }
bool               TANDebug::PerformanceMonitorEnabled()            { return false; }
void               TANDebug::AssertsEnable(bool enable)             { return; }
bool               TANDebug::AssertsEnabled()                       { return false; }
