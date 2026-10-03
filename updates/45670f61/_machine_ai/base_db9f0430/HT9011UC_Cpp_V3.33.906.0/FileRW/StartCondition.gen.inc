// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 StartCondition 的讀寫檔（HTEditList： → system\lastdata.dat＋lastdata_backup.dat（WriteLastDataFile）、config.ini [O_Count]／[Vibrate_Time]／[SocketContact]（WriteLastDataFile 內）、<recipe>\HandlerCondition.Data [HeadCondition]／[O_Count]（或 IniData\SocketCount.ini，D05_1））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cStartCondition.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 213 個（header 宣告 370 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "forms/fMain.h"
#include "forms/fSecurity.h"
#include "forms/fSetup.h"
#include "forms/fLotInfo.h"
#include "csystem.h"
#include "cMyDB.h"

extern bool authCounterClr[9];          // cAuthority.h:61（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）
void GetCountClrAuth();                 // cAuthority.h:76
void GetLimitAuth();                    // cAuthority.h:77
void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket，1＝FTestSuck）
// golden TMyKitSuck 用到的四個欄位的轉接物件：TestSocket／FTestSuck 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
struct SC_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };
static inline SC_Kit SC_KitOf(int w) { SC_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }
// golden TPanel 的 Caption 由頁面收送（PanelAa..Dh 操作員可改；palVibrator* 顯示）：TEdit 衍生 → filerw 照 text 收送，Caption 是 Text 的別名
class SC_CapEdit : public TEdit { public: AnsiString& Caption; SC_CapEdit() : Caption(Text) {} };

// golden cStartCondition.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden cStartCondition.h:431 —— 本 TU 自己的（移植樹沒有 fStartCondition 實例；golden Command.cpp:7358 讀 fStartCondition->fShow 決定 Bit4_HandlerDiagnostics，移植樹那段不讀這裡）
static TTabSheet *SC_pgLifeTimeActivePage=nullptr;   // golden pgLifeTime->ActivePage（VCL 屬性，跨開頁保留；開機＝DFM tsSocketCount，存檔時＝頁面目前的分頁，見 FileRW/StartCondition.cpp）
static TEdit       *SocketID[MAX_SOCKET_ROW][MAX_SOCKET_COL];           // golden cStartCondition.cpp:30
static SC_CapEdit  *SocketContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL];   // golden cStartCondition.cpp:31（TPanel* → SC_CapEdit，見 decls）
static TLabel      *TestLabelCol[MAX_SOCKET_COL];                       // golden cStartCondition.cpp:32
static TLabel      *TestLabelRow[MAX_SOCKET_ROW];                       // golden cStartCondition.cpp:33
static TLabel      *TestIDLabelRow[MAX_SOCKET_ROW];                     // golden cStartCondition.cpp:34
static TLabel      *TestCountLabelRow[MAX_SOCKET_ROW];                  // golden cStartCondition.cpp:35
static TButton     *ClearCntButton[MAX_SOCKET_ROW][MAX_SOCKET_COL];     // golden cStartCondition.cpp:36
#define TestSocket SC_KitOf(0)   // golden TestSocket.iShtRow／iShtCol／iMaxRow／iMaxCol → FileRW/_KitSuck.cpp
#define FTestSuck  SC_KitOf(1)   // golden FTestSuck.iShtRow／iShtCol（BuildHeadRowMap）→ FileRW/_KitSuck.cpp
static int  g_iHeadRowMap[2][16];      // golden cStartCondition.cpp:39（檔案層級 static）
static bool g_bHeadRowMapReady=false;  // golden cStartCondition.cpp:40
static void BuildHeadRowMap(int iGridRowCount)                                  //AI(ht9045-config) 20260728 (RogerYang) : 有效head依序排成連續列,Arm1排完緊接Arm2,中間不留空格
{
    bool bUsed[16];
    bool bAny=false;
    int  iRow=1;
    int  a=0, p=0;

    for(p=0; p<16; p++)
    {
        bUsed[p]=false;
        g_iHeadRowMap[0][p]=0;
        g_iHeadRowMap[1][p]=0;
    }

    for(int i=0; i<FTestSuck.iShtRow; i++)                                      //iPos算式與ProcessHeadContactCount一致
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(j*2+i<16)
            {
                bUsed[j*2+i]=true;
                bAny=true;
            }
        }
    }

    if(bAny==false)                                                             //保護:FTestSuck未設定時全開,避免整頁空白
    {
        for(p=0; p<16; p++)
            bUsed[p]=true;
    }

    for(a=0; a<2; a++)
    {
        for(p=0; p<16; p++)
        {
            if(bUsed[p]==false)
                continue;
            if(iRow>=iGridRowCount)                                             //超出grid列數就停,不越界
                break;

            g_iHeadRowMap[a][p]=iRow;
            iRow++;
        }
    }

    g_bHeadRowMapReady=true;
}   // golden cStartCondition.cpp:42-89（照抄）
static void SC_CheckRadio(TRadioButton *rb)   // VCL TRadioButton.SetChecked(true)：同 Parent（gbStartMode）的其他 radio 變 false
{
    TRadioButton *g[3]={filerw::EL<TRadioButton>("TfStartCondition", "rbStartMode1"), filerw::EL<TRadioButton>("TfStartCondition", "rbStartMode2"),
                        filerw::EL<TRadioButton>("TfStartCondition", "rbStartMode3")};
    for(int i=0; i<3; i++) if(g[i]!=rb) g[i]->Checked=false;
    rb->Checked=true;
}
static void SC_VclTabShowing(TTabSheet *t)   // VCL TTabSheet.SetTabShowing(false)→TPageControl.DeleteTab：藏起來的是 ActivePage →
{                                      // 換到「同一個索引」＝下一個可見頁（沒有就最後一個可見頁）。pgLifeTime 的頁照 golden DFM 順序
    static const char* const kTabs[]={"tsCondition01", "tsCondition02", "tsCondition03", "TabsSocketID", "tsVibration", "tsSmartDiagnostic", "tsSocketCount", "tsPickerLifeTime", "tsCylinderView"};
    if(t!=SC_pgLifeTimeActivePage || t->TabVisible) return;
    int n=(int)(sizeof(kTabs)/sizeof(kTabs[0])), at=-1;
    for(int i=0; i<n; i++) if(filerw::EL<TTabSheet>("TfStartCondition", kTabs[i])==t) at=i;
    for(int i=at+1; i<n; i++) { TTabSheet *p=filerw::EL<TTabSheet>("TfStartCondition", kTabs[i]); if(p->TabVisible) { SC_pgLifeTimeActivePage=p; return; } }
    for(int i=at-1; i>=0; i--) { TTabSheet *p=filerw::EL<TTabSheet>("TfStartCondition", kTabs[i]); if(p->TabVisible) { SC_pgLifeTimeActivePage=p; return; } }
    SC_pgLifeTimeActivePage=nullptr;
}
static void SC_DfmStatic()   // golden cStartCondition.dfm 設計期 Tag（btnClear*／btnInArm*…／sgHeadCondition2..）、grid ColCount／RowCount、pgLifeTime.ActivePage —— 產生器的 DfmState 不帶這三樣；FileRW_StartCondition_Boot 在建構子之前呼叫
{
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ab")->Tag=1;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ac")->Tag=2;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ad")->Tag=3;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ae")->Tag=4;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Af")->Tag=5;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ag")->Tag=6;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ah")->Tag=7;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Ba")->Tag=8;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bb")->Tag=9;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bc")->Tag=10;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bd")->Tag=11;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Be")->Tag=12;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bf")->Tag=13;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bg")->Tag=14;
    filerw::EL<TButton>("TfStartCondition", "btnArm1Bh")->Tag=15;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ab")->Tag=1;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ac")->Tag=2;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ad")->Tag=3;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ae")->Tag=4;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Af")->Tag=5;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ag")->Tag=6;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ah")->Tag=7;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Ba")->Tag=8;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bb")->Tag=9;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bc")->Tag=10;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bd")->Tag=11;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Be")->Tag=12;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bf")->Tag=13;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bg")->Tag=14;
    filerw::EL<TButton>("TfStartCondition", "btnArm2Bh")->Tag=15;
    filerw::EL<TButton>("TfStartCondition", "btnClearAa")->Tag=1;
    filerw::EL<TButton>("TfStartCondition", "btnClearAb")->Tag=2;
    filerw::EL<TButton>("TfStartCondition", "btnClearAc")->Tag=3;
    filerw::EL<TButton>("TfStartCondition", "btnClearAd")->Tag=4;
    filerw::EL<TButton>("TfStartCondition", "btnClearAe")->Tag=5;
    filerw::EL<TButton>("TfStartCondition", "btnClearAf")->Tag=6;
    filerw::EL<TButton>("TfStartCondition", "btnClearAg")->Tag=7;
    filerw::EL<TButton>("TfStartCondition", "btnClearAh")->Tag=8;
    filerw::EL<TButton>("TfStartCondition", "btnClearBa")->Tag=9;
    filerw::EL<TButton>("TfStartCondition", "btnClearBb")->Tag=10;
    filerw::EL<TButton>("TfStartCondition", "btnClearBc")->Tag=11;
    filerw::EL<TButton>("TfStartCondition", "btnClearBd")->Tag=12;
    filerw::EL<TButton>("TfStartCondition", "btnClearBe")->Tag=13;
    filerw::EL<TButton>("TfStartCondition", "btnClearBf")->Tag=14;
    filerw::EL<TButton>("TfStartCondition", "btnClearBg")->Tag=15;
    filerw::EL<TButton>("TfStartCondition", "btnClearBh")->Tag=16;
    filerw::EL<TButton>("TfStartCondition", "btnClearCa")->Tag=17;
    filerw::EL<TButton>("TfStartCondition", "btnClearCb")->Tag=18;
    filerw::EL<TButton>("TfStartCondition", "btnClearCc")->Tag=19;
    filerw::EL<TButton>("TfStartCondition", "btnClearCd")->Tag=20;
    filerw::EL<TButton>("TfStartCondition", "btnClearCe")->Tag=21;
    filerw::EL<TButton>("TfStartCondition", "btnClearCf")->Tag=22;
    filerw::EL<TButton>("TfStartCondition", "btnClearCg")->Tag=23;
    filerw::EL<TButton>("TfStartCondition", "btnClearCh")->Tag=24;
    filerw::EL<TButton>("TfStartCondition", "btnClearDa")->Tag=25;
    filerw::EL<TButton>("TfStartCondition", "btnClearDb")->Tag=26;
    filerw::EL<TButton>("TfStartCondition", "btnClearDc")->Tag=27;
    filerw::EL<TButton>("TfStartCondition", "btnClearDd")->Tag=28;
    filerw::EL<TButton>("TfStartCondition", "btnClearDe")->Tag=29;
    filerw::EL<TButton>("TfStartCondition", "btnClearDf")->Tag=30;
    filerw::EL<TButton>("TfStartCondition", "btnClearDg")->Tag=31;
    filerw::EL<TButton>("TfStartCondition", "btnClearDh")->Tag=32;
    filerw::EL<TButton>("TfStartCondition", "btnInArmB")->Tag=4;
    filerw::EL<TButton>("TfStartCondition", "btnInArmC")->Tag=1;
    filerw::EL<TButton>("TfStartCondition", "btnInArmD")->Tag=5;
    filerw::EL<TButton>("TfStartCondition", "btnInArmE")->Tag=2;
    filerw::EL<TButton>("TfStartCondition", "btnInArmF")->Tag=6;
    filerw::EL<TButton>("TfStartCondition", "btnInArmG")->Tag=3;
    filerw::EL<TButton>("TfStartCondition", "btnInArmH")->Tag=7;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmB")->Tag=4;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmC")->Tag=1;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmD")->Tag=5;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmE")->Tag=2;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmF")->Tag=6;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmG")->Tag=3;
    filerw::EL<TButton>("TfStartCondition", "btnOutArmH")->Tag=7;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->grid.ColCount=3; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->grid.RowCount=3;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->grid.ColCount=4; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->grid.RowCount=33;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Tag=1;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->grid.ColCount=4; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->grid.RowCount=33;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Tag=2;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->grid.ColCount=4; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->grid.RowCount=33;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Tag=3;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->grid.ColCount=4; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->grid.RowCount=33;
    filerw::EL<filerw::ELStringGrid>("TfStartCondition", "strngrdCylinderView")->grid.ColCount=4; filerw::EL<filerw::ELStringGrid>("TfStartCondition", "strngrdCylinderView")->grid.RowCount=10;
    SC_pgLifeTimeActivePage=filerw::EL<TTabSheet>("TfStartCondition", "tsSocketCount");   /* golden DFM pgLifeTime.ActivePage */
}

using filerw::EL;

// 前置宣告
static void SC_TfStartCondition();
static void SC_FormShow();
static void SC_FormClose();
static void SC_sbSameAsHead1Click();
static void SC_sbClearCountClick();
static void SC_sbSaveClick();
static void SC_spbExitClick();
static void SC_sbHeadCondition1ClearClick(TObject *Sender);
static void SC_ReadWriteStartCondition(bool bRead);
static void SC_sbHeadCondition1SaveClick(TObject *Sender);
static void SC_DoIniDataToForm();
static void SC_pgLifeTimeChange();
static void SC_ClearPickerCount(int iArea, int iTag);
static void SC_btnInArmAClick(TObject *Sender);
static void SC_btnOutArmAClick(TObject *Sender);
static void SC_btnArm2AaClick(TObject *Sender);
static void SC_btnArm1AaClick(TObject *Sender);
static void SC_WritePickerCount();
static void SC_btnClearAaClick(TObject *Sender);

// golden cStartCondition.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void SC_DfmItems()
{
}

