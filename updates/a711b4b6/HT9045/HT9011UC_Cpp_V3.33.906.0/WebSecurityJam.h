// =============================================================================
//  WebSecurityJam.h -- Status.Security.html Jam 分頁（tsJamCode／tsStatisticsJam）的 WS 指令 security.jam。
//  Steven 團隊 20260926。golden V912 TfSecurity（cSecurity.cpp）；本體說明在 WebSecurityJam.cpp 檔頭。
//
//  分派（tools/wb_serve.cpp，towerlight.op 那一行前面同行插入）：
//      } else if (wc.cmd == "security.jam") { ... W906_SecurityJamOp(payload, &ok) ... }
// =============================================================================
#ifndef HT9045_WEBSECURITYJAM_H
#define HT9045_WEBSECURITYJAM_H

#include <string>

// payloadJson = {"op":"open"} | {"op":"select","from":{...},"to":{...},"values":{...}} | {"op":"save","from":{...},"values":{...}} |
//               {"op":"import","from":{...},"csvBase64":"..."} | {"op":"export","from":{...}} | {"op":"stats"} |
//               {"op":"exportStatus","job":N} | {"op":"exportChunk","job":N,"offset":o} | {"op":"exportRelease","job":N}
//               //AI(W906-SEC-S54) 20260926: export 立刻回 {"started":true,"job":N}，背景跑完後用 exportStatus／exportChunk 取（本體說明在 .cpp 檔頭）
// 回傳 JSON；*ok = 執行完成（含 golden 權限擋下的 guard 回應）＝ true；參數錯＝ false。自己持 FormLock。
// ⚠ 不碰密碼：golden FormClose 的 SavePassword／ReadPassword（cSecurity.cpp:464/:467）不在這裡，login.dat 的值不會出現在回應。
std::string W906_SecurityJamOp(const std::string& payloadJson, bool* ok);

//AI(W906-SEC-S54) 20260926: export 改在背景執行緒跑（Steven S54）。wb_serve 關站時呼叫這支收掉執行緒（cancel＋join，數十毫秒）。
//  插入點（tools/wb_serve.cpp，正常關站那一行，接在 WriteIniDataGeneral("Record", "Program Close", 1); 後面、// 註解前面）：
//      { extern void W906_SecurityJamShutdown(); W906_SecurityJamShutdown(); }
//  沒插也不會當掉：WbThread 解構只 CloseHandle，檔內的工作物件在行程結束時自己 cancel＋join（保底）。
void W906_SecurityJamShutdown();

#endif // HT9045_WEBSECURITYJAM_H
