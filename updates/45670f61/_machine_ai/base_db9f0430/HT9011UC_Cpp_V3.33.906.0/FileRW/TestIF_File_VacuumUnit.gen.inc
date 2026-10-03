// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_VacuumUnit 的讀寫檔（HTEditList：elVacuumUnit → <recipe>\HandlerCondition.Data [Vacuum Threshold]／[Tray]）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\VacuumUnit\VacuumUnit.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden VacuumUnit\VacuumUnit.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 15 個（header 宣告 39 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "vclcompat/SysUtils.h"
#include "forms/fMain.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

const int TOTAL_VACUUM_UNIT=8;     // golden VacuumUnit.h:21（移植樹 VacuumUnit/VacuumUnit.h 帶 MyVacuumPanel.h 等真元件，本 TU 不 include）
const int VACUUM_UNIT_WIDTH=81;    // golden VacuumUnit.h:22（SetPanelPos 的 Left，只存值）
const int VACUUM_UNIT_HEIGHT=177;  // golden VacuumUnit.h:23
void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket，1＝FTestSuck）
// golden TMyKitSuck 用到的四個欄位的轉接物件：FTestSuck 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
struct VU_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };
static inline VU_Kit VU_KitOf(int w) { VU_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }
// golden TMyVacuumPanel（VacuumUnit/MyVacuumPanel.h:12）的 C 路替身：只有 golden VacuumUnit.cpp 讀寫的兩個成員。
//   GroupBox：golden ctor :151 new、:159 Visible=false；Parent＝scrlbx*（VacuumUnit.cpp:69/72/85/88/105/108）
//   edSV    ：golden ctor :153 new、:178 Parent=GroupBox、:183 Text＝樣板 edSV->Text（DFM '0'）；elVacuumUnit 的來源元件
//   名稱＝頁面 id：<arr>_<iCol>_<iRow>（面板）、<arr>_<iCol>_<iRow>_edSV（web/page/hwidgets.js makeVacuumPanel）
struct VU_TMyVacuumPanel {
    TGroupBox *GroupBox;
    TEdit     *edSV;
    VU_TMyVacuumPanel(const char* arr, int iCol, int iRow, const char* parent) {
        AnsiString gb; gb.sprintf("%s_%d_%d", arr, iCol, iRow);
        AnsiString sv=gb+"_edSV";
        GroupBox=filerw::EL<TGroupBox>("TfVacuumUnit", gb.c_str());
        GroupBox->Visible=false;                                                         // golden MyVacuumPanel.cpp:159
        edSV=filerw::EL<TEdit>("TfVacuumUnit", sv.c_str());
        edSV->Text=filerw::EL<TEdit>("TfVacuumUnit", "edSV")->Text;                      // golden MyVacuumPanel.cpp:183
        const char* pr[2][2]={{sv.c_str(), gb.c_str()}, {gb.c_str(), parent}};          // golden MyVacuumPanel.cpp:178；VacuumUnit.cpp:69 等
        filerw::ELSetParents("TfVacuumUnit", pr, 2);
    }
};

// golden VacuumUnit\VacuumUnit.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static AnsiString LastFileName;   // golden VacuumUnit.h:77
static bool fShow=false;          // golden VacuumUnit.h:92
static int iCount=0;              // golden VacuumUnit.h:93（建構子 :29 設 5）
static int iIndexColMax=0;        // golden VacuumUnit.h:100（Initial :38-48 設）
static int iInOutColMax=0;        // golden VacuumUnit.h:101
static int iTest1=0;              // golden VacuumUnit.h:102（elVacuumUnit [Tray] Edit1；VCL 表單記憶體清零 → 0）
static bool bTest1=false;         // golden VacuumUnit.h:103（elVacuumUnit [Tray] CheckBox1）
static VU_TMyVacuumPanel *myPalArm2  [TOTAL_VACUUM_UNIT][2];     // golden VacuumUnit.h:81（TMyVacuumPanel* → 替身，見 decls）
static VU_TMyVacuumPanel *myPalArm1  [TOTAL_VACUUM_UNIT][2];     // golden VacuumUnit.h:82
static VU_TMyVacuumPanel *myPalInArm [TOTAL_VACUUM_UNIT/2][2];   // golden VacuumUnit.h:83
static VU_TMyVacuumPanel *myPalOutArm[TOTAL_VACUUM_UNIT/2][2];   // golden VacuumUnit.h:84
#define FTestSuck VU_KitOf(1)   // golden FTestSuck.iShtRow／iShtCol／iMaxRow／iMaxCol（ShowSuckMode）→ FileRW/_KitSuck.cpp

