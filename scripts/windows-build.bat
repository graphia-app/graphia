call scripts\defaults.bat

set BUILD_DIR=build
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

cd %BUILD_DIR%
qmake -version || EXIT /B 1
qmake ..\GraphTool.pro || EXIT /B 1
jom || EXIT /B 1
jom clean || EXIT /B 1
cd ..

setlocal EnableDelayedExpansion

source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%BUILD_DIR%\%PRODUCT_NAME%.exe > %BUILD_DIR%\%PRODUCT_NAME%.sym

FOR %%f IN (%BUILD_DIR%\plugins\*.dll) DO (
	source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
		%%f > %%~df%%~pf%%~nf.sym
)
