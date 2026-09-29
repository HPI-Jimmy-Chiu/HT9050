// ===========================================================================
//  FileRW/TestIF_File_VacuumUnit.cpp -- golden TfVacuumUnit（VacuumUnit\VacuumUnit.cpp，V912）的 C 路入口（C 形狀：HTEditList elVacuumUnit
//  → <recipe>\HandlerCondition.Data [Vacuum Threshold]／[Tray]）。頁面：web/page/HW.VacuumUnit.html（頁面補件 web/page/ht9045_vacuumunit_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_VacuumUnit.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_VacuumUnit.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    Initial（:32）＝ elVacuumUnit 的註冊（面板 edSV 34 筆：HT9045 4 欄；HT9046／HT9046LS／HT1032／USE_46_SUCKER_DB 8 欄 → 50 筆）；
//    FormShow（:210）＝開頁（ReadFile → SetPanelPos → ShowSuckMode 依 FTestSuck 顯示／隱藏 Index 面板）；
//    spbSaveClick（:378）＝存檔鈕（A02 守衛 → SaveSetupFile → elVacuumUnit->SaveEditTextToFile → BackupSetupFile → ReadFile → SECS）。
//  mustSend＝0：存檔流程讀的只有 elVacuumUnit 登記的元件，頁面沒送的沿用開頁時從檔案讀到的值。
//  reload＝golden FormClose（:304，ReadFile＋DoIniDataToForm）：golden A02 路徑 Close() 之後跑的就是它。
//
//  ---- 開機與換配方（golden 呼叫點，D:\HT9045 V912 行號）-----------------------------------------------------
//    main.cpp:1542  TfMain 建構子  elVacuumUnit=new HTEditList        → 移植樹 FileRW_IniConfig_Boot 已做（本檔再防一次 NULL）
//    main.cpp:10933 TfMain::FormShow fVacuumUnit->Initial()          → FileRW_VacuumUnit_Boot()（註冊；不讀檔）
//    main.cpp:10934 TfMain::FormShow SetIOTableByECAT_VC8_Sucker()   → 不在本檔（TMySucker 的 VC8 位址複製到面板；移植樹 VacuumUnit.cpp GATE 1）
//    ReadFile：golden 只在 FormShow（:214）、FormClose（:307）、spbSaveClick（:394）呼叫。DoReadLastData（main.cpp:9264-9420，
//      開程式 :9995、換工作檔 cbSetupFileNameChange :25724／:25772）**沒有** fVacuumUnit->ReadFile()；全 golden 樹
//      `grep -rn "fVacuumUnit\|elVacuumUnit"`（20260925）只有上面三個 main.cpp 呼叫點＋ delete（:12555）＋ Show（:35560）。
//    ⇒ 照 V912：開機只註冊、不讀檔；換配方不讀。FileRW_VacuumUnit_ReadFile() 給整合者／Steven 決定要不要偏離 golden 開機讀。
//  ---- 讀檔之後誰用這些值 ----------------------------------------------------------------------------------
//    TestIF_File.iVaccumThrdIndexArm1／IndexArm2／InArm／OutArm（cprod.h:2564-2567）：全 golden 樹除了 VacuumUnit.cpp:121-133 的
//    elVacuumUnit->Add 之外 0 個讀者（grep "iVaccumThrd"，20260925）；也沒有 TestIF=TestIF_File 之後的讀者。
//    真空判斷用的是 ECAT-VC8 模組裡的閥值：只有 btnSV（MyVacuumPanel.cpp:385 WriteVaccumThreshold(atof(edSV->Text))）與
//    Set All（VacuumUnit.cpp:551 btnSetInArmClick）會經 MyLaneIO.SetIOValueThread 寫進硬體 —— 開機／存檔都不寫。
//    ⇒ 開始讀檔（或開機註冊）不改變任何執行期行為；頁面的 Set／Set All／吸破真空鈕是硬體指令，網頁停用（ht9045_vacuumunit_c.js）。
//
//  ⚠ 移植樹 VacuumUnit/VacuumUnit.cpp 的 TfVacuumUnit::Initial() 以真元件註冊（頁面拿不到名稱）；**不要**再呼叫它，
//    否則 elVacuumUnit 同一組鍵會有兩筆（同 FileRW/Ld_UldDelayTime.cpp 的 fLd_ULd->Init() 說明）。
// ===========================================================================
#include "FileRW/TestIF_File_VacuumUnit.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elVacuumUnit};
const char* const kListNames[] = {"elVacuumUnit"};

const filerw::PageDesc kPage = {
    "TestIF_File_VacuumUnit", "TfVacuumUnit", "HW.VacuumUnit.html",
    kLists, kListNames, 1,
    nullptr, 0,                      // golden 存檔流程只讀 elVacuumUnit 登記的元件 → 沒有 mustSend
    &VU_FormShow, &VU_spbSaveClick, "SaveSetupFile", &VU_FormClose, &Booted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden main.cpp:10933 TfMain::FormShow 的 fVacuumUnit->Initial()（前提：golden HT9045.cpp:275 CreateForm(TfVacuumUnit) ＝
// DFM 設計期狀態＋建構子 :26 iCount=5；main.cpp:1542 elVacuumUnit=new HTEditList）。不讀檔（golden 開機不讀，見檔頭）。
// 要在 SetMyKitSuckItemAmount 之後、第一次 editlist.get 之前（開頁 ShowSuckMode 讀 FTestSuck）。冪等。
void FileRW_VacuumUnit_Boot()
{
    if (g_booted) return;
    if (!elVacuumUnit) elVacuumUnit = new HTEditList;                           // golden main.cpp:1542（FileRW_IniConfig_Boot 通常已建）
    VU_DfmItems();
    VU_DfmState();
    VU_TfVacuumUnit();
    VU_Initial();
    VU_CreateSaveProxies();
    VU_CreateContainerProxies();
    // 偏離（port-only）：Set All Value 的三個輸入框只給 btnSetInArmClick（:551，寫 ECAT-VC8 硬體閥值）用，那顆鈕網頁不做
    // （指令通道未設計）→ 標唯讀，頁面停用、存檔不收。golden DFM 沒有 ReadOnly；它們的替身是產生器從 FormShow :248-252
    // 那段 golden 註解裡掃到才建的，不在任何清單、存檔流程也不讀。
    for (const char* n : {"edSetInArm", "edSetIndexArm", "edSetOutArm"}) filerw::ELSetReadOnly("TfVacuumUnit", n);
    std::printf("FileRW TestIF_File_VacuumUnit: elVacuumUnit registered by name (%d entries, index cols %d, in/out cols %d) -- golden VacuumUnit.cpp Initial :32\n",
                elVacuumUnit->FEditList->Count, iIndexColMax, iInOutColMax);
    g_booted = true;
}

// golden TfVacuumUnit::ReadFile（:351）。golden 開機／換配方都沒有呼叫（見檔頭）——給整合者選用，預設不接。
void FileRW_VacuumUnit_ReadFile()
{
    if (g_booted) VU_ReadFile();
}

#undef FTestSuck
