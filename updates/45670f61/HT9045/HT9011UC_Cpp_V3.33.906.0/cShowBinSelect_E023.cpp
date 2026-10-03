// =============================================================================
//  cShowBinSelect_E023.cpp -- todo E-023 SB-1 / SB-3 / SB-4: the St01 operator actions of Status.ShowBinSelect (golden TfShowBinSelect)
//
//  //AI(W906-E023-SB1) 20261002 [W906] (St01) new file, ht9045_sm (CMakeLists.txt:2498, same line).  todo
//    D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-023; St02 inventory
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.10 (golden 906_0625 numbers there; the cShowBinSelect.cpp / main.cpp numbers below are 0618, AI(W906-E032) 20261003).  AI(W906-E030-CITE) 20261003 (St01)：下面裸的 ":N" 是 906 行號，（V912 :N）是 V912
//    （main.cpp 用 c2f6c75a 之前的 V912；cShowBinSelect.dfm 兩邊相同，dfm 行號只寫一次）; Jimmy RULINGS_20261001 #0 "translate per golden and wire everything".  Kept out of jimmychiu's cShowBinSelect.cpp
//    (ST01-E 20261002: prefer an St01 file when the code is only free functions + a dispatcher).
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] (cp950, not in git); V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in （）:
//    SB-1 cShowBinSelect.cpp:2101-2166（V912 :2248-2315） btnAutoCleanClick   (dfm :3010 btnAutoClean, in gbAutoCleanCount :3003 on tsIndex :2793;
//                                                            Visible = IniConfig.bEnableAutoCleanFunction, FormShow :803（V912 :864）)
//    SB-3 cShowBinSelect.cpp:2054-2099（V912 :2201-2246） UPH_StringGridDblClick (dfm :1339 UPH_StringGrid on Tab_UPH :1336, OnDblClick :1358)
//    SB-4 cShowBinSelect.cpp:2845-2850（V912 :2994-2999） sbCopyRecipeClick   (dfm :3326 sbCopyRecipe in gbCopyRecipe :3319 on tsIndex; no Visible rule)
//         -> main.cpp:34444-34451（V912 :35596-35603） TfMain::RunBatchCopyRecipe
//  Not here: SB-2 (Clean Reset level 97 / Auto Clean Count level 43): golden fCleaning->btnResetCleanCountClick already exists in the port
//    as the generated static CL_btnResetCleanCountClick (FileRW/TestIF_File_Cleaning.gen.inc:3352, the Cleaning page's form.event); the
//    row waits on Q41 CL-2.  SB-5 (TimerAutoCleanCountTimer) waits on E-025.  CLEAR keeps its own arm act.showBinSelect.clearCount
//    (tools/wb_serve.cpp:4826 -> WebShowBinSelect.cpp).
//
//  WS route (tools/wb_serve.cpp NOT touched): every act.showBinSelect.* except clearCount reaches the act.* catch-all (wb_serve.cpp:4833
//    IsActionCommand -> JsonBridge/ChanAction.cpp HandleActionWithTag); its :346 (same-line append) calls ht9045::sjson::
//    W906_ShowBinSelectAct below:
//      act.showBinSelect.state        {}                                                   read only: what golden shows on tsIndex
//      act.showBinSelect.autoClean    {}                                                   SB-1
//      act.showBinSelect.uphDblClick  {"row":r,"pageIndex":p,"cells":[...],"answer":null|"ok"|"cancel"}   SB-3
//      act.showBinSelect.copyRecipe   {}                                                   SB-4
//    ok = the reply has "executed":true (the ChanAction rule); a refusal or the OK / Cancel question comes back ok:false, JSON in ack.error.
//    Operator token: required (act.* is not in WebBridgeServer.cpp:1448's exemption list).  Double-click guard: WebCmdGuard at the
//    dispatch head (not whitelisted).  act.* runs on the main-loop (tick) thread WITHOUT FormLock (ChanAction.cpp:454).
//  Window open: golden fShowBinSelect is a modeless window (906 main.cpp:8765-8770（V912 :9190-9195） Show() when IniConfig.bShowBinCT); its buttons can only be
//    pressed while it is shown -> W906_FormShowing("fShowBinSelect", bShow) (the page table; tools/fshow_audit.py rule).  Port-only
//    refusal not-open (golden cannot reach it).
//
//  SB-1 -- the Index tab's Auto Clean.  ONE golden body, no second copy: golden TfCleaning::btnStartAutoCleanClick
//    (906 AutoClean\uCleaning.cpp:2869-2872（V912 :2901-2904）) is only `fShowBinSelect->btnAutoCleanClick(fShowBinSelect);`, so the Cleaning page button (St01
//    B8 CL-4, 413cf6da) and this button run the same golden function.  Its translation is CL-4's W906_ShowBinSelect_btnAutoCleanClick
//    (FileRW/TestIF_File_Cleaning.cpp tail; a free function because forms/fShowBinSelect.h:128-132 lists the member SAFETY-QUEUED).
//    golden's Index-tab entry has NO guards of its own.  What differs is how the button is reached:
//      * Visible = IniConfig.bEnableAutoCleanFunction (FormShow :803（V912 :864）) -> re-checked here, refusal "hidden";
//      * no level check (the Cleaning entry sits in grpCleanPara, Enabled=fSecurity->Insufficient(43,false), uCleaning.cpp FormShow :1500（V912 :1511）);
//      * golden lets it be pressed WHILE RUNNING (modeless window; nothing disables btnAutoClean during a run).
//    Running (SystemStart||SoftStart): ALLOWED, as golden -- Steven Q67 = B (1002 08:0x): 「可以按, 按了之後機台會執行one cycle, 然後才是auto clean」.
//      act.* has no runexc table (that one is form.event's, FileRW/_FormEvent.cpp tail): this handler simply has no running check.
//      golden: 906 main.cpp:8765-8770 (V912 :9190-9195) (fShowBinSelect is modeless, Show()); 906 cShowBinSelect.cpp:2101-2166 (V912 :2248-2315)
//      (no running check), reached here through CL-4's W906_ShowBinSelect_btnAutoCleanClick; a part in the machine -> InitialAutoCleanAllTask
//      (906 AutoClean\AutoClean.cpp:689-701 (V912 :767-779)) -> fMain->BtnOneCycleClick (906 main.cpp:4332 (V912 :4469)): One Cycle first,
//      then the auto clean.
//      //AI(W906-E030A) 20261003 [W906] (St01): a #20 EXCEPTION is in that body -- Jimmy RULINGS_20261002 #23 item 5 / Q77 (Q-A kept, a safety
//      guard): CL-4 keeps V912's `if(iOneCycle!=0) return;` (V912 cShowBinSelect.cpp:2254-2255, RogerYang 20260810), which 906 :2101-2166 does
//      not have.  So a press while a One Cycle is ALREADY running (iOneCycle!=0) returns at once, with no message and nothing changed: steps
//      2-5 below do not happen (in particular no second InitOneCycle resetting other tasks' cursors mid-run).  The trace below is a running
//      press with no One Cycle in progress.  No behaviour change in E-030 (the guard was already there; this note only labels it).
//      //AI(W906-E023-Q67B) 20261002 [W906] (St01): until this ruling the port refused it while running (the interim Q67 A, ST01-E / ST01-M
//      20261002 under the Q59 IO / motion-safety rule); that refusal is removed.  What a running press does (the trace that made it a Q59
//      question; all of it stays exactly as golden):
//      1. the body: flags and reads only -- MOT[MTrayX].ReadPos(), CheckIndexIsNormal (csystem.cpp:13449, encoder reads + MovFlag),
//         HasICUnderMachine (csystem.cpp:13320, memory), bRunAutoClean=true + hAutoCleanHangUp.SetSecAndOn, RecordProcess;
//      2. InitialAutoCleanTask / InitialShuttleAutoCleanTask / InitialIndexAutoCleanTask (AutoClean/AutoClean.cpp:836-874): cursors = 1;
//      3. InitialAutoCleanAllTask (AutoClean.cpp:822): flags + fMain->BtnOneCycleClick;
//      4. TfMain::BtnOneCycleClick (cCleanOut.cpp:293): InitOneCycle (csystem.cpp:391 -- it also RESETS iCleanOut / iHome / iReset /
//         iTrayFeed to 0, i.e. other tasks' cursors, mid-run), BtnOneCycle->Down, NewRecordProcess, SECS EventReport(DoOneCycle);
//         no motor, cylinder, SW output, START or Pause;
//      5. the empty-machine branch's golden ShowMyMessage (:2126（V912 :2275） / :2132（V912 :2281） / :2138（V912 :2287） / :2146（V912 :2295） / :2154（V912 :2303）) = TMyMessageBox::FormShow, which at the
//         press does SystemStart=false, SoftStart=false, StopAllMotor() (golden 906 mymessbox.cpp:302-309（V912 :303-310）; the port's MbFormShow in
//         tools/wb_serve.cpp does the same) -- a direct stop of every motor.  Kept as golden (Steven Q67 = B): golden's stopped-machine
//         refusal boxes and this ShowMyMessage / StopAllMotor path are not changed.
//      The stopped press and the running press are both golden.  The reply carries "running" (the state at the press).
//    Messages: CL-4's body reports golden's ShowMyMessage through filerw::ELMessage (it was written for form.event, which holds FormLock and
//      cannot wait for the browser).  act.* holds no FormLock, so the seat (FileRW_Cleaning_E023Seat) takes FormLock, opens a filerw
//      session, runs the body and returns the session; this file then shows each message through golden ShowMyMessage
//      (canary_support.cpp:157 -> wb_serve's web message box, which waits for OK in MbWait -- the same as E-021's act.observer.*).
//      Every golden ShowMyMessage in that body is followed by `return;`, so showing it after the body gives golden's order and end state.
//    Seat: the body is compiled only into wb_serve (FileRW/ is not a library) and ChanAction.cpp is also compiled into ctests
//      (test_sjson_chan, test_e022_contactct), so this ht9045_sm file reaches it through g_W906_E023_BtnAutoCleanSeat, installed at
//      wb_serve boot by FileRW_Cleaning_EvBoot (FileRW/TestIF_File_Cleaning.cpp:569, same line).  Not installed -> "not-installed".
//      St02's D-033 seats for GPIB / SECS are theirs; this one is for the page.
//    No START: golden has none; the machine moves only after the operator's START (ctest START_SitesCensus unchanged).
//
//  SB-3 -- UPH grid double-click (golden :2054-2099（V912 :2201-2246）, "20090720 Steven: Delete 1 UPH Record").  W906_E023_UphDblClick is an St01 copy of
//    jimmychiu's TfShowBinSelect::UPH_StringGridDblClick (cShowBinSelect.cpp:850, untouched).  His GATE (B2) (forms/fShowBinSelect.h:244)
//    keeps the delete inside #if 0 because the port had no Application->MessageBoxA: "fail-closed: the destructive row-delete loop does not
//    run without a real confirm dialog".  What it protected: a record deleted without the operator's OK.  This copy keeps that protection:
//    golden's OK / Cancel box is asked in the browser (window.confirm with golden's text and caption) and C++ deletes only on answer "ok",
//    after running golden's guards again.  No protection is dropped; the missing dialog is supplied.  (human-review C: shadow.)
//    golden guards, in C++: !SystemStart && AccessLevel>=iDefHonPrecLevel && PageControl1->ActivePageIndex==2 (Tab_UPH; dfm page order
//      tsTestBin 0, tsCategoryInfo 1, Tab_UPH 2, tsUnloadMap 3, tsIndex 4, ...; the page sends its active tab in that order because the
//      port form does not follow the browser's tab), then 0<iRow<11 with iRow = UPH_StringGrid->Selection.Top (the double-clicked row;
//      VCL selects it on the mouse-down before OnDblClick, so the port sets Selection.Top from the page).  golden returns silently when
//      one fails; the port answers with the guard (executed:false) so the page can say why.
//    [W906] port-only (golden's box is modal, it cannot reach this): the page sends the row's shown cells; if the grid differs now
//      (CalculateUPH wrote a record between the two requests) -> stale-view, nothing deleted.
//    golden oddities, kept: the shift copies row i+1 into row i for i=iRow..10, so row 11 moves into row 10; the average counts every
//      non-empty Cells[3][1..10] (atoi: a non-number counts as 0 but still counts); RunInfo.iAvgUPH is an AnsiString (cprod.h:2724)
//      assigned an int; MB_OKCANCEL's close box = Cancel.  Memory only: golden writes no file here.
//
//  SB-4 -- Copy Recipe (golden :2845-2850（V912 :2994-2999） sbCopyRecipeClick -> 906 main.cpp:34444-34451（V912 :35596-35603） TfMain::RunBatchCopyRecipe).  forms/fShowBinSelect.h:146
//    lists sbCopyRecipeClick SAFETY-QUEUED ("a setup-file mutation entry point"); forms/fMain.h has no RunBatchCopyRecipe.  St01 copies
//    W906_E023_sbCopyRecipeClick / W906_E023_RunBatchCopyRecipe (human-review C).  What golden does: two shell commands,
//    `if not exist D:\Run mkdir D:\Run` and `xcopy /y "<DataPath><recipe>\*" "D:\Run\" /s /e` -- it copies the current recipe folder into
//    D:\Run (golden also runs it at boot and on a recipe change for CC_GIGAS, 906 main.cpp:9443 / :24895（V912 :9876 / :25607）).  It reads the recipe and writes only
//    under the hard-coded D:\Run: no machine file, no motion, no IO.  golden checks no level and no running state; the button is always
//    shown on tsIndex.  golden blocks its main thread for the copy, and golden MainProc runs on that thread (Synchronize, uruncontrol.cpp:38);
//    act.* runs on the tick thread, so the port blocks the tick the same way.  Allowed while running, as golden (human-review B).
//    Seam: W906_COPYRECIPE_ROOT (unset = golden "D:\\Run"; '/' -> '\\' because cmd's mkdir reads '/' as a switch).  ctest sets it
//      (tests/CMakeLists.txt end, a DEFERred ENVIRONMENT APPEND) and the test refuses to run without it.
//    [W906] port-only, stricter than golden (Steven Q66 = B (1002 08:0x), as E-021): the recipe name (fMain->cbSetupFileName->Text: typed by an operator
//      when the recipe was created; golden splices it into a shell command) is refused when it holds \ / : * ? " < > | or ".." --
//      W906_E023_RecipeNameRefused, one function (human-review C; Steven Q66 = B (1002 08:0x)).  An EMPTY name is golden (xcopy then copies all of
//      DataPath); kept and noted.
//    sbCopyRecipe->Enabled=false / =true (:2847（V912 :2996） / :2849（V912 :2998）): the facade has no sbCopyRecipe; the page disables its button while the request is
//      out, and WebCmdGuard refuses the same request again within its window -- no second copy from a double click (same effect).
//
//  [W906] one console line per request: "[E023-SB1] ...", "[E023-SB3] ...", "[E023-SB4] ...".
//  Tests: ctest E023_StatusEvents (tests/test_e023_statusev.cpp) and E023_StatusPages (tools/webprobe/e023_status_selftest.cjs).
// =============================================================================
#include "forms/fShowBinSelect.h"
#include "MachineType.h"
#include "cmydef.h"       // SystemStart, SoftStart, AccessLevel, iDefHonPrecLevel, bRunAutoClean, bIsAutoOneCycle, iDoAutoCleanTask
#include "cprod.h"        // RunInfo
#include "Config.h"       // IniConfig
#include "common.h"       // DataPath
#include "cMyDB.h"        // MyDBIProcess (golden :2091（V912 :2238）)
#include "FormsFacade.h"  // fMain
#include "Public/cJSON.h"
#include "W906FormShowing.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>   // atoi, getenv, system
#include <cstring>
#include <string>
#include <utility>
#include <vector>

