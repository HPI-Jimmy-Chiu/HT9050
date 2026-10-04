// 產生檔 -- tools/gen_formbridge.py（Steven 20260924，S12 第二型）。不要手改：改產生器的 FORMS 後重跑。
// ---------------------------------------------------------------------------
//  結構 HotPlateForm_File 的寫檔 bridge。一個結構一支 cpp，裡面每個 BCB 表單一組函式（namespace f_<Class>）。
//  來源一律是 golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy 的原檔（cp950 → UTF-8），不看移植樹的表單。
//  規則：widget->Prop = v  ->  J.SetProp("widget", v)；讀值 -> J.GetProp("widget")；
//        控制流程、右式、WriteIniData 原樣保留。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "JsonBridge/FormBridge.h"
#include "cprod.h"
#include "Config.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "forms/fMain.h"
#include "forms/fHotPlate.h"
#include "LastSet.h"
#include "csystem.h"
#include "cAuthority.h"
#include "forms/fSecurity.h"
#include "aHotPlateSubstrate.h"
#include "ainarm_SearchPlacePlate.h"
#include "FileRW/CfgTrayPlate.h"
#include "BarcodeReader.h"
#include "FileRW/MainClickTail.h"

namespace ht9045 {
namespace formbridge {

// ===========================================================================
//  BCB 表單 TfHotPlate  ——  golden cHotPlate.cpp（widget 名單取自 golden cHotPlate.h，29 個）
//  頁面 Setup.HotPlate.html
//  寫檔 HotPlate.Data
// ===========================================================================
namespace f_TfHotPlate {

// 前置宣告（golden 的方法之間互相呼叫）
static void B_FormShow(FormState& J);
static void B_DoIniDataToForm(FormState& J);
static void B_spbSaveClick(FormState& J);
static void B_SaveSetupFile(FormState& J, AnsiString szDir, AnsiString S);
static void B_cbSelectHPFromDBChange(FormState& J);

// golden cHotPlate.cpp:36  TfHotPlate::FormShow(TObject *Sender)
static void B_FormShow(FormState& J)
{

    AnsiString S;
    /* golden: LoadImage() —— 示意圖，HTML 自己有 */
    fHotPlate->ReadFile();
    B_DoIniDataToForm(J);
    S.sprintf("Hot Plate  ''%s''  ",GetLastOpenFN());
    /* golden（視窗屬性，HTML 不用）: Caption=S; */
    J.SetItemIndex("cbSelectHPFromDB", 0);
    J.ItemsClear("cbSelectHPFromDB");
    J.SetText("cbSelectHPFromDB", AnsiString("Select from Database..."));

    if(bHasPlateCSV)                                                            //Steven 20210629 : Plate Form改成CSV
    {
        TfConfigurationTrayPlate *fConfiguration=W906_CfgTrayPlate();   //AI(W906-FRW-S98) 20260926: golden 全域 fConfiguration 的 Plate 表（FileRW/CfgTrayPlate.cpp）；同 C 路 #define 的接法，見 tools/formbridge/TfHotPlate.py
        fConfiguration->sbtReloadHP->Click();
        for(int i=0; i<fConfiguration->strngrdHP->RowCount; i++)
        {
            if(fConfiguration->strngrdHP->Cells[0][i]!="" &&
               fConfiguration->strngrdHP->Cells[0][i]!=" ")
            {
                J.ItemsAdd("cbSelectHPFromDB", fConfiguration->strngrdHP->Cells[0][i]);
            }
        }
    }
    J.M("fShow")=true;

    if(LastSet.iLanguageCountry==1)
        J.SetCaption("Label8", AnsiString("Note : 需要Clean Out後,才能改變Hotplate資料"));
    else
        J.SetCaption("Label8", AnsiString("Note : Need Clean Out Can Change data"));

    /* golden（視窗屬性，HTML 不用）: Top=10; */
    /* golden（視窗屬性，HTML 不用）: Left=200; */

    if(IniConfig.bHotPlateMove1CM)                                              //jou 2010-12-15
    {
        J.SetVisible("chkUseWideHotplate", true);
    }
    else
    {
        J.SetVisible("chkUseWideHotplate", false);
    }

   if(USE_ROTATE_KIT==1 && iRotate_Type==eCynRotate)                            //kevin 20130722  氣缸版
    {
        if(iRotate_In_Index==eripHotPlate1)
        {
            J.SetEnabled("cbEnableHP1", false);
            J.SetChecked("cbEnableHP1", false);
        }
        else if(iRotate_In_Index==eripHotPlate2)
        {
            J.SetEnabled("cbEnableHP2", false);
            J.SetChecked("cbEnableHP2", false);
        }
    }

    if(HasICUnderHotPlate())                                                    //Steven 20110826 : 不使用HasIcUnderMachine
    {
        J.SetEnabled("GroupBox1", false);
        J.SetEnabled("GroupBox2", false);
        J.SetEnabled("Panel1", false);
    }
    else                                                                        //jou 981207 權限控制
    {
        if(CUSTOMER_CODE==CC_SCC &&
           IniConfig.bEnableRms &&
           AccessLevel<=iDefEngineerLevel)                                      //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            J.SetEnabled("GroupBox1", false);
            J.SetEnabled("GroupBox2", false);
            J.SetEnabled("Panel1", false);
        }
        else if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)     //JerryYang 20220311 : ATP鎖定Critical parameter
        {
            if(bAuthCriticalPara[19])
            {
                J.SetEnabled("GroupBox1", false);
                J.SetEnabled("GroupBox2", false);
                J.SetEnabled("Panel1", false);
            }
        }
        else
        {
            J.SetEnabled("GroupBox1", fSecurity->Insufficient(15, false));
            J.SetEnabled("GroupBox2", fSecurity->Insufficient(15, false));
            J.SetEnabled("Panel1", fSecurity->Insufficient(15, false));
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //20140320 wei   KYEC 低於權限顯示不能修改
    {
        if(AccessLevel<LevelSet.AccessLevel[15])
        {
            J.SetEnabled("GroupBox2", false);
            J.SetEnabled("GroupBox1", false);
            J.SetEnabled("Panel1", false);
        }
        else
        {
            J.SetEnabled("GroupBox2", true);
            J.SetEnabled("GroupBox1", true);
            J.SetEnabled("Panel1", true);
        }
    }

    J.SetVisible("chkTrayHotplateCheck", (IniConfig.bVTESTFunction==true));             //jou 20240126 : Tray & hotplate by recipe MES控制檢查

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cHotPlate.cpp:311  TfHotPlate::DoIniDataToForm()
static void B_DoIniDataToForm(FormState& J)
{

    J.SetText("HotPlateName", AnsiString(HotPlateForm_File.Alias));

    J.SetText("XST1", AnsiString(FormatFloat("0.000", HotPlateForm_File.XStart)));    //Steven 20160513 : 0.00 --> 0.0000 避免自動四捨五入
    J.SetText("YST1", AnsiString(FormatFloat("0.000", HotPlateForm_File.YStart)));
    J.SetText("XPitch1", AnsiString(FormatFloat("0.000", HotPlateForm_File.XPitch)));
    J.SetText("YPitch1", AnsiString(FormatFloat("0.000", HotPlateForm_File.YPitch)));

    J.SetText("XCT1", AnsiString(HotPlateForm_File.XDivision));
    J.SetText("YCT1", AnsiString(HotPlateForm_File.YDivision));

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {                                                                           //ChungHung 20131120 AutoClean use Hotplate1
        if(TestIF_File.iAutoClean_Function &&
           IniConfig.bE43AutoCleanUseHotplate)
        {
            J.SetVisible("cbEnableHP1", false);
        }
        else
        {
            J.SetVisible("cbEnableHP1", true);
        }
    }
    else
    {
        if(IniConfig.bE43AutoCleanUseHotplate)
        {
            J.SetVisible("cbEnableHP1", false);
        }
        else
        {
            J.SetVisible("cbEnableHP1", true);
        }
    }

    if(USE_PRECISER==1 && iPreciserInstallArea==2)                              //Ifor 20191008 : add Preciser Install Area
    {
        J.SetVisible("cbEnableHP1", false);
    }

    J.SetChecked("cbEnableHP2", HotPlateForm_File.iPlateSelect&0x02);
    J.SetChecked("cbEnableHP1", HotPlateForm_File.iPlateSelect&0x01);

    if(IniConfig.bHotPlateMove1CM)                                              //jou 2010-12-15
    {
        J.SetChecked("chkUseWideHotplate", HotPlateForm_File.bUseWideHotplate);
    }
    else
    {
        J.SetChecked("chkUseWideHotplate", true);
    }

    if(IniConfig.bVTESTFunction==true)
        J.SetChecked("chkTrayHotplateCheck", HotPlateForm_File.bTrayHotplateCheck);     //jou 20240126 : Tray & hotplate by recipe MES控制檢查

}

// golden cHotPlate.cpp:441  TfHotPlate::spbSaveClick(TObject *Sender)
static void B_spbSaveClick(FormState& J)
{

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        J.Message("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        J.M("closed")=1;   /* golden: Close() */
    // ---- golden cHotPlate.cpp:447-447 整段覆寫（見 gen_formbridge.py FORMS.blocks）----
    return;   //AI(W906-E031) 20261003 [W906] (St01): Q78 handler #20, golden text kept (comment only). (1) golden 906 0618 cHotPlate.cpp:446: the A02 branch (RogerYang 20260305 [A01_2], :442-447) only calls Close() -- no return, so it closes and goes on to the HP check and SaveSetupFile (:449-460); fHotPlate is ShowModal (0618 main.cpp:27431), Close() only sets ModalResult => the values the operator changed are still written. (2) golden V912 cHotPlate.cpp:447 `return;` kept => closed, nothing saved. (3) #20 exception (Steven 1003 standing rule; Q78)
    }

    if(J.GetChecked("cbEnableHP1")==false && J.GetChecked("cbEnableHP2")==false)
    {
        J.Message("Need select at least one hotplate.", "請至少選擇一個加熱盤。");
        J.SetChecked("cbEnableHP2", true);
        return;
    }
    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);

    B_SaveSetupFile(J, szDir, S);                                                    //kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(S, "JOBFILE");                                 //kevin 20150105  Start 另存 jobfile
    #endif
    AddSpace(szDir);
    fHotPlate->ReadFile();
    J.SetDown("spbSave", false);
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }
    fMain->BackupSetupFile(); J.Todo("fMain->BackupSetupFile() is a no-op in the port (forms/fMain.cpp:458) -- no auto backup");                                                   //Ifor 20170508 (wei) add Auto BackUp Setup File & Last Data
    J.Todo("golden spbSaveClick: fMain->ChangeATCSiteUse() has no body in the port (GATE W906-HOME-W1-ATCSITE)");                                                  //Ztex 2024.09.06 Add Hot Plate ChangeATCSiteUse

}

// golden cHotPlate.cpp:481  TfHotPlate::SaveSetupFile(AnsiString szDir, AnsiString S)
static void B_SaveSetupFile(FormState& J, AnsiString szDir, AnsiString S)
{
    J.M("saved")=1;   // bridge：golden 存檔函式有被呼叫（form.save 的 ack.saved）

    int flag=0;

    int iXItem=atoi(J.GetText("XCT1").c_str());
    int iXPitch1=atoi(J.GetText("XPitch1").c_str());
    int iYPitch1=atoi(J.GetText("YPitch1").c_str());                                   //JerryYang 20230822 : add

    if(USE_PICKER_COUNT==ep1Picker)                                             //JerryYang 20250902 : 單吸嘴模組可以支援X數量3
    {
    }
    else
    {
        if(i8PickerHPMode==iHPWideHP)                                               //JerryYang 20161007 簡化Hotplate判斷式  //Steven 20151117 : 2x2 8Picker at Hot mode
        {                                                                           //ChungHung Add 20101025 CC_ASE_CL HotPlate Offset 10mm
            if(iXItem==12)                                                          //ChungHung 20140421 add 開放 XItem 12
            {
                J.SetText("XCT1", AnsiString(12));
            }
            else
            {
                if(iXItem>8)
                    J.SetText("XCT1", AnsiString(8));
            }
        }
        else
        {
            if(iXItem==12 && (TestIF.iTestMode==QualSite2X2 ||                      //Steven 20180409 (Jou) : Add 2x2 support X=12
                              TestIF.iTestMode==_8Site2X4   ||
                              TestIF.iTestMode==_12Site2X6  ||
                              TestIF.iTestMode==_16Site2X8  ||
                              TestIF.iTestMode==_10Site2X5  ||
                              TestIF.iTestMode==_16Site4X4  ||                      //Sam 20190226 : 16Site4X4
                              TestIF.iTestMode==_32Site4X8N))
            {
                J.SetText("XCT1", AnsiString(12));
            }
            else if(iXItem==16 && (TestIF.iTestMode==_8Site2X4  ||
                                   TestIF.iTestMode==_12Site2X6 ||
                                   TestIF.iTestMode==_16Site2X8 ||
                                   TestIF.iTestMode==_16Site4X4 ||                  //Sam 20190226 : 16Site4X4
                                   TestIF.iTestMode==_32Site4X8N))                  //Steven 20150826 : 16x24 Hot Plate for 32Site
            {
                J.SetText("XCT1", AnsiString(16));
            }
            else
            {
                if(iXItem>8)
                    J.SetText("XCT1", AnsiString(8));
            }
        }

        if(!(TestIF.iTestMode==DualSite ||                                          //20111015 Dell For Korea
             TestIF.iTestMode==QualSite2X2N))                                       //Frank 20200520 2X2NN Mode
        {
            if(TestIF_File.iTestMode==SingleSite    ||
               TestIF_File.iTestMode==TriSite1X3    ||                              //Steven 20241126 : Add 1x3 for 3x7 HP
               TestIF_File.iTestMode==DualSite2x1   ||                              //Steven 20211228 : add for 2x1 mode
               TestIF_File.iTestMode==_6Site2X3N    ||                              //Steven 20220425 : 2X3NN Mode
               TestIF_File.iTestMode==_8Site2X4N)                                   //Wei 20231211 : 2X4NN Mode
            {
                ;
            }
            else if(TestIF.iTestMode==_8Site2X4 ||                                  //Ifor 20151006 Add
                    TestIF.iTestMode==_16Site4X4)                                   //Sam 20190226 : 16Site4X4
            {
                if(ArmCanSuck4IC(0)==false)
                {
                    if(iXItem==3)
                    {
                        J.SetText("XCT1", AnsiString(2));
                        iXPitch1=iXPitch1*2;
                        J.SetText("XPitch1", AnsiString(iXPitch1));
                    }
                }
                else
                {
                    if(iXItem==6 && iYPitch1==20)                                   //JerryYang 20230822 : add
                    {
                        J.SetText("XCT1", AnsiString(4));
                        J.Message("Not support X-division: 6 (y-pitch: 20)");
                    }
                }
            }
            else
            {
                if(iXItem==3)
                    J.SetText("XCT1", AnsiString(2));
            }
        }
    }

    MyForceDirectories(szDir);
    if(J.GetChecked("cbEnableHP2"))
        flag+=2;
    if(J.GetChecked("cbEnableHP1"))
        flag+=1;

//    LastSet.iRunStartMode=rsmInitialStart;                                    //jou 2012-01-17 make code,Save會讓Auto Tray疊料 //Eloit 2010_0819
//    fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode;

    int iYStart=atof(J.GetText("YST1").c_str())*100;
    if(USE_IN_Y_IS_AUTO_PITCH==false && iYStart<=1100)                                //Steven 20141203 : Hand Pitch 63.5遇到Y-Start=10且不能一次放時B排行程不足  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(TestIF.iTestMode>=QualSite2X2 && HotPlateYPitchCanPutAll()==false)
        {
            J.SetText("YCT1", AnsiString(AnsiString(atoi(J.GetText("YCT1").c_str())-1)));
            J.SetText("YST1", AnsiString(AnsiString(atof(J.GetText("YST1").c_str())+atof(J.GetText("YPitch1").c_str()))));
        }
    }

