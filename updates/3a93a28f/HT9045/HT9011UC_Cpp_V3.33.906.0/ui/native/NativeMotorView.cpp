// ===========================================================================
//  ui/native/NativeMotorView.cpp
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: Main.MotorView 原生 Win32 視窗（唯讀原型）。說明在 NativeMotorView.h。
//  形狀與 NativeIoView.cpp 相同，差別：沒有任何按鈕；燈號欄畫 Motor Test 的十顆燈。
//
//  AI(W906-NATIVE-PROTO) 20260929 [W906]: 表格從 ListView 換成 NativeGrid（不閃、只畫變了的格子），摘要改雙緩衝標籤。
//    Steven 20260929：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」「這樣的閃爍是不被允許的」。理由同 NativeIoView.cpp 檔頭。
//    位置每 20 ms 變一次時，每一軸只重畫「目前位置／目標位置」那兩格（其他格跟畫面上一樣就不畫）。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // 與其他 ui/native 檔一致（本檔已不用 commctrl.h）
#endif
#include "ui/native/NativeMotorView.h"
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

bool MotorLedOn(const MotorRow& r, int led)
{
    if (!r.ledKnown) return false;
    const unsigned long io = r.motionIO;
    switch (led) {
    case kLedCw:      return (io & 0x00000008ul) != 0;                          // LMT-
    case kLedHome:    return (io & 0x00000010ul) != 0;                          // ORG
    case kLedCcw:     return (io & 0x00000004ul) != 0;                          // LMT+
    case kLedEmg:     return (io & 0x00000040ul) != 0;                          // EMG
    case kLedAlarm:   return (io & 0x00000002ul) != 0 || (r.state & 0xFFu) == 3u;   // ALM 或 ERROR_STOP
    case kLedSoftCw:  return (io & 0x00010000ul) != 0;                          // SLMT_P
    case kLedSoftCcw: return (io & 0x00020000ul) != 0;                          // SLMT_N
    case kLedSAlarm:  return false;                                             // golden 對 1203 軸不設
    case kLedInPos:   return false;                                             // golden 註解掉（"RogerYang 20250421 not work"）
    case kLedServo:   return (io & 0x00004000ul) != 0;                          // SVON
    default:          return false;
    }
}

namespace {

const wchar_t kClassName[] = L"W906NativeMotorView";
const int kIdcTitle = 2001, kIdcNote = 2002, kIdcSummary = 2003, kIdcLblFilt = 2004, kIdcFilter = 2005,
          kIdcLblSrch = 2006, kIdcSearch = 2007, kIdcList = 2008;
const wchar_t* const kLedNames[kLedCount] = {L"CW", L"HOME", L"CCW", L"EMG", L"ALM", L"SCW", L"SCCW", L"SALM", L"INP", L"SVON"};

// 靜態欄位的寬字串：馬達表換了才重算；why（說明）只在 errText 變了時重算。
struct RowText {
    std::wstring row, alias, no, card, station, why;
    std::wstring aliasLower, noLower;
};

struct ViewState {
    HWND hwnd, title, note, summary, lblFilter, filter, lblSearch, search, grid;
    std::vector<MotorRow> rows;
    std::vector<RowText>  text;
    std::vector<int>      filtered, pos, scratch, changed;
    MotorSummary          sum;
    int                   filterMode;
    std::wstring          searchText;
    double                lastUpdateMs, maxUpdateMs, lastT0, avgGapMs;
    ViewState() : hwnd(0), title(0), note(0), summary(0), lblFilter(0), filter(0), lblSearch(0), search(0), grid(0),
                  filterMode(kMotFilterGolden), lastUpdateMs(0), maxUpdateMs(0), lastT0(0), avgGapMs(0) {}
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

void SetTri(std::wstring& out, int v, const wchar_t* on, const wchar_t* off)
{
    out = v < 0 ? L"—" : (v ? on : off);
}

std::wstring LowerW(std::wstring s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] >= L'A' && s[i] <= L'Z') s[i] = (wchar_t)(s[i] - L'A' + L'a');
    return s;
}

