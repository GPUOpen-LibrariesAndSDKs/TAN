SETLOCAL

RMDIR /S /Q vs2019
MKDIR vs2019
CD vs2019

SET CMAKE_PREFIX_PATH=..\..\..\..\thirdparty\Qt\Qt5.9.9\5.9.9\msvc2019_64\lib\cmake\Qt5Widgets
cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 15 2019" -A x64 -DOpenCL_INCLUDE_DIR=..\..\..\..\thirdparty\OpenCL-Headers -DPortAudio_DIR=..\..\..\..\..\thirdparty\portaudio -DIPP_DIR="C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2020.0.166\windows\ipp" -DDEFINE_AMD_OPENCL_EXTENSION=1
