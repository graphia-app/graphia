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
for /d /r %%i in (*) do @if exist %%i\*.qml (set QML_DIRS=--qmldir %%i !QML_DIRS!)
echo %QML_DIRS%

windeployqt %QML_DIRS% --no-angle --no-compiler-runtime ^
	--no-opengl-sw --dir %INSTALLER_DIR% %INSTALLER_DIR%\%PRODUCT_NAME%.exe || EXIT /B 1
windeployqt --no-angle --no-compiler-runtime --no-opengl-sw ^
  --dir %INSTALLER_DIR% %INSTALLER_DIR%\thirdparty.dll

for %%i in (%INSTALLER_DIR%\plugins\*.dll) do windeployqt --no-angle --no-compiler-runtime ^
	--no-opengl-sw --dir %INSTALLER_DIR% %%i

set QML_DIR=source\crashreporter
IF NOT EXIST %QML_DIR%\NUL EXIT /B 1
windeployqt --qmldir %QML_DIR% --no-angle --no-compiler-runtime ^
	--no-opengl-sw %INSTALLER_DIR%\CrashReporter.exe || EXIT /B 1
windeployqt --no-angle --no-compiler-runtime ^
	--no-opengl-sw %INSTALLER_DIR%\MessageBox.exe || EXIT /B 1

xcopy "%CRTDIRECTORY%*.*" %INSTALLER_DIR% || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" %INSTALLER_DIR% || EXIT /B 1

IF EXIST %WINDOWS_EXTRA_FILES%\NUL (
  xcopy "%WINDOWS_EXTRA_FILES%*.*" %INSTALLER_DIR% || EXIT /B 1
)

signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% ^
	/tr %SIGN_TSA% /td SHA256 %INSTALLER_DIR%\%PRODUCT_NAME%.exe || EXIT /B 1

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