// golden cStartCondition.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kSC_ParentOf[][2] = {
    {"GroupBox1", "ScrollBox1"},
    {"GroupBox2", "ScrollBox1"},
    {"GroupBox3", "ScrollBox1"},
    {"GroupBox4", "ScrollBox1"},
    {"LabelCnt_RowA", "groupbSocketID"},
    {"LabelCnt_RowB", "groupbSocketID"},
    {"LabelCnt_RowC", "groupbSocketID"},
    {"LabelCnt_RowD", "groupbSocketID"},
    {"LabelID_RowA", "groupbSocketID"},
    {"LabelID_RowB", "groupbSocketID"},
    {"LabelID_RowC", "groupbSocketID"},
    {"LabelID_RowD", "groupbSocketID"},
    {"Panel1", "tsLifeTime"},
    {"PanelAa", "groupbSocketID"},
    {"PanelAb", "groupbSocketID"},
    {"PanelAc", "groupbSocketID"},
    {"PanelAd", "groupbSocketID"},
    {"PanelAe", "groupbSocketID"},
    {"PanelAf", "groupbSocketID"},
    {"PanelAg", "groupbSocketID"},
    {"PanelAh", "groupbSocketID"},
    {"PanelBa", "groupbSocketID"},
    {"PanelBb", "groupbSocketID"},
    {"PanelBc", "groupbSocketID"},
    {"PanelBd", "groupbSocketID"},
    {"PanelBe", "groupbSocketID"},
    {"PanelBf", "groupbSocketID"},
    {"PanelBg", "groupbSocketID"},
    {"PanelBh", "groupbSocketID"},
    {"PanelCa", "groupbSocketID"},
    {"PanelCb", "groupbSocketID"},
    {"PanelCc", "groupbSocketID"},
    {"PanelCd", "groupbSocketID"},
    {"PanelCe", "groupbSocketID"},
    {"PanelCf", "groupbSocketID"},
    {"PanelCg", "groupbSocketID"},
    {"PanelCh", "groupbSocketID"},
    {"PanelDa", "groupbSocketID"},
    {"PanelDb", "groupbSocketID"},
    {"PanelDc", "groupbSocketID"},
    {"PanelDd", "groupbSocketID"},
    {"PanelDe", "groupbSocketID"},
    {"PanelDf", "groupbSocketID"},
    {"PanelDg", "groupbSocketID"},
    {"PanelDh", "groupbSocketID"},
    {"ScrollBox1", "tsPickerLifeTime"},
    {"TabsSocketID", "pgLifeTime"},
    {"btnClearAa", "groupbSocketID"},
    {"btnClearAb", "groupbSocketID"},
    {"btnClearAc", "groupbSocketID"},
    {"btnClearAd", "groupbSocketID"},
    {"btnClearAe", "groupbSocketID"},
    {"btnClearAf", "groupbSocketID"},
    {"btnClearAg", "groupbSocketID"},
    {"btnClearAh", "groupbSocketID"},
    {"btnClearBa", "groupbSocketID"},
    {"btnClearBb", "groupbSocketID"},
    {"btnClearBc", "groupbSocketID"},
    {"btnClearBd", "groupbSocketID"},
    {"btnClearBe", "groupbSocketID"},
    {"btnClearBf", "groupbSocketID"},
    {"btnClearBg", "groupbSocketID"},
    {"btnClearBh", "groupbSocketID"},
    {"btnClearCa", "groupbSocketID"},
    {"btnClearCb", "groupbSocketID"},
    {"btnClearCc", "groupbSocketID"},
    {"btnClearCd", "groupbSocketID"},
    {"btnClearCe", "groupbSocketID"},
    {"btnClearCf", "groupbSocketID"},
    {"btnClearCg", "groupbSocketID"},
    {"btnClearCh", "groupbSocketID"},
    {"btnClearDa", "groupbSocketID"},
    {"btnClearDb", "groupbSocketID"},
    {"btnClearDc", "groupbSocketID"},
    {"btnClearDd", "groupbSocketID"},
    {"btnClearDe", "groupbSocketID"},
    {"btnClearDf", "groupbSocketID"},
    {"btnClearDg", "groupbSocketID"},
    {"btnClearDh", "groupbSocketID"},
    {"cbAlarmData_FT", "gbInitialStart"},
    {"cbAlarmData_RT", "grRetestStart"},
    {"cbContactCount_FT", "gbInitialStart"},
    {"cbContactCount_RT", "grRetestStart"},
    {"cbLoadingCount_FT", "gbInitialStart"},
    {"cbLoadingCount_RT", "grRetestStart"},
    {"cbScanner_FT", "gbInitialStart"},
    {"cbScanner_RT", "grRetestStart"},
    {"cbSortingCount_FT", "gbInitialStart"},
    {"cbSortingCount_RT", "grRetestStart"},
    {"cbStartModeOnlyFT", "tsFunction"},
    {"cbTestCategory_FT", "gbInitialStart"},
    {"cbTestCategory_RT", "grRetestStart"},
    {"cbTimeData_FT", "gbInitialStart"},
    {"cbTimeData_RT", "grRetestStart"},
    {"editContactCountAlarm", "Panel1"},
    {"editSocketAa", "groupbSocketID"},
    {"editSocketAb", "groupbSocketID"},
    {"editSocketAc", "groupbSocketID"},
    {"editSocketAd", "groupbSocketID"},
    {"editSocketAe", "groupbSocketID"},
    {"editSocketAf", "groupbSocketID"},
    {"editSocketAg", "groupbSocketID"},
    {"editSocketAh", "groupbSocketID"},
    {"editSocketBa", "groupbSocketID"},
    {"editSocketBb", "groupbSocketID"},
    {"editSocketBc", "groupbSocketID"},
    {"editSocketBd", "groupbSocketID"},
    {"editSocketBe", "groupbSocketID"},
    {"editSocketBf", "groupbSocketID"},
    {"editSocketBg", "groupbSocketID"},
    {"editSocketBh", "groupbSocketID"},
    {"editSocketCa", "groupbSocketID"},
    {"editSocketCb", "groupbSocketID"},
    {"editSocketCc", "groupbSocketID"},
    {"editSocketCd", "groupbSocketID"},
    {"editSocketCe", "groupbSocketID"},
    {"editSocketCf", "groupbSocketID"},
    {"editSocketCg", "groupbSocketID"},
    {"editSocketCh", "groupbSocketID"},
    {"editSocketDa", "groupbSocketID"},
    {"editSocketDb", "groupbSocketID"},
    {"editSocketDc", "groupbSocketID"},
    {"editSocketDd", "groupbSocketID"},
    {"editSocketDe", "groupbSocketID"},
    {"editSocketDf", "groupbSocketID"},
    {"editSocketDg", "groupbSocketID"},
    {"editSocketDh", "groupbSocketID"},
    {"edtArm1Aa", "GroupBox4"},
    {"edtArm1Ab", "GroupBox4"},
    {"edtArm1Ac", "GroupBox4"},
    {"edtArm1Ad", "GroupBox4"},
    {"edtArm1Ae", "GroupBox4"},
    {"edtArm1Af", "GroupBox4"},
    {"edtArm1Ag", "GroupBox4"},
    {"edtArm1Ah", "GroupBox4"},
    {"edtArm1Ba", "GroupBox4"},
    {"edtArm1Bb", "GroupBox4"},
    {"edtArm1Bc", "GroupBox4"},
    {"edtArm1Bd", "GroupBox4"},
    {"edtArm1Be", "GroupBox4"},
    {"edtArm1Bf", "GroupBox4"},
    {"edtArm1Bg", "GroupBox4"},
    {"edtArm1Bh", "GroupBox4"},
    {"edtArm2Aa", "GroupBox3"},
    {"edtArm2Ab", "GroupBox3"},
    {"edtArm2Ac", "GroupBox3"},
    {"edtArm2Ad", "GroupBox3"},
    {"edtArm2Ae", "GroupBox3"},
    {"edtArm2Af", "GroupBox3"},
    {"edtArm2Ag", "GroupBox3"},
    {"edtArm2Ah", "GroupBox3"},
    {"edtArm2Ba", "GroupBox3"},
    {"edtArm2Bb", "GroupBox3"},
    {"edtArm2Bc", "GroupBox3"},
    {"edtArm2Bd", "GroupBox3"},
    {"edtArm2Be", "GroupBox3"},
    {"edtArm2Bf", "GroupBox3"},
    {"edtArm2Bg", "GroupBox3"},
    {"edtArm2Bh", "GroupBox3"},
    {"edtInArmA", "GroupBox1"},
    {"edtInArmB", "GroupBox1"},
    {"edtInArmC", "GroupBox1"},
    {"edtInArmD", "GroupBox1"},
    {"edtInArmE", "GroupBox1"},
    {"edtInArmF", "GroupBox1"},
    {"edtInArmG", "GroupBox1"},
    {"edtInArmH", "GroupBox1"},
    {"edtInOutArmPickerAlmCnt", "ScrollBox1"},
    {"edtKitNo1", "pnlKitNo1"},
    {"edtKitNo2", "pnlKitNo2"},
    {"edtKitNo3", "pnlKitNo3"},
    {"edtOutArmA", "GroupBox2"},
    {"edtOutArmB", "GroupBox2"},
    {"edtOutArmC", "GroupBox2"},
    {"edtOutArmD", "GroupBox2"},
    {"edtOutArmE", "GroupBox2"},
    {"edtOutArmF", "GroupBox2"},
    {"edtOutArmG", "GroupBox2"},
    {"edtOutArmH", "GroupBox2"},
    {"edtSetXYOffsetLimit", "gbSetOffsetLimit"},
    {"edtSetZOffsetLimit", "gbSetOffsetLimit"},
    {"gbContactCount", "tsStartMode"},
    {"gbCounterClear", "tsStartMode"},
    {"gbInitialStart", "gbCounterClear"},
    {"gbSetOffsetLimit", "tsFunction"},
    {"gbStartMode", "tsStartMode"},
    {"grRetestStart", "gbCounterClear"},
    {"groupbSocketID", "TabsSocketID"},
    {"labContactCountAlarm", "Panel1"},
    {"labTestColA", "groupbSocketID"},
    {"labTestColB", "groupbSocketID"},
    {"labTestColC", "groupbSocketID"},
    {"labTestColD", "groupbSocketID"},
    {"labTestColE", "groupbSocketID"},
    {"labTestColF", "groupbSocketID"},
    {"labTestColG", "groupbSocketID"},
    {"labTestColH", "groupbSocketID"},
    {"labTestRowA", "groupbSocketID"},
    {"labTestRowB", "groupbSocketID"},
    {"labTestRowC", "groupbSocketID"},
    {"labTestRowD", "groupbSocketID"},
    {"palVibratorHP1", "tsVibration"},
    {"palVibratorSht1", "tsVibration"},
    {"palVibratorSht2", "tsVibration"},
    {"palVibratorUnloader", "tsVibration"},
    {"pgLifeTime", "tsLifeTime"},
    {"pnlKitNo1", "tsCondition01"},
    {"pnlKitNo2", "tsCondition02"},
    {"pnlKitNo3", "tsCondition03"},
    {"rbStartMode1", "gbStartMode"},
    {"rbStartMode2", "gbStartMode"},
    {"rbStartMode3", "gbStartMode"},
    {"sgContactCount", "gbContactCount"},
    {"sgHeadCondition1", "tsCondition01"},
    {"sgHeadCondition2", "tsCondition02"},
    {"sgHeadCondition3", "tsCondition03"},
    {"sgSocketCount", "tsSocketCount"},
    {"strngrdCylinderView", "tsCylinderView"},
    {"tsCondition01", "pgLifeTime"},
    {"tsCondition02", "pgLifeTime"},
    {"tsCondition03", "pgLifeTime"},
    {"tsCylinderView", "pgLifeTime"},
    {"tsFunction", "PageControl1"},
    {"tsLifeTime", "PageControl1"},
    {"tsPickerLifeTime", "pgLifeTime"},
    {"tsSmartDiagnostic", "pgLifeTime"},
    {"tsSocketCount", "pgLifeTime"},
    {"tsStartMode", "PageControl1"},
    {"tsVibration", "pgLifeTime"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void SC_CreateContainerProxies()
{
    EL<TGroupBox>("TfStartCondition", "GroupBox1");
    EL<TGroupBox>("TfStartCondition", "GroupBox2");
    EL<TGroupBox>("TfStartCondition", "GroupBox3");
    EL<TGroupBox>("TfStartCondition", "GroupBox4");
    EL<TPageControl>("TfStartCondition", "PageControl1");
    EL<TPanel>("TfStartCondition", "Panel1");
    EL<TControl>("TfStartCondition", "ScrollBox1");
    EL<TTabSheet>("TfStartCondition", "TabsSocketID");
    EL<TGroupBox>("TfStartCondition", "gbContactCount");
    EL<TGroupBox>("TfStartCondition", "gbCounterClear");
    EL<TGroupBox>("TfStartCondition", "gbInitialStart");
    EL<TGroupBox>("TfStartCondition", "gbSetOffsetLimit");
    EL<TGroupBox>("TfStartCondition", "gbStartMode");
    EL<TGroupBox>("TfStartCondition", "grRetestStart");
    EL<TGroupBox>("TfStartCondition", "groupbSocketID");
    EL<TPageControl>("TfStartCondition", "pgLifeTime");
    EL<TPanel>("TfStartCondition", "pnlKitNo1");
    EL<TPanel>("TfStartCondition", "pnlKitNo2");
    EL<TPanel>("TfStartCondition", "pnlKitNo3");
    EL<TTabSheet>("TfStartCondition", "tsCondition01");
    EL<TTabSheet>("TfStartCondition", "tsCondition02");
    EL<TTabSheet>("TfStartCondition", "tsCondition03");
    EL<TTabSheet>("TfStartCondition", "tsCylinderView");
    EL<TTabSheet>("TfStartCondition", "tsFunction");
    EL<TTabSheet>("TfStartCondition", "tsLifeTime");
    EL<TTabSheet>("TfStartCondition", "tsPickerLifeTime");
    EL<TTabSheet>("TfStartCondition", "tsSocketCount");
    EL<TTabSheet>("TfStartCondition", "tsStartMode");
    EL<TTabSheet>("TfStartCondition", "tsVibration");
    filerw::ELSetParents("TfStartCondition", kSC_ParentOf, (int)(sizeof(kSC_ParentOf) / sizeof(kSC_ParentOf[0])));
}

// golden cStartCondition.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void SC_DfmState()
{
    EL<TEdit>("TfStartCondition", "edtArm1Aa")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ab")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ac")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ad")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ae")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Af")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ag")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ah")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Ba")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bb")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bc")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bd")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Be")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bf")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bg")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm1Bh")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Aa")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ab")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ac")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ad")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ae")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Af")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ag")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ah")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Ba")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bb")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bc")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bd")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Be")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bf")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bg")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtArm2Bh")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmA")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmB")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmC")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmD")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmE")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmF")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmG")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInArmH")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmA")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmB")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmC")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmD")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmE")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmF")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmG")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtOutArmH")->Enabled = false;
    EL<TEdit>("TfStartCondition", "edtInOutArmPickerAlmCnt")->Text = "100000";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kSC_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "GroupBox4", "LabelCnt_RowA", "LabelCnt_RowB",
    "LabelCnt_RowC", "LabelCnt_RowD", "LabelID_RowA", "LabelID_RowB", "LabelID_RowC", "LabelID_RowD",
    "PageControl1", "Panel1", "PanelAa", "PanelAb", "PanelAc", "PanelAd",
    "PanelAe", "PanelAf", "PanelAg", "PanelAh", "PanelBa", "PanelBb",
    "PanelBc", "PanelBd", "PanelBe", "PanelBf", "PanelBg", "PanelBh",
    "PanelCa", "PanelCb", "PanelCc", "PanelCd", "PanelCe", "PanelCf",
    "PanelCg", "PanelCh", "PanelDa", "PanelDb", "PanelDc", "PanelDd",
    "PanelDe", "PanelDf", "PanelDg", "PanelDh", "ScrollBox1", "TabsSocketID",
    "btnClearAa", "btnClearAb", "btnClearAc", "btnClearAd", "btnClearAe", "btnClearAf",
    "btnClearAg", "btnClearAh", "btnClearBa", "btnClearBb", "btnClearBc", "btnClearBd",
    "btnClearBe", "btnClearBf", "btnClearBg", "btnClearBh", "btnClearCa", "btnClearCb",
    "btnClearCc", "btnClearCd", "btnClearCe", "btnClearCf", "btnClearCg", "btnClearCh",
    "btnClearDa", "btnClearDb", "btnClearDc", "btnClearDd", "btnClearDe", "btnClearDf",
    "btnClearDg", "btnClearDh", "cbAlarmData_FT", "cbAlarmData_RT", "cbContactCount_FT", "cbContactCount_RT",
    "cbLoadingCount_FT", "cbLoadingCount_RT", "cbScanner_FT", "cbScanner_RT", "cbSortingCount_FT", "cbSortingCount_RT",
    "cbStartModeOnlyFT", "cbTestCategory_FT", "cbTestCategory_RT", "cbTimeData_FT", "cbTimeData_RT", "gbContactCount",
    "gbCounterClear", "gbInitialStart", "gbSetOffsetLimit", "gbStartMode", "grRetestStart", "groupbSocketID",
    "labContactCountAlarm", "labTestColA", "labTestColB", "labTestColC", "labTestColD", "labTestColE",
    "labTestColF", "labTestColG", "labTestColH", "labTestRowA", "labTestRowB", "labTestRowC",
    "labTestRowD", "palVibratorHP1", "palVibratorSht1", "palVibratorSht2", "palVibratorUnloader", "pgLifeTime",
    "pnlKitNo1", "pnlKitNo2", "pnlKitNo3", "rbStartMode1", "rbStartMode2", "rbStartMode3",
    "tsCondition01", "tsCondition02", "tsCondition03", "tsCylinderView", "tsFunction", "tsLifeTime",
    "tsPickerLifeTime", "tsSmartDiagnostic", "tsSocketCount", "tsStartMode", "tsVibration",
};