void BuildTexts()
{
    g.text.assign(g.rows.size(), RowText());
    for (std::size_t i = 0; i < g.rows.size(); ++i) {
        const MotorRow& r = g.rows[i];
        RowText& t = g.text[i];
        GridSetInt(t.row, r.row);
        t.alias = Widen(r.alias);
        t.no = Widen(r.no);
        t.card = Widen(r.cardModel);
        if (r.boardId < 0) t.station = L"—";
        else {
            char b[32];
            std::snprintf(b, sizeof(b), "%d/%d", r.boardId, r.port);
            t.station = Widen(b);
        }
        t.why = Widen(r.errText);
        t.aliasLower = LowerW(t.alias);
        t.noLower = LowerW(t.no);
    }
}

bool PassFilter(std::size_t i)
{
    const MotorRow& r = g.rows[i];
    switch (g.filterMode) {
    case kMotFilterGolden: if (!r.mtVisible) return false; break;
    case kMotFilter1203:   if (r.cardModel != "PCI1203") return false; break;
    case kMotFilterAlarm:  if (r.alarm != 1 && r.homeFlag != 2) return false; break;
    case kMotFilterNull:   if (r.quality != "nosource") return false; break;
    default: break;
    }
    if (!g.searchText.empty()) {
        const RowText& t = g.text[i];
        if (t.aliasLower.find(g.searchText) == std::wstring::npos && t.noLower.find(g.searchText) == std::wstring::npos) return false;
    }
    return true;
}

void ComputeFiltered(std::vector<int>& out)
{
    out.clear();
    for (std::size_t i = 0; i < g.rows.size(); ++i)
        if (PassFilter(i)) out.push_back((int)i);
}

void RebuildPos()
{
    g.pos.assign(g.rows.size(), -1);
    for (std::size_t k = 0; k < g.filtered.size(); ++k) g.pos[(std::size_t)g.filtered[k]] = (int)k;
}

void RebuildFiltered()
{
    ComputeFiltered(g.filtered);
    RebuildPos();
    if (g.grid) GridSetRowCount(g.grid, (int)g.filtered.size());
}

void SetLedText(std::wstring& out, const MotorRow& r)
{
    if (!r.ledKnown) { out = L"null"; return; }
    out.clear();
    for (int k = 0; k < kLedCount; ++k)
        if (MotorLedOn(r, k)) { if (!out.empty()) out += L' '; out += kLedNames[k]; }
    if (out.empty()) out = L"（全暗）";
}

void SetHomeText(std::wstring& out, int f)
{
    switch (f) {
    case 0: out = L"0 未歸零"; break;
    case 1: out = L"1 完成"; break;
    case 2: out = L"2 失敗"; break;
    default: if (f < 0) out = L"—"; else GridSetInt(out, f); break;
    }
}

