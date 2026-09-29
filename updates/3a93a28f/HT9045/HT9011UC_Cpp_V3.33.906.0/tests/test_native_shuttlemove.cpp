// ===========================================================================
//  tests/test_native_shuttlemove.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): ctest NativeShuttleMove_Headless -- the HW.ShuttleMove native window (v1,
//  display only, ui/native/NativeShuttleMove.h).  Headless: hidden window, synthetic state, read back through the grids.
//  ⚠ Links only the ui/native window files (NativeForms.cmake): no path to motion, IO, the card or any file.
//    1. created hidden; every golden command button disabled -- only Exit enabled
//    2. encoders ("—" without a value), the 12 teach points, the tray layout, the FormShow rule rows
//    3. golden visibility: no bar code -> its points / tray / buttons hidden; auto latch -> the latch group shown;
//       btRetry only in SIM; SPIL -> the points show "locked"
//    4. no-change update -> 0 cells in every grid; one value -> exactly one cell
//    5. a WM_COMMAND from a disabled command id does nothing; Exit closes only this window
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"
#include "ui/native/NativeShuttleMove.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace w906native;

namespace {

int g_fail = 0, g_pass = 0;

void Check(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  ok   %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL %s\n", what); }
    std::fflush(stdout);
}

void Pump()
{
    for (int i = 0; i < 20; ++i) PumpThreadMessages(500, 0);
}

void Point(ShuttleMoveState& s, const char* name, const char* group, const char* field, bool vis, int value)
{
    ShuttlePoint p;
    p.name = name; p.group = group; p.field = field; p.visible = vis; p.value = value;
    s.points.push_back(p);
}

void Tray(ShuttleMoveState& s, const char* name, int x, int y, bool vis)
{
    ShuttleTray t;
    t.name = name; t.xItem = x; t.yItem = y; t.visible = vis;
    s.trays.push_back(t);
}

// the same 12 points / 4 trays the glue builds (NativeShuttleMoveGlue.cpp), visibility per the golden rules
ShuttleMoveState Make(bool barCode, bool latch, bool sim, bool spil)
{
    ShuttleMoveState s;
    s.buildConfig = "TEST";
    s.enc[0].alias = "InShuttle1"; s.enc[0].has = true; s.enc[0].enc = 12345; s.enc[0].source = "pci1203-monitor";
    s.enc[1].alias = "InShuttle2"; s.enc[1].has = false; s.enc[1].source = "none"; s.enc[1].why = "no monitor value";
    s.barCodeVisible = barCode; s.latchVisible = latch; s.retryVisible = sim; s.spilLocked = spil;
    Point(s, "edInSH1Sen7DetectPos", "In Fiber Check", "Tech.iInSH1Sen7DetectPos", true, 100);
    Point(s, "edInSH2Sen7DetectPos", "In Fiber Check", "Tech.iInSH2Sen7DetectPos", true, 200);
    Point(s, "edInSH1BarCodePos", "Move to Bar Code Pos", "Tech.iInSH1BarCodePos", barCode, 300);
    Point(s, "edInSH2BarCodePos", "Move to Bar Code Pos", "Tech.iInSH2BarCodePos", barCode, 400);
    Point(s, "edOutSH1OneRowDetectPos", "Out Fiber Check", "Tech.OutSH1ZOneRowDetectPos", true, 500);
    Point(s, "edOutSH2OneRowDetectPos", "Out Fiber Check", "Tech.OutSH2ZOneRowDetectPos", true, 600);
    Point(s, "edOutSH1ZDetectPos", "Out Fiber Check", "Tech.OutSH1ZDetectPos", true, 700);
    Point(s, "edOutSH2ZDetectPos", "Out Fiber Check", "Tech.OutSH2ZDetectPos", true, 800);
    Point(s, "edInSH1SenICDetectPos", "In Fiber Check - Shuttle sensor latch", "Tech.iInSH1SenICDetectPos", latch, 900);
    Point(s, "edInSH2SenICDetectPos", "In Fiber Check - Shuttle sensor latch", "Tech.iInSH2SenICDetectPos", latch, 1000);
    Point(s, "edInSH1SenICDetectZPos", "In Fiber Check - Shuttle sensor latch", "Prod.iInSH1SenICAddPos", latch, 11);
    Point(s, "edInSH2SenICDetectZPos", "In Fiber Check - Shuttle sensor latch", "Prod.iInSH2SenICAddPos", latch, 12);
    for (std::size_t i = 0; i < s.points.size(); ++i) s.points[i].locked = spil && s.points[i].visible;
    Tray(s, "mtedInSHSen7DetectPos", 4, 2, true);
    Tray(s, "mtInSHBarCodePos", 4, 2, barCode);
    Tray(s, "mtOutSHDetectPos", 4, 4, true);
    Tray(s, "mtedInSHSenICDetectPos", 4, 2, latch);
    return s;
}

unsigned long Painted(int which) { return GridGetStats((HWND)ShuttleMoveGrid(which)).cellsPainted; }

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("=== NativeShuttleMove_Headless (ui/native/NativeShuttleMove, v1 display only) ===\n");