// 存檔流程（sbSaveClick、spbExitClick、FormClose、sbHeadCondition1SaveClick、ReadWriteStartCondition）讀的替身 23 個 —— FileRW/StartCondition.cpp 用它決定頁面必須送哪些值
static const char* const kSC_SaveReads[] = {
    "cbAlarmData_FT", "cbAlarmData_RT", "cbContactCount_FT", "cbContactCount_RT", "cbLoadingCount_FT", "cbLoadingCount_RT",
    "cbScanner_FT", "cbScanner_RT", "cbSortingCount_FT", "cbSortingCount_RT", "cbTestCategory_FT", "cbTestCategory_RT",
    "cbTimeData_FT", "cbTimeData_RT", "editContactCountAlarm", "edtInOutArmPickerAlmCnt", "edtKitNo1", "edtKitNo2",
    "edtKitNo3", "rbStartMode1", "rbStartMode2", "sgContactCount", "sgSocketCount",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void SC_CreateSaveProxies()
{
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_FT");
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_RT");
    EL<TCheckBox>("TfStartCondition", "cbContactCount_FT");
    EL<TCheckBox>("TfStartCondition", "cbContactCount_RT");
    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_FT");
    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_RT");
    EL<TCheckBox>("TfStartCondition", "cbScanner_FT");
    EL<TCheckBox>("TfStartCondition", "cbScanner_RT");
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_FT");
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_RT");
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_FT");
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_RT");
    EL<TCheckBox>("TfStartCondition", "cbTimeData_FT");
    EL<TCheckBox>("TfStartCondition", "cbTimeData_RT");
    EL<TEdit>("TfStartCondition", "editContactCountAlarm");
    EL<TEdit>("TfStartCondition", "edtInOutArmPickerAlmCnt");
    EL<TEdit>("TfStartCondition", "edtKitNo1");
    EL<TEdit>("TfStartCondition", "edtKitNo2");
    EL<TEdit>("TfStartCondition", "edtKitNo3");
    EL<TRadioButton>("TfStartCondition", "rbStartMode1");
    EL<TRadioButton>("TfStartCondition", "rbStartMode2");
    EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount");
    EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount");
}

// golden cStartCondition.cpp:94  TfStartCondition::TfStartCondition(TComponent* Owner)
static void SC_TfStartCondition()
{

    //JerryYang 20170214 (wei) add for socket ID and contact count
    //==>
    TEdit *TempSocketID[MAX_SOCKET_ROW][MAX_SOCKET_COL]=
    {
        {EL<TEdit>("TfStartCondition", "editSocketAa"), EL<TEdit>("TfStartCondition", "editSocketAb"), EL<TEdit>("TfStartCondition", "editSocketAc"), EL<TEdit>("TfStartCondition", "editSocketAd"), EL<TEdit>("TfStartCondition", "editSocketAe"), EL<TEdit>("TfStartCondition", "editSocketAf"), EL<TEdit>("TfStartCondition", "editSocketAg"), EL<TEdit>("TfStartCondition", "editSocketAh")},
        {EL<TEdit>("TfStartCondition", "editSocketBa"), EL<TEdit>("TfStartCondition", "editSocketBb"), EL<TEdit>("TfStartCondition", "editSocketBc"), EL<TEdit>("TfStartCondition", "editSocketBd"), EL<TEdit>("TfStartCondition", "editSocketBe"), EL<TEdit>("TfStartCondition", "editSocketBf"), EL<TEdit>("TfStartCondition", "editSocketBg"), EL<TEdit>("TfStartCondition", "editSocketBh")},
        {EL<TEdit>("TfStartCondition", "editSocketCa"), EL<TEdit>("TfStartCondition", "editSocketCb"), EL<TEdit>("TfStartCondition", "editSocketCc"), EL<TEdit>("TfStartCondition", "editSocketCd"), EL<TEdit>("TfStartCondition", "editSocketCe"), EL<TEdit>("TfStartCondition", "editSocketCf"), EL<TEdit>("TfStartCondition", "editSocketCg"), EL<TEdit>("TfStartCondition", "editSocketCh")},
        {EL<TEdit>("TfStartCondition", "editSocketDa"), EL<TEdit>("TfStartCondition", "editSocketDb"), EL<TEdit>("TfStartCondition", "editSocketDc"), EL<TEdit>("TfStartCondition", "editSocketDd"), EL<TEdit>("TfStartCondition", "editSocketDe"), EL<TEdit>("TfStartCondition", "editSocketDf"), EL<TEdit>("TfStartCondition", "editSocketDg"), EL<TEdit>("TfStartCondition", "editSocketDh")}
    };
    SC_CapEdit *TempSocketContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL]; for(int r=0; r<MAX_SOCKET_ROW; r++) for(int c=0; c<MAX_SOCKET_COL; c++) TempSocketContactCnt[r][c]=EL<SC_CapEdit>("TfStartCondition", (AnsiString("Panel")+AnsiString((char)('A'+r))+AnsiString((char)('a'+c))).c_str());   // golden cStartCondition.cpp:106-112 取代：TPanel *TempSocketContactCnt[4][8]={{PanelAa,…}}：golden TPanel 的 Caption 由操作員改（PanelAaDblClick 小鍵盤），替身改 SC_CapEdit（TEdit 衍生、Caption＝Text 別名），頁面值照 text 收送。名稱照 golden（PanelAa..PanelDh）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:106-112 -- TPanel *TempSocketContactCnt[4][8]={{PanelAa,…}}：golden TPanel 的 Caption 由操作員改（PanelAaDblClick 小鍵盤），替身改 SC_CapEdit（TEdit 衍生、Caption＝Text 別名），頁面值照 text 收送。名稱照 golden（PanelAa..PanelDh）
    TPanel *TempSocketContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL]=
    {
        {EL<TPanel>("TfStartCondition", "PanelAa"), EL<TPanel>("TfStartCondition", "PanelAb"), EL<TPanel>("TfStartCondition", "PanelAc"), EL<TPanel>("TfStartCondition", "PanelAd"), EL<TPanel>("TfStartCondition", "PanelAe"), EL<TPanel>("TfStartCondition", "PanelAf"), EL<TPanel>("TfStartCondition", "PanelAg"), EL<TPanel>("TfStartCondition", "PanelAh")},
        {EL<TPanel>("TfStartCondition", "PanelBa"), EL<TPanel>("TfStartCondition", "PanelBb"), EL<TPanel>("TfStartCondition", "PanelBc"), EL<TPanel>("TfStartCondition", "PanelBd"), EL<TPanel>("TfStartCondition", "PanelBe"), EL<TPanel>("TfStartCondition", "PanelBf"), EL<TPanel>("TfStartCondition", "PanelBg"), EL<TPanel>("TfStartCondition", "PanelBh")},
        {EL<TPanel>("TfStartCondition", "PanelCa"), EL<TPanel>("TfStartCondition", "PanelCb"), EL<TPanel>("TfStartCondition", "PanelCc"), EL<TPanel>("TfStartCondition", "PanelCd"), EL<TPanel>("TfStartCondition", "PanelCe"), EL<TPanel>("TfStartCondition", "PanelCf"), EL<TPanel>("TfStartCondition", "PanelCg"), EL<TPanel>("TfStartCondition", "PanelCh")},
        {EL<TPanel>("TfStartCondition", "PanelDa"), EL<TPanel>("TfStartCondition", "PanelDb"), EL<TPanel>("TfStartCondition", "PanelDc"), EL<TPanel>("TfStartCondition", "PanelDd"), EL<TPanel>("TfStartCondition", "PanelDe"), EL<TPanel>("TfStartCondition", "PanelDf"), EL<TPanel>("TfStartCondition", "PanelDg"), EL<TPanel>("TfStartCondition", "PanelDh")}
    };
#endif // GATE (S12-C save)
    TLabel *tempTestLabelCol[MAX_SOCKET_COL]=
    {
        EL<TLabel>("TfStartCondition", "labTestColA"), EL<TLabel>("TfStartCondition", "labTestColB"), EL<TLabel>("TfStartCondition", "labTestColC"), EL<TLabel>("TfStartCondition", "labTestColD"), EL<TLabel>("TfStartCondition", "labTestColE"), EL<TLabel>("TfStartCondition", "labTestColF"), EL<TLabel>("TfStartCondition", "labTestColG"), EL<TLabel>("TfStartCondition", "labTestColH")
    };
    TButton *TempCleanBtn[MAX_SOCKET_ROW][MAX_SOCKET_COL]=                      //JerryYang 20250120 : add
    {
        {EL<TButton>("TfStartCondition", "btnClearAa"), EL<TButton>("TfStartCondition", "btnClearAb"), EL<TButton>("TfStartCondition", "btnClearAc"), EL<TButton>("TfStartCondition", "btnClearAd"), EL<TButton>("TfStartCondition", "btnClearAe"), EL<TButton>("TfStartCondition", "btnClearAf"), EL<TButton>("TfStartCondition", "btnClearAg"), EL<TButton>("TfStartCondition", "btnClearAh")},
        {EL<TButton>("TfStartCondition", "btnClearBa"), EL<TButton>("TfStartCondition", "btnClearBb"), EL<TButton>("TfStartCondition", "btnClearBc"), EL<TButton>("TfStartCondition", "btnClearBd"), EL<TButton>("TfStartCondition", "btnClearBe"), EL<TButton>("TfStartCondition", "btnClearBf"), EL<TButton>("TfStartCondition", "btnClearBg"), EL<TButton>("TfStartCondition", "btnClearBh")},
        {EL<TButton>("TfStartCondition", "btnClearCa"), EL<TButton>("TfStartCondition", "btnClearCb"), EL<TButton>("TfStartCondition", "btnClearCc"), EL<TButton>("TfStartCondition", "btnClearCd"), EL<TButton>("TfStartCondition", "btnClearCe"), EL<TButton>("TfStartCondition", "btnClearCf"), EL<TButton>("TfStartCondition", "btnClearCg"), EL<TButton>("TfStartCondition", "btnClearCh")},
        {EL<TButton>("TfStartCondition", "btnClearDa"), EL<TButton>("TfStartCondition", "btnClearDb"), EL<TButton>("TfStartCondition", "btnClearDc"), EL<TButton>("TfStartCondition", "btnClearDd"), EL<TButton>("TfStartCondition", "btnClearDe"), EL<TButton>("TfStartCondition", "btnClearDf"), EL<TButton>("TfStartCondition", "btnClearDg"), EL<TButton>("TfStartCondition", "btnClearDh")}
    };
    TLabel *tempTestLabelRow[MAX_SOCKET_ROW]={EL<TLabel>("TfStartCondition", "labTestRowA"), EL<TLabel>("TfStartCondition", "labTestRowB"), EL<TLabel>("TfStartCondition", "labTestRowC"), EL<TLabel>("TfStartCondition", "labTestRowD")};
    TLabel *tempLabelIDRow[MAX_SOCKET_ROW]={EL<TLabel>("TfStartCondition", "LabelID_RowA"), EL<TLabel>("TfStartCondition", "LabelID_RowB"), EL<TLabel>("TfStartCondition", "LabelID_RowC"), EL<TLabel>("TfStartCondition", "LabelID_RowD")};
    TLabel *tempLabelCountRow[MAX_SOCKET_ROW]={EL<TLabel>("TfStartCondition", "LabelCnt_RowA"), EL<TLabel>("TfStartCondition", "LabelCnt_RowB"), EL<TLabel>("TfStartCondition", "LabelCnt_RowC"), EL<TLabel>("TfStartCondition", "LabelCnt_RowD")};
    int iCol, iRow;
    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        iRow=i/MAX_SOCKET_COL;
        iCol=i%MAX_SOCKET_COL;
        if(i<MAX_SOCKET_ROW)
        {
            TestLabelRow[i]=tempTestLabelRow[i];
            TestLabelRow[i]->Visible=false;
            TestIDLabelRow[i]=tempLabelIDRow[i];
            TestIDLabelRow[i]->Visible=false;
            TestCountLabelRow[i]=tempLabelCountRow[i];
            TestCountLabelRow[i]->Visible=false;
        }

        if(i<MAX_SOCKET_COL)
        {
            TestLabelCol[i]=tempTestLabelCol[i];
            TestLabelCol[i]->Visible=false;
        }
        SocketID[iRow][iCol]=TempSocketID[iRow][iCol];
        SocketID[iRow][iCol]->Visible=false;
        SocketContactCnt[iRow][iCol]=TempSocketContactCnt[iRow][iCol];
        SocketContactCnt[iRow][iCol]->Visible=false;

        ClearCntButton[iRow][iCol]=TempCleanBtn[iRow][iCol];                    //JerryYang 20250120 : add
    }
    //<==
    //JerryYang 20170214 (wei) add for socket ID and contact count
    ;   // golden cStartCondition.cpp:156-157 取代：GetDC(strngrdCylinderView->Handle)／new TCanvas：Cylinder 頁的 GDI 繪圖（strngrdCylinderViewDrawCell），HTML 自己畫；Cylinder 頁未移植（見 FileRW/StartCondition.cpp）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:156-157 -- GetDC(strngrdCylinderView->Handle)／new TCanvas：Cylinder 頁的 GDI 繪圖（strngrdCylinderViewDrawCell），HTML 自己畫；Cylinder 頁未移植（見 FileRW/StartCondition.cpp）
    PtrDCCyl=GetDC(EL<filerw::ELStringGrid>("TfStartCondition", "strngrdCylinderView")->Handle);                                //Eastsun 20260521 整合//Sam 20230516 : Pre Alrm Cylinder
    EL<TControl>("TfStartCondition", "pCanvasCyl")=new TCanvas;                                                     //Eastsun 20260521 整合//Sam 20230516 : Pre Alrm Cylinder
