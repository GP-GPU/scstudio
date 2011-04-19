; scstudio - Sequence Chart Studio
; http://scstudio.sourceforge.net
;
;       This library is free software; you can redistribute it and/or
;       modify it under the terms of the GNU Lesser General Public
;       License version 2.1, as published by the Free Software Foundation.
;
;       This library is distributed in the hope that it will be useful,
;       but WITHOUT ANY WARRANTY; without even the implied warranty of
;       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;       Lesser General Public License for more details.
;
; Copyright (c) 2008-2010 Petr Gotthard <petr.gotthard@centrum.cz>
;
; $Id: scstudio.nsi 1064 2011-03-17 13:42:42Z xrehak $

; compile this file using NSIS from http://nsis.sourceforge.net
; e.g. "C:\Program Files\NSIS\makensis" scstudio.nsi

!include 'MUI2.nsh'

; -- General ---------------------------

Name "Sequence Chart Studio"
; Name "Sequence Chart Studio (for NSN)"
OutFile "scstudio-setup-${VERSION}.exe"
; OutFile "scstudio-setup-${VERSION}-NSN.exe"

InstallDir "$PROGRAMFILES\Sequence Chart Studio"

!define RegMainPath "Software\Sequence Chart Studio"
!define RegModulesPath "Software\Sequence Chart Studio\Modules"
!define RegStencilsPath "Software\Sequence Chart Studio\Stencils"
!define RegChecksPath "Software\Sequence Chart Studio\Checks"

!define Visio11RegPath "Software\Microsoft\Office\11.0\Visio"
!define Visio12RegPath "Software\Microsoft\Office\12.0\Visio"
!define Visio14RegPath "Software\Microsoft\Office\14.0\Visio"

RequestExecutionLevel admin

; -- Pages -----------------------------

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; -- Languages -------------------------

!insertmacro MUI_LANGUAGE "English"

; -- Installer Sections ----------------

!define StrStr "!insertmacro StrStr"
!define AppendRegStr "!insertmacro AppendRegStr"
!define RemoveMatchedFileLines "!insertmacro RemoveMatchedFileLines"

!macro StrStr ResultVar String SubString
  Push `${String}`
  Push `${SubString}`
  Call StrStr
  Pop `${ResultVar}`
!macroend

; appends ${value} to HKCU registry ${key}, if not already included
!macro AppendRegStr key1 key2 name value
  !define Index 'Line${__LINE__}'
  Push ${key1}
  Call CountRegKeys
  Pop $0
  IntCmp $0 0 "${Index}-Skip" "${Index}-Skip" 0

  ReadRegStr $0 HKCU "${key1}\${key2}" ${name}
  ; check if the value already included in the path-list
  ${StrStr} $1 $0 ${value}
  ; if no, write value to registry
  ${If} $1 == ""
    ; check if some other path is already in the path-list
    ${If} $0 != ""
      ; if yes, append our value to the list
      StrCpy $2 "$0;${value}"
    ${Else}
      StrCpy $2 "${value}"
    ${Endif}
    WriteRegStr HKCU "${key1}\${key2}" ${name} $2
  ${EndIf}
"${Index}-Skip:"
  !undef Index
!macroend

; scan ${File} and remove lines matching the ${Pattern}
!macro RemoveMatchedFileLines File Pattern
  Push "${File}"
  Push "${Pattern}"
  CallInstDLL "$TEMP\nsis.dll" RemoveMatchedFileLines
!macroend

