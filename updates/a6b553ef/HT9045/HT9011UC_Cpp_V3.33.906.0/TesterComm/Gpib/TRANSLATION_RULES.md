# GPIB bridge → V906 translation rules (shared by every TesterComm/Gpib/*.cpp)

AI(W906-GB-P1) 20260926.  Golden: `D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525` (Big5 / CP950 — decode with cp950).
Target: `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` (C++17, UTF-8, MinGW.org GCC 6.3 win32 threads; **no std::thread / std::mutex / std::to_string**).

1. **Header is fixed.** Everything is declared in `TesterComm/Gpib/GpibBridge.h` (namespace `gpibbridge`) and `TesterComm/Gpib/GpibDriver.h`. Do **not** edit them. If a golden symbol you need is missing, declare it `static` inside your own .cpp when it is local to your functions; if it is shared, list it in your final report and gate the use (rule 6).
2. **One file per worker, namespace gpibbridge.** Start the file with `#include "TesterComm/Gpib/GpibBridge.h"` and wrap all code in `namespace gpibbridge { ... }`. Define TSerialPoll member functions as `ReturnType TSerialPoll::Name(args)`.
3. **Golden text, faithful.** Copy bodies line by line; keep golden comments (Big5 → UTF-8). Keep golden quirks and bugs (e.g. `ALed8` assigned twice); if a quirk would be undefined behaviour in C++, keep the golden intent and add `//AI(W906-GB-P1) 20260926: ...`. Drop `__fastcall`. Keep `static` locals as they are.
4. **Mechanical replacements** (see the header comment of GpibBridge.h):
   - `SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)..., (LPARAM)pcp)` → `PostToHandler(pcp)`
   - `Close()` → `RequestClose("<function name>")`
   - `Application->ProcessMessages()` → nothing (comment it)
   - `MessageDlg(...)` / `ShowMessage(...)` / `Application->MessageBox(...)` → `UiNotice(text)`; if golden branched on the answer, take the default/OK branch and comment it
   - `Application->Title = x` → comment only
   - `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` → `GHandler2Gpib = reinterpret_cast<MV*>(P->lpData);` (BCB idiom: iSendCommand is the struct's first member)
   - `this->Handle` → `reinterpret_cast<HWND>(this)` (a stable non-NULL token)
   - `Pointer` → `void*`, `TCloseAction &Action` → dropped
   - widget colours: `clXxx` constants may not exist; use the numeric BGR value with a comment
5. **vclcompat API.** AnsiString (`vclcompat/AnsiString.h`: sprintf, Pos, AnsiPos, SubString, Delete, Insert, Trim, UpperCase, c_str, Length, ToInt…), TStringList (`vclcompat/TStringList.h`), TIniFile / TMemIniFile (`vclcompat/IniFiles.h`), SysUtils (`vclcompat/SysUtils.h`: FileExists, DirectoryExists, ForceDirectories, IntToStr, StrToInt, Now, DecodeDate/DecodeTime, FormatDateTime…), TDateTime (`vclcompat/TDateTime.h`), HTimer (`vclcompat/HTimer.h`), TQPF_Timer (`myTimer.h`), TComm (`vclcompat/Comm.h`). **Check the header before using a method.** If a method is missing, write a small `static` helper in your file with the golden semantics, or gate (rule 6). Never edit vclcompat.
6. **Gates.** Code that cannot be made to compile faithfully goes in `#if 0 // TODO(W906-GB-P1): <reason, golden file:line>` … `#endif`, with the golden text inside. Every gate is listed in your final report.
7. **No compiler on this machine.** Nothing can be built here. Review your own file carefully: every identifier declared (header, your statics, vclcompat, windows.h), braces balanced, no C++11 library features missing from MinGW 6.3 (no std::to_string, std::thread, std::mutex, std::stoi is also unreliable — use atoi/strtol).
8. **No git.** Do not run any git command (another session shares this checkout). Only create/modify your own file.
9. **Final report** (plain text): file written, golden line ranges covered, every gate with reason, every missing shared symbol you needed, any golden quirk you kept that looks like a bug.
