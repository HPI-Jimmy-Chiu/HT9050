// =============================================================================
//  tests/test_adam6024_flow.cpp -- ST02-ADAM, helper H4: the integrated EP flow (Adam6024Integrate_St02.cpp over
//  H1's Adam6024Comm_St02.cpp / Public/AdamTcp_St02.cpp, H2's Adam6024Pressure_St02.cpp, H3's Adam6024Apax_St02.cpp).
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H4).  Suite: Adam6024_Flow.  Both configurations (SIM / SHIP).
//
//  CONTAINMENT FIRST (st02_test_containment.h): refuses (exit 2) unless the ctest redirect roots are set.  Memory only:
//  ADAMTCP.dll is NEVER loaded -- a fake table (AdamTcp_St02_InstallApiForTest) is installed before any ADAM call, and
//  section M's "DLL missing" relies on H1's [W906] S1 / S2 (SIM build / inside ctest: the real DLL is never loaded).
//  The fake never opens a socket; the IPs it sees are golden's literals, recorded and compared, nothing more.
//
//  WHAT IT PINS (golden 912 lines; 906_0625_Steven in brackets)
//    OFF  the live switch off (W906_AdamEpLive()==false): no transport call from the integration, the boot, the
//         Timer2 tick, the exit, the home check, the uhome condition -- and (R2, St02-E reconcile) from the stand-in'd
//         entry points ADAM_DirectWriteData / ADAM_WriteVoltage / ADAM_Alarm.
//    A    boot: main.cpp:9890 [9457] Open_ADAM_6024() + HT9045.cpp:239 CreateForm guard.
//    B    Timer2 production writer main.cpp:21677-21803 [21058-21184]: first tick writes DeviceForm.dPress through
//         TransformFuntion to register 12 (11+iAdd, iAdd default 1); no write while nothing changes; the ct>=10
//         refresh; the InitialOK guard :21495 and the 1 s limiter of W906_AdamTimer2Pump.
//    C    the server contact force :21688-21692 (fProductionInfo, [W906] D2 fContactForm->dDutCount).
//    D    THE HOLD-BACK SCENARIO (3c348627 B1): AutoClean writes TestIF.fAutoClean_AireForce (AutoClean.cpp 912
//         :8498 etc.); while bRunAutoClean the pump keeps the AutoClean force (:21753-21757); when AutoClean ends the
//         next tick restores the production force (:21742 / :21761), so the next production touchdown presses with
//         the production force.  Plus golden's latency quirk: a short AutoClean is restored by the ct>=10 refresh.
//    E    heater door :21745-21749 (0 kg while a door is open and no real IC is on the test heads, then dPress again).
//    F    Contact / IO page showing :21677-21683 (no write; leaving the page re-writes at once).
//    G    die force :21716-21731 (register 11 = 11+0; AutoClean die force :21719-21720).
//    H    Start, Double EP main.cpp:6500-6521 [6233-6254] = WebStart.cpp:3762-3782: TransformFuntion(DoubleForce,true)
//         -> ADAM_DirectWriteData(iInputValue, 0, 0) -> register 11, not while the Contact page shows.
//    I    3c348627 M2: ADAM_ReturnValueCheck() in the pump (:22354-22355 [21724-21725]): the 60-tick keep-alive read
//         (Read6KAI 6017, 1) and, with [D24], WAR16322 after 100 bad ticks (SHIP; SIM compiles it out, golden :2677).
//    J    home: csystem.cpp:10999 [10386] ADAM_ReturnValueCheck(true) -> WAR16322 + fAllMotorHome=false below 0.8 V.
//    K0   the IO page helpers (not wired yet): iosetview.cpp:495-522 [404-431] EP zero, :293-294 [219-220] Close / Open.
//    K    3c348627 M1: exit main.cpp:11947-11952 [11462-11467] (register 12 ends at 0) and :12194 [11677] Close.
//    L    3c348627 B2: H1's Close refcount -- an unmatched ADAMTCP_Close (golden fCheckConnectStatus :2996 on a failed
//         Connect) never reaches the DLL's WSACleanup.
//    M    DLL missing: every wrapper -1; SHIP Open_ADAM_6024()==false -> the uhome condition (claim C-4) stops HOME in
//         REALLY mode (golden uhome.cpp:1923-1928 [1800-1805]); SIM true (golden :395-396); writes do not crash.
// =============================================================================
#include "Adam6024_St02.h"
#include "Public/AdamTcp_St02.h"
#include "adam6024.h"                        // TransformFuntion
#include "atester_shims.h"                   // fContact / fiosetview (needs claim C-1)
#include "csystem.h"                         // bHeaterDoorIsOpen
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "MachineType.h"
#include "aHotPlateSubstrate.h"              // FTestSuck / BTestSuck
#include "forms/fContact.h"                  // fContactForm->dDutCount
#include "forms/fProductionInfo.h"           // fProductionInfo
#include "canary_support.h"                  // W906_ShowErrorMessage_* / W906_ShowMyMessage_*
#include "st02_test_containment.h"

