// =============================================================================
//  tests/test_adam6024_pressure.cpp -- ST02-ADAM, helper H2: the pressure / alarm half of golden 912 adam6024.cpp
//  (Adam6024Pressure_St02.cpp).  AI(W906-ST02-ADAM) 20261002 (St02-E helper H2).  Suite: Adam6024_Pressure.
//  Both configurations: SIM (SOFT_SIMULTE defined, the default) and SHIP (-DW906_NO_SOFT_SIMULTE=ON).
//
//  CONTAINMENT FIRST (st02_test_containment.h): refuses (exit 2) unless the ctest redirect roots are set.
//  MEMORY ONLY, no network, no DLL:
//    * the three callees that could leave the process are faked through the seams of Adam6024Pressure_St02.cpp
//      ([W906] D4), installed BEFORE any call: ADAM_ReadPA (scripted PA / voltage per channel),
//      fCheckConnectStatus_ADAM6024 (scripted answer), MNetLog (captured text);
//    * second line of defence for H1's comm layer: bADAM6420Install=false (golden ADAM_ReadVoltage :445-449 then
//      returns 0 before any ADAMTCP call) and Address[0..2] = 192.0.2.x (TEST-NET-1, RFC 5737);
//    * ShowErrorMessage / ShowMyMessage are canary_support.cpp's recording sims (canary_support.h:116-169);
//    * the ContactForce tables are seeded in memory (ContactForceTables(), as tests/test_transform_funtion.cpp
//      section f does) -- LoadContactForceTables() is never called, no ini is read.
//  Expected kg values are pinned literals computed outside the port (Python, IEEE double):
//      kg = kPa * 10.197 * (D*D*pi/4 * loadRate) / 1000      (golden :968-970 / :1037-1039)
//
//  SECTIONS
//    1. fakes in place.
//    1b. H4's live switch OFF ([W906] D6): ADAM_Alarm / _Kg / ADAM_DualAlarm false, ADAM_ReturnValueCheck no-op,
//        nothing read; then ON (W906_AdamEpLive_SetForTest(1)) for every section below, -1 again at the end.
//    2. IsMultiEPPressureRouteActive / IsIndependentEPPressureRouteActive (golden :96-120): every return.
//    3. AdamOutputToPA (:528-541) for EP_Install 3 / 5 / 2 / 0, truncation, EP_MINMPA; ADAM_Rang (:543-546).
//    4. KpaTransferKG (:714-972): dynamic SLK walk (row pick, A3 5.6, no match -> row 0, last match wins),
//       NS head, TSMC hot offset, Die Force table, INSTALL_DOUBLE_EP 0 message, INDIVIAL arm (Q4), the
//       static LastSet.dIndexLoadRate arm (6.0 / 5.6 / 4.0 / 3.0 / 0 / other, hot, NS), S25 ASE-KH / KYEC.
//    5. MultiTransferKG (:974-1041): equals KpaTransferKG on the plain case, [912] fractional kPa, 40.2 -> 4.0,
//       last match after the 40.2 rewrite (Q5), Die Force + message, no NS / hot arms, S25 KYEC.
//    6. ADAM_Alarm (:549-605): SIM returns false and reads nothing; SHIP: status check once, channel by
//       EP_Install / iArm, iReadAdamEP / iAdamOutValue, both bounds (strict), MNetLog text, failed status
//       check, S25 JCET, iWritePA follows (Q1 / Q2).
//    7. ADAM_Alarm_Kg (:607-668): iAdd 0 / 1 / other, every range arm (ATC and not), both bounds, the text,
//       and the empty-edSetKg throw the banner warns about.
//    8. ADAM_DualAlarm (:670-712): iType 0 (Q3: the kg text as a DA code) / 1 (Q3: kg truncated to int),
//       fixed and percentage ranges.
//    9. ADAM_ReturnValueCheck (:2675-2759): SIM does nothing; SHIP: EP_Install gate, the 60-call keep-alive,
//       each opener (three IniConfig flags, Contact page, bHome), channels per EP_Install / INSTALL_DOUBLE_EP,
//       the labels, the 101st-bad-read alarm WAR16322 / WAR16323 (cumulative, Q6), bHome: at once +
//       fAllMotorHome=false, the 0.8 / 5.2 bounds, S25 GIGAS.
//   10. everything restored.
// =============================================================================
#define _USE_MATH_DEFINES
#define ADAM6024_ST02_INTERNAL          // Adam6024_St02.h:162-185 (bConnectStatus, iWritePA, Address, iADAMRange, ...)
#include "Adam6024_St02.h"
#include "MachineType.h"                // SOFT_SIMULTE, CC_*
#include "cmydef.h"                     // EP_*, INSTALL_DOUBLE_EP, DOUBLE_EP_*, SwMultiEp, iReadAdamEP, fAllMotorHome, K_RETRY
#include "cprod.h"                      // TestIF_File, DeviceForm_File, Temperature
#include "CosFunction.h"
#include "Config.h"
#include "LastSet.h"
#include "ContactForce.h"
#include "myswitch.h"
#include "canary_support.h"             // W906_ShowErrorMessage_* / W906_ShowMyMessage_* recorders
#include "forms/fContact.h"             // fContactForm (golden fContact)
#include "common.h"
#include "st02_test_containment.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

// ---- Adam6024Pressure_St02.cpp: the three seams ([W906] D4; 0 restores golden's callee) ----
void W906_AdamPressSetReadPA(int (*fn)(double*, int));
void W906_AdamPressSetCheckConnect(bool (*fn)(int));
void W906_AdamPressSetNetLog(bool (*fn)(AnsiString));
// ---- H4's live switch (Adam6024Integrate_St02.cpp:83-84 / :103-117; [W906] D6 of Adam6024Pressure_St02.cpp) ----
bool W906_AdamEpLive();
void W906_AdamEpLive_SetForTest(int iLive);

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

void CheckNear(double got, double want, const char* what, int line)
{
    const bool ok = std::fabs(got - want) < 1e-9;
    if (!ok)
        std::printf("    got %.15f want %.15f\n", got, want);
    Check(ok, what, line);
}
#define CHECK_NEAR(got, want, what) CheckNear((got), (want), (what), __LINE__)

// ---- fake ADAM_ReadPA: PA / voltage per channel, read counts ----
const int kSlots = 8;                   // golden channels 0, 1, 2, 5; slot 7 = anything else
int    g_pa[kSlots];
double g_volt[kSlots];
int    g_reads[kSlots];
int    g_readsTotal = 0;

int Slot(int iCH)
{
    return (iCH >= 0 && iCH < kSlots - 1) ? iCH : kSlots - 1;
}
int FakeReadPA(double* dValue, int iCH)
{
    const int k = Slot(iCH);
    ++g_reads[k];
    ++g_readsTotal;
    *dValue = g_volt[k];
    return g_pa[k];
}
void ResetReads()
{
    for (int k = 0; k < kSlots; ++k)
        g_reads[k] = 0;
    g_readsTotal = 0;
}
void SetAll(int pa, double volt)
{
    for (int k = 0; k < kSlots; ++k)
    {
        g_pa[k] = pa;
        g_volt[k] = volt;
    }
}

