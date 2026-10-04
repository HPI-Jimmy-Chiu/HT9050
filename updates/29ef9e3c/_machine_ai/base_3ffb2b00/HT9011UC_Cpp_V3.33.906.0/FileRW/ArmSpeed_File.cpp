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
#include <map>              //AI(W906-FRW-S167) 20260928 [W906] R119=B: 檔尾「開頁鎖住的欄位」快照／還原
#include <set>
#include <stdexcept>

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbSpeedClick 關窗尾段，R85）；佔用原本的空行
void Reload();                                                                              // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden FormClose 的資料那一半）
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled);        // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（Q41 SP-1／SP-2／SP-5 滑桿存檔前重播）
void FormShowFlow();                                  //AI(W906-FRW-S167) 20260928 [W906] R119=B: 定義在檔尾（golden FormShow＋記下開頁鎖住的欄位）
std::string ExtraJson();                              //AI(W906-FRW-S167) 20260928 [W906] R119=B: 定義在檔尾（editlist.get 的 extra.lockedAtOpen）
void R119Restore(const char* after);                  //AI(W906-FRW-S167) 20260928 [W906] R119=B: 定義在檔尾（鎖住的欄位放回開頁值）
void R119BootProxies();                               //AI(W906-FRW-S167) 20260928 [W906]: 定義在檔尾（form.event 控制項的替身＋開機停用名單）
const filerw::PageEvent* R119EventTable();            //AI(W906-FRW-S167) 20260928 [W906] R119=B: 定義在檔尾（kSP_Events 同一張表、處理器包一層）
const filerw::PageDesc kPage = {
    "ArmSpeed_File", "TfSpeed", "Setup.Speed.html",
    nullptr, nullptr, 0,
    kSP_SaveReads, (int)(sizeof(kSP_SaveReads) / sizeof(kSP_SaveReads[0])),
    //AI(W906-FRW-S167) 20260928 [W906] R119=B: formShow &SP_FormShow → &FormShowFlow（檔尾：golden FormShow 之後記下開頁鎖住的欄位）；extraJson nullptr → &ExtraJson（檔尾）
    &FormShowFlow, &SaveFlow, "spbSaveClick", &Reload, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &SP_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；reload &SP_ReadFile → &Reload（檔尾，ReadFile＋DoIniDataToForm）；同一行改寫
    &BeforeApply, &ExtraJson,   // AI(W906-FRW-S158) 20260927 [W906]: beforeApply（檔尾，滑桿存檔前重播）；extraJson 不用（⛔ 20260928 R119 起用，見上一行）
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
    R119BootProxies();   //AI(W906-FRW-S167) 20260928 [W906]: form.event 控制項缺的替身（spbSetToDef／spbSelectAll…）＋記下「開機就停用」（DFM／建構子）的名單（檔尾）
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
    R119Restore("the save-time slider replay (BeforeApply) / page values");   //AI(W906-FRW-S167) 20260928 [W906] R119=B: golden 存檔鈕讀替身之前，開頁鎖住的欄位放回開頁值（檔尾）
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
//      //AI(W906-FRW-S167) 20260928 [W906] ⛔ 更正：Steven 已裁決 R119＝B（「BCB的不動, c++版本要修正」）——上面「留下重播寫的值 ＝ golden」
//      不再成立：開頁被 golden FormShow 鎖住的欄位，SaveFlow 在 golden 存檔鈕之前放回開頁值（偏離 golden，見檔尾 R119 段）。
//    ⚠ 這裡只看得到結果、看不到點擊：±10、全選、回預設值要伺服器照 golden 算，頁面就送 form.event（下面 g_evreg；
//      回預設值會改到頁面停用的欄位，只有 form.event 做得到 —— 見交件 R 題）。
//      //AI(W906-EVB1) 20260928 [W906] X-2：拖滑桿本身也可以送 form.event 了（{"control":"tbAllSpeed","event":"change","position":n}，
//      下面 g_evreg 那段）；這段重播只替「沒送事件就存」的頁面補。
//      //AI(W906-FRW-S167) 20260928 [W906] ⛔ 更正：R119＝B 之後，「開頁就被 golden FormShow 鎖住」的欄位 form.event 也不改（處理器之後放回開頁值，
//      檔尾 R119 段）；DFM 設計期就停用的唯讀欄（edAllSpeed…）與看不見的欄照舊由處理器改。
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
//   //AI(W906-EVB1) 20260928 [W906] X-2 ⛔ 更正：上面「滑桿不在表裡」已過期 —— form.event 多了 "position"（FileRW/_FormEvent.h 檔頭），
//   kSP_Events 表尾多三列 tbAllSpeed／tbAccSpeed／tbEPControl "change" → golden tbAllSpeedChange（cSpeed.cpp:1282）／tbAccSpeedChange（:1345）／
//   tbEPControlChange（:1408）。RunPageEvent 先照 VCL 設 Position（夾 Min..Max，SetPosition(v,false) 不走替身的 OnChange 指標）再跑表上的
//   處理器 ⇒ 經 R119EventTable 的包裝（R119EvKeep：處理器之後放回開頁鎖住的欄位），不會繞過 R119。三條滑桿的替身開機就有：
//   SP_DfmState（ArmSpeed_File.gen.inc :709-715 DfmInit／OnChange，FileRW_Speed_Boot 在 R119BootProxies 之前呼叫）；R119BootProxies 的
//   「沒有替身就警告」迴圈照 kSP_Events 逐列查，這三列一併查到。tbAllSpeed 被建構子停用（cSpeed.cpp:35）：頁面要先送勾選框 click
//   （cbIndexArmClick :1428 打開它）或全選／回預設值，否則 RunPageEvent 第 2 步照 golden 回 bad-payload（點不到）。
//   存檔前重播（上面 BeforeApply）照留：頁面沒送事件就存時還是靠它；送過事件的滑桿伺服器值已同步，BeforeApply 看到一樣就跳過。
//   //AI(W906-EVB1) 20260928 [W906] ⛔ 更正：上面「±10 要頁面在 state 帶 tbAllSpeed＋九個勾選框」已過期 —— 它們都在事件表上，RunPageEvent 第 4 步不收它們的 state（處理器讀伺服器端的值）；頁面要先送它們自己的 form.event。
filerw::PageEventsRegistrar g_evreg("ArmSpeed_File", R119EventTable(), (int)(sizeof(kSP_Events) / sizeof(kSP_Events[0])));   //AI(W906-FRW-S167) 20260928 [W906] R119=B: kSP_Events → R119EventTable()（檔尾）；同一行改寫
}  // namespace

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S167) 20260928 [W906] R119=B —— 偏離 golden（Steven 20260928：「R119 BCB的不動, c++版本要修正」；
//    D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md「### R119.」）。
//
//    golden V912（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp）開頁 FormShow 會把幾個欄位鎖住（Enabled=false），
//    操作員打不了字；但「全部速度／加速度／EP」滑桿（tbAllSpeedChange :1282／tbAccSpeedChange :1345／tbEPControlChange :1408；
//    ±10 spbSpeedAddClick :1414／spbSpeedDecClick :1421 經 tbAllSpeedChange）與「Set to define」（spbSetToDefClick :1812）是程式
//    直接寫欄位、不管鎖，存檔鈕照存 —— 鎖可以被繞過：
//      * ATP 開批鎖關鍵參數（CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart，:317-329）：edIndexVacumCheckTime／
//        edIndexAirOnTime（bAuthCriticalPara[6]）、edIndexSpeed（bAuthCriticalPara[11]）；
//      * 機台內有料時 Input Arm 的 Auto Skip（IniConfig.bRecordSkipPosition，:143-154）：rgInArmAutoSpeed；
//      * 多站＋大 IC 吸嘴開合只能 Fixed（:131-141）：rgInArmPitch／rgOutArmPitch（值已由 ReadFile :504-511／:634-642 改成 Fixed）。
//    R119=B：這些欄位滑桿與回預設值都不改、存回開頁值；其他欄照 golden 改。
//
//    「開頁鎖住」怎麼算（照 golden 當下的狀態算，不寫死欄位名單）：
//      (1) 開機時（R119BootProxies：DFM 設計期狀態 SP_DfmState＋建構子 SP_TfSpeed 之後）就停用的替身記成 g_bootOff，不算鎖：
//          DFM 就停用的唯讀顯示欄 edAllSpeed／edAllAccSpeed／edEPControl（cSpeed.dfm:79／:105／:716，滑桿本來就是要寫它們）、
//          edIndexAccDec／udIndexAcc（DFM 停用＋看不見）、edtTryAcc、建構子停用的 tbAllSpeed／spbSpeedAdd／spbSpeedDec（:35-37）。
//      (2) 開頁（FormShowFlow：golden FormShow 跑完，含它呼叫的 ReadFile／DoIniDataToForm）時 Enabled=false、又不在 g_bootOff 的
//          替身 ＝「鎖點」。golden FormShow 的 ->Enabled= 目標（48 個，:56-58／:129-141／:143-154／:158-187／:193-194／:235-245／
//          :279-296／:299-309／:317-329）沒有一個在 g_bootOff 裡（20260928 對 ArmSpeed_File.gen.inc 的 SP_DfmState 與 SP_TfSpeed 核過），
//          所以 (1) 不會把真的鎖漏掉；ReadFile／DoIniDataToForm 沒有 ->Enabled=。
//      (3) 自己或 DFM 父層鏈（kSP_ParentOf＋kR119EvParents，同 filerw::ELOperable 看的那一條）碰到鎖點、而且帶值（TEdit／TCheckBox／
//          TRadioButton／TRadioGroup／TComboBox／TTrackBar·TUpDown）的替身 ＝ 鎖住的欄位；另外 TUpDown 的 Associate 是鎖住的欄位時
//          它也算（VCL 設 udIndexSpd->Position 會把數字寫進 edIndexSpeed —— tbAllSpeedChange :1289 就這樣寫；DFM :2269 Associate）。
//          記下開頁值（g_kept）。只看 Enabled、不看 Visible：看不見不是鎖（golden 會存看不見的欄位，回預設值照樣改它們）。
//      ⚠ 同一條規則比 R119 題目舉的三組多涵蓋一種：CC_ASE_CL 依權限停用的群組（:158-177，例 gbInArmRetry 裡的 edInArmRetryCount：
//        golden 回預設值改成 2，這裡保留原值）。其他鎖點（分頁權限 :181-187、SPIL 的 pnl*Y :235-245、rgOutArmPickupErrMode :299-309…）
//        滑桿與回預設值本來就寫不到或點不到，結果不變。
//    放回的時機：
//      * editlist.save：SaveFlow 在 golden 存檔鈕之前（滑桿重播 BeforeApply、PageSave 套頁面值都做完了。頁面送的鎖住欄位值 PageSave
//        本來就丟掉（不可改）；這裡蓋掉的是重播寫的值，以及頁面送的 TUpDown position 經 Associate 寫進去的值）。
//      * form.event：R119EventTable 的每一個處理器之後（處理器丟例外也放回再丟）——RunPageEvent 的 changed 就不會帶鎖住的欄位。
//    有放回時 ack 的 todo 記一筆（列出欄位）。開頁值在每次 editlist.get（golden FormShow）重記。editlist.get 多帶
//    "extra":{"rule":"R119=B","lockedAtOpen":[…],"lockRoots":[…]}，頁面自己算的滑桿／回預設值要跳過 lockedAtOpen 的欄位。
// ---------------------------------------------------------------------------
namespace {
const char* const kR119Form = "TfSpeed";
// form.event 控制項缺的替身：產生器（tools/gen_editlist.py 的 names_used）只建 golden 方法本體裡用到的名字 ——
//   spbSetToDef 只出現在事件表、spbSelectAll 只出現在它自己的處理器裡 ⇒ 開機沒有替身，RunPageEvent 回 handler-failed
//   「internal: event proxy not created」（FileRW/_EditPage.cpp RunPageEvent），editlist.get 的 events 也標 operable:false。
//   這裡照 golden cSpeed.h 的型別先建好；spbSetToDef 的父層照 golden cSpeed.dfm:506（在 gbUnitForChange :127 裡；
//   其他 12 個事件控制項已在 kSP_ParentOf）。
const char* const kR119EvParents[][2] = {{"spbSetToDef", "gbUnitForChange"}};
std::set<std::string> g_bootOff;       // (1)
std::vector<std::string> g_lockRoots;  // (2)
struct Kept {
    std::string name;
    TControl* c = nullptr;
    AnsiString text;
    int index = -1;
    bool checked = false;
    int pos = 0;
};
std::vector<Kept> g_kept;              // (3)：TTrackBar／TUpDown 排前面（放回時先放它們，Associate 寫的字再由 TEdit 放回原字）
bool g_keptValid = false;

std::map<std::string, bool> R119EnabledNow()
{
    std::map<std::string, bool> r;
    cJSON* s = cJSON_Parse(filerw::ProxyStateJson(kR119Form).c_str());
    if (s && cJSON_IsObject(s))
        for (const cJSON* it = s->child; it; it = it->next) {
            if (!it->string) continue;
            const cJSON* e = cJSON_GetObjectItemCaseSensitive(it, "enabled");
            r[it->string] = !(e && cJSON_IsFalse(e));
        }
    if (s) cJSON_Delete(s);
    return r;
}

const std::map<std::string, std::string>& R119Parents()
{
    static std::map<std::string, std::string> m;
    if (m.empty()) {
        for (std::size_t i = 0; i < sizeof(kSP_ParentOf) / sizeof(kSP_ParentOf[0]); ++i) m[kSP_ParentOf[i][0]] = kSP_ParentOf[i][1];
        for (std::size_t i = 0; i < sizeof(kR119EvParents) / sizeof(kR119EvParents[0]); ++i) m[kR119EvParents[i][0]] = kR119EvParents[i][1];
    }
    return m;
}

bool R119HasValue(TControl* c)
{
    return dynamic_cast<filerw::ELTrackBar*>(c) || dynamic_cast<TCustomEdit*>(c) || dynamic_cast<TCheckBox*>(c) ||
           dynamic_cast<TRadioButton*>(c) || dynamic_cast<TRadioGroup*>(c) || dynamic_cast<TComboBox*>(c);
}

void R119BootProxies()
{
    for (int i = 0; i < 9; ++i) EL<TCheckBox>(kR119Form, kSP_Axis[i]);                                    // golden cSpeed.h：TCheckBox（DFM :520-655）
    static const char* const kBtn[] = {"spbSelectAll", "spbSetToDef", "spbSpeedAdd", "spbSpeedDec"};   // golden cSpeed.h：TSpeedButton（DFM :367／:506／:141／:254）
    for (std::size_t i = 0; i < sizeof(kBtn) / sizeof(kBtn[0]); ++i) EL<TSpeedButton>(kR119Form, kBtn[i]);
    filerw::ELSetParents(kR119Form, kR119EvParents, (int)(sizeof(kR119EvParents) / sizeof(kR119EvParents[0])));
    for (std::size_t i = 0; i < sizeof(kSP_Events) / sizeof(kSP_Events[0]); ++i)
        if (!filerw::ELFind(kR119Form, kSP_Events[i].control))
            std::printf("FileRW ArmSpeed_File: WARNING form.event control %s has no proxy (add it to R119BootProxies)\n",
                        kSP_Events[i].control);
    const std::map<std::string, bool> en = R119EnabledNow();
    g_bootOff.clear();
    for (std::map<std::string, bool>::const_iterator it = en.begin(); it != en.end(); ++it)
        if (!it->second) g_bootOff.insert(it->first);
}

void R119Snapshot()
{
    g_kept.clear();
    g_lockRoots.clear();
    g_keptValid = true;
    const std::map<std::string, bool> en = R119EnabledNow();
    std::set<std::string> roots;
    for (std::map<std::string, bool>::const_iterator it = en.begin(); it != en.end(); ++it)
        if (!it->second && !g_bootOff.count(it->first)) {
            roots.insert(it->first);
            g_lockRoots.push_back(it->first);
        }
    if (roots.empty()) return;
    const std::map<std::string, std::string>& par = R119Parents();
    std::vector<std::string> names;
    std::set<TControl*> locked;
    for (std::map<std::string, bool>::const_iterator it = en.begin(); it != en.end(); ++it) {
        TControl* c = filerw::ELFind(kR119Form, it->first.c_str());
        if (!c || !R119HasValue(c)) continue;
        bool hit = false;
        std::string cur = it->first;
        for (int depth = 0; depth < 64 && !cur.empty() && !hit; ++depth) {
            hit = roots.count(cur) != 0;
            std::map<std::string, std::string>::const_iterator p = par.find(cur);
            cur = p == par.end() ? std::string() : p->second;
        }
        if (hit) {
            names.push_back(it->first);
            locked.insert(c);
        }
    }
    for (std::map<std::string, bool>::const_iterator it = en.begin(); it != en.end(); ++it) {
        filerw::ELTrackBar* t = dynamic_cast<filerw::ELTrackBar*>(filerw::ELFind(kR119Form, it->first.c_str()));
        if (t && t->Associate && locked.count(t->Associate) && !locked.count(t)) {
            names.push_back(it->first);
            locked.insert(t);
        }
    }
    for (int pass = 0; pass < 2; ++pass)
        for (std::size_t i = 0; i < names.size(); ++i) {
            TControl* c = filerw::ELFind(kR119Form, names[i].c_str());
            filerw::ELTrackBar* tb = dynamic_cast<filerw::ELTrackBar*>(c);
            if ((pass == 0) != (tb != nullptr)) continue;
            Kept k;
            k.name = names[i];
            k.c = c;
            if (tb) k.pos = tb->Position;
            else if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(c)) k.text = x->Text;
            else if (TCheckBox* x = dynamic_cast<TCheckBox*>(c)) k.checked = x->Checked;
            else if (TRadioButton* x = dynamic_cast<TRadioButton*>(c)) k.checked = x->Checked;
            else if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(c)) k.index = x->ItemIndex;
            else if (TComboBox* x = dynamic_cast<TComboBox*>(c)) { k.index = x->ItemIndex; k.text = x->Text; }
            g_kept.push_back(k);
        }
}

