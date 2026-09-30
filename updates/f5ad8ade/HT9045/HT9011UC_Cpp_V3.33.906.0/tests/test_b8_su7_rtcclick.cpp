// =============================================================================
//  test_b8_su7_rtcclick.cpp -- B8 SU-7：Setup 頁 RTC 等 6 個勾選框（golden TfSetup::cbEnableRealTimeCCDClick）的點擊檢查、改回、存檔重查
//
//  //AI(W906-B8-SU7) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「SU-7」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4364-4399（cSetUp.dfm 6 個元件的 OnClick 都綁它）：
//    (a) :4366-4375 取消 RTC、[D55] 開且 [M01-09] 沒開 ⇒ 勾回去、ShowMyMessage、return
//    (b) :4377-4388 RTC 正在跑（!COM2->bCCDDummyRum）：IndexStatus!=Z1_Z2_Normal ⇒「* Turn off need Z1 Z2 up!」＋勾回去；否則送 rtInspEnd（沒移植＝todo）
//    (c) :4390-4397 RTC 關著：fMain->CheckCanChangeRealDummy()==false ⇒ 取消勾選＋「* Turn On need Clean-Out !」
//    VCL 程式設 Checked 值有變 ⇒ 再進一次處理器（B8 P-8）。
//  受測的是 wb_serve 編進去的同一份 FileRW/TestIF_File_SetUp.cpp（檔尾 FileRW_Setup_B8Su7Boot／Su7VclOnClick／FileRW_Setup_B8Su7BeforeApply、
//    BeforeApply (5) 的呼叫點、事件表註冊）＋產生檔 FileRW/TestIF_File_SetUp.gen.inc 的 SU_cbEnableRealTimeCCDClick，經真的 WS 入口
//    W906_FormEvent（FileRW/_FormEvent.cpp）與 RunPageEvent（FileRW/_EditPage.cpp）；CheckCanChangeRealDummy 是 cMainStatus.cpp:323 的活本體
//    （god-stack 用 RESCAN 連，同 ctest B8_Os5_SortButtons）；有沒有料由 tests/test_b8_su7_ic.cpp 改活物件的格子。
//    [1] 事件表：6 個勾選框 click → cSetUp.cpp:4364；sbtExit 那一列還在；開頁後 6 個都 operable
//    [2] (c) 機台空的：打開 RTC 照收、沒有提示字、沒有再進一次
//    [3] (c) 六種有料（Plate1／Plate2／Shuttle／Index／In Arm／Out Arm）逐一：打開 RTC 被取消、「* Turn On need Clean-Out !」、ack.changed 帶回、再進一次（值相等就停）
//    [4] (b) RTC 正在跑、Index 不在安全位：關 RTC 被勾回去、「* Turn off need Z1 Z2 up!」
//    [5] (b) Index 在安全位：關 RTC 照收、ack.todo 說 rtInspEnd 沒送
//    [6] (a) [D55] 開、[M01-09] 沒開：關 RTC 被勾回去＋訊息（再進一次那一層照 golden 走 (b)）；[M01-09] 開 ⇒ 不擋；RTC 關著、機台空 ⇒ 勾回去後 (c) 放行
//    [7] golden 無限遞迴的組合（[D55] 開、[M01-09] 沒開、RTC 關、有料）：停在上限、勾選＝機台目前 RTC 狀態、訊息＋todo、不當掉
//    [8] 共用處理器：點另外 5 格也跑同一支（有料時提示字出現、RTC 不動、點的那一格留新值）；[D55] 時點 cbSocketSensor 會把 RTC 勾起來（golden (a)）
//    [9] 存檔重查（PageDesc::beforeApply，跟 editlist.save 同一支）：頁面值不同才重播；被改回記訊息；點不到的不重播（交給 PageSave 丟掉）；
//        點別格時 RTC 最後的值由處理器決定（列入 handled）
//    [10] 運轉中：SystemStart／SoftStart ⇒ running，什麼都沒動
//    [11] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）：產生檔處理器照 golden、呼叫的是 fMain->CheckCanChangeRealDummy；
//         BeforeApply (5) 與開機登記在程式碼裡（不在 // 後面）；頁面 ht9045_setup_c_wire.js 有 6 格的送出點
//  NOT COVERED：golden FormShow／ReadFile（開頁讀配方，會寫 HandlerCondition.Data／config.ini）不在這裡跑 —— 開頁用「formShow 換成空函式」的
//    PageDesc 複本記「開過頁」；golden 存檔 sbUpdateClick 不跑（會寫檔），[9] 只驗存檔前重查那一段。rtInspEnd 的序列埠送出（沒移植）。
//  開跑前後比對 D:\HT9045\system\Gerneral.ini 與 D:\HT9045\config\config.ini 的內容，有變就失敗（本測試不寫任何檔）。
// =============================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "atester_shims.h"   // COM2（TCOM2Shim）
#include "forms/fSetup.h"    // fSetup->Init()（FileRW_Setup_Boot 的前提，同 wb_serve.cpp:3198）
#include "forms/fMain.h"     // fMain->CheckCanChangeRealDummy()（cMainStatus.cpp:323）

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_Setup_Boot();
void Su7SetIC(int which, bool on);   // tests/test_b8_su7_ic.cpp
bool Su7Pred(int which);

