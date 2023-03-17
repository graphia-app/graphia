; makensis would normally be invoked from the root of the source tree
!addincludedir "installers\windows"
!addplugindir "installers\windows"

!include "MUI2.nsh"
!include "fileassoc.nsh"

!ifndef PRODUCT_NAME
!define PRODUCT_NAME "unspecified-product"
!endif

!define EXE "${PRODUCT_NAME}.exe"

!ifndef NATIVE_EXTENSION
!define NATIVE_EXTENSION "unspecified-extension"
!endif

!ifndef VERSION
!define VERSION "unspecified-version"
!endif

!ifndef PUBLISHER
!define PUBLISHER "unspecified-publisher"
!endif

!ifndef COPYRIGHT
!define COPYRIGHT "unspecified-copyright"
!endif

!searchreplace COPYRIGHT "${COPYRIGHT}" "(c)" "©"

!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

Var ACCOUNT_TYPE
Var ALREADY_RUNNING
Var ADMIN_INSTALL
Var USER_INSTALL

Name "${PRODUCT_NAME}"

VIProductVersion "1.0.0.0"
VIAddVersionKey ProductName "${PRODUCT_NAME}"
VIAddVersionKey Comments "${PRODUCT_NAME}"
VIAddVersionKey LegalCopyright "${COPYRIGHT}"
VIAddVersionKey FileDescription "${PRODUCT_NAME}"
VIAddVersionKey FileVersion "1.0.0.0"
VIAddVersionKey ProductVersion "1.0.0.0"

InstallDir ""

; Take the highest execution level available
; This means that if it's possible to, we become an administrator
RequestExecutionLevel highest

!macro ConsoleLog MESSAGE
    System::Call 'kernel32::AttachConsole(i -1)i.r0' ;attach to parent console
    System::Call 'kernel32::GetStdHandle(i -11)i.r0' ;console attached -- get stdout
    FileWrite $0 "${MESSAGE}$\r$\n"
!macroend

!macro CheckIfStillRunning
    nsProcess::_FindProcess "${EXE}"
    Pop $ALREADY_RUNNING
!macroend

!macro NormalisePath PATH
    nsExec::ExecToStack '"cmd" /q /c "for %a in ($\"${PATH}$\") do echo | set /p=$\"%~fa$\""'
    Pop $0 ; Exit code, should be 0
    Pop ${PATH}
!macroend

; Test if an installation actually exists, i.e. it is not just a stale registry entry
!macro ValidateInstallation INSTALLATION
    !insertmacro NormalisePath ${INSTALLATION}
    ${If} ${INSTALLATION} != ""
        IfFileExists "${INSTALLATION}\${EXE}" +2
            StrCpy "${INSTALLATION}" ""
    ${EndIf}
!macroend

!macro FatalError MESSAGE
    MessageBox MB_OK|MB_ICONEXCLAMATION "${MESSAGE}" /SD IDOK
    !insertmacro ConsoleLog "${MESSAGE}"
    Abort
!macroend

