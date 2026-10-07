// =============================================================================
//  test_st02_w132_sckart.cpp -- W-132: the ART LOTRETESTCLEARED arms follow golden 0618 now that TfSCKART carries
//                               golden's iCurrentStatus / iLOTSTATUS_A.
//
//  AI(W906-W132) 20261007 (St02-E).  Suite name (add_test): St02_W132SckArtLotRt
//
//  Covers forms/fSCKART.{h,cpp} (golden 0618 Automation/SCK_ART.h:251 / :258, ctor SCK_ART.cpp:43 / :49, SetLotStatus
//  :665) and TesterComm/Handler/HandlerGpibMsg.cpp ProcessARTMessage MSG_CMD_SCKART_LOTRTCLEAR with G2 / G4 / G6 open
//  (golden 0618 main.cpp:15443-15526):
//    [1] ctor: iCurrentStatus 0 (VCL zero-init), iLOTSTATUS_R 4, iLOTSTATUS_A 6 -- a new object and the global;
//    [2] SetLotStatus writes iCurrentStatus (golden :665), the two W906_ seams still count;
//    [3] path R: iTesterType 0, status R -> the if-arm runs and G4 sets W + iWaitGPIBLotR 3;
//    [4] path A: iTesterType 0, status A -> the else arm only (G6): W + iCurrentFlexARTStep 10, no lot clearing;
//    [5] other : iTesterType 0, status T -> the if-arm runs, G4 does not (status / iWaitGPIBLotR unchanged);
//    [6] iTesterType 1, status A -> golden short-circuit: the if-arm runs, not the else arm;
//    [7] real files: the if-arm's WriteLastDataFile went to this process's sandbox (tests/test_bootstrap.cpp ->
//        cprod.cpp W906_LastDataPath); D:\HT9045\system\lastdata*.dat and D:\HT9045\config\config.ini are byte-identical.
//  Every CHECK prints the values it read, pass or fail.  Recipe files: %TEMP%\ht9045_w132_<tick> (DataPath / LastDataPath
//  set here; the test aborts before any call if a path is under D:\HT9045).  bUseSCKART stays false, so AccessFile is a
//  no-op (golden :195) and nothing is forwarded to the ART bridge.  The sandbox is removed on a green run.
// =============================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "forms/fSCKART.h"
#include "forms/fMain.h"
#include "MessageDef.h"
#include "LastSet.h"
#include "common.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "Config.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

AnsiString W906_LastDataPath(const char* goldenPath);   // cprod.cpp file end

static int g_pass = 0, g_fail = 0;
static char g_got[512];
#define GOT(...) std::snprintf(g_got, sizeof(g_got), __VA_ARGS__)
#define CHECK(cond, msg)                                                                                   \
    do {                                                                                                   \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got); ++g_pass; }                                 \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got, __LINE__); ++g_fail; }        \
        g_got[0] = 0;                                                                                      \
    } while (0)

static std::string g_root;

