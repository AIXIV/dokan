cd /d "C:\Development\Projects\dokan\Package" &msbuild "dokanDriverPackage.vcxproj" /t:sdvViewer /p:configuration="Win8.1 Release" /p:platform=x64
exit %errorlevel% 