!macro ONINIT un
    Function ${un}.onInit
        !insertmacro CheckIfStillRunning
        IfSilent 0 notSilent
            IntCmp $ALREADY_RUNNING 603 notRunning
                !insertmacro ConsoleLog "Waiting for ${PRODUCT_NAME} to close..."
                Sleep 10000 ; In silent mode, give the app a few seconds to close then...
                !insertmacro CheckIfStillRunning ; ...check again

        notSilent:
        IntCmp $ALREADY_RUNNING 603 notRunning
            !insertmacro FatalError "${PRODUCT_NAME} is still running. Please close it before making changes."
        notRunning:

        UserInfo::GetAccountType
        Pop $ACCOUNT_TYPE
        !insertmacro ConsoleLog "Account type: $ACCOUNT_TYPE"

        ; Find existing installations
        ReadRegStr $ADMIN_INSTALL HKLM "Software\${PRODUCT_NAME}" ""
        !insertmacro ValidateInstallation $ADMIN_INSTALL

        ReadRegStr $USER_INSTALL HKCU "Software\${PRODUCT_NAME}" ""
        !insertmacro ValidateInstallation $USER_INSTALL

        ${If} $INSTDIR == ""
            ; This only happens in the installer, because the uninstaller already knows INSTDIR

            ; The value of SetShellVarContext detetmines whether SHCTX is HKLM or HKCU
            ; and whether SMPROGRAMS refers to all users or just the current user

            ${If} $USER_INSTALL != ""
                ; There is an existing user install
                SetShellVarContext current
                StrCpy $INSTDIR "$USER_INSTALL"
                !insertmacro ConsoleLog "Found existing user installation: $INSTDIR"
            ${ElseIf} $ADMIN_INSTALL != ""
            ${AndIf} $ACCOUNT_TYPE == "Admin"
                ; There is an existing admin install
                SetShellVarContext all
                StrCpy $INSTDIR "$ADMIN_INSTALL"
                !insertmacro ConsoleLog "Found existing admin installation: $INSTDIR"
            ${Else}
                ${If} $ACCOUNT_TYPE == "Admin"
                    ; If we're an admin, default to installing to C:\Program Files
                    SetShellVarContext all
                    StrCpy $INSTDIR "$PROGRAMFILES64\${PRODUCT_NAME}"
                    !insertmacro ConsoleLog "Creating new admin installation: $INSTDIR"
                ${Else}
                    ; If we're just a user, default to installing to ~\AppData\Local
                    SetShellVarContext current
                    StrCpy $INSTDIR "$LOCALAPPDATA\${PRODUCT_NAME}"
                    !insertmacro ConsoleLog "Creating new user installation: $INSTDIR"
                ${EndIf}
            ${EndIf}
        ${ElseIf} "${un}" == "un"
            ${If} $ACCOUNT_TYPE == "Admin"
                SetShellVarContext all
            ${Else}
                SetShellVarContext current
            ${EndIf}
        ${Else}
            ; When INSTDIR is given on the command line, and we're not making the uninstaller
            !insertmacro NormalisePath $INSTDIR

            ${If} $INSTDIR == $ADMIN_INSTALL
                SetShellVarContext all
                !insertmacro ConsoleLog "Requested directory matches admin installation: $INSTDIR"
            ${ElseIf} $INSTDIR == $USER_INSTALL
                SetShellVarContext current
                !insertmacro ConsoleLog "Requested directory matches user installation: $INSTDIR"
            ${EndIf}
        ${EndIf}
    FunctionEnd
!macroend

; Define the function twice, once for the installer and again for the uninstaller
!insertmacro ONINIT ""
!insertmacro ONINIT "un"

; Installer Icons
!insertmacro MUI_DEFAULT MUI_ICON "source\app\icon\Installer.ico"
!insertmacro MUI_DEFAULT MUI_UNICON "source\app\icon\Installer.ico"

Icon "${MUI_ICON}"
UninstallIcon "${MUI_UNICON}"

WindowIcon on

; This bitmap needs to be in BMP3 (Windows 3.x) format, for some reason
!define MUI_WELCOMEFINISHPAGE_BITMAP "installers\windows\welcomepage.bmp"
!define MUI_WELCOMEPAGE_TEXT \
"Setup will guide you through the installation of ${PRODUCT_NAME}.$\r$\n$\r$\n\
${PRODUCT_NAME} is a tool for the visualisation and analysis of \
graphs.$\r$\n$\r$\n\
Click Next to continue."
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE "source\app\ui\licensing\LICENSE.rtf"

!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${PRODUCT_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_CHECKED
!define MUI_FINISHPAGE_RUN_TEXT "Start ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_RUN_FUNCTION "Launch"

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "-Main Component"
    SetOutPath "$INSTDIR"

    ; If there is an uninstaller at the destination, invoke it first
    IfFileExists "$INSTDIR\Uninstall.exe" 0 +2
    ExecWait '"$INSTDIR\Uninstall.exe" /S _?=$INSTDIR'

    File /r "installer\*.*"

    WriteRegStr SHCTX "Software\${PRODUCT_NAME}" "" $INSTDIR

    ; These registry entries are necessary for the program to show up in the Add/Remove programs dialog
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\${EXE}"
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "Publisher" "${PUBLISHER}"
    WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "EstimatedSize" "${BUILD_SIZE}"
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr SHCTX "${UNINSTALL_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "NoRepair" 1

    ; Register protocol handler for dealing with hyperlinks
    WriteRegStr HKCR "${NATIVE_EXTENSION}" "" "URL:${PRODUCT_NAME} Protocol"
    WriteRegStr HKCR "${NATIVE_EXTENSION}" "URL Protocol" ""
    WriteRegStr HKCR "${NATIVE_EXTENSION}\shell\open\command" "" "$\"$INSTDIR\${EXE}$\" $\"%1$\""

    IfFileExists "$INSTDIR\Uninstall.exe" +2 ; Don't make Uninstall.exe if it already exists
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    !insertmacro MUI_STARTMENU_WRITE_BEGIN ${PRODUCT_NAME}
        CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\"
        CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE}"
    !insertmacro MUI_STARTMENU_WRITE_END

    !insertmacro UPDATEFILEASSOC
