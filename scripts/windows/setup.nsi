Unicode true
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "Sections.nsh"
!include "WinVer.nsh"
!include "x64.nsh"

!define APP_NAME "ProjectCane"
!define APP_KEY "Software\ProjectCane\Launcher"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\ProjectCane"

Name "${APP_NAME}"
OutFile "${OUTPUT_EXE}"
InstallDir "$LOCALAPPDATA\Programs\ProjectCane"
RequestExecutionLevel user
SetCompressor /SOLID lzma
SetCompressorDictSize 32
SetOverwrite on
CRCCheck force
VIProductVersion "${VERSION}.0"
VIAddVersionKey /LANG=1033 "ProductName" "ProjectCane"
VIAddVersionKey /LANG=1033 "FileDescription" "ProjectCane Offline Setup"
VIAddVersionKey /LANG=1033 "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=1033 "LegalCopyright" "ProjectCane contributors"

!define MUI_ICON "${APP_ICON}"
!define MUI_UNICON "${APP_ICON}"
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "Install ProjectCane"
!define MUI_WELCOMEPAGE_TEXT "Install or update the ProjectCane launcher for your Windows account.$\r$\n$\r$\nSaves, settings and extracted game files are kept in AppData. Updates and uninstalling the launcher preserve them.$\r$\n$\r$\nYou will need your Sly 1 USA ISO when installing the game for the first time."
!define MUI_FINISHPAGE_RUN "$INSTDIR\bin\cane-launcher.exe"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Var SetupMutex
Var Parameters
Var OptionValue
Var HadDesktopShortcut


Function .onVerifyInstDir
  IfFileExists "$INSTDIR\portable.txt" 0 done
  MessageBox MB_OK|MB_ICONSTOP "Choose a separate folder for the installed launcher. This folder contains a portable copy." /SD IDOK
  Abort
  done:
FunctionEnd

Section "Launcher and Start Menu shortcut" LauncherFiles
  SectionIn RO
  ; Also guard silent installs, where the directory page is skipped.
  IfFileExists "$INSTDIR\portable.txt" 0 +4
    MessageBox MB_OK|MB_ICONSTOP "Setup cannot install into a portable copy." /SD IDOK
    SetErrorLevel 2
    Quit
  ; Check all executable files before changing the installed application.
  !include "${CHECK_LOCKS}"
  SetOutPath "$INSTDIR"
  !include "${PAYLOAD_FILES}"
  IfErrors install_failed
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\ProjectCane"
  SetOutPath "$INSTDIR\bin"
  CreateShortcut "$SMPROGRAMS\ProjectCane\ProjectCane.lnk" "$INSTDIR\bin\cane-launcher.exe" "" "$INSTDIR\bin\cane-launcher.exe" 0
  CreateShortcut "$SMPROGRAMS\ProjectCane\Uninstall ProjectCane.lnk" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "${APP_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "ProjectCane"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "ProjectCane contributors"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\bin\cane-launcher.exe,0"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '$\"$INSTDIR\Uninstall.exe$\"'
  WriteRegStr HKCU "${UNINSTALL_KEY}" "QuietUninstallString" '$\"$INSTDIR\Uninstall.exe$\" /S'
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1
  Goto installed
  install_failed:
    MessageBox MB_OK|MB_ICONSTOP "Installation failed. Close ProjectCane and check that the installation folder is writable, then run setup again." /SD IDOK
    SetErrorLevel 2
    Quit
  installed:
SectionEnd

Section /o "Desktop shortcut" DesktopShortcut
  CreateShortcut "$DESKTOP\ProjectCane.lnk" "$INSTDIR\bin\cane-launcher.exe" "" "$INSTDIR\bin\cane-launcher.exe" 0
SectionEnd

Section -RememberOptions
  ${If} ${SectionIsSelected} ${DesktopShortcut}
    WriteRegDWORD HKCU "${APP_KEY}" "DesktopShortcut" 1
  ${Else}
    Delete "$DESKTOP\ProjectCane.lnk"
    WriteRegDWORD HKCU "${APP_KEY}" "DesktopShortcut" 0
  ${EndIf}
SectionEnd

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "ProjectCane requires 64-bit Windows 10 or later." /SD IDOK
    SetErrorLevel 1
    Quit
  ${EndIf}
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_OK|MB_ICONSTOP "ProjectCane requires Windows 10 or later." /SD IDOK
    SetErrorLevel 1
    Quit
  ${EndIf}
  SetShellVarContext current
  SetRegView 64
  System::Call 'kernel32::CreateMutexW(p 0, i 0, w "Local\ProjectCaneSetup") p .r0 ?e'
  Pop $1
  StrCpy $SetupMutex $0
  ${If} $1 = 183
    MessageBox MB_OK|MB_ICONSTOP "Another ProjectCane setup is already running." /SD IDOK
    SetErrorLevel 2
    Quit
  ${EndIf}
  ${If} $INSTDIR == "$LOCALAPPDATA\Programs\ProjectCane"
    ReadRegStr $0 HKCU "${APP_KEY}" "InstallLocation"
    ${If} $0 != ""
      StrCpy $INSTDIR $0
    ${EndIf}
  ${EndIf}
  ReadRegDWORD $HadDesktopShortcut HKCU "${APP_KEY}" "DesktopShortcut"
  ${If} $HadDesktopShortcut = 1
    !insertmacro SelectSection ${DesktopShortcut}
  ${EndIf}
  ${GetParameters} $Parameters
  ClearErrors
  ${GetOptions} $Parameters "/DESKTOP" $OptionValue
  ${IfNot} ${Errors}
    !insertmacro SelectSection ${DesktopShortcut}
  ${EndIf}
FunctionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${LauncherFiles} "Install the launcher, game runtime and extractor, and add ProjectCane to the Start Menu."
  !insertmacro MUI_DESCRIPTION_TEXT ${DesktopShortcut} "Also add a ProjectCane shortcut to your desktop."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.onInit
  SetShellVarContext current
  SetRegView 64
FunctionEnd

Section "Uninstall"
  ; Delete only files shipped by setup. Preserve all AppData game/user files.
  !include "${CHECK_LOCKS}"
  !include "${UNINSTALL_FILES}"
  Delete "$DESKTOP\ProjectCane.lnk"
  Delete "$SMPROGRAMS\ProjectCane\ProjectCane.lnk"
  Delete "$SMPROGRAMS\ProjectCane\Uninstall ProjectCane.lnk"
  RMDir "$SMPROGRAMS\ProjectCane"
  DeleteRegKey HKCU "${UNINSTALL_KEY}"
  DeleteRegKey HKCU "${APP_KEY}"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
SectionEnd