using filerw::EL;

// 前置宣告
static void VU_TfVacuumUnit();
static void VU_Initial();
static void VU_SetPanelPos(bool bInit);
static void VU_ShowSuckMode(int iSel);
static void VU_FormShow();
static void VU_FormClose();
static void VU_ReadFile();
static void VU_DoIniDataToForm();
static void VU_SaveSetupFile(AnsiString szDir, AnsiString S);
static void VU_spbSaveClick();

// golden VacuumUnit\VacuumUnit.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void VU_DfmItems()
{
}

// golden VacuumUnit\VacuumUnit.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kVU_ParentOf[][2] = {
    {"CheckBox1", "tsManual"},
    {"Edit1", "tsManual"},
    {"GroupBox1", "tsManual"},
    {"edSV", "GroupBox1"},
    {"edSetInArm", "grpSetAllValue"},
    {"edSetIndexArm", "grpSetAllValue"},
    {"edSetOutArm", "grpSetAllValue"},
    {"grpInarm", "tsInArm"},
    {"grpIndexArm1", "tsIndex"},
    {"grpIndexArm2", "tsIndex"},
    {"grpOutarm", "tsOutArm"},
    {"grpSetAllValue", "tsManual"},
    {"scrlbxInArm", "grpInarm"},
    {"scrlbxIndexArm1", "grpIndexArm1"},
    {"scrlbxIndexArm2", "grpIndexArm2"},
    {"scrlbxOutArm", "grpOutarm"},
    {"spbSave", "Panel1"},
    {"tsInArm", "pgcVacuumUnit"},
    {"tsIndex", "pgcVacuumUnit"},
    {"tsManual", "pgcVacuumUnit"},
    {"tsOutArm", "pgcVacuumUnit"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void VU_CreateContainerProxies()
{
    EL<TGroupBox>("TfVacuumUnit", "GroupBox1");
    EL<TPanel>("TfVacuumUnit", "Panel1");
    EL<TGroupBox>("TfVacuumUnit", "grpInarm");
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm1");
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2");
    EL<TGroupBox>("TfVacuumUnit", "grpOutarm");
    EL<TGroupBox>("TfVacuumUnit", "grpSetAllValue");
    EL<TPageControl>("TfVacuumUnit", "pgcVacuumUnit");
    EL<TTabSheet>("TfVacuumUnit", "tsInArm");
    EL<TTabSheet>("TfVacuumUnit", "tsIndex");
    EL<TTabSheet>("TfVacuumUnit", "tsManual");
    EL<TTabSheet>("TfVacuumUnit", "tsOutArm");
    filerw::ELSetParents("TfVacuumUnit", kVU_ParentOf, (int)(sizeof(kVU_ParentOf) / sizeof(kVU_ParentOf[0])));
}

// golden VacuumUnit\VacuumUnit.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void VU_DfmState()
{
    EL<TGroupBox>("TfVacuumUnit", "GroupBox1")->Visible = false;
    EL<TEdit>("TfVacuumUnit", "Edit1")->Text = "Edit1";
    EL<TEdit>("TfVacuumUnit", "edSV")->Text = "0";
    EL<TEdit>("TfVacuumUnit", "edSetInArm")->Text = "0";
    EL<TEdit>("TfVacuumUnit", "edSetIndexArm")->Text = "0";
    EL<TEdit>("TfVacuumUnit", "edSetOutArm")->Text = "0";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kVU_EnableAll[] = {
    "CheckBox1", "GroupBox1", "Panel1", "grpInarm", "grpIndexArm1", "grpIndexArm2",
    "grpOutarm", "grpSetAllValue", "pgcVacuumUnit", "scrlbxInArm", "scrlbxIndexArm1", "scrlbxIndexArm2",
    "scrlbxOutArm", "spbSave", "tsInArm", "tsIndex", "tsManual", "tsOutArm",
};

// 存檔流程（spbSaveClick、SaveSetupFile）讀的替身 0 個 —— FileRW/TestIF_File_VacuumUnit.cpp 用它決定頁面必須送哪些值
static const char* const kVU_SaveReads[] = {
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void VU_CreateSaveProxies()
{
}

// golden VacuumUnit\VacuumUnit.cpp:26  TfVacuumUnit::TfVacuumUnit(TComponent* Owner)
static void VU_TfVacuumUnit()
{

    iCount=5;

}

// golden VacuumUnit\VacuumUnit.cpp:32  TfVacuumUnit::Initial()
static void VU_Initial()
{

    bool bShow=true;//, bNoShow=false;
    bool bEnable=true;//, bDisable=false;
    bool bReadFromFile=true;//, bFixedValue=false;
    AnsiString s="";
    if(MachineTypeChoice==Type_HT9046      ||
       MachineTypeChoice==Type_HT9046_LS   ||
       MachineTypeChoice==Type_HT1032      ||
       USE_46_SUCKER_DB==1)                                                     //HT9046LS sucker_9046.db
    {
        iIndexColMax=TOTAL_VACUUM_UNIT;
    }
    else  //HT9045 sucker.db
    {
        iIndexColMax=TOTAL_VACUUM_UNIT/2;
    }
    iInOutColMax=TOTAL_VACUUM_UNIT/2;

    //initial IndexArm2/IndexArm1 Vacuum unit
    ;   // golden VacuumUnit\VacuumUnit.cpp:52-60 取代：grpIndexArm2／grpIndexArm1 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排（vclcompat TControl 沒有 Width／Height）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:52-60 -- grpIndexArm2／grpIndexArm1 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排（vclcompat TControl 沒有 Width／Height）
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2")->Top       =-3;                                                //IndexArm2
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2")->Left      =0;
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2")->Width     =(VACUUM_UNIT_WIDTH+2)*iIndexColMax;
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2")->Height    =(VACUUM_UNIT_HEIGHT+12)*2;

    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm1")->Top       =EL<TGroupBox>("TfVacuumUnit", "grpIndexArm2")->Height;                              //IndexArm1
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm1")->Left      =0;
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm1")->Width     =(VACUUM_UNIT_WIDTH+2)*iIndexColMax;
    EL<TGroupBox>("TfVacuumUnit", "grpIndexArm1")->Height    =(VACUUM_UNIT_HEIGHT+12)*2;
#endif // GATE (S12-C save)

    EL<TEdit>("TfVacuumUnit", "edSV"); EL<TControl>("TfVacuumUnit", "scrlbxIndexArm1"); for(int iCol=0; iCol<iIndexColMax; iCol++) for(int iRow=0; iRow<2; iRow++) myPalArm1[iCol][iRow]=new VU_TMyVacuumPanel("myPalArm1", iCol, iRow, "scrlbxIndexArm1"); EL<TControl>("TfVacuumUnit", "scrlbxIndexArm2"); for(int iCol=0; iCol<iIndexColMax; iCol++) for(int iRow=0; iRow<2; iRow++) myPalArm2[iCol][iRow]=new VU_TMyVacuumPanel("myPalArm2", iCol, iRow, "scrlbxIndexArm2");   // golden VacuumUnit\VacuumUnit.cpp:62-89 取代：new TMyVacuumPanel(this,kind,iCol,iRow)＋GroupBox->Parent=scrlbxIndexArm1/2：面板是執行期元件（IO 位址／畫布／DO 按鈕，MyVacuumPanel.cpp:16-272）。C 路只需要 GroupBox（Visible）與 edSV → VU_TMyVacuumPanel 具名替身（名稱＝頁面 id）；HT9046LS（kind 0/1）與 HT9045（kind 4/5）兩支只差 IO 位址與吸嘴排序，替身相同。樣板 edSV（DFM Text='0'）先建，面板 edSV 照 MyVacuumPanel.cpp:183 抄它
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:62-89 -- new TMyVacuumPanel(this,kind,iCol,iRow)＋GroupBox->Parent=scrlbxIndexArm1/2：面板是執行期元件（IO 位址／畫布／DO 按鈕，MyVacuumPanel.cpp:16-272）。C 路只需要 GroupBox（Visible）與 edSV → VU_TMyVacuumPanel 具名替身（名稱＝頁面 id）；HT9046LS（kind 0/1）與 HT9045（kind 4/5）兩支只差 IO 位址與吸嘴排序，替身相同。樣板 edSV（DFM Text='0'）先建，面板 edSV 照 MyVacuumPanel.cpp:183 抄它
    if(iIndexColMax==TOTAL_VACUUM_UNIT)                                         //HT9046LS sucker_9046.db
    {
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalArm1[iCol][iRow]                     = new TMyVacuumPanel(this,0,iCol,iRow);      //IndexArm1
                myPalArm1[iCol][iRow]->GroupBox->Parent   = EL<TControl>("TfVacuumUnit", "scrlbxIndexArm1");

                myPalArm2[iCol][iRow]                     = new TMyVacuumPanel(this,1,iCol,iRow);      //IndexArm2
                myPalArm2[iCol][iRow]->GroupBox->Parent   = EL<TControl>("TfVacuumUnit", "scrlbxIndexArm2");
            }
        }
    }
    else    //HT9045 sucker.db
    {
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalArm1[iCol][iRow]                     = new TMyVacuumPanel(this,4,iCol,iRow);      //IndexArm1
                myPalArm1[iCol][iRow]->GroupBox->Parent   = EL<TControl>("TfVacuumUnit", "scrlbxIndexArm1");

                myPalArm2[iCol][iRow]                     = new TMyVacuumPanel(this,5,iCol,iRow);      //IndexArm2
                myPalArm2[iCol][iRow]->GroupBox->Parent   = EL<TControl>("TfVacuumUnit", "scrlbxIndexArm2");
            }
        }
    }
