// =============================================================================
//  hmi_shell.cpp -- the HMI's own program window (WebView2 host), not a browser.
//
//  AI(W906-HMI-SHELL) 20260930: EastSun「客戶反映 希望可以跟機台一樣 外框不是chrom 不要讓人員可以有額外操作的空間」
//    ／「我需要的是 按快捷鍵不會更改到畫面 下面工具列 是軟體的圖標」.
//  Chrome / Edge (normal, --app, --kiosk) all show the BROWSER's icon on the taskbar, and a page cannot block Ctrl+W / Ctrl+T.
//  This window hosts the same web HMI in the Windows WebView2 engine (Edge's core, part of Windows 11) with:
//    * the browser's own keyboard shortcuts OFF at the engine (reload, zoom, find, print, back/forward, DevTools ...),
//      zoom (Ctrl+wheel / pinch), swipe navigation, autofill, status bar OFF; context menu OFF in release;
//    * its own taskbar button: this exe's icon and the page's title (background.html sets it to machine.caption, e.g.
//      "HT-9050"), and an explicit AppUserModelID so Windows does not group it with a browser;
//    * navigation only to this machine (http://127.0.0.1 / localhost, file:// for boot_wait.html); pop-ups blocked in release;
//    * ✕ / Alt+F4: ONE question "確定要關閉軟體嗎？" (EastSun: 只需要讓客戶確認是否真的要關閉軟體); YES runs the main
//      screen's Exit (background.html HT9045ShellCloseRequest -> ht9045_main_close.js, already answered) -- the same guards
//      and the same normal shutdown; when wb_serve has closed it posts "HT9045_HMI_SHELL_QUIT" and this window goes away.
//      No page / no server -> it just closes.
//    * debug (URL has mode=debug, or --debug): DevTools on Ctrl+Shift+F12 and the context menu stay available.
//  Usage: ht9045_hmi.exe [URL] [--debug|--release]      URL default = %W906_HMI_URL% or http://127.0.0.1:8055/background.html
//  One instance per user session (a second start brings the first window to the front and exits).
//  Needs WebView2Loader.dll next to the exe (tools/hmi_shell/vendor/webview2, BSD-style licence) and the WebView2 Runtime
//  (in Windows 11). Log: %LOCALAPPDATA%\HT9045_HMI_Shell\hmi_shell.log
// =============================================================================
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <objbase.h>
#include <shellapi.h>
#include <cstdio>
#include <cwchar>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include "WebView2.h"

// The interface ids (IID_ICoreWebView2...) come from WebView2.h itself (__declspec(selectany) definitions).

