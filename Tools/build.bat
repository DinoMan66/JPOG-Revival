@echo off
setlocal
rem ======================================================================
rem  JPOG matching build - run inside the Windows XP VM.
rem
rem  Put this script either (a) in the same folder as the .cpp files, or
rem  (b) in a tools\ folder next to a src\ folder - it looks in both places.
rem
rem  Usage:  build.bat              compile every .cpp it finds
rem          build.bat Foo.cpp      compile just Foo.cpp
rem
rem  EVERY run starts from scratch (old copies and old results are deleted),
rem  so a stale file can never be mistaken for a fresh one.
rem  Output: C:\work\results\NAME.txt, NAME.compile.txt and BUILD_INFO.txt
rem ======================================================================

rem ---- Edit these if your paths differ ----
set VCVARS=C:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT
set ICCDIR=C:\ICC501
rem ---- Optional: also copy the results somewhere (shared folder, drive letter, ...). Leave empty to skip.
set COPYTO=

rem ---- Compiler flags copied from the game's DLLs ----
set FLAGS=-Qvc6 -nologo -Gz -MD -W3 -vmg -vmm -Zi -O2 -D TOSHI_SKU_WINDOWS -D WIN32 -D _WINDOWS -D _UNICODE -D TOSHI_FINAL -D NDEBUG -D USER_JPBUILD

set WORK=C:\work\src
set OUT=C:\work\results

rem ---- Find the sources ----
set SRC=%~dp0
if exist "%SRC%*.cpp" goto found
if exist "%~dp0..\src\*.cpp" set SRC=%~dp0..\src\
:found
if not exist "%SRC%*.cpp" (
  echo.
  echo ERROR: no .cpp files found next to build.bat or in ..\src
  echo        Looked in: %~dp0   and   %~dp0..\src\
  pause
  exit /b 1
)

call "%ICCDIR%\bin\iccvars.bat" >nul

rem ---- Start from a clean slate ----
if exist "%OUT%"  rd /s /q "%OUT%"
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%OUT%"
mkdir "%WORK%"
xcopy /E /I /Y "%SRC%*" "%WORK%\" >nul
cd /d "%WORK%"

rem ---- Record exactly what is being built (dates + sizes prove it is the new source) ----
echo Build started: %date% %time%  > "%OUT%\BUILD_INFO.txt"
echo Sources copied from: %SRC% >> "%OUT%\BUILD_INFO.txt"
echo. >> "%OUT%\BUILD_INFO.txt"
dir /s /a-d "%WORK%" >> "%OUT%\BUILD_INFO.txt"
rem ---- Headers belong ONLY in include\. A same-named .h next to the .cpp would silently
rem ---- override the one in include\ (quoted includes search the .cpp's own folder first).
if exist "%WORK%\*.h" (
  echo.
  echo *** WARNING: stray header files found next to the .cpp - removing them from the build copy:
  dir /b "%WORK%\*.h"
  echo *** Delete them from your real Src folder too, or they will keep appearing.
  echo STRAY HEADERS REMOVED FROM BUILD COPY: >> "%OUT%\BUILD_INFO.txt"
  dir /b "%WORK%\*.h" >> "%OUT%\BUILD_INFO.txt"
  del /q "%WORK%\*.h"
)

echo.
echo ===== Building from: %SRC% =====
type "%OUT%\BUILD_INFO.txt"

if "%~1"=="" (
  for %%f in (*.cpp) do call :one %%f
) else (
  for %%f in (%*) do call :one %%f
)

if defined COPYTO (
  xcopy /Y "%OUT%\*" "%COPYTO%\" >nul
  echo Copied results to %COPYTO%
)

echo.
echo ============================================================
echo  Finished at %time%.
echo  Copy EVERYTHING in %OUT%
echo  back to the PC's results\ folder, overwriting the old files.
echo ============================================================
pause
goto :eof

:one
echo.
echo ===== Compiling %1 =====
if exist "%~n1.obj" del "%~n1.obj"
icl %FLAGS% -I "%WORK%\include" -c %1 > "%OUT%\%~n1.compile.txt" 2>&1
type "%OUT%\%~n1.compile.txt"
findstr /i /c:": error" "%OUT%\%~n1.compile.txt" >nul
if not errorlevel 1 echo *** The compiler reported ERRORS in %1 - see above ***
if exist "%~n1.obj" (
  dumpbin /disasm "%~n1.obj" > "%OUT%\%~n1.txt"
  echo OK: wrote %OUT%\%~n1.txt
) else (
  echo *** FAILED: no .obj was produced for %1 ***
)
goto :eof
