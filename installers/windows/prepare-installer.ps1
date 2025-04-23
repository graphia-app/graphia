# Copyright © 2013-2025 Tim Angus
# Copyright © 2013-2025 Tom Freeman
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.

$ErrorActionPreference = 'Stop'

$BUILD_DIR = "build"
$INSTALLER_DIR = "installer"

Get-Content ".\$BUILD_DIR\variables.ps1"
. ".\$BUILD_DIR\variables.ps1"

Remove-Item $INSTALLER_DIR -Recurse -Force -ErrorAction Ignore
New-Item -ItemType Directory -Path $INSTALLER_DIR | Out-Null

Copy-Item "$BUILD_DIR\${PRODUCT_NAME}.exe" $INSTALLER_DIR
Copy-Item "$BUILD_DIR\thirdparty.dll" $INSTALLER_DIR

New-Item "$INSTALLER_DIR\plugins" -ItemType Directory | Out-Null
Copy-Item "$BUILD_DIR\plugins\*.dll" "$INSTALLER_DIR\plugins" -Recurse

New-Item "$INSTALLER_DIR\examples" -ItemType Directory | Out-Null
Copy-Item "source\app\examples\*.*" "$INSTALLER_DIR\examples"

Copy-Item "$BUILD_DIR\CrashReporter.exe", "$BUILD_DIR\MessageBox.exe" $INSTALLER_DIR

$QML_DIRS = @()
Get-ChildItem -Recurse -Directory | ForEach-Object {
    if(Test-Path "$($_.FullName)\*.qml")
    {
        $QML_DIRS += "--qmldir"
        $QML_DIRS += "`"$($_.FullName)`""
    }
}

$WINDEPLOYQT_ARGS = @("--no-compiler-runtime", "--no-opengl-sw")

Write-Host "------ windeployqt"
& windeployqt @QML_DIRS @WINDEPLOYQT_ARGS --dir $INSTALLER_DIR "$INSTALLER_DIR\${PRODUCT_NAME}.exe"
& windeployqt @WINDEPLOYQT_ARGS --dir $INSTALLER_DIR "$INSTALLER_DIR\thirdparty.dll"

Get-ChildItem "$INSTALLER_DIR\plugins\*.dll" | ForEach-Object {
    & windeployqt @WINDEPLOYQT_ARGS --dir $INSTALLER_DIR $_.FullName
}

$QML_DIR = "source\crashreporter"
if(!(Test-Path $QML_DIR)) { exit 1 }
& windeployqt --qmldir $QML_DIR @WINDEPLOYQT_ARGS "$INSTALLER_DIR\CrashReporter.exe"
& windeployqt @WINDEPLOYQT_ARGS "$INSTALLER_DIR\MessageBox.exe"

Write-Host "------ copying runtime + extras"
Copy-Item "${Env:CRTDIRECTORY}\*.*" $INSTALLER_DIR -Recurse -Verbose
Copy-Item "${Env:WindowsSdkDir}\redist\ucrt\DLLs\x64\*.*" $INSTALLER_DIR -Recurse -Verbose
Copy-Item "misc\windows-extras\*.*" $INSTALLER_DIR -Recurse -Verbose

Write-Host "------ copying Updater"
$UPDATER_DIR = "$INSTALLER_DIR\Updater"
New-Item -ItemType Directory -Path $UPDATER_DIR | Out-Null
Copy-Item "$BUILD_DIR\Updater.exe", "$BUILD_DIR\thirdparty.dll" $UPDATER_DIR -Verbose

$QML_DIR = "source\updater"
if(!(Test-Path $QML_DIR)) { exit 1 }
& windeployqt --qmldir $QML_DIR @WINDEPLOYQT_ARGS "$UPDATER_DIR\Updater.exe"
& windeployqt @WINDEPLOYQT_ARGS --dir $UPDATER_DIR "$UPDATER_DIR\thirdparty.dll"

Copy-Item "${Env:CRTDIRECTORY}\*.*", "${Env:WindowsSdkDir}\redist\ucrt\DLLs\x64\*.*" $UPDATER_DIR -Recurse -Verbose

Get-ChildItem -Path $UPDATER_DIR -File -Recurse | ForEach-Object {
    $relPath = $_.FullName.Substring((Resolve-Path "$UPDATER_DIR").Path.Length + 1)
    Add-Content "$INSTALLER_DIR\Updater.deps" $relPath
}

Move-Item "$UPDATER_DIR\*.*" $INSTALLER_DIR -Verbose -Force
Remove-Item $UPDATER_DIR -Recurse -Force

$RM_NSH = "installers\windows\rm.nsh"
Set-Content $RM_NSH ""

Get-ChildItem $INSTALLER_DIR -Directory | ForEach-Object {
    Add-Content $RM_NSH "RMDir /r `"`$INSTDIR\$($_.Name)`""
}

Get-ChildItem $INSTALLER_DIR -File | ForEach-Object {
    Add-Content $RM_NSH "Delete `"`$INSTDIR\$($_.Name)`""
}

# Not recursive to avoid catastrophe when a user installs into a non-empty directory
Add-Content $RM_NSH 'RMDir "$INSTDIR"'

Write-Host "------ contents of ${RM_NSH}:"
Get-Content $RM_NSH

$BUILD_SIZE = 0
Get-ChildItem "$INSTALLER_DIR" -Recurse -File | ForEach-Object {
    $BUILD_SIZE += [math]::Floor($_.Length / 1024)
}

$MAKENSIS_ARGS = @(
    "/NOCD",
    "/DPRODUCT_NAME=${PRODUCT_NAME}",
    "/DVERSION=${VERSION}",
    "/DPUBLISHER=${PUBLISHER}",
    "/DNATIVE_EXTENSION=${NATIVE_EXTENSION}",
    "/DCOPYRIGHT=${COPYRIGHT}",
    "/DBUILD_SIZE=$BUILD_SIZE"
)

$MAKENSIS_ARGS | Set-Content "$BUILD_DIR\makensis_args.txt"

$UNINSTALLER_INSTALLER = "uninstaller-installer.exe"

Write-Host "------ making uninstaller-installer"
& makensis @MAKENSIS_ARGS "/XOutFile $UNINSTALLER_INSTALLER" "installers\windows\installer.nsi"

$UNINSTALLER_INSTALL_DIR = "$PWD\uninstaller"
Write-Host "------ running uninstaller-installer"
& ".\$UNINSTALLER_INSTALLER" /S /D=$UNINSTALLER_INSTALL_DIR | Out-Default

$UNSIGNED_UNINSTALLER_EXE = "$UNINSTALLER_INSTALL_DIR\Uninstall.exe"
Copy-Item $UNSIGNED_UNINSTALLER_EXE "$INSTALLER_DIR\Uninstall.exe" -Verbose

Write-Host "------ uninstalling uninstaller-installer"
& "$UNSIGNED_UNINSTALLER_EXE" /S
Remove-Item $UNINSTALLER_INSTALLER -Force
