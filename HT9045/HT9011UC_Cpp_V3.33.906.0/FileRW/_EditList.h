// ===========================================================================
//  FileRW/_EditList.h -- C 類（HTEditList）讀寫檔的共用層：具名替身 ＋ JSON 匯出／套用。
//
//  Steven 20260924.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二、decisions.md 二之三。
//
//  使用者 20260924：「CPP 端沒有元件，但是元件在 html 端」「所以結構 new 的時候要稍作修正」
//  「元件的部分改用名稱即可，因為改到 html 端去使用了」。
//
//  golden 的註冊碼 `elConfig->Add(cbA01, &IniConfig.bA01…, ECBool, "Function", "…", …)` 由產生器
//  （tools/gen_editlist.py）從 golden 原檔改寫成 `elConfig->Add(EL<TCheckBox>("cbA01"), …)`：
//    * EL<T>("名稱") 回傳一個**具名替身**：vclcompat 的 T（TEdit／TCheckBox／TComboBox…），只存值、
//      不是畫面元件。同一個名稱永遠拿到同一個物件，所以 golden 對它的 `->Tag`／`->Visible` 照舊成立。
//    * 替身建立時登記名稱（HTEditList_RegisterControlName），HTEditList::Add() 建立 THTEdit 時填進
//      THTEdit::ControlName —— 那就是 HTML 元件的 id。
//    * HTEditList 的 ReadEditTextFromFile／SaveEditTextToFile／InitialDataToEdit 本體**完全不動**，
//      它們照 golden 讀寫替身的 Text／Checked／ItemIndex。
//
//  讀：golden ReadEditTextFromFile ＋ InitialDataToEdit 之後，EditListToJson() 依 ControlName 匯出。
//  寫：EditListApply() 把頁面 JSON 依 ControlName 填回替身，再由各結構的 cpp 呼叫 golden 的存檔入口。
// ===========================================================================
#pragma once

#include <string>
#include <vector>

#include "Public/HTEditList.h"
#include "vclcompat/StringGrid.h"
#include "vclcompat/TDateTime.h"

#include <cmath>

