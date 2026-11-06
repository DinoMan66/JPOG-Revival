@echo off
setlocal enableextensions
rem ======================================================================
rem  Exports Ghidra's decompiled C for EVERY game DLL (and SimJP.exe) in one go.
rem  Run this on your Windows PC (not in the XP VM): double-click it and leave it running.
rem  Output: decompiled\<file>.c   - Claude reads these directly from the project folder.
rem  Uses the Ghidra folder saved in Tools\ghidra_path.txt by export_decompiled.bat.
rem  Files that already have an export are skipped, so it is safe to re-run.
rem ======================================================================
pushd "%~dp0.."
set ROOT=%CD%
set GHIDRA=
if exist "%ROOT%\Tools\ghidra_path.txt" set /p GHIDRA=<"%ROOT%\Tools\ghidra_path.txt"
if not defined GHIDRA set GHIDRA=C:\OpenSourceProjects\ghidra_12.1.4_PUBLIC
if not exist "%GHIDRA%\support\analyzeHeadless.bat" (
  echo ERROR: Ghidra not found at %GHIDRA%
  pause
  exit /b 1
)
if not exist "%ROOT%\decompiled" mkdir "%ROOT%\decompiled"
if not exist "%ROOT%\ghidra_project" mkdir "%ROOT%\ghidra_project"

for %%F in ("%ROOT%\Original\DLLS\*.dll" "%ROOT%\Original\DLLS\D3D\*.dll" "%ROOT%\Original\SimJP.exe") do call :one "%%~fF"
echo.
echo All done. Exports are in %ROOT%\decompiled
> "%ROOT%\decompiled\EXPORT_DONE.txt" echo done
pause
exit /b 0

:one
set NAME=%~nx1
if /i "%NAME:~0,6%"=="binkw3" goto :eof
if /i "%NAME:~0,4%"=="fmod" goto :eof
if /i "%NAME:~0,6%"=="libmmd" goto :eof
if exist "%ROOT%\decompiled\%NAME%.c" (
  echo skip %NAME% ^(already exported^)
  goto :eof
)
echo.
echo ===== %NAME% =====
call "%GHIDRA%\support\analyzeHeadless.bat" "%ROOT%\ghidra_project" JPOG -import %1 -overwrite -scriptPath "%ROOT%\Tools\ghidra" -postScript ExportDecompiled.java "%ROOT%\decompiled" > "%ROOT%\decompiled\%NAME%.log" 2>&1
if exist "%ROOT%\decompiled\%NAME%.c" (echo   OK) else (echo   FAILED - see decompiled\%NAME%.log)
goto :eof
