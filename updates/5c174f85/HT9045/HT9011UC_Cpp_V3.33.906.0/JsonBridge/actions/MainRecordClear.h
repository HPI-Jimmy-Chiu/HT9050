// ===========================================================================
//  JsonBridge/actions/MainRecordClear.h -- 主畫面 Record 分頁（main.dfm tsRecord）的兩個動作（S119）
//
//  AI(W906-S119) 20260927 (St02).  github-59 GO（FROM_STEVEN §1 00:02 認領）；帳本 docs/S119_MAIN_RECORD.md。
//  golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy
//    act.main.clearRecord    ← main.cpp:31109-31138 `TfMain::spbClearRecordClick`（CLEAR 按鈕）
//    act.main.meShuttle2Dbl  ← main.cpp:29471-29475 `TfMain::meShuttle2DblClick`（AseRecordMemo 的 OnDblClick，
//                              main.dfm:16259；清的是 meShuttle2／meShuttle1，不是 AseRecordMemo 自己）
//  命名照 ht9045-json-bridge api-shape.md §4.5 規則 1（去掉 spb 前綴與 Click 後綴）。
//
//  ⚠ 守衛：golden 的 CLEAR 沒有確認框（github-59：照 golden，不加）；只有 IniConfig.bShowMainDebugRecord 一關，
//    預設 false（CosFunction.cpp:4208），只有 CC_SIGURD_HUKOU（:1074）／CC_RICHTEK（:2959）打開 ⇒ 其他客戶按了是 no-op。
//    單一操作員權杖由 WebBridgeServer 在分派之前擋（跟其他 act.main.* 一樣）；唯讀開關 SetReadOnly(!allowCmd) 自 ZEROARG
//    （wb_serve.cpp:3670）起恆為可寫，沒有參數關得掉。
//  ⚠ 執行緒：wb_serve 在主迴圈的 drain 呼叫 HandleActionWithTag（tick 執行緒＝golden 主執行緒），ArmData 在這裡改是安全的。
//  分派：JsonBridge/ChanAction.cpp（include、兩行分派、兩個 schema；github-59 GO，FROM_STEVEN §1 a31aaaf7），
//    wb_serve 與 test_sjson_chan 都編本檔。ctest：MainRecord_Clear（本體）、SjsonChan（分派走得到，dryRun）。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_ACTIONS_MAINRECORDCLEAR_H
#define HT9045_JSONBRIDGE_ACTIONS_MAINRECORDCLEAR_H

#include <string>

namespace webbridge { class JsonWriter; }

// golden 本體（不含 JSON）。回傳值只給動作與 ctest 分辨停在哪。
enum W906ClearRecordStop
{
    kCrDone = 0,          // 跑完 golden 本體（UpdateRecordScreen(true) 那一行除外，見 .cpp 的 S113 gate）
    kCrDebugRecordOff     // IniConfig.bShowMainDebugRecord==false：golden :31111 直接 return
};
W906ClearRecordStop W906_SpbClearRecordClick();   // golden main.cpp:31109-31138

// AI(W906-S119) 20260927: R1 -- golden `UpdateRecordScreen(true)` (main.cpp:31137).  The body is St01's S113
//   W906_TfMain_UpdateRecordScreen (FileRW/MainRecord.cpp), compiled only into wb_serve, while this file is also in the
//   ctests; so it is a pointer St01 installs at its own wb_serve boot line once S113 is in main (the forms/fMain.cpp:507
//   W906_*Hook pattern).  NULL = not installed: CLEAR skips that one line.
extern void (*W906_UpdateRecordScreenBody)(bool);
void W906_MeShuttle2DblClick();                   // golden main.cpp:29471-29475

namespace ht9045 {
namespace sjson {

// payload：{"dryRun":true} 只跑守衛；沒帶或 false 就執行（RULINGS_20260926 第 12 條，同 act.main.clarnData）。
std::string DoClearRecordAction(const std::string& payloadJson);
std::string DoMeShuttle2DblAction(const std::string& payloadJson);

// ActionSchemaJson() 的 actions 陣列裡的物件。
void WriteClearRecordActionSchema(webbridge::JsonWriter& w);
void WriteMeShuttle2DblActionSchema(webbridge::JsonWriter& w);

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_ACTIONS_MAINRECORDCLEAR_H
