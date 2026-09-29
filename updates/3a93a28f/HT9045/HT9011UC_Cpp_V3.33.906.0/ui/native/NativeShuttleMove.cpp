// ===========================================================================
//  ui/native/NativeShuttleMove.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove native window, v1 display only.  See NativeShuttleMove.h.
//  Shape follows NativeMotorTest.cpp: NativeGrid for every table, a LabelCreate summary, static texts once, only changed rows
//  marked (a no-change update repaints 0 cells; ctest NativeShuttleMove_Headless).
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // as the other ui/native files
#endif
#include "ui/native/NativeShuttleMove.h"
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

namespace {

const wchar_t kClassName[] = L"W906NativeShuttleMove";
const int kIdcTitle = 6001, kIdcNote = 6002, kIdcSummary = 6003, kIdcEnc = 6004, kIdcPoints = 6005, kIdcTrays = 6006,
          kIdcFlags = 6007, kIdcExit = 6008, kIdcLog = 6009, kIdcCmdLabel = 6010, kIdcCmd0 = 6100;

// golden command buttons (ShuttleMove.dfm captions) and the FormShow rule that decides whether golden shows them
struct CmdDef { const wchar_t* caption; int vis; };
const CmdDef kCmds[] = {
    {L"Shuttle1 Left", kSmVisAlways},  {L"Shuttle1 Right", kSmVisAlways}, {L"Shuttle2 Left", kSmVisAlways},
    {L"Shuttle2 Right", kSmVisAlways}, {L"Out Shuttle 1", kSmVisAlways},  {L"Out Shuttle 2", kSmVisAlways},
    {L"In Shuttle1 8 site kit pos1", kSmVisAlways}, {L"In Shuttle2 8 site kit pos1", kSmVisAlways},
    {L"In Shuttle 1 Bar Code Pos", kSmVisBarCode},  {L"In Shuttle 2 Bar Code Pos", kSmVisBarCode},
    {L"T.Step", kSmVisTStep},
    {L"OutShuttle1 One Row kit pos1", kSmVisAlways}, {L"OutShuttle2 One Row kit pos1", kSmVisAlways},
    {L"OutShuttle1 8 site kit pos1", kSmVisAlways},  {L"OutShuttle2 8 site kit pos1", kSmVisAlways},
    {L"Retry", kSmVisRetry},
    {L"In Shuttle 1 16 site kit pos", kSmVisLatch},    {L"In Shuttle 2 16 site kit pos", kSmVisLatch},
    {L"In Shuttle 1 16 site kit Action", kSmVisLatch}, {L"In Shuttle 2 16 site kit Action", kSmVisLatch},
    {L"Start", kSmVisNever},
    {L"Sensor Adj.", kSmVisSensor}, {L"Sen. Latch", kSmVisSensorLatch}, {L"Bar Code", kSmVisAlways},
    {L"Save", kSmVisAlways},
};
const int kCmdCount = (int)(sizeof(kCmds) / sizeof(kCmds[0]));

const wchar_t* const kFlagNames[kSmFlCount] = {
    L"gbBarCode（Move to Bar Code Pos）顯示", L"gbInFiberCheckShtSnLct（latch）顯示", L"btRetry 顯示（只有模擬版）",
    L"sbShuttleSensor 顯示", L"sbSensorLatch 顯示", L"btnTStep 顯示", L"教導點鎖定（SPIL）", L"單邊 Shuttle", L"meShuttleMaintain 紀錄"};

struct ViewState {
    HWND hwnd, title, note, summary, enc, points, trays, flags, exitBtn, log, cmdLabel;
    std::vector<HWND>         cmd;
    ShuttleMoveState          s;
    bool                      haveState;
    std::vector<int>          ptRows, trRows;   // visible point / tray indexes
    std::vector<std::wstring> ptName, ptGroup, ptField, trName, trLayout;
    std::vector<std::wstring> encAlias;
    double                    lastUpdateMs;
    ViewState() : hwnd(0), title(0), note(0), summary(0), enc(0), points(0), trays(0), flags(0), exitBtn(0), log(0), cmdLabel(0),
                  haveState(false), lastUpdateMs(0) {}
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

bool CmdVisible(int vis, const ShuttleMoveState& s)
{
    switch (vis) {
    case kSmVisAlways:      return true;
    case kSmVisBarCode:     return s.barCodeVisible;
    case kSmVisLatch:       return s.latchVisible;
    case kSmVisRetry:       return s.retryVisible;
    case kSmVisSensor:      return s.sensorVisible;
    case kSmVisSensorLatch: return s.sensorLatchVisible;
    case kSmVisTStep:       return s.barCodeVisible && s.tStepVisible;   // btnTStep sits in gbBarCode
    default:                return false;
    }
}

std::wstring TrayLayout(int x, int y)
{
    if (x <= 0 || y <= 0) return L"—";
    if (x > 16) x = 16;
    std::wstring out;
    for (int r = 0; r < y; ++r) {
        if (r) out += L" / ";
        out.append((std::size_t)x, L'□');
    }
    return out;
}

void EncCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row > 1) return;
    const ShuttleEncoder& e = g.s.enc[row];
    if (!e.has) out.color = RGB(128, 128, 128);
    switch (col) {
    case kSmEncName:   out.text = row == 0 ? L"palSh1Encoder（In Shuttle 1）" : L"palSh2Encoder（In Shuttle 2）"; break;
    case kSmEncAlias:  out.text = g.encAlias.size() > (std::size_t)row ? g.encAlias[(std::size_t)row] : L""; break;
    case kSmEncValue:  if (e.has) GridSetInt(out.text, e.enc); else out.text = L"—"; break;
    case kSmEncSource: out.text = Widen(e.has ? e.source : (e.why.empty() ? e.source : e.why)); break;
    default: break;
    }
}

void PointCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.ptRows.size()) return;
    const std::size_t i = (std::size_t)g.ptRows[(std::size_t)row];
    const ShuttlePoint& p = g.s.points[i];
    switch (col) {
    case kSmPtGroup: out.text = g.ptGroup[i]; out.color = RGB(60, 60, 60); break;
    case kSmPtName:  out.text = g.ptName[i]; break;
    case kSmPtValue: GridSetInt(out.text, p.value); break;
    case kSmPtField: out.text = g.ptField[i]; out.color = RGB(90, 90, 90); break;
    case kSmPtState: out.text = p.locked ? L"鎖定（SPIL，golden Enabled=false）" : L""; if (p.locked) out.color = RGB(160, 100, 0); break;
    default: break;
    }
}

void TrayCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.trRows.size()) return;
    const std::size_t i = (std::size_t)g.trRows[(std::size_t)row];
    const ShuttleTray& t = g.s.trays[i];
    switch (col) {
    case kSmTrName:   out.text = g.trName[i]; break;
    case kSmTrSize: {
        char b[32];
        std::snprintf(b, sizeof(b), "%d x %d", t.xItem, t.yItem);
        GridSetAscii(out.text, b);
        break;
    }
    case kSmTrLayout: out.text = g.trLayout[i]; break;
    default: break;
    }
}

void FlagValue(int row, std::wstring& out)
{
    const ShuttleMoveState& s = g.s;
    switch (row) {
    case kSmFlBarCode:     out = s.barCodeVisible ? L"是" : L"否"; break;
    case kSmFlLatch:       out = s.latchVisible ? L"是" : L"否（沒有 auto latch）"; break;
    case kSmFlRetry:       out = s.retryVisible ? L"是" : L"否"; break;
    case kSmFlSensor:      out = s.sensorVisible ? L"是" : L"否"; break;
    case kSmFlSensorLatch: out = s.sensorLatchVisible ? L"是" : L"否"; break;
    case kSmFlTStep:       out = (s.barCodeVisible && s.tStepVisible) ? L"是" : L"否"; break;
    case kSmFlSpil:        out = s.spilLocked ? L"是（教導點不能改）" : L"否"; break;
    case kSmFlOneSide:
        out = s.oneSide < 0 ? L"否（兩邊都用）"
                            : (s.oneSide == 0 ? L"只用 Shuttle 1（golden 停用 Shuttle2 Left／Right）"
                                              : L"只用 Shuttle 2（golden 停用 Shuttle1 Left／Right）");
        break;
    case kSmFlLog:         out = L"v1 空白：golden 的移動／掃描序列（:161-1784）還沒移植"; break;
    default: break;
    }
}

void FlagCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= kSmFlCount) return;
    if (col == 0) { out.text = kFlagNames[row]; out.color = RGB(60, 60, 60); return; }
    if (col == 1) FlagValue(row, out.text);
}

HWND MakeChild(const wchar_t* cls, const wchar_t* text, DWORD style, int id, DWORD exStyle = 0)
{
    HWND h = ::CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
                               g.hwnd, (HMENU)(INT_PTR)id, ::GetModuleHandleW(0), 0);
    if (h) ::SendMessageW(h, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    return h;
}

HWND MakeGrid(int id, GridCellFn fn)
{
    HWND h = GridCreate(g.hwnd, id, fn, 0);
    if (h) ::SendMessageW(h, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    return h;
}

void CreateChildren()
{
    g.title = MakeChild(L"STATIC", L"HW.ShuttleMove — 原生 Win32 視窗（v1 只顯示，St02 20260929）", SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"golden TfShuttleMove 的顯示部分：兩個編碼器、教導點、四個格子的版面、開窗時的顯示規則。"
                       L"指令鈕照 golden 列出但全部停用；Exit 只關這個視窗（不跑 golden FormClose：它會把 SystemStart 關掉）。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"（等待第一批資料）");
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);

    g.enc = MakeGrid(kIdcEnc, &EncCell);
    std::vector<GridColumn> ec((std::size_t)kSmEncCount);
    ec[kSmEncName]   = GridColumn(L"編碼器", 200, kGridLeft);
    ec[kSmEncAlias]  = GridColumn(L"Alias", 140, kGridLeft);
    ec[kSmEncValue]  = GridColumn(L"位置", 90, kGridRight);
    ec[kSmEncSource] = GridColumn(L"來源／說明", 340, kGridLeft);
    GridSetColumns(g.enc, ec, std::vector<int>());
    GridSetRowCount(g.enc, 2);

    g.points = MakeGrid(kIdcPoints, &PointCell);
    std::vector<GridColumn> pc((std::size_t)kSmPtCount);
    pc[kSmPtGroup] = GridColumn(L"群組", 230, kGridLeft);
    pc[kSmPtName]  = GridColumn(L"教導點（golden TEdit）", 200, kGridLeft);
    pc[kSmPtValue] = GridColumn(L"值", 80, kGridRight);
    pc[kSmPtField] = GridColumn(L"來源欄位", 210, kGridLeft);
    pc[kSmPtState] = GridColumn(L"狀態", 230, kGridLeft);
    GridSetColumns(g.points, pc, std::vector<int>());

    g.trays = MakeGrid(kIdcTrays, &TrayCell);
    std::vector<GridColumn> tc((std::size_t)kSmTrCount);
    tc[kSmTrName]   = GridColumn(L"格子（golden TTMyTray）", 200, kGridLeft);
    tc[kSmTrSize]   = GridColumn(L"欄 x 列", 70, kGridCenter);
    tc[kSmTrLayout] = GridColumn(L"版面（格子數字 v1 不顯示）", 420, kGridLeft);
    GridSetColumns(g.trays, tc, std::vector<int>());

    g.flags = MakeGrid(kIdcFlags, &FlagCell);
    std::vector<GridColumn> fc(2);
    fc[0] = GridColumn(L"golden FormShow 規則", 280, kGridLeft);
    fc[1] = GridColumn(L"這台", 420, kGridLeft);
    GridSetColumns(g.flags, fc, std::vector<int>());
    GridSetRowCount(g.flags, kSmFlCount);

    g.cmdLabel = MakeChild(L"STATIC", L"golden 指令（v1 停用；只列 golden 會顯示的）：", SS_LEFT, kIdcCmdLabel);
    g.cmd.clear();
    for (int i = 0; i < kCmdCount; ++i)
        g.cmd.push_back(MakeChild(L"BUTTON", kCmds[i].caption, BS_PUSHBUTTON | WS_DISABLED, kIdcCmd0 + i));
    g.log = MakeChild(L"STATIC", L"meShuttleMaintain：v1 空白（序列還沒移植）", SS_LEFT | SS_NOPREFIX, kIdcLog);
    g.exitBtn = MakeChild(L"BUTTON", L"Exit（只關這個視窗）", BS_PUSHBUTTON | WS_TABSTOP, kIdcExit);
}

void Layout()
{
    if (!g.hwnd) return;
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top, m = 8, gw = w - 2 * m;
    ::MoveWindow(g.title,   m, 6, gw, 22, TRUE);
    ::MoveWindow(g.note,    m, 30, gw, 20, TRUE);
    ::MoveWindow(g.summary, m, 52, gw, 44, TRUE);
    int y = 100;
    ::MoveWindow(g.enc, m, y, gw, 70, TRUE);        y += 76;
    ::MoveWindow(g.points, m, y, gw, 170, TRUE);    y += 176;
    ::MoveWindow(g.trays, m, y, gw, 118, TRUE);     y += 124;
    const int btnRows = (kCmdCount + 4) / 5, cmdH = 22 + btnRows * 30 + 30;
    const int flagsH = (h - y - m - cmdH) > 80 ? (h - y - m - cmdH) : 80;
    ::MoveWindow(g.flags, m, y, gw, flagsH, TRUE);  y += flagsH + 4;
    ::MoveWindow(g.cmdLabel, m, y, 400, 20, TRUE);
    // visible buttons flow left-to-right (hidden ones take no place)
    int k = 0;
    for (int i = 0; i < kCmdCount; ++i) {
        HWND b = g.cmd[(std::size_t)i];
        // the button's own WS_VISIBLE (IsWindowVisible is false for every child while this window is hidden)
        if (g.haveState && !(::GetWindowLongW(b, GWL_STYLE) & WS_VISIBLE)) continue;
        ::MoveWindow(b, m + (k % 5) * 236, y + 22 + (k / 5) * 30, 230, 26, TRUE);
        ++k;
    }
    ::MoveWindow(g.log, m, y + 22 + btnRows * 30 + 4, gw - 210, 20, TRUE);
    ::MoveWindow(g.exitBtn, w - m - 200, y + 22 + btnRows * 30, 200, 26, TRUE);
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
        mm->ptMinTrackSize.x = 1200;
        mm->ptMinTrackSize.y = 700;
        return 0;
    }
    case WM_COMMAND:
        // Exit only; golden's command buttons are WS_DISABLED and deliberately have no branch here.
        if (LOWORD(wp) == kIdcExit && HIWORD(wp) == BN_CLICKED) ::DestroyWindow(hwnd);   // NOT golden sbtExitClick / FormClose
        return 0;
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HostWindowDestroyed(hwnd);
        break;
    case WM_NCDESTROY:
        g.hwnd = g.title = g.note = g.summary = g.enc = g.points = g.trays = g.flags = g.exitBtn = g.log = g.cmdLabel = 0;
        g.cmd.clear();
        g.haveState = false;
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

struct ButtonCount { int all, visible, enabled; };
BOOL CALLBACK CountButtons(HWND h, LPARAM lp)
{
    ButtonCount* c = reinterpret_cast<ButtonCount*>(lp);
    wchar_t cls[32];
    if (::GetClassNameW(h, cls, 32) > 0 && ::lstrcmpiW(cls, L"Button") == 0) {
        ++c->all;
        if (::GetWindowLongW(h, GWL_STYLE) & WS_VISIBLE) ++c->visible;
        if (::IsWindowEnabled(h)) ++c->enabled;
    }
    return TRUE;
}

// the rows' identity (names, fields, which are shown): a change here rebuilds the grids' rows
bool Structural(const ShuttleMoveState& a, const ShuttleMoveState& b)
{
    if (a.points.size() != b.points.size() || a.trays.size() != b.trays.size()) return true;
    for (std::size_t i = 0; i < a.points.size(); ++i)
        if (a.points[i].name != b.points[i].name || a.points[i].group != b.points[i].group ||
            a.points[i].field != b.points[i].field || a.points[i].visible != b.points[i].visible)
            return true;
    for (std::size_t i = 0; i < a.trays.size(); ++i)
        if (a.trays[i].name != b.trays[i].name || a.trays[i].visible != b.trays[i].visible || a.trays[i].xItem != b.trays[i].xItem ||
            a.trays[i].yItem != b.trays[i].yItem)
            return true;
    return a.enc[0].alias != b.enc[0].alias || a.enc[1].alias != b.enc[1].alias;
}

bool FlagsChanged(const ShuttleMoveState& a, const ShuttleMoveState& b)
{
    return a.barCodeVisible != b.barCodeVisible || a.latchVisible != b.latchVisible || a.retryVisible != b.retryVisible ||
           a.sensorVisible != b.sensorVisible || a.sensorLatchVisible != b.sensorLatchVisible || a.tStepVisible != b.tStepVisible ||
           a.spilLocked != b.spilLocked || a.oneSide != b.oneSide;
}

bool EncChanged(const ShuttleEncoder& a, const ShuttleEncoder& b)
{
    return a.has != b.has || a.enc != b.enc || a.source != b.source || a.why != b.why;
}

void ApplyButtonVisibility()
{
    for (int i = 0; i < kCmdCount; ++i) {
        HWND b = g.cmd[(std::size_t)i];
        const bool want = CmdVisible(kCmds[i].vis, g.s);
        if (want != ((::GetWindowLongW(b, GWL_STYLE) & WS_VISIBLE) != 0)) ::ShowWindow(b, want ? SW_SHOWNA : SW_HIDE);
    }
    Layout();
}

std::wstring BuildSummary()
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats a = GridGetStats(g.enc), b = GridGetStats(g.points), c = GridGetStats(g.trays), d = GridGetStats(g.flags);
    const std::string why = g.s.why.empty() ? std::string() : "｜" + g.s.why;
    char buf[900];
    std::snprintf(buf, sizeof(buf),
                  "[%s] 教導點 %u 個（顯示 %u）｜格子 %u 個（顯示 %u）｜1203 監看器：%s｜拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u%s\n"
                  "畫面效能：上次更新 %.2f ms｜累計重畫 %lu 格（編碼器 %lu／教導點 %lu／格子 %lu／規則 %lu）",
                  g.s.buildConfig.empty() ? "?" : g.s.buildConfig.c_str(), (unsigned)g.s.points.size(), (unsigned)g.ptRows.size(),
                  (unsigned)g.s.trays.size(), (unsigned)g.trRows.size(), g.s.monitorOpen ? "已開卡" : "沒有開卡", g.s.keepaliveCalls,
                  (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned)st.wMilliseconds, why.c_str(),
                  g.lastUpdateMs, a.cellsPainted + b.cellsPainted + c.cellsPainted + d.cellsPainted, a.cellsPainted, b.cellsPainted,
                  c.cellsPainted, d.cellsPainted);
    return Widen(buf);
}

