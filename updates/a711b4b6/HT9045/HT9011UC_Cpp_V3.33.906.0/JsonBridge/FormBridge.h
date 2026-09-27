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
//  與第一型（呼叫移植樹翻好的 DoIniDataToForm＋兩輪哨兵；AI(W906-Q4-S126) 20260927 退役）的差別：
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
    // AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Q40＝A，RULINGS_20260926 S157）—— FormState::ClearTouched() 之後
    //   被賦值過的屬性（kTouch* 位元，值與原本相同也算：golden 的賦值會蓋掉畫面上的值）。
    unsigned touched = 0;
};

// AI(W906-FRW-S157) 20260927 [W906]：WidgetState::touched 的位元
enum : unsigned {
    kTouchText = 1u, kTouchIndex = 2u, kTouchChecked = 4u, kTouchCaption = 8u, kTouchVisible = 16u,
    kTouchEnabled = 32u, kTouchTabVisible = 64u, kTouchActivePage = 128u, kTouchDown = 256u, kTouchItems = 512u
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

    // ---- AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Q40＝A，RULINGS_20260926 S157；FileRW/_FormEvent.cpp）----
    // ClearTouched()：處理器跑之前清掉記號。TouchedJson()：只列處理器「有賦值」的屬性（值與原本相同也列 ——
    // golden 的賦值會蓋掉畫面上的值；頁面手改過、沒放進 state 的欄位也要被蓋回去），鍵名與 WidgetsJson 相同
    // （頁面用 /api/form 同一個套值函式；沒賦值的鍵不出現，不送 null）。
    void ClearTouched();
    std::string TouchedJson() const;
    // ClearTouched() 之後才加進來的 messages／todo（處理器自己的；display 的在開頁時已回過）
    std::string MessagesJsonSinceClear() const;
    std::string TodoJsonSinceClear() const;
    // 只讀：這個元件目前的狀態；沒被設過回 nullptr（form.event 判斷「golden 使用者點得到嗎」用）。
    const WidgetState* Peek(const char* id) const { return Find(id); }

private:
    WidgetState& W(const char* id);
    const WidgetState* Find(const char* id) const;

    std::map<std::string, WidgetState> w_;
    std::vector<std::string> order_;          // 第一次出現的順序，JSON 照這個輸出
    std::map<std::string, int> members_;
    std::vector<std::pair<AnsiString, AnsiString> > messages_;
    std::vector<std::string> todos_;
    mutable std::vector<std::string> missing_;
    std::size_t msgMark_ = 0, todoMark_ = 0;   // AI(W906-FRW-S157) 20260927 [W906]：ClearTouched() 當下的 messages_／todos_ 筆數
    void Miss(const char* id, const char* prop) const;
};

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 的事件表（Q40＝A，RULINGS_20260926 S157）。
//   tools/gen_formbridge.py 由 tools/formbridge/<Class>.py 的 'events' 產生；golden 事件處理器照 methods 同樣機械轉換。
struct EventGuard {           // golden DFM：控制項自己＋每一層容器（由內而外）與它們設計期的 Enabled／Visible
    const char* name;
    bool dfmEnabled;
    bool dfmVisible;
};
struct EventDesc {
    const char* control;      // golden 元件名（＝頁面元件 id）
    const char* event;        // "change"／"click"（＝頁面元件的 data-ht-event）
    const char* golden;       // 處理器的 golden 位置（例 "cHotPlate.cpp:412 TfHotPlate::cbSelectHPFromDBChange"）
    void (*handler)(FormState& J);
    const EventGuard* chain;  // 跑完 display 後，鏈上任一層 Enabled＝false 或 Visible＝false ⇒ golden 使用者點不到，拒絕
    int nChain;
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
    // AI(W906-FRW-S157) 20260927 [W906]：WS form.event（見上面 EventDesc）。nullptr／0＝這頁沒有接事件。
    const EventDesc* events;
    int nEvents;
    // golden header 宣告的元件名（逗號分隔）：form.event 分辨 unknown-control（golden 沒這個元件）與
    // no-handler（有這個元件、沒接這個事件）。
    const char* widgets;
};

const BridgeDesc* FindBridge(const std::string& page);
std::size_t BridgeCount();
const BridgeDesc* BridgeAt(std::size_t i);

}  // namespace formbridge
}  // namespace ht9045

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 的 A 形狀本體（FormBridge.cpp；型別在 FileRW/_FormEvent.h）。
//   流程：golden display → 點得到嗎（EventDesc::chain）→ 套 state → 套控制項自己的值 → ClearTouched → golden 處理器
//   → TouchedJson。呼叫端（FileRW/_FormEvent.cpp W906_FormEvent）持 FormLock。回 false 時 out->code／out->why 填好。
namespace formevent { struct Request; struct Result; }
namespace ht9045 {
namespace formbridge {
bool RunEvent(const BridgeDesc& b, const formevent::Request& r, formevent::Result* out);
}  // namespace formbridge
}  // namespace ht9045
