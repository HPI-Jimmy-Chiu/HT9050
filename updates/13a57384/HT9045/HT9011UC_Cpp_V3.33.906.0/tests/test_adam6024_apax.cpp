// =============================================================================
//  tests/test_adam6024_apax.cpp -- ST02-ADAM, helper H3: golden 912 adam6024.cpp:2218-2673 (Adam6024Apax_St02.cpp)
//  and the ADSMOD.dll run-time shim (Public/ApaxShim_St02.{h,cpp}).
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H3).  Suite: Adam6024_Apax.  Both configurations (SIM / SHIP).
//
//  CONTAINMENT FIRST (st02_test_containment.h): refuses (exit 2) unless the ctest redirect roots are set.  Memory only:
//  every callee that could leave the process goes through a fake --
//    * ADAMTCP_WriteReg, Close_ADAM_6024 / Open_ADAM_6024, NewRecordProcess: the three seams of Adam6024Apax_St02.cpp
//      (W906_ApaxSet*), installed BEFORE any APAX call;
//    * MOD_*: ApaxMod_InstallApiForTest(fake table), installed before any call, so ADSMOD.dll is never loaded;
//    * Address[2] is set to 192.0.2.112 (TEST-NET-1, RFC 5737) for the whole run, a second line of defence.
//  Section 2b resolves the shim for real: the shim's [W906] S1 (SIM build) / S2 (ctest environment) rules refuse
//  before any LoadLibrary, and the section runs only when that condition holds (it is section 0's condition too).
//
//  SECTIONS
//    1. seams + fake ADSMOD table in place.
//    2. shim: a table with a null entry = not bound -> every MOD_* returns 801 (MODERR_WSASTART_FAILED), no fake call.
//       2b. real resolution -> not bound, 801, BindInfo names S1 (SIM) / S2 (SHIP inside ctest); no LoadLibrary.
//    3. Open_APAX (golden :2219-2267): EP_Install 0 -> false, nothing called; EP_Install 3 / 5 -> cAddress = IP,
//       MOD_Initialize, MOD_AddTcpClientConnect(cAddress, 100, 3000, 100, handlers, NULL, &ulClientHandle),
//       MOD_StartTcpClient, MOD_SetTcpClientPriority(THREAD_PRIORITY_HIGHEST) -> true; Init / Add fail -> true and
//       stops (Q5); Start fails -> false; not bound -> true.
//    4. the completion handlers (golden :2269-2303), through the pointers Open_APAX handed to ADSMOD and through the
//       global forwarders: result 0 / non-0, the record texts, ClientWriteReg_1_8_Handler.
//    5. APAX_WriteData gate (golden :2307-2315): INSTALL_DOUBLE_EP 0 / 1 -> nothing; 3 with the route inactive ->
//       nothing (IsMultiEPPressureRouteActive, H2: bIndEPSLK, SW[SwMultiEp] Enable / status).
//    6. MULTI (golden :2324-2460): both blocks every call (start 1 / 33, unit 1, count 8), channel layout, bDir,
//       iArm 1 / 2, shuttle mode / select, WORD wrap (Q7), retries: 10 per block in both builds, SHIP logs + Close +
//       Open per failure, 817 = success.
//    7. INDIVIAL (golden :2462-2673): iIndEPCnt 16 / 8 / 4, DualSite, bDir, iArm -> block mapping (Q2), retries:
//       SHIP ONE try per block (Q1, the reused i), SIM 10; INSTALL_DOUBLE_EP 4 takes this path too.
//    8. APAX_WriteData never writes iAPAXEPValue / iAPAXDualEPValue.
// =============================================================================
#define ADAM6024_ST02_INTERNAL
#include "Adam6024_St02.h"
#include "Public/ApaxShim_St02.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "myswitch.h"
#include "common.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>

// ---- Adam6024Apax_St02.cpp: seams and the global handler forwarders (golden :50-51 / :2300) ----
void W906_ApaxSetWriteReg(int (*fn)(const char*, WORD, WORD, WORD, WORD*));
void W906_ApaxSetAdamReconnect(void (*closeFn)(), bool (*openFn)());
void W906_ApaxSetNewRecordProcess(void (*fn)(AnsiString, AnsiString, AnsiString));
void ConnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);
void DisconnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);
void ClientWriteReg_1_8_Handler(long i_lResult, void *i_Param);

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const char* what, int line)
{
    if (ok)
        ++g_pass;
    else
    {
        ++g_fail;
        std::printf("  FAIL (line %d) %s\n", line, what);
    }
}
#define CHECK(c, what) Check((c), (what), __LINE__)