// NativeGrid 的回呼。只有「值變了」的列、或整張重畫時才會被問。
void MotorCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.filtered.size()) return;
    const std::size_t i = (std::size_t)g.filtered[(std::size_t)row];
    const MotorRow& r = g.rows[i];
    const RowText& t = g.text[i];
    if (r.quality == "nosource") out.color = RGB(128, 128, 128);
    switch (col) {
    case kMColRow:     out.text = t.row; break;
    case kMColAlias:   out.text = t.alias; break;
    case kMColNo:      out.text = t.no; break;
    case kMColCur:     SetNum(out.text, r.hasCur, r.cur); break;
    case kMColTarget:  SetNum(out.text, r.hasTarget, r.target); break;
    case kMColSpeed:   SetNum(out.text, r.hasSpeed, r.speed); break;
    case kMColCan:     SetNum(out.text, r.can >= 0, r.can); break;
    case kMColL:       SetNum(out.text, r.canL >= 0, r.canL); break;
    case kMColM:       SetNum(out.text, r.canM >= 0, r.canM); break;
    case kMColR:       SetNum(out.text, r.canR >= 0, r.canR); break;
    case kMColLeds:    SetLedText(out.text, r); break;
    case kMColServo:   SetTri(out.text, r.servoOn, L"ON", L"OFF"); break;
    case kMColAlarm:   SetTri(out.text, r.alarm, L"警報", L"正常"); if (r.alarm == 1) out.color = RGB(210, 0, 0); break;
    case kMColInPos:   SetTri(out.text, r.inPos, L"到位", L"未到"); break;
    case kMColBusy:    SetTri(out.text, r.busy, L"忙", L"閒"); break;
    case kMColHome:    SetHomeText(out.text, r.homeFlag); if (r.homeFlag == 2) out.color = RGB(210, 0, 0); break;
    case kMColCard:    out.text = t.card; break;
    case kMColStation: out.text = t.station; break;
    case kMColQuality: GridSetAscii(out.text, r.quality); break;
    case kMColSource:  GridSetAscii(out.text, r.source); break;
    case kMColWhy:     out.text = t.why; break;
    default:
        if (col >= kMColLamp0 && col < kMColLamp0 + kLedCount) {
            const int k = col - kMColLamp0;
            out.kind = kGridLed;
            out.ledEdge = RGB(40, 40, 40);
            if (!r.ledKnown) { out.ledFill = RGB(255, 255, 255); out.ledEdge = RGB(150, 150, 150); out.text = L"—"; }   // null：白底灰框，不是「暗」
            else if (MotorLedOn(r, k)) { out.ledFill = (k == kLedAlarm || k == kLedEmg) ? RGB(255, 60, 40) : RGB(0, 210, 60); out.text = L"1"; }
            else { out.ledFill = RGB(60, 70, 60); out.text = L"0"; }
        }
        break;
    }
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
    g.title = MakeChild(L"STATIC", L"Main.MotorView — 原生 Win32 視窗（唯讀原型 DEMO，20260929 不閃版）", SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"golden 主畫面 MotionView 分頁的馬達表（UpdateMotorScreen：Alias／目前位置／目標位置／速度／Can L M R）＋補充欄。"
                       L"沒有任何運動指令：本視窗沒有按鈕，程式不連 motor.access。每 20 ms 比對一次、只重畫變了的格子。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"（等待第一批資料）");   // 雙緩衝：每一拍換字也不閃
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    g.lblFilter = MakeChild(L"STATIC", L"篩選：", SS_LEFT, kIdcLblFilt);
    g.filter = MakeChild(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, kIdcFilter);
    const wchar_t* items[] = {L"golden 會列的（Motor Test 可見）", L"馬達表全部", L"只看 PCI1203", L"只看警報／歸零失敗", L"只看沒有值的"};
    for (int i = 0; i < 5; ++i) ::SendMessageW(g.filter, CB_ADDSTRING, 0, (LPARAM)items[i]);
    ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)g.filterMode, 0);
    g.lblSearch = MakeChild(L"STATIC", L"搜尋 Alias／No：", SS_LEFT, kIdcLblSrch);
    g.search = MakeChild(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, kIdcSearch, WS_EX_CLIENTEDGE);

    g.grid = GridCreate(g.hwnd, kIdcList, &MotorCell, 0);
    if (g.grid) ::SendMessageW(g.grid, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> c((std::size_t)kMColCount);
    c[kMColRow]     = GridColumn(L"列",        40, kGridRight);
    c[kMColAlias]   = GridColumn(L"Alias",    150, kGridLeft);
    c[kMColNo]      = GridColumn(L"No",        46, kGridLeft);
    c[kMColCur]     = GridColumn(L"目前位置",  84, kGridRight);
    c[kMColTarget]  = GridColumn(L"目標位置",  84, kGridRight);
    c[kMColSpeed]   = GridColumn(L"速度",      70, kGridRight);
    c[kMColCan]     = GridColumn(L"Can",       46, kGridCenter);
    c[kMColL]       = GridColumn(L"L",         30, kGridCenter);
    c[kMColM]       = GridColumn(L"M",         30, kGridCenter);
    c[kMColR]       = GridColumn(L"R",         30, kGridCenter);
    c[kMColLeds]    = GridColumn(L"亮的燈",   110, kGridLeft);
    c[kMColServo]   = GridColumn(L"伺服",      48, kGridLeft);
    c[kMColAlarm]   = GridColumn(L"警報",      48, kGridLeft);
    c[kMColInPos]   = GridColumn(L"到位",      52, kGridLeft);
    c[kMColBusy]    = GridColumn(L"忙碌",      58, kGridLeft);
    c[kMColHome]    = GridColumn(L"HomeFlag",  74, kGridLeft);
    c[kMColCard]    = GridColumn(L"卡",        78, kGridLeft);
    c[kMColStation] = GridColumn(L"站/軸",     52, kGridLeft);
    c[kMColQuality] = GridColumn(L"品質",      64, kGridLeft);
    c[kMColSource]  = GridColumn(L"來源",     112, kGridLeft);
    c[kMColWhy]     = GridColumn(L"說明",     320, kGridLeft);
    for (int k = 0; k < kLedCount; ++k)
        c[(std::size_t)(kMColLamp0 + k)] = GridColumn(kLedNames[k], k == kLedSoftCcw || k == kLedHome || k == kLedSAlarm || k == kLedServo ? 60 : 50, kGridCenter);
    // 畫面順序：golden 八欄 → 十顆燈（golden Motor Test ALed1..10 的順序）→ 其餘補充欄。
    std::vector<int> order;
    for (int k = kMColRow; k <= kMColR; ++k) order.push_back(k);
    for (int k = 0; k < kLedCount; ++k) order.push_back(kMColLamp0 + k);
    for (int k = kMColLeds; k <= kMColWhy; ++k) order.push_back(k);
    GridSetColumns(g.grid, c, order);
}

