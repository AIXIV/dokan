cd /d "C:\Development\Projects\dokAIX\Package" &msbuild "dokAIXDriverPackage.vcxproj" /t:sdvViewer /p:configuration="Win8.1 Release" /p:platform=x64
exit %errorlevel% 