// ===========================================================================
//  tests/test_note_jamcount.cpp
//
//  AI(W906-J2) 20260926: golden TfNote::FormClose 的 Jam 計數（note.cpp:2528-2529 → CheckRecordJamType :344-389），
//  本體在 forms/fNote_JamCount.cpp（ht9045_sm），wb_serve 的兩個回答出口呼叫 W906_NoteJamCountOnClose(k)。
//
//    [1] JAM0508、AlarmType 1、答 RETRY ⇒ iJamCount[0..2]、iDayJamCount、iRecordJamRateByTime_JamCount 各 +1
//    [2] 答 TRAY END ⇒ 不計（:2523-2524 bLampTrayEnd=Select[3]）
//    [3] ShowErrorMessage(…, bDuplicateErr=true) 之後 ⇒ 不計（iDuplicateError==1，:802-803）；下一次 false ⇒ 又會計
//    [4] WAR（AlarmType 2）⇒ 不計
//    [5] iRealDummy=DUMMY ⇒ 不計
//    [6] JAM1201（UnitNo 12 ≥ 9）⇒ 不計；CUSTOMER_CODE=CC_AnalogDevice_Phil ⇒ 計（:353 bIncludAllUnit）
//    [7] CosFunction.bIncludeMTBA ⇒ 不計（G1 閘：fNote 沒有 sJamArea／sJamCode —— 這一格釘住今天的行為）
//    [8] VTEST（bVTESTFunction＋ON_LINE＋bLotStart＋bCheckFile）⇒ fMesSystem->iJamRateTotalForAlways 也 +1（:381-385）
//    [9] 直接呼叫：小寫 "jam0508" 會計（:350 UpperCase）；bCheckOnly=true ⇒ 回 false、不計（:366）
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include "forms/fNote.h"
#include "forms/fMesSystem.h"
#include "canary_support.h"
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "cmydef.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "Config.h"
#include "MachineType.h"

bool W906_CheckRecordJamType(AnsiString asJamCode, bool bCheckOnly);   // forms/fNote_JamCount.cpp
void W906_NoteJamCountOnClose(int k);

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_note_jamcount.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

struct Snap { int j0, j1, j2, day, rate, always; };
static Snap Take()
{
    Snap s = { LastSet.iJamCount[0], LastSet.iJamCount[1], LastSet.iJamCount[2], LastSet.iDayJamCount,
               iRecordJamRateByTime_JamCount, fMesSystem ? fMesSystem->iJamRateTotalForAlways : -1 };
    return s;
}
static bool Counted(const Snap& a, const Snap& b)
{
    return b.j0 == a.j0 + 1 && b.j1 == a.j1 + 1 && b.j2 == a.j2 + 1 && b.day == a.day + 1 && b.rate == a.rate + 1;
}
static bool Same(const Snap& a, const Snap& b)
{
    return b.j0 == a.j0 && b.j1 == a.j1 && b.j2 == a.j2 && b.day == a.day && b.rate == a.rate && b.always == a.always;
}
static void Arm(const char* code, int alarmType)
{
    fNote->edErrorCode->Text = AnsiString(code);
    fNote->AlarmType = alarmType;
}

int main()
{
    CHECK(fNote != 0);
    CHECK(fMesSystem != 0);
    if (fNote == 0) { std::printf("FAIL: no fNote\n"); return 1; }

    const int  saveReal = LastSet.iRealDummy, saveTester = LastSet.iTester, saveCC = CUSTOMER_CODE;
    const bool saveMTBA = CosFunction.bIncludeMTBA, saveLot = RunInfo.bLotStart, saveCheck = IniConfig.bCheckFile;
    const int  saveVT = IniConfig.bVTESTFunction;
    LastSet.iRealDummy = REALLY;
    CosFunction.bIncludeMTBA = false;
    if (CUSTOMER_CODE == CC_AnalogDevice_Phil) CUSTOMER_CODE = 0;
    IniConfig.bVTESTFunction = 0;
    W906_ShowErrorMessage_Reset();
    CHECK(W906_ShowErrorMessage_LastDuplicate == false);

    std::printf("[1] JAM0508 answered RETRY -> counted\n");
    Arm("JAM0508", 1);
    Snap a = Take();  W906_NoteJamCountOnClose(K_RETRY);  Snap b = Take();
    CHECK(Counted(a, b));  CHECK(b.always == a.always);

    std::printf("[2] answered TRAY END -> not counted\n");
    a = Take();  W906_NoteJamCountOnClose(K_TRAY_END);  b = Take();
    CHECK(Same(a, b));

    std::printf("[3] bDuplicateErr -> not counted; next non-duplicate call -> counted again\n");
    ShowErrorMessage("JAM0508", K_RETRY, 0, true, " ");
    CHECK(W906_ShowErrorMessage_LastDuplicate == true);
    Arm("JAM0508", 1);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Same(a, b));
    ShowErrorMessage("JAM0508", K_RETRY, 0);
    CHECK(W906_ShowErrorMessage_LastDuplicate == false);
    Arm("JAM0508", 1);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Counted(a, b));

    std::printf("[4] WAR (AlarmType 2) -> not counted\n");
    Arm("WAR0101", 2);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Same(a, b));

    std::printf("[5] DUMMY run -> not counted\n");
    LastSet.iRealDummy = DUMMY;
    Arm("JAM0508", 1);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Same(a, b));
    LastSet.iRealDummy = REALLY;

    std::printf("[6] JAM1201 (unit 12) -> not counted; AnalogDevice_Phil counts every unit\n");
    Arm("JAM1201", 1);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Same(a, b));
    CUSTOMER_CODE = CC_AnalogDevice_Phil;
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Counted(a, b));
    CUSTOMER_CODE = (saveCC == CC_AnalogDevice_Phil) ? 0 : saveCC;

    std::printf("[7] bIncludeMTBA -> not counted (G1 gate pins today's behaviour)\n");
    CosFunction.bIncludeMTBA = true;
    Arm("JAM0508", 1);
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Same(a, b));
    CosFunction.bIncludeMTBA = false;

    std::printf("[8] VTEST always-record -> iJamRateTotalForAlways +1 too\n");
    IniConfig.bVTESTFunction = 1;  LastSet.iTester = ON_LINE;  RunInfo.bLotStart = true;  IniConfig.bCheckFile = true;
    a = Take();  W906_NoteJamCountOnClose(K_RETRY);  b = Take();
    CHECK(Counted(a, b));  CHECK(b.always == a.always + 1);
    IniConfig.bVTESTFunction = 0;

    std::printf("[9] direct: lower-case code counts (UpperCase); bCheckOnly=true returns false and does not count\n");
    a = Take();  CHECK(W906_CheckRecordJamType("jam0508", false) == true);  b = Take();
    CHECK(Counted(a, b));
    a = Take();  CHECK(W906_CheckRecordJamType("JAM0508", true) == false);  b = Take();
    CHECK(Same(a, b));

    LastSet.iRealDummy = saveReal;  LastSet.iTester = saveTester;  CUSTOMER_CODE = saveCC;
    CosFunction.bIncludeMTBA = saveMTBA;  RunInfo.bLotStart = saveLot;  IniConfig.bCheckFile = saveCheck;  IniConfig.bVTESTFunction = saveVT;
    W906_ShowErrorMessage_Reset();

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
