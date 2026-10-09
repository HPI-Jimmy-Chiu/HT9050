// ===========================================================================
//  tests/test_st02_w205_asm.cpp -- W-205 L03 (W-195 (2) general items, golden 913): the ASM hot-plate borrow pieces of golden 913
//  r913 log "L03 ASM 熱盤借料帳錯亂 (WAR0150)" (RogerYang 20260914, VTEST HHT-83).  AI(W906-W195) 20261009 (St02-E).
//
//  Memory only: the motors stay unconstructed (CopyToTray's ASM path returns before Mot.Tray.PordRec / IO; SetTraySingleData only sets
//  Tray.Data / iTarget), the Process row of RecordProcess goes to the ctest log scratch (the W906_*_ROOT redirects of every ctest,
//  re-checked below), argv[1] = the tree root (read only, source pins).
//
//    [A] L03a golden 913 mykitsuck.cpp:1638-1650 through the real TMyKitSuck::CopyToTray: the backups come from the nozzle that
//        borrowed the cell (InArmSiteMapData) when the cell matches; otherwise from [iSuckR][iSuckC] (912 / V906 before).
//    [B] L03b golden 913 uhome.cpp:1410-1433 W906_AsmHomeReleaseBorrow_St02: HOME frees the HAS_NULL_IC placeholder of the borrowed
//        cell, clears the borrow flags / step / InArmSiteMapData, one Process row; guards as golden.
//    [C] source pins: mykitsuck.cpp CopyToTray reads [iBkR][iBkC] three times after the helper call; uhome.cpp ProcessMotorHome calls
//        the HOME helper after the ASM_Home log block's #endif (outside #ifndef SOFT_SIMULTE) and before SW[SwAirOff].
// ===========================================================================
#include "MachineDefine.h"
#include "aHotPlateSubstrate.h"     // InArmSuck (TMyKitSuck), InArmSiteMapData
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "Config.h"
#include "CosFunction.h"
#include "common.h"
#include "forms/fTemp_Set.h"        // the W-195 rule: create fTemp_Set first (see main)

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern AnsiString ExString;        // cMyDB.h:143 (not included: its default-argument clash)
void W906_AsmHomeReleaseBorrow_St02();

static int g_fail = 0;
static int g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool Forbidden(const char* p)
{
    std::string s = p ? p : "";
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    if (s.find("\\machine_log_scratch") != std::string::npos) return false;   // AI(W906-W205) 20261009 (laptop, batch 146 integration): the ctest redirect counts as contained wherever the build dir lives -- same one line as test_st02_w205_citems.cpp (c59acf66); the laptop's gate builds under D:\HT9045\.claude\worktrees\... (same containment rule as test_c10_hana_rms.cpp:254)
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}

static std::string Body(const std::string& src, const char* head)
{
    const size_t fn = src.find(head);
    if (fn == std::string::npos) return "";
    const size_t end = src.find("\n}", fn);
    return end == std::string::npos ? "" : src.substr(fn, end - fn);
}

static void SeedSuck()
{
    for (int r = 0; r < _MAX_SUCK_ROW_ITEM; ++r)
        for (int c = 0; c < _MAX_SUCK_COL_ITEM; ++c) {
            InArmSuck.iWhichShuttleAutoSitemapping[r][c] = 0;
            InArmSuck.iWhichKitAutoSitemapping[r][c]     = 0;
            InArmSuck.iHotCountAutoSitemapping[r][c]     = 0;
        }
    InArmSuck.iWhichShuttleAutoSitemapping[0][0] = 21; InArmSuck.iWhichKitAutoSitemapping[0][0] = 22; InArmSuck.iHotCountAutoSitemapping[0][0] = 23;
    InArmSuck.iWhichShuttleAutoSitemapping[2][5] = 7;  InArmSuck.iWhichKitAutoSitemapping[2][5] = 3;  InArmSuck.iHotCountAutoSitemapping[2][5] = 9;
    InArmSuck.iWhichShuttleBackup = -5; InArmSuck.iWhichKitBackup = -5; InArmSuck.HotCount = -5;
    InArmSuck.Item[0][0] = HAS_IC;
}

