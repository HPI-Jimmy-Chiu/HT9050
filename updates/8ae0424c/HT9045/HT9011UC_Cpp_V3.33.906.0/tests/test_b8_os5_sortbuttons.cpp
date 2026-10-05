// =============================================================================
//  test_b8_os5_sortbuttons.cpp -- B8 OS-5：Offset 頁 A30 Setup Teach 的 12 顆排序鈕（golden TfOffSet::btnSortAuto1Click）
//
//  //AI(W906-B8-OS5) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-5」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3211-3221（12 顆共用）：
//    IniConfig.bA30SetupTeachFunction && LastSet.iTester==OFF_LINE && LastSet.bNeedSetupTeach ⇒ iSortUnloadT6=Sender->Tag；
//    Tag＝cOffSet.dfm grpSetupTeach 的設計期值（Auto1～6＝0～5、Fix1～6＝6～11）＋建構子 :425-430（Auto1～3／Fix1～3＝eAuto1…eFix3）。
//  受測的是 wb_serve 編進去的同一份 FileRW/Offset_File.cpp（檔尾 OS_EvSort／OS_EvB8BootProxies、事件表註冊）＋產生檔 FileRW/Offset_File.gen.inc
//    的 OS_btnSortAuto1Click，經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp：運轉中守衛、解析、分派）與 RunPageEvent
//    （FileRW/_EditPage.cpp：開過頁、ELOperable 沿 DFM 父層）；god-stack 用 RESCAN 連（同 ctest HSys_HeaterMix）。
//    [1] 事件表：12 顆都在別名頁 "Setup.OffSet" 的 events、golden 出處 cOffSet.cpp:3211
//    [2] Tag：12 顆的替身 Tag＝golden（DFM＋建構子）＝MachineType.h eAuto1…eFix6
//    [3] 看得見才點得到：grpSetupTeach 看不見（DFM Visible=False／golden 非 A30 或連線）⇒ operable=false、送了回 bad-payload、iSortUnloadT6 不動
//    [4] 看得見（golden FormShow :712-721 A30＋離線的狀態）⇒ 12 顆 operable；grpOutArm456 看不見（AUTO_EMPTY_COLOR<3）⇒ 只剩 Auto1～3／Fix1～3
//        （Auto4／5、Fix4／5／6 的父層是本列補登記的，btnSortAuto6 是產生器的）；btnSortAuto6 看不見（AUTO_EMPTY_COLOR<4）⇒ 只有它點不到
//    [5] 三條件 8 種組合：只有三個都成立才設 iSortUnloadT6，其他不動（golden 不跳訊息）
//    [6] 12 顆逐顆：iSortUnloadT6＝那一顆的 Tag；ack 走 C 路、golden 出處、changed／todo 都空、不用先點部位鈕
//    [7] 運轉中（//AI(W906-FE-RUNEXC) 20260930：FileRW/_FormEvent.cpp 檔尾的運轉中例外表；golden 運轉中按得到，Offset 非模態）：
//        表上 TfOffSet 的列＝這 12 顆（click、fOffSet、SystemStart／SoftStart 都放行、golden cOffSet.cpp:3211-3221）；
//        頁面表沒說 fOffSet 開著（沒有頁面表／開著的是別的表單）⇒ 照舊 running、iSortUnloadT6 不動；
//        開著 ⇒ SystemStart 12 顆逐顆跑（iSortUnloadT6＝Tag）、golden 三條件 8 種組合照套；SoftStart、兩個都立也跑；
//        grpSetupTeach 看不見 ⇒ 運轉中一樣 bad-payload（例外不跳過 RunPageEvent）；
//        不在表上的（微調鈕、部位鈕、同一顆的 "change"、不存在的鈕、解析不了的 value）運轉中照舊 running、訊息同以前
//    [8] 微調鈕照舊走 OS_EvAuto（本列改了註冊迴圈，確認沒有把微調鈕改道）：它自己的開頁檢查擋下（本測試沒跑 FileRW_Offset_Page）
//    [9] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）：產生檔處理器照 golden、建構子設 Tag、FormShow 的顯示；
//        頁面 ht9045_offset_ev.js 有 12 顆的送出點與 grpSetupTeach 的 display 打開
//    [10] 等級（//AI(W906-Q61-LEVEL) 20260930，Steven Q61「golden應該是有卡權限吧? 依照golden」）：golden 這 12 顆沒有等級閘——
//        開窗 sbOffsetClick（V912 main.cpp:28738-28752）不查等級；FormShow 的等級只停用 pnlPicker／btnOffsetList／pnlIndexOfs（cOffSet.cpp:584-590）
//        與 tsScale（:575-582），排序鈕不在裡面；處理器不查。⇒ Operator 在 [02]／[00]／[03] 都要 HonPrec 時：Speed 頁被擋（對照組）、Offset 頁
//        停機／SystemStart／SoftStart 都開得了，12 顆停機／運轉中都照 golden 跑；開頁後換等級 ⇒ reload page，重開（不查等級）就又能按
//  NOT COVERED：golden FormShow／ReadFile（讀配方的 offset 檔）不在這裡跑 —— [4] 的「看得見」直接設替身，FormShow 那幾行由 [9] 的棘輪看；
//    Out Arm 放料那一端（aoutarm.cpp:4172）不在這一列。
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
#include "LastSet.h"
#include "MachineType.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_Offset_Boot();
void EnsureArmOffsetObjects();   // forms/fOffSet.h:450（本體 cOffSet.cpp:166）
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421（本體 csystem.cpp）；W906_FormShowing(obj,false)＝hook(obj)。AI(W906-FE-RUNEXC) 20260930：[7] 用它當頁面表
extern int iMaxLevelItem;   // cSecurity.cpp:47（開機 W906_SecurityBoot 設 180；golden V912 cSecurity.cpp:210）。//AI(W906-Q61-LEVEL) 20260930：[10] 用

