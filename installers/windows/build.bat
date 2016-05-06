copy %PRODUCT_NAME%.exe build\
copy CrashReporter.exe build\

set QML_DIR=source\app\ui\qml
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% --no-angle --no-compiler-runtime ^
	--no-opengl-sw build\%PRODUCT_NAME%.exe || EXIT /B 1

set QML_DIR=source\crashreporter
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% --no-angle --no-compiler-runtime ^
	--no-opengl-sw build\CrashReporter.exe || EXIT /B 1

xcopy "%CRTDIRECTORY%*.*" build || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" build || EXIT /B 1

signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% ^
	/tr %SIGN_TSA% /td SHA256 build\%PRODUCT_NAME%.exe || EXIT /B 1

@echo off
setLocal EnableDelayedExpansion
set /a value=0
set /a BUILD_SIZE=0
FOR /R %1 %%I IN (build\*) DO (
set /a value=%%~zI/1024
set /a BUILD_SIZE=!BUILD_SIZE!+!value!
)
@echo on

makensis /NOCD /DPRODUCT_NAME=%PRODUCT_NAME% /DVERSION=%VERSION% ^
	/DPUBLISHER="%PUBLISHER%" /DCOPYRIGHT="%COPYRIGHT%" /DBUILD_SIZE=%BUILD_SIZE% ^
	"/XOutFile build\%PRODUCT_NAME%-%VERSION%-installer.exe" ^
	installers\windows\installer.nsi

signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% ^
	/tr %SIGN_TSA% /td SHA256 ^
	build\%PRODUCT_NAME%-%VERSION%-installer.exe || EXIT /B 1