static bool Backups(int sht, int kit, int hot)
{
    std::printf("     backups: shuttle %d kit %d hot %d\n", InArmSuck.iWhichShuttleBackup, InArmSuck.iWhichKitBackup, InArmSuck.HotCount);
    return InArmSuck.iWhichShuttleBackup == sht && InArmSuck.iWhichKitBackup == kit && InArmSuck.HotCount == hot;
}

static void PartA()
{
    std::printf("\n-- [A] L03a: CopyToTray reads the borrowing nozzle --\n");
    check(InArmSuck.bLed[0][0] == false, "precondition: no LED proxy on nozzle [0][0] (SetItemData stays memory only)");
    const bool savRun = bRunAutoSiteMapping, savSave = bAutoSiteMapHotplateSave;
    const strAUTOSITEMAP savMap = InArmSiteMapData;
    bRunAutoSiteMapping = true; bAutoSiteMapHotplateSave = true;
    const int cellSentinel = MOT[MMPlate1].Tray.Data[4][1];

    // cursor reset to (0,0) by ReStartAutoSiteMapping, borrow was made by nozzle [2][5] at plate cell R1 C4
    SeedSuck();
    InArmSiteMapData.UpdateData(0, 1, 4, 2, 5);
    InArmSuck.CopyToTray(0, 0, NULL_IC, MOT[MMPlate1], 1, 4, HAS_NULL_IC);
    check(Backups(7, 3, 9), "A1 cell matches -> Shuttle / Kit / hot count of the borrowing nozzle [2][5] (golden 913 :1640-1650)");
    check(iAutoSiteMapHotplatePlateC == 4 && iAutoSiteMapHotplatePlateR == 1 && InArmSuck.Item[0][0] == NULL_IC &&
          MOT[MMPlate1].Tray.Data[4][1] == cellSentinel,
          "A1 rest of the ASM path unchanged: plate R/C kept, nozzle item set, early return before the tray write (golden :1651-1656)");

    SeedSuck();
    InArmSiteMapData.UpdateData(0, 1, 4, 2, 5);
    InArmSuck.CopyToTray(0, 0, NULL_IC, MOT[MMPlate1], 1, 5, HAS_NULL_IC);
    check(Backups(21, 22, 23), "A2 another cell (C5 vs C4) -> the cursor nozzle [0][0], as 912");

    SeedSuck();
    InArmSiteMapData.UpdateData(0, 1, 4, -1, 5);
    InArmSuck.CopyToTray(0, 0, NULL_IC, MOT[MMPlate1], 1, 4, HAS_NULL_IC);
    check(Backups(21, 22, 23), "A3 recorded nozzle row out of range (-1) -> the cursor nozzle [0][0]");

    SeedSuck();
    InArmSiteMapData.ClearData();
    InArmSuck.CopyToTray(0, 0, NULL_IC, MOT[MMPlate1], 1, 4, HAS_NULL_IC);
    check(Backups(21, 22, 23), "A4 no borrow recorded (ClearData) -> the cursor nozzle [0][0]");

    bRunAutoSiteMapping = savRun; bAutoSiteMapHotplateSave = savSave; InArmSiteMapData = savMap;
}

