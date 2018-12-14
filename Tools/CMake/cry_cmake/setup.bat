@echo off

REM Tested with Python35
REM Assumes Python 3.5 installed in c:\Python35

REM PyInstaller must be installed before
REM To Install PyInstaller run:
REM  c:\Python35\Scripts\pip.exe install PyInstaller

SET sdir=%~dp0

REM Build CrySelect
echo PyInstalling EXE.....
echo.
c:\Python35\Scripts\PyInstaller.exe --distpath=%sdir%dist --workpath=%sdir%build %sdir%cry_cmake.spec &&^
echo. && echo Copying cry_cmake.exe to ENGINE_ROOT..... &&^
copy /Y %sdir%dist\cry_cmake.exe %sdir%..\..\..\cry_cmake.exe

REM Pause unless there's any arguments.
IF "%1" == "" ( echo. && PAUSE )