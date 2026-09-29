// ===========================================================================
//  tests/test_native_motortest.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): ctest NativeMotorTest_Headless -- the HW.MotorTest native window (v1,
//  display only, ui/native/NativeMotorTest.h).  Headless: the window is created hidden, fed synthetic axes, read back
//  through the grids' own window procedures, then closed.  No machine file, no card, no motor.
//
//  ⚠ Links only ui/native/NativeHost.cpp, NativeGrid.cpp, NativeMotorView.cpp (MotorLedOn) and NativeMotorTest.cpp
//    (ui/native/NativeForms.cmake).  That it links at all proves the window has no call path to motion, IO or the card.
//    1. created hidden; every golden command BUTTON is disabled -- the only enabled one is Exit
//    2. list / detail / lamps show the data; null is "—", never 0; lamps decode motionIO like MotorView
//    3. selection is screen state: before a click it follows C++'s ActiveIndex (page.selectedMotor), a click moves it
//    4. no-change update -> 0 cells repainted in all three grids; one value -> exactly one cell (Steven: no flicker)
//    5. a WM_COMMAND from a disabled command id does nothing; Exit closes only this window
//    6. cost: 60 axes all moving, 1000 updates (budget 20 ms)
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"
#include "ui/native/NativeMotorTest.h"
#include "ui/native/NativeMotorView.h"

#include <cstdio>
#include <cstring>
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

MotorTestAxis Axis(int i, bool live)
{
    MotorTestAxis a;
    char alias[32], no[8];
    std::snprintf(alias, sizeof(alias), "MTest%02d", i);
    std::snprintf(no, sizeof(no), "M%02d", i);
    a.row = i;
    a.alias = alias;
    a.no = no;
    a.cardModel = "PCI1203";
    a.boardId = i;
    a.port = 0;
    if (live) {
        a.hasCmd = true; a.cmd = 1000 + i;
        a.hasEnc = true; a.enc = 1002 + i;
        a.hasSpeed = true; a.speed = 50;
        a.hasHomeOffset = true; a.homeOffset = -12;
        a.servoOn = 1; a.alarm = 0; a.busy = 0; a.inPos = 1; a.homeFlag = 1;
        a.ledKnown = true; a.motionIO = 0x00004000ul;   // SVON
        a.loopJob = 0; a.homeJob = 0; a.hasLoopCount = true; a.loopCount = 7;
        a.hasParams = true; a.initSpeed = 100; a.jogHigh = 5000; a.jogLow = 500; a.homeHigh = 3000; a.homeLow = 300;
        a.softP = 90000; a.softN = -1000; a.acc = 0.25; a.dec = 0.5; a.range = 20;
        a.quality = "good"; a.source = "pci1203-monitor";
    } else {
        a.quality = "nosource"; a.source = "none"; a.errText = "no value";
    }
    return a;
}

