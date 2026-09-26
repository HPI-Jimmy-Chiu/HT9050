# Handler-side tester communication → V906 translation rules (TesterComm/Handler/*.cpp)

AI(W906-GB-P2a) 20260926.  Golden: `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` (+ `main.h` for member declarations) — Big5 / CP950, decode with cp950.
Target: `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` (C++17, UTF-8, MinGW.org GCC 6.3 win32 threads; **no std::thread / std::mutex / std::to_string**).
This is **Handler** code (the machine side), so unlike the bridge engines it lives in the **global namespace** and uses the V906 Handler globals and the `fMain` facade. Read `TesterComm/Handler/HandlerTesterSide.h` first: its banner is part of these rules.

1. **Header is fixed.** `TesterComm/Handler/HandlerTesterSide.h` declares class `THandlerTesterSide`. Do **not** edit it (or any other existing file). Define its members as `ReturnType THandlerTesterSide::Name(args)`.
2. **Which object owns a name** (golden bodies were TfMain members, so unqualified names were TfMain members or globals):
   - a member declared in `THandlerTesterSide` → keep it unqualified;
   - any other golden TfMain member (check golden `main.h` class TfMain) → `fMain->X`, and **verify X exists in V906 `forms/fMain.h`** (grep). If it does not, gate the use (rule 6) and list it;
   - golden already wrote `fMain->X` → keep it if X is in V906 `forms/fMain.h`; if X is a `THandlerTesterSide` member (e.g. `fMain->bFind`, `fMain->HVisionWnd`), write `X` (this object) with an AI note;
   - globals → unqualified, as golden; each must be declared by a V906 header you include (Command.cpp's include list at `Command.cpp:226-262` is the reference for where Handler globals live). If a global does not exist in V906, gate and list it.
3. **Golden text, faithful.** Copy bodies line by line; keep golden comments (Big5 → UTF-8) and golden quirks. Undefined behaviour in C++ → keep the golden intent, add `//AI(W906-GB-P2a) 20260926: ...`. Drop `__fastcall`. Keep `static` locals (and see rule 9).
4. **Mechanical replacements:**
   - `SendMessage(HVisionWnd, WM_COPYDATA, ...)` / `SendMessage(fMain->HVisionWnd, WM_COPYDATA, ...)` → `SendToBridge(pcp)`
   - `FindWindow("TSerialPoll", ...)` / `FindWindow("TfRS232Main", ...)` → `FindBridgeWindow()` (keep golden's if/else ladder around it)
   - `CreateProcess(... H9046_32GPIB.exe / RS232Standard.exe ...)` → `iProgramReady=StartBridgeProgram();`; `GetExitCodeProcess/TerminateProcess(pi...)` of the old bridge → comment (the hub stops the old engine when the type changes)
   - `this->Handle` → `HandlerWndToken()`
   - `&HGpib2Handler->iCommand=(unsigned int *)P->lpData;` → `HGpib2Handler = reinterpret_cast<VM*>(P->lpData);` (HGpib2Handler is the global `VM*`)
   - `Application->ProcessMessages()` → comment; `MessageDlg`/`ShowMessage` → `ShowMyMessage(...)` if V906 has it (canary_support.h), else gate
   - `_OnMyCopyMsg_Interface(msg)` → `_OnMyCopyMsg_Interface(lParam)` (V906 Interface/InterfaceSYS.h signature)
5. **Out of scope → gate, not translate:** the SPEA `Interface.exe` launch (`TestIF.iGpibMode==InterfaceType_SPEA_Type` branches of ProcessHVisionConnect/WakeupGPIB), ESD (`WakeupESD`, `HESDWnd`), EventLogSaver/EventlogAnalyzer launching (`WakeupEventLogSaver`). Gate them with `#if 0 // TODO(W906-GB-P2a): <program> is not the tester bridge (plan: <ESD / event-log analyzer / SPEA>) — golden main.cpp:<lines>` and golden text inside. The `TTL_CARD_TYPE<2` direct DIO-card branch (user ruling: not used on the new machines) is also gated when you meet it.
6. **Gates.** `#if 0 // TODO(W906-GB-P2a): <reason, golden file:line>` … `#endif` with the golden text inside. Every gate is listed in your final report with the missing symbol that forced it. Prefer the smallest gate (one statement / one `if` block) over gating a whole branch.
7. **vclcompat API.** Check the header before using a method (`vclcompat/AnsiString.h`, `TStringList.h`, `SysUtils.h`, `Controls.h`, `IniFiles.h`). Missing method → a small `static` helper with BCB6 semantics, or gate.
8. **Traps already measured in this tree:** `X->Click()` is a no-op (call the golden handler directly); `Strings[i]` is a proxy — wrap in `AnsiString(...)` before a variadic `sprintf` or a member call; `AnsiString == 0` means `== "0"` in BCB6 (write `== AnsiString(0)`); `strupr`/`itoa` need file-static helpers under -std=c++17.
9. **Function-local statics.** These are Handler statics (the Handler process is NOT relaunched), so keep them exactly as golden — no re-arm.
10. **No compiler on this machine; no git.** Review by hand/script: every identifier declared, braces balanced. Do not run git. Only create your own file. Never touch anything under `D:\HT9045` outside `TesterComm/Handler/`.
11. **Final report** (plain text): file written, golden line ranges covered, every gate with the symbol/reason, every `fMain->` member you relied on (so the facade owner knows), every THandlerTesterSide member you needed that the header lacks (do not add it — report it), golden quirks kept that look like bugs.
