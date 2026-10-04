// =============================================================================
//  test_temperfrom_core.cpp -- FW-3 cTemperFrom Wave A test coverage
//
//  AI(W906-FW3-TemperFrom-WA) 20260818: new file.
//
//  Covers (per this wave's task brief):
//   (a) ShowThermo -- 3 direct-write oracle branches, all through the same
//       "seed bUT150Install[]/UN150Read[]/Temperature.* globals then call"
//       idiom as tests/test_yieldmon_core.cpp:
//         1. NOT installed             -> Caption "---", Color 0x0025AB12
//         2. installed, inside band    -> green (0x0025AB12), no alarm class
//         3. installed, OVER the band  -> 0x008000FF, iTempOverShowAlarmT==1
//       (a 4th, UNDER-the-band case backs test (b)'s "false" half below.)
//   (b) TempRunShowAlarmHigh/Low -- one true case each (derived from (a)'s
//       OVER/UNDER ShowThermo calls) and one explicit false case (freshly
//       reset channel globals, no alarm classification pending).
//   (c) Every asserted temperature (100.0/150.0/50.0) is an exact-in-both-
//       float-and-double whole degree, and every SetTemp/range constant
//       (fWorkTemperBase=100.0, iL04TemptureRange=5) is chosen so the
//       in-band/over-band/under-band comparisons land 5+ degrees clear of
//       their cutoff, not on it -- the boundary-value/float-truncation
//       lesson this campaign already paid for once (uYieldMonitoring's own
//       iUPH cutoff flake, see docs/DEVLOG.md).
//
//  CHANNEL CHOICE -- every ShowThermo case below uses tcHotPlate1 (Addr==0).
//  It is NOT one of bGetHeaterUsed's special-cased tcHead1..tcHead4 channels
//  (bthermo.cpp:944-1108: every branch keyed off a specific Addr==tcHeadN;
//  tcHotPlate1 matches none of them, so bGetHeaterUsed(tcHotPlate1) reduces
//  to TestIF.iTestMode's own branch's unconditional `return true;`, made
//  deterministic below by pinning iTestMode==SingleSite and
//  IniConfig.bD30EnableSiteModeSelect==false, which skips that branch's one
//  conditional early-return entirely) -- chosen so this test does not also
//  have to seed the bTestSiteUse[][][] site-enable grid that ONLY the
//  tcHeadN branches consult.
//
//  ISOLATION -- bUT150Install[]/UN150Read[]/iTempOverShowAlarmT[]/
//  asGPIBTempShow[]/asTempCtrl[] are ALL file-scope globals sized
//  [tcTotalCount], shared across this whole test BINARY (not per-instance
//  state) -- exactly like uYieldMonitoring's own ArmData[]/Prod globals.
//  ResetChannelGlobals() below zeroes every channel's install/reading/alarm
//  state before each case, so no test can leak into another via a stale
//  channel slot.
//
//  SAFETY -- TfTemperFrom's ctor (real, in scope this wave) allocates ~150
//  TPanel/TfTemperFromPanel children via NSDMI and copies them into the
//  file-scope ShowTempComp[]/NameTempComp[] arrays (see forms/fTemperFrom.h's
//  "SINGLE-INSTANCE ASSUMPTION" banner) -- exactly ONE TfTemperFrom is
//  constructed below (`g_form`, function-local static in main(), never a
//  second one), matching that assumption.
// =============================================================================
#include "forms/fTemperFrom.h"

#include "MachineType.h"   // tc* channel enum / eATC*/eht*/eDut*ea / CC_*
#include "cmydef.h"         // bUT150Install[]/UN150Read[]/iTempOverShowAlarmT[]/
                             //   asGPIBTempShow[]/asTempCtrl[]/USE_16_HEATER/ATC_SYSTEM/
                             //   CUSTOMER_CODE/Tri_Temp_Machine/bUseInitTempOffset/...
#include "cprod.h"          // Temperature / TestIF / TestIF_File / DeviceForm_File
#include "Config.h"         // IniConfig
#include "CosFunction.h"    // CosFunction
#include "LastSet.h"        // LastSet.iTemperature / iRunStartMode

#include <cstdio>

// ---------------------------------------------------------------------------
//  ShowTempComp[]/NameTempComp[] are file-scope globals DEFINED in
//  cTemperFrom.cpp (golden cTemperFrom.cpp:36-37) -- not TfTemperFrom
//  members, so forms/fTemperFrom.h does not declare them (see that header's
//  own banner). This test TU needs its own `extern` to see them, same as
//  SECSGEM/uHGemHT9045_SV.cpp:390's pre-existing `extern TPanel
//  *ShowTempComp[tcTotalCount];` (both resolve to the one real definition in
//  cTemperFrom.cpp, now that this wave gives that extern a home).
// ---------------------------------------------------------------------------
extern TPanel *ShowTempComp[tcTotalCount];
extern TPanel *NameTempComp[tcTotalCount];