// 連結用（不是受測碼）：
//  * FileRW_IniConfig_ChangeCBListProperty：同 tests/test_b8_os5_sortbuttons.cpp（不給的話連結器抽 FileRW/_fallback.cpp，跟 _EditList.cpp 撞名）。
//  * JsonBridge（只編進 wb_serve）：FileRW/_FormEvent.cpp 先找 A 形狀頁 —— 這裡一頁都沒有，一定走 C 路 RunPageEvent；鎖是空的（單執行緒）。
//  * FileRW/TestIF_File_SetUp.cpp 引用、但只在 wb_serve 目標裡的四支（FileRW/MainClick.cpp、FileRW/TestIF_File_Cleaning.cpp、WebLogin.cpp）：
//    本測試不跑存檔流程（SaveFlow）、不按存檔，所以只要連得起來。W906_ReauthOpenJson 在開頁（ExtraJson）會被叫到 —— 回 null。
void FileRW_IniConfig_ChangeCBListProperty() {}
void W906_Main_sbSetupClickTail() {}
void FileRW_Cleaning_LoadAutoCleanData() {}
bool W906_ReauthSetupDoPassword(bool, bool, bool, bool, bool* bHandled) { if (bHandled) *bHandled = false; return false; }
std::string W906_ReauthOpenJson(const char*, bool, bool, bool, bool) { return "null"; }
namespace ht9045 {
namespace formbridge {
const BridgeDesc* FindBridge(const std::string&) { return nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out)
{
    out->code = "handler-failed"; out->why = "test: no A-shape page";
    return false;
}
}  // namespace formbridge
namespace formjson {
void FormLock() {}
void FormUnlock() {}
}  // namespace formjson
}  // namespace ht9045

using filerw::EL;

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

const char* const kTag = "TestIF_File_SetUp";
const char* const kF = "TfSetup";
const char* const kBoxes[6] = {"cbOutUseBackRow", "cbInUseBackRow", "cbEnableRealTimeCCD", "cbOcrFunction", "cbdisibleinitialcheck", "cbSocketSensor"};
const char* const kGolden = "cSetUp.cpp:4364 TfSetup::cbEnableRealTimeCCDClick";
const char* const kIcName[6] = {"Plate1", "Plate2", "Shuttle", "Index", "InArm", "OutArm"};
const char* const kCleanOut = "* Turn On need Clean-Out !";
const char* const kZ1Z2 = "* Turn off need Z1 Z2 up!";

TCheckBox* Rtc() { return EL<TCheckBox>(kF, "cbEnableRealTimeCCD"); }
TLabel* Lb() { return EL<TLabel>(kF, "lbShowMessage"); }

void NoShow() {}