void RebuildRows()
{
    g.ptRows.clear();
    g.trRows.clear();
    g.ptName.assign(g.s.points.size(), std::wstring());
    g.ptGroup.assign(g.s.points.size(), std::wstring());
    g.ptField.assign(g.s.points.size(), std::wstring());
    for (std::size_t i = 0; i < g.s.points.size(); ++i) {
        g.ptName[i] = Widen(g.s.points[i].name);
        g.ptGroup[i] = Widen(g.s.points[i].group);
        g.ptField[i] = Widen(g.s.points[i].field);
        if (g.s.points[i].visible) g.ptRows.push_back((int)i);
    }
    g.trName.assign(g.s.trays.size(), std::wstring());
    g.trLayout.assign(g.s.trays.size(), std::wstring());
    for (std::size_t i = 0; i < g.s.trays.size(); ++i) {
        g.trName[i] = Widen(g.s.trays[i].name);
        g.trLayout[i] = TrayLayout(g.s.trays[i].xItem, g.s.trays[i].yItem);
        if (g.s.trays[i].visible) g.trRows.push_back((int)i);
    }
    g.encAlias.assign(2, std::wstring());
    g.encAlias[0] = Widen(g.s.enc[0].alias);
    g.encAlias[1] = Widen(g.s.enc[1].alias);
    if (g.points) GridSetRowCount(g.points, (int)g.ptRows.size());
    if (g.trays) GridSetRowCount(g.trays, (int)g.trRows.size());
    if (g.enc) GridSetRowCount(g.enc, 2);
}

}  // namespace

