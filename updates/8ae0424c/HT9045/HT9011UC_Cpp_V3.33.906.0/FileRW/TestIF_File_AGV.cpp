// ===========================================================================
//  FileRW/TestIF_File_AGV.cpp -- TestIF_File 的 TfAGV 半邊（D:\HT9045\config\AGV.ini [Configuration]）讀寫檔，C 形狀。
//  頁面：web/page/Setup.AGV.html（頁面補件 web/page/ht9045_agv_c.js）。
//
//  AI(W906-B8-AG1) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//    設定：tools/editlist/TestIF_File_AGV.py（每條 replace 附原因）→ tools/gen_editlist.py --only TestIF_File_AGV → TestIF_File_AGV.gen.inc。
//
//  golden TfAGV（V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp）：
//    FormShow（:1304）＝開頁：ReadFile（:1227，AGV.ini 26 鍵 → TestIF_File）→ DoIniDataToForm（:1264）→ fShow=true。
//    spbSaveClick（:1184）＝存檔鈕：26 個 WriteIniData（:1189-1221）→ ReadFile（:1223）→ spbSave->Down=false。golden 不問、不查權限
//      ⇒ 存檔流程讀的 26 個替身全部 mustSend；savedMark＝"spbSaveClick"（golden 進來就無條件寫）。
//    reload＝DoIniDataToForm（替身放回記憶體 TestIF_File 的值，不讀檔）：只有 PageSave 在跑 golden 之前就回 400 時用得到
//      （golden 沒有「沒寫檔」的路徑）。golden FormClose（:1311）只有 fShow=false，不能當 reload。
//  開窗閘（FileRW/_EditPage.cpp kOpenGates "TestIF_File_AGV"）：golden spbAGV 在工具選單 palSetup（main.dfm），看得見＝USE_E84_Sensor
//    （V912 main.cpp:24333 SBPtr[] {spbAGV, USE_E84_Sensor}）；spbAGVClick（:35806-35811）自己不查。看得見的判斷經 W906_AgvGateVisible
//    （本檔開機裝上），_EditPage.cpp 因此不必連 USE_E84_Sensor（test_openenter_log 等單獨連 _EditPage.cpp）。
//    開頁記 golden :35809 NewRecordProcess("MES2198", "Enter AGV Form")（_EditPage.cpp kOpenEnters）。
//
//  ⚠⚠ 這張頁會讓 E84 交握開始跑（golden 行為，照做；Steven 請看）：
//    讀檔的是 golden ReadFile（開頁 FormShow、存檔 spbSaveClick 都會跑），它把 AGV.ini [Configuration] "E84 Enable" 讀進
//    TestIF_File.bEnableE84。讀者：移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30394
//      if(USE_E84_Sensor==1 && TestIF_File.bEnableE84)      （MainProc，每一拍；USE_E84_Sensor＝Gerneral.ini [System] AGVModal，database.cpp:1655）
//    成立時每一拍跑 DoE84LoaderScan(i)／DoE84UnloaderScan(i)（i=0..2，Automation/AGV_PortScan.cpp）、E84StatusChange、DoE84Loader、DoE84Unloader
//    （Automation/AGV_E84.cpp:209／:598，jimmychiu 的檔）＝E84 交握狀態機：讀 Sen[SnE84_1_*]／Sen[SnE84_2_*]（VALID、CS_0、CS_1、AM_AVBL、TR_REQ、
//    BUSY、COMPT、CONT、GO），開關 SW[SwE84_1_*]／SW[SwE84_2_*]（L_REQ、U_REQ、READY 開關；HO_AVBL、ES 打開；VA、VS_0、VS_1 關掉；POWER 不碰）
//    ＝對 AGV／AMR 的交握輸出，逾時用 iE84TimeOut_K12（也是這裡讀的）。
//    ⇒ AGVModal=1 而且 AGV.ini "E84 Enable"=1 的機台：網頁第一次開 Setup.AGV（或存一次檔）之後，E84 交握就開始跑 —— 跟 golden 開過 AGV
//      畫面一樣。AGVModal=0（Steven01 開發機的 Gerneral.ini）不會跑，開窗閘也照 golden 不讓這一頁開（spbAGV 看不見）。
//    golden 開機也讀一次（main.cpp:9378 fAGV->ReadFile()，TfMain::DoReadLastData）⇒ 開機就開始跑 —— 本檔檔尾 FileRW_AGV_ReadFile（patch B）。
//
//  測試縫：W906_AgvIniPath()（golden 寫死的 "D:\\HT9045\\config\\AGV.ini"，:1187、:1230）——環境變數 W906_AGVINI_PATH 沒設或是空字串＝golden 字面值。
//    tests/CMakeLists.txt 用 _ht9045_env_extra 給每一支 ctest 一個沙盒路徑（ctest 一律不碰真的 AGV.ini）。
// ===========================================================================
#include "FileRW/TestIF_File_AGV.gen.inc"