#endif // GATE (S12-C save)

}

// golden cStartCondition.cpp:160  TfStartCondition::FormShow(TObject *Sender)
static void SC_FormShow()
{

    GetCountClrAuth();
    GetLimitAuth();                                                             //JerryYang 20200504 銦片計數下限
    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_FT")->Enabled=authCounterClr[3];
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_FT")->Enabled=authCounterClr[1];
    EL<TCheckBox>("TfStartCondition", "cbContactCount_FT")->Enabled=authCounterClr[4];
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_FT")->Enabled=authCounterClr[6];
    EL<TCheckBox>("TfStartCondition", "cbScanner_FT")     ->Enabled=authCounterClr[2];
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_FT")   ->Enabled=authCounterClr[0];
    EL<TCheckBox>("TfStartCondition", "cbTimeData_FT")    ->Enabled=authCounterClr[7];
    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_RT")->Enabled=authCounterClr[3];
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_RT")->Enabled=authCounterClr[1];
    EL<TCheckBox>("TfStartCondition", "cbContactCount_RT")->Enabled=authCounterClr[4];
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_RT")->Enabled=authCounterClr[6];
    EL<TCheckBox>("TfStartCondition", "cbScanner_RT")     ->Enabled=authCounterClr[2];
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_RT")   ->Enabled=authCounterClr[0];
    EL<TCheckBox>("TfStartCondition", "cbTimeData_RT")    ->Enabled=authCounterClr[7];

    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_FT")->Checked=LastSet.bCTClear[0][0];
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_FT")->Checked=LastSet.bCTClear[0][1];
    EL<TCheckBox>("TfStartCondition", "cbContactCount_FT")->Checked=LastSet.bCTClear[0][2];
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_FT")->Checked=LastSet.bCTClear[0][3];
    EL<TCheckBox>("TfStartCondition", "cbScanner_FT")     ->Checked=LastSet.bCTClear[0][4];
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_FT")   ->Checked=LastSet.bCTClear[0][5];
    EL<TCheckBox>("TfStartCondition", "cbTimeData_FT")    ->Checked=LastSet.bCTClear[0][6];
    EL<TCheckBox>("TfStartCondition", "cbLoadingCount_RT")->Checked=LastSet.bCTClear[1][0];
    EL<TCheckBox>("TfStartCondition", "cbTestCategory_RT")->Checked=LastSet.bCTClear[1][1];
    EL<TCheckBox>("TfStartCondition", "cbContactCount_RT")->Checked=LastSet.bCTClear[1][2];
    EL<TCheckBox>("TfStartCondition", "cbSortingCount_RT")->Checked=LastSet.bCTClear[1][3];
    EL<TCheckBox>("TfStartCondition", "cbScanner_RT")     ->Checked=LastSet.bCTClear[1][4];
    EL<TCheckBox>("TfStartCondition", "cbAlarmData_RT")   ->Checked=LastSet.bCTClear[1][5];
    EL<TCheckBox>("TfStartCondition", "cbTimeData_RT")    ->Checked=LastSet.bCTClear[1][6];

    AnsiString str;

    if(IniConfig.bShowFTandRTButtonCanClick==true &&                            //Steven 20131224 : FT & RT Buttion 可以按
       CUSTOMER_CODE!=CC_SCC)                                                   //Steven 20200309 : SCC要使用
    {
        LastSet.iStartMode=2;
        EL<TGroupBox>("TfStartCondition", "gbStartMode")->Visible=false;
    }
    TEdit *edtInArmPickLifeCnt[2][4]={                                          //JerryYang 20220331 : add吸嘴作動次數計數
                                     {EL<TEdit>("TfStartCondition", "edtInArmA"), EL<TEdit>("TfStartCondition", "edtInArmC"), EL<TEdit>("TfStartCondition", "edtInArmE"), EL<TEdit>("TfStartCondition", "edtInArmG")},
                                     {EL<TEdit>("TfStartCondition", "edtInArmB"), EL<TEdit>("TfStartCondition", "edtInArmD"), EL<TEdit>("TfStartCondition", "edtInArmF"), EL<TEdit>("TfStartCondition", "edtInArmH")},
                                     };

    TEdit *edtOutArmPickLifeCnt[2][4]={
                                     {EL<TEdit>("TfStartCondition", "edtOutArmA"), EL<TEdit>("TfStartCondition", "edtOutArmC"), EL<TEdit>("TfStartCondition", "edtOutArmE"), EL<TEdit>("TfStartCondition", "edtOutArmG")},
                                     {EL<TEdit>("TfStartCondition", "edtOutArmB"), EL<TEdit>("TfStartCondition", "edtOutArmD"), EL<TEdit>("TfStartCondition", "edtOutArmF"), EL<TEdit>("TfStartCondition", "edtOutArmH")},
                                     };

    TEdit *edtArm1PickLifeCnt[2][8]={
                                     {EL<TEdit>("TfStartCondition", "edtArm1Aa"), EL<TEdit>("TfStartCondition", "edtArm1Ab"), EL<TEdit>("TfStartCondition", "edtArm1Ac"), EL<TEdit>("TfStartCondition", "edtArm1Ad"), EL<TEdit>("TfStartCondition", "edtArm1Ae"), EL<TEdit>("TfStartCondition", "edtArm1Af"), EL<TEdit>("TfStartCondition", "edtArm1Ag"), EL<TEdit>("TfStartCondition", "edtArm1Ah")},
                                     {EL<TEdit>("TfStartCondition", "edtArm1Ba"), EL<TEdit>("TfStartCondition", "edtArm1Bb"), EL<TEdit>("TfStartCondition", "edtArm1Bc"), EL<TEdit>("TfStartCondition", "edtArm1Bd"), EL<TEdit>("TfStartCondition", "edtArm1Be"), EL<TEdit>("TfStartCondition", "edtArm1Bf"), EL<TEdit>("TfStartCondition", "edtArm1Bg"), EL<TEdit>("TfStartCondition", "edtArm1Bh")},
                                     };

    TEdit *edtArm2PickLifeCnt[2][8]={
                                     {EL<TEdit>("TfStartCondition", "edtArm2Aa"), EL<TEdit>("TfStartCondition", "edtArm2Ab"), EL<TEdit>("TfStartCondition", "edtArm2Ac"), EL<TEdit>("TfStartCondition", "edtArm2Ad"), EL<TEdit>("TfStartCondition", "edtArm2Ae"), EL<TEdit>("TfStartCondition", "edtArm2Af"), EL<TEdit>("TfStartCondition", "edtArm2Ag"), EL<TEdit>("TfStartCondition", "edtArm2Ah")},
                                     {EL<TEdit>("TfStartCondition", "edtArm2Ba"), EL<TEdit>("TfStartCondition", "edtArm2Bb"), EL<TEdit>("TfStartCondition", "edtArm2Bc"), EL<TEdit>("TfStartCondition", "edtArm2Bd"), EL<TEdit>("TfStartCondition", "edtArm2Be"), EL<TEdit>("TfStartCondition", "edtArm2Bf"), EL<TEdit>("TfStartCondition", "edtArm2Bg"), EL<TEdit>("TfStartCondition", "edtArm2Bh")},
                                     };
    fSetup->ReadFile(); SC_ReadWriteStartCondition(true);   // golden cStartCondition.cpp:221-221 取代：fSetup->ReadFile()：移植樹 TfSetup::ReadFile（cSetUp.cpp:416）。golden ReadFile 在 cSetUp.cpp:2954 呼叫 fStartCondition->ReadWriteStartCondition(true)，移植樹那一行是 #if 0 GATE(G-SU-StartCond)（cSetUp.cpp:1190，沒有 fStartCondition 實例）→ 在這裡緊接著呼叫本表單的那一支（golden ReadFile 在 :2954 之後沒有任何敘述用到它讀的值，順序等價）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:221-221 -- fSetup->ReadFile()：移植樹 TfSetup::ReadFile（cSetUp.cpp:416）。golden ReadFile 在 cSetUp.cpp:2954 呼叫 fStartCondition->ReadWriteStartCondition(true)，移植樹那一行是 #if 0 GATE(G-SU-StartCond)（cSetUp.cpp:1190，沒有 fStartCondition 實例）→ 在這裡緊接著呼叫本表單的那一支（golden ReadFile 在 :2954 之後沒有任何敘述用到它讀的值，順序等價）
    fSetup->ReadFile();
#endif // GATE (S12-C save)

     if(EL<TCheckBox>("TfStartCondition", "cbStartModeOnlyFT")->Checked==true)                                       //KaiChen 20181206 ：Start Run Mode Only FT
    {
        LastSet.iStartMode=1;
    }

    if(LastSet.iStartMode==0)
    SC_CheckRadio(EL<TRadioButton>("TfStartCondition", "rbStartMode1"));   // golden cStartCondition.cpp:229-229 取代：自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:229-229 -- 自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
        EL<TRadioButton>("TfStartCondition", "rbStartMode1")->Checked=true;
#endif // GATE (S12-C save)
    else if(LastSet.iStartMode==1)
    SC_CheckRadio(EL<TRadioButton>("TfStartCondition", "rbStartMode2"));   // golden cStartCondition.cpp:231-231 取代：自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:231-231 -- 自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
        EL<TRadioButton>("TfStartCondition", "rbStartMode2")->Checked=true;
#endif // GATE (S12-C save)
    else
    SC_CheckRadio(EL<TRadioButton>("TfStartCondition", "rbStartMode3"));   // golden cStartCondition.cpp:233-233 取代：自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:233-233 -- 自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
        EL<TRadioButton>("TfStartCondition", "rbStartMode3")->Checked=true;
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20140918 ASET 不能使用ReTest 功能
    {
        EL<TRadioButton>("TfStartCondition", "rbStartMode3")->Enabled=false;
    SC_CheckRadio(EL<TRadioButton>("TfStartCondition", "rbStartMode2"));   // golden cStartCondition.cpp:238-238 取代：自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:238-238 -- 自動：TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）
        EL<TRadioButton>("TfStartCondition", "rbStartMode2")->Checked=true;
#endif // GATE (S12-C save)
    }

    if(CosFunction.bContactAlmNeedOneCycle)                                     //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    {
    EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->grid.ColCount=4;   // golden cStartCondition.cpp:243-243 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:243-243 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->ColCount=4;
#endif // GATE (S12-C save)
    ;   // golden cStartCondition.cpp:244-244 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:244-244 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Width=657;
#endif // GATE (S12-C save)
        if(LastSet.iLanguageCountry==1)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][0]=" 接觸幾次要發出提示";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][0]=" 接觸幾次要發出警告";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[3][0]=" 目前接觸次數";
        }
        else
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][0]=" Contact times for warning";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][0]=" Contact times for alarm";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[3][0]=" Current Status";
        }
        for(int i=0; i<2; i++)
        {
            if(LastSet.iLanguageCountry==1)
                str.sprintf(" %d 號測試頭接觸次數", i+1);
            else
                str.sprintf(" Head %d Contact ", i+1);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[0][1+i]=str;
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][1+i]=" "+AnsiString(TestIF_File.iContactWarningCount[i]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][1+i]=" "+AnsiString(LastSet.ContactSet[i]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[3][1+i]=" "+AnsiString(LastSet.iContactCT[i]);                                        //jou 2012-06-05
        }
    }
    else
    {
    EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->grid.ColCount=3;   // golden cStartCondition.cpp:271-271 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:271-271 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->ColCount=3;
#endif // GATE (S12-C save)
    ;   // golden cStartCondition.cpp:272-272 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:272-272 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Width=513;
#endif // GATE (S12-C save)
        if(LastSet.iLanguageCountry==1)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][0]=" 接觸幾次要發出警告";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][0]=" 目前接觸次數";
        }
        else
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][0]=" Contact times for alarm";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][0]=" Current Status";
        }

        for(int i=0; i<2; i++)
        {
            if(LastSet.iLanguageCountry==1)
                str.sprintf(" %d 號測試頭接觸次數", i+1);
            else
                str.sprintf(" Head %d Contact ", i+1);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[0][1+i]=str;
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][1+i]=" "+AnsiString(LastSet.ContactSet[i]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][1+i]=" "+AnsiString(LastSet.iContactCT[i]);                                        //jou 2012-06-05
        }
    }

    for(int iRow=1; iRow<EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->grid.RowCount; iRow++)   // golden cStartCondition.cpp:296-296 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:296-296 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
    for(int iRow=1; iRow<EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->RowCount; iRow++)                    //AI(ht9045-config) 20260728 (RogerYang) : 切換模式後site數變少,殘留的site名稱會讓未開啟的列仍可被編輯
#endif // GATE (S12-C save)
    {
    for(int iCol=0; iCol<EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->grid.ColCount; iCol++)   // golden cStartCondition.cpp:298-298 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:298-298 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        for(int iCol=0; iCol<EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->ColCount; iCol++)
#endif // GATE (S12-C save)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[iCol][iRow]="";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[iCol][iRow]="";
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[iCol][iRow]="";
        }
    }
    //ChungHung 20160420 add for Head Count Start
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[1][0]=" Contact times for alarm";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[2][0]=" Current Status";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[3][0]=" Total Contact Count";

    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[1][0]=" Contact times for alarm";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[2][0]=" Current Status";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[3][0]=" Total Contact Count";

    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[1][0]=" Contact times for alarm";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[2][0]=" Current Status";
    EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[3][0]=" Total Contact Count";
    //ChungHung 20160420 add for Head Count End

    BuildHeadRowMap(EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->grid.RowCount);   // golden cStartCondition.cpp:319-319 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:319-319 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
    BuildHeadRowMap(EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->RowCount);                                 //AI(ht9045-config) 20260728 (RogerYang) : 建立(Arm,iPos)→列號映射,有效head排成連續列;Save沿用同一份不重算
