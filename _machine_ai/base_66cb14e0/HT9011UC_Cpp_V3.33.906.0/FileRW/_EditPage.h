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

class HTEditList;

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

}  // namespace filerw