// ---- event order across the fakes: "W" write, "R" record, "C" close, "O" open ----
std::string g_order;

// ---- fake ADAMTCP_WriteReg ----
struct WriteCall
{
    std::string ip;
    int id;
    int start;
    int count;
    std::vector<int> data;
};
std::vector<WriteCall> g_writes;
std::vector<int> g_writeScript;          // return value per call; past the end -> the last one; empty -> 0
size_t g_writeScriptPos = 0;

int FakeWriteReg(const char* ip, WORD id, WORD start, WORD count, WORD* data)
{
    WriteCall c;
    c.ip = ip ? ip : "";
    c.id = id;
    c.start = start;
    c.count = count;
    for (int k = 0; k < count && k < 16; ++k)
        c.data.push_back(data[k]);
    g_writes.push_back(c);
    g_order += "W";
    int r = 0;
    if (!g_writeScript.empty())
    {
        size_t p = g_writeScriptPos < g_writeScript.size() ? g_writeScriptPos : g_writeScript.size() - 1;
        r = g_writeScript[p];
        ++g_writeScriptPos;
    }
    return r;
}

// ---- fake Close_ADAM_6024 / Open_ADAM_6024 ----
int g_close = 0;
int g_open = 0;
void FakeCloseAdam() { ++g_close; g_order += "C"; }
bool FakeOpenAdam()  { ++g_open;  g_order += "O"; return true; }

// ---- fake NewRecordProcess ----
struct RecordCall
{
    std::string code;
    std::string text;
    std::string debug;
};
std::vector<RecordCall> g_records;
void FakeRecord(AnsiString AlarmCode, AnsiString S, AnsiString Debug)
{
    RecordCall r;
    r.code = AlarmCode.c_str();
    r.text = S.c_str();
    r.debug = Debug.c_str();
    g_records.push_back(r);
    g_order += "R";
}

// ---- fake ADSMOD table ----
struct ModState
{
    int init, add, start, prio;
    long initRet, addRet, startRet, prioRet;
    char* addIp;
    int addScan, addConn, addTrans;
    OnConnectTcpServerCompletedEvent addOnConnect;
    OnDisconnectTcpServerCompletedEvent addOnDisconnect;
    void* addParam;
    unsigned long* addHandle;
    int prioValue;
};
ModState g_mod;

void ResetMod()
{
    std::memset(&g_mod, 0, sizeof(g_mod));
}

long APAXMOD_CALL FakeModInit() { ++g_mod.init; return g_mod.initRet; }
long APAXMOD_CALL FakeModAdd(char *ip, int scan, int conn, int trans, OnConnectTcpServerCompletedEvent onC,
                             OnDisconnectTcpServerCompletedEvent onD, void *param, unsigned long *handle)
{
    ++g_mod.add;
    g_mod.addIp = ip;
    g_mod.addScan = scan;
    g_mod.addConn = conn;
    g_mod.addTrans = trans;
    g_mod.addOnConnect = onC;
    g_mod.addOnDisconnect = onD;
    g_mod.addParam = param;
    g_mod.addHandle = handle;
    if (handle)
        *handle = 0x1234UL;
    return g_mod.addRet;
}
long APAXMOD_CALL FakeModStart() { ++g_mod.start; return g_mod.startRet; }
long APAXMOD_CALL FakeModPrio(int p) { ++g_mod.prio; g_mod.prioValue = p; return g_mod.prioRet; }

ApaxModApi FullTable()
{
    ApaxModApi a;
    a.Initialize = FakeModInit;
    a.AddTcpClientConnect = FakeModAdd;
    a.StartTcpClient = FakeModStart;
    a.SetTcpClientPriority = FakeModPrio;
    return a;
}

void ResetCalls()
{
    g_writes.clear();
    g_writeScript.clear();
    g_writeScriptPos = 0;
    g_close = 0;
    g_open = 0;
    g_records.clear();
    g_order.clear();
}