    std::printf("\n-- 1. window, buttons\n");
    Check(ShuttleMoveOpen(false), "1. the window is created (hidden)");
    Pump();
    Check(ShuttleMoveButtonCount() >= 20, "1. golden's command buttons exist");
    Check(ShuttleMoveEnabledButtonCount() == 1, "1. only one button is enabled (Exit)");

    std::printf("\n-- 2. data (bar code installed, no latch, SIM, no SPIL)\n");
    ShuttleMoveState s = Make(true, false, true, false);
    ShuttleMoveUpdate(s);
    Pump();
    Check(ShuttleMoveEncText(0, kSmEncValue) == L"12345" && ShuttleMoveEncText(1, kSmEncValue) == L"—", "2. encoders: a value, and — without one");
    Check(ShuttleMovePointCount() == 8, "2. 8 teach points shown (the latch group is hidden)");
    Check(ShuttleMovePointText(0, kSmPtValue) == L"100" && ShuttleMovePointText(0, kSmPtName) == L"edInSH1Sen7DetectPos", "2. point row 0");
    Check(ShuttleMoveTrayCount() == 3 && ShuttleMoveTrayText(2, kSmTrSize) == L"4 x 4", "2. trays: 3 shown, mtOutSHDetectPos 4 x 4");
    Check(ShuttleMoveTrayText(0, kSmTrLayout) == L"□□□□ / □□□□", "2. tray layout drawn");
    Check(ShuttleMoveButtonVisible(L"Retry") && !ShuttleMoveButtonVisible(L"In Shuttle 1 16 site kit pos"),
          "2. btRetry shown (SIM), the latch buttons hidden");
    Check(ShuttleMoveButtonVisible(L"In Shuttle 1 Bar Code Pos"), "2. the bar code buttons shown");

    std::printf("\n-- 3. golden visibility rules\n");
    ShuttleMoveState s2 = Make(false, true, false, true);
    ShuttleMoveUpdate(s2);
    Check(ShuttleMovePointCount() == 10, "3. no bar code, latch: 10 points (8 - 2 bar code + 4 latch)");
    Check(ShuttleMovePointText(0, kSmPtState).find(L"SPIL") != std::wstring::npos, "3. SPIL: the points show locked");
    Check(!ShuttleMoveButtonVisible(L"In Shuttle 1 Bar Code Pos") && ShuttleMoveButtonVisible(L"In Shuttle 1 16 site kit pos"),
          "3. bar code buttons hidden, latch buttons shown");
    Check(!ShuttleMoveButtonVisible(L"Retry"), "3. btRetry hidden (not SIM)");
    Check(ShuttleMoveTrayCount() == 3 && ShuttleMoveFlagText(kSmFlSpil).find(L"是") == 0, "3. trays follow, the SPIL rule row says yes");
    Check(ShuttleMoveEnabledButtonCount() == 1, "3. still only Exit enabled");

    std::printf("\n-- 4. only changed cells\n");
    ShuttleMoveUpdate(s2);
    const unsigned long e0 = Painted(0), p0 = Painted(1), t0 = Painted(2), f0 = Painted(3);
    ShuttleMoveUpdate(s2);
    Check(Painted(0) == e0 && Painted(1) == p0 && Painted(2) == t0 && Painted(3) == f0, "4. nothing changed -> 0 cells repainted");
    s2.points[0].value += 1;
    ShuttleMoveUpdate(s2);
    Check(Painted(1) - p0 == 1 && Painted(0) == e0, "4. one teach value -> exactly one cell");
    s2.enc[0].enc += 5;
    ShuttleMoveUpdate(s2);
    Check(Painted(0) - e0 == 1 && ShuttleMoveEncText(0, kSmEncValue) == L"12350", "4. an encoder moves -> one cell");

    std::printf("\n-- 5. commands\n");
    ::SendMessageW((HWND)ShuttleMoveHwnd(), WM_COMMAND, MAKEWPARAM(6100, BN_CLICKED), 0);   // kIdcCmd0 = "Shuttle1 Left"
    Pump();
    Check(ShuttleMoveIsOpen(), "5. a WM_COMMAND from a (disabled) command id does nothing");
    HWND exitBtn = 0;
    for (HWND h = ::GetWindow((HWND)ShuttleMoveHwnd(), GW_CHILD); h; h = ::GetWindow(h, GW_HWNDNEXT))
        if (::GetDlgCtrlID(h) == 6008) { exitBtn = h; break; }
    Check(exitBtn != 0, "5. the Exit button exists");
    if (exitBtn) ::SendMessageW((HWND)ShuttleMoveHwnd(), WM_COMMAND, MAKEWPARAM(6008, BN_CLICKED), (LPARAM)exitBtn);
    Pump();
    Check(!ShuttleMoveIsOpen(), "5. Exit closes this window (not golden FormClose)");

    std::printf("\n=== NativeShuttleMove_Headless: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