void Layout()
{
    if (!g.hwnd) return;
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top, m = 8;
    ::MoveWindow(g.title,     m, 6, w - 2 * m, 22, TRUE);
    ::MoveWindow(g.note,      m, 30, w - 2 * m, 20, TRUE);
    ::MoveWindow(g.summary,   m, 52, w - 2 * m, 62, TRUE);   // 三行：摘要兩行＋畫面效能一行
    ::MoveWindow(g.lblFilter, m, 122, 60, 22, TRUE);
    ::MoveWindow(g.filter,    m + 62, 119, 280, 300, TRUE);
    ::MoveWindow(g.lblSearch, m + 356, 122, 140, 22, TRUE);
    ::MoveWindow(g.search,    m + 498, 119, 200, 24, TRUE);
    const int top = 150;
    ::MoveWindow(g.grid, m, top, w - 2 * m, (h - top - m) > 50 ? (h - top - m) : 50, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (HostWindowMessage(hwnd, msg, wp, lp)) return 0;   // 保活計時器（NativeHost.h）
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
        mm->ptMinTrackSize.x = 900;
        mm->ptMinTrackSize.y = 400;
        return 0;
    }
    case WM_MOUSEWHEEL:   // 焦點在篩選／搜尋框時，滾輪也捲表格
        if (g.grid) return ::SendMessageW(g.grid, msg, wp, lp);
        break;
    case WM_COMMAND: {
        // 只有篩選與搜尋（純畫面狀態）。本視窗沒有任何按鈕。
        const int id = LOWORD(wp), code = HIWORD(wp);
        if (id == kIdcFilter && code == CBN_SELCHANGE) {
            const LRESULT s = ::SendMessageW(g.filter, CB_GETCURSEL, 0, 0);
            g.filterMode = (s >= 0 && s <= kMotFilterNull) ? (int)s : kMotFilterGolden;
            RebuildFiltered();
        } else if (id == kIdcSearch && code == EN_CHANGE) {
            wchar_t buf[128];
            ::GetWindowTextW(g.search, buf, 128);
            g.searchText = LowerW(buf);
            RebuildFiltered();
        }
        return 0;
    }
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HostWindowDestroyed(hwnd);
        break;
    case WM_NCDESTROY:
        g.hwnd = g.title = g.note = g.summary = g.lblFilter = g.filter = g.lblSearch = g.search = g.grid = 0;
        break;   // 不 PostQuitMessage：這條執行緒是 wb_serve 主迴圈
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

BOOL CALLBACK CountButtons(HWND h, LPARAM lp)
{
    int* c = reinterpret_cast<int*>(lp);
    wchar_t cls[32];
    if (::GetClassNameW(h, cls, 32) > 0 && ::lstrcmpiW(cls, L"Button") == 0) ++*c;
    return TRUE;
}

std::wstring BuildSummary()
{
    int nShown = 0, nGood = 0, nPartial = 0, nNull = 0, nAlarm = 0, nHomeFail = 0;
    for (std::size_t i = 0; i < g.rows.size(); ++i) {
        const MotorRow& r = g.rows[i];
        if (r.mtVisible) ++nShown;
        if (r.quality == "good") ++nGood; else if (r.quality == "partial") ++nPartial; else ++nNull;
        if (r.alarm == 1) ++nAlarm;
        if (r.homeFlag == 2) ++nHomeFail;
    }
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats gs = GridGetStats(g.grid);
    // 三行（標籤不自動換行，只重畫變了的那一行）：軸數｜監看與更新時間（＋一軸都沒有值的原因）｜畫面效能。
    const std::string why = g.sum.why.empty() ? std::string() : "｜" + g.sum.why;
    char b[1400];
    std::snprintf(b, sizeof(b),
                  "[%s] 馬達表 %u 軸（golden 會列 %d）｜good %d  partial %d  沒有值 %d｜警報 %d  歸零失敗 %d\n"
                  "1203 監看器：%s（開著 %d 軸，poll #%lu）｜拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u%s\n"
                  "畫面效能：上次更新 %.2f ms（最大 %.2f ms）｜實際更新間隔約 %.0f ms｜累計重畫 %lu 格、整張重畫 %lu 次（上次 %.1f ms）",
                  g.sum.buildConfig.empty() ? "?" : g.sum.buildConfig.c_str(), (unsigned)g.rows.size(), nShown,
                  nGood, nPartial, nNull, nAlarm, nHomeFail, g.sum.monitorOpen ? "已開卡" : "沒有開卡",
                  g.sum.monitorAxes, g.sum.pollCount, g.sum.keepaliveCalls,
                  (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned)st.wMilliseconds, why.c_str(),
                  g.lastUpdateMs, g.maxUpdateMs, g.avgGapMs, gs.cellsPainted, gs.fullRenders, gs.lastFullMs);
    return Widen(b);
}

bool StaticChanged(const MotorRow& a, const MotorRow& b)
{
    return a.row != b.row || a.mtVisible != b.mtVisible || a.mtOrder != b.mtOrder || a.motIndex != b.motIndex ||
           a.enable != b.enable || a.boardId != b.boardId || a.port != b.port ||
           a.alias != b.alias || a.no != b.no || a.cardModel != b.cardModel;
}

bool ValueChanged(const MotorRow& a, const MotorRow& b)
{
    return a.hasCur != b.hasCur || a.cur != b.cur || a.hasTarget != b.hasTarget || a.target != b.target ||
           a.hasSpeed != b.hasSpeed || a.speed != b.speed || a.can != b.can || a.canL != b.canL ||
           a.canM != b.canM || a.canR != b.canR || a.servoOn != b.servoOn || a.alarm != b.alarm ||
           a.inPos != b.inPos || a.busy != b.busy || a.homeFlag != b.homeFlag || a.ledKnown != b.ledKnown ||
           a.motionIO != b.motionIO || a.state != b.state || a.quality != b.quality || a.source != b.source ||
           a.errText != b.errText;
}

}  // namespace

