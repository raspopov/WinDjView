@echo off
setlocal
set PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%
set INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\include;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
set LIB=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\lib;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib;%LIB%
set MAKETARGET=all
if "%1"=="r" set MAKETARGET=rebuild
echo Building Release configuration...
nmake /nologo %MAKETARGET%
if errorlevel 1 goto end
echo Building Debug configuration...
nmake /nologo %MAKETARGET% "DEBUG=1"
if errorlevel 1 goto end
:end
endlocal
