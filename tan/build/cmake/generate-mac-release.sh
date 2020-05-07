#/bin/sh

rm -rf ./mac-gnumake
mkdir mac-gnumake
cd mac-gnumake
#cmake .. -DCMAKE_PREFIX_PATH="/Applications/Qt5.6.3/5.6.3/clang_64" -DOpenCL_INCLUDE_DIR="../../../../thirdparty/OpenCL-Headers" -DOpenCL_LIBRARY="/System/Library/Frameworks/OpenCL.framework" -DPortAudio_DIR="../../../../../thirdparty/portaudio" -DDEFINE_AMD_OPENCL_EXTENSION=1
cmake -DCMAKE_BUILD_TYPE=Release .. -DCMAKE_PREFIX_PATH="/Applications/Qt5.6.3/5.6.3/clang_64" -DOpenCL_INCLUDE_DIR="../../../../thirdparty/OpenCL-Headers" -DPortAudio_DIR="../../../../../thirdparty/portaudio" -DDEFINE_AMD_OPENCL_EXTENSION=1
cd ..