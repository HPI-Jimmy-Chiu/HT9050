// ===========================================================================
//  ui/native/NativeHome.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home ("Motor Home Monitor") native window, v1 display only.
//  See NativeHome.h.  Shape follows NativeMotorTest.cpp (St02-E): NativeGrid for both tables (no ListView -- no manifest,
//  comctl32 v5 flickers), double-buffered LabelCreate status / summary, static texts built once, 20 ms updates that only
//  mark changed rows, so an update where nothing changed repaints 0 cells (ctest NativeHome_Headless checks it).
//  The row grid is golden's layout loop (V912 uhome.cpp:350-375): motor j of the shown ones sits at row j % 15 of column
//  group j / 15, each group Name | Lamp | Pos (golden label / ledHome / edPos, pitch 270 px).
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // as the other ui/native files
#endif
#include "ui/native/NativeHome.h"
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

namespace {

const wchar_t kClassName[] = L"W906NativeHome";
const int kIdcTitle = 4001, kIdcNote = 4002, kIdcStatus = 4003, kIdcSummary = 4004, kIdcRows = 4005, kIdcLog = 4006,
          kIdcAbort = 4007, kIdcExit = 4008, kIdcLogLabel = 4009;

// one column group (golden iLPitch = 270: label x 5, ledHome x 170, edPos x 195 width 50); a little wider for the
// lamp text and for the MS JhengHei font
const int kWName = 150, kWLamp = 76, kWPos = 74;

struct ViewState {
    HWND hwnd, title, note, status, summary, rowGrid, logGrid, abortBtn, exitBtn, logLabel;
    std::vector<HomeRowData>  items;
    std::vector<std::wstring> nameText, posText;
    std::vector<std::string>  log;
    std::vector<std::wstring> logText;
    HomePage                  page;
    HomeSummary               sum;
    int                       groups;       // column groups set on the row grid; -1 = none yet
    double                    lastUpdateMs, lastT0, avgGapMs;
    ViewState() : hwnd(0), title(0), note(0), status(0), summary(0), rowGrid(0), logGrid(0), abortBtn(0), exitBtn(0),
                  logLabel(0), groups(-1), lastUpdateMs(0), lastT0(0), avgGapMs(0) {}
};

ViewState g;

double NowMs()
{
    static LARGE_INTEGER freq;
    static bool haveFreq = false;
    if (!haveFreq) { ::QueryPerformanceFrequency(&freq); haveFreq = true; }
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return freq.QuadPart ? (double)c.QuadPart * 1000.0 / (double)freq.QuadPart : 0.0;
}

int GroupsFor(std::size_t n)
{
    return n == 0 ? 1 : (int)((n + (std::size_t)kHomeMaxRowItem - 1) / (std::size_t)kHomeMaxRowItem);
}

int GridRowsFor(std::size_t n)
{
    return n < (std::size_t)kHomeMaxRowItem ? (int)n : kHomeMaxRowItem;
}

void NameText(std::wstring& out, const HomeRowData& r)
{
    if (!r.name.empty()) { GridSetAscii(out, r.name); return; }
    std::wstring idx;
    GridSetInt(idx, r.index);
    out = L"MOT[" + idx + L"]（labName 空白）";   // golden :82 caption is NumberAlias; empty = InitialHomeClass ran before SetAlias
}

// golden ledHome: FalseColor clSilver (:98); ShowLed :664-680 TrueColor lime / red / yellow + Value
void LampCell(const HomeRowData& r, GridCell& out)
{
    out.kind = kGridLedText;
    out.ledEdge = RGB(40, 40, 40);
    switch (r.lamp) {
    case 0: out.ledFill = RGB(192, 192, 192); out.text = L"未亮"; break;                              // Value=false (:677)
    case 1: out.ledFill = RGB(0, 255, 0);     out.text = L"完成"; break;                              // clLime (:670), "home finish."
    case 2: out.ledFill = RGB(255, 0, 0);     out.text = L"錯誤"; out.color = RGB(210, 0, 0); break;   // clRed (:671)
    case 3: out.ledFill = RGB(255, 255, 0);   out.text = L"歸零中"; break;                            // clYellow (:672), "homeing ...."
    default:
        // -1 = no source (null: white, not "off"); any other attr: golden keeps the last TrueColor, which is not tracked
        out.ledFill = RGB(255, 255, 255);
        out.ledEdge = RGB(150, 150, 150);
        out.color = RGB(128, 128, 128);
        if (r.lamp < 0) out.text = L"—";
        else GridSetInt(out.text, r.lamp);
        break;
    }
}

// ---- grid callbacks (asked only for rows marked changed, or on a full render) ----
void RowCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= kHomeMaxRowItem || col < 0) return;
    const int grp = col / kHmFieldCount, field = col % kHmFieldCount;
    const int item = grp * kHomeMaxRowItem + row;   // golden layout loop: RowItem++, wrap at iMaxRowItem -> ColItem++
    if (item >= (int)g.items.size()) return;       // past the last shown motor: a blank cell
    const HomeRowData& r = g.items[(std::size_t)item];
    switch (field) {
    case kHmFieldName: out.text = g.nameText[(std::size_t)item]; out.color = RGB(0, 0, 128); break;   // golden Font clNavy (:79)
    case kHmFieldLamp: LampCell(r, out); break;
    case kHmFieldPos:
        if (r.hasPos) out.text = g.posText[(std::size_t)item];
        else { out.text = L"—"; out.color = RGB(128, 128, 128); }
        break;
    default: break;
    }
}