// golden mymessbox.h:58 ShowMyMessage (S3=NULL, Ok=false, bServoOff=false); body canary_support.cpp:157.  canary_support.h cannot share
//   a TU with cMyDB.h (fNote_ShowError.cpp:602), so it is declared again here, same signature, no defaults.
void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString S3, bool Ok, bool bServoOff);

// SB-1 seat (see the banner): returns filerw::SessionJson() after the golden body ran under FormLock; nullptr = not installed.
std::string (*g_W906_E023_BtnAutoCleanSeat)() = nullptr;

namespace {

std::string E023_Int(long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);   // MinGW 6.3: no std::to_string
    return b;
}
const char* E023_B(bool b) { return b ? "true" : "false"; }

std::string E023_Q(const std::string& s)
{
    std::string o = "\"";
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c == '\n') o += "\\n";
        else if (c == '\r') o += "\\r";
        else if (c == '\t') o += "\\t";
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b; }
        else o += (char)c;
    }
    return o + "\"";
}

std::string E023_Refuse(const char* ck, const char* guard, const char* goldenLine, const std::string& detail, const char* zh = "")
{
    std::printf("[%s] refused: %s (%s) -- %s\n", ck, guard, goldenLine, detail.c_str());
    return "{\"executed\":false,\"guard\":" + E023_Q(guard) + ",\"goldenLine\":" + E023_Q(goldenLine) + ",\"detail\":" + E023_Q(detail) +
           ",\"zh\":" + E023_Q(zh) + "}";
}