// PORT-ONLY local mirror, same convention cTemperFrom.cpp itself documents
// in its own file banner (golden Graphics.hpp VCL system colours; FIVE
// existing files each already carry an identical copy rather than a shared
// header) -- this test TU is now a sixth. Confirmed safe (no transitive
// definition already reaches this TU): none of the headers included above
// pull in vclcompat/{TrayCore,BtnPanelCore,LedCore}.h, acatchtray_shims.h,
// or SECSGEM/uHGemEquipment.h.
static const TColor clRed    = 0x000000FF;
static const TColor clLime   = 0x0000FF00;
static const TColor clYellow = 0x0000FFFF;

// ---------------------------------------------------------------------------
//  Minimal CHECK harness (matches tests/test_yieldmon_core.cpp /
//  tests/test_observer_core.cpp precedent)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
        if (cond) { g_pass++; } \
        else { g_fail++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
    } while (0)

// ---------------------------------------------------------------------------
//  Shared baseline -- reset every global ShowThermo/TempRunShowAlarmHigh/Low
//  read, for exactly the one test channel (tcHotPlate1) plus the two other
//  channels the alarm-scan loop (TempRunShowAlarmHigh/Low, tcHotPlate1..
//  tcLBDown) would otherwise also see mid-test if left dirty from an earlier
//  case in this same binary.
// ---------------------------------------------------------------------------
static const int kCh = tcHotPlate1;

static void ResetChannelGlobals()
{
    for (int i = tcHotPlate1; i < tcTotalCount; ++i)
    {
        bUT150Install[i] = false;
        bUT150Install1To2[i] = false;
        bUT150HasUse[i] = false;
        bUT150State[i] = false;
        UN150Read[i] = 0.0;
        UN150ReadSecondpoint[i] = 0.0;
        iTempOverShowAlarmT[i] = 0;
        asGPIBTempShow[i] = "";
        asTempCtrl[i] = "";
    }

    // Config knobs every ShowThermo branch below reads -- pinned so this
    // test controls only what it actually varies (same rationale as
    // tests/test_yieldmon_core.cpp's own ResetCommonGuards()).
    Tri_Temp_Machine = 0;                 // take the bUT150Install-driven branch, not TriTemp
    CUSTOMER_CODE = 0;                    // not CC_ATEC (891) -- avoid the SLK-mirroring branch
    USE_16_HEATER = -1;                   // not any eht*Heater* value -- skip the Head-channel early return (irrelevant to tcHotPlate1 anyway)
    ATC_SYSTEM = eATCHonPrecType;         // a plain, non-eNewATCSystem/eNonChamber value
    Temperature.bATCActiveCooling = false;
    Temperature.bActiveHeatGun = false;
    Temperature.bUseFixTemp = false;
    Temperature.bMultiZoneEnable = false;
    Temperature.bUseIndividualTemp = false;
    CosFunction.bUseIndividulTempSet = false;
    Temperature.fWorkTemperBase = 100.0;  // SetTemp for tcHotPlate1 under Tempture_Hot
    Temperature.fAbitTemp = 25.0;
    IniConfig.bL07UseSingleTenmpertureLimit = false;
    IniConfig.iL04TemptureRange = 5;      // +-5 degree band around fWorkTemperBase
    IniConfig.bL29AmbientNotShowTemp = false;
    IniConfig.bL20AbientGuardBand = false;
    Temperature.bAmbientGuardbandCheck = false;
    bUseInitTempOffset = false;           // skip the init-temp-offset band-narrowing block entirely
    Temperature.bBoostFuncttion = false;
    Temperature.bLBTempFunction = false;
    iTriggerBoostFunction = -1;
    INSTALL_ATC_HEAT_GUN = 0;
    LastSet.iTemperature = Tempture_Hot;  // the "machine is heating" state ShowThermo's main ladder targets
    TestIF.iTestMode = SingleSite;        // bGetHeaterUsed's unconditional-true branch (see file banner)
    IniConfig.bD30EnableSiteModeSelect = false;
    iSocketBaseTempCount = eDut1ea;
    SystemStart = true;
    iHome = 0;
}

