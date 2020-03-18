setLocal EnableDelayedExpansion

set BUILD_DIR=build
set INSTALLER_DIR=installer

call %BUILD_DIR%\variables.bat

rmdir /s /q %INSTALLER_DIR%
mkdir %INSTALLER_DIR%
copy %BUILD_DIR%\%PRODUCT_NAME%.exe %INSTALLER_DIR%\
copy %BUILD_DIR%\thirdparty.dll %INSTALLER_DIR%\

mkdir %INSTALLER_DIR%\plugins
xcopy "%BUILD_DIR%\plugins\*.dll" %INSTALLER_DIR%\plugins

mkdir %INSTALLER_DIR%\examples
xcopy "source\app\examples\*.*" %INSTALLER_DIR%\examples

copy %BUILD_DIR%\CrashReporter.exe %INSTALLER_DIR%\
copy %BUILD_DIR%\MessageBox.exe %INSTALLER_DIR%\

set QML_DIRS=
FOR /d /r %%i IN (*) DO @IF EXIST %%i\*.qml (set QML_DIRS=--qmldir %%i !QML_DIRS!)

set WINDEPLOYQT_ARGS=--no-angle --no-compiler-runtime --no-opengl-sw

echo ------ windeployqt
windeployqt %QML_DIRS% %WINDEPLOYQT_ARGS% ^
  --dir %INSTALLER_DIR% %INSTALLER_DIR%\%PRODUCT_NAME%.exe || EXIT /B 1
windeployqt %WINDEPLOYQT_ARGS% --dir %INSTALLER_DIR% %INSTALLER_DIR%\thirdparty.dll

FOR %%i IN (%INSTALLER_DIR%\plugins\*.dll) DO ^
  windeployqt %WINDEPLOYQT_ARGS% --dir %INSTALLER_DIR% %%i

set QML_DIR=source\crashreporter
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% %WINDEPLOYQT_ARGS% ^
  %INSTALLER_DIR%\CrashReporter.exe || EXIT /B 1

windeployqt %WINDEPLOYQT_ARGS% %INSTALLER_DIR%\MessageBox.exe || EXIT /B 1

echo ------ copying runtime + extras
xcopy "%CRTDIRECTORY%*.*" %INSTALLER_DIR% || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" %INSTALLER_DIR% || EXIT /B 1

IF EXIST %WINDOWS_EXTRA_FILES%\NUL (
  xcopy "%WINDOWS_EXTRA_FILES%*.*" %INSTALLER_DIR% || EXIT /B 1
)

echo ------ copying Updater
set UPDATER_DIR=%INSTALLER_DIR%\Updater
mkdir %UPDATER_DIR%
copy %BUILD_DIR%\Updater.exe %UPDATER_DIR%\
copy %BUILD_DIR%\thirdparty.dll %UPDATER_DIR%\

set QML_DIR=source\updater
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% %WINDEPLOYQT_ARGS% %UPDATER_DIR%\Updater.exe || EXIT /B 1
windeployqt %WINDEPLOYQT_ARGS% --dir %UPDATER_DIR% %UPDATER_DIR%\thirdparty.dll

xcopy "%CRTDIRECTORY%*.*" %UPDATER_DIR% || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" %UPDATER_DIR% || EXIT /B 1

FOR /f "delims=" %%i IN ('dir /S /B /A:-D %UPDATER_DIR%') DO (
  SET "filename=%%i"
  ECHO(!filename:%cd%\%UPDATER_DIR%\=!
) >> %INSTALLER_DIR%\Updater.deps

move /Y %UPDATER_DIR%\*.* %INSTALLER_DIR%
rmdir /s /q %UPDATER_DIR%

set SIGNTOOL_ARGS=sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% /tr %SIGN_TSA% /td SHA256

echo ------ signing executables
FOR %%i IN (%PRODUCT_NAME% CrashReporter MessageBox Updater) DO (
  signtool %SIGNTOOL_ARGS% %INSTALLER_DIR%\%%i.exe || EXIT /B 1
)

set /a value=0
set /a BUILD_SIZE=0
FOR /R %1 %%I IN (%INSTALLER_DIR%\*) DO (
set /a value=%%~zI/1024
set /a BUILD_SIZE=!BUILD_SIZE!+!value!
)

set MAKENSIS_ARGS=/NOCD /DPRODUCT_NAME=%PRODUCT_NAME% ^
  /DVERSION=%VERSION% /DPUBLISHER="%PUBLISHER%" ^
  /DNATIVE_EXTENSION=%NATIVE_EXTENSION% ^
  /DCOPYRIGHT="%COPYRIGHT%" ^
  /DBUILD_SIZE=%BUILD_SIZE%

echo ------ making uninstaller-installer
set UNINSTALLER_INSTALLER=uninstaller-installer.exe
makensis %MAKENSIS_ARGS% "/XOutFile %UNINSTALLER_INSTALLER%" ^
  installers\windows\installer.nsi || EXIT /B 1

set UNINSTALLER_INSTALL_DIR=%CD%\uninstaller
echo ------ running uninstaller-installer
%UNINSTALLER_INSTALLER% /S /D=%UNINSTALLER_INSTALL_DIR% || EXIT /B 1
set UNSIGNED_UNINSTALLER_EXE=%UNINSTALLER_INSTALL_DIR%\Uninstall.exe
echo ------ signing uninstaller
copy %UNSIGNED_UNINSTALLER_EXE% %INSTALLER_DIR%\Uninstall.exe
signtool %SIGNTOOL_ARGS% %INSTALLER_DIR%\Uninstall.exe || EXIT /B 1
echo ------ uninstalling uninstaller-installer
%UNSIGNED_UNINSTALLER_EXE% /S
del /F /Q %UNINSTALLER_INSTALLER%

echo ------ making final installer
set INSTALLER_EXE=%INSTALLER_DIR%\%PRODUCT_NAME%-%VERSION%-installer.exe
makensis %MAKENSIS_ARGS% "/XOutFile %INSTALLER_EXE%" ^
  installers\windows\installer.nsi || EXIT /B 1
signtool %SIGNTOOL_ARGS% %INSTALLER_EXE% || EXIT /B 1
copy %INSTALLER_EXE% %BUILD_DIR%\
rmdir /s /q %INSTALLER_DIR%