const char* const kNotOpenLine = "906 main.cpp:8765-8770 (V912 :9190-9195) fShowBinSelect->Show() (IniConfig.bShowBinCT)";

bool E023_Shown(TfShowBinSelect* f) { return f && W906_FormShowing("fShowBinSelect", f->bShow); }

// whole number in [-1e6, 1e6]
bool E023_Whole(const cJSON* v, int* out)
{
    if (!v || !cJSON_IsNumber(v)) return false;
    const double d = v->valuedouble;
    if (d != std::floor(d) || d < -1e6 || d > 1e6) return false;
    *out = (int)d;
    return true;
}

// ---- state: what golden FormShow shows on tsIndex (read only) ----------------------------------------------------------------
std::string E023_State()
{
    TfShowBinSelect* f = fShowBinSelect;
    const bool running = SystemStart || SoftStart;
    return std::string("{\"executed\":true,\"shown\":") + E023_B(E023_Shown(f)) +
           ",\"autoCleanVisible\":" + E023_B(IniConfig.bEnableAutoCleanFunction) +   // golden FormShow :803（V912 :864）
           ",\"autoCleanRefusedWhileRunning\":false,\"running\":" + E023_B(running) +   // Steven Q67 = B (1002 08:0x): allowed while running  //AI(W906-E023-Q67B) 20261002 [W906] (St01)
           ",\"copyRecipeVisible\":true" +                                             // dfm :3319-3343, nothing changes it
           ",\"autoCleanInstalled\":" + E023_B(g_W906_E023_BtnAutoCleanSeat != nullptr) +
           ",\"goldenLine\":" + E023_Q("906 cShowBinSelect.cpp:803 (V912 :864) btnAutoClean->Visible; dfm :3319 gbCopyRecipe") + "}";
}

