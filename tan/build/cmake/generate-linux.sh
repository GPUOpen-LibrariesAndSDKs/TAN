#/bin/sh

rm -rf ./linux
mkdir linux
cd linux
#cmake .. -DCMAKE_PREFIX_PATH="/opt/Qt/5.6.3/gcc_64" -DOpenCL_INCLUDE_DIR="/opt/rocm/opencl/include" -DOpenCL_LIBRARY="/opt/rocm/opencl/lib/x86_64/libOpenCL.so" -DPortAudio_INCLUDE_DIRS="../../../../../thirdparty/portaudio/include" -DPortAudio_LIBRARY="~/TAN/thirdparty/portaudio/lib/.libs/libportaudio.so"
#cmake .. -DCMAKE_PREFIX_PATH="/opt/Qt5.6.3/5.6.3/gcc_64" -DOpenCL_INCLUDE_DIR="../../../../thirdparty/OpenCL-Headers" -DOpenCL_LIBRARY="~/amd/amdgpu-pro/lib/x86_64-linux-gnu/libOpenCL.so.1" -DPortAudio_DIR="../../../../../thirdparty/portaudio" -DDEFINE_AMD_OPENCL_EXTENSION=1
#cmake .. -DCMAKE_PREFIX_PATH="../../../tan.thirdparty/QT5.6/5.6.3/gcc_64" -DOpenCL_INCLUDE_DIR="../../../../thirdparty/OpenCL-Headers" -DOpenCL_LIBRARY="/opt/rocm/opencl/lib/x86_64/libOpenCL.so" -DPortAudio_DIR="../../../../../thirdparty/portaudio" -DDEFINE_AMD_OPENCL_EXTENSION=1
cmake .. -DCMAKE_PREFIX_PATH="/opt/Qt5.6.3/5.6.3/gcc_64" -DOpenCL_INCLUDE_DIR="../../../../thirdparty/OpenCL-Headers" -DOpenCL_LIBRARY="/opt/rocm/opencl/lib/x86_64/libOpenCL.so" -DPortAudio_DIR="../../../../../thirdparty/portaudio" -DDEFINE_AMD_OPENCL_EXTENSION=1
cd ..
