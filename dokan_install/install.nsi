!define VERSION "0.7.0"

!include LogicLib.nsh
!include x64.nsh
!include WinVer.nsh

Name "dokAIX Library Installer ${VERSION} (enhanced Dokan Library 0.6.0)"
OutFile "dokAIXInstall_${VERSION}.exe"

InstallDir $PROGRAMFILES64\dokAIX\DokanLibrary

RequestExecutionLevel admin
LicenseData "licdata.rtf"
ShowUninstDetails show

Page license
Page components
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

RequestExecutionLevel admin


!macro X86Files os

  SetOutPath $PROGRAMFILES\dokAIX\DokanLibrary
 
    File ..\dokan\readme.txt
    File ..\dokan\readme.ja.txt
    ;File ..\dokan\dokan.h
    File ..\license.gpl.txt
    File ..\license.lgpl.txt
    File ..\license.mit.txt
    ;File ..\dokan\objfre_${os}_x86\i386\dokan.lib
    File ..\dokan_control\objfre_${os}_x86\i386\dokanctl.exe
    File ..\dokan_mount\objfre_${os}_x86\i386\mounter.exe

  SetOutPath $SYSDIR

    File ..\dokan\objfre_${os}_x86\i386\dokan.dll

!macroend


!macro X64Files os

  SetOutPath $PROGRAMFILES64\dokAIX\DokanLibrary

    File ..\dokan\readme.txt
    File ..\dokan\readme.ja.txt
    ;File ..\dokan\dokan.h
    File ..\license.gpl.txt
    File ..\license.lgpl.txt
    File ..\license.mit.txt
    ;File ..\dokan\objfre_${os}_amd64\amd64\dokan.lib
    File ..\dokan_control\objfre_${os}_amd64\amd64\dokanctl.exe
    File ..\dokan_mount\objfre_${os}_amd64\amd64\mounter.exe

  ; Install x86 library version in SysWoW64
  SetOutPath $SYSDIR
    File ..\dokan\objfre_${os}_x86\i386\dokan.dll
    
  ; Install x64 library version in System32
  ${DisableX64FSRedirection}
      SetOutPath $SYSDIR
        File ..\dokan\objfre_${os}_amd64\amd64\dokan.dll
  ${EnableX64FSRedirection}

!macroend


!macro WriteRegistry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "DisplayName" "dokAIX Library ${VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "NoRepair" 1
!macroend

!macro X86DokanSetup
  ExecWait '"$PROGRAMFILES\dokAIX\DokanLibrary\dokanctl.exe" /i a' $0
  DetailPrint "dokanctl returned $0"
  WriteUninstaller $PROGRAMFILES\dokAIX\DokanLibrary\DokanUninstall.exe
  !insertmacro WriteRegistry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "UninstallString" '"$PROGRAMFILES\dokAIX\DokanLibrary\DokanUninstall.exe"'
!macroend

!macro X64DokanSetup
  ExecWait '"$PROGRAMFILES64\dokAIX\DokanLibrary\dokanctl.exe" /i a' $0
  DetailPrint "dokanctl returned $0"
  WriteUninstaller $PROGRAMFILES64\dokAIX\DokanLibrary\DokanUninstall.exe
  !insertmacro WriteRegistry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary" "UninstallString" '"$PROGRAMFILES64\dokAIX\DokanLibrary\DokanUninstall.exe"'
!macroend

!macro DokanUninstall
  ${If} ${RunningX64}
    SetRegView 64
  ${Else}
    SetRegView 32
  ${EndIf}
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DokanLibrary"
  ; try stop x86
  ExecWait '"$PROGRAMFILES\dokAIX\DokanLibrary\dokanctl.exe" /r a' $0
  DetailPrint "dokanctl.exe returned $0"
  ; try stop x64
  ExecWait '"$PROGRAMFILES64\dokAIX\DokanLibrary\dokanctl.exe" /r a' $0
  DetailPrint "dokanctl.exe returned $0"
  
  ; remove all x86 files
  RMDir /r $PROGRAMFILES\dokAIX\DokanLibrary
  RMDir $PROGRAMFILES\dokAIX
  Delete $SYSDIR\dokan.dll
  Delete $SYSDIR\drivers\dokan.sys
  
  ; remove all x64 files
  RMDir /r $PROGRAMFILES64\dokAIX\DokanLibrary
  RMDir $PROGRAMFILES64\dokAIX
  ${DisableX64FSRedirection}
    Delete $SYSDIR\dokan.dll
    Delete $SYSDIR\drivers\dokan.sys
  ${EnableX64FSRedirection}