static void PartB()
{
    std::printf("\n-- [B] L03b: HOME ends an interrupted ASM borrow --\n");
    const bool savJcet = CosFunction.bUSEJCETSiteMapMode, savAsm = IniConfig.bI21EnableASM;
    const bool savSave = bAutoSiteMapHotplateSave, savPick = bAutoSiteMapHasPickHP;
    const int  savStep = iResetSiteMappingStep;
    const strAUTOSITEMAP savMap = InArmSiteMapData;
    const int  savData = MOT[MMPlate1 + 1].Tray.Data[3][2], savSite = MOT[MMPlate1 + 1].Tray.SiteMapData[3][2];

    CosFunction.bUSEJCETSiteMapMode = true; IniConfig.bI21EnableASM = true;
    MOT[MMPlate1 + 1].Tray.Data[3][2] = HAS_NULL_IC; MOT[MMPlate1 + 1].Tray.SiteMapData[3][2] = 5;
    InArmSiteMapData.UpdateData(1, 2, 3, 0, 1);
    bAutoSiteMapHotplateSave = true; bAutoSiteMapHasPickHP = true; iResetSiteMappingStep = 1; ExString = "";
    W906_AsmHomeReleaseBorrow_St02();
    std::printf("     cell %d sitemap %d | save %d pick %d step %d | iP %d | ExString \"%s\"\n",
                MOT[MMPlate1 + 1].Tray.Data[3][2], MOT[MMPlate1 + 1].Tray.SiteMapData[3][2], (int)bAutoSiteMapHotplateSave,
                (int)bAutoSiteMapHasPickHP, iResetSiteMappingStep, InArmSiteMapData.iP, ExString.c_str());
    check(MOT[MMPlate1 + 1].Tray.Data[3][2] == NULL_IC && MOT[MMPlate1 + 1].Tray.SiteMapData[3][2] == 0,
          "B1 placeholder HAS_NULL_IC of plate 2 R2 C3 -> NULL_IC, SiteMapData 0 (golden 913 :1421-1424; Data is [C][R])");
    check(std::string(ExString.c_str()) == "ASM Home : release borrowed cell P1 R2 C3",
          "B1 one Process row \"ASM Home : release borrowed cell P1 R2 C3\" (golden :1425-1426)");
    check(!bAutoSiteMapHotplateSave && !bAutoSiteMapHasPickHP && iResetSiteMappingStep == 0 &&
          InArmSiteMapData.iP == -1 && InArmSiteMapData.iSuckR == -1,
          "B1 flags false, step 0, InArmSiteMapData cleared (golden :1429-1432)");

    MOT[MMPlate1 + 1].Tray.Data[3][2] = HAS_IC; MOT[MMPlate1 + 1].Tray.SiteMapData[3][2] = 5;
    InArmSiteMapData.UpdateData(1, 2, 3, 0, 1);
    bAutoSiteMapHotplateSave = true; bAutoSiteMapHasPickHP = true; iResetSiteMappingStep = 1; ExString = "";
    W906_AsmHomeReleaseBorrow_St02();
    check(MOT[MMPlate1 + 1].Tray.Data[3][2] == HAS_IC && MOT[MMPlate1 + 1].Tray.SiteMapData[3][2] == 5 && std::string(ExString.c_str()).empty() &&
          !bAutoSiteMapHotplateSave && !bAutoSiteMapHasPickHP && iResetSiteMappingStep == 0 && InArmSiteMapData.iP == -1,
          "B2 cell already holds an IC -> cell and log untouched, the transaction is still ended (golden :1421 / :1429-1432)");

    MOT[MMPlate1 + 1].Tray.Data[3][2] = HAS_NULL_IC;
    InArmSiteMapData.UpdateData(1, 2, 3, 0, 1);
    bAutoSiteMapHotplateSave = true; bAutoSiteMapHasPickHP = true; iResetSiteMappingStep = 1;
    CosFunction.bUSEJCETSiteMapMode = false;
    W906_AsmHomeReleaseBorrow_St02();
    const bool offJcet = MOT[MMPlate1 + 1].Tray.Data[3][2] == HAS_NULL_IC && bAutoSiteMapHotplateSave && bAutoSiteMapHasPickHP &&
                         iResetSiteMappingStep == 1 && InArmSiteMapData.iP == 1;
    CosFunction.bUSEJCETSiteMapMode = true; IniConfig.bI21EnableASM = false;
    W906_AsmHomeReleaseBorrow_St02();
    const bool offAsm = MOT[MMPlate1 + 1].Tray.Data[3][2] == HAS_NULL_IC && bAutoSiteMapHotplateSave && iResetSiteMappingStep == 1 &&
                        InArmSiteMapData.iP == 1;
    check(offJcet && offAsm, "B3 not JCET site-map mode, or I21 ASM off -> nothing changes (golden :1414-1416)");

    IniConfig.bI21EnableASM = true;
    InArmSiteMapData.UpdateData(2, 2, 3, 0, 1);
    W906_AsmHomeReleaseBorrow_St02();
    check(MOT[MMPlate1 + 1].Tray.Data[3][2] == HAS_NULL_IC && !bAutoSiteMapHotplateSave && InArmSiteMapData.iP == -1,
          "B4 plate index out of range (iP 2) -> no cell touched, transaction ended (golden :1418)");

    CosFunction.bUSEJCETSiteMapMode = savJcet; IniConfig.bI21EnableASM = savAsm;
    bAutoSiteMapHotplateSave = savSave; bAutoSiteMapHasPickHP = savPick; iResetSiteMappingStep = savStep; InArmSiteMapData = savMap;
    MOT[MMPlate1 + 1].Tray.Data[3][2] = savData; MOT[MMPlate1 + 1].Tray.SiteMapData[3][2] = savSite;
}

