// ===========================================================================
//  ui/native/NativeIoView.cpp
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: HW.IoSetView 原生 Win32 視窗（唯讀原型）。說明在 NativeIoView.h。
//
//  為什麼是一張表，而不是復原 GA-4 引擎＋dfm2rc 的 iosetview.rc（20260928 晚的取捨，寫給 Steven／Jimmy）：
//    * GA-4 引擎（commit 7b86cfdf 刪掉的 ui/layout/*，約 2,800 行）當年是 MSVC 編的、有 CDialog 殼，
//      搬到 MinGW 6.3 沒驗證過（方案 §4 最後一列、§8）；iosetview.dfm 37,988 行，是六頁裡最大的對話框樹
//      （巢狀 DIALOGEX、Stack 分頁、HT9050 專用群組、LEDSqSmall 自訂控件）。一晚做不完，做一半反而看不出結論。
//    * Steven 要的是「先表列全部的 IO 與相關的狀態」—— 一張表正好是這句話，而且看得出三件真正要驗的事：
//      (1) 訊息泵掛在 wb_serve 主迴圈行不行（反應、1203 節拍）；(2) 資料不經 JSON、直接讀 C++；
//      (3) 輸出鈕停用是「構造上」做到的（本檔不連任何機台碼）。
//      之後要照 .dfm 版面，換掉的是本檔的「畫面」半邊；資料來源（IoRow）與泵的掛法不用動。
//
//  AI(W906-NATIVE-PROTO) 20260929 [W906]: 表格從 ListView 換成 NativeGrid（ui/native/NativeGrid.h）。
//    Steven 20260929：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」「這樣的閃爍是不被允許的」。
//    ListView（comctl32 v5，沒有雙緩衝）每次 LVM_REDRAWITEMS 都先擦白整列再畫，摘要 STATIC 也是 ⇒ 一直閃。
//    現在：只比較「值變了」的列、只重畫跟畫面上不一樣的格子、只把那些矩形從背景圖送上螢幕；摘要列是雙緩衝標籤。
//    每一拍的成本也降下來：靜態欄位（Alias、位址…）的寬字串在點表換了時才算一次，不再每一格 Widen；
//    篩選的小寫字串也預先算好。畫面效能（上次／最大更新毫秒、重畫格數、實際更新間隔）直接寫在摘要第三行。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // 與其他 ui/native 檔一致（本檔已不用 commctrl.h）
#endif
#include "ui/native/NativeIoView.h"
#include "ui/native/NativeGrid.h"   // 不閃的表格＋雙緩衝標籤
#include "ui/native/NativeHost.h"   // 泵、拖曳保活、字型、Widen（<windows.h>）

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