// ---- SB-1: golden TfShowBinSelect::btnAutoCleanClick :2101-2166（V912 :2248-2315） through CL-4's body --------------------------------------------
std::string E023_AutoClean()
{
    static const char* const ck = "E023-SB1";
    TfShowBinSelect* f = fShowBinSelect;
    if (!f || !fMain)
        return E023_Refuse(ck, "no-form", "", "fShowBinSelect / fMain is NULL");
    if (!E023_Shown(f))
        return E023_Refuse(ck, "not-open", kNotOpenLine, "BinSelect is not open: golden presses btnAutoClean on the shown window only",
                           "BinSelect 視窗沒有開著");
    if (!IniConfig.bEnableAutoCleanFunction)
        return E023_Refuse(ck, "hidden", "906 cShowBinSelect.cpp:803 (V912 :864) FormShow btnAutoClean->Visible=IniConfig.bEnableAutoCleanFunction",
                           "[Enable Auto Clean] is off: golden does not show the Auto Clean button", "Auto Clean 功能沒有開，golden 不顯示這顆鈕");
    // running (SystemStart||SoftStart): no check -- golden has none (906 cShowBinSelect.cpp:2101-2166（V912 :2248-2315）; modeless window, 906 main.cpp:8765-8770（V912 :9190-9195）).
    //   //AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B (1002 08:0x) removed the interim running refusal (banner "Running").
    const bool running0 = SystemStart || SoftStart;   // reported only (reply "running", console line); golden's box may clear both (906 mymessbox.cpp:302-309（V912 :303-310）)
    if (!g_W906_E023_BtnAutoCleanSeat)
        return E023_Refuse(ck, "not-installed", "FileRW/TestIF_File_Cleaning.cpp W906_ShowBinSelect_btnAutoCleanClick (wb_serve only)",
                           "the golden body lives in wb_serve's FileRW/TestIF_File_Cleaning.cpp; its seat is installed at boot "
                           "(FileRW_Cleaning_EvBoot) and is missing in this process");

    const bool run0 = bRunAutoClean, auto0 = bIsAutoOneCycle;
    const std::string session = g_W906_E023_BtnAutoCleanSeat();               // golden :2101-2166（V912 :2248-2315） (CL-4 body, under FormLock)
    std::vector<std::pair<std::string, std::string> > msgs;
    if (cJSON* s = cJSON_Parse(session.c_str())) {
        const cJSON* m = cJSON_GetObjectItemCaseSensitive(s, "messages");
        for (const cJSON* it = (m && cJSON_IsArray(m)) ? m->child : nullptr; it; it = it->next) {
            const cJSON* en = cJSON_GetObjectItemCaseSensitive(it, "en");
            const cJSON* zh = cJSON_GetObjectItemCaseSensitive(it, "zh");
            msgs.push_back(std::make_pair(std::string(cJSON_IsString(en) && en->valuestring ? en->valuestring : ""),
                                          std::string(cJSON_IsString(zh) && zh->valuestring ? zh->valuestring : "")));
        }
        cJSON_Delete(s);
    }
    // golden ShowMyMessage(S1,S2) at :2126（V912 :2275） / :2132（V912 :2281） / :2138（V912 :2287） / :2146（V912 :2295） / :2154（V912 :2303） -- each followed by return; shown now, outside FormLock
    for (std::size_t i = 0; i < msgs.size(); ++i)
        ShowMyMessage(AnsiString(msgs[i].first.c_str()), AnsiString(msgs[i].second.c_str()), AnsiString(""), false, false);

    const char* result = (bRunAutoClean && !run0) ? "armed" : (bIsAutoOneCycle && !auto0) ? "oneCycle" : "nothing";
    std::printf("[E023-SB1] Index-tab Auto Clean -> golden btnAutoCleanClick (906 cShowBinSelect.cpp:2101, V912 :2248) ran through CL-4's body: %s, "
                "%d golden message(s) shown; machine %s at the press\n", result, (int)msgs.size(), running0 ? "running" : "stopped");
    std::string j = std::string("{\"executed\":true,\"result\":\"") + result + "\",\"running\":" + E023_B(running0) + ",\"messages\":[";
    for (std::size_t i = 0; i < msgs.size(); ++i)
        j += std::string(i ? "," : "") + "{\"en\":" + E023_Q(msgs[i].first) + ",\"zh\":" + E023_Q(msgs[i].second) + "}";
    j += std::string("],\"bRunAutoClean\":") + E023_B(bRunAutoClean) + ",\"bIsAutoOneCycle\":" + E023_B(bIsAutoOneCycle) +
         ",\"iDoAutoCleanTask\":" + E023_Int(iDoAutoCleanTask) +
         ",\"goldenLine\":" + E023_Q("906 cShowBinSelect.cpp:2101-2166 (V912 :2248-2315) (= 906 uCleaning.cpp:2869-2872 (V912 :2901-2904) btnStartAutoCleanClick)") + "}";
    return j;
}

