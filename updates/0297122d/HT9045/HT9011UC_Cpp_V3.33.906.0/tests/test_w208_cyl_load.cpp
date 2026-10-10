// =============================================================================
//  test_w208_cyl_load.cpp -- AI(W906-W208) 20261009 (Ifor01)
//
//  W-208 MR 1 (POOL-11 ①後半, TO_IFOR 1009 18:5x): Pre-Alarm Cylinder backend per golden 913 -- read side.
//    [1] fStartCondition is live (golden HT9045.cpp:206 CreateForm; end of cStartCondition.cpp), fShow starts false
//    [2] fStartCondition->LoadCylinderLife() (golden 913 cStartCondition.cpp:1765-1827) on a sandbox MachineLife.ini:
//        counts / alarms / reset count+time / the On/Off time records; a cylinder with no keys gets golden's values --
//        including golden's own argument order: ReadWriteIni(...,3000000,true,true,3000000,5000000) passes `true` as the
//        DEFAULT (=1) and checks [0, 5000000], so a missing *_OnOffCountAlarm / *_TimeOutCountAlarm reads 1 and is written
//        back as 1 (the HT9050 snapshot MachineLife.ini has 31 `_OnOffCountAlarm=1` lines from the BCB program); the grid resize
//    [3] W906_StartConditionTimer1Tick (golden 913 main.cpp:3278 in Timer1Timer): nothing before InitialOK (:2768); with the page
//        showing, the header is set on the first call and the rows on every 11th call (:1728-1731); nothing while fShow is false
//    [4] source pins (argv[1] = port root, read only): cinitial.cpp N1-G2e opened between InitHontechHardware and LoadMachineRecord
//        (golden 913 cinitial.cpp:5844-5846); tools/wb_serve.cpp calls the tick at both Timer1 sites (main loop / modal wait)
//    [5] the machine's D:\HT9045\system\MachineLife.ini is not touched (asMachineLifePath points at %TEMP% before any read)
// =============================================================================
#include "MachineDefine.h"            // the include hub first, as cStartCondition.cpp does (forms/fStartCondition.h needs AnsiString)
#include "forms/fStartCondition.h"
#include "cmydef.h"
#include "common.h"
#include "LastSet.h"
#include "MachineType.h"
#include "mycylin.h"
#include "w906_ctest_guard.h"   // W906TestRequireCtestRedirects: refuses to run outside ctest's redirect roots
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
static bool Near(double a, double b) { return a - b < 1e-6 && b - a < 1e-6; }
static bool Stamp(const char* p, WIN32_FILE_ATTRIBUTE_DATA* d) { return ::GetFileAttributesExA(p, GetFileExInfoStandard, d) != 0; }
static bool Has(const std::string& s, const char* line) { return s.find(line) != std::string::npos; }

