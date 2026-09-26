// =============================================================================
//  WebLotInfo.h -- Data.LotInfo.html 的 WS 指令 lotinfo.op（golden V912 TfLotInfo，uLotInfo.cpp）C++ 入口。
//  Steven 20260925 (Data.LotInfo 其餘分頁)。本體與兩段式確認的說明在 WebLotInfo.cpp 檔頭。
//
//  整合者加分派時 include 這個標頭（宣告在全域命名空間）。例（tools/wb_serve.cpp，照 smartdiag.op／builder.op 那一臂的形狀）：
//      } else if (wc.cmd == "lotinfo.op") {
//          extern std::string W906_LotInfoOp(const std::string& payloadJson, bool* ok);
//          const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
//          bool liOk = false; std::string liRes;
//          try { liRes = W906_LotInfoOp(payload, &liOk); } catch (...) { liOk = false; liRes = "exception in golden TfLotInfo"; }
//          server.CompleteCommand((unsigned long long)wc.id, liOk, liRes);
//      }
// =============================================================================
#ifndef HT9045_WEBLOTINFO_H
#define HT9045_WEBLOTINFO_H

#include <string>

// payloadJson = {"op":"barcode.clearCount","confirmed":bool} | {"op":"testerLog.get"} | {"op":"selection.get"} |
//               {"op":"selection.save","values":{"chkTempOffset":bool,...}}。回傳 JSON；執行完成時含 "executed":true。
// *ok = 執行完成，或走到確認框（needConfirm）＝ true；參數錯＝ false。
// 自己持 FormLock（CRITICAL_SECTION，同執行緒可重入），外面再包一層也不會死結。
std::string W906_LotInfoOp(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBLOTINFO_H