namespace {

const wchar_t kClassName[] = L"W906NativeIoView";
const int     kIdcFilter   = 1001;
const int     kIdcSearch   = 1002;
const int     kIdcOutBtn   = 1003;
const int     kIdcList     = 1004;
const int     kIdcTitle    = 1005;
const int     kIdcSummary  = 1006;
const int     kIdcNote     = 1007;
const int     kIdcLblFilt  = 1008;
const int     kIdcLblSrch  = 1009;

// 靜態欄位的寬字串：點表換了（structural）才重算，每一拍不 Widen、不配置。
struct RowText {
    std::wstring no, alias, type, code, lane, ip, port, bit, inType, enable;
    std::wstring aliasLower, codeLower;   // 搜尋用
};

struct ViewState {
    HWND  hwnd;
    HWND  title, summary, note, lblFilter, filter, lblSearch, search, outBtn, grid;
    std::vector<IoRow>   rows;
    std::vector<RowText> text;
    std::vector<int>     filtered;   // 顯示列 -> rows index
    std::vector<int>     pos;        // rows index -> 顯示列（-1＝篩掉了）
    std::vector<int>     scratch;    // 狀態篩選重算用
    std::vector<int>     changed;    // 這一拍值變了的 rows index
    IoSummary            sum;
    int                  filterMode;
    std::wstring         searchText;
    unsigned long        updateCount;
    unsigned long        redrawn;
    double               lastUpdateMs, maxUpdateMs, lastT0, avgGapMs;
    ViewState()
        : hwnd(0), title(0), summary(0), note(0), lblFilter(0), filter(0), lblSearch(0), search(0), outBtn(0), grid(0),
          filterMode(kFilterAll), updateCount(0), redrawn(0), lastUpdateMs(0), maxUpdateMs(0), lastT0(0), avgGapMs(0) {}
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

std::wstring NumText(int v)
{
    if (v < 0) return L"—";
    std::wstring s;
    GridSetInt(s, v);
    return s;
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
        const IoRow& r = g.rows[i];
        RowText& t = g.text[i];
        t.no = NumText(r.row);
        t.alias = Widen(r.alias);
        t.type = Widen(r.ioType);
        t.code = r.ioCode.empty() ? std::wstring(L"—") : Widen(r.ioCode);
        t.lane = NumText(r.lane);
        t.ip = NumText(r.ip);
        t.port = NumText(r.port);
        t.bit = NumText(r.bit);
        t.inType = NumText(r.inType);
        t.enable = NumText(r.enable);
        t.aliasLower = LowerW(t.alias);
        t.codeLower = LowerW(Widen(r.ioCode));
    }
}

bool PassFilter(std::size_t i)
{
    const IoRow& r = g.rows[i];
    switch (g.filterMode) {
    case kFilterIn:      if (r.dir != kIoDirIn) return false; break;
    case kFilterOut:     if (r.dir != kIoDirOut) return false; break;
    case kFilterOn:      if (r.isOn != 1) return false; break;
    case kFilterOff:     if (r.isOn != 0) return false; break;
    case kFilterUnknown: if (r.isOn >= 0) return false; break;
    default: break;
    }
    if (!g.searchText.empty()) {
        const RowText& t = g.text[i];
        if (t.aliasLower.find(g.searchText) == std::wstring::npos && t.codeLower.find(g.searchText) == std::wstring::npos) return false;
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

// 列整批換了（篩選、搜尋、點表）：整張重畫一次（仍然不閃）。
void RebuildFiltered()
{
    ComputeFiltered(g.filtered);
    RebuildPos();
    if (g.grid) GridSetRowCount(g.grid, (int)g.filtered.size());
}

const wchar_t* DirText(int dir)
{
    return dir == kIoDirIn ? L"輸入" : dir == kIoDirOut ? L"輸出" : L"？";
}

void SetQualityText(std::wstring& out, const std::string& q)
{
    if (q == "good") out = L"good";
    else if (q == "bad") out = L"bad 讀卡失敗";
    else if (q == "disabled") out = L"disabled 表上 Enable=0";
    else if (q == "nosource") out = L"nosource 沒有來源";
    else GridSetAscii(out, q);
}

// 燈號的樣子（顏色與旁邊的字）。null（沒有值）是白底灰框，不是 off。
void LedLook(const IoRow& r, GridCell& out)
{
    out.kind = kGridLedText;
    out.color = ::GetSysColor(COLOR_WINDOWTEXT);
    out.ledEdge = RGB(40, 40, 40);
    if (r.quality == "bad")           { out.ledFill = RGB(255, 190, 0);   out.text = L"bad"; }
    else if (r.quality == "disabled") { out.ledFill = RGB(215, 215, 215); out.ledEdge = RGB(150, 150, 150); out.text = L"停用"; }
    else if (r.isOn < 0)              { out.ledFill = RGB(255, 255, 255); out.ledEdge = RGB(150, 150, 150); out.text = L"null"; }
    else if (r.isOn == 1)             { out.ledFill = (r.dir == kIoDirOut) ? RGB(255, 70, 40) : RGB(0, 210, 60); out.text = L"ON"; }
    else                              { out.ledFill = RGB(60, 70, 60);    out.text = L"OFF"; }
}

// NativeGrid 的回呼：顯示列 row、欄 col 要畫什麼。只有「值變了」的列、或整張重畫時才會被問。
void IoCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.filtered.size()) return;
    const std::size_t i = (std::size_t)g.filtered[(std::size_t)row];
    const IoRow& r = g.rows[i];
    const RowText& t = g.text[i];
    if (r.isOn < 0) out.color = RGB(128, 128, 128);   // 沒有值的列用灰字，一眼分得出「null」與「off」
    switch (col) {
    case kColRow:     out.text = t.no; break;
    case kColLed:     LedLook(r, out); break;
    case kColDir:     out.text = DirText(r.dir); break;
    case kColAlias:   out.text = t.alias; break;
    case kColType:    out.text = t.type; break;
    case kColCode:    out.text = t.code; break;
    case kColLane:    out.text = t.lane; break;
    case kColIp:      out.text = t.ip; break;
    case kColPort:    out.text = t.port; break;
    case kColBit:     out.text = t.bit; break;
    case kColInType:  out.text = t.inType; break;
    case kColEnable:  out.text = t.enable; break;
    case kColRaw:     if (r.raw < 0) out.text = L"—"; else GridSetInt(out.text, r.raw); break;
    case kColQuality: SetQualityText(out.text, r.quality); break;
    case kColSource:  if (r.source.empty()) out.text = L"—"; else GridSetAscii(out.text, r.source); break;
    case kColOutBtn:  out.kind = kGridButtonOff; out.text = r.dir == kIoDirOut ? L"停用" : L""; break;
    default: break;
    }
}

void Layout()
{
    if (!g.hwnd) return;
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top;
    const int m = 8;
    ::MoveWindow(g.title,     m, 6, w - 2 * m, 22, TRUE);
    ::MoveWindow(g.note,      m, 30, w - 2 * m, 20, TRUE);
    ::MoveWindow(g.summary,   m, 52, w - 2 * m, 62, TRUE);   // 三行：摘要兩行＋畫面效能一行
    ::MoveWindow(g.lblFilter, m, 122, 60, 22, TRUE);
    ::MoveWindow(g.filter,    m + 62, 119, 190, 300, TRUE);
    ::MoveWindow(g.lblSearch, m + 266, 122, 150, 22, TRUE);
    ::MoveWindow(g.search,    m + 418, 119, 200, 24, TRUE);
    ::MoveWindow(g.outBtn,    m + 636, 118, 320, 26, TRUE);
    const int top = 150;
    ::MoveWindow(g.grid, m, top, w - 2 * m, (h - top - m) > 50 ? (h - top - m) : 50, TRUE);
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
    g.title = MakeChild(L"STATIC", L"HW.IoSetView — 原生 Win32 視窗（唯讀原型 DEMO，20260929 不閃版）", SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"只看燈號：輸出鈕全部停用；本視窗的程式不連任何 IO 寫入／motor.access／1203 指令。資料直接讀 C++（不經 JSON），每 20 ms 比對一次、只重畫變了的格子。拖曳視窗時主迴圈由視窗計時器保活。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"（等待第一批資料）");   // 雙緩衝：每一拍換字也不閃
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    g.lblFilter = MakeChild(L"STATIC", L"篩選：", SS_LEFT, kIdcLblFilt);
    g.filter = MakeChild(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, kIdcFilter);
    const wchar_t* items[] = {L"全部", L"只看輸入", L"只看輸出", L"只看 ON", L"只看 OFF", L"只看 null（沒有值）"};
    for (int i = 0; i < 6; ++i) ::SendMessageW(g.filter, CB_ADDSTRING, 0, (LPARAM)items[i]);
    ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)g.filterMode, 0);
    g.lblSearch = MakeChild(L"STATIC", L"搜尋 Alias／位址：", SS_LEFT, kIdcLblSrch);
    g.search = MakeChild(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, kIdcSearch, WS_EX_CLIENTEDGE);
    // 唯一一顆真的按鈕：永遠 WS_DISABLED（1b 階段才會接 IoBtnPanelClick.cpp 的同一本體）。本檔沒有它的 WM_COMMAND 處理。
    g.outBtn = MakeChild(L"BUTTON", L"輸出操作（1b 階段才開放，現在停用）", BS_PUSHBUTTON | WS_DISABLED, kIdcOutBtn);

