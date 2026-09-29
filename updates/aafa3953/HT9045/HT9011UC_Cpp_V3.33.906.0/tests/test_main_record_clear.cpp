// =============================================================================
//  test_main_record_clear.cpp -- S119: Main.Record CLEAR / AseRecordMemo double-click
//  (JsonBridge/actions/MainRecordClear.cpp; golden spbClearRecordClick main.cpp:31109-31138,
//  meShuttle2DblClick :29471-29475).
//
//  AI(W906-S119) 20260927.  Suite name (add_test): MainRecord_Clear
//
//    1. bShowMainDebugRecord==false (the default; golden :31111): the body returns at once, the counts stay, and
//       act.main.clearRecord answers guard "debug-record-off";
//    2. on + {"dryRun":true} (or a payload that does not parse): guards only, nothing cleared;
//    3. on + no payload / {"dryRun":false}: ArmData[0..2] counts are zeroed (TArm::ClearALLCT), executed:true,
//       UpdateRecordScreen reported as skipped (gate R1, St01 S113);
//    4. act.main.meShuttle2Dbl: dryRun keeps the lines; otherwise meShuttle1 / meShuttle2 are really emptied
//       (TfMainMemo stores lines since AI(W906-MEMO) 42ea830b);
//    5. R1 hook W906_UpdateRecordScreenBody: NULL -> not called (reported skipped); installed -> called once with true
//       after the clears; not called when the debug record is off or on dryRun.
//  Memory only (ArmData / fProductionInfo / IniConfig); no file is written by the code under test.
//  AI(W906-TEST-SAFE) 20260928 (St02-E helper): NOT true since cMyDB P4 -- DoClearRecordAction / DoMeShuttle2DblAction call
//  LogAppend (MainRecordClear.cpp :125 / :163) -> golden RecordProcess -> HANDLER LOG / EventLogTxt rows under the cMyDB
//  log roots.  Those are ctest's machine_log_scratch only when ctest runs it, so the test refuses to run (exit 2, nothing
//  called) outside ctest's redirect roots (st02_test_containment.h).
// =============================================================================
#include "JsonBridge/actions/MainRecordClear.h"

#include "Config.h"      // IniConfig.bShowMainDebugRecord
#include "cSocket.h"     // ArmData[3]
#include "forms/fMain.h" // fMain->meShuttle1 / meShuttle2 (part 4)

#include <cstdio>
#include <string>

#include "st02_test_containment.h"   // AI(W906-TEST-SAFE) 20260928 (St02-E helper)

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

static int g_urCalls = 0, g_urAttr = -1;
static bool g_urAfterClear = false;
static void FakeUpdateRecordScreen(bool attr)   // stands in for St01's W906_TfMain_UpdateRecordScreen
{
    ++g_urCalls;
    g_urAttr = attr ? 1 : 0;
    g_urAfterClear = ArmData[0]->GetTotalCT() == 0 && ArmData[1]->GetTotalCT() == 0 && ArmData[2]->GetTotalCT() == 0;
}

static void Seed()
{
    for (int k = 0; k < 3; ++k) ArmData[k]->ClearALLCT();
    ArmData[0]->SetPassCT(0, 0, 3);
    ArmData[0]->SetFailCT(0, 0, 1);
    ArmData[1]->SetPassCT(1, 2, 5);
    ArmData[2]->SetFailCT(0, 1, 2);
}

static bool Seeded()
{
    return ArmData[0]->GetTotalCT() == 4 && ArmData[1]->GetTotalCT() == 5 && ArmData[2]->GetTotalCT() == 2;
}

