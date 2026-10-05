// ===========================================================================
//  FileRW/Main_D025MenuOpen.cpp -- 主畫面 Tools ▾／Config ▾ 選單鈕按下去（golden TfMain::sbSettingClick／sbConfigClick）。
//
//  AI(W906-D025) 20261001 [W906] St01 新檔。todo D-025（ST01-M 登記；Steven 20260928 S169「任何畫面的事件都是我們做」
//    「沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）。golden＝906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003]（cp950，不在 git），
//    （V912 :N）＝主 repo 那份 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（main.cpp 用 c2f6c75a 之後的行號）。AI(W906-E030-CITE) 20261003：兩支 golden 內容相同，只改引用（Jimmy RULINGS_20261002 第 23 條第 6 項 (b)）。
//    AI(W906-E041) 20261004 [W906] (St01)：todo E-041 —— 本檔的 V912 main.cpp 行號原本是 c2f6c75a（V912 StateRecordImage 多 30 行）之前的，
//    回覆字串（bad-payload、ack.golden 的 kGolden、ack.ran 三條）與註解一起 +30（sbConfigClick :29039-29058、sbSettingClick :29060-29079、
//    sbExitClick :28470-28476、sbExitSetupClick :29192-29196；c2f6c75a^ 的第 n 行＝現在的第 n+30 行逐行核對過）。:26829 之前的 V912 行號不受影響、不動。
//    頁面 web/page/ht9045_main_st01_ev.js 只把 ack.golden 當字顯示（:407），不比對內容。測試 tests/test_d025_menuopen.cpp [2]／[3] 釘新字串。
//    ⚠ 被擋時（filerw::MenuClickRefused）回覆的 golden 是 FileRW/_EditPage.cpp GToolsMenu／GConfigMenu 的閘字串，不是本檔——那兩條還寫 c2f6c75a 之前的
//    V912 :29033-29034／:29012-29013（今天 :29063-29064／:29042-29043），不在 E-041（E-036c 範圍）；測試 [4] 照舊釘那兩個舊字串。
//
//  golden（main.cpp）：
//    :28007-28026（V912 :29039-29058） sbConfigClick（Config ▾ → palConfig）
//        :28009（V912 :29041） sbConfig->Down=false;  :28010-28011（V912 :29042-29043） if(SystemStart) return;  :28013-28014（V912 :29045-29046） if(fSecurity->Insufficient(1)==false) return;
//        :28016（V912 :29048） ProceeToolBar()（:24383-24387（V912 :25091-25095） palSetup／palConfig 都藏）  :28017-28020（V912 :29049-29052） palConfig->Visible=true、BringToFront、Left=3、Top=40
//        :28021（V912 :29053） SetWorkParameter();  :28022（V912 :29054） UpdateMainOperateMode();
//        :28024-28025（V912 :29056-29057） if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.EnterConfig);   //18 Enter Maintenance Page
//    :28028-28047（V912 :29060-29079） sbSettingClick（Tools ▾ → palSetup）
//        :28030（V912 :29062） sbSetting->Down=false;  :28031-28032（V912 :29063-29064） if(SystemStart) return;  :28034-28035（V912 :29066-29067） if(fSecurity->Insufficient(0)==false) return;
//        :28037（V912 :29069） ProceeToolBar()  :28038-28041（V912 :29070-29073） palSetup->Visible=true…  :28042-28043（V912 :29074-29075） MyMaintanBtn[0..9]->Refresh()（只重畫）
//        :28045-28046（V912 :29077-29078） if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.EnterTool);     //17 Enter Tool Page
//    ⚠ Tools ▾ 沒有 SetWorkParameter／UpdateMainOperateMode（只有 Config ▾ 有）—— 照 golden。
//    沒有「再按一次就收起來」：第二下照樣整段重跑（ProceeToolBar 先藏再開 ⇒ 選單還是開著、尾段再跑一次）；收起來靠選單裡的 Exit
//    （sbExitSetupClick :28160-28164（V912 :29192-29196）、sbExitClick :27450-27456（V912 :28470-28476））、運轉中 DoMainPadProcess（:3840-3848（V912 :3970-3978））、ChangeLevelAttr 降到 0（:12662-12668（V912 :13183-13189））。
//
//  移植樹：
//    WS act.main.menuOpen，value={"menu":"setup"|"config"}（tools/wb_serve.cpp:4826 St01 那一行，act.main.menuVisible 分支的後面，同一行）。
//    頁面 web/page/ht9045_main_st01_ev.js initMenuOpen：在 main.html 自己的 click 之前（window capture）攔下 #sbSetting／#sbConfig，
//    送這一條；回 allow 才讓 main.html 原本的 click 跑（toggleMenu 開選單＝上面的 ProceeToolBar＋palX->Visible=true）；已經開著就不再叫
//    toggleMenu（它會收起來，golden 不會）；deny ⇒ 選單不動、狀態列寫理由。開／關之後照舊由 D-015 A01b 的 MutationObserver 回報
//    act.main.menuVisible（本檔不碰那兩個旗標：按鈕＝本檔、看得見＝那一條，互不干擾）。
//    閘：FileRW/_EditPage.cpp 檔尾 filerw::MenuClickRefused ＝ 開窗閘同一份 GToolsMenu／GConfigMenu：
//      SystemStart（:28010（V912 :29042）／:28031（V912 :29063））→ 按鈕的 Enabled 開關 authMainForm[0]／[1]（ChangeLevelAttr :12430（V912 :12951）／:12431（V912 :12952），按鈕灰的 ⇒ 按不到）
//      → fSecurity->Insufficient(0)／(1)。
//    尾段：照 golden 的順序直接呼叫移植樹現有的本體（沒有另翻、沒有解任何閘）：
//      SetWorkParameter()（cinitial.cpp:7086，cinitial.h:76；經下面的測試縫 W906_MenuOpenSetWorkParameter，預設就是它）——
//        ⚠ 會動機台：ChangeSite 寫 IO 輸出 SW[SwZ1SuckMode0/1]、SW[SwZ2SuckMode0/1]（cinitial.cpp:19039-19057，HT9045／12Site／
//        USE_46_SUCKER_DB 機型每次都寫）、DoSetupSystemToProd 寫 SW[Sw10Bit]／SW[Sw10Bit2]（cinitial.cpp:8766-8772）；ReadTechData →
//        fTeach->ReadFile 會重寫 D:\HT9045\system\teach.ini（forms/fTeachPara.cpp:536-537 elTeach 讀完就 UpdateFile），缺鍵時補寫 Gerneral.ini
//        （cinitial.cpp:16139-16140）。不送溫度設定點、不動馬達（20261001 盤點；golden 906 cinitial.cpp:13493-13582（V912 :13505-13594） 同樣這些輸出）。
//        同一支本體已經被 10 個網頁存檔／開窗尾段呼叫（FileRW/MainClick.cpp:341… 等；那些照 R86 運轉中不跑，這裡照 golden 只擋 SystemStart）。
//      fMain->UpdateMainOperateMode()（forms/fMain.cpp:507；wb_serve 開機裝上真本體 forms/fMain_OperateMode.cpp —— 寫
//        D:\HT9045\system\lastdata.dat、切加熱器繼電器、送 ATC7；沒裝＝計數樁，每一支 ctest 都是這樣）
//      EventReport(SECS_EVENT.EnterConfig／EnterTool)（SECSGEM/SecsEventReport.cpp:15 —— 移植樹的自由函式只記最後的 CEID 與次數，
//        沒有接到 HSMS 引擎 ⇒ 今天不會真的送到主機；接上之後這裡一個字都不用改）
//    FormJson 鎖（同 W906_Main_EvB6Op：尾段動到的全域跟 WS 指令、/api/editlist 同一把鎖；CRITICAL_SECTION，可重入）。
//
//  跟 golden 不同的地方 [W906]
//    ① fSecurity->Insufficient 傳 bAlarm=false（沿用開窗閘「偏離 1」，FileRW/_EditPage.cpp S158 段）：golden 不夠時跳 WAR1676。
//       golden 穩態下按不到這一步 —— ChangeLevelAttr（:12430-12431（V912 :12951-12952），Timer2Timer :20978（V912 :21675） 每拍）在等級不夠時就把按鈕設灰了；
//       網頁的按鈕不灰（main.html 是筆電的檔），所以這裡回 not-authorized＋理由（頁面狀態列），不開 WAR1676 框。
//    ② sbSetting／sbConfig->Down=false、ProceeToolBar、palX 的 Visible／BringToFront／Left／Top、MyMaintanBtn->Refresh：畫面，
//       由頁面（main.html toggleMenu）做；這裡只回 allow。
//    ③ golden 只看 SystemStart（不看 SoftStart）—— 照 golden。
//
//  只編進 wb_serve（FileRW/_editlist_sources.cmake；tools/editlist/_integrated.txt 有這個名字，gen_editlist.py:629-631 重產時會保留）。
//  ctest：D025_MenuOpen（tests/test_d025_menuopen.cpp）；頁面那一半 D025_MenuOpenPage（tools/webprobe/d025_menu_selftest.cjs，node，離線）。
// ===========================================================================
#include "cmydef.h"                    // SystemStart（:221）
#include "Config.h"                    // IniConfig.bEnable_SECS_GEM
#include "forms/fMain.h"               // fMain->UpdateMainOperateMode（forms/fMain.cpp:507）
#include "SECSGEM/SecsEventType.h"     // SECS_EVENT.EnterTool／EnterConfig（:59-60）
#include "SECSGEM/SecsEventReport.h"   // EventReport（:55）
#include "Public/cJSON.h"              // value／回覆（ht9045_public）
#include <cstdio>
#include <string>