unsigned long Painted(void* grid) { return GridGetStats((HWND)grid).cellsPainted; }

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("=== NativeMotorTest_Headless (ui/native/NativeMotorTest, v1 display only) ===\n");

    std::printf("\n-- 1. window, buttons\n");
    Check(MotorTestOpen(false), "1. the window is created (hidden)");
    Pump();
    std::printf("       buttons: %d, enabled: %d\n", MotorTestButtonCount(), MotorTestEnabledButtonCount());
    Check(MotorTestButtonCount() >= 20, "1. golden's command buttons are shown");
    Check(MotorTestEnabledButtonCount() == 1, "1. only one button is enabled (Exit): every command is disabled");

    std::printf("\n-- 2. data\n");
    std::vector<MotorTestAxis> ax;
    ax.push_back(Axis(0, true));
    ax.push_back(Axis(1, true));
    ax.push_back(Axis(2, false));
    ax[1].motionIO = 0x00004002ul;   // SVON + ALM
    ax[1].alarm = 1;
    MotorTestPage page;              // known = false: no page source hooked yet
    MotorTestSummary sum;
    sum.buildConfig = "TEST";
    sum.pageWhy = "page-wide state not hooked (test)";
    MotorTestUpdate(ax, page, sum);
    Pump();
    Check(MotorTestListCount() == 3, "2. three motors listed");
    Check(MotorTestListText(0, kMtColCmd) == L"1000" && MotorTestListText(0, kMtColEnc) == L"1002", "2. list: command / encoder position");
    Check(MotorTestListText(2, kMtColCmd) == L"—" && MotorTestListText(2, kMtColServo) == L"—", "2. list: no value is —, not 0");
    Check(MotorTestSelected() == 0, "2. before any click and without ActiveIndex: row 0");
    Check(MotorTestDetailText(kMtRowCmd) == L"1000" && MotorTestDetailText(kMtRowEnc) == L"1002", "2. detail: positions of the selected motor");
    Check(MotorTestDetailText(kMtRowHomeOffset) == L"-12" && MotorTestDetailText(kMtRowSpeed) == L"50", "2. detail: home offset, real speed");
    Check(MotorTestDetailText(kMtRowJogHigh) == L"5000" && MotorTestDetailText(kMtRowSoftN) == L"-1000" &&
              MotorTestDetailText(kMtRowAcc) == L"0.250", "2. detail: strngrdMotor parameters");
    Check(MotorTestDetailText(kMtRowPower) == L"—" && MotorTestDetailText(kMtRowJogPTime) == L"—",
          "2. detail: page-wide rows are — while no page source is hooked");
    Check(MotorTestLampText(kLedServo) == L"1" && MotorTestLampText(kLedAlarm) == L"0", "2. lamps: SVON on, ALM off (motionIO decode)");

    std::printf("\n-- 3. selection\n");
    page.known = true;
    page.selectedMotor = "MTest01";   // C++'s ActiveIndex (the web page picked M01)
    page.hasPower = true; page.relayOn = true; page.motorPowerState = true;
    MotorTestUpdate(ax, page, sum);
    Check(MotorTestSelected() == 1, "3. no click yet: follows C++'s ActiveIndex");
    Check(MotorTestLampText(kLedAlarm) == L"1" && MotorTestDetailText(kMtRowAlarm) == L"警報", "3. detail / lamps follow it (M01 in alarm)");
    Check(MotorTestDetailText(kMtRowPower).find(L"Relay ON") == 0, "3. page row: Motor Power relay state");
    MotorTestSelect(2);
    MotorTestUpdate(ax, page, sum);
    Check(MotorTestSelected() == 2, "3. a click selects row 2 (screen state only)");
    Check(MotorTestDetailText(kMtRowCmd) == L"—" && MotorTestLampText(kLedServo) == L"—", "3. the nosource motor: — and white lamps");
    MotorTestSelect(0);
    MotorTestUpdate(ax, page, sum);
    Check(MotorTestSelected() == 0, "3. a click back to row 0");

    std::printf("\n-- 4. only changed cells\n");
    unsigned long l0 = Painted(MotorTestListGrid()), d0 = Painted(MotorTestDetailGrid()), m0 = Painted(MotorTestLampGrid());
    MotorTestUpdate(ax, page, sum);
    Check(Painted(MotorTestListGrid()) == l0 && Painted(MotorTestDetailGrid()) == d0 && Painted(MotorTestLampGrid()) == m0,
          "4. nothing changed -> 0 cells repainted (list, detail, lamps)");
    ax[1].cmd += 5;   // not the selected motor
    MotorTestUpdate(ax, page, sum);
    Check(Painted(MotorTestListGrid()) - l0 == 1 && Painted(MotorTestDetailGrid()) == d0, "4. another motor moves -> one list cell, detail untouched");
    l0 = Painted(MotorTestListGrid());
    ax[0].cmd += 5;   // the selected motor
    MotorTestUpdate(ax, page, sum);
    Check(Painted(MotorTestListGrid()) - l0 == 1 && Painted(MotorTestDetailGrid()) - d0 == 1 && MotorTestDetailText(kMtRowCmd) == L"1005",
          "4. the selected motor moves -> one list cell + one detail cell");

    std::printf("\n-- 5. commands\n");
    ::SendMessageW((HWND)MotorTestHwnd(), WM_COMMAND, MAKEWPARAM(3100, BN_CLICKED), 0);   // kIdcCmd0 = "Jog -"
    Pump();
    Check(MotorTestIsOpen(), "5. a WM_COMMAND from a (disabled) command id does nothing");

    std::printf("\n-- 6. cost\n");
    std::vector<MotorTestAxis> many;
    for (int i = 0; i < 60; ++i) many.push_back(Axis(i, true));
    MotorTestUpdate(many, page, sum);
    double total = 0, worst = 0;
    const int kRuns = 1000;
    for (int k = 0; k < kRuns; ++k) {
        for (std::size_t i = 0; i < many.size(); ++i) { many[i].cmd += 13; many[i].enc += 13; }
        MotorTestUpdate(many, page, sum);
        total += MotorTestLastUpdateMs();
        if (MotorTestLastUpdateMs() > worst) worst = MotorTestLastUpdateMs();
    }
    std::printf("       %d updates, 60 axes moving: avg %.3f ms, worst %.3f ms\n", kRuns, total / kRuns, worst);
    Check(total / kRuns < 3.0, "6. average update < 3 ms (budget 20 ms)");

    std::printf("\n-- 5b. Exit\n");
    HWND exitBtn = 0;
    {
        HWND h = ::GetWindow((HWND)MotorTestHwnd(), GW_CHILD);
        for (; h; h = ::GetWindow(h, GW_HWNDNEXT))
            if (::GetDlgCtrlID(h) == 3007) { exitBtn = h; break; }
    }
    Check(exitBtn != 0, "5b. the Exit button exists");
    // what the Exit button sends its parent (BM_CLICK is unreliable on a hidden, inactive window)
    if (exitBtn) ::SendMessageW((HWND)MotorTestHwnd(), WM_COMMAND, MAKEWPARAM(3007, BN_CLICKED), (LPARAM)exitBtn);
    Pump();
    Check(!MotorTestIsOpen(), "5b. Exit closes this window (not golden FormClose)");

    std::printf("\n=== NativeMotorTest_Headless: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
