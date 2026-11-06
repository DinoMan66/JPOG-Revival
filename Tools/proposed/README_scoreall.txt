Proposed change to Tools\scoreall.py (protected - review, copy over the original, then re-run the baseline):

Original\SimJP.exe was added to Notes\function_tracker.csv on 2026-10-28 (13,664 exported functions).
scoreall.py only looked for *.dll originals, so a build of Src\SimJP\ would print
"SimJP : no original DLL found under Original/". The change adds Original\*.exe to the search and strips
".exe" from the key, so Src\SimJP\ maps to Original\SimJP.exe. Nothing else changes (2 lines).

To apply:  copy Tools\proposed\scoreall.py Tools\scoreall.py
           py C:\AntiCheat\anticheat.py C:\OpenSourceProjects\JPOGDecompiled baseline
