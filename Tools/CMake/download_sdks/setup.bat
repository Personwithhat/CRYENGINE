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
c:\Python35\Scripts\PyInstaller.exe --distpath=%sdir%dist --workpath=%sdir%build %sdir%download_sdks.spec &&^
echo. && echo Copying download_sdks.exe..... &&^
copy /Y %sdir%dist\download_sdks.exe %sdir%download_sdks.exe

REM Pause unless there's any arguments.
IF "%1" == "" ( echo. && PAUSE )