// =============================================================================
//  (a)+(c) ShowThermo -- NOT installed
// =============================================================================
static void Test_ShowThermo_NotInstalled(TfTemperFrom &f)
{
    ResetChannelGlobals();
    bUT150Install[kCh] = false;   // the one thing this case actually varies

    bool bOverTemp = f.ShowThermo(kCh);

    CHECK(ShowTempComp[kCh]->Caption == AnsiString("---"), "ShowThermo/NotInstalled: Caption == \"---\"");
    CHECK(ShowTempComp[kCh]->Color == (TColor)0x0025AB12, "ShowThermo/NotInstalled: Color == green (0x0025AB12)");
    CHECK(bOverTemp == false, "ShowThermo/NotInstalled: returns false (no CCD-cooling need)");
}

// =============================================================================
//  (a)+(c) ShowThermo -- installed, reading dead-center in band -> green
// =============================================================================
static void Test_ShowThermo_InBand_Green(TfTemperFrom &f)
{
    ResetChannelGlobals();
    bUT150Install[kCh] = true;
    UN150Read[kCh] = 100.0;   // == fWorkTemperBase, dead center of the +-5 band

    bool bOverTemp = f.ShowThermo(kCh);

    CHECK(ShowTempComp[kCh]->Caption == AnsiString("100.0"), "ShowThermo/InBand: Caption == \"100.0\"");
    CHECK(ShowTempComp[kCh]->Color == (TColor)0x0025AB12, "ShowThermo/InBand: Color == green (0x0025AB12)");
    CHECK(iTempOverShowAlarmT[kCh] == 0, "ShowThermo/InBand: iTempOverShowAlarmT == 0 (no alarm class)");
    CHECK(bOverTemp == false, "ShowThermo/InBand: returns false");
}

// =============================================================================
//  (a)+(b)+(c) ShowThermo -- installed, OVER the band -> orange + alarm-high
// =============================================================================
static void Test_ShowThermo_OverBand_AlarmHigh(TfTemperFrom &f)
{
    ResetChannelGlobals();
    bUT150Install[kCh] = true;
    UN150Read[kCh] = 150.0;   // fWorkTemperBase(100) + range(5) = 105 ceiling; 150 is 45 degrees clear of it

    bool bOverTemp = f.ShowThermo(kCh);

    CHECK(ShowTempComp[kCh]->Caption == AnsiString("150.0"), "ShowThermo/OverBand: Caption == \"150.0\"");
    CHECK(ShowTempComp[kCh]->Color == (TColor)0x008000FF, "ShowThermo/OverBand: Color == over-temp orange (0x008000FF)");
    CHECK(iTempOverShowAlarmT[kCh] == 1, "ShowThermo/OverBand: iTempOverShowAlarmT == 1 (over)");
    CHECK(bOverTemp == false, "ShowThermo/OverBand: returns false (tcHotPlate1 < tcHead1, no CCD-cooling trigger)");

    CHECK(f.TempRunShowAlarmHigh() == true, "TempRunShowAlarmHigh: true after an OVER classification");
    CHECK(f.TempRunShowAlarmLow() == false, "TempRunShowAlarmLow: false when only the HIGH alarm is set");
}

// =============================================================================
//  (a)+(b)+(c) ShowThermo -- installed, UNDER the band, Hot mode -> yellow + alarm-low
// =============================================================================
static void Test_ShowThermo_UnderBand_AlarmLow(TfTemperFrom &f)
{
    ResetChannelGlobals();
    bUT150Install[kCh] = true;
    UN150Read[kCh] = 50.0;    // fWorkTemperBase(100) - range(5) = 95 floor; 50 is 45 degrees clear of it

    bool bOverTemp = f.ShowThermo(kCh);

    CHECK(ShowTempComp[kCh]->Caption == AnsiString(" 50.0"), "ShowThermo/UnderBand: Caption == \" 50.0\" (%5.1f pads to width 5)");
    CHECK(ShowTempComp[kCh]->Color == clYellow, "ShowThermo/UnderBand: Color == clYellow (LastSet.iTemperature==Tempture_Hot path)");
    CHECK(iTempOverShowAlarmT[kCh] == 2, "ShowThermo/UnderBand: iTempOverShowAlarmT == 2 (below)");
    CHECK(bOverTemp == false, "ShowThermo/UnderBand: returns false");

    CHECK(f.TempRunShowAlarmLow() == true, "TempRunShowAlarmLow: true after an UNDER classification");
    CHECK(f.TempRunShowAlarmHigh() == false, "TempRunShowAlarmHigh: false when only the LOW alarm is set");
}