#include <windows.h>                         // Sleep (one 1.1 s wait for the pump limiter)
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// ---- the Adam6024Integrate_St02.cpp API is declared in Adam6024_St02.h (H4 integration pass 20261002) ----

namespace {

int g_pass = 0;
int g_fail = 0;

void FlowCheck(bool ok, const char* what, int line)
{
    if (ok)
        ++g_pass;
    else
    {
        ++g_fail;
        std::printf("  FAIL (line %d) %s\n", line, what);
    }
}
#define CHECK(c, what) FlowCheck((c), (what), __LINE__)

// ---- the fake ADAMTCP.dll ----
struct FakeCall
{
    std::string fn;
    std::string ip;
    int a, b, c, d;
    int data;
};
std::vector<FakeCall> g_calls;
int    g_udpOpenRet  = -3;                   // ADAMTCP_UdpSocketFailure: Open_ADAM_6024 skips the range check (golden :290)
int    g_connectRet  = 0;
int    g_writeRegRet = 0;
double g_ai[16]      = {0};
int    g_reg11       = -1;                   // the last value written to Address[0] register 11 (die force, iAdd 0)
int    g_reg12       = -1;                   // the last value written to Address[0] register 12 (EP, iAdd 1)

void RecCall(const char* fn, const char* ip, int a, int b, int c, int d, int data)
{
    FakeCall k;
    k.fn = fn;
    k.ip = ip ? ip : "";
    k.a = a; k.b = b; k.c = c; k.d = d; k.data = data;
    g_calls.push_back(k);
}
int  ADAMTCP_ST02_CALL F_Open()                    { RecCall("Open", 0, 0, 0, 0, 0, 0); return 0; }
void ADAMTCP_ST02_CALL F_Close()                   { RecCall("Close", 0, 0, 0, 0, 0, 0); }
int  ADAMTCP_ST02_CALL F_Connect(char* ip, unsigned short port, int ct, int st, int rt)
{
    RecCall("Connect", ip, port, ct, st, rt, 0);
    return g_connectRet;
}
void ADAMTCP_ST02_CALL F_Disconnect()              { RecCall("Disconnect", 0, 0, 0, 0, 0, 0); }
int  ADAMTCP_ST02_CALL F_ReadReg(char* ip, unsigned short id, unsigned short start, unsigned short n, unsigned short* w)
{
    RecCall("ReadReg", ip, id, start, n, 0, 0);
    for (int i = 0; w && i < n && i < 16; ++i) w[i] = 0;
    return 0;
}
int  ADAMTCP_ST02_CALL F_WriteReg(char* ip, unsigned short id, unsigned short start, unsigned short n, unsigned short* w)
{
    const int v = w ? (int)w[0] : -1;
    RecCall("WriteReg", ip, id, start, n, 0, v);
    if (g_writeRegRet == 0 && ip && std::string(ip) == "172.16.8.110" && id == 1 && n == 1)
    {
        if (start == 11) g_reg11 = v;
        if (start == 12) g_reg12 = v;
    }
    return g_writeRegRet;
}
int  ADAMTCP_ST02_CALL F_UDPOpen(int st, int rt)   { RecCall("UDPOpen", 0, st, rt, 0, 0, 0); return g_udpOpenRet; }
int  ADAMTCP_ST02_CALL F_UDPClose()                { RecCall("UDPClose", 0, 0, 0, 0, 0, 0); return 0; }
int  ADAMTCP_ST02_CALL F_SendRecv(char* ip, char* s, char* r)
{
    RecCall("SendRecv", ip, 0, 0, 0, 0, 0);
    if (!r) return -6;
    r[0] = 0;
    const std::string cmd = s ? s : "";
    if (cmd.compare(0, 4, "$01M") == 0) { std::strcpy(r, "!016024-D"); return 0; }   // golden GetModuleName :2779 SubString(3) -> "16024-D"
    if (cmd.compare(0, 13, "%01GETMBTCPCN") == 0) { std::strcpy(r, "!01"); return 0; } // ClearAllConnection '!' / GetModuleConnectionCount '1'
    return -6;                                                                       // ADAMTCP_ReceiveFailure
}
int  ADAMTCP_ST02_CALL F_Read6KAI(char* ip, unsigned short mod, unsigned short id, unsigned short* g, unsigned short* h, double* v)
{
    RecCall("Read6KAI", ip, mod, id, 0, 0, 0);
    (void)g;
    for (int i = 0; i < 16; ++i)
    {
        if (v) v[i] = g_ai[i];
        if (h) h[i] = 0;
    }
    return 0;
}
int  ADAMTCP_ST02_CALL F_GetIdle(char* ip, int* t) { RecCall("GetHostIdleTime", ip, 0, 0, 0, 0, 0); if (t) *t = 0; return 0; }

AdamTcpApi_St02 FakeApi()
{
    AdamTcpApi_St02 a;
    a.Open = F_Open; a.Close = F_Close; a.Connect = F_Connect; a.Disconnect = F_Disconnect;
    a.ReadReg = F_ReadReg; a.WriteReg = F_WriteReg; a.UDPOpen = F_UDPOpen; a.UDPClose = F_UDPClose;
    a.SendReceive6KUDPCmd = F_SendRecv; a.Read6KAI = F_Read6KAI; a.GetHostIdleTime = F_GetIdle;
    return a;
}

int CallCount(const char* fn)
{
    int n = 0;
    for (size_t i = 0; i < g_calls.size(); ++i)
        if (g_calls[i].fn == fn) ++n;
    return n;
}

int CountReg(int reg)
{
    int n = 0;
    for (size_t i = 0; i < g_calls.size(); ++i)
        if (g_calls[i].fn == "WriteReg" && g_calls[i].b == reg) ++n;
    return n;
}

std::string CallSeq()
{
    std::string s;
    for (size_t i = 0; i < g_calls.size(); ++i) { s += g_calls[i].fn; s += " "; }
    return s;
}

// what golden ADAM_WriteVoltage(kg) / ADAM_DirectWriteData(TransformFuntion(kg, bDual), ...) puts in the register
// (EP_Install 3: CheckRange 0..4095, the 1250..1252 -> 1253 skip, golden adam6024.cpp:1990-1993)
int RegSent(int tf)
{
    int s = tf < 0 ? 0 : (tf > 4095 ? 4095 : tf);
    if (s == 1250 || s == 1251 || s == 1252) s = 1253;
    return s;
}
int ExpEp(double kg)  { return RegSent(TransformFuntion(kg)); }
int ExpDie(double kg) { return RegSent(TransformFuntion(kg, true)); }

void FlowTicks(int n) { for (int i = 0; i < n; ++i) W906_AdamTimer2Tick(); }

// the condition claim C-4 writes into uhome.cpp:1510 (golden uhome.cpp:1923-1924 + the [W906] live switch)
bool UhomeStopsHome()
{
    return W906_AdamEpLive() && Open_ADAM_6024()==false && LastSet.iRealDummy==REALLY;
}

} // namespace

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    std::printf("Adam6024_Flow: the integrated EP flow (golden 912 main.cpp Timer2 / Start / FormShow / FormClose, uhome, csystem)\n");
#ifdef SOFT_SIMULTE
    const bool sim = true;