bool DataIs(const WriteCall& c, std::initializer_list<int> v)
{
    if (c.data.size() != v.size())
        return false;
    size_t k = 0;
    for (int x : v)
    {
        if (c.data[k] != x)
            return false;
        ++k;
    }
    return true;
}

bool BlockIs(const WriteCall& c, int start)
{
    return c.ip == "192.0.2.112" && c.id == 1 && c.start == start && c.count == 8;
}

void SetMultiRoute(bool on)
{
    TestIF_File.bIndEPSLK = true;
    SW[SwMultiEp].Enable = true;
    SW[SwMultiEp].ISABase = 99;          // no IO branch in TMySwitch::Status (myswitch.cpp:190-200) -> OutValue
    SW[SwMultiEp].Type = 1;
    SW[SwMultiEp].OutValue = on;
}

} // namespace

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    std::printf("Adam6024_Apax: golden 912 adam6024.cpp:2218-2673 + ADSMOD shim\n");

    // ---- 0. containment first ----
    if (!W906TestInsideCtestRoots("Adam6024_Apax"))
        return 2;
    W906_AdamEpLive_SetForTest(1);   // (H4 integration pass 20261002) R4: APAX_WriteData asks the EP live switch first (default OFF = no-op)

    // ---- 1. seams + fake ADSMOD table, before anything else ----
    W906_ApaxSetWriteReg(FakeWriteReg);
    W906_ApaxSetAdamReconnect(FakeCloseAdam, FakeOpenAdam);
    W906_ApaxSetNewRecordProcess(FakeRecord);
    ApaxModApi full = FullTable();
    ApaxMod_InstallApiForTest(&full);
    CHECK(ApaxMod_IsBound(), "1. fake ADSMOD table bound");
    CHECK(std::string(ApaxMod_BindInfo()) == "fake table (test)", "1. BindInfo = fake table (test)");

    // save the globals this test changes
    const int saveEP = EP_Install;
    const int saveDouble = INSTALL_DOUBLE_EP;
    const int saveIndCnt = iIndEPCnt;
    const int saveTestMode = TestIF_File.iTestMode;
    const int saveShMode = TestIF_File.iShuttleMode;
    const int saveShSel = TestIF_File.iShuttle_Sel;
    const bool saveIndSlk = TestIF_File.bIndEPSLK;
    const TMySwitch saveSw = SW[SwMultiEp];
    int saveEPV[16], saveDualV[16];
    std::memcpy(saveEPV, iAPAXEPValue, sizeof(saveEPV));
    std::memcpy(saveDualV, iAPAXDualEPValue, sizeof(saveDualV));
    const bool saveAlarm = bAPAXConnectFileAlarm;
    const bool saveWF = bApaxWriteFinish;
    const bool saveRF = bApaxReadFinish;
    const AnsiString saveAddr2 = Address[2];
    char saveCAddr[100];
    std::memcpy(saveCAddr, cAddress, sizeof(saveCAddr));

    Address[2] = "192.0.2.112";
    CHECK(std::string(cAddress) == "172.16.8.112", "1. cAddress golden :43 initial value");

    // ---- 2. shim: a null entry = not bound ----
    {
        ApaxModApi half = FullTable();
        half.SetTcpClientPriority = 0;
        ApaxMod_InstallApiForTest(&half);
        ResetMod();
        CHECK(!ApaxMod_IsBound(), "2. table with a null entry is not bound");
        CHECK(MOD_Initialize() == MODERR_WSASTART_FAILED, "2. MOD_Initialize -> 801");
        unsigned long h = 7;
        CHECK(MOD_AddTcpClientConnect(cAddress, 1, 2, 3, 0, 0, 0, &h) == MODERR_WSASTART_FAILED, "2. MOD_AddTcpClientConnect -> 801");
        CHECK(MOD_StartTcpClient() == MODERR_WSASTART_FAILED, "2. MOD_StartTcpClient -> 801");
        CHECK(MOD_SetTcpClientPriority(1) == MODERR_WSASTART_FAILED, "2. MOD_SetTcpClientPriority -> 801");
        CHECK(g_mod.init == 0 && g_mod.add == 0 && g_mod.start == 0 && g_mod.prio == 0 && h == 7, "2. no fake entry called");

        // 2b. real resolution: the shim's S1 (SIM build) / S2 (ctest: W906_GENERAL_INI_PATH is a general_ini_scratch
        //     path -- exactly what section 0 already required) refuse before any LoadLibrary.  Run only when that holds.
        const char* gi = std::getenv("W906_GENERAL_INI_PATH");
        const bool s2 = gi != 0 && std::strstr(gi, "general_ini_scratch") != 0;
#ifdef SOFT_SIMULTE
        (void)s2;                                   // S1 alone already refuses in a SIM build
        const bool safe = true;
        const char* expectInfo = "SIMULATION build";
#else
        const bool safe = s2;
        const char* expectInfo = "ctest environment";
#endif
        if (safe)
        {
            ApaxMod_InstallApiForTest(0);
            CHECK(MOD_Initialize() == MODERR_WSASTART_FAILED, "2b. no table, S1 / S2 -> MOD_Initialize 801");
            CHECK(!ApaxMod_IsBound(), "2b. not bound");
            CHECK(std::string(ApaxMod_BindInfo()).find(expectInfo) != std::string::npos, "2b. BindInfo names S1 (SIM) / S2 (SHIP in ctest)");
            std::printf("  2b. %s\n", ApaxMod_BindInfo());
        }
        else
            std::printf("  2b. SKIP: W906_GENERAL_INI_PATH is not a general_ini_scratch path (%s)\n", gi ? gi : "(unset)");
        ApaxMod_InstallApiForTest(&full);
        CHECK(ApaxMod_IsBound(), "2. fake table back");
    }

    // ---- 3. Open_APAX ----
    char ip[] = "192.0.2.12";
    {
        EP_Install = 0;
        ResetMod();
        CHECK(Open_APAX(ip) == false, "3a. EP_Install 0 -> false");
        CHECK(g_mod.init == 0 && g_mod.add == 0 && g_mod.start == 0 && g_mod.prio == 0, "3a. nothing called");
        CHECK(std::string(cAddress) == "172.16.8.112", "3a. cAddress untouched");

        EP_Install = 3;
        ResetMod();
        ulClientHandle = 0;
        CHECK(Open_APAX(ip) == true, "3b. EP_Install 3, all succeed -> true");
        CHECK(std::string(cAddress) == "192.0.2.12", "3b. strcpy(cAddress, IP)");
        CHECK(g_mod.init == 1 && g_mod.add == 1 && g_mod.start == 1 && g_mod.prio == 1, "3b. Init, Add, Start, Priority once each");
        CHECK(g_mod.addIp == cAddress, "3b. Add gets cAddress (not IP)");
        CHECK(g_mod.addScan == 100 && g_mod.addConn == 3000 && g_mod.addTrans == 100, "3b. 100 / 3000 / 100 ms");
        CHECK(g_mod.addOnConnect != 0 && g_mod.addOnDisconnect != 0, "3b. both handlers handed over");
        CHECK(g_mod.addParam == 0, "3b. i_Param NULL");
        CHECK(g_mod.addHandle == &ulClientHandle && ulClientHandle == 0x1234UL, "3b. &ulClientHandle filled");
        CHECK(g_mod.prioValue == THREAD_PRIORITY_HIGHEST, "3b. THREAD_PRIORITY_HIGHEST");

        ModState handed = g_mod;   // keep the handler pointers for section 4

        EP_Install = 5;
        ResetMod();
        CHECK(Open_APAX(ip) == true && g_mod.prio == 1, "3c. any non-zero EP_Install (5) runs the same path");

        EP_Install = 3;
        ResetMod();
        g_mod.initRet = MODERR_WSASTART_FAILED;
        CHECK(Open_APAX(ip) == true, "3d. Init fails -> still true (Q5)");
        CHECK(g_mod.init == 1 && g_mod.add == 0 && g_mod.start == 0 && g_mod.prio == 0, "3d. stops after Init");

        ResetMod();
        g_mod.addRet = 807;
        CHECK(Open_APAX(ip) == true, "3e. Add fails -> still true (Q5)");
        CHECK(g_mod.add == 1 && g_mod.start == 0 && g_mod.prio == 0, "3e. stops after Add");

        ResetMod();
        g_mod.startRet = 815;
        CHECK(Open_APAX(ip) == false, "3f. Start fails -> false");
        CHECK(g_mod.start == 1 && g_mod.prio == 0, "3f. no priority call");

        ResetMod();
        g_mod.prioRet = 821;
        CHECK(Open_APAX(ip) == true, "3g. a failing SetTcpClientPriority is ignored");

        ApaxModApi half = FullTable();
        half.Initialize = 0;
        ApaxMod_InstallApiForTest(&half);
        ResetMod();
        CHECK(Open_APAX(ip) == true, "3h. ADSMOD not bound -> golden's Init-failed branch, true");
        CHECK(g_mod.init == 0 && g_mod.add == 0, "3h. nothing reached the table");
        ApaxMod_InstallApiForTest(&full);

        // ---- 4. the completion handlers ----
        ResetCalls();
        bAPAXConnectFileAlarm = true;
        bApaxWriteFinish = false;
        bApaxReadFinish = false;
        handed.addOnConnect(0, ip, 0);
        CHECK(bAPAXConnectFileAlarm == false && bApaxWriteFinish == true && bApaxReadFinish == true, "4a. connect result 0 -> alarm off, write / read finished");
        CHECK(g_records.size() == 1 && g_records[0].code.empty() && g_records[0].debug == " " &&
              g_records[0].text == "APAX Connect to '192.0.2.12' result = 0\n", "4a. record text (golden :2274), Debug default \" \"");

        ResetCalls();
        bApaxWriteFinish = false;
        bApaxReadFinish = false;
        handed.addOnConnect(807, ip, 0);
        CHECK(bAPAXConnectFileAlarm == true && bApaxWriteFinish == false && bApaxReadFinish == false, "4b. connect result 807 -> alarm on, finish flags untouched");
        CHECK(g_records.size() == 1 && g_records[0].text == "APAX Connect to '192.0.2.12' result = 807\n", "4b. record text");

        ResetCalls();
        handed.addOnDisconnect(814, ip, 0);
        CHECK(bAPAXConnectFileAlarm == true && bApaxWriteFinish == false && bApaxReadFinish == false, "4c. disconnect touches no flag");
        CHECK(g_records.size() == 1 && g_records[0].text == "APAX Disconnect from '192.0.2.12' result = 814\n", "4c. record text (golden :2296)");

        ResetCalls();
        ConnectTcpServerCompletedEventHandler(0, ip, 0);
        CHECK(bAPAXConnectFileAlarm == false && bApaxWriteFinish == true && g_records.size() == 1, "4d. global forwarder = the same body");
        DisconnectTcpServerCompletedEventHandler(3, ip, 0);
        CHECK(g_records.size() == 2 && g_records[1].text == "APAX Disconnect from '192.0.2.12' result = 3\n", "4d. global disconnect forwarder");

        bApaxWriteFinish = false;
        ClientWriteReg_1_8_Handler(5, 0);
        CHECK(bApaxWriteFinish == true, "4e. ClientWriteReg_1_8_Handler -> bApaxWriteFinish true (any result)");
    }

    // ---- 5. APAX_WriteData gate ----
    for (int k = 0; k < 16; ++k)
    {
        iAPAXDualEPValue[k] = 100 + k;
        iAPAXEPValue[k] = 200 + k;
    }
    TestIF_File.iTestMode = SingleSite;
    TestIF_File.iShuttleMode = 0;
    TestIF_File.iShuttle_Sel = 0;
    {
        for (int v = 0; v < 2; ++v)
        {
            INSTALL_DOUBLE_EP = v;
            ResetCalls();
            bApaxWriteFinish = true;
            APAX_WriteData(false, 0, 0);
            APAX_WriteData(true, 100, 1);
            CHECK(g_writes.empty() && bApaxWriteFinish == true, "5a. INSTALL_DOUBLE_EP 0 / 1 -> no write, flag untouched");
        }
        INSTALL_DOUBLE_EP = DOUBLE_EP_MULTI;
        SetMultiRoute(true);
        TestIF_File.bIndEPSLK = false;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.empty(), "5b. MULTI, bIndEPSLK false -> route inactive, no write");
        SetMultiRoute(true);
        SW[SwMultiEp].Enable = false;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.empty(), "5c. MULTI, SW[SwMultiEp] disabled -> no write");
        SetMultiRoute(false);
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.empty(), "5d. MULTI, SwMultiEp OFF -> no write");
    }

    // ---- 6. MULTI ----
    {
        INSTALL_DOUBLE_EP = DOUBLE_EP_MULTI;
        SetMultiRoute(true);
        ResetCalls();
        bApaxWriteFinish = true;
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && BlockIs(g_writes[0], 1) && BlockIs(g_writes[1], 33), "6a. two writes: S0 start 1, S1 start 33, unit 1, count 8");
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {100, 101, 200, 201, 202, 203, 102, 103}), "6a. Arm1 layout Dual1 Dual2 Site1..4 Dual3 Dual4");
        CHECK(g_writes.size() == 2 && DataIs(g_writes[1], {104, 105, 204, 205, 206, 207, 106, 107}), "6a. Arm2 layout");
        CHECK(bApaxWriteFinish == false && g_records.empty() && g_close == 0, "6a. bApaxWriteFinish false, no record");

        ResetCalls();
        APAX_WriteData(true, 100, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {1600, 1600, 1600, 1600, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {1600, 1600, 1600, 1600, 0, 0, 0, 0}), "6b. bDir: wdata*16 on the first four, 0 on the rest");

        ResetCalls();
        APAX_WriteData(false, 0, 1);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {100, 101, 200, 201, 202, 203, 102, 103}) &&
              DataIs(g_writes[1], {0, 0, 0, 0, 0, 0, 0, 0}), "6c. iArm 1 -> Arm1 values, S1 still written with zeros (Q3)");
        ResetCalls();
        APAX_WriteData(false, 0, 2);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {0, 0, 0, 0, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {104, 105, 204, 205, 206, 207, 106, 107}), "6c. iArm 2 -> S0 zeros, Arm2 values");

        TestIF_File.iShuttleMode = 1;
        TestIF_File.iShuttle_Sel = 0;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {100, 101, 200, 201, 202, 203, 102, 103}) &&
              DataIs(g_writes[1], {0, 0, 0, 0, 0, 0, 0, 0}), "6d. shuttle mode 1, select 0 -> Arm1 only");
        TestIF_File.iShuttle_Sel = 1;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {0, 0, 0, 0, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {104, 105, 204, 205, 206, 207, 106, 107}), "6d. shuttle mode 1, select 1 -> Arm2 only");
        TestIF_File.iShuttle_Sel = 0;
        ResetCalls();
        APAX_WriteData(true, 7, 2);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {0, 0, 0, 0, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {112, 112, 112, 112, 0, 0, 0, 0}), "6e. iArm 2 overrides the shuttle select");
        TestIF_File.iShuttleMode = 0;

        ResetCalls();
        APAX_WriteData(true, 4095, 1);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {65520, 65520, 65520, 65520, 0, 0, 0, 0}), "6f. 4095*16 = 65520");
        ResetCalls();
        APAX_WriteData(true, 4096, 1);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {0, 0, 0, 0, 0, 0, 0, 0}), "6f. 4096*16 wraps to 0 in a WORD (Q7)");

        // retries
        ResetCalls();
        g_writeScript.push_back(5);
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 20, "6g. always failing -> 10 tries per block in both builds");
        bool order = g_writes.size() == 20;
        for (size_t k = 0; order && k < 20; ++k)
            order = BlockIs(g_writes[k], k < 10 ? 1 : 33);
        CHECK(order, "6g. the 10 S0 tries come first, then the 10 S1 tries");
