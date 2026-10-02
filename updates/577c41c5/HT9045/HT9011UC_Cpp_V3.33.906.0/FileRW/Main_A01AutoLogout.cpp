// ===========================================================================
//  FileRW/Main_A01AutoLogout.cpp -- golden [A01] 閒置自動切回 Operator（TfMain::Timer3Timer 的 A01 三段）＋ A01 30 分鐘自動重開。
//
//  AI(W906-D015-A01) 20260930 [W906] St01 新檔。todo D-015。golden 一律照主 repo 那份 V912：
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950），行號都是那一份的。
//
//  golden（TfMain::Timer3Timer main.cpp:25820 起；Timer3 沒寫 Interval ⇒ VCL 預設 1000 ms，main.dfm:17333-17338；
//    FormShow :10613 打開；本體第一關 :25830-25831 `if(InitialOK==false) return;`）⇒ 計數的單位是「秒」：
//    (1) :25957-25982  哪些畫面開著就歸 0（閒置不算）：
//          SPIL／VTEST（IniConfig.bSPILFunction || bVTESTFunction==true）：只看 fiosetview／fSpeed／fObserver(bShow)／fContact；
//          其餘：palSetup／palConfig／fOffSet／fiosetview／fSpeed／fObserver／fContact 任一個開著 ⇒
//            CC_SJ_Semiconductor 永遠不歸 0；fOffSet 開著時只有 CUSTOMER_CODE!=CC_KYEC_LEE 才歸 0；其他情況歸 0。
//    (2) :26022-26078  IniConfig.bA01AutoSwitchToOperatorMode && AccessLevel>=1 ⇒ 每秒 +1；#ifdef SOFT_SIMULTE 立刻歸 0
//          （⇒ 模擬組態 golden 永遠不會自動登出，照留）；>=10 而且 >=IniConfig.iA01ChangeOpTime ⇒ 登出本體 :26040-26071；
//          否則（A01 關或已是 Operator）歸 0。
//    (3) :26080-26096  CosFunction.bAutoOpenConfigA01 && A01 關 ⇒ iOpenA01Count 每秒 +1，>=1800 ⇒ A01=true 並寫
//          AuthPath+"config.ini" [Function] bAutoSwitchToOperatorMode；A01 開著 ⇒ iOpenA01Count=0。
//    (4) cOffSet.cpp:2811  TfOffSet::spbSaveClick 開頭 fMain->iOperatorModeCount=0 ⇒ 本檔 W906_A01ResetOperatorModeCount()
//          （FileRW/Offset_File.gen.inc，設定 tools/editlist/Offset_File.py spbSaveClick 那一條 R1）。
//    兩個計數器 golden 是 TfMain 成員（main.h:1430-1431，建構 :2207-2208 歸 0）；移植樹 forms/fMain.h 沒有 ⇒ 放在本檔（檔內 static＋存取函式）。
//
//  登出本體＝既有的 W906_WebLoginForceOperator（WebLogin.cpp:531，D4 golden :12621-12648），經安裝座 W906_WebLoginForceOperatorHook
//    （LogObjects.cpp 檔尾；WebLogin.cpp 靜態初始化裝上；ctest 可以換成假的）。逐行比過：A01 :26040-26071 跟 D4 :12622-12648 一模一樣，
//    只多兩件 —— :26043-26044 fOffSet->Close()、:26045-26048 palSetup／palConfig 藏起來（見下面 [W906] ②③）。
//    D4 本體自己拿 FormJson 鎖（CRITICAL_SECTION，同一條執行緒可重入）；本檔呼叫它的時候**不**拿鎖（派工要求：不要疊鎖）。
//
//  跟 golden 不同的地方 [W906]
//    ① 節拍：wb_serve 主迴圈 500 ms 的 pumpBeat 呼叫 W906_A01AutoLogoutTick()（tools/wb_serve.cpp:5953，St01 那一行的同一行）；
//       三個阻塞框（告警框／是否框／ShowMyMessage）的等待迴圈每一圈（≤100 ms）也經 W906_ModalWaitTick 呼叫（tools/wb_serve.cpp:7621，
//       St01 D-012 那一行的同一行；AI(W906-D015-A01b) 20260930，Jimmy 20260930 的答覆）⇒ 同 golden：VCL Timer3 在 ShowModal 的訊息迴圈裡
//       照跑，框開著時照樣算秒、時間到照樣登出（golden 的登出本體不等框關掉）。
//       本檔自己用 GetTickCount 截止時間限成每秒一次（同主迴圈 nextPump 的寫法：截止時間每次 +1000，不是「距上次 >=1000」——
//       後者在 500 ms 拍子上會變成 1.0～1.5 秒一次，A01 的秒數會被拉長最多 50%）。第一次呼叫只起算，1 秒後才第一次跑（golden Timer3 同）。
//       主迴圈真的卡住（例：拖曳原生視窗的保活只照抄了部分行）時不補秒：落後超過一秒就從現在重新起算。
//       重入（AI(W906-D015-A01b) 20260930 查過）：本檔的呼叫鏈 —— 頁面表問答（W906_FormShowing）、D4 登出本體 W906_WebLoginForceOperator
//       （DoChangeLevel、NewRecordProcess→MyDBIProcessNew、TemperatureEditDisable）、fOffSet 程式關窗（W906_FormProgramShow 只設頁面表
//       want、印一行；FormClose 在之後主迴圈的 PageTableTick 邊緣才跑）、WriteIniData —— 都不開阻塞框 ⇒ 今天不會從本檔裡面進到
//       W906_ModalWaitTick 再叫回本檔。仍加一道重入保險（g_inTick：巢狀呼叫直接 return、不動截止時間），免得以後登出本體多了一個框
//       （golden D4 的呼叫端 :12649-12655 就有 ShowMyMessage）時同一秒跑兩次。FormJson 鎖是可重入的 CRITICAL_SECTION、都在 tick 執行緒：
//       框若是某個 WS 指令開的（例 editlist.save 的是否框），那條指令已拿著鎖，本檔再拿一次沒事；登出就發生在那條指令中途
//       （golden 同：Timer3 在 MessageDlg 期間照樣登出）。
//    ② palSetup／palConfig（主畫面 Tools／Config 下拉選單，web/page/main.html #palSetup／#palConfig）—— AI(W906-D015-A01b) 20260930：
//       頁面 web/page/ht9045_main_st01_ev.js 用 MutationObserver 看這兩個選單的 style.display（main.html 不改：toggleMenu、選單項、點外面
//       三條路都是改 style.display），開／關時送一條 WS act.main.menuVisible {"setup":bool,"config":bool}（tools/wb_serve.cpp:4826 St01
//       那一行分派 → 本檔 W906_A01MenuVisibleOp）；載入時先送兩個 false；只有失敗才重送（1→10 秒退避），不輪詢、不發 tag。
//       本檔記兩個旗標，(1) 歸 0 那一段照 golden :25965 讀它們。
//       過期保險：頁面表說主畫面不在（W906_FormShowing("fMain", false)＝WebPageTable.cpp 規則 3～7：瀏覽器全關時最後一份總表 ⇒ 立刻；
//       瀏覽器當掉 ⇒ 總表 15 秒過期而且 WebSocket 數是 0 時）⇒ 兩個旗標當作沒開 —— 當掉的瀏覽器不會讓 A01 永遠不登出。
//       收起來：golden V912 ChangeLevelAttr :12928-12935＋:13182-13190（Eastsun 20260526 #026-4.C4／C5）權限從高降到 0 ⇒
//       sbExitSetup->Click()／sbExit->Click() —— 所以 A01（:26042）、Logout 鈕（:28102）、D4（:12624）三條路都會收；A01 本體
//       :26045-26048 再藏一次。移植樹：頁面那一份看 tag auth.level 從 >0 變成 0 就把兩個選單 style.display='none'（接著回報 false）；
//       C++ 這一份在登出本體之後照 :26045-26048 清掉，每一拍開頭也照 ChangeLevelAttr 的「降到 0」清掉（s_iOldAccessLevel）。
//       [W906] 差異：旗標只有一份（最後回報的 HMI 算數；多個 HMI 時其中一個當掉而別的還開著，它最後說的「開著」會留到有人再開關選單
//       或重新載入主畫面 —— WebBridgeServer 沒有逐連線的存活查詢，同 WebWindowRegistry.h 的說明）；auth.level 是訊框粒度，一個訊框內
//       1→0→1 看不到；golden sbExit->Click() 另跑 SetWorkParameter()／UpdateMainOperateMode()（:28440-28446），頁面收 Config 選單時不跑
//       （main.html 自己的 Exit 項也不跑，既有缺口）；權杖照 act.main.* 的規矩要（別的 HMI 拿著時回報被拒、之後重送）；阻塞框開著時回報
//       收到 modal-pending（框關掉後重送成功，這段時間 C++ 用舊狀態）；wb_serve 重開時頁面不補送（選單開著也照數＝本項之前的行為）；
//       golden DoMainPadProcess（:3970-3978，運轉中藏兩個選單）不在本項。
//       從這兩個選單開出去的畫面由頁面表回答（W906_FormShowing）。
//    ③ fOffSet->Close()：TfOffSet 在移植樹沒有 fShow 成員（forms/fOffSet.h）⇒ 開著嗎只問頁面表 W906_FormShowing("fOffSet", false)。
//       關窗走頁面表的「程式關窗」W906_FormProgramShow("fOffSet", false, …)（W906FormShowing.h）：fOffSet 是 kPgBoth 列，
//       WebPageTable.cpp:58 本來就把 golden main.cpp:26043-26044 列為它的程式寫入；每個 HMI 照 want=close 關一次
//       （D:\HT9045\web\background.html applyPageTable）。ctest 裡安裝座是 0 ＝ 什麼都不做。
//    ④ chk_Honprec_Use（:26030-26034，主畫面除錯勾選）：網頁端沒有這個勾選 ⇒ 當作沒勾（同 WebLogin.cpp:287），那一段不做。
//    ⑤ config.ini 的路徑：golden AuthPath+"config.ini"（AuthPath 自己有 W906_AUTH_PATH 縫，common.cpp:139/:168）；另加一道
//       W906_A01CONFIGINI_PATH（沒設或空字串＝golden）讓 ctest 寫自己的沙盒、不碰共用的 machine_config_scratch 副本。
//    ⑥ 觀測用（golden 沒有）：登出前先印一行 [A01]（D4 本體接著印它自己那一行，log 看得出是 A01 觸發的）；自動重開時印一行。
//    ⑦ fSpeed：只問頁面表，成員 TfSpeed::fShow 不傳。網頁 Speed 頁（C 路）開著的旗標是 FileRW/ArmSpeed_File.gen.inc:32 自己的 static fShow，
//       全域 fSpeed（cSpeed.cpp:1695）的 fShow 只有 TfSpeed::FormShow（cSpeed.cpp:132）設，wb_serve 裡不是網頁頁面的狀態；而且一引用 fSpeed
//       就會把 cSpeed.cpp 的目的檔拉進每個連本檔的 ctest（它要 wb_serve 才有的 FileRW_LdUld_Boot／ReadFile，連結失敗）。
//
//  只編進 wb_serve（FileRW/_editlist_sources.cmake；tools/editlist/_integrated.txt 有這個名字，gen_editlist.py:629-631 重產時會保留）。
//  ctest：D015_A01AutoLogout（tests/test_d015_a01_autologout.cpp，SIM／出貨兩種組態都要過）；頁面那一半 D015_A01MenuPage
//  （tools/webprobe/d015_menu_selftest.cjs，node，離線）。
// ===========================================================================
#include "cmydef.h"             // InitialOK（:220）／AccessLevel（:3506）／CUSTOMER_CODE（:3184）；MachineType.h：SOFT_SIMULTE、CC_SJ_Semiconductor、CC_KYEC_LEE
#include "Config.h"             // IniConfig（bSPILFunction／bVTESTFunction／bA01AutoSwitchToOperatorMode／iA01ChangeOpTime，Config.h:133/:252/:272/:275）
#include "CosFunction.h"        // CosFunction.bAutoOpenConfigA01（CosFunction.h:385）
#include "common.h"             // AuthPath（:71）／WriteIniData（:247）
#include "atester_shims.h"      // fContact（TfContactShim::fShow :157）、fiosetview（TfiosetviewShim::fShow :359）
#include "forms/fObserver.h"    // fObserver->bShow（:755）
#include "W906FormShowing.h"    // W906_FormShowing／W906_FormProgramShow（本體 csystem.cpp:30049）
#include "Public/cJSON.h"       // AI(W906-D015-A01b) 20260930：act.main.menuVisible 的 value（ht9045_public）
#include <windows.h>            // GetTickCount
#include <cstdio>
#include <cstdlib>              // getenv
#include <string>

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp（CRITICAL_SECTION，可重入）
extern void (*W906_WebLoginForceOperatorHook)();                                 // LogObjects.cpp 檔尾；本體 WebLogin.cpp W906_WebLoginForceOperator（自己拿 FormJson 鎖）
AnsiString W906_A01ConfigIniPath();                                              // 本檔下面（檔頭 ⑤）

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