void LogCell(void*, int row, int col, GridCell& out)
{
    if (col != 0 || row < 0 || row >= (int)g.logText.size()) return;
    out.text = g.logText[(std::size_t)row];
}

void SetGroups(int groups)
{
    if (!g.rowGrid) return;
    std::vector<GridColumn> cols((std::size_t)(groups * kHmFieldCount));
    for (int k = 0; k < groups; ++k) {
        cols[(std::size_t)(k * kHmFieldCount + kHmFieldName)] = GridColumn(L"名稱", kWName, kGridLeft);
        cols[(std::size_t)(k * kHmFieldCount + kHmFieldLamp)] = GridColumn(L"燈", kWLamp, kGridLeft);
        cols[(std::size_t)(k * kHmFieldCount + kHmFieldPos)]  = GridColumn(L"位置", kWPos, kGridRight);
    }
    GridSetColumns(g.rowGrid, cols, std::vector<int>());
    g.groups = groups;
}

HWND MakeChild(const wchar_t* cls, const wchar_t* text, DWORD style, int id, DWORD exStyle = 0)
{
    HWND h = ::CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
                               g.hwnd, (HMENU)(INT_PTR)id, ::GetModuleHandleW(0), 0);
    if (h) ::SendMessageW(h, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    return h;
}

