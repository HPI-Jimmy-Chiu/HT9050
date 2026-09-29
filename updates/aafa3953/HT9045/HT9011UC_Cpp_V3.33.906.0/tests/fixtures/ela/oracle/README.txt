ELA_Reports section 7 -- oracle files for SaveSummary.  AI(W906-ELA-R2) 20260927 (St02-E).

St02 compiles only and never runs an exe.  Whoever makes the oracle (the laptop or St01) needs a PC with the BCB
analyzer D:\EventlogAnalyzer\EventlogAnalyzer.exe (SVN Rev891, FileVersion 20.25.891.0; a copy is at
D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe).

1. Start EventlogAnalyzer.exe fresh.
2. "Import" (event log folder)      -> <repo>\HT9011UC_Cpp_V3.33.906.0\tests\fixtures\ela\EventLogTxt
   "Import Prod" (Production_Log)   -> <repo>\HT9011UC_Cpp_V3.33.906.0\tests\fixtures\ela\Production_Log
3. Start 2025/09/10 00:00:00, End 2025/09/10 23:59:59.  "Summary of Alarm" filter: By JAM.
4. Press Query.  Leave "Include alarm list" UNCHECKED.  Press "Save Summary", any folder, file name  r2detail
   (no extension)  ->  <Machine ID>-SummaryData_r2detail.txt
5. Tick "Include alarm list".  Press Query AGAIN.  "Save Summary", file name  r2alarm
   ->  <Machine ID>-SummaryData_r2alarm.txt
6. Copy both files into this folder unchanged (cp950; the test converts them to UTF-8 first -- ruling D-e: V906 writes
   its reports in UTF-8).  Also copy that PC's D:\HT9045\Error\English\JAM0000.dat (after the run) here as
   JAM0000.dat: the MTBA / MTBF look-ups read it.  The test reads copies in %TEMP%, never this folder.

Notes
- Save only once per Query: golden appends the detail to the summary memo in place, so a second Save without a new
  Query repeats it.
- The exe's "Fail Count" also counts its earlier queries (golden G1: iFailCount is never reset, e.g. its start-up
  query of that day).  The test takes that offset from the oracle's "Fail Count:" line and prints it.
- The Handler ID is taken from the oracle's second line (the PC's Gerneral.ini Machine ID, "HT-90xx" when missing).
- A missing oracle file is a SKIP, not a FAIL.  The port side runs in golden mode (bcbCommaText, keepGoldenBugs,
  goldenFileRule all true).
