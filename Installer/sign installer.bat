@echo off
signtool sign /sha1 DFAD1123683CB6A00E7838C66D939DCE0FBF2BF7 DokanInstall_0.7.0.exe
sleep 1
signtool timestamp /t http://timestamp.verisign.com/scripts/timestamp.dll DokanInstall_0.7.0.exe