// editlist.get 的「開過頁」記號，但不跑 golden FormShow（它的 ReadFile 會寫 HandlerCondition.Data／config.ini）
cJSON* OpenPage()
{
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    if (!d) return nullptr;
    filerw::PageDesc c = *d;
    c.formShow = &NoShow;
    std::string json;
    if (filerw::PageJson(c, &json) != 200) return nullptr;
    return cJSON_Parse(json.c_str());
}
bool Operable(const cJSON* page, const char* id)
{
    const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
    const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, id) : nullptr;
    const cJSON* o = e ? cJSON_GetObjectItemCaseSensitive(e, "operable") : nullptr;
    return o && cJSON_IsTrue(o);
}
std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}

// WS form.event 的 value（頁面 ht9045_setup_c_wire.js (8) 送的形狀）
struct Ack {
    bool sent = false;
    std::string err, raw;
    cJSON* j = nullptr;
    ~Ack() { if (j) cJSON_Delete(j); }
    const cJSON* changed(const char* id) const
    {
        const cJSON* ch = j ? cJSON_GetObjectItemCaseSensitive(j, "changed") : nullptr;
        return ch ? cJSON_GetObjectItemCaseSensitive(ch, id) : nullptr;
    }
    bool listHas(const char* key, const char* needle) const
    {
        const cJSON* a = j ? cJSON_GetObjectItemCaseSensitive(j, key) : nullptr;
        char* p = a ? cJSON_PrintUnformatted(a) : nullptr;
        const bool ok = p && std::strstr(p, needle) != nullptr;
        if (p) cJSON_free(p);
        return ok;
    }
    bool listEmpty(const char* key) const
    {
        const cJSON* a = j ? cJSON_GetObjectItemCaseSensitive(j, key) : nullptr;
        return !a || (cJSON_IsArray(a) && !a->child);
    }
};
void Click(const char* id, bool checked, Ack* a)
{
    const std::string v = std::string("{\"form\":\"TfSetup\",\"control\":\"") + id + "\",\"event\":\"click\",\"checked\":" + (checked ? "true" : "false") + "}";
    a->sent = W906_FormEvent(kTag, v, &a->raw, &a->err);
    if (a->j) cJSON_Delete(a->j);
    a->j = a->sent ? cJSON_Parse(a->raw.c_str()) : nullptr;
}
bool ChangedChecked(const Ack& a, const char* id, bool v)
{
    const cJSON* c = a.changed(id);
    const cJSON* k = c ? cJSON_GetObjectItemCaseSensitive(c, "checked") : nullptr;
    return k && cJSON_IsBool(k) && (cJSON_IsTrue(k) != 0) == v;
}
bool ChangedCaption(const Ack& a, const char* text)
{
    const cJSON* c = a.changed("lbShowMessage");
    return Str(c, "caption") == text;
}

// 機台狀態：d55／m0109＝IniConfig，dummy＝COM2->bCCDDummyRum，index＝IndexStatus，ic＝-1 空機或 0..5 哪一樣有料
void Machine(bool d55, bool m0109, bool dummy, int index, int ic)
{
    IniConfig.bD55DisableIndexCheck = d55;
    IniConfig.bM0109RTCOffCheckYieldPiggyBack = m0109;
    COM2->bCCDDummyRum = dummy;
    IndexStatus = index;
    for (int i = 0; i < 6; ++i) Su7SetIC(i, i == ic);
}
// 畫面：RTC 勾選、提示字清掉（golden FormShow :1899 lbShowMessage->Visible=false）
void Screen(bool rtc)
{
    Rtc()->Checked = rtc;
    Lb()->Caption = "";
    Lb()->Visible = false;
}

bool CodeHas(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    bool gated = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}

