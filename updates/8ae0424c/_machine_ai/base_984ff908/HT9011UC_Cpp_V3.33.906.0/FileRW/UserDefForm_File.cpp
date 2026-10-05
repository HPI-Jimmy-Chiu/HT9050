// ===========================================================================
//  FileRW/UserDefForm_File.cpp -- 結構 UserDefForm_File 的讀寫檔（A＋C：elTrayForm＋逐鍵 → <recipe>\Tray.Data）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（UserDefForm_File）。
//
//  golden TfTrayForm（cTrayForm.cpp）由 tools/gen_editlist.py 轉成 UserDefForm_File.gen.inc：
//    建構子（:28）＝ elTrayForm 的註冊；FormShow（:141）＝開頁（ReadFile＋DoIniDataToForm＋權限）；
//    spbSaveClick（:610）＝存檔鈕（A02 守衛 → SaveSetupFile（elTrayForm＋Name／Memo／Bin Box…逐鍵）→ ReadFile → SECS）。
//  移植樹其他程式還在用 fTrayForm 的真元件（uPAT_Function、csystem／fBuilder 的 fTrayForm->SaveSetupFile），
//  所以名稱型別對得上的元件「收養」移植樹那個物件（TF_AdoptPortWidgets），兩邊共用同一份值。
//  本檔只放開機與 C 路頁面描述（FileRW/_EditPage.h）。
//
//  開機：golden HT9045.cpp:192 CreateForm(TfTrayForm)。tools/wb_serve.cpp 的 tray.* 讀檔鏈原本呼叫
//  fTrayForm->Init()（移植樹以真元件註冊）＋ fTrayForm->ReadFile()，改呼叫本檔的
//  FileRW_TrayForm_Boot()＋FileRW_TrayForm_ReadFile()（golden 建構子與 ReadFile，元件有名稱）。
// ===========================================================================
#include "FileRW/UserDefForm_File.gen.inc"

#include <cstdio>
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbTrayFormClickTail（FileRW/MainClick.cpp；Q41 第 3 項 TF-5）；佔用原本的空行
#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elTrayForm};
const char* const kListNames[] = {"elTrayForm"};
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbTrayFormClick 關窗尾段，R85）；佔用原本的空行
const filerw::PageDesc kPage = {
    "UserDefForm_File", "TfTrayForm", "Setup.TrayForm.html",
    kLists, kListNames, 1,
    kTF_SaveReads, (int)(sizeof(kTF_SaveReads) / sizeof(kTF_SaveReads[0])),
    &TF_FormShow, &SaveFlow, "SaveSetupFile", &TF_ReadFile, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &TF_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；同一行改寫
};
filerw::PageRegistrar g_reg(&kPage);
// AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157）—— 頁面選 cbTrayType1／2／3 一筆
//   → golden cbTrayType1Change（cTrayForm.cpp:570）→ ShowTypePage(Tag, ItemIndex)（:580，填那一型的 10 個欄位）。
//   表 kTF_Events 由 tools/gen_editlist.py 產生（UserDefForm_File.gen.inc 檔尾），本體 FileRW/_EditPage.cpp RunPageEvent。
filerw::PageEventsRegistrar g_evreg("UserDefForm_File", kTF_Events, (int)(sizeof(kTF_Events) / sizeof(kTF_Events[0])));
}  // namespace
static void TF_EvBootProxies();   //AI(W906-EVB3) 20260928 [W906]：TF-4 form.event 控制項缺的替身（btnBinBoxReset），定義在檔尾；佔用原本的空行
// golden TfTrayForm 建構（HT9045.cpp:192）
void FileRW_TrayForm_Boot()
{
    if (g_booted) return;
    if (!elTrayForm) elTrayForm = new HTEditList;                               // golden main.cpp:1487（TfMain 建構子）
    TF_AdoptPortWidgets();
    TF_DfmItems();
    TF_DfmState();
    TF_TfTrayForm();
    TF_CreateSaveProxies();
    TF_CreateContainerProxies(); TF_EvBootProxies();   //AI(W906-EVB3) 20260928 [W906]：檔尾（TF-4 按鈕替身＋DFM 父層）；同一行附加
    std::printf("FileRW UserDefForm_File: elTrayForm registered by name (%d entries) -- golden TfTrayForm ctor cTrayForm.cpp:28\n",
                elTrayForm->FEditList->Count);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:8898 fTrayForm->ReadFile()
void FileRW_TrayForm_ReadFile()
{
    if (g_booted) TF_ReadFile();
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 TF-5（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 spbSaveClick（cTrayForm.cpp:610）跑完之後，補 golden 主畫面
//    TfMain::sbTrayFormClick 的關窗尾段（V912 main.cpp:28411-28413：DoStructUnitConvert、SetWorkParameter；:28413 ShowTrayDeviceDir
//    是主畫面方向圖、不翻；本體 FileRW/MainClick.cpp W906_Main_sbTrayFormClickTail，運轉中不跑 R86）。照 FileRW/Ld_UldDelayTime.cpp 的做法：
//      (1) A02 權限不足（"closed"）—— golden :616 Close() → FormClose（cTrayForm.cpp:559：ReadFile、DoIniDataToForm、fShow=false）
//          → ShowModal 回來 → 尾段：與 golden 相同。
//      (2) 真的寫了檔（"SaveSetupFile"）—— golden 存完視窗還開著、等 Exit 才跑 FormClose＋尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//          FormClose 不跑（頁面還開著；存檔鈕最後已 ReadFile，cTrayForm.cpp:631）。
//    差別：golden「開頁、沒存就關」也跑尾段；這裡不跑（值沒變，結果相同）。
//    Q40 的事件表（上面 g_evreg，FileRW/_EditPage.cpp RunPageEvent）不經 saveFlow，不受影響。
//    描述檔 tools/editlist/UserDefForm_File.py 不改、gen.inc 不重產。
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    TF_spbSaveClick();                                                          // golden cTrayForm.cpp:610
    if (filerw::ELMarked("closed")) {
        TF_FormClose();                                                         // golden cTrayForm.cpp:559
        if (const char* w = W906_Main_sbTrayFormClickTail()) filerw::ELTodo(w);   // golden main.cpp:28411-28413
    } else if (filerw::ELMarked("SaveSetupFile")) {
        if (const char* w = W906_Main_sbTrayFormClickTail()) filerw::ELTodo(w);   // golden main.cpp:28411-28413（時機見上）
    }
}
}  // namespace