// ---- SB-3: golden TfShowBinSelect::UPH_StringGridDblClick :2054-2099（V912 :2201-2246） ------------------------------------------------------------
struct E023UphReq {
    int row = -1;
    int pageIndex = -1;
    std::vector<std::string> cells;
    bool hasAnswer = false;
    bool ok = false;
};

bool E023_ParseUph(const cJSON* root, E023UphReq* q, std::string* why)
{
    if (!E023_Whole(cJSON_GetObjectItemCaseSensitive(root, "row"), &q->row)) { *why = "row must be a whole number (the grid row double-clicked)"; return false; }
    if (!E023_Whole(cJSON_GetObjectItemCaseSensitive(root, "pageIndex"), &q->pageIndex)) { *why = "pageIndex must be a whole number (the page's active tab, golden dfm order)"; return false; }
    const cJSON* c = cJSON_GetObjectItemCaseSensitive(root, "cells");
    if (!c || !cJSON_IsArray(c) || cJSON_GetArraySize(c) < 1 || cJSON_GetArraySize(c) > 32) { *why = "cells must be the row's shown texts (1..32 strings)"; return false; }
    for (const cJSON* it = c->child; it; it = it->next) {
        if (!cJSON_IsString(it) || !it->valuestring) { *why = "cells must be strings"; return false; }
        q->cells.push_back(it->valuestring);
    }
    const cJSON* a = cJSON_GetObjectItemCaseSensitive(root, "answer");
    if (a && !cJSON_IsNull(a)) {
        if (cJSON_IsString(a) && a->valuestring && std::strcmp(a->valuestring, "ok") == 0)          { q->hasAnswer = true; q->ok = true; }
        else if (cJSON_IsString(a) && a->valuestring && std::strcmp(a->valuestring, "cancel") == 0) { q->hasAnswer = true; q->ok = false; }
        else { *why = "answer must be null, \"ok\" or \"cancel\" (golden MB_OKCANCEL)"; return false; }
    }
    return true;
}

