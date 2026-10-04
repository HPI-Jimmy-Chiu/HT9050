// =============================================================================
//  test_ta5_scrollbars.cpp -- TA-5：Setup.TrayAssignment 圖像模式的兩條捲軸（golden TfTrayAssignment::sbNormalTestChange／
//  sbNormalTest_RTChange）經 WS form.event 帶 "position"（X-2）跑 golden 處理器
//
//  //AI(W906-TA5) 20261001 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md「TA-5」
//    （Steven 20260928「任何畫面的事件, 都是我們做」「沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）。
//  //AI(W906-E031) 20261003 [W906] (St01)：產生器對 TrayForm 改讀 golden 906 0618（E-031 全面切換第 1 批，tools/golden_root.py）⇒ [1]／[3] 釘的
//    kTA_Events golden 字串改成 0618 行號 :1550／:1585（V912 :1639／:1674；兩支方法兩棵逐行相同）。下面其餘行號照舊是 V912
//    （0618＝V912−89，:1403 以後；換算：python tools/golden_root.py map cTrayAssignment.cpp <V912 行>）。
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp（cp950）：
//    :1639 sbNormalTestChange → :1644 GraphicToRadio：RGAuto3->ItemIndex=Position&0x01、RGAuto2=&0x02、RGAuto1=&0x04、RGLoader=&0x08
//          （VCL TCustomRadioGroup.SetItemIndex 夾在 -1..Count-1；值有變 ⇒ 那個群組的 DFM OnClick）；
//    :1674 sbNormalTest_RTChange → :1679 GraphicToRadio_RT：:1685-1691 bFTTrayAss2RTTrayAss 而且 pgRunMode->ActivePage==tsNormalTestGraph
//          ⇒ 先把 sbNormalTest 的位置抄過來；:1693-1696 rgAuto3_RT／rgAuto2_RT／rgAuto1_RT／rgLoad_RT。
//    DFM cTrayAssignment.dfm:569／:592 TScrollBar Max=15、OnChange＝上面兩支；父層 tsNormalTestGraph（頁 2）／tsReTestGraph（頁 3）。
//  受測的是 wb_serve 編進去的同一份 FileRW/TrayForm.cpp（事件表註冊、kTA_OnTab 分頁跳板）＋產生檔 FileRW/TrayForm.gen.inc
//    （kTA_Events 的兩列、TA_sbNormalTestChange／TA_GraphicToRadio…），經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp）與
//    RunPageEvent（FileRW/_EditPage.cpp：開過頁、ELOperable 沿 DFM 父層、第 3／5 步的 position）；god-stack 用 RESCAN 連（同 B8_Os5_SortButtons）。
//    [1] 事件表：editlist.get 的 events 有 sbNormalTest／sbNormalTest_RT（change、golden 出處 0618 :1550／:1585＝V912 :1639／:1674）
//    [2] 看不見就點不到：非圖像模式（tsNormalTestGraph／tsReTestGraph TabVisible=false）⇒ operable=false、送了 bad-payload、什麼都不動
//    [3] sbNormalTest position 5：伺服器 Position=5、RGAuto3=1、RGAuto2=0、RGAuto1=1（4 夾成 1）、RGLoader=0；
//        分頁跳板：RGLoader 1→0 的 OnClick（RGLoaderClick :1235-1249 只在頁 0／2 做）真的在頁 2 跑了 ⇒ cbEmpty->Enabled=false；
//        跑完分頁還原；ack 走 C 路、golden 出處、changed 帶 RGAuto3／RGLoader／cbEmpty
//    [4] sbNormalTest_RT position 9：rgAuto3_RT=1、rgAuto2_RT=0、rgAuto1_RT=0、rgLoad_RT=1（8 夾成 1）；bFTTrayAss2RTTrayAss 開、
//        頁面 state 說分頁是 2 ⇒ 跳板照 golden 放在頁 3（使用者只能在頁 3 拖它）⇒ 不抄 sbNormalTest；state 裡的 sbNormalTest 被丟（有自己的事件列）
//    [5] 夾值：position 20 ⇒ 15（todo 一筆、changed 帶實際 15）；bit 全 1 ⇒ 四個群組都是 1
//    [6] 拒絕：event "click" ⇒ no-handler；運轉中（SystemStart）⇒ running、什麼都不動
//    [7] 原始碼棘輪（argv[1]＝移植樹根目錄、argv[2]＝web\page，唯讀）：TrayForm.py 兩列、TrayForm.cpp kTA_OnTab 頁 2／3、
//        頁面 ht9045_trayassign_ev.js 有兩個送出點與 position
//  每一個 case 開頭 Baseline() 把替身、旗標、計數放回同一個起點。
//  不寫任何真實檔：golden FormShow（editlist.get）讀 <DataPath><配方>\Tray.Data —— DataPath 由全域 ENVIRONMENT 的 W906_INIDATA_ROOT
//    轉到 machine_log_scratch；開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini、D:\HT9045\SetUp.inf，有變就失敗。
// =============================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern void FileRW_TrayAssignment_Boot();
extern int iMaxLevelItem;   // cSecurity.cpp:47（開機 W906_SecurityBoot 設 180；golden V912 cSecurity.cpp:210）

