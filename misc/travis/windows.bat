@echo off
setlocal EnableDelayedExpansion

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set PATH=%PATH%;C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin;C:\Qt\Qt5.14.2\Tools\QtCreator\bin;C:\Program Files (x86)\NSIS\Bin
set CRTDIRECTORY=%VcToolsRedistDir%\x64\Microsoft.VC142.CRT

call "scripts\windows-build.bat"
call "installers\windows\build.bat"
