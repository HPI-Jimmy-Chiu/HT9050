// ===========================================================================
//  tests/test_teach_formshow_bridge.cpp -- AI(W906-TEACH-FORMSHOW) 20261002: the Teach page's golden FormShow bridge
//  (FileRW/TeachFormShow_File.cpp, golden uteach.cpp:1413-2012 screen half).
//
//  The bridge is evaluated twice with the machine-option globals set to two recognisable configurations; every assertion
//  names the golden line it pins.  Memory only: no machine file is read or written, no card, no motor.
//  This test owns the bridge table (kBridges = { Teach }), so FormBridge.cpp's FindBridge is exercised too.
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <string>

#include "JsonBridge/FormBridge.h"
#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"
#include "CosFunction.h"

namespace ht9045 { namespace formbridge {
extern const BridgeDesc kBridge_TfTeach;
extern const BridgeDesc* const kBridges[] = { &kBridge_TfTeach };
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
    const BridgeDesc* b = ht9045::formbridge::FindBridge("HW.teach.html");
    check(b != nullptr, "FindBridge(\"HW.teach.html\") = the Teach bridge");
    if (!b) return 1;
    check(b->display != nullptr && b->save == nullptr && b->saveFlow == nullptr,
          "display only: no save / saveFlow (form.save answers 405 'no save flow'; Teach saves through FileRW/Teach.cpp)");
    check(b->sourceGap[0] == '\0', "sourceGap empty (the conditions read globals LoadMachineConfig already filled)");

    // ---- configuration A ----------------------------------------------------------------------------------------
    std::printf("[A] Preciser on, OCR off, 3 auto colours, Fix3 short shuttle, no rotate kit, plain pitch\n");
    USE_PRECISER = 1;  INSTALL_OCR = eocrUninstal;  BOTTOM_2DID = 0;  USE_LASER_DISTANCE = 0;
    AUTO_EMPTY_COLOR = 3;  FIX3_FULL_PLACE = Fix3K_ShortShuttle;  USE_ROTATE_KIT = 0;  USE_DIE_CLEAN = 0;
    USE_PICKER_COUNT = 0;  USE_IN_OUT_ARM_Y_PITCH = 0;  USE_OUT_SORT_ARM = eartUninstall;  CUSTOMER_CODE = 0;
    IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange = false;
    FormState J;
    b->display(J);
    const std::string js = J.WidgetsJson();
    if (dumpA) {
        if (FILE* f = std::fopen(dumpA, "wb")) {
            std::fprintf(f, "{\"page\":\"HW.teach.html\",\"form\":\"TfTeach\",\"kind\":\"golden-bridge\",\"available\":true,\"sourceGap\":\"\",\"widgets\":%s,\"todo\":%s}",
                         js.c_str(), J.TodoJson().c_str());
            std::fclose(f);
        }
    }
    check(J.GetVisible("pnlPrecisor_Go") && J.GetVisible("pnlPreciser_XY") && J.GetVisible("grpPrecisor_Open"),
          ":1443-1446 USE_PRECISER==1 -> the four Preciser panels visible");
    check(!J.GetVisible("pnlOcr_Go") && !J.GetVisible("pnlOcr_XY"), ":1437-1438 INSTALL_OCR==eocrUninstal -> OCR hidden");
    check(!J.GetVisible("pnlLaserHP1_Go") && !J.GetVisible("palLaserInShuttle"), ":1619-1623 USE_LASER_DISTANCE==0 -> laser hidden");
    check(J.GetVisible("pnlAuto4_XY") && J.GetVisible("pnlFix6_XY") && !J.GetVisible("pnlAuto6_XY") && !J.GetVisible("btnAuto6"),
          ":1830-1864 AUTO_EMPTY_COLOR=3 -> Auto4/5 + Fix4-6 visible, Auto6 hidden");
    check(has(js, "\"setEditFix1X\":{\"color\":\"#ffff00\"}"), ":1645 FIX3_FULL_PLACE==Fix3K_ShortShuttle -> setEditFix1X clYellow (#ffff00)");
    check(has(js, "\"pnlStop\":{\"color\":\"#ccd9df\"}"), ":1435 pnlStop (TColor)0x00DFD9CC -> #ccd9df");
    check(!J.GetTabVisible("tsRotate") && !J.GetVisible("grpRotate_Axis") && !J.GetVisible("pnlInRot_Go"),
          ":1651-1668 USE_ROTATE_KIT==0 -> rotate tab / axis group / station hidden");
    check(has(js, "\"lblInRot_XY\":{\"caption\":\"Rotate\"}"), ":1707 USE_DIE_CLEAN!=1 -> lblInRot_XY 'Rotate'");
    check(!J.GetVisible("gbInX240mm") && !J.GetTabVisible("tsYPitch15"), ":1710/:1739 no variable pitch -> XP2 / Y-pitch tab hidden");
    check(has(js, "\"PageControl2\":{\"activePageIndex\":8,\"activePage\":\"tsAxleCtrl\"}"), ":1599-1600 opens on Axle Control");
    check(has(js, "\"pgcShuttle\":{\"activePage\":\"tsShuttlePos\"}"), ":1456 pgcShuttle -> tsShuttlePos");
    check(has(js, "\"lblFindPhaseNotes\":{\"caption\":\" \"}"), ":1881 no find-phase difference -> lblFindPhaseNotes ' '");
    check(!J.GetVisible("grpLoadY"), ":1993 no OCR Y motor / tray OCR / cassette mode -> grpLoadY hidden");
    check(!J.GetVisible("SetButton070") && J.GetVisible("SetButton064"), ":1427-1434 Index Arm buttons 070/071 hidden, 064/065 shown");
    check(!J.GetTabVisible("tsPickUpErrorPlacement"), ":1759 no fMain in this process -> the port's offline InArmPlacementEnable()==false");
    bool todoCaption = false;
    for (std::size_t i = 0; i < J.Todos().size(); ++i) if (has(J.Todos()[i], ":1893-1939")) todoCaption = true;
    check(todoCaption, ":1893-1939 no TfTeach facade here -> the pitch captions are reported as a todo, not invented");

