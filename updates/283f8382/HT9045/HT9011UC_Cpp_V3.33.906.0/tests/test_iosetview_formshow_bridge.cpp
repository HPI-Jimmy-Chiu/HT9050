// ===========================================================================
//  tests/test_iosetview_formshow_bridge.cpp -- AI(W906-IOSV-FORMSHOW) 20261002: the IO page's golden FormShow bridge
//  (FileRW/IoSetViewFormShow_File.cpp, golden iosetview.cpp:236-1023 screen half + ShowSuckMode / ShowShuttleSensor /
//  LabSiteMap / Hide_1032_IO).
//
//  The bridge is evaluated twice with the machine-option globals set to (A) this HT9050's Gerneral.ini values (2026-10-01)
//  and (B) a recognisably different set; every assertion names the golden line it pins.
//  Memory only: no machine file is read or written, no card, no motor.  This test owns the bridge table.
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <string>

#include "JsonBridge/FormBridge.h"
#include "FileRW/_FormEvent.h"   // AI(W906-IOSV-SHOWALL) 20261002: formevent::Request / Result for RunEvent
#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "cprod.h"
#include "LastSet.h"
#include "Motor/mymotor.h"

namespace ht9045 { namespace formbridge {
extern const BridgeDesc kBridge_Tfiosetview;
extern const BridgeDesc* const kBridges[] = { &kBridge_Tfiosetview };
extern const std::size_t kBridgeCount = 1;
} }

using ht9045::formbridge::FormState;
using ht9045::formbridge::BridgeDesc;

static int g_fail = 0, g_pass = 0;
static void check(bool ok, const char* what) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", what);
    if (ok) ++g_pass; else ++g_fail;
}
static bool has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

