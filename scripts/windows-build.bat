:: Copyright © 2013-2024 Graphia Technologies Ltd.
::
:: This file is part of Graphia.
::
:: Graphia is free software: you can redistribute it and/or modify
:: it under the terms of the GNU General Public License as published by
:: the Free Software Foundation, either version 3 of the License, or
:: (at your option) any later version.
::
:: Graphia is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with Graphia.  If not, see <http://www.gnu.org/licenses/>.

@echo off

call scripts\defaults.bat

set BUILD_DIR=build
IF EXIST %BUILD_DIR% rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

cd %BUILD_DIR%
cmake --version || EXIT /B 1
cmake -DCMAKE_UNITY_BUILD=%UNITY_BUILD% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -GNinja ^
    -DCMAKE_C_COMPILER=%CC% -DCMAKE_CXX_COMPILER=%CXX% ^
    .. || EXIT /B 1
type variables.bat
call variables.bat
cmake --build . --target all | tee compiler-%VERSION%.log || EXIT /B 1

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
