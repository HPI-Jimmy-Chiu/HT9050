// =============================================================================
//  WebLevelSet.h -- Status.Security.html 權限表（system\levelset.dat）存檔：WS system.levels.put 的本體。
//  //AI(W906-FRW-S64) 20260926: 新檔（Steven 團隊，S64）。golden V912 TfSecurity::FormClose（cSecurity.cpp:439-468）
//  ＋SetLevelSet（:1511-1515）。本體說明在 WebLevelSet.cpp 檔頭。
//
//  分派（tools/wb_serve.cpp 既有的 system.levels.put 那一臂改成呼叫這支；片段見 S64 報告）：
//      } else if (wc.cmd == "system.levels.put") { ... W906_LevelSetPut(tag, payload, gAllowSystemWrite, &ok) ... }
//  建置：CMakeLists.txt wb_serve 來源清單（WebSecurityJam.cpp 同一行）加 WebLevelSet.cpp。
// =============================================================================
#ifndef HT9045_WEBLEVELSET_H
#define HT9045_WEBLEVELSET_H

#include <string>

// tag         = "levelset"（或 "levelset.dat"，不分大小寫）
// payloadJson = {"values":{"<AccessLevel 索引>":<等級>,...}, "dryRun":bool}   稀疏：沒列的格子＝golden 畫面上沒動的 radio
//               {"op":"layout"}   //AI(W906-FRW-S64F) 20260927: 唯讀版面（Q28=B：radio 選項、隱藏／停用格子、PageControl1 可見性），
//                                 不需要 --allow-system-write、不讀檔進記憶體、不寫任何東西。分派不用改（同一臂 system.levels.put）。
// allowSystemWrite = wb_serve --allow-system-write（沒帶時只有 dryRun 與 layout 能跑，語意同舊版）
// 回傳：*ok=true 時是 ack JSON 物件 {changed, identical, notFound, blocked[], clamped, normalized, jamSaved, backup, verified, ...}；
//       *ok=false 時是錯誤訊息（純文字），什麼都沒寫、記憶體 LevelSet 維持原值。
//       //AI(W906-FRW-S64F) 20260927: Q26=B 停用／隱藏／PageControl1 看不到的格子不再整批拒寫 —— 只擋那一格（列在 blocked[]），其餘照存。
//       Q30=B 真的寫檔改走 golden SecurityExitClick → FormClose（SavePassword／ReadPassword 在這條路跳過，Q24 排後）。
// 自己持 FormLock（呼叫端不要先拿）。
std::string W906_LevelSetPut(const std::string& tag, const std::string& payloadJson, bool allowSystemWrite, bool* ok);

#endif // HT9045_WEBLEVELSET_H
