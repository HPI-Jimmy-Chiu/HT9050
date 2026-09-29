// ===========================================================================
//  ui/native/NativeMotorTest.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest native window, v1 display only.  See NativeMotorTest.h.
//  Shape follows NativeMotorView.cpp (ST01-E3): NativeGrid for every table (no ListView -- no manifest, comctl32 v5
//  flickers), a double-buffered LabelCreate summary, static texts built once, 20 ms updates that only mark changed rows,
//  so an update where nothing changed repaints 0 cells (ctest NativeMotorTest_Headless checks it).
//  The lamp decode is NativeMotorView's MotorLedOn (one decode for both windows).
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // as the other ui/native files
#endif
#include "ui/native/NativeMotorTest.h"
#include "ui/native/NativeMotorView.h"   // MotorLedOn / MotorLed (the golden ALed1..10 decode)
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

namespace {

const wchar_t kClassName[] = L"W906NativeMotorTest";
const int kIdcTitle = 3001, kIdcNote = 3002, kIdcSummary = 3003, kIdcList = 3004, kIdcLamps = 3005, kIdcDetail = 3006,
          kIdcExit = 3007, kIdcBlank = 3008, kIdcCmdLabel = 3009, kIdcCmd0 = 3100;

// golden Label112-122 / Label10 captions over ALed1..10 (uMotorTest.dfm), in MotorLed order
const wchar_t* const kLampNames[kLedCount] = {L"CW", L"HOME", L"CCW", L"EM", L"Alarm", L"SoftCW", L"SoftCCW", L"S-Alarm",
                                             L"InPos", L"ServoOn"};

// golden's command controls (inventory §1 "Commands"): shown, WS_DISABLED, and WndProc has no branch for them
const wchar_t* const kCmdNames[] = {
    L"Jog -", L"Jog +", L"Move -", L"Move +", L"Loop Move", L"Home", L"Go", L"Go Soft +", L"Go Soft -",
    L"Set Pos +", L"Set Pos -", L"Servo Off", L"Motor Power", L"Stop", L"High Speed", L"Low Speed", L"Home High",
    L"Home Low", L"Soft + Pos", L"Soft - Pos", L"Range", L"Rate", L"Set Range", L"Set Rate", L"Reload Motor Data",
    L"Reset MNet", L"Copy From"};
const int kCmdCount = (int)(sizeof(kCmdNames) / sizeof(kCmdNames[0]));

// detail grid item names (golden strngrdMotor captions for the ten parameters, :655-669 English)
const wchar_t* const kItemNames[kMtRowCount] = {
    L"Motor（pnlMotorAlias）", L"Command Pos（edtCommandPos）", L"Encoder Pos（pnlEncoderPos）", L"Real Speed（lblRealSpeed）",
    L"Home Offset（edtHomeOffset）", L"HomeFlag", L"Servo", L"Alarm", L"Busy", L"InPos",
    L"Loop Count（lblLoopCount）", L"Loop 執行中", L"Home 執行中",
    L"Init Speed", L"Jog High Speed", L"Jog Low Speed", L"Home High Speed", L"Home Low Speed", L"Soft Limit +",
    L"Soft Limit -", L"Acc", L"Dec", L"Range",
    L"Jog + Time（lblJogPTime）", L"Jog - Time（lblJogNTime）", L"Avg Time（lblAvgTime）", L"Motor Power（btnMotorPower）",
    L"Lock（pnlStop）", L"網頁／C++ 選的軸（ActiveIndex）", L"品質", L"來源", L"說明"};

struct ViewState {
    HWND hwnd, title, note, summary, list, lamps, detail, exitBtn, blank, cmdLabel;
    std::vector<HWND>          cmd;
    std::vector<MotorTestAxis> axes;
    std::vector<std::wstring>  noText, aliasText;
    MotorTestPage              page;
    MotorTestSummary           sum;
    int                        sel;         // effective selection (index into axes); -1 = none
    double                     lastUpdateMs, lastT0, avgGapMs;
    ViewState() : hwnd(0), title(0), note(0), summary(0), list(0), lamps(0), detail(0), exitBtn(0), blank(0), cmdLabel(0),
                  sel(-1), lastUpdateMs(0), lastT0(0), avgGapMs(0) {}
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

void SetNum(std::wstring& out, bool has, int v)
{
    if (!has) out = L"—";
    else GridSetInt(out, v);
}

// snprintf + GridSetAscii (as the other ui/native files): MinGW.org's swprintf is not reliably the ISO one with a size
void SetUns(std::wstring& out, bool has, unsigned long v)
{
    if (!has) { out = L"—"; return; }
    char b[24];
    std::snprintf(b, sizeof(b), "%lu", v);
    GridSetAscii(out, b);
}

void SetDbl(std::wstring& out, bool has, double v, int decimals)
{
    if (!has) { out = L"—"; return; }
    char b[40];
    std::snprintf(b, sizeof(b), "%.*f", decimals, v);
    GridSetAscii(out, b);
}

void SetTri(std::wstring& out, int v, const wchar_t* on, const wchar_t* off)
{
    out = v < 0 ? L"—" : (v ? on : off);
}

MotorRow LampRow(const MotorTestAxis& a)
{
    MotorRow r;
    r.ledKnown = a.ledKnown;
    r.motionIO = a.motionIO;
    r.state = a.state;
    return r;
}

const MotorTestAxis* Sel()
{
    return (g.sel >= 0 && g.sel < (int)g.axes.size()) ? &g.axes[(std::size_t)g.sel] : 0;
}

// ---- grid callbacks (asked only for rows marked changed, or on a full render) ----
void ListCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.axes.size()) return;
    const MotorTestAxis& a = g.axes[(std::size_t)row];
    if (a.quality == "nosource") out.color = RGB(128, 128, 128);
    if (row == g.sel) out.color = RGB(210, 0, 0);   // golden ShowMotorSelect :671-698: the selected motor is red
    switch (col) {
    case kMtColNo:    out.text = g.noText[(std::size_t)row]; break;
    case kMtColAlias: out.text = g.aliasText[(std::size_t)row]; break;
    case kMtColCmd:   SetNum(out.text, a.hasCmd, a.cmd); break;
    case kMtColEnc:   SetNum(out.text, a.hasEnc, a.enc); break;
    case kMtColServo: SetTri(out.text, a.servoOn, L"ON", L"OFF"); break;
    case kMtColAlarm: SetTri(out.text, a.alarm, L"警報", L"正常"); if (a.alarm == 1) out.color = RGB(210, 0, 0); break;
    case kMtColHome:  if (a.homeFlag < 0) out.text = L"—"; else GridSetInt(out.text, a.homeFlag); break;
    default: break;
    }
}