int iOperatorModeCount = 0;     // golden TfMain::iOperatorModeCount（main.h:1430；建構 main.cpp:2207 歸 0）
int iOpenA01Count      = 0;     // golden TfMain::iOpenA01Count（main.h:1431；建構 main.cpp:2208 歸 0）

unsigned long (*g_clockForTest)() = 0;   // 0 ＝ GetTickCount
bool          g_armed  = false;          // ① 起算了沒
unsigned long g_next   = 0;              // ① 下一次跑的截止時間
unsigned      g_passes = 0;              // golden Timer3 觸發了幾次（測試用）
bool          g_warnedNoHook = false;

// AI(W906-D015-A01b) 20260930 [W906] ②：golden palSetup->Visible／palConfig->Visible 的 C++ 那一份（頁面最後一次 act.main.menuVisible）。
//   只在 tick 執行緒讀寫（WS 指令在 tick 執行緒 drain；本檔的 tick 也是）⇒ 不用鎖。
bool          g_menuSetup  = false;      // golden palSetup->Visible（Tools ▾）
bool          g_menuConfig = false;      // golden palConfig->Visible（Config ▾）
unsigned      g_menuReports = 0;         // 收到幾次回報（觀測用）
int           s_iOldAccessLevel = -999;  // golden ChangeLevelAttr :12929 static int iOldAccessLevel=-999;
bool          g_inTick     = false;      // 檔頭 ① 重入保險
unsigned      g_reentries  = 0;          // 擋下幾次巢狀呼叫（測試用）

