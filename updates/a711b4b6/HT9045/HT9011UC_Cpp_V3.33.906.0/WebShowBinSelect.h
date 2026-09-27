// =============================================================================
//  WebShowBinSelect.h -- Status.ShowBinSelect.html 的 Index 分頁 Clear Counter 鈕（golden TfShowBinSelect::btnClearCountClick）C++ 入口。
//  AI(W906-PROD-S114) 20260926（Steven 團隊）。本體與兩段式確認的說明在 WebShowBinSelect.cpp 檔頭。
//
//  宣告在全域命名空間（同 WebSortCT.h 的理由：在 namespace 裡寫區塊內 extern 會宣告成別的符號，連結時找不到）。
//  分派：tools/wb_serve.cpp WS 指令 act.showBinSelect.clearCount（片段見交件報告；wb_serve.cpp 是共用檔）。
// =============================================================================
#ifndef HT9045_WEBSHOWBINSELECT_H
#define HT9045_WEBSHOWBINSELECT_H

#include <string>

// payloadJson = {"confirmed":bool}。回傳 JSON；執行完成時含 "executed":true（act.* 的 ok 慣例）。
// *ok = 執行完成，或走到確認框（needConfirm）＝ true；被 golden 守衛擋下／參數錯＝ false。
// 自己持 FormLock（CRITICAL_SECTION，同執行緒可重入），外面再包一層也不會死結。
std::string W906_ShowBinSelectClearCount(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBSHOWBINSELECT_H
