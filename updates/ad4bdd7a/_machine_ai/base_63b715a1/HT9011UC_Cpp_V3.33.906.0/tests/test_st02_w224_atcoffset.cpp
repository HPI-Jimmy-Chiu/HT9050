// =============================================================================
//  test_st02_w224_atcoffset.cpp -- W-224 POOL-13 MR-A (St02-E): golden 913 TfLotInfo::SetATCOffset + ConvertPackageOffset
//  (LotInfo_ATC913.cpp).  Suite name (add_test): St02_W224AtcOffset.  argv[1] = port root (source pins, read only).
//
//  AI(W906-POOL13) 20261010 (St02-E).  In memory only: nothing is read or written on disk except the source pins (read).
//  Containment first: refuses to run when the exe or the cwd is under D:\HT9045* (that covers D:\HT9045_Log) unless the path
//  contains \obj\v906\ (as tests/test_st02_pool2_tempoffset.cpp:46-51).
//  Every ATC send golden makes is read back from the recorder (W906_ATC913_Rec, LotInfo_ATC913.h) -- the sends stay gated.
//    [A1] L07 off: SetOffset = dATCTempOffset through iSiteToATC / iSiteToOfs (golden 913 uLotInfo.cpp:10114-10126)
//    [A2] L07 on, first remote change (count 1): the down arm gets the FIRST group dATCPreOffset, the other arm dATCTempOffset (:9813-9818)
//    [A3] L07 on, second change (count 2, back to production temp): the down arm gets the SECOND group d2ndATCPreOffset (:9820-9825)
//    [A4] bOFSClose + timer expired + site enabled -> the after-offset (d2ndATCAfterOfs / dATCAfterOfs); a channel whose timer never ran stays Pre
//    [A5] a disabled site (bATC_EnableSiteMap false) stays Pre
//    [A6] L07 wins over Boost (:10065-10071 before :10083): with L07 no +dBoostOffset on any site; without L07 every site gets it
//    [A7] the 912 path (bATCPreOffset false, bChangeTest_TempOffset != 0): dATCSecondTempOffset / 0 on close (:9830-9861)
//    [A8] ATC 7.0 (shim iATC_MODE_TYPE 70): four heads = dATCInPC (:9713-9772)
//    [A9] Set2ndFunction recorded on every non-7.0 call, also when bFirstSetOffset is false (:10185-10194)
//    [P1] ConvertPackageOffset (:9643-9694): exact points and the interpolation
//    [P2] three-point adjustment: first call waits (static bFirstChange, :9880-9885), second call sends dATCTempAdjustmentOffset + package
//    [P3] L07 skips that wait (:9870-9875) and still adds the package offset
//    [G1] GOLDEN BUG guard 1: USE_16_HEATER==eht4Heater + 1x4 kit -> iSiteToATC -1 for columns 2-3 (measured); those sites skipped, 0-1 as golden
//    [G2] GOLDEN BUG guard 2: 2-row kit, TC2 on -> arm 2 row 2 would read dATCTempOffset[32..39]; not read (stays 0), the rest as golden
//    [S1] LotInfo_ATC913.cpp: every ATC_InterfaceForm->Set* line sits under `#if 0 // GATE(W906-POOL13-ATCSEND)` (8 lines)
//    [S2] no live caller yet (MR-C is separate): each fLotInfo->SetATCOffset( line in the five caller files is under `#if 0`
//    [S3] forms/fLotInfo.h declares SetATCOffset(bool, bool=false)
// =============================================================================
#include "forms/fLotInfo.h"
#include "forms/fTemp_Set.h"
#include "LotInfo_ATC913.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "acarry_shims.h"
#include "aHotPlateSubstrate.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern bool bUnderTest;             // atester_shims.cpp:101

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg)
{
    if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else    { std::printf("  FAIL: %s\n", msg); ++g_fail; }
}
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

