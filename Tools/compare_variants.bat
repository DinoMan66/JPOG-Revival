@echo off
setlocal
rem ----------------------------------------------------------------------
rem  Compares every experiment variant (VarA..VarE) against the original DLL.
rem  Put this file in the same folder as compare.py (your Tools folder).
rem  It works from ANY starting folder: the project root is taken to be the
rem  folder one level above this script (it must contain original\, results\, notes\).
rem ----------------------------------------------------------------------
set TOOLS=%~dp0
cd /d "%~dp0.."
set ROOT=%CD%

if not exist "%ROOT%\original\TApplication.dll" (
  echo.
  echo ERROR: could not find %ROOT%\original\TApplication.dll
  echo        This script assumes it sits in a Tools folder directly inside the project root.
  echo        Script folder: %TOOLS%
  echo        Project root it assumed: %ROOT%
  pause
  exit /b 1
)
if not exist "%TOOLS%compare.py" (
  echo ERROR: compare.py is not in %TOOLS%
  pause
  exit /b 1
)

echo Project root: %ROOT%
for %%v in (A B C D E) do (
  echo.
  echo ================ Variant %%v ================
  if exist "results\Var%%v.txt" (
    py "%TOOLS%compare.py" original\TApplication.dll results\Var%%v.txt --map notes\TApplication.map --brief
  ) else (
    echo   results\Var%%v.txt not found - was it built and copied back?
  )
)
echo.
pause
