// ===========================================================================
//  tests/test_native_teach.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): ctest NativeTeach_Headless -- the HW.teach native window (v1,
//  a table, display only, ui/native/NativeTeach.h).  Headless: the window is created hidden, fed synthetic teach points
//  and axes, read back through the grids' own window procedures, then closed.  No machine file, no card, no motor.
//
//  ⚠ Links only ui/native/NativeHost.cpp, NativeGrid.cpp, NativeMotorView.cpp (MotorLedOn), NativeTeach.cpp and the
//    other window files of _nf_ui (ui/native/NativeForms.cmake).  That it links at all proves the window has no call
//    path to motion, IO, teach.ini or the card.
//    1. created hidden; every golden command BUTTON is disabled -- the only enabled one is Exit
//    2. 260 TECH_PARA + 52 TECH_TWOPARA synthetic rows (260 = the live count); null is "—", never 0; Set / Go are drawn cells
//    3. the tab filter (screen state only): by tab, nested tabs by their top tab, the "(no tab)" item, back to all
//    4. selection is screen state: before a click the panel follows C++'s ActiveMotorIndex; a click moves it; a
//       two-axis row shows both NumberAlias (golden Panel2) and the second axis
//    5. no-change update -> 0 cells repainted in every grid; one axis moving -> exactly the cells of the visible rows
//       on that motor (+ the panel's Now cell when it is the panel axis); one Tech value -> one cell
//    6. a WM_COMMAND from a disabled command id does nothing; Exit closes only this window
//    7. cost: 260 + 52 rows, 60 motors all moving, 1000 updates (budget 20 ms)
//    8. the Key -> golden tab table (generated from the dfm2rc IR) answers known keys
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"
#include "ui/native/NativeMotorView.h"
#include "ui/native/NativeTeach.h"

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

const int kOne = 260, kTwo = 52, kAxes = 60, kMot0 = 100;

// the synthetic registry: P row i uses motor kMot0 + i % 60 (row 3: no motor); T row j uses kMot0 + j % 60 and
// kMot0 + (j + 30) % 60
int MotorOf(int point, int slot)
{
    if (point < kOne) {
        if (slot != 0 || point == 3) return -1;
        return kMot0 + point % kAxes;
    }
    const int j = point - kOne;
    return kMot0 + (slot == 0 ? j % kAxes : (j + 30) % kAxes);
}

std::string Str(const char* fmt, int v)
{
    char b[64];
    std::snprintf(b, sizeof(b), fmt, v);
    return b;
}

std::vector<TeachAxis> Axes()
{
    std::vector<TeachAxis> v;
    for (int i = 0; i < kAxes; ++i) {
        TeachAxis a;
        a.motIndex = kMot0 + i;
        a.alias = Str("MAx%02d", i);
        a.numberAlias = Str("M%03d", kMot0 + i);
        if (i == kAxes - 1) {   // no source: every value null
            a.quality = "nosource"; a.source = "none"; a.errText = "no value";
        } else {
            a.motorType = 1;   // golden: the encoder column reads ReadEncoderPos (it does not follow Now)
            a.hasNow = true; a.now = 1000 + i;
            a.hasEnc = true; a.enc = 2000 + i;
            a.hasHomeOffset = true; a.homeOffset = -12;
            a.servoOn = 1; a.alarm = 0; a.busy = 0; a.inPos = 1; a.homeFlag = 1;
            a.ledKnown = true; a.motionIO = 0x00004000ul;   // SVON
            a.quality = "good"; a.source = "pci1203-monitor";
        }
        v.push_back(a);
    }
    return v;
}

