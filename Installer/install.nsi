!define VERSION "0.7.0"

!include LogicLib.nsh
!include x64.nsh
!include WinVer.nsh

Name "Dokan Library ${VERSION}"
OutFile "DokanInstall_${VERSION}.exe"

InstallDir $PROGRAMFILES64\DokanLibrary

RequestExecutionLevel admin
LicenseData "licdata.rtf"
ShowUninstDetails show

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

RequestExecutionLevel admin

!macro VCRedist arch
    SetOutPath $TEMP
    DetailPrint "Installing Visual Studio runtime for ${arch}.."
    File vcredist_${arch}.exe
    ExecWait '"$TEMP\vcredist_${arch}.exe" /q' $0
    DetailPrint "VCRedist returned $0"
    Delete $TEMP\vcredist_${arch}.exe
!macroend

!macro LicenseFiles
    File ..\readme.txt
    File ..\license.gpl.txt
    File ..\license.lgpl.txt
    File ..\license.mit.txt
!macroend

!macro X86DriverFiles os
    SetOutPath $SYSDIR\drivers
    File ..\Driver\${os}Release\x86\dokan.sys
!macroend

!macro X64DriverFiles os
    ${DisableX64FSRedirection}
    SetOutPath $SYSDIR\drivers
    File ..\Driver\${os}Release\x64\dokan.sys
    ${EnableX64FSRedirection}
!macroend

!macro X86LibraryFiles os installdir
    !insertmacro VCRedist x86
    SetOutPath ${installdir}
    File ..\Library\${os}Release\x86\dokan.dll
!macroend

!macro X64LibraryFiles os installdir
    !insertmacro VCRedist x64
    ${DisableX64FSRedirection}
    SetOutPath ${installdir}
    File ..\Library\${os}Release\x64\dokan64.dll
    ${EnableX64FSRedirection}
!macroend

!macro X86Files os
    SetOutPath $INSTDIR
    !insertmacro LicenseFiles
    File ..\Control\${os}Release\x86\dokanctl.exe
    File ..\MountService\${os}Release\x86\dokanMountService.exe
    
    !insertmacro X86DriverFiles ${os}
!macroend

!macro X64Files os
    SetOutPath $INSTDIR
    !insertmacro LicenseFiles
    File ..\Control\${os}Release\x64\dokanctl.exe
    File ..\MountService\${os}Release\x64\dokanMountService.exe
    
    ${DisableX64FSRedirection}
    !insertmacro X64DriverFiles ${os}
    ${EnableX64FSRedirection}
!macroend

!macro DevelFiles os
    SetOutPath $INSTDIR
    File ..\Library\dokan.h
    File ..\Library\${os}Release\x86\dokan.lib
    File ..\Library\${os}Release\x64\dokan64.lib
!macroend

!macro DokanSetup
    ExecWait '"$INSTDIR\dokanctl.exe" /i d' $0
    ${If} $0 == 0
        DetailPrint "Successfully registered driver!"
    ${Else}
        DetailPrint "Driver registration failed!"
    ${EndIf}
    ExecWait '"$INSTDIR\dokanctl.exe" /i s' $0
    ${If} $0 == 0
        DetailPrint "Successfully registered mounter service!"
    ${Else}
        DetailPrint "Mounter service registration failed!"
    ${EndIf}
    WriteUninstaller $INSTDIR\DokanUninstall.exe
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "DisplayName" "Dokan Library ${VERSION}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "NoRepair" 1
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "UninstallString" '"$INSTDIR\DokanUninstall.exe"'
!macroend

!macro DokanUninstall
    ${If} ${RunningX64}
        SetRegView 64
    ${Else}
        SetRegView 32
    ${EndIf}
    ExecWait '"$INSTDIR\dokanctl.exe" /r d' $0
    ${If} $0 == 0
        DetailPrint "Successfully unregistered driver!"
    ${Else}
        DetailPrint "Driver removal failed!"
    ${EndIf}
    ExecWait '"$INSTDIR\dokanctl.exe" /r s' $0
    ${If} $0 == 0
        DetailPrint "Successfully unregistered mounter service!"
    ${Else}
        DetailPrint "Mounter service removal failed!"
    ${EndIf}

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary"
    
    RMDir /r $INSTDIR
    RMDir $INSTDIR
    Delete $SYSDIR\dokan.dll
    Delete $SYSDIR\drivers\dokan.sys
    ${DisableX64FSRedirection}
        Delete $SYSDIR\dokan64.dll
        Delete $SYSDIR\drivers\dokan.sys
    ${EnableX64FSRedirection}
!macroend

SectionGroup "Dokan Library" sectiongroup_library