static std::string Slurp(const char* p)
{
    FILE* f = std::fopen(p, "rb");
    if (!f) return std::string("<missing>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static void WriteText(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    //AI(W906-W132-SANDBOX) 20261007 laptop: a build dir under D:\HT9045 (<tree>\Obj\V906\<build>\tests\w906_ctest_lastdata_<pid>,
    //  where tests/test_bootstrap.cpp redirects lastdata.dat) is a sandbox, not the machine's data -- the test ABORTed on every box whose
    //  build dir is under D:\HT9045 (gate b83a). The machine's own lastdata*.dat / config.ini are still byte-checked (check 7).
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

// one LOTRETESTCLEAR packet through the handler, as the GPIB thread hands it over
static bool Process()
{
    VM vm;
    std::memset(&vm, 0, sizeof(vm));
    vm.iCommand = MSG_CMD_SCKART_LOTRTCLEAR;
    HGpib2Handler = &vm;
    std::memset(HHandler2Gpib.Message, 0, sizeof(HHandler2Gpib.Message));
    THandlerTesterSide side;
    const bool r = side.ProcessARTMessage();
    HGpib2Handler = 0;
    return r;
}

// the state each path starts from: markers the if-arm clears and markers only the else arm / G4 change
static void Seed(int testerType, int status)
{
    fSCKART->iTesterType         = testerType;
    fSCKART->iCurrentStatus      = status;
    fSCKART->iWaitGPIBLotR       = 5;
    fSCKART->iCurrentFlexARTStep = 99;
    fSCKART->iInputJamCnt        = 7;
    fSCKART->iOutputJamCnt       = 8;
    fSCKART->iInputCount         = 0;
    LastSet.lShuttleCount        = 77;
    LastSet.iSCKART_RTUnitCount  = 9;
    fMain->W906_Clarn_DataCallCount = 0;
}

static bool IfArmRan()   // golden :15464-15510: Clarn_Data(2), lShuttleCount 0, jam counts 0
{
    return fMain->W906_Clarn_DataCallCount == 1 && LastSet.lShuttleCount == 0 &&
           fSCKART->iInputJamCnt == 0 && fSCKART->iOutputJamCnt == 0;
}

static bool NothingCleared()
{
    return fMain->W906_Clarn_DataCallCount == 0 && LastSet.lShuttleCount == 77 &&
           fSCKART->iInputJamCnt == 7 && fSCKART->iOutputJamCnt == 8 && LastSet.iSCKART_RTUnitCount == 9;
}

static void GotState()
{
    GOT("status=%d waitR=%d flexStep=%d clarn=%d shuttle=%ld jam=%d/%d input=%d rtUnit=%d reply=%.20s",
        fSCKART->iCurrentStatus, fSCKART->iWaitGPIBLotR, fSCKART->iCurrentFlexARTStep, fMain->W906_Clarn_DataCallCount,
        (long)LastSet.lShuttleCount, fSCKART->iInputJamCnt, fSCKART->iOutputJamCnt, fSCKART->iInputCount,
        LastSet.iSCKART_RTUnitCount, HHandler2Gpib.Message);
    for (char* c = g_got; *c; ++c) if (*c == '\r' || *c == '\n') *c = '~';
}

int main()
{
    printf("St02_W132SckArtLotRt\n");
    static const char* const kReal[4] = {
        "D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\lastdata_backup.dat",
        "D:\\HT9045\\system\\lastdata_backup2.dat", "D:\\HT9045\\config\\config.ini" };
    std::string before[4];
    for (int i = 0; i < 4; ++i) before[i] = Slurp(kReal[i]);

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w132_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    WriteText(g_root + "\\SetUp.inf", "W132R\r\n");
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());

    // containment first: nothing below may write the machine's files
    const std::string lastData = std::string(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat").c_str());
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(lastData))
    {
        printf("  ABORT: a path is not in a sandbox (DataPath=%s, lastdata=%s) -- nothing was called\n",
               DataPath.c_str(), lastData.c_str());
        return 2;
    }
    GOT("lastdata -> %s", lastData.c_str());
    CHECK(true, "0. WriteLastDataFile is redirected for this process (tests/test_bootstrap.cpp)");
    GOT("fSCKART=%p fMain=%p", (void*)fSCKART, (void*)fMain);
    CHECK(fSCKART != 0 && fMain != 0, "0. the fSCKART / fMain facades exist");
    CosFunction.bUseSCKART       = false;   // AccessFile is a no-op (golden :195); LOTRTCLEAR is not forwarded to the bridge
    IniConfig.bA10_AutoReTest    = false;
    CUSTOMER_CODE                = 0;       // not CC_ASE_KaohSiung (golden :15448 returns early there)

    // [1] ctor
    {
        TfSCKART local;
        GOT("new: status=%d A=%d W=%d R=%d NONE=%d; global: status=%d A=%d R=%d", local.iCurrentStatus, local.iLOTSTATUS_A,
            local.iLOTSTATUS_W, local.iLOTSTATUS_R, local.iLOTSTATUS_NONE, fSCKART->iCurrentStatus, fSCKART->iLOTSTATUS_A,
            fSCKART->iLOTSTATUS_R);
        CHECK(local.iCurrentStatus == 0 && local.iLOTSTATUS_A == 6 && local.iLOTSTATUS_R == 4 && local.iLOTSTATUS_W == 1 &&
              local.iLOTSTATUS_NONE == 0 && fSCKART->iCurrentStatus == 0 && fSCKART->iLOTSTATUS_A == 6 && fSCKART->iLOTSTATUS_R == 4,
              "1. ctor: iCurrentStatus 0 (VCL zero-init), NONE 0 / W 1 / R 4 / A 6 (golden SCK_ART.cpp:43-49), new object and global");
    }

    // [2] SetLotStatus
    const int n0 = fSCKART->W906_SetLotStatus_Count;
    fSCKART->SetLotStatus(4);
    GOT("status=%d lastArg=%d count+%d", fSCKART->iCurrentStatus, fSCKART->W906_SetLotStatus_LastArg, fSCKART->W906_SetLotStatus_Count - n0);
    CHECK(fSCKART->iCurrentStatus == 4 && fSCKART->W906_SetLotStatus_LastArg == 4 && fSCKART->W906_SetLotStatus_Count == n0 + 1,
          "2. SetLotStatus(4) -> iCurrentStatus 4 (golden :665), seams still record");

    // [3] path R
    Seed(0, fSCKART->iLOTSTATUS_R);
    bool r = Process();
    GotState();
    CHECK(r && std::strcmp(HHandler2Gpib.Message, "LOTRETESTCLEARED\r\n") == 0, "3. R: handled, reply LOTRETESTCLEARED");
    GotState();
    CHECK(IfArmRan() && fSCKART->iInputCount == 9 && LastSet.iSCKART_RTUnitCount == 0,
          "3. R: if-arm ran (G2: iCurrentStatus != iLOTSTATUS_A) -- Clarn_Data, counts cleared, iTesterType 0 takes the RT unit count");
    GotState();
    CHECK(fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_W && fSCKART->iWaitGPIBLotR == 3,
          "3. R: G4 -> SetLotStatus(W) = 1, iWaitGPIBLotR 3 (golden :15512-15516)");
    GotState();
    CHECK(fSCKART->iCurrentFlexARTStep == 99, "3. R: the else arm did not run (iCurrentFlexARTStep untouched)");

    // [4] path A
    Seed(0, fSCKART->iLOTSTATUS_A);
    r = Process();
    GotState();
    CHECK(r && fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_W && fSCKART->iCurrentFlexARTStep == 10,
          "4. A: else arm (G6) -> SetLotStatus(W) = 1, iCurrentFlexARTStep 10 (golden :15521-15525)");
    GotState();
    CHECK(NothingCleared() && fSCKART->iWaitGPIBLotR == 5, "4. A: the if-arm did not run (nothing cleared, iWaitGPIBLotR untouched)");

    // [5] other status (T = 2)
    Seed(0, 2);
    r = Process();
    GotState();
    CHECK(r && IfArmRan(), "5. T: if-arm ran");
    GotState();
    CHECK(fSCKART->iCurrentStatus == 2 && fSCKART->iWaitGPIBLotR == 5 && fSCKART->iCurrentFlexARTStep == 99,
          "5. T: G4 did not fire (status / iWaitGPIBLotR unchanged), the else arm did not run");

    // [6] iTesterType 1 short-circuits even with status A
    Seed(1, fSCKART->iLOTSTATUS_A);
    r = Process();
    GotState();
    CHECK(r && IfArmRan() && fSCKART->iCurrentFlexARTStep == 99 && fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_A,
          "6. iTesterType 1 + A: golden short-circuit -> if-arm, not the else arm; status stays A (G4 needs R)");
    GotState();
    CHECK(fSCKART->iInputCount == 0 && LastSet.iSCKART_RTUnitCount == 9,
          "6. iTesterType 1: the RT unit count is not moved (golden :15504 needs iTesterType 0)");

    // [7] real files
    int same = 0;
    for (int i = 0; i < 4; ++i) if (Slurp(kReal[i]) == before[i]) ++same;
    const std::string sandboxLast = Slurp(lastData.c_str());
    GOT("%d of 4 real files byte-identical; sandbox lastdata.dat %u bytes", same, (unsigned)sandboxLast.size());
    CHECK(same == 4 && sandboxLast != "<missing>",
          "7. WriteLastDataFile wrote the sandbox; the machine's lastdata*.dat / config.ini are unchanged");

    printf("St02_W132SckArtLotRt: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