// ---- state --------------------------------------------------------------------------------------------------------------------
static void ResetState()
{
    W906_ATC913_ResetRecorder();
    ATC_SYSTEM = eNewATCSystem;
    ATC_InterfaceForm->iATC_MODE_TYPE = 0;
    Tri_Temp_Machine = 0;
    CosFunction.bUseSecondATCTempOffset = false;
    CosFunction.bATCUseTempAdjustment = false;
    CosFunction.bATCUsePackageOffset = false;
    CosFunction.bHiSiliconFunction = false;
    CosFunction.bATC32UseTJMode = false;
    Temperature.bATCPreOffset = false;
    Temperature.bEnableTempOffsetForInitial = false;
    Temperature.bBoostFuncttion = false;
    Temperature.bLBTempFunction = false;
    Temperature.bMultiZoneEnable = false;
    Temperature.bUseReferTempSensor = false;
    Temperature.bUseTC2Offset = false;
    Temperature.bATC_MultiSensorEnable = false;
    Temperature.dATCPackageOffsettemp = 0.0;
    bChangeTest_TempOffset = 0;
    iATCRemoteChangeTempCnt = 0;
    bNeedInitialTestDelay = false;
    bGPIBOffsetCommand = false;
    bATCTempAdjustmentOffset = false;
    iTriggerBoostFunction = -1;
    iTriggerBoostFuncBack = -1;
    iBoostFuncStep = 0;
    bUnderTest = false;
    IndexStatus = Z1Down_Z2Up;
    TestIF.iTestMode = SingleSite;
    TestIF_File.iTestMode = SingleSite;
    TestIF_File.bUse32Heater = false;
    TestIF_File.bOctal_16Kit = false;
    TestIF_File.bOctal_12Kit = false;
    for (int k = 0; k < 32; k++)
    {
        Temperature.dATCTempOffset[k]       = 100 + k;
        Temperature.dATCPreOffset[k]        = 200 + k;
        Temperature.dATCAfterOfs[k]         = 300 + k;
        Temperature.d2ndATCPreOffset[k]     = 400 + k;
        Temperature.d2ndATCAfterOfs[k]      = 500 + k;
        Temperature.dATCSecondTempOffset[k] = 600 + k;
        Temperature.dIndexATCSecondTempOffset[k] = 0.0;
        dATCTempAdjustmentOffset[k]         = 700 + k;
        bATC_EnableSiteMap[k] = true;
        ATCPreOFSDelay[k].Clear();
    }
}
// 2 rows x 8 columns, 32 ATC heads, not the 4-heater layout: every site has a channel (uTemp_Set.cpp InitialAddrToATC, count 32).
static void Config2x8()
{
    USE_16_HEATER = eht16Heater;
    iATC_Use_Heat_Count = 32;
    FTestSuck.iShtRow = 2;
    FTestSuck.iShtCol = 8;
    fTemp_Set->InitialAddrToATC();
}
static void Call(bool bFirst, bool bClose)
{
    W906_ATC913_ResetRecorder();
    fLotInfo->SetATCOffset(bFirst, bClose);
}
// every site of the current kit: SetOffset[j1] == exp(j3) and SetOffset[j2] == exp(j4)
template <class F> static bool SitesAre(F exp)
{
    if (W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET] != 1) { std::printf("    SetOffset count %d\n", W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET]); return false; }
    bool ok = true;
    for (int i = 0; i < FTestSuck.iShtRow; i++)
        for (int j = 0; j < FTestSuck.iShtCol; j++)
        {
            const int j1 = fTemp_Set->iSiteToATC[0][i][j], j2 = fTemp_Set->iSiteToATC[1][i][j];
            const int j3 = fTemp_Set->iSiteToOfs[0][i][j], j4 = fTemp_Set->iSiteToOfs[1][i][j];
            const double* d = W906_ATC913_Rec.dLast[W906_ATC913_SETOFFSET];
            if (d[j1] != exp(j3) || d[j2] != exp(j4))
            {
                if (ok) std::printf("    site r%d c%d: [%d]=%g want %g, [%d]=%g want %g\n", i, j, j1, d[j1], exp(j3), j2, d[j2], exp(j4));
                ok = false;
            }
        }
    return ok;
}