// ---- fake fCheckConnectStatus_ADAM6024 ----
int  g_connCalls = 0;
int  g_connLastNum = -1;
bool g_connAnswer = true;
bool FakeCheckConnect(int Num)
{
    ++g_connCalls;
    g_connLastNum = Num;
    return g_connAnswer;
}

// ---- fake MNetLog ----
int g_netLogs = 0;
std::string g_lastNetLog;
bool FakeNetLog(AnsiString Message)
{
    ++g_netLogs;
    g_lastNetLog = Message.c_str();
    return true;
}

SlkForceData Row(double dDiameter, double dLoadRate, double dLoadRateNS = 1.0, double dHotOffset = 0.0)
{
    SlkForceData r;
    r.dDiameter = dDiameter;
    r.dLoadRate = dLoadRate;
    r.dLoadRate_NS = dLoadRateNS;
    r.dHotOffset = dHotOffset;
    return r;
}

// The standard SLK table of sections 4 / 5 / 7: 30 / 40 / 60 / 56 mm (golden's default Type=30,40,60,56).
void SeedSlk(SlkForceTables& t)
{
    t.SLKClass.items.clear();
    t.SLKClass.items.push_back(Row(30.0, 0.90, 0.80, 0.01));
    t.SLKClass.items.push_back(Row(40.0, 0.95, 0.85, 0.02));
    t.SLKClass.items.push_back(Row(60.0, 1.00, 0.88, 0.03));
    t.SLKClass.items.push_back(Row(56.0, 1.05, 0.89, 0.04));
    t.SLKClass.bLoaded = true;
}

#ifndef SOFT_SIMULTE
bool ShowErrorWas(const char* code, const char* errPart)
{
    return W906_ShowErrorMessage_LastCode == code &&
           W906_ShowErrorMessage_LastKCode == K_RETRY &&
           W906_ShowErrorMessage_LastErrPart == errPart;
}
#endif

} // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("==== Adam6024_Pressure -- SIM build (SOFT_SIMULTE defined) ====\n");
#else
    std::printf("==== Adam6024_Pressure -- SHIP build (SOFT_SIMULTE not defined) ====\n");