    g.grid = GridCreate(g.hwnd, kIdcList, &IoCell, 0);
    if (g.grid) ::SendMessageW(g.grid, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> c((std::size_t)kColCount);
    c[kColRow]     = GridColumn(L"列",        44, kGridRight);
    c[kColLed]     = GridColumn(L"燈號",      76, kGridLeft);
    c[kColDir]     = GridColumn(L"方向",      48, kGridLeft);
    c[kColAlias]   = GridColumn(L"Alias",    200, kGridLeft);
    c[kColType]    = GridColumn(L"IOType",   100, kGridLeft);
    c[kColCode]    = GridColumn(L"位址",      76, kGridLeft);
    c[kColLane]    = GridColumn(L"Lane",      52, kGridRight);
    c[kColIp]      = GridColumn(L"站號(IP)",  78, kGridRight);
    c[kColPort]    = GridColumn(L"Port",      50, kGridRight);
    c[kColBit]     = GridColumn(L"Bit",       40, kGridRight);
    c[kColInType]  = GridColumn(L"InType",    64, kGridRight);
    c[kColEnable]  = GridColumn(L"Enable",    64, kGridRight);
    c[kColRaw]     = GridColumn(L"Raw",       48, kGridRight);
    c[kColQuality] = GridColumn(L"品質",     150, kGridLeft);
    c[kColSource]  = GridColumn(L"來源",      92, kGridLeft);
    c[kColOutBtn]  = GridColumn(L"輸出鈕",    86, kGridCenter);
    GridSetColumns(g.grid, c, std::vector<int>());
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
        // 只處理篩選與搜尋（純畫面狀態）。刻意沒有 kIdcOutBtn 的分支：那顆鈕是 WS_DISABLED，也沒有任何輸出本體可呼叫。
        const int id = LOWORD(wp), code = HIWORD(wp);
        if (id == kIdcFilter && code == CBN_SELCHANGE) {
            const LRESULT s = ::SendMessageW(g.filter, CB_GETCURSEL, 0, 0);
            g.filterMode = (s >= 0 && s <= kFilterUnknown) ? (int)s : kFilterAll;
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
        g.hwnd = g.title = g.summary = g.note = g.lblFilter = g.filter = g.lblSearch = g.search = g.outBtn = g.grid = 0;
        // 不 PostQuitMessage：這條執行緒是 wb_serve 主迴圈，關掉 IO 視窗不等於關程式。
        break;
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
    int* c = reinterpret_cast<int*>(lp);   // c[0] = total, c[1] = enabled
    wchar_t cls[32];
    if (::GetClassNameW(h, cls, 32) > 0 && ::lstrcmpiW(cls, L"Button") == 0) {
        ++c[0];
        if (::IsWindowEnabled(h)) ++c[1];
    }
    return TRUE;
}

std::wstring BuildSummary()
{
    int nOn = 0, nOff = 0, nNull = 0, nGood = 0, nBad = 0, nNoSrc = 0, nDis = 0, nIn = 0, nOut = 0;
    for (std::size_t i = 0; i < g.rows.size(); ++i) {
        const IoRow& r = g.rows[i];
        if (r.isOn == 1) ++nOn; else if (r.isOn == 0) ++nOff; else ++nNull;
        if (r.quality == "good") ++nGood; else if (r.quality == "bad") ++nBad;
        else if (r.quality == "disabled") ++nDis; else ++nNoSrc;
        if (r.dir == kIoDirIn) ++nIn; else if (r.dir == kIoDirOut) ++nOut;
    }
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats gs = GridGetStats(g.grid);
    // 窄字串組（本檔是 UTF-8，字面值就是 UTF-8 位元組），最後一次 Widen —— MinGW.org 沒有可靠的 std::swprintf。
    // 三行（標籤不自動換行，只重畫變了的那一行）：點數｜監看與更新時間（＋沒有連線的原因）｜畫面效能。
    const std::string why = (!g.sum.connected && !g.sum.why.empty()) ? "｜" + g.sum.why : std::string();
    char b[1400];
    std::snprintf(b, sizeof(b),
                  "[%s] %s｜共 %u 點（輸入 %d／輸出 %d）｜ON %d  OFF %d  null %d｜good %d  bad %d  nosource %d  disabled %d\n"
                  "監看 poll #%lu（錯 %lu）｜略過無 Alias %d 列｜拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u%s\n"
                  "畫面效能：上次更新 %.2f ms（最大 %.2f ms）｜實際更新間隔約 %.0f ms｜累計重畫 %lu 格、整張重畫 %lu 次（上次 %.1f ms）",
                  g.sum.buildConfig.empty() ? "?" : g.sum.buildConfig.c_str(),
                  g.sum.connected ? "1203 監看器：已連線" : "1203 監看器：未連線（每一點都是 null，不是 off）",
                  (unsigned)g.rows.size(), nIn, nOut, nOn, nOff, nNull, nGood, nBad, nNoSrc, nDis,
                  g.sum.pollCount, g.sum.pollErrors, g.sum.skippedNoAlias, g.sum.keepaliveCalls,
                  (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned)st.wMilliseconds, why.c_str(),
                  g.lastUpdateMs, g.maxUpdateMs, g.avgGapMs, gs.cellsPainted, gs.fullRenders, gs.lastFullMs);
    return Widen(b);
}

bool StaticChanged(const IoRow& a, const IoRow& b)
{
    return a.row != b.row || a.dir != b.dir || a.isaBase != b.isaBase || a.lane != b.lane || a.ip != b.ip ||
           a.port != b.port || a.bit != b.bit || a.inType != b.inType || a.enable != b.enable ||
           a.alias != b.alias || a.ioType != b.ioType || a.ioCode != b.ioCode;
}

}  // namespace

bool IoViewOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — HW.IoSetView（原生唯讀原型）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1420, 880,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    // WM_CREATE 已經把 g.hwnd 設好、子視窗建好。
    RebuildFiltered();
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void IoViewClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool IoViewIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void IoViewUpdate(const std::vector<IoRow>& rows, const IoSummary& sum)
{
    if (!IoViewIsOpen()) return;
    const double t0 = NowMs();
    if (g.lastT0 > 0) {
        const double gap = t0 - g.lastT0;
        g.avgGapMs = g.avgGapMs > 0 ? g.avgGapMs * 0.9 + gap * 0.1 : gap;
    }
    g.lastT0 = t0;
    ++g.updateCount;
    bool structural = rows.size() != g.rows.size();
    for (std::size_t i = 0; !structural && i < rows.size(); ++i)
        if (StaticChanged(rows[i], g.rows[i])) structural = true;
    if (structural) {
        g.rows = rows;
        BuildTexts();
        RebuildFiltered();
    } else {
        // 只複製「值」欄位，而且只有變了的列（字串一樣就不指定，不配置記憶體）。
        g.changed.clear();
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const IoRow& a = rows[i];
            IoRow& b = g.rows[i];
            if (a.isOn == b.isOn && a.raw == b.raw && a.quality == b.quality && a.source == b.source) continue;
            b.isOn = a.isOn;
            b.raw = a.raw;
            if (b.quality != a.quality) b.quality = a.quality;
            if (b.source != a.source) b.source = a.source;
            g.changed.push_back((int)i);
        }
        if (!g.changed.empty() && g.grid) {
            bool reflow = false;
            if (g.filterMode == kFilterOn || g.filterMode == kFilterOff || g.filterMode == kFilterUnknown) {
                // 狀態篩選：有列進出篩選才整張重排；沒有就照樣只畫變了的格子。
                ComputeFiltered(g.scratch);
                if (g.scratch != g.filtered) {
                    g.filtered.swap(g.scratch);
                    RebuildPos();
                    GridSetRowCount(g.grid, (int)g.filtered.size());
                    reflow = true;
                }
            }
            if (!reflow) {
                const unsigned long before = GridGetStats(g.grid).rowsChecked;
                for (std::size_t k = 0; k < g.changed.size(); ++k) {
                    const int p = g.pos[(std::size_t)g.changed[k]];
                    if (p >= 0) GridMarkRow(g.grid, p);
                }
                GridFlush(g.grid);
                g.redrawn += GridGetStats(g.grid).rowsChecked - before;
            }
        }
    }
    g.sum = sum;
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
    if (g.lastUpdateMs > g.maxUpdateMs) g.maxUpdateMs = g.lastUpdateMs;
}