#endif // GATE (S12-C save)

    for(int iArm=0; iArm<2; iArm++)
    {
        for(int iPos=0; iPos<16; iPos++)
        {
            int iRowGrid=g_iHeadRowMap[iArm][iPos];
            if(iRowGrid<=0)                                                      //該head未使用,不填;保持空白讓SelectCell擋住編輯
                continue;

            str.sprintf("Arm%d Head %d Contact ", iArm+1, iPos+1);

//            if(CUSTOMER_CODE==CC_KYEC_LEE)                                       // || CosFunction.bHiSiliconFunction==true)
//            {
//                sgHeadCondition1->Cells[1][iRowGrid]=" "+AnsiString(TestIF_File.iContactAlarmCount[0]);
//                sgHeadCondition2->Cells[1][iRowGrid]=" "+AnsiString(TestIF_File.iContactAlarmCount[1]);
//                sgHeadCondition3->Cells[1][iRowGrid]=" "+AnsiString(TestIF_File.iContactAlarmCount[2]);
//            }
//            else
//            {
                EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[1][iRowGrid]=" "+AnsiString(IniConfig.ContactSet[0][iArm][iPos]);   //AI(ht9045-config) 20260728 (RogerYang) : 一律顯示ContactSet;原CC_KYEC_LEE顯示單一值會與grid可編輯衝突(改完數字回彈)
                EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[1][iRowGrid]=" "+AnsiString(IniConfig.ContactSet[1][iArm][iPos]);
                EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[1][iRowGrid]=" "+AnsiString(IniConfig.ContactSet[2][iArm][iPos]);
//            }

            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[0][iRowGrid]=str;
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[2][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCount[0][iArm][iPos]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1")->Cells[3][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCountHistory[0][iArm][iPos]);

            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[0][iRowGrid]=str;
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[2][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCount[1][iArm][iPos]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2")->Cells[3][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCountHistory[1][iArm][iPos]);

            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[0][iRowGrid]=str;
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[2][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCount[2][iArm][iPos]);
            EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")->Cells[3][iRowGrid]=" "+AnsiString(IniConfig.HeadContactCountHistory[2][iArm][iPos]);
        }
    }

    if(CosFunction.bUseHeadContactCount)                                        //Ifor 20160516 京元要求銦片 Life Time 功能
    {
    { EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=IniConfig.bLifeTimeCount[0]; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition01")); }   // golden cStartCondition.cpp:360-360 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:360-360 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=IniConfig.bLifeTimeCount[0];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=IniConfig.bLifeTimeCount[1]; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition02")); }   // golden cStartCondition.cpp:361-361 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:361-361 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=IniConfig.bLifeTimeCount[1];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=IniConfig.bLifeTimeCount[2]; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition03")); }   // golden cStartCondition.cpp:362-362 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:362-362 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=IniConfig.bLifeTimeCount[2];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
#endif // GATE (S12-C save)
        if(IniConfig.bAMDFunction)
    { EL<TTabSheet>("TfStartCondition", "TabsSocketID")->TabVisible=true; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "TabsSocketID")); }   // golden cStartCondition.cpp:364-364 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:364-364 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
            EL<TTabSheet>("TfStartCondition", "TabsSocketID")->TabVisible=true;
#endif // GATE (S12-C save)
    }
    else if(CUSTOMER_CODE==CC_ASE_CL ||                                         //JerryYang 20170214 (wei) ASE中壢才顯示Socket contact life time
            CUSTOMER_CODE==CC_PTI ||                                            //Sam 20170526 (jou) PTI要求socket contact計數
            CUSTOMER_CODE==CC_TERAPOWER ||                                      //Sam 20190916 : 晶兆成也記錄測頭 Count
            CUSTOMER_CODE==CC_HANA_MICRON ||                                    //JimmyChiu 20211008 R211005-Hana-H9-01
            CUSTOMER_CODE==CC_ASE_JP)                                           //Sam 20220706 : ASE_JP 也記錄測頭 Count
    {
    { EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition01")); }   // golden cStartCondition.cpp:372-372 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:372-372 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=false;
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition02")); }   // golden cStartCondition.cpp:373-373 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:373-373 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=false;
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition03")); }   // golden cStartCondition.cpp:374-374 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:374-374 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=false;
#endif // GATE (S12-C save)
    }
    else
    {
    { EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition01")); }   // golden cStartCondition.cpp:378-378 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:378-378 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition01")->TabVisible=false;
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition02")); }   // golden cStartCondition.cpp:379-379 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:379-379 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition02")->TabVisible=false;
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsCondition03")); }   // golden cStartCondition.cpp:380-380 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:380-380 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsCondition03")->TabVisible=false;
#endif // GATE (S12-C save)
    }

    { EL<TTabSheet>("TfStartCondition", "TabsSocketID")->TabVisible=CosFunction.bUseSocketContactCount; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "TabsSocketID")); }   // golden cStartCondition.cpp:383-383 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:383-383 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
    EL<TTabSheet>("TfStartCondition", "TabsSocketID")->TabVisible=CosFunction.bUseSocketContactCount;                //Steven 20221216 : Modify for Socket ID
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsSocketCount")->TabVisible=CosFunction.bUseSocketContactCount; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsSocketCount")); }   // golden cStartCondition.cpp:384-384 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:384-384 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
    EL<TTabSheet>("TfStartCondition", "tsSocketCount")->TabVisible=CosFunction.bUseSocketContactCount;               //Sam 20220720 : 新增一組 Socket Count
#endif // GATE (S12-C save)
    { EL<TTabSheet>("TfStartCondition", "tsVibration")->TabVisible=(TRAY_VIBRATION==VibrationMotor); SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsVibration")); }   // golden cStartCondition.cpp:385-385 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:385-385 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
    EL<TTabSheet>("TfStartCondition", "tsVibration")->TabVisible =(TRAY_VIBRATION==VibrationMotor);                  //Steven 20240904 : 修正Start condition畫面顯示
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CosFunction.bHiSiliconFunction==true ||
       CosFunction.bUseHeadContactCount==true)                                  //RogerYang 20260515 : 修正無法修改alarm setting
    {
    if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))   // golden cStartCondition.cpp:391-391 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:391-391 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))                               //Active Page 在第1頁
#endif // GATE (S12-C save)
        {
            if(IniConfig.bLifeTimeCount[0]==false)                              //O12 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[1]==true)                           //O13 功能開啟 切換Active Page 至第2頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition02");   // golden cStartCondition.cpp:397-397 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:397-397 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition02");
#endif // GATE (S12-C save)
                }
                else if(IniConfig.bLifeTimeCount[2]==true)                      //O14 功能開啟 切換Active Page 至第3頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition03");   // golden cStartCondition.cpp:401-401 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:401-401 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition03");
#endif // GATE (S12-C save)
                }
            }
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))   // golden cStartCondition.cpp:405-405 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:405-405 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))                          //Active Page 在第2頁
#endif // GATE (S12-C save)
        {
            if(IniConfig.bLifeTimeCount[1]==false)                              //O13 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[0]==true)                           //O12 功能開啟 切換Active Page 至第1頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition01");   // golden cStartCondition.cpp:411-411 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:411-411 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition01");
#endif // GATE (S12-C save)
                }
                else if(IniConfig.bLifeTimeCount[2]==true)                      //O14 功能開啟 切換Active Page 至第3頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition03");   // golden cStartCondition.cpp:415-415 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:415-415 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition03");
#endif // GATE (S12-C save)
                }
            }
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))   // golden cStartCondition.cpp:419-419 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:419-419 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))                          //Active Page 在第3頁
#endif // GATE (S12-C save)
        {
            if(IniConfig.bLifeTimeCount[2]==false)                              //O14 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[0]==true)                           //O12 功能開啟 切換Active Page 至第1頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition01");   // golden cStartCondition.cpp:425-425 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:425-425 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition01");
#endif // GATE (S12-C save)
                }
                else if(IniConfig.bLifeTimeCount[1]==true)                      //O13 功能開啟 切換Active Page 至第2頁
                {
    SC_pgLifeTimeActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition02");   // golden cStartCondition.cpp:429-429 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:429-429 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
                    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage=EL<TTabSheet>("TfStartCondition", "tsCondition02");
#endif // GATE (S12-C save)
                }
            }
        }

    if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))   // golden cStartCondition.cpp:434-434 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:434-434 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))
#endif // GATE (S12-C save)
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[0]);
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))   // golden cStartCondition.cpp:438-438 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:438-438 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))
#endif // GATE (S12-C save)
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[1]);
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))   // golden cStartCondition.cpp:442-442 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:442-442 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))
#endif // GATE (S12-C save)
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[2]);
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))   // golden cStartCondition.cpp:446-446 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:446-446 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))                           //Steven 20200211 : Add socket count for ASE-CL
#endif // GATE (S12-C save)
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
        }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))   // golden cStartCondition.cpp:450-450 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:450-450 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
        else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))                          //AI(ht9045-config) 20260728 (RogerYang) : 與pgLifeTimeChange一致,避免開啟時沿用上一頁殘值
#endif // GATE (S12-C save)
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(IniConfig.SocketContactSet[0][0]);
        }


        if(CUSTOMER_CODE==CC_SCC ||
            IniConfig.bVTESTFunction ||                                         //AI(ht9045-config) 20260515 (RogerYang) : SCC/VTEST工程師以上即可修改alarm setting
            CUSTOMER_CODE==CC_XINYUN)                                           //RogerYang 20260728 : Add
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Enabled=(AccessLevel>=iDefEngineerLevel);
        else
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Enabled=(AccessLevel>=iDefHonPrecLevel);
    }
    else
    {
        EL<TLabel>("TfStartCondition", "labContactCountAlarm")->Visible=(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI);
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Visible=(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI);

        if(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI)                   //Steven 20200211 : Add socket count for ASE-CL
        {
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
        }
    }

    EL<TTabSheet>("TfStartCondition", "tsCondition01")->Caption=IniConfig.ContactConditionName[0];
    EL<TTabSheet>("TfStartCondition", "tsCondition02")->Caption=IniConfig.ContactConditionName[1];
    EL<TTabSheet>("TfStartCondition", "tsCondition03")->Caption=IniConfig.ContactConditionName[2];

    ;   // golden cStartCondition.cpp:478-479 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:478-479 -- Left／Top：視窗位置（HTML 不用）
    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    for(int i=0; i<MAX_SOCKET_ROW; i++)                                         //JerryYang 20170214 (wei) add for socket ID and contact count
    {
        TestLabelRow[i]->Visible=false;
        TestIDLabelRow[i]->Visible=false;
        TestCountLabelRow[i]->Visible=false;
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            SocketID[i][j]->Text="";
            SocketID[i][j]->Visible=false;
            TestLabelCol[j]->Visible=false;
            SocketContactCnt[i][j]->Visible=false;
            ClearCntButton[i][j]->Visible=false;                                //JerryYang 20250120 : add
        }
    }

    for(int j=0; j<TestSocket.iShtCol; j++)                                     //JerryYang 20170206
    {
        TestLabelCol[j]->Visible=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            SocketID[i][j]->Visible=true;
            SocketID[i][j]->Enabled=true;
            SocketID[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
            SocketContactCnt[i][j]->Visible=true;
            SocketContactCnt[i][j]->Enabled=true;
            SocketContactCnt[i][j]->Caption=AnsiString(LastSet.iSocketContactCount[i][j]);
            ClearCntButton[i][j]->Visible=true;                                 //JerryYang 20250120 : add
            TestLabelRow[i]->Visible=true;
            TestIDLabelRow[i]->Visible=true;
            TestCountLabelRow[i]->Visible=true;
        }
    }

    EL<SC_CapEdit>("TfStartCondition", "palVibratorHP1")->Caption=IniConfig.iVibratorHP1;   // golden cStartCondition.cpp:514-514 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:514-514 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
    EL<TPanel>("TfStartCondition", "palVibratorHP1")->Caption=IniConfig.iVibratorHP1;                             //JerryYang 20200612 振動馬達作動時間累計
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorSht1")->Caption=IniConfig.iVibratorSht1;   // golden cStartCondition.cpp:515-515 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:515-515 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
    EL<TPanel>("TfStartCondition", "palVibratorSht1")->Caption=IniConfig.iVibratorSht1;
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorSht2")->Caption=IniConfig.iVibratorSht2;   // golden cStartCondition.cpp:516-516 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:516-516 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
    EL<TPanel>("TfStartCondition", "palVibratorSht2")->Caption=IniConfig.iVibratorSht2;
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorUnloader")->Caption=IniConfig.iVibratorUnloader;   // golden cStartCondition.cpp:517-517 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:517-517 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
    EL<TPanel>("TfStartCondition", "palVibratorUnloader")->Caption=IniConfig.iVibratorUnloader;
#endif // GATE (S12-C save)

    EL<TEdit>("TfStartCondition", "edtInOutArmPickerAlmCnt")->Text=TestIF_File.InOutArmLifeCntSet;               //JerryYang 20220331 : add吸嘴作動次數計數
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            edtInArmPickLifeCnt[i][j]->Text=TestIF_File.InArmPickerLifeCnt[i][j];
            edtOutArmPickLifeCnt[i][j]->Text=TestIF_File.OutArmPickerLifeCnt[i][j];
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            edtArm1PickLifeCnt[i][j]->Text=TestIF_File.Arm1PickerLifeCnt[i][j];
            edtArm2PickLifeCnt[i][j]->Text=TestIF_File.Arm2PickerLifeCnt[i][j];
        }
    }

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU || CosFunction.bSetOffsetLimitToAll)      //KaiChen 20181206 ：Start Run Mode Only FT
    {
        if(CosFunction.bSetOffsetLimitToAll)                                    //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
        {
            EL<TTabSheet>("TfStartCondition", "tsFunction")->TabVisible=true;
            EL<TGroupBox>("TfStartCondition", "gbSetOffsetLimit")->Visible=true;
        }
        else
        {
            EL<TGroupBox>("TfStartCondition", "gbSetOffsetLimit")->Visible=false;
        }

        if(CUSTOMER_CODE==CC_SIGURD_HUKOU)
        {
            EL<TCheckBox>("TfStartCondition", "cbStartModeOnlyFT")->Visible=true;
            if(LastSet.iRunStartMode==rsmInitialStart ||
                LastSet.iRunStartMode==rsmCInitialRetest)                       //KaiChen 20181211 ：增加卡關 RunStartMode
            {
                EL<TTabSheet>("TfStartCondition", "tsFunction")->TabVisible=true;
            }
            else
            {
                EL<TTabSheet>("TfStartCondition", "tsFunction")->TabVisible=false;
            }
        }
        else
        {
            EL<TCheckBox>("TfStartCondition", "cbStartModeOnlyFT")->Visible=false;
        }
    }
    else
    {
        EL<TCheckBox>("TfStartCondition", "cbStartModeOnlyFT")->Visible=false;
        EL<TTabSheet>("TfStartCondition", "tsFunction")->TabVisible=false;
    }

    if(CosFunction.bCylinderOnOffTimeLog)                                       //JerryYang 20220215 : cylinder作動次數計數
    {
    { EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")->TabVisible=true; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")); }   // golden cStartCondition.cpp:576-576 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:576-576 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")->TabVisible=true;
#endif // GATE (S12-C save)
    }
    else
    {
    { EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")->TabVisible=false; SC_VclTabShowing(EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")); }   // golden cStartCondition.cpp:580-580 取代：自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:580-580 -- 自動：TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）
        EL<TTabSheet>("TfStartCondition", "tsSmartDiagnostic")->TabVisible=false;
