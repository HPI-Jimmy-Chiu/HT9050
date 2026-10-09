// =============================================================================
//  test_adam6024_comm.cpp -- ctest ADAM6024_Comm.  AI(W906-ST02-ADAM) 20261002 (St02-E helper H1).
//
//  Covers H1's files against golden 912 adam6024.cpp: Adam6024Comm_St02.cpp (the comm layer + TfAdam6024) and
//  Public/AdamTcp_St02.cpp (the ADAMTCP.dll run-time shim).  The vendor DLL is NEVER loaded and no socket is opened:
//  every ADAMTCP_* call goes to the fake table below (AdamTcp_St02_InstallApiForTest), the only exception is section 1,
//  which runs the shim with NO table and proves it binds nothing (ADAMTCP.dll is not mapped into the process, checked
//  with GetModuleHandleA before and after).  TfAdam6024's TClientSocket stays a vclcompat SIM socket (asserted).
//
//    0. containment first: exit 2 before any Handler code unless the ctest scratch roots are set
//       (st02_test_containment.h, the b12ab375 rule).
//    1. the DLL-missing path: no table -> not bound (SIM: [W906] S1; SHIP: the ctest refusal S2), every wrapper -1,
//       out-parameters untouched, the DLL never mapped; golden's own failure path (SHIP: Open_ADAM_6024 Num 0 false at
//       ADAMTCP_Open; SIM: golden's SIM arm true).  A table with a null entry is "not bound" too.
//    2. the shim's Close refcount ([W906] S3): Open 0 -> +1, a Close is forwarded only while an Open is outstanding,
//       otherwise absorbed and counted; a failed Open does not count.
//    3. error code -> ADAMErrorMessage: golden's 16 strings (:124-139) and the [W906] E1 bound check.
//    4. module queries (not SIM-gated in golden): GetModuleName / GetFirmwareName / GetModuleConnectionCount /
//       ClearAllConnection / GetModuleHostIdleTime / SetModuleHostIdleTime -- commands, timeouts, parsing, UDPClose
//       always called (golden Q7 / Q8).
//    5. FW / name / status checks: SIM answers; SHIP fCheckModuleFWISNew (B21 boundary), fCheckModuleName,
//       fCheckConnectStatus (each failure message, Disconnect + Connect, the absorbed Close).
//    6. open / close sequencing (SHIP): the full call order of Open_ADAM_6024(IP,0) first and second time, the range
//       sets (golden Q1 / Q2 / Q3), the iCount=90 quirk with the Double EP guide (Q4, [912] :368), EP_Install 0 / 4
//       (E4), Open_ADAM_6024() fan-out, Close_ADAM_6024; SIM: golden's arms (true, no I/O).
//    7. write path + conversion: ADAM_DirectWriteData (clamp, reg 11+iAdd, 1250..1252 -> 1253, EP_Install 5 channel
//       split, EP_Install 2 digital bits, EP_Install 4 -> WriteAO bytes ([W906] E3), bConnectStatus on failure),
//       ADAM_WriteMaxData constants, ADAM_WriteVoltage (v<0, v -> TransformFuntion(v), EP_Install 5 iWritePA quirk).
//    8. read path + conversion: ADAM_ReadVoltage (Read6KAI args, channel select, the iCH 3 hex formula, the
//       reconnect on failure Q5), ADAM_ReadPA (the interpolation), ADAM_ReadAIValue (dew-point integer slopes Q6).
//    9. TfAdam6024: ClientSocket1Connect resets cmdBuf, ClientSocket1Error clears ErrorCode and does nothing else (E2),
//       OpenSocket / CloseSocket on the SIM socket.
//  CONTROL: section 7's expected WriteAO bytes are written out by hand from golden :2087-2105, not computed by the
//  port's WriteAO; section 3's strings are golden's literals.
//  Not covered: a real module (on-machine, human review A); the reconnect-storm timing.
// =============================================================================
#define ADAM6024_ST02_INTERNAL
#include "Adam6024_St02.h"
#include "adam6024.h"                        // TransformFuntion
#include "Public/AdamTcp_St02.h"
#include "cmydef.h"
#include "cprod.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "myswitch.h"
#include "canary_support.h"                  // W906_ShowMyMessage_*
#include "EJ1N/TextProcess.h"                // float2hex (section 7 expectation)
#include "st02_test_containment.h"

#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

extern void (*W906_ShowMyMessageEx_Hook)(const char* S1, const char* S2, const char* S3, bool Ok, bool bServoOff);   // canary_support.cpp:154

static int g_pass = 0, g_fail = 0;
static void Check(bool c, const char* msg, int line)
{
    if (c) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else   { std::printf("  FAIL: %s  (line %d)\n", msg, line); ++g_fail; }
}
#define CHECK(c, msg) Check((c), (msg), __LINE__)

// ---- messages (ShowMyMessage S1 / S2 / S3) -----------------------------------------------------------------------
struct Msg { std::string s1, s2, s3; };
static std::vector<Msg> g_msgs;
static void OnMsg(const char* S1, const char* S2, const char* S3, bool, bool)
{
    Msg m;
    m.s1 = S1 ? S1 : "";
    m.s2 = S2 ? S2 : "";
    m.s3 = S3 ? S3 : "";
    g_msgs.push_back(m);
}

// ---- the fake ADAMTCP table --------------------------------------------------------------------------------------
struct Reply { int ret; std::string text; };
struct Fake
{
    std::vector<std::string> log;
    std::map<std::string, Reply> replies;    // key: the command without its trailing CR
    Reply defReply;                          // anything not in the map
    int retOpen, retConnect, retUDPOpen, retUDPClose, retWriteReg, retRead6KAI, retHostIdle, hostIdle;
    double ai[16];
    unsigned short hex[16];
    unsigned short gainSeen[10];
};
static Fake g_f;

static void FakeReset()
{
    g_f.log.clear();
    g_f.replies.clear();
    g_f.defReply.ret = 0;
    g_f.defReply.text = "!01";
    g_f.retOpen = 0; g_f.retConnect = 0; g_f.retUDPOpen = 0; g_f.retUDPClose = 0;
    g_f.retWriteReg = 0; g_f.retRead6KAI = 0; g_f.retHostIdle = 0; g_f.hostIdle = 0;
    for (int i = 0; i < 16; ++i) { g_f.ai[i] = 0.0; g_f.hex[i] = 0; }
    for (int i = 0; i < 10; ++i) g_f.gainSeen[i] = 0xFFFF;
}
static void Log(const std::string& s) { g_f.log.push_back(s); }
static std::string N(long v) { char b[32]; std::snprintf(b, sizeof(b), "%ld", v); return b; }
static std::string Seq()
{
    std::string s;
    for (size_t i = 0; i < g_f.log.size(); ++i) { s += g_f.log[i]; s += "|"; }
    return s;
}
static size_t Count(const std::string& prefix)
{
    size_t n = 0;
    for (size_t i = 0; i < g_f.log.size(); ++i)
        if (g_f.log[i].compare(0, prefix.size(), prefix) == 0) ++n;
    return n;
}

