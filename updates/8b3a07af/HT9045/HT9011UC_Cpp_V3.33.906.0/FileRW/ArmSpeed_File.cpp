// ===========================================================================
//  FileRW/ArmSpeed_File.cpp -- ArmSpeed_File 族（ArmSpeed_File[]／SHSpeed_File／MGSpeed_File）的讀寫檔
//  （<recipe>\ArmCondition.Data；A10 存到 machine 時 sSaveByMachine）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（ArmSpeed_File）。
//
//  golden TfSpeed（cSpeed.cpp）由 tools/gen_editlist.py 轉成 ArmSpeed_File.gen.inc（元件改成具名替身）：
//    FormShow（:41）＝開頁（ReadFile＋DoIniDataToForm＋權限）；spbSaveClick（:1433）＝存檔鈕（逐鍵 WriteIniData
//    ＋ReadWriteFile(false)＋存後 ReadFile／SetWorkParameter…）。沒有 HTEditList —— 存檔流程讀的替身全部是
//    mustSend（頁面必須送回開頁時拿到的值）。
//  原本的 A 形狀 bridge（tools/formbridge/TfSpeed.py）只能顯示、不能存，已退役（tools/formbridge/_retired/）。
//  開機讀檔仍是移植樹 fSpeed->ReadFile()（cSpeed.cpp W906_BootReadHotPlateAndSpeed，與 golden 讀法逐行相同）。
// ===========================================================================
#include "FileRW/ArmSpeed_File.gen.inc"

