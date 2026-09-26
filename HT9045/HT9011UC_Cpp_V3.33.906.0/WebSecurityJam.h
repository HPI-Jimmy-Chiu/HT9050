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
//               {"op":"import","from":{...},"csvBase64":"..."} | {"op":"export","from":{...}} | {"op":"stats"}
// 回傳 JSON；*ok = 執行完成（含 golden 權限擋下的 guard 回應）＝ true；參數錯＝ false。自己持 FormLock。
// ⚠ 不碰密碼：golden FormClose 的 SavePassword／ReadPassword（cSecurity.cpp:464/:467）不在這裡，login.dat 的值不會出現在回應。
std::string W906_SecurityJamOp(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBSECURITYJAM_H
