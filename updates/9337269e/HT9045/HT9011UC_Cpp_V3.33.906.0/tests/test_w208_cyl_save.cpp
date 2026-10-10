// =============================================================================
//  test_w208_cyl_save.cpp -- AI(W906-W208) 20261009 (Ifor01)
//
//  W-208 part 2 (POOL-11 ①後半, TO_IFOR 1009 18:5x): Pre-Alarm Cylinder write side per golden 913 cStartCondition.cpp.
//    [1] SaveCylinderLife (:1829-1888) writes MachineLife.ini -- the time alarms / time records only for a cylinder with that sensor
//    [2] step 0, W906_SC_CylinderMenuProbe: OnSelectCell (:1626-1651) picks the pmCylinder item; Reset Count asks golden's YES/NO
//        texts, the four "Set ... alarm" open the keypad with golden's ranges; a by-passed / sensor-less cylinder gets no item; the
//        fixed row/column gets nothing; nothing is changed or written by step 0
//    [3] step 1, W906_SC_CylinderMenuAnswer (mniResetOnOffCountClick :1495-1590): NO does nothing; YES clears the counts, bumps the
//        reset count/time, records it (NewRecordProcess) and saves; keypad text is range-checked like golden's keypad; Cancel keeps
//        the box text (and golden still saves it); text the keypad cannot type is refused; "Set ... standard" acts at once
//    [4] GATE (SC4) opened: UpdateCylinderScreen (:1739-1746) clears a negative count, records it and saves
//    [5] the machine's D:\HT9045\system\MachineLife.ini is not touched (asMachineLifePath points at %TEMP% before any read)
// =============================================================================
#include "MachineDefine.h"            // the include hub first, as cStartCondition.cpp does (forms/fStartCondition.h needs AnsiString)
#include "forms/fStartCondition.h"
#include "cmydef.h"
#include "cMyDB.h"
#include "common.h"
#include "LastSet.h"
#include "MachineType.h"
#include "mycylin.h"
#include "w906_ctest_guard.h"   // W906TestRequireCtestRedirects: NewRecordProcess writes the cMyDB logs -- only into ctest's scratch
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
static bool Near(double a, double b) { return a - b < 1e-6 && b - a < 1e-6; }
static bool Stamp(const char* p, WIN32_FILE_ATTRIBUTE_DATA* d) { return ::GetFileAttributesExA(p, GetFileExInfoStandard, d) != 0; }
static void Refresh() { for (int k = 0; k < 11; k++) fStartCondition->UpdateCylinderScreen(); }   // golden: every 11th call (:1728-1731)
static int RdI(const char* grp, const char* key, int def) { return ReadIniData(asMachineLifePath, grp, key, def); }
static double RdD(const char* grp, const char* key, double def) { return ReadIniData(asMachineLifePath, grp, key, def); }

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("W208_CylinderLifeSave", rt))
        return 2;
    std::printf("W208_CylinderLifeSave\n");
    const char* real = "D:\\HT9045\\system\\MachineLife.ini";
    WIN32_FILE_ATTRIBUTE_DATA before; std::memset(&before, 0, sizeof(before));
    const bool realExisted = Stamp(real, &before);

    char tmp[MAX_PATH]; ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32]; std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string dir = std::string(tmp) + "ht9045_w208b_" + stamp;
    ::CreateDirectoryA(dir.c_str(), 0);
    const std::string ini = dir + "\\MachineLife.ini";
    asMachineLifePath = AnsiString(ini.c_str());
    { std::ofstream f(ini.c_str(), std::ios::binary); f << "[CylinderLife]\r\n"; }

    // three cylinders: A both sensors, B no On sensor, C both sensors but Pre-Alarm by-passed for times
    for (int i = 0; i < MaxCylinderItem; i++) Cylinder[i].Enable = false;
    const char* names[3] = {"W208_A", "W208_B", "W208_C"};
    for (int i = 0; i < 3; i++) { Cylinder[i].CylinderName = names[i]; Cylinder[i].Enable = true; Cylinder[i].OnSenEnable = true; Cylinder[i].OffSenEnable = true; Cylinder[i].bCylPreAlarmByPassT = false; }
    TMyCylinder &A = Cylinder[0], &B = Cylinder[1], &C = Cylinder[2];
    B.OnSenEnable = false;
    C.bCylPreAlarmByPassT = true;
    fStartCondition->LoadCylinderLife();                                       // sizes the grid (rows 1..3), seeds the alarm keys
    A.iOnOffCount = 10; A.iTimeOutCount = 2; A.iOnOffCountAlarm = 300; A.iTimeOutCountAlarm = 40; A.dOnTimeAlarm = 1.5; A.dOffTimeAlarm = 2.5;
    A.iResetCount = 4; A.sResetTime = "2026-10-01 08:00:00"; A.AddOnTime(0.25); A.AddOnTime(0.75); A.AddOffTime(0.5);
    B.dOnTimeAlarm = 9.0; B.AddOnTime(0.3);

    // [1]
    fStartCondition->SaveCylinderLife();
    Check(RdI("CylinderLife", "W208_A_OnOffCount", -1) == 10 && RdI("CylinderLife", "W208_A_TimeOutCount", -1) == 2
          && RdI("CylinderAlarm", "W208_A_OnOffCountAlarm", -1) == 300 && RdI("CylinderAlarm", "W208_A_TimeOutCountAlarm", -1) == 40,
          "1a. SaveCylinderLife: counts and count alarms (golden :1836-1845)");
    Check(Near(RdD("CylinderAlarm", "W208_A_OnTimeAlarm", -1), 1.5) && Near(RdD("CylinderAlarm", "W208_A_OffTimeAlarm", -1), 2.5)
          && RdI("CylinderResetCount", "W208_A_ResetCount", -1) == 4 && ReadIniData(asMachineLifePath, "CylinderResetTime", "W208_A_ResetTime", AnsiString("")) == "2026-10-01 08:00:00",
          "1b. time alarms, reset count / time (golden :1847-1863)");
    Check(Near(RdD("OnTimeRecord", "W208_A_OnTime_0", -1), 0.75) && Near(RdD("OnTimeRecord", "W208_A_OnTime_1", -1), 0.25)
          && Near(RdD("OffTimeRecord", "W208_A_OffTime_0", -1), 0.5), "1c. the On/Off time records, newest first (golden :1865-1884)");
    Check(Near(RdD("CylinderAlarm", "W208_B_OnTimeAlarm", -7), -7) && Near(RdD("OnTimeRecord", "W208_B_OnTime_0", -7), -7)
          && Near(RdD("CylinderAlarm", "W208_B_OffTimeAlarm", -7), 999.99),
          "1d. no On sensor: no On time alarm / record written (golden :1847／:1869), the Off ones are");

    // the grid text the menu works on (golden: the page is showing)
    LastSet.iLanguageCountry = 0; SystemInitialOK = true; fStartCondition->fShow = true;
    Refresh();
    Check(fStartCondition->sCylinderData[eCylName][1] == "W208_A" && fStartCondition->sCylinderData[eCylName][3] == "W208_C", "2. the grid shows the three cylinders");

    // [2] step 0
    const std::string file0 = Slurp(ini);
    W906CylPrompt p;
    Check(W906_SC_CylinderMenuProbe(eCylOnOffCnt, 1, &p) && p.item == 0 && p.kind == 1 && p.sEn == "W208_A Sure To reset count?"
          && p.sCh == "W208_A 確定要歸零次數？", "2a. OnOff Cnt.: item 0 Reset Count asks golden's YES/NO (:1509-1511)");
    Check(W906_SC_CylinderMenuProbe(eCylResetCount, 1, &p) && p.item == 0 && p.kind == 1, "2b. Reset Count column: the same item 0");
    Check(W906_SC_CylinderMenuProbe(eCylOnOffCntAlarm, 1, &p) && p.item == 1 && p.kind == 2 && (p.iFunction & N_INTEGER) && p.bCheckRange
          && Near(p.min, 0) && Near(p.max, 6000000) && p.current == "300", "2c. on/off count alarm: keypad N_INTEGER 0..6000000, opens with the cell text (:1537-1538)");
    Check(W906_SC_CylinderMenuProbe(eCylOffTimeAlarm, 1, &p) && p.item == 3 && p.kind == 2 && (p.iFunction & N_DOUBLE) && Near(p.max, 999999.999),
          "2d. off time alarm: keypad N_DOUBLE 0..999999.999 (:1554)");
    Check(W906_SC_CylinderMenuProbe(eCylTimeOutCntAlarm, 1, &p) && p.item == 4 && p.kind == 2 && Near(p.max, 10000), "2e. time out count alarm: keypad 0..10000 (:1582)");
    Check(W906_SC_CylinderMenuProbe(eCylAvgOnTime, 1, &p) && p.item == 5 && p.kind == 0, "2f. Avg. On T.: item 5 asks nothing (:1564-1568)");
    Check(!W906_SC_CylinderMenuProbe(eCylOnTimeAlarm, 2, &p) && p.item == -1, "2g. no On sensor: no item on On T. alarm (:1641)");
    Check(!W906_SC_CylinderMenuProbe(eCylOnTimeAlarm, 3, &p) && !W906_SC_CylinderMenuProbe(eCylAvgOffTime, 3, &p)
          && W906_SC_CylinderMenuProbe(eCylOnOffCnt, 3, &p) && p.item == 0, "2h. by-passed cylinder: no time items, Reset Count still there (:1641-1649)");
    Check(!W906_SC_CylinderMenuProbe(eCylOnOffCnt, 0, &p) && !W906_SC_CylinderMenuProbe(eCylName, 1, &p) && !W906_SC_CylinderMenuProbe(eCylOnOffCnt, 4, &p),
          "2i. the fixed row / column and a row past the grid select nothing (VCL)");
    Check(Slurp(ini) == file0 && A.iOnOffCount == 10 && Near(A.dOnTimeAlarm, 1.5) && A.iOnOffCountAlarm == 300,
          "2j. step 0 changed nothing and wrote nothing");

    // [3] step 1
    Check(W906_SC_CylinderMenuAnswer(eCylOnOffCnt, 1, false, "", false) && A.iOnOffCount == 10 && A.iResetCount == 4 && Slurp(ini) == file0,
          "3a. Reset Count, NO: nothing (:1512)");
    Check(W906_SC_CylinderMenuAnswer(eCylOnOffCnt, 1, true, "", false) && A.iOnOffCount == 0 && A.iTimeOutCount == 0 && A.iResetCount == 5
          && A.sResetTime.Length() == 19 && A.sResetTime != "2026-10-01 08:00:00", "3b. Reset Count, YES: counts 0, reset count +1, reset time now (:1514-1523)");
    Check(ExString == "W208_A clear PreAlarm count by manually", "3c. ...recorded through NewRecordProcess (:1516-1518)");
    Check(RdI("CylinderLife", "W208_A_OnOffCount", -1) == 0 && RdI("CylinderResetCount", "W208_A_ResetCount", -1) == 5, "3d. ...and saved (:1588-1589)");
    Check(W906_SC_CylinderMenuAnswer(eCylResetCount, 1, true, "", false) && A.iResetCount == 0 && RdI("CylinderResetCount", "W208_A_ResetCount", -1) == 0
          && ExString == "W208_A clear reset count by manually", "3e. Reset Count column, YES: reset count 0, recorded, saved (:1525-1531)");
    Check(W906_SC_CylinderMenuAnswer(eCylOnOffCntAlarm, 1, false, "123456", false) && A.iOnOffCountAlarm == 123456
          && RdI("CylinderAlarm", "W208_A_OnOffCountAlarm", -1) == 123456, "3f. on/off count alarm typed 123456: set and saved (:1536-1541)");
    Check(W906_SC_CylinderMenuAnswer(eCylOnOffCntAlarm, 1, false, "9999999", false) && A.iOnOffCountAlarm == 6000000,
          "3g. 9999999 is clamped to the keypad's 6000000 (myQwertyKeyBoard :285-292)");
    Refresh();
    Check(W906_SC_CylinderMenuAnswer(eCylTimeOutCntAlarm, 1, false, "", true) && A.iTimeOutCountAlarm == 40 && RdI("CylinderAlarm", "W208_A_TimeOutCountAlarm", -1) == 40,
          "3h. Cancel: the box text (the cell) goes back and golden still uses and saves it (:1583-1586)");
    Check(!W906_SC_CylinderMenuAnswer(eCylTimeOutCntAlarm, 1, false, "12a", false) && A.iTimeOutCountAlarm == 40, "3i. text the keypad cannot type is refused, nothing changes");
    Check(W906_SC_CylinderMenuAnswer(eCylOnTimeAlarm, 1, false, "0.125", false) && Near(A.dOnTimeAlarm, 0.125)
          && Near(RdD("CylinderAlarm", "W208_A_OnTimeAlarm", -1), 0.125), "3j. on time alarm typed 0.125 (:1544-1549)");
    Check(W906_SC_CylinderMenuAnswer(eCylAvgOffTime, 1, false, "", false) && Near(A.dOffTimeAlarm, 0.5)
          && Near(RdD("CylinderAlarm", "W208_A_OffTimeAlarm", -1), 0.5), "3k. Avg. Off T.: off time alarm = the average at once, saved (:1574-1578)");
    Check(!W906_SC_CylinderMenuAnswer(eCylOnTimeAlarm, 2, false, "5", false) && Near(B.dOnTimeAlarm, 9.0), "3l. no item, no click (B has no On sensor)");

    // [4] GATE (SC4)
    A.iOnOffCount = -5;
    Refresh();
    Check(A.iOnOffCount == 0 && RdI("CylinderLife", "W208_A_OnOffCount", -1) == 0 && ExString == "W208_A clear PreAlarm count by over"
          && fStartCondition->sCylinderData[eCylOnOffCnt][1] == "0", "4. a negative count is cleared, recorded and saved on refresh (golden :1739-1746, SC4 opened)");

    WIN32_FILE_ATTRIBUTE_DATA now; std::memset(&now, 0, sizeof(now));
    const bool realExists = Stamp(real, &now);
    Check(realExists == realExisted && (!realExists || (std::memcmp(&before.ftLastWriteTime, &now.ftLastWriteTime, sizeof(FILETIME)) == 0
                                                       && before.nFileSizeLow == now.nFileSizeLow)),
          "5. the machine's D:\\HT9045\\system\\MachineLife.ini is untouched");

    CloseIniFile();
    ::DeleteFileA(ini.c_str()); ::RemoveDirectoryA(dir.c_str());
    std::printf("W208_CylinderLifeSave: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