#endif // GATE (S12-C save)
    //initial InArm/OutArm Vacuum unit
    ;   // golden VacuumUnit\VacuumUnit.cpp:91-99 取代：grpInarm／grpOutarm 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:91-99 -- grpInarm／grpOutarm 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排
    EL<TGroupBox>("TfVacuumUnit", "grpInarm")->Top       =-3;                                                     //InArm
    EL<TGroupBox>("TfVacuumUnit", "grpInarm")->Left      =0;
    EL<TGroupBox>("TfVacuumUnit", "grpInarm")->Width     =(VACUUM_UNIT_WIDTH+2)*iInOutColMax;
    EL<TGroupBox>("TfVacuumUnit", "grpInarm")->Height    =(VACUUM_UNIT_HEIGHT+12)*2;

    EL<TGroupBox>("TfVacuumUnit", "grpOutarm")->Top      =-3;                                                     //OutArm
    EL<TGroupBox>("TfVacuumUnit", "grpOutarm")->Left     =0;
    EL<TGroupBox>("TfVacuumUnit", "grpOutarm")->Width    =(VACUUM_UNIT_WIDTH+2)*iInOutColMax;
    EL<TGroupBox>("TfVacuumUnit", "grpOutarm")->Height   =(VACUUM_UNIT_HEIGHT+12)*2;