#endif // GATE (S12-C save)
    }

    if(CosFunction.bUseSocketContactCount)                                      //Sam 20220720 : 新增一組 Socket Count
    {
    for(int iRow=1; iRow<EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->grid.RowCount; iRow++)   // golden cStartCondition.cpp:585-585 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:585-585 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        for(int iRow=1; iRow<EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->RowCount; iRow++)                   //AI(ht9045-config) 20260728 (RogerYang) : 切換模式後socket數變少,殘留名稱會讓未開啟的列仍可被編輯
#endif // GATE (S12-C save)
        {
    for(int iCol=0; iCol<EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->grid.ColCount; iCol++)   // golden cStartCondition.cpp:587-587 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:587-587 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
            for(int iCol=0; iCol<EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->ColCount; iCol++)
#endif // GATE (S12-C save)
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[iCol][iRow]="";
        }

        EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[1][0]=" Contact times for alarm";
        EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[2][0]=" Current Status";
        EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[3][0]=" Total Contact Count";

        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                str.sprintf("Socket %s Contact ", IndexSuckName[i][j]);
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[0][1+(i*TestSocket.iShtCol+j)]=str;
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[1][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactSet[i][j]);
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[2][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactCount[i][j]);
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[3][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactCountHistory[i][j]);
            }
        }
    }

    if(CosFunction.bSetOffsetLimitToAll)                                        //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
    {
        EL<TEdit>("TfStartCondition", "edtSetXYOffsetLimit")->Text=InputLimit.iOffsetXYHigh;
        EL<TEdit>("TfStartCondition", "edtSetZOffsetLimit")->Text=InputLimit.iOffsetZHigh;
    }

    fShow=true;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cStartCondition.cpp:619  TfStartCondition::FormClose(TObject *Sender, TCloseAction &Action)
static void SC_FormClose()
{
    filerw::ELMark("FormClose");   // FileRW：存檔流程 trace（ack.trace）

    SC_sbSaveClick();   // golden cStartCondition.cpp:621-621 取代：sbSaveClick(this)：事件處理器的 Sender 參數已拿掉
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:621-621 -- sbSaveClick(this)：事件處理器的 Sender 參數已拿掉
    SC_sbSaveClick(this);
#endif // GATE (S12-C save)
    if(HasICUnderMachine()==false)                                              //Steven 20210309 : 避免生產中被修改模式
    {
        if(EL<TRadioButton>("TfStartCondition", "rbStartMode1")->Checked)
            LastSet.iStartMode=0;
        else if(EL<TRadioButton>("TfStartCondition", "rbStartMode2")->Checked)
            LastSet.iStartMode=1;
        else
            LastSet.iStartMode=2;
    }
    fShow=false;

    for(int i=0; i<TestSocket.iMaxRow; i++)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            fLotInfo->edSocket[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
        }
    }
    //Sam 20210615 : 修正 Count Alarm 寫入 ini 需要讀取回來。
    //==>
    AnsiString S;
    S=GetLastOpenFN();
    AnsiString szDir="";

    if(IniConfig.bD05_1SaveSocketCntByHandler)                                  //Steven 20250807 : By handler save contact count
    {
        szDir="D:\\HT9045\\IniData\\SocketCount.ini";
    }
    else
    {
        szDir=GetRecipeFileName("HandlerCondition.Data");
    }

    AnsiString szDirIndium=szDir;                                               //AI(ht9045-config) 20260517 (RogerYang) : VTEST銦片SPEC一律跟Recipe,不受D05影響
    if(IniConfig.bVTESTFunction && CosFunction.bUseHeadContactCount)
        szDirIndium=GetRecipeFileName("HandlerCondition.Data");

    TestIF_File.iContactAlarmCount[0]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[1]",   6000);   //Ifor 20200306 : add 讀不到檔案需寫入預設值
    TestIF_File.iContactAlarmCount[1]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[2]",   6000);   //Ifor 20200306 : add 讀不到檔案需寫入預設值
    TestIF_File.iContactAlarmCount[2]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[3]",   6000);   //Ifor 20200306 : add 讀不到檔案需寫入預設值
    TestIF_File.iContactAlarmCount[3]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[4]",   0);      //Ifor 20200306 : add 讀不到檔案需寫入預設值
    //<==
    //Sam 20210615 : 修正 Count Alarm 寫入 ini 需要讀取回來。

    SC_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);    //Steven 20100629

}

// golden cStartCondition.cpp:671  TfStartCondition::sbSameAsHead1Click(TObject *Sender)
static void SC_sbSameAsHead1Click()
{

    if(CosFunction.bContactAlmNeedOneCycle)                                     //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    {
        for(int i=1; i<2; i++)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][i+1]=EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][1];
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][i+1]=EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][1];
        }
    }
    else
    {
        for(int i=1; i<2; i++)
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][i+1]=EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][1];
    }

}

// golden cStartCondition.cpp:688  TfStartCondition::sbClearCountClick(TObject *Sender)
static void SC_sbClearCountClick()
{

   if(CosFunction.bContactAlmNeedOneCycle)                                      //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    {
        for(int i=0; i<2; i++)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[3][1+i]="0";
            LastSet.iContactCT[i]=0;
        }
    }
    else
    {
        for(int i=0; i<2; i++)
        {
            EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][1+i]="0";
            LastSet.iContactCT[i]=0;                                            //jou 2012-06-05
        }
    }

}

// golden cStartCondition.cpp:708  TfStartCondition::sbSaveClick(TObject *Sender)
static void SC_sbSaveClick()
{
    filerw::ELMark("sbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szDir;
    if(CosFunction.bContactAlmNeedOneCycle)                                     //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    {
        if(IniConfig.bD05_1SaveSocketCntByHandler)                              //Steven 20250807 : By handler save contact count
        {
            szDir="D:\\HT9045\\IniData\\SocketCount.ini";
        }
        else
        {
            szDir=GetRecipeFileName("HandlerCondition.Data");
        }

        WriteIniData(szDir, "HeadCondition",       "HandContactWarningSet[0]",   EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][1]);
        WriteIniData(szDir, "HeadCondition",       "HandContactWarningSet[1]",   EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][2]);
        for(int i=0; i<2; i++)                                                  //jou 2012-06-05
        {
            LastSet.ContactSet[i]=atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][i+1].c_str());
            LastSet.iContactCT[i]=atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[3][i+1].c_str());
        }
    }
    else
    {
        for(int i=0; i<2; i++)                                                  //jou 2012-06-05
        {
            LastSet.ContactSet[i]=atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[1][i+1].c_str());
            LastSet.iContactCT[i]=atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgContactCount")->Cells[2][i+1].c_str());
        }
    }
    WriteLastDataFile();                                                        //Steven 20231017 : add

}

// golden cStartCondition.cpp:751  TfStartCondition::spbExitClick(TObject *Sender)
static void SC_spbExitClick()
{
    filerw::ELMark("spbExitClick");   // FileRW：存檔流程 trace（ack.trace）

    LastSet.bCTClear[0][0]=EL<TCheckBox>("TfStartCondition", "cbLoadingCount_FT")->Checked;
    LastSet.bCTClear[0][1]=EL<TCheckBox>("TfStartCondition", "cbTestCategory_FT")->Checked;
    LastSet.bCTClear[0][2]=EL<TCheckBox>("TfStartCondition", "cbContactCount_FT")->Checked;
    LastSet.bCTClear[0][3]=EL<TCheckBox>("TfStartCondition", "cbSortingCount_FT")->Checked;
    LastSet.bCTClear[0][4]=EL<TCheckBox>("TfStartCondition", "cbScanner_FT")->Checked;
    LastSet.bCTClear[0][5]=EL<TCheckBox>("TfStartCondition", "cbAlarmData_FT")->Checked;
    LastSet.bCTClear[0][6]=EL<TCheckBox>("TfStartCondition", "cbTimeData_FT")->Checked;
    LastSet.bCTClear[1][0]=EL<TCheckBox>("TfStartCondition", "cbLoadingCount_RT")->Checked;
    LastSet.bCTClear[1][1]=EL<TCheckBox>("TfStartCondition", "cbTestCategory_RT")->Checked;
    LastSet.bCTClear[1][2]=EL<TCheckBox>("TfStartCondition", "cbContactCount_RT")->Checked;
    LastSet.bCTClear[1][3]=EL<TCheckBox>("TfStartCondition", "cbSortingCount_RT")->Checked;
    LastSet.bCTClear[1][4]=EL<TCheckBox>("TfStartCondition", "cbScanner_RT")->Checked;
    LastSet.bCTClear[1][5]=EL<TCheckBox>("TfStartCondition", "cbAlarmData_RT")->Checked;
    LastSet.bCTClear[1][6]=EL<TCheckBox>("TfStartCondition", "cbTimeData_RT")->Checked;

    filerw::ELMark("closed");   // golden cStartCondition.cpp:768-768 取代：Close()：golden 關表單（ShowModal 的表單 → ModalResult，spbExitClick 回傳後 modal 迴圈才跑 FormClose）→ 記 closed；FormClose 由 FileRW/StartCondition.cpp 在本處理器之後呼叫（同 VCL 順序）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:768-768 -- Close()：golden 關表單（ShowModal 的表單 → ModalResult，spbExitClick 回傳後 modal 迴圈才跑 FormClose）→ 記 closed；FormClose 由 FileRW/StartCondition.cpp 在本處理器之後呼叫（同 VCL 順序）
    Close();
#endif // GATE (S12-C save)
    WriteLastDataFile();

}

// golden cStartCondition.cpp:772  TfStartCondition::sbHeadCondition1ClearClick(TObject *Sender)
static void SC_sbHeadCondition1ClearClick(TObject *Sender)
{

    TSpeedButton *PSpeedButton=dynamic_cast <TSpeedButton *>(Sender);
    int iTag=PSpeedButton->Tag;

    if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))   // golden cStartCondition.cpp:778-778 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:778-778 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))
#endif // GATE (S12-C save)
        iTag=0;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))   // golden cStartCondition.cpp:780-780 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:780-780 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))
#endif // GATE (S12-C save)
        iTag=1;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))   // golden cStartCondition.cpp:782-782 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:782-782 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))
#endif // GATE (S12-C save)
        iTag=2;
//JerryYang 20170214 (wei) 清除socket ID及socket contact count
//==>
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))   // golden cStartCondition.cpp:786-786 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:786-786 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))
#endif // GATE (S12-C save)
        iTag=3;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsVibration"))   // golden cStartCondition.cpp:788-788 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:788-788 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsVibration"))                                //JerryYang 20200612 振動馬達作動時間累計
#endif // GATE (S12-C save)
        iTag=4;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))   // golden cStartCondition.cpp:790-790 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:790-790 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))                              //Sam 20220720 : 新增一組 Socket Count
#endif // GATE (S12-C save)
        iTag=5;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsPickerLifeTime"))   // golden cStartCondition.cpp:792-792 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:792-792 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsPickerLifeTime"))                           //Steven 20230110 : 清除Picker Count
#endif // GATE (S12-C save)
        iTag=6;

    if(iTag>=0 && iTag<=2)                                                      //JerryYang 20170214 (wei) HeadCondition1~3
    {
    filerw::ELStringGrid *StrGrid[]={EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")};   // golden cStartCondition.cpp:797-797 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:797-797 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        TStringGrid *StrGrid[]={EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")};
#endif // GATE (S12-C save)

    for(int i=1; i<StrGrid[iTag]->grid.RowCount; i++)   // golden cStartCondition.cpp:799-799 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:799-799 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
        for(int i=1; i<StrGrid[iTag]->RowCount; i++)                            //AI(ht9045-config) 20260728 (RogerYang) : RowCount已擴為33,原寫死16列會漏清;只清有site名稱的列
#endif // GATE (S12-C save)
        {
            if(StrGrid[iTag]->Cells[0][i].Trim()!="")
                StrGrid[iTag]->Cells[2][i]="0";
        }
    }
    else if(iTag==3)
    {
        for(int i=0; i<TestSocket.iMaxRow; i++)
        {
            for(int j=0; j<TestSocket.iMaxCol; j++)
            {
                SocketID[i][j]->Text="";
                LastSet.iSocketContactCount[i][j]=0;
                SocketContactCnt[i][j]->Caption="0";
            }
        }
    }
    else if(iTag==4)                                                            //JerryYang 20200612 振動馬達作動時間累計
    {
    EL<SC_CapEdit>("TfStartCondition", "palVibratorHP1")->Caption="0";   // golden cStartCondition.cpp:819-819 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:819-819 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
        EL<TPanel>("TfStartCondition", "palVibratorHP1")->Caption="0";
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorSht1")->Caption="0";   // golden cStartCondition.cpp:820-820 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:820-820 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
        EL<TPanel>("TfStartCondition", "palVibratorSht1")->Caption="0";
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorSht2")->Caption="0";   // golden cStartCondition.cpp:821-821 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:821-821 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
        EL<TPanel>("TfStartCondition", "palVibratorSht2")->Caption="0";
#endif // GATE (S12-C save)
    EL<SC_CapEdit>("TfStartCondition", "palVibratorUnloader")->Caption="0";   // golden cStartCondition.cpp:822-822 取代：自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:822-822 -- 自動：palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示
        EL<TPanel>("TfStartCondition", "palVibratorUnloader")->Caption="0";
#endif // GATE (S12-C save)

        IniConfig.iVibratorHP1=0;
        IniConfig.iVibratorSht1=0;
        IniConfig.iVibratorSht2=0;
        IniConfig.iVibratorUnloader=0;
        RecordProcess("Clear all vibrator time");
    }
    else if(iTag==5)                                                            //Sam 20220720 : 新增一組 Socket Count
    {
//        for(int i=0; i<TestSocket.iShtCnt; i++)
//            sgSocketCount->Cells[2][1+i]="0";
        for(int i=0; i<TestSocket.iShtRow; i++)                                     //AI(ht9045-config) 20260728 (RogerYang) : 與填表/回讀的row算式一致
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
                EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[2][1+(i*TestSocket.iShtCol+j)]="0";
        }
    }
    else if(iTag==6)                                                            //Steven 20230110 : 清除Picker Count
    {
    SC_ClearPickerCount(-1, -1);   // golden cStartCondition.cpp:842-842 取代：ClearPickerCount()：golden 預設參數 (iArea=-1, iTag=-1) 在 header；產生的 static 函式沒有預設值 → 明寫
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:842-842 -- ClearPickerCount()：golden 預設參數 (iArea=-1, iTag=-1) 在 header；產生的 static 函式沒有預設值 → 明寫
        SC_ClearPickerCount();
#endif // GATE (S12-C save)
    }
//<==
//JerryYang 20170214 (wei) 清除socket ID及socket contact count

}

