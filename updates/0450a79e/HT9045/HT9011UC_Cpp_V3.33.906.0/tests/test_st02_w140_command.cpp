// =============================================================================
//  test_st02_w140_command.cpp -- W-140 (POOL-2): Command.cpp's SETTEMP / UPH? / SETSITEMAP_ gates lifted as golden 0618.
//
//  AI(W906-W140) 20261007 (St02-E).  Suite name (add_test): St02_W140Command.
//    [1] UPH? -- TfMain::UPHStrings = golden Command.cpp:12276-12284: fShowBinSelect->UPH_StringGrid->Cells[3][1], "0" when empty;
//    [2] SETTEMP -- TfMain::WriteSetTempStatus = golden :1623-1624 / :1640-1641: after SetTemp, ChangeTempMode(TempMode, ...)
//        really switches LastSet.iTemperature (both arms: the I38 SETTEMP-respond arm -- two calls, counted in a sandbox
//        slEventLog -- and the default arm), reply SETTINGOK; the I38 arm on a machine that is not ambient keeps the current mode
//        (golden leaves TempMode unset there; Command.cpp:1886 safe default);
//    [3] SETSITEMAP_ -- TfMain::SetSiteMapData = golden :5320: with no IC, ChangeToSiteMap("SINGLE1X1-1_") switches
//        TestIF_File.iTestMode to SingleSite and replies SETTINGOK (it replied SETTINGNG every time while gated).
//  The reply is captured through W906_TesterForward.SendMSG_CMD_Msg (forms/fMain.h:1406).  Machine files: CosFunction's setup-file
//  flags stay off (ChangeTempMode saves nothing), DataPath / LastDataPath / asTeachPath point at %TEMP%\ht9045_w140_<tick>, the
//  general ini is this test's scratch copy (tests/test_bootstrap.cpp; SetWorkParameter's ReadTechData needs it open), and the
//  event log goes to ctest's redirect roots.  The test refuses a path under D:\HT9045 that is not a build dir (\obj\v906\ rule).
// =============================================================================
#include "forms/fMain.h"
#include "forms/fShowBinSelect.h"
#include "MessageDef.h"
#include "LastSet.h"
#include "cprod.h"
#include "cmydef.h"
#include "common.h"
#include "Config.h"
#include "CosFunction.h"
#include "csystem.h"
#include "Public/MyStringList.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261008 (Ifor01): fTemp_Set (see main)

static int g_pass = 0, g_fail = 0;
static std::string g_got;
#define CHECK(cond, msg)                                                                                         \
    do {                                                                                                         \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got.c_str()); ++g_pass; }                               \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got.c_str(), __LINE__); ++g_fail; }      \
        g_got.clear();                                                                                           \
    } while (0)

static std::string g_reply;
static int g_replyCmd = -1;
static void CaptureReply(int cmd, AnsiString msg) { g_replyCmd = cmd; g_reply = msg.c_str(); }

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

static void Gpib(const char* text)
{
    static VM vm;
    std::memset(&vm, 0, sizeof(vm));
    std::strncpy(vm.cReturn, text, sizeof(vm.cReturn) - 1);
    HGpib2Handler = &vm;
    g_reply.clear();
    g_replyCmd = -1;
}