    szDir+="\\HotPlate.Data";

//    if(atof(XPitch1->Text.c_str())==26.67)
//    {
//        XPitch1->Text="26.66";
//    }

    WriteIniData(szDir, "Hotplate Form", "Name",       J.GetText("HotPlateName"));
    WriteIniData(szDir, "Hotplate Form", "X Start",    FormatFloat("0.000", J.GetText("XST1").ToDouble()));    //Steven 20220801 : 存檔時候要指定小數點位數
    WriteIniData(szDir, "Hotplate Form", "Y Start",    FormatFloat("0.000", J.GetText("YST1").ToDouble()));
    WriteIniData(szDir, "Hotplate Form", "X Pitch",    FormatFloat("0.000", J.GetText("XPitch1").ToDouble()));
    WriteIniData(szDir, "Hotplate Form", "Y Pitch",    FormatFloat("0.000", J.GetText("YPitch1").ToDouble()));
    WriteIniData(szDir, "Hotplate Form", "X Division", J.GetText("XCT1"));
    WriteIniData(szDir, "Hotplate Form", "Y Division", J.GetText("YCT1"));
    WriteIniData(szDir, "Hotplate Form", "Using Flag", flag);

    if(IniConfig.bHotPlateMove1CM)                                              //jou 2010-12-15
    {
        if(atoi(J.GetText("XCT1").c_str())==8 &&                                       //jou 2012-08-22 開啟右移1CM & 2x2 & 8支吸嘴 & Hotplate X=8 & X Pitch > 26.6 Hang up，須強制將Use Wide Hotplate=true
           TestIF.iUseSuckMode==8 &&
           TestIF.iTestMode==QualSite2X2 &&
           atof(J.GetText("XPitch1").c_str())>=26.67)
        {
            J.SetChecked("chkUseWideHotplate", true);
        }

        WriteIniData(szDir, "Hotplate Form", "Use Wide Hotplate", J.GetChecked("chkUseWideHotplate"));
    }
    else
    {
        WriteIniData(szDir, "Hotplate Form", "Use Wide Hotplate", true);
    }