bool MotorViewOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — Main.MotorView（原生唯讀原型）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1500, 760,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    RebuildFiltered();
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void MotorViewClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool MotorViewIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void MotorViewUpdate(const std::vector<MotorRow>& rows, const MotorSummary& sum)
{
    if (!MotorViewIsOpen()) return;
    const double t0 = NowMs();
    if (g.lastT0 > 0) {
        const double gap = t0 - g.lastT0;
        g.avgGapMs = g.avgGapMs > 0 ? g.avgGapMs * 0.9 + gap * 0.1 : gap;
    }
    g.lastT0 = t0;
    bool structural = rows.size() != g.rows.size();
    for (std::size_t i = 0; !structural && i < rows.size(); ++i)
        if (StaticChanged(rows[i], g.rows[i])) structural = true;
    if (structural) {
        g.rows = rows;
        BuildTexts();
        RebuildFiltered();
    } else {
        g.changed.clear();
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const MotorRow& a = rows[i];
            MotorRow& b = g.rows[i];
            if (!ValueChanged(a, b)) continue;
            if (a.errText != b.errText) g.text[i].why = Widen(a.errText);
            // 靜態欄位一樣：整筆指定只會複製值欄位的內容（字串一樣時 std::string 指定不配置）。
            b.hasCur = a.hasCur; b.cur = a.cur; b.hasTarget = a.hasTarget; b.target = a.target;
            b.hasSpeed = a.hasSpeed; b.speed = a.speed; b.can = a.can; b.canL = a.canL; b.canM = a.canM; b.canR = a.canR;
            b.servoOn = a.servoOn; b.alarm = a.alarm; b.inPos = a.inPos; b.busy = a.busy; b.homeFlag = a.homeFlag;
            b.ledKnown = a.ledKnown; b.motionIO = a.motionIO; b.state = a.state;
            if (b.quality != a.quality) b.quality = a.quality;
            if (b.source != a.source) b.source = a.source;
            if (b.errText != a.errText) b.errText = a.errText;
            g.changed.push_back((int)i);
        }
        if (!g.changed.empty() && g.grid) {
            bool reflow = false;
            if (g.filterMode == kMotFilterAlarm || g.filterMode == kMotFilterNull) {
                ComputeFiltered(g.scratch);
                if (g.scratch != g.filtered) {
                    g.filtered.swap(g.scratch);
                    RebuildPos();
                    GridSetRowCount(g.grid, (int)g.filtered.size());
                    reflow = true;
                }
            }
            if (!reflow) {
                for (std::size_t k = 0; k < g.changed.size(); ++k) {
                    const int p = g.pos[(std::size_t)g.changed[k]];
                    if (p >= 0) GridMarkRow(g.grid, p);
                }
                GridFlush(g.grid);
            }
        }
    }
    g.sum = sum;
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
    if (g.lastUpdateMs > g.maxUpdateMs) g.maxUpdateMs = g.lastUpdateMs;
}

