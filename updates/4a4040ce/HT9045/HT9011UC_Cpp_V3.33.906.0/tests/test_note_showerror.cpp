// ===========================================================================
//  tests/test_note_showerror.cpp
//
//  AI(W906-SHOWERR) 20260929: golden ShowErrorMessage's alarm-record half (note.cpp:538 / :794 / :802-818 / :839-868)
//  + TfNote::ErrShowToForm (:4307-4509) + CheckRecordJamDrivingRecord (:391-436), forms/fNote_ShowError.cpp (ht9045_sm);
//  the wb_serve host calls W906_ShowErrorMessageRecordLikeGolden from ForwardShowErrorMessage.
//
//    [1] JAM0101 (lower case in) -> recorded; edErrorCode "JAM0101", Edit3 "01", edUnitName "Input Arm", AlarmType 1,
//        sAlarmMes = the AlarmCodeList text, iDuplicateError 0; the EventTracker csv of the day has the text
//    [2] duplicate + errPart -> sAlarmMes "<text> : <errPart> (Again!!)", iDuplicateError 1
//    [3] WAR0205 -> AlarmType 2, "Output Arm";  [4] MES0301 -> AlarmType 3, "Index Unit"
//    [5] JAM9999 (not in AlarmCodeList) -> recorded, AlarmType 0, UnitName "", "Unknown Alarm Code"
//    [6] fNote->fShow -> not recorded (golden :814-818), fields unchanged
//    [7] InitialOK false -> not recorded (golden :538-542), fields unchanged
//    [8] iDuplicateError 4 (bMaintanceMode) and 3 (fContact->fShow)
//    [9] CheckRecordJamDrivingRecord: bC09_CarRecord + JAM -> bCarRecordTimeStart; MES -> not
//    [10] canary ShowErrorMessage (no hook) records errPart into W906_ShowErrorMessage_LastErrPart; Reset clears it
//  Writes only through the common.cpp log seams (EventTracker / HANDLER LOG / RMS), which ctest points into
//  machine_log_scratch -- the guard below refuses to run anywhere else.
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "forms/fNote.h"
#include "forms/fMain.h"
#include "cMyDB.h"            // AlarmUnit[] / iAlarmUnitTotal
#include "common.h"           // as9045LogPath / asSaveEventLogPath / asProductionLogPath
#include "cmydef.h"           // InitialOK / sAlarmMes / bMaintanceMode / bCarRecordTimeStart / K_RETRY / ON_LINE / SystemYear
#include "LastSet.h"
#include "Config.h"
#include "MachineType.h"
#include "atester_shims.h"    // fContact
#include "w906_ctest_guard.h"

bool W906_ShowErrorMessageRecordLikeGolden(AnsiString Code, bool bDuplicateErr, AnsiString errPart);   // forms/fNote_ShowError.cpp
extern bool W906_ShowErrorMessage_Recorded;
extern int  iDuplicateError;                                                  // golden note.cpp:84 (forms/fNote_ShowError.cpp)
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr, AnsiString errPart);   // canary_support.cpp (canary_support.h cannot share a TU with cMyDB.h)
extern AnsiString W906_ShowErrorMessage_LastErrPart;
void W906_ShowErrorMessage_Reset();

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_note_showerror.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)
static bool Eq(const AnsiString& a, const char* b) { return std::strcmp(a.c_str(), b) == 0; }