    if(IniConfig.bVTESTFunction==true)
        WriteIniData(szDir, "System", "bTrayHotplateCheck", J.GetChecked("chkTrayHotplateCheck"));    //jou 20240126 : Tray & hotplate by recipe MES控制檢查

}

// golden cHotPlate.cpp:413  TfHotPlate::cbSelectHPFromDBChange(TObject *Sender)
static void B_cbSelectHPFromDBChange(FormState& J)
{

    if(J.M("fShow")==false)                                                            //Steven 20140401 : 避免讀取HotPlate資料庫
        return;

    // ---- golden cHotPlate.cpp:417-420 整段覆寫（見 gen_formbridge.py FORMS.blocks）----
    if(Barcode_Reader(bcPlateForm)==0) { J.Todo("golden cHotPlate.cpp:417 Barcode_Reader(bcPlateForm) returned 0 (KYEC operator-ID prompt; the port input box is an offline shell) -- golden returns without filling"); return; }
    /* golden :421 TComboBox *ComboBox=(TComboBox *)Sender;  —— Sender＝cbSelectHPFromDB（OnChange 只掛在它，cHotPlate.dfm:322） */
    int index=J.GetItemIndex("cbSelectHPFromDB");   // golden :422 ComboBox->ItemIndex

    if(bHasPlateCSV)                                                            //Steven 20210629 : Plate Form改成CSV
    {
        if(index<1)
            return;

        TfConfigurationTrayPlate *fConfiguration=W906_CfgTrayPlate();   //AI(W906-FRW-S157) 20260927 [W906]: golden 全域 fConfiguration 的 Plate 表（FileRW/CfgTrayPlate.cpp），同 FormShow :49 的接法
        fConfiguration->sbtReloadHP->Click();
        J.SetText("HotPlateName", AnsiString(fConfiguration->strngrdHP->Cells[0][index].Trim())); //"Package Type"
        J.SetText("XST1", AnsiString(fConfiguration->strngrdHP->Cells[1][index].Trim())); //"X Start Pos"
        J.SetText("YST1", AnsiString(fConfiguration->strngrdHP->Cells[2][index].Trim())); //"Y Start Pos"
        J.SetText("XPitch1", AnsiString(fConfiguration->strngrdHP->Cells[3][index].Trim())); //"X Pitch"
        J.SetText("YPitch1", AnsiString(fConfiguration->strngrdHP->Cells[4][index].Trim())); //"Y Pitch"
        J.SetText("XCT1", AnsiString(fConfiguration->strngrdHP->Cells[5][index].Trim())); //"Columns (X)"
        J.SetText("YCT1", AnsiString(fConfiguration->strngrdHP->Cells[6][index].Trim())); //"Rows (Y)"
    }

}

static void Display(FormState& J)
{
    B_FormShow(J);
}

static void Save(FormState& J, AnsiString szDir, AnsiString S)
{
    B_SaveSetupFile(J, szDir, S);
}

static void SaveFlow(FormState& J)
{
    B_spbSaveClick(J);
    if (J.M("closed")) fHotPlate->ReadFile();   //AI(W906-FRW-S158) 20260927 [W906]: golden FormClose（cHotPlate.cpp:401）的 :404 ReadFile（A02 Close() 之後；DoIniDataToForm／fShow 是這次請求的 J，不跑）
    if (J.M("closed") || J.M("saved")) { if (const char* w = ::W906_Main_sbPlateFormClickTail()) J.Todo(w); const char* e_ = 0; const char* z_ = 0; const char* t_ = 0; while (::W906_Main_Hp3TailTakeMessage(&e_, &z_)) J.Message(AnsiString(e_), AnsiString(z_)); while (::W906_Main_Hp3TailTakeTodo(&t_)) J.Todo(t_); }   //AI(W906-FRW-S158) 20260927 [W906]: golden TfMain::sbPlateFormClick 關窗尾段 V912 main.cpp:28422-28425（FileRW/MainClick.cpp；R85／S107-1、R86）  AI(W906-EVB6) 20260928 [W906]: R107 尾段裡 golden LoadAutoCleanData 的 ShowMyMessage／todo 放進存檔回覆（ack.messages／ack.todo）
}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/formbridge/TfHotPlate.py 的 events）。
//   chain＝golden DFM 的控制項自己＋每一層容器與設計期 Enabled／Visible（RunEvent 判斷使用者點不點得到）
static const EventGuard kGuard0[] = {{"cbSelectHPFromDB", true, true}, {"GroupBox2", true, true}};
static const EventDesc kEvents[] = {
    {"cbSelectHPFromDB", "change", "cHotPlate.cpp:412 TfHotPlate::cbSelectHPFromDBChange", &B_cbSelectHPFromDBChange, kGuard0, 2},
};

}  // namespace f_TfHotPlate

