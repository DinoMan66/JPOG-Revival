@echo off
setlocal enableextensions
title JPOG build watcher
rem ======================================================================
rem  JPOG build watcher - start it once and leave the window open.
rem  It only waits for Tools\queue\request.txt and then runs Tools\build_core.bat,
rem  so Claude can change the build steps (build_core.bat) without you ever
rem  restarting this window. Ctrl+C stops it.
rem ======================================================================
pushd "%~dp0.."
set ROOT=%CD%
rem a drive root comes back as "Y:\" - drop the trailing backslash so paths never contain "\\"
if "%ROOT:~-1%"=="\" set ROOT=%ROOT:~0,-1%
set QUEUE=%ROOT%\Tools\queue
if not exist "%QUEUE%" mkdir "%QUEUE%"

echo Project : %ROOT%
echo Watching: %QUEUE%\request.txt
echo (leave this window open; Ctrl+C stops it)
echo.

:loop
if exist "%QUEUE%\request.txt" goto build
ping -n 4 127.0.0.1 >nul
goto loop

:build
set REQ=
set /p REQ=<"%QUEUE%\request.txt"
del "%QUEUE%\request.txt"
echo [%date% %time%] request: %REQ%
if exist "%ROOT%\Tools\build_core.bat" (
  call "%ROOT%\Tools\build_core.bat" "%ROOT%" "%REQ%"
) else (
  echo   ERROR: %ROOT%\Tools\build_core.bat is missing
)
echo [%date% %time%] finished
echo.
goto loop
