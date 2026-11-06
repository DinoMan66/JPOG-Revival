@echo off
setlocal
rem ======================================================================
rem  Exports the decompiled C of a binary using Ghidra in HEADLESS mode.
rem
rem  Usage:   tools\export_decompiled.bat  original\DLLS\SomeFile.dll
rem           (or run it with no arguments / drag a DLL onto it, and it will ask)
rem  Output:  decompiled\SomeFile.dll.c   (in the project root)
rem
rem  The first time, it asks where Ghidra is installed (the folder that contains
rem  ghidraRun.bat and a folder called "support") and remembers the answer in
rem  tools\ghidra_path.txt. Delete that file to be asked again.
rem  NOTE: not yet tested by Claude - if anything fails, send me the whole window text.
rem ======================================================================

set TARGET=%~1
set TOOLS=%~dp0
cd /d "%~dp0.."
set ROOT=%CD%
set PROJ=%ROOT%\ghidra_project
set OUT=%ROOT%\decompiled

if defined TARGET goto havetarget
echo.
echo Which file do you want to export? Type or paste its path, relative to
echo %ROOT%   or a full path.
echo (Example:  original\DLLS\TInputInterface_inst.dll   - no quote marks needed)
echo (Tip: you can also drag a DLL onto this batch file in File Explorer.)
set /p TARGET=File: 
set TARGET=%TARGET:"=%
if not defined TARGET goto usage
:havetarget
if not exist "%TARGET%" goto nofile
for %%F in ("%TARGET%") do set TARGET=%%~fF

set GHIDRA=
if exist "%TOOLS%ghidra_path.txt" set /p GHIDRA=<"%TOOLS%ghidra_path.txt"
if defined GHIDRA if exist "%GHIDRA%\support\analyzeHeadless.bat" goto found

echo.
echo Where is Ghidra installed? Paste the full path of the folder that contains
echo ghidraRun.bat and the "support" folder, then press Enter.
echo (Example:  C:\Tools\ghidra_11.3.2_PUBLIC   - do not include any quote marks)
set /p GHIDRA=Ghidra folder: 
set GHIDRA=%GHIDRA:"=%
if not exist "%GHIDRA%\support\analyzeHeadless.bat" goto noghidra
>"%TOOLS%ghidra_path.txt" echo %GHIDRA%

:found
echo Using Ghidra at: %GHIDRA%
if not exist "%PROJ%" mkdir "%PROJ%"
if not exist "%OUT%"  mkdir "%OUT%"
call "%GHIDRA%\support\analyzeHeadless.bat" "%PROJ%" JPOG -import "%TARGET%" -overwrite -scriptPath "%TOOLS%ghidra" -postScript ExportDecompiled.java "%OUT%"
echo.
echo Output folder: %OUT%
dir /b "%OUT%\*.c"
pause
exit /b 0

:usage
echo Usage: %~nx0 path\to\file.dll
pause
exit /b 1

:nofile
echo ERROR: file not found: %TARGET%
echo Tip: run   dir original\DLLS   to see the exact file names (they may end in _inst.dll).
pause
exit /b 1

:noghidra
echo ERROR: "%GHIDRA%\support\analyzeHeadless.bat" does not exist.
echo        Find ghidraRun.bat on your PC and use the folder it is in.
pause
exit /b 1
