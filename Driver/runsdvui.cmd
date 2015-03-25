cd /d "C:\Development\Projects\dokAIX\Driver" &msbuild "dokanDriver.vcxproj" /t:sdvViewer /p:configuration="Win7 Release" /p:platform=Win32
exit %errorlevel% 