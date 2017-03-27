call scripts\defaults.bat

rmdir /s /q build
mkdir build
mkdir build\plugins

call C:\Jenkins\environment.bat
jom distclean || echo "distclean failed"
qmake -version || EXIT /B 1
qmake GraphTool.pro || EXIT /B 1
jom clean || EXIT /B 1
jom || EXIT /B 1

call installers\windows\build.bat || EXIT /B 1

setlocal EnableDelayedExpansion

source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%PRODUCT_NAME%.exe > %PRODUCT_NAME%.sym

FOR %%f IN (plugins\*.dll) DO (
	source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
		%%f > %%~df%%~pf%%~nf.sym
)
copy %PRODUCT_NAME%.sym build\
xcopy plugins\*.sym build\plugins\
