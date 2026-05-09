@echo off
rem Thin wrapper that forwards all arguments to scripts\test.ps1.
rem Lets users invoke the test runner from cmd.exe without remembering
rem the PowerShell execution policy flags.

setlocal
set "SCRIPT_DIR=%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%test.ps1" %*
exit /b %ERRORLEVEL%