!macroend

/*!macro X86Driver os
  SetOutPath $SYSDIR\drivers
    ;File ..\sys\objfre_${os}_x86\i386\dokan.sys
!macroend*/

!macro X64Driver os
  ${DisableX64FSRedirection}

  SetOutPath $SYSDIR\drivers
    ;File ..\sys\objfre_${os}_amd64\amd64\dokan.sys
    File signed_dokan_drivers\x64\dokan.sys

  ${EnableX64FSRedirection}
!macroend

/*Section "Dokan Driver x86" section_x86_driver
  ${If} ${IsWin7}
    !insertmacro X86Driver "win7"
  ${ElseIf} ${IsWinVista}
    !insertmacro X86Driver "wlh"
  ${ElseIf} ${IsWin2008}
    !insertmacro X86Driver "wlh"
  ${ElseIf} ${IsWin2003}
    !insertmacro X86Driver "wnet"
  ${ElseIf} ${IsWinXp}
    !insertmacro X86Driver "wxp"
  ${EndIf}
SectionEnd*/

Section "Dokan Driver x64" section_x64_driver
  ${If} ${IsWin7}
    !insertmacro X64Driver "win7"
  ${ElseIf} ${IsWin2008R2}
    !insertmacro X64Driver "win7"
  /*${ElseIf} ${IsWinVista}
    !insertmacro X64Driver "wlh"
  ${ElseIf} ${IsWin2008}
    !insertmacro X64Driver "wlh"
  ${ElseIf} ${IsWin2003}
    !insertmacro X64Driver "wnet"*/
  ${EndIf}
SectionEnd

SectionGroup "Dokan Library" sectiongroup_lib

Section "Dokan Library x86" section_x86
  ${If} ${IsWin7}
    !insertmacro X86Files "win7"
  ${ElseIf} ${IsWin2008R2}
    !insertmacro X86Files "win7"
  /*${ElseIf} ${IsWinVista}
    !insertmacro X86Files "wlh"
  ${ElseIf} ${IsWin2008}
    !insertmacro X86Files "wlh"
  ${ElseIf} ${IsWin2003}
    !insertmacro X86Files "wnet"
  ${ElseIf} ${IsWinXp}
    !insertmacro X86Files "wxp"*/
  ${EndIf}
  !insertmacro X86DokanSetup
SectionEnd