bool ShuttleMoveOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — HW.ShuttleMove（原生，v1 只顯示）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1260, 900,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void ShuttleMoveClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool ShuttleMoveIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void ShuttleMoveUpdate(const ShuttleMoveState& s)
{
    if (!ShuttleMoveIsOpen()) return;
    const double t0 = NowMs();
    const bool first = !g.haveState;
    const bool structural = first || Structural(s, g.s);
    const bool flags = first || FlagsChanged(s, g.s);
    const bool enc0 = first || EncChanged(s.enc[0], g.s.enc[0]), enc1 = first || EncChanged(s.enc[1], g.s.enc[1]);
    std::vector<int> ptChanged;
    if (!structural)
        for (std::size_t i = 0; i < s.points.size(); ++i)
            if (s.points[i].value != g.s.points[i].value || s.points[i].locked != g.s.points[i].locked) ptChanged.push_back((int)i);
    g.s = s;
    g.haveState = true;
    if (structural) {
        RebuildRows();   // full render of the row grids
    } else {
        for (std::size_t k = 0; k < ptChanged.size(); ++k)
            for (std::size_t r = 0; r < g.ptRows.size(); ++r)
                if (g.ptRows[r] == ptChanged[k]) GridMarkRow(g.points, (int)r);
        if (!ptChanged.empty()) GridFlush(g.points);
        if (enc0) GridMarkRow(g.enc, 0);
        if (enc1) GridMarkRow(g.enc, 1);
        if (enc0 || enc1) GridFlush(g.enc);
    }
    if (flags) {
        GridMarkAll(g.flags);   // only the changed cells are painted
        GridFlush(g.flags);
        ApplyButtonVisibility();
    }
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
}