#else
    const bool sim = false;
#endif
    std::printf("  build: %s\n", sim ? "SIM (SOFT_SIMULTE)" : "SHIP (no SOFT_SIMULTE)");

    // ---- 0. containment first ----
    if (!W906TestInsideCtestRoots("Adam6024_Flow"))
        return 2;

    // ---- 1. the fake transport, before any ADAM call ----
    AdamTcpApi_St02 fake = FakeApi();
    AdamTcp_St02_InstallApiForTest(&fake);
    CHECK(AdamTcp_St02_IsBound(), "1. fake ADAMTCP table bound");

    // ---- fixture (declared here, never read from Gerneral.ini) ----
    const int    sEp = EP_Install, sDbl = INSTALL_DOUBLE_EP, sWc = WEIGHT_CALIBRATION, sCc = CUSTOMER_CODE;
    const int    sMap00 = IniConfig.iContactForceMap[0][0], sMap01 = IniConfig.iContactForceMap[0][1], sMin = IniConfig.iEP_Min_KG;
    const bool   sCkd = USE_CKD_FCM_CleanAir, sInd = TestIF_File.bIndEPSLK, sDyn = CosFunction.bUseDynamicKitDiameter;
    const bool   sRtc = CosFunction.bRTCAutoModelVerify, sAdd = IniConfig.bIndexAddPressEP, sEvery = IniConfig.bIndexEveryTimeCheckEP;
    const bool   sD24 = IniConfig.bD24EnableEPCheckFuntion, sD26a = IniConfig.bD26EnableEncodeShow, sD26b = IniConfig.bD26EnableEPEncoderRange;
    const bool   sD06 = IniConfig.bD06ContactOffsetDefaultValue, sInit = InitialOK, sHome = fAllMotorHome, sRun = bRunAutoClean;
    const double sKit = DeviceForm_File.dKitDiameter, sDieKit = DeviceForm_File.dDieForceKitDiameter, sDouble = DeviceForm_File.DoubleForce;
    const double sPress = DeviceForm.dPress, sAire = DeviceForm.fAireForce, sAc = TestIF.fAutoClean_AireForce, sAcDie = TestIF_File.fAutoClean_DieForce;
    const double sDut = fContactForm->dDutCount;
    const int    sReal = LastSet.iRealDummy;

    EP_Install = 3;                                  // HT9050 (machines/HT9050 Gerneral.ini EP_Install=3)
    INSTALL_DOUBLE_EP = 0;
    USE_CKD_FCM_CleanAir = false;                    // Open_ADAM_6024() opens 172.16.8.110 only (golden :404)
    TestIF_File.bIndEPSLK = false;                   // no APAX route (IsIndependentEPPressureRouteActive()==false)
    CosFunction.bUseDynamicKitDiameter = false;
    CosFunction.bRTCAutoModelVerify = false;
    IniConfig.bIndexAddPressEP = false;
    IniConfig.bIndexEveryTimeCheckEP = false;
    IniConfig.bD24EnableEPCheckFuntion = false;
    IniConfig.bD26EnableEncodeShow = false;
    IniConfig.bD26EnableEPEncoderRange = false;
    IniConfig.bD06ContactOffsetDefaultValue = false;
    IniConfig.iEP_Min_KG = 0;
    WEIGHT_CALIBRATION = 1;                          // TransformFuntion's linear map (port adam6024.cpp:170-207):
    IniConfig.iContactForceMap[0][0] = 1000;         //   16 kg -> 1000
    IniConfig.iContactForceMap[0][1] = 3400;         //   64 kg -> 3400  => 50 per kg (20 kg 1200, 8 kg 600, 0 kg 200)
    DeviceForm_File.dKitDiameter = 3.0;
    DeviceForm_File.dDieForceKitDiameter = 3.0;
    DeviceForm_File.DoubleForce = 10.0;
    DeviceForm.dPress = 20.0;                        // the production force
    DeviceForm.fAireForce = 0.0;
    TestIF.fAutoClean_AireForce = 8.0;               // the AutoClean force
    TestIF_File.fAutoClean_DieForce = 5.0;
    bRunAutoClean = false;
    bHeaterDoorIsOpen[0] = bHeaterDoorIsOpen[1] = false;
    bIndexEveryTimeCheckEPing = false;
    fContact->fShow = false;
    fiosetview->fShow = false;
    if (CUSTOMER_CODE == CC_GIGAS || CUSTOMER_CODE == CC_KYEC_LEE) CUSTOMER_CODE = 0;   // golden adam6024.cpp:2729 GIGAS skips the alarm; main.cpp:6505 KYEC branch
    InitialOK = true;
    g_ai[5] = 3.0;                                   // a healthy EP feedback (1..5 V, golden adam6024.cpp:2728)
    CHECK(ExpEp(20.0) != ExpEp(8.0), "fixture: production force and AutoClean force give different set-points");
    std::printf("  set-points: 20 kg -> %d, 8 kg -> %d, 0 kg -> %d, die 10 kg -> %d, die 5 kg -> %d\n",
                ExpEp(20.0), ExpEp(8.0), ExpEp(0.0), ExpDie(10.0), ExpDie(5.0));

    // ================= OFF: the live switch off =================
    std::printf("-- OFF: W906_AdamEpLive()==false --\n");
    W906_AdamEpLive_SetForTest(0);
    CHECK(W906_AdamEpLive() == false, "OFF. switch reads false");
    std::printf("    state: %s\n", W906_AdamEpState());
    g_calls.clear();
    W906_AdamFormShowOpen();
    W906_AdamTimer2_ResetForTest();
    FlowTicks(12);
    W906_AdamFormCloseZero();
    W906_AdamFormCloseDisconnect();
    W906_ShowErrorMessage_Reset();
    g_ai[5] = 0.5;
    W906_AdamHomeReturnValueCheck(true);
    g_ai[5] = 3.0;
    LastSet.iRealDummy = REALLY;
    CHECK(UhomeStopsHome() == false, "OFF. claim C-4: the uhome interlock does not run (Open_ADAM_6024 not even called)");
    CHECK(g_calls.empty(), "OFF. boot / Timer2 / exit / home / uhome: zero transport calls");
    CHECK(DeviceForm.fAireForce == 0.0, "OFF. the pump did not touch DeviceForm.fAireForce");
    CHECK(W906_ShowErrorMessage_Count == 0, "OFF. no EP alarm");
    g_calls.clear();
    ADAM_DirectWriteData(100, 0);
    ADAM_WriteVoltage(20.0);
    const bool offAlarm = ADAM_Alarm();
    CHECK(g_calls.empty(), "OFF. R2: ADAM_DirectWriteData / ADAM_WriteVoltage / ADAM_Alarm answer like the retired stand-ins (no I/O)");
    CHECK(offAlarm == false, "OFF. R2: ADAM_Alarm() == false (stand-in atester_shims.cpp:328)");
    if (!g_calls.empty()) std::printf("    calls: %s\n", CallSeq().c_str());

    // ================= A: boot =================
    std::printf("-- A: boot (golden main.cpp:9890 Open_ADAM_6024) --\n");
    W906_AdamEpLive_SetForTest(1);
    CHECK(W906_AdamEpLive() == true, "A. switch reads true");
    g_calls.clear();
    W906_AdamFormShowOpen();
    CHECK(fAdam6024 != NULL, "A. fAdam6024 exists after the boot open (golden HT9045.cpp:239)");
    if (sim)
    {
        CHECK(g_calls.empty(), "A. SIM: Open_ADAM_6024 returns true without I/O (golden :395-396)");
    }
    else
    {
        std::printf("    sequence: %s\n", CallSeq().c_str());
        CHECK(CallCount("Open") == 1 && CallCount("Connect") == 1, "A. SHIP: ADAMTCP_Open once, ADAMTCP_Connect once (golden :278 / :359)");
        bool okArgs = false;
        for (size_t i = 0; i < g_calls.size(); ++i)
            if (g_calls[i].fn == "Connect")
                okArgs = g_calls[i].ip == "172.16.8.110" && g_calls[i].a == 502 && g_calls[i].b == 2000 &&
                         g_calls[i].c == 2000 && g_calls[i].d == 2000;
        CHECK(okArgs, "A. SHIP: Connect(172.16.8.110, 502, 2000, 2000, 2000)");
        CHECK(bADAM6420Install == true, "A. SHIP: bADAM6420Install=true (golden :388)");
        CHECK(AdamTcp_St02_OpenRefCount() == 1, "A. SHIP: one ADAMTCP_Open outstanding (H1 S3)");
    }

    // ================= B: the Timer2 production writer =================
    std::printf("-- B: Timer2 production writer (golden main.cpp:21677-21803) --\n");
    W906_AdamTimer2_ResetForTest();
    g_calls.clear();
    FlowTicks(1);
    CHECK(DeviceForm.fAireForce == 20.0, "B1. DeviceForm.fAireForce = DeviceForm.dPress (:21696)");
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "B2. first tick writes the production set-point to register 12 (:21742 / :21778)");
    g_calls.clear();
    FlowTicks(9);
    CHECK(CountReg(12) == 0, "B3. ticks 2..10: nothing changed, ct<10 -> no write");
    FlowTicks(1);
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "B4. tick 11: the ct>=10 refresh re-writes the same set-point (:21742 / :21800)");
    // the wb_serve caller: InitialOK guard + 1 s limiter
    W906_AdamTimer2_ResetForTest();
    InitialOK = false;
    g_calls.clear();
    W906_AdamTimer2Pump();
    CHECK(CountReg(12) == 0, "B5. W906_AdamTimer2Pump: InitialOK==false -> nothing (golden Timer2Timer :21495-21496)");
    InitialOK = true;
    W906_AdamTimer2Pump();
    CHECK(CountReg(12) == 0, "B6. W906_AdamTimer2Pump: a second call within 1 s is skipped (Timer2 1000 ms)");
    ::Sleep(1100);
    W906_AdamTimer2Pump();
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "B7. W906_AdamTimer2Pump after 1.1 s runs one tick");

    // ================= C: the server contact force =================
    std::printf("-- C: server contact force (golden :21688-21692) --\n");
    {
        const AnsiString sF = fProductionInfo->sDevice_Pin_Force, sC = fProductionInfo->sDevice_Pin_Count;
        const double sOff = fProductionInfo->GetOffsetContactForce();
        fProductionInfo->sDevice_Pin_Force = "5";
        fProductionInfo->sDevice_Pin_Count = "10";
        IniConfig.bD06ContactOffsetDefaultValue = true;
        fProductionInfo->SetOffsetContactForce(0.5);
        fContactForm->dDutCount = 4.0;
        g_calls.clear();
        FlowTicks(1);
        CHECK(DeviceForm.fAireForce == 22.0, "C1. fAireForce = dPress + offset * fContactForm->dDutCount = 20 + 0.5*4 ([W906] D2)");
        CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(22.0), "C2. the changed force is written at once");
        fProductionInfo->sDevice_Pin_Force = sF;
        fProductionInfo->sDevice_Pin_Count = sC;
        fProductionInfo->SetOffsetContactForce(sOff);
        IniConfig.bD06ContactOffsetDefaultValue = false;
        FlowTicks(1);
        CHECK(DeviceForm.fAireForce == 20.0 && g_reg12 == ExpEp(20.0), "C3. no server force -> back to dPress");
    }

    // ================= D: AutoClean, then production (3c348627 B1) =================
    std::printf("-- D: AutoClean then production (the hold-back scenario) --\n");
    W906_AdamTimer2_ResetForTest();
    FlowTicks(1);                                                                   // production set-point in place, ct=0
    CHECK(g_reg12 == ExpEp(20.0), "D0. before AutoClean: register 12 holds the production set-point");
    bRunAutoClean = true;
    if (EP_Install)
        ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);                         // golden 912 AutoClean.cpp:8498 / :9324 / :10121 (port :6645 / :7448)
    CHECK(g_reg12 == ExpEp(8.0), "D1. AutoClean's own write: register 12 = the AutoClean set-point");
    std::printf("    without the pump nothing writes register 12 again: this is what 3c348627 held back\n");
    g_calls.clear();
    FlowTicks(10);
    bool allAc = CountReg(12) > 0;
    for (size_t i = 0; i < g_calls.size(); ++i)
        if (g_calls[i].fn == "WriteReg" && g_calls[i].b == 12 && g_calls[i].data != ExpEp(8.0)) allAc = false;
    CHECK(allAc, "D2. while bRunAutoClean the pump writes only the AutoClean set-point (:21753-21757)");
    g_calls.clear();
    FlowTicks(1);
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(8.0), "D3. ... every tick once it took over (fAirForce != fAireForce)");
    bRunAutoClean = false;                                                      // AutoClean finished (golden AutoClean.cpp:8314 bRunAutoClean=false)
    g_calls.clear();
    FlowTicks(1);
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "D4. first tick after AutoClean restores the production set-point (:21742 / :21761)");
    CHECK(g_reg12 == ExpEp(20.0), "D5. a production touchdown (normal contact mode: no engine write) presses with the production set-point");
    // the soft-contact touchdown writer (golden 912 aTester_Front.cpp:6889-6891, port :9182-9184): reads fAireForce
    if (DeviceForm.fAireForce == 0)
        DeviceForm.fAireForce = DeviceForm.dPress;
    ADAM_WriteVoltage(DeviceForm.fAireForce);
    CHECK(g_reg12 == ExpEp(20.0), "D6. a soft-contact touchdown writes the production set-point too (fAireForce kept fresh by the pump)");
    // golden's latency quirk: a SHORT AutoClean (ended before the pump's next ct>=10 refresh)
    W906_AdamTimer2_ResetForTest();
    FlowTicks(1);                                                                   // ct=0, fAirForce=20
    bRunAutoClean = true;
    ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);
    bRunAutoClean = false;
    int iRestore = 0;
    for (int i = 1; i <= 12 && iRestore == 0; ++i)
    {
        FlowTicks(1);
        if (g_reg12 == ExpEp(20.0)) iRestore = i;
    }
    std::printf("    short AutoClean: production set-point restored after %d tick(s)\n", iRestore);
    CHECK(iRestore == 10, "D7. golden quirk kept: a short AutoClean is restored by the ct>=10 refresh (<= 10 s), not at once");

    // ================= E: heater door =================
    std::printf("-- E: heater door (golden :21745-21749) --\n");
    W906_AdamTimer2_ResetForTest();
    FlowTicks(1);
    bHeaterDoorIsOpen[0] = true;
    g_calls.clear();
    int iZero = 0;
    for (int i = 1; i <= 12 && iZero == 0; ++i)
    {
        FlowTicks(1);
        if (g_reg12 == ExpEp(0.0)) iZero = i;
    }
    CHECK(iZero == 10, "E1. door open, no real IC on the test heads: 0 kg at the next refresh");
    g_calls.clear();
    FlowTicks(2);
    CHECK(CountReg(12) == 2 && g_reg12 == ExpEp(0.0), "E2. ... and every tick while it stays open (bOpenChambo)");
    bHeaterDoorIsOpen[0] = false;
    g_calls.clear();
    FlowTicks(1);
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "E3. door closed: the next tick writes dPress again");

    // ================= F: Contact / IO page showing =================
    std::printf("-- F: Contact / IO page showing (golden :21677-21683) --\n");
    fContact->fShow = true;
    g_calls.clear();
    FlowTicks(12);
    CHECK(CountReg(12) == 0, "F1. Contact page showing: the pump does not write");
    fContact->fShow = false;
    FlowTicks(1);
    CHECK(CountReg(12) == 1 && g_reg12 == ExpEp(20.0), "F2. page closed: the next tick writes at once (fAirForce=-1)");
    fiosetview->fShow = true;
    g_calls.clear();
    FlowTicks(12);
    CHECK(CountReg(12) == 0, "F3. IO page showing: the pump does not write");
    fiosetview->fShow = false;
    FlowTicks(1);
    CHECK(CountReg(12) == 1, "F4. IO page closed: written at once");

    // ================= G: die force =================
    std::printf("-- G: die force (golden :21716-21731) --\n");
    INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
    W906_AdamTimer2_ResetForTest();
    g_calls.clear();
    FlowTicks(1);
    CHECK(CountReg(11) == 1 && g_reg11 == ExpDie(10.0), "G1. register 11 (11+0) = TransformFuntion(DoubleForce, true)");
    g_calls.clear();
    FlowTicks(5);
    CHECK(CountReg(11) == 0, "G2. unchanged die force: no write");
    bRunAutoClean = true;
    FlowTicks(1);
    CHECK(g_reg11 == ExpDie(5.0), "G3. AutoClean: TransformFuntion(fAutoClean_DieForce, true) (:21719-21720)");
    bRunAutoClean = false;
    FlowTicks(1);
    CHECK(g_reg11 == ExpDie(10.0), "G4. AutoClean over: the production die force again");

    // ================= H: Start, Double EP =================
    std::printf("-- H: Start Double EP (golden main.cpp:6500-6521 = WebStart.cpp:3762-3782) --\n");
    g_calls.clear();
    g_reg11 = -1;
    if (INSTALL_DOUBLE_EP == 1 || INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)         // the WebStart.cpp statements, word for word
    {
        int iInputValue = 0;
        iInputValue = TransformFuntion(DeviceForm_File.DoubleForce, true);
        if (W906_FormShowing("fContact", fContact->fShow) == false)
            ADAM_DirectWriteData(iInputValue, 0, 0);
    }
    CHECK(CountReg(11) == 1 && g_reg11 == ExpDie(10.0), "H1. Start writes TransformFuntion(DoubleForce, true) to register 11");
    fContact->fShow = true;
    g_calls.clear();
    {
        int iInputValue = TransformFuntion(DeviceForm_File.DoubleForce, true);
        if (W906_FormShowing("fContact", fContact->fShow) == false)
            ADAM_DirectWriteData(iInputValue, 0, 0);
    }
    CHECK(CountReg(11) == 0, "H2. not while the Contact page shows (:6518)");
    fContact->fShow = false;
    INSTALL_DOUBLE_EP = 0;

    // ================= I: ADAM_ReturnValueCheck in the pump (3c348627 M2) =================
    std::printf("-- I: ADAM_ReturnValueCheck in the pump (golden :22354-22355) --\n");
    W906_ShowErrorMessage_Reset();
    g_calls.clear();
    FlowTicks(62);
    if (sim)
        CHECK(CallCount("Read6KAI") == 0, "I1. SIM: ADAM_ReturnValueCheck is compiled out (golden :2677)");
    else
        CHECK(CallCount("Read6KAI") >= 1 && W906_ShowErrorMessage_Count == 0, "I1. SHIP: the 60-tick keep-alive read (golden :2748-2756), no alarm at 3.0 V");
    IniConfig.bD24EnableEPCheckFuntion = true;
    g_ai[5] = 0.5;
    W906_ShowErrorMessage_Reset();
    fAllMotorHome = true;
    FlowTicks(102);
    if (sim)
        CHECK(W906_ShowErrorMessage_Count == 0, "I2. SIM: no EP-voltage alarm");
    else
        CHECK(W906_ShowErrorMessage_Count >= 1 && W906_ShowErrorMessage_LastCode == "WAR16322" && fAllMotorHome == true,
              "I2. SHIP [D24]: 0.5 V for >100 ticks -> WAR16322 (golden :2728-2744), fAllMotorHome untouched (not bHome)");
    IniConfig.bD24EnableEPCheckFuntion = false;
    g_ai[5] = 3.0;

    // ================= J: the home check =================
    std::printf("-- J: home check (golden csystem.cpp:10999) --\n");
    g_ai[5] = 0.5;
    fAllMotorHome = true;
    W906_ShowErrorMessage_Reset();
    W906_AdamHomeReturnValueCheck(true);
    if (sim)
        CHECK(W906_ShowErrorMessage_Count == 0 && fAllMotorHome == true, "J1. SIM: nothing (golden #ifndef SOFT_SIMULTE)");
    else
        CHECK(W906_ShowErrorMessage_LastCode == "WAR16322" && fAllMotorHome == false, "J1. SHIP: 0.5 V after HOME -> WAR16322, fAllMotorHome=false");
    g_ai[5] = 3.0;
    fAllMotorHome = true;
    W906_ShowErrorMessage_Reset();
    W906_AdamHomeReturnValueCheck(true);
    CHECK(W906_ShowErrorMessage_Count == 0 && fAllMotorHome == true, "J2. 3.0 V after HOME: no alarm");

    // ================= K0: the IO page helpers (not wired yet, open item O-3) =================
    std::printf("-- K0: IO page EP lines (golden iosetview.cpp:495-522 / :293-294) --\n");
    g_calls.clear();
    W906_AdamIoPageShowEp();
    CHECK(IndexHasIC() || (CallCount("WriteReg") == 1 && g_reg12 == ExpEp(0.0)), "K0a. IO page FormShow: ADAM_WriteVoltage(0) when no IC is on the index (golden :495 / :511)");
    g_calls.clear();
    W906_AdamIoPageClose();
    if (sim)
        CHECK(g_calls.empty(), "K0b. SIM: Close / Open touch nothing");
    else
        CHECK(CallCount("Disconnect") == 1 && CallCount("Close") == 1 && CallCount("Open") == 1 && CallCount("Connect") == 1 &&
              bADAM6420Install == true && AdamTcp_St02_OpenRefCount() == 1,
              "K0b. SHIP: IO page FormClose = Close_ADAM_6024 then Open_ADAM_6024 (golden :293-294), refcount balanced");
    W906_AdamTimer2_ResetForTest();

    // ================= K: exit (3c348627 M1) =================
    std::printf("-- K: exit (golden main.cpp:11947-11952, :12194) --\n");
    g_calls.clear();
    W906_AdamFormCloseZero();
    CHECK(CountReg(12) == 2 && g_reg12 == 0, "K1. ADAM_WriteVoltage(0) then ADAM_DirectWriteData(0,0): register 12 ends at 0");
    g_calls.clear();
    W906_AdamFormCloseDisconnect();
    if (sim)
        CHECK(g_calls.empty(), "K2. SIM: Close_ADAM_6024 touches nothing (golden :413)");
    else
        CHECK(CallSeq() == "Disconnect Close " && bADAM6420Install == false && AdamTcp_St02_OpenRefCount() == 0,
              "K2. SHIP: Disconnect, Close (forwarded: one Open was outstanding), bADAM6420Install=false");

    // ================= L: the Close refcount (3c348627 B2) =================
    std::printf("-- L: ADAMTCP_Close refcount (H1 [W906] S3) --\n");
    {
        const int absorbed0 = AdamTcp_St02_AbsorbedCloseCount();
        g_calls.clear();
        ADAMTCP_Close();
        CHECK(CallCount("Close") == 0 && AdamTcp_St02_AbsorbedCloseCount() == absorbed0 + 1, "L1. Close with no Open outstanding: absorbed, the DLL's WSACleanup is not reached");
        CHECK(ADAMTCP_Open() == 0 && AdamTcp_St02_OpenRefCount() == 1, "L2. Open: one outstanding");
        ADAMTCP_Close();
        CHECK(CallCount("Close") == 1 && AdamTcp_St02_OpenRefCount() == 0, "L3. the matching Close is forwarded");
        ADAMTCP_Close();
        CHECK(CallCount("Close") == 1, "L4. a second Close is absorbed");
        if (!sim)
        {
            // golden fCheckConnectStatus_ADAM6024 :2958-3016 with a new firmware and a failing Connect -> ADAMTCP_Close :2996
            const bool sFw = bADAM6024FWIsNew[0];
            bADAM6024FWIsNew[0] = true;
            g_udpOpenRet = 0;
            g_connectRet = -11;
            g_calls.clear();
            W906_ShowMyMessage_Reset();
            const bool ok = fCheckConnectStatus_ADAM6024(0);
            std::printf("    sequence: %s\n", CallSeq().c_str());
            CHECK(ok == false && CallCount("Disconnect") == 1 && CallCount("Connect") == 1, "L5. SHIP: the golden failure path ran (Disconnect, Connect -11)");
            CHECK(CallCount("Close") == 0, "L6. SHIP: its unmatched ADAMTCP_Close was absorbed (B2: wb_serve's sockets survive)");
            bADAM6024FWIsNew[0] = sFw;
            g_udpOpenRet = -3;
            g_connectRet = 0;
        }
    }

    // ================= M: DLL missing =================
    std::printf("-- M: ADAMTCP.dll missing --\n");
    AdamTcp_St02_InstallApiForTest(0);                                          // forget the fake: the next call resolves for real
    CHECK(AdamTcp_St02_IsBound() == false, "M1. not bound (H1 S1 SIM / S2 inside ctest: the real DLL is never loaded)");
    std::printf("    BindInfo: %s\n", AdamTcp_St02_BindInfo());
    CHECK(ADAMTCP_Open() == ADAMTCP_StartupFailure, "M2. ADAMTCP_Open -> -1");
    {
        const bool sFw = bADAM6024FWIsNew[0];
        bADAM6024FWIsNew[0] = false;
        LastSet.iRealDummy = REALLY;
        const bool stop = UhomeStopsHome();
        if (sim)
            CHECK(stop == false, "M3. SIM: Open_ADAM_6024() is true without I/O -> HOME goes on (golden :395-396)");
        else
            CHECK(stop == true, "M3. SHIP: Open_ADAM_6024()==false in REALLY -> claim C-4 stops HOME ('Adam Connect Error and Stop Home', golden uhome.cpp:1923-1928)");
        bADAM6024FWIsNew[0] = sFw;
    }
    ADAM_DirectWriteData(100, 0);
    W906_AdamTimer2_ResetForTest();
    FlowTicks(2);
    CHECK(true, "M4. writes and pump ticks without the DLL return (no crash)");

    // ---- restore ----
    W906_AdamEpLive_SetForTest(-1);
    AdamTcp_St02_InstallApiForTest(0);
    EP_Install = sEp;  INSTALL_DOUBLE_EP = sDbl;  WEIGHT_CALIBRATION = sWc;  CUSTOMER_CODE = sCc;
    IniConfig.iContactForceMap[0][0] = sMap00;  IniConfig.iContactForceMap[0][1] = sMap01;  IniConfig.iEP_Min_KG = sMin;
    USE_CKD_FCM_CleanAir = sCkd;  TestIF_File.bIndEPSLK = sInd;  CosFunction.bUseDynamicKitDiameter = sDyn;
    CosFunction.bRTCAutoModelVerify = sRtc;  IniConfig.bIndexAddPressEP = sAdd;  IniConfig.bIndexEveryTimeCheckEP = sEvery;
    IniConfig.bD24EnableEPCheckFuntion = sD24;  IniConfig.bD26EnableEncodeShow = sD26a;  IniConfig.bD26EnableEPEncoderRange = sD26b;
    IniConfig.bD06ContactOffsetDefaultValue = sD06;  InitialOK = sInit;  fAllMotorHome = sHome;  bRunAutoClean = sRun;
    DeviceForm_File.dKitDiameter = sKit;  DeviceForm_File.dDieForceKitDiameter = sDieKit;  DeviceForm_File.DoubleForce = sDouble;
    DeviceForm.dPress = sPress;  DeviceForm.fAireForce = sAire;  TestIF.fAutoClean_AireForce = sAc;  TestIF_File.fAutoClean_DieForce = sAcDie;
    fContactForm->dDutCount = sDut;  LastSet.iRealDummy = sReal;
    bHeaterDoorIsOpen[0] = bHeaterDoorIsOpen[1] = false;
    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();

    std::printf("Adam6024_Flow: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
