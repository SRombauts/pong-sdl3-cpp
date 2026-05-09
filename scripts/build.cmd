@echo off
rem Thin wrapper that forwards all arguments to scripts\build.ps1.
rem Lets users invoke the build from cmd.exe without remembering the
rem PowerShell execution policy flags.

setlocal
set "SCRIPT_DIR=%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%build.ps1" %*
exit /b %ERRORLEVEL%