// =============================================================================
//  (b) TempRunShowAlarmHigh/Low -- explicit false/false case, freshly reset
// =============================================================================
static void Test_TempRunShowAlarm_BothFalse_WhenNoneClassified(TfTemperFrom &f)
{
    ResetChannelGlobals();   // every iTempOverShowAlarmT[i]==0, no bUT150Install[] set

    CHECK(f.TempRunShowAlarmHigh() == false, "TempRunShowAlarmHigh: false on a freshly reset channel set");
    CHECK(f.TempRunShowAlarmLow() == false, "TempRunShowAlarmLow: false on a freshly reset channel set");
}

// =============================================================================
//  Bonus coverage -- SetShowYield/ShowYieldFuntion (golden :1473-1579)
//
//  strShowYield[]'s OnOff/bFlag/iCount fields are NOT touched by the ctor
//  (golden's own ctor loop only sets UsePanel/asName/Caption/Color/bShow --
//  verified this wave, see cTemperFrom.cpp's ctor) -- exactly like golden's
//  own BCB6 object layout, they are indeterminate until first written. This
//  test writes OnOff explicitly before ever reading it (through
//  SetShowYield, which is itself the real, only production writer), so no
//  case here ever reads an indeterminate value.
// =============================================================================
static void Test_SetShowYield_TogglesCaptionAndColor(TfTemperFrom &f)
{
    f.strShowYield[TfTemperFrom::esytYieldMonitor].bShow = true;   // "this function is enabled" gate

    f.SetShowYield(TfTemperFrom::esytYieldMonitor, true);
    CHECK(f.strShowYield[TfTemperFrom::esytYieldMonitor].UsePanel->Caption == AnsiString("Yield Monitoring On"),
          "SetShowYield(true): Caption == \"Yield Monitoring On\"");
    CHECK(f.strShowYield[TfTemperFrom::esytYieldMonitor].UsePanel->Color == clLime,
          "SetShowYield(true): Color == clLime");

    CUSTOMER_CODE = 0;   // not CC_ASE_M -- take the "show ... Off" arm, not the ASEM blank-out arm
    f.SetShowYield(TfTemperFrom::esytYieldMonitor, false);
    CHECK(f.strShowYield[TfTemperFrom::esytYieldMonitor].UsePanel->Caption == AnsiString("Yield Monitoring Off"),
          "SetShowYield(false): Caption == \"Yield Monitoring Off\"");
    CHECK(f.strShowYield[TfTemperFrom::esytYieldMonitor].UsePanel->Color == clRed,
          "SetShowYield(false): Color == clRed");
}

// =============================================================================
//  Bonus coverage -- ShowHotName (golden :1772-1818)
// =============================================================================
static void Test_ShowHotName_NameVsNotInstalled(TfTemperFrom &f)
{
    ResetChannelGlobals();
    Tri_Temp_Machine = 0;             // skip the Tri_Temp_Machine==1 shuttle/door name block

    bUT150Install[tcHotPlate1] = true;
    asTempCtrl[tcHotPlate1] = "MyHotPlate1";
    bUT150Install[tcHotPlate2] = false;

    f.ShowHotName();

    CHECK(NameTempComp[tcHotPlate1]->Caption == AnsiString("MyHotPlate1"),
          "ShowHotName: installed channel takes its name from asTempCtrl[]");
    CHECK(NameTempComp[tcHotPlate2]->Caption == AnsiString("---"),
          "ShowHotName: not-installed channel's Name AND Show panels both show \"---\"");
    CHECK(ShowTempComp[tcHotPlate2]->Caption == AnsiString("---"),
          "ShowHotName: not-installed channel's Show panel also \"---\"");
}

// =============================================================================
//  Bonus coverage -- FormClose (golden :1684-1690)
// =============================================================================
static void Test_FormClose_StopsShowAndTimer(TfTemperFrom &f)
{
    f.bShow = true;
    f.Timer1->Enabled = true;

    f.FormClose();

    CHECK(f.bShow == false, "FormClose: bShow becomes false");
    CHECK(f.Timer1->Enabled == false, "FormClose: Timer1->Enabled becomes false");
}

int main()
{
    // Exactly ONE TfTemperFrom for the whole binary -- see file-head SAFETY note.
    static TfTemperFrom g_form;

    Test_ShowThermo_NotInstalled(g_form);
    Test_ShowThermo_InBand_Green(g_form);
    Test_ShowThermo_OverBand_AlarmHigh(g_form);
    Test_ShowThermo_UnderBand_AlarmLow(g_form);
    Test_TempRunShowAlarm_BothFalse_WhenNoneClassified(g_form);
    Test_SetShowYield_TogglesCaptionAndColor(g_form);
    Test_ShowHotName_NameVsNotInstalled(g_form);
    Test_FormClose_StopsShowAndTimer(g_form);

    std::printf("%d/%d checks passed (test_temperfrom_core)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