// golden cStartCondition.cpp:848  TfStartCondition::ReadWriteStartCondition(bool bRead)
static void SC_ReadWriteStartCondition(bool bRead)
{
    filerw::ELMark("ReadWriteStartCondition");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString str1, str2, str3, str4;
    AnsiString szDir="";
    int iPage=0;
    int iSetCount=0;

    filerw::ELStringGrid *StrGrid[]={EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")};   // golden cStartCondition.cpp:855-855 取代：自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:855-855 -- 自動：TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面
    TStringGrid *StrGrid[]={EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition1"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition2"), EL<filerw::ELStringGrid>("TfStartCondition", "sgHeadCondition3")};
#endif // GATE (S12-C save)

    TEdit *edtInArmPickLifeCnt[2][4]={                                          //JerryYang 20220331 : add吸嘴作動次數計數
                                     {EL<TEdit>("TfStartCondition", "edtInArmA"), EL<TEdit>("TfStartCondition", "edtInArmC"), EL<TEdit>("TfStartCondition", "edtInArmE"), EL<TEdit>("TfStartCondition", "edtInArmG")},
                                     {EL<TEdit>("TfStartCondition", "edtInArmB"), EL<TEdit>("TfStartCondition", "edtInArmD"), EL<TEdit>("TfStartCondition", "edtInArmF"), EL<TEdit>("TfStartCondition", "edtInArmH")},
                                     };

    TEdit *edtOutArmPickLifeCnt[2][4]={
                                     {EL<TEdit>("TfStartCondition", "edtOutArmA"), EL<TEdit>("TfStartCondition", "edtOutArmC"), EL<TEdit>("TfStartCondition", "edtOutArmE"), EL<TEdit>("TfStartCondition", "edtOutArmG")},
                                     {EL<TEdit>("TfStartCondition", "edtOutArmB"), EL<TEdit>("TfStartCondition", "edtOutArmD"), EL<TEdit>("TfStartCondition", "edtOutArmF"), EL<TEdit>("TfStartCondition", "edtOutArmH")},
                                     };

    TEdit *edtArm1PickLifeCnt[2][8]={
                                     {EL<TEdit>("TfStartCondition", "edtArm1Aa"), EL<TEdit>("TfStartCondition", "edtArm1Ab"), EL<TEdit>("TfStartCondition", "edtArm1Ac"), EL<TEdit>("TfStartCondition", "edtArm1Ad"), EL<TEdit>("TfStartCondition", "edtArm1Ae"), EL<TEdit>("TfStartCondition", "edtArm1Af"), EL<TEdit>("TfStartCondition", "edtArm1Ag"), EL<TEdit>("TfStartCondition", "edtArm1Ah")},
                                     {EL<TEdit>("TfStartCondition", "edtArm1Ba"), EL<TEdit>("TfStartCondition", "edtArm1Bb"), EL<TEdit>("TfStartCondition", "edtArm1Bc"), EL<TEdit>("TfStartCondition", "edtArm1Bd"), EL<TEdit>("TfStartCondition", "edtArm1Be"), EL<TEdit>("TfStartCondition", "edtArm1Bf"), EL<TEdit>("TfStartCondition", "edtArm1Bg"), EL<TEdit>("TfStartCondition", "edtArm1Bh")},
                                     };

    TEdit *edtArm2PickLifeCnt[2][8]={
                                     {EL<TEdit>("TfStartCondition", "edtArm2Aa"), EL<TEdit>("TfStartCondition", "edtArm2Ab"), EL<TEdit>("TfStartCondition", "edtArm2Ac"), EL<TEdit>("TfStartCondition", "edtArm2Ad"), EL<TEdit>("TfStartCondition", "edtArm2Ae"), EL<TEdit>("TfStartCondition", "edtArm2Af"), EL<TEdit>("TfStartCondition", "edtArm2Ag"), EL<TEdit>("TfStartCondition", "edtArm2Ah")},
                                     {EL<TEdit>("TfStartCondition", "edtArm2Ba"), EL<TEdit>("TfStartCondition", "edtArm2Bb"), EL<TEdit>("TfStartCondition", "edtArm2Bc"), EL<TEdit>("TfStartCondition", "edtArm2Bd"), EL<TEdit>("TfStartCondition", "edtArm2Be"), EL<TEdit>("TfStartCondition", "edtArm2Bf"), EL<TEdit>("TfStartCondition", "edtArm2Bg"), EL<TEdit>("TfStartCondition", "edtArm2Bh")},
                                     };

    if(IniConfig.bD05_1SaveSocketCntByHandler)                                  //Steven 20250807 : By handler save contact count
    {
        szDir="D:\\HT9045\\IniData\\SocketCount.ini";
    }
    else
    {
        szDir=GetRecipeFileName("HandlerCondition.Data");
    }

    AnsiString szDirIndium=szDir;                                               //AI(ht9045-config) 20260517 (RogerYang) : VTEST銦片SPEC一律跟Recipe,不受D05影響
    if(IniConfig.bVTESTFunction && CosFunction.bUseHeadContactCount)
        szDirIndium=GetRecipeFileName("HandlerCondition.Data");

    if(bRead)
    {
        TestIF_File.iContactAlarmCount[0]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[1]",   6000);                                       //Ifor 20200306 : add 讀不到檔案需寫入預設值
        TestIF_File.iContactAlarmCount[1]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[2]",   6000);
        TestIF_File.iContactAlarmCount[2]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[3]",   6000);
        TestIF_File.iContactAlarmCount[3]=CheckAndReadIniData(szDirIndium, "HeadCondition",       "HeadContactSet[4]",   0);

        EL<TEdit>("TfStartCondition", "edtKitNo1")->Text=CheckAndReadIniData(szDir, "HeadCondition", "Kit No 1", AnsiString(""));
        EL<TEdit>("TfStartCondition", "edtKitNo2")->Text=CheckAndReadIniData(szDir, "HeadCondition", "Kit No 2", AnsiString(""));
        EL<TEdit>("TfStartCondition", "edtKitNo3")->Text=CheckAndReadIniData(szDir, "HeadCondition", "Kit No 3", AnsiString(""));

        if(CosFunction.bContactAlmNeedOneCycle)                                 //Sam 20241226 : Contact Alarm 需要先做 OneCycle
        {
            TestIF_File.iContactWarningCount[0]=CheckAndReadIniData(szDir, "HeadCondition",       "HandContactWarningSet[0]",   0);
            TestIF_File.iContactWarningCount[1]=CheckAndReadIniData(szDir, "HeadCondition",       "HandContactWarningSet[1]",   0);
        }
    }
    else
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE ||
           CosFunction.bHiSiliconFunction==true ||                              //Ifor 20190927 : add
           CosFunction.bUseHeadContactCount==true)
        {

            int iActiveCond=-1;                                                 //AI(ht9045-config) 20260728 (RogerYang) : 非Life Time 1~3頁不可落到iPage=0誤寫Group1
    if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))   // golden cStartCondition.cpp:915-915 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:915-915 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
            if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))
#endif // GATE (S12-C save)
                iActiveCond=0;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))   // golden cStartCondition.cpp:917-917 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:917-917 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
            else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))
#endif // GATE (S12-C save)
                iActiveCond=1;
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))   // golden cStartCondition.cpp:919-919 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:919-919 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
            else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))
#endif // GATE (S12-C save)
                iActiveCond=2;

            if(iActiveCond>=0)
            {
                iPage=iActiveCond;
                str1.sprintf("HeadContactSet[%d]", iPage+1);
                iSetCount=atoi(EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text.c_str());
                if(iSetCount>InputLimit.iContactCntAlm)                         //JerryYang 20200504 銦片計數限制
                    iSetCount=InputLimit.iContactCntAlm;
                else if(iSetCount<0)
                    iSetCount=0;
                EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=iSetCount;

                TestIF_File.iContactAlarmCount[iPage]=iSetCount;
                WriteIniData(szDirIndium, "HeadCondition", str1, iSetCount);    //AI(ht9045-config) 20260517 (RogerYang) : VTEST銦片寫入跟Recipe

                for(int iArm=0; iArm<2; iArm++)                                 //AI(ht9045-config) 20260728 (RogerYang) : 一鍵套用:比照sgSocketCount,把單一設定值寫進本頁所有實際存在的head,後續由回讀寫進ContactSet
                {
                    for(int iPos=0; iPos<16; iPos++)
                    {
                        int iRowGrid=g_iHeadRowMap[iArm][iPos];
                        if(iRowGrid<=0)                                         //該head未使用,不套
                            continue;

                        StrGrid[iPage]->Cells[1][iRowGrid]=" "+AnsiString(iSetCount);
                    }
                }
            }
//            if(pgLifeTime->ActivePage==tsCondition01)
//                iPage=0;
//            else if(pgLifeTime->ActivePage==tsCondition02)
//                iPage=1;
//            else if(pgLifeTime->ActivePage==tsCondition03)
//                iPage=2;
//            str1.sprintf("HeadContactSet[%d]", iPage+1);
//            iSetCount=atoi(editContactCountAlarm->Text.c_str());
//            if(iSetCount>InputLimit.iContactCntAlm)                             //JerryYang 20200504 銦片計數限制
//                iSetCount=InputLimit.iContactCntAlm;
//            else if(iSetCount<0)
//                iSetCount=0;
//            editContactCountAlarm->Text=iSetCount;
//
//            TestIF_File.iContactAlarmCount[iPage]=iSetCount;
//            WriteIniData(szDirIndium, "HeadCondition", str1, iSetCount);         //AI(ht9045-config) 20260517 (RogerYang) : VTEST銦片寫入跟Recipe
        }

        if((CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI) &&               //Steven 20200211 : Add socket count for ASE-CL
    SC_pgLifeTimeActivePage!=EL<TTabSheet>("TfStartCondition", "tsSocketCount") &&   // golden cStartCondition.cpp:967-967 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:967-967 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
            EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage!=EL<TTabSheet>("TfStartCondition", "tsSocketCount") &&                            //AI(ht9045-config) 20260728 (RogerYang) : 加頁面限制,避免在Socket Count頁把socket設定值寫進HeadContactSet[4]
#endif // GATE (S12-C save)
    SC_pgLifeTimeActivePage!=EL<TTabSheet>("TfStartCondition", "TabsSocketID"))   // golden cStartCondition.cpp:968-968 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:968-968 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
            EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage!=EL<TTabSheet>("TfStartCondition", "TabsSocketID"))
#endif // GATE (S12-C save)
        {
            WriteIniData(szDir, "HeadCondition", "HeadContactSet[4]", atoi(EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text.c_str()));
        }

        if(CosFunction.bUseSocketContactCount &&                                //AI(ht9045-config) 20260728 (RogerYang) : Socket Count頁的alarm setting原本無寫入路徑,SocketContactSet恆為0導致超壽命警報從不觸發
    SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))   // golden cStartCondition.cpp:974-974 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:974-974 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
           EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))
#endif // GATE (S12-C save)
        {
            int iSocketAlm=atoi(EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text.c_str());
            if(iSocketAlm>InputLimit.iContactCntAlm)
                iSocketAlm=InputLimit.iContactCntAlm;
            else if(iSocketAlm<0)
                iSocketAlm=0;
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=iSocketAlm;

            for(int i=0; i<TestSocket.iShtRow; i++)                             //單一設定值套用到所有實際存在的socket
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                    EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[1][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(iSocketAlm);
            }
        }

        if(CosFunction.bUseSocketContactCount &&                                //AI(ht9045-config) 20260728 (RogerYang) : Socket ID頁原本只顯示不寫入,HeadContactSet[4]永遠存不進去
    SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))   // golden cStartCondition.cpp:991-991 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:991-991 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
           EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))
#endif // GATE (S12-C save)
        {
            int iSktIDAlm=atoi(EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text.c_str());
            if(iSktIDAlm>InputLimit.iContactCntAlm)
                iSktIDAlm=InputLimit.iContactCntAlm;
            else if(iSktIDAlm<0)
                iSktIDAlm=0;
            EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=iSktIDAlm;

            TestIF_File.iContactAlarmCount[3]=iSktIDAlm;
            WriteIniData(szDir, "HeadCondition", "HeadContactSet[4]", iSktIDAlm);
        }


        WriteIniData(szDir, "HeadCondition", "Kit No 1", EL<TEdit>("TfStartCondition", "edtKitNo1")->Text);
        WriteIniData(szDir, "HeadCondition", "Kit No 2", EL<TEdit>("TfStartCondition", "edtKitNo2")->Text);
        WriteIniData(szDir, "HeadCondition", "Kit No 3", EL<TEdit>("TfStartCondition", "edtKitNo3")->Text);
    }

    for(int iTag=0; iTag<3; iTag++)
    {
        for(int iArm=0; iArm<2; iArm++)                                         //AI(ht9045-config) 20260728 (RogerYang) : 全客戶統一by-arm,列算式與FormShow/refill一致
        {
            for(int iPos=0; iPos<16; iPos++)
            {
                int iRowGrid=g_iHeadRowMap[iArm][iPos];                         //AI(ht9045-config) 20260728 (RogerYang) : 列號查FormShow建好的映射,不重算(FTestSuck中途變動會讓列號漂移把值寫進錯槽位)
                if(iRowGrid<=0)                                                 //該head未使用,grid沒填,不可回讀成0把保留槽清掉
                    continue;

                if(bRead==false)                                                //AI(ht9045-config) 20260728 (RogerYang) : 讀取路徑不可用grid覆寫ContactSet;ReadFile()可能在畫面從未開啟時呼叫
                    IniConfig.ContactSet[iTag][iArm][iPos]=atoi(StrGrid[iTag]->Cells[1][iRowGrid].c_str());

                if(IniConfig.HeadContactCount[iTag][iArm][iPos]!=atoi(StrGrid[iTag]->Cells[2][iRowGrid].c_str()))         //JerryYang 20200214 add change log
                {
                    str2.sprintf("%s Arm%d Head %d Contact",  IniConfig.ContactConditionName[iTag], iArm+1, iPos+1);
                    str3.sprintf("%d -> %s", IniConfig.HeadContactCount[iTag][iArm][iPos], StrGrid[iTag]->Cells[2][iRowGrid]);
                    RecordChangeLogProcess(str2, str3);
                }

                if(bRead==false)                                                 //RogerYang 20260203 : 修正formshow裡的fSetup->ReadFile()又調用ReadWriteFile()會造成銦片計數異常
                {
                    IniConfig.HeadContactCount[iTag][iArm][iPos]=atoi(StrGrid[iTag]->Cells[2][iRowGrid].c_str());
                }
            }
        }
    }

    TestIF_File.InOutArmLifeCntSet=ReadWriteIni(szDir, "O_Count", "O_20InOutArmLifeCntSet", atoi(EL<TEdit>("TfStartCondition", "edtInOutArmPickerAlmCnt")->Text.c_str()), 100000, bRead);        //RogerYang 20260205 : 修正設定值異常

    if(bRead==false && g_bHeadRowMapReady)                                      //AI(ht9045-config) 20260728 (RogerYang) : 映射未建立時不可判斷哪些head未使用,否則會把32槽全當未使用而誤補
    {
        for(int t=0; t<3; t++)
        {
            int iDefSet=0;                                                      //取該組條件第一個已設定的head當預設,不依賴已隱藏的單一輸入框
            for(int p=0; p<16 && iDefSet<=0; p++)
            {
                if(g_iHeadRowMap[0][p]>0 && IniConfig.ContactSet[t][0][p]>0)
                    iDefSet=IniConfig.ContactSet[t][0][p];
            }

            if(iDefSet<=0)
                continue;                                                       //整組都還沒設定過就不補

            for(int a=0; a<2; a++)
            {
                for(int p=0; p<16; p++)
                {
                    if(g_iHeadRowMap[0][p]<=0 &&
                       IniConfig.ContactSet[t][a][p]<=0)                        //只補還沒設定過的,不覆蓋既有個別設定
                    {
                        IniConfig.ContactSet[t][a][p]=iDefSet;
                    }
                }
            }
        }
    }

    for(int i=0; i<2; i++)                                                      //JerryYang 20220331 : add 吸真空次數計數
    {
        for(int j=0; j<4; j++ )
        {
            str1.sprintf("InArmSuckCnt%d_%d", i, j);
            str2.sprintf("OutArmSuckCnt%d_%d", i, j);
            TestIF_File.InArmPickerLifeCnt[i][j] =ReadWriteIni(szDir, "O_Count", str1, atoi(edtInArmPickLifeCnt[i][j]->Text.c_str()),  0, bRead);
            TestIF_File.OutArmPickerLifeCnt[i][j]=ReadWriteIni(szDir, "O_Count", str2, atoi(edtOutArmPickLifeCnt[i][j]->Text.c_str()), 0, bRead);
        }
    }

    for(int i=0; i<2; i++)                                                      //JerryYang 20220331 : add 吸真空次數計數
    {
        for(int j=0; j<8; j++ )
        {
            str1.sprintf("Arm1SuckCnt%d_%d", i, j);
            str2.sprintf("Arm2SuckCnt%d_%d", i, j);
            TestIF_File.Arm1PickerLifeCnt[i][j]=ReadWriteIni(szDir, "O_Count", str1, atoi(edtArm1PickLifeCnt[i][j]->Text.c_str()), 0, bRead);
            TestIF_File.Arm2PickerLifeCnt[i][j]=ReadWriteIni(szDir, "O_Count", str2, atoi(edtArm2PickLifeCnt[i][j]->Text.c_str()), 0, bRead);
        }
    }

}

