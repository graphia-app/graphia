setLocal EnableDelayedExpansion

set BUILD_DIR=build
set INSTALLER_DIR=installer

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
echo %QML_DIRS%

set WINDEPLOYQT_ARGS=--no-angle --no-compiler-runtime --no-opengl-sw

windeployqt %QML_DIRS% %WINDEPLOYQT_ARGS% ^
	--dir %INSTALLER_DIR% %INSTALLER_DIR%\%PRODUCT_NAME%.exe || EXIT /B 1
windeployqt %WINDEPLOYQT_ARGS% --dir %INSTALLER_DIR% %INSTALLER_DIR%\thirdparty.dll

FOR %%i IN (%INSTALLER_DIR%\plugins\*.dll) DO ^
  windeployqt %WINDEPLOYQT_ARGS% --dir %INSTALLER_DIR% %%i

set QML_DIR=source\crashreporter
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% %WINDEPLOYQT_ARGS% %INSTALLER_DIR%\CrashReporter.exe || EXIT /B 1

windeployqt %WINDEPLOYQT_ARGS% %INSTALLER_DIR%\MessageBox.exe || EXIT /B 1

xcopy "%CRTDIRECTORY%*.*" %INSTALLER_DIR% || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" %INSTALLER_DIR% || EXIT /B 1

IF EXIST %WINDOWS_EXTRA_FILES%\NUL (
  xcopy "%WINDOWS_EXTRA_FILES%*.*" %INSTALLER_DIR% || EXIT /B 1
)

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

FOR %%i IN (%PRODUCT_NAME% CrashReporter MessageBox Updater) DO (
  signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% ^
	  /tr %SIGN_TSA% /td SHA256 %INSTALLER_DIR%\%%i.exe || EXIT /B 1
)

set /a value=0
set /a BUILD_SIZE=0
FOR /R %1 %%I IN (%INSTALLER_DIR%\*) DO (
set /a value=%%~zI/1024
set /a BUILD_SIZE=!BUILD_SIZE!+!value!
)

makensis /NOCD /DPRODUCT_NAME=%PRODUCT_NAME% /DVERSION=%VERSION% /DPUBLISHER="%PUBLISHER%" ^
	/DNATIVE_EXTENSION=%NATIVE_EXTENSION% /DCOPYRIGHT="%COPYRIGHT%" /DBUILD_SIZE=%BUILD_SIZE% ^
	"/XOutFile %INSTALLER_DIR%\%PRODUCT_NAME%-%VERSION%-installer.exe" ^
	installers\windows\installer.nsi

signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% ^
	/tr %SIGN_TSA% /td SHA256 ^
	%INSTALLER_DIR%\%PRODUCT_NAME%-%VERSION%-installer.exe || EXIT /B 1

copy %INSTALLER_DIR%\%PRODUCT_NAME%-%VERSION%-installer.exe %BUILD_DIR%\

rmdir /s /q %INSTALLER_DIR%