int main(int argc, char** argv) {
    const char* dumpA = argc > 1 ? argv[1] : nullptr;   // optional: write configuration A's /api/form answer there (page probe input)
    const BridgeDesc* b = ht9045::formbridge::FindBridge("HW.IoSetView.html");
    check(b != nullptr, "FindBridge(\"HW.IoSetView.html\") = the IO page bridge");
    if (!b) return 1;
    check(b->display != nullptr && b->save == nullptr && b->saveFlow == nullptr,
          "display only: no save / saveFlow (the IO table saves through the engine's sysGrid)");

    // ---- configuration A: this machine's D:\HT9045\system\Gerneral.ini (read 2026-10-01) --------------------------------
    std::printf("[A] HT9050 as HT9046_LS: AUTO_EMPTY_COLOR=0, PCI1203_IO, ION_FAN_TYPE=3, 1 picker, tray arm above, no Z motors\n");
    MachineTypeChoice = Type_HT9046_LS;  IO_CARD_TYPE = PCI1203_IO;  AUTO_EMPTY_COLOR = 0;  USE_2nd_LOADER = 0;
    USE_OUT_SORT_ARM = eartUninstall;  USE_AOI_Inspection = 0;  USE_Fix_AI_CCD = 0;  USE_Scanner_AOI_Inspection = 0;
    USE_PICKER_COUNT = 1;  ENABLE_OUT_SHUTTLE_SENEOR = true;  IN_SHT_LAST_SENSOR = 0;  EP_Install = 3;
    ION_FAN_TYPE = e2IoforOne;  INDEX_SUCKER_TYPE = 1;  TRAY_ARM_MODE = eAboveCoveyor;  IniConfig.bC03UseCatchTray = false;
    CUSTOMER_CODE = 957;  NUMBER_PANEL_TYPE = 3;  Enable_PLCSafety_IO = false;  USE_E84_Sensor = 0;  USE_LdUldCassetteMode = 0;
    Tri_Temp_Machine = 0;  SHUTTLE_FLOODGATE = 0;  USE_AUTO_RETEST = eartUninstall;  REAL_TIME_CCD = false;
    for (int i = 0; i < 9; ++i) LOAD_Z_USE_MOTOR[i] = false;
    TestIF_File.iTestMode = SingleSite;
    FormState J;
    b->display(J);
    const std::string js = J.WidgetsJson();
    if (dumpA) {
        if (FILE* f = std::fopen(dumpA, "wb")) {
            std::fprintf(f, "{\"page\":\"HW.IoSetView.html\",\"form\":\"Tfiosetview\",\"kind\":\"golden-bridge\",\"available\":true,\"sourceGap\":\"\",\"widgets\":%s,\"todo\":%s}",
                         js.c_str(), J.TodoJson().c_str());
            std::fclose(f);
        }
    }
    check(!J.GetVisible("grpEmpty") && !J.GetVisible("grpColor") && !J.Has("grpManual"),
          ":285-293 AUTO_EMPTY_COLOR==0 -> grpEmpty / grpColor hidden, grpManual untouched");
    check(!J.GetTabVisible("tsUnLoader2") && !J.GetVisible("grpAuto6"), ":299-300 AUTO_EMPTY_COLOR<3 -> Stack 3 tab hidden, grpAuto6 hidden");
    check(!J.GetVisible("grpLoader2") && !J.GetVisible("grpLoader2_Under") && !J.GetVisible("pnlOutArm2Suck"),
          ":295-297 no 2nd loader / no out sort arm -> hidden");
    check(!J.GetVisible("palArm1_A") && !J.GetVisible("palArm2_B") && !J.Has("palArm1_A_16"),
          ":258-282 MachineTypeChoice!=Type_HT9045 -> the 8-sucker index panels hidden, the 16 ones untouched");
    check(J.GetTabVisible("tsIOTable"), ":248 IO table tab shown for PCI1203_IO (Jimmy 20260925 ruling A, as cinitial.cpp:505)");
    check(!J.GetTabVisible("tsSafePLC"), ":251 Enable_PLCSafety_IO==false -> Safe PLC tab hidden");
    check(J.GetVisible("gbIonFanPower") && has(js, "\"lblIonFan01\":{\"caption\":\"Ion Fan 1\"}") && J.GetVisible("ledIonFan11"),
          ":549-569 ION_FAN_TYPE==e2IoforOne -> power group, 'Ion Fan 1' captions, fans 6-11 shown");
    check(!J.GetVisible("ledIonFan13") && !J.GetVisible("ledSnSystemPower"), ":594-596 ledIonFan13/14 + ledSnSystemPower always hidden");
    check(J.GetTabVisible("tsTrayArm") && !J.GetTabVisible("tsUnderArm") && !J.GetTabVisible("tsRTArm") && J.GetVisible("palTrayArm"),
          ":642-661 tray arm above conveyor, no catch tray -> Tray Arm tab only");
    check(J.GetVisible("ledNegAir1") && !J.GetVisible("ledNegAir2"), ":691-704 INDEX_SUCKER_TYPE==1, SnNegativePressureAir2 off -> Neg Air 1 only");
    check(!J.GetVisible("btnSwInArmZBreaker") && J.GetVisible("pnl8PickerInArm") && !J.GetVisible("pnl16PickerInArm"),
          ":322 / :1017-1018 USE_PICKER_COUNT=1 (not 1-picker, not 16) -> no Z breaker button, 8-picker panel");
    check(!J.GetTabVisible("tsStack1_Above") && !J.GetTabVisible("tsStack2_Cassette") && has(js, "\"pgcStack1\":{\"activePageIndex\":0,\"activePage\":\"tsStack1_Above\"}"),
          ":3909-3929 Hide_1032_IO: the Above/Under/Cassette tabs hidden, pgcStack1 left on Above (TRAY_ARM_MODE==eAboveCoveyor)");
    check(!J.GetVisible("lblLoaderZ") && !J.Has("btnC_Load_Up"), ":811 / :830 no Z motor -> 'not homed' label hidden, buttons untouched");
    check(!J.GetVisible("grpOutShuttle") && J.GetVisible("MyLedInShuttleNumR9"), ":1725-1733 not HT9045 -> out-shuttle group hidden, R8/R9/F8/F9 shown");
    check(!J.GetVisible("btnAllVacuum") || !J.Has("btnAllVacuum"), ":682 CUSTOMER_CODE!=CC_HONPREC_QC -> All Vacuum not shown");
    check(J.GetVisible("chkShowIndexAll"), ":374-378 iTestMode<_8Site2X4 -> 'show all index' checkbox shown");
    check(J.Has("btnSwCCDCooling") && J.GetCaption("btnSwCCDCooling") == AnsiString("Ion Air"), ":732-741 no real-time CCD -> 'Ion Air' caption");

    SThreadPara.iScanSensor = 2;  SThreadPara.bUseM204Mode = false;
    { FormState L; b->display(L); check(L.GetCaption("labUseSensor1") == AnsiString("Use Z Sensor with Thread") && L.GetCaption("labUseSensor2") == AnsiString("Use Z Sensor with Thread"),
          "cinitial.cpp:15034-15047 two scan sensors, no M204 -> both labUseSensor captions 'Use Z Sensor with Thread'"); }
    SThreadPara.iScanSensor = 1;  SThreadPara.bExeShuttleThread = false;  SThreadPara.bOutYUseLatch = true;
    { FormState L; b->display(L); check(L.GetCaption("labUseSensor1") == AnsiString("Use Y Sensor with Latch"), "cinitial.cpp:15012-15024 one scan sensor, Y latch -> 'Use Y Sensor with Latch'"); }

    // ---- AI(W906-IOSV-SHOWALL) 20261002: form.event chkShowIndexAll click = golden chkShowIndexAllClick :1760-1766 ---------
    {
        formevent::Request r;  r.form = "Tfiosetview";  r.control = "chkShowIndexAll";  r.event = "click";  r.hasChecked = true;  r.checked = true;
        formevent::Result res;
        const bool ok = ht9045::formbridge::RunEvent(*b, r, &res);
        const std::string& ch = res.changedJson;
        const std::size_t at = ch.find("\"palArm1_Ab_Name\":{");
        const std::string ab = at == std::string::npos ? std::string() : ch.substr(at, ch.find('}', at) - at);
        check(ok && has(ch, "\"chkShowIndexAll\":{\"visible\":false}"), ":1765 click -> the box hides itself");
        check(has(ab, "\"color\":\"#3e3a39\"") && has(ab, "\"caption\":"), ":1318-1344 ShowSuckMode(-1) -> Ab drawn in use (0x00393A3E) with its name (was #dcdddd, '')");
        check(has(ch, "\"labOutArmAh\":{\"color\":\"#3e3a39\"}"), ":1342 OutArm_16 drawn in use too");
        TestIF_File.iTestMode = _8Site2X4;                                               // FormShow :380 then hides the box
        formevent::Result no;
        check(!ht9045::formbridge::RunEvent(*b, r, &no) && no.code == "bad-payload", ":374-383 box hidden by FormShow -> golden cannot click it -> refused");
        TestIF_File.iTestMode = SingleSite;
    }

    // ---- configuration B ----------------------------------------------------------------------------------------
    std::printf("[B] HT9045, 4 auto colours, 5-IO ion fan, tray arm under conveyor, Loader Z motor not homed, HONPREC QC, ISA IO\n");
    MachineTypeChoice = Type_HT9045;  IO_CARD_TYPE = 0;  AUTO_EMPTY_COLOR = 4;  ION_FAN_TYPE = e5IOforAll;
    TRAY_ARM_MODE = eUnderCoveyor;  LOAD_Z_USE_MOTOR[0] = true;  MOT[MLoaderZ].HomeFlag = 0;  CUSTOMER_CODE = CC_HONPREC_QC;
    USE_PICKER_COUNT = ep16Picker;  USE_LdUldCassetteMode = 1;
    FormState K;
    b->display(K);
    const std::string ks = K.WidgetsJson();
    check(!K.GetVisible("grpManual") && !K.Has("grpEmpty") && K.GetTabVisible("tsUnLoader2") && K.GetVisible("grpAuto6"),
          ":285-300 AUTO_EMPTY_COLOR=4 -> manual track hidden, Stack 3 tab + grpAuto6 shown");
    check(!K.GetVisible("palArm1_A_16") && !K.Has("palArm1_A"), ":258-263 Type_HT9045 -> the 16-sucker index panels hidden");
    check(!K.GetTabVisible("tsIOTable"), ":248 ISA IO card -> IO table tab hidden");
    check(has(ks, "\"lblIonFan01\":{\"caption\":\"IonFanAlarm\"}") && !K.GetVisible("ledIonFan06"), ":574-592 e5IOforAll -> alarm captions, fans 6-11 hidden");
    check(K.GetTabVisible("tsUnderArm") && !K.GetTabVisible("tsTrayArm") && !K.GetVisible("bplC_TrayX_UpDown"), ":603-611 under conveyor -> Under Arm tab only");
    check(K.GetEnabled("btnC_Load_Up") && K.GetEnabled("btnC_Load_Middle_U") && K.GetVisible("lblLoaderZ") && K.GetVisible("lblLoaderZ_U"),
          ":837-845 Loader Z is a motor, not homed -> its buttons enabled, 'not homed' shown");
    check(K.GetVisible("btnAllVacuum") && K.GetEnabled("btnAllDestory") && K.GetVisible("grpShtRotate"), ":682-689 HONPREC QC -> All Vacuum / Destroy + rotate group");
    check(K.GetVisible("grpOutShuttle") && !K.GetVisible("MyLedInShuttleNumR8"), ":1702-1724 HT9045 -> out-shuttle group = ENABLE_OUT_SHUTTLE_SENEOR, R8 hidden");
    check(K.GetVisible("pnl16PickerInArm") && !K.GetVisible("pnl8PickerOutArm"), ":1017-1020 16 pickers -> 16-picker panels");
    check(has(ks, "\"pgcStack1\":{\"activePageIndex\":2,\"activePage\":\"tsStack1_Cassette\"}"), ":3917-3922 cassette mode -> Stack 1 opens on Cassette");

    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