void CreateChildren()
{
    g.title = MakeChild(L"STATIC", L"HW.home（Motor Home Monitor）— 原生 Win32 視窗（v1 只顯示，St02 20260929）",
                        SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"golden TfHome 的顯示部分：要歸零的馬達（名稱｜燈｜位置，15 列一欄，照 golden 版面），下面是歸零進度（新的在上）。"
                       L"Abort Home 照 golden 列出但停用；Exit 只關這個視窗（不是 golden 的 Close()，不會停掉歸零）。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.status = LabelCreate(g.hwnd, kIdcStatus, L"（等待第一批資料）");
    if (g.status) ::SendMessageW(g.status, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"");
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);

    g.rowGrid = GridCreate(g.hwnd, kIdcRows, &RowCell, 0);
    if (g.rowGrid) ::SendMessageW(g.rowGrid, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    SetGroups(GroupsFor(g.items.size()));
    if (g.rowGrid) GridSetRowCount(g.rowGrid, GridRowsFor(g.items.size()));

    g.logLabel = MakeChild(L"STATIC", L"歸零進度（golden ListBox1，新的在上）：", SS_LEFT | SS_NOPREFIX, kIdcLogLabel);
    g.logGrid = GridCreate(g.hwnd, kIdcLog, &LogCell, 0);
    if (g.logGrid) ::SendMessageW(g.logGrid, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> lc(1);
    lc[0] = GridColumn(L"訊息", 860, kGridLeft);
    GridSetColumns(g.logGrid, lc, std::vector<int>());
    if (g.logGrid) GridSetRowCount(g.logGrid, (int)g.logText.size());

    // golden sbAbortHome (GaliMotorServoOff + fAbort + Close, :5130-5136): shown, WS_DISABLED, no WM_COMMAND branch
    g.abortBtn = MakeChild(L"BUTTON", L"Abort Home（v1 停用）", BS_PUSHBUTTON | WS_DISABLED, kIdcAbort);
    g.exitBtn = MakeChild(L"BUTTON", L"Exit（只關這個視窗）", BS_PUSHBUTTON | WS_TABSTOP, kIdcExit);
}

void Layout()
{
    if (!g.hwnd) return;
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top, m = 8;
    ::MoveWindow(g.title,   m, 6, w - 2 * m, 22, TRUE);
    ::MoveWindow(g.note,    m, 30, w - 2 * m, 20, TRUE);
    ::MoveWindow(g.status,  m, 52, w - 2 * m, 24, TRUE);
    ::MoveWindow(g.summary, m, 78, w - 2 * m, 62, TRUE);
    const int top = 146, logH = 200, btnW = 228;
    const int y0 = (h - m - logH) > (top + 126) ? (h - m - logH) : (top + 126);
    ::MoveWindow(g.rowGrid, m, top, w - 2 * m, y0 - top - 6, TRUE);
    ::MoveWindow(g.logLabel, m, y0, 400, 20, TRUE);
    const int logW = (w - 2 * m - btnW - 8) > 300 ? (w - 2 * m - btnW - 8) : 300;
    ::MoveWindow(g.logGrid, m, y0 + 22, logW, logH - 22, TRUE);
    ::MoveWindow(g.abortBtn, w - m - btnW, y0 + 22, btnW, 70, TRUE);
    ::MoveWindow(g.exitBtn,  w - m - btnW, y0 + 22 + 80, btnW, 70, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (HostWindowMessage(hwnd, msg, wp, lp)) return 0;   // keepalive timer (NativeHost.h)
    switch (msg) {
    case WM_CREATE:
        g.hwnd = hwnd;
        HostWindowCreated(hwnd);
        CreateChildren();
        Layout();
        return 0;
    case WM_SIZE:
        Layout();
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* mm = reinterpret_cast<MINMAXINFO*>(lp);
        mm->ptMinTrackSize.x = 1000;
        mm->ptMinTrackSize.y = 620;
        return 0;
    }
    case WM_COMMAND:
        // Exit only.  sbAbortHome is WS_DISABLED and deliberately has no branch here.
        if (LOWORD(wp) == kIdcExit && HIWORD(wp) == BN_CLICKED) ::DestroyWindow(hwnd);   // NOT golden SpeedButton1Click / Close()
        return 0;
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HostWindowDestroyed(hwnd);
        break;
    case WM_NCDESTROY:
        g.hwnd = g.title = g.note = g.status = g.summary = g.rowGrid = g.logGrid = g.abortBtn = g.exitBtn = g.logLabel = 0;
        g.groups = -1;
        break;   // no PostQuitMessage: this thread is the wb_serve main loop
    default:
        break;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

bool RegisterOnce()
{
    static bool done = false;
    if (done) return true;
    WNDCLASSEXW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = ::GetModuleHandleW(0);
    wc.hCursor = ::LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    wc.hIcon = ::LoadIconW(0, (LPCWSTR)IDI_APPLICATION);
    if (!::RegisterClassExW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
    done = true;
    return true;
}

struct ButtonCount { int all, enabled; };
BOOL CALLBACK CountButtons(HWND h, LPARAM lp)
{
    ButtonCount* c = reinterpret_cast<ButtonCount*>(lp);
    wchar_t cls[32];
    if (::GetClassNameW(h, cls, 32) > 0 && ::lstrcmpiW(cls, L"Button") == 0) {
        ++c->all;
        if (::IsWindowEnabled(h)) ++c->enabled;
    }
    return TRUE;
}

bool StaticChanged(const HomeRowData& a, const HomeRowData& b)
{
    return a.index != b.index || a.name != b.name;
}

bool ValueChanged(const HomeRowData& a, const HomeRowData& b)
{
    return a.hasPos != b.hasPos || a.pos != b.pos || a.lamp != b.lamp;
}

std::wstring BuildStatus()
{
    const HomePage& p = g.page;
    if (!p.known) return L"Reset OK（Panel2／Label26）：—｜Home Monitor 沒有來源（fHome 是 NULL）";
    std::string reset;
    if (p.resetOk < 0) reset = "—" + (g.sum.resetWhy.empty() ? std::string() : "（" + g.sum.resetWhy + "）");
    else reset = p.resetOk ? "Reset OK" : "沒有顯示";
    char buf[1200];
    std::snprintf(buf, sizeof(buf), "歸零畫面 fShow：%s｜iHomeStep：%d%s｜fAbort：%s｜Reset OK（Panel2／Label26）：%s",
                  p.fShow ? "顯示中（歸零進行中）" : "沒有顯示", p.homeStep, p.homeStep == 1 ? "（閒置）" : "",
                  p.fAbort ? "是" : "否", reset.c_str());
    return Widen(buf);
}

std::wstring BuildSummary()
{
    int nLamp = 0, nNoPos = 0;
    for (std::size_t i = 0; i < g.items.size(); ++i) {
        if (g.items[i].lamp >= 0) ++nLamp;
        if (!g.items[i].hasPos) ++nNoPos;
    }
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats a = GridGetStats(g.rowGrid), b = GridGetStats(g.logGrid);
    const std::string why = (g.items.empty() && !g.sum.why.empty()) ? "｜" + g.sum.why : std::string();
    const std::string lamp = nLamp > 0 ? std::string("有來源") : ("—" + (g.sum.lampWhy.empty() ? std::string() : "（" + g.sum.lampWhy + "）"));
    char buf[1600];
    std::snprintf(buf, sizeof(buf),
                  "[%s] 顯示 %u 顆馬達（HomeClass %d 列，golden 不顯示的 %d 列；沒有 edPos %d）｜歸零進度 %u 行%s\n"
                  "燈：%s｜拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u\n"
                  "畫面效能：上次更新 %.2f ms｜實際更新間隔約 %.0f ms｜累計重畫 %lu 格（馬達 %lu／進度 %lu）",
                  g.sum.buildConfig.empty() ? "?" : g.sum.buildConfig.c_str(), (unsigned)g.items.size(), g.sum.classCount,
                  g.sum.hiddenCount, nNoPos, (unsigned)g.log.size(), why.c_str(), lamp.c_str(), g.sum.keepaliveCalls,
                  (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned)st.wMilliseconds,
                  g.lastUpdateMs, g.avgGapMs, a.cellsPainted + b.cellsPainted, a.cellsPainted, b.cellsPainted);
    return Widen(buf);
}

std::wstring LabelText(HWND h)
{
    if (!h) return L"";
    wchar_t b[1600];
    b[0] = 0;
    ::GetWindowTextW(h, b, 1600);
    return b;
}

}  // namespace

bool HomeOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — HW.home Motor Home Monitor（原生，v1 只顯示）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1240, 900,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void HomeClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool HomeIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void HomeUpdate(const std::vector<HomeRowData>& rows, const std::vector<std::string>& log, const HomePage& page,
                const HomeSummary& sum)
{
    if (!HomeIsOpen()) return;
    const double t0 = NowMs();
    if (g.lastT0 > 0) {
        const double gap = t0 - g.lastT0;
        g.avgGapMs = g.avgGapMs > 0 ? g.avgGapMs * 0.9 + gap * 0.1 : gap;
    }
    g.lastT0 = t0;
    g.sum = sum;
    g.page = page;

    // ---- motor rows: the list / names changed -> rebuild; else mark only the rows whose value changed ----
    bool structural = rows.size() != g.items.size();
    for (std::size_t i = 0; !structural && i < rows.size(); ++i)
        if (StaticChanged(rows[i], g.items[i])) structural = true;
    if (structural) {
        g.items = rows;
        g.nameText.assign(g.items.size(), std::wstring());
        g.posText.assign(g.items.size(), std::wstring());
        for (std::size_t i = 0; i < g.items.size(); ++i) {
            NameText(g.nameText[i], g.items[i]);
            GridSetAscii(g.posText[i], g.items[i].pos);
        }
        const int groups = GroupsFor(g.items.size());
        if (groups != g.groups) SetGroups(groups);
        if (g.rowGrid) GridSetRowCount(g.rowGrid, GridRowsFor(g.items.size()));   // full render (no flicker: back buffer)
    } else {
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const HomeRowData& a = rows[i];
            HomeRowData& b = g.items[i];
            if (!ValueChanged(a, b)) continue;
            if (a.pos != b.pos) { b.pos = a.pos; GridSetAscii(g.posText[i], b.pos); }
            b.hasPos = a.hasPos;
            b.lamp = a.lamp;
            if (g.rowGrid) GridMarkRow(g.rowGrid, (int)(i % (std::size_t)kHomeMaxRowItem));
        }
        if (g.rowGrid) GridFlush(g.rowGrid);   // only the changed cells of the marked rows are painted
    }

    // ---- log (newest first): a line added / cleared -> new row count (full render); same count -> changed lines only ----
    if (log.size() != g.log.size()) {
        g.log = log;
        g.logText.assign(g.log.size(), std::wstring());
        for (std::size_t k = 0; k < g.log.size(); ++k) GridSetAscii(g.logText[k], g.log[k]);
        if (g.logGrid) GridSetRowCount(g.logGrid, (int)g.log.size());
    } else {
        for (std::size_t k = 0; k < log.size(); ++k) {
            if (log[k] == g.log[k]) continue;
            g.log[k] = log[k];
            GridSetAscii(g.logText[k], g.log[k]);
            if (g.logGrid) GridMarkRow(g.logGrid, (int)k);
        }
        if (g.logGrid) GridFlush(g.logGrid);
    }

    if (g.status) LabelSetText(g.status, BuildStatus());     // same text = no repaint
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
}

void* HomeHwnd() { return g.hwnd; }

int HomeRowCount() { return g.rowGrid ? (int)g.items.size() : -1; }

std::wstring HomeRowText(int item, int field)
{
    if (!g.rowGrid || item < 0 || field < 0 || field >= kHmFieldCount) return L"";
    return GridCellText(g.rowGrid, item % kHomeMaxRowItem, (item / kHomeMaxRowItem) * kHmFieldCount + field);
}

std::wstring HomeLampText(int item)
{
    if (item < 0 || item >= (int)g.items.size()) return L"";
    const int v = g.items[(std::size_t)item].lamp;
    if (v < 0) return L"—";
    std::wstring s;
    GridSetInt(s, v);
    return s;
}

int HomeLogCount() { return g.logGrid ? GridRowCount(g.logGrid) : -1; }

std::wstring HomeLogText(int line) { return g.logGrid ? GridCellText(g.logGrid, line, 0) : L""; }

std::wstring HomeStatusText() { return LabelText(g.status); }

int HomeButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.all;
}

int HomeEnabledButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.enabled;
}

void* HomeRowGrid() { return g.rowGrid; }
void* HomeLogGrid() { return g.logGrid; }

std::wstring HomeSummaryText() { return LabelText(g.summary); }

double HomeLastUpdateMs() { return g.lastUpdateMs; }

}  // namespace w906native