unsigned long NowMs() { return g_clockForTest ? g_clockForTest() : (unsigned long)::GetTickCount(); }

// golden 在同一拍讀的畫面狀態（(1) 與登出本體的 fOffSet->fShow）。先取樣再拿鎖：頁面表的問答不在 FormJson 鎖裡做。
struct Pages {
    bool palSetup, palConfig, offset, iosetview, speed, observer, contact;
};
Pages SamplePages()
{
    Pages p;
    const bool menus = (g_menuSetup || g_menuConfig) &&
                       W906_FormShowing("fMain", false);                        // [W906] ② 過期保險：主畫面不在（頁面表規則 3～7）⇒ 兩個選單當作沒開
    p.palSetup  = menus && g_menuSetup;                                         // golden palSetup->Visible   AI(W906-D015-A01b) 20260930：頁面回報（act.main.menuVisible）
    p.palConfig = menus && g_menuConfig;                                        // golden palConfig->Visible  AI(W906-D015-A01b) 20260930
    p.offset    = W906_FormShowing("fOffSet", false);                           // [W906] ③ TfOffSet 沒有 fShow 成員 ⇒ 只問頁面表
    p.iosetview = W906_FormShowing("fiosetview", fiosetview != 0 && fiosetview->fShow);
    p.speed     = W906_FormShowing("fSpeed", false);                            // [W906] ⑦ 成員不傳（見檔頭）
    p.observer  = W906_FormShowing("fObserver", fObserver != 0 && fObserver->bShow);
    p.contact   = W906_FormShowing("fContact", fContact != 0 && fContact->fShow);
    return p;
}

