// ===========================================================================
//  JsonBridge/FormBridge.h -- S12 第二型：直接由 golden BCB 原檔產生的表單 JSON bridge。
//
//  Steven 20260924.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/references/decisions.md 二之三、phases.md S12
//
//  使用者 20260924：「DoIniDataToForm 不需要翻譯吧？直接使用 BCB 的原檔，設計成
//  JSON bridge 就好了」「不要管 cpp 版本的，直接參考 bcb 版本做成 JSON bridge + 寫檔；
//  讀檔的部分，如果沒有實作的，就列入待辦」。
//
//  所以沒有 widget 物件：tools/gen_formbridge.py 讀 golden 的 TfXxx::DoIniDataToForm()、
//  SaveSetupFile() 與它們呼叫到的事件處理器，控制流程原樣保留，只把
//      widget->Prop = 右式;      改寫成  J.SetProp("widget", 右式);
//      widget->Prop（讀）        改寫成  J.GetProp("widget")
//  右式照抄，對的是移植樹已有的全域結構（TestIF_File、CosFunction…）與 WriteIniData。
//
//  兩個方向用同一個 FormState：
//    顯示（GET /api/form/<Page>）：golden DoIniDataToForm 寫進 J → ToJson()
//    存檔（WS form.save）        ：頁面送來的值 FromJson() 進 J → golden SaveSetupFile 讀 J → WriteIniData
//
//  與第一型（FormJson.h：呼叫移植樹翻好的 DoIniDataToForm＋兩輪哨兵）的差別：
//  這裡**有執行到的賦值才會進 J**，不需要哨兵；也不需要表單的 widget 宣告。
// ===========================================================================
#pragma once

#include <map>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"   // AnsiString

namespace ht9045 {
namespace formbridge {

struct WidgetState {
    bool hasText = false, hasIndex = false, hasChecked = false, hasCaption = false;
    bool hasVisible = false, hasEnabled = false, hasTabVisible = false;
    bool hasActivePage = false, hasDown = false, hasItems = false;
    AnsiString text, caption;
    int  itemIndex = -1, activePageIndex = 0;
    bool checked = false, visible = true, enabled = true, tabVisible = true, down = false;
    std::vector<AnsiString> items;
};

class FormState {
public:
    // ---- golden 的賦值（widget->Prop = v）------------------------------------
    void SetText(const char* id, const AnsiString& v);
    void SetItemIndex(const char* id, int v);
    void SetChecked(const char* id, bool v);
    void SetCaption(const char* id, const AnsiString& v);
    void SetVisible(const char* id, bool v);
    void SetEnabled(const char* id, bool v);
    void SetTabVisible(const char* id, bool v);
    void SetActivePageIndex(const char* id, int v);
    void SetDown(const char* id, bool v);

    // ---- TComboBox／TListBox 的 Items -----------------------------------------
    void ItemsClear(const char* id);
    void ItemsAdd(const char* id, const AnsiString& v);
    int  ItemCount(const char* id) const;
    AnsiString Item(const char* id, int i) const;

    // ---- golden 的讀值（widget->Prop）------------------------------------------
    // 沒被設過的屬性回 VCL 建構子的預設值（Text=""、ItemIndex=-1、Checked=false、
    // Visible／Enabled=true）。TComboBox 的 Text 在沒設過、但有 Items 與 ItemIndex 時
    // 回 Items[ItemIndex]（VCL csDropDownList 的行為）。
    AnsiString GetText(const char* id) const;
    int  GetItemIndex(const char* id) const;
    bool GetChecked(const char* id) const;
    AnsiString GetCaption(const char* id) const;
    bool GetVisible(const char* id) const;
    bool GetEnabled(const char* id) const;
    bool GetTabVisible(const char* id) const;
    bool GetDown(const char* id) const;

    // 表單自己的成員變數（golden 的 fShow、bflag…），預設 0。
    int& M(const char* name) { return members_[name]; }