#include <cstdio>
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbSpeedClickTail（FileRW/MainClick.cpp；Q41 第 3 項 SP-6）；佔用原本的空行
#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"   // AI(W906-FRW-S158) 20260927 [W906]: 檔尾 BeforeApply 讀頁面送的 widgets（同 FileRW/TrayForm.cpp）
#include <cmath>            // AI(W906-FRW-S158) 20260927 [W906]: std::floor（檔尾 BeforeApply 驗 position 是整數）

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbSpeedClick 關窗尾段，R85）；佔用原本的空行
void Reload();                                                                              // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden FormClose 的資料那一半）
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled);        // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（Q41 SP-1／SP-2／SP-5 滑桿存檔前重播）
const filerw::PageDesc kPage = {
    "ArmSpeed_File", "TfSpeed", "Setup.Speed.html",
    nullptr, nullptr, 0,
    kSP_SaveReads, (int)(sizeof(kSP_SaveReads) / sizeof(kSP_SaveReads[0])),
    &SP_FormShow, &SaveFlow, "spbSaveClick", &Reload, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &SP_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；reload &SP_ReadFile → &Reload（檔尾，ReadFile＋DoIniDataToForm）；同一行改寫
    &BeforeApply, nullptr,   // AI(W906-FRW-S158) 20260927 [W906]: beforeApply（檔尾，滑桿存檔前重播）；extraJson 不用
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfSpeed 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_Speed_Boot()
{
    if (g_booted) return;
    SP_DfmItems();
    SP_DfmState();
    SP_TfSpeed();
    SP_CreateSaveProxies();
    SP_CreateContainerProxies();
    std::printf("FileRW ArmSpeed_File: TfSpeed proxies ready (%d save reads) -- golden cSpeed.cpp\n",
                (int)(sizeof(kSP_SaveReads) / sizeof(kSP_SaveReads[0])));
    g_booted = true;
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 SP-6（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 spbSaveClick（cSpeed.cpp:1433）跑完之後，補 golden 主畫面
//    TfMain::sbSpeedClick 的關窗尾段（V912 main.cpp:28696-28699：DoStructUnitConvert、bEnable_SECS_GEM 時 EventReport(EnterSpeed)；
//    本體 FileRW/MainClick.cpp W906_Main_sbSpeedClickTail，運轉中不跑 R86）。
//    golden Speed 存檔鈕沒有 A02 守衛、沒有 Close()，一定寫檔（savedMark "spbSaveClick" 一進來就記）→ 只有「存完就跑」一種；
//    golden 要等 Exit 關窗（FormClose cSpeed.cpp:1272）才跑尾段，網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//    差別：golden「開頁、沒存就關」也跑尾段（值沒變，換算結果相同；EnterSpeed 事件會少送一次 —— 見尾段本體的註解）。
//    描述檔 tools/editlist/ArmSpeed_File.py 不改、gen.inc 不重產。（⛔ 20260927 晚補：這句只講關窗尾段這一段；之後的滑桿重播／form.event 改了描述檔、重產 gen.inc，見檔尾）
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    SP_spbSaveClick();                                                          // golden cSpeed.cpp:1433
    if (filerw::ELMarked("spbSaveClick"))                                       // ＝PageDesc 的 savedMark（golden 存檔鈕一定寫檔）
        if (const char* w = W906_Main_sbSpeedClickTail()) filerw::ELTodo(w);    // golden main.cpp:28696-28699
}
}  // namespace

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 盤點 SP-1／SP-2／SP-5（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.5；
//    St02 FROM_STEVEN §4 20260927 17:21 (a)）—— editlist.save 套值前（FileRW/_EditPage.h PageDesc::beforeApply）：頁面拖了滑桿，
//    伺服器照 golden 重播滑桿的 OnChange，存檔讀的三個唯讀欄位由 C++ 算，不靠頁面。
//
//    golden V912（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp）：
//      存檔鈕 spbSaveClick :1453-1455 寫 [All] Speed／Accel／EPControl ＝ edAllSpeed／edAllAccSpeed／edEPControl 的 Text。
//      這三格 DFM 是 Enabled=False＋ReadOnly=True（cSpeed.dfm:79／:105／:716），使用者打不了字；值只有這幾個來源：
//        ReadFile :354-363（開頁＝檔案值）、滑桿 OnChange tbAllSpeedChange :1284／tbAccSpeedChange :1347／tbEPControlChange :1410
//        （＝Position）、spbSetToDefClick :1816-1817（＝100，同時 :1814-1815 也把 Position 設成 100）。
//      ⇒ 存進檔的 [All] 值＝存檔當下滑桿的 Position。處理器的先後：使用者拖一下就跑一次（VCL SetPosition → Changed → OnChange）。
//    伺服器端原本的問題：三格唯讀 → PageSave「不可改的丟掉」丟頁面值；頁面送回的 position 用 SetPosition(v,false) 直接套、
//      不觸發 OnChange（FileRW/_EditList.h ELTrackBar），而且 tbAllSpeed 被建構子停用（:35）、連 position 都丟掉
//      ⇒ 存進去的是開頁時的檔案值（St02 的頁面算對了也存不進去）。
//
//    做法（頁面有沒有自己算、有沒有送 form.event 都一樣）：頁面送來的 position 與伺服器端不同 ⇒ 照 VCL 設 Position
//    （夾在 Min..Max，值有變就觸發 OnChange＝golden tb*Change；速度上限 CosFunction.iLimitMaxSpeed 由 FormShow :91 設的 Max 管）。
//      (1) 先套九個軸勾選框（cSpeed.dfm:533-655，每點一下 golden 就跑 cbIndexArmClick :1426）：tb*Change 依勾選決定改哪幾軸。
//      (2) tbAllSpeed：golden 要先點過勾選框（:1428）、全選（:1804）或回預設值（:1818 → :1804）才拖得動；伺服器端還停用、
//          頁面卻拖了 ⇒ 那一下點擊伺服器沒看到。只要其中一個元件點得到，golden 使用者就做得到 ⇒ 照 golden 跑 cbIndexArmClick
//          （只把三個元件 Enabled=true，沒有資料效果；golden 之後也一直開著），ack.session.trace 記一筆。一個都點不到 ⇒ 不動
//          （PageSave 丟值、ack.ignored）。
//      (3) 三條滑桿照 DFM 順序（:53／:108／:692）重播。輸出互不重疊（tbAll 只寫速度欄＋ud*Spd、tbAcc 只寫加速度欄＋ud*Acc、
//          tbEP 只寫 edEPControl），輸入只有勾選框與自己的 Position ⇒ 使用者拖的先後不影響結果，不用像 Temp_Set TS-1 那樣拒存。
//    各軸速度／加速度欄（edIndexSpeed、edInXYSpd…）照舊是「頁面最後狀態」：它們可改，PageSave 在這之後套頁面值（蓋掉重播寫的值）
//      ——頁面上看到什麼就存什麼。頁面自己照 golden 算了（St02 web/page/ht9045_speed_c.js），或使用者拖完又手改，都以頁面為準；
//      伺服器分不出「頁面沒算」與「使用者又改回原值」，不替頁面決定。
//      例外：不可改的軸欄（權限、ATP 開批鎖 edIndexSpeed :325-328）PageSave 丟頁面值 ⇒ 留下重播寫的值 ＝ golden（滑桿照樣改停用的欄）。
//      //AI(W906-FRW-S158) 20260927: 照翻，但看起來是 golden 的漏洞 —— CosFunction.bLotStartLockCriticalPara 開批後鎖住的
//      Index 速度（bAuthCriticalPara[11]），勾 Index Arm 拖「全部速度」滑桿就改得掉、存得進去。要不要擋是 Steven 的決定（交件 R 題）。
//    ⚠ 這裡只看得到結果、看不到點擊：±10、全選、回預設值要伺服器照 golden 算，頁面就送 form.event（下面 g_evreg；
//      回預設值會改到頁面停用的欄位，只有 form.event 做得到 —— 見交件 R 題）。
// ---------------------------------------------------------------------------
namespace {
const char* const kSP_Axis[9] = {"cbIndexArm", "cbInArm", "cbOutArm", "cbShuttle", "cbTrayArm",
                                 "cbInArmZ", "cbOutArmZ", "cbInRotate", "cbOutRotate"};   // golden cSpeed.dfm:533-655（OnClick＝cbIndexArmClick）

void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {   // PageSave 已驗過是物件；保險
        if (root) cJSON_Delete(root);
        return;
    }
    const char* const F = "TfSpeed";
    // (1) 九個軸勾選框
    for (int i = 0; i < 9; ++i) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, kSP_Axis[i]);
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
        if (!v || !cJSON_IsBool(v)) continue;                                   // 沒送／型別不對 → PageSave（型別不對回 400）
        TCheckBox* c = EL<TCheckBox>(F, kSP_Axis[i]);
        const bool want = cJSON_IsTrue(v) != 0;
        if (want == (bool)c->Checked || !filerw::ELEditable(F, kSP_Axis[i])) continue;   // 沒點過／點不到 → PageSave（ack.ignored）
        c->Checked = want;                                                      // VCL：點一下 ⇒ Checked 反轉 → OnClick
        SP_cbIndexArmClick();                                                   // golden cSpeed.cpp:1426
        handled->push_back(kSP_Axis[i]);
    }
    // (2)(3) 三條滑桿
    static const char* const kTb[3] = {"tbAllSpeed", "tbAccSpeed", "tbEPControl"};   // golden cSpeed.dfm:53／:108／:692
    for (int k = 0; k < 3; ++k) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, kTb[k]);
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "position") : nullptr;
        if (!v || !cJSON_IsNumber(v) || v->valuedouble != std::floor(v->valuedouble) ||
            v->valuedouble < -1e9 || v->valuedouble > 1e9) continue;            // 沒送／型別不對 → PageSave
        filerw::ELTrackBar* t = EL<filerw::ELTrackBar>(F, kTb[k]);
        const int want = (int)v->valuedouble;
        if (want == (int)t->Position) continue;                                 // 沒拖過，或頁面送過 form.event、值已同步
        if (k == 0 && !t->Enabled) {                                            // (2)
            bool reach = filerw::ELOperable(F, "spbSelectAll") || filerw::ELOperable(F, "spbSetToDef");
            for (int i = 0; i < 9 && !reach; ++i) reach = filerw::ELOperable(F, kSP_Axis[i]);
            if (reach) {
                SP_cbIndexArmClick();                                           // golden cSpeed.cpp:1426（tbAllSpeed／spbSpeedAdd／spbSpeedDec Enabled=true）
                filerw::ELMark("tbAllSpeed moved on the page while the server still had it disabled (golden TfSpeed constructor cSpeed.cpp:35): "
                               "the enabling click (cbIndexArmClick / spbSelectAllClick) was not sent -- golden cbIndexArmClick (cSpeed.cpp:1426) replayed");
            }
        }
        if (!filerw::ELEditable(F, kTb[k])) continue;                           // 點不到（容器停用／看不見，例：沒裝 CKD 時 gbEPControl 藏起來 ReadFile :368）→ PageSave（ack.ignored）
        t->Position = want;                                                     // VCL SetPosition：夾 Min..Max、值有變 ⇒ OnChange＝golden tb*Change
        handled->push_back(kTb[k]);
    }
    cJSON_Delete(root);
}

