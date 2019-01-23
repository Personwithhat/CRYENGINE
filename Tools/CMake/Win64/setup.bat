@echo off

REM Assumes QT 5.12.0 installed in C:\Qt\5.12.0
REM Get it from https://www.qt.io/download it's ~50GB

SET sdir=%~dp0

REM Just copies redistributable installer, not really needed.
REM 	set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC

REM For CMake-GUI - Assumes binaries copied already & MSVStudio2017
echo Adding req's to run gui as a standalone
echo.
C:\Qt\5.12.0\msvc2017_64\bin\windeployqt.exe --release %sdir%bin/cmake-gui.exe

REM Pause unless there's any arguments.
IF "%1" == "" ( echo. && PAUSE )