std::vector<TeachPoint> Points(const std::vector<TeachAxis>& ax)
{
    std::vector<TeachPoint> v;
    for (int i = 0; i < kOne; ++i) {
        TeachPoint p;
        p.index = i;
        p.two = false;
        p.key[0] = Str("setEditP%03d", i);
        p.motIndex[0] = MotorOf(i, 0);
        p.alias[0] = p.motIndex[0] < 0 ? std::string() : ax[(std::size_t)(p.motIndex[0] - kMot0)].alias;
        if (i % 50 == 49) p.tab = "";                              // 5 rows with no tab: 49, 99, 149, 199, 249
        else if (i >= 20 && i < 40) p.tab = "Tab01 > Inner";       // a nested tab: filtered by its top tab
        else p.tab = Str("Tab%02d", i / 20);
        p.group = "Group";
        p.label = Str("Label %d", i);
        p.hasValue[0] = (i != 7);                                  // row 7: a NULL Parameter
        p.value[0] = 10 * i;
        v.push_back(p);
    }
    for (int j = 0; j < kTwo; ++j) {
        TeachPoint p;
        p.index = j;
        p.two = true;
        p.key[0] = Str("setEditT%02dX", j);
        p.key[1] = Str("setEditT%02dY", j);
        for (int k = 0; k < 2; ++k) {
            p.motIndex[k] = MotorOf(kOne + j, k);
            p.alias[k] = ax[(std::size_t)(p.motIndex[k] - kMot0)].alias;
            p.hasValue[k] = true;
            p.value[k] = (k == 0 ? 5000 : 6000) + j;
        }
        p.tab = "TwoTab";
        v.push_back(p);
    }
    return v;
}

unsigned long Painted(void* grid) { return GridGetStats((HWND)grid).cellsPainted; }

// how many "current position" cells of the visible rows use this motor (the cells one axis move must repaint)
int VisibleSlotsOn(int mot)
{
    HWND list = (HWND)TeachListGrid();
    const int top = GridTopRow(list), vis = GridVisibleRows(list), rows = TeachRowCount();
    int n = 0;
    for (int r = top; r < top + vis && r < rows; ++r) {
        const int p = TeachRowPoint(r);
        for (int k = 0; k < 2; ++k)
            if (MotorOf(p, k) == mot) ++n;
    }
    return n;
}