#endif
    if (!W906TestInsideCtestRoots("Adam6024_Pressure"))
        return 2;

    // ---- 1. fakes first ------------------------------------------------------------------------------------
    std::printf(" 1. fakes in place\n");
    W906_AdamPressSetReadPA(FakeReadPA);
    W906_AdamPressSetCheckConnect(FakeCheckConnect);
    W906_AdamPressSetNetLog(FakeNetLog);
    const bool savedAdamInstall = bADAM6420Install;
    bADAM6420Install = false;
    AnsiString savedAddress[3];
    for (int i = 0; i < 3; ++i)
        savedAddress[i] = Address[i];
    Address[0] = "192.0.2.110";
    Address[1] = "192.0.2.111";
    Address[2] = "192.0.2.112";
    SetAll(0, 3.0);
    ResetReads();

    // ---- saved globals (restored in section 10) ----
    const int    sEpInstall = EP_Install;
    const double sEpMaxKpa = EP_MAXKPA, sEpMinMpa = EP_MINMPA;
    const int    sDoubleEp = INSTALL_DOUBLE_EP, sCustomer = CUSTOMER_CODE;
    const int    sReadAdamEP = iReadAdamEP, sWritePA = iWritePA, sAdamRange = iADAMRange;
    const double sRangeKg = dADAMRange_Kg, sAdamOut = iAdamOutValue;
    const bool   sConn0 = bConnectStatus[0], sAllHome = fAllMotorHome;
    const bool   sIndEp = TestIF_File.bIndEPSLK, sNsPress = TestIF_File.bNSKitPress, sNs7k = TestIF_File.bNS7000kit,
                 sNs7cs = TestIF_File.bNS7000CS, sNs8cs = TestIF_File.bNS8000CS;
    const double sKit = DeviceForm_File.dKitDiameter, sDieKit = DeviceForm_File.dDieForceKitDiameter;
    const bool   sDyn = CosFunction.bUseDynamicKitDiameter, sNsSlk = CosFunction.bEPUseNSSLK,
                 sHotOfs = CosFunction.bUseLoadCellOffsetByHeater;
    const int    sTemp = LastSet.iTemperature;
    double sLoadRate[3][4];
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 4; ++b)
            sLoadRate[a][b] = LastSet.dIndexLoadRate[a][b];
    const int    sFixPct = IniConfig.iD26_3FixValueOrPercentage, sDualRange = IniConfig.iD26_3DualEPEncoderRange;
    const bool   sEncShow = IniConfig.bD26EnableEncodeShow, sEpCheck = IniConfig.bD24EnableEPCheckFuntion,
                 sEncRange = IniConfig.bD26EnableEPEncoderRange;
    const bool   sAtcCool = Temperature.bATCActiveCooling;
    const TMySwitch sSwMulti = SW[SwMultiEp];
    const bool   sShow = fContactForm->fShow;
    const AnsiString sSetKg = fContactForm->edSetKg->Text, sDouble = fContactForm->edDoubleForce->Text,
                     sLbl1 = fContactForm->lblReadEP->Caption, sLbl2 = fContactForm->lblReadEP2->Caption,
                     sLblDie = fContactForm->lblDieForceEP->Caption;
    SlkForceTables& t = ContactForceTables();
    const SlkForceTable sSlk = t.SLKClass;
    const SlkForceTable sDie = t.DieForceSLKClass;

    // ---- 1b. the live switch OFF: the four ADAM entry points read nothing ([W906] D6) ----------------------------
    std::printf(" 1b. live switch OFF (H4 R2): stand-in answers, no ADAM read\n");
    {
        W906_AdamEpLive_SetForTest(0);
        EP_Install = 3;
        bConnectStatus[0] = false;
        IniConfig.bD24EnableEPCheckFuntion = true;
        fContactForm->edSetKg->Text = "20.0";
        fContactForm->edDoubleForce->Text = "3.0";
        SetAll(9999, 0.0);
        ResetReads();
        g_connCalls = 0;
        W906_ShowErrorMessage_Reset();
        fAllMotorHome = true;
        CHECK(W906_AdamEpLive() == false, "1b-a switch reads false");
        CHECK(ADAM_Alarm() == false && ADAM_Alarm(0) == false && ADAM_Alarm_Kg(0) == false && ADAM_Alarm_Kg(1) == false &&
              ADAM_DualAlarm(0) == false && ADAM_DualAlarm(1) == false,
              "1b-b OFF: ADAM_Alarm / _Kg / ADAM_DualAlarm -> false (atester_shims.cpp:328-329)");
        ADAM_ReturnValueCheck(true);
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 0 && g_connCalls == 0 && W906_ShowErrorMessage_Count == 0 && fAllMotorHome == true,
              "1b-c OFF: no read, no status check, no alarm, homing untouched (csystem.cpp:6369 no-op)");
        IniConfig.bD24EnableEPCheckFuntion = sEpCheck;
        bConnectStatus[0] = sConn0;
        fAllMotorHome = sAllHome;
        EP_Install = sEpInstall;
        W906_AdamEpLive_SetForTest(1);
        CHECK(W906_AdamEpLive() == true, "1b-d switch ON for the golden bodies below");
    }

    // ---- 2. routes --------------------------------------------------------------------------------------------
    std::printf(" 2. IsMultiEPPressureRouteActive / IsIndependentEPPressureRouteActive (golden :96-120)\n");
    {
        TMySwitch& sw = SW[SwMultiEp];
        sw.ISABase = 99;     // neither IO branch of TMySwitch::Status (myswitch.cpp:190-200): OutValue is the answer
        sw.Type = 1;         // Status() = OutValue (Type 0: !OutValue, myswitch.cpp:202-205)
        sw.Enable = true;
        sw.OutValue = true;
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
        TestIF_File.bIndEPSLK = true;
        CHECK(!IsMultiEPPressureRouteActive(), "2a INSTALL_DOUBLE_EP 1 -> Multi route off (golden :98)");
        CHECK(!IsIndependentEPPressureRouteActive(), "2b INSTALL_DOUBLE_EP 1 -> Independent route off (golden :119)");
        INSTALL_DOUBLE_EP = DOUBLE_EP_MULTI;
        TestIF_File.bIndEPSLK = false;
        CHECK(!IsMultiEPPressureRouteActive(), "2c MULTI without bIndEPSLK -> off (golden :98)");
        CHECK(!IsIndependentEPPressureRouteActive(), "2d no bIndEPSLK -> Independent off (golden :110)");
        TestIF_File.bIndEPSLK = true;
        sw.Enable = false;
        CHECK(!IsMultiEPPressureRouteActive(), "2e SW[SwMultiEp] not enabled -> off (golden :101)");
        CHECK(!IsIndependentEPPressureRouteActive(), "2f MULTI: Independent follows the Multi route -> off (golden :116-117)");
        sw.Enable = true;
        sw.OutValue = false;
        CHECK(!IsMultiEPPressureRouteActive(), "2g SwMultiEp OFF -> off (golden :104)");
        sw.OutValue = true;
        CHECK(IsMultiEPPressureRouteActive(), "2h MULTI + bIndEPSLK + SwMultiEp ON -> on (golden :104)");
        CHECK(IsIndependentEPPressureRouteActive(), "2i MULTI with the Multi route on -> Independent on (golden :117)");
        sw.Type = 0;
        sw.OutValue = false;
        CHECK(IsMultiEPPressureRouteActive(), "2j Type 0 point: OutValue false reads ON -> on (TMySwitch::Status)");
        INSTALL_DOUBLE_EP = DOUBLE_EP_INDIVIAL;
        sw.Enable = false;
        CHECK(!IsMultiEPPressureRouteActive(), "2k INDIVIAL -> Multi route off");
        CHECK(IsIndependentEPPressureRouteActive(), "2l INDIVIAL + bIndEPSLK -> Independent on, SwMultiEp not read (golden :113-114)");
        TestIF_File.bIndEPSLK = false;
        CHECK(!IsIndependentEPPressureRouteActive(), "2m INDIVIAL without bIndEPSLK -> off (golden :110)");
        SW[SwMultiEp] = sSwMulti;
        TestIF_File.bIndEPSLK = sIndEp;
    }

    // ---- 3. AdamOutputToPA / ADAM_Rang ---------------------------------------------------------------------------
    std::printf(" 3. AdamOutputToPA (golden :528-541) / ADAM_Rang (:543-546)\n");
    {
        EP_MAXKPA = 500.0;
        EP_MINMPA = 0.0;
        EP_Install = 3;
        CHECK_NEAR(AdamOutputToPA(2048), 250.0, "3a EP_Install 3: 500 / 4096 * 2048 = 250");
        CHECK_NEAR(AdamOutputToPA(4095), 499.0, "3b 499.88 truncated by `int iPA` (Q2)");
        EP_Install = 5;
        CHECK_NEAR(AdamOutputToPA(2048), 250.0, "3c EP_Install 5: same 4095 scale (golden :531)");
        EP_Install = 2;
        CHECK_NEAR(AdamOutputToPA(511), 249.0, "3d EP_Install 2: 500 / 1023 * 511 = 249.76 -> 249 (golden :535-537)");
        CHECK_NEAR(AdamOutputToPA(1022), 499.0, "3e EP_Install 2: full scale 1022 -> 499");
        EP_Install = 0;
        CHECK_NEAR(AdamOutputToPA(2), 1000.0, "3f EP_Install 0: fMaxUnit stays 0 -> span * code (Q2)");
        EP_Install = 3;
        EP_MAXKPA = 499.0;
        CHECK_NEAR(AdamOutputToPA(2048), 249.0, "3g golden default 499 kPa: 249.5 -> 249");
        EP_MAXKPA = 500.0;
        EP_MINMPA = 0.1;
        // AI(W906-ST02-ADAM) 20261002 (St02-E): 3h used to be code 2048 -> "exactly" 200, which sits ON the int truncation
        //   boundary: 0.1 is not exact in binary, and 32-bit MinGW evaluates golden :539 on the x87 FPU with 80-bit
        //   intermediates, so (500 - 0.1*1000.0) stays a hair under 400 and the product truncates to 199 (St01 1002 proxy
        //   run: got 199).  BCB6 runs x87 at extended precision too, so golden behaves the same; the code stays verbatim
        //   (no +0.5) and the test now uses codes whose result is well away from an integer under x87 AND SSE.
        CHECK_NEAR(AdamOutputToPA(2047), 199.0, "3h EP_MINMPA 0.1: (500-100)/4096*2047 = 199.90 -> 199, the 100 kPa offset is not added back (Q2)");
        CHECK_NEAR(AdamOutputToPA(2049), 200.0, "3h2 EP_MINMPA 0.1: (500-100)/4096*2049 = 200.10 -> 200 (off the truncation boundary)");
        EP_MINMPA = 0.0;
        ADAM_Rang(15);
        CHECK(iADAMRange == 15, "3i ADAM_Rang stores iADAMRange");
    }

    // ---- 4. KpaTransferKG ------------------------------------------------------------------------------------------
    std::printf(" 4. KpaTransferKG (golden :714-972)\n");
    SeedSlk(t);
    t.DieForceSLKClass.items.clear();
    t.DieForceSLKClass.items.push_back(Row(20.0, 0.70));
    t.DieForceSLKClass.items.push_back(Row(30.0, 0.75));
    t.DieForceSLKClass.items.push_back(Row(56.0, 0.77));
    t.DieForceSLKClass.bLoaded = true;
    {
        const double kRates[3][4] = { { 1.01, 1.02, 1.03, 1.04 },      // [0] HT   60 / 56 / 40 / 30
                                      { 0.91, 0.92, 0.93, 0.94 },      // [1] NS
                                      { 0.001, 0.002, 0.003, 0.004 } };// [2] hot offset
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 4; ++b)
                LastSet.dIndexLoadRate[a][b] = kRates[a][b];
    }
    CosFunction.bUseDynamicKitDiameter = true;
    CosFunction.bEPUseNSSLK = false;
    CosFunction.bUseLoadCellOffsetByHeater = false;
    LastSet.iTemperature = 0;                         // not Tempture_Hot (cmydef.cpp:2981 = 1)
    CUSTOMER_CODE = 0;
    INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
    TestIF_File.bNSKitPress = false;
    TestIF_File.bNS7000kit = false;
    TestIF_File.bNS7000CS = false;
    TestIF_File.bNS8000CS = false;
    DeviceForm_File.dKitDiameter = 4.0;
    DeviceForm_File.dDieForceKitDiameter = 3.0;
    {
        CHECK_NEAR(KpaTransferKG(300), 36.519695129066839, "4a dynamic, kit 4.0 -> SLK row 40 (0.95)");
        DeviceForm_File.dKitDiameter = 5.6;
        CHECK_NEAR(KpaTransferKG(300), 79.113192184862683, "4b kit 5.6 -> row 56 (1.05): A3 tolerance, 56/10.0 vs 5.6 ([W906] D2)");
        DeviceForm_File.dKitDiameter = 3.0;
        CHECK_NEAR(KpaTransferKG(300), 19.461153325357987, "4c kit 3.0 -> row 30 (0.90)");
        DeviceForm_File.dKitDiameter = 7.0;
        CHECK_NEAR(KpaTransferKG(300), 105.955168104726809, "4d kit 7.0, no row -> iTag 0 (row 30's 0.90) with D 7.0 (golden :862-863)");
        DeviceForm_File.dKitDiameter = 4.0;
        t.SLKClass.items.push_back(Row(40.0, 1.10));
        CHECK_NEAR(KpaTransferKG(300), 42.285962781024764, "4e two 40 rows -> the LAST one (no break in golden's walk, Q4)");
        t.SLKClass.items.pop_back();

        CosFunction.bEPUseNSSLK = true;
        CHECK_NEAR(KpaTransferKG(300), 36.519695129066839, "4f bEPUseNSSLK without an NS kit flag -> dLoadRate (golden :943-947)");
        TestIF_File.bNSKitPress = true;
        CHECK_NEAR(KpaTransferKG(300), 32.675516694428218, "4g bEPUseNSSLK + bNSKitPress -> dLoadRate_NS (0.85, golden :949)");
        TestIF_File.bNSKitPress = false;
        TestIF_File.bNS7000kit = true;
        CUSTOMER_CODE = CC_ASE_KaohSiung;
        CHECK_NEAR(KpaTransferKG(300), 32.675516694428218,
                   "4h [W906] S25: ASE-Kaohsiung arm gated -> NS switch on without bNSKitPress (golden would use 0.95)");
        CUSTOMER_CODE = 0;
        TestIF_File.bNS7000kit = false;
        CosFunction.bEPUseNSSLK = false;

        CosFunction.bUseLoadCellOffsetByHeater = true;
        CHECK_NEAR(KpaTransferKG(300), 36.519695129066839, "4i hot-offset flag but not Tempture_Hot -> no offset");
        LastSet.iTemperature = Tempture_Hot;
        CHECK_NEAR(KpaTransferKG(300), 37.288530815994562, "4j TSMC hot: dLoadRate + dHotOffset (0.95 + 0.02, golden :960-964)");
        LastSet.iTemperature = 0;
        CosFunction.bUseLoadCellOffsetByHeater = false;

        CHECK_NEAR(KpaTransferKG(300, true), 16.217627771131653, "4k bDualForce -> Die Force table, D = dDieForceKitDiameter 3.0 (0.75)");
        DeviceForm_File.dDieForceKitDiameter = 5.6;
        CHECK_NEAR(KpaTransferKG(300, true), 58.016340935565964, "4l Die Force kit 5.6 -> row 56 (0.77): A3 tolerance");
        DeviceForm_File.dDieForceKitDiameter = 3.0;
        W906_ShowMyMessage_Reset();
        INSTALL_DOUBLE_EP = DOUBLE_EP_NONE;
        CHECK_NEAR(KpaTransferKG(300, true), 16.217627771131653, "4m INSTALL_DOUBLE_EP 0: message, then the Die Force table anyway (golden :819-836)");
        CHECK(W906_ShowMyMessage_Count == 1 && W906_ShowMyMessage_LastS1 == "無安裝dual force, 請確認硬體選項",
              "4n ... and golden's message text (:821)");
        INSTALL_DOUBLE_EP = DOUBLE_EP_INDIVIAL;
        CHECK_NEAR(KpaTransferKG(300, true), 39.595037876777731,
                   "4o bDualForce + INDIVIAL: static arm on dKitDiameter 4.0, LastSet [0][2] 1.03 (golden :771-791, Q4)");
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;

        CosFunction.bUseDynamicKitDiameter = false;
        DeviceForm_File.dKitDiameter = 6.0;
        CHECK_NEAR(KpaTransferKG(300), 87.358954927162515, "4p static arm: 6.0 -> LastSet [0][0] 1.01 (golden :796-799)");
        DeviceForm_File.dKitDiameter = 5.6;
        CHECK_NEAR(KpaTransferKG(300), 76.852815265295163, "4q static arm: 5.6 -> [0][1] 1.02");
        DeviceForm_File.dKitDiameter = 4.0;
        CHECK_NEAR(KpaTransferKG(300), 39.595037876777731, "4r static arm: 4.0 -> [0][2] 1.03");
        DeviceForm_File.dKitDiameter = 3.0;
        CHECK_NEAR(KpaTransferKG(300), 22.488443842635895, "4s static arm: 3.0 -> [0][3] 1.04");
        DeviceForm_File.dKitDiameter = 0.0;
        CHECK_NEAR(KpaTransferKG(300), 22.488443842635895, "4t static arm: 0 -> D 3.0 and [0][3] (golden :810-812)");
        DeviceForm_File.dKitDiameter = 7.0;
        CHECK_NEAR(KpaTransferKG(300), 122.437083143239860, "4u static arm: other -> [0][3] with D 7.0");
        DeviceForm_File.dKitDiameter = 4.0;
        CosFunction.bUseLoadCellOffsetByHeater = true;
        LastSet.iTemperature = Tempture_Hot;
        CHECK_NEAR(KpaTransferKG(300), 39.710363229816885, "4v static arm hot: [0][2] + [2][2] = 1.033 (golden :742-749)");
        LastSet.iTemperature = 0;
        CosFunction.bUseLoadCellOffsetByHeater = false;
        CosFunction.bEPUseNSSLK = true;
        TestIF_File.bNSKitPress = true;
        CHECK_NEAR(KpaTransferKG(300), 35.750859442139117, "4w static arm NS: [1][2] 0.93 (golden :750-760)");
        TestIF_File.bNSKitPress = false;
        CosFunction.bEPUseNSSLK = false;
        CosFunction.bUseDynamicKitDiameter = true;

        t.SLKClass.items.clear();
        t.SLKClass.items.push_back(Row(40.0, 0.95));
        t.SLKClass.items.push_back(Row(28.0, 0.99));
        CUSTOMER_CODE = CC_KYEC_LEE;
        DeviceForm_File.dKitDiameter = 3.0;
        CHECK_NEAR(KpaTransferKG(300), 20.542328510100095,
                   "4x [W906] S25: KYEC 28 -> 3.0 rewrite gated -> no match, row 0 (golden would pick the 28 row, 0.99)");
        CUSTOMER_CODE = 0;
        DeviceForm_File.dKitDiameter = 4.0;
        SeedSlk(t);
    }

    // ---- 5. MultiTransferKG --------------------------------------------------------------------------------------
    std::printf(" 5. MultiTransferKG (golden :974-1041, [912] double argument)\n");
    {
        CHECK_NEAR(MultiTransferKG(300.0), KpaTransferKG(300), "5a plain case equals KpaTransferKG");
        CHECK_NEAR(MultiTransferKG(300.0), 36.519695129066839, "5b kit 4.0 -> row 40 (0.95)");
        CHECK_NEAR(MultiTransferKG(300.5), 36.580561287615282, "5c [912] fractional kPa is kept (906 took an int)");
        CosFunction.bUseLoadCellOffsetByHeater = true;
        LastSet.iTemperature = Tempture_Hot;
        CosFunction.bEPUseNSSLK = true;
        TestIF_File.bNSKitPress = true;
        CHECK_NEAR(MultiTransferKG(300.0), 36.519695129066839, "5d no NS / hot arms (golden's simplified walk, :974-976)");
        CosFunction.bUseLoadCellOffsetByHeater = false;
        LastSet.iTemperature = 0;
        CosFunction.bEPUseNSSLK = false;
        TestIF_File.bNSKitPress = false;

        t.SLKClass.items.clear();
        t.SLKClass.items.push_back(Row(30.0, 0.90));
        t.SLKClass.items.push_back(Row(402.0, 0.91));
        DeviceForm_File.dKitDiameter = 40.2;
        CHECK_NEAR(MultiTransferKG(300.0), 34.982023755211394, "5e kit 40.2 -> the 402 row, then D 4.0 (golden :1026-1027)");
        t.SLKClass.items.push_back(Row(40.0, 0.96));
        CHECK_NEAR(MultiTransferKG(300.0), 36.904112972530697,
                   "5f after the rewrite a later 40 row matches 4.0 and wins (Q5)");
        DeviceForm_File.dKitDiameter = 4.0;

        t.SLKClass.items.clear();
        t.SLKClass.items.push_back(Row(40.0, 0.95));
        t.SLKClass.items.push_back(Row(28.0, 0.99));
        CUSTOMER_CODE = CC_KYEC_LEE;
        DeviceForm_File.dKitDiameter = 3.0;
        CHECK_NEAR(MultiTransferKG(300.0), 20.542328510100095, "5g [W906] S25: KYEC 28 -> 3.0 rewrite gated -> row 0");
        CUSTOMER_CODE = 0;
        DeviceForm_File.dKitDiameter = 4.0;
        SeedSlk(t);

        CHECK_NEAR(MultiTransferKG(300.0, true), 16.217627771131653, "5h bDualForce -> Die Force row 30 (0.75)");
        W906_ShowMyMessage_Reset();
        INSTALL_DOUBLE_EP = DOUBLE_EP_NONE;
        CHECK_NEAR(MultiTransferKG(300.0, true), 16.217627771131653, "5i DOUBLE_EP_NONE: message, then the table anyway");
        CHECK(W906_ShowMyMessage_Count == 1 &&
              W906_ShowMyMessage_LastS1 == "No dual force installed, please check hardware option",
              "5j ... and golden's message text (:987)");
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
    }

    // ---- 6. ADAM_Alarm -------------------------------------------------------------------------------------------
    std::printf(" 6. ADAM_Alarm (golden :549-605)\n");
    {
        EP_Install = 3;
        EP_MAXKPA = 500.0;
        EP_MINMPA = 0.0;
        iWritePA = 2048;
        ADAM_Rang(10);
        CUSTOMER_CODE = 0;
        IniConfig.iD26_3FixValueOrPercentage = 0;
        bConnectStatus[0] = false;
        g_connAnswer = true;
        g_connCalls = 0;
        g_netLogs = 0;
        ResetReads();
#ifdef SOFT_SIMULTE
        iReadAdamEP = -7;
        SetAll(9999, 0.0);
        CHECK(ADAM_Alarm() == false && ADAM_Alarm(0) == false && ADAM_Alarm(1) == false,
              "6a SIM: golden :551-552 returns false");
        CHECK(g_readsTotal == 0 && g_connCalls == 0 && iReadAdamEP == -7 && g_netLogs == 0,
              "6b SIM: nothing read, no status check, iReadAdamEP untouched");
#else
        SetAll(255, 3.0);
        CHECK(!ADAM_Alarm(), "6a PA 255 vs 250 +-10 -> no alarm");
        CHECK(g_connCalls == 1 && g_connLastNum == 0 && bConnectStatus[0] == true,
              "6b not connected -> fCheckConnectStatus_ADAM6024(0) once, stored (golden :556-557)");
        CHECK(g_reads[5] == 1 && g_readsTotal == 1, "6c EP_Install 3 reads channel 5 (adam6024.h:8 default, golden :564)");
        CHECK(iReadAdamEP == 255, "6d iReadAdamEP = PA (golden :566)");
        CHECK_NEAR(iAdamOutValue, 250.0, "6e iAdamOutValue = AdamOutputToPA(iWritePA 2048) (golden :567, Q1)");
        ADAM_Alarm();
        CHECK(g_connCalls == 1, "6f connected -> no second status check");
        SetAll(260, 3.0);
        CHECK(!ADAM_Alarm(), "6g PA 260 = out + range -> no alarm (strict >, golden :594)");
        CHECK(g_netLogs == 0, "6h no alarm -> no MNetLog");
        SetAll(261, 3.0);
        CHECK(ADAM_Alarm(), "6i PA 261 -> alarm");
        CHECK(g_netLogs == 1 && g_lastNetLog == "AdamOutValue=250.000000, ReadAdamValue=261, Range=10",
              "6j golden's EP alarm log text (:597)");
        SetAll(240, 3.0);
        CHECK(!ADAM_Alarm(), "6k PA 240 = out - range -> no alarm (strict <, golden :595)");
        SetAll(239, 3.0);
        CHECK(ADAM_Alarm(), "6l PA 239 -> alarm");

        EP_Install = 5;
        SetAll(250, 3.0);
        ResetReads();
        ADAM_Alarm(0);
        ADAM_Alarm(1);
        ADAM_Alarm();
        CHECK(g_reads[0] == 1 && g_reads[1] == 1 && g_reads[2] == 1 && g_readsTotal == 3,
              "6m EP_Install 5 reads channel iArm; ADAM_Alarm() is iArm 2 (adam6024.h:11, golden :561-562)");
        EP_Install = 3;
        ResetReads();
        ADAM_Alarm(1);
        CHECK(g_reads[5] == 1 && g_readsTotal == 1, "6n EP_Install 3 ignores iArm (golden :563-564)");

        bConnectStatus[0] = false;
        g_connAnswer = false;
        g_connCalls = 0;
        ResetReads();
        ADAM_Alarm();
        ADAM_Alarm();
        CHECK(g_connCalls == 2 && g_readsTotal == 2 && bConnectStatus[0] == false,
              "6o status check fails: golden reads anyway and asks again next call");
        g_connAnswer = true;
        bConnectStatus[0] = true;

        CUSTOMER_CODE = CC_JCET;
        IniConfig.iD26_3FixValueOrPercentage = 1;
        SetAll(270, 3.0);
        CHECK(ADAM_Alarm(), "6p [W906] S25: JCET percentage arm gated -> fixed +-10 (golden would allow 250*1.10 = 275)");
        CUSTOMER_CODE = 0;
        IniConfig.iD26_3FixValueOrPercentage = 0;

        iWritePA = 4095;
        SetAll(499, 3.0);
        CHECK(!ADAM_Alarm(), "6q iWritePA 4095 -> 499 kPa expected (Q1), PA 499 -> no alarm");
        CHECK_NEAR(iAdamOutValue, 499.0, "6r ... 499.88 truncated (Q2)");
        iWritePA = 2048;
#endif
    }

    // ---- 7. ADAM_Alarm_Kg ------------------------------------------------------------------------------------------
    std::printf(" 7. ADAM_Alarm_Kg (golden :607-668; golden has no caller)\n");
    {
        EP_Install = 3;
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
        DeviceForm_File.dKitDiameter = 4.0;
        DeviceForm_File.dDieForceKitDiameter = 3.0;
        Temperature.bATCActiveCooling = false;
        fContactForm->edSetKg->Text = "20.0";
        iReadAdamEP = -1;
        g_netLogs = 0;
        ResetReads();
        SetAll(164, 3.0);
        CHECK(!ADAM_Alarm_Kg(0), "7a 164 kPa = 19.964 kg vs set 20 +-1.0 -> no alarm");
        CHECK(g_reads[5] == 1 && g_readsTotal == 1 && iReadAdamEP == 164,
              "7b iAdd 0: channel 5, iReadAdamEP = PA (golden :617 / :620)");
        CHECK_NEAR(dADAMRange_Kg, 1.0, "7c set 11..60 kg -> range 1.0 (golden :646-648)");
        SetAll(157, 3.0);
        CHECK(!ADAM_Alarm_Kg(0), "7d 19.112 kg -> no alarm");
        SetAll(156, 3.0);
        CHECK(ADAM_Alarm_Kg(0), "7e 18.990 kg < 19 -> alarm");
        CHECK(g_netLogs == 1 && g_lastNetLog == "AdamOutValue=20.000000, ReadAdamValue=18.990241, Range=1.000000",
              "7f golden's log text (:663)");
        SetAll(172, 3.0);
        CHECK(!ADAM_Alarm_Kg(0), "7g 20.938 kg -> no alarm");
        SetAll(173, 3.0);
        CHECK(ADAM_Alarm_Kg(0), "7h 21.060 kg > 21 -> alarm");

        struct RangeCase { const char* set; bool atc; double range; const char* what; };
        const RangeCase kCases[] = {
            { "70.0", false, 2.0,  "7i set > 60 -> 2.0 (golden :642-645)" },
            { "60.0", false, 1.0,  "7j set 60 (not > 60) -> 1.0" },
            { "10.0", false, 0.5,  "7k set 10 (not > 10) -> 0.5 (golden :650-653)" },
            { "8.0",  false, 0.5,  "7l set 6..10 -> 0.5" },
            { "5.0",  false, 0.25, "7m set 5 (not > 5) -> 0.25 (golden :654-657)" },
            { "3.0",  false, 0.25, "7n set 1..5 -> 0.25" },
            { "70.0", true,  2.0,  "7o ATC kit, set > 60 -> 2.0 (golden :629-634)" },
            { "20.0", true,  1.0,  "7p ATC kit, set 20 -> 1.0 (golden :635-638)" },
            { "3.0",  true,  1.0,  "7q ATC kit, set 3 -> 1.0 too" },
        };
        for (size_t k = 0; k < sizeof(kCases) / sizeof(kCases[0]); ++k)
        {
            Temperature.bATCActiveCooling = kCases[k].atc;
            fContactForm->edSetKg->Text = kCases[k].set;
            dADAMRange_Kg = -1.0;
            ADAM_Alarm_Kg(0);
            CHECK_NEAR(dADAMRange_Kg, kCases[k].range, kCases[k].what);
        }
        Temperature.bATCActiveCooling = false;

        fContactForm->edDoubleForce->Text = "3.0";
        iReadAdamEP = -1;
        ResetReads();
        SetAll(56, 3.0);
        CHECK(!ADAM_Alarm_Kg(1), "7r iAdd 1: 56 kPa on the Die Force row 30 = 3.027 kg vs 3.0 +-0.25 -> no alarm");
        CHECK(g_reads[2] == 1 && g_readsTotal == 1 && iReadAdamEP == -1,
              "7s iAdd 1: channel 2, iReadAdamEP untouched (golden :624)");
        SetAll(80, 3.0);
        CHECK(ADAM_Alarm_Kg(1), "7t iAdd 1: 4.325 kg -> alarm");
        ResetReads();
        CHECK(!ADAM_Alarm_Kg(2), "7u iAdd 2: no read, 0 kg vs set 0 -> false (golden)");
        CHECK(g_readsTotal == 0, "7v iAdd 2: nothing read");

        fContactForm->edSetKg->Text = "";
        bool threw = false;
        try
        {
            ADAM_Alarm_Kg(0);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }
        CHECK(threw, "7w empty edSetKg -> AnsiString::ToDouble throws (Adam6024Pressure_St02.cpp banner: fill it before wiring a caller)");
        fContactForm->edSetKg->Text = "20.0";
    }

    // ---- 8. ADAM_DualAlarm -------------------------------------------------------------------------------------------
    std::printf(" 8. ADAM_DualAlarm (golden :670-712)\n");
    {
        EP_Install = 3;
        EP_MAXKPA = 500.0;
        EP_MINMPA = 0.0;
        DeviceForm_File.dDieForceKitDiameter = 3.0;
        fContactForm->edDoubleForce->Text = "3.0000";
        IniConfig.iD26_3FixValueOrPercentage = 0;
        IniConfig.iD26_3DualEPEncoderRange = 5;
        g_netLogs = 0;
        ResetReads();
        SetAll(5, 3.0);
        CHECK(!ADAM_DualAlarm(0), "8a iType 0: PA 5 vs AdamOutputToPA(3) = 0 +-5 -> no alarm (Q3: kg text as a DA code)");
        CHECK(g_reads[2] == 1 && g_readsTotal == 1, "8b reads channel 2 (golden :677)");
        SetAll(6, 3.0);
        CHECK(ADAM_DualAlarm(0), "8c iType 0: PA 6 -> alarm");
        CHECK(g_netLogs == 1, "8d alarm -> MNetLog (text not compared: golden passes the int PA to %f, Q3)");
        SetAll(-5, 3.0);
        CHECK(!ADAM_DualAlarm(0), "8e iType 0: PA -5 -> no alarm");
        SetAll(-6, 3.0);
        CHECK(ADAM_DualAlarm(0), "8f iType 0: PA -6 -> alarm");

        IniConfig.iD26_3DualEPEncoderRange = 0;
        SetAll(56, 3.0);
        CHECK(!ADAM_DualAlarm(1), "8g iType 1: 3.027 kg truncated to 3 vs 3.0 +-0 -> no alarm (Q3)");
        SetAll(55, 3.0);
        CHECK(ADAM_DualAlarm(1), "8h iType 1: 2.973 kg truncated to 2 -> alarm (Q3)");
        SetAll(74, 3.0);
        CHECK(ADAM_DualAlarm(1), "8i iType 1: 4.0003 kg truncated to 4 -> alarm");

        IniConfig.iD26_3FixValueOrPercentage = 1;
        IniConfig.iD26_3DualEPEncoderRange = 10;
        SetAll(56, 3.0);
        CHECK(!ADAM_DualAlarm(1), "8j percentage 10 %: 3 in [2.7, 3.3] -> no alarm (golden :691-699)");
        SetAll(74, 3.0);
        CHECK(ADAM_DualAlarm(1), "8k percentage: 4 -> alarm");
        SetAll(0, 3.0);
        CHECK(!ADAM_DualAlarm(0), "8l percentage, iType 0: target 0 -> PA 0 no alarm");
        SetAll(1, 3.0);
        CHECK(ADAM_DualAlarm(0), "8m percentage, iType 0: PA 1 -> alarm");
        IniConfig.iD26_3FixValueOrPercentage = 0;
    }

    // ---- 9. ADAM_ReturnValueCheck -------------------------------------------------------------------------------------
    std::printf(" 9. ADAM_ReturnValueCheck (golden :2675-2759)\n");
    {
        IniConfig.bD26EnableEncodeShow = false;
        IniConfig.bD24EnableEPCheckFuntion = false;
        IniConfig.bD26EnableEPEncoderRange = false;
        fContactForm->fShow = false;
        CUSTOMER_CODE = 0;
        W906_ShowErrorMessage_Reset();
        ResetReads();
#ifdef SOFT_SIMULTE
        EP_Install = 3;
        IniConfig.bD24EnableEPCheckFuntion = true;
        SetAll(0, 0.0);
        fAllMotorHome = true;
        ADAM_ReturnValueCheck(true);
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 0 && W906_ShowErrorMessage_Count == 0 && fAllMotorHome == true,
              "9a SIM: golden :2677 #ifndef SOFT_SIMULTE -> nothing read, no alarm, homing untouched");
        IniConfig.bD24EnableEPCheckFuntion = false;