namespace {

const wchar_t kClass[]  = L"HT9045HmiShell";
const wchar_t kMutex[]  = L"Local\\HT9045_HMI_Shell";
const wchar_t kAppId[]  = L"HonPrec.HT9045.HMI";
const wchar_t kQuitMsg[] = L"HT9045_HMI_SHELL_QUIT";      // wb_serve posts this when its normal close has finished

HWND                    g_hwnd = 0;
UINT                    g_quitMsg = 0;
ICoreWebView2Controller* g_ctl = 0;
ICoreWebView2*          g_web = 0;
bool                    g_debug = false;
bool                    g_quitting = false;
bool                    g_closeAsked = false;
std::wstring            g_url, g_title = L"HMI", g_logPath;

HANDLE                  g_mutex = 0;

void Log(const wchar_t* fmt, ...)
{
    if (g_logPath.empty()) return;
    wchar_t body[2048];
    va_list ap; va_start(ap, fmt); _vsnwprintf(body, 2047, fmt, ap); va_end(ap);
    body[2047] = 0;
    SYSTEMTIME st; ::GetLocalTime(&st);
    wchar_t line[2200];
    _snwprintf(line, 2199, L"%04d-%02d-%02d %02d:%02d:%02d.%03d  %ls\r\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, body);
    line[2199] = 0;
    char u8[6600];
    const int n = ::WideCharToMultiByte(CP_UTF8, 0, line, -1, u8, (int)sizeof(u8), 0, 0);
    if (n <= 1) return;
    HANDLE f = ::CreateFileW(g_logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD wr = 0; ::WriteFile(f, u8, (DWORD)(n - 1), &wr, 0);
    ::CloseHandle(f);
}

// One COM callback object for every two-argument WebView2 handler interface.
template <class I, class A, class B>
class Cb : public I {
public:
    Cb(const IID& iid, std::function<HRESULT(A, B)> fn) : ref_(1), iid_(iid), fn_(fn) {}
    virtual ~Cb() {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, iid_)) { *ppv = static_cast<I*>(this); AddRef(); return S_OK; }
        *ppv = 0; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return (ULONG)::InterlockedIncrement(&ref_); }
    ULONG STDMETHODCALLTYPE Release() override { const LONG r = ::InterlockedDecrement(&ref_); if (r == 0) delete this; return (ULONG)r; }
    HRESULT STDMETHODCALLTYPE Invoke(A a, B b) override { return fn_ ? fn_(a, b) : S_OK; }
private:
    volatile LONG ref_;
    const IID& iid_;
    std::function<HRESULT(A, B)> fn_;
};

// =============================================================================
// AI(W906-HMI-DLG) 20260930: EastSun (screenshot of the page's confirm "127.0.0.1:8055 說 / Sure close??")「你官軟體的小視窗
//   怎不是同一種風格? 我需要同一種風格」. Every alert() / confirm() / prompt() of the HMI (68 call sites in 27 files) and this
//   window's own close question are drawn here in the HMI's message-box style -- golden MyMessageBox as the web draws it
//   (Alert.MyMessageBox.html): the window title bar of the HMI (--tbar-focus1/2, --tbar-text), form background (--form-bg), a
//   raised panel with the text in navy (#000080; the theme's --text on a dark form), panel buttons in --dfm-hdr with white
//   bold captions. Colours and the zoom (--uiz) are read from the page every 3 s, so the box follows the chosen theme.
//   Why here and not in the pages: confirm() STOPS the page's script until it is answered and the callers rely on that; a box
//   drawn by the page cannot do it, so the pages would all have to be rewritten. WebView2 hands the dialog to this window
//   (ScriptDialogOpening + deferral); the page waits as before, nothing in the web changes.
// =============================================================================
struct DlgTheme {
    COLORREF formBg = RGB(0xec, 0xe9, 0xd8), hdr = RGB(0x51, 0x7b, 0x91), tb1 = RGB(0x0a, 0x24, 0x6a), tb2 = RGB(0x4a, 0x7a, 0xc2),
             tbText = RGB(255, 255, 255), text = RGB(0x22, 0x22, 0x22);
    double   zoom = 1.0;
};
DlgTheme g_theme;

bool ParseCssColor(std::wstring s, COLORREF& out)
{
    while (!s.empty() && (s[0] == L' ' || s[0] == L'\t')) s.erase(0, 1);
    while (!s.empty() && (s.back() == L' ' || s.back() == L'\t')) s.pop_back();
    if (s.size() == 7 && s[0] == L'#') { unsigned v = 0; if (swscanf(s.c_str() + 1, L"%6x", &v) != 1) return false; out = RGB((v >> 16) & 255, (v >> 8) & 255, v & 255); return true; }
    if (s.size() == 4 && s[0] == L'#') { unsigned v = 0; if (swscanf(s.c_str() + 1, L"%3x", &v) != 1) return false;
        const unsigned r = (v >> 8) & 15, g = (v >> 4) & 15, b = v & 15; out = RGB(r * 17, g * 17, b * 17); return true; }
    int r = 0, g = 0, b = 0;
    if (swscanf(s.c_str(), L"rgb(%d, %d, %d", &r, &g, &b) == 3 || swscanf(s.c_str(), L"rgba(%d, %d, %d", &r, &g, &b) == 3) { out = RGB(r, g, b); return true; }
    return false;
}

void RefreshTheme()
{
    if (!g_web) return;
    g_web->ExecuteScript(
        L"(function(){var s=getComputedStyle(document.documentElement);function v(n){return (s.getPropertyValue(n)||'').trim();}"
        L"var fb=v('--form-bg')||getComputedStyle(document.body).backgroundColor;"
        L"return [fb,v('--dfm-hdr'),v('--tbar-focus1'),v('--tbar-focus2'),v('--tbar-text'),v('--text'),v('--uiz')].join('|');})()",
        new Cb<ICoreWebView2ExecuteScriptCompletedHandler, HRESULT, LPCWSTR>(
            IID_ICoreWebView2ExecuteScriptCompletedHandler, [](HRESULT e, LPCWSTR json) -> HRESULT {
                if (FAILED(e) || !json) return S_OK;
                std::wstring r = json;
                if (r.size() >= 2 && r.front() == L'"' && r.back() == L'"') r = r.substr(1, r.size() - 2); else return S_OK;
                std::wstring part[7]; int k = 0;
                for (size_t i = 0; i <= r.size() && k < 7; ++i) { if (i == r.size() || r[i] == L'|') { ++k; continue; } part[k] += r[i]; }
                DlgTheme t = g_theme;
                ParseCssColor(part[0], t.formBg); ParseCssColor(part[1], t.hdr); ParseCssColor(part[2], t.tb1);
                ParseCssColor(part[3], t.tb2); ParseCssColor(part[4], t.tbText); ParseCssColor(part[5], t.text);
                const double z = _wtof(part[6].c_str()); t.zoom = (z >= 0.5 && z <= 3.0) ? z : 1.0;
                g_theme = t;
                return S_OK;
            }));
}

struct DlgState {
    int         kind = 0;              // 0 alert, 1 confirm, 2 prompt
    std::wstring text, result;
    bool        done = false, ok = false;
    int         pressed = -1, hot = -1;
    RECT        btn[2] = {};           // [0] 確定, [1] 取消
    int         nBtn = 1;
    HWND        edit = 0;
    HFONT       fText = 0, fBtn = 0, fTitle = 0;
    double      s = 1.0;               // pixels per design unit (DPI x HMI zoom)
    int         titleH = 24;
};

int Px(const DlgState* d, int v) { return (int)(v * d->s + 0.5); }

bool IsDark(COLORREF c) { return (GetRValue(c) * 299 + GetGValue(c) * 587 + GetBValue(c) * 114) / 1000 < 110; }

void Bevel(HDC dc, RECT r, bool raised)
{
    HPEN lt = ::CreatePen(PS_SOLID, 1, raised ? RGB(255, 255, 255) : RGB(96, 96, 96));
    HPEN dk = ::CreatePen(PS_SOLID, 1, raised ? RGB(96, 96, 96) : RGB(255, 255, 255));
    HGDIOBJ o = ::SelectObject(dc, lt);
    ::MoveToEx(dc, r.left, r.bottom - 1, 0); ::LineTo(dc, r.left, r.top); ::LineTo(dc, r.right - 1, r.top);
    ::SelectObject(dc, dk);
    ::LineTo(dc, r.right - 1, r.bottom - 1); ::LineTo(dc, r.left - 1, r.bottom - 1);
    ::SelectObject(dc, o); ::DeleteObject(lt); ::DeleteObject(dk);
}

void DlgPaint(HWND h, DlgState* d)
{
    PAINTSTRUCT ps; HDC dc = ::BeginPaint(h, &ps);
    RECT rc; ::GetClientRect(h, &rc);
    const DlgTheme& t = g_theme;
    // window frame + title bar (the HMI windows' gradient, left to right)
    HBRUSH bg = ::CreateSolidBrush(t.formBg); ::FillRect(dc, &rc, bg); ::DeleteObject(bg);
    const int th = Px(d, d->titleH);
    for (int x = 0; x < rc.right; ++x) {
        const double f = rc.right > 1 ? (double)x / (rc.right - 1) : 0;
        const COLORREF c = RGB((int)(GetRValue(t.tb1) + (GetRValue(t.tb2) - GetRValue(t.tb1)) * f),
                               (int)(GetGValue(t.tb1) + (GetGValue(t.tb2) - GetGValue(t.tb1)) * f),
                               (int)(GetBValue(t.tb1) + (GetBValue(t.tb2) - GetBValue(t.tb1)) * f));
        HPEN p = ::CreatePen(PS_SOLID, 1, c); HGDIOBJ o = ::SelectObject(dc, p);
        ::MoveToEx(dc, x, 0, 0); ::LineTo(dc, x, th); ::SelectObject(dc, o); ::DeleteObject(p);
    }
    ::SetBkMode(dc, TRANSPARENT);
    ::SetTextColor(dc, t.tbText);
    HGDIOBJ of = ::SelectObject(dc, d->fTitle);
    RECT tr = { Px(d, 8), 0, rc.right - Px(d, 8), th };
    ::DrawTextW(dc, g_title.c_str(), -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
    Bevel(dc, rc, true);
    // raised message panel (golden pnlMain) with the text
    RECT pr = { Px(d, 8), th + Px(d, 8), rc.right - Px(d, 8), d->btn[0].top - Px(d, 10) };
    Bevel(dc, pr, true);
    ::SelectObject(dc, d->fText);
    ::SetTextColor(dc, IsDark(t.formBg) ? t.text : RGB(0, 0, 0x80));
    RECT txt = { pr.left + Px(d, 12), pr.top + Px(d, 10), pr.right - Px(d, 12), pr.bottom - Px(d, 10) };
    if (d->edit) txt.bottom -= Px(d, 34);
    ::DrawTextW(dc, d->text.c_str(), -1, &txt, DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
    // panel buttons (golden pnlPause: --dfm-hdr, white bold caption, raised; pressed = sunken)
    static const wchar_t* cap[2] = { L"確定", L"取消" };
    ::SelectObject(dc, d->fBtn);
    for (int i = 0; i < d->nBtn; ++i) {
        RECT b = d->btn[i];
        COLORREF c = t.hdr;
        if (d->hot == i && d->pressed != i) c = RGB(std::min(255, GetRValue(c) + 18), std::min(255, GetGValue(c) + 18), std::min(255, GetBValue(c) + 18));
        HBRUSH bb = ::CreateSolidBrush(c); ::FillRect(dc, &b, bb); ::DeleteObject(bb);
        Bevel(dc, b, d->pressed != i);
        ::SetTextColor(dc, RGB(255, 255, 255));
        RECT bt = b; if (d->pressed == i) ::OffsetRect(&bt, 1, 1);
        ::DrawTextW(dc, cap[i], -1, &bt, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX);
        if (i == 0) { RECT fr = b; ::InflateRect(&fr, -Px(d, 4), -Px(d, 4)); ::DrawFocusRect(dc, &fr); }
    }
    ::SelectObject(dc, of);
    ::EndPaint(h, &ps);
}

int DlgHit(DlgState* d, LPARAM l)
{
    POINT p = { (short)LOWORD(l), (short)HIWORD(l) };
    for (int i = 0; i < d->nBtn; ++i) if (::PtInRect(&d->btn[i], p)) return i;
    return -1;
}

void DlgFinish(DlgState* d, bool ok)
{
    d->ok = ok;
    if (ok && d->edit) {
        const int n = ::GetWindowTextLengthW(d->edit);
        std::wstring s(n + 1, L'\0'); ::GetWindowTextW(d->edit, &s[0], n + 1); s.resize(n);
        d->result = s;
    }
    d->done = true;
}

WNDPROC g_editProc = 0;
LRESULT CALLBACK DlgEditProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_KEYDOWN && (w == VK_RETURN || w == VK_ESCAPE)) { ::SendMessageW(::GetParent(h), WM_KEYDOWN, w, l); return 0; }
    if (m == WM_CHAR && (w == L'\r' || w == 27)) return 0;
    return ::CallWindowProcW(g_editProc, h, m, w, l);
}

LRESULT CALLBACK DlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    DlgState* d = (DlgState*)::GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (m) {
    case WM_NCCREATE: ::SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTW*)l)->lpCreateParams); break;
    case WM_PAINT: if (d) { DlgPaint(h, d); return 0; } break;
    case WM_ERASEBKGND: return 1;
    case WM_NCHITTEST: {                                   // drag by the title bar
        LRESULT r = ::DefWindowProcW(h, m, w, l);
        if (d && r == HTCLIENT) { POINT p = { (short)LOWORD(l), (short)HIWORD(l) }; ::ScreenToClient(h, &p); if (p.y < Px(d, d->titleH)) return HTCAPTION; }
        return r;
    }
    case WM_MOUSEMOVE: if (d) { const int k = DlgHit(d, l); if (k != d->hot) { d->hot = k; ::InvalidateRect(h, 0, FALSE); }
                                TRACKMOUSEEVENT tm = { sizeof(tm), TME_LEAVE, h, 0 }; ::TrackMouseEvent(&tm); } return 0;
    case WM_MOUSELEAVE: if (d && d->hot != -1) { d->hot = -1; ::InvalidateRect(h, 0, FALSE); } return 0;
    case WM_LBUTTONDOWN: if (d) { d->pressed = DlgHit(d, l); if (d->pressed >= 0) ::SetCapture(h); ::InvalidateRect(h, 0, FALSE); } return 0;
    case WM_LBUTTONUP: if (d) { const int k = DlgHit(d, l); const int p = d->pressed; d->pressed = -1; ::ReleaseCapture(); ::InvalidateRect(h, 0, FALSE);
                                if (p >= 0 && k == p) DlgFinish(d, p == 0); } return 0;
    case WM_KEYDOWN: if (d) { if (w == VK_RETURN || w == VK_SPACE) { DlgFinish(d, true); return 0; }
                              if (w == VK_ESCAPE) { DlgFinish(d, d->kind == 0); return 0; } } break;
    case WM_CLOSE: if (d) DlgFinish(d, d->kind == 0); return 0;
    }
    return ::DefWindowProcW(h, m, w, l);
}