void LampCell(void*, int row, int col, GridCell& out)
{
    if (row != 0 || col < 0 || col >= kLedCount) return;
    const MotorTestAxis* a = Sel();
    out.kind = kGridLedText;
    out.ledEdge = RGB(40, 40, 40);
    out.text = kLampNames[col];
    if (!a || !a->ledKnown) { out.ledFill = RGB(255, 255, 255); out.ledEdge = RGB(150, 150, 150); out.color = RGB(128, 128, 128); }   // null: white, not "off"
    else if (MotorLedOn(LampRow(*a), col)) out.ledFill = (col == kLedAlarm || col == kLedEmg) ? RGB(255, 60, 40) : RGB(0, 210, 60);
    else out.ledFill = RGB(60, 70, 60);
}

void DetailValue(int row, std::wstring& out)
{
    const MotorTestAxis* a = Sel();
    const MotorTestPage& p = g.page;
    const bool pk = p.known;
    if (!a && row < kMtRowJogPTime) { out = row == kMtRowAlias ? L"（沒有選馬達）" : L"—"; return; }
    switch (row) {
    case kMtRowAlias:      out = g.noText[(std::size_t)g.sel] + L"  " + g.aliasText[(std::size_t)g.sel]; break;
    case kMtRowCmd:        SetNum(out, a->hasCmd, a->cmd); break;
    case kMtRowEnc:        SetNum(out, a->hasEnc, a->enc); break;
    case kMtRowSpeed:      SetNum(out, a->hasSpeed, a->speed); break;
    case kMtRowHomeOffset: SetNum(out, a->hasHomeOffset, a->homeOffset); break;
    case kMtRowHomeFlag:
        switch (a->homeFlag) {
        case 0: out = L"0 未歸零"; break;
        case 1: out = L"1 完成"; break;
        case 2: out = L"2 失敗"; break;
        default: if (a->homeFlag < 0) out = L"—"; else GridSetInt(out, a->homeFlag); break;
        }
        break;
    case kMtRowServo:      SetTri(out, a->servoOn, L"ON", L"OFF"); break;
    case kMtRowAlarm:      SetTri(out, a->alarm, L"警報", L"正常"); break;
    case kMtRowBusy:       SetTri(out, a->busy, L"忙", L"閒"); break;
    case kMtRowInPos:      SetTri(out, a->inPos, L"到位", L"未到"); break;
    case kMtRowLoopCount:  SetUns(out, a->hasLoopCount, a->loopCount); break;
    case kMtRowLoopJob:    SetTri(out, a->loopJob, L"是", L"否"); break;
    case kMtRowHomeJob:    SetTri(out, a->homeJob, L"是", L"否"); break;
    case kMtRowInitSpeed:  SetUns(out, a->hasParams, a->initSpeed); break;
    case kMtRowJogHigh:    SetUns(out, a->hasParams, a->jogHigh); break;
    case kMtRowJogLow:     SetUns(out, a->hasParams, a->jogLow); break;
    case kMtRowHomeHigh:   SetUns(out, a->hasParams, a->homeHigh); break;
    case kMtRowHomeLow:    SetUns(out, a->hasParams, a->homeLow); break;
    case kMtRowSoftP:      SetNum(out, a->hasParams, a->softP); break;
    case kMtRowSoftN:      SetNum(out, a->hasParams, a->softN); break;
    case kMtRowAcc:        SetDbl(out, a->hasParams, a->acc, 3); break;
    case kMtRowDec:        SetDbl(out, a->hasParams, a->dec, 3); break;
    case kMtRowRange:      SetUns(out, a->hasParams, a->range); break;
    case kMtRowJogPTime:   SetNum(out, pk && p.hasJogP, p.jogP); break;
    case kMtRowJogNTime:   SetNum(out, pk && p.hasJogN, p.jogN); break;
    case kMtRowAvgTime:    SetDbl(out, pk && p.hasAvg, p.avg, 3); break;
    case kMtRowPower:
        if (!pk || !p.hasPower) out = L"—";
        else out = std::wstring(p.relayOn ? L"Relay ON" : L"Relay OFF") + (p.motorPowerState ? L"｜Power ON" : L"｜Power OFF") +
                   (p.powerPending ? L"｜開啟中" : L"");
        break;
    case kMtRowLock:
        if (!pk || !p.hasLock) out = L"—";
        else out = std::wstring(p.locked ? L"鎖定（黃）" : L"未鎖定") + (p.lockText.empty() ? L"" : L"｜" + Widen(p.lockText));
        break;
    case kMtRowSelectedWeb: out = !pk ? L"—" : (p.selectedMotor.empty() ? L"（沒有）" : Widen(p.selectedMotor)); break;
    case kMtRowQuality:    out = a ? Widen(a->quality) : L"—"; break;
    case kMtRowSource:     out = a ? Widen(a->source) : L"—"; break;
    case kMtRowWhy:        out = a ? Widen(a->errText) : L""; if (!pk && !g.sum.pageWhy.empty()) out += (out.empty() ? L"" : L"；") + Widen(g.sum.pageWhy); break;
    default: break;
    }
}

void DetailCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= kMtRowCount) return;
    if (col == 0) { out.text = kItemNames[row]; out.color = RGB(60, 60, 60); return; }
    if (col == 1) DetailValue(row, out.text);
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
    g.title = MakeChild(L"STATIC", L"HW.MotorTest — 原生 Win32 視窗（v1 只顯示，St02 20260929）", SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"golden TfMotorTest 的顯示部分：左邊點選馬達（只換畫面，不寫卡），右邊是選到的馬達。"
                       L"指令鈕照 golden 列出但全部停用、程式沒有接任何運動或 IO；開窗不送 Motor Power／Servo On。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"（等待第一批資料）");
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);

    g.list = GridCreate(g.hwnd, kIdcList, &ListCell, 0);
    if (g.list) ::SendMessageW(g.list, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> lc((std::size_t)kMtColCount);
    lc[kMtColNo]    = GridColumn(L"No", 46, kGridLeft);
    lc[kMtColAlias] = GridColumn(L"Alias", 150, kGridLeft);
    lc[kMtColCmd]   = GridColumn(L"命令位置", 84, kGridRight);
    lc[kMtColEnc]   = GridColumn(L"編碼器位置", 90, kGridRight);
    lc[kMtColServo] = GridColumn(L"伺服", 48, kGridLeft);
    lc[kMtColAlarm] = GridColumn(L"警報", 48, kGridLeft);
    lc[kMtColHome]  = GridColumn(L"Home", 48, kGridCenter);
    GridSetColumns(g.list, lc, std::vector<int>());

    g.lamps = GridCreate(g.hwnd, kIdcLamps, &LampCell, 0);
    if (g.lamps) ::SendMessageW(g.lamps, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> mc((std::size_t)kLedCount);
    for (int k = 0; k < kLedCount; ++k) mc[(std::size_t)k] = GridColumn(L"", k == kLedSoftCcw || k == kLedServo ? 86 : 72, kGridLeft);
    GridSetColumns(g.lamps, mc, std::vector<int>());
    GridSetRowCount(g.lamps, 1);

    g.detail = GridCreate(g.hwnd, kIdcDetail, &DetailCell, 0);
    if (g.detail) ::SendMessageW(g.detail, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> dc(2);
    dc[0] = GridColumn(L"項目", 230, kGridLeft);
    dc[1] = GridColumn(L"值", 300, kGridLeft);
    GridSetColumns(g.detail, dc, std::vector<int>());
    GridSetRowCount(g.detail, kMtRowCount);

    g.cmdLabel = MakeChild(L"STATIC", L"golden 指令（v1 停用）：", SS_LEFT, kIdcCmdLabel);
    g.cmd.clear();
    for (int i = 0; i < kCmdCount; ++i)
        g.cmd.push_back(MakeChild(L"BUTTON", kCmdNames[i], BS_PUSHBUTTON | WS_DISABLED, kIdcCmd0 + i));
    g.blank = MakeChild(L"STATIC", L"Pressure／Light Scale／Motor Database 分頁：v1 未做（Galil 讀值還沒移植，留白）", SS_LEFT | SS_NOPREFIX, kIdcBlank);
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
    ::MoveWindow(g.summary, m, 52, w - 2 * m, 62, TRUE);
    const int top = 122, listW = 530;
    const int btnRows = (kCmdCount + 8) / 9, cmdH = 22 + btnRows * 30 + 26;
    const int bodyH = (h - top - m - cmdH) > 120 ? (h - top - m - cmdH) : 120;
    ::MoveWindow(g.list, m, top, listW, bodyH, TRUE);
    const int rx = m + listW + 8, rw = (w - rx - m) > 300 ? (w - rx - m) : 300;
    ::MoveWindow(g.lamps, rx, top, rw, 50, TRUE);
    ::MoveWindow(g.detail, rx, top + 56, rw, bodyH - 56 > 60 ? bodyH - 56 : 60, TRUE);
    const int cy = top + bodyH + 6;
    ::MoveWindow(g.cmdLabel, m, cy, 300, 20, TRUE);
    for (int i = 0; i < kCmdCount; ++i)
        ::MoveWindow(g.cmd[(std::size_t)i], m + (i % 9) * 126, cy + 22 + (i / 9) * 30, 120, 26, TRUE);
    ::MoveWindow(g.blank, m, cy + 22 + btnRows * 30 + 2, w - 2 * m - 200, 20, TRUE);
    ::MoveWindow(g.exitBtn, w - m - 190, cy + 22 + btnRows * 30 - 2, 190, 26, TRUE);
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
        mm->ptMinTrackSize.x = 1100;
        mm->ptMinTrackSize.y = 560;
        return 0;
    }
    case WM_COMMAND:
        // Exit only (the golden command buttons are WS_DISABLED and deliberately have no branch here).
        if (LOWORD(wp) == kIdcExit && HIWORD(wp) == BN_CLICKED) ::DestroyWindow(hwnd);   // NOT golden FormClose
        return 0;
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HostWindowDestroyed(hwnd);
        break;
    case WM_NCDESTROY:
        g.hwnd = g.title = g.note = g.summary = g.list = g.lamps = g.detail = g.exitBtn = g.blank = g.cmdLabel = 0;
        g.cmd.clear();
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

bool StaticChanged(const MotorTestAxis& a, const MotorTestAxis& b)
{
    return a.row != b.row || a.alias != b.alias || a.no != b.no || a.cardModel != b.cardModel || a.boardId != b.boardId ||
           a.port != b.port;
}

bool ListValueChanged(const MotorTestAxis& a, const MotorTestAxis& b)
{
    return a.hasCmd != b.hasCmd || a.cmd != b.cmd || a.hasEnc != b.hasEnc || a.enc != b.enc || a.servoOn != b.servoOn ||
           a.alarm != b.alarm || a.homeFlag != b.homeFlag || a.quality != b.quality;
}

bool DetailValueChanged(const MotorTestAxis& a, const MotorTestAxis& b)
{
    return ListValueChanged(a, b) || a.hasSpeed != b.hasSpeed || a.speed != b.speed || a.hasHomeOffset != b.hasHomeOffset ||
           a.homeOffset != b.homeOffset || a.busy != b.busy || a.inPos != b.inPos || a.loopJob != b.loopJob ||
           a.homeJob != b.homeJob || a.hasLoopCount != b.hasLoopCount || a.loopCount != b.loopCount ||
           a.hasParams != b.hasParams || a.initSpeed != b.initSpeed || a.jogHigh != b.jogHigh || a.jogLow != b.jogLow ||
           a.homeHigh != b.homeHigh || a.homeLow != b.homeLow || a.range != b.range || a.softP != b.softP ||
           a.softN != b.softN || a.acc != b.acc || a.dec != b.dec || a.source != b.source || a.errText != b.errText;
}

bool LampChanged(const MotorTestAxis& a, const MotorTestAxis& b)
{
    return a.ledKnown != b.ledKnown || a.motionIO != b.motionIO || a.state != b.state;
}

bool PageChanged(const MotorTestPage& a, const MotorTestPage& b)
{
    return a.known != b.known || a.selectedMotor != b.selectedMotor || a.hasJogP != b.hasJogP || a.hasJogN != b.hasJogN ||
           a.hasAvg != b.hasAvg || a.jogP != b.jogP || a.jogN != b.jogN || a.avg != b.avg || a.hasPower != b.hasPower ||
           a.relayOn != b.relayOn || a.motorPowerState != b.motorPowerState || a.powerPending != b.powerPending ||
           a.hasLock != b.hasLock || a.locked != b.locked || a.lockText != b.lockText;
}

// the grid's own selection (a click) wins; before any click, follow C++'s ActiveIndex (the web page's pick), else row 0
int EffectiveSel()
{
    const int gs = g.list ? GridSelectedRow(g.list) : -1;
    if (gs >= 0 && gs < (int)g.axes.size()) return gs;
    for (std::size_t i = 0; g.page.known && !g.page.selectedMotor.empty() && i < g.axes.size(); ++i)
        if (g.axes[i].alias == g.page.selectedMotor) return (int)i;
    return g.axes.empty() ? -1 : 0;
}

std::wstring BuildSummary()
{
    int nGood = 0, nNull = 0, nAlarm = 0;
    for (std::size_t i = 0; i < g.axes.size(); ++i) {
        if (g.axes[i].quality == "nosource") ++nNull; else ++nGood;
        if (g.axes[i].alarm == 1) ++nAlarm;
    }
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats a = GridGetStats(g.list), b = GridGetStats(g.detail), c = GridGetStats(g.lamps);
    const std::string why = g.sum.why.empty() ? std::string() : "｜" + g.sum.why;
    char buf[1400];
    std::snprintf(buf, sizeof(buf),
                  "[%s] Motor Test 列出 %u 軸｜有值 %d  沒有值 %d｜警報 %d｜整頁狀態：%s\n"
                  "1203 監看器：%s（開著 %d 軸，poll #%lu）｜拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u%s\n"
                  "畫面效能：上次更新 %.2f ms｜實際更新間隔約 %.0f ms｜累計重畫 %lu 格（清單 %lu／明細 %lu／燈 %lu）",
                  g.sum.buildConfig.empty() ? "?" : g.sum.buildConfig.c_str(), (unsigned)g.axes.size(), nGood, nNull, nAlarm,
                  g.page.known ? "有" : "沒有來源（—）", g.sum.monitorOpen ? "已開卡" : "沒有開卡", g.sum.monitorAxes,
                  g.sum.pollCount, g.sum.keepaliveCalls, (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond,
                  (unsigned)st.wMilliseconds, why.c_str(), g.lastUpdateMs, g.avgGapMs,
                  a.cellsPainted + b.cellsPainted + c.cellsPainted, a.cellsPainted, b.cellsPainted, c.cellsPainted);
    return Widen(buf);
}

}  // namespace

bool MotorTestOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — HW.MotorTest（原生，v1 只顯示）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1180, 820,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    if (g.list) GridSetRowCount(g.list, (int)g.axes.size());
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void MotorTestClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool MotorTestIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void MotorTestUpdate(const std::vector<MotorTestAxis>& axes, const MotorTestPage& page, const MotorTestSummary& sum)
{
    if (!MotorTestIsOpen()) return;
    const double t0 = NowMs();
    if (g.lastT0 > 0) {
        const double gap = t0 - g.lastT0;
        g.avgGapMs = g.avgGapMs > 0 ? g.avgGapMs * 0.9 + gap * 0.1 : gap;
    }
    g.lastT0 = t0;
    g.sum = sum;

    bool structural = axes.size() != g.axes.size();
    for (std::size_t i = 0; !structural && i < axes.size(); ++i)
        if (StaticChanged(axes[i], g.axes[i])) structural = true;
    const bool pageChanged = PageChanged(page, g.page);
    if (pageChanged) g.page = page;

    if (structural) {
        g.axes = axes;
        g.noText.assign(g.axes.size(), std::wstring());
        g.aliasText.assign(g.axes.size(), std::wstring());
        for (std::size_t i = 0; i < g.axes.size(); ++i) {
            g.noText[i] = Widen(g.axes[i].no);
            g.aliasText[i] = Widen(g.axes[i].alias);
        }
        if (g.list) GridSetRowCount(g.list, (int)g.axes.size());   // full render of the list
        g.sel = EffectiveSel();
        if (g.detail) { GridMarkAll(g.detail); GridFlush(g.detail); }
        if (g.lamps) { GridMarkAll(g.lamps); GridFlush(g.lamps); }
    } else {
        const int oldSel = g.sel;
        g.sel = EffectiveSel();
        const bool selMoved = g.sel != oldSel;
        bool detailDirty = selMoved || pageChanged, lampDirty = selMoved;
        for (std::size_t i = 0; i < axes.size(); ++i) {
            const MotorTestAxis& a = axes[i];
            MotorTestAxis& b = g.axes[i];
            const bool lv = ListValueChanged(a, b), dv = lv || DetailValueChanged(a, b), lp = LampChanged(a, b);
            if (!dv && !lp) continue;
            if ((int)i == g.sel) { detailDirty = detailDirty || dv; lampDirty = lampDirty || lp; }
            b = a;   // the static fields are equal: this copies the values
            if (lv && g.list) GridMarkRow(g.list, (int)i);
        }
        if (selMoved && g.list) {   // the red "selected" colour moves (golden ShowMotorSelect)
            if (oldSel >= 0) GridMarkRow(g.list, oldSel);
            if (g.sel >= 0) GridMarkRow(g.list, g.sel);
        }
        if (g.list) GridFlush(g.list);
        if (detailDirty && g.detail) { GridMarkAll(g.detail); GridFlush(g.detail); }   // only changed cells are painted
        if (lampDirty && g.lamps) { GridMarkAll(g.lamps); GridFlush(g.lamps); }
    }
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
}

void* MotorTestHwnd() { return g.hwnd; }

int MotorTestListCount() { return g.list ? GridRowCount(g.list) : -1; }

std::wstring MotorTestListText(int item, int column) { return g.list ? GridCellText(g.list, item, column) : L""; }

std::wstring MotorTestDetailText(int row) { return g.detail ? GridCellText(g.detail, row, 1) : L""; }

std::wstring MotorTestLampText(int led)
{
    const MotorTestAxis* a = Sel();
    if (!a || !a->ledKnown) return L"—";
    return MotorLedOn(LampRow(*a), led) ? L"1" : L"0";
}

int MotorTestSelected() { return g.sel; }

void MotorTestSelect(int listRow)
{
    RECT rc;
    if (!g.list || !GridCellRect(g.list, listRow, kMtColAlias, &rc)) return;
    ::SendMessageW(g.list, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(rc.left + 2, rc.top + 2));   // as the operator's click
}

int MotorTestButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.all;
}

int MotorTestEnabledButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.enabled;
}

void* MotorTestListGrid() { return g.list; }
void* MotorTestDetailGrid() { return g.detail; }
void* MotorTestLampGrid() { return g.lamps; }

std::wstring MotorTestSummaryText()
{
    if (!g.summary) return L"";
    wchar_t b[1400];
    b[0] = 0;
    ::GetWindowTextW(g.summary, b, 1400);
    return b;
}

double MotorTestLastUpdateMs() { return g.lastUpdateMs; }

}  // namespace w906native