// golden main.cpp:25957-25982
void StepResets(const Pages& p)
{
    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20190115 SPIL要求在contact頁面不要登出權限
       IniConfig.bVTESTFunction==true)                                          //jou 20211112 : 上海偉測 兵兵 要求offset也要登出權限
    {                                                                           //wei 20151023 矽品開啟offset 登出計數  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        if(p.iosetview || p.speed || p.observer || p.contact)                  // golden fiosetview->fShow || fSpeed->fShow || fObserver->bShow || fContact->fShow
        {
            iOperatorModeCount=0;
        }
    }
    else if(p.palSetup || p.palConfig || p.offset || p.iosetview || p.speed || p.observer || p.contact)
    {
        if(CUSTOMER_CODE==CC_SJ_Semiconductor)                                  //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        {
        }
        else
        {
            if(p.offset)                                                        // golden fOffSet->fShow
            {
                if(CUSTOMER_CODE!=CC_KYEC_LEE)                                  //Ifor 20170803 (Steven) add KYEC Offset 頁面計數
                    iOperatorModeCount=0;
            }
            else
            {
                iOperatorModeCount=0;
            }
        }
    }
}

// golden main.cpp:26022-26078 的計數半邊；回 true ＝ 這一拍要跑登出本體（:26040-26071，由 DoLogout 在鎖外面跑）
bool StepCount()
{
    if(IniConfig.bA01AutoSwitchToOperatorMode && AccessLevel>=1)
    {
        iOperatorModeCount++;

        #if defined(SOFT_SIMULTE) || defined(W906_LOGIN_HONPREC_ALWAYS)   //AI(W906-LOGIN-HONPREC) 20261001: + MachineType.h W906_LOGIN_HONPREC_ALWAYS（EastSun 現場測試）——golden 模擬組態的作法：閒置計數每拍歸零，A01 不自動登出
        iOperatorModeCount=0;
        #endif

        // golden :26030-26034 if(chk_Honprec_Use->Checked==true) { IniConfig.bUseAutoSiteMapping=false; iOperatorModeCount=0; }
        //   [W906] ④ 網頁端沒有 chk_Honprec_Use ⇒ 當作沒勾（同 WebLogin.cpp:287），不做

        if(iOperatorModeCount>=10)
        {
            if(iOperatorModeCount>=IniConfig.iA01ChangeOpTime)                  //Steven 20101109 : [A01]     //kevin 20150206 超過時間變成op模式
                return true;
        }
    }
    else
    {
        iOperatorModeCount=0;                                                   //Steven 20101121
    }
    return false;
}