// kind: 0 alert (確定), 1 confirm (確定 / 取消), 2 prompt (text box + 確定 / 取消). Modal to the HMI window.
bool StyledDialog(int kind, const std::wstring& text, const std::wstring& defText, std::wstring* result)
{
    static bool reg = false;
    HINSTANCE hi = ::GetModuleHandleW(0);
    if (!reg) {
        WNDCLASSEXW wc; ZeroMemory(&wc, sizeof(wc)); wc.cbSize = sizeof(wc); wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = DlgProc; wc.hInstance = hi; wc.hCursor = ::LoadCursorW(0, IDC_ARROW); wc.lpszClassName = L"HT9045HmiDlg";
        ::RegisterClassExW(&wc); reg = true;
    }
    DlgState d; d.kind = kind; d.text = text; d.nBtn = (kind == 0) ? 1 : 2;
    UINT dpi = 96;
    typedef UINT (WINAPI *DpiFn)(HWND);
    if (DpiFn f = (DpiFn)(void*)::GetProcAddress(::GetModuleHandleW(L"user32.dll"), "GetDpiForWindow")) { const UINT v = f(g_hwnd); if (v) dpi = v; }
    d.s = dpi / 96.0 * g_theme.zoom;
    d.fText  = ::CreateFontW(-Px(&d, 15), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Microsoft JhengHei");
    d.fBtn   = ::CreateFontW(-Px(&d, 14), 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Microsoft JhengHei");
    d.fTitle = ::CreateFontW(-Px(&d, 13), 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Microsoft JhengHei");
    // size: golden MyMessageBox is 472 wide; the panel grows with the text
    const int W = Px(&d, 472);
    HDC dc = ::GetDC(g_hwnd); HGDIOBJ o = ::SelectObject(dc, d.fText);
    RECT m = { 0, 0, W - Px(&d, 16 + 24), 0 };
    ::DrawTextW(dc, text.c_str(), -1, &m, DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS);
    ::SelectObject(dc, o); ::ReleaseDC(g_hwnd, dc);
    const int textH = std::max((int)(m.bottom - m.top), Px(&d, 44));
    const int editH = (kind == 2) ? Px(&d, 34) : 0;
    const int panelH = textH + Px(&d, 20) + editH;
    const int H = Px(&d, d.titleH) + Px(&d, 8) + panelH + Px(&d, 10) + Px(&d, 33) + Px(&d, 12);
    const int bw = Px(&d, 130), bh = Px(&d, 33), gap = Px(&d, 16);
    const int total = d.nBtn * bw + (d.nBtn - 1) * gap;
    for (int i = 0; i < d.nBtn; ++i) {
        d.btn[i].left = (W - total) / 2 + i * (bw + gap); d.btn[i].right = d.btn[i].left + bw;
        d.btn[i].top = H - Px(&d, 12) - bh;              d.btn[i].bottom = d.btn[i].top + bh;
    }
    RECT own; ::GetWindowRect(g_hwnd, &own);
    const int x = own.left + ((own.right - own.left) - W) / 2, y = own.top + ((own.bottom - own.top) - H) / 2;
    HWND h = ::CreateWindowExW(0, L"HT9045HmiDlg", g_title.c_str(), WS_POPUP, x, y, W, H, g_hwnd, 0, hi, &d);
    if (!h) { Log(L"styled dialog: CreateWindow failed %lu", ::GetLastError()); return kind == 0; }
    if (kind == 2) {
        d.edit = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", defText.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                   Px(&d, 20), Px(&d, d.titleH) + Px(&d, 8) + Px(&d, 10) + textH + Px(&d, 4), W - Px(&d, 40), Px(&d, 26), h, 0, hi, 0);
        ::SendMessageW(d.edit, WM_SETFONT, (WPARAM)d.fText, TRUE);
        g_editProc = (WNDPROC)::SetWindowLongPtrW(d.edit, GWLP_WNDPROC, (LONG_PTR)DlgEditProc);
        ::SendMessageW(d.edit, EM_SETSEL, 0, -1);
    }
    ::EnableWindow(g_hwnd, FALSE);
    ::ShowWindow(h, SW_SHOW);
    ::SetForegroundWindow(h);
    ::SetFocus(d.edit ? d.edit : h);
    MSG msg;
    while (!d.done && ::GetMessageW(&msg, 0, 0, 0) > 0) {
        if (msg.message == g_quitMsg && g_quitMsg) { ::PostMessageW(g_hwnd, g_quitMsg, 0, 0); DlgFinish(&d, kind == 0); break; }   // program closing: leave the box
        ::TranslateMessage(&msg); ::DispatchMessageW(&msg);
    }
    ::EnableWindow(g_hwnd, TRUE);
    ::DestroyWindow(h);
    ::SetForegroundWindow(g_hwnd);
    ::DeleteObject(d.fText); ::DeleteObject(d.fBtn); ::DeleteObject(d.fTitle);
    if (result) *result = d.result;
    return d.ok;
}

// A page's alert / confirm / prompt waits for this (WebView2 deferral); shown after the event handler returns.
struct PendingDlg { ICoreWebView2ScriptDialogOpeningEventArgs* args; ICoreWebView2Deferral* def; int kind; std::wstring text, defText; };
std::vector<PendingDlg> g_pendingDlg;
bool g_inDlg = false;

void RunPendingDialogs()
{
    if (g_inDlg) return;
    g_inDlg = true;
    while (!g_pendingDlg.empty()) {
        PendingDlg p = g_pendingDlg.front(); g_pendingDlg.erase(g_pendingDlg.begin());
        std::wstring res;
        const bool ok = (p.kind == 3) ? true : StyledDialog(p.kind, p.text, p.defText, &res);   // 3 = beforeunload: leave
        if (ok) { if (p.kind == 2) p.args->put_ResultText(res.c_str()); p.args->Accept(); }
        p.def->Complete(); p.def->Release(); p.args->Release();
    }
    g_inDlg = false;
}

std::wstring ExeDir()
{
    wchar_t p[MAX_PATH] = L"";
    ::GetModuleFileNameW(0, p, MAX_PATH);
    std::wstring s(p);
    const size_t k = s.find_last_of(L"\\/");
    return k == std::wstring::npos ? std::wstring(L".") : s.substr(0, k);
}

bool StartsWithI(const std::wstring& s, const wchar_t* pre)
{
    const size_t n = wcslen(pre);
    return s.size() >= n && _wcsnicmp(s.c_str(), pre, n) == 0;
}

// Only this machine's pages: the web server on the loopback address, and the local boot_wait.html.
bool AllowedUrl(const std::wstring& u)
{
    return StartsWithI(u, L"http://127.0.0.1:") || StartsWithI(u, L"http://127.0.0.1/") ||
           StartsWithI(u, L"http://localhost:") || StartsWithI(u, L"http://localhost/") ||
           StartsWithI(u, L"file:///") || StartsWithI(u, L"about:blank") || StartsWithI(u, L"data:");
}

// AI(W906-HMI-SHELL-2) 20260930: EastSun (screenshot of the title bar)「圖片上的我不需要」-- no title bar, no border, no min /
//   max / close buttons: the window covers its monitor's WORK AREA exactly (the Windows taskbar below stays visible and shows
//   this program's icon), and follows the work area when the resolution or the taskbar changes.
void FitWorkArea()
{
    if (!g_hwnd || ::IsIconic(g_hwnd)) return;
    MONITORINFO mi; ZeroMemory(&mi, sizeof(mi)); mi.cbSize = sizeof(mi);
    RECT r;
    if (::GetMonitorInfoW(::MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) r = mi.rcWork;
    else ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &r, 0);
    ::SetWindowPos(g_hwnd, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

void Resize()
{
    if (!g_ctl || !g_hwnd) return;
    RECT r; ::GetClientRect(g_hwnd, &r);
    g_ctl->put_Bounds(r);
}

void Settings()
{
    ICoreWebView2Settings* s = 0;
    if (FAILED(g_web->get_Settings(&s)) || !s) return;
    s->put_AreDevToolsEnabled(g_debug ? TRUE : FALSE);
    s->put_AreDefaultContextMenusEnabled(g_debug ? TRUE : FALSE);
    s->put_IsStatusBarEnabled(FALSE);
    s->put_IsZoomControlEnabled(FALSE);
    s->put_AreDefaultScriptDialogsEnabled(FALSE);  // AI(W906-HMI-DLG) 20260930: alert / confirm / prompt drawn by this window in the HMI style (StyledDialog)
    ICoreWebView2Settings3* s3 = 0;
    if (SUCCEEDED(s->QueryInterface(IID_ICoreWebView2Settings3, (void**)&s3)) && s3) { s3->put_AreBrowserAcceleratorKeysEnabled(FALSE); s3->Release(); }
    else Log(L"Settings3 not available: browser shortcuts could not be turned off");
    ICoreWebView2Settings4* s4 = 0;
    if (SUCCEEDED(s->QueryInterface(IID_ICoreWebView2Settings4, (void**)&s4)) && s4) { s4->put_IsGeneralAutofillEnabled(FALSE); s4->put_IsPasswordAutosaveEnabled(FALSE); s4->Release(); }
    ICoreWebView2Settings5* s5 = 0;
    if (SUCCEEDED(s->QueryInterface(IID_ICoreWebView2Settings5, (void**)&s5)) && s5) { s5->put_IsPinchZoomEnabled(FALSE); s5->Release(); }
    ICoreWebView2Settings6* s6 = 0;
    if (SUCCEEDED(s->QueryInterface(IID_ICoreWebView2Settings6, (void**)&s6)) && s6) { s6->put_IsSwipeNavigationEnabled(FALSE); s6->Release(); }
    s->Release();
}

void Relaunch()
{
    // the browser process died: start a fresh copy of this program with the same command line, then leave
    STARTUPINFOW si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof(pi));
    std::wstring cmd = ::GetCommandLineW();
    g_quitting = true;
    if (g_mutex) { ::ReleaseMutex(g_mutex); ::CloseHandle(g_mutex); g_mutex = 0; }   // before the new copy asks for it
    ::DestroyWindow(g_hwnd);
    if (::CreateProcessW(0, &cmd[0], 0, 0, FALSE, 0, 0, 0, &si, &pi)) { ::CloseHandle(pi.hThread); ::CloseHandle(pi.hProcess); }
    Log(L"browser process exited -> relaunched this window");
}

void OnController(ICoreWebView2Controller* ctl)
{
    g_ctl = ctl; g_ctl->AddRef();
    g_ctl->get_CoreWebView2(&g_web);
    if (!g_web) { Log(L"no CoreWebView2"); return; }
    Settings();
    EventRegistrationToken t;
    g_web->add_NavigationStarting(new Cb<ICoreWebView2NavigationStartingEventHandler, ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs*>(
        IID_ICoreWebView2NavigationStartingEventHandler, [](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* a) -> HRESULT {
            LPWSTR u = 0;
            if (SUCCEEDED(a->get_Uri(&u)) && u) {
                if (!AllowedUrl(u)) { a->put_Cancel(TRUE); Log(L"navigation blocked: %ls", u); }
                ::CoTaskMemFree(u);
            }
            return S_OK;
        }), &t);
    g_web->add_NewWindowRequested(new Cb<ICoreWebView2NewWindowRequestedEventHandler, ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs*>(
        IID_ICoreWebView2NewWindowRequestedEventHandler, [](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* a) -> HRESULT {
            LPWSTR u = 0; a->get_Uri(&u);
            if (!g_debug || !u || !AllowedUrl(u)) { a->put_Handled(TRUE); Log(L"new window blocked: %ls", u ? u : L"?"); }
            if (u) ::CoTaskMemFree(u);
            return S_OK;
        }), &t);
    g_web->add_DocumentTitleChanged(new Cb<ICoreWebView2DocumentTitleChangedEventHandler, ICoreWebView2*, IUnknown*>(
        IID_ICoreWebView2DocumentTitleChangedEventHandler, [](ICoreWebView2* w, IUnknown*) -> HRESULT {
            LPWSTR s = 0;
            if (SUCCEEDED(w->get_DocumentTitle(&s)) && s) {
                if (*s) { g_title = s; ::SetWindowTextW(g_hwnd, s); }
                ::CoTaskMemFree(s);
            }
            return S_OK;
        }), &t);
    g_web->add_ProcessFailed(new Cb<ICoreWebView2ProcessFailedEventHandler, ICoreWebView2*, ICoreWebView2ProcessFailedEventArgs*>(
        IID_ICoreWebView2ProcessFailedEventHandler, [](ICoreWebView2* w, ICoreWebView2ProcessFailedEventArgs* a) -> HRESULT {
            COREWEBVIEW2_PROCESS_FAILED_KIND k = COREWEBVIEW2_PROCESS_FAILED_KIND_BROWSER_PROCESS_EXITED;
            a->get_ProcessFailedKind(&k);
            Log(L"process failed, kind %d", (int)k);
            if (k == COREWEBVIEW2_PROCESS_FAILED_KIND_BROWSER_PROCESS_EXITED) Relaunch();
            else if (k == COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED ||
                     k == COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_UNRESPONSIVE) w->Reload();
            return S_OK;
        }), &t);
    g_ctl->add_AcceleratorKeyPressed(new Cb<ICoreWebView2AcceleratorKeyPressedEventHandler, ICoreWebView2Controller*, ICoreWebView2AcceleratorKeyPressedEventArgs*>(
        IID_ICoreWebView2AcceleratorKeyPressedEventHandler, [](ICoreWebView2Controller*, ICoreWebView2AcceleratorKeyPressedEventArgs* a) -> HRESULT {
            COREWEBVIEW2_KEY_EVENT_KIND kind; UINT vk = 0;
            a->get_KeyEventKind(&kind); a->get_VirtualKey(&vk);
            // Alt+F4 while the page has the focus: the same one question as the window's own close (AskClose)
            if (kind == COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN && vk == VK_F4) {
                a->put_Handled(TRUE);
                ::PostMessageW(g_hwnd, WM_CLOSE, 0, 0);
                return S_OK;
            }
            // debug only: Ctrl+Shift+F12 opens DevTools (F12 / Ctrl+Shift+I are browser shortcuts, and those are off)
            if (g_debug && kind == COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN && vk == VK_F12 &&
                (::GetKeyState(VK_CONTROL) & 0x8000) && (::GetKeyState(VK_SHIFT) & 0x8000)) {
                a->put_Handled(TRUE);
                if (g_web) g_web->OpenDevToolsWindow();
            }
            return S_OK;
        }), &t);
    // AI(W906-HMI-DLG) 20260930: the page's alert / confirm / prompt -> StyledDialog (the page waits on the deferral)
    g_web->add_ScriptDialogOpening(new Cb<ICoreWebView2ScriptDialogOpeningEventHandler, ICoreWebView2*, ICoreWebView2ScriptDialogOpeningEventArgs*>(
        IID_ICoreWebView2ScriptDialogOpeningEventHandler, [](ICoreWebView2*, ICoreWebView2ScriptDialogOpeningEventArgs* a) -> HRESULT {
            PendingDlg p; p.args = a; p.def = 0; p.kind = 0;
            COREWEBVIEW2_SCRIPT_DIALOG_KIND k = COREWEBVIEW2_SCRIPT_DIALOG_KIND_ALERT; a->get_Kind(&k);
            p.kind = (k == COREWEBVIEW2_SCRIPT_DIALOG_KIND_CONFIRM) ? 1 : (k == COREWEBVIEW2_SCRIPT_DIALOG_KIND_PROMPT) ? 2
                   : (k == COREWEBVIEW2_SCRIPT_DIALOG_KIND_BEFOREUNLOAD) ? 3 : 0;
            LPWSTR s = 0;
            if (SUCCEEDED(a->get_Message(&s)) && s) { p.text = s; ::CoTaskMemFree(s); }
            s = 0; if (SUCCEEDED(a->get_DefaultText(&s)) && s) { p.defText = s; ::CoTaskMemFree(s); }
            if (FAILED(a->GetDeferral(&p.def)) || !p.def) return S_OK;   // no deferral: WebView2 treats it as dismissed
            a->AddRef();
            g_pendingDlg.push_back(p);
            ::PostMessageW(g_hwnd, WM_APP + 3, 0, 0);
            return S_OK;
        }), &t);
    ::SetTimer(g_hwnd, 7, 3000, 0);                      // the theme / zoom of the page for StyledDialog
    Resize();
    g_ctl->put_IsVisible(TRUE);
    Log(L"navigate %ls (debug=%d)", g_url.c_str(), (int)g_debug);
    g_web->Navigate(g_url.c_str());
}

bool StartWebView()
{
    const std::wstring dll = ExeDir() + L"\\WebView2Loader.dll";
    HMODULE h = ::LoadLibraryW(dll.c_str());
    if (!h) { Log(L"cannot load %ls (err %lu)", dll.c_str(), ::GetLastError()); return false; }
    typedef HRESULT (STDAPICALLTYPE *CreateEnvFn)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
    typedef HRESULT (STDAPICALLTYPE *VersionFn)(PCWSTR, LPWSTR*);
    CreateEnvFn create = (CreateEnvFn)(void*)::GetProcAddress(h, "CreateCoreWebView2EnvironmentWithOptions");
    VersionFn   ver    = (VersionFn)(void*)::GetProcAddress(h, "GetAvailableCoreWebView2BrowserVersionString");
    if (!create) { Log(L"WebView2Loader.dll has no CreateCoreWebView2EnvironmentWithOptions"); return false; }
    if (ver) {
        LPWSTR v = 0;
        if (SUCCEEDED(ver(0, &v)) && v) { Log(L"WebView2 Runtime %ls", v); ::CoTaskMemFree(v); }
        else { Log(L"no WebView2 Runtime installed"); return false; }
    }
    wchar_t lad[MAX_PATH] = L"";
    ::GetEnvironmentVariableW(L"LOCALAPPDATA", lad, MAX_PATH);
    const std::wstring udf = std::wstring(lad[0] ? lad : L".") + L"\\HT9045_HMI_Shell";
    const HRESULT hr = create(0, udf.c_str(), 0,
        new Cb<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, HRESULT, ICoreWebView2Environment*>(
            IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, [](HRESULT e, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(e) || !env) { Log(L"environment failed 0x%08lx", (unsigned long)e); ::PostMessageW(g_hwnd, WM_APP + 2, 0, 0); return S_OK; }
                env->CreateCoreWebView2Controller(g_hwnd,
                    new Cb<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, HRESULT, ICoreWebView2Controller*>(
                        IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, [](HRESULT e2, ICoreWebView2Controller* c) -> HRESULT {
                            if (FAILED(e2) || !c) { Log(L"controller failed 0x%08lx", (unsigned long)e2); ::PostMessageW(g_hwnd, WM_APP + 2, 0, 0); return S_OK; }
                            OnController(c);
                            return S_OK;
                        }));
                return S_OK;
            }));
    if (FAILED(hr)) { Log(L"CreateCoreWebView2EnvironmentWithOptions 0x%08lx", (unsigned long)hr); return false; }
    return true;
}

void AskClose()
{
    if (g_closeAsked) return;
    g_closeAsked = true;
    const int a = StyledDialog(1, L"確定要關閉軟體嗎？", L"", 0) ? IDYES : IDNO;   // AI(W906-HMI-DLG) 20260930: same style as the HMI's boxes
    g_closeAsked = false;
    if (a != IDYES) return;
    if (!g_web) { Log(L"close: no page -> close window"); g_quitting = true; ::DestroyWindow(g_hwnd); return; }
    Log(L"close: YES -> main screen Exit");
    g_web->ExecuteScript(L"(window.HT9045ShellCloseRequest ? window.HT9045ShellCloseRequest() : 'none')",
        new Cb<ICoreWebView2ExecuteScriptCompletedHandler, HRESULT, LPCWSTR>(
            IID_ICoreWebView2ExecuteScriptCompletedHandler, [](HRESULT e, LPCWSTR json) -> HRESULT {
                const std::wstring r = json ? json : L"";
                Log(L"close: page answered 0x%08lx %ls", (unsigned long)e, r.c_str());
                // 'exit' = the main screen's Exit runs (its guards may refuse and say why; on success wb_serve posts the quit)
                if (FAILED(e) || r.find(L"exit") == std::wstring::npos) { g_quitting = true; ::DestroyWindow(g_hwnd); }
                return S_OK;
            }));
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (g_quitMsg && m == g_quitMsg) {                       // wb_serve's normal close has finished
        Log(L"quit message from wb_serve");
        g_quitting = true; ::DestroyWindow(h); return 0;
    }
    switch (m) {
    case WM_SIZE: Resize(); return 0;
    case WM_DISPLAYCHANGE: FitWorkArea(); return 0;
    case WM_SETTINGCHANGE: if (w == SPI_SETWORKAREA) FitWorkArea(); break;
    case WM_CLOSE:
        if (g_quitting) { ::DestroyWindow(h); return 0; }
        AskClose();
        return 0;
    case WM_APP + 2:
        StyledDialog(0, L"無法啟動畫面元件（WebView2）。\n請確認 Microsoft Edge WebView2 Runtime 已安裝。\n\n詳細記錄："
                        L"%LOCALAPPDATA%\\HT9045_HMI_Shell\\hmi_shell.log", L"", 0);   // AI(W906-HMI-DLG) 20260930
        g_quitting = true; ::DestroyWindow(h); return 0;
    case WM_APP + 3: RunPendingDialogs(); return 0;           // AI(W906-HMI-DLG) 20260930: a page's alert / confirm / prompt
    case WM_TIMER: if (w == 7) RefreshTheme(); return 0;
    case WM_DESTROY:
        if (g_ctl) { g_ctl->Close(); g_ctl->Release(); g_ctl = 0; }
        if (g_web) { g_web->Release(); g_web = 0; }
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(h, m, w, l);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, LPWSTR, int nShow)
{
    wchar_t lad[MAX_PATH] = L"";
    ::GetEnvironmentVariableW(L"LOCALAPPDATA", lad, MAX_PATH);
    const std::wstring dir = std::wstring(lad[0] ? lad : L".") + L"\\HT9045_HMI_Shell";
    ::CreateDirectoryW(dir.c_str(), 0);
    g_logPath = dir + L"\\hmi_shell.log";

    // arguments: [URL] [--debug|--release]
    int argc = 0; LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    int forceMode = 0;                                      // 1 debug, 2 release
    for (int i = 1; argv && i < argc; ++i) {
        if (!_wcsicmp(argv[i], L"--debug")) forceMode = 1;
        else if (!_wcsicmp(argv[i], L"--release")) forceMode = 2;
        else if (g_url.empty()) g_url = argv[i];
    }
    if (argv) ::LocalFree(argv);
    if (g_url.empty()) {
        wchar_t e[2048] = L"";
        if (::GetEnvironmentVariableW(L"W906_HMI_URL", e, 2048) > 0) g_url = e;
        else g_url = L"http://127.0.0.1:8055/background.html";
    }
    g_debug = (forceMode == 1) || (forceMode == 0 && g_url.find(L"mode=debug") != std::wstring::npos);

    HANDLE mx = ::CreateMutexW(0, TRUE, kMutex);
    g_mutex = mx;
    if (mx && ::GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND other = ::FindWindowW(kClass, 0);
        if (other) { if (::IsIconic(other)) ::ShowWindow(other, SW_RESTORE); ::SetForegroundWindow(other); }
        Log(L"second start (%ls) -> brought the running window to the front", g_url.c_str());
        return 0;
    }
    Log(L"start %ls", g_url.c_str());

    typedef HRESULT (WINAPI *AppIdFn)(PCWSTR);
    if (HMODULE sh = ::GetModuleHandleW(L"shell32.dll")) {
        if (AppIdFn f = (AppIdFn)(void*)::GetProcAddress(sh, "SetCurrentProcessExplicitAppUserModelID")) f(kAppId);
    }
    ::CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    g_quitMsg = ::RegisterWindowMessageW(kQuitMsg);

    WNDCLASSEXW wc; ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.hIcon = (HICON)::LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
    wc.hIconSm = (HICON)::LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    wc.hCursor = ::LoadCursorW(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kClass;
    ::RegisterClassExW(&wc);
    // WS_POPUP: no caption / frame (HMI-SHELL-2); WS_SYSMENU keeps Alt+F4 -> WM_CLOSE; WS_EX_APPWINDOW keeps the taskbar button
    g_hwnd = ::CreateWindowExW(WS_EX_APPWINDOW, kClass, g_title.c_str(), WS_POPUP | WS_SYSMENU, 0, 0, 1280, 800, 0, 0, hi, 0);
    if (!g_hwnd) { Log(L"CreateWindow failed %lu", ::GetLastError()); return 2; }
    // maximized like the machine program; a caller that asked for minimized (tests, a start-up shortcut) gets that instead
    const bool minimized = (nShow == SW_SHOWMINIMIZED || nShow == SW_SHOWMINNOACTIVE || nShow == SW_MINIMIZE);
    if (!minimized) FitWorkArea();
    ::ShowWindow(g_hwnd, minimized ? SW_SHOWMINNOACTIVE : SW_SHOW);
    ::UpdateWindow(g_hwnd);

    if (!StartWebView()) {
        ::PostMessageW(g_hwnd, WM_APP + 2, 0, 0);
    }
    MSG msg;
    while (::GetMessageW(&msg, 0, 0, 0) > 0) { ::TranslateMessage(&msg); ::DispatchMessageW(&msg); }
    Log(L"exit");
    ::CoUninitialize();
    if (g_mutex) { ::ReleaseMutex(g_mutex); ::CloseHandle(g_mutex); g_mutex = 0; }
    return 0;
}