std::string W906_E023_UphDblClick(const E023UphReq& q)
{
    static const char* const ck = "E023-SB3";
    TfShowBinSelect* f = fShowBinSelect;
    if (!f || !f->UPH_StringGrid || !f->PageControl1)
        return E023_Refuse(ck, "no-form", "", "fShowBinSelect / UPH_StringGrid / PageControl1 is NULL");
    if (!E023_Shown(f))
        return E023_Refuse(ck, "not-open", kNotOpenLine, "BinSelect is not open", "BinSelect 視窗沒有開著");
    TfShowBinSelectGrid* UPH_StringGrid = f->UPH_StringGrid;
    int iRow=0, iTotalUPH=0, iCount=0;                                          // :2056（V912 :2203）
    // :2057-2058（V912 :2204-2205） -- one condition in golden; split so the page can say which part failed (golden returns silently)
    if (SystemStart)
        return E023_Refuse(ck, "running", "906 cShowBinSelect.cpp:2057 (V912 :2204) if(!SystemStart && ...)", "SystemStart: golden deletes no UPH record while running",
                           "機台運轉中，golden 不刪 UPH 紀錄");
    if (!(AccessLevel>=iDefHonPrecLevel))                                       //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
        return E023_Refuse(ck, "not-authorized", "906 cShowBinSelect.cpp:2057 (V912 :2204) AccessLevel>=iDefHonPrecLevel",
                           "AccessLevel " + E023_Int(AccessLevel) + " < iDefHonPrecLevel " + E023_Int(iDefHonPrecLevel), "等級不足（要 HonPrec 以上）");
    if (!(q.pageIndex==2))                                                      // :2058（V912 :2205） PageControl1->ActivePageIndex==2 (Tab_UPH)
        return E023_Refuse(ck, "not-uph-tab", "906 cShowBinSelect.cpp:2058 (V912 :2205) PageControl1->ActivePageIndex==2",
                           "the page's active tab is " + E023_Int(q.pageIndex) + ", not Tab_UPH (2)");
    UPH_StringGrid->Selection.Top = q.row;                                      // [W906] VCL selects the double-clicked row on the mouse-down
    iRow=UPH_StringGrid->Selection.Top;                                         // :2060（V912 :2207）
    if (!(iRow>0 && iRow<11))                                                   // :2061（V912 :2208） Row介於1~10間
        return E023_Refuse(ck, "bad-row", "906 cShowBinSelect.cpp:2061 (V912 :2208) if(iRow>0 && iRow<11)", "row " + E023_Int(iRow) + " is not a record row (1..10)");
    for (std::size_t c = 0; c < q.cells.size(); ++c)                            // [W906] port-only: the record the operator saw is still there
        if (std::string(UPH_StringGrid->Cells[(int)c][iRow].c_str()) != q.cells[c])
            return E023_Refuse(ck, "stale-view", "[W906] port-only", "row " + E023_Int(iRow) + " column " + E023_Int((long)c) + " is now \"" +
                               UPH_StringGrid->Cells[(int)c][iRow].c_str() + "\", the page showed \"" + q.cells[c] + "\" -- reload and try again",
                               "這一列已經變了（可能剛寫入新的 UPH 紀錄），請重新整理再試");
    if (!q.hasAnswer) {                                                         // :2063（V912 :2210） Application->MessageBoxA(text, caption, MB_OKCANCEL)
        std::printf("[%s] golden reached the OK/Cancel box (906 cShowBinSelect.cpp:2063, V912 :2210), row %d -> needConfirm\n", ck, iRow);
        return std::string("{\"executed\":false,\"needConfirm\":true,\"buttons\":\"okcancel\",\"prompt\":[\"Do you want to delete this record?\",\"Confirm\"],\"row\":") +
               E023_Int(iRow) + ",\"goldenLine\":" + E023_Q("906 cShowBinSelect.cpp:2063 (V912 :2210)") + "}";
    }
    if (!q.ok) {                                                                // :2063（V912 :2210） != IDOK -> nothing
        std::printf("[%s] Cancel -> nothing deleted (golden :2063 (V912 :2210))\n", ck);
        return std::string("{\"executed\":true,\"deleted\":false,\"answer\":\"cancel\",\"row\":") + E023_Int(iRow) + "}";
    }
    for(int i=iRow; i<=10; i++)                                                 // :2065（V912 :2212） 將row[i+1]的值放到row[i]
    {
        UPH_StringGrid->Cells[0][i]=UPH_StringGrid->Cells[0][i+1];
        UPH_StringGrid->Cells[1][i]=UPH_StringGrid->Cells[1][i+1];
        UPH_StringGrid->Cells[2][i]=UPH_StringGrid->Cells[2][i+1];
        UPH_StringGrid->Cells[3][i]=UPH_StringGrid->Cells[3][i+1];
        if(IniConfig.bVTESTFunction==true)                                      // :2071（V912 :2218） RogerYang 20250224 偉測需求 新增三列信息,耗時,數量,site
        {
            UPH_StringGrid->Cells[4][i]=UPH_StringGrid->Cells[4][i+1];
            UPH_StringGrid->Cells[5][i]=UPH_StringGrid->Cells[5][i+1];
            UPH_StringGrid->Cells[6][i]=UPH_StringGrid->Cells[6][i+1];
        }
    }

    for(int i=1; i<=10; i++)                                                    // :2079（V912 :2226） 重新計算UPH的平均值
    {
        try
        {
            if(UPH_StringGrid->Cells[3][i]!="")
            {
                iTotalUPH+=atoi(AnsiString(UPH_StringGrid->Cells[3][i]).c_str());
                iCount++;
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "UPH_StringGridDblClick");
        }
    }
    RunInfo.iAvgUPH=(iCount==0)?0:iTotalUPH/iCount;                             // :2094（V912 :2241）
    UPH_StringGrid->Cells[3][12]=RunInfo.iAvgUPH;                               // :2095（V912 :2242）
    std::printf("[%s] golden UPH_StringGridDblClick (906 cShowBinSelect.cpp:2054, V912 :2201) deleted row %d; RunInfo.iAvgUPH=%s over %d record(s)\n",
                ck, iRow, RunInfo.iAvgUPH.c_str(), iCount);
    return std::string("{\"executed\":true,\"deleted\":true,\"answer\":\"ok\",\"row\":") + E023_Int(iRow) +
           ",\"avgUPH\":" + E023_Q(RunInfo.iAvgUPH.c_str()) + ",\"records\":" + E023_Int(iCount) +
           ",\"goldenLine\":" + E023_Q("906 cShowBinSelect.cpp:2054-2099 (V912 :2201-2246)") + "}";
}

}  // namespace