#include <cstdio>
#include <cstdlib>

#include "FileRW/_EditPage.h"

// golden AGV.cpp:1187／:1230 的 "D:\\HT9045\\config\\AGV.ini"（見檔頭「測試縫」）
AnsiString W906_AgvIniPath()
{
    const char* p = std::getenv("W906_AGVINI_PATH");
    if (p && *p) return AnsiString(p);
    return AnsiString("D:\\HT9045\\config\\AGV.ini");
}

extern int USE_E84_Sensor;                  // cmydef.h:2901（Gerneral.ini [System] AGVModal，database.cpp:1655）
namespace filerw { extern bool (*W906_AgvGateVisible)(); }   // FileRW/_EditPage.cpp（開窗閘 GAgv；見檔頭）

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
bool AgvButtonVisible() { return USE_E84_Sensor != 0; }   // golden V912 main.cpp:24333 {spbAGV, USE_E84_Sensor}

const filerw::PageDesc kPage = {
    "TestIF_File_AGV", "TfAGV", "Setup.AGV.html",
    nullptr, nullptr, 0,
    kAG_SaveReads, (int)(sizeof(kAG_SaveReads) / sizeof(kAG_SaveReads[0])),
    &AG_FormShow, &AG_spbSaveClick, "spbSaveClick", &AG_DoIniDataToForm, &Booted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

static void AG_EvBootProxies();   // AI(W906-B8-AG1) 20260930 patch B：form.event 兩顆鈕的替身＋DFM 父層（檔尾）

// golden TfAGV 建構（HT9045.cpp:278 CreateForm，TfVacuumUnit :275 之後）：DFM 設計期狀態 → 存檔流程讀的替身 → 容器替身與父子。冪等。
// 不讀檔（golden 的讀檔在 FormShow 與 DoReadLastData main.cpp:9378，見檔頭）；建構子 :30-34 只設 ATK 旗標，不轉（tools/editlist/TestIF_File_AGV.py）。
void FileRW_AGV_Boot()
{
    if (g_booted) return;
    AG_DfmItems();
    AG_DfmState();
    AG_CreateSaveProxies();
    AG_CreateContainerProxies();
    AG_EvBootProxies();                                   // AI(W906-B8-AG1) 20260930 patch B（檔尾）
    filerw::W906_AgvGateVisible = &AgvButtonVisible;
    std::printf("FileRW TestIF_File_AGV: TfAGV proxies ready (%d save reads) -- golden Automation/AGV.cpp; AGV.ini = %s\n",
                (int)(sizeof(kAG_SaveReads) / sizeof(kAG_SaveReads[0])), W906_AgvIniPath().c_str());
    g_booted = true;
}

// ===========================================================================
//  AI(W906-B8-AG1) 20260930 [W906] St01 patch B（**Steven 請看：這一段會動真機的 E84 交握輸出**）
//
//  (1) 開機／換配方讀檔：golden TfMain::DoReadLastData（V912 main.cpp:9264）:9378 fAGV->ReadFile()（RogerYang 20260825：「AGV.ini 原本只在
//      TfAGV::FormShow 讀, 沒開過 AGV 畫面時 bEnableE84 與各 TP/TA 逾時全是 0」）。呼叫端 tools/wb_serve.cpp W906_DoReadLastData 的 FrmAOI
//      那一行（golden :9376 之後），開機（bBoot）與換配方都跑（golden DoReadLastData 兩種都跑）。
//      ⚠ 這就是 E84 的開機開關：讀進 TestIF_File.bEnableE84 之後，移植樹 csystem.cpp:30394 `if(USE_E84_Sensor==1 && TestIF_File.bEnableE84)`
//      每一拍跑 E84 交握（檔頭）。AGVModal=1、AGV.ini "E84 Enable"=1 的機台：開機就開始開關 SW[SwE84_1_*]／SW[SwE84_2_*]，不必先開網頁。
//      AGVModal=0 的機台讀了也不跑。只讀（golden ReadIniData，檔不在＝預設值、不建檔）。
//  (2) Initial Load／Initial Unload 兩顆鈕（WS form.event，tag "Setup.AGV" 或 "TestIF_File_AGV"）：產生檔 AG_btInitalLoadClick／
//      AG_btInitalUnLoadClick＝golden :1316-1322／:1324-1330 原樣：
//        Load   → InitialE84LoadTask()（iE84LoadTask=1）、InitialE84LoadSensor()（SW[SwE84_1_LREQ／UREQ／VA／READY／VS0／VS1].Off()）、
//                 bE84LoaderActionflag[0..2]=false
//        Unload → InitialE84UnLoaderTask()（iE84UnloadTask=1）、InitialE84UnloadSensor()（SW[SwE84_2_* 同 6 顆].Off()）、
//                 bE84UnloaderActionflag[0..2]=false
//      （本體 Automation/AGV_E84.cpp:165-207，jimmychiu 的檔，只呼叫。）按下當下就關 6 顆交握輸出、把狀態機拉回第 1 步、放掉軌道互鎖旗標；
//      不動軸。golden 兩顆鈕沒有任何檢查（不查 SystemStart、不查交握進行中）；C++ 端的重查＝既有的 form.event 規則：
//        運轉中（SystemStart||SoftStart）回 running（FileRW/_FormEvent.cpp，RULINGS_20260927 第 2 條第 7 題 A）、要先在同一個 AccessLevel 開過頁
//        （開頁時過了開窗閘：工具選單＋spbAGV 看得見）、鈕與父層看得見也可用（ELOperable）。
//      ⚠ 偏離（比 golden 嚴）：golden 是非模態 fAGV->Show()（main.cpp:35780），畫面開著時運轉中也按得到（AMR 卡在埠口時就是這樣救）；
//        網頁運轉中送不進來。同 B8 OS-5 的寫法，交 ST01-E／Steven。⛔ 20260930 更正 //AI(W906-FE-RUNEXC)：已開例外——FileRW/_FormEvent.cpp 檔尾的運轉中例外表列了這兩顆，SystemStart／SoftStart 時照 golden 放行（頁面表要說 fAGV 開著）；本檔不用改。
//      ⚠ 沒改（筆電的檔）：Automation/AGV_E84.cpp 的 case 5000 自動 recover 直接呼叫兩支 Initial*，沒有 V912 這裡新增的 bE84*Actionflag 清除
//        （那一份是照 V906_0618 翻的，AGV_E84.cpp:584-596／:947-957）；:926-937 的 YES／NO 閘仍是 ret=2。
// ===========================================================================
// golden fAGV->ReadFile()（main.cpp:9378）——開機讀檔鏈用，不動 filerw session（golden ReadFile 沒有訊息／待辦）。
void FileRW_AGV_ReadFile()
{
    FileRW_AGV_Boot();
    AG_ReadFile();
    std::printf("FileRW TestIF_File_AGV: golden DoReadLastData main.cpp:9378 fAGV->ReadFile() -- %s: E84 Enable=%d (USE_E84_Sensor=%d -> E84 handshake %s)\n",
                W906_AgvIniPath().c_str(), (int)TestIF_File.bEnableE84, USE_E84_Sensor,
                (USE_E84_Sensor == 1 && TestIF_File.bEnableE84) ? "RUNS every MainProc tick (csystem.cpp:30394)" : "off");
}

// form.event 的兩顆鈕：golden 方法本體沒提到它們 ⇒ 產生器沒建替身、沒接父層（route-c-golden-bridge.md §3.0g-11「開機補替身」）
static void AG_EvBootProxies()
{
    EL<TButton>("TfAGV", "btInitalLoad");                                        // golden AGV.h:71（AGV.dfm:642，Panel3 裡）
    EL<TButton>("TfAGV", "btInitalUnLoad");                                      // golden AGV.h:72（AGV.dfm:651，Panel3 裡）
    static const char* const kEvParents[][2] = {{"btInitalLoad", "Panel3"}, {"btInitalUnLoad", "Panel3"}};
    filerw::ELSetParents("TfAGV", kEvParents, (int)(sizeof(kEvParents) / sizeof(kEvParents[0])));
    for (std::size_t i = 0; i < sizeof(kAG_Events) / sizeof(kAG_Events[0]); ++i)
        if (!filerw::ELFind("TfAGV", kAG_Events[i].control))
            std::printf("FileRW TestIF_File_AGV: WARNING form.event control %s has no proxy (add it to AG_EvBootProxies)\n", kAG_Events[i].control);
}
namespace {
filerw::PageEventsRegistrar g_evreg("TestIF_File_AGV", kAG_Events, (int)(sizeof(kAG_Events) / sizeof(kAG_Events[0])));
}  // namespace
