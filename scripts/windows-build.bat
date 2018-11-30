call scripts\defaults.bat

REM Use clcache (https://github.com/frerich/clcache) if available
WHERE clcache >nul 2>&1 && (
  SET CC=clcache
  SET CXX=clcache
)

set BUILD_DIR=build
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

cd %BUILD_DIR%
cmake --version || EXIT /B 1
cmake -DUNITY_BUILD=ON -DCMAKE_BUILD_TYPE=Release -GNinja .. || EXIT /B 1
cmake --build . --target all || EXIT /B 1

REM Clean intermediate build products
FOR /f "tokens=2" %%R IN ('findstr "_COMPILER_ _STATIC_LIBRARY_" rules.ninja') DO (
  ninja -t clean -r %%~R
)

cd ..

setlocal EnableDelayedExpansion

source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%BUILD_DIR%\%PRODUCT_NAME%.exe > %BUILD_DIR%\%PRODUCT_NAME%.sym || EXIT /B 1
source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
	%BUILD_DIR%\thirdparty.dll > %BUILD_DIR%\thirdparty.dll.sym || EXIT /B 1

FOR %%f IN (%BUILD_DIR%\plugins\*.dll) DO (
	source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe ^
		%%f > %%~df%%~pf%%~nf.sym || EXIT /B 1
)
