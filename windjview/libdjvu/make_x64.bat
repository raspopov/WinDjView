@echo off
setlocal
set PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\amd64;C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%
set INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\include;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
set LIB=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib\amd64;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\lib\amd64;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64;%LIB%
set MAKETARGET=all
if "%1"=="r" set MAKETARGET=rebuild
echo Building X64 Release configuration...
nmake /nologo %MAKETARGET% "X64=1"
if errorlevel 1 goto end
echo Building X64 Debug configuration...
nmake /nologo %MAKETARGET% "X64=1" "DEBUG=1"
if errorlevel 1 goto end
:end
endlocal