void FormShowFlow()
{
    SP_FormShow();     // golden cSpeed.cpp:41 TfSpeed::FormShow
    R119Snapshot();    // (2)(3)
}

bool R119Differs(const Kept& k)
{
    if (filerw::ELTrackBar* x = dynamic_cast<filerw::ELTrackBar*>(k.c)) return (int)x->Position != k.pos;
    if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(k.c)) return x->Text != k.text;
    if (TCheckBox* x = dynamic_cast<TCheckBox*>(k.c)) return x->Checked != k.checked;
    if (TRadioButton* x = dynamic_cast<TRadioButton*>(k.c)) return x->Checked != k.checked;
    if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(k.c)) return x->ItemIndex != k.index;
    if (TComboBox* x = dynamic_cast<TComboBox*>(k.c)) return x->ItemIndex != k.index || x->Text != k.text;
    return false;
}

void R119Restore(const char* after)
{
    if (!g_keptValid) return;
    std::string put;   // 先全部比（放回 TUpDown 會經 Associate 改到 TEdit，放回後再比就看不出 TEdit 被改過）
    for (std::size_t i = 0; i < g_kept.size(); ++i)
        if (R119Differs(g_kept[i])) put += (put.empty() ? "" : ", ") + g_kept[i].name;
    if (put.empty()) return;
    for (std::size_t i = 0; i < g_kept.size(); ++i) {   // g_kept 裡 TTrackBar／TUpDown 在前
        const Kept& k = g_kept[i];
        if (filerw::ELTrackBar* x = dynamic_cast<filerw::ELTrackBar*>(k.c)) x->SetPosition(k.pos, false);   // 不觸發 OnChange（放回，不是使用者拖）
        else if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(k.c)) x->Text = k.text;
        else if (TCheckBox* x = dynamic_cast<TCheckBox*>(k.c)) x->Checked = k.checked;
        else if (TRadioButton* x = dynamic_cast<TRadioButton*>(k.c)) x->Checked = k.checked;
        else if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(k.c)) x->ItemIndex = k.index;
        else if (TComboBox* x = dynamic_cast<TComboBox*>(k.c)) { x->ItemIndex = k.index; x->Text = k.text; }
    }
    if (!put.empty())
        filerw::ELTodo(("[W906] R119=B (Steven 20260928): after golden " + std::string(after) +
                        ", fields locked at page open by golden TfSpeed::FormShow keep their open-time values: " + put).c_str());
}