static int  __stdcall FOpen()  { Log("Open"); return g_f.retOpen; }
static void __stdcall FClose() { Log("Close"); }
static int  __stdcall FConnect(char* ip, unsigned short port, int ct, int st, int rt)
{ Log(std::string("Connect ") + ip + " " + N(port) + " " + N(ct) + " " + N(st) + " " + N(rt)); return g_f.retConnect; }
static void __stdcall FDisconnect() { Log("Disconnect"); }
static int  __stdcall FReadReg(char* ip, unsigned short id, unsigned short start, unsigned short cnt, unsigned short* w)
{ Log(std::string("ReadReg ") + ip + " " + N(id) + " " + N(start) + " " + N(cnt)); (void)w; return 0; }
static int  __stdcall FWriteReg(char* ip, unsigned short id, unsigned short start, unsigned short cnt, unsigned short* w)
{ Log(std::string("WriteReg ") + ip + " " + N(id) + " " + N(start) + " " + N(cnt) + " " + N(w ? w[0] : -1)); return g_f.retWriteReg; }
static int  __stdcall FUDPOpen(int st, int rt) { Log("UDPOpen " + N(st) + " " + N(rt)); return g_f.retUDPOpen; }
static int  __stdcall FUDPClose() { Log("UDPClose"); return g_f.retUDPClose; }
static int  __stdcall FCmd(char* ip, char* send, char* recv)
{
    std::string cmd = send ? send : "";
    const bool crTerminated = !cmd.empty() && cmd[cmd.size() - 1] == char(13);
    if (crTerminated) cmd.erase(cmd.size() - 1);
    Log(std::string("Cmd ") + ip + " " + cmd + (crTerminated ? "" : " (no CR)"));
    std::map<std::string, Reply>::const_iterator it = g_f.replies.find(cmd);
    const Reply& r = (it != g_f.replies.end()) ? it->second : g_f.defReply;
    if (recv) std::strcpy(recv, r.text.c_str());
    return r.ret;
}
static int  __stdcall FRead6KAI(char* ip, unsigned short module, unsigned short id, unsigned short* gain,
                                unsigned short* hex, double* val)
{
    Log(std::string("Read6KAI ") + ip + " " + N(module) + " " + N(id));
    for (int i = 0; i < 10; ++i) g_f.gainSeen[i] = gain[i];
    if (g_f.retRead6KAI == 0)
        for (int i = 0; i < 16; ++i) { hex[i] = g_f.hex[i]; val[i] = g_f.ai[i]; }
    return g_f.retRead6KAI;
}
static int  __stdcall FHostIdle(char* ip, int* t) { Log(std::string("HostIdle ") + ip); if (g_f.retHostIdle == 0) *t = g_f.hostIdle; return g_f.retHostIdle; }

static AdamTcpApi_St02 FakeApi()
{
    AdamTcpApi_St02 a;
    a.Open = FOpen; a.Close = FClose; a.Connect = FConnect; a.Disconnect = FDisconnect; a.ReadReg = FReadReg;
    a.WriteReg = FWriteReg; a.UDPOpen = FUDPOpen; a.UDPClose = FUDPClose; a.SendReceive6KUDPCmd = FCmd;
    a.Read6KAI = FRead6KAI; a.GetHostIdleTime = FHostIdle;
    return a;
}
static void InstallFake()
{
    AdamTcpApi_St02 a = FakeApi();
    AdamTcp_St02_InstallApiForTest(&a);
    FakeReset();
}
static bool DllMapped() { return ::GetModuleHandleA("ADAMTCP.dll") != NULL; }
static bool Near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("ADAM6024_Comm\n");
    // ---- 0. containment first -------------------------------------------------------------------------------------
    if (!W906TestInsideCtestRoots("ADAM6024_Comm"))
        return 2;
    W906_AdamEpLive_SetForTest(1);   // (H4 integration pass 20261002) R4: the golden bodies, not the OFF (stand-in) answers -- the switch defaults to OFF
#ifdef SOFT_SIMULTE
    const bool sim = true;
    std::printf("  build: SIM (SOFT_SIMULTE defined)\n");
#else
    const bool sim = false;
    std::printf("  build: SHIP (SOFT_SIMULTE not defined)\n");