#endif // GATE (S12-C save)
    EL<TControl>("TfVacuumUnit", "scrlbxInArm"); for(int iCol=0; iCol<iInOutColMax; iCol++) for(int iRow=0; iRow<2; iRow++) myPalInArm[iCol][iRow]=new VU_TMyVacuumPanel("myPalInArm", iCol, iRow, "scrlbxInArm"); EL<TControl>("TfVacuumUnit", "scrlbxOutArm"); for(int iCol=0; iCol<iInOutColMax; iCol++) for(int iRow=0; iRow<2; iRow++) myPalOutArm[iCol][iRow]=new VU_TMyVacuumPanel("myPalOutArm", iCol, iRow, "scrlbxOutArm");   // golden VacuumUnit\VacuumUnit.cpp:100-110 取代：new TMyVacuumPanel(this,2/3,iCol,iRow)＋GroupBox->Parent=scrlbxInArm/OutArm：同上（In／Out Arm 面板）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:100-110 -- new TMyVacuumPanel(this,2/3,iCol,iRow)＋GroupBox->Parent=scrlbxInArm/OutArm：同上（In／Out Arm 面板）
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]                    = new TMyVacuumPanel(this,2,iCol,iRow);     //InArm
            myPalInArm[iCol][iRow]->GroupBox->Parent  = EL<TControl>("TfVacuumUnit", "scrlbxInArm");

            myPalOutArm[iCol][iRow]                   = new TMyVacuumPanel(this,3,iCol,iRow);     //OutArm
            myPalOutArm[iCol][iRow]->GroupBox->Parent = EL<TControl>("TfVacuumUnit", "scrlbxOutArm");
        }
    }
