// ===========================================================================
//  ui/native/NativeGrid.cpp
//
//  AI(W906-NATIVE-PROTO) 20260929 [W906]: 不閃、只畫變了的格子的表格控件＋雙緩衝標籤。說明在 NativeGrid.h。
//  只 include <windows.h> 與 STL，不含任何機台碼。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // MinGW.org 6.3：見 NativeIoView.cpp 檔頭
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"   // Widen

#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace w906native {

namespace {

const wchar_t kGridClass[]  = L"W906NativeGrid";
const wchar_t kLabelClass[] = L"W906NativeLabel";
const UINT    kMsgCellText  = WM_APP + 0x2E1;   // wp = MAKEWPARAM(row, col)，lp = std::wstring*（GridCellText）
const int     kPad          = 4;
const COLORREF kGridLine    = RGB(224, 224, 224);

// 筆刷／畫筆快取：顏色種類很少（十幾種），整個行程共用、不刪（同 NativeHost.cpp 的 HostFont）。每格不必 Create／Delete。
std::map<COLORREF, HBRUSH> g_brushes;
std::map<COLORREF, HPEN>   g_pens;

HBRUSH Brush(COLORREF c)
{
    std::map<COLORREF, HBRUSH>::iterator it = g_brushes.find(c);
    if (it != g_brushes.end()) return it->second;
    HBRUSH b = ::CreateSolidBrush(c);
    g_brushes[c] = b;
    return b;
}

HPEN Pen(COLORREF c)
{
    std::map<COLORREF, HPEN>::iterator it = g_pens.find(c);
    if (it != g_pens.end()) return it->second;
    HPEN p = ::CreatePen(PS_SOLID, 1, c);
    g_pens[c] = p;
    return p;
}

double NowMs()
{
    static LARGE_INTEGER freq;
    static bool haveFreq = false;
    if (!haveFreq) { ::QueryPerformanceFrequency(&freq); haveFreq = true; }
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return freq.QuadPart ? (double)c.QuadPart * 1000.0 / (double)freq.QuadPart : 0.0;
}

// 背景點陣圖（整個客戶區）。寬高往上取整到 64 px：拖曳改大小時不必每一步都重配。
struct BackBuffer {
    HDC     mem;
    HBITMAP bmp;
    HGDIOBJ oldBmp;
    HGDIOBJ oldFont;
    int     w, h;
    BackBuffer() : mem(0), bmp(0), oldBmp(0), oldFont(0), w(0), h(0) {}
    void Free()
    {
        if (!mem) return;
        if (oldFont) ::SelectObject(mem, oldFont);
        ::SelectObject(mem, oldBmp);
        ::DeleteObject(bmp);
        ::DeleteDC(mem);
        mem = 0; bmp = 0; oldBmp = 0; oldFont = 0; w = h = 0;
    }
    // 至少 cw×ch；回 false＝客戶區是 0（最小化）
    bool Ensure(HWND hwnd, int cw, int ch, HFONT font)
    {
        if (cw <= 0 || ch <= 0) { Free(); return false; }
        if (mem && w >= cw && h >= ch) return true;
        Free();
        const int nw = (cw + 63) & ~63, nh = (ch + 63) & ~63;
        HDC sc = ::GetDC(hwnd);
        mem = ::CreateCompatibleDC(sc);
        bmp = ::CreateCompatibleBitmap(sc, nw, nh);
        ::ReleaseDC(hwnd, sc);
        if (!mem || !bmp) {
            if (bmp) ::DeleteObject(bmp);
            if (mem) ::DeleteDC(mem);
            mem = 0; bmp = 0;
            return false;
        }
        oldBmp = ::SelectObject(mem, bmp);
        oldFont = font ? ::SelectObject(mem, font) : 0;
        ::SetBkMode(mem, TRANSPARENT);
        w = nw; h = nh;
        return true;
    }
    void SetFont(HFONT font)
    {
        if (!mem || !font) return;
        HGDIOBJ prev = ::SelectObject(mem, font);
        if (!oldFont) oldFont = prev;
    }
};

// ------------------------------------------------------------------ Grid ----
struct Grid {
    HWND                    hwnd;
    GridCellFn              fn;
    void*                   ctx;
    std::vector<GridColumn> cols;
    std::vector<int>        order;      // 畫面第 i 欄 = column index
    std::vector<int>        colX;       // column index 的左緣（內容座標）
    int                     totalW;
    int                     rows;
    int                     rowH, headH;
    int                     textH, aveW;   // 字高、平均字寬（垂直置中；估字串寬度決定要不要加「…」）
    int                     top, xOff, sel;
    int                     cw, ch;
    HFONT                   font;
    BackBuffer              buf;
    std::vector<GridCell>   cache;      // cacheVis × ncols：上次畫進背景圖的內容
    std::vector<char>       cacheOk;    // 每個可見列：cache 是不是這一列現在的樣子
    int                     cacheTop, cacheVis;
    std::vector<int>        dirty;
    std::vector<char>       dirtyFlag;  // 依顯示列，去重
    int                     wheelAcc;
    GridStats               st;
    GridCell                tmp;
    Grid() : hwnd(0), fn(0), ctx(0), totalW(0), rows(0), rowH(22), headH(24), textH(15), aveW(7), top(0), xOff(0), sel(-1), cw(0), ch(0),
             font(0), cacheTop(0), cacheVis(0), wheelAcc(0) {}
};

struct GridInit {
    GridCellFn fn;
    void*      ctx;
};

Grid* G(HWND h)
{
    return h ? reinterpret_cast<Grid*>(::GetWindowLongPtrW(h, GWLP_USERDATA)) : 0;
}

int NCols(const Grid& g) { return (int)g.cols.size(); }

int VisRows(const Grid& g)   // 含露出一半的最後一列
{
    const int a = g.ch - g.headH;
    return a > 0 ? (a + g.rowH - 1) / g.rowH : 0;
}

int FullRows(const Grid& g)
{
    const int a = g.ch - g.headH;
    return a > 0 ? a / g.rowH : 0;
}

void ComputeColX(Grid& g)
{
    const int nc = NCols(g);
    if ((int)g.order.size() != nc) {
        g.order.resize((std::size_t)nc);
        for (int i = 0; i < nc; ++i) g.order[(std::size_t)i] = i;
    }
    g.colX.assign((std::size_t)nc, 0);
    int x = 0;
    for (int i = 0; i < nc; ++i) {
        const int c = g.order[(std::size_t)i];
        if (c < 0 || c >= nc) continue;
        g.colX[(std::size_t)c] = x;
        x += g.cols[(std::size_t)c].width;
    }
    g.totalW = x;
}

void ClampScroll(Grid& g)
{
    const int page = std::max(1, FullRows(g));
    const int maxTop = std::max(0, g.rows - page);
    if (g.top > maxTop) g.top = maxTop;
    if (g.top < 0) g.top = 0;
    const int maxX = std::max(0, g.totalW - g.cw);
    if (g.xOff > maxX) g.xOff = maxX;
    if (g.xOff < 0) g.xOff = 0;
}

void UpdateScroll(Grid& g)
{
    // SIF_DISABLENOSCROLL：用不到時是「停用」而不是消失 —— 捲軸不會忽隱忽現改掉客戶區大小（那又會觸發 WM_SIZE 重算）。
    SCROLLINFO si;
    std::memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = g.rows > 0 ? g.rows - 1 : 0;
    si.nPage = (UINT)std::max(1, FullRows(g));
    si.nPos = g.top;
    ::SetScrollInfo(g.hwnd, SB_VERT, &si, TRUE);
    si.nMax = g.totalW > 0 ? g.totalW - 1 : 0;
    si.nPage = (UINT)std::max(1, g.cw);
    si.nPos = g.xOff;
    ::SetScrollInfo(g.hwnd, SB_HORZ, &si, TRUE);
}

void QueryCell(Grid& g, int row, int col, GridCell& out)
{
    out.kind = kGridText;
    out.color = ::GetSysColor(COLOR_WINDOWTEXT);
    out.ledFill = 0;
    out.ledEdge = 0;
    out.text.clear();   // 保留容量：比較時不配置記憶體
    if (g.fn) g.fn(g.ctx, row, col, out);
}

bool CellRectVis(const Grid& g, int vi, int col, RECT* rc)
{
    const int x = g.colX[(std::size_t)col] - g.xOff;
    rc->left = x;
    rc->right = x + g.cols[(std::size_t)col].width;
    rc->top = g.headH + vi * g.rowH;
    rc->bottom = rc->top + g.rowH;
    return rc->right > 0 && rc->left < g.cw && rc->top < g.ch;
}

UINT AlignFlags(int align)
{
    return align == kGridRight ? DT_RIGHT : align == kGridCenter ? DT_CENTER : DT_LEFT;
}

void DrawLedDot(HDC dc, int x, int y, int d, COLORREF fill, COLORREF edge)
{
    ::SelectObject(dc, Brush(fill));
    ::SelectObject(dc, Pen(edge));
    ::Ellipse(dc, x, y, x + d, y + d);
}

void OpaqueFill(HDC dc, const RECT& r, COLORREF c)
{
    ::SetBkColor(dc, c);
    ::ExtTextOutW(dc, 0, 0, ETO_OPAQUE, &r, 0, 0, 0);   // 最便宜的實心填色（不選筆刷）
}

// 一行字：ExtTextOutW（ETO_OPAQUE 同時填底色＋寫字，一次 GDI 呼叫）。DrawTextW 在這台量到一次 60～80 µs、
// ExtTextOutW 約 15 µs（20260929 scratch gdibench）。估計寫不下（寬字算兩格）時才走 DrawTextW 加「…」。
void TextCell(Grid& g, HDC dc, const RECT& in, const std::wstring& s, int align, COLORREF fg, COLORREF bg, bool opaque)
{
    ::SetTextColor(dc, fg);
    ::SetBkColor(dc, bg);
    const int avail = (in.right - in.left) - 2 * kPad;
    int est = 0;
    for (std::size_t i = 0; i < s.size() && est <= avail; ++i) est += s[i] < 0x80 ? g.aveW : 2 * g.aveW;
    if (est > avail) {
        if (opaque) ::ExtTextOutW(dc, 0, 0, ETO_OPAQUE, &in, 0, 0, 0);
        RECT t = in;
        t.left += kPad;
        t.right -= kPad;
        ::DrawTextW(dc, s.c_str(), (int)s.size(), &t, AlignFlags(align) | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        return;
    }
    int x;
    UINT ta;
    if (align == kGridRight)       { x = in.right - kPad; ta = TA_RIGHT; }
    else if (align == kGridCenter) { x = (in.left + in.right) / 2; ta = TA_CENTER; }
    else                           { x = in.left + kPad; ta = TA_LEFT; }
    ::SetTextAlign(dc, ta | TA_TOP | TA_NOUPDATECP);
    const int y = in.top + ((in.bottom - in.top) - g.textH) / 2;
    ::ExtTextOutW(dc, x, y, (opaque ? ETO_OPAQUE : 0) | ETO_CLIPPED, &in, s.c_str(), (UINT)s.size(), 0);
}

// 把一格畫進背景圖。只畫格子裡面：右邊與下面那一條像素是格線，整張重畫時畫一次（RenderAll），之後不再碰。
// 回 true＝這一格在客戶區內（out＝它的矩形，含格線）。
bool DrawCell(Grid& g, int vi, int col, const GridCell& cell, RECT* out)
{
    RECT rc;
    const bool vis = CellRectVis(g, vi, col, &rc);
    if (out) *out = rc;
    if (!vis || !g.buf.mem) return false;
    HDC dc = g.buf.mem;
    const bool sel = (g.top + vi) == g.sel;
    RECT in = rc;
    in.right -= 1;
    in.bottom -= 1;
    const COLORREF bg = ::GetSysColor(sel ? COLOR_HIGHLIGHT : COLOR_WINDOW);
    const COLORREF fg = sel ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : cell.color;
    const int hgt = in.bottom - in.top;
    int d = hgt - 7;
    if (d > 14) d = 14;
    if (d < 6) d = 6;
    switch (cell.kind) {
    case kGridLed:
        OpaqueFill(dc, in, bg);
        DrawLedDot(dc, in.left + (in.right - in.left - d) / 2, in.top + (hgt - d) / 2, d, cell.ledFill, cell.ledEdge);
        break;
    case kGridLedText: {
        OpaqueFill(dc, in, bg);
        const int x = in.left + 6;
        DrawLedDot(dc, x, in.top + (hgt - d) / 2, d, cell.ledFill, cell.ledEdge);
        RECT t = in;
        t.left = x + d + 6 - kPad;
        TextCell(g, dc, t, cell.text, kGridLeft, fg, bg, false);
        break;
    }
    case kGridButtonOff: {
        OpaqueFill(dc, in, bg);
        if (cell.text.empty()) break;
        // 只是「畫」一顆停用的按鈕，不是控件：沒有 HWND、點下去不會產生任何 WM_COMMAND。
        RECT b = in;
        ::InflateRect(&b, -6, -2);
        ::DrawFrameControl(dc, &b, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_INACTIVE);
        TextCell(g, dc, b, cell.text, kGridCenter, ::GetSysColor(COLOR_GRAYTEXT), bg, false);
        break;
    }
    default:
        TextCell(g, dc, in, cell.text, g.cols[(std::size_t)col].align, fg, bg, true);
        break;
    }
    return true;
}

// 格線：每欄右邊一條、每列下面一條，只在整張重畫時畫（DrawCell 不碰那些像素）。
void DrawGridLines(Grid& g, int vis)
{
    HDC dc = g.buf.mem;
    int drawn = g.rows - g.top;
    if (drawn > vis) drawn = vis;
    if (drawn <= 0) return;
    const int bottom = g.headH + drawn * g.rowH;
    const int right = g.totalW - g.xOff;
    ::SelectObject(dc, Pen(kGridLine));
    for (int c = 0; c < NCols(g); ++c) {
        const int x = g.colX[(std::size_t)c] + g.cols[(std::size_t)c].width - 1 - g.xOff;
        if (x < 0 || x >= g.cw) continue;
        ::MoveToEx(dc, x, g.headH, 0);
        ::LineTo(dc, x, bottom);
    }
    for (int vi = 0; vi < drawn; ++vi) {
        const int y = g.headH + (vi + 1) * g.rowH - 1;
        ::MoveToEx(dc, -g.xOff, y, 0);
        ::LineTo(dc, right, y);
    }
}

void DrawHeader(Grid& g)
{
    HDC dc = g.buf.mem;
    RECT hr = {0, 0, g.cw, g.headH};
    ::FillRect(dc, &hr, ::GetSysColorBrush(COLOR_BTNFACE));
    ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
    ::SelectObject(dc, Pen(::GetSysColor(COLOR_BTNSHADOW)));
    for (int c = 0; c < NCols(g); ++c) {
        const int x = g.colX[(std::size_t)c] - g.xOff, w = g.cols[(std::size_t)c].width;
        if (x + w <= 0 || x >= g.cw) continue;
        RECT t = {x + kPad, 0, x + w - kPad, g.headH - 1};
        const std::wstring& s = g.cols[(std::size_t)c].title;
        ::DrawTextW(dc, s.c_str(), (int)s.size(), &t,
                    AlignFlags(g.cols[(std::size_t)c].align) | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        ::MoveToEx(dc, x + w - 1, 3, 0);
        ::LineTo(dc, x + w - 1, g.headH - 3);
    }
    ::MoveToEx(dc, 0, g.headH - 1, 0);
    ::LineTo(dc, g.cw, g.headH - 1);
}

void ClearDirty(Grid& g)
{
    for (std::size_t k = 0; k < g.dirty.size(); ++k) {
        const int r = g.dirty[k];
        if (r >= 0 && r < (int)g.dirtyFlag.size()) g.dirtyFlag[(std::size_t)r] = 0;
    }
    g.dirty.clear();
}

// 整張重畫進背景圖（捲動／改大小／換篩選／換欄位）。畫面由之後的 WM_PAINT 一次 BitBlt 上去（不擦背景，所以不閃）。
void RenderAll(Grid& g)
{
    const double t0 = NowMs();
    ClearDirty(g);
    if (!g.buf.Ensure(g.hwnd, g.cw, g.ch, g.font)) {
        g.cacheOk.clear();
        g.cacheVis = 0;
        return;
    }
    RECT all = {0, 0, g.cw, g.ch};
    ::FillRect(g.buf.mem, &all, ::GetSysColorBrush(COLOR_WINDOW));
    DrawHeader(g);
    const int vis = VisRows(g), nc = NCols(g);
    g.cache.resize((std::size_t)(vis * nc));
    g.cacheOk.assign((std::size_t)vis, 0);
    g.cacheTop = g.top;
    g.cacheVis = vis;
    for (int vi = 0; vi < vis; ++vi) {
        const int row = g.top + vi;
        if (row >= g.rows) break;
        for (int c = 0; c < nc; ++c) {
            GridCell& cell = g.cache[(std::size_t)(vi * nc + c)];
            QueryCell(g, row, c, cell);
            DrawCell(g, vi, c, cell, 0);
        }
        g.cacheOk[(std::size_t)vi] = 1;
    }
    DrawGridLines(g, vis);
    ++g.st.fullRenders;
    g.st.lastFullMs = NowMs() - t0;
    ::InvalidateRect(g.hwnd, 0, FALSE);
}

void RenderAllNow(Grid& g)
{
    RenderAll(g);
    ::UpdateWindow(g.hwnd);
}

int Flush(Grid& g)
{
    const double t0 = NowMs();
    ++g.st.flushes;
    int painted = 0;
    if (g.dirty.empty() || !g.buf.mem) {
        ClearDirty(g);
    } else if (g.cacheTop != g.top || g.cacheVis != VisRows(g)) {
        RenderAllNow(g);   // 不該發生（捲動／改大小都已整張重畫）；保險
    } else {
        const int nc = NCols(g);
        for (std::size_t k = 0; k < g.dirty.size(); ++k) {
            const int row = g.dirty[k];
            if (row >= 0 && row < (int)g.dirtyFlag.size()) g.dirtyFlag[(std::size_t)row] = 0;
            const int vi = row - g.top;
            if (vi < 0 || vi >= g.cacheVis || row >= g.rows) continue;
            ++g.st.rowsChecked;
            const bool ok = g.cacheOk[(std::size_t)vi] != 0;
            RECT u = {0, 0, 0, 0};
            bool any = false;
            for (int c = 0; c < nc; ++c) {
                QueryCell(g, row, c, g.tmp);
                GridCell& old = g.cache[(std::size_t)(vi * nc + c)];
                if (ok && g.tmp == old) continue;   // 跟畫面上的一樣：不畫
                std::swap(old, g.tmp);              // 換進快取（不複製字串）
                RECT rc;
                if (DrawCell(g, vi, c, old, &rc)) {
                    if (!any) u = rc; else ::UnionRect(&u, &u, &rc);
                    any = true;
                }
                ++painted;
            }
            g.cacheOk[(std::size_t)vi] = 1;
            if (any) ::InvalidateRect(g.hwnd, &u, FALSE);   // 只有變了的那一段送上螢幕
        }
        g.dirty.clear();
        if (painted) ::UpdateWindow(g.hwnd);   // 同步 WM_PAINT：回來時螢幕已經是新的
    }
    g.st.cellsPainted += (unsigned long)painted;
    g.st.lastFlushMs = NowMs() - t0;
    if (g.st.lastFlushMs > g.st.maxFlushMs) g.st.maxFlushMs = g.st.lastFlushMs;
    return painted;
}

void MarkRow(Grid& g, int row)
{
    if (row < 0 || row >= g.rows || row >= (int)g.dirtyFlag.size()) return;
    if (row < g.top || row >= g.top + g.cacheVis) return;   // 看不到：捲到它時會整張重畫
    if (g.dirtyFlag[(std::size_t)row]) return;
    g.dirtyFlag[(std::size_t)row] = 1;
    g.dirty.push_back(row);
}

void ForgetRow(Grid& g, int row)   // 下一次 Flush 整列重畫（選取列換了：底色要換）
{
    const int vi = row - g.top;
    if (vi >= 0 && vi < (int)g.cacheOk.size()) g.cacheOk[(std::size_t)vi] = 0;
    MarkRow(g, row);
}

void Resize(Grid& g)
{
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    g.cw = rc.right - rc.left;
    g.ch = rc.bottom - rc.top;
    ClampScroll(g);
    UpdateScroll(g);
    RenderAll(g);
}

void SetTop(Grid& g, int t)
{
    const int old = g.top;
    g.top = t;
    ClampScroll(g);
    if (g.top == old) return;
    UpdateScroll(g);
    RenderAllNow(g);
}

void SetXOff(Grid& g, int x)
{
    const int old = g.xOff;
    g.xOff = x;
    ClampScroll(g);
    if (g.xOff == old) return;
    UpdateScroll(g);
    RenderAllNow(g);
}

void EnsureVisible(Grid& g, int row)
{
    if (row < 0) return;
    const int page = std::max(1, FullRows(g));
    if (row < g.top) SetTop(g, row);
    else if (row >= g.top + page) SetTop(g, row - page + 1);
}

void SetSel(Grid& g, int row)
{
    if (row >= g.rows) row = g.rows - 1;
    if (row < -1) row = -1;
    if (row == g.sel) return;
    const int old = g.sel;
    g.sel = row;
    if (old >= 0) ForgetRow(g, old);
    if (row >= 0) ForgetRow(g, row);
    Flush(g);
    EnsureVisible(g, row);
}

void ApplyFont(Grid& g, HFONT f)
{
    g.font = f;
    HDC dc = ::GetDC(g.hwnd);
    HGDIOBJ o = f ? ::SelectObject(dc, f) : 0;
    TEXTMETRICW tm;
    std::memset(&tm, 0, sizeof(tm));
    ::GetTextMetricsW(dc, &tm);
    if (o) ::SelectObject(dc, o);
    ::ReleaseDC(g.hwnd, dc);
    g.textH = (int)tm.tmHeight;
    g.aveW = tm.tmAveCharWidth > 0 ? (int)tm.tmAveCharWidth : 7;
    g.rowH = (int)tm.tmHeight + 8;
    if (g.rowH < 16) g.rowH = 16;
    g.headH = g.rowH + 2;
    g.buf.SetFont(f);
}

LRESULT CALLBACK GridProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) {
        const CREATESTRUCTW* cs = reinterpret_cast<const CREATESTRUCTW*>(lp);
        const GridInit* init = reinterpret_cast<const GridInit*>(cs->lpCreateParams);
        Grid* g = new Grid();
        g->hwnd = hwnd;
        if (init) { g->fn = init->fn; g->ctx = init->ctx; }
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)g);
        return ::DefWindowProcW(hwnd, msg, wp, lp);
    }
    Grid* g = G(hwnd);
    if (!g) return ::DefWindowProcW(hwnd, msg, wp, lp);
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;   // 不擦：畫面永遠從背景圖 BitBlt 過來
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = ::BeginPaint(hwnd, &ps);
        const RECT& r = ps.rcPaint;
        if (g->buf.mem) ::BitBlt(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, g->buf.mem, r.left, r.top, SRCCOPY);
        else ::FillRect(dc, &r, ::GetSysColorBrush(COLOR_WINDOW));
        ::EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_PRINTCLIENT:   // PrintWindow（--snapshot）
        if (g->buf.mem) ::BitBlt((HDC)wp, 0, 0, g->cw, g->ch, g->buf.mem, 0, 0, SRCCOPY);
        return 0;
    case WM_SIZE:
        Resize(*g);
        return 0;
    case WM_SETFONT:
        ApplyFont(*g, (HFONT)wp);
        Resize(*g);
        if (LOWORD(lp)) ::UpdateWindow(hwnd);
        return 0;
    case WM_GETFONT:
        return (LRESULT)g->font;
    case WM_VSCROLL: {
        const int page = std::max(1, FullRows(*g));
        int t = g->top;
        switch (LOWORD(wp)) {
        case SB_LINEUP:   t -= 1; break;
        case SB_LINEDOWN: t += 1; break;
        case SB_PAGEUP:   t -= page; break;
        case SB_PAGEDOWN: t += page; break;
        case SB_TOP:      t = 0; break;
        case SB_BOTTOM:   t = g->rows; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: {
            SCROLLINFO si;
            std::memset(&si, 0, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
            ::GetScrollInfo(hwnd, SB_VERT, &si);
            t = si.nTrackPos;
            break;
        }
        default: break;
        }
        SetTop(*g, t);
        return 0;
    }
    case WM_HSCROLL: {
        int x = g->xOff;
        switch (LOWORD(wp)) {
        case SB_LINELEFT:  x -= 40; break;
        case SB_LINERIGHT: x += 40; break;
        case SB_PAGELEFT:  x -= g->cw; break;
        case SB_PAGERIGHT: x += g->cw; break;
        case SB_LEFT:      x = 0; break;
        case SB_RIGHT:     x = g->totalW; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: {
            SCROLLINFO si;
            std::memset(&si, 0, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
            ::GetScrollInfo(hwnd, SB_HORZ, &si);
            x = si.nTrackPos;
            break;
        }
        default: break;
        }
        SetXOff(*g, x);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        g->wheelAcc += GET_WHEEL_DELTA_WPARAM(wp);
        const int n = g->wheelAcc / 40;   // 一格（120）＝3 列
        g->wheelAcc -= n * 40;
        if (n) SetTop(*g, g->top - n);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        ::SetFocus(hwnd);
        const int y = (short)HIWORD(lp);
        if (y >= g->headH) {
            const int row = g->top + (y - g->headH) / g->rowH;
            if (row < g->rows) SetSel(*g, row);
        }
        return 0;
    }
    case WM_KEYDOWN: {
        const int page = std::max(1, FullRows(*g));
        switch (wp) {
        case VK_UP:    SetSel(*g, g->sel > 0 ? g->sel - 1 : 0); return 0;
        case VK_DOWN:  SetSel(*g, g->sel + 1); return 0;
        case VK_PRIOR: SetSel(*g, std::max(0, g->sel - page)); return 0;
        case VK_NEXT:  SetSel(*g, g->sel + page); return 0;
        case VK_HOME:  SetSel(*g, 0); return 0;
        case VK_END:   SetSel(*g, g->rows - 1); return 0;
        case VK_LEFT:  SetXOff(*g, g->xOff - 40); return 0;
        case VK_RIGHT: SetXOff(*g, g->xOff + 40); return 0;
        default: break;
        }
        break;
    }
    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;
    case kMsgCellText: {
        std::wstring* out = reinterpret_cast<std::wstring*>(lp);
        const int row = (int)LOWORD(wp), col = (int)HIWORD(wp);
        if (!out) return 0;
        out->clear();
        if (row < 0 || row >= g->rows || col < 0 || col >= NCols(*g)) return 0;
        GridCell c;
        QueryCell(*g, row, col, c);
        *out = c.text;
        return 1;
    }
    case WM_NCDESTROY:
        g->buf.Free();
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        delete g;
        return ::DefWindowProcW(hwnd, msg, wp, lp);
    default:
        break;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

// ----------------------------------------------------------------- Label ----
// 標籤：一行一行畫（'\n' 分行，不自動換行），換字時只把「變了的那幾行」送上螢幕。
struct Label {
    HFONT                     font;
    int                       lineH;
    BackBuffer                buf;
    std::wstring              text;
    std::vector<std::wstring> lines;
    unsigned long             paints;
    Label() : font(0), lineH(20), paints(0) {}
};

Label* L_(HWND h)
{
    return h ? reinterpret_cast<Label*>(::GetWindowLongPtrW(h, GWLP_USERDATA)) : 0;
}

void SplitLines(const std::wstring& s, std::vector<std::wstring>& out)
{
    out.clear();
    std::size_t a = 0;
    for (;;) {
        const std::size_t b = s.find(L'\n', a);
        out.push_back(s.substr(a, b == std::wstring::npos ? std::wstring::npos : b - a));
        if (b == std::wstring::npos) break;
        a = b + 1;
    }
}

void PaintLabel(HWND hwnd, Label& l, HDC target, const RECT& clip)
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top;
    if (!l.buf.Ensure(hwnd, w, h, l.font)) return;
    l.buf.SetFont(l.font);
    HDC dc = l.buf.mem;
    const COLORREF bg = ::GetSysColor(COLOR_BTNFACE);
    ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
    ::SetTextAlign(dc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
    ::SetBkColor(dc, bg);
    int y = 0;
    for (std::size_t i = 0; i < l.lines.size() && y < h; ++i, y += l.lineH) {
        RECT lr = {0, y, w, y + l.lineH};
        if (lr.bottom <= clip.top || lr.top >= clip.bottom) continue;   // 這一行不在要畫的範圍
        ::ExtTextOutW(dc, 0, y + 1, ETO_OPAQUE | ETO_CLIPPED, &lr, l.lines[i].c_str(), (UINT)l.lines[i].size(), 0);
    }
    if (y < h) {
        RECT rest = {0, y, w, h};
        ::ExtTextOutW(dc, 0, 0, ETO_OPAQUE, &rest, 0, 0, 0);
    }
    ::BitBlt(target, clip.left, clip.top, clip.right - clip.left, clip.bottom - clip.top, dc, clip.left, clip.top, SRCCOPY);
    ++l.paints;
}

void LabelFont(HWND hwnd, Label& l, HFONT f)
{
    l.font = f;
    HDC dc = ::GetDC(hwnd);
    HGDIOBJ o = f ? ::SelectObject(dc, f) : 0;
    TEXTMETRICW tm;
    std::memset(&tm, 0, sizeof(tm));
    ::GetTextMetricsW(dc, &tm);
    if (o) ::SelectObject(dc, o);
    ::ReleaseDC(hwnd, dc);
    l.lineH = tm.tmHeight > 0 ? (int)tm.tmHeight + 1 : 20;
}

LRESULT CALLBACK LabelProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) {
        Label* l = new Label();
        const CREATESTRUCTW* cs = reinterpret_cast<const CREATESTRUCTW*>(lp);
        if (cs->lpszName) l->text = cs->lpszName;
        SplitLines(l->text, l->lines);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)l);
        return ::DefWindowProcW(hwnd, msg, wp, lp);
    }
    Label* l = L_(hwnd);
    if (!l) return ::DefWindowProcW(hwnd, msg, wp, lp);
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = ::BeginPaint(hwnd, &ps);
        PaintLabel(hwnd, *l, dc, ps.rcPaint);
        ::EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_PRINTCLIENT: {
        RECT rc;
        ::GetClientRect(hwnd, &rc);
        PaintLabel(hwnd, *l, (HDC)wp, rc);
        return 0;
    }
    case WM_SETTEXT: {
        const LRESULT r = ::DefWindowProcW(hwnd, msg, wp, lp);
        const wchar_t* s = reinterpret_cast<const wchar_t*>(lp);
        l->text = s ? s : L"";
        std::vector<std::wstring> nl;
        SplitLines(l->text, nl);
        RECT rc;
        ::GetClientRect(hwnd, &rc);
        const std::size_t n = nl.size() > l->lines.size() ? nl.size() : l->lines.size();
        for (std::size_t i = 0; i < n; ++i) {
            if (i < nl.size() && i < l->lines.size() && nl[i] == l->lines[i]) continue;   // 這一行沒變：不重畫
            RECT lr = {0, (int)i * l->lineH, rc.right, (int)(i + 1) * l->lineH};
            ::InvalidateRect(hwnd, &lr, FALSE);
        }
        l->lines.swap(nl);
        ::UpdateWindow(hwnd);
        return r;
    }
    case WM_SETFONT:
        LabelFont(hwnd, *l, (HFONT)wp);
        ::InvalidateRect(hwnd, 0, FALSE);
        if (LOWORD(lp)) ::UpdateWindow(hwnd);
        return 0;
    case WM_GETFONT:
        return (LRESULT)l->font;
    case WM_SIZE:
        ::InvalidateRect(hwnd, 0, FALSE);
        return 0;
    case WM_NCDESTROY:
        l->buf.Free();
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        delete l;
        return ::DefWindowProcW(hwnd, msg, wp, lp);
    default:
        break;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

