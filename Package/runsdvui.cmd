cd /d "C:\Development\dokAIX\dirs-Package" &msbuild "dirs-Package.vcxproj" /t:sdvViewer /p:configuration="Win8 Release" /p:platform=Win32
exit %errorlevel% 