Section "Dokan Library x86" section_library_x86
    ${If} ${IsWin7}
        !insertmacro X86LibraryFiles "Win7" $SYSDIR
    ${ElseIf} ${IsWin8}
        !insertmacro X86LibraryFiles "Win8" $SYSDIR
    ${ElseIf} ${IsWin8.1}
        !insertmacro X86LibraryFiles "Win8.1" $SYSDIR
    ${Else}
        !insertmacro X86LibraryFiles "Win7" $SYSDIR
    ${EndIf}
SectionEnd

Section "Dokan Library x64" section_library_x64
    ${DisableX64FSRedirection}
    ${If} ${IsWin7}
        !insertmacro X64LibraryFiles "Win7" $SYSDIR
    ${ElseIf} ${IsWin8}
        !insertmacro X64LibraryFiles "Win8" $SYSDIR
    ${ElseIf} ${IsWin8.1}
        !insertmacro X64LibraryFiles "Win8.1" $SYSDIR
    ${Else}
        !insertmacro X64LibraryFiles "Win7" $SYSDIR
    ${EndIf}
    ${EnableX64FSRedirection}
SectionEnd

Section "Development Files" section_library_devel
    ${If} ${IsWin7}
        !insertmacro DevelFiles "Win7"
    ${ElseIf} ${IsWin8}
        !insertmacro DevelFiles "Win8"
    ${ElseIf} ${IsWin8.1}
        !insertmacro DevelFiles "Win8.1"
    ${Else}
        !insertmacro DevelFiles "Win7"
    ${EndIf}
SectionEnd

SectionGroupEnd

SectionGroup "Dokan Driver" sectiongroup_driver

Section "Dokan Driver x86" section_driver_x86
    ${If} ${IsWin7}
        !insertmacro X86Files "Win7"
    ${ElseIf} ${IsWin8}
        !insertmacro X86Files "Win8"
    ${ElseIf} ${IsWin8.1}
        !insertmacro X86Files "Win8.1"
    ${Else}
        !insertmacro X86Files "Win7"
    ${EndIf}
    !insertmacro DokanSetup
SectionEnd

Section "Dokan Driver x64" section_driver_x64
    ${If} ${IsWin7}
        !insertmacro X64Files "Win7"
    ${ElseIf} ${IsWin8}
        !insertmacro X64Files "Win8"
    ${ElseIf} ${IsWin8.1}
        !insertmacro X64Files "Win8.1"
    ${Else}
        !insertmacro X64Files "Win7"
    ${EndIf}
    !insertmacro DokanSetup
SectionEnd

SectionGroupEnd


Section "Uninstall"

  !insertmacro DokanUninstall

  IfSilent noreboot
    MessageBox MB_YESNO "A reboot is required to finish the uninstallation. Do you wish to reboot now?" IDNO noreboot
    Reboot
  noreboot:


SectionEnd

Function .onInit
  IntOp $0 ${SF_SECGRP} | ${SF_EXPAND}
  SectionSetFlags ${sectiongroup_library} $0
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  ${If} ${RunningX64}
    SetRegView 64
    SectionSetFlags ${section_driver_x86} ${SF_RO}
    SectionSetFlags ${section_driver_x64} $0
    SectionSetFlags ${section_library_x64} $0
  ${Else}
    SetRegView 32
    SectionSetFlags ${section_driver_x86} $0
    SectionSetFlags ${section_driver_x64} ${SF_RO}
    SectionSetFlags ${section_library_x86} $0
  ${EndIf}

  ; Windows version check

    ${If} ${IsWin7}
    ${ElseIf} ${IsWin8}
    ${ElseIf} ${IsWin8.1}
    ;${ElseIf} ${IsWinVista}
    ${Else}
      MessageBox MB_OK "Your OS is not supported. Dokan library supports Windows 7, 8 and 8.1."
      Abort
    ${EndIf}

  ; Previous version check
  ;${If} ${RunningX64}
    ${DisableX64FSRedirection}
      IfFileExists $SYSDIR\drivers\dokan.sys HasPreviousVersionX64 NoPreviousVersionX64
      HasPreviousVersionX64:
        MessageBox MB_OK "Please remove the previous version and restart your computer before running this installer."
        Abort
      NoPreviousVersionX64:
    ${EnableX64FSRedirection}
  /*${Else}
    IfFileExists $SYSDIR\drivers\dokan.sys HasPreviousVersion NoPreviousVersion
    HasPreviousVersion:
      MessageBox MB_OK "Please remove the previous version and restart your computer before running this installer."
      Abort
    NoPreviousVersion:
  ${EndIf}*/


FunctionEnd