bool SetWorkParameter();                                                        // cinitial.h:76（golden cinitial.h:17；本體 cinitial.cpp:7086）
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp（CRITICAL_SECTION，可重入）
namespace filerw { bool MenuClickRefused(bool config, std::string* code, std::string* detail, std::string* golden); }   // FileRW/_EditPage.cpp 檔尾

// 測試縫（函式指標安裝座）：預設＝golden SetWorkParameter 的移植本體；ctest 換成記錄器（真本體會重讀 teach.ini 等機台檔）。
bool (*W906_MenuOpenSetWorkParameter)() = &SetWorkParameter;

namespace {
struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};
std::string Print(cJSON* o)
{
    char* s = cJSON_PrintUnformatted(o);
    std::string r = s ? s : "{}";
    if (s) cJSON_free(s);
    cJSON_Delete(o);
    return r;
}
}  // namespace

std::string W906_Main_MenuOpenOp(const std::string& payloadJson, bool* ok)
{
    cJSON* root = payloadJson.empty() ? 0 : cJSON_Parse(payloadJson.c_str());
    const cJSON* m = (root && cJSON_IsObject(root)) ? cJSON_GetObjectItemCaseSensitive(root, "menu") : 0;
    const std::string menu = (m && cJSON_IsString(m)) ? m->valuestring : std::string();
    if (root) cJSON_Delete(root);
    if (menu != "setup" && menu != "config") {
        if (ok) *ok = false;
        return "bad-payload: act.main.menuOpen needs value={\"menu\":\"setup\"|\"config\"} (golden TfMain::sbSettingClick / sbConfigClick, 906 main.cpp:28028 (V912 :29060) / :28007 (V912 :29039))";
    }
    const bool config = (menu == "config");
    const char* const kGolden = config ? "golden 906 main.cpp:28007-28026 (V912 :29039-29058) TfMain::sbConfigClick" : "golden 906 main.cpp:28028-28047 (V912 :29060-29079) TfMain::sbSettingClick";
    FormLockGuard lock;
    std::string code, detail, gate;
    if (filerw::MenuClickRefused(config, &code, &detail, &gate)) {                // golden :28010-28014（V912 :29042-29046）／:28031-28035（V912 :29063-29067）（及按鈕 Enabled :12430-12431（V912 :12951-12952））
        std::printf("act.main.menuOpen %s -> refused (%s): %s\n", menu.c_str(), code.c_str(), gate.c_str());
        std::fflush(stdout);
        if (ok) *ok = false;
        cJSON* o = cJSON_CreateObject();
        cJSON_AddBoolToObject(o, "executed", 0);
        cJSON_AddBoolToObject(o, "allow", 0);
        cJSON_AddStringToObject(o, "menu", menu.c_str());
        cJSON_AddStringToObject(o, "guard", code.c_str());
        cJSON_AddStringToObject(o, "detail", detail.c_str());
        cJSON_AddStringToObject(o, "golden", gate.c_str());
        return Print(o);
    }
    // golden :28016-28020（V912 :29048-29052）／:28037-28043（V912 :29069-29075）：ProceeToolBar＋palX 顯示＋MyMaintanBtn Refresh —— 畫面，頁面收到 allow 才做（檔頭 [W906] ②）
    cJSON* ran = cJSON_CreateArray();
    bool secs = false;
    if (config) {
        if (W906_MenuOpenSetWorkParameter) W906_MenuOpenSetWorkParameter();   // golden 906 main.cpp:28021（V912 :29053） SetWorkParameter();（回傳值 golden 不看）
        cJSON_AddItemToArray(ran, cJSON_CreateString("SetWorkParameter (golden :28021 (V912 :29053))"));
        if (fMain != NULL) fMain->UpdateMainOperateMode();                      // golden 906 main.cpp:28022（V912 :29054）
        cJSON_AddItemToArray(ran, cJSON_CreateString("UpdateMainOperateMode (golden :28022 (V912 :29054))"));
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem   // golden :28024（V912 :29056）
        {
            EventReport(SECS_EVENT.EnterConfig);                                //18     Enter Maintenance Page   // golden :28025（V912 :29057）
            secs = true;
        }
    } else {
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem   // golden :28045（V912 :29077）
        {
            EventReport(SECS_EVENT.EnterTool);                                  //17     Enter Tool Page   // golden :28046（V912 :29078）
            secs = true;
        }
    }
    if (secs) cJSON_AddItemToArray(ran, cJSON_CreateString(config ? "EventReport(SECS_EVENT.EnterConfig) (golden :28025 (V912 :29057))" : "EventReport(SECS_EVENT.EnterTool) (golden :28046 (V912 :29078))"));
    std::printf("act.main.menuOpen %s -> allowed (%s)%s\n", menu.c_str(), kGolden,
                config ? (secs ? ": SetWorkParameter, UpdateMainOperateMode, EventReport EnterConfig" : ": SetWorkParameter, UpdateMainOperateMode")
                       : (secs ? ": EventReport EnterTool" : ""));
    std::fflush(stdout);
    if (ok) *ok = true;
    cJSON* o = cJSON_CreateObject();
    cJSON_AddBoolToObject(o, "executed", 1);
    cJSON_AddBoolToObject(o, "allow", 1);
    cJSON_AddStringToObject(o, "menu", menu.c_str());
    cJSON_AddItemToObject(o, "ran", ran);
    cJSON_AddStringToObject(o, "golden", kGolden);
    return Print(o);
}