#endif // GATE (S12-C save)

    //VacuumUnit
    elVacuumUnit->Add(EL<TCheckBox>("TfVacuumUnit", "CheckBox1"),        &bTest1,          ECBool,           "Tray",    "CheckBox1",      bShow, bEnable, bReadFromFile, 0);
    elVacuumUnit->Add(EL<TEdit>("TfVacuumUnit", "Edit1"),            &iTest1,          ECInteger,        "Tray",    "Edit1",          bShow, bEnable, bReadFromFile, iTest1,          true,  0,   300);

    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            s.sprintf("IndexArm1_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalArm1[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdIndexArm1[iRow][iCol], ECInteger,   "Vacuum Threshold",  s,    bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdIndexArm1[iRow][iCol],    true,  -116.0,   148);
            s.sprintf("IndexArm2_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalArm2[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdIndexArm2[iRow][iCol], ECInteger,   "Vacuum Threshold",  s,    bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdIndexArm2[iRow][iCol],    true,  -116.0,   148);
        }
    }
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            s.sprintf("InArm_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalInArm[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdInArm[iRow][iCol], ECInteger,   "Vacuum Threshold",  s, bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdInArm[iRow][iCol],    true,  -116.0,   148);
            s.sprintf("OutArm_%d_%d",iRow,iCol);
            elVacuumUnit->Add(myPalOutArm[iCol][iRow]->edSV, &TestIF_File.iVaccumThrdOutArm[iRow][iCol], ECInteger,  "Vacuum Threshold",  s, bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdOutArm[iRow][iCol],    true,  -116.0,   148);
        }
    }

}

// golden VacuumUnit\VacuumUnit.cpp:138  TfVacuumUnit::SetPanelPos(bool bInit)
static void VU_SetPanelPos(bool bInit)
{

    //Arm2/Arm1
    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalArm2[iCol][iRow]->GroupBox->Left         =VACUUM_UNIT_WIDTH*iCol;
            myPalArm2[iCol][iRow]->GroupBox->Top          =VACUUM_UNIT_HEIGHT*iRow;

            myPalArm1[iCol][iRow]->GroupBox->Left         =VACUUM_UNIT_WIDTH*iCol;
            myPalArm1[iCol][iRow]->GroupBox->Top          =VACUUM_UNIT_HEIGHT*iRow;
        }
    }
    //In/OutArm
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]->GroupBox->Visible     =bInit;
            myPalInArm[iCol][iRow]->GroupBox->Left        =VACUUM_UNIT_WIDTH*iCol;
            myPalInArm[iCol][iRow]->GroupBox->Top         =VACUUM_UNIT_HEIGHT*iRow;

            myPalOutArm[iCol][iRow]->GroupBox->Visible    =bInit;
            myPalOutArm[iCol][iRow]->GroupBox->Left       =VACUUM_UNIT_WIDTH*iCol;
            myPalOutArm[iCol][iRow]->GroupBox->Top        =VACUUM_UNIT_HEIGHT*iRow;
        }
    }

}