#ifdef SOFT_SIMULTE
        CHECK(g_records.empty() && g_close == 0 && g_open == 0, "6g. SIM: no record, no reconnect (#ifndef SOFT_SIMULTE)");
#else
        CHECK(g_records.size() == 20 && g_close == 20 && g_open == 20, "6g. SHIP: a record + Close + Open per failure");
        CHECK(g_records.size() == 20 && g_records[0].text == "APAX MEP3 ARM1 S0 SEND DATA FAIL 5, 100101200201202203102103 \n" &&
              g_records[10].text == "APAX MEP3 ARM2 S1 SEND DATA FAIL 5, 104105204205206207106107 \n", "6g. SHIP record texts (golden :2428 / :2451)");
        CHECK(g_order.compare(0, 8, "WRCOWRCO") == 0, "6g. SHIP order: write, record, Close, Open");
#endif
        ResetCalls();
        g_writeScript.push_back(5);
        g_writeScript.push_back(5);
        g_writeScript.push_back(0);
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 4 && g_writes[2].start == 1 && g_writes[3].start == 33, "6h. two failures then success -> 3 S0 tries, 1 S1 try");
        ResetCalls();
        g_writeScript.push_back(817);
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && g_records.empty(), "6i. 817 counts as success");
    }

    // ---- 7. INDIVIAL ----
    for (int k = 0; k < 16; ++k)
        iAPAXEPValue[k] = 300 + k;
    {
        INSTALL_DOUBLE_EP = DOUBLE_EP_INDIVIAL;
        TestIF_File.bIndEPSLK = false;                  // not consulted on this path
        TestIF_File.iTestMode = SingleSite;
        iIndEPCnt = 16;

        ResetCalls();
        bApaxWriteFinish = true;
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && BlockIs(g_writes[0], 1) && BlockIs(g_writes[1], 33), "7a. 16 EP, iArm 0 -> start 1 then start 33");
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {300, 301, 302, 303, 304, 305, 306, 307}) &&
              DataIs(g_writes[1], {308, 309, 310, 311, 312, 313, 314, 315}), "7a. iAPAXEPValue[0..7] / [8..15]");
        CHECK(bApaxWriteFinish == false, "7a. bApaxWriteFinish false");

        ResetCalls();
        APAX_WriteData(true, 10, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {160, 160, 160, 160, 160, 160, 160, 160}) &&
              DataIs(g_writes[1], {160, 160, 160, 160, 160, 160, 160, 160}), "7b. bDir -> wdata*16 on all eight");

        ResetCalls();
        APAX_WriteData(false, 0, 1);
        CHECK(g_writes.size() == 1 && BlockIs(g_writes[0], 33), "7c. iArm 1 -> ONLY start 33 (Q2)");
        ResetCalls();
        APAX_WriteData(false, 0, 2);
        CHECK(g_writes.size() == 1 && BlockIs(g_writes[0], 1), "7c. iArm 2 -> ONLY start 1 (Q2)");
        ResetCalls();
        APAX_WriteData(false, 0, 3);
        CHECK(g_writes.empty(), "7c. iArm 3 -> neither block (16 EP)");

        TestIF_File.iTestMode = DualSite;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {300, 301, 0, 0, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {308, 309, 0, 0, 0, 0, 0, 0}), "7d. DualSite: [0],[1] then [8],[9]");
        ResetCalls();
        APAX_WriteData(true, 10, 0);
        CHECK(g_writes.size() == 2 && DataIs(g_writes[0], {160, 160, 0, 0, 0, 0, 0, 0}) &&
              DataIs(g_writes[1], {160, 160, 0, 0, 0, 0, 0, 0}), "7d. DualSite bDir");

        iIndEPCnt = 4;
        TestIF_File.iTestMode = SingleSite;
        ResetCalls();
        APAX_WriteData(false, 0, 1);
        CHECK(g_writes.size() == 1 && BlockIs(g_writes[0], 1) && DataIs(g_writes[0], {300, 301, 302, 303, 0, 0, 0, 0}),
              "7e. 4 EP: start 1 only, whatever iArm, [0..3]");
        ResetCalls();
        APAX_WriteData(true, 10, 0);
        CHECK(g_writes.size() == 1 && DataIs(g_writes[0], {160, 160, 0, 0, 160, 160, 0, 0}), "7e. 4 EP bDir: ch 0, 1, 4, 5");
        TestIF_File.iTestMode = DualSite;
        ResetCalls();
        APAX_WriteData(false, 0, 0);
        CHECK(g_writes.size() == 1 && DataIs(g_writes[0], {300, 301, 0, 0, 304, 305, 0, 0}), "7e. 4 EP DualSite: [0],[1],[4],[5]");

        iIndEPCnt = 8;
        TestIF_File.iTestMode = SingleSite;
        ResetCalls();
        APAX_WriteData(false, 0, 1);
        CHECK(g_writes.size() == 1 && BlockIs(g_writes[0], 1) && DataIs(g_writes[0], {300, 301, 302, 303, 304, 305, 306, 307}),
              "7f. 8 EP: start 1 only, [0..7]");
        ResetCalls();
        APAX_WriteData(true, 10, 2);
        CHECK(g_writes.size() == 1 && DataIs(g_writes[0], {160, 160, 160, 160, 160, 160, 160, 160}), "7f. 8 EP bDir: all eight");
        TestIF_File.iTestMode = DualSite;
        ResetCalls();
        APAX_WriteData(true, 10, 0);
        CHECK(g_writes.size() == 1 && DataIs(g_writes[0], {160, 160, 0, 0, 160, 160, 0, 0}), "7f. 8 EP DualSite takes the 4-EP layout (Q6)");

        // retries (Q1)
        iIndEPCnt = 16;
        TestIF_File.iTestMode = SingleSite;
        ResetCalls();
        g_writeScript.push_back(5);
        APAX_WriteData(false, 0, 0);
