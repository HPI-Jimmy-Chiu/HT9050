// ===========================================================================
//  JsonBridge/FormJson.h -- S12：C++ 跑 DoIniDataToForm()，把表單狀態送成 JSON。
//
//  Steven 20260924.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/references/decisions.md 二之二、
//        phases.md S12。
//
//  使用者 20260924：「DoIniDataToForm() 就等於是 C++ 發送 JSON 給 HTML」。
//  所以 widget ↔ 結構欄位的對照不在這裡，也不在 JS —— 它就是各表單翻譯好的
//  TfXxx::DoIniDataToForm() 本身。本層只做三件事：
//    1. 呼叫 DoIniDataToForm()（golden 的條件分支用機台真實旗標判斷）
//    2. 找出**這次呼叫真的有賦值**的 widget 屬性（見 FormJson.cpp 的兩輪哨兵法）
//    3. 依產生的 widget 表（gen/form_<Class>.gen.cpp）輸出 JSON
//
//  端點：GET /api/form/          已接的頁面清單
//        GET /api/form/<Page>    {"page","form","widgets":{id:{text|checked|
//                                  itemIndex|caption|position|down|visible|enabled}}}
//
//  ⚠ 只送「這次有賦值」的屬性。widget 建構子的預設值（Text=""、Visible=true）
//    和頁面上 dfm 的預設值不同；golden 某個分支沒碰到的 widget 如果照送，
//    會把頁面上正確的值蓋成空字串。
// ===========================================================================
#pragma once

#include <string>
#include <vector>

#include "forms/FormWidgets.h"
#include "vclcompat/ScrollBar.h"

namespace ht9045 {
namespace formjson {

// widget 的種類決定要看哪些屬性。依 vclcompat 的型別，由 W() 多載自動判定，
// 產生器不必知道型別名稱。
enum WidgetKind {
    kEdit,         // TCustomEdit 家族（TEdit／TLabeledEdit／TMemo）：Text
    kCheckBox,     // Checked, Caption
    kRadioButton,  // Checked, Caption
    kRadioGroup,   // ItemIndex（vclcompat 的 TRadioGroup 沒有 Caption）
    kComboBox,     // ItemIndex, Text
    kListBox,      // ItemIndex
    kLabel,        // Caption
    kPanel,        // Caption
    kGroupBox,     // Caption
    kScrollBar,    // Position
    kSpeedButton,  // Down, Caption
    kButton,       // Caption（TButton／TBitBtn）
    kOther         // 只看 Visible／Enabled
};

struct WidgetRef {
    const char* name;
    WidgetKind  kind;
    TControl*   ctl;
};

inline WidgetRef W(const char* n, TCustomEdit* c)  { return WidgetRef{n, kEdit, c}; }
inline WidgetRef W(const char* n, TCheckBox* c)    { return WidgetRef{n, kCheckBox, c}; }
inline WidgetRef W(const char* n, TRadioButton* c) { return WidgetRef{n, kRadioButton, c}; }
inline WidgetRef W(const char* n, TRadioGroup* c)  { return WidgetRef{n, kRadioGroup, c}; }
inline WidgetRef W(const char* n, TComboBox* c)    { return WidgetRef{n, kComboBox, c}; }
inline WidgetRef W(const char* n, TListBox* c)     { return WidgetRef{n, kListBox, c}; }
inline WidgetRef W(const char* n, TLabel* c)       { return WidgetRef{n, kLabel, c}; }
inline WidgetRef W(const char* n, TPanel* c)       { return WidgetRef{n, kPanel, c}; }
inline WidgetRef W(const char* n, TGroupBox* c)    { return WidgetRef{n, kGroupBox, c}; }
inline WidgetRef W(const char* n, vclcompat::TScrollBar* c) { return WidgetRef{n, kScrollBar, c}; }   // 全域另有 handlerlog.h 的 TScrollBar，這裡指 vclcompat 那個
inline WidgetRef W(const char* n, TSpeedButton* c) { return WidgetRef{n, kSpeedButton, c}; }
inline WidgetRef W(const char* n, TButton* c)      { return WidgetRef{n, kButton, c}; }
inline WidgetRef W(const char* n, TBitBtn* c)      { return WidgetRef{n, kButton, c}; }
inline WidgetRef W(const char* n, TControl* c)     { return WidgetRef{n, kOther, c}; }

// 每個表單一份產生的描述（gen/form_<Class>.gen.cpp）。
struct FormDesc {
    const char* page;        // "Setup.HotPlate.html"
    const char* formClass;   // "TfHotPlate"
    const char* portSource;  // DoIniDataToForm() 在移植樹的位置
    const char* golden;      // golden 的位置
    // 表單實例不存在（NULL）時回 false，其餘兩個函式不會被呼叫。
    bool (*collect)(std::vector<WidgetRef>* out);
    void (*run)();           // 呼叫 TfXxx::DoIniDataToForm()
    const char* skipped;     // 產生器略過的成員（陣列等），逗號分隔，給 JSON 誠實回報
};

// GET /api/form/ 與 /api/form/<Page> 的本體。回傳 HTTP 狀態碼，body 寫進 *json。
int FormListJson(std::string* json);
int FormPageJson(const std::string& page, std::string* json);
// WS form.save 的本體（第二型 bridge）。回 HTTP 風格狀態碼；200 時 *ackJson 是 ack 內容，否則 *err。
int FormSave(const std::string& page, const std::string& widgetsJson,
             std::string* ackJson, std::string* err);

// /api/form 與「存檔後重讀」共用的鎖：DoIniDataToForm() 跑在 HTTP 執行緒，
// ReadFile() 跑在 tick 執行緒，兩者會碰同一批表單物件。
void FormLock();
void FormUnlock();

}  // namespace formjson
}  // namespace ht9045
