@echo off
rem Sweep pass 6: combinations of the hidden mcpcom options that helped in passes 3-5, compiled over
rem every TRenderInterface source (not just two files), with relocations, so each result can be scored
rem exactly against the original. Diagnostic only: nothing here goes into Src\ or flags.txt.
rem   A = P3OPT_pcg_ra_optim_color        (TViewport/TRenderAdapter dtors, DeleteHandlers closer; no losses)
rem   B = P3OPT_pcg_ra_region_picking=0   (same effect as A in pass 4)
rem   C = P3OPT_fast_float_to_int         (MakeRGBA 197 -> 93 differing bytes)
rem   D = IPOPT_inline_max_total_stats=32 (less inlining: dtors, CreateResource, SetAsLastChild closer)
rem Every compile has a 60 s watchdog. Results: Results\sweep6\<label>\<file>.txt / .rel, log.txt.
setlocal
set SW=%OUT%\sweep6
if not exist "%SW%" mkdir "%SW%"
set T=%TEMP%\jpogsweep6
if exist "%T%" rd /s /q "%T%"
mkdir "%T%"
copy /y "%WORK%\TRenderInterface\*.cpp" "%T%\" >nul
del /q "%T%\Var*.cpp" >nul 2>&1
set XF=
if exist "%WORK%\TRenderInterface\flags.txt" set /p XF=<"%WORK%\TRenderInterface\flags.txt"
set INC=%WORK%\include
pushd "%T%"
set LOG=%SW%\log.txt
> "%LOG%" echo sweep6 start %date% %time% flags [%XF%]
call :combo BASE ""
call :combo A "-Qoption,c,-mP3OPT_pcg_ra_optim_color"
call :combo B "-Qoption,c,-mP3OPT_pcg_ra_region_picking=0"
call :combo C "-Qoption,c,-mP3OPT_fast_float_to_int"
call :combo D "-Qoption,c,-mIPOPT_inline_max_total_stats=32"
call :combo AC "-Qoption,c,-mP3OPT_pcg_ra_optim_color -Qoption,c,-mP3OPT_fast_float_to_int"
call :combo AD "-Qoption,c,-mP3OPT_pcg_ra_optim_color -Qoption,c,-mIPOPT_inline_max_total_stats=32"
call :combo ACD "-Qoption,c,-mP3OPT_pcg_ra_optim_color -Qoption,c,-mP3OPT_fast_float_to_int -Qoption,c,-mIPOPT_inline_max_total_stats=32"
call :combo BCD "-Qoption,c,-mP3OPT_pcg_ra_region_picking=0 -Qoption,c,-mP3OPT_fast_float_to_int -Qoption,c,-mIPOPT_inline_max_total_stats=32"
>> "%LOG%" echo sweep6 end %date% %time%
popd
goto :eof

rem %1 = label, %2 = extra options (quoted)
:combo
if not exist "%SW%\%1" mkdir "%SW%\%1"
set RES=
for %%f in (*.cpp) do call :one %%~nf %1 %2
>> "%LOG%" echo %1 %RES%
goto :eof

:one
set TIMEDOUT=
if exist %1.obj del /q %1.obj
call :guarded "%FLAGS% %XF% -I %INC% %~3 -c %1.cpp" o.log
if defined TIMEDOUT goto onetimeout
if not exist %1.obj goto oneerr
dumpbin /disasm %1.obj > "%SW%\%2\%1.txt"
dumpbin /relocations /symbols %1.obj > "%SW%\%2\%1.rel"
goto :eof
:onetimeout
set RES=%RES% %1=TIMEOUT
goto :eof
:oneerr
set RES=%RES% %1=ERR
copy /y o.log "%SW%\%2\%1.err.txt" >nul
goto :eof

rem run one compile in the background and wait for it (max 60 s): %1 = command line tail, %2 = log
:guarded
if exist done.flg del /q done.flg
start "" /b cmd /c icl %~1 ^> %2 2^>^&1 ^& echo x^> done.flg
set /a WAITED=0
:gwait
if exist done.flg goto :eof
ping -n 2 127.0.0.1 >nul
set /a WAITED+=1
if %WAITED% LSS 60 goto gwait
taskkill /f /im mcpcom.exe >nul 2>&1
taskkill /f /im icl.exe >nul 2>&1
taskkill /f /im dwwin.exe >nul 2>&1
taskkill /f /im drwtsn32.exe >nul 2>&1
ping -n 3 127.0.0.1 >nul
set TIMEDOUT=1
goto :eof
