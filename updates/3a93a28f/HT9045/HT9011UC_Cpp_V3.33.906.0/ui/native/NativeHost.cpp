// ===========================================================================
//  ui/native/NativeHost.cpp
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生視窗共用主機。說明（尤其「拖曳時保活」為什麼這樣做）在 NativeHost.h。
//  只 include <windows.h> 與 STL，不含任何機台碼。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // MinGW.org 6.3：見 NativeIoView.cpp 檔頭
#endif
#include "ui/native/NativeHost.h"

#include <cstring>
#include <map>
#include <set>
#include <string>

namespace w906native {

namespace {

const UINT_PTR kHostTimerId = 0x57D6;   // 每個原生視窗上的保活計時器
const UINT     kHostTimerMs = 50;       // = wb_serve 主迴圈一圈的上限（截止時間迴圈的 50 ms）
const DWORD    kKeepaliveMinGapMs = 40; // 兩個視窗的計時器都被分派時，一圈只跑一次
const int      kMaxDepth = 16;

struct Level {
    KeepaliveFn fn;
    bool        inDispatch;
    DWORD       lastKa;    // 這一層上次跑 keepalive 的時間（0＝這次泵還沒跑過）
};

Level            g_levels[kMaxDepth + 1];
int              g_depth = 0;
int              g_kaRunningDepth = 0;   // 正在跑的 keepalive 屬於哪一層（0＝沒有）
unsigned long    g_kaCalls = 0;
unsigned long    g_dropped = 0;
unsigned long    g_sizeMoveEntered = 0;
unsigned long    g_kaAtEnter = 0;
SizeMoveNotifyFn g_notify = 0;
std::set<HWND>   g_windows;
std::map<int, HotkeyFn> g_hotkeys;
HFONT            g_font = 0;
HFONT            g_fontBold = 0;

bool IsHostWindow(HWND h)
{
    return h != 0 && g_windows.count(h) != 0;
}

void RunKeepalive()
{
    const int d = g_depth;
    if (d <= 0) return;                               // 不在我們的泵裡：別人的訊息迴圈，不插手
    if (!g_levels[d].inDispatch) return;              // 泵自己拿到（理論上已被丟掉），不是內部迴圈
    if (d <= g_kaRunningDepth) return;                // 同一層的 keepalive 正在跑：不重入
    KeepaliveFn fn = g_levels[d].fn;
    if (!fn) return;
    const DWORD now = ::GetTickCount();
    if (g_levels[d].lastKa != 0 && now - g_levels[d].lastKa < kKeepaliveMinGapMs) return;
    g_levels[d].lastKa = now ? now : 1;
    const int prev = g_kaRunningDepth;
    g_kaRunningDepth = d;
    ++g_kaCalls;
    fn();
    g_kaRunningDepth = prev;
}

}  // namespace

std::wstring Widen(const std::string& s)
{
    if (s.empty()) return std::wstring();
    UINT cp = CP_UTF8;
    DWORD flags = MB_ERR_INVALID_CHARS;
    int n = ::MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), 0, 0);
    if (n <= 0) { cp = CP_ACP; flags = 0; n = ::MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), 0, 0); }
    if (n <= 0) return std::wstring();
    std::wstring w((std::size_t)n, L'\0');
    ::MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), &w[0], n);
    return w;
}

HFONT HostFont(bool bold)
{
    HFONT& f = bold ? g_fontBold : g_font;
    if (!f)
        f = ::CreateFontW(bold ? -17 : -15, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                          L"Microsoft JhengHei UI");
    return f;
}

int PumpThreadMessages(int maxMessages, KeepaliveFn keepalive)
{
    if (g_depth >= kMaxDepth) return 0;   // 不該發生；寧可這一層不泵，也不要寫爆陣列
    ++g_depth;
    g_levels[g_depth].fn = keepalive;
    g_levels[g_depth].inDispatch = false;
    g_levels[g_depth].lastKa = 0;
    MSG m;
    int n = 0;
    while (n < maxMessages && ::PeekMessageW(&m, 0, 0, 0, PM_REMOVE)) {
        ++n;
        if (m.message == WM_QUIT) {
            // 這條執行緒上沒有人該發 WM_QUIT（原生視窗都不發）。放回去交給真正的主人，不吃掉它。
            ::PostQuitMessage((int)m.wParam);
            break;
        }
        if (m.message == WM_TIMER && m.wParam == kHostTimerId && IsHostWindow(m.hwnd)) {
            ++g_dropped;   // 主迴圈正在跑：保活計時器不用分派
            continue;
        }
        if (m.message == WM_HOTKEY && m.hwnd == 0) {
            std::map<int, HotkeyFn>::const_iterator it = g_hotkeys.find((int)m.wParam);
            if (it != g_hotkeys.end() && it->second) it->second();
            continue;
        }
        g_levels[g_depth].inDispatch = true;
        bool handled = false;
        const HWND root = m.hwnd ? ::GetAncestor(m.hwnd, GA_ROOT) : 0;
        if (IsHostWindow(root)) handled = ::IsDialogMessageW(root, &m) != 0;   // Tab 在控件之間移動
        if (!handled) {
            ::TranslateMessage(&m);
            ::DispatchMessageW(&m);
        }
        g_levels[g_depth].inDispatch = false;
    }
    g_levels[g_depth].fn = 0;
    --g_depth;
    return n;
}

void HostWindowCreated(HWND hwnd)
{
    g_windows.insert(hwnd);
    ::SetTimer(hwnd, kHostTimerId, kHostTimerMs, 0);
}

void HostWindowDestroyed(HWND hwnd)
{
    ::KillTimer(hwnd, kHostTimerId);
    g_windows.erase(hwnd);
}

bool HostWindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
{
    switch (msg) {
    case WM_TIMER:
        if (wp != kHostTimerId || !IsHostWindow(hwnd)) return false;
        RunKeepalive();   // 能走到這裡，就是 Windows 的某個內部迴圈（拖曳／改大小／捲軸／選單）分派了它
        return true;
    case WM_ENTERSIZEMOVE:
        ++g_sizeMoveEntered;
        g_kaAtEnter = g_kaCalls;
        if (g_notify) g_notify(true, 0);
        return false;     // 讓 DefWindowProc 照常處理
    case WM_EXITSIZEMOVE:
        if (g_notify) g_notify(false, g_kaCalls - g_kaAtEnter);
        return false;
    default:
        return false;
    }
}

void HostSetSizeMoveNotify(SizeMoveNotifyFn fn) { g_notify = fn; }

bool HostRegisterHotkey(int id, UINT mods, UINT vk, HotkeyFn fn)
{
    if (g_hotkeys.count(id)) { g_hotkeys[id] = fn; return true; }
    if (!::RegisterHotKey(0, id, mods, vk)) return false;
    g_hotkeys[id] = fn;
    return true;
}

void HostUnregisterHotkey(int id)
{
    if (!g_hotkeys.count(id)) return;
    ::UnregisterHotKey(0, id);
    g_hotkeys.erase(id);
}

unsigned long HostKeepaliveCalls() { return g_kaCalls; }
unsigned long HostTimerDropped() { return g_dropped; }
unsigned long HostSizeMoveEntered() { return g_sizeMoveEntered; }
int           HostPumpDepth() { return g_depth; }

}  // namespace w906native