namespace filerw {

// ---- vclcompat 沒有、golden 存檔流程會讀的元件型別：只帶 golden 用到的屬性 ----
// TTrackBar／TUpDown：VCL 語意（Steven 20260924，TfSpeed 需要）——
//   * Position 夾在 Min..Max；值有變時更新 Associate（TUpDown 綁的 TEdit）的 Text，並觸發 OnChange
//     （golden cSpeed.cpp ReadFile 設 tbAllSpeed->Position 就靠 OnChange=tbAllSpeedChange 把值帶到其他欄位）。
//   * Min／Max 變了會把 Position 重新夾一次（VCL TTrackBar.SetParams／TUpDown 同）。
//   * 設計期的 Min／Max／Position／Associate／OnChange 由產生器從 golden DFM 帶入（Dfm 狀態那段，不觸發事件）。
//   * 頁面送回的值用 SetPosition(v, false) 直接套（頁面送的是畫面最後狀態，不能再觸發事件改別的欄位）。
class ELTrackBar : public TControl {
public:
    struct Pos {
        ELTrackBar* o;
        operator int() const { return o->pos_; }
        Pos& operator=(int v) { o->SetPosition(v, true); return *this; }
        Pos& operator=(const Pos& p) { return *this = (int)p; }
    };
    struct Lim {
        ELTrackBar* o;
        int v;
        operator int() const { return v; }
        Lim& operator=(int x) { v = x; o->SetPosition(o->pos_, true); return *this; }
        Lim& operator=(const Lim& p) { return *this = (int)p; }
    };
    Pos Position{this};
    Lim Min{this, 0};
    Lim Max{this, 10};
    int SelStart = 0, SelEnd = 0, Increment = 1, Frequency = 1;
    TCustomEdit* Associate = nullptr;
    void (*OnChange)() = nullptr;
    void SetPosition(int v, bool fire) {
        if (v < Min.v) v = Min.v;
        if (v > Max.v) v = Max.v;
        if (v == pos_) return;
        pos_ = v;
        if (Associate) Associate->Text = pos_;
        if (fire && OnChange) OnChange();
    }
    // 設計期（DFM）初值：不夾、不觸發事件
    void DfmInit(int mn, int mx, int pos) { Min.v = mn; Max.v = mx; pos_ = pos; }
    ELTrackBar() = default;
    ELTrackBar(const ELTrackBar&) = delete;
    ELTrackBar& operator=(const ELTrackBar&) = delete;
private:
    int pos_ = 0;
};
// TDateTimePicker：golden 讀 ->Date（日期部分）、->Time（時間部分），兩者都是 DateTime 的一半。
class ELDateTimePicker : public TDateTimePicker {
public:
    struct DatePart {
        TDateTimePicker* o;
        operator TDateTime() const { return TDateTime(std::floor(o->DateTime)); }
        DatePart& operator=(const TDateTime& v) {
            o->DateTime = std::floor(v.Val()) + (o->DateTime - std::floor(o->DateTime));
            return *this;
        }
    };
    struct TimePart {
        TDateTimePicker* o;
        operator TDateTime() const { return TDateTime(o->DateTime - std::floor(o->DateTime)); }
        TimePart& operator=(const TDateTime& v) {
            o->DateTime = std::floor(o->DateTime) + (v.Val() - std::floor(v.Val()));
            return *this;
        }
    };
    DatePart Date{this};
    TimePart Time{this};
    // golden：dtO06_LastDate->Date + dtpO06NextTime->Time
    friend TDateTime operator+(const DatePart& d, const TimePart& t) {
        return TDateTime(TDateTime(d).Val() + TDateTime(t).Val());
    }
};
// TStringGrid：golden 讀 ->Cells[c][r]。vclcompat::TStringGrid 也是 TObject 衍生，不能與 TControl 多重繼承
// （兩份 TObject），所以包一個成員、把 Cells 轉出來。
class ELStringGrid : public TControl {
public:
    vclcompat::TStringGrid grid{7, 2};
    decltype(grid.Cells)& Cells = grid.Cells;
};

TControl* ELFind(const char* form, const char* name);
void      ELKeep(const char* form, const char* name, TControl* c);
const char* ELFormOf(TControl* c);     // 替身屬於哪個 golden 表單類別；不是替身回 ""

// 具名替身。第一次呼叫建立並登記；之後回傳同一個物件。
// 鍵是 (golden 表單類別, 元件名稱)：golden 不同表單會有同名元件（TfConfiguration::cbP13 與
// TfLd_ULd::cbP13 分別寫 config.ini 與 UdUld.Data），只用名稱會把兩個併成一個（審查 20260924 M3）。
// THTEdit::ControlName 仍是不帶表單的名稱 —— 就是那一頁 HTML 的元件 id。
template <class T>
T* EL(const char* form, const char* name) {
    if (TControl* c = ELFind(form, name)) return static_cast<T*>(c);
    T* t = new T();
    ELKeep(form, name, t);
    return t;
}

// 某個 golden 表單的具名替身的 Visible／Enabled（golden 註冊碼裡會設面板／分頁的顯示），給頁面套用。
std::string ProxyStateJson(const char* form);

// 一個 HTEditList 的全部筆：{"entries":[{id, group, key, content, readFromFile, min, max,
//   text|checked|itemIndex}]}。id＝THTEdit::ControlName（空的代表不是由具名替身註冊的，例如移植樹
//   自己的表單以真元件註冊的那幾筆，照實標出來）。
std::string EditListToJson(HTEditList* el, const char* onlyForm = nullptr);
// onlyForm：只列這個 golden 表單的替身登記的筆（同一份清單會被多個表單登記：golden TfConfiguration 也把自己的
// edP16_1／edP16_2… 加進 elUdUld —— 那幾筆屬於 Configuration 頁，不是 Ld_ULd 頁；Steven 20260924）


// ---- 存檔流程的「對話」：golden 的訊息框在伺服器端不彈，改記進這次請求的 JSON ----
// SessionBegin(answers)：answers 是頁面送來的 {"<golden 英文題目>":1|2}（1＝YES，2＝NO）。
// ELAsk 查得到就回那個答案；查不到回 2（NO，保守）並記進 asked，頁面可帶答案重送。
// ELMessage 對應 golden ShowMyMessage(S1,S2)；ELTodo 記下尚未接上的 golden 段落。
void        SessionBegin(const std::string& answersJson);
std::string SessionJson();        // {"messages":[{en,zh}],"asked":[{en,zh,answer}],"todo":[…]}
void ELMessage(AnsiString S1, AnsiString S2 = "", AnsiString S3 = "", bool Ok = false, bool bServoOff = false);
int  ELAsk(AnsiString S1, AnsiString S2, AnsiString S3 = "");
void ELTodo(const char* what);
bool ELPasswordRefused(const char* what);
void ELMark(const char* what);                // 存檔流程 trace
// golden cAuthority.cpp:444 ChangeCompomentEnabled 的「容器本身」那一半（子元件那一半由頁面依 DOM 做）
void ELChangeCompomentEnabled(TControl* c, bool bEnable, bool bMustEnable = false);
// 權限（審查 H-A）：父子關係照 golden DFM。可改＝自己 Enabled 且每一層祖先都 Enabled（不看 Visible ——
// golden 會存看不見的元件的值）。
void ELSetParents(const char* form, const char* const (*pairs)[2], int n);
// 可改＝自己與每一層祖先都 Enabled 且 Visible（TTabSheet 還要 TabVisible），且自己不是 ReadOnly。
// 看不見的元件 golden 使用者碰不到 —— 值照樣存（伺服器端的值），頁面送來的改動不收（審查 20260924 #1）。
bool ELEditable(const char* form, const char* name);
void ELSetReadOnly(const char* form, const char* name);   // golden DFM ReadOnly=True
// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true)：只打開名單上的（容器＋五種元件）
void ELEnableNames(const char* form, const char* const* names, int n);
bool ELMarked(const char* what);

// 頁面送來的 {名稱:{text?|checked?|itemIndex?|position?|dateTime?|cells?}} 依名稱套到 form 的替身。
// 先全部檢查（型別對得上才算），再一次套用；回 false 時一個都沒動，*err 說明。
// *applied 收到套用了哪些名稱，*unknown 收到頁面送了、但這個表單沒有這個替身的名稱（不算錯，照實回報）。
bool ELApplyProxies(const char* form, const std::string& widgetsJson, std::vector<std::string>* applied,
                    std::vector<std::string>* unknown, std::string* err);
// 替身是否被任何一個 HTEditList 登記（是 → 值來自檔案，頁面沒送就沿用；否 → 值只能來自頁面）
bool ELInAnyList(TControl* c, HTEditList* const* lists, int n);   // golden 密碼框：一律 false（不假裝驗過）

// Steven 20260925：TComboBox 的 VCL 連動（vclcompat TComboBox 只是欄位，Text 與 ItemIndex 各自獨立）。
// gen_editlist.py 把 golden 方法本體裡的 `cb->Text = x;`／`cb->ItemIndex = n;` 改寫成這兩支（DFM 設計期狀態不改寫）。
//   ELComboText：清單有同名項 → 選到那一項（VCL 設 Text 的效果）；沒有 → ItemIndex=-1、保留 Text（值原樣來回，
//                不會被頁面當成第 0 項存回去）。例：HSys cbKASUGA_Fan->Text="COM18" 以前 ItemIndex 停在 0 → 存成 COM1。
//   ELComboIndex：n 在範圍內 → ItemIndex=n、Text=Items[n]；n<0 或超出清單 → ItemIndex=-1、Text=""
//                （VCL SetItemIndex → Win32 CB_SETCURSEL：超出範圍就是沒有選取、清掉文字。SetUp 工程師 20260925 指出：
//                golden 看不見的 site 格子 Items 是空的，設值後讀回 -1、SaveSetupFile 寫 0；保留原值會把舊模式的值寫回檔案）。
inline void ELComboText(TComboBox* c, const AnsiString& t)
{
    c->Text = t;
    const int i = c->Items ? c->Items->IndexOf(t) : -1;
    c->ItemIndex = i;
}
inline void ELComboIndex(TComboBox* c, int n)
{
    if (c->Items && n >= 0 && n < c->Items->Count) { c->ItemIndex = n; c->Text = c->Items->Strings[n]; }
    else { c->ItemIndex = -1; c->Text = ""; }
}

}  // namespace filerw
