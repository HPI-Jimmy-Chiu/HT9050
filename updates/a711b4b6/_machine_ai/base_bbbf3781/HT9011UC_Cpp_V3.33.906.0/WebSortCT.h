// =============================================================================
//  WebSortCT.h -- Data.SortCT.html 的 Clear Count 鈕（golden TfSortCT::btnClearCountClick）C++ 入口。
//  Steven 20260925 (Data.SortCT)。本體與兩段式確認的說明在 WebSortCT.cpp 檔頭。
//
//  整合者加分派時 include 這個標頭（宣告在全域命名空間；在 namespace 裡寫區塊內 extern 會宣告成
//  ht9045::sjson::W906_SortCTClearCount，連結時找不到）。例（JsonBridge/ChanAction.cpp HandleActionWithTag）：
//      if (cmd == "act.sortCT.clearCount") { bool ok = false; return ::W906_SortCTClearCount(payloadJson, &ok); }
// =============================================================================
#ifndef HT9045_WEBSORTCT_H
#define HT9045_WEBSORTCT_H

#include <string>

// payloadJson = {"confirmed":bool}。回傳 JSON；執行完成時含 "executed":true（act.* 的 ok 慣例）。
// *ok = 執行完成，或走到確認框（needConfirm）＝ true；被 golden 守衛擋下／參數錯＝ false。
// 自己持 FormLock（CRITICAL_SECTION，同執行緒可重入），外面再包一層也不會死結。
std::string W906_SortCTClearCount(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBSORTCT_H