#ifdef SOFT_SIMULTE
        CHECK(g_writes.size() == 20 && g_records.empty() && g_close == 0 && g_open == 0, "7g. SIM: 10 tries per block, no record");
#else
        CHECK(g_writes.size() == 2 && BlockIs(g_writes[0], 1) && BlockIs(g_writes[1], 33), "7g. SHIP: ONE try per block (Q1, the reused i)");
        CHECK(g_records.size() == 2 && g_close == 2 && g_open == 2, "7g. SHIP: one record + Close + Open per block");
        CHECK(g_records.size() == 2 &&
              g_records[0].text == "APAX SEND DATA FAIL 5, 30030130230330430530630700000000 \n" &&
              g_records[1].text == "APAX SEND DATA FAIL 5, 30830931031131231331431500000000 \n", "7g. SHIP record texts: all 16 wData (golden :2594 / :2663)");
        CHECK(g_order == "WRCOWRCO", "7g. SHIP order");
#endif
        ResetCalls();
        g_writeScript.push_back(817);
        APAX_WriteData(true, 1, 0);
        CHECK(g_writes.size() == 2 && g_records.empty(), "7h. 817 counts as success");

        INSTALL_DOUBLE_EP = 4;
        ResetCalls();
        APAX_WriteData(false, 0, 2);
        CHECK(g_writes.size() == 1 && BlockIs(g_writes[0], 1), "7i. INSTALL_DOUBLE_EP 4 (>= 2, not MULTI) takes the INDIVIAL path");
    }

    // ---- 8. the value arrays are read only ----
    {
        bool same = true;
        for (int k = 0; k < 16; ++k)
            same = same && iAPAXEPValue[k] == 300 + k && iAPAXDualEPValue[k] == 100 + k;
        CHECK(same, "8. iAPAXEPValue / iAPAXDualEPValue unchanged");
    }

    // ---- restore (memory only; the seams and the fake table stay installed until exit) ----
    EP_Install = saveEP;
    INSTALL_DOUBLE_EP = saveDouble;
    iIndEPCnt = saveIndCnt;
    TestIF_File.iTestMode = saveTestMode;
    TestIF_File.iShuttleMode = saveShMode;
    TestIF_File.iShuttle_Sel = saveShSel;
    TestIF_File.bIndEPSLK = saveIndSlk;
    SW[SwMultiEp] = saveSw;
    std::memcpy(iAPAXEPValue, saveEPV, sizeof(saveEPV));
    std::memcpy(iAPAXDualEPValue, saveDualV, sizeof(saveDualV));
    bAPAXConnectFileAlarm = saveAlarm;
    bApaxWriteFinish = saveWF;
    bApaxReadFinish = saveRF;
    Address[2] = saveAddr2;
    std::memcpy(cAddress, saveCAddr, sizeof(saveCAddr));

#ifdef SOFT_SIMULTE
    std::printf("Adam6024_Apax (SIM): %d passed, %d failed\n", g_pass, g_fail);
#else
    std::printf("Adam6024_Apax (SHIP): %d passed, %d failed\n", g_pass, g_fail);
#endif
    W906_AdamEpLive_SetForTest(-1);  // (H4 integration pass 20261002) R4: back to the build default
    return g_fail == 0 ? 0 : 1;
}
