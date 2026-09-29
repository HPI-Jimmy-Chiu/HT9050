// ===========================================================================
//  ui/native/NativeHost.h
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生視窗共用的「主機」—— 訊息泵、拖曳／改大小期間的主迴圈保活、快速鍵、字型。
//  NOT in golden（golden 的這一層是 VCL 的 Application->Run／TTimer）。
//
//  ⚠ 為什麼要「保活」（keepalive）：
//    使用者按住標題列拖曳、拉邊框改大小、按住捲軸拇指、打開系統選單時，Windows 會在 DefWindowProc 裡面
//    跑它自己的訊息迴圈，直到放開滑鼠才回來。泵掛在 wb_serve 主迴圈（同 golden 單執行緒、1203 單執行緒規則），
//    所以那段時間 **主迴圈整個停住**：MainProc、1203 Poll、Index 煞車保護（DoAvoidIndexMotorFallDown）都不跑。
//    golden 不會這樣：VCL 的 TTimer 是 WM_TIMER，Windows 的那些內部迴圈照樣會分派 WM_TIMER，所以 golden 的
//    Timer1Timer／Timer2Timer 在拖曳時仍在跑。這裡用同一個機制：每個原生視窗建立時開一個 50 ms 的視窗計時器；
//      * 平常由我們自己的泵拿到它 —— 直接丟掉（主迴圈正在跑，不需要）；
//      * 只有在「我們的泵正在分派某則訊息，而那則訊息裡面跑起了 Windows 的內部迴圈」時，計時器才會被別人分派到
//        視窗程序 —— 這時呼叫泵的呼叫端給的 keepalive（wb_serve 主迴圈那一處給「跑一拍主迴圈」，
//        阻塞等待框那一處給「只跑 Index 煞車保護」，見 tools/wb_serve.cpp 檔尾）。
//    仍在主迴圈執行緒上（Windows 的內部迴圈就在這條執行緒），1203 單執行緒規則不變。
//    ⓘ ST01-E 原本建議只在 WM_ENTERSIZEMOVE～WM_EXITSIZEMOVE 開計時器；這裡改成視窗活著就一直開，
//      因為捲軸拇指、系統選單、標題列按鈕的內部迴圈都**不送** WM_ENTERSIZEMOVE（捲軸是表格控件 NativeGrid 自己的）。
//      代價：泵每秒多丟掉約 20 則 WM_TIMER。
//
//  ⚠ 全部函式只能在同一條執行緒呼叫（建視窗、泵訊息的那條）。
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEHOST_H
#define W906_UI_NATIVE_NATIVEHOST_H

// ⚠ 不定 WIN32_LEAN_AND_MEAN：本標頭會先於機台標頭被 include（NativeFormsWbServe.cpp），而 Motor/HTMotor.h:92、cmydef.h:127
//   用的 `byte` 來自完整 <windows.h> 帶進來的 rpcndr.h —— 定了 LEAN 就是 "'byte' does not name a type"（20260929 ON 建置實測）。
#include <windows.h>

#include <string>

namespace w906native {

typedef void (*KeepaliveFn)();
typedef void (*HotkeyFn)();
// enter=true：某個原生視窗進入拖曳／改大小（WM_ENTERSIZEMOVE）；false：結束（WM_EXITSIZEMOVE），kaCalls＝這段期間 keepalive 被呼叫幾次。
typedef void (*SizeMoveNotifyFn)(bool enter, unsigned long kaCalls);

// 泵本執行緒的訊息，最多 maxMessages 則（有上限：滑鼠拖過表格會產生一串訊息，不能卡住主迴圈），回傳處理幾則。
// keepalive 可為 0（不保活）。可以巢狀呼叫（keepalive 本身跑到阻塞等待框時，那裡會再泵一次），每一層各有自己的 keepalive。
int  PumpThreadMessages(int maxMessages, KeepaliveFn keepalive);

// 每個原生視窗：WM_CREATE 呼叫 HostWindowCreated、WM_DESTROY 呼叫 HostWindowDestroyed，
// 並在視窗程序最前面先問 HostWindowMessage（回 true＝主機處理掉了，視窗程序直接 return 0）。
void HostWindowCreated(HWND hwnd);
void HostWindowDestroyed(HWND hwnd);
bool HostWindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void HostSetSizeMoveNotify(SizeMoveNotifyFn fn);

// 快速鍵（RegisterHotKey(NULL, …)，綁在本執行緒的訊息佇列）。失敗（別的程式佔用）回 false，不影響其他功能。
bool HostRegisterHotkey(int id, UINT mods, UINT vk, HotkeyFn fn);
void HostUnregisterHotkey(int id);

// 共用小工具
std::wstring Widen(const std::string& utf8);       // UTF-8 → UTF-16；不是合法 UTF-8 就退回 CP_ACP
HFONT        HostFont(bool bold);                   // 微軟正黑體 15／17 px，整個行程共用一份，不必刪

// 測試用統計
unsigned long HostKeepaliveCalls();
unsigned long HostTimerDropped();                   // 我們自己的泵丟掉的保活計時器訊息數
unsigned long HostSizeMoveEntered();
int           HostPumpDepth();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEHOST_H
