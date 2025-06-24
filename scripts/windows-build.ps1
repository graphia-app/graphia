# Copyright © 2013-2025 Tim Angus
# Copyright © 2013-2025 Tom Freeman
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.

$ErrorActionPreference = 'Stop'

. .\scripts\defaults.ps1

$BUILD_DIR = "build"

if(Test-Path $BUILD_DIR)
{
    Remove-Item -Recurse -Force $BUILD_DIR
}
New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null

Push-Location $BUILD_DIR

cmake --version

cmake -DCMAKE_UNITY_BUILD="$Env:UNITY_BUILD" `
    -DCMAKE_BUILD_TYPE="$Env:BUILD_TYPE" `
    -GNinja `
    -DCMAKE_C_COMPILER="$Env:CC" `
    -DCMAKE_CXX_COMPILER="$Env:CXX" `
    ..

Get-Content .\variables.ps1
. .\variables.ps1

cmake --build . --target all 2>&1 | Tee-Object "compiler-${VERSION}.log"

Pop-Location

# VS2022 needs these registered manually for some reason
try
{
    Write-Host "Registering 32-bit msdia140.dll..."
    & regsvr32 /s "C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\msdia140.dll"

    Write-Host "Registering 64-bit msdia140.dll..."
    & regsvr32 /s "C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll"
}
catch
{
    Write-Error "Registration failed: $_"
}

$dumpSyms = "source\thirdparty\breakpad\src\tools\windows\binaries\dump_syms.exe"

& $dumpSyms "$BUILD_DIR\${PRODUCT_NAME}.exe" > "$BUILD_DIR\${PRODUCT_NAME}.sym"
& $dumpSyms "$BUILD_DIR\thirdparty.dll" > "$BUILD_DIR\thirdparty.dll.sym"

Get-ChildItem "$BUILD_DIR\plugins\*.dll" | ForEach-Object {
    $dllPath = $_.FullName
    $symPath = "$($_.DirectoryName)\$($_.BaseName).sym"
    & $dumpSyms $dllPath > $symPath
}