// ---- source pins ---------------------------------------------------------------------------------------------------------------
static std::vector<std::string> ReadLines(const std::string& p)
{
    std::vector<std::string> v;
    std::ifstream f(p.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) { if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1); v.push_back(s); }
    return v;
}
static std::string LTrim(const std::string& s) { size_t i = s.find_first_not_of(" \t"); return i == std::string::npos ? "" : s.substr(i); }

int main(int argc, char** argv)
{
    std::printf("St02_W224AtcOffset\n");
    char exe[MAX_PATH] = {0}, cwd[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    GetCurrentDirectoryA(MAX_PATH, cwd);
    if (UnderMachineTree(exe) || UnderMachineTree(cwd))
    {
        std::printf("REFUSE: exe '%s' / cwd '%s' is under D:\\HT9045* (allowed only under \\obj\\v906\\)\n", exe, cwd);
        return 2;
    }

    if (fLotInfo == 0) fLotInfo = new TfLotInfo();
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();

    // [A1]
    ResetState(); Config2x8();
    Call(true, false);
    Check(W906_ATC913_Rec.iChCount[W906_ATC913_SETOFFSET] == 32 && SitesAre([](int k) { return 100.0 + k; }),
          "[A1] L07 off: SetOffset(32, dATCTempOffset via iSiteToATC/iSiteToOfs) (golden 913 :10114-10126)");

    // [A2]
    ResetState(); Config2x8();
    CosFunction.bUseSecondATCTempOffset = true; Temperature.bATCPreOffset = true;
    bChangeTest_TempOffset = 5; iATCRemoteChangeTempCnt = 1;
    Call(true, false);
    Check(SitesAre([](int k) { return k < 16 ? 200.0 + k : 100.0 + k; }),
          "[A2] count 1: arm 1 (down) = FIRST group dATCPreOffset, arm 2 = dATCTempOffset (:9813-9818)");

    // [A3]
    bChangeTest_TempOffset = 0; iATCRemoteChangeTempCnt = 2;
    Call(true, false);
    Check(SitesAre([](int k) { return k < 16 ? 400.0 + k : 100.0 + k; }),
          "[A3] count 2 (back to production temp, bChangeTest_TempOffset 0): arm 1 = SECOND group d2ndATCPreOffset (:9820-9825)");

    // [A4]
    for (int k = 0; k < 8; k++) ATCPreOFSDelay[k].SetSecAndOn(0);   // expired (HTimer Off(): iTimeLen <= 0 -> true)
    Call(true, true);
    Check(SitesAre([](int k) { return k < 8 ? 500.0 + k : (k < 16 ? 400.0 + k : 100.0 + k); }),
          "[A4] bOFSClose + expired timer: d2ndATCAfterOfs; channels 8-15 (timer never ran) stay d2ndATCPreOffset");
    iATCRemoteChangeTempCnt = 1; bChangeTest_TempOffset = 5;
    Call(true, true);
    Check(SitesAre([](int k) { return k < 8 ? 300.0 + k : (k < 16 ? 200.0 + k : 100.0 + k); }),
          "[A4] count 1: the after-offset is the FIRST group dATCAfterOfs");

    // [A5]
    iATCRemoteChangeTempCnt = 2; bChangeTest_TempOffset = 0; bATC_EnableSiteMap[3] = false;
    Call(true, true);
    Check(SitesAre([](int k) { return (k < 8 && k != 3) ? 500.0 + k : (k < 16 ? 400.0 + k : 100.0 + k); }),
          "[A5] site 3 disabled (bATC_EnableSiteMap false): stays d2ndATCPreOffset");

    // [A6]
    ResetState(); Config2x8();
    Temperature.bBoostFuncttion = true; Temperature.iBoostFunctionMode = 2;
    iTriggerBoostFunction = 0; iBoostFuncStep = 0; bUnderTest = false; Temperature.dBoostOffset[0] = 7.0;
    Call(true, false);
    Check(SitesAre([](int k) { return 100.0 + k + 7.0; }), "[A6] L07 off: Boost adds dBoostOffset[0]=7 on every site (:10083-10093)");
    CosFunction.bUseSecondATCTempOffset = true; Temperature.bATCPreOffset = true; iATCRemoteChangeTempCnt = 2;
    Call(true, false);
    Check(SitesAre([](int k) { return k < 16 ? 400.0 + k : 100.0 + k; }),
          "[A6] L07 on: the L07 branch (:10065-10071) wins -- no Boost offset on any site (human-review H1)");
    Temperature.dBoostOffset[0] = 0.0;

    // [A7]
    ResetState(); Config2x8();
    CosFunction.bUseSecondATCTempOffset = true; bChangeTest_TempOffset = 5;
    Call(true, false);
    Check(SitesAre([](int k) { return k < 16 ? 600.0 + k : 100.0 + k; }), "[A7] 912 path: arm 1 = dATCSecondTempOffset (:9830-9843)");
    Call(true, true);
    Check(SitesAre([](int k) { return k < 16 ? 0.0 : 100.0 + k; }), "[A7] 912 path, bOFSClose: arm 1 = 0");

    // [A8]
    ResetState(); Config2x8();
    ATC_InterfaceForm->iATC_MODE_TYPE = 70;
    LastSet.iTemperature = Tempture_Hot; LastSet.iRunStartMode = 0;
    Temperature.bEnableATCTestTimeOffset = false; Temperature.bEnableATCConFailOffset = false;
    for (int h = 0; h < 4; h++) Temperature.dATCInPC[h] = 1.5 + h;
    Call(false, false);
    {
        const double* d = W906_ATC913_Rec.dLast[W906_ATC913_SETOFFSET];
        Check(W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET] == 1 && W906_ATC913_Rec.iChCount[W906_ATC913_SETOFFSET] == 32 &&
              d[0] == 1.5 && d[1] == 2.5 && d[2] == 3.5 && d[3] == 4.5 && d[4] == 0.0 &&
              W906_ATC913_Rec.iCount[W906_ATC913_SET2NDFUNCTION] == 0,
              "[A8] ATC 7.0: SetOffset(iATC_Use_Heat_Count, dATCInPC[0..3]) even with bFirstSetOffset false; no Set2ndFunction (:9713-9772)");
    }
    ATC_InterfaceForm->iATC_MODE_TYPE = 0;

    // [A9]
    ResetState(); Config2x8();
    Temperature.bUseReferTempSensor = true;
    Call(false, false);
    Check(W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET] == 0 && W906_ATC913_Rec.iCount[W906_ATC913_SET2NDFUNCTION] == 1 && W906_ATC913_Rec.bLast2nd,
          "[A9] bFirstSetOffset false: no SetOffset, Set2ndFunction(bUseReferTempSensor=true) still recorded (:10185-10194)");

    // [P1]
    ResetState();
    Temperature.dATCPackageTemp[0] = 50; Temperature.dATCPackageTemp[1] = 100; Temperature.dATCPackageTemp[2] = 150;
    Temperature.dATCPackageOffset[0] = 1; Temperature.dATCPackageOffset[1] = 2; Temperature.dATCPackageOffset[2] = 3;
    LastSet.iTemperature = Tempture_Hot;
    Temperature.fWorkTemperBase = 100; const double p100 = ConvertPackageOffset();
    Temperature.fWorkTemperBase = 125; const double p125 = ConvertPackageOffset();
    Temperature.fWorkTemperBase = 75;  const double p75  = ConvertPackageOffset();
    Temperature.fWorkTemperBase = 110; const double p110 = ConvertPackageOffset();   // off the midpoint: m = 0.8, s = 112.2
    LastSet.iTemperature = Tempture_Ambient; IniConfig.dATCAmbientTemperature = 50; const double pAmb = ConvertPackageOffset();
    std::printf("    ConvertPackageOffset: 100 -> %g, 125 -> %g, 75 -> %g, 110 -> %.12g, ambient 50 -> %g\n", p100, p125, p75, p110, pAmb);
    Check(p100 == 2.0 && p125 == 2.5 && p75 == 1.5 && p110 > 2.2 - 1e-9 && p110 < 2.2 + 1e-9 && pAmb == 1.0,
          "[P1] ConvertPackageOffset: exact 100 -> 2, 125 -> 2.5, 110 -> 2.2 (between 100/150), 75 -> 1.5 (between 50/100), ambient 50 -> 1 (:9643-9694)");

    // [P2]
    ResetState(); Config2x8();
    LastSet.iTemperature = Tempture_Hot; Temperature.fWorkTemperBase = 125;
    CosFunction.bATCUseTempAdjustment = true; CosFunction.bATCUsePackageOffset = true;
    bATCTempAdjustmentOffset = true;
    Call(true, false);
    Check(W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET] == 0 && W906_ATC913_Rec.iCount[W906_ATC913_SET2NDFUNCTION] == 0 && bATCTempAdjustmentOffset == false,
          "[P2] three-point adjustment, first call: waits (bFirstChange), clears bATCTempAdjustmentOffset, returns before any send (:9880-9885)");
    bATCTempAdjustmentOffset = true;
    Call(true, false);
    Check(Temperature.dATCPackageOffsettemp == 2.5 && SitesAre([](int k) { return 700.0 + k + 2.5; }),
          "[P2] second call: dATCPackageOffsettemp = ConvertPackageOffset() = 2.5; SetOffset = dATCTempAdjustmentOffset + 2.5 (:9886-9891, :10078-10082, :10128-10132)");

    // [P3]
    CosFunction.bUseSecondATCTempOffset = true; Temperature.bATCPreOffset = true; iATCRemoteChangeTempCnt = 2;
    bATCTempAdjustmentOffset = false;
    Call(true, false);
    Check(SitesAre([](int k) { return (k < 16 ? 400.0 + k : 100.0 + k) + 2.5; }),
          "[P3] L07 skips the three-point wait (:9870-9875): sent at once, L07 values + package offset 2.5");

    // [G1]
    ResetState();
    USE_16_HEATER = eht4Heater; iATC_Use_Heat_Count = 4; FTestSuck.iShtRow = 1; FTestSuck.iShtCol = 4;
    fTemp_Set->InitialAddrToATC();
    std::printf("    measured iSiteToATC (eht4Heater, 1x4): arm1 %d %d %d %d / arm2 %d %d %d %d\n",
                fTemp_Set->iSiteToATC[0][0][0], fTemp_Set->iSiteToATC[0][0][1], fTemp_Set->iSiteToATC[0][0][2], fTemp_Set->iSiteToATC[0][0][3],
                fTemp_Set->iSiteToATC[1][0][0], fTemp_Set->iSiteToATC[1][0][1], fTemp_Set->iSiteToATC[1][0][2], fTemp_Set->iSiteToATC[1][0][3]);
    Check(fTemp_Set->iSiteToATC[0][0][2] == -1 && fTemp_Set->iSiteToATC[0][0][3] == -1 && fTemp_Set->iSiteToATC[1][0][2] == -1,
          "[G1] measured: InitialAddrToATC leaves iSiteToATC -1 for columns 2-3 (golden would index dbATC_Offset[-1])");
    Call(true, false);
    {
        const double* d = W906_ATC913_Rec.dLast[W906_ATC913_SETOFFSET];
        const int a0 = fTemp_Set->iSiteToATC[0][0][0], a1 = fTemp_Set->iSiteToATC[0][0][1];
        const int b0 = fTemp_Set->iSiteToATC[1][0][0], b1 = fTemp_Set->iSiteToATC[1][0][1];
        Check(W906_ATC913_Rec.iCount[W906_ATC913_SETOFFSET] == 1 && W906_ATC913_Rec.iGuardSite == 2 &&
              d[a0] == 100.0 + fTemp_Set->iSiteToOfs[0][0][0] && d[a1] == 100.0 + fTemp_Set->iSiteToOfs[0][0][1] &&
              d[b0] == 100.0 + fTemp_Set->iSiteToOfs[1][0][0] && d[b1] == 100.0 + fTemp_Set->iSiteToOfs[1][0][1],
              "[G1] guard 1: the two sites without a channel are skipped (iGuardSite 2); columns 0-1 computed as golden");
    }

    // [G2]
    ResetState(); Config2x8();
    Temperature.bUseReferTempSensor = true; Temperature.bUseTC2Offset = true; ATC_InterfaceForm->iATC_MODE_TYPE = 33;
    Call(true, false);
    {
        const double* t = W906_ATC913_Rec.dLast[W906_ATC913_SETTC2OFFSET];
        bool ok = W906_ATC913_Rec.iCount[W906_ATC913_SETTC2OFFSET] == 1 && W906_ATC913_Rec.iGuardTc2 == 8;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 8; j++)
            {
                const int j1 = fTemp_Set->iSiteToATC[0][i][j], j2 = fTemp_Set->iSiteToATC[1][i][j];
                const int j3 = fTemp_Set->iSiteToOfs[0][i][j], j4 = fTemp_Set->iSiteToOfs[1][i][j];
                const double want1 = 100.0 + j3 + 8;
                const double want2 = (j4 + 8 < 32) ? 100.0 + j4 + 8 : 0.0;
                if (t[j1] != want1 || t[j2] != want2) { if (ok) std::printf("    TC2 r%d c%d: [%d]=%g want %g, [%d]=%g want %g\n", i, j, j1, t[j1], want1, j2, t[j2], want2); ok = false; }
            }
        Check(ok, "[G2] guard 2: arm 2 row 2 TC2 (index j4+8 = 32..39) not read -> 0, iGuardTc2 8; every other TC2 offset = dATCTempOffset[j+8] as golden");
    }
    ATC_InterfaceForm->iATC_MODE_TYPE = 0;

    // [S1]-[S3] source pins
    if (argc > 1)
    {
        const std::string root = std::string(argv[1]) + "/";
        std::vector<std::string> L = ReadLines(root + "LotInfo_ATC913.cpp");
        int nSend = 0, nGated = 0;
        for (size_t i = 0; i < L.size(); i++)
            if (L[i].find("ATC_InterfaceForm->Set") != std::string::npos && LTrim(L[i]).compare(0, 2, "//") != 0 && LTrim(L[i]).compare(0, 1, "#") != 0)
            {
                ++nSend;
                if (i > 0 && L[i - 1].compare(0, 37, "#if 0 // GATE(W906-POOL13-ATCSEND) go") == 0) ++nGated;
            }
        std::printf("    LotInfo_ATC913.cpp: %d ATC_InterfaceForm->Set* lines, %d under GATE(W906-POOL13-ATCSEND)\n", nSend, nGated);
        Check(L.size() > 100 && nSend == 8 && nGated == 8, "[S1] the 8 ATC sends are all under #if 0 // GATE(W906-POOL13-ATCSEND)");

        const char* callers[] = { "Command.cpp", "aTester_Front.cpp", "aTester_Rear.cpp", "atester.cpp", "uTemp_Set.cpp" };
        int nCall = 0, nCallGated = 0;
        for (size_t c = 0; c < sizeof(callers) / sizeof(callers[0]); c++)
        {
            std::vector<std::string> C = ReadLines(root + callers[c]);
            for (size_t i = 0; i < C.size(); i++)
                if (C[i].find("fLotInfo->SetATCOffset(") != std::string::npos && LTrim(C[i]).compare(0, 2, "//") != 0 && LTrim(C[i]).compare(0, 1, "#") != 0)
                {
                    ++nCall;
                    if (i > 0 && C[i - 1].compare(0, 5, "#if 0") == 0) ++nCallGated;
                }
        }
        std::printf("    callers: %d fLotInfo->SetATCOffset( lines, %d directly under #if 0\n", nCall, nCallGated);
        Check(nCall == 5 && nCallGated == 5, "[S2] no live caller (MR-C separate): the 5 golden caller lines stay under #if 0");

        std::vector<std::string> H = ReadLines(root + "forms/fLotInfo.h");
        bool decl = false;
        for (size_t i = 0; i < H.size(); i++)
            if (H[i].find("void SetATCOffset(bool bFirstSetOffset, bool bOFSClose=false);") != std::string::npos) decl = true;
        Check(decl, "[S3] forms/fLotInfo.h declares SetATCOffset(bool bFirstSetOffset, bool bOFSClose=false) (golden 913 uLotInfo.h:1331)");
    }
    else
        Check(false, "[S1]-[S3] need argv[1] = port root");

    std::printf("St02_W224AtcOffset: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