static bool Cleared()
{
    return ArmData[0]->GetTotalCT() == 0 && ArmData[1]->GetTotalCT() == 0 && ArmData[2]->GetTotalCT() == 0;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);   // AI(W906-TEST-SAFE) 20260928 (St02-E helper): unbuffered -- a crash report shows the exact last line
    printf("MainRecord_Clear\n");
    // AI(W906-TEST-SAFE) 20260928 (St02-E helper): containment first -- run by hand (no ctest environment),
    //   the golden RecordProcess behind every act.* LogAppend (JsonBridge/EventLog.cpp) would write the machine's real D:\HT9045_Log / D:\RMS trees.
    //   Refuse, exit 2, before any Handler code (st02_test_containment.h).
    if (!W906TestInsideCtestRoots("MainRecord_Clear"))
        return 2;
    const bool oldShow = IniConfig.bShowMainDebugRecord;

    // 1
    IniConfig.bShowMainDebugRecord = false;
    Seed();
    CHECK(Seeded(), "1. seeded ArmData[0..2] counts");
    CHECK(W906_SpbClearRecordClick() == kCrDebugRecordOff && Seeded(), "1. debug record off: golden returns, counts stay");
    std::string r = ht9045::sjson::DoClearRecordAction("");
    CHECK(Has(r, "\"executed\":false") && Has(r, "\"guard\":\"debug-record-off\"") && Seeded(),
          "1. act.main.clearRecord: guard debug-record-off");

    // 2
    IniConfig.bShowMainDebugRecord = true;
    r = ht9045::sjson::DoClearRecordAction("{\"dryRun\":true}");
    CHECK(Has(r, "\"executed\":false") && Has(r, "\"dryRun\":true") && Seeded(), "2. dryRun true: nothing cleared");
    r = ht9045::sjson::DoClearRecordAction("not json");
    CHECK(Has(r, "\"executed\":false") && Seeded(), "2. a payload that does not parse: preview only");

    // 3
    r = ht9045::sjson::DoClearRecordAction("");
    CHECK(Has(r, "\"executed\":true") && Has(r, "UpdateRecordScreen(true)") && Cleared(),
          "3. no payload: ArmData[0..2] cleared, UpdateRecordScreen reported skipped");
    Seed();
    r = ht9045::sjson::DoClearRecordAction("{\"dryRun\":false}");
    CHECK(Has(r, "\"executed\":true") && Cleared(), "3. dryRun false: cleared");
    Seed();
    CHECK(W906_SpbClearRecordClick() == kCrDone && Cleared(), "3. the golden body directly");

    // 4
    fMain->meShuttle1->Clear();
    fMain->meShuttle2->Clear();
    fMain->meShuttle1->Lines->Add("shuttle 1 line");
    fMain->meShuttle2->Lines->Add("shuttle 2 line a");
    fMain->meShuttle2->Lines->Add("shuttle 2 line b");
    CHECK(fMain->meShuttle1->Lines->Count == 1 && fMain->meShuttle2->Lines->Count == 2, "4. the memos store lines (AI(W906-MEMO))");
    r = ht9045::sjson::DoMeShuttle2DblAction("{\"dryRun\":true}");
    CHECK(Has(r, "\"executed\":false") && fMain->meShuttle1->Lines->Count == 1 && fMain->meShuttle2->Lines->Count == 2,
          "4. dryRun: not run, the lines stay");
    r = ht9045::sjson::DoMeShuttle2DblAction("");
    CHECK(Has(r, "\"executed\":true") && fMain->meShuttle1->Lines->Count == 0 && fMain->meShuttle2->Lines->Count == 0,
          "4. act.main.meShuttle2Dbl empties meShuttle2 and meShuttle1 (golden 906 :28427-28431)");

    // 5
    CHECK(W906_UpdateRecordScreenBody == 0, "5. nothing installs the R1 hook by default");
    r = ht9045::sjson::DoClearRecordAction("");
    CHECK(Has(r, "\"skipped\":[\"UpdateRecordScreen(true)"), "5. NULL hook: UpdateRecordScreen reported skipped");
    W906_UpdateRecordScreenBody = &FakeUpdateRecordScreen;
    Seed();
    IniConfig.bShowMainDebugRecord = false;
    W906_SpbClearRecordClick();
    CHECK(g_urCalls == 0, "5. debug record off: hook not called");
    IniConfig.bShowMainDebugRecord = true;
    ht9045::sjson::DoClearRecordAction("{\"dryRun\":true}");
    CHECK(g_urCalls == 0 && Seeded(), "5. dryRun: hook not called");
    r = ht9045::sjson::DoClearRecordAction("");
    CHECK(g_urCalls == 1 && g_urAttr == 1 && g_urAfterClear, "5. installed: called once with true, after the clears");
    CHECK(Has(r, "\"skipped\":[]") && Has(r, "\"UpdateRecordScreen(true)\"]"), "5. reported as done, not skipped");
    W906_UpdateRecordScreenBody = 0;

    IniConfig.bShowMainDebugRecord = oldShow;
    for (int k = 0; k < 3; ++k) ArmData[k]->ClearALLCT();
    printf("MainRecord_Clear: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