Section "Microsoft Visio Add-On" SecAddon
  SetOutPath $INSTDIR\bin
  DetailPrint "Installing Microsoft libraries"
  File "redistribute\Microsoft.VC90.ATL.manifest"
  File "redistribute\atl90.dll"
  File "redistribute\Microsoft.VC90.CRT.manifest"
  File "redistribute\msvcm90.dll"
  File "redistribute\msvcp90.dll"
  File "redistribute\msvcr90.dll"
  File "redistribute\lpsolve55.dll"
  File "redistribute\capicom.dll"
  ExecWait 'regsvr32.exe /s "$INSTDIR\capicom.dll"'

  SetOutPath $TEMP
  !define CertificateName "scstudio.cer"
  File "setup-nsis\${BUILD_TYPE}\nsis.dll"
  File ${CertificateName}
  DetailPrint "Installing the publisher certificate"
  Push "$TEMP\${CertificateName}"
  CallInstDLL "$TEMP\nsis.dll" InstallPublisherCertificate
  Pop $R1
  DetailPrint "InstallPublisherCertificate returned $R1"
  Delete "$TEMP\${CertificateName}"

  DetailPrint "Updating Microsoft Visio configuration"
  ; remove possibly ill entries from the content.dat prior (re-)installation
  ${RemoveMatchedFileLines} "Microsoft\Visio\content.dat" "scstudio"
  ${RemoveMatchedFileLines} "Microsoft\Visio\content12.dat" "scstudio"
  ${RemoveMatchedFileLines} "Microsoft\Visio\content14.dat" "scstudio"
  Delete "$TEMP\nsis.dll"

  SetOutPath $INSTDIR\bin
  File "addon\${BUILD_TYPE}\scstudio.vsl"
  File "addon\${BUILD_TYPE}\scstudio.vsl.intermediate.manifest"
  ; File "..\..\..\doc\help\scstudio.chm"
  File "..\..\..\${BUILD_TYPE}\*.dll"


  ${AppendRegStr} ${Visio11RegPath} "Application" "AddonsPath" "$INSTDIR\bin"
  ${AppendRegStr} ${Visio12RegPath} "Application" "AddonsPath" "$INSTDIR\bin"
  ${AppendRegStr} ${Visio14RegPath} "Application" "AddonsPath" "$INSTDIR\bin"

  SetOutPath $INSTDIR\bin\help
  File /r "..\..\..\doc\help\*.html"
  File /r "..\..\..\doc\help\*.css"
  File /r "..\..\..\doc\help\*.png"

  SetOutPath "$INSTDIR\stencils\Sequence Chart Studio"
  File "stencils\Sequence Chart Studio\Basic MSC.vsx"
  File "stencils\Sequence Chart Studio\HMSC.vsx"
  ; File "stencils\Sequence Chart Studio\NSN Pictograms.vsx"
  File "stencils\Sequence Chart Studio\MSC.vtx"
  ; File "stencils\Sequence Chart Studio\MSC (for NSN).vtx"

  ; modify Visio 2003 add-on paths
  ${AppendRegStr} ${Visio11RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${AppendRegStr} ${Visio11RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"
  ; modify Visio 2007 add-on paths
  ${AppendRegStr} ${Visio12RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${AppendRegStr} ${Visio12RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"
  ; modify Visio 2010 add-on paths
  ${AppendRegStr} ${Visio14RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${AppendRegStr} ${Visio14RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"

  ; remove old modules
  ; FIXME: remove modules with the "sc_" prefix
  DeleteRegKey HKCU "${RegMainPath}"
  ; register modules
  WriteRegStr HKCU '${RegMainPath}' 'ModulesPath' '$INSTDIR\bin'
  WriteRegDWORD HKCU '${RegMainPath}' 'ChannelType' '0'
  WriteRegDWORD HKCU '${RegMainPath}' 'OutputLevel' '2'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_boundedness' 'scboundedness.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_liveness' 'scliveness.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_localchoice' 'sclocalchoice.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_order' 'scorder.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_structure' 'scstructure.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_race' 'scrace.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_realizability' 'screalizability.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_time' 'sctime.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_z120' 'scZ120.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_engmann' 'scengmann.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_modelchecking' 'scmodelchecking.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_membership' 'scmembership.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_montecarlo' 'scmontecarlo.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_beautify' 'scbeautify.dll'
  WriteRegStr HKCU '${RegModulesPath}' 'sc_pycheck' 'scpycheck.dll'
  ; register stencils
  WriteRegStr HKCU '${RegStencilsPath}' 'bmsc' 'Basic MSC.vsx'
  WriteRegStr HKCU '${RegStencilsPath}' 'hmsc' 'HMSC.vsx'
  ; WriteRegStr HKCU '${RegStencilsPath}' 'nsn_pictograms' 'NSN Pictograms.vsx'
  ; configure checks
  WriteRegDWORD HKCU '${RegChecksPath}\Universal Boundedness' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Deadlock Free' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Livelock Free' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Local Choice' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Acyclic' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\FIFO' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Unique Instance Names' 'Priority' '2'
  WriteRegDWORD HKCU '${RegChecksPath}\Nonrecursivity' 'Priority' '2'
  WriteRegDWORD HKCU '${RegChecksPath}\Race Free' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Strong Realizability' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Correct Time Constraint Syntax' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Time Consistent' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\Time Race' 'Priority' '0'
  ; configure beautify
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'InstanceHeadDistance' '5'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'SuccessorDistance' '5'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'SendReceiveDistance' '0'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'IncompleteMessageLength' '20'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'CrossingPenalization' '1'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'GoingBackPenalization' '1'
  WriteRegDWORD HKCU '${RegMainPath}\Beautify' 'MinInstanceBottomDistance' '5'
  ; configure pycheck
  ;WriteRegDWORD HKCU '${RegChecksPath}\PyBDeadlockFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyHDeadlockFree' 'Priority' '0'
  ;WriteRegDWORD HKCU '${RegChecksPath}\PyBLivelockFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyHLivelockFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyBAcyclicFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyHAcyclicFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyBFIFOFree' 'Priority' '0'
  WriteRegDWORD HKCU '${RegChecksPath}\PyHFIFOFree' 'Priority' '0'
  ;WriteRegDWORD HKCU '${RegChecksPath}\PyBUniversalFree' 'Priority' '0'
  ;WriteRegDWORD HKCU '${RegChecksPath}\PyHUniversalFree' 'Priority' '0'
  ; configure simulator
  WriteRegBin HKCU '${RegMainPath}\Simulator' 'BinWidth' 0000803F
  WriteRegBin HKCU '${RegMainPath}\Simulator' 'MaxMessageDelay' 00002041

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ; Write uninstall information
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SCStudio" \
                   "DisplayName" "Sequence Chart Studio ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SCStudio" \
                   "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

LangString DESC_SecAddon ${LANG_ENGLISH} "Installs Microsoft Visio Add-on and related stencils and templates."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAddon} $(DESC_SecAddon)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; -- Uninstaller Section ---------------

!define StrRep "!insertmacro StrRep"
!define RemoveRegStr "!insertmacro RemoveRegStr"

!macro StrRep ResultVar String OldText NewText
  Push `${String}` ;String to do replacement in (haystack)
  Push `${OldText}` ;String to replace (needle)
  Push `${NewText}` ;Replacement
  Call un.StrRep
  Pop `${ResultVar}`
!macroend

; removes ${value} from HKCU registry ${key}, if included
!macro RemoveRegStr key1 key2 name value
  !define Index 'Line${__LINE__}'
  Push ${key1}
  Call un.CountRegKeys
  Pop $0
  IntCmp $0 0 "${Index}-Skip" "${Index}-Skip" 0

  ReadRegStr $0 HKCU "${key1}\${key2}" ${name}
  ; remove the value
  ${StrRep} $1 $0 "${value}" ""
  ; remove unnecessary delimiters
  ${StrRep} $2 $1 ";;" ";"
  WriteRegStr HKCU "${key1}\${key2}" ${name} $2
"${Index}-Skip:"
  !undef Index
!macroend

Section "Uninstall"
  ExecWait 'regsvr32.exe /s /u "$INSTDIR\capicom.dll"'

  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\stencils\Sequence Chart Studio"
  RMDir "$INSTDIR\stencils"

  ; modify Visio 2003 add-on paths
  ${RemoveRegStr} ${Visio11RegPath} "Application" "AddonsPath" "$INSTDIR\bin"
  ${RemoveRegStr} ${Visio11RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${RemoveRegStr} ${Visio11RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"
  ; modify Visio 2007 add-on paths
  ${RemoveRegStr} ${Visio12RegPath} "Application" "AddonsPath" "$INSTDIR\bin"
  ${RemoveRegStr} ${Visio12RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${RemoveRegStr} ${Visio12RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"
  ; modify Visio 2010 add-on paths
  ${RemoveRegStr} ${Visio14RegPath} "Application" "AddonsPath" "$INSTDIR\bin"
  ${RemoveRegStr} ${Visio14RegPath} "Application" "StencilPath" "$INSTDIR\stencils"
  ${RemoveRegStr} ${Visio14RegPath} "Application" "TemplatePath" "$INSTDIR\stencils"

  ; unregister modules
  DeleteRegKey HKCU "Software\Sequence Chart Studio"

  Delete "$INSTDIR\Uninstall.exe"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SCStudio"

  RMDir "$INSTDIR"
SectionEnd

; -- Functions -------------------------

; StrStr $0 "String" "SubString"
; http://nsis.sourceforge.net/StrStr

!macro __StrStr un
Function ${un}StrStr
/*After this point:
  ------------------------------------------
  $R0 = SubString (input)
  $R1 = String (input)
  $R2 = SubStringLen (temp)
  $R3 = StrLen (temp)
  $R4 = StartCharPos (temp)
  $R5 = TempStr (temp)*/
 
  ;Get input from user
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
 
  ;Get "String" and "SubString" length
  StrLen $R2 $R0
  StrLen $R3 $R1
  ;Start "StartCharPos" counter
  StrCpy $R4 0
 
  ;Loop until "SubString" is found or "String" reaches its end
  ${Do}
    ;Remove everything before and after the searched part ("TempStr")
    StrCpy $R5 $R1 $R2 $R4
 
    ;Compare "TempStr" with "SubString"
    ${IfThen} $R5 == $R0 ${|} ${ExitDo} ${|}
    ;If not "SubString", this could be "String"'s end
    ${IfThen} $R4 >= $R3 ${|} ${ExitDo} ${|}
    ;If not, continue the loop
    IntOp $R4 $R4 + 1
  ${Loop}
 
/*After this point:
  ------------------------------------------
  $R0 = ResultVar (output)*/
 
  ;Remove part before "SubString" on "String" (if there has one)
  StrCpy $R0 $R1 `` $R4
 
  ;Return output to user
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd
!macroend
!insertmacro __StrStr ""

; StrRep $0 "String" "OldText" "NewText"
; http://nsis.sourceforge.net/Replace_Sub_String_%28macro%29

!macro __StrRep un
Function ${un}StrRep
  ;Written by dirtydingus 2003-02-20 04:30:09 
  ; USAGE
  ;Push String to do replacement in (haystack)
  ;Push String to replace (needle)
  ;Push Replacement
  ;Call StrRep
  ;Pop $R0 result	
 
  Exch $R4 ; $R4 = Replacement String
  Exch
  Exch $R3 ; $R3 = String to replace (needle)
  Exch 2
  Exch $R1 ; $R1 = String to do replacement in (haystack)
  Push $R2 ; Replaced haystack
  Push $R5 ; Len (needle)
  Push $R6 ; len (haystack)
  Push $R7 ; Scratch reg
  StrCpy $R2 ""
  StrLen $R5 $R3
  StrLen $R6 $R1
loop:
  StrCpy $R7 $R1 $R5
  StrCmp $R7 $R3 found
  StrCpy $R7 $R1 1 ; - optimization can be removed if U know len needle=1
  StrCpy $R2 "$R2$R7"
  StrCpy $R1 $R1 $R6 1
  StrCmp $R1 "" done loop
found:
  StrCpy $R2 "$R2$R4"
  StrCpy $R1 $R1 $R6 $R5
  StrCmp $R1 "" done loop
done:
  StrCpy $R3 $R2
  Pop $R7
  Pop $R6
  Pop $R5
  Pop $R2
  Pop $R1
  Pop $R4
  Exch $R3
FunctionEnd
!macroend
!insertmacro __StrRep "un."

; CountKeys "Subkey"
; http://nsis.sourceforge.net/Check_for_a_Registry_Key

!macro __CountRegKeys un
Function ${un}CountRegKeys
  Exch $R0 ; subkey
  Push $R1
  Exch
  Push $R2
  StrCpy $R1 "0"
loop: 
  ;Check for Key
  EnumRegKey $R2 HKCU $R0 $R1
  StrCmp $R2 "" done
  IntOp $R1 $R1 + 1
  goto loop
done:
  Pop $R2
  Pop $R0
  Exch $R1
FunctionEnd
!macroend
!insertmacro __CountRegKeys ""
!insertmacro __CountRegKeys "un."

; $Id: scstudio.nsi 1064 2011-03-17 13:42:42Z xrehak $
