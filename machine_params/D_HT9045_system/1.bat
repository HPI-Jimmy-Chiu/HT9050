robocopy "D:\HT9045_Log\EventLogTxt" "D:\HT9045_StateRecord\2026-09-29 11_26_45\EventLogTxt" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP
XCOPY /y/a/e/c/i/h/f/r "D:\HT9045\system" "D:\HT9045_StateRecord\2026-09-29 11_26_45\HT9045\system"
XCOPY /y/a/e/c/i/h/f/r "D:\HT9045\config" "D:\HT9045_StateRecord\2026-09-29 11_26_45\HT9045\config"
XCOPY /y/a/e/c/i/h/f/r "D:\GPIB9045\system" "D:\HT9045_StateRecord\2026-09-29 11_26_45\GPIB9045\system"
robocopy "D:\GPIBLOG\Log\2026_09" "D:\HT9045_StateRecord\2026-09-29 11_26_45\GPIBLOG" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP
robocopy "D:\HT9045_Log\Galil_Log" "D:\HT9045_StateRecord\2026-09-29 11_26_45\Galil_LOG" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP
robocopy "D:\HT9045_Log\MNetLog" "D:\HT9045_StateRecord\2026-09-29 11_26_45\MNetLog" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP
XCOPY /y/a/e/c/i/h/f/r "D:\HT9045_Log\Automation\2026_09" "D:\HT9045_StateRecord\2026-09-29 11_26_45\Automation\2026_09"
d:\HT9045\7z.exe a -tzip "D:\HT9045_StateRecord\2026-09-29 11_26_45\Automation\2026_09.7z" "D:\HT9045_StateRecord\2026-09-29 11_26_45\Automation\2026_09\*.*"
RMDIR /s/q "D:\HT9045_StateRecord\2026-09-29 11_26_45\Automation\2026_09"