std::string ExtraJson()
{
    cJSON* o = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "rule", cJSON_CreateString("R119=B"));
    cJSON* a = cJSON_CreateArray();
    for (std::size_t i = 0; i < g_kept.size(); ++i) cJSON_AddItemToArray(a, cJSON_CreateString(g_kept[i].name.c_str()));
    cJSON_AddItemToObject(o, "lockedAtOpen", a);
    cJSON* r = cJSON_CreateArray();
    for (std::size_t i = 0; i < g_lockRoots.size(); ++i) cJSON_AddItemToArray(r, cJSON_CreateString(g_lockRoots[i].c_str()));
    cJSON_AddItemToObject(o, "lockRoots", r);
    char* p = cJSON_PrintUnformatted(o);
    const std::string s = p ? p : "{}";
    if (p) cJSON_free(p);
    cJSON_Delete(o);
    return s;
}

// form.event：RunPageEvent 以 Sender＝ELFind(表單, control) 呼叫（FileRW/_EditPage.cpp RunPageEvent 第 6 步）→ 找回 kSP_Events 那一列
void R119EvKeep(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kSP_Events) / sizeof(kSP_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kR119Form, kSP_Events[i].control) == sender) e = &kSP_Events[i];
    if (!e) throw std::runtime_error("R119: form.event sender is not a TfSpeed event control proxy");
    try {
        e->handler(sender);
    } catch (...) {
        R119Restore(e->golden);
        throw;
    }
    R119Restore(e->golden);
}