// PageSave 在 BeforeApply 改過替身之後回 400（頁面值型別不對）時的還原 —— golden 關頁 FormClose（cSpeed.cpp:1272-1276）的資料那一半
//   （ReadFile＋DoIniDataToForm，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）；不做 fShow=false（頁面沒關）。
//   原本這一格是 &SP_ReadFile：BeforeApply 重播過 tbAllSpeedChange 之後，ReadFile 把 Position 設回檔案值會再觸發一次 OnChange、
//   把勾選的軸欄寫成 [All] 的檔案值；不接著 DoIniDataToForm 就留在替身上，下次存檔時那一格若不可改（丟頁面值）就被存進去。
//   golden 存檔鈕一定寫檔（savedMark 一進來就記），PageSave 的「沒寫檔就 reload」在這一頁不會發生。
void Reload()
{
    SP_ReadFile();          // golden cSpeed.cpp:1275
    SP_DoIniDataToForm();   // golden cSpeed.cpp:1276
}

// AI(W906-FRW-S158) 20260927 [W906]: WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 SP-2／SP-3／SP-4）——
//   頁面點軸勾選框／全選／回預設值／±10 → golden cbIndexArmClick（cSpeed.cpp:1426）／spbSelectAllClick（:1795）／spbSetToDefClick（:1812）／
//   spbSpeedAddClick（:1414）／spbSpeedDecClick（:1421），changed 回頁面。表 kSP_Events 由 tools/gen_editlist.py 產生（ArmSpeed_File.gen.inc 檔尾），
//   本體 FileRW/_EditPage.cpp RunPageEvent。五支都只改元件的值／Enabled，不寫檔、不碰機台。
//   滑桿（tb*Change）不在表裡：form.event 沒有 position 欄位（見 tools/editlist/ArmSpeed_File.py 'events' 註解）。
//   ±10 要頁面在 state 帶目前的 {"tbAllSpeed":{"position":n}}＋九個勾選框（處理器讀伺服器端的 Position 與勾選）。
filerw::PageEventsRegistrar g_evreg("ArmSpeed_File", kSP_Events, (int)(sizeof(kSP_Events) / sizeof(kSP_Events[0])));
}  // namespace
