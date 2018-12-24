@echo off

REM Tested with Python35
REM Assumes Python 3.5 installed in c:\Python35
REM Also requires the following: pip install pypiwin32 

REM PyInstaller must be installed before
REM To Install PyInstaller run:
REM  c:\Python35\Scripts\pip.exe install PyInstaller

SET sdir=%~dp0

REM Build CrySelect
echo PyInstalling CrySelect EXE.....
echo.
c:\Python35\Scripts\PyInstaller.exe --distpath=%sdir%dist --workpath=%sdir%build %sdir%cryselect.spec &&^
echo. && echo Copying cryselect.exe..... &&^
copy /Y %sdir%dist\cryselect.exe %sdir%cryselect.exe

REM Pause unless there's any arguments.
IF "%1" == "" ( echo. && PAUSE )

REM Build CryRun
echo PyInstalling CryRun EXE.....
echo.
c:\Python35\Scripts\PyInstaller.exe --distpath=%sdir%dist --workpath=%sdir%build %sdir%cryrun.spec &&^
echo. && echo Copying cryrun.exe..... &&^
copy /Y %sdir%dist\cryrun.exe %sdir%cryrun.exe

REM Pause unless there's any arguments.
IF "%1" == "" ( echo. && PAUSE )

REM Installing new CrySelect
echo Installing new CrySelect.
%sdir%\cryselect.exe install

REM Pause unless there's any arguments.
IF "%1" == "" ( echo Installed successfully && PAUSE )