const filerw::PageEvent* R119EventTable()
{
    static filerw::PageEvent t[sizeof(kSP_Events) / sizeof(kSP_Events[0])];
    for (std::size_t i = 0; i < sizeof(kSP_Events) / sizeof(kSP_Events[0]); ++i) {
        t[i] = kSP_Events[i];
        t[i].handler = &R119EvKeep;
    }
    return t;
}
}  // namespace

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 6 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfSpeed::FormClose（V912 cSpeed.cpp:1272-1276）：fShow=false; ReadFile(); DoIniDataToForm();
//    （JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）＝產生檔的 SP_FormClose。
//  網頁的 Exit 是 .exitbtn（外框直接關視窗，不送伺服器），✕ 也一樣 ⇒ 頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）
//    呼叫這裡：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過（filerw::PageCloseEdgeRefused）才跑；運轉中不跑（golden ShowModal
//    main.cpp:28695）。golden 存檔鈕沒有 Close()（cSpeed.cpp:1433），存完再關照跑（golden 也是：存檔 → Exit → FormClose）。
//  重讀 <配方>\ArmCondition.Data（A57 by machine 時 sSaveByMachine）進 ArmSpeed_File[]／SHSpeed_File／MGSpeed_File ＝ 丟掉這一次開窗
//    裡沒存的改動；跟開頁 FormShow 的 ReadFile 同一段（CheckAndReadIniData 補缺鍵、MyForceDirectories 照 golden）。
//  主畫面尾段 TfMain::sbSpeedClick :28696-28699 照 R85／S107-1 留在存檔之後（上面 SaveFlow），關窗不再跑。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_Speed_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfSpeed proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("ArmSpeed_File")) return no;
    SP_FormClose();                                                             // golden cSpeed.cpp:1272
    return "ran golden TfSpeed::FormClose (cSpeed.cpp:1272-1276): fShow=false, ReadFile (ArmCondition.Data), DoIniDataToForm";
}
