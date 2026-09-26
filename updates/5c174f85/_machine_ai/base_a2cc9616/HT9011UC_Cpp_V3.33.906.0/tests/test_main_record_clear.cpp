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
//    4. act.main.meShuttle2Dbl: executed (the memos are TfMainMemo stand-ins, so Clear() is a no-op for now);
//       dryRun does not run it.
//  Memory only (ArmData / fProductionInfo / IniConfig); no file is written by the code under test.
// =============================================================================
#include "JsonBridge/actions/MainRecordClear.h"

#include "Config.h"      // IniConfig.bShowMainDebugRecord
#include "cSocket.h"     // ArmData[3]

#include <cstdio>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

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
    printf("MainRecord_Clear\n");
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
    r = ht9045::sjson::DoMeShuttle2DblAction("");
    CHECK(Has(r, "\"executed\":true") && Has(r, "TfMainMemo"), "4. act.main.meShuttle2Dbl runs (stand-in memos)");
    r = ht9045::sjson::DoMeShuttle2DblAction("{\"dryRun\":true}");
    CHECK(Has(r, "\"executed\":false"), "4. dryRun: not run");

    IniConfig.bShowMainDebugRecord = oldShow;
    for (int k = 0; k < 3; ++k) ArmData[k]->ClearALLCT();
    printf("MainRecord_Clear: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
