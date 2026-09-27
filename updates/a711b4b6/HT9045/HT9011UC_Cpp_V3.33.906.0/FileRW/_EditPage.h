// ===========================================================================
//  FileRW/_EditPage.h -- C 路（golden 表單橋，HTEditList 形狀）的共用頁面層：開頁 JSON ＋ 存檔。
//
//  Steven 20260924.  NOT in golden.
//  規格：.claude/skills/ht9045-html-json/references/route-c-golden-bridge.md、
//        .claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二。
//
//  FileRW/IniConfig.cpp 是第一個 C 路結構（手寫，含 config.ini 專屬的必送／保留規則）。第二個起
//  （Ld_UldDelayTime …）共用這裡：每個結構的 cpp 只給一份 PageDesc（golden 方法由 tools/gen_editlist.py
//  產生），WS editlist.get／editlist.save 依 tag 找到它。規則與 IniConfig 相同：
//    * editlist.get ＝ golden 開頁（FormShow），回 {struct, form, lists, proxies, mustSend}
//    * editlist.save 要在「同一個 AccessLevel 下開過頁」之後（否則 409）；
//      存檔流程讀、但不在任何清單裡的替身（mustSend）頁面沒送 → 400；
//      不可改的元件（自己或祖先停用／看不見、ReadOnly、清單筆 bEnable=false）頁面送的值丟掉（ack.ignored）；
//      然後跑 golden 的存檔流程（saveFlow），saved ＝ trace 裡有 savedMark。
// ===========================================================================
#pragma once

#include <string>
#include <vector>

#include "FileRW/_FormEvent.h"   // AI(W906-FRW-S157) 20260927 [W906]：formevent::Request／Result（只用 <string>）

class HTEditList;
namespace vclcompat { class TControl; }   // AI(W906-FRW-S157) 20260927 [W906]：PageEvent::handler 的參數（vclcompat/Controls.h:213）

namespace filerw {

struct PageDesc {
    const char* tag;                 // WS editlist.* 的 tag（＝結構名，例 "Ld_UldDelayTime"）
    const char* form;                // golden 表單類別（替身的鍵，例 "TfLd_ULd"）
    const char* page;                // HTML 頁（文件用，例 "Setup.Ld_ULd.html"）
    HTEditList** const* lists;       // 這個表單的 HTEditList（指向全域指標）
    const char* const* listNames;    // 與 lists 對應的名稱（JSON 的鍵）
    int nLists;
    const char* const* saveReads;    // 產生器掃出的「存檔流程讀的替身」
    int nSaveReads;
    void (*formShow)();              // golden FormShow（開頁）
    void (*saveFlow)();              // golden 存檔鈕（例 spbSaveClick）
    const char* savedMark;           // ELMarked(savedMark) ⇒ 真的寫了檔（例 "SaveSetupFile"）
    void (*reload)();                // 沒寫檔時把替身還原成檔案值（golden 關頁 FormClose 的 ReadFile）
    bool (*booted)();                // 開機註冊做完了沒
    // ---- 以下可選（Steven 團隊 20260925，TfSetup 起用；其他頁不設＝nullptr，行為不變）----
    // 套值前的 golden 事件：golden 某些元件「值一改就觸發事件」、事件會改別的元件的可見／可改／選項
    // （例 TfSetup ScrollBar1Change 依新 Test Mode 重建 Site 格子、CoSocketComboChange 決定哪幾個 rgSensor 看得見）。
    // PageSave 在必送檢查之後、「不可改的丟掉」之前呼叫：hook 對頁面值與伺服器端不同的那幾個元件自己套值＋跑 golden 事件，
    // 回傳它處理掉的名稱（PageSave 不再套、算 applied，ack.events 列出）→ 丟值照「事件之後」的可見／可改判斷。
    // 其餘元件照舊：頁面最後狀態、不觸發事件。hook 跑過之後若 PageSave 因頁面值錯誤回 400，會先 reload() 還原。
    void (*beforeApply)(const std::string& widgetsJson, std::vector<std::string>* handled);
    // editlist.get 回應多帶的 "extra":<JSON>（例 TfSetup：golden CompChange 執行期重建的 Site 下拉選項）
    std::string (*extraJson)();
};

void RegisterPage(const PageDesc* d);
const PageDesc* FindPage(const std::string& tag);

// WS editlist.get：呼叫端持 FormLock、在主迴圈。回 HTTP 式狀態碼。
int PageJson(const PageDesc& d, std::string* json);
// WS editlist.save：呼叫端持 FormLock、在主迴圈。回 HTTP 式狀態碼；成功時 *ack 是 JSON。
int PageSave(const PageDesc& d, const std::string& widgetsJson, const std::string& answersJson,
             std::string* ack, std::string* err);

// 靜態註冊：各結構 cpp 裡 `static filerw::PageRegistrar reg(&kPage);`
struct PageRegistrar {
    explicit PageRegistrar(const PageDesc* d) { RegisterPage(d); }
};

// ---- AI(W906-FRW-S157) 20260927 [W906]：WS form.event 的 C 路（Steven ★ Q40＝A，RULINGS_20260926 S157）--------------
// 事件表另外註冊、不加進 PageDesc（PageDesc 是逐欄位初始化的彙總型別，加欄位會讓每個結構的 cpp 多一個
// -Wmissing-field-initializers 警告）。表由 tools/gen_editlist.py 依 tools/editlist/<struct>.py 的 'events' 產生
// （k<P>_Events），各結構的手寫入口 cpp 用 PageEventsRegistrar 註冊。
struct PageEvent {
    const char* control;                          // golden 元件名（＝頁面元件 id）
    const char* event;                            // "change"／"click"（＝頁面元件的 data-ht-event）
    const char* golden;                           // 處理器的 golden 位置
    void (*handler)(vclcompat::TControl* sender); // 轉出的 golden 處理器（Sender＝這個元件的替身）
};
void RegisterPageEvents(const char* tag, const PageEvent* table, int n);
struct PageEventsRegistrar {
    PageEventsRegistrar(const char* tag, const PageEvent* table, int n) { RegisterPageEvents(tag, table, n); }
};
// tag 可以是結構名（"UserDefForm_File"）或頁名（"Setup.TrayForm"／"Setup.TrayForm.html"，比對 PageDesc::page）
const PageDesc* FindPageForEvent(const std::string& tag);
// WS form.event 的 C 路本體：呼叫端持 FormLock、在主迴圈。回 false 時 out->code／out->why 填好。
//   流程：開過頁（同 editlist.save 的 AccessLevel 規則）→ 點得到嗎（ELOperable）→ 套 state（不可改的丟掉，同 PageSave）
//   → 套控制項自己的值 → 記替身狀態 → golden 處理器 → 比對前後，回有變的替身。
bool RunPageEvent(const PageDesc& d, const formevent::Request& r, formevent::Result* out);

}  // namespace filerw
