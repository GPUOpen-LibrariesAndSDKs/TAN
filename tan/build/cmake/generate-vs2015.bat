SETLOCAL

RMDIR /S /Q vs2015
MKDIR vs2015
CD vs2015

SET CMAKE_PREFIX_PATH=..\..\..\..\thirdparty\Qt\Qt5.9.9\5.9.9\msvc2017_64\lib\cmake\Qt5Widgets
cmake .. -G "Visual Studio 14 2015" -A x64 -DOpenCL_INCLUDE_DIR=..\..\..\..\thirdparty\OpenCL-Headers -DPortAudio_DIR=..\..\..\..\..\thirdparty\portaudio -DIPP_DIR="C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2020.0.166\windows\ipp" -DDEFINE_AMD_OPENCL_EXTENSION=1