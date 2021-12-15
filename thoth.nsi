; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install thoth into a directory that the user selects.


!macro BIMAGE IMAGE
  Push $0
  GetTempFileName $0
  File /oname=$0 "${IMAGE}"
  SetBrandingImage $0 /RESIZETOFIT
  Delete $0
  Pop $0
!macroend
;--------------------------------


; The name of the installer
Name "Thoth"


; The file to write
OutFile "thoth.exe"

; Request application privileges for Windows Vista and higher
RequestExecutionLevel admin

; Build Unicode installer
Unicode True


; The default installation directory
InstallDir $PROGRAMFILES\Thoth

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Thoth" "Install_Dir"

;--------------------------------
SetFont Verdana 8

AddBrandingImage left 100

; Pages

Function thothImage
  !insertmacro BIMAGE "thoth.bmp"
FunctionEnd
Function un.thothImage
  !insertmacro BIMAGE "thoth.bmp"
FunctionEnd

; Pages

Page components thothImage
Page directory thothImage
Page instfiles thothImage

UninstPage uninstConfirm un.thothImage
UninstPage instfiles un.thothImage

;--------------------------------

; The stuff to install
Section "Thoth (required)"

  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r "thoth-windows\*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Thoth "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Thoth" "DisplayName" "Thoth"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Thoth" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Thoth" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Thoth" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Thoth"
  CreateShortcut "$SMPROGRAMS\Thoth\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\thoth.lnk" "$INSTDIR\thoth.exe" "" "$INSTDIR\thoth.ico" 0
  CreateShortcut "$SMPROGRAMS\Thoth\Thoth.lnk" "$INSTDIR\thoth.exe"

SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Thoth"
  DeleteRegKey HKLM SOFTWARE\Thoth

  ; Remove files and uninstaller
  RMDir /r $INSTDIR
 

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Thoth\*.lnk"
  Delete "$DESKTOP\thoth.lnk"

  ; Remove directories
  RMDir "$SMPROGRAMS\Thoth"
  RMDir "$INSTDIR"

SectionEnd