int main(int argc, char** argv)
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("W208_CylinderLifeLoad", rt))
        return 2;
    std::printf("W208_CylinderLifeLoad\n");
    const char* real = "D:\\HT9045\\system\\MachineLife.ini";
    Check(std::strcmp(asMachineLifePath.c_str(), real) == 0, "0. asMachineLifePath is golden's D:\\HT9045\\system\\MachineLife.ini before the sandbox swap");
    WIN32_FILE_ATTRIBUTE_DATA before; std::memset(&before, 0, sizeof(before));
    const bool realExisted = Stamp(real, &before);

    char tmp[MAX_PATH]; ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32]; std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string dir = std::string(tmp) + "ht9045_w208a_" + stamp;
    ::CreateDirectoryA(dir.c_str(), 0);
    const std::string ini = dir + "\\MachineLife.ini";
    asMachineLifePath = AnsiString(ini.c_str());
    {
        std::ofstream f(ini.c_str(), std::ios::binary);
        f << "[CylinderLife]\r\nW208_A_OnOffCount=1234\r\nW208_A_TimeOutCount=5\r\n"
             "[CylinderAlarm]\r\nW208_A_OnOffCountAlarm=4000000\r\nW208_A_OnTimeAlarm=1.25\r\nW208_A_OffTimeAlarm=2.5\r\nW208_A_TimeOutCountAlarm=7000\r\n"
             "W208_C_OnOffCountAlarm=9000000\r\nW208_C_TimeOutCountAlarm=-5\r\n"
             "[CylinderResetCount]\r\nW208_A_ResetCount=3\r\n"
             "[CylinderResetTime]\r\nW208_A_ResetTime=2026-10-01 08:00:00\r\n"
             "[OnTimeRecord]\r\nW208_A_OnTime_0=0.111\r\nW208_A_OnTime_1=0.222\r\n"
             "[OffTimeRecord]\r\nW208_A_OffTime_0=0.333\r\n";
    }

    Check(fStartCondition != 0 && fStartCondition->fShow == false, "1. fStartCondition is live (golden HT9045.cpp:206) and fShow starts false");

    for (int i = 0; i < MaxCylinderItem; i++) Cylinder[i].Enable = false;
    const char* names[3] = {"W208_A", "W208_B", "W208_C"};
    for (int i = 0; i < 3; i++) { Cylinder[i].CylinderName = names[i]; Cylinder[i].Enable = true; Cylinder[i].OnSenEnable = true; Cylinder[i].OffSenEnable = true; }

    fStartCondition->LoadCylinderLife();
    TMyCylinder &A = Cylinder[0], &B = Cylinder[1], &C = Cylinder[2];
    Check(A.iOnOffCount == 1234 && A.iTimeOutCount == 5, "2a. [CylinderLife] counts read (golden :1773-1776)");
    Check(A.iOnOffCountAlarm == 4000000 && Near(A.dOnTimeAlarm, 1.25) && Near(A.dOffTimeAlarm, 2.5) && A.iTimeOutCountAlarm == 7000,
          "2b. [CylinderAlarm] read (golden :1779-1786)");
    Check(A.iResetCount == 3 && A.sResetTime == "2026-10-01 08:00:00", "2c. reset count / time read (golden :1789-1792)");
    Check(A.sListOnTime->Count == 2 && A.sListOffTime->Count == 1 && Near(A.dOnTime, 0.111) && Near(A.dOffTime, 0.333)
          && A.sListOnTime->Strings[0] == "0.222" && A.sListOnTime->Strings[1] == "0.111",
          "2d. On/Off time records: non-zero ones added (newest first), record 0 is the current time (golden :1795-1812)");
    Check(B.iOnOffCount == 0 && B.iTimeOutCount == 0 && B.iResetCount == 0 && Near(B.dOnTimeAlarm, 999.99) && Near(B.dOffTimeAlarm, 999.99),
          "2e. no keys: counts 0, time alarms 999.99 (golden defaults)");
    Check(B.iOnOffCountAlarm == 1 && B.iTimeOutCountAlarm == 1,
          "2f. no keys: *_OnOffCountAlarm / *_TimeOutCountAlarm read 1 -- golden passes `true` as ReadWriteIni's DEFAULT (:1780／:1786)");
    const std::string after = Slurp(ini);
    Check(Has(after, "W208_B_OnOffCountAlarm=1") && Has(after, "W208_B_TimeOutCountAlarm=1") && Has(after, "W208_B_ResetCount=0"),
          "2g. ...and written back into MachineLife.ini as 1 (CheckAndReadIniData seeds a missing key)");
    Check(!Has(after, "W208_B_OnOffCount=") && !Has(after, "W208_B_OnTimeAlarm="), "2h. plain ReadIniData keys are not written (counts, time alarms)");
    Check(C.iOnOffCountAlarm == 5000000 && C.iTimeOutCountAlarm == 0, "2i. alarm counts clamped to [0, 5000000] / [0, 10000] (CheckRange, golden argument order)");
    Check(fStartCondition->strngrdCylinderView->RowCount == 4 && fStartCondition->strngrdCylinderView->ColCount == eCylPrAlarmItemTotal,
          "2j. first load sizes the grid: RowCount = enabled cylinders + 1, ColCount = eCylPrAlarmItemTotal (golden :1818-1823)");

    // [3] the Timer1 call site
    LastSet.iLanguageCountry = 0;
    SystemInitialOK = true;
    fStartCondition->fShow = true;
    InitialOK = false;
    for (int k = 0; k < 30; k++) W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylName][0] == "" && fStartCondition->sCylinderData[eCylName][1] == "",
          "3a. InitialOK false: the tick returns before UpdateCylinderScreen (golden main.cpp:2768)");
    InitialOK = true;
    W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylName][0] == "Cylinder Name" && fStartCondition->sCylinderData[eCylName][1] == "",
          "3b. first call with the page showing: header only (golden :1676-1724)");
    for (int k = 0; k < 9; k++) W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylName][1] == "", "3c. calls 2..10: still no rows (golden :1729 ct++<10)");
    W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylName][1] == "W208_A" && fStartCondition->sCylinderData[eCylOnOffCnt][1] == "1234"
          && fStartCondition->sCylinderData[eCylOnOffCntAlarm][2] == "1" && fStartCondition->sCylinderData[eCylName][3] == "W208_C",
          "3d. 11th call fills one row per enabled cylinder (golden :1733-1762)");
    fStartCondition->fShow = false;
    A.iOnOffCount = 99;
    for (int k = 0; k < 30; k++) W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylOnOffCnt][1] == "1234", "3e. page not showing: no refresh (golden :1673)");
    fStartCondition->fShow = true;
    for (int k = 0; k < 11; k++) W906_StartConditionTimer1Tick();
    Check(fStartCondition->sCylinderData[eCylOnOffCnt][1] == "99", "3f. page showing again: the next 11th call refreshes");

    if (argc > 1) {
        const std::string root = argv[1];
        const std::string ci = Slurp(root + "/cinitial.cpp"), wb = Slurp(root + "/tools/wb_serve.cpp");
        Check(!ci.empty() && !wb.empty(), "4. sources readable");
        const size_t ih = ci.find("void InitialHandler()"), hw = ci.find("InitHontechHardware();", ih);
        const size_t op = ci.find("#if 1 // was: #if 0 -- opened AI(W906-W208) 20261009 (Ifor01): N1-G2e reason expired", ih);
        const size_t ld = ci.find("fStartCondition->LoadCylinderLife();", ih), mr = ci.find("LoadMachineRecord();", ih);
        Check(ih != std::string::npos && hw < op && op < ld && ld < mr && ci.find("#if 0", ci.find('\n', op)) > mr,
              "4a. InitialHandler: InitHontechHardware -> N1-G2e opened -> LoadCylinderLife -> LoadMachineRecord (golden 913 cinitial.cpp:5844-5846)");
        const std::string tick = "extern void W906_StartConditionTimer1Tick(); W906_StartConditionTimer1Tick();";
        size_t n = 0; for (size_t p = wb.find(tick); p != std::string::npos; p = wb.find(tick, p + 1)) n++;
        Check(n == 2 && wb.find("W906_MainRecordTimer1Tick(); " + tick) != std::string::npos
              && wb.find("W906_ModalTimer1Segments(); }  { " + tick) != std::string::npos,
              "4b. tools/wb_serve.cpp: the tick at both Timer1 sites -- main loop after W906_MainRecordTimer1Tick, modal wait after W906_ModalTimer1Segments");
    } else std::printf("  (no argv[1]: source pins skipped)\n");

    WIN32_FILE_ATTRIBUTE_DATA now; std::memset(&now, 0, sizeof(now));
    const bool realExists = Stamp(real, &now);
    Check(realExists == realExisted && (!realExists || (std::memcmp(&before.ftLastWriteTime, &now.ftLastWriteTime, sizeof(FILETIME)) == 0
                                                       && before.nFileSizeLow == now.nFileSizeLow)),
          "5. the machine's D:\\HT9045\\system\\MachineLife.ini is untouched");

    CloseIniFile();
    ::DeleteFileA(ini.c_str()); ::RemoveDirectoryA(dir.c_str());
    std::printf("W208_CylinderLifeLoad: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
