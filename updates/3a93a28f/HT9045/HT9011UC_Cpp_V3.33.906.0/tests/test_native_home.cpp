// ===========================================================================
//  tests/test_native_home.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): ctest NativeHome_Headless -- the HW.home native window (v1,
//  display only, ui/native/NativeHome.h).  Headless: the window is created hidden, fed synthetic THomeClass rows and
//  ListBox1 lines, read back through the grids' own window procedures, then closed.  No machine file, no card, no motor.
//
//  ⚠ Links only the ui/native window files (ui/native/NativeForms.cmake).  That it links at all proves the window has no
//    call path to motion, IO, the card, or fHome (no Close() -> fShow=false, no sbAbortHomeClick, no ShowMotorHomePos).
//    1. created hidden; two buttons (Abort Home, Exit) -- the only enabled one is Exit
//    2. rows / log / status show the data; null is "—", never 0; the lamp column is "—" (the port's ShowLed is a GATE)
//    3. golden layout loop: motor 15 of the shown ones is row 0 of the second column group; 15 grid rows at most
//    4. no-change update -> 0 cells repainted in both grids; one position -> exactly one cell; one log line -> one cell
//    5. a WM_COMMAND from the disabled sbAbortHome id does nothing; Exit closes only this window
//    6. cost: 60 motors all moving + a 200-line log, 1000 updates (budget 20 ms)
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"
#include "ui/native/NativeHome.h"

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

HomeRowData Row(int i, bool hasPos)
{
    HomeRowData r;
    char name[48], pos[16];
    std::snprintf(name, sizeof(name), "[%02d] HomeAxis%02d", i, i);   // golden NumberAlias "[%02d] %s"
    std::snprintf(pos, sizeof(pos), "%d", 100 + i);
    r.index = i;
    r.name = name;
    r.hasPos = hasPos;
    if (hasPos) r.pos = pos;
    r.lamp = -1;   // what the glue sends while the port's ShowLed is a GATE no-op
    return r;
}

unsigned long Painted(void* grid) { return GridGetStats((HWND)grid).cellsPainted; }

