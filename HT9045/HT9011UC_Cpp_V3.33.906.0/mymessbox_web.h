// =============================================================================
//  mymessbox_web.h -- golden mymessbox.h（TMyMessageBox 家族）的網頁宿主接縫
//
//  AI(W906-SMM) 20260925  Steven 20260925 指示：C++ 的 ShowMyMessage 在網頁上看不到，
//  操作員會漏看 golden 的警告 —— 照 golden 在網頁上顯示並能回應。
//  golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy\mymessbox.cpp／.h／.dfm（cp950）。
//
//  ## 為什麼不是加在 canary_support.h
//
//  canary_support.h 被 123 個 TU include；在那裡加宣告會讓整棵樹重編，而且那個
//  標頭會重給 RecordProcess／MyDBIProcessNew 預設引數，tools/wb_serve.cpp 不能
//  include 它（wb_serve.cpp:152-157 的說明）。這裡只放「宿主要看得到的東西」，
//  沒有任何預設引數會跟別的標頭打架（ShowMyMessageBox_YES_NO 在本樹沒有第二個
//  全域宣告：其餘都是 TU 內的 static 替身＋#define，見檔尾清單）。
//
//  ⚠ 這幾個 hook 的**定義**在 canary_support.cpp，**安裝**在 tools/wb_serve.cpp 檔尾
//    （W906_MsgBoxHostInstall）。全域變數在 Itanium ABI 不帶型別，宣告與定義必須逐字
//    相同 —— 所以兩邊都 include 這一份，不要各自手寫 extern。
//
//  ## NULL（預設）= 沒有宿主 = 行為與 20260925 之前完全相同
//  所有測試與非 wb_serve 的行程都沒有安裝，ShowMyMessage 仍只記錄＋printf。
// =============================================================================
#ifndef mymessbox_webH
#define mymessbox_webH

#include "vclcompat/vcl_compat.h"   // AnsiString

// -----------------------------------------------------------------------------
//  1. ShowMyMessage 的完整參數（golden mymessbox.h:58）。
//     既有的 W906_ShowMyMessage_Hook 只帶 S1/S2，而 golden 的按鈕字樣（OK／Pause）
//     由 Ok 決定（mymessbox.cpp:832-835），MyDBIProcess 要 S3（:849）。
//     ⚠ 舊 hook 保留原樣：WebBuilder.cpp／WebSmartDiag.cpp 把它接成「這一次請求的
//       訊息收集器」（訊息放進 ack 的 messages，C 路設計）。canary_support.cpp 的
//       ShowMyMessage 先呼叫舊 hook、再呼叫這一支。
extern void (*W906_ShowMyMessageEx_Hook)(const char* S1, const char* S2, const char* S3,
                                         bool Ok, bool bServoOff);

// -----------------------------------------------------------------------------
//  2. ShowUnloaderTrayMessage（golden mymessbox.cpp:930-955）：MyMessageBox->Show()
//     非阻塞。停不停機由呼叫端事先設好的 iUnLoaderCount 決定（FormShow :303）。
extern void (*W906_ShowUnloaderTrayMessage_Hook)(const char* S1, const char* S2);

// （3. 問答型／4. ShowMyMessageBox_YES_NO：合併 main 時拿掉，改用 Jimmy ee5de164 的 canary_support.h 宣告與 W906_ShowMyMessageBoxYesNo_Hook）

#endif