// golden VacuumUnit\VacuumUnit.cpp:168  TfVacuumUnit::ShowSuckMode(int iSel)
static void VU_ShowSuckMode(int iSel)
{

    for(int iRow=0; iRow<FTestSuck.iMaxRow; iRow++) for(int iCol=0; iCol<FTestSuck.iMaxCol; iCol++) { if(iCol>=TOTAL_VACUUM_UNIT || iRow>=2 || myPalArm2[iCol][iRow]==NULL || myPalArm1[iCol][iRow]==NULL) { AnsiString t; t.sprintf("golden VacuumUnit.cpp:170-186 ShowSuckMode: FTestSuck %dx%d but only %d index columns were created (iIndexColMax) -- golden would access-violate on myPalArm[%d][%d]; skipped", FTestSuck.iMaxRow, FTestSuck.iMaxCol, iIndexColMax, iCol, iRow); filerw::ELTodo(t.c_str()); continue; } if(iRow<FTestSuck.iShtRow && iCol<FTestSuck.iShtCol) { myPalArm2[iCol][iRow]->GroupBox->Visible=true; myPalArm1[iCol][iRow]->GroupBox->Visible=true; } else { myPalArm2[iCol][iRow]->GroupBox->Visible=false; myPalArm1[iCol][iRow]->GroupBox->Visible=false; } }   // golden VacuumUnit\VacuumUnit.cpp:170-186 取代：golden 原句＋陣列邊界／NULL 守衛（golden 會 access violation 的情形改成跳過＋todo，見本檔 _SUCK 註解）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:170-186 -- golden 原句＋陣列邊界／NULL 守衛（golden 會 access violation 的情形改成跳過＋todo，見本檔 _SUCK 註解）
    for(int iRow=0; iRow<FTestSuck.iMaxRow; iRow++)
    {
        for(int iCol=0; iCol<FTestSuck.iMaxCol; iCol++)
        {
            if(iRow<FTestSuck.iShtRow &&
               iCol<FTestSuck.iShtCol)
            {
                myPalArm2[iCol][iRow]->GroupBox->Visible=true;
                myPalArm1[iCol][iRow]->GroupBox->Visible=true;
            }
            else
            {
                myPalArm2[iCol][iRow]->GroupBox->Visible=false;
                myPalArm1[iCol][iRow]->GroupBox->Visible=false;
            }
        }
    }
#endif // GATE (S12-C save)

    switch(iSel)
    {
        case SingleSite:        //1x1 BusyShuttle
        case DualSite:          //1x2
        case QualSite2X2N:      //Frank 20200520 2X2NN Mode
        case TriSite1X3:        //Frank 20160329 add for 1x3_4
        case _6Site2X3N:        //Steven 20220425 : 2X3NN Mode
        case QualSite1X4:       //1x4
        case _8Site1X4:         //ChungHung 20150528 add for 海思 _8Site1x4
        case DualSite2x1:       //2x1
        case QualSite2X2:       //2x2
        case _6Site2X3:         //ChungHung 20140115 add for 2x3_6
        case _16Site4X4:        //Sam 20190226 : 16Site4X4
        case _8Site2X4:         //2x4
        case _16Site2X8:        //2x8  //Steven 20110520 : 換位置
        case _10Site2X5:        //wei 20190614 10 site
        case _12Site2X6:
        case _32Site4X8N:       //2x8
            break;
    }

}

// golden VacuumUnit\VacuumUnit.cpp:210  TfVacuumUnit::FormShow(TObject *Sender)
static void VU_FormShow()
{

    AnsiString S="";
    LastFileName=GetLastOpenFN();
    VU_ReadFile();
    S.sprintf("Vacuum Unit Condition  ''%s''  ",LastFileName);
    ;   // golden VacuumUnit\VacuumUnit.cpp:216-216 取代：Caption=S：視窗標題（HTML 不用）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:216-216 -- Caption=S：視窗標題（HTML 不用）
    Caption=S;
#endif // GATE (S12-C save)
    ;   // golden VacuumUnit\VacuumUnit.cpp:217-220 取代：this->Top／Left／Width／Height：視窗位置大小（HTML 不用）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:217-220 -- this->Top／Left／Width／Height：視窗位置大小（HTML 不用）
    this->Top=0;
    this->Left=110;
    this->Width=690;
    this->Height=1020;
#endif // GATE (S12-C save)
    fShow=true;
    ;   // golden VacuumUnit\VacuumUnit.cpp:222-222 取代：tmr1->Enabled=true：tmr1Timer（:255）每 tick 讀 ECAT-VC8 現值／閥值／DI（MyLaneIO／Acm_*）＝即時硬體顯示，不是讀寫檔
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:222-222 -- tmr1->Enabled=true：tmr1Timer（:255）每 tick 讀 ECAT-VC8 現值／閥值／DI（MyLaneIO／Acm_*）＝即時硬體顯示，不是讀寫檔
    EL<TControl>("TfVacuumUnit", "tmr1")->Enabled=true;
#endif // GATE (S12-C save)
    VU_SetPanelPos(fShow);     //設定畫面
    iCount=2;
    VU_ShowSuckMode(TestIF.iTestMode);

    ;   // golden VacuumUnit\VacuumUnit.cpp:227-252 取代：myPal*->RefreshDOIO()：讀 ECAT-VC8 DO（Acm_DaqDoGetBitEx，MyVacuumPanel.cpp:467）更新 bplOn／bplOff 顯示＝即時硬體顯示；其後 :248-252 golden 本來就是註解
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:227-252 -- myPal*->RefreshDOIO()：讀 ECAT-VC8 DO（Acm_DaqDoGetBitEx，MyVacuumPanel.cpp:467）更新 bplOn／bplOff 顯示＝即時硬體顯示；其後 :248-252 golden 本來就是註解
    //IndexArm12
    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalArm1[iCol][iRow]->RefreshDOIO();

            myPalArm2[iCol][iRow]->RefreshDOIO();
        }
    }

    //In/OutArm
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]->RefreshDOIO();

            myPalOutArm[iCol][iRow]->RefreshDOIO();
        }
    }
   /*
   EL<TEdit>("TfVacuumUnit", "edSetInArm")->Text     =(int)myPalInArm[0][0]->ReadVaccumThreshold();      //以第一個位置閥值來當初始值。
   EL<TEdit>("TfVacuumUnit", "edSetIndexArm")->Text  =(int)myPalArm1[0][0]->ReadVaccumThreshold();
   EL<TEdit>("TfVacuumUnit", "edSetOutArm")->Text    =(int)myPalOutArm[0][0]->ReadVaccumThreshold();
   */