int main()
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261008 (Ifor01): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite, which calls fTemp_Set->InitialAddrToATC() since N1-G5
    printf("St02_W140Command\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_w140_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((root + "\\Data\\W140R").c_str(), 0);
    { FILE* f = std::fopen((root + "\\SetUp.inf").c_str(), "wb"); if (f) { std::fputs("W140R\r\n", f); std::fclose(f); } }
    DataPath     = AnsiString((root + "\\Data\\").c_str());
    LastDataPath = AnsiString((root + "\\SetUp.inf").c_str());
    asTeachPath  = AnsiString((root + "\\teach.ini").c_str());
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(std::string(asTeachPath.c_str())) ||
        UnderMachineTree(std::string(asGeneralPath.c_str())))
    {
        printf("  ABORT: a path is not in a sandbox (DataPath=%s teach=%s general=%s) -- nothing was called\n",
               DataPath.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
        return 2;
    }
    g_got = std::string("general=") + asGeneralPath.c_str();
    CHECK(fMain != 0 && fShowBinSelect != 0, "0. fMain / fShowBinSelect exist; paths are sandboxed");
    OpenGeneralIniFile();  { void EnsureArmOffsetObjects(); EnsureArmOffsetObjects(); }   //AI(W906-POOL2-TEMPOFS) 20261008 (St02-E): golden TfMain ctor allocates the offset arrays (main.cpp:2123-2139; cOffSet.cpp:166); ChangeTempMode now re-reads the offsets (MainTempMode.cpp:246)
    W906_TesterForward.SendMSG_CMD_Msg = &CaptureReply;
    CosFunction.bLastSetInSetUpFile = false;
    CosFunction.bFTPDownLoadTempModeBySetupFile = false;
    CosFunction.bDLLCommands = false;
    Temperature.bATC70Active = false;
    SystemStart = false;

    // [1] UPH?
    fShowBinSelect->UPH_StringGrid->Cells[3][1] = "533";
    AnsiString u = fMain->UPHStrings();
    g_got = u.c_str();
    CHECK(std::string(u.c_str()) == "533", "1. UPH? -> fShowBinSelect->UPH_StringGrid->Cells[3][1] (golden :12282-12283)");
    fShowBinSelect->UPH_StringGrid->Cells[3][1] = "";
    u = fMain->UPHStrings();
    g_got = u.c_str();
    CHECK(std::string(u.c_str()) == "0", "1. empty cell -> \"0\" (golden :12276-12278)");

    // [2] SETTEMP -- the default arm (iI38SETTEMPRespondSetTemp != 1): golden :1640-1641
    const bool noIc = !HasICUnderMachine();
    g_got = noIc ? "no IC" : "IC under machine";
    CHECK(noIc, "2. fixture: no IC under the machine (ChangeTempMode would refuse otherwise, MainTempMode.cpp:69-89)");
    IniConfig.iI38SETTEMPRespondSetTemp = 0;
    LastSet.iTemperature = Tempture_Ambient;
    Gpib("125");
    fMain->WriteSetTempStatus();
    g_got = "iTemperature=" + std::to_string(LastSet.iTemperature) + " reply=" + g_reply;
    CHECK(LastSet.iTemperature == Tempture_Hot && g_reply == "SETTINGOK", "2. SETTEMP 125 (default arm) -> ChangeTempMode(1): LastSet.iTemperature hot, SETTINGOK");
    Gpib("20");
    fMain->WriteSetTempStatus();
    g_got = "iTemperature=" + std::to_string(LastSet.iTemperature) + " reply=" + g_reply;
    CHECK(LastSet.iTemperature == Tempture_Ambient && g_reply == "SETTINGOK", "2. SETTEMP 20 -> ChangeTempMode(0): back to ambient");
    // the I38 arm (golden :1623-1624): from ambient, > 50 -> hot.  Golden calls ChangeTempMode twice on this arm (:1624 inside it,
    // :1641 after it); each call records "MES2151 Change Hot Mode" (MainTempMode.cpp) into slEventLog -- a sandbox one here -- so
    // the inner call is counted, not just the final state.
    slEventLog = new TMyStringList(AnsiString((root + "\\EventLogTxt").c_str()), "EventLogTxt", "Date, Time, UnitName, AlarmCode");
    const std::string evFile = std::string(slEventLog->GetFileName().c_str());
    if (UnderMachineTree(evFile) || Lower(evFile).find(Lower(root)) != 0) { printf("  ABORT: event log %s not in the sandbox\n", evFile.c_str()); return 2; }
    IniConfig.iI38SETTEMPRespondSetTemp = 1;
    LastSet.iTemperature = Tempture_Ambient;
    Gpib("125");
    fMain->WriteSetTempStatus();
    slEventLog->MySaveToFile();
    int hot = 0;
    {
        FILE* f = std::fopen(evFile.c_str(), "rb");
        std::string body;
        if (f) { char buf[4096]; size_t n; while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) body.append(buf, n); std::fclose(f); }
        for (size_t p = body.find("MES2151"); p != std::string::npos; p = body.find("MES2151", p + 1)) ++hot;
    }
    g_got = "iTemperature=" + std::to_string(LastSet.iTemperature) + " reply=" + g_reply + " MES2151 x" + std::to_string(hot);
    CHECK(LastSet.iTemperature == Tempture_Hot && g_reply == "SETTINGOK" && hot == 2,
          "2. I38 arm, SETTEMP 125 from ambient -> ChangeTempMode(1) twice (golden :1624 and :1641), hot, SETTINGOK");
    // I38 arm with the machine NOT ambient: golden leaves TempMode unset (:1605-1616); the port starts it from the current mode
    // (Command.cpp:1886, a safe default until Jimmy decides) -> the mode stays what it was
    LastSet.iTemperature = Tempture_Hot;
    Gpib("20");
    fMain->WriteSetTempStatus();
    g_got = "iTemperature=" + std::to_string(LastSet.iTemperature) + " reply=" + g_reply;
    CHECK(LastSet.iTemperature == Tempture_Hot && g_reply == "SETTINGOK",
          "2. I38 arm, machine already hot -> the mode stays hot (TempMode starts from the current mode, not an indeterminate value)");
    delete slEventLog;
    slEventLog = 0;
    IniConfig.iI38SETTEMPRespondSetTemp = 0;
    LastSet.iTemperature = Tempture_Ambient;

    // [3] SETSITEMAP_
    const int savedMode = TestIF_File.iTestMode, savedMap = TestIF_File.iSiteMap[0][0];
    TestIF_File.iTestMode = DualSite;
    Gpib("SINGLE1X1-1_");
    fMain->SetSiteMapData();
    g_got = "iTestMode=" + std::to_string(TestIF_File.iTestMode) + " map00=" + std::to_string(TestIF_File.iSiteMap[0][0]) + " reply=" + g_reply;
    CHECK(TestIF_File.iTestMode == SingleSite && TestIF_File.iSiteMap[0][0] == 1 && g_reply == "SETTINGOK",
          "3. SETSITEMAP_ SINGLE1X1-1_ with no IC -> ChangeToSiteMap: single site, map 1, SETTINGOK (golden :5320)");
    TestIF_File.iTestMode = savedMode;
    TestIF_File.iSiteMap[0][0] = savedMap;

    W906_TesterForward.SendMSG_CMD_Msg = 0;
    HGpib2Handler = 0;
    printf("St02_W140Command: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
