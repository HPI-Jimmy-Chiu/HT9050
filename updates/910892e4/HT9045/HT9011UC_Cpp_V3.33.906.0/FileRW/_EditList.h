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
//AI(W906-FRW-S158) 20260927 [W906]：TPageControl 替身多帶 "activePageIndex":<int>＝替身目前的 ActivePageIndex
//   （golden DFM 頁序、0 起算、TabVisible=false 的頁也算一格＝VCL PageIndex；頁面 .tabs .tab 的 data-t 同序）。頁面送回用同一個鍵
//   （ELApplyProxies）。form.event 的 changed 也會帶（golden 處理器改了分頁時，同 JsonBridge/FormBridge.cpp:188 A 形狀的鍵名）。

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
//AI(W906-FRW-S158) 20260927 [W906]：TPageControl 的分頁名單（golden DFM 頁序：DFM 裡 TTabSheet 子物件的先後
//   就是 VCL TPageControl.GetChildren 寫出的頁序）。可選：登記了，activePageIndex 的範圍＝0..n-1；沒登記＝ELSetParents 父子表裡
//   以它為父的 TTabSheet 替身個數 —— 產生器（tools/gen_editlist.py:410-416）只收「用到的替身＋它們的祖先」，沒用到的分頁不在表裡，
//   所以那是下限，超過下限的值不收（記 todo）。目前沒有呼叫端（產生器的 dfm_parents 已經照 DFM 順序走，補一行就有精確範圍）。
void ELSetPageOrder(const char* form, const char* pageControl, const char* const* tabs, int n);
// 可改＝自己與每一層祖先都 Enabled 且 Visible（TTabSheet 還要 TabVisible），且自己不是 ReadOnly。
// 看不見的元件 golden 使用者碰不到 —— 值照樣存（伺服器端的值），頁面送來的改動不收（審查 20260924 #1）。
bool ELEditable(const char* form, const char* name);
// AI(W906-FRW-S157) 20260927 [W906]：WS form.event —— golden 使用者「點得到」這個元件嗎：同 ELEditable，但不看 ReadOnly
// （VCL TEdit ReadOnly 只擋打字，OnClick／下拉 OnChange 照樣發生；同 FileRW/TestIF_File_Cleaning.cpp Clickable 的規則）。
bool ELOperable(const char* form, const char* name);
void ELSetReadOnly(const char* form, const char* name);   // golden DFM ReadOnly=True
// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true)：只打開名單上的（容器＋五種元件）
void ELEnableNames(const char* form, const char* const* names, int n);
bool ELMarked(const char* what);

// 頁面送來的 {名稱:{text?|checked?|itemIndex?|position?|dateTime?|cells?}} 依名稱套到 form 的替身。
// 先全部檢查（型別對得上才算），再一次套用；回 false 時一個都沒動，*err 說明。
// *applied 收到套用了哪些名稱，*unknown 收到頁面送了、但這個表單沒有這個替身的名稱（不算錯，照實回報）。
//AI(W906-FRW-S158) 20260927 [W906]：TPageControl 收 {"activePageIndex":<整數>}。沒帶這個鍵＝照舊（有 "tag" 當 tag，
//   否則 unknown）——舊頁面行為不變。帶了但不合法（不是數字、不是整數、超出 0..分頁數-1（見 ELSetPageOrder）、或 golden 使用者
//   點不到這個分頁控制（ELOperable：自己或容器停用／看不見））⇒ 只有這一筆不套、其餘照套、不整批拒（和其他種類「型別不對整批拒」
//   不同：分頁是輔助值，不該擋住使用者的其他改動），理由收進 *notes；notes 為 nullptr 時直接 ELTodo（呼叫端若在這之後才
//   SessionBegin，理由會被清掉 —— 所以 PageSave／RunPageEvent 都傳 notes、在 SessionBegin 之後補記）。
//   不看目標分頁的 TabVisible：golden 會用程式切到藏起來的分頁（golden V912 cOffSet.cpp:3383 btnToIndexOffsetClick
//   ActivePage=tsIndexOffset，建構子 :192 把它 TabVisible=false），產生器把 `->ActivePage=` 當 UI-only 交給頁面做
//   ⇒ 頁面停在藏起來的分頁是合法狀態。套值不觸發 OnChange（同其他種類：頁面最後狀態）。
bool ELApplyProxies(const char* form, const std::string& widgetsJson, std::vector<std::string>* applied,
                    std::vector<std::string>* unknown, std::string* err,
                    std::vector<std::string>* notes = nullptr);   //AI(W906-FRW-S158) 20260927 [W906]：notes（見上）