// ---- SB-4: golden sbCopyRecipeClick :2845-2850（V912 :2994-2999） -> TfMain::RunBatchCopyRecipe 906 main.cpp:34444-34451（V912 :35596-35603） ---------------------------------
// [W906] Steven Q66 = B (1002 08:0x) (as E-021's W906_E021_RecordNameRefused): \ / : * ? " < > | or ".." in the recipe name -> refused.  true = refused.
bool W906_E023_RecipeNameRefused(const std::string& name, std::string* why)
{
    if (name.find("..") != std::string::npos) {
        if (why) *why = "the recipe name contains \"..\"";
        return true;
    }
    for (std::size_t i = 0; i < name.size(); ++i)
        if (std::strchr("\\/:*?\"<>|", name[i])) {
            if (why) *why = std::string("the recipe name contains '") + name[i] + "' (path characters \\ / : * ? \" < > | are refused)";
            return true;
        }
    return false;
}

// golden target_folder "D:\\Run"; seam W906_COPYRECIPE_ROOT (unset = the golden literal)
std::string W906_E023_CopyRecipeRoot()
{
    const char* e = std::getenv("W906_COPYRECIPE_ROOT");
    std::string r = (e && *e) ? std::string(e) : std::string("D:\\Run");
    for (std::size_t i = 0; i < r.size(); ++i)
        if (r[i] == '/') r[i] = '\\';                                           // [W906] seam only: cmd's mkdir reads '/' as a switch
    return r;
}

struct W906_E023_CopyResult {
    std::string cmd1, cmd2, target;
    int rc1 = -1, rc2 = -1;
};

// golden 906 main.cpp:34444-34451（V912 :35596-35603） TfMain::RunBatchCopyRecipe(AnsiString source_folder) -- line by line; only target_folder goes through the seam
void W906_E023_RunBatchCopyRecipe(AnsiString source_folder, W906_E023_CopyResult* out)
{
    AnsiString command="", target_folder =W906_E023_CopyRecipeRoot().c_str();   // :34446（V912 :35598） golden target_folder ="D:\\Run"
    command.sprintf("if not exist %s mkdir %s", target_folder .c_str(), target_folder .c_str());   // :34447（V912 :35599）
    out->cmd1 = command.c_str();
    out->rc1 = system(command.c_str());                                         // :34448（V912 :35600）
    command.sprintf("xcopy /y \"%s\\*\" \"%s\\\" /s /e", source_folder.c_str(),target_folder.c_str());   // :34449（V912 :35601）
    out->cmd2 = command.c_str();
    out->rc2 = system(command.c_str());                                         // :34450（V912 :35602）
    out->target = target_folder.c_str();
}

