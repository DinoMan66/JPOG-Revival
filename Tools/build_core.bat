@echo off
rem ======================================================================
rem  Called by watch.bat:  build_core.bat <project root> <request id>
rem  Compiles every .cpp in Src\ and writes Results\. Claude may update this file.
rem ======================================================================
setlocal enableextensions
set ROOT=%~1
set REQ=%~2
set OUT=%ROOT%\Results
rem optional 3rd word of the request = build only that Src\<Dll> folder
set ONLY=
for /f "tokens=3" %%a in ("%REQ%") do set ONLY=%%a
set WORK=%TEMP%\jpogwork

set VCVARS=C:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT
set ICCDIR=
if exist "C:\ICC501\bin\icl.exe" set ICCDIR=C:\ICC501
if not defined ICCDIR if exist "C:\OpenSourceProjects\ICC501\bin\icl.exe" set ICCDIR=C:\OpenSourceProjects\ICC501
rem optional 4th word of the request = alternative compiler folder (C:\<word>), e.g. ICC60
set ALTICC=
for /f "tokens=4" %%a in ("%REQ%") do set ALTICC=%%a
if defined ALTICC if exist "C:\%ALTICC%\bin\icl.exe" set ICCDIR=C:\%ALTICC%
set FLAGS=-Qvc6 -nologo -Gz -MD -W3 -vmg -vmm -Zi -O2 -D TOSHI_SKU_WINDOWS -D WIN32 -D _WINDOWS -D _UNICODE -D TOSHI_FINAL -D NDEBUG -D USER_JPBUILD

if not exist "%OUT%" mkdir "%OUT%"
del /q "%OUT%\*.txt" >nul 2>&1
del /q "%OUT%\*.obj" >nul 2>&1
del /q "%OUT%\*.rel" >nul 2>&1

if not defined ICCDIR (
  > "%OUT%\DONE.txt" echo %REQ% ERROR no Intel compiler found
  echo   ERROR: Intel compiler not found
  goto :eof
)
if not exist "%VCVARS%" (
  > "%OUT%\DONE.txt" echo %REQ% ERROR VCVARS32.BAT not found
  echo   ERROR: VC6 not found
  goto :eof
)
call "%ICCDIR%\bin\iccvars.bat" >nul

if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
xcopy /E /I /Y /Q "%ROOT%\Src\*" "%WORK%\" >nul
rem headers belong only in include\ - a stray one next to the .cpp would override it
del /q "%WORK%\*.h" >nul 2>&1

rem optional diagnostic commands from Claude (Tools\queue\cmd.bat), output to Results\cmd.txt
if exist "%ROOT%\Tools\queue\cmd.bat" call :runcmd "%ROOT%\Tools\queue\cmd.bat"
rem uniquely named ones too (Tools\queue\cmd_<id>.bat): a re-created cmd.bat can be served stale by the shared folder
for %%c in ("%ROOT%\Tools\queue\cmd_*.bat") do call :runcmd "%%~c"

rem one folder per DLL: Src\<Dll>\*.cpp  (Src\include is shared). Results\<Dll>.<file>.txt
for /d %%D in ("%WORK%\*") do if /i not "%%~nxD"=="include" call :dir "%%~nxD"
rem variant experiments are one-shot: remove them so the next round starts clean
del /q /s "%ROOT%\Src\Var*.cpp" >nul 2>&1
del /q /s "%ROOT%\Src\Var*.flags" >nul 2>&1
rem one bundle with everything, so a single file can be fetched
for %%f in ("%OUT%\*.txt") do call :bundle "%%f"
copy /y "%OUT%\ALL.tmp" "%OUT%\ALL.txt" >nul
del /q "%OUT%\ALL.tmp" >nul 2>&1
> "%OUT%\DONE.txt" echo %REQ% OK
goto :eof

:dir
if defined ONLY if /i not "%~1"=="%ONLY%" goto :eof
pushd "%WORK%\%~1"
for %%f in (*.cpp) do call :one "%~1" "%%f"
popd
goto :eof

:one
set N=%~1.%~n2
rem extra compiler flags: Src\<Dll>\flags.txt (whole folder) and <file>.cpp.flags (one file)
set XF=
set XF2=
if exist "flags.txt" set /p XF=<"flags.txt"
if exist "%~2.flags" set /p XF2=<"%~2.flags"
echo   compiling %~1\%~2 %XF% %XF2%
> "%OUT%\%N%.compile.txt" echo extra flags: [%XF% %XF2%]
icl %FLAGS% %XF% %XF2% -I "%WORK%\include" -c "%~2" >> "%OUT%\%N%.compile.txt" 2>&1
if exist "%~n2.obj" dumpbin /disasm "%~n2.obj" > "%OUT%\%N%.txt"
if exist "%~n2.obj" dumpbin /relocations /symbols "%~n2.obj" > "%OUT%\%N%.rel"
if not exist "%~n2.obj" echo   FAILED: %~1\%~2
goto :eof

:runcmd
rem anti-cheat: a command file could replace the compiler, so only run them when the user allows it
if not exist "C:\anticheat_allow_cmd.txt" (
  >> "%OUT%\cmd.txt" echo REFUSED %~nx1 - commands are disabled by the anti-cheat. To allow them, create C:\anticheat_allow_cmd.txt INSIDE the VM yourself.
  del /q "%~1" >nul 2>&1
  goto :eof
)
call "%~1" >> "%OUT%\cmd.txt" 2>&1
del /q "%~1" >nul 2>&1
goto :eof

:bundle
>> "%OUT%\ALL.tmp" echo ##FILE %~nx1
type %1 >> "%OUT%\ALL.tmp"
if exist "%OUT%\%~n1.rel" >> "%OUT%\ALL.tmp" echo ##FILE %~n1.rel
if exist "%OUT%\%~n1.rel" type "%OUT%\%~n1.rel" >> "%OUT%\ALL.tmp"
goto :eof
