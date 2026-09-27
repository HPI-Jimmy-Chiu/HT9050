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
// AI(W906-FRW-S65) 20260926: payloadJson = {"op":"dblClick","panel":"<元件 id>"} ＝ golden pnlAuto1DblClick（cSortCT.cpp:1933-1950，
//   [O22] 點兩下清單站數量、寫 lastdata.dat；一段式）。*ok＝本體走完才 true。見 WebSortCT.cpp W906_SortCTDblClickOp。
// *ok = 執行完成，或走到確認框（needConfirm）＝ true；被 golden 守衛擋下／參數錯＝ false。
// 自己持 FormLock（CRITICAL_SECTION，同執行緒可重入），外面再包一層也不會死結。
// AI(W906-FRW-S97) 20260926: payloadJson = {"op":"lotId","text":"<字>"} ＝ golden Lot ID 欄＋Timer1Timer（cSortCT.cpp:869-884，
//   運轉中字不同就寫 config.ini [Count] Lot ID；只有 CC_ASE_M，其他客戶回 customer-only）。見 WebSortCT.cpp W906_SortCTLotIdOp。
std::string W906_SortCTClearCount(const std::string& payloadJson, bool* ok);

// AI(W906-FRW-S97) 20260926: golden Timer1（Interval 1000 ms）的每拍入口。主迴圈每拍呼叫（自己節流到 1000 ms、自己持 FormLock）；
//   Timer1 沒開（非 CC_ASE_M，或網頁還沒送過 op=lotId）直接 return。接線片段見交件報告（tools/wb_serve.cpp 是共用檔）。
void W906_SortCTTimer1Tick();

#endif // HT9045_WEBSORTCT_H