Section "Dokan Library x64" section_x64
  ${If} ${IsWin7}
    !insertmacro X64Files "win7"
  /*${ElseIf} ${IsWinVista}
    !insertmacro X64Files "wlh"
  ${ElseIf} ${IsWin2008}
    !insertmacro X64Files "wlh"
  ${ElseIf} ${IsWin2003}
    !insertmacro X64Files "wnet"*/
  ${EndIf}
  !insertmacro X64DokanSetup
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
  SectionSetFlags ${sectiongroup_lib} $0
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  ${If} ${RunningX64}
    SetRegView 64
    SectionSetFlags ${section_x86} 0
    ;SectionSetFlags ${section_x86_driver} ${SF_RO}
    SectionSetFlags ${section_x64} ${SF_SELECTED}
    SectionSetFlags ${section_x64_driver} $0
    
    StrCpy $R9 ${section_x64}
  ${Else}
    SetRegView 32
    SectionSetFlags ${section_x86} ${SF_SELECTED}
    ;SectionSetFlags ${section_x86_driver} $0
    SectionSetFlags ${section_x64} 0
    SectionSetFlags ${section_x64_driver} ${SF_RO}
    
    StrCpy $R9 ${section_x86}
  ${EndIf}

  ; Windows Version check

  ${If} ${RunningX64}
    /*${If} ${IsWinXP}
    ${ElseIf} ${IsWin2003}
    ${ElseIf} ${IsWinVista}
    ${ElseIf} ${IsWin2008}
    ${ElseIf} ${IsWin7}*/
    ${If} ${IsWin7}
    ${Else}
      MessageBox MB_OK "Your OS is not supported. Dokan library supports Windows 2003, Vista, 2008, 2008R2 and 7 for x64."
      Abort
    ${EndIf}
  ${Else}
    /*${If} ${IsWinXP}
    ${ElseIf} ${IsWin2003}
    ${ElseIf} ${IsWinVista}
    ${ElseIf} ${IsWin2008}
    ${ElseIf} ${IsWin7}*/
    ${If} ${IsWin7}
    ${Else}
      MessageBox MB_OK "Your OS is not supported. Dokan library supports Windows XP, 2003, Vista, 2008 and 7 for x86."
      Abort
    ${EndIf}
  ${EndIf}

  ; Previous version
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
      IfFileExists $SYSDIR\drivers\dokan.sys HasPreviousVersionX64 NoPreviousVersionX64
      ; To make EnableX64FSRedirection called in both cases, needs duplicated MessageBox code. How can I avoid this?
      HasPreviousVersionX64:
        MessageBox MB_OK "Please unstall the previous version and restart your computer before running this installer."
        Abort
      NoPreviousVersionX64:
    ${EnableX64FSRedirection}
  ${Else}
    IfFileExists $SYSDIR\drivers\dokan.sys HasPreviousVersion NoPreviousVersion
    HasPreviousVersion:
      MessageBox MB_OK "Please unstall the previous version and restart your computer before running this installer."
      Abort
    NoPreviousVersion:
  ${EndIf}


FunctionEnd

Function .onSelChange
  Push $0
 
  StrCmp $R9 ${section_x86} check_x86
    ; Previously, x64 was selected
    SectionGetFlags ${section_x86} $0
    IntOp $0 $0 & ${SF_SELECTED}
    IntCmp $0 ${SF_SELECTED} 0 enable_x64 enable_x64 ; deselect x64 if x86 is now selected; make sure that x64 remains selected otherwise
      StrCpy $R9 ${section_x86}
      SectionGetFlags ${section_x64} $0
      IntOp $0 $0 & ${SECTION_OFF}
      SectionSetFlags ${section_x64} $0
      Goto done
      
    enable_x64: ; make sure that x64 remains selected
      SectionGetFlags ${section_x64} $0
      IntOp $0 $0 | ${SF_SELECTED}
      SectionSetFlags ${section_x64} $0
      Goto done
 
  check_x86:
    ; Previously, x86 was selected
    SectionGetFlags ${section_x64} $0
    IntOp $0 $0 & ${SF_SELECTED}
    IntCmp $0 ${SF_SELECTED} 0 enable_x86 enable_x86 ; deselect x86 if x64 is now selected; make sure that x86 remains selected otherwise
      StrCpy $R9 ${section_x64}
      SectionGetFlags ${section_x86} $0
      IntOp $0 $0 & ${SECTION_OFF}
      SectionSetFlags ${section_x86} $0
      Goto done
      
    enable_x86:
      SectionGetFlags ${section_x86} $0
      IntOp $0 $0 | ${SF_SELECTED}
      SectionSetFlags ${section_x86} $0
      
 
  done:
 
  Pop $0
FunctionEnd

Function .onInstSuccess
  ${If} ${RunningX64}
    IfSilent noshellopen
      ExecShell "open" "$PROGRAMFILES64\dokAIX\DokanLibrary"
  ${Else}
    IfSilent noshellopen
      ExecShell "open" "$PROGRAMFILES\dokAIX\DokanLibrary"
  ${EndIf}
  noshellopen:
FunctionEnd