void* ShuttleMoveHwnd() { return g.hwnd; }

std::wstring ShuttleMoveEncText(int row, int col) { return g.enc ? GridCellText(g.enc, row, col) : L""; }
int ShuttleMovePointCount() { return g.points ? GridRowCount(g.points) : -1; }
std::wstring ShuttleMovePointText(int row, int col) { return g.points ? GridCellText(g.points, row, col) : L""; }
int ShuttleMoveTrayCount() { return g.trays ? GridRowCount(g.trays) : -1; }
std::wstring ShuttleMoveTrayText(int row, int col) { return g.trays ? GridCellText(g.trays, row, col) : L""; }
std::wstring ShuttleMoveFlagText(int row) { return g.flags ? GridCellText(g.flags, row, 1) : L""; }

int ShuttleMoveButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.all;
}

int ShuttleMoveVisibleButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.visible;
}

int ShuttleMoveEnabledButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.enabled;
}

bool ShuttleMoveButtonVisible(const wchar_t* caption)
{
    for (int i = 0; i < kCmdCount && i < (int)g.cmd.size(); ++i)
        if (::lstrcmpW(kCmds[i].caption, caption) == 0) return (::GetWindowLongW(g.cmd[(std::size_t)i], GWL_STYLE) & WS_VISIBLE) != 0;
    return false;
}

void* ShuttleMoveGrid(int which)
{
    switch (which) {
    case 0: return g.enc;
    case 1: return g.points;
    case 2: return g.trays;
    case 3: return g.flags;
    default: return 0;
    }
}

std::wstring ShuttleMoveSummaryText()
{
    if (!g.summary) return L"";
    wchar_t b[900];
    b[0] = 0;
    ::GetWindowTextW(g.summary, b, 900);
    return b;
}

double ShuttleMoveLastUpdateMs() { return g.lastUpdateMs; }

}  // namespace w906native