int FindFilterItem(const wchar_t* prefix)
{
    for (int i = 0; i < TeachFilterCount(); ++i)
        if (TeachFilterText(i).find(prefix) == 0) return i;
    return -1;
}

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("=== NativeTeach_Headless (ui/native/NativeTeach, v1 display only) ===\n");

    std::printf("\n-- 1. window, buttons\n");
    Check(TeachOpen(false), "1. the window is created (hidden)");
    Pump();
    std::printf("       buttons: %d, enabled: %d\n", TeachButtonCount(), TeachEnabledButtonCount());
    Check(TeachButtonCount() >= 13, "1. golden's motion-panel commands are shown");
    Check(TeachEnabledButtonCount() == 1, "1. only one button is enabled (Exit): every command is disabled");
    Check(TeachNoteText().find(L"FormShow") != std::wstring::npos && TeachNoteText().find(L"gbTopBtmAOI") != std::wstring::npos,
          "1. the note says FormShow visibility is not applied and the Top&Bottom AOI edits are not rows");

    std::printf("\n-- 2. data\n");
    std::vector<TeachAxis> ax = Axes();
    std::vector<TeachPoint> pt = Points(ax);
    TeachPage page;                    // known = false: no page source
    TeachSummary sum;
    sum.buildConfig = "TEST";
    sum.techParaCount = kOne;
    sum.twoParaCount = kTwo;
    TeachUpdate(pt, ax, page, sum);
    Pump();
    Check(TeachRowCount() == kOne + kTwo, "2. 260 + 52 rows listed (all, no FormShow visibility in v1)");
    Check(TeachCellText(0, kTcColNo) == L"P0" && TeachCellText(kOne, kTcColNo) == L"T0", "2. P rows first, then T rows");
    Check(TeachCellText(1, kTcColValue) == L"10" && TeachCellText(1, kTcColPos) == L"1001", "2. Tech value and the motor's current position");
    Check(TeachCellText(1, kTcColMotor) == L"MAx01" && TeachCellText(1, kTcColKey) == L"setEditP001", "2. motor alias and Key");
    Check(TeachCellText(1, kTcColIni) == L"[MAx01] setEditP001", "2. teach.ini [Alias] Key");
    Check(TeachCellText(7, kTcColValue) == L"—", "2. a NULL Parameter is —, not 0");
    Check(TeachCellText(3, kTcColPos) == L"—" && TeachCellText(3, kTcColMotor) == L"（空）", "2. a row with no motor: —");
    Check(TeachCellText(59, kTcColPos) == L"—", "2. a motor with no source: —, not 0");
    Check(TeachCellText(0, kTcColSet) == L"Set" && TeachCellText(0, kTcColGo) == L"Go", "2. Set / Go cells (drawn, disabled)");
    Check(TeachCellText(1, kTcColKey2).empty() && TeachCellText(1, kTcColPos2).empty(), "2. a one-axis row has no second axis");
    Check(TeachCellText(kOne, kTcColKey2) == L"setEditT00Y" && TeachCellText(kOne, kTcColValue2) == L"6000" &&
              TeachCellText(kOne, kTcColPos2) == L"1030", "2. a two-axis row: both axes");
    Check(TeachCellText(49, kTcColTab) == L"（沒有對到頁籤）", "2. a row whose Key has no golden tab says so");
    Check(TeachSelectedPoint() == -1 && TeachPanelMotor() == -1 && TeachDetailText(kTdRowNow) == L"—",
          "2. golden ActiveMotorIndex=-1: no axis in the panel before any pick");
    Check(TeachLampText(kLedServo) == L"—", "2. lamps are null (white), not off, without an axis");
    Check(TeachDetailText(kTdRowLock).find(L"—") == 0, "2. Lock is — (no page-state accessor)");

    std::printf("\n-- 3. filter\n");
    std::printf("       filter items: %d (first %ls, last %ls)\n", TeachFilterCount(), TeachFilterText(0).c_str(),
                TeachFilterText(TeachFilterCount() - 1).c_str());
    Check(TeachFilterCount() == 1 + 14 + 1 + 1, "3. items: all + Tab00..Tab13 + TwoTab + (no tab)");
    Check(TeachSetFilter("Tab01") && TeachRowCount() == 20 && TeachRowPoint(0) == 20 && TeachCellText(0, kTcColNo) == L"P20",
          "3. a tab: only its rows (a nested tab counts under its top tab)");
    Check(TeachSetFilter("TwoTab") && TeachRowCount() == kTwo && TeachCellText(0, kTcColNo) == L"T0", "3. the two-axis tab");
    const int noTab = FindFilterItem(L"（沒有對到頁籤）");
    Check(noTab > 0 && TeachSetFilterItem(noTab) && TeachRowCount() == 5 && TeachRowPoint(0) == 49, "3. the (no tab) item");
    Check(!TeachSetFilter("NoSuchTab"), "3. a tab that is not offered is refused");
    Check(TeachSetFilter("") && TeachRowCount() == kOne + kTwo, "3. back to all");

    std::printf("\n-- 4. selection\n");
    {
        const TeachKeyInfo* h = TeachFindKey("setInPickX");
        const TeachKeyInfo* v = TeachFindKey("SetEditAuto1Front");
        Check(h && h->goldenHidden && v && !v->goldenHidden,
              "4. the key table marks the rows golden builds with Visible=false (setInPickX), not the others");
    }
    page.known = true;
    page.activeMotor = kMot0 + 5;      // C++'s ActiveMotorIndex (the web page's teach axis)
    TeachUpdate(pt, ax, page, sum);
    Check(TeachDetailText(kTdRowSetToOffset).find(L"golden") != std::wstring::npos,
          "4. edtSetToOffset empty until a teach-home (golden V912 Timer1 :1399 / :1406), not LastHomePos");
    page.setToOffset = "-12";          // golden edtSetToOffset->Text after a home from teach
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == -1 && TeachPanelMotor() == kMot0 + 5 && TeachDetailText(kTdRowNow) == L"1005",
          "4. no click yet: the panel follows C++'s ActiveMotorIndex");
    Check(TeachDetailText(kTdRowPanel2) == L"M105" && TeachDetailText(kTdRowSetToOffset) == L"-12" &&
              TeachDetailText(kTdRowEnc) == L"2005", "4. Panel2 = NumberAlias, edtSetToOffset = golden's edit text, encoder");
    Check(TeachLampText(kLedServo) == L"1" && TeachLampText(kLedAlarm) == L"0", "4. lamps: SVON on, ALM off (motionIO decode)");
    TeachSelect(2);
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == 2 && TeachPanelMotor() == kMot0 + 2 && TeachDetailText(kTdRowNow) == L"1002",
          "4. a click selects row 2 and the panel follows it (screen state only)");
    Check(TeachDetailText(kTdRowValue) == L"20" && TeachDetailText(kTdRowIni) == L"[MAx02] setEditP002",
          "4. the panel shows the row's Tech value and teach.ini key");
    TeachSelect(3);
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == 3 && TeachPanelMotor() == -1 && TeachLampText(kLedServo) == L"—", "4. a row with no motor: empty panel");
    TeachSetFilter("TwoTab");
    TeachSelect(0);
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == kOne && TeachPanelMotor() == kMot0 && TeachDetailText(kTdRowPanel2) == L"M100 M130" &&
              TeachDetailText(kTdRowNow2) == L"1030", "4. a two-axis row: Panel2 'X Y' (golden :1313) and the second axis");
    TeachSetFilter("");
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == kOne, "4. a filter change keeps the panel's row (the grid highlight is cleared)");
    TeachSelect(1);
    TeachUpdate(pt, ax, page, sum);
    Check(TeachSelectedPoint() == 1 && TeachPanelMotor() == kMot0 + 1, "4. a click on row 1");

    std::printf("\n-- 5. only changed cells\n");
    unsigned long l0 = Painted(TeachListGrid()), d0 = Painted(TeachDetailGrid()), m0 = Painted(TeachLampGrid());
    TeachUpdate(pt, ax, page, sum);
    Check(Painted(TeachListGrid()) == l0 && Painted(TeachDetailGrid()) == d0 && Painted(TeachLampGrid()) == m0,
          "5. nothing changed -> 0 cells repainted (table, panel, lamps)");
    {
        const int mot = kMot0 + 10;   // not the panel axis
        const int want = VisibleSlotsOn(mot);
        ax[10].now += 5;
        TeachUpdate(pt, ax, page, sum);
        std::printf("       motor %d moved: table %lu cells (visible rows on it: %d), panel %lu\n", mot, Painted(TeachListGrid()) - l0,
                    want, Painted(TeachDetailGrid()) - d0);
        Check(want >= 1 && Painted(TeachListGrid()) - l0 == (unsigned long)want && Painted(TeachDetailGrid()) == d0,
              "5. another motor moves -> exactly its visible rows' cells, panel untouched");
    }
    l0 = Painted(TeachListGrid());
    {
        const int mot = kMot0 + 1;    // the panel axis
        const int want = VisibleSlotsOn(mot);
        ax[1].now += 5;
        TeachUpdate(pt, ax, page, sum);
        std::printf("       panel motor %d moved: table %lu cells (visible rows on it: %d), panel %lu, lamps %lu\n", mot,
                    Painted(TeachListGrid()) - l0, want, Painted(TeachDetailGrid()) - d0, Painted(TeachLampGrid()) - m0);
        Check(want >= 1 && Painted(TeachListGrid()) - l0 == (unsigned long)want && Painted(TeachDetailGrid()) - d0 == 1 &&
                  Painted(TeachLampGrid()) == m0 && TeachDetailText(kTdRowNow) == L"1006",
              "5. the panel axis moves -> its visible rows' cells + the panel's Now cell, lamps untouched");
    }
    l0 = Painted(TeachListGrid());
    d0 = Painted(TeachDetailGrid());
    pt[4].value[0] += 1;   // a Tech value changes (not the selected row)
    TeachUpdate(pt, ax, page, sum);
    Check(Painted(TeachListGrid()) - l0 == 1 && Painted(TeachDetailGrid()) == d0 && TeachCellText(4, kTcColValue) == L"41",
          "5. one Tech value -> one cell");
    l0 = Painted(TeachListGrid());
    ax[1].motionIO = 0x00004002ul;   // the panel axis: ALM lamp on
    TeachUpdate(pt, ax, page, sum);
    Check(Painted(TeachLampGrid()) - m0 == 1 && Painted(TeachListGrid()) == l0 && TeachLampText(kLedAlarm) == L"1",
          "5. one lamp -> one lamp cell, table untouched");

    std::printf("\n-- 6. commands\n");
    ::SendMessageW((HWND)TeachHwnd(), WM_COMMAND, MAKEWPARAM(5100, BN_CLICKED), 0);   // kIdcCmd0 = "Move +"
    ::SendMessageW((HWND)TeachHwnd(), WM_COMMAND, MAKEWPARAM(5109, BN_CLICKED), 0);   // "STOP"
    Pump();
    Check(TeachIsOpen(), "6. a WM_COMMAND from a (disabled) command id does nothing");

    std::printf("\n-- 7. cost\n");
    double total = 0, worst = 0;
    const int kRuns = 1000;
    for (int k = 0; k < kRuns; ++k) {
        for (std::size_t i = 0; i < ax.size(); ++i) { ax[i].now += 13; ax[i].enc += 13; }   // the no-source one stays —
        TeachUpdate(pt, ax, page, sum);
        total += TeachLastUpdateMs();
        if (TeachLastUpdateMs() > worst) worst = TeachLastUpdateMs();
    }
    std::printf("       %d updates, %d + %d rows, %d motors moving: avg %.3f ms, worst %.3f ms\n", kRuns, kOne, kTwo, kAxes,
                total / kRuns, worst);
    Check(total / kRuns < 3.0, "7. average update < 3 ms (budget 20 ms)");

    std::printf("\n-- 8. the golden Key -> tab table\n");
    const TeachKeyInfo* z = TeachFindKey("setEditInZSafeHeight");
    std::printf("       %d keys; setEditInZSafeHeight -> %s\n", TeachKeyTableSize(), z ? z->tab : "(none)");
    Check(TeachKeyTableSize() > 300, "8. the table has the registry's keys");
    Check(z != 0 && std::strcmp(z->tab, "Input Arm") == 0, "8. setEditInZSafeHeight is on golden's Input Arm tab");
    Check(TeachFindKey("setEditInXPitch40") != 0 && TeachFindKey("noSuchKey") == 0, "8. found / not found");
    {
        int found = 0, withTab = 0;
        for (int i = 0; i < TeachKeyTableSize(); ++i) {
            const TeachKeyInfo* e = TeachKeyAt(i);
            if (e && TeachFindKey(e->key) == e) ++found;
            if (e && e->tab[0] != 0) ++withTab;
        }
        Check(found == TeachKeyTableSize(), "8. every key is found by the binary search (the table is sorted)");
        Check(withTab == TeachKeyTableSize(), "8. every registry key has a golden tab");
    }

    std::printf("\n-- 6b. Exit\n");
    HWND exitBtn = 0;
    {
        HWND h = ::GetWindow((HWND)TeachHwnd(), GW_CHILD);
        for (; h; h = ::GetWindow(h, GW_HWNDNEXT))
            if (::GetDlgCtrlID(h) == 5007) { exitBtn = h; break; }
    }
    Check(exitBtn != 0, "6b. the Exit button exists");
    // what the Exit button sends its parent (BM_CLICK is unreliable on a hidden, inactive window)
    if (exitBtn) ::SendMessageW((HWND)TeachHwnd(), WM_COMMAND, MAKEWPARAM(5007, BN_CLICKED), (LPARAM)exitBtn);
    Pump();
    Check(!TeachIsOpen(), "6b. Exit closes this window (not golden FormClose)");

    std::printf("\n=== NativeTeach_Headless: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