void* MotorViewHwnd() { return g.hwnd; }

int MotorViewListCount()
{
    if (!g.grid) return -1;
    return GridRowCount(g.grid);
}

std::wstring MotorViewCellText(int item, int column)
{
    if (!g.grid) return L"";
    return GridCellText(g.grid, item, column);   // 經表格控件的視窗程序問回呼
}

int MotorViewButtonCount()
{
    if (!g.hwnd) return -1;
    int c = 0;
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c;
}

void MotorViewSetFilter(int filter)
{
    if (!g.filter) return;
    ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)filter, 0);
    ::SendMessageW(g.hwnd, WM_COMMAND, MAKEWPARAM(kIdcFilter, CBN_SELCHANGE), (LPARAM)g.filter);
}

void MotorViewSetSearch(const std::wstring& text)
{
    if (g.search) ::SetWindowTextW(g.search, text.c_str());
}

std::wstring MotorViewSummaryText()
{
    if (!g.summary) return L"";
    wchar_t b[1400];
    b[0] = 0;
    ::GetWindowTextW(g.summary, b, 1400);
    return b;
}

void*  MotorViewGridHwnd() { return g.grid; }
double MotorViewLastUpdateMs() { return g.lastUpdateMs; }
double MotorViewMaxUpdateMs() { return g.maxUpdateMs; }

}  // namespace w906native