void* IoViewHwnd() { return g.hwnd; }

int IoViewListCount()
{
    if (!g.grid) return -1;
    return GridRowCount(g.grid);
}

std::wstring IoViewCellText(int item, int column)
{
    if (!g.grid) return L"";
    // 經表格控件的視窗程序問回呼（GridCellText 送訊息），所以同時驗了「訊息路由通」，不是直接讀 g.rows。
    return GridCellText(g.grid, item, column);
}

int IoViewLedState(int item)
{
    const std::wstring t = IoViewCellText(item, kColLed);
    if (t == L"ON") return 1;
    if (t == L"OFF") return 0;
    return -1;
}

int IoViewEnabledButtonCount()
{
    if (!g.hwnd) return -1;
    int c[2] = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)c);
    return c[1];
}

int IoViewButtonCount()
{
    if (!g.hwnd) return -1;
    int c[2] = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)c);
    return c[0];
}

void IoViewSetFilter(int filter)
{
    if (!g.filter) return;
    ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)filter, 0);
    // CB_SETCURSEL 不會送 CBN_SELCHANGE —— 照使用者操作的路徑補送，走同一個 WM_COMMAND 分支。
    ::SendMessageW(g.hwnd, WM_COMMAND, MAKEWPARAM(kIdcFilter, CBN_SELCHANGE), (LPARAM)g.filter);
}

void IoViewSetSearch(const std::wstring& text)
{
    if (!g.search) return;
    ::SetWindowTextW(g.search, text.c_str());   // edit 控件自己送 EN_CHANGE
}

std::wstring IoViewSummaryText()
{
    if (!g.summary) return L"";
    wchar_t b[1400];
    b[0] = 0;
    ::GetWindowTextW(g.summary, b, 1400);
    return b;
}

unsigned long IoViewUpdateCount() { return g.updateCount; }
unsigned long IoViewRedrawnItems() { return g.redrawn; }
void*         IoViewGridHwnd() { return g.grid; }
double        IoViewLastUpdateMs() { return g.lastUpdateMs; }
double        IoViewMaxUpdateMs() { return g.maxUpdateMs; }

}  // namespace w906native