bool RegisterClasses()
{
    static bool done = false;
    if (done) return true;
    WNDCLASSEXW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &GridProc;
    wc.hInstance = ::GetModuleHandleW(0);
    wc.hCursor = ::LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = 0;   // 沒有背景刷：Windows 不會替我們擦
    wc.lpszClassName = kGridClass;
    if (!::RegisterClassExW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
    wc.lpfnWndProc = &LabelProc;
    wc.lpszClassName = kLabelClass;
    if (!::RegisterClassExW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
    done = true;
    return true;
}

}  // namespace

HWND GridCreate(HWND parent, int id, GridCellFn fn, void* ctx)
{
    if (!RegisterClasses()) return 0;
    GridInit init;
    init.fn = fn;
    init.ctx = ctx;
    return ::CreateWindowExW(0, kGridClass, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_TABSTOP,
                             0, 0, 10, 10, parent, (HMENU)(INT_PTR)id, ::GetModuleHandleW(0), &init);
}

void GridSetColumns(HWND grid, const std::vector<GridColumn>& cols, const std::vector<int>& order)
{
    Grid* g = G(grid);
    if (!g) return;
    g->cols = cols;
    g->order = order;
    ComputeColX(*g);
    g->cache.clear();
    g->cacheOk.clear();
    ClampScroll(*g);
    UpdateScroll(*g);
    RenderAllNow(*g);
}

void GridSetRowCount(HWND grid, int rows)
{
    Grid* g = G(grid);
    if (!g) return;
    g->rows = rows < 0 ? 0 : rows;
    g->dirty.clear();
    g->dirtyFlag.assign((std::size_t)g->rows, 0);
    if (g->sel >= g->rows) g->sel = -1;
    ClampScroll(*g);
    UpdateScroll(*g);
    RenderAllNow(*g);
}

int GridRowCount(HWND grid)
{
    Grid* g = G(grid);
    return g ? g->rows : -1;
}

void GridMarkRow(HWND grid, int row)
{
    Grid* g = G(grid);
    if (g) MarkRow(*g, row);
}

void GridMarkAll(HWND grid)
{
    Grid* g = G(grid);
    if (!g) return;
    for (int vi = 0; vi < g->cacheVis; ++vi) MarkRow(*g, g->top + vi);
}

int GridFlush(HWND grid)
{
    Grid* g = G(grid);
    return g ? Flush(*g) : 0;
}

int GridTopRow(HWND grid)
{
    Grid* g = G(grid);
    return g ? g->top : 0;
}

int GridVisibleRows(HWND grid)
{
    Grid* g = G(grid);
    return g ? VisRows(*g) : 0;
}

int GridSelectedRow(HWND grid)
{
    Grid* g = G(grid);
    return g ? g->sel : -1;
}

GridStats GridGetStats(HWND grid)
{
    Grid* g = G(grid);
    return g ? g->st : GridStats();
}

std::wstring GridCellText(HWND grid, int row, int col)
{
    std::wstring s;
    if (grid && row >= 0 && col >= 0 && row <= 0xFFFF && col <= 0xFFFF)
        ::SendMessageW(grid, kMsgCellText, MAKEWPARAM((WORD)row, (WORD)col), (LPARAM)&s);
    return s;
}

COLORREF GridBackPixel(HWND grid, int x, int y)
{
    Grid* g = G(grid);
    if (!g || !g->buf.mem) return CLR_INVALID;
    return ::GetPixel(g->buf.mem, x, y);
}

bool GridCellRect(HWND grid, int row, int col, RECT* rc)
{
    Grid* g = G(grid);
    if (!g || !rc || col < 0 || col >= NCols(*g)) return false;
    const int vi = row - g->top;
    if (vi < 0 || vi >= VisRows(*g) || row >= g->rows) return false;
    return CellRectVis(*g, vi, col, rc);
}

void GridSetInt(std::wstring& out, int v)
{
    wchar_t b[16];
    int n = 0;
    unsigned int u = v < 0 ? 0u - (unsigned int)v : (unsigned int)v;
    do { b[n++] = (wchar_t)(L'0' + (int)(u % 10u)); u /= 10u; } while (u && n < 15);
    out.clear();
    if (v < 0) out.push_back(L'-');
    while (n > 0) out.push_back(b[--n]);
}

void GridSetAscii(std::wstring& out, const std::string& s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
        if ((unsigned char)s[i] >= 0x80) { out = Widen(s); return; }
    out.assign(s.begin(), s.end());
}

HWND LabelCreate(HWND parent, int id, const wchar_t* text)
{
    if (!RegisterClasses()) return 0;
    return ::CreateWindowExW(0, kLabelClass, text ? text : L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, parent,
                             (HMENU)(INT_PTR)id, ::GetModuleHandleW(0), 0);
}

bool LabelSetText(HWND label, const std::wstring& text)
{
    Label* l = L_(label);
    if (!l || l->text == text) return false;
    ::SetWindowTextW(label, text.c_str());   // WM_SETTEXT：存字、同步重畫（不擦）
    return true;
}

unsigned long LabelPaints(HWND label)
{
    Label* l = L_(label);
    return l ? l->paints : 0;
}

}  // namespace w906native