// golden cStartCondition.cpp:1091  TfStartCondition::sbHeadCondition1SaveClick(TObject *Sender)
static void SC_sbHeadCondition1SaveClick(TObject *Sender)
{
    filerw::ELMark("sbHeadCondition1SaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cStartCondition.cpp:1098-1098 取代：Close()：A02 權限不足時關表單 → 記 closed（FileRW/StartCondition.cpp 照 VCL 接著跑 FormClose）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1098-1098 -- Close()：A02 權限不足時關表單 → 記 closed（FileRW/StartCondition.cpp 照 VCL 接著跑 FormClose）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    SC_ReadWriteStartCondition(false);

    AnsiString str2, str3, str4;

    for(int i=0; i<TestSocket.iShtRow; i++)                                     //AI(ht9045-config) 20260728 (RogerYang) : 原i<4/j<8會讀到FormShow已清空且隱藏的控制項,把未使用site的Socket ID與計數洗掉;改成與FormShow顯示範圍一致
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(LastSet.iSocketContactCount[i][j]!=atoi(SocketContactCnt[i][j]->Caption.c_str()))                        //JerryYang 20200214 add change log
            {
                str2.sprintf("%s Contact ccount",  LastSet.strSocketID[i][j]);
                str3.sprintf("%d -> %d", LastSet.iSocketContactCount[i][j], atoi(SocketContactCnt[i][j]->Caption.c_str()));
                RecordChangeLogProcess(str2, str3);
            }

            str2=AnsiString(LastSet.strSocketID[i][j]);
            str3=SocketID[i][j]->Text;

            if(str2!=str3)
            {
                str4.sprintf("Socket ID %s -> %s",  str2, str3);
                RecordChangeLogProcess(str2, str4);
            }
            strncpy(LastSet.strSocketID[i][j], SocketID[i][j]->Text.c_str(), sizeof(LastSet.strSocketID[i][j]));
            LastSet.iSocketContactCount[i][j]=atoi(SocketContactCnt[i][j]->Caption.c_str());
        }
    }

    SC_DoIniDataToForm();

    for(int i=0; i<TestSocket.iShtRow; i++)                                     //AI(ht9045-config) 20260728 (RogerYang) : 原i<4/j<8且row用i*8+j,與FormShow的i*iShtCol+j不一致,2x2時Ba/Bb被歸零
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            int iRowGrid=1+(i*TestSocket.iShtCol+j);
            IniConfig.SocketContactSet[i][j]        =atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[1][iRowGrid].c_str());
            IniConfig.SocketContactCount[i][j]      =atoi(EL<filerw::ELStringGrid>("TfStartCondition", "sgSocketCount")->Cells[2][iRowGrid].c_str());
        }
    }

}

// golden cStartCondition.cpp:1179  TfStartCondition::DoIniDataToForm()
static void SC_DoIniDataToForm()
{

    for(int i=0; i<TestSocket.iShtRow; i++)                                     //JerryYang 20170206
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            SocketID[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
            SocketContactCnt[i][j]->Caption=AnsiString(LastSet.iSocketContactCount[i][j]);
        }
    }

    for(int i=0; i<TestSocket.iMaxRow; i++)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            fLotInfo->edSocket[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
        }
    }

}

// golden cStartCondition.cpp:1199  TfStartCondition::pgLifeTimeChange(TObject *Sender)
static void SC_pgLifeTimeChange()
{

    if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))   // golden cStartCondition.cpp:1201-1201 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1201-1201 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition01"))
#endif // GATE (S12-C save)
    {
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[0]);
    }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))   // golden cStartCondition.cpp:1205-1205 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1205-1205 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition02"))
#endif // GATE (S12-C save)
    {
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[1]);
    }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))   // golden cStartCondition.cpp:1209-1209 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1209-1209 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsCondition03"))
#endif // GATE (S12-C save)
    {
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[2]);
    }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))   // golden cStartCondition.cpp:1213-1213 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1213-1213 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "TabsSocketID"))                               //Steven 20200211 : Add socket count for ASE-CL
#endif // GATE (S12-C save)
    {
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
    }
    else if(SC_pgLifeTimeActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))   // golden cStartCondition.cpp:1217-1217 取代：自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
#if 0 // GATE (S12-C save) golden cStartCondition.cpp:1217-1217 -- 自動：pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）
    else if(EL<TPageControl>("TfStartCondition", "pgLifeTime")->ActivePage==EL<TTabSheet>("TfStartCondition", "tsSocketCount"))                              //AI(ht9045-config) 20260728 (RogerYang) : Socket Count頁顯示目前設定值
#endif // GATE (S12-C save)
    {
        EL<TEdit>("TfStartCondition", "editContactCountAlarm")->Text=AnsiString(IniConfig.SocketContactSet[0][0]);
    }

}

// golden cStartCondition.cpp:1260  TfStartCondition::ClearPickerCount(int iArea, int iTag)
static void SC_ClearPickerCount(int iArea, int iTag)
{
                                                                               //iArea -1: all area, 0: in arm, 1: out arm, 2: Index arm 1, 3: Index arm 2
    TEdit *edtInArmPickLifeCnt[8]={EL<TEdit>("TfStartCondition", "edtInArmA"), EL<TEdit>("TfStartCondition", "edtInArmC"), EL<TEdit>("TfStartCondition", "edtInArmE"), EL<TEdit>("TfStartCondition", "edtInArmG"),
                                   EL<TEdit>("TfStartCondition", "edtInArmB"), EL<TEdit>("TfStartCondition", "edtInArmD"), EL<TEdit>("TfStartCondition", "edtInArmF"), EL<TEdit>("TfStartCondition", "edtInArmH")};

    TEdit *edtOutArmPickLifeCnt[8]={EL<TEdit>("TfStartCondition", "edtOutArmA"), EL<TEdit>("TfStartCondition", "edtOutArmC"), EL<TEdit>("TfStartCondition", "edtOutArmE"), EL<TEdit>("TfStartCondition", "edtOutArmG"),
                                    EL<TEdit>("TfStartCondition", "edtOutArmB"), EL<TEdit>("TfStartCondition", "edtOutArmD"), EL<TEdit>("TfStartCondition", "edtOutArmF"), EL<TEdit>("TfStartCondition", "edtOutArmH")};

    TEdit *edtArm1PickLifeCnt[16]={EL<TEdit>("TfStartCondition", "edtArm1Aa"), EL<TEdit>("TfStartCondition", "edtArm1Ab"), EL<TEdit>("TfStartCondition", "edtArm1Ac"), EL<TEdit>("TfStartCondition", "edtArm1Ad"), EL<TEdit>("TfStartCondition", "edtArm1Ae"), EL<TEdit>("TfStartCondition", "edtArm1Af"), EL<TEdit>("TfStartCondition", "edtArm1Ag"), EL<TEdit>("TfStartCondition", "edtArm1Ah"),
                                   EL<TEdit>("TfStartCondition", "edtArm1Ba"), EL<TEdit>("TfStartCondition", "edtArm1Bb"), EL<TEdit>("TfStartCondition", "edtArm1Bc"), EL<TEdit>("TfStartCondition", "edtArm1Bd"), EL<TEdit>("TfStartCondition", "edtArm1Be"), EL<TEdit>("TfStartCondition", "edtArm1Bf"), EL<TEdit>("TfStartCondition", "edtArm1Bg"), EL<TEdit>("TfStartCondition", "edtArm1Bh")};

    TEdit *edtArm2PickLifeCnt[16]={EL<TEdit>("TfStartCondition", "edtArm2Aa"), EL<TEdit>("TfStartCondition", "edtArm2Ab"), EL<TEdit>("TfStartCondition", "edtArm2Ac"), EL<TEdit>("TfStartCondition", "edtArm2Ad"), EL<TEdit>("TfStartCondition", "edtArm2Ae"), EL<TEdit>("TfStartCondition", "edtArm2Af"), EL<TEdit>("TfStartCondition", "edtArm2Ag"), EL<TEdit>("TfStartCondition", "edtArm2Ah"),
                                   EL<TEdit>("TfStartCondition", "edtArm2Ba"), EL<TEdit>("TfStartCondition", "edtArm2Bb"), EL<TEdit>("TfStartCondition", "edtArm2Bc"), EL<TEdit>("TfStartCondition", "edtArm2Bd"), EL<TEdit>("TfStartCondition", "edtArm2Be"), EL<TEdit>("TfStartCondition", "edtArm2Bf"), EL<TEdit>("TfStartCondition", "edtArm2Bg"), EL<TEdit>("TfStartCondition", "edtArm2Bh")};

    if(iArea==0)                                                                //0: in arm
    {
        edtInArmPickLifeCnt[iTag]->Text="0";
    }
    else if(iArea==1)                                                           //1: out arm
    {
        edtOutArmPickLifeCnt[iTag]->Text="0";
    }
    else if(iArea==2)                                                           //2: Index arm 1
    {
        edtArm1PickLifeCnt[iTag]->Text="0";
    }
    else if(iArea==3)                                                           //3: Index arm 2
    {
        edtArm2PickLifeCnt[iTag]->Text="0";
    }
    else
    {
        for(int i=0; i<8; i++)
        {
            edtInArmPickLifeCnt[i]->Text="0";
            edtOutArmPickLifeCnt[i]->Text="0";
        }

        for(int i=0; i<16; i++)
        {
            edtArm1PickLifeCnt[i]->Text="0";
            edtArm2PickLifeCnt[i]->Text="0";
        }
    }

}

// golden cStartCondition.cpp:1306  TfStartCondition::btnInArmAClick(TObject *Sender)
static void SC_btnInArmAClick(TObject *Sender)
{

    TButton *Ptr;
    Ptr=(TButton *) Sender;
    SC_ClearPickerCount(0, Ptr->Tag);

}

// golden cStartCondition.cpp:1313  TfStartCondition::btnOutArmAClick(TObject *Sender)
static void SC_btnOutArmAClick(TObject *Sender)
{

    TButton *Ptr;
    Ptr=(TButton *) Sender;
    SC_ClearPickerCount(1, Ptr->Tag);

}

// golden cStartCondition.cpp:1320  TfStartCondition::btnArm2AaClick(TObject *Sender)
static void SC_btnArm2AaClick(TObject *Sender)
{

    TButton *Ptr;
    Ptr=(TButton *) Sender;
    SC_ClearPickerCount(3, Ptr->Tag);

}

// golden cStartCondition.cpp:1327  TfStartCondition::btnArm1AaClick(TObject *Sender)
static void SC_btnArm1AaClick(TObject *Sender)
{

    TButton *Ptr;
    Ptr=(TButton *) Sender;
    SC_ClearPickerCount(2, Ptr->Tag);

}

// golden cStartCondition.cpp:1334  TfStartCondition::WritePickerCount()
static void SC_WritePickerCount()
{

    if(IniConfig.bO20InOutArmPickerLifeTimeCount==false)
        return;

    AnsiString S="";
    AnsiString str1, str2, str3;
    S=GetLastOpenFN();
    AnsiString szDir="";

    if(IniConfig.bD05_1SaveSocketCntByHandler)                                  //Steven 20250807 : By handler save contact count
    {
        szDir="D:\\HT9045\\IniData\\SocketCount.ini";
    }
    else
    {
        szDir=GetRecipeFileName("HandlerCondition.Data");
    }

//    TestIF_File.InOutArmLifeCntSet=atoi(edtInOutArmPickerAlmCnt->Text.c_str()); //RogerYang 20260205 : 設定值不需要一直寫入
//    WriteIniData(szDir, "O_Count", "O_20InOutArmLifeCntSet", TestIF_File.InOutArmLifeCntSet);

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++ )
        {
            str1.sprintf("InArmSuckCnt%d_%d", i, j);
            str2.sprintf("OutArmSuckCnt%d_%d", i, j);
            WriteIniData(szDir, "O_Count", str1, TestIF_File.InArmPickerLifeCnt[i][j]);
            WriteIniData(szDir, "O_Count", str2, TestIF_File.OutArmPickerLifeCnt[i][j]);
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++ )
        {
            str1.sprintf("Arm1SuckCnt%d_%d", i, j);
            str2.sprintf("Arm2SuckCnt%d_%d", i, j);
            WriteIniData(szDir, "O_Count", str1, TestIF_File.Arm1PickerLifeCnt[i][j]);
            WriteIniData(szDir, "O_Count", str2, TestIF_File.Arm2PickerLifeCnt[i][j]);
        }
    }

}

// golden cStartCondition.cpp:1390  TfStartCondition::btnClearAaClick(TObject *Sender)
static void SC_btnClearAaClick(TObject *Sender)
{

    TButton *Ptr;
    Ptr=(TButton *) Sender;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<8; j++ )
        {
            if(ClearCntButton[i][j]->Tag==Ptr->Tag)
            {
                LastSet.iSocketContactCount[i][j]=0;
                SocketContactCnt[i][j]->Caption="0";
            }
        }
    }

}

