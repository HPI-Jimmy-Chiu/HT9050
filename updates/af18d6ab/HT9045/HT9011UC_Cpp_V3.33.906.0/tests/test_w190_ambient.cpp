// =============================================================================
//  test_w190_ambient.cpp -- AI(W906-W190-AMB) 20261009 (Ifor01)
//
//  W-190 ③ (TO_IFOR 1009 14:3x; Steven 1009 13:5x "913 新增功能照 913 補進 cpp"): ConvertGetTempOffset (bthermo.cpp) ambient
//  block per golden 913 bthermo.cpp:428-457 (RogerYang 20260911, 偉測常溫校正).  In ambient mode:
//    * SPIL Ambient guardband (bAmbientGuardbandCheck && bL20AbientGuardBand && bSPILFunction): offsets applied, base =
//      the working temperature (as before);
//    * VTEST (IniConfig.bVTESTFunction, new in 913): offsets applied and the base becomes Temperature.fAbitTemp -- except a
//      fixed-temperature DUT channel (bUseFixTemp, tcDUT1..4) keeps dFixedTemp;
//    * every other customer: the controller's temperature is returned unchanged (as before).
//  Observed through the five-point mode (iTempMode 8): a working temperature that equals one of the bases returns
//  T - that base's offset, so the result names the base that was used.  Memory only; no files.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "forms/fTemp_Set.h"     // LowBase / MidBase / HigBase / UserOffSet / AmbientHotLow / AmbientHotMid
#include <cstdio>
#include <cmath>

double ConvertGetTempOffset(int Addr, double T);   // bthermo.cpp

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static bool Near(double a, double b) { return std::fabs(a - b) < 1e-9; }

static void Setup()
{
    LastSet.iTemperature = Tempture_Ambient;
    CosFunction.bUseIndividulTempSet = false; Temperature.bUseIndividualTemp = false;
    CosFunction.bTemp5PointKitOffset = false;
    Temperature.iTempMode = 8;                         // five-point base
    Temperature.fAmbientHotLowBase = -40.0; Temperature.fAmbientHotMiddBase = 0.0;
    Temperature.fLowBase = 25.0; Temperature.fMiddBase = 85.0; Temperature.fHighBase = 125.0;
    Temperature.fWorkTemperBase = 85.0;                // working temperature = the middle base
    Temperature.fAbitTemp = 25.0;                      // ambient = the low base
    Temperature.bUseFixTemp = false; Temperature.dFixedTemp = 125.0;
    Temperature.bAmbientGuardbandCheck = false; IniConfig.bL20AbientGuardBand = false; IniConfig.bSPILFunction = false;
    IniConfig.bVTESTFunction = 0;
    CUSTOMER_CODE = 0;
    for (int a = 0; a < tcTotalCount; ++a) {
        Temperature.fTempOffSet[UserOffSet][a] = 0.0;
        Temperature.fTempOffSet[LowBase][a] = 1.0; Temperature.fTempOffSet[MidBase][a] = 3.0; Temperature.fTempOffSet[HigBase][a] = 5.0;
        Temperature.fTempOffSet[AmbientHotLow][a] = 7.0; Temperature.fTempOffSet[AmbientHotMid][a] = 9.0;
    }
}

int main()
{
    std::printf("W190_AmbientOffset\n");
    const double T = 50.0;
    char m[160];

    Setup();
    double r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "1. ambient, other customer: controller temperature unchanged (%.3f)", r); Check(Near(r, 50.0), m);

    Setup(); IniConfig.bVTESTFunction = 1;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "2. ambient, VTEST: base = fAbitTemp 25 = low base -> 50 - 1 (%.3f)", r); Check(Near(r, 49.0), m);

    Setup(); Temperature.bAmbientGuardbandCheck = true; IniConfig.bL20AbientGuardBand = true; IniConfig.bSPILFunction = true;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "3. ambient, SPIL guardband: base = working temperature 85 = middle base -> 50 - 3 (%.3f)", r); Check(Near(r, 47.0), m);

    Setup(); Temperature.bAmbientGuardbandCheck = true; IniConfig.bL20AbientGuardBand = true; IniConfig.bSPILFunction = true; IniConfig.bVTESTFunction = 1;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "4. ambient, SPIL and VTEST both: SPIL branch first (working temperature) -> 47 (%.3f)", r); Check(Near(r, 47.0), m);

    Setup(); IniConfig.bVTESTFunction = 1; Temperature.bUseFixTemp = true;
    r = ConvertGetTempOffset(tcDUT1, T);
    std::snprintf(m, sizeof(m), "5. ambient, VTEST, fixed-temperature DUT1: keeps dFixedTemp 125 = high base -> 50 - 5 (%.3f)", r); Check(Near(r, 45.0), m);

    Setup(); IniConfig.bVTESTFunction = 1; Temperature.bUseFixTemp = true;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "6. ambient, VTEST, fixed DUT temperature on but channel Aa1 (not a DUT): base = fAbitTemp -> 49 (%.3f)", r); Check(Near(r, 49.0), m);

    Setup(); IniConfig.bVTESTFunction = 1; Temperature.fTempOffSet[UserOffSet][tcAa1] = 0.5;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "7. ambient, VTEST: the user offset is applied too -> 50 - 0.5 - 1 (%.3f)", r); Check(Near(r, 48.5), m);

    Setup(); LastSet.iTemperature = Tempture_Hot; IniConfig.bVTESTFunction = 1;
    r = ConvertGetTempOffset(tcAa1, T);
    std::snprintf(m, sizeof(m), "8. hot mode, VTEST: unchanged (working temperature 85 -> 47) (%.3f)", r); Check(Near(r, 47.0), m);

    std::printf("W190_AmbientOffset: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
