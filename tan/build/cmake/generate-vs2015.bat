SETLOCAL

RMDIR /S /Q vs2015
MKDIR vs2015
CD vs2015

SET CMAKE_PREFIX_PATH=..\..\..\tan\thirdparty\QT5.6\5.6.3\msvc2015_64  
cmake .. -G "Visual Studio 14 2015" -A x64 -DOpenCL_INCLUDE_DIR=..\..\..\tan\thirdparty\OpenCL-Headers -DPortAudio_DIR=..\..\..\tan\thirdparty\portaudio -DDEFINE_AMD_OPENCL_EXTENSION=1