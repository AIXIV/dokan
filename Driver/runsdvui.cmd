cd /d "C:\Development\Projects\dokAIX\Driver" &msbuild "dokAIXDriver.vcxproj" /t:sdvViewer /p:configuration="Win8.1 Debug" /p:platform=x64
exit %errorlevel% 