#endif
    CHECK(!DllMapped(), "0. ADAMTCP.dll is not mapped at start");

    // fixture (declared here, never read from Gerneral.ini)
    EP_Install = 3;
    INSTALL_DOUBLE_EP = 0;
    USE_CKD_FCM_CleanAir = false;
    CHECK_EP_SETTING = 1;
    MachineTypeChoice = 0;
    Tri_Temp_Machine = 0;
    TestIF_File.bIndEPSLK = false;                 // IsIndependentEPPressureRouteActive()==false (golden :110-111)
    TestIF_File.iShuttleMode = 0;
    TestIF_File.iShuttle_Sel = 0;
    CosFunction.bUseDynamicKitDiameter = false;    // TransformFuntion's table-free branch
    for (int i = 0; i < 3; ++i) { bADAM6024FWIsNew[i] = false; bConnectStatus[i] = true; }
    for (int i = 0; i < 4; ++i) bADAM6420CheckRange[i] = true;
    for (int i = 0; i < 10; ++i) SW[SwEP_D0 + i].Enable = false;   // OutValue only, no IO
    if (fAdam6024 == NULL)
        fAdam6024 = new TfAdam6024(NULL);          // golden HT9045.cpp:239 CreateForm (W906_AdamFormShowOpen in the exe)
    W906_ShowMyMessageEx_Hook = OnMsg;

    // ---- 1. the DLL-missing path ------------------------------------------------------------------------------------
    std::printf("-- 1. no table: nothing is bound --\n");
    AdamTcp_St02_InstallApiForTest(0);
    {
        unsigned short w[16] = {7};
        double d[16] = {1.5};
        char rx[128] = "untouched";
        int idle = 42;
        CHECK(ADAMTCP_Open() == ADAMTCP_StartupFailure, "1a ADAMTCP_Open -> ADAMTCP_StartupFailure (-1)");
        CHECK(ADAMTCP_Connect("172.16.8.110", 502, 2000, 2000, 2000) == -1, "1b ADAMTCP_Connect -> -1");
        CHECK(ADAMTCP_ReadReg("172.16.8.110", 4, 33, 24, w) == -1 && w[0] == 7, "1c ADAMTCP_ReadReg -> -1, buffer untouched");
        CHECK(ADAMTCP_WriteReg("172.16.8.110", 1, 12, 1, w) == -1, "1d ADAMTCP_WriteReg -> -1");
        CHECK(ADAMTCP_UDPOpen(1000, 1000) == -1 && ADAMTCP_UDPClose() == -1, "1e ADAMTCP_UDPOpen / UDPClose -> -1");
        CHECK(ADAMTCP_SendReceive6KUDPCmd("172.16.8.110", "$01M\r", rx) == -1 && std::strcmp(rx, "untouched") == 0,
              "1f ADAMTCP_SendReceive6KUDPCmd -> -1, reply buffer untouched");
        CHECK(ADAMTCP_Read6KAI("172.16.8.110", 6017, 1, w, w, d) == -1 && d[0] == 1.5, "1g ADAMTCP_Read6KAI -> -1, values untouched");
        CHECK(ADAMTCP_GetHostIdleTime("172.16.8.110", &idle) == -1 && idle == 42, "1h ADAMTCP_GetHostIdleTime -> -1");
        ADAMTCP_Close();
        ADAMTCP_Disconnect();
        CHECK(AdamTcp_St02_IsBound() == false, "1i AdamTcp_St02_IsBound()==false");
        std::printf("    BindInfo: %s\n", AdamTcp_St02_BindInfo());
        CHECK(std::strstr(AdamTcp_St02_BindInfo(), sim ? "SIMULATION build" : "ctest environment") != 0,
              sim ? "1j BindInfo: SIM build never loads the vendor DLL ([W906] S1)"
                  : "1j BindInfo: the ctest refusal ([W906] S2)");
        CHECK(AdamTcp_St02_OpenRefCount() == 0 && AdamTcp_St02_AbsorbedCloseCount() == 0,
              "1k not bound: Close neither forwarded nor counted");
        CHECK(!DllMapped(), "1l ADAMTCP.dll still not mapped");

        bADAM6420Install = false;
        const bool ok = Open_ADAM_6024("172.16.8.110", 0);
        if (sim)
            CHECK(ok == true && bADAM6420Install == false, "1m SIM: Open_ADAM_6024 = golden :395-396 (true, no I/O)");
        else
            CHECK(ok == false && bADAM6420Install == false,
                  "1m SHIP: Open_ADAM_6024(Num 0) takes golden :278-284 (ADAMTCP_Open fail -> MyDBIProcess, false)");
        bConnectStatus[0] = true;
        ADAM_DirectWriteData(100, 0);
        CHECK(bConnectStatus[0] == false, "1n a write without the DLL -> golden :1995-1998 bConnectStatus[0]=false");
        bConnectStatus[0] = true;
    }
    {
        AdamTcpApi_St02 a = FakeApi();
        a.GetHostIdleTime = 0;
        AdamTcp_St02_InstallApiForTest(&a);
        CHECK(AdamTcp_St02_IsBound() == false && ADAMTCP_Open() == -1, "1o a table with a null entry is not bound (all-or-nothing)");
    }

    // ---- 2. the Close refcount ---------------------------------------------------------------------------------------
    std::printf("-- 2. ADAMTCP_Close refcount ([W906] S3) --\n");
    InstallFake();
    CHECK(AdamTcp_St02_IsBound() && std::strcmp(AdamTcp_St02_BindInfo(), "fake table (test)") == 0, "2a fake table bound");
    ADAMTCP_Close();
    CHECK(Count("Close") == 0 && AdamTcp_St02_AbsorbedCloseCount() == 1, "2b Close with no Open outstanding is absorbed");
    CHECK(ADAMTCP_Open() == 0 && AdamTcp_St02_OpenRefCount() == 1, "2c Open 0 -> refcount 1");
    ADAMTCP_Close();
    CHECK(Count("Close") == 1 && AdamTcp_St02_OpenRefCount() == 0, "2d the matching Close is forwarded");
    ADAMTCP_Close();
    CHECK(Count("Close") == 1 && AdamTcp_St02_AbsorbedCloseCount() == 2, "2e a second Close is absorbed");
    g_f.retOpen = -2;
    CHECK(ADAMTCP_Open() == -2 && AdamTcp_St02_OpenRefCount() == 0, "2f a failed Open does not count");
    g_f.retOpen = 0;
    FakeReset();
    CHECK(ADAMTCP_Connect("172.16.8.110", 502, 2000, 2000, 2000) == 0 && Seq() == "Connect 172.16.8.110 502 2000 2000 2000|",
          "2g Connect forwards the IP / port / three timeouts");

    // ---- 3. error code -> ADAMErrorMessage ---------------------------------------------------------------------------
    std::printf("-- 3. ADAMErrorMessage --\n");
    {
        static const char* const golden[16] = { "",
            "ADAM5KTCP_StartupFailure (-1)", "ADAM5KTCP_SocketFailure (-2)", "ADAM5KTCP_UdpSocketFailure (-3)",
            "ADAM5KTCP_SetTimeoutFailure (-4)", "ADAM5KTCP_SendFailure (-5)", "ADAM5KTCP_ReceiveFailure (-6)",
            "ADAM5KTCP_ExceedMaxFailure (-7)", "ADAM5KTCP_CreateWsaEventFailure (-8)", "ADAM5KTCP_ReadStreamDataFailure (-9)",
            "ADAM5KTCP_InvalidIP (-10)", "ADAM5KTCP_ThisIPNotConnected (-11)", "ADAM5KTCP_AlarmInfoEmpty (-12)",
            "ADAM5KTCP_NotSupportModule (-13)", "ADAM5KTCP_ExceedDONo (-14)", "ADAM5KTCP_InvalidRange (-15)" };
        bool all = true;
        for (int i = 0; i < 16; ++i)
            if (std::strcmp(fAdam6024->ADAMErrorMessage[i].c_str(), golden[i]) != 0 ||
                std::strcmp(W906_Adam6024_ErrorText(i).c_str(), golden[i]) != 0)
            {
                std::printf("    [%d] '%s'\n", i, fAdam6024->ADAMErrorMessage[i].c_str());
                all = false;
            }
        CHECK(all, "3a the 16 golden strings (:124-139), table and E1 accessor agree");
        CHECK(W906_Adam6024_ErrorText(0 - ADAMTCP_ThisIPNotConnected) == "ADAM5KTCP_ThisIPNotConnected (-11)",
              "3b iRet=0-(-11) -> ADAM5KTCP_ThisIPNotConnected (-11)");
        CHECK(W906_Adam6024_ErrorText(0 - ADAMTCP_EventError) == "ADAMTCP error (-100)",
              "3c -100 (outside golden's table) -> 'ADAMTCP error (-100)' ([W906] E1, golden reads past the array)");
        CHECK(W906_Adam6024_ErrorText(-817) == "ADAMTCP error (817)", "3d a positive DLL code -> no negative index");
        TfAdam6024* keep = fAdam6024;
        fAdam6024 = NULL;
        CHECK(W906_Adam6024_ErrorText(1) == "ADAMTCP error (-1)", "3e fAdam6024 not built -> text, no null dereference");
        fAdam6024 = keep;
    }

    // ---- 4. module queries -------------------------------------------------------------------------------------------
    std::printf("-- 4. module queries --\n");
    InstallFake();
    g_f.replies["$01M"] = Reply{0, "!016024-D"};
    CHECK(GetModuleName(0) == "16024-D" && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01M|UDPClose|",
          "4a GetModuleName: $01M<CR> over UDP 1000/1000, '!016024-D' -> '16024-D' (golden Q7)");
    FakeReset();
    g_f.replies["$01M"] = Reply{0, "?01"};
    CHECK(GetModuleName(1) == "" && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.111 $01M|UDPClose|", "4b '?01' -> '', Address[1]");
    FakeReset();
    g_f.retUDPOpen = -3;
    CHECK(GetModuleName(0) == "" && Seq() == "UDPOpen 1000 1000|UDPClose|", "4c UDPOpen fails -> '', no command, UDPClose still called");
    FakeReset();
    g_f.replies["$01F"] = Reply{0, "!01 6.01 B21"};
    CHECK(GetFirmwareName(0) == "6.01 B21" && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|",
          "4d GetFirmwareName: '!01 6.01 B21' -> '6.01 B21'");
    FakeReset();
    g_f.replies["%01GETMBTCPCN"] = Reply{0, "!05"};
    CHECK(GetModuleConnectionCount(0) == 5 && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|",
          "4e GetModuleConnectionCount: '!05' -> 5 (szRecv[2]-'0')");
    g_f.replies["%01GETMBTCPCN"] = Reply{0, "?01"};
    CHECK(GetModuleConnectionCount(0) == -1, "4f '?01' -> -1 (format error)");
    g_f.replies["%01GETMBTCPCN"] = Reply{-6, ""};
    CHECK(GetModuleConnectionCount(0) == -1, "4g command fails -> -1");
    FakeReset();
    g_f.replies["%01GETMBTCPCN"] = Reply{0, "!03"};
    CHECK(ClearAllConnection(0) == true && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|",
          "4h ClearAllConnection sends the count query and only checks '!' (golden Q8)");
    g_f.replies["%01GETMBTCPCN"] = Reply{0, "?01"};
    CHECK(ClearAllConnection(0) == false, "4i '?01' -> false");
    FakeReset();
    g_f.hostIdle = 30;
    CHECK(GetModuleHostIdleTime(0) == 30 && Seq() == "UDPOpen 1000 1000|HostIdle 172.16.8.110|UDPClose|",
          "4j GetModuleHostIdleTime(int): ADAMTCP_GetHostIdleTime, the built query never sent (golden Q8)");
    FakeReset();
    g_f.retHostIdle = ADAMTCP_ThisIPNotConnected;
    CHECK(GetModuleHostIdleTime(0) == -1, "4k GetHostIdleTime error -> -1");
    FakeReset();
    CHECK(SetModuleHostIdleTime(5) == false && g_f.log.empty(), "4l SetModuleHostIdleTime: golden body commented out -> false, no call");

    // ---- 5. FW / name / status checks -------------------------------------------------------------------------------
    std::printf("-- 5. FW / name / status checks --\n");
    InstallFake();
    if (sim)
    {
        g_f.replies["$01M"] = Reply{0, "!016017"};
        CHECK(fCheckModuleName_ADAM6024(0) == true && g_f.log.empty(), "5a SIM: fCheckModuleName true, no I/O (golden :2912)");
        CHECK(fCheckModuleFWISNew_ADAM6024(0) == false && g_f.log.empty(), "5b SIM: fCheckModuleFWISNew false, no I/O (:2932)");
        bADAM6024FWIsNew[0] = true;
        CHECK(fCheckConnectStatus_ADAM6024(0) == true && g_f.log.empty(), "5c SIM: fCheckConnectStatus true, no I/O (:3012-3014)");
        bADAM6024FWIsNew[0] = false;
    }
    else
    {
        g_f.replies["$01F"] = Reply{0, "!01 6.01 B21"};
        CHECK(fCheckModuleFWISNew_ADAM6024(0) == true, "5a FW '6.01 B21' -> new (>=21)");
        g_f.replies["$01F"] = Reply{0, "!01 6.01 B20"};
        CHECK(fCheckModuleFWISNew_ADAM6024(0) == false, "5b FW '6.01 B20' -> old");
        g_f.replies["$01F"] = Reply{0, "!01 6.02 B30"};
        CHECK(fCheckModuleFWISNew_ADAM6024(0) == false, "5c FW '6.02 B30' -> false (only 6.01 counts)");
        g_f.replies["$01F"] = Reply{-6, ""};
        CHECK(fCheckModuleFWISNew_ADAM6024(0) == false, "5d FW query fails -> false");
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        CHECK(fCheckModuleName_ADAM6024(0) == true, "5e name '16024-D' -> true");
        g_f.replies["$01M"] = Reply{0, "!016017"};
        CHECK(fCheckModuleName_ADAM6024(0) == false, "5f name '16017' -> false");

        bADAM6024FWIsNew[0] = false;
        FakeReset();
        CHECK(fCheckConnectStatus_ADAM6024(0) == true && g_f.log.empty(), "5g old FW -> true, nothing sent (:3007-3011)");
        bADAM6024FWIsNew[0] = true;
        FakeReset();
        g_f.replies["$01M"] = Reply{0, "!016017"};
        g_msgs.clear();
        CHECK(fCheckConnectStatus_ADAM6024(0) == false && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01M|UDPClose|" &&
              g_msgs.size() == 1 && g_msgs[0].s1 == "ADAM Read ModuleName Fail, Please Check Lan Cable",
              "5h wrong module -> 'ADAM Read ModuleName Fail, Please Check Lan Cable', false");
        FakeReset();
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        g_f.replies["%01GETMBTCPCN"] = Reply{0, "?01"};
        g_msgs.clear();
        CHECK(fCheckConnectStatus_ADAM6024(0) == false && g_msgs.size() == 1 && g_msgs[0].s1 == "Clear ADAM Connection Fail!",
              "5i clear fails -> 'Clear ADAM Connection Fail!', false");
        FakeReset();
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        g_f.replies["%01GETMBTCPCN"] = Reply{0, "!08"};
        g_msgs.clear();
        CHECK(fCheckConnectStatus_ADAM6024(0) == false && g_msgs.size() == 1 &&
              g_msgs[0].s1 == "Adam Clear Fail, Please Check Network Cable or IP address" && Count("Connect") == 0,
              "5j 8 connections -> 'Adam Clear Fail, ...', false, no connect");
        FakeReset();
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        g_f.replies["%01GETMBTCPCN"] = Reply{0, "!03"};
        CHECK(fCheckConnectStatus_ADAM6024(0) == true &&
              Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01M|UDPClose|"
                       "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|"
                       "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|"
                       "Disconnect|Connect 172.16.8.110 502 2000 2000 2000|",
              "5k new FW, all good: name, clear, count, Disconnect (ALL modules, Q9), Connect :2990-2991 -> true");
        InstallFake();                                     // refcount 0
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        g_f.replies["%01GETMBTCPCN"] = Reply{0, "!03"};
        g_f.retConnect = ADAMTCP_ThisIPNotConnected;
        g_msgs.clear();
        const bool r = fCheckConnectStatus_ADAM6024(0);
        CHECK(r == false && g_msgs.size() == 1 && g_msgs[0].s1 == "ADAMTCP Connect Fail..., Error Code:-11" &&
              Count("Close") == 0 && AdamTcp_St02_AbsorbedCloseCount() == 1,
              "5l connect fails -> 'ADAMTCP Connect Fail..., Error Code:-11'; its ADAMTCP_Close is absorbed (no Open outstanding, S3)");
        bADAM6024FWIsNew[0] = false;
    }

    // ---- 6. open / close sequencing ---------------------------------------------------------------------------------
    std::printf("-- 6. open / close --\n");
    InstallFake();
    if (sim)
    {
        bADAM6420Install = false;
        CHECK(Open_ADAM_6024("172.16.8.110", 0) == true && g_f.log.empty() && bADAM6420Install == false,
              "6a SIM: Open_ADAM_6024 true, no I/O, bADAM6420Install untouched (golden :395-396)");
        CHECK(Open_ADAM_6024() == true && g_f.log.empty(), "6b SIM: Open_ADAM_6024() true");
        bADAM6420Install = true;
        Close_ADAM_6024();
        CHECK(g_f.log.empty() && bADAM6420Install == true, "6c SIM: Close_ADAM_6024 does nothing (golden :413 #ifndef)");
    }
    else
    {
        // 6a first open of Num 0: FW query, ADAMTCP_Open, range check (all ranges already right), Connect
        bADAM6024FWIsNew[0] = false;
        bADAM6420CheckRange[0] = false;
        bADAM6420Install = false;
        for (int i = 0; i < 10; ++i) wGain[i] = 77;
        g_f.replies["$01F"] = Reply{0, "!01 6.01 B21"};
        for (int ch = 0; ch < 6; ++ch)
            g_f.replies["$01B0" + N(ch)] = Reply{0, ch == 3 ? "!0107" : "!0108"};
        g_f.replies["$01C00"] = Reply{0, "!0107"};
        g_f.replies["$01C01"] = Reply{0, "!0107"};
        const bool ok = Open_ADAM_6024("172.16.8.110", 0);
        bool gains = true;
        for (int i = 0; i < 10; ++i) if (wGain[i] != ADAMTCP_BI_10V) gains = false;
        CHECK(ok && Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|Open|UDPOpen 2000 2000|"
                             "Cmd 172.16.8.110 $01B00|Cmd 172.16.8.110 $01B01|Cmd 172.16.8.110 $01B02|"
                             "Cmd 172.16.8.110 $01B03|Cmd 172.16.8.110 $01B04|Cmd 172.16.8.110 $01B05|"
                             "Cmd 172.16.8.110 $01C00|Cmd 172.16.8.110 $01C01|UDPClose|"
                             "Connect 172.16.8.110 502 2000 2000 2000|",
              "6a first open (Num 0): FW, ADAMTCP_Open, 6 AI + 2 AO range reads, UDPClose, Connect :502 2000/2000/2000");
        CHECK(bADAM6420Install && bADAM6024FWIsNew[0] && bADAM6420CheckRange[0] && gains && Address[0] == "172.16.8.110" &&
              AdamTcp_St02_OpenRefCount() == 1,
              "6b afterwards: installed, FW new, range checked, every wGain = ADAMTCP_BI_10V, one Open outstanding");

        // 6c second open: the new-FW status path runs first, the range check does not
        FakeReset();
        g_f.replies["$01F"] = Reply{0, "!01 6.01 B21"};
        g_f.replies["$01M"] = Reply{0, "!016024-D"};
        g_f.replies["%01GETMBTCPCN"] = Reply{0, "!03"};
        CHECK(Open_ADAM_6024("172.16.8.110", 0) &&
              Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01M|UDPClose|"
                       "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|"
                       "UDPOpen 1000 1000|Cmd 172.16.8.110 %01GETMBTCPCN|UDPClose|"
                       "Disconnect|Connect 172.16.8.110 502 2000 2000 2000|"
                       "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|Open|Connect 172.16.8.110 502 2000 2000 2000|",
              "6c second open: status check (name / clear / count / reconnect), FW, ADAMTCP_Open again, no range check");
        CHECK(AdamTcp_St02_OpenRefCount() == 2, "6d golden opens again without a Close: refcount 2");

        // 6e range sets: AI ch0 reads 7 (should be 8), ch3 reads 8 (should be 7), AO ch0 reads 1 (golden Q2: != 7 -> set)
        FakeReset();
        bADAM6024FWIsNew[0] = false;
        bADAM6420CheckRange[0] = false;
        g_f.replies["$01F"] = Reply{0, "?01"};
        for (int ch = 0; ch < 6; ++ch)
            g_f.replies["$01B0" + N(ch)] = Reply{0, ch == 3 ? "!0107" : "!0108"};
        g_f.replies["$01B00"] = Reply{0, "!0107"};
        g_f.replies["$01B03"] = Reply{0, "!0108"};
        g_f.replies["$01C00"] = Reply{0, "!0101"};
        g_f.replies["$01C01"] = Reply{0, "!0107"};
        CHECK(Open_ADAM_6024("172.16.8.110", 0) &&
              Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|Open|UDPOpen 2000 2000|"
                       "Cmd 172.16.8.110 $01B00|Cmd 172.16.8.110 $01A0008|Cmd 172.16.8.110 $01B01|Cmd 172.16.8.110 $01B02|"
                       "Cmd 172.16.8.110 $01B03|Cmd 172.16.8.110 $01A0307|Cmd 172.16.8.110 $01B04|Cmd 172.16.8.110 $01B05|"
                       "Cmd 172.16.8.110 $01C00|Cmd 172.16.8.110 $01C0001|Cmd 172.16.8.110 $01C01|UDPClose|"
                       "Connect 172.16.8.110 502 2000 2000 2000|",
              "6e range sets: AI ch0 -> $01A0008 (-10..10 V), ch3 -> $01A0307 (4-20 mA), AO ch0 read 1 -> $01C0001 (golden Q2)");

        // 6f CHECK_EP_SETTING==0: no range reads; the read-back stays 0 (golden Q1) so every channel is SET
        FakeReset();
        bADAM6420CheckRange[0] = false;
        CHECK_EP_SETTING = 0;
        g_f.replies["$01F"] = Reply{0, "?01"};
        CHECK(Open_ADAM_6024("172.16.8.110", 0) &&
              Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|Open|UDPOpen 2000 2000|"
                       "Cmd 172.16.8.110 $01A0008|Cmd 172.16.8.110 $01A0108|Cmd 172.16.8.110 $01A0208|"
                       "Cmd 172.16.8.110 $01A0307|Cmd 172.16.8.110 $01A0408|Cmd 172.16.8.110 $01A0508|"
                       "Cmd 172.16.8.110 $01C0001|Cmd 172.16.8.110 $01C0101|UDPClose|"
                       "Connect 172.16.8.110 502 2000 2000 2000|",
              "6f CHECK_EP_SETTING=0: Get*Range return true with the pointer quirk (golden Q1) -> every range is set");
        CHECK_EP_SETTING = 1;

        // 6g a failed range read: ShowMyMessage, false, no UDPClose (golden Q3), no Connect
        FakeReset();
        bADAM6420CheckRange[0] = false;
        g_f.replies["$01F"] = Reply{0, "?01"};
        g_f.replies["$01B00"] = Reply{ADAMTCP_ReceiveFailure, ""};
        g_msgs.clear();
        bADAM6420Install = false;
        CHECK(Open_ADAM_6024("172.16.8.110", 0) == false &&
              Seq() == "UDPOpen 1000 1000|Cmd 172.16.8.110 $01F|UDPClose|Open|UDPOpen 2000 2000|Cmd 172.16.8.110 $01B00|" &&
              g_msgs.size() == 1 && g_msgs[0].s1 == "Failed to Get ADAM AI Range! IP=172.16.8.110, Channel=0, Readrange=0" &&
              bADAM6420CheckRange[0] == false && bADAM6420Install == false,
              "6g range read fails -> 'Failed to Get ADAM AI Range! ...', false, no UDPClose (Q3), still unchecked");
        bADAM6420CheckRange[0] = true;

        // 6h ADAMTCP_Connect failures (golden Q4) on Num 2 with INSTALL_DOUBLE_EP=2: one success first (iCount=50),
        //    50 failures still return true, the 51st alarms with the [912] Double EP guide + 'Connect Fail!'.
        INSTALL_DOUBLE_EP = DOUBLE_EP_INDIVIAL;
        FakeReset();
        g_f.replies["$01F"] = Reply{0, "?01"};
        CHECK(Open_ADAM_6024("172.16.8.112", 2) == true && Count("Open") == 0 && Count("Cmd 172.16.8.112 $01B") == 0,
              "6h Num 2: no ADAMTCP_Open (Num!=0), no range check (Num==2, :287); connect ok -> iCount=50");
        g_f.retConnect = ADAMTCP_ThisIPNotConnected;
        int trues = 0;
        for (int i = 0; i < 50; ++i)
        {
            bADAM6420Install = false;
            if (Open_ADAM_6024("172.16.8.112", 2) && bADAM6420Install) ++trues;
        }
        CHECK(trues == 50, "6i golden Q4: 50 connect failures still return true and set bADAM6420Install");
        g_msgs.clear();
        bADAM6420Install = false;
        const bool r51 = Open_ADAM_6024("172.16.8.112", 2);
        CHECK(r51 == false && g_msgs.size() == 2 &&
              g_msgs[0].s1.find("Double EP board connect failed.") == 0 &&
              g_msgs[0].s1.find("Mode=2 is Individual EP. Expected IP=172.16.8.112.") != std::string::npos &&
              g_msgs[0].s1.find("ADAM5KTCP_ThisIPNotConnected (-11)") != std::string::npos &&
              g_msgs[1].s1 == "Connect Fail! Please Check ADAM IP!" && g_msgs[1].s2 == "172.16.8.112" &&
              g_msgs[1].s3 == "ADAM5KTCP_ThisIPNotConnected (-11)" && bADAM6420Install == false,
              "6j the 51st: [912] Double EP guide (with the error text) then 'Connect Fail! Please Check ADAM IP!' | IP | text");
        g_msgs.clear();
        CHECK(Open_ADAM_6024("172.16.8.112", 2) == true && g_msgs.empty(), "6k iCount restarts at 0 -> true again, no message");
        // the guide is shown once per run: a status failure now shows only its own message
        bADAM6024FWIsNew[2] = true;
        FakeReset();
        g_f.replies["$01M"] = Reply{0, "!016017"};
        g_msgs.clear();
        CHECK(Open_ADAM_6024("172.16.8.112", 2) == false && g_msgs.size() == 1 &&
              g_msgs[0].s1 == "ADAM Read ModuleName Fail, Please Check Lan Cable",
              "6l status check fails -> false; the Double EP guide is not shown a second time (:75 once per run)");
        bADAM6024FWIsNew[2] = false;
        INSTALL_DOUBLE_EP = 0;

        // 6m EP_Install 0 / 4
        FakeReset();
        EP_Install = 0;
        CHECK(Open_ADAM_6024("172.16.8.110", 0) == false && g_f.log.empty(), "6m EP_Install=0 -> false, nothing sent (:391-394)");
        EP_Install = 4;
        bADAM6420Install = false;
        g_f.replies["$01F"] = Reply{0, "?01"};
        CHECK(Open_ADAM_6024("172.16.8.110", 0) == false && Count("Open") == 0 && Count("Connect") == 0 &&
              bADAM6420Install == false && fAdam6024->ClientSocket1->IsActiveNow() == false,
              "6n EP_Install=4 (PISO DA): [W906] E4 not wired -> golden's 'not installed' false, socket not opened");
        EP_Install = 3;

        // 6o Open_ADAM_6024(): the three boards
        FakeReset();
        g_f.retConnect = 0;
        g_f.replies["$01F"] = Reply{0, "?01"};
        CHECK(Open_ADAM_6024() && Count("Connect") == 1 && Count("Connect 172.16.8.110") == 1,
              "6o Open_ADAM_6024(): only 172.16.8.110 without CKD clean air / double EP");
        FakeReset();
        g_f.replies["$01F"] = Reply{0, "?01"};
        USE_CKD_FCM_CleanAir = true;
        INSTALL_DOUBLE_EP = DOUBLE_EP_MULTI;
        CHECK(Open_ADAM_6024() && Count("Connect") == 3 && g_f.log.back() == "Connect 172.16.8.112 502 2000 2000 2000" &&
              Count("Connect 172.16.8.111") == 1,
              "6p CKD clean air + Multi EP: 172.16.8.110, .111, .112 in that order");
        USE_CKD_FCM_CleanAir = false;
        INSTALL_DOUBLE_EP = 0;

        // 6q Close_ADAM_6024
        FakeReset();
        bADAM6420Install = true;
        const int refBefore = AdamTcp_St02_OpenRefCount();
        Close_ADAM_6024();
        CHECK(Seq() == "Disconnect|Close|" && bADAM6420Install == false && AdamTcp_St02_OpenRefCount() == refBefore - 1,
              "6q Close_ADAM_6024: Disconnect + Close (forwarded: an Open is outstanding), not installed afterwards");
        FakeReset();
        Close_ADAM_6024();
        CHECK(g_f.log.empty(), "6r not installed -> Close_ADAM_6024 does nothing");
    }

    // ---- 7. write path -----------------------------------------------------------------------------------------------
    std::printf("-- 7. write path --\n");
    InstallFake();
    EP_Install = 3;
    bConnectStatus[0] = true;
    bConnectStatus[1] = true;
    bADAM6024FWIsNew[0] = false;
    ADAM_DirectWriteData(5000, 0);
    CHECK(Seq() == "WriteReg 172.16.8.110 1 12 1 4095|" && iWritePA == 4095 && bCanReadData == true,
          "7a EP_Install=3: 5000 clamped to 4095, register 11+iAdd(1)=12, iWritePA, bCanReadData restored");
    FakeReset();
    ADAM_DirectWriteData(1251, 0, 0);
    CHECK(Seq() == "WriteReg 172.16.8.110 1 11 1 1253|", "7b 1251 -> 1253 (golden Q10), iAdd 0 -> register 11");
    FakeReset();
    ADAM_DirectWriteData(1250, 0); ADAM_DirectWriteData(1252, 0); ADAM_DirectWriteData(1249, 0); ADAM_DirectWriteData(1253, 0);
    CHECK(Seq() == "WriteReg 172.16.8.110 1 12 1 1253|WriteReg 172.16.8.110 1 12 1 1253|"
                   "WriteReg 172.16.8.110 1 12 1 1249|WriteReg 172.16.8.110 1 12 1 1253|",
          "7c 1250 / 1252 -> 1253, 1249 and 1253 unchanged");
    FakeReset();
    {
        int iNeg = -1;
        ADAM_DirectWriteData(iNeg, 1);
    }
    CHECK(Seq() == "WriteReg 172.16.8.111 1 12 1 4095|", "7d an int -1 becomes WORD 65535 -> clamped 4095; Num 1 -> Address[1]");
    FakeReset();
    g_f.retWriteReg = ADAMTCP_ThisIPNotConnected;
    ADAM_DirectWriteData(100, 0);
    CHECK(bConnectStatus[0] == false, "7e WriteReg fails -> bConnectStatus[0]=false (:1995-1998)");
    g_f.retWriteReg = 0;
    ADAM_DirectWriteData(100, 0);
    CHECK(bConnectStatus[0] == true, "7f the next write re-checks the status (old FW -> true) and writes");
    EP_Install = 5;
    FakeReset();
    ADAM_DirectWriteData(1251, 0, 10); ADAM_DirectWriteData(700, 0, 11); ADAM_DirectWriteData(9000, 0);
    CHECK(Seq() == "WriteReg 172.16.8.110 1 11 1 1251|WriteReg 172.16.8.110 1 12 1 700|"
                   "WriteReg 172.16.8.110 1 11 1 4095|WriteReg 172.16.8.110 1 12 1 4095|",
          "7g EP_Install=5: iAdd 10 -> reg 11, 11 -> reg 12, else both; no 1250 rule");
    EP_Install = 2;
    FakeReset();
    ADAM_DirectWriteData(5, 0);
    bool bits5 = true;
    for (int i = 0; i < 10; ++i) if (SW[SwEP_D0 + i].OutValue != (i == 0 || i == 2)) bits5 = false;
    ADAM_DirectWriteData(2000, 0);
    bool bits1022 = true;
    for (int i = 0; i < 10; ++i) if (SW[SwEP_D0 + i].OutValue != (i != 0)) bits1022 = false;
    CHECK(g_f.log.empty() && bits5 && bits1022,
          "7h EP_Install=2: 10-bit digital EP on SW[SwEP_D0..+9], LSB first; 2000 clamped to 1022; nothing over ADAMTCP");
    EP_Install = 0;
    FakeReset();
    ADAM_DirectWriteData(100, 0);
    CHECK(g_f.log.empty(), "7i EP_Install=0: nothing");
    EP_Install = 4;
    CHECK(fAdam6024->ClientSocket1->IsSimMode(), "7j TfAdam6024::ClientSocket1 is a vclcompat SIM socket (no OS socket)");
    fAdam6024->ClientSocket1->Socket->SimClearTx();
    ADAM_DirectWriteData(3, 0);
    {
        const std::vector<char>& tx = fAdam6024->ClientSocket1->Socket->SimTxBuffer();
        const unsigned h0 = float2hex(0x35, 3.0, 0), hz = float2hex(0x35, 0.0, 0);
        // golden :2086-2107 by hand: [0]=2 [5]=15 [6]=NetID [7]=16, ref 0, word count 4 big-endian at [10..11],
        // byte count 8 at [12], then the four AO values big-endian at [13..20]; 21 bytes sent.
        const unsigned char want[21] = { 2, 0, 0, 0, 0, 15, 1, 16, 0, 0, 0, 4, 8,
            (unsigned char)((h0 >> 8) & 0xFF), (unsigned char)(h0 & 0xFF),
            (unsigned char)((hz >> 8) & 0xFF), (unsigned char)(hz & 0xFF),
            (unsigned char)((hz >> 8) & 0xFF), (unsigned char)(hz & 0xFF),
            (unsigned char)((hz >> 8) & 0xFF), (unsigned char)(hz & 0xFF) };
        bool same = tx.size() == 21;
        for (size_t i = 0; same && i < 21; ++i) if ((unsigned char)tx[i] != want[i]) same = false;
        CHECK(same && g_f.log.empty(), "7k EP_Install=4: WriteAO frame byte for byte (golden :2086-2107, [W906] E3), SIM socket only");
    }
    EP_Install = 3;
    FakeReset();
    DeviceForm_File.dKitDiameter = 2.0;
    EP_MAXKPA = 500;  ADAM_WriteMaxData(true);
    EP_MAXKPA = 1000; ADAM_WriteMaxData(true);
    DeviceForm_File.dKitDiameter = 3.0;
    EP_MAXKPA = 500;  ADAM_WriteMaxData(true);
    EP_MAXKPA = 1000; ADAM_WriteMaxData(true);
    ADAM_WriteMaxData(false);
    CHECK(Seq() == "WriteReg 172.16.8.110 1 12 1 820|WriteReg 172.16.8.110 1 12 1 455|WriteReg 172.16.8.110 1 12 1 4095|"
                   "WriteReg 172.16.8.110 1 12 1 2275|WriteReg 172.16.8.110 1 12 1 0|",
          "7l ADAM_WriteMaxData: 820 / 455 (kit <= 2.5), 4095 / 2275, false -> 0");
    EP_Install = 0;
    FakeReset();
    CHECK(ADAM_WriteVoltage(10.0) == false && g_f.log.empty(), "7m EP_Install=0: ADAM_WriteVoltage false, nothing");
    EP_Install = 3;
    CHECK(ADAM_WriteVoltage(-1.0) == true && Seq() == "WriteReg 172.16.8.110 1 12 1 0|", "7n v<0 -> ADAM_DirectWriteData(0,0)");
    {
        const int expect = TransformFuntion(20.0);
        std::printf("    TransformFuntion(20.0) = %d\n", expect);
        const unsigned short asWord = (unsigned short)expect;          // golden passes the int to a WORD parameter
        int sent = asWord > 4095 ? 4095 : asWord;                        // CheckRange(data, 0, 4095)
        if (sent == 1250 || sent == 1251 || sent == 1252) sent = 1253;
        FakeReset();
        CHECK(ADAM_WriteVoltage(20.0) == true && Seq() == "WriteReg 172.16.8.110 1 12 1 " + N(sent) + "|" &&
              iWritePA == expect && Near(iAdamOutValue, AdamOutputToPA(expect)),
              "7o v -> TransformFuntion(v) on reg 12; iWritePA = the unclamped code, iAdamOutValue = AdamOutputToPA (:1912-1913)");
        EP_Install = 5;
        const int e0 = TransformFuntion(20.0, false, false, 0), e1 = TransformFuntion(20.0, false, false, 1);
        FakeReset();
        CHECK(ADAM_WriteVoltage(20.0) == true && Count("WriteReg 172.16.8.110 1 11 1") == 1 && Count("WriteReg 172.16.8.110 1 12 1") == 1 &&
              iWritePA == 0,
              "7p EP_Install=5: arm 0 on reg 11, arm 1 on reg 12; golden leaves iInputValue 0 -> iWritePA=0");
        (void)e0; (void)e1;
        EP_Install = 3;
    }

    // ---- 8. read path ------------------------------------------------------------------------------------------------
    std::printf("-- 8. read path --\n");
    InstallFake();
    EP_Install = 3;
    bConnectStatus[0] = true;
    bADAM6420Install = false;
    CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && g_f.log.empty(), "8a not installed -> 0, nothing read");
    bADAM6420Install = true;
    for (int i = 0; i < 16; ++i) g_f.ai[i] = i + 0.25;
    g_f.hex[3] = 2048;
    for (int i = 0; i < 10; ++i) wGain[i] = 0;
    wGain[3] = ADAMTCP_UNI_4TO20mA;
    CHECK(Near(ADAM_ReadVoltage(0, 5), 5.25) && Seq() == "Read6KAI 172.16.8.110 6017 1|" && g_f.gainSeen[3] == ADAMTCP_UNI_4TO20mA,
          "8b ADAM_ReadVoltage(0,5): Read6KAI(Address[0], 6017, 1, wGain, ...) -> fValue[5]");
    Tri_Temp_Machine = 0;
    CHECK(Near(ADAM_ReadVoltage(0, 3), 2048 / 4095.9375 + 4), "8c iCH 3 (dew point): wHex[3]/4095.9375+4");
    Tri_Temp_Machine = 1;
    CHECK(Near(ADAM_ReadVoltage(0, 3), 3.25), "8d iCH 3 on a tri-temp 1032: fValue[3]");
    Tri_Temp_Machine = 0;
    EP_MAXAFB = 5.0; EP_MinAFB = 1.0; EP_MINMPA = 0.1; EP_MAXKPA = 500;
    g_f.ai[5] = 3.0;
    {
        double d = 0;
        CHECK(ADAM_ReadPA(&d) == 300 && Near(d, 3.0), "8e ADAM_ReadPA: 100 + (500-100)*(3-1)/(5-1) = 300 kPa, *dValue = volts");
        EP_MinAFB = 0.0;
        CHECK(ADAM_ReadPA(&d) == 3, "8f EP_MinAFB==0 -> the voltage itself (int)");
        EP_MinAFB = 1.0;
        EPDual_MAXAFB = 2.0; EPDual_MinAFB = 2.0;
        g_f.ai[2] = 4.5;
        CHECK(ADAM_ReadPA(&d, 2) == 4 && Near(d, 4.5), "8g iCH 2 with EPDual_MAXAFB==EPDual_MinAFB -> zero-guard returns v");
    }
    {
        double mA = 0, deg = 0;
        DewPoint_Hardware_Install = 1;
        if (sim)
        {
            CHECK(ADAM_ReadAIValue(0, 5, &mA, &deg) && Near(mA, 5.877) && Near(deg, -46.9),
                  "8h SIM: 5.87654 -> 5.877 mA -> -60+7*(5.877-4) = -46.9 C (60/8 is the integer 7, golden Q6)");
        }
        else
        {
            g_f.ai[5] = 12.0;
            DewPoint_Hardware_Install = 2;
            CHECK(ADAM_ReadAIValue(0, 5, &mA, &deg) && Near(mA, 12.0) && Near(deg, -32.0),
                  "8h SHIP: 12 mA, -80~+20 sensor: -80+6*8 = -32.0 C (50/8 is the integer 6, golden Q6)");
            g_f.ai[5] = 16.0;
            CHECK(ADAM_ReadAIValue(0, 5, &mA, &deg) && Near(deg, -6.0), "8i 16 mA: -30+6*4 = -6.0 C");
        }
        DewPoint_Hardware_Install = 0;
        CHECK(ADAM_ReadAIValue(0, 5, &mA, &deg) && Near(deg, 9999.0), "8j no dew-point sensor -> 9999");
        EP_Install = 0;
        CHECK(ADAM_ReadAIValue(0, 5, &mA, &deg) == false, "8k EP_Install=0 -> false");
        EP_Install = 3;
    }
    FakeReset();
    g_f.retRead6KAI = ADAMTCP_ReceiveFailure;
    g_f.replies["$01F"] = Reply{0, "?01"};
    for (int i = 0; i < 4; ++i) bADAM6420CheckRange[i] = true;
    bADAM6024FWIsNew[0] = false;
    CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0), "8l a failed read returns 0");
    if (sim)
        CHECK(Seq() == "Read6KAI 172.16.8.110 6017 1|", "8m SIM: Close/Open do no I/O");
    else
        CHECK(g_f.log.size() > 2 && g_f.log[1] == "Disconnect" && Count("Connect 172.16.8.110") == 1 && bADAM6420Install,
              "8m SHIP: golden Q5 -- Close_ADAM_6024 (Disconnect) + Open_ADAM_6024() (connect again) on every failed read");
    // AI(W906-W191) 20261009 (St02-E): golden 913 adam6024.cpp:467-482 reopen cooldown (RogerYang 20260914).
    if (sim)
    {
        bADAM6420Install = true;
        g_f.log.clear();
        CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && Seq() == "Read6KAI 172.16.8.110 6017 1|", "8n SIM: Open_ADAM_6024() is golden true with no I/O, so no cooldown ever starts");
    }
    else
    {
        bADAM6420Install = true;
        g_f.retOpen = -1;                                // ADAMTCP_Open fails -> Open_ADAM_6024(IP,0) false (golden :278-284)
        g_f.log.clear();
        CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && Count("Disconnect") >= 1 && Count("Open") == 1 && bADAM6420Install == false,
              "8n SHIP: a failed read whose reopen FAILS -> Close + one reopen try (Open logged once), install false");
        bADAM6420Install = true;                          // something else marked it installed again (the :553 early return no longer holds)
        g_f.log.clear();
        CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && Count("Disconnect") >= 1 && Count("Open") == 0 && Count("Connect") == 0,
              "8o SHIP: the next failed read inside the 5 s cooldown -> Close only, NO reopen (golden 913; 0618 reopened every time)");
        ::Sleep(5200);
        g_f.retOpen = 0;
        bADAM6420Install = true;
        g_f.log.clear();
        CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && Count("Open") == 1 && bADAM6420Install == true,
              "8p SHIP: after the cooldown a failed read reopens again; it succeeds -> installed");
        g_f.log.clear();
        CHECK(Near(ADAM_ReadVoltage(0, 5), 0.0) && Count("Open") == 1,
              "8q SHIP: after a successful reopen the next failure reopens at once (no cooldown)");
    }
    g_f.retRead6KAI = 0;

    // ---- 9. TfAdam6024 -----------------------------------------------------------------------------------------------
    std::printf("-- 9. TfAdam6024 --\n");
    {
        TfAdam6024 f(NULL);
        CHECK(f.ClientSocket1 != NULL && f.ClientSocket1->Address == "172.16.8.110" && f.ClientSocket1->Port == 10001 &&
              f.ClientSocket1->IsSimMode() && f.bufIdx == 0 && f.cmdBuf[7].status == 0,
              "9a ctor: .dfm Address 172.16.8.110 / Port 10001, SIM socket, VCL zero-fill");
        if (f.ClientSocket1 != NULL && f.ClientSocket1->IsSimMode())      // never open a real socket from a test
        {
            f.cmdBuf[3].status = 5; f.cmdBuf[3].cmdBuf[0] = 'x'; f.cmdBuf[3].cmdSize = 9; f.bufIdx = 7;
            f.OpenSocket("172.16.8.110", 10001);
            CHECK(f.ClientSocket1->IsActiveNow() && f.cmdBuf[3].status == 0 && f.cmdBuf[3].cmdBuf[0] == 0 && f.cmdBuf[3].cmdSize == 0 &&
                  f.bufIdx == 0,
                  "9b OpenSocket (SIM): OnConnect = ClientSocket1Connect clears the eight command buffers (:2055-2061)");
            int ec = 10061;
            f.ClientSocket1Error(NULL, f.ClientSocket1->Socket, eeConnect, ec);
            CHECK(ec == 0 && f.ClientSocket1->IsActiveNow(), "9c ClientSocket1Error: ErrorCode=0, then Abort() -- nothing else ([W906] E2)");
            f.CloseSocket();
            CHECK(!f.ClientSocket1->IsActiveNow(), "9d CloseSocket");
        }
        else
        {
            CHECK(false, "9b-9d skipped: the TfAdam6024 socket is not a SIM socket (nothing opened)");
        }
    }

    AdamTcp_St02_InstallApiForTest(0);
    W906_ShowMyMessageEx_Hook = 0;
    CHECK(!DllMapped(), "end: ADAMTCP.dll was never mapped into this process");
    W906_AdamEpLive_SetForTest(-1);  // (H4 integration pass 20261002) R4: back to the build default
    std::printf("ADAM6024_Comm: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
