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
Name "Zim"


; The file to write
OutFile "zim.exe"

; Request application privileges for Windows Vista and higher
RequestExecutionLevel admin

; Build Unicode installer
Unicode True


; The default installation directory
InstallDir $PROGRAMFILES\Zim

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Zim" "Install_Dir"

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
Section "Zim (required)"

  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r "thoth-windows\*"
  CreateDirectory "$APPDATA\zim"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Zim "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Zim" "DisplayName" "Zim"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Zim" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Zim" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Zim" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Zim"
  CreateShortcut "$SMPROGRAMS\Zim\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\zim.lnk" "$INSTDIR\zim.exe" "" "$INSTDIR\zim.ico" 0
  CreateShortcut "$SMPROGRAMS\Zim\Zim.lnk" "$INSTDIR\Zim.exe"

SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Zim"
  DeleteRegKey HKLM SOFTWARE\Zim

  ; Remove files and uninstaller
  RMDir /r $INSTDIR
  RMDir /r "$APPDATA\zim"
 

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Zim\*.lnk"
  Delete "$DESKTOP\zim.lnk"

  ; Remove directories
  RMDir "$SMPROGRAMS\Zim"
  RMDir "$INSTDIR"

SectionEnd