    // golden 的 ShowMyMessage／MessageBox：不在 GET 裡彈窗，改成 JSON 的 messages 讓頁面決定。
    void Message(const AnsiString& en, const AnsiString& zh = AnsiString(""));
    // 產生器或人工判定「這一行沒有照 golden 做」的地方，照實進 JSON 的 todo。
    void Todo(const std::string& what);

    const std::vector<std::string>& Todos() const { return todos_; }
    bool Has(const char* id) const { return w_.count(id) != 0; }

    // Steven 20260924：golden 讀了、但這份狀態裡沒有的屬性（"id.prop"）。
    // form.save 先用它空跑 SaveSetupFile（寫到暫存夾），有缺就整筆拒寫 ——
    // 靜態的 saveReads 會把條件分支裡才讀的欄位也算進去，動態才準。
    const std::vector<std::string>& MissingReads() const { return missing_; }

    // {"widgets":{id:{text,itemIndex,checked,caption,visible,enabled,tabVisible,
    //   activePageIndex,down,items}}, "messages":[{en,zh}], "todo":[...]}
    // 只有 widgets/messages/todo 三個鍵；外層的 page/form 由呼叫端包。
    std::string WidgetsJson() const;
    std::string MessagesJson() const;
    std::string TodoJson() const;

    // 頁面送來的 {id:{text?,itemIndex?,checked?}}（存檔方向）。失敗回 false 並寫 *err。
    bool FromJson(const std::string& widgetsJson, std::string* err);

private:
    WidgetState& W(const char* id);
    const WidgetState* Find(const char* id) const;

    std::map<std::string, WidgetState> w_;
    std::vector<std::string> order_;          // 第一次出現的順序，JSON 照這個輸出
    std::map<std::string, int> members_;
    std::vector<std::pair<AnsiString, AnsiString> > messages_;
    std::vector<std::string> todos_;
    mutable std::vector<std::string> missing_;
    void Miss(const char* id, const char* prop) const;
};

// 每個表單一份，由 gen/bridge_<Class>.gen.cpp 提供。
struct BridgeDesc {
    const char* page;         // "Setup.TesterIF.html"
    const char* formClass;    // "TFTestIF"
    const char* golden;       // "cTesterIF.cpp"
    // 顯示：golden 開表單時的順序（建構子的 Init* ＋ DoIniDataToForm）
    void (*display)(FormState& J);
    // 存檔（只寫檔）：golden SaveSetupFile(szDir, S)。szDir 是配方資料夾（不含檔名）。
    // ctest 直接呼叫它寫到暫存夾；NULL＝這頁沒有存檔 bridge
    void (*save)(FormState& J, AnsiString szDir, AnsiString S);
    // 存檔（完整流程）：golden 存檔鈕（例：spbSaveClick）—— 權限守衛、SaveSetupFile、SECS、
    // 存後重讀、備份、SetWorkParameter。WS form.save 走這條。
    void (*saveFlow)(FormState& J);
    // save 可能讀到的 widget（產生器靜態抽出），逗號分隔。**只給頁面參考**：頁面照它收集、
    // 只送有可信來源的欄位。真正的缺值判定在 form.save 的空跑（FormState::MissingReads，動態）——
    // 靜態清單會把條件分支裡才讀的欄位也算進去（審查更正 20260924，原本拿它當拒寫依據）。
    const char* saveReads;
    // 讀檔端缺口：非空＝填 display 用到的結構的 golden 讀檔器在移植樹還沒有 / 沒接上，
    // display 算出來的值是結構初值，**頁面不可拿它蓋檔案值、也不可走 form.save**。
    const char* sourceGap;
};

const BridgeDesc* FindBridge(const std::string& page);
std::size_t BridgeCount();
const BridgeDesc* BridgeAt(std::size_t i);

}  // namespace formbridge
}  // namespace ht9045