//AI(W906-EVB1) 20260928 [W906] X-2：同一種值（activePageIndex）的判斷給 WS form.event 用（FileRW/_EditPage.cpp RunPageEvent 第 3 步：
//   TPageControl OnChange 帶的新分頁）。回 "" ＝收得下；否則回理由（同 ELApplyProxies 記進 notes 的「ignored: 」後面那一段：
//   超出 0..分頁數-1（ELSetPageOrder 精確／父子表下限）、伺服器不知道任何分頁、ELOperable 點不到）。不是 TPageControl 替身也回理由。不改替身。
std::string ELPageIndexRefused(const char* form, const char* name, int index);
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

//AI(W906-EVB10B) 20260929 [W906] X-3（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；R100、R118 照 BCB；
//   Steven 20260929「照 BCB 的邏輯」）：VCL「程式設值也會觸發 OnClick」。golden 程式（DoIniDataToForm、FormShow、HTEditList 的
//   Add／ReadEditTextFromFile／InitialDataToEdit …）設 TRadioGroup::ItemIndex、TCheckBox::Checked、TRadioButton::Checked 時，VCL 當場
//   呼叫那個元件 DFM 的 OnClick；vclcompat 替身只是欄位、不會觸發。
//   ELSetOnClick：登記替身的 OnClick（產生器的 'vcl_clicks' 在 <P>_DfmState 裡照 golden DFM 登記；只登記有轉的處理器）。
//   ELClickIndex／ELClickChecked：golden 程式設值的等價寫法（產生器 'vcl_clicks' 改寫；手寫碼也可用）：
//     TRadioGroup（VCL TCustomRadioGroup.SetItemIndex）：夾在 -1..Items.Count-1；值有變、而且新值 >=0 ⇒ OnClick
//                  （設成 -1 只把舊的那顆取消，VCL 不 Click）。
//     TCheckBox（VCL TCustomCheckBox.SetState）：值有變 ⇒ OnClick。
//     TRadioButton（VCL TRadioButton.SetChecked）：false→true ⇒ OnClick（true→false 不 Click；TurnSiblingsOff 不在這裡）。
//   ELClickIndex 一律照 VCL 夾值；沒登記 OnClick 的替身只設值、不觸發。HTEditList 的兩支 hook（ELHTEditSet*，下面）對沒登記的替身
//   連夾值都不做（純設值，跟以前一樣）。頁面送的值（ELApplyProxies）與 form.event 控制項自己的值不經過這裡
//   （使用者點的那一下由事件表上的處理器跑一次）。處理器裡再設別的元件 ⇒ 巢狀觸發（VCL 同）。
using ELClickFn = void (*)(TControl* sender);
void      ELSetOnClick(TControl* c, ELClickFn fn);
ELClickFn ELOnClickOf(TControl* c);          // 沒登記回 nullptr
void ELClickIndex(TRadioGroup* g, int v);
void ELClickChecked(TCheckBox* c, bool v);
void ELClickChecked(TRadioButton* c, bool v);
long ELClickCount();                         // 測試／診斷用：到目前為止 ELClick* 觸發了幾次 OnClick
// Public/HTEditList.cpp 的 HTEditList_SetClickHooks 用的兩支（只替有登記 OnClick 的替身觸發；其餘照舊純設值、不夾）
void ELHTEditSetChecked(TCheckBox* c, bool v);
void ELHTEditSetItemIndex(TRadioGroup* g, int v);

//AI(W906-EVB10B) 20260929 [W906] X-5／CC-L1：切分頁事件（TPageControl OnChange）轉出來的 golden 程式讀分頁用的三支（要先 ELSetPageOrder）。
//   ELActivePage：golden `pc->ActivePage`（回目前那一頁的替身；沒登記頁序或超出範圍回 nullptr）。
//   ELPageIndexOf：golden `ts->PageIndex`（頁序位置，藏起來的頁也算；不在這一排回 -1）。
//   ELTabIndexOf：golden `ts->TabIndex`（VCL TTabSheet.GetTabIndex：自己 TabVisible=false 回 -1，否則＝前面 TabVisible 的頁數）。
TControl* ELActivePage(const char* form, const char* pageControl);
int       ELPageIndexOf(const char* form, const char* pageControl, const char* tab);
int       ELTabIndexOf(const char* form, const char* pageControl, const char* tab);

}  // namespace filerw
