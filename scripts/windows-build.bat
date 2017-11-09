call scripts\defaults.bat

set BUILD_DIR=build
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

cd %BUILD_DIR%
cmake --version || EXIT /B 1
cmake -DCMAKE_BUILD_TYPE=Release -GNinja .. || EXIT /B 1
cmake --build . --target all || EXIT /B 1

REM Clean intermediate build products
FOR /f "tokens=2" %%R IN ('findstr "_COMPILER_ _STATIC_LIBRARY_" rules.ninja') DO (
  ninja -t clean -r %%~R
)

cd ..

setlocal EnableDelayedExpansion

source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%BUILD_DIR%\%PRODUCT_NAME%.exe > %BUILD_DIR%\%PRODUCT_NAME%.sym || EXIT /B 1

FOR %%f IN (%BUILD_DIR%\plugins\*.dll) DO (
	source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
		%%f > %%~df%%~pf%%~nf.sym || EXIT /B 1
)
