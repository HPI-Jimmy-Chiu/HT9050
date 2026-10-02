// ===========================================================================
//  FileRW/Main_D025MenuOpen.cpp -- 主畫面 Tools ▾／Config ▾ 選單鈕按下去（golden TfMain::sbSettingClick／sbConfigClick）。
//
//  AI(W906-D025) 20261001 [W906] St01 新檔。todo D-025（ST01-M 登記；Steven 20260928 S169「任何畫面的事件都是我們做」
//    「沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）。golden 一律照主 repo 那份 V912：
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950），行號都是那一份的。
//
//  golden（main.cpp）：
//    :29009-29028 sbConfigClick（Config ▾ → palConfig）
//        :29011 sbConfig->Down=false;  :29012-29013 if(SystemStart) return;  :29015-29016 if(fSecurity->Insufficient(1)==false) return;
//        :29018 ProceeToolBar()（:25091-25095 palSetup／palConfig 都藏）  :29019-29022 palConfig->Visible=true、BringToFront、Left=3、Top=40
//        :29023 SetWorkParameter();  :29024 UpdateMainOperateMode();
//        :29026-29027 if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.EnterConfig);   //18 Enter Maintenance Page
//    :29030-29049 sbSettingClick（Tools ▾ → palSetup）
//        :29032 sbSetting->Down=false;  :29033-29034 if(SystemStart) return;  :29036-29037 if(fSecurity->Insufficient(0)==false) return;
//        :29039 ProceeToolBar()  :29040-29043 palSetup->Visible=true…  :29044-29045 MyMaintanBtn[0..9]->Refresh()（只重畫）
//        :29047-29048 if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.EnterTool);     //17 Enter Tool Page
//    ⚠ Tools ▾ 沒有 SetWorkParameter／UpdateMainOperateMode（只有 Config ▾ 有）—— 照 golden。
//    沒有「再按一次就收起來」：第二下照樣整段重跑（ProceeToolBar 先藏再開 ⇒ 選單還是開著、尾段再跑一次）；收起來靠選單裡的 Exit
//    （sbExitSetupClick :29162-29166、sbExitClick :28440-28446）、運轉中 DoMainPadProcess（:3970-3978）、ChangeLevelAttr 降到 0（:13183-13189）。
//
//  移植樹：
//    WS act.main.menuOpen，value={"menu":"setup"|"config"}（tools/wb_serve.cpp:4826 St01 那一行，act.main.menuVisible 分支的後面，同一行）。
//    頁面 web/page/ht9045_main_st01_ev.js initMenuOpen：在 main.html 自己的 click 之前（window capture）攔下 #sbSetting／#sbConfig，
//    送這一條；回 allow 才讓 main.html 原本的 click 跑（toggleMenu 開選單＝上面的 ProceeToolBar＋palX->Visible=true）；已經開著就不再叫
//    toggleMenu（它會收起來，golden 不會）；deny ⇒ 選單不動、狀態列寫理由。開／關之後照舊由 D-015 A01b 的 MutationObserver 回報
//    act.main.menuVisible（本檔不碰那兩個旗標：按鈕＝本檔、看得見＝那一條，互不干擾）。
//    閘：FileRW/_EditPage.cpp 檔尾 filerw::MenuClickRefused ＝ 開窗閘同一份 GToolsMenu／GConfigMenu：
//      SystemStart（:29012／:29033）→ 按鈕的 Enabled 開關 authMainForm[0]／[1]（ChangeLevelAttr :12951／:12952，按鈕灰的 ⇒ 按不到）
//      → fSecurity->Insufficient(0)／(1)。
//    尾段：照 golden 的順序直接呼叫移植樹現有的本體（沒有另翻、沒有解任何閘）：
//      SetWorkParameter()（cinitial.cpp:7086，cinitial.h:76；經下面的測試縫 W906_MenuOpenSetWorkParameter，預設就是它）——
//        ⚠ 會動機台：ChangeSite 寫 IO 輸出 SW[SwZ1SuckMode0/1]、SW[SwZ2SuckMode0/1]（cinitial.cpp:19039-19057，HT9045／12Site／
//        USE_46_SUCKER_DB 機型每次都寫）、DoSetupSystemToProd 寫 SW[Sw10Bit]／SW[Sw10Bit2]（cinitial.cpp:8766-8772）；ReadTechData →
//        fTeach->ReadFile 會重寫 D:\HT9045\system\teach.ini（forms/fTeachPara.cpp:536-537 elTeach 讀完就 UpdateFile），缺鍵時補寫 Gerneral.ini
//        （cinitial.cpp:16139-16140）。不送溫度設定點、不動馬達（20261001 盤點；golden cinitial.cpp:13505-13594 同樣這些輸出）。
//        同一支本體已經被 10 個網頁存檔／開窗尾段呼叫（FileRW/MainClick.cpp:341… 等；那些照 R86 運轉中不跑，這裡照 golden 只擋 SystemStart）。
//      fMain->UpdateMainOperateMode()（forms/fMain.cpp:507；wb_serve 開機裝上真本體 forms/fMain_OperateMode.cpp —— 寫
//        D:\HT9045\system\lastdata.dat、切加熱器繼電器、送 ATC7；沒裝＝計數樁，每一支 ctest 都是這樣）
//      EventReport(SECS_EVENT.EnterConfig／EnterTool)（SECSGEM/SecsEventReport.cpp:15 —— 移植樹的自由函式只記最後的 CEID 與次數，
//        沒有接到 HSMS 引擎 ⇒ 今天不會真的送到主機；接上之後這裡一個字都不用改）
//    FormJson 鎖（同 W906_Main_EvB6Op：尾段動到的全域跟 WS 指令、/api/editlist 同一把鎖；CRITICAL_SECTION，可重入）。
//
//  跟 golden 不同的地方 [W906]
//    ① fSecurity->Insufficient 傳 bAlarm=false（沿用開窗閘「偏離 1」，FileRW/_EditPage.cpp S158 段）：golden 不夠時跳 WAR1676。
//       golden 穩態下按不到這一步 —— ChangeLevelAttr（:12951-12952，Timer2Timer :21675 每拍）在等級不夠時就把按鈕設灰了；
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
        return "bad-payload: act.main.menuOpen needs value={\"menu\":\"setup\"|\"config\"} (golden TfMain::sbSettingClick / sbConfigClick, V912 main.cpp:29030 / :29009)";
    }
    const bool config = (menu == "config");
    const char* const kGolden = config ? "golden V912 main.cpp:29009-29028 TfMain::sbConfigClick" : "golden V912 main.cpp:29030-29049 TfMain::sbSettingClick";
    FormLockGuard lock;
    std::string code, detail, gate;
    if (filerw::MenuClickRefused(config, &code, &detail, &gate)) {                // golden :29012-29016／:29033-29037（及按鈕 Enabled :12951-12952）
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
    // golden :29018-29022／:29039-29045：ProceeToolBar＋palX 顯示＋MyMaintanBtn Refresh —— 畫面，頁面收到 allow 才做（檔頭 [W906] ②）
    cJSON* ran = cJSON_CreateArray();
    bool secs = false;
    if (config) {
        if (W906_MenuOpenSetWorkParameter) W906_MenuOpenSetWorkParameter();   // golden main.cpp:29023 SetWorkParameter();（回傳值 golden 不看）
        cJSON_AddItemToArray(ran, cJSON_CreateString("SetWorkParameter (golden :29023)"));
        if (fMain != NULL) fMain->UpdateMainOperateMode();                      // golden main.cpp:29024
        cJSON_AddItemToArray(ran, cJSON_CreateString("UpdateMainOperateMode (golden :29024)"));
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem   // golden :29026
        {
            EventReport(SECS_EVENT.EnterConfig);                                //18     Enter Maintenance Page   // golden :29027
            secs = true;
        }
    } else {
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem   // golden :29047
        {
            EventReport(SECS_EVENT.EnterTool);                                  //17     Enter Tool Page   // golden :29048
            secs = true;
        }
    }
    if (secs) cJSON_AddItemToArray(ran, cJSON_CreateString(config ? "EventReport(SECS_EVENT.EnterConfig) (golden :29027)" : "EventReport(SECS_EVENT.EnterTool) (golden :29048)"));
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
