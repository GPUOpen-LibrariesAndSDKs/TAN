@echo off
xcopy /Y /R "..\..\..\..\Thirdparty\OculusSDK1.4\Samples\OculusRoomTiny\OculusRoomTiny (DX11)\main.cpp" .
..\..\..\..\Thirdparty\patchtool\patch.exe -R main.cpp main.cpp.patch
xcopy /Y /R  "..\..\..\..\Thirdparty\OculusSDK1.4\Samples\OculusRoomTiny_Advanced\Common\Win32_DirectXAppUtil.h" .
..\..\..\..\Thirdparty\patchtool\patch.exe -R Win32_DirectXAppUtil.h Win32_DirectXAppUtil.h.patch