#else
        EP_Install = 0;
        SetAll(250, 0.0);
        ADAM_ReturnValueCheck(true);
        EP_Install = 1;
        ADAM_ReturnValueCheck(true);
        CHECK(g_readsTotal == 0 && W906_ShowErrorMessage_Count == 0, "9a EP_Install 0 / 1 -> nothing (golden :2683)");

        EP_Install = 3;
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
        SetAll(250, 3.0);
        ResetReads();
        for (int n = 0; n < 60; ++n)
            ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 0, "9b no opener: 60 calls read nothing");
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 1 && g_reads[5] == 1, "9c 61st call: one keep-alive read on channel 5 (golden :2750-2755)");
        for (int n = 0; n < 60; ++n)
            ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 1, "9d next 60 calls: nothing");
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 2, "9e and the next 61st: one more");

        IniConfig.bD24EnableEPCheckFuntion = true;
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_reads[5] == 1 && g_reads[2] == 1 && g_readsTotal == 2 && W906_ShowErrorMessage_Count == 0,
              "9f [D24] opener, INSTALL_DOUBLE_EP 1: channel 5 + Die Force channel 2, 3.0 V -> no alarm");
        INSTALL_DOUBLE_EP = DOUBLE_EP_NONE;
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_reads[5] == 1 && g_readsTotal == 1, "9g INSTALL_DOUBLE_EP != 1 -> channel 2 skipped (golden :2719-2720)");
        IniConfig.bD24EnableEPCheckFuntion = false;
        IniConfig.bD26EnableEncodeShow = true;
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 1, "9h [D26] encoder-show opener reads");
        IniConfig.bD26EnableEncodeShow = false;
        IniConfig.bD26EnableEPEncoderRange = true;
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 1, "9i [D26] encoder-range opener reads");
        IniConfig.bD26EnableEPEncoderRange = false;

        fContactForm->lblReadEP->Caption = "x";
        fContactForm->lblReadEP2->Caption = "x";
        fContactForm->lblDieForceEP->Caption = "x";
        fContactForm->fShow = true;
        INSTALL_DOUBLE_EP = DOUBLE_EP_NORMAL;
        g_pa[5] = 333;
        g_pa[2] = 444;
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_readsTotal == 2, "9j Contact page open is an opener ([W906] D3 W906_FormShowing)");
        CHECK(fContactForm->lblReadEP->Caption == "Read=333.00" && fContactForm->lblDieForceEP->Caption == "Read=444.00" &&
              fContactForm->lblReadEP2->Caption == "x",
              "9k EP_Install 3 labels: lblReadEP (ch 5) and lblDieForceEP (ch 2), GetFloatFormatString(PA, 3, 2)");
        EP_Install = 5;
        g_pa[0] = 111;
        g_pa[1] = 222;
        fContactForm->lblReadEP->Caption = "x";
        fContactForm->lblDieForceEP->Caption = "x";
        ResetReads();
        ADAM_ReturnValueCheck();
        CHECK(g_reads[0] == 1 && g_reads[1] == 1 && g_readsTotal == 2, "9l EP_Install 5 reads channels 0 and 1 (golden :2694-2707)");
        CHECK(fContactForm->lblReadEP->Caption == "Read=111.00" && fContactForm->lblReadEP2->Caption == "Read=222.00" &&
              fContactForm->lblDieForceEP->Caption == "x",
              "9m EP_Install 5 labels: lblReadEP and lblReadEP2");
        fContactForm->fShow = false;
        IniConfig.bD24EnableEPCheckFuntion = true;
        fContactForm->lblReadEP->Caption = "x";
        ADAM_ReturnValueCheck();
        CHECK(fContactForm->lblReadEP->Caption == "x", "9n page closed -> labels not written");
        EP_Install = 3;
        SetAll(250, 3.0);

        // the 101st bad read raises (counter is cumulative, Q6)
        g_volt[5] = 0.5;
        W906_ShowErrorMessage_Reset();
        for (int n = 0; n < 100; ++n)
            ADAM_ReturnValueCheck();
        CHECK(W906_ShowErrorMessage_Count == 0, "9o 100 bad reads on channel 5 -> no alarm yet");
        ADAM_ReturnValueCheck();
        CHECK(W906_ShowErrorMessage_Count == 1 && ShowErrorWas("WAR16322", "dReadVoltage=0.500000"),
              "9p 101st -> WAR16322, K_RETRY, errPart dReadVoltage (golden :2732-2736)");
        for (int n = 0; n < 50; ++n)
            ADAM_ReturnValueCheck();
        g_volt[5] = 3.0;
        for (int n = 0; n < 10; ++n)
            ADAM_ReturnValueCheck();
        g_volt[5] = 0.5;
        for (int n = 0; n < 50; ++n)
            ADAM_ReturnValueCheck();
        CHECK(W906_ShowErrorMessage_Count == 1, "9q counter was reset after the alarm (golden :2743): 100 bad with 10 good between -> none");
        ADAM_ReturnValueCheck();
        CHECK(W906_ShowErrorMessage_Count == 2, "9r good reads do not clear it: the 101st bad read in total raises (Q6)");
        g_volt[5] = 3.0;
        g_volt[2] = 5.5;
        W906_ShowErrorMessage_Reset();
        for (int n = 0; n < 101; ++n)
            ADAM_ReturnValueCheck();
        CHECK(W906_ShowErrorMessage_Count == 1 && ShowErrorWas("WAR16323", "dReadVoltage=5.500000"),
              "9s Die Force channel 2 at 5.5 V -> WAR16323 on the 101st (golden :2738)");
        g_volt[2] = 3.0;
        IniConfig.bD24EnableEPCheckFuntion = false;

        // bHome: alone it opens the check, raises at once and fails the homing
        INSTALL_DOUBLE_EP = DOUBLE_EP_NONE;
        const double kOk[] = { 0.8, 5.2 };
        for (size_t k = 0; k < sizeof(kOk) / sizeof(kOk[0]); ++k)
        {
            g_volt[5] = kOk[k];
            fAllMotorHome = true;
            W906_ShowErrorMessage_Reset();
            ADAM_ReturnValueCheck(true);
            CHECK(W906_ShowErrorMessage_Count == 0 && fAllMotorHome == true, "9t bHome, 0.8 V / 5.2 V are inside (strict, golden :2728)");
        }
        const double kBad[] = { 0.79, 5.21 };
        for (size_t k = 0; k < sizeof(kBad) / sizeof(kBad[0]); ++k)
        {
            g_volt[5] = kBad[k];
            fAllMotorHome = true;
            W906_ShowErrorMessage_Reset();
            ADAM_ReturnValueCheck(true);
            CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "WAR16322" && fAllMotorHome == false,
                  "9u bHome, 0.79 V / 5.21 V -> WAR16322 at once and fAllMotorHome=false (golden :2732 / :2740-2741)");
        }
        EP_Install = 5;
        SetAll(250, 3.0);
        g_volt[1] = 0.0;
        fAllMotorHome = true;
        W906_ShowErrorMessage_Reset();
        ResetReads();
        ADAM_ReturnValueCheck(true);
        CHECK(g_reads[0] == 1 && g_reads[1] == 1 && W906_ShowErrorMessage_Count == 1 &&
              W906_ShowErrorMessage_LastCode == "WAR16323" && fAllMotorHome == false,
              "9v bHome, EP_Install 5, channel 1 at 0 V -> WAR16323, homing fails");
        EP_Install = 3;
        SetAll(250, 0.0);
        CUSTOMER_CODE = CC_GIGAS;
        fAllMotorHome = true;
        W906_ShowErrorMessage_Reset();
        ADAM_ReturnValueCheck(true);
        CHECK(W906_ShowErrorMessage_Count == 1 && fAllMotorHome == false,
              "9w [W906] S25: the GIGAS exemption is gated -> a GIGAS machine is checked too (golden :2729 skips it)");
        CUSTOMER_CODE = 0;
