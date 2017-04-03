call scripts\defaults.bat

set BUILD_DIR=build
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

jom distclean || echo "distclean failed"

cd %BUILD_DIR%
qmake -version || EXIT /B 1
qmake ..\GraphTool.pro || EXIT /B 1
jom || EXIT /B 1
jom clean || EXIT /B 1

cd ..
call installers\windows\build.bat || EXIT /B 1
cd %BUILD_DIR%

setlocal EnableDelayedExpansion

..\source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%PRODUCT_NAME%.exe > %PRODUCT_NAME%.sym

FOR %%f IN (plugins\*.dll) DO (
	..\source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
		%%f > %%~df%%~pf%%~nf.sym
)
