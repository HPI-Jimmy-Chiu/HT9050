// ===========================================================================
//  JsonBridge/actions/MainTesterConnect.h -- act.main.testerConnect（主畫面 Tester 圖示：On-Line／Off-Line 切換）
//
//  AI(W906-GB-P2e) 20260926.  使用者裁決（github-59 轉達）：「那就幫我進行 C++ 的接線吧」—— 只接 C++，
//  web/page/main-control.js 不動（它現在還走已退場的 HTSimulatorBridge，改頁面要另外問）。
//  golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy / main.cpp:29732-29794
//          `void __fastcall TfMain::imgTesterClick(TObject *Sender)`（906 :28766 起，程式碼相同）
//  本檔把 imgTesterClick 的本體照翻成 W906_ImgTesterClick()，再加網頁要的守衛與 JSON 回報。
//
//  ⚠ 切換本身在 TfMain::ChangeTesterConnect（golden :12581-12778）。它在 P2d 翻好之前是替身
//    （forms/fMain.cpp:510，回 W906_ChangeTesterConnect_Sim、什麼都不改），所以這個動作在 P2d 之前
//    會照 golden 跑完外圍（權限、ASE_CL 確認、OffLineBin、XILINX、P53、SECS 事件），但模式不會變；
//    回應的 modeChanged 會是 false，並說明原因。
//  ⚠ 執行緒：wb_serve 在主迴圈的 drain 呼叫 HandleActionWithTag（tools/wb_serve.cpp:4835），也就是
//    tick 執行緒 —— 等於 golden 的主執行緒。ASE_CL 的是／否框走既有的網頁模態等待。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_ACTIONS_MAINTESTERCONNECT_H
#define HT9045_JSONBRIDGE_ACTIONS_MAINTESTERCONNECT_H

#include <string>

namespace webbridge { class JsonWriter; }

namespace ht9045 {
namespace sjson {

// act.main.testerConnect 的分派本體。payload 不看：golden 的按鈕沒有參數，是切換（ChangeTesterConnect(10)）。
std::string DoTesterConnectAction(const std::string& payloadJson);

// ActionSchemaJson() 的 actions 陣列裡的一個物件。
void WriteTesterConnectActionSchema(webbridge::JsonWriter& w);

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_ACTIONS_MAINTESTERCONNECT_H