// ---------------------------------------------------------------------------
//  //AI(W906-EVB3) 20260928 [W906]：TF-4（事件移植批次 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B3；
//    Steven 20260928「任何畫面的事件, 都是我們做」「如果沒有移植的, 我們直接實作」）—— Bin Box 分頁的「Reset」鈕：
//    WS form.event {control:"btnBinBoxReset", event:"click"} → golden btnBinBoxResetClick（cTrayForm.cpp:729-734）：
//    edtBinBoxNow 顯示 0、LastSet.iBinBoxCount=0、MOT[MManualTray3].ClearTray（Fix3 盤的格子清空，Motor/mymotor.cpp:2352；只改記憶體、
//    記一行 MNetLog，不動馬達）。處理器由產生器轉（tools/editlist/UserDefForm_File.py，kTF_Events 已含這一列，上面 g_evreg 照舊註冊）。
//    按鈕替身不在產生器的 names_used（處理器本體沒提到它）⇒ 這裡開機補建並接上 DFM 父層 pnlBixBox（cTrayForm.dfm:11653 在 pnlBixBox 裡；
//    同 FileRW/ArmSpeed_File.cpp R119BootProxies，d7fa099a），filerw::ELOperable 才看得到 pnlBixBox（Visible＝IniConfig.bBinBox，golden FormShow :180）
//    與 tsBinBox（TabVisible＝bBinBox||bShowTrayAndDeviceDir，:177）。
//    ⚠ 機台狀態：ClearTray 清的是 Fix3 盤的格子資料（盤上實際有料時 golden 也照清，操作員責任）；運轉中 form.event 一律回 running（_FormEvent.cpp）。
//    按下即生效、不寫檔（golden 同：LastSet.iBinBoxCount 由 lastdata.dat 的週期寫檔帶走）；存檔補點做不到（按鈕沒有值）。
//    頁面送出點：Setup.TrayForm.html 是 B2 的檔，寫法見交件（B3 lane 2 報告）。
// ---------------------------------------------------------------------------
static void TF_EvBootProxies()
{
    EL<TButton>("TfTrayForm", "btnBinBoxReset");                              // golden cTrayForm.h:144 TButton（dfm:11653）
    static const char* const kEvParents[][2] = {{"btnBinBoxReset", "pnlBixBox"}};
    filerw::ELSetParents("TfTrayForm", kEvParents, (int)(sizeof(kEvParents) / sizeof(kEvParents[0])));
    for (std::size_t i = 0; i < sizeof(kTF_Events) / sizeof(kTF_Events[0]); ++i)
        if (!filerw::ELFind("TfTrayForm", kTF_Events[i].control))
            std::printf("FileRW UserDefForm_File: WARNING form.event control %s has no proxy (add it to TF_EvBootProxies)\n",
                        kTF_Events[i].control);
}

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 8 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfTrayForm::FormClose（V912 cTrayForm.cpp:559-568）：ReadFile(); DoIniDataToForm(); fShow=false;
//    ＝產生檔的 TF_FormClose。重讀 <配方>\Tray.Data（elTrayForm＋UserDefForm_File[] 的 Alias／Memo／Bin Box／方向）＝丟掉這一次開窗裡
//    沒存的改動（例 form.event cbTrayType1Change 換過的型別頁、btnBinBoxReset 之外的欄位）。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused；A02 的 Close() 上面 SaveFlow 已經跑過 TF_FormClose ⇒ 不跑第二次）才跑；運轉中不跑（ShowModal main.cpp:28409）。
//  主畫面尾段 sbTrayFormClick :28411-28413 照 R85／S107-1 留在存檔之後（上面 SaveFlow），關窗不再跑。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_TrayForm_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfTrayForm proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("UserDefForm_File")) return no;
    TF_FormClose();                                                             // golden cTrayForm.cpp:559
    return "ran golden TfTrayForm::FormClose (cTrayForm.cpp:559-568): ReadFile (Tray.Data), DoIniDataToForm, fShow=false";
}
