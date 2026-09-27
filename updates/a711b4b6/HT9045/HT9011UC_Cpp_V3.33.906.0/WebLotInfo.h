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
//               {"op":"selection.save","values":{"chkTempOffset":bool,...}} |
//               {"op":"barcode.clearList","confirmed":bool} | {"op":"ocr.clearList"}（AI(W906-FRW-S94) 20260926）|
//               {"op":"lotEnd.state"} | {"op":"lotEnd","confirmed":bool}（AI(W906-PROD-S117) 20260926：Lot End，golden sbSECSLotEndClick→SetLotEnd）。
//               回傳 JSON；執行完成時含 "executed":true。
// *ok = 執行完成，或走到確認框（needConfirm）＝ true；參數錯＝ false。
// 自己持 FormLock（CRITICAL_SECTION，同執行緒可重入），外面再包一層也不會死結。
std::string W906_LotInfoOp(const std::string& payloadJson, bool* ok);

// AI(W906-FRW-S94) 20260926（Steven 團隊）：TfLotInfo::btClearBarcodeListClick（golden V912 uLotInfo.cpp:10186-10195）本體的安裝座。
//   指標 W906_ClearBarcodeListBody 在 forms/fLotInfo.h 檔尾；本體 ClearBarcodeListBody 在 WebLotInfo.cpp。
//   wb_serve 開機呼叫一次 W906_InstallLotInfoClearListBody()。裝上之後，**任何**呼叫 fLotInfo->btClearBarcodeListClick() 的地方
//   都會真的清 2D 重複碼清單、把 D:\HT9045_Log\2DBarCode\LotData.txt 寫成空檔、再跑 btClearBarcodeCountClick
//   （今天的呼叫者：lotinfo.op barcode.clearList；Steven02 的 TesterComm/Handler/HandlerGpibMsg.cpp G1／G5 解閘後也是）。
//   移植樹既有的 btClearBarcodeList->Click() 呼叫點仍是 vclcompat no-op，不受影響。
void W906_InstallLotInfoClearListBody();
bool W906_LotInfoClearListBodyInstalled();

#endif // HT9045_WEBLOTINFO_H