// golden 906 cShowBinSelect.cpp:2845-2850（V912 :2994-2999） TfShowBinSelect::sbCopyRecipeClick
void W906_E023_sbCopyRecipeClick(W906_E023_CopyResult* out)
{
    // :2847（V912 :2996） sbCopyRecipe->Enabled=false; -- the facade has no sbCopyRecipe (see the banner: the page + WebCmdGuard)
    W906_E023_RunBatchCopyRecipe(AnsiString().sprintf("%s%s", DataPath.c_str(), fMain->cbSetupFileName->Text.c_str()), out);   // :2848（V912 :2997）
    // :2849（V912 :2998） sbCopyRecipe->Enabled=true;
}

namespace {
std::string E023_CopyRecipe()
{
    static const char* const ck = "E023-SB4";
    TfShowBinSelect* f = fShowBinSelect;
    if (!f || !fMain || !fMain->cbSetupFileName)
        return E023_Refuse(ck, "no-form", "", "fShowBinSelect / fMain / fMain->cbSetupFileName is NULL");
    if (!E023_Shown(f))
        return E023_Refuse(ck, "not-open", kNotOpenLine, "BinSelect is not open", "BinSelect 視窗沒有開著");
    std::string why;
    const std::string name = fMain->cbSetupFileName->Text.c_str();
    if (W906_E023_RecipeNameRefused(name, &why))
        return E023_Refuse(ck, "recipe-name", "[W906] Steven Q66 = B (1002 08:0x), stricter than golden (906 cShowBinSelect.cpp:2848, V912 :2997)", why,
                           "配方名稱含有路徑字元（\\ / : * ? \" < > | 或 ..），沒有複製");
    W906_E023_CopyResult r;
    W906_E023_sbCopyRecipeClick(&r);
    std::printf("[%s] golden sbCopyRecipeClick (906 cShowBinSelect.cpp:2845, V912 :2994) -> RunBatchCopyRecipe (906 main.cpp:34444, V912 :35596): recipe \"%s\" -> %s "
                "(mkdir rc %d, xcopy rc %d)\n", ck, name.c_str(), r.target.c_str(), r.rc1, r.rc2);
    return std::string("{\"executed\":true,\"recipe\":") + E023_Q(name) + ",\"source\":" + E023_Q(std::string(DataPath.c_str()) + name) +
           ",\"target\":" + E023_Q(r.target) + ",\"commands\":[" + E023_Q(r.cmd1) + "," + E023_Q(r.cmd2) + "],\"rc\":[" + E023_Int(r.rc1) + "," +
           E023_Int(r.rc2) + "],\"goldenLine\":" + E023_Q("906 cShowBinSelect.cpp:2845-2850 (V912 :2994-2999) -> 906 main.cpp:34444-34451 (V912 :35596-35603)") + "}";
}
}  // namespace

namespace ht9045 {
namespace sjson {
// JsonBridge/ChanAction.cpp:346 declares this at block scope (same namespace) and calls it for every act.showBinSelect.*.
std::string W906_ShowBinSelectAct(const std::string& cmd, const std::string& payloadJson)
{
    const char* ck = cmd == "act.showBinSelect.autoClean" ? "E023-SB1" : cmd == "act.showBinSelect.uphDblClick" ? "E023-SB3"
                   : cmd == "act.showBinSelect.copyRecipe" ? "E023-SB4" : "E023";
    if (cmd == "act.showBinSelect.clearCount")
        return E023_Refuse(ck, "wrong-route", "tools/wb_serve.cpp:4826", "act.showBinSelect.clearCount has its own wb_serve arm (WebShowBinSelect.cpp)");
    if (cmd != "act.showBinSelect.state" && cmd != "act.showBinSelect.autoClean" && cmd != "act.showBinSelect.uphDblClick" &&
        cmd != "act.showBinSelect.copyRecipe")
        return E023_Refuse(ck, "unknown-action", "", "act.showBinSelect.* has state, autoClean, uphDblClick and copyRecipe (clearCount: own arm); got " + cmd);
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return E023_Refuse(ck, "bad-payload", "", "value must be a JSON object string");
    }
    std::string out;
    try {
        if (cmd == "act.showBinSelect.state") out = E023_State();
        else if (cmd == "act.showBinSelect.autoClean") out = E023_AutoClean();
        else if (cmd == "act.showBinSelect.copyRecipe") out = E023_CopyRecipe();
        else {
            E023UphReq q;
            std::string why;
            out = E023_ParseUph(root, &q, &why) ? W906_E023_UphDblClick(q) : E023_Refuse(ck, "bad-payload", "", why);
        }
    } catch (const std::exception& e) {
        out = E023_Refuse(ck, "handler-failed", "", std::string("exception: ") + e.what());
    } catch (...) {
        out = E023_Refuse(ck, "handler-failed", "", "non-std exception");
    }
    cJSON_Delete(root);
    return out;
}
}  // namespace sjson
}  // namespace ht9045