// 連結用（不是受測碼）：
//  * FileRW_IniConfig_ChangeCBListProperty：同 tests/test_b8_os5_sortbuttons.cpp（不給的話連結器抽 FileRW/_fallback.cpp，跟 _EditList.cpp 撞名）。
//  * JsonBridge（只編進 wb_serve）：_FormEvent.cpp 先找 A 形狀頁 —— 這裡一頁都沒有，一定走 C 路 RunPageEvent；鎖是空的（單執行緒）。
//  * W906_Main_sbTrayAssignClickTail（FileRW/MainClick.cpp，wb_serve 專屬）：只有存檔流程 SaveFlow 叫它，本測試不存檔 ⇒ 被叫到就算失敗。
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
static int g_tailCalls = 0;
const char* W906_Main_sbTrayAssignClickTail() { ++g_tailCalls; return "test stand-in: must not be called (no save here)"; }

using filerw::EL;

namespace {

const char* const kF = "TfTrayAssignment";
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
std::string Num(int v) { char b[24]; std::snprintf(b, sizeof(b), "%d", v); return b; }

TRadioGroup* RG(const char* n) { return EL<TRadioGroup>(kF, n); }
filerw::ELTrackBar* SBar(const char* n) { return EL<filerw::ELTrackBar>(kF, n); }
TTabSheet* Tab(const char* n) { return dynamic_cast<TTabSheet*>(filerw::ELFind(kF, n)); }
TPageControl* Pc() { return EL<TPageControl>(kF, "pgRunMode"); }
TComboBox* Cb(const char* n) { return EL<TComboBox>(kF, n); }

// editlist.get TrayForm（＝golden FormShow）→ "events"
cJSON* g_events = nullptr;
bool OpenPage()
{
    const filerw::PageDesc* d = filerw::FindPageForEvent("Setup.TrayAssignment");
    if (!d) return false;
    std::string json;
    if (filerw::PageJson(*d, &json) != 200) { std::printf("  PageJson: %s\n", json.substr(0, 300).c_str()); return false; }
    cJSON* root = cJSON_Parse(json.c_str());
    if (!root) return false;
    if (g_events) cJSON_Delete(g_events);
    g_events = cJSON_DetachItemFromObjectCaseSensitive(root, "events");
    cJSON_Delete(root);
    return g_events != nullptr;
}
const cJSON* Ev(const char* id) { return g_events ? cJSON_GetObjectItemCaseSensitive(g_events, id) : nullptr; }
std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}
bool IsTrue(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsTrue(v);
}

bool Send(const std::string& control, const std::string& event, const std::string& extra, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    const std::string v = "{\"form\":\"TfTrayAssignment\",\"control\":\"" + control + "\",\"event\":\"" + event + "\"" + extra + "}";
    return W906_FormEvent("Setup.TrayAssignment", v, ack, err);
}

// 每個 case 的起點：圖像模式（golden ShowCompnet :1053-1070 的條件：bTrayAssignUseGraphic、AUTO_EMPTY_COLOR<3、rgLoaderType!=0 ⇒
// 兩個圖像頁看得見）；群組與捲軸放回已知值（直接設、不跑 OnClick／OnChange）；分頁＝DFM 的 tsReTestGroup（1）；旗標、計數歸零。
void Baseline(bool graphic)
{
    SystemStart = false; SoftStart = false;
    IniConfig.bTrayAssignUseGraphic = graphic;
    IniConfig.bFTTrayAss2RTTrayAss = false;
    AUTO_EMPTY_COLOR = 0;
    RG("rgLoaderType")->ItemIndex = graphic ? 1 : 0;
    Tab("tsNormalTestGroup")->TabVisible = !graphic;
    Tab("tsReTestGroup")->TabVisible = !graphic;
    Tab("tsNormalTestGraph")->TabVisible = graphic;
    Tab("tsReTestGraph")->TabVisible = graphic;
    RG("RGAuto3")->ItemIndex = 0; RG("RGAuto2")->ItemIndex = 1; RG("RGAuto1")->ItemIndex = 0; RG("RGLoader")->ItemIndex = 1;
    RG("rgAuto3_RT")->ItemIndex = 0; RG("rgAuto2_RT")->ItemIndex = 1; RG("rgAuto1_RT")->ItemIndex = 1; RG("rgLoad_RT")->ItemIndex = 0;
    SBar("sbNormalTest")->SetPosition(0, false);
    SBar("sbNormalTest_RT")->SetPosition(0, false);
    Cb("cbEmpty")->Enabled = true; Cb("cbColor")->Enabled = true;
    Pc()->ActivePageIndex = 1;
    g_tailCalls = 0;
}