// PageDesc::beforeApply（editlist.save 在必送檢查之後、丟值之前呼叫的同一支）＋這一次的 session 訊息
struct Replay {
    std::vector<std::string> handled;
    std::string session;
    bool has(const char* id) const { for (const std::string& s : handled) if (s == id) return true; return false; }
};
void BeforeApply(const std::string& widgets, Replay* r)
{
    r->handled.clear();
    filerw::SessionBegin("");
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    if (d && d->beforeApply) d->beforeApply(widgets, &r->handled);
    r->session = filerw::SessionJson();
}
std::string W(const char* id, bool v) { return std::string("\"") + id + "\":{\"checked\":" + (v ? "true" : "false") + "}"; }

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_su7_rtcclick -- B8 SU-7 golden TfSetup::cbEnableRealTimeCCDClick (V912 cSetUp.cpp:4364-4399)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);

    SystemStart = false; SoftStart = false;
    fSetup->Init();                              // wb_serve.cpp:3198（golden 建構子；tSiteMap 由它建）
    FileRW_Setup_Boot();
    Check(filerw::FindPage(kTag) != nullptr, "[0] boot: page TestIF_File_SetUp registered");

    std::printf("[1] event table\n");
    cJSON* page = OpenPage();
    Check(page != nullptr, "[1] editlist.get (golden FormShow replaced by a no-op) = 200");
    {
        const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
        int n = 0, op = 0;
        for (const char* b : kBoxes) {
            const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, b) : nullptr;
            if (e && Str(e, "event") == "click" && Str(e, "golden") == kGolden) ++n;
            else std::printf("    missing / wrong row: %s\n", b);
            if (Operable(page, b)) ++op;
            else std::printf("    not operable after open: %s\n", b);
        }
        Check(n == 6, "[1] all 6 check boxes: click -> cSetUp.cpp:4364 TfSetup::cbEnableRealTimeCCDClick (golden cSetUp.dfm OnClick)");
        Check(ev && Str(cJSON_GetObjectItemCaseSensitive(ev, "sbtExit"), "golden") == "cSetUp.cpp:3476 TfSetup::sbtExitClick",
              "[1] SU-9 sbtExit row still there");
        Check(op == 6, "[1] all 6 operable (golden DFM parents visible / enabled)");
    }
    if (page) cJSON_Delete(page);

    Ack a;
    std::printf("[2] (c) RTC off, machine empty\n");
    Machine(false, false, true, Z1_Z2_Normal, -1);
    Screen(false);
    {
        const long c0 = filerw::ELClickCount();
        Click("cbEnableRealTimeCCD", true, &a);
        Check(a.sent && Rtc()->Checked && !Lb()->Visible && !a.changed("cbEnableRealTimeCCD") && filerw::ELClickCount() == c0 &&
              Str(a.j, "golden") == kGolden && Str(a.j, "route") == "C",
              "[2] turning RTC on is kept, no hint, no re-entry, ack golden cSetUp.cpp:4364 route C (err=" + a.err + ")");
    }

    std::printf("[3] (c) RTC off, IC in the machine (each of golden main.cpp:12897-12899)\n");
    {
        int ok = 0;
        for (int w = 0; w < 6; ++w) {
            Machine(false, false, true, Z1_Z2_Normal, w);
            Screen(false);
            const bool pred = Su7Pred(w) && !fMain->CheckCanChangeRealDummy();
            const long c0 = filerw::ELClickCount();
            Click("cbEnableRealTimeCCD", true, &a);
            const bool good = pred && a.sent && !Rtc()->Checked && Lb()->Visible && Lb()->Caption == kCleanOut &&
                              ChangedCaption(a, kCleanOut) && ChangedChecked(a, "cbEnableRealTimeCCD", false) &&
                              filerw::ELClickCount() == c0 + 1;
            if (good) ++ok;
            else std::printf("    %s: pred=%d sent=%d err=%s rtc=%d lb=%d '%s' clicks+%ld ack=%s\n", kIcName[w], (int)Su7Pred(w), (int)a.sent, a.err.c_str(),
                             (int)Rtc()->Checked, (int)Lb()->Visible, Lb()->Caption.c_str(), filerw::ELClickCount() - c0, a.raw.c_str());
        }
        Check(ok == 6, "[3] Plate1 / Plate2 / Shuttle / Index / In Arm / Out Arm: box put back off, \"* Turn On need Clean-Out !\", ack.changed carries both, one VCL re-entry");
    }

    std::printf("[4] (b) RTC running, Index Z not safe\n");
    Machine(false, false, false, Z1_Z2_Normal + 1, -1);
    Screen(true);
    {
        const long c0 = filerw::ELClickCount();
        Click("cbEnableRealTimeCCD", false, &a);
        Check(a.sent && Rtc()->Checked && Lb()->Visible && Lb()->Caption == kZ1Z2 && ChangedCaption(a, kZ1Z2) &&
              ChangedChecked(a, "cbEnableRealTimeCCD", true) && filerw::ELClickCount() == c0 + 1 && !a.listHas("todo", "SendCommToVision"),
              "[4] turning RTC off is put back on, \"* Turn off need Z1 Z2 up!\", one re-entry (Z still not safe, value equal -> stops)");
    }

    std::printf("[5] (b) RTC running, Index Z safe\n");
    Machine(false, false, false, Z1_Z2_Normal, 3);   // 有料也不看（golden (b) 不查料）
    Screen(true);
    Click("cbEnableRealTimeCCD", false, &a);
    Check(a.sent && !Rtc()->Checked && !Lb()->Visible && !a.changed("cbEnableRealTimeCCD") && a.listHas("todo", "SendCommToVision") &&
          a.listHas("todo", "NOT sent"),
          "[5] turning RTC off is kept; ack.todo says the vision rtInspEnd was NOT sent (RTC vision link not ported)");

    std::printf("[6] (a) [D55] Disable index check\n");
    Machine(true, false, false, Z1_Z2_Normal + 1, -1);
    Screen(true);
    Click("cbEnableRealTimeCCD", false, &a);
    Check(a.sent && Rtc()->Checked && a.listHas("messages", "Must cancel") && a.listHas("messages", "[D55]Disable index check") &&
          ChangedChecked(a, "cbEnableRealTimeCCD", true) && Lb()->Caption == kZ1Z2,
          "[6] D55 on, M01-09 off: put back on + \"Must cancel [D55]...\"; the VCL re-entry runs (b) (golden: Z not safe -> hint)");
    Machine(true, false, false, Z1_Z2_Normal, -1);
    Screen(true);
    Click("cbEnableRealTimeCCD", false, &a);
    Check(a.sent && Rtc()->Checked && a.listHas("messages", "Must cancel") && a.listHas("todo", "SendCommToVision"),
          "[6] D55, Z safe: still put back on, and the re-entry reaches golden's rtInspEnd send (golden quirk, recorded as todo)");
    Machine(true, true, false, Z1_Z2_Normal, -1);
    Screen(true);
    Click("cbEnableRealTimeCCD", false, &a);
    Check(a.sent && !Rtc()->Checked && !a.listHas("messages", "Must cancel"), "[6] D55 on but [M01-09] on: not blocked by (a)");
    Machine(true, false, true, Z1_Z2_Normal, -1);
    Screen(false);
    Rtc()->Checked = true;                        // 勾選跟機台狀態不一致（RTC 關著但勾著）：使用者取消
    Click("cbEnableRealTimeCCD", false, &a);
    Check(a.sent && Rtc()->Checked && a.listHas("messages", "Must cancel") && !Lb()->Visible,
          "[6] D55, RTC off, machine empty: put back on, re-entry (c) lets it stay on");

    std::printf("[7] golden endless recursion ([D55] on, [M01-09] off, RTC off, IC in the machine)\n");
    Machine(true, false, true, Z1_Z2_Normal, 2);
    Screen(false);
    Rtc()->Checked = true;
    {
        const long c0 = filerw::ELClickCount();
        Click("cbEnableRealTimeCCD", false, &a);
        const long n = filerw::ELClickCount() - c0;
        Check(a.sent && !Rtc()->Checked && a.listHas("messages", "recurses without end") && a.listHas("todo", "without end") && n >= 4 && n <= 5,
              "[7] stopped after the nest limit (re-entries " + std::to_string(n) + "), box = machine's RTC state (off), message + todo, no crash");
    }

    std::printf("[8] the other 5 boxes run the same handler\n");
    {
        int ok = 0;
        for (const char* b : kBoxes) {
            if (std::strcmp(b, "cbEnableRealTimeCCD") == 0) continue;
            Machine(false, false, true, Z1_Z2_Normal, 4);
            Screen(false);
            TCheckBox* x = EL<TCheckBox>(kF, b);
            const bool want = !x->Checked;
            Click(b, want, &a);
            if (a.sent && x->Checked == want && !Rtc()->Checked && Lb()->Visible && Lb()->Caption == kCleanOut && ChangedCaption(a, kCleanOut) &&
                !a.changed("cbEnableRealTimeCCD"))
                ++ok;
            else std::printf("    %s: sent=%d err=%s box=%d rtc=%d lb='%s'\n", b, (int)a.sent, a.err.c_str(), (int)x->Checked, (int)Rtc()->Checked, Lb()->Caption.c_str());
        }
        Check(ok == 5, "[8] clicking any other box shows RTC's \"* Turn On need Clean-Out !\" (IC in the machine), RTC untouched, the clicked box keeps its value");
        Machine(true, false, false, Z1_Z2_Normal + 1, -1);
        Screen(false);
        TCheckBox* s = EL<TCheckBox>(kF, "cbSocketSensor");
        Click("cbSocketSensor", !s->Checked, &a);
        Check(a.sent && Rtc()->Checked && ChangedChecked(a, "cbEnableRealTimeCCD", true) && a.listHas("messages", "Must cancel"),
              "[8] D55 on and RTC box off: clicking cbSocketSensor ticks RTC (golden (a) only looks at cbEnableRealTimeCCD)");
    }

    std::printf("[9] save-time recheck (PageDesc::beforeApply, same hook editlist.save runs)\n");
    {
        Replay r;
        Machine(false, false, true, Z1_Z2_Normal, 5);
        Screen(false);
        BeforeApply("{" + W("cbEnableRealTimeCCD", true) + "}", &r);
        Check(r.has("cbEnableRealTimeCCD") && !Rtc()->Checked && r.session.find("put back to OFF") != std::string::npos &&
              r.session.find(kCleanOut) != std::string::npos,
              "[9] page turned RTC on without form.event, Out Arm has IC -> replayed, put back off, message in this save's session");
        Machine(false, false, true, Z1_Z2_Normal, -1);
        Screen(false);
        BeforeApply("{" + W("cbEnableRealTimeCCD", true) + "}", &r);
        Check(r.has("cbEnableRealTimeCCD") && Rtc()->Checked && r.session.find("put back") == std::string::npos,
              "[9] machine empty -> replayed, kept on, no message");
        const long c0 = filerw::ELClickCount();
        BeforeApply("{" + W("cbEnableRealTimeCCD", true) + "}", &r);
        Check(!r.has("cbEnableRealTimeCCD") && Rtc()->Checked && filerw::ELClickCount() == c0,
              "[9] page value == server value (the page already sent form.event) -> no replay");
        Machine(false, false, false, Z1_Z2_Normal + 1, -1);
        Screen(true);
        BeforeApply("{" + W("cbEnableRealTimeCCD", false) + "}", &r);
        Check(r.has("cbEnableRealTimeCCD") && Rtc()->Checked && r.session.find("put back to ON") != std::string::npos,
              "[9] RTC running, Z not safe: turning off at save is put back on");
        Machine(false, false, true, Z1_Z2_Normal, 0);
        Screen(false);
        EL<TGroupBox>(kF, "gbRTC")->Enabled = false;   // golden FormShow :2048 可停用 gbRTC 的元件（例 [RTC Lock by file]）
        BeforeApply("{" + W("cbEnableRealTimeCCD", true) + "}", &r);
        Check(!r.has("cbEnableRealTimeCCD") && !Rtc()->Checked && !filerw::ELEditable(kF, "cbEnableRealTimeCCD"),
              "[9] not operable -> not replayed; PageSave then drops the value (ELEditable false = ack.ignored)");
        EL<TGroupBox>(kF, "gbRTC")->Enabled = true;
        Machine(true, false, false, Z1_Z2_Normal + 1, -1);
        Screen(false);
        TCheckBox* o = EL<TCheckBox>(kF, "cbOcrFunction");
        const bool ocr = !o->Checked;
        BeforeApply("{" + W("cbOcrFunction", ocr) + "," + W("cbEnableRealTimeCCD", false) + "}", &r);
        Check(r.has("cbOcrFunction") && r.has("cbEnableRealTimeCCD") && o->Checked == ocr && Rtc()->Checked &&
              r.session.find("Must cancel") != std::string::npos,
              "[9] another box changed: replayed; the handler's RTC value (D55 ticks it) stands over the page's stale value");
    }

    std::printf("[10] running\n");
    Machine(false, false, true, Z1_Z2_Normal, 1);
    Screen(false);
    SystemStart = true;
    Click("cbEnableRealTimeCCD", true, &a);
    Check(!a.sent && a.err.find("running") == 0 && !Rtc()->Checked && !Lb()->Visible, "[10] SystemStart -> running, nothing changed");
    SystemStart = false; SoftStart = true;
    Click("cbEnableRealTimeCCD", true, &a);
    Check(!a.sent && a.err.find("running") == 0, "[10] SoftStart -> running");
    SoftStart = false;
    for (int i = 0; i < 6; ++i) Su7SetIC(i, false);

    std::printf("[11] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[11] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        std::string gen, cpp, js;
        const bool rd = ReadAll(std::string(argv[1]) + "/FileRW/TestIF_File_SetUp.gen.inc", &gen) &&
                        ReadAll(std::string(argv[1]) + "/FileRW/TestIF_File_SetUp.cpp", &cpp) &&
                        ReadAll(std::string(argv[2]) + "/ht9045_setup_c_wire.js", &js);
        Check(rd, "[11] read FileRW/TestIF_File_SetUp.gen.inc, FileRW/TestIF_File_SetUp.cpp, ht9045_setup_c_wire.js");
        Check(gen.find("// golden cSetUp.cpp:4364  TfSetup::cbEnableRealTimeCCDClick(TObject *Sender)") != std::string::npos &&
              CodeHas(gen, "if(IniConfig.bD55DisableIndexCheck==true &&") && CodeHas(gen, "if(COM2->bCCDDummyRum==false)") &&
              CodeHas(gen, "if(IndexStatus!=Z1_Z2_Normal)") && CodeHas(gen, "if(fMain->CheckCanChangeRealDummy()==false)") &&
              CodeHas(gen, "filerw::ELClickChecked(EL<TCheckBox>(\"TfSetup\", \"cbEnableRealTimeCCD\"), false);") &&
              CodeHas(gen, "EL<TLabel>(\"TfSetup\", \"lbShowMessage\")->Caption=\"* Turn On need Clean-Out !\";") &&
              !CodeHas(gen, "SendCommToVision(COM2->rtInspEnd, false);") && CodeHas(gen, "SendCommToVision(COM2->rtInspEnd, false) NOT sent"),
              "[11] generated SU_cbEnableRealTimeCCDClick is the golden body (live code), calls fMain->CheckCanChangeRealDummy, rtInspEnd gated");
        Check(CodeHas(cpp, "FileRW_Setup_B8Su7BeforeApply(root, handled);") && CodeHas(cpp, "FileRW_Setup_B8Su7Boot();") &&
              CodeHas(cpp, "filerw::ELSetOnClick(EL<TCheckBox>(\"TfSetup\", id), &Su7VclOnClick);"),
              "[11] TestIF_File_SetUp.cpp: BeforeApply (5) and the boot registration are code (not after //)");
        int nb = 0;
        for (const char* b : kBoxes) if (js.find(std::string("'") + b + "'") != std::string::npos) ++nb;
        Check(nb == 6 && js.find("var v = { form: 'TfSetup', control: item.control, event: 'click', checked: item.checked };") != std::string::npos &&
              js.find("x.addEventListener('change', su7OnUser(id));") != std::string::npos && js.find("if (su7Busy()) return Promise.reject(") != std::string::npos,
              "[11] page: the 6 boxes send form.event click with checked; save waits for them");
    }

    for (int i = 0; i < 2; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    std::printf("test_b8_su7_rtcclick: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