// 連結用（不是受測碼）：
//  * FileRW_IniConfig_ChangeCBListProperty：同 tests/test_hsys_heater_mix.cpp（不給的話連結器抽 FileRW/_fallback.cpp，跟 _EditList.cpp 撞名）。
//  * JsonBridge（JsonBridge/FormBridge.cpp、FormJson.cpp 只編進 wb_serve）：FileRW/_FormEvent.cpp 先找 A 形狀頁 —— 這裡一頁都沒有（回 nullptr），
//    所以一定走 C 路 RunPageEvent；鎖是空的（單執行緒）。同 tests/test_formevent_position.cpp 的做法。
void FileRW_IniConfig_ChangeCBListProperty() {}
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

const char* const kButtons[12] = {"btnSortAuto1", "btnSortAuto2", "btnSortAuto3", "btnSortAuto4", "btnSortAuto5", "btnSortAuto6",
                                  "btnSortFix1", "btnSortFix2", "btnSortFix3", "btnSortFix4", "btnSortFix5", "btnSortFix6"};
const int kGoldenTag[12] = {eAuto1, eAuto2, eAuto3, eAuto4, eAuto5, eAuto6, eFix1, eFix2, eFix3, eFix4, eFix5, eFix6};

// WS form.event 的 value（頁面 ht9045_offset_ev.js 送的形狀）
bool Click(const char* control, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    const std::string v = std::string("{\"form\":\"TfOffSet\",\"control\":\"") + control + "\",\"event\":\"click\"}";
    return W906_FormEvent("Setup.OffSet", v, ack, err);
}
//AI(W906-FE-RUNEXC) 20260930：[7] 用 —— 任意事件／任意 value，與頁面表的替身（g_open＝哪一個 golden 表單開在 HMI 上，nullptr＝都沒開）
bool Raw(const std::string& value, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    return W906_FormEvent("Setup.OffSet", value, ack, err);
}
bool ClickEv(const char* control, const char* event, std::string* ack, std::string* err)
{
    return Raw(std::string("{\"form\":\"TfOffSet\",\"control\":\"") + control + "\",\"event\":\"" + event + "\"}", ack, err);
}
const char* g_open = nullptr;
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }

// editlist.get 別名頁（formShow 是空的）→ "events":{控制項:{event, golden, operable}}
cJSON* EventsNow()
{
    const filerw::PageDesc* d = filerw::FindPageForEvent("Setup.OffSet");
    if (!d) return nullptr;
    std::string json;
    if (filerw::PageJson(*d, &json) != 200) return nullptr;
    cJSON* root = cJSON_Parse(json.c_str());
    if (!root) return nullptr;
    cJSON* ev = cJSON_DetachItemFromObjectCaseSensitive(root, "events");
    cJSON_Delete(root);
    return ev;
}
bool Operable(const cJSON* ev, const char* id)
{
    const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, id) : nullptr;
    const cJSON* o = e ? cJSON_GetObjectItemCaseSensitive(e, "operable") : nullptr;
    return o && cJSON_IsTrue(o);
}
std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}
// 12 顆裡 operable 的個數，另回哪幾顆
int CountOperable(std::string* which)
{
    cJSON* ev = EventsNow();
    int n = 0;
    which->clear();
    for (const char* b : kButtons)
        if (Operable(ev, b)) { ++n; *which += std::string(*which->c_str() ? "," : "") + b; }
    if (ev) cJSON_Delete(ev);
    return n;
}

void SetConds(bool a30, bool offline, bool need)
{
    IniConfig.bA30SetupTeachFunction = a30;
    LastSet.iTester = offline ? OFF_LINE : 1;
    LastSet.bNeedSetupTeach = need;
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

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_os5_sortbuttons -- B8 OS-5 golden TfOffSet::btnSortAuto1Click (V912 cOffSet.cpp:3211-3221)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);

    SystemStart = false; SoftStart = false;
    EnsureArmOffsetObjects();                    // wb_serve.cpp:3502（FileRW_Offset_Boot 的前提）
    FileRW_Offset_Boot();
    Check(filerw::FindPageForEvent("Setup.OffSet") != nullptr, "[0] boot: form.event alias page Setup.OffSet registered");

    std::printf("[1] event table\n");
    {
        cJSON* ev = EventsNow();
        int n = 0;
        for (const char* b : kButtons) {
            const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, b) : nullptr;
            if (e && Str(e, "event") == "click" && Str(e, "golden") == "cOffSet.cpp:3211 TfOffSet::btnSortAuto1Click") ++n;
            else std::printf("    missing / wrong row: %s\n", b);
        }
        Check(n == 12, "[1] all 12 sort buttons: click -> cOffSet.cpp:3211 TfOffSet::btnSortAuto1Click");
        if (ev) cJSON_Delete(ev);
    }

    std::printf("[2] Tags\n");
    {
        int n = 0;
        for (int i = 0; i < 12; ++i) {
            TControl* c = filerw::ELFind("TfOffSet", kButtons[i]);
            if (c && c->Tag == kGoldenTag[i] && kGoldenTag[i] == i) ++n;
            else std::printf("    %s: Tag %d, golden %d\n", kButtons[i], c ? c->Tag : -99, kGoldenTag[i]);
        }
        Check(n == 12, "[2] proxy Tag == golden DFM/ctor Tag == eAuto1..eFix6 (0..11)");
    }

    std::string ack, err, which;
    std::printf("[3] group hidden (DFM Visible=False / golden not A30 or on line)\n");
    SetConds(true, true, true);
    iSortUnloadT6 = -1;
    Check(!EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible, "[3] grpSetupTeach starts hidden (cOffSet.dfm:12782)");
    Check(CountOperable(&which) == 0, "[3] events.operable=false for all 12");
    Check(!Click("btnSortAuto3", &ack, &err) && err.find("bad-payload") == 0 && iSortUnloadT6 == -1,
          "[3] click refused (bad-payload, cannot be operated), iSortUnloadT6 untouched");

    std::printf("[4] group visible (golden FormShow :712-721, A30 + off line)\n");
    EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible = true;
    EL<TTabSheet>("TfOffSet", "tsSetupTeach")->TabVisible = true;
    Check(CountOperable(&which) == 12, "[4] all 12 operable");
    EL<TPanel>("TfOffSet", "grpOutArm456")->Visible = false;            // golden FormShow :592 AUTO_EMPTY_COLOR<3
    {
        const int n6 = CountOperable(&which);    // 先算再組訊息（引數求值順序沒有規定）
        Check(n6 == 6 && which == "btnSortAuto1,btnSortAuto2,btnSortAuto3,btnSortFix1,btnSortFix2,btnSortFix3",
              "[4] grpOutArm456 hidden -> only Auto1-3 / Fix1-3 (Auto4/5, Fix4/5/6 parents registered by this row): " + which);
    }
    EL<TPanel>("TfOffSet", "grpOutArm456")->Visible = true;
    EL<TButton>("TfOffSet", "btnSortAuto6")->Visible = false;            // golden FormShow :594 AUTO_EMPTY_COLOR<4
    Check(CountOperable(&which) == 11 && which.find("btnSortAuto6") == std::string::npos, "[4] btnSortAuto6 hidden -> 11 operable");
    EL<TButton>("TfOffSet", "btnSortAuto6")->Visible = true;
    CountOperable(&which);                        // 最後一次 editlist.get（開過頁、這個等級）

    std::printf("[5] three conditions, 8 combinations\n");
    {
        int ok = 0;
        for (int m = 0; m < 8; ++m) {
            const bool a = (m & 1) != 0, o = (m & 2) != 0, nd = (m & 4) != 0;
            SetConds(a, o, nd);
            iSortUnloadT6 = -1;
            const bool sent = Click("btnSortFix2", &ack, &err);
            const int want = (a && o && nd) ? eFix2 : -1;
            if (sent && iSortUnloadT6 == want) ++ok;
            else std::printf("    A30=%d offline=%d need=%d: sent=%d err=%s iSortUnloadT6=%d want %d\n", a, o, nd, sent, err.c_str(), iSortUnloadT6, want);
        }
        Check(ok == 8, "[5] iSortUnloadT6 = Tag only when A30 && OFF_LINE && bNeedSetupTeach; otherwise untouched");
    }

    std::printf("[6] each button\n");
    SetConds(true, true, true);
    {
        int ok = 0;
        for (int i = 0; i < 12; ++i) {
            iSortUnloadT6 = -1;
            const bool sent = Click(kButtons[i], &ack, &err);
            cJSON* a = sent ? cJSON_Parse(ack.c_str()) : nullptr;
            const cJSON* ch = a ? cJSON_GetObjectItemCaseSensitive(a, "changed") : nullptr;
            const cJSON* td = a ? cJSON_GetObjectItemCaseSensitive(a, "todo") : nullptr;
            const bool shape = a && Str(a, "route") == "C" && Str(a, "golden") == "cOffSet.cpp:3211 TfOffSet::btnSortAuto1Click" &&
                               ch && cJSON_IsObject(ch) && !ch->child && (!td || (cJSON_IsArray(td) && !td->child));
            if (sent && iSortUnloadT6 == i && shape) ++ok;
            else std::printf("    %s: sent=%d err=%s iSortUnloadT6=%d ack=%s\n", kButtons[i], sent, err.c_str(), iSortUnloadT6, ack.c_str());
            if (a) cJSON_Delete(a);
        }
        Check(ok == 12, "[6] every button sets iSortUnloadT6 to its own Tag; ack route C, golden cOffSet.cpp:3211, no changed, no todo, no part clicked first");
    }

    std::printf("[7] running: the running-exception table (FileRW/_FormEvent.cpp tail, AI(W906-FE-RUNEXC) 20260930; golden allows these buttons during a run)\n");
    {
        int rows = 0, match = 0;
        for (int i = 0; i < formevent::runexc::RowCount(); ++i) {
            const formevent::runexc::RowInfo* r = formevent::runexc::RowAt(i);
            if (!r || std::strcmp(r->form, "TfOffSet") != 0) continue;
            ++rows;
            for (const char* b : kButtons)
                if (std::strcmp(r->control, b) == 0 && std::strcmp(r->event, "click") == 0 && std::strcmp(r->shownObj, "fOffSet") == 0 &&
                    r->systemStart && r->softStart && r->golden && std::strstr(r->golden, "cOffSet.cpp:3211-3221")) ++match;
        }
        Check(rows == 12 && match == 12,
              "[7] table: the TfOffSet rows are exactly the 12 sort buttons (click, fOffSet, SystemStart + SoftStart allowed, golden cOffSet.cpp:3211-3221)");
    }
    SetConds(true, true, true);
    iSortUnloadT6 = -1;
    SystemStart = true;
    W906_FormFShowHook = nullptr;                // 沒有頁面表（ctest 的預設）＝ Offset 沒開
    {
        const bool sent = Click("btnSortAuto1", &ack, &err);
        Check(!sent && Starts(err, "running: ") && err.find("fOffSet") != std::string::npos && iSortUnloadT6 == -1,
              "[7] SystemStart, no page table -> running (the row's reason names fOffSet), iSortUnloadT6 untouched");
    }
    W906_FormFShowHook = &FakePageTable;
    g_open = "fAGV";                             // 開著的是別的表單
    Check(!Click("btnSortAuto1", &ack, &err) && Starts(err, "running: ") && iSortUnloadT6 == -1,
          "[7] SystemStart, the page table says only fAGV is open -> running, untouched");
    g_open = "fOffSet";
    {
        int ok = 0;
        for (int i = 0; i < 12; ++i) {
            iSortUnloadT6 = -1;
            const bool sent = Click(kButtons[i], &ack, &err);
            cJSON* a = sent ? cJSON_Parse(ack.c_str()) : nullptr;
            const bool shape = a && Str(a, "route") == "C" && Str(a, "golden") == "cOffSet.cpp:3211 TfOffSet::btnSortAuto1Click";
            if (sent && shape && iSortUnloadT6 == i) ++ok;
            else std::printf("    %s: sent=%d err=%s iSortUnloadT6=%d\n", kButtons[i], sent, err.c_str(), iSortUnloadT6);
            if (a) cJSON_Delete(a);
        }
        Check(ok == 12, "[7] SystemStart + Offset open: every sort button runs golden btnSortAuto1Click (iSortUnloadT6 = its Tag, ack route C)");
    }
    {
        int ok = 0;
        for (int m = 0; m < 8; ++m) {
            const bool a = (m & 1) != 0, o = (m & 2) != 0, nd = (m & 4) != 0;
            SetConds(a, o, nd);
            iSortUnloadT6 = -1;
            const bool sent = Click("btnSortFix5", &ack, &err);
            const int want = (a && o && nd) ? eFix5 : -1;
            if (sent && iSortUnloadT6 == want) ++ok;
            else std::printf("    running A30=%d offline=%d need=%d: sent=%d err=%s iSortUnloadT6=%d want %d\n", a, o, nd, sent, err.c_str(), iSortUnloadT6, want);
        }
        Check(ok == 8, "[7] SystemStart + Offset open: golden's own A30 / OFF_LINE / bNeedSetupTeach check still decides (8 combinations)");
    }
    SetConds(true, true, true);
    SystemStart = false; SoftStart = true;
    iSortUnloadT6 = -1;
    {
        const bool sent = Click("btnSortFix4", &ack, &err);
        Check(sent && iSortUnloadT6 == eFix4, "[7] SoftStart + Offset open -> runs (iSortUnloadT6 = eFix4) " + err);
    }
    SystemStart = true;
    iSortUnloadT6 = -1;
    {
        const bool sent = Click("btnSortAuto2", &ack, &err);
        Check(sent && iSortUnloadT6 == eAuto2, "[7] SystemStart + SoftStart + Offset open -> runs (iSortUnloadT6 = eAuto2) " + err);
    }
    SoftStart = false;
    EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible = false;
    iSortUnloadT6 = -1;
    Check(!Click("btnSortAuto3", &ack, &err) && Starts(err, "bad-payload") && iSortUnloadT6 == -1,
          "[7] SystemStart + Offset open, grpSetupTeach hidden -> still bad-payload (the exception does not skip RunPageEvent's ELOperable)");
    EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible = true;
    {
        const std::string gen = "running: 機台運轉中（SystemStart）不能從網頁操作設定畫面";   // 原本的訊息（FileRW/_FormEvent.cpp:79）
        const bool nudge = Click("sb_AutoOffsetUp", &ack, &err) || !Starts(err, gen);
        const bool part = Click("sbLoader", &ack, &err) || !Starts(err, gen);
        const bool chg = ClickEv("btnSortAuto1", "change", &ack, &err) || !Starts(err, gen);
        const bool nosuch = Click("btnSortAuto7", &ack, &err) || !Starts(err, gen);
        const bool junk = Raw("not json", &ack, &err) || !Starts(err, gen);
        Check(!nudge, "[7] not listed, SystemStart + Offset open: nudge button sb_AutoOffsetUp -> running (message as before)");
        Check(!part, "[7] not listed: part button sbLoader (golden SpBotSelClick) -> running");
        Check(!chg && iSortUnloadT6 == -1, "[7] not listed: btnSortAuto1 \"change\" (the table is per event) -> running, untouched");
        Check(!nosuch, "[7] not listed: btnSortAuto7 (no such button) -> running, not unknown-control (checked before the handler lookup, as before)");
        Check(!junk, "[7] unparsable value while running -> running (checked before the value, as before)");
    }
    SystemStart = false; SoftStart = true;
    {
        const std::string gen = "running: 機台正要啟動或回原點（SoftStart）";                // 原本的訊息（FileRW/_FormEvent.cpp:80）
        const bool sent = Click("sb_AutoOffsetDown", &ack, &err);
        Check(!sent && Starts(err, gen), "[7] not listed, SoftStart + Offset open: sb_AutoOffsetDown -> running (message as before)");
    }
    SoftStart = false;
    W906_FormFShowHook = nullptr;
    g_open = nullptr;

    std::printf("[8] nudge buttons still go through OS_EvAuto\n");
    EL<TPanel>("TfOffSet", "pan_AutoOffsetMove")->Visible = true;        // golden FormShow :572（bUseAutoOffsetFunction）
    CountOperable(&which);
    //   OS_EvAuto 第一道是本檔自己的開頁紀錄（OS_EvRequireShown，要 FileRW_Offset_Page 跑過；本測試沒跑）⇒ 它的訊息「tag=Offset_File」
    //   證明微調鈕還是走 OS_EvAuto（排序鈕的 OS_EvSort 不查這一道，上面 [5]／[6] 照樣送得進去）
    {
        const bool sent = Click("sb_AutoOffsetUp", &ack, &err);   // 先送再組訊息（引數求值順序沒有規定）
        Check(!sent && err.find("tag=Offset_File") != std::string::npos,
              "[8] sb_AutoOffsetUp -> OS_EvAuto's own page-open check (not rerouted to the sort dispatcher): " + err.substr(0, 110));
    }

    std::printf("[9] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[9] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        std::string gen, cpp, js;
        const bool rd = ReadAll(std::string(argv[1]) + "/FileRW/Offset_File.gen.inc", &gen) &&
                        ReadAll(std::string(argv[1]) + "/FileRW/Offset_File.cpp", &cpp) &&
                        ReadAll(std::string(argv[2]) + "/ht9045_offset_ev.js", &js);
        Check(rd, "[9] read FileRW/Offset_File.gen.inc, FileRW/Offset_File.cpp, ht9045_offset_ev.js");
        Check(gen.find("// golden cOffSet.cpp:3211  TfOffSet::btnSortAuto1Click(TObject *Sender)") != std::string::npos &&
              CodeHas(gen, "Ptr=(TButton *)Sender;") && CodeHas(gen, "if(IniConfig.bA30SetupTeachFunction &&") &&
              CodeHas(gen, "LastSet.iTester==OFF_LINE &&") && CodeHas(gen, "LastSet.bNeedSetupTeach)") && CodeHas(gen, "iSortUnloadT6=Ptr->Tag;"),
              "[9] generated OS_btnSortAuto1Click is the golden body (live code)");
        Check(CodeHas(gen, "EL<TButton>(\"TfOffSet\", \"btnSortAuto1\")->Tag=eAuto1;") && CodeHas(gen, "EL<TButton>(\"TfOffSet\", \"btnSortFix3\")->Tag =eFix3;"),
              "[9] golden ctor :425-430 Tags");
        Check(CodeHas(gen, "EL<TGroupBox>(\"TfOffSet\", \"grpSetupTeach\")->Visible  =true;") &&
              CodeHas(gen, "EL<TButton>(\"TfOffSet\", \"btnSortAuto6\")->Visible=(AUTO_EMPTY_COLOR>=4);"),
              "[9] golden FormShow display lines (:717, :594)");
        Check(CodeHas(cpp, "g_osEvents[n].handler = OS_EvIsSort(kOS_Events[i].control) ? &OS_EvSort : &OS_EvAuto;") &&
              CodeHas(cpp, "OS_EvB8BootProxies();"), "[9] Offset_File.cpp: sort rows dispatched by OS_EvSort, parents registered at boot (code, not after //)");
        int nb = 0;
        for (const char* b : kButtons) if (js.find(std::string("'") + b + "'") != std::string::npos) ++nb;
        Check(nb == 12 && js.find("send({ form: FORM, control: id, event: 'click' }, 0)") != std::string::npos &&
              js.find("b.addEventListener('click', onSort(id));") != std::string::npos &&
              js.find("pg.visible === true && grp.style.display === 'none'") != std::string::npos,
              "[9] page: 12 sort buttons send form.event click; grpSetupTeach display opened when the server says visible");
    }

    //AI(W906-Q61-LEVEL) 20260930 [W906]：Steven 20260930 Q61「golden應該是有卡權限吧? 依照golden」——golden 這 12 顆沒有等級閘（V912，cp950）：
    //   開窗 sbOffsetClick（main.cpp:28708-28722）不查等級也不查 SystemStart（同一排的 sbSpeedClick :28690 有 Insufficient(3)）；sbOffset 在主畫面
    //   tsMain（main.dfm:601），ChangeLevelAttr（:12926-13191）不碰它。TfOffSet::FormShow 的等級只停用 pnlPicker／btnOffsetList／pnlIndexOfs
    //   （cOffSet.cpp:584-590，[02] Main - Offset＋authMainForm[2]）與 tsScale（:575-582，HonPrec）；排序鈕在 tsSetupTeach > grpSetupTeach > grpOutArm
    //   （cOffSet.dfm:12765-13212），不在那幾個容器裡；處理器 :3211-3221 不查等級。golden 唯一的等級效果是關窗：A01 自動登出（Timer3Timer
    //   main.cpp:26036-26046）關 fOffSet，重開不查等級。移植樹：GOffset 恆放行（FileRW/_EditPage.cpp）；事件要「在同一個等級開過頁」（RunPageEvent 第 1 步）。
    std::printf("[10] access level (Q61): golden has no level gate for the sort buttons (sbOffsetClick main.cpp:28708-28722; cOffSet.cpp:584-590 gate other panels)\n");
    {
        const int lv0 = AccessLevel, n0 = LevelSet.AccessLevel[0], n2 = LevelSet.AccessLevel[2], n3 = LevelSet.AccessLevel[3], mx0 = iMaxLevelItem;
        iMaxLevelItem = 180;                            // 開機後的值（W906_SecurityBoot；golden cSecurity.cpp:210）
        LevelSet.AccessLevel[0] = 3; LevelSet.AccessLevel[2] = 3; LevelSet.AccessLevel[3] = 3;   // [00] Tools／[02] Offset／[03] Speed 都要 HonPrec(3)
        AccessLevel = 0;                                // Operator(0)
        std::string why;
        Check(filerw::OpenGateRefused("ArmSpeed_File", false, &why) && Starts(why, "not-authorized:") && why.find("main.cpp:28690") != std::string::npos,
              "[10] contrast: Operator(0) < [03] Main - Speed HonPrec(3) -> Speed page refused (golden sbSpeedClick main.cpp:28690) -- the level table is live here");
        bool opens[3];
        why.clear();
        opens[0] = !filerw::OpenGateRefused("Offset_File", false, &why);
        SystemStart = true;
        opens[1] = !filerw::OpenGateRefused("Offset_File", false, &why);
        SystemStart = false; SoftStart = true;
        opens[2] = !filerw::OpenGateRefused("Offset_File", false, &why);
        SoftStart = false;
        Check(opens[0] && opens[1] && opens[2],
              "[10] Operator(0), [02] Main - Offset = HonPrec(3): Offset page opens stopped / SystemStart / SoftStart "
              "(golden sbOffsetClick main.cpp:28708-28722 checks neither the level nor SystemStart) " + why);
        // golden FormShow 在 Operator、[02]=HonPrec 時會停用的容器（cOffSet.cpp:575-590）——照 golden 停用，排序鈕不在裡面
        EL<TPanel>("TfOffSet", "pnlPicker")->Enabled = false;
        EL<TControl>("TfOffSet", "btnOffsetList")->Enabled = false;
        EL<TPanel>("TfOffSet", "pnlIndexOfs")->Enabled = false;
        EL<TTabSheet>("TfOffSet", "tsScale")->Enabled = false;
        Check(!filerw::ELOperable("TfOffSet", "EditPickA"),
              "[10] contrast: EditPickA (inside pnlPicker) cannot be operated once golden FormShow's [02] gate disables pnlPicker (cOffSet.cpp:584)");
        SetConds(true, true, true);
        const int nOp = CountOperable(&which);          // editlist.get 別名頁：在 Operator(0) 開過頁
        Check(nOp == 12, "[10] Operator(0), pnlPicker / btnOffsetList / pnlIndexOfs / tsScale disabled: all 12 sort buttons still operable "
                         "(tsSetupTeach > grpSetupTeach > grpOutArm, cOffSet.dfm:12765-13212)");
        iSortUnloadT6 = -1;
        bool sent = Click("btnSortAuto4", &ack, &err);
        Check(sent && iSortUnloadT6 == eAuto4, "[10] stopped, Operator(0): btnSortAuto4 runs golden :3211 (iSortUnloadT6 = eAuto4) " + err);
        W906_FormFShowHook = &FakePageTable;
        g_open = "fOffSet";
        SystemStart = true;
        iSortUnloadT6 = -1;
        sent = Click("btnSortFix6", &ack, &err);
        Check(sent && iSortUnloadT6 == eFix6, "[10] SystemStart + Offset open, Operator(0): btnSortFix6 runs (iSortUnloadT6 = eFix6) -- no level gate, as golden " + err);
        SystemStart = false; SoftStart = true;
        iSortUnloadT6 = -1;
        sent = Click("btnSortAuto1", &ack, &err);
        Check(sent && iSortUnloadT6 == eAuto1, "[10] SoftStart + Offset open, Operator(0): btnSortAuto1 runs (iSortUnloadT6 = eAuto1) " + err);
        SoftStart = false; SystemStart = true;
        AccessLevel = 1;                                // 等級換了（golden A01 自動登出 Timer3Timer main.cpp:26036-26046 直接關 fOffSet）
        iSortUnloadT6 = -1;
        Check(!Click("btnSortFix1", &ack, &err) && err.find("reload page") != std::string::npos && iSortUnloadT6 == -1,
              "[10] SystemStart, level changed after the page was opened -> reload page, iSortUnloadT6 untouched");
        why.clear();
        const bool reopen = !filerw::OpenGateRefused("Offset_File", false, &why);
        CountOperable(&which);                          // 在新的等級重開（editlist.get 別名頁）
        iSortUnloadT6 = -1;
        sent = Click("btnSortFix1", &ack, &err);
        Check(reopen && sent && iSortUnloadT6 == eFix1,
              "[10] SystemStart: reopened at the new level (golden: reopening checks no level) -> btnSortFix1 runs (iSortUnloadT6 = eFix1) " + err);

        SystemStart = false; SoftStart = false;
        EL<TPanel>("TfOffSet", "pnlPicker")->Enabled = true;
        EL<TControl>("TfOffSet", "btnOffsetList")->Enabled = true;
        EL<TPanel>("TfOffSet", "pnlIndexOfs")->Enabled = true;
        EL<TTabSheet>("TfOffSet", "tsScale")->Enabled = true;
        AccessLevel = lv0; LevelSet.AccessLevel[0] = n0; LevelSet.AccessLevel[2] = n2; LevelSet.AccessLevel[3] = n3; iMaxLevelItem = mx0;
        W906_FormFShowHook = nullptr;
        g_open = nullptr;
    }

    for (int i = 0; i < 2; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    std::printf("test_b8_os5_sortbuttons: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
