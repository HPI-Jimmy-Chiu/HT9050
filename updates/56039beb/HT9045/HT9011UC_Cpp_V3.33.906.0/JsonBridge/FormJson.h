// ===========================================================================
//  JsonBridge/FormJson.h -- S12：/api/form 的外殼與表單物件鎖。
//
//  Steven 20260924.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/references/decisions.md 二之二、
//        phases.md S12。
//
//  AI(W906-Q4-S126) 20260927 (St02): 第一型（呼叫移植樹的 DoIniDataToForm＋兩輪哨兵，WidgetRef／
//    FormDesc／W()，gen/form_*.gen.cpp）退役，見 FormJson.cpp 檔頭。現在只有第二型（FormBridge.h：
//    golden 原檔產生的 bridge，HotPlate 在用）。
//
//  端點：GET /api/form/          有第二型 bridge 的頁面清單
//        GET /api/form/<Page>    {"page","form","kind":"golden-bridge","widgets":{...},...}；沒有 bridge → 404
//        WS  form.save           FormSave
// ===========================================================================
#pragma once

#include <string>

namespace ht9045 {
namespace formjson {

// GET /api/form/ 與 /api/form/<Page> 的本體。回傳 HTTP 狀態碼，body 寫進 *json。
int FormListJson(std::string* json);
int FormPageJson(const std::string& page, std::string* json);
// WS form.save 的本體（第二型 bridge）。回 HTTP 風格狀態碼；200 時 *ackJson 是 ack 內容，否則 *err。
int FormSave(const std::string& page, const std::string& widgetsJson,
             std::string* ackJson, std::string* err);

// /api/form 與「存檔後重讀」共用的鎖：表單的顯示本體跑在 HTTP 執行緒，
// ReadFile() 跑在 tick 執行緒，兩者會碰同一批表單物件。
void FormLock();
void FormUnlock();

}  // namespace formjson
}  // namespace ht9045