// golden main.cpp:26040-26071（登出本體）。不在 FormJson 鎖裡：D4 本體自己拿鎖。
void DoLogout(const Pages& p)
{
    std::printf("[A01] idle %d s >= max(10, iA01ChangeOpTime=%d) at AccessLevel=%d -> force back to Operator (golden V912 main.cpp:26036-26072)\n",
                iOperatorModeCount, IniConfig.iA01ChangeOpTime, AccessLevel);   // [W906] ⑥
    std::fflush(stdout);
    // golden :26040-26042 cbUserSelect->ItemIndex=0; AccessLevel=0; ChangeLevelAttr();  :26049-26071 5 Level 標題／MES2140、btLogin "Login"、
    //   TemperatureEditDisable() ＝ D4 本體（WebLogin.cpp:531-563，golden :12622-12648 同一段）
    if(W906_WebLoginForceOperatorHook != 0)
        W906_WebLoginForceOperatorHook();
    else if(g_warnedNoHook == false)
    {
        g_warnedNoHook = true;
        std::printf("[A01] W906_WebLoginForceOperatorHook is not installed (WebLogin.cpp not linked) -- the logout body did NOT run\n");
        std::fflush(stdout);
    }
    if(p.offset==true)                                                          //Ifor 20151027 時間到登出   // golden :26043 if(fOffSet->fShow==true)
        W906_FormProgramShow("fOffSet", false, "main.cpp:26043-26044 A01 idle logout fOffSet->Close()");   // golden :26044 fOffSet->Close();  [W906] ③ 頁面表的程式關窗
    // AI(W906-D015-A01b) 20260930 [W906] ②：C++ 這一份照 golden 藏起來；頁面那一份由 ht9045_main_st01_ev.js 看 auth.level 降到 0 收起
    //   （＝golden ChangeLevelAttr :13183-13189，:26042 已經收過一次）
    if(g_menuSetup)                                                             //JerryYang 20190115 登出權限的時候panel要隱藏   // golden :26045 if(palSetup->Visible)
        g_menuSetup=false;                                                      // golden :26046 palSetup->Visible=false;
    if(g_menuConfig)                                                            // golden :26047 if(palConfig->Visible)
        g_menuConfig=false;                                                     // golden :26048 palConfig->Visible=false;
}