bool CodeHas(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) {
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_ta5_scrollbars -- TA-5 golden TfTrayAssignment::sbNormalTestChange / sbNormalTest_RTChange (906 cTrayAssignment.cpp:1550 / :1585, V912 :1639 / :1674)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\SetUp.inf"};
    std::string before[3];
    bool had[3];
    for (int i = 0; i < 3; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    const char* inidata = std::getenv("W906_INIDATA_ROOT");
    if (!inidata || !*inidata) {
        std::printf("  FAIL: W906_INIDATA_ROOT is not set -- golden FormShow would read / write the real recipe folder; run through ctest\n");
        return 1;
    }

    SystemStart = false; SoftStart = false;
    // golden FormShow :935 palTrayAssign->Enabled=fSecurity->Insufficient(16,false)（一般客戶）⇒ 權限表要像開機後一樣載入、[16] 夠
    iMaxLevelItem = 180; AccessLevel = 0; LevelSet.AccessLevel[16] = 0;
    FileRW_TrayAssignment_Boot();
    const filerw::PageDesc* d = filerw::FindPageForEvent("Setup.TrayAssignment");
    Check(d != nullptr && std::strcmp(d->tag, "TrayForm") == 0, "[0] boot: form.event page Setup.TrayAssignment -> struct TrayForm");
    Check(OpenPage(), "[0] editlist.get TrayForm (golden FormShow) -> 200 with \"events\"");

    std::printf("[1] event table (FileRW/TrayForm.gen.inc kTA_Events, tools/editlist/TrayForm.py events)\n");
    Check(Str(Ev("sbNormalTest"), "event") == "change" &&
          Str(Ev("sbNormalTest"), "golden") == "cTrayAssignment.cpp:1550 TfTrayAssignment::sbNormalTestChange",   // AI(W906-E031)：0618（V912 :1639）
          "[1] sbNormalTest: change -> cTrayAssignment.cpp:1550 sbNormalTestChange (906 0618, V912 :1639) (got " + Str(Ev("sbNormalTest"), "golden") + ")");
    Check(Str(Ev("sbNormalTest_RT"), "event") == "change" &&
          Str(Ev("sbNormalTest_RT"), "golden") == "cTrayAssignment.cpp:1585 TfTrayAssignment::sbNormalTest_RTChange",   // AI(W906-E031)：0618（V912 :1674）
          "[1] sbNormalTest_RT: change -> cTrayAssignment.cpp:1585 sbNormalTest_RTChange (906 0618, V912 :1674)");
    Check(SBar("sbNormalTest")->Max == 15 && SBar("sbNormalTest")->Min == 0 && SBar("sbNormalTest_RT")->Max == 15,
          "[1] proxies are ELTrackBar, DFM Min 0 / Max 15 (cTrayAssignment.dfm:576 / :599)");

    std::string ack, err;
    std::printf("[2] not graphic mode: the two graph tabs are hidden (golden ShowCompnet :1071-1077) -> not operable\n");
    Baseline(false);
    Check(OpenPage() && !IsTrue(Ev("sbNormalTest"), "operable") && !IsTrue(Ev("sbNormalTest_RT"), "operable"),
          "[2] editlist.get events: operable=false for both scroll bars");
    Check(filerw::ELOperable(kF, "pgRunMode") && !Tab("tsNormalTestGraph")->TabVisible && !Tab("tsReTestGraph")->TabVisible,
          "[2] ... because of the two hidden graph tabs only (pgRunMode / palTrayAssign operable, golden FormShow :935 Insufficient(16) passes)");
    Baseline(false);
    bool ok = Send("sbNormalTest", "change", ",\"position\":5", &ack, &err);
    Check(!ok && err.find("bad-payload") == 0 && err.find("cannot be operated now") != std::string::npos,
          "[2] form.event sbNormalTest -> bad-payload cannot be operated now (" + err.substr(0, 90) + ")");
    Check((int)SBar("sbNormalTest")->Position == 0 && RG("RGAuto3")->ItemIndex == 0 && RG("RGLoader")->ItemIndex == 1,
          "[2] nothing moved (Position 0, RGAuto3 0, RGLoader 1)");

    std::printf("[3] sbNormalTest position 5 (0101b) -> golden GraphicToRadio :1650-1653\n");
    Baseline(true);
    Check(OpenPage(), "[3] reopen (golden FormShow)");
    Baseline(true);                                                             // FormShow 重讀了 Tray.Data：再放回起點
    Check(filerw::ELOperable(kF, "sbNormalTest") && filerw::ELOperable(kF, "sbNormalTest_RT"), "[3] graphic mode: both scroll bars operable");
    if (!filerw::ELOperable(kF, "sbNormalTest"))
        for (const char* n : {"sbNormalTest", "tsNormalTestGraph", "pgRunMode", "palTrayAssign"}) {
            TControl* c = filerw::ELFind(kF, n);
            TTabSheet* t = dynamic_cast<TTabSheet*>(c);
            std::printf("    chain %s: %s Enabled=%d Visible=%d TabVisible=%d\n", n, c ? "found" : "MISSING",
                        c ? (int)c->Enabled : -1, c ? (int)c->Visible : -1, t ? (int)t->TabVisible : -1);
        }
    ok = Send("sbNormalTest", "change", ",\"position\":5", &ack, &err);
    Check(ok, "[3] form.event ok (" + (ok ? ack.substr(0, 120) : err) + ")");
    Check((int)SBar("sbNormalTest")->Position == 5, "[3] server Position = 5 (RunPageEvent step 5, before the handler)");
    Check(RG("RGAuto3")->ItemIndex == 1 && RG("RGAuto2")->ItemIndex == 0 && RG("RGAuto1")->ItemIndex == 1 && RG("RGLoader")->ItemIndex == 0,
          "[3] RGAuto3=5&1=1, RGAuto2=5&2=0, RGAuto1=5&4=4 clamped to 1, RGLoader=5&8=0 (got " + Num(RG("RGAuto3")->ItemIndex) + "," +
          Num(RG("RGAuto2")->ItemIndex) + "," + Num(RG("RGAuto1")->ItemIndex) + "," + Num(RG("RGLoader")->ItemIndex) + ")");
    Check(!Cb("cbEmpty")->Enabled && Cb("cbColor")->Enabled,
          "[3] page trampoline: RGLoader 1->0 ran RGLoaderClick on page 2 (golden :1235-1242 cbEmpty->Enabled=false) -- DFM page 1 would skip it");
    Check(Pc()->ActivePageIndex == 1, "[3] pgRunMode page restored after the handler (1)");
    {
        cJSON* a = cJSON_Parse(ack.c_str());
        const cJSON* ch = a ? cJSON_GetObjectItemCaseSensitive(a, "changed") : nullptr;
        Check(Str(a, "route") == "C" && Str(a, "golden") == "cTrayAssignment.cpp:1550 TfTrayAssignment::sbNormalTestChange",   // AI(W906-E031)：0618（V912 :1639）
              "[3] ack: route C, golden cTrayAssignment.cpp:1550 (906 0618, V912 :1639)");
        Check(ch && cJSON_GetObjectItemCaseSensitive(ch, "RGAuto3") && cJSON_GetObjectItemCaseSensitive(ch, "RGLoader") &&
              cJSON_GetObjectItemCaseSensitive(ch, "cbEmpty"),
              "[3] ack.changed carries RGAuto3, RGLoader, cbEmpty (the page applies them)");
        if (a) cJSON_Delete(a);
    }
    Check(g_tailCalls == 0, "[3] no save tail ran");

    std::printf("[4] sbNormalTest_RT position 9 (1001b) -> golden GraphicToRadio_RT :1693-1696; page 3 trampoline vs :1685-1691\n");
    Baseline(true);
    SBar("sbNormalTest")->SetPosition(5, false);
    IniConfig.bFTTrayAss2RTTrayAss = true;
    ok = Send("sbNormalTest_RT", "change", ",\"position\":9,\"state\":{\"pgRunMode\":{\"activePageIndex\":2},\"sbNormalTest\":{\"position\":3}}", &ack, &err);
    Check(ok, "[4] form.event ok (" + (ok ? ack.substr(0, 120) : err) + ")");
    Check((int)SBar("sbNormalTest_RT")->Position == 9,
          "[4] bFTTrayAss2RTTrayAss on, state said page 2, but the handler ran on page 3 -> no copy from sbNormalTest (Position 9, got " +
          Num((int)SBar("sbNormalTest_RT")->Position) + ")");
    Check(RG("rgAuto3_RT")->ItemIndex == 1 && RG("rgAuto2_RT")->ItemIndex == 0 && RG("rgAuto1_RT")->ItemIndex == 0 && RG("rgLoad_RT")->ItemIndex == 1,
          "[4] rgAuto3_RT=1, rgAuto2_RT=0, rgAuto1_RT=0, rgLoad_RT=8 clamped to 1 (got " + Num(RG("rgAuto3_RT")->ItemIndex) + "," +
          Num(RG("rgAuto2_RT")->ItemIndex) + "," + Num(RG("rgAuto1_RT")->ItemIndex) + "," + Num(RG("rgLoad_RT")->ItemIndex) + ")");
    Check((int)SBar("sbNormalTest")->Position == 5 && ack.find("state.sbNormalTest ignored") != std::string::npos,
          "[4] state.sbNormalTest dropped (it has its own golden event) -- server keeps 5, todo says so");
    Check(Pc()->ActivePageIndex == 2, "[4] page restored to the state's value (2) after the handler");

    std::printf("[5] clamp and all bits\n");
    Baseline(true);
    ok = Send("sbNormalTest", "change", ",\"position\":20", &ack, &err);
    Check(ok && (int)SBar("sbNormalTest")->Position == 15 && ack.find("outside Min..Max") != std::string::npos &&
          ack.find("\"position\":15") != std::string::npos,
          "[5] position 20 -> VCL clamps to 15, todo + changed.position 15");
    Check(RG("RGAuto3")->ItemIndex == 1 && RG("RGAuto2")->ItemIndex == 1 && RG("RGAuto1")->ItemIndex == 1 && RG("RGLoader")->ItemIndex == 1,
          "[5] 15 -> all four groups 1 (2 / 4 / 8 clamped to Count-1)");

    std::printf("[6] refusals\n");
    Baseline(true);
    ok = Send("sbNormalTest", "click", "", &ack, &err);
    Check(!ok && err.find("no-handler") == 0, "[6] event click -> no-handler (" + err.substr(0, 60) + ")");
    Baseline(true);
    SystemStart = true;
    ok = Send("sbNormalTest", "change", ",\"position\":5", &ack, &err);
    Check(!ok && err.find("running") == 0 && (int)SBar("sbNormalTest")->Position == 0 && RG("RGLoader")->ItemIndex == 1,
          "[6] SystemStart -> running, nothing moved");
    SystemStart = false;

    std::printf("[7] source ratchets (read-only)\n");
    if (argc >= 3) {
        const std::string root = argv[1], web = argv[2];
        std::string py, cpp, js;
        Check(ReadAll(root + "/tools/editlist/TrayForm.py", &py) &&
              py.find("('sbNormalTest', 'change', 'sbNormalTestChange')") != std::string::npos &&
              py.find("('sbNormalTest_RT', 'change', 'sbNormalTest_RTChange')") != std::string::npos,
              "[7] tools/editlist/TrayForm.py events: both rows");
        Check(ReadAll(root + "/FileRW/TrayForm.cpp", &cpp) && CodeHas(cpp, "{\"sbNormalTest\", 2}, {\"sbNormalTest_RT\", 3}"),
              "[7] FileRW/TrayForm.cpp kTA_OnTab: sbNormalTest page 2, sbNormalTest_RT page 3 (live code, not a comment)");
        Check(ReadAll(web + "/ht9045_trayassign_ev.js", &js) && CodeHas(js, "['sbNormalTest', 'sbNormalTest_RT'].forEach(function (id) { CTLS.push([id, 'change']); })") &&
              CodeHas(js, "p.position = v.position"),
              "[7] web/page/ht9045_trayassign_ev.js sends both with position");
    } else {
        Check(false, "[7] argv: <port root> <web/page>");
    }

    for (int i = 0; i < 3; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("real file unchanged: ") + kGuard[i]);
    }
    if (g_events) cJSON_Delete(g_events);
    std::printf("test_ta5_scrollbars: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
