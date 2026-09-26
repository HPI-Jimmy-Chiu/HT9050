# RS232Standard → V906 translation rules (shared by every TesterComm/Rs232/*.cpp)

AI(W906-GB-P4) 20260926.  Golden: `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410` (Big5 / CP950 — decode with cp950).
Target: `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` (C++17, UTF-8, MinGW.org GCC 6.3 win32 threads; **no std::thread / std::mutex / std::to_string**).
These are the GPIB rules (`TesterComm/Gpib/TRANSLATION_RULES.md`) with the names changed, plus the lessons the GPIB translation taught (rules 10-14).

1. **Header is fixed.** Everything is declared in `TesterComm/Rs232/Rs232Bridge.h` (namespace `rs232std`). Do **not** edit it. If a golden symbol you need is missing, declare it `static` inside your own .cpp when it is local to your functions; if it is shared, list it in your final report and gate the use (rule 6).
2. **One file per worker, namespace rs232std.** Start the file with `#include "TesterComm/Rs232/Rs232Bridge.h"`, then (files holding MainForm.cpp text only) the golden build switches exactly as the header banner shows (`#if RS232STD_GOLDEN_DEBUG` / `#define DEBUG` … and `SOFT_SIMULTE`), then wrap all code in `namespace rs232std { ... }`. Define members as `ReturnType TfRS232Main::Name(args)`.
3. **Golden text, faithful.** Copy bodies line by line; keep golden comments (Big5 → UTF-8). Keep golden quirks and bugs; if a quirk would be undefined behaviour in C++, keep the golden intent and add `//AI(W906-GB-P4) 20260926: ...`. Drop `__fastcall`. Keep `static` locals as they are (and see rule 13).
4. **Mechanical replacements:**
   - `SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)..., (LPARAM)pcp)` → `PostToHandler(pcp)`
   - `Close()` → `RequestClose("<function name>")`
   - `Application->ProcessMessages()` → nothing (comment it); `Application->Title = x` → comment only
   - `MessageDlg(...)` / `ShowMessage(...)` / `Application->MessageBox(...)` → `UiNotice(text)`; if golden branched on the answer, take the default/OK branch and comment it
   - `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` → `GHandler2Gpib = reinterpret_cast<MV*>(P->lpData);`
   - `this->Handle` → `reinterpret_cast<HWND>(this)`; `FindWindow("TfMain", ...)` for the Handler window → attached when `mailbox != NULL && HMountWnd != NULL` (Rs232Engine sets HMountWnd to its token before FormCreate); `FindWindow` for anything else → keep
   - `Pointer` → `void*`; `TCloseAction &Action` → dropped; mouse-event parameters `(Sender, Button, Shift, X, Y)` → `(Sender)` (the header declares them that way)
   - widget colours: `clXxx` constants may not exist; use the numeric BGR value with a comment
5. **vclcompat API.** AnsiString (`vclcompat/AnsiString.h`), TStringList, TIniFile / TMemIniFile (`vclcompat/IniFiles.h`), SysUtils, TDateTime, TComm (`vclcompat/Comm.h`), TServerSocket / TClientSocket (`vclcompat/ServerSocket.h`, `ClientSocket.h` — default SIM mode), widgets (`vclcompat/Controls.h`). **Check the header before using a method.** If a method is missing, write a small `static` helper in your file with the golden semantics, or gate (rule 6). Never edit vclcompat.
6. **Gates.** Code that cannot be made to compile faithfully goes in `#if 0 // TODO(W906-GB-P4): <reason, golden file:line>` … `#endif`, with the golden text inside. Every gate is listed in your final report. A golden `/* ... */` block is not copied inside an `#if 0` (it would swallow the `#endif`).
7. **No compiler on this machine.** Review your own file carefully: every identifier declared (header, your statics, vclcompat, windows.h), braces balanced, no C++11 library features missing from MinGW 6.3 (no std::to_string, std::thread, std::mutex; std::stoi is unreliable — use atoi/strtol). `strupr`/`itoa` are hidden under -std=c++17: write a file-static helper with Borland semantics.
8. **No git.** Do not run any git command (other sessions share this checkout). Only create/modify your own file. Never touch anything under `D:\HT9045` outside `TesterComm/Rs232/`.
9. **Final report** (plain text): file written, golden line ranges covered, every gate with reason, every missing shared symbol you needed, any golden quirk you kept that looks like a bug, every real file / COM / socket the code touches.
10. **`X->Click()`** is a no-op in vclcompat: translate as a direct call of the golden OnClick handler (`btnFooClick(btnFoo);`) with an AI note (check the .dfm for which handler is wired).
11. **`Strings[i]` is a proxy.** `x->Strings[i]` / `Items->Strings[i]` / `Lines->Strings[i]` passed to a variadic `sprintf` / `printf`, or used as `.Length()` / `.Pos()`, must be wrapped: `AnsiString(x->Strings[i])`. AnsiString itself may go to `%s` raw (the vclcompat sprintf template converts it).
12. **`AnsiString == 0` / `!= NULL`** means `== "0"` in BCB6 (AnsiString(int)); vclcompat would pick the `const char*` overload and compare with "". Write `== AnsiString(0)` with an AI note.
13. **Re-arm function-local statics per program life.** Golden relaunches the exe, so statics restart at their initialisers. Right after a function's statics add (see TesterComm/Gpib/GpibCore.cpp for the exact form):
    ```
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        <each static = its golden initialiser; buffers memset 0>;
    }
    ```
    Pure scratch statics (always written before read) and `static const` tables need none — say so in a comment.
14. **Receive callbacks only queue.** vclcompat TComm fires `OnReceiveData`, and TServerSocket / TClientSocket fire their events, on their own threads. Wire them as `CommTester->OnReceiveData = [this](TObject*, void* b, Word n){ QueueRx(kRxTester, b, n); };` (`kRxTtl1` for CommTester_TTL, `kRxTtl2` for CommTester_TTL_2), and `uServer->SetReceiveFunc([this](char* c, int n){ QueueRx(kRxTcp, c, (Word)n); });` for golden `SetReceiveFunc(ReceiveData_TCPIP)`. Never call a golden receive handler from those threads; `DrainRx()` (Rs232Engine.cpp) does it on the TesterComm thread.
15. **Fixed-size buffers.** Where golden `strcpy`s a string that comes from the Handler or the tester into a fixed array, and a long input would overrun it, use a bounded copy with an AI note (in-process an overrun corrupts the Handler). Byte-identical for inputs golden handles correctly.