    // ---- configuration B ----------------------------------------------------------------------------------------
    std::printf("[B] Preciser off, OCR on, 4 auto colours, Fix3 stepper, 8-motor rotate kit, find-phase difference\n");
    USE_PRECISER = 0;  INSTALL_OCR = (eocrUninstal == 0) ? 1 : 0;  AUTO_EMPTY_COLOR = 4;  FIX3_FULL_PLACE = Fix3K_UseStepperMotor;
    USE_ROTATE_KIT = 1;  iRotate_Type = e8MotRotate;
    IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange = true;  bY1ModifyDistanceRef = true;
    FormState K;
    b->display(K);
    const std::string ks = K.WidgetsJson();
    check(!K.GetVisible("pnlPrecisor_Go"), ":1443 USE_PRECISER==0 -> Preciser hidden");
    check(K.GetVisible("pnlOcr_Go"), ":1437 OCR installed -> pnlOcr_Go visible");
    check(K.GetVisible("pnlAuto6_XY") && K.GetVisible("btnAuto6"), ":1836/:1862 AUTO_EMPTY_COLOR=4 -> Auto6 visible");
    check(has(ks, "\"setEditFix1X\":{\"color\":\"#ffffff\"}"), ":1645 not short shuttle -> clWhite (#ffffff)");
    check(K.GetVisible("gbFix3") && has(ks, "\"SetButtonPlace6\":{\"caption\":\"Fix3\"}"), ":1760-1762 Fix3 stepper -> gbFix3 + caption 'Fix3'");
    check(K.GetTabVisible("tsRotate") && K.GetVisible("grpInRot_Axis") && K.GetVisible("grbInRC") && !K.GetVisible("grpRotate_Axis"),
          ":1651-1703 e8MotRotate -> rotate tab, per-sucker groups; not the 1-motor group");
    check(!K.GetVisible("lblOutRotateFixG") && !K.Has("lblOutRotateFixE"), ":1684-1688 not 4/2-motor -> only lblOutRotateFixG hidden");
    check(has(ks, "\"PageControl2\":{\"activePageIndex\":8,\"activePage\":\"TabSheet10\"}"), ":1875 find-phase difference -> forced to TabSheet10");
    check(has(ks, "Index Y"), ":1876 the find-phase note is shown");

    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