static bool FileHas(const std::string& path, const char* needle)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s.find(needle) != std::string::npos;
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NoteShowError", rt))
        return 2;
    CHECK(fNote != 0);
    CHECK(fMain != 0);
    if (fNote == 0 || fMain == 0) { std::printf("FAIL: no fNote / fMain\n"); return 1; }

    // the boot state MyDBUpdateDB leaves (cMyDB.cpp:2190-2225), just the rows this test needs
    fMain->AlarmCodeMap["JAM0101"] = "Input arm pick up error";
    fMain->AlarmCodeMap["WAR0205"] = "Output arm warning";
    fMain->AlarmCodeMap["MES0301"] = "Index message";
    if (fMain->UnitNameMap == 0) fMain->UnitNameMap = new TStringList();
    fMain->UnitNameMap->Clear();
    for (int i = 0; i < iAlarmUnitTotal; i++) fMain->UnitNameMap->Add(AlarmUnit[i]);

    const bool saveInit = InitialOK, saveMaint = bMaintanceMode, saveCar = IniConfig.bC09_CarRecord, saveO06 = IniConfig.bO06SaveLogTimePeriod;
    const int  saveTester = LastSet.iTester, saveCC = CUSTOMER_CODE;
    InitialOK = true;  bMaintanceMode = false;  IniConfig.bC09_CarRecord = false;  IniConfig.bO06SaveLogTimePeriod = false;
    LastSet.iTester = ON_LINE;
    if (CUSTOMER_CODE == CC_KYEC_LEE) CUSTOMER_CODE = 0;
    fNote->fShow = false;  if (fContact) fContact->fShow = false;

    std::printf("[1] JAM0101 -> recorded\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("jam0101", false, " ") == true);
    CHECK(W906_ShowErrorMessage_Recorded == true);
    CHECK(Eq(fNote->edErrorCode->Text, "JAM0101"));
    CHECK(Eq(fNote->Edit3->Text, "01"));
    CHECK(Eq(fNote->edUnitName->Text, "Input Arm"));
    CHECK(fNote->AlarmType == 1);
    CHECK(Eq(sAlarmMes, "Input arm pick up error"));
    CHECK(iDuplicateError == 0);
    {
        char day[64];
        std::snprintf(day, sizeof(day), "\\ASE log\\%04d\\%02d\\%02d\\", (int)SystemYear, (int)SystemMonth, (int)SystemDate);
        char name[64];
        std::snprintf(name, sizeof(name), "@%04d_%02d_%02d_EventTracker.csv", (int)SystemYear, (int)SystemMonth, (int)SystemDate);
        const std::string tracker = std::string(as9045LogPath.c_str()) + day + IniConfig.SocketHandlerID.c_str() + name;
        std::printf("    EventTracker: %s\n", tracker.c_str());
        CHECK(FileHas(tracker, "Input arm pick up error"));
    }

    std::printf("[2] duplicate + errPart\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM0101", true, "DoInArm") == true);
    CHECK(Eq(sAlarmMes, "Input arm pick up error : DoInArm (Again!!)"));
    CHECK(iDuplicateError == 1);

    std::printf("[3] WAR0205 -> AlarmType 2, Output Arm\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("WAR0205", false, "x") == true);
    CHECK(fNote->AlarmType == 2);
    CHECK(Eq(fNote->edUnitName->Text, "Output Arm"));
    CHECK(Eq(sAlarmMes, "Output arm warning : x"));

    std::printf("[4] MES0301 -> AlarmType 3, Index Unit\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("MES0301", false, " ") == true);
    CHECK(fNote->AlarmType == 3);
    CHECK(Eq(fNote->edUnitName->Text, "Index Unit"));

    std::printf("[5] JAM9999 not in AlarmCodeList -> Unknown\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM9999", false, " ") == true);
    CHECK(fNote->AlarmType == 0);
    CHECK(Eq(fNote->edUnitName->Text, ""));
    CHECK(Eq(fNote->Edit3->Text, "99"));
    CHECK(Eq(sAlarmMes, "Unknown Alarm Code"));

    std::printf("[6] fNote->fShow -> not recorded\n");
    CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM0101", false, " ") == true);
    fNote->fShow = true;
    CHECK(W906_ShowErrorMessageRecordLikeGolden("WAR0205", false, " ") == false);
    CHECK(W906_ShowErrorMessage_Recorded == false);
    CHECK(Eq(fNote->edUnitName->Text, "Input Arm"));
    CHECK(fNote->AlarmType == 1);
    fNote->fShow = false;

    std::printf("[7] InitialOK false -> not recorded\n");
    InitialOK = false;
    CHECK(W906_ShowErrorMessageRecordLikeGolden("MES0301", false, " ") == false);
    CHECK(Eq(fNote->edUnitName->Text, "Input Arm"));
    InitialOK = true;

    std::printf("[8] iDuplicateError 4 / 3\n");
    bMaintanceMode = true;
    CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM0101", false, " ") == true);
    CHECK(iDuplicateError == 4);
    bMaintanceMode = false;
    if (fContact) {
        fContact->fShow = true;
        CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM0101", false, " ") == true);
        CHECK(iDuplicateError == 3);
        fContact->fShow = false;
    }

    std::printf("[9] CheckRecordJamDrivingRecord\n");
    IniConfig.bC09_CarRecord = true;  bCarRecordTimeStart = false;
    CHECK(W906_ShowErrorMessageRecordLikeGolden("MES0301", false, " ") == true);
    CHECK(bCarRecordTimeStart == false);
    CHECK(W906_ShowErrorMessageRecordLikeGolden("JAM0101", false, " ") == true);
    CHECK(bCarRecordTimeStart == true);
    IniConfig.bC09_CarRecord = false;  bCarRecordTimeStart = false;

    std::printf("[10] canary records errPart\n");
    W906_ShowErrorMessage_Reset();
    CHECK(Eq(W906_ShowErrorMessage_LastErrPart, ""));
    CHECK(ShowErrorMessage("JAM0101", K_RETRY, 0, false, "PartX") == K_RETRY);
    CHECK(Eq(W906_ShowErrorMessage_LastErrPart, "PartX"));
    W906_ShowErrorMessage_Reset();
    CHECK(Eq(W906_ShowErrorMessage_LastErrPart, ""));

    InitialOK = saveInit;  bMaintanceMode = saveMaint;  IniConfig.bC09_CarRecord = saveCar;  IniConfig.bO06SaveLogTimePeriod = saveO06;
    LastSet.iTester = saveTester;  CUSTOMER_CODE = saveCC;
    std::printf("\n%d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
