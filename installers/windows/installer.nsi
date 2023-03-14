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

Var INSTDIR_BASE
Var ACCOUNT_TYPE

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

!macro ConsoleLog message
    System::Call 'kernel32::AttachConsole(i -1)i.r0' ;attach to parent console
    System::Call 'kernel32::GetStdHandle(i -11)i.r0' ;console attached -- get stdout
    FileWrite $0 "${message}$\r$\n"
!macroend

!macro CheckIfStillRunning
    ExecCmd::exec "%SystemRoot%\System32\tasklist /NH /FI \
        $\"IMAGENAME eq ${EXE}$\" | \
        %SystemRoot%\System32\find /I $\"${EXE}$\""
    Pop $0 ; The handle for the process
    ExecCmd::wait $0
    Pop $0
!macroend

!macro ONINIT un
    Function ${un}.onInit
        !insertmacro CheckIfStillRunning
        IfSilent 0 notSilent
            IntCmp $0 1 notRunning
                !insertmacro ConsoleLog "Waiting for ${PRODUCT_NAME} to close..."
                Sleep 10000 ; In silent mode, give the app a few seconds to close then...
                !insertmacro CheckIfStillRunning ; ...check again

        notSilent:
        IntCmp $0 1 notRunning
            MessageBox MB_OK|MB_ICONEXCLAMATION \
                "${PRODUCT_NAME} is still running. Please close it before making changes." /SD IDOK
            !insertmacro ConsoleLog "${PRODUCT_NAME} is still running. Please close it before making changes."
            Abort
        notRunning:

        UserInfo::GetAccountType
        Pop $ACCOUNT_TYPE
        !insertmacro ConsoleLog "Account type: $ACCOUNT_TYPE"

        ${If} $INSTDIR == ""
            ; This only happens in the installer, because the uninstaller already knows INSTDIR

            ; The value of SetShellVarContext detetmines whether SHCTX is HKLM or HKCU
            ; and whether SMPROGRAMS refers to all users or just the current user
            SetShellVarContext all

            ReadRegStr $0 SHCTX "Software\${PRODUCT_NAME}" ""
            ${If} $0 != ""
                ; We're already installed as admin, so use this context even if we're
                ; just a user, otherwise we'll end up confusingly installing two copies
                StrCpy $INSTDIR "$0"

                ${If} $ACCOUNT_TYPE != "Admin"
                    MessageBox MB_OK|MB_ICONEXCLAMATION \
                        "${PRODUCT_NAME} cannot be updated as its installation directory is not writable. Please contact an administrator." /SD IDOK
                    !insertmacro ConsoleLog "${PRODUCT_NAME} cannot be updated as its installation directory is not writable."
                    Abort
                ${EndIf}
            ${Else}
                ${If} $ACCOUNT_TYPE == "Admin"
                    ; If we're an admin, default to installing to C:\Program Files
                    SetShellVarContext all
                    StrCpy $INSTDIR_BASE "$PROGRAMFILES64"
                ${Else}
                    ; If we're just a user, default to installing to ~\AppData\Local
                    SetShellVarContext current
                    StrCpy $INSTDIR_BASE "$LOCALAPPDATA"
                ${EndIf}
            ${EndIf}

            ReadRegStr $0 SHCTX "Software\${PRODUCT_NAME}" ""
            ${If} $0 != ""
                ; If we're already installed, use the existing directory
                StrCpy $INSTDIR "$0"
            ${Else}
                StrCpy $INSTDIR "$INSTDIR_BASE\${PRODUCT_NAME}"
            ${Endif}
        ${ElseIf} "${un}" == "un"
            ${If} $ACCOUNT_TYPE == "Admin"
                SetShellVarContext all
            ${Else}
                SetShellVarContext current
            ${EndIf}
        ${Else}
            ; When INSTDIR is given on the command line, and we're not making the uninstaller
            SetShellVarContext current
        ${EndIf}
    FunctionEnd
!macroend

; Define the function twice, once for the installer and again for the uninstaller
!insertmacro ONINIT ""
!insertmacro ONINIT "un"

!macro REMOVE_OLD_INSTALLATION
    RMDir /r "$INSTDIR\bearer"
    RMDir /r "$INSTDIR\examples"
    RMDir /r "$INSTDIR\iconengines"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\networkinformation"
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\position"
    RMDir /r "$INSTDIR\printsupport"
    RMDir /r "$INSTDIR\qmltooling"
    RMDir /r "$INSTDIR\Qt"
    RMDir /r "$INSTDIR\QtGraphicalEffects"
    RMDir /r "$INSTDIR\QtQml"
    RMDir /r "$INSTDIR\QtQuick"
    RMDir /r "$INSTDIR\QtQuick.2"
    RMDir /r "$INSTDIR\QtTest"
    RMDir /r "$INSTDIR\QtWebChannel"
    RMDir /r "$INSTDIR\QtWebEngine"
    RMDir /r "$INSTDIR\resources"
    RMDir /r "$INSTDIR\scenegraph"
    RMDir /r "$INSTDIR\sqldrivers"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\tls"
    RMDir /r "$INSTDIR\translations"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\*.exe"
    RMDir /r "$INSTDIR\plugins"
    Delete "$INSTDIR\Updater.deps"

    ; Not recursive, in case the user chose to install it to a non-empty directory
    RMDir "$INSTDIR"
!macroend

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

    ; Clear out the target directory first
    !insertmacro REMOVE_OLD_INSTALLATION

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
    !insertmacro REMOVE_OLD_INSTALLATION

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
