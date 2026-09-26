// ===========================================================================
//  JsonBridge/ChanAction.h -- 動作通道 act.* 的分派表（S11）。
//
//  AI(W906-SJSON-S11) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 4.5、八 S11
//
//  ---------------------------------------------------------------------------
//  形狀
//  ---------------------------------------------------------------------------
//      WS  act.<單元>.<動作>   value = {"...args...", "dryRun":bool}
//          -> {"executed":bool, "guard":"…", "dryRun":bool, "would":[…], …}
//
//  ⚠ 回應**不寫 `ok`**。AckJson（WebBridge/WebBridgeServer.cpp:1262）成功時
//    已經寫了一個 "ok":true，然後把這個物件去掉大括號原地拼接進同一層。
//    兩邊都寫會產生重複鍵。S0/S1 各踩過一次，這裡沿用它們的解法：
//    用 `executed` 當成敗旗標，wb_serve 也看這個鍵。
//
//  ⚠⚠ **上一段只講了一半，而少掉的那一半正是瀏覽器每天會遇到的那一半**
//    （20260923 實測補記，wb_serve --dry --port 8088 逐條打過）。
//    AckJson 的「去掉大括號原地拼接」**只在 ok==true 時發生**
//    （`WebBridgeServer.cpp:1271` 的 `} else if (...)`）。ok==false 時走的是
//    `:1270` 的 `os << ",\"error\":" << QuoteString(error);`
//    —— 整個 JSON 物件被**當成字串轉義**塞進 `error`。
//
//    而本通道 ok 的判準是 `executed==true`（tools/wb_serve.cpp 的分派），
//    所以**每一次 dryRun 預覽、每一次守衛擋下**都是 ok==false。實際線上形狀：
//
//      執行成功  {"type":"ack","id":N,"ok":true,"executed":true,"family":"…"}
//                 ⇒ 欄位在同一層，`ack.executed` 直接讀得到
//      dryRun/守衛 {"type":"ack","id":N,"ok":false,"error":"{\"dryRun\":true,…}"}
//                 ⇒ 瀏覽器必須 `JSON.parse(ack.error)` 才拿得到 would/guard
//
//    ⇒ **兩條路的形狀不一樣**，寫前端的人一定要知道。這不是缺陷而是既有
//      AckJson 契約的結果，而 SKILL.md §4.5 規則 2 明寫守衛要回 `ok:false`，
//      所以這裡照規格走、不去改 ok 的語意。
//    ⇒ 哪天覺得該統一，改的是 wb_serve 那一行 `ok` 的判準（讓「守衛擋下」
//      也算指令成功），不是改 AckJson —— AckJson 是 system.file.put 等
//      其他指令共用的。
//
//  ⓘ counter.clear 收編的**失敗路徑**線上形狀因此也變了：舊版送
//    `"error":"not-authorized"`（純字串），新版送
//    `"error":"{\"executed\":false,\"guard\":\"not-authorized\",…}"`。
//    成功路徑的 `ok` 不變。20260923 實測 `D:\HT9045\web` 與 `client\` 全樹
//    **零處**比對 "not-authorized"／"unknown counter family" 字串，
//    所以今天沒有東西會壞；但這一條要寫下來，否則下一個人看到會以為是回歸。
//
//  ---------------------------------------------------------------------------
//  三條規則（SKILL §4.5，逐條落實）
//  ---------------------------------------------------------------------------
//  R2「守衛一律回報，不吞掉」：golden 的 `return;` 在網頁上會變成「按了沒反應」。
//     每一個 return 對應一個 `guard` 字串，回應一定帶。
//  R3「dryRun 只跑守衛，不跑本體」：頁面用它決定按鈕要不要 disable。
//     ⚠ act.main.clarnData 的 dryRun **預設 false＝直接執行**（AI(W906-CLARN-R12) 20260926，RULINGS_20260926 第 12 條：與 golden 相同）；要預覽明確帶 dryRun:true。
//       本體會寫 `D:\HT9045\system\lastdata.dat`，
//       那條路徑是硬編的，`--dry` 蓋不到（SKILL §十二 R8）。
//       payload 壞掉、型別不對 —— 不執行（只預覽）。確認對話框由瀏覽器端先問完再送（golden 的按鈕處理器有些會先問）。
//  R4「跑不起來的副作用要標 sideEffectsSkipped」：沿用 counter.clear 的作法。
//
//  ---------------------------------------------------------------------------
//  ⚠ 非阻塞（派工單第 3 條）
//  ---------------------------------------------------------------------------
//  本檔任何一條路都**不可以**等對話框、Sleep 或等硬體。tick 迴圈是單執行緒，
//  卡住一次整台停止輪詢。需要操作員確認的（golden 的
//  ShowMyMessageBox_YES_NO）走 S10 的 dialog 通道由**瀏覽器端**先問完再送，
//  不在這裡等。目前兩個動作都沒有確認框。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_CHANACTION_H
#define HT9045_JSONBRIDGE_CHANACTION_H

#include <cstddef>
#include <string>

namespace ht9045 {
namespace sjson {

// 這個指令名是不是 act.* 家族（含別名）。wb_serve 用它決定要不要進本檔。
bool IsActionCommand(const std::string& cmd);

// 執行（或預覽）一個動作。cmd 是完整指令名，payloadJson 是 WS 的 value。
// 回傳要塞進 ack 的 JSON 物件（不含外層 ok）。
std::string HandleAction(const std::string& cmd, const std::string& payloadJson);

// 同上，另外帶 WS 指令的 `tag` 欄。
// ⚠ 存在的理由很具體：舊的 `counter.clear` 把 family 放在 `tag` 而不是 value
//   裡（tools/wb_serve.cpp 的原分支）。要把它收編成別名又不改瀏覽器，
//   就得把那一欄也遞進來。新的 act.* 一律用 value，不用這一欄。
std::string HandleActionWithTag(const std::string& cmd,
                                const std::string& payloadJson,
                                const std::string& wsTag);

// GET /api/struct/act/schema -- 有哪些動作、各自的參數、13 個 tag 的語意、
// golden 呼叫者清單。
std::string ActionSchemaJson();

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_CHANACTION_H