SectionEnd

; File Associations
SectionGroup /e "File associations"
    Section "${PRODUCT_NAME} file (.${NATIVE_EXTENSION})"
        IfSilent skipFileAssociation
        !insertmacro APP_ASSOCIATE "${NATIVE_EXTENSION}" "${PRODUCT_NAME}.Native" "${PRODUCT_NAME} File" \
            "$INSTDIR\${EXE},0" "Open with ${PRODUCT_NAME}" \
            "$\"$INSTDIR\${EXE}$\" $\"%1$\""
        skipFileAssociation:
    SectionEnd
    Section "Graph Modelling Language file (.gml)"
        IfSilent skipFileAssociation
        !insertmacro APP_ASSOCIATE "gml" "${PRODUCT_NAME}.gml" "${PRODUCT_NAME} GML File" \
            "$INSTDIR\${EXE},0" "Open with ${PRODUCT_NAME}" \
            "$\"$INSTDIR\${EXE}$\" $\"%1$\""
        skipFileAssociation:
    SectionEnd
    Section "Graph Markup Language file (.graphml)"
        IfSilent skipFileAssociation
        !insertmacro APP_ASSOCIATE "graphml" "${PRODUCT_NAME}.graphml" "${PRODUCT_NAME} GraphML File" \
            "$INSTDIR\${EXE},0" "Open with ${PRODUCT_NAME}" \
            "$\"$INSTDIR\${EXE}$\" $\"%1$\""
        skipFileAssociation:
    SectionEnd
    Section "Biopax OWL file (.owl)"
        IfSilent skipFileAssociation
        !insertmacro APP_ASSOCIATE "owl" "${PRODUCT_NAME}.owl" "${PRODUCT_NAME} OWL File" \
            "$INSTDIR\${EXE},0" "Open with ${PRODUCT_NAME}" \
            "$\"$INSTDIR\${EXE}$\" $\"%1$\""
        skipFileAssociation:
    SectionEnd
    Section "Graphviz DOT file (.dot)"
        IfSilent skipFileAssociation
        !insertmacro APP_ASSOCIATE "dot" "${PRODUCT_NAME}.dot" "${PRODUCT_NAME} DOT File" \
            "$INSTDIR\${EXE},0" "Open with ${PRODUCT_NAME}" \
            "$\"$INSTDIR\${EXE}$\" $\"%1$\""
        skipFileAssociation:
    SectionEnd
SectionGroupEnd

Section /o "Desktop shortcut"
    IfSilent skipDesktopShortcut
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE}"
    skipDesktopShortcut:
SectionEnd

;Launch function
Function Launch
    ShellExecAsUser::ShellExecAsUser "open" "$INSTDIR\${EXE}"
FunctionEnd

Section "Uninstall"
    Delete "$INSTDIR\Uninstall.exe"
    !include "rm.nsh"

    !insertmacro MUI_STARTMENU_GETFOLDER ${PRODUCT_NAME} $STARTMENU_FOLDER
    Delete "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT_NAME}.lnk"
    RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER"

    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

    DeleteRegKey SHCTX "Software\${PRODUCT_NAME}"

    DeleteRegKey SHCTX "${UNINSTALL_KEY}"

    DeleteRegKey HKCR "${NATIVE_EXTENSION}"

    !insertmacro APP_UNASSOCIATE "${NATIVE_EXTENSION}" "${PRODUCT_NAME}.Native"
    !insertmacro APP_UNASSOCIATE "gml" "${PRODUCT_NAME}.gml"
    !insertmacro APP_UNASSOCIATE "graphml" "${PRODUCT_NAME}.graphml"
    !insertmacro APP_UNASSOCIATE "owl" "${PRODUCT_NAME}.owl"
    !insertmacro APP_UNASSOCIATE "dot" "${PRODUCT_NAME}.dot"
    !insertmacro UPDATEFILEASSOC
SectionEnd