static void PartC(const char* src)
{
    std::printf("\n-- [C] source pins --\n");
    if (!src) { check(false, "argv[1] = the tree root"); return; }
    const std::string k = Body(Read(std::string(src) + "/mykitsuck.cpp"), "\nvoid TMyKitSuck::CopyToTray(");
    const size_t call = k.find("W906_AsmBorrowSuckRC_St02(TrayR, TrayC, iBkR, iBkC);");
    const size_t s1 = k.find("=iWhichShuttleAutoSitemapping   [iBkR][iBkC];");
    const size_t s2 = k.find("=iWhichKitAutoSitemapping       [iBkR][iBkC];");
    const size_t s3 = k.find("=iHotCountAutoSitemapping       [iBkR][iBkC];");
    check(call != std::string::npos && s1 != std::string::npos && s2 != std::string::npos && s3 != std::string::npos &&
          call < s1 && s1 < s2 && s2 < s3 && k.find("AutoSitemapping       [iSuckR][iSuckC]") == std::string::npos &&
          k.find("AutoSitemapping   [iSuckR][iSuckC]") == std::string::npos,
          "C1 mykitsuck.cpp CopyToTray: helper call, then the three backups read [iBkR][iBkC] (golden 913 :1638-1650)");

    const std::string h = Body(Read(std::string(src) + "/uhome.cpp"), "\nbool ProcessMotorHome(");
    const size_t asmLog = h.find("ASM_Home\"");
    const size_t endif  = asmLog == std::string::npos ? std::string::npos : h.find("#endif", asmLog);
    const size_t hcall  = h.find("{ void W906_AsmHomeReleaseBorrow_St02(); W906_AsmHomeReleaseBorrow_St02(); }");
    const size_t air    = asmLog == std::string::npos ? std::string::npos : h.find("if(SW[SwAirOff].Enable==true)", asmLog);
    const size_t ifndef = (endif == std::string::npos || hcall == std::string::npos) ? std::string::npos : h.find("#if", endif);
    check(asmLog != std::string::npos && endif != std::string::npos && hcall != std::string::npos && air != std::string::npos &&
          endif < hcall && hcall < air && (ifndef == std::string::npos || ifndef > hcall) &&
          h.find("W906_AsmHomeReleaseBorrow_St02()", hcall + std::strlen("{ void W906_AsmHomeReleaseBorrow_St02(); W906_AsmHomeReleaseBorrow_St02(); }")) == std::string::npos,
          "C2 uhome.cpp ProcessMotorHome: one call after the ASM_Home block's #endif (outside #ifndef SOFT_SIMULTE) and before SwAirOff (golden 913 :1408-1435)");
}

int main(int argc, char** argv)
{
    const char* roots[] = { as9045LogPath.c_str(), asSaveEventLogPath.c_str() };
    for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]); ++i)
        if (Forbidden(roots[i])) {
            std::printf("STOP: log root %s is a machine path -- the ctest W906_*_ROOT redirect is missing; nothing called\n", roots[i]);
            return 1;
        }
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136)
    GetTimeInfo();

    PartA();
    PartB();
    PartC(argc > 1 ? argv[1] : 0);

    std::printf("test_st02_w205_asm: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