// golden ChangeLevelAttr :12929-12935＋:13183-13189（Eastsun 20260526 #026-4.C4／C5，避免權限登出後停留在工作列上）：
//   權限比上一次低、而且是 0 ⇒ palSetup／palConfig 收起來（sbExitSetup->Click()／sbExit->Click()）。
//   [W906] ② golden 在每次 ChangeLevelAttr 判；這裡每一拍看一次 AccessLevel（同一秒內 1→0→1 看不到）。AI(W906-D015-A01b) 20260930
void MenuLevelDrop()
{
    bool bHasChange=false;
    if(s_iOldAccessLevel>AccessLevel)
        bHasChange=true;
    s_iOldAccessLevel=AccessLevel;
    if(AccessLevel==0 && bHasChange==true)
    {
        if(g_menuSetup)                                                         // golden :13185 if(palSetup->Visible) sbExitSetup->Click();（:29164 palSetup->Visible=false）
            g_menuSetup=false;
        if(g_menuConfig)                                                        // golden :13187 if(palConfig->Visible) sbExit->Click();（:28442 palConfig->Visible=false；
            g_menuConfig=false;                                                 //   :28443-28444 SetWorkParameter／UpdateMainOperateMode 不在本檔，檔頭 ②）
    }
}

// golden main.cpp:26080-26096
void StepReopen()
{
    if(CosFunction.bAutoOpenConfigA01)                                          //Sam 20220929 : Config A01 自動開啟。
    {
        if(IniConfig.bA01AutoSwitchToOperatorMode==false)                       //A01 被關閉後 30分 鐘後自動開啟
        {
            iOpenA01Count++;
            if(iOpenA01Count>=1800)
            {
                AnsiString sPath=W906_A01ConfigIniPath();                       // golden AuthPath+"config.ini"   [W906] ⑤
                IniConfig.bA01AutoSwitchToOperatorMode=true;
                WriteIniData(sPath, "Function", "bAutoSwitchToOperatorMode", IniConfig.bA01AutoSwitchToOperatorMode);
                std::printf("[A01] A01 was off for %d s -> re-opened, %s [Function] bAutoSwitchToOperatorMode=1 (golden V912 main.cpp:26080-26096)\n",
                            iOpenA01Count, sPath.c_str());                      // [W906] ⑥
                std::fflush(stdout);
            }
        }
        else
        {
            iOpenA01Count=0;
        }
    }
}

// 一次 golden Timer3Timer（只含 A01 三段，順序照 golden）；mask 1＝(1) 歸 0、2＝(2) 計數＋登出、4＝(3) 自動重開
void RunSteps(int mask)
{
    MenuLevelDrop();                                                            // AI(W906-D015-A01b) 20260930：golden ChangeLevelAttr 的「降到 0 收選單」（不在 Timer3 裡，不受 InitialOK 管）
    if(InitialOK==false)                                                        // golden Timer3Timer :25830-25831
        return;
    const Pages p = SamplePages();
    bool logout = false;
    {
        FormLockGuard lock;                                                     // AccessLevel／IniConfig 跟 WS 指令、/api/editlist 同一把鎖
        if(mask & 1) StepResets(p);
        if(mask & 2) logout = StepCount();
    }
    if(logout)
        DoLogout(p);
    if(mask & 4)
    {
        FormLockGuard lock;                                                     // IniConfig.bA01AutoSwitchToOperatorMode 寫入＋config.ini
        StepReopen();
    }
}

}  // namespace

// golden main.cpp:26087 AuthPath+"config.ini"（見檔頭 ⑤）
AnsiString W906_A01ConfigIniPath()
{
    const char* e = std::getenv("W906_A01CONFIGINI_PATH");
    if (e && *e) return AnsiString(e);
    return AuthPath+"config.ini";
}

// wb_serve 主迴圈每個 pumpBeat（500 ms）呼叫；這裡限成每秒一次 ＝ golden Timer3（見檔頭 ①）
void W906_A01AutoLogoutTick()
{
    if (g_inTick) { ++g_reentries; return; }                                    // AI(W906-D015-A01b) 20260930：重入保險（檔頭 ①），截止時間不動
    struct InTick { InTick() { g_inTick = true; } ~InTick() { g_inTick = false; } } inTick;
    const unsigned long now = NowMs();
    if (!g_armed) { g_armed = true; g_next = now + 1000ul; return; }            // 起算：1 秒後第一次（golden Timer3 Enabled 之後 1 秒）
    if ((long)(now - g_next) < 0) return;
    g_next += 1000ul;
    if ((long)(now - g_next) >= 0) g_next = now + 1000ul;                       // 落後超過一秒（主迴圈被模態框擋住）：不補秒，從現在重新起算
    ++g_passes;
    RunSteps(1 | 2 | 4);
}