extern const BridgeDesc kBridge_TfHotPlate = {
    "Setup.HotPlate.html", "TfHotPlate", "cHotPlate.cpp",
    &f_TfHotPlate::Display, &f_TfHotPlate::Save, &f_TfHotPlate::SaveFlow,
    // SaveSetupFile 會讀到的 widget（11 個）：頁面少送任何一個就整筆拒寫
    "XCT1,XPitch1,YPitch1,cbEnableHP2,cbEnableHP1,YST1,YCT1,HotPlateName,XST1,chkUseWideHotplate,chkTrayHotplateCheck",
    "",
    // AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（1 個）＋golden header 的元件名
    f_TfHotPlate::kEvents, (int)(sizeof(f_TfHotPlate::kEvents) / sizeof(f_TfHotPlate::kEvents[0])),
    "GroupBox1,GroupBox2,HotPlateName,Image1,Image2,Image3,Label1,Label2,Label3,Label4,Label5,Label6,Label7,Label8,Panel1,Panel2,XCT1,XPitch1,XST1,YCT1,YPitch1,YST1,cbEnableHP1,cbEnableHP2,cbSelectHPFromDB,chkTrayHotplateCheck,chkUseWideHotplate,sbtExit,spbSave"
};

}  // namespace formbridge
}  // namespace ht9045