#endif
    }

    // ---- 10. restore --------------------------------------------------------------------------------------------------
    t.SLKClass = sSlk;
    t.DieForceSLKClass = sDie;
    fContactForm->fShow = sShow;
    fContactForm->edSetKg->Text = sSetKg;
    fContactForm->edDoubleForce->Text = sDouble;
    fContactForm->lblReadEP->Caption = sLbl1;
    fContactForm->lblReadEP2->Caption = sLbl2;
    fContactForm->lblDieForceEP->Caption = sLblDie;
    SW[SwMultiEp] = sSwMulti;
    Temperature.bATCActiveCooling = sAtcCool;
    IniConfig.iD26_3FixValueOrPercentage = sFixPct;
    IniConfig.iD26_3DualEPEncoderRange = sDualRange;
    IniConfig.bD26EnableEncodeShow = sEncShow;
    IniConfig.bD24EnableEPCheckFuntion = sEpCheck;
    IniConfig.bD26EnableEPEncoderRange = sEncRange;
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 4; ++b)
            LastSet.dIndexLoadRate[a][b] = sLoadRate[a][b];
    LastSet.iTemperature = sTemp;
    CosFunction.bUseDynamicKitDiameter = sDyn;
    CosFunction.bEPUseNSSLK = sNsSlk;
    CosFunction.bUseLoadCellOffsetByHeater = sHotOfs;
    DeviceForm_File.dKitDiameter = sKit;
    DeviceForm_File.dDieForceKitDiameter = sDieKit;
    TestIF_File.bIndEPSLK = sIndEp;
    TestIF_File.bNSKitPress = sNsPress;
    TestIF_File.bNS7000kit = sNs7k;
    TestIF_File.bNS7000CS = sNs7cs;
    TestIF_File.bNS8000CS = sNs8cs;
    fAllMotorHome = sAllHome;
    bConnectStatus[0] = sConn0;
    dADAMRange_Kg = sRangeKg;
    iAdamOutValue = sAdamOut;
    iADAMRange = sAdamRange;
    iWritePA = sWritePA;
    iReadAdamEP = sReadAdamEP;
    CUSTOMER_CODE = sCustomer;
    INSTALL_DOUBLE_EP = sDoubleEp;
    EP_MAXKPA = sEpMaxKpa;
    EP_MINMPA = sEpMinMpa;
    EP_Install = sEpInstall;
    for (int i = 0; i < 3; ++i)
        Address[i] = savedAddress[i];
    bADAM6420Install = savedAdamInstall;
    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();
    W906_AdamPressSetReadPA(0);
    W906_AdamPressSetCheckConnect(0);
    W906_AdamPressSetNetLog(0);
    W906_AdamEpLive_SetForTest(-1);

    std::printf("\n==== Adam6024_Pressure summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