// golden cOffSet.cpp:2811 TfOffSet::spbSaveClick：fMain->iOperatorModeCount=0;   //Ifor 20151027 時間到登出
void W906_A01ResetOperatorModeCount() { iOperatorModeCount = 0; }

int W906_A01OperatorModeCount() { return iOperatorModeCount; }
int W906_A01OpenCount()         { return iOpenA01Count; }

// ===========================================================================
//  AI(W906-D015-A01b) 20260930 [W906]（St01）：WS act.main.menuVisible —— 主畫面 Tools ▾／Config ▾ 下拉選單（golden palSetup／palConfig）
//    開著沒有（檔頭 ②）。value＝JSON 字串 {"setup":bool,"config":bool}（兩個都要、都要是 bool，否則 bad-payload、旗標不動）。
//    入口：tools/wb_serve.cpp:4826 St01 那一行的分派（act.main.runMode 那一支前面）；頁面 web/page/ht9045_main_st01_ev.js。
//    只記狀態、不動機台；權杖／防連點照 act.main.* 的規矩（WebBridgeServer 權杖、WebCmdGuard 400 ms）。
//    回覆 {"executed":true,"setup":b,"config":b,"reports":n,"golden":"..."}。
// ===========================================================================
std::string W906_A01MenuVisibleOp(const std::string& payloadJson, bool* ok)
{
    cJSON* root = payloadJson.empty() ? 0 : cJSON_Parse(payloadJson.c_str());
    const cJSON* s = (root && cJSON_IsObject(root)) ? cJSON_GetObjectItemCaseSensitive(root, "setup")  : 0;
    const cJSON* c = (root && cJSON_IsObject(root)) ? cJSON_GetObjectItemCaseSensitive(root, "config") : 0;
    if (!s || !c || !cJSON_IsBool(s) || !cJSON_IsBool(c)) {
        if (root) cJSON_Delete(root);
        if (ok) *ok = false;
        return "bad-payload: act.main.menuVisible needs value={\"setup\":true|false,\"config\":true|false} (golden palSetup->Visible / palConfig->Visible, V912 main.cpp:25965)";
    }
    const bool setup  = cJSON_IsTrue(s) != 0;
    const bool config = cJSON_IsTrue(c) != 0;
    cJSON_Delete(root);
    ++g_menuReports;
    if (setup != g_menuSetup || config != g_menuConfig) {
        std::printf("[A01] main-screen menus (golden palSetup / palConfig, V912 main.cpp:25965): Tools %d -> %d, Config %d -> %d (act.main.menuVisible #%u)\n",
                    (int)g_menuSetup, (int)setup, (int)g_menuConfig, (int)config, g_menuReports);
        std::fflush(stdout);
    }
    g_menuSetup  = setup;
    g_menuConfig = config;
    if (ok) *ok = true;
    char b[256];
    std::snprintf(b, sizeof(b), "{\"executed\":true,\"setup\":%s,\"config\":%s,\"reports\":%u,\"golden\":\"V912 main.cpp:25965 Timer3Timer palSetup->Visible || palConfig->Visible\"}",
                  setup ? "true" : "false", config ? "true" : "false", g_menuReports);
    return b;
}

// ---- 測試用（⛔ 正常路徑不要呼叫）----
void     W906_A01SetClockForTest(unsigned long (*nowMs)()) { g_clockForTest = nowMs; g_armed = false; g_next = 0; }
void     W906_A01SetCountsForTest(int operatorMode, int openA01) { iOperatorModeCount = operatorMode; iOpenA01Count = openA01; }
void     W906_A01RunStepsForTest(int mask) { RunSteps(mask); }
unsigned W906_A01PassesForTest() { return g_passes; }
int      W906_A01MenuStateForTest() { return (g_menuSetup ? 1 : 0) | (g_menuConfig ? 2 : 0); }                          // AI(W906-D015-A01b) 20260930：1＝Tools、2＝Config
void     W906_A01ResetMenuForTest() { g_menuSetup = false; g_menuConfig = false; s_iOldAccessLevel = -999; }          // AI(W906-D015-A01b) 20260930
unsigned W906_A01ReentriesForTest() { return g_reentries; }                                                            // AI(W906-D015-A01b) 20260930