HWND ChildById(int id)
{
    HWND h = ::GetWindow((HWND)HomeHwnd(), GW_CHILD);
    for (; h; h = ::GetWindow(h, GW_HWNDNEXT))
        if (::GetDlgCtrlID(h) == id) return h;
    return 0;
}

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("=== NativeHome_Headless (ui/native/NativeHome, v1 display only) ===\n");

    std::printf("\n-- 1. window, buttons\n");
    Check(HomeOpen(false), "1. the window is created");
    Pump();
    Check(HomeIsOpen() && !::IsWindowVisible((HWND)HomeHwnd()), "1. ... hidden");
    std::printf("       buttons: %d, enabled: %d\n", HomeButtonCount(), HomeEnabledButtonCount());
    Check(HomeButtonCount() == 2, "1. golden's two buttons are shown (Abort Home, Exit)");
    Check(HomeEnabledButtonCount() == 1, "1. only one button is enabled (Exit): Abort Home is disabled");
    HWND abortBtn = ChildById(4007), exitBtn = ChildById(4008);
    Check(abortBtn != 0 && !::IsWindowEnabled(abortBtn), "1. the Abort Home button (4007) is WS_DISABLED");
    Check(exitBtn != 0 && ::IsWindowEnabled(exitBtn), "1. the Exit button (4008) is enabled");

    std::printf("\n-- 2. data\n");
    std::vector<HomeRowData> rows;
    rows.push_back(Row(0, true));
    rows.push_back(Row(3, true));
    rows.push_back(Row(11, false));   // no edPos object
    rows[1].pos = "-12";
    std::vector<std::string> log;
    log.push_back("M 4 homeing ....");   // golden SetHomeSerial: Insert(0, ...) -> newest first
    log.push_back("M01 home finish.");
    HomePage page;
    page.known = true;
    page.fShow = true;
    page.homeStep = 600;
    page.resetOk = -1;                   // the port has no Panel2
    HomeSummary sum;
    sum.buildConfig = "TEST";
    sum.classCount = 164;
    sum.hiddenCount = 161;
    sum.lampWhy = "ShowLed is a GATE no-op (test)";
    sum.resetWhy = "no Panel2 (test)";
    HomeUpdate(rows, log, page, sum);
    Pump();
    Check(HomeRowCount() == 3, "2. three motors shown");
    Check(HomeRowText(0, kHmFieldName) == L"[00] HomeAxis00" && HomeRowText(1, kHmFieldName) == L"[03] HomeAxis03",
          "2. name = labName caption (golden NumberAlias), golden order");
    Check(HomeRowText(0, kHmFieldPos) == L"100" && HomeRowText(1, kHmFieldPos) == L"-12", "2. position = edPos text");
    Check(HomeRowText(2, kHmFieldPos) == L"—", "2. no edPos -> —, not 0");
    Check(HomeRowText(0, kHmFieldLamp) == L"—" && HomeLampText(0) == L"—" && HomeLampText(2) == L"—",
          "2. lamp column is — (null lamp) while ShowLed has no source");
    Check(HomeLogCount() == 2 && HomeLogText(0) == L"M 4 homeing ...." && HomeLogText(1) == L"M01 home finish.",
          "2. log: ListBox1 lines, newest first");
    const std::wstring st = HomeStatusText();
    Check(st.find(L"Reset OK") != std::wstring::npos && st.find(L"600") != std::wstring::npos &&
              st.find(L"no Panel2 (test)") != std::wstring::npos,
          "2. status line: Reset OK is — with the reason, iHomeStep shown");
    Check(HomeSummaryText().find(L"TEST") != std::wstring::npos, "2. summary shows the build config");
    rows[1].lamp = 1;                    // what the glue sends once ShowLed stores its state (the claim)
    HomeUpdate(rows, log, page, sum);
    Check(HomeLampText(1) == L"1" && HomeRowText(1, kHmFieldLamp) == L"完成", "2. a known lamp state (1 = golden clLime, home finish)");
    rows[1].lamp = -1;
    HomeUpdate(rows, log, page, sum);
    page.known = false;
    HomeUpdate(rows, log, page, sum);
    Check(HomeStatusText().find(L"fHome") != std::wstring::npos && HomeStatusText().find(L"600") == std::wstring::npos,
          "2. no fHome -> status says so and shows no iHomeStep");
    page.known = true;
    HomeUpdate(rows, log, page, sum);

    std::printf("\n-- 3. golden layout (15 rows per column)\n");
    std::vector<HomeRowData> twenty;
    for (int i = 0; i < 20; ++i) twenty.push_back(Row(i, true));
    HomeUpdate(twenty, log, page, sum);
    Pump();
    Check(HomeRowCount() == 20 && GridRowCount((HWND)HomeRowGrid()) == kHomeMaxRowItem, "3. 20 motors -> 15 grid rows");
    Check(HomeRowText(15, kHmFieldName) == L"[15] HomeAxis15" &&
              GridCellText((HWND)HomeRowGrid(), 0, kHmFieldCount + kHmFieldName) == L"[15] HomeAxis15",
          "3. motor 15 is row 0 of the second column group (golden RowItem wraps at iMaxRowItem)");
    Check(GridCellText((HWND)HomeRowGrid(), 5, kHmFieldCount + kHmFieldName).empty(), "3. past the last motor: a blank cell");
    HomeUpdate(rows, log, page, sum);   // back to three
    Check(HomeRowCount() == 3 && GridRowCount((HWND)HomeRowGrid()) == 3, "3. back to 3 motors -> 3 grid rows");

    std::printf("\n-- 4. only changed cells\n");
    HomeUpdate(rows, log, page, sum);
    unsigned long r0 = Painted(HomeRowGrid()), l0 = Painted(HomeLogGrid());
    HomeUpdate(rows, log, page, sum);
    Check(Painted(HomeRowGrid()) == r0 && Painted(HomeLogGrid()) == l0, "4. nothing changed -> 0 cells repainted (rows, log)");
    rows[0].pos = "105";
    HomeUpdate(rows, log, page, sum);
    Check(Painted(HomeRowGrid()) - r0 == 1 && Painted(HomeLogGrid()) == l0 && HomeRowText(0, kHmFieldPos) == L"105",
          "4. one position changes -> exactly one cell, log untouched");
    r0 = Painted(HomeRowGrid());
    log[1] = "M01 home finish!";
    HomeUpdate(rows, log, page, sum);
    Check(Painted(HomeLogGrid()) - l0 == 1 && Painted(HomeRowGrid()) == r0 && HomeLogText(1) == L"M01 home finish!",
          "4. one log line changes -> exactly one cell, rows untouched");
    const unsigned long full0 = GridGetStats((HWND)HomeLogGrid()).fullRenders;
    log.insert(log.begin(), std::string("M04 home finish."));
    HomeUpdate(rows, log, page, sum);
    Check(HomeLogCount() == 3 && HomeLogText(0) == L"M04 home finish." && HomeLogText(2) == L"M01 home finish!" &&
              GridGetStats((HWND)HomeLogGrid()).fullRenders > full0,
          "4. a new line at the top (golden Insert(0, ...)) -> row count changes, the log is re-rendered, newest first");

    std::printf("\n-- 5. commands\n");
    const int rowsBefore = HomeRowCount();
    ::SendMessageW((HWND)HomeHwnd(), WM_COMMAND, MAKEWPARAM(4007, BN_CLICKED), (LPARAM)abortBtn);   // sbAbortHome
    Pump();
    Check(HomeIsOpen() && HomeRowCount() == rowsBefore, "5. a WM_COMMAND from the disabled Abort Home id does nothing");

    std::printf("\n-- 6. cost\n");
    std::vector<HomeRowData> many;
    for (int i = 0; i < 60; ++i) many.push_back(Row(i, true));
    std::vector<std::string> longLog;
    for (int i = 0; i < 200; ++i) {
        char b[32];
        std::snprintf(b, sizeof(b), "M%02d home finish.", i % 60 + 1);
        longLog.push_back(b);
    }
    HomeUpdate(many, longLog, page, sum);
    double total = 0, worst = 0;
    const int kRuns = 1000;
    for (int k = 0; k < kRuns; ++k) {
        for (std::size_t i = 0; i < many.size(); ++i) {
            char b[16];
            std::snprintf(b, sizeof(b), "%d", 1000 + k * 13 + (int)i);
            many[i].pos = b;
        }
        HomeUpdate(many, longLog, page, sum);
        total += HomeLastUpdateMs();
        if (HomeLastUpdateMs() > worst) worst = HomeLastUpdateMs();
    }
    std::printf("       %d updates, 60 motors moving, 200-line log: avg %.3f ms, worst %.3f ms\n", kRuns, total / kRuns, worst);
    Check(total / kRuns < 5.0, "6. average update < 5 ms (budget 20 ms)");

    std::printf("\n-- 5b. Exit\n");
    Check(exitBtn != 0 && ChildById(4008) == exitBtn, "5b. the Exit button exists");
    // what the Exit button sends its parent (BM_CLICK is unreliable on a hidden, inactive window)
    if (exitBtn) ::SendMessageW((HWND)HomeHwnd(), WM_COMMAND, MAKEWPARAM(4008, BN_CLICKED), (LPARAM)exitBtn);
    Pump();
    Check(!HomeIsOpen(), "5b. Exit closes this window (not golden SpeedButton1Click / Close())");

    std::printf("\n=== NativeHome_Headless: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
