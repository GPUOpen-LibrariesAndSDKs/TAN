SETLOCAL

RMDIR /S /Q vs2017
MKDIR vs2017
CD vs2017

SET CMAKE_PREFIX_PATH=c:\Qt\Qt5.6.3\5.6.3\msvc2015_64
cmake .. -G "Visual Studio 15 2017" -A x64 -DOpenCL_INCLUDE_DIR="C:\Program Files (x86)\OCL_SDK_Light\include" -DOpenCL_LIBRARY="C:\Program Files (x86)\OCL_SDK_Light\lib\x86_64\OpenCL.lib" -DPortAudio_DIR="../../../../../thirdparty/portaudio"