#endif // GATE (S12-C save)

}

// golden VacuumUnit\VacuumUnit.cpp:304  TfVacuumUnit::FormClose(TObject *Sender, TCloseAction &Action)
static void VU_FormClose()
{

    VU_ReadFile();
    ;   // golden VacuumUnit\VacuumUnit.cpp:308-308 取代：tmr1->Enabled=false：同 FormShow :222（即時硬體顯示）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:308-308 -- tmr1->Enabled=false：同 FormShow :222（即時硬體顯示）
    EL<TControl>("TfVacuumUnit", "tmr1")->Enabled=false;
#endif // GATE (S12-C save)
    fShow=false;
    VU_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔

}

// golden VacuumUnit\VacuumUnit.cpp:351  TfVacuumUnit::ReadFile()
static void VU_ReadFile()
{

    AnsiString szDir=GetRecipePath();
    LastFileName=GetLastOpenFN();

    if(elVacuumUnit!=NULL)
    {
        elVacuumUnit->ReadEditTextFromFile(szDir, "HandlerCondition.Data");
    }
    VU_DoIniDataToForm();

}

// golden VacuumUnit\VacuumUnit.cpp:363  TfVacuumUnit::DoIniDataToForm()
static void VU_DoIniDataToForm()
{

    if(elVacuumUnit!=NULL)
        elVacuumUnit->InitialDataToEdit();

}

// golden VacuumUnit\VacuumUnit.cpp:369  TfVacuumUnit::SaveSetupFile(AnsiString szDir, AnsiString S)
static void VU_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    MyForceDirectories(szDir);
    if(elVacuumUnit!=NULL)
        elVacuumUnit->SaveEditTextToFile(szDir, "HandlerCondition.Data");

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

// golden VacuumUnit\VacuumUnit.cpp:378  TfVacuumUnit::spbSaveClick(TObject *Sender)
static void VU_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden VacuumUnit\VacuumUnit.cpp:384-384 取代：Close()：golden A02 權限不足時關表單 → 記 closed（PageSave 沒寫檔就跑 golden FormClose 的 ReadFile 還原）
#if 0 // GATE (S12-C save) golden VacuumUnit\VacuumUnit.cpp:384-384 -- Close()：golden A02 權限不足時關表單 → 記 closed（PageSave 沒寫檔就跑 golden FormClose 的 ReadFile 還原）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString szDir="";
    szDir.sprintf("%s%s\\", DataPath, LastFileName);
    VU_SaveSetupFile(szDir, LastFileName);
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(LastFileName, "JOBFILE");
    #endif
    VU_ReadFile();
    EL<TSpeedButton>("TfVacuumUnit", "spbSave")->Down=false;
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }

}

