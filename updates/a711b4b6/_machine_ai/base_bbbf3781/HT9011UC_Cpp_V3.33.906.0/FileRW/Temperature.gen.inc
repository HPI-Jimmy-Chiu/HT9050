// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Temperature 的讀寫檔（HTEditList： → Temperature.Data、Tester.Data、DefineTemp\*.Data、Config\ATC.ini）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden uTemp_Set.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 319 個（header 宣告 858 個）。
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
#include "forms/fLotInfo.h"
#include "forms/fTemp_Set.h"
#include "forms/fATCHandlerSide.h"
#include "forms/fHS.h"
#include "forms/fDynamicTemp.h"
#include "atester_shims.h"
#include "ATC/ATCInterface.h"
#include "TempCtrl/TriTemp.h"
#include "database.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h；cAuthority.h 與 HTEditList.h 衝突）
void GetLimitAuth();                  // cAuthority.h:77
// csystem.h／cinitial.h／canary_support.h 帶進 aHotPlateSubstrate.h 或另一份 LAST_GENERAL_SET → 只前置宣告
bool HasICUnderMachine();             // csystem.h:105
bool HasICUnderHotPlate();            // csystem.h:110
bool ShuttleHasIC();                  // csystem.h:135
bool IndexHasIC();                    // csystem.h:140
void InitialHeaterDoor();             // cinitial.h:42
void RecordProcess(AnsiString S, AnsiString S2="");   // canary_support.h:70（golden cMyDB.h:63）
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr=false, AnsiString errPart=" ");   // canary_support.h:66
// acarry_shims.h 帶進 aHotPlateSubstrate.h → 照抄同一個類別定義（ODR：逐 token 相同）
class TATC_InterfaceFormShim
{
public:
    int iATC_MODE_TYPE;
    TATC_InterfaceFormShim();
};
extern TATC_InterfaceFormShim *ATC_InterfaceForm;   // acarry_shims.h:115
// golden uTemp_Set.cpp:72-93 檔案層級全域（移植樹 uTemp_Set.cpp:248-270 定義、fTemp_Set->Init() 填）
extern TEdit *ATCOffsetEdit[32];
extern TEdit *ATCPackageOffsetEdit[3];
extern TEdit *ATCPackageTempEdit[3];
extern TEdit *ATC_FFCOffsetOnTimeEdit[2][10];
extern TEdit *ATC_FFCOffsetOffTimeEdit[2][10];
extern TEdit *ATC_FFCOffsetEdit[2][10];
extern TCheckBox *ATC_FFCPointUse[2][10];
extern TStringList *sATC_CH_Tj;
extern TCheckBox *ZoneTempUse[4];
extern TEdit *ZoneTempSetting[4];
extern TCheckBox *MultiSensorOffsetUse[4];
extern TEdit *ATC_MultiSensorOffsetEdit[32];
extern TfDynamicTemp *fDynamicTemp;   // DynamicTemp.cpp:58
// golden FormHS->CheckTempOffset（HS_Function.cpp:3958）：移植樹 forms/fHS.cpp:804 TFormHS::CheckTempOffset 有本體（純門檻比較＋WAR15194），
// 但全域名 FormHS 被 Automation/SCK_ART_Remainder.h:629 的 stub 佔走 → 本 TU 自己的一個實例（函式內 static 狀態照舊）
static TFormHS* TS_FormHS() { static TFormHS* p = new TFormHS(); return p; }
// golden cprod.cpp:3818-3823（CustomerFunctionSelect）決定 rgIndexHeatMode 各選項的 Visible；vclcompat TRadioGroup 沒有 Controls[]
static bool TS_IndexHeatModeItemVisible(int i) {
    switch(i) {
    case HeadOnly:          return true;
    case ChamberOnly:       return (ATC_SYSTEM==eATCUninstall);
    case HeadChamber:       return (ATC_SYSTEM==eATCUninstall && IniConfig.bNoHeadaddChamberOption==false);
    case SocketChamber:     return (ATC_SYSTEM==eATCUninstall);
    case HeadSocket:        return (IniConfig.bHeadSocketMode);
    case HeadChamberSocket: return (ATC_SYSTEM==eATCUninstall && IniConfig.bHeadChamberSocketMode);
    default:                return true;   // 之後 Items->Add 的選項（VCL 新建 radio button 預設 Visible）
    }
}
// golden cTesterIF.cpp:563-570（路徑）＋:713-779（[InitialMode]）—— FTestIF->ReadTestIFFile() 的子集（其餘 GATE F-5）
static void TS_ReadTestIFFile_InitialMode()
{
        AnsiString S="";   // golden cTesterIF.cpp:566
        S=GetLastOpenFN();   // golden cTesterIF.cpp:567
        AnsiString szDir="";//, szDir2;   // golden cTesterIF.cpp:568
        szDir=DataPath+S;   // golden cTesterIF.cpp:569
        szDir+="\\Tester.Data";   // golden cTesterIF.cpp:570
        TestIF_File.bEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",          false);   // golden cTesterIF.cpp:713
    
        if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail   // golden cTesterIF.cpp:715
            TestIF_File.bUseOtherArmToTestAfterInitialDelay     =ReadIniData(szDir, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",       false);   // golden cTesterIF.cpp:716
        else   // golden cTesterIF.cpp:717
            TestIF_File.bUseOtherArmToTestAfterInitialDelay     =false;   // golden cTesterIF.cpp:718
    
        TestIF_File.bAfterShowAlarmMessageUseInitialDelay       =ReadIniData(szDir, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",     false);   // golden cTesterIF.cpp:720
        TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay   =ReadIniData(szDir, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay", false);   // golden cTesterIF.cpp:721
        TestIF_File.iEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",          1.00);   // golden cTesterIF.cpp:722
    
        #ifdef SOFT_SIMULTE                                                         //Steven 20241108 : 軟體模擬有做五秒就好
        TestIF_File.iInitialDelay                               =5;   // golden cTesterIF.cpp:725
        TestIF_File.iInitialDelay_2                             =5;   // golden cTesterIF.cpp:726
        TestIF_File.iInitialDelay_3                             =5;   // golden cTesterIF.cpp:727
        TestIF_File.iInitialDelay_4                             =5;   // golden cTesterIF.cpp:728
        TestIF_File.iInitialDelay_5                             =5;   // golden cTesterIF.cpp:729
        TestIF_File.iInitialDelay_6                             =5;   // golden cTesterIF.cpp:730
        TestIF_File.iInitialDelay_7                             =5;   // golden cTesterIF.cpp:731
        TestIF_File.iInitialDelay_8                             =5;   // golden cTesterIF.cpp:732
        TestIF_File.iInitialDelay_9                             =5;   // golden cTesterIF.cpp:733
        TestIF_File.dInitialDelay_10                            =5;   // golden cTesterIF.cpp:734
        #else
        TestIF_File.iInitialDelay                               =ReadIniData(szDir, "InitialMode",       "iInitialDelay",            1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:736
        TestIF_File.iInitialDelay_2                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_2",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:737
        TestIF_File.iInitialDelay_3                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_3",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:738
        TestIF_File.iInitialDelay_4                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_4",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:739
        TestIF_File.iInitialDelay_5                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_5",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:740
        TestIF_File.iInitialDelay_6                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_6",          1.00);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:741
        TestIF_File.iInitialDelay_7                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_7",          1.00);     //wei 20151228 No FullSite delay   // golden cTesterIF.cpp:742
        TestIF_File.iInitialDelay_8                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_8",          1.00);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:743
        TestIF_File.iInitialDelay_9                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_9",          1.00);     //Steven 20160818 : OTD打開Delay   // golden cTesterIF.cpp:744
        TestIF_File.dInitialDelay_10                            =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10",         0.0);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41 wait time   // golden cTesterIF.cpp:745
        #endif
    
        if(CosFunction.bHiSiliconFunction && CUSTOMER_CODE==CC_ASE_KaohSiung)       //kevin 20191227 add  Initial Start Delay Time 需卡下限, 最小30秒.   // golden cTesterIF.cpp:748
        {   // golden cTesterIF.cpp:749
            if(TestIF_File.dInitStartDelayTime<30)                                  //kevin 20200110   // golden cTesterIF.cpp:750
            {   // golden cTesterIF.cpp:751
                TestIF_File.dInitStartDelayTime=30;   // golden cTesterIF.cpp:752
                TestIF_File.iInitialDelay=30;   // golden cTesterIF.cpp:753
            }   // golden cTesterIF.cpp:754
        }   // golden cTesterIF.cpp:755
    
        TestIF_File.bOTDUnlockDelay                             =ReadIniData(szDir, "InitialMode",       "bOTDUnlockDelay",                             false);     //Steven 20160818 : OTD打開Delay   // golden cTesterIF.cpp:757
        TestIF_File.bWhenPressStopOverUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           false);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:758
        TestIF_File.iWhenPressStopOver                          =ReadIniData(szDir, "InitialMode",       "iWhenPressStopOver",                          1.00);      //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:759
        TestIF_File.bWhenNoFullSiteUseInitialDelay              =ReadIniData(szDir, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              false);     //wei 20151228 No FullSite delay   // golden cTesterIF.cpp:760
        TestIF_File.bAfterAutoCleanFunctionUseInitialDelay      =ReadIniData(szDir, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      false);   // golden cTesterIF.cpp:761
        TestIF_File.bTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "bTestFinishToNextTestOver",                   false);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:762
        TestIF_File.iTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "iTestFinishToNextTestOver",                   1.00);      //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:763
        TestIF_File.bAfterOpenHeatDoorUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           false);     //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time   // golden cTesterIF.cpp:764
        TestIF_File.bTestStartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "bTestStartToNextTestStart",                   false);     //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41   // golden cTesterIF.cpp:765
        TestIF_File.dTeststartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "dTeststartToNextTestStart",                   1.00);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41  over time   // golden cTesterIF.cpp:766
    
        //Steven 20190313 : Initial Start Delay use different setting in FT and RT   // golden cTesterIF.cpp:768
        //==>   // golden cTesterIF.cpp:769
        TestIF_File.dInitialDelay_1_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_1_RT",              0.0);   // golden cTesterIF.cpp:770
        TestIF_File.dInitialDelay_2_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_2_RT",              0.0);   // golden cTesterIF.cpp:771
        TestIF_File.dInitialDelay_3_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_3_RT",              0.0);   // golden cTesterIF.cpp:772
        TestIF_File.dInitialDelay_4_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_4_RT",              0.0);   // golden cTesterIF.cpp:773
        TestIF_File.dInitialDelay_5_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_5_RT",              0.0);   // golden cTesterIF.cpp:774
        TestIF_File.dInitialDelay_6_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_6_RT",              0.0);   // golden cTesterIF.cpp:775
        TestIF_File.dInitialDelay_7_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_7_RT",              0.0);   // golden cTesterIF.cpp:776
        TestIF_File.dInitialDelay_8_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_8_RT",              0.0);   // golden cTesterIF.cpp:777
        TestIF_File.dInitialDelay_9_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_9_RT",              0.0);   // golden cTesterIF.cpp:778
        TestIF_File.dInitialDelay_10_RT                         =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10_RT",             0.0);   // golden cTesterIF.cpp:779
}
// golden MyTempPanel.h TMyTempPanel 的替身包裝（Steven 團隊 20260925）：golden 程式寫 myTempPal[i]->edBase->Text、
// ->Caption=… —— 成員就是移植樹 fTemp_Set->myTempPal[i]（fTemp_Set->Init()＝golden 建構子 :152 建的）那個物件的元件，
// Caption 轉成移植樹的 SetCaption／GetCaption（golden __property Caption={read=GetCaption, write=SetCaption}）。
struct TS_TMyTempPanel {
    TMyTempPanel* p;
    struct CaptionProp { TMyTempPanel* p; CaptionProp& operator=(const AnsiString& s) { p->SetCaption(s); return *this; } operator AnsiString() const { return p->GetCaption(); } };
    CaptionProp Caption;
    int &iIndexTag, &iOffsetByRecipeMaxLimit, &iOffsetByRecipeMinLimit;
    TPanel *&palTemp, *&palLine; TLabel *&labName;
    TEdit *&edLow, *&edMid, *&edLowbase, *&edBase, *&edHighBase, *&edSHighBase, *&edPreOffset, *&edPreOfsTime, *&edAfterOfs, *&edKit_Low, *&edKit_Mid, *&edKit_Lowbase, *&edKit_Base, *&edKit_HighBase, *&edOffset, *&edSingleLimit, *&edIndiTemp, *&edInitTempOffset, *&edEOTTempOffset;
    void SetEnable(bool b) { p->SetEnable(b); }
    void SetParent(TTabSheet* t) { p->SetParent(t); }
    void SetIndexTag(int t) { p->SetIndexTag(t); }
    explicit TS_TMyTempPanel(TMyTempPanel* q) : p(q), Caption{q}, iIndexTag(q->iIndexTag),
        iOffsetByRecipeMaxLimit(q->iOffsetByRecipeMaxLimit), iOffsetByRecipeMinLimit(q->iOffsetByRecipeMinLimit),
        palTemp(q->palTemp), palLine(q->palLine), labName(q->labName),
        edLow(q->edLow), edMid(q->edMid), edLowbase(q->edLowbase), edBase(q->edBase), edHighBase(q->edHighBase), edSHighBase(q->edSHighBase), edPreOffset(q->edPreOffset), edPreOfsTime(q->edPreOfsTime), edAfterOfs(q->edAfterOfs), edKit_Low(q->edKit_Low), edKit_Mid(q->edKit_Mid), edKit_Lowbase(q->edKit_Lowbase), edKit_Base(q->edKit_Base), edKit_HighBase(q->edKit_HighBase), edOffset(q->edOffset), edSingleLimit(q->edSingleLimit), edIndiTemp(q->edIndiTemp), edInitTempOffset(q->edInitTempOffset), edEOTTempOffset(q->edEOTTempOffset) {}
};
static TS_TMyTempPanel* TS_myTempPal[tcTotalCount] = {};   // golden uTemp_Set.h:1019 TMyTempPanel *myTempPal[tcTotalCount]
static void TS_AdoptPortWidgets() { filerw::ELKeep("TfTemp_Set", "GroupBox1", fTemp_Set->GroupBox1); filerw::ELKeep("TfTemp_Set", "GroupBox3", fTemp_Set->GroupBox3); filerw::ELKeep("TfTemp_Set", "GroupBox4", fTemp_Set->GroupBox4); filerw::ELKeep("TfTemp_Set", "GroupBox5", fTemp_Set->GroupBox5); filerw::ELKeep("TfTemp_Set", "GroupBox6", fTemp_Set->GroupBox6); filerw::ELKeep("TfTemp_Set", "Label1", fTemp_Set->Label1); filerw::ELKeep("TfTemp_Set", "Label10", fTemp_Set->Label10); filerw::ELKeep("TfTemp_Set", "Label109", fTemp_Set->Label109); filerw::ELKeep("TfTemp_Set", "Label11", fTemp_Set->Label11); filerw::ELKeep("TfTemp_Set", "Label110", fTemp_Set->Label110); filerw::ELKeep("TfTemp_Set", "Label111", fTemp_Set->Label111); filerw::ELKeep("TfTemp_Set", "Label112", fTemp_Set->Label112); filerw::ELKeep("TfTemp_Set", "Label113", fTemp_Set->Label113); filerw::ELKeep("TfTemp_Set", "Label114", fTemp_Set->Label114); filerw::ELKeep("TfTemp_Set", "Label115", fTemp_Set->Label115); filerw::ELKeep("TfTemp_Set", "Label12", fTemp_Set->Label12); filerw::ELKeep("TfTemp_Set", "Label13", fTemp_Set->Label13); filerw::ELKeep("TfTemp_Set", "Label14", fTemp_Set->Label14); filerw::ELKeep("TfTemp_Set", "Label15", fTemp_Set->Label15); filerw::ELKeep("TfTemp_Set", "Label16", fTemp_Set->Label16); filerw::ELKeep("TfTemp_Set", "Label17", fTemp_Set->Label17); filerw::ELKeep("TfTemp_Set", "Label18", fTemp_Set->Label18); filerw::ELKeep("TfTemp_Set", "Label19", fTemp_Set->Label19); filerw::ELKeep("TfTemp_Set", "Label2", fTemp_Set->Label2); filerw::ELKeep("TfTemp_Set", "Label20", fTemp_Set->Label20); filerw::ELKeep("TfTemp_Set", "Label21", fTemp_Set->Label21); filerw::ELKeep("TfTemp_Set", "Label22", fTemp_Set->Label22); filerw::ELKeep("TfTemp_Set", "Label23", fTemp_Set->Label23); filerw::ELKeep("TfTemp_Set", "Label24", fTemp_Set->Label24); filerw::ELKeep("TfTemp_Set", "Label25", fTemp_Set->Label25); filerw::ELKeep("TfTemp_Set", "Label26", fTemp_Set->Label26); filerw::ELKeep("TfTemp_Set", "Label27", fTemp_Set->Label27); filerw::ELKeep("TfTemp_Set", "Label28", fTemp_Set->Label28); filerw::ELKeep("TfTemp_Set", "Label29", fTemp_Set->Label29); filerw::ELKeep("TfTemp_Set", "Label3", fTemp_Set->Label3); filerw::ELKeep("TfTemp_Set", "Label30", fTemp_Set->Label30); filerw::ELKeep("TfTemp_Set", "Label31", fTemp_Set->Label31); filerw::ELKeep("TfTemp_Set", "Label32", fTemp_Set->Label32); filerw::ELKeep("TfTemp_Set", "Label33", fTemp_Set->Label33); filerw::ELKeep("TfTemp_Set", "Label34", fTemp_Set->Label34); filerw::ELKeep("TfTemp_Set", "Label35", fTemp_Set->Label35); filerw::ELKeep("TfTemp_Set", "Label36", fTemp_Set->Label36); filerw::ELKeep("TfTemp_Set", "Label37", fTemp_Set->Label37); filerw::ELKeep("TfTemp_Set", "Label371", fTemp_Set->Label371); filerw::ELKeep("TfTemp_Set", "Label372", fTemp_Set->Label372); filerw::ELKeep("TfTemp_Set", "Label373", fTemp_Set->Label373); filerw::ELKeep("TfTemp_Set", "Label374", fTemp_Set->Label374); filerw::ELKeep("TfTemp_Set", "Label375", fTemp_Set->Label375); filerw::ELKeep("TfTemp_Set", "Label376", fTemp_Set->Label376); filerw::ELKeep("TfTemp_Set", "Label38", fTemp_Set->Label38); filerw::ELKeep("TfTemp_Set", "Label39", fTemp_Set->Label39); filerw::ELKeep("TfTemp_Set", "Label4", fTemp_Set->Label4); filerw::ELKeep("TfTemp_Set", "Label40", fTemp_Set->Label40); filerw::ELKeep("TfTemp_Set", "Label41", fTemp_Set->Label41); filerw::ELKeep("TfTemp_Set", "Label42", fTemp_Set->Label42); filerw::ELKeep("TfTemp_Set", "Label43", fTemp_Set->Label43); filerw::ELKeep("TfTemp_Set", "Label44", fTemp_Set->Label44); filerw::ELKeep("TfTemp_Set", "Label45", fTemp_Set->Label45); filerw::ELKeep("TfTemp_Set", "Label46", fTemp_Set->Label46); filerw::ELKeep("TfTemp_Set", "Label47", fTemp_Set->Label47); filerw::ELKeep("TfTemp_Set", "Label48", fTemp_Set->Label48); filerw::ELKeep("TfTemp_Set", "Label49", fTemp_Set->Label49); filerw::ELKeep("TfTemp_Set", "Label5", fTemp_Set->Label5); filerw::ELKeep("TfTemp_Set", "Label50", fTemp_Set->Label50); filerw::ELKeep("TfTemp_Set", "Label51", fTemp_Set->Label51); filerw::ELKeep("TfTemp_Set", "Label52", fTemp_Set->Label52); filerw::ELKeep("TfTemp_Set", "Label53", fTemp_Set->Label53); filerw::ELKeep("TfTemp_Set", "Label54", fTemp_Set->Label54); filerw::ELKeep("TfTemp_Set", "Label55", fTemp_Set->Label55); filerw::ELKeep("TfTemp_Set", "Label56", fTemp_Set->Label56); filerw::ELKeep("TfTemp_Set", "Label57", fTemp_Set->Label57); filerw::ELKeep("TfTemp_Set", "Label577", fTemp_Set->Label577); filerw::ELKeep("TfTemp_Set", "Label578", fTemp_Set->Label578); filerw::ELKeep("TfTemp_Set", "Label58", fTemp_Set->Label58); filerw::ELKeep("TfTemp_Set", "Label59", fTemp_Set->Label59); filerw::ELKeep("TfTemp_Set", "Label6", fTemp_Set->Label6); filerw::ELKeep("TfTemp_Set", "Label60", fTemp_Set->Label60); filerw::ELKeep("TfTemp_Set", "Label61", fTemp_Set->Label61); filerw::ELKeep("TfTemp_Set", "Label62", fTemp_Set->Label62); filerw::ELKeep("TfTemp_Set", "Label63", fTemp_Set->Label63); filerw::ELKeep("TfTemp_Set", "Label64", fTemp_Set->Label64); filerw::ELKeep("TfTemp_Set", "Label65", fTemp_Set->Label65); filerw::ELKeep("TfTemp_Set", "Label66", fTemp_Set->Label66); filerw::ELKeep("TfTemp_Set", "Label67", fTemp_Set->Label67); filerw::ELKeep("TfTemp_Set", "Label68", fTemp_Set->Label68); filerw::ELKeep("TfTemp_Set", "Label69", fTemp_Set->Label69); filerw::ELKeep("TfTemp_Set", "Label699", fTemp_Set->Label699); filerw::ELKeep("TfTemp_Set", "Label7", fTemp_Set->Label7); filerw::ELKeep("TfTemp_Set", "Label70", fTemp_Set->Label70); filerw::ELKeep("TfTemp_Set", "Label700", fTemp_Set->Label700); filerw::ELKeep("TfTemp_Set", "Label701", fTemp_Set->Label701); filerw::ELKeep("TfTemp_Set", "Label702", fTemp_Set->Label702); filerw::ELKeep("TfTemp_Set", "Label703", fTemp_Set->Label703); filerw::ELKeep("TfTemp_Set", "Label704", fTemp_Set->Label704); filerw::ELKeep("TfTemp_Set", "Label705", fTemp_Set->Label705); filerw::ELKeep("TfTemp_Set", "Label706", fTemp_Set->Label706); filerw::ELKeep("TfTemp_Set", "Label707", fTemp_Set->Label707); filerw::ELKeep("TfTemp_Set", "Label708", fTemp_Set->Label708); filerw::ELKeep("TfTemp_Set", "Label709", fTemp_Set->Label709); filerw::ELKeep("TfTemp_Set", "Label71", fTemp_Set->Label71); filerw::ELKeep("TfTemp_Set", "Label710", fTemp_Set->Label710); filerw::ELKeep("TfTemp_Set", "Label711", fTemp_Set->Label711); filerw::ELKeep("TfTemp_Set", "Label712", fTemp_Set->Label712); filerw::ELKeep("TfTemp_Set", "Label713", fTemp_Set->Label713); filerw::ELKeep("TfTemp_Set", "Label714", fTemp_Set->Label714); filerw::ELKeep("TfTemp_Set", "Label715", fTemp_Set->Label715); filerw::ELKeep("TfTemp_Set", "Label716", fTemp_Set->Label716); filerw::ELKeep("TfTemp_Set", "Label717", fTemp_Set->Label717); filerw::ELKeep("TfTemp_Set", "Label718", fTemp_Set->Label718); filerw::ELKeep("TfTemp_Set", "Label719", fTemp_Set->Label719); filerw::ELKeep("TfTemp_Set", "Label72", fTemp_Set->Label72); filerw::ELKeep("TfTemp_Set", "Label720", fTemp_Set->Label720); filerw::ELKeep("TfTemp_Set", "Label721", fTemp_Set->Label721); filerw::ELKeep("TfTemp_Set", "Label722", fTemp_Set->Label722); filerw::ELKeep("TfTemp_Set", "Label723", fTemp_Set->Label723); filerw::ELKeep("TfTemp_Set", "Label724", fTemp_Set->Label724); filerw::ELKeep("TfTemp_Set", "Label725", fTemp_Set->Label725); filerw::ELKeep("TfTemp_Set", "Label726", fTemp_Set->Label726); filerw::ELKeep("TfTemp_Set", "Label727", fTemp_Set->Label727); filerw::ELKeep("TfTemp_Set", "Label728", fTemp_Set->Label728); filerw::ELKeep("TfTemp_Set", "Label729", fTemp_Set->Label729); filerw::ELKeep("TfTemp_Set", "Label73", fTemp_Set->Label73); filerw::ELKeep("TfTemp_Set", "Label730", fTemp_Set->Label730); filerw::ELKeep("TfTemp_Set", "Label731", fTemp_Set->Label731); filerw::ELKeep("TfTemp_Set", "Label732", fTemp_Set->Label732); filerw::ELKeep("TfTemp_Set", "Label733", fTemp_Set->Label733); filerw::ELKeep("TfTemp_Set", "Label734", fTemp_Set->Label734); filerw::ELKeep("TfTemp_Set", "Label735", fTemp_Set->Label735); filerw::ELKeep("TfTemp_Set", "Label736", fTemp_Set->Label736); filerw::ELKeep("TfTemp_Set", "Label737", fTemp_Set->Label737); filerw::ELKeep("TfTemp_Set", "Label738", fTemp_Set->Label738); filerw::ELKeep("TfTemp_Set", "Label739", fTemp_Set->Label739); filerw::ELKeep("TfTemp_Set", "Label74", fTemp_Set->Label74); filerw::ELKeep("TfTemp_Set", "Label740", fTemp_Set->Label740); filerw::ELKeep("TfTemp_Set", "Label741", fTemp_Set->Label741); filerw::ELKeep("TfTemp_Set", "Label742", fTemp_Set->Label742); filerw::ELKeep("TfTemp_Set", "Label743", fTemp_Set->Label743); filerw::ELKeep("TfTemp_Set", "Label744", fTemp_Set->Label744); filerw::ELKeep("TfTemp_Set", "Label745", fTemp_Set->Label745); filerw::ELKeep("TfTemp_Set", "Label746", fTemp_Set->Label746); filerw::ELKeep("TfTemp_Set", "Label747", fTemp_Set->Label747); filerw::ELKeep("TfTemp_Set", "Label748", fTemp_Set->Label748); filerw::ELKeep("TfTemp_Set", "Label749", fTemp_Set->Label749); filerw::ELKeep("TfTemp_Set", "Label75", fTemp_Set->Label75); filerw::ELKeep("TfTemp_Set", "Label750", fTemp_Set->Label750); filerw::ELKeep("TfTemp_Set", "Label751", fTemp_Set->Label751); filerw::ELKeep("TfTemp_Set", "Label752", fTemp_Set->Label752); filerw::ELKeep("TfTemp_Set", "Label753", fTemp_Set->Label753); filerw::ELKeep("TfTemp_Set", "Label754", fTemp_Set->Label754); filerw::ELKeep("TfTemp_Set", "Label755", fTemp_Set->Label755); filerw::ELKeep("TfTemp_Set", "Label756", fTemp_Set->Label756); filerw::ELKeep("TfTemp_Set", "Label757", fTemp_Set->Label757); filerw::ELKeep("TfTemp_Set", "Label758", fTemp_Set->Label758); filerw::ELKeep("TfTemp_Set", "Label79", fTemp_Set->Label79); filerw::ELKeep("TfTemp_Set", "Label8", fTemp_Set->Label8); filerw::ELKeep("TfTemp_Set", "Label80", fTemp_Set->Label80); filerw::ELKeep("TfTemp_Set", "Label803", fTemp_Set->Label803); filerw::ELKeep("TfTemp_Set", "Label804", fTemp_Set->Label804); filerw::ELKeep("TfTemp_Set", "Label822", fTemp_Set->Label822); filerw::ELKeep("TfTemp_Set", "Label824", fTemp_Set->Label824); filerw::ELKeep("TfTemp_Set", "Label83", fTemp_Set->Label83); filerw::ELKeep("TfTemp_Set", "Label883", fTemp_Set->Label883); filerw::ELKeep("TfTemp_Set", "Label884", fTemp_Set->Label884); filerw::ELKeep("TfTemp_Set", "Label885", fTemp_Set->Label885); filerw::ELKeep("TfTemp_Set", "Label9", fTemp_Set->Label9); filerw::ELKeep("TfTemp_Set", "Memo1", fTemp_Set->Memo1); filerw::ELKeep("TfTemp_Set", "Memo2", fTemp_Set->Memo2); filerw::ELKeep("TfTemp_Set", "Memo3", fTemp_Set->Memo3); filerw::ELKeep("TfTemp_Set", "PageControl1", fTemp_Set->PageControl1); filerw::ELKeep("TfTemp_Set", "Panel1", fTemp_Set->Panel1); filerw::ELKeep("TfTemp_Set", "Panel10", fTemp_Set->Panel10); filerw::ELKeep("TfTemp_Set", "Panel11", fTemp_Set->Panel11); filerw::ELKeep("TfTemp_Set", "Panel12", fTemp_Set->Panel12); filerw::ELKeep("TfTemp_Set", "Panel129", fTemp_Set->Panel129); filerw::ELKeep("TfTemp_Set", "Panel13", fTemp_Set->Panel13); filerw::ELKeep("TfTemp_Set", "Panel130", fTemp_Set->Panel130); filerw::ELKeep("TfTemp_Set", "Panel131", fTemp_Set->Panel131); filerw::ELKeep("TfTemp_Set", "Panel132", fTemp_Set->Panel132); filerw::ELKeep("TfTemp_Set", "Panel14", fTemp_Set->Panel14); filerw::ELKeep("TfTemp_Set", "Panel15", fTemp_Set->Panel15); filerw::ELKeep("TfTemp_Set", "Panel16", fTemp_Set->Panel16); filerw::ELKeep("TfTemp_Set", "Panel17", fTemp_Set->Panel17); filerw::ELKeep("TfTemp_Set", "Panel18", fTemp_Set->Panel18); filerw::ELKeep("TfTemp_Set", "Panel19", fTemp_Set->Panel19); filerw::ELKeep("TfTemp_Set", "Panel2", fTemp_Set->Panel2); filerw::ELKeep("TfTemp_Set", "Panel20", fTemp_Set->Panel20); filerw::ELKeep("TfTemp_Set", "Panel21", fTemp_Set->Panel21); filerw::ELKeep("TfTemp_Set", "Panel22", fTemp_Set->Panel22); filerw::ELKeep("TfTemp_Set", "Panel23", fTemp_Set->Panel23); filerw::ELKeep("TfTemp_Set", "Panel24", fTemp_Set->Panel24); filerw::ELKeep("TfTemp_Set", "Panel25", fTemp_Set->Panel25); filerw::ELKeep("TfTemp_Set", "Panel26", fTemp_Set->Panel26); filerw::ELKeep("TfTemp_Set", "Panel27", fTemp_Set->Panel27); filerw::ELKeep("TfTemp_Set", "Panel28", fTemp_Set->Panel28); filerw::ELKeep("TfTemp_Set", "Panel29", fTemp_Set->Panel29); filerw::ELKeep("TfTemp_Set", "Panel3", fTemp_Set->Panel3); filerw::ELKeep("TfTemp_Set", "Panel30", fTemp_Set->Panel30); filerw::ELKeep("TfTemp_Set", "Panel31", fTemp_Set->Panel31); filerw::ELKeep("TfTemp_Set", "Panel32", fTemp_Set->Panel32); filerw::ELKeep("TfTemp_Set", "Panel33", fTemp_Set->Panel33); filerw::ELKeep("TfTemp_Set", "Panel34", fTemp_Set->Panel34); filerw::ELKeep("TfTemp_Set", "Panel35", fTemp_Set->Panel35); filerw::ELKeep("TfTemp_Set", "Panel36", fTemp_Set->Panel36); filerw::ELKeep("TfTemp_Set", "Panel37", fTemp_Set->Panel37); filerw::ELKeep("TfTemp_Set", "Panel38", fTemp_Set->Panel38); filerw::ELKeep("TfTemp_Set", "Panel39", fTemp_Set->Panel39); filerw::ELKeep("TfTemp_Set", "Panel4", fTemp_Set->Panel4); filerw::ELKeep("TfTemp_Set", "Panel40", fTemp_Set->Panel40); filerw::ELKeep("TfTemp_Set", "Panel41", fTemp_Set->Panel41); filerw::ELKeep("TfTemp_Set", "Panel42", fTemp_Set->Panel42); filerw::ELKeep("TfTemp_Set", "Panel43", fTemp_Set->Panel43); filerw::ELKeep("TfTemp_Set", "Panel5", fTemp_Set->Panel5); filerw::ELKeep("TfTemp_Set", "Panel6", fTemp_Set->Panel6); filerw::ELKeep("TfTemp_Set", "Panel7", fTemp_Set->Panel7); filerw::ELKeep("TfTemp_Set", "Panel8", fTemp_Set->Panel8); filerw::ELKeep("TfTemp_Set", "Panel9", fTemp_Set->Panel9); filerw::ELKeep("TfTemp_Set", "PnlDewPointAlarm", fTemp_Set->PnlDewPointAlarm); filerw::ELKeep("TfTemp_Set", "PnlIndexSoakTime", fTemp_Set->PnlIndexSoakTime); filerw::ELKeep("TfTemp_Set", "RaWriteATCFile", fTemp_Set->RaWriteATCFile); filerw::ELKeep("TfTemp_Set", "RachoiceATCFile", fTemp_Set->RachoiceATCFile); filerw::ELKeep("TfTemp_Set", "bgTempOffset", fTemp_Set->bgTempOffset); filerw::ELKeep("TfTemp_Set", "btClearAll", fTemp_Set->btClearAll); filerw::ELKeep("TfTemp_Set", "btnDefrostEnd", fTemp_Set->btnDefrostEnd); filerw::ELKeep("TfTemp_Set", "btnDefrostStart", fTemp_Set->btnDefrostStart); filerw::ELKeep("TfTemp_Set", "btnSameAsArm1", fTemp_Set->btnSameAsArm1); filerw::ELKeep("TfTemp_Set", "btnSort", fTemp_Set->btnSort); filerw::ELKeep("TfTemp_Set", "btn_DefrostAllUseEnd", fTemp_Set->btn_DefrostAllUseEnd); filerw::ELKeep("TfTemp_Set", "btn_DefrostAllUseStart", fTemp_Set->btn_DefrostAllUseStart); filerw::ELKeep("TfTemp_Set", "cb3SigmaTempMonitior_Enable", fTemp_Set->cb3SigmaTempMonitior_Enable); filerw::ELKeep("TfTemp_Set", "cbATCHeaterGun", fTemp_Set->cbATCHeaterGun); filerw::ELKeep("TfTemp_Set", "cbATCReferTempSensor", fTemp_Set->cbATCReferTempSensor); filerw::ELKeep("TfTemp_Set", "cbATCTempRise", fTemp_Set->cbATCTempRise); filerw::ELKeep("TfTemp_Set", "cbATCTestTimeOffset", fTemp_Set->cbATCTestTimeOffset); filerw::ELKeep("TfTemp_Set", "cbAfterAutoCleanFunctionUseInitialDelay", fTemp_Set->cbAfterAutoCleanFunctionUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbAfterOpenHeatDoorUseInitialDelay", fTemp_Set->cbAfterOpenHeatDoorUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbAfterShowAlarmMessageUseInitialDelay", fTemp_Set->cbAfterShowAlarmMessageUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbBlowLB", fTemp_Set->cbBlowLB); filerw::ELKeep("TfTemp_Set", "cbByPassChamber", fTemp_Set->cbByPassChamber); filerw::ELKeep("TfTemp_Set", "cbEnableATCConsFailOffset", fTemp_Set->cbEnableATCConsFailOffset); filerw::ELKeep("TfTemp_Set", "cbEnableATCQAModeOffset", fTemp_Set->cbEnableATCQAModeOffset); filerw::ELKeep("TfTemp_Set", "cbEnableIndividualMode", fTemp_Set->cbEnableIndividualMode); filerw::ELKeep("TfTemp_Set", "cbEnableTJFunction", fTemp_Set->cbEnableTJFunction); filerw::ELKeep("TfTemp_Set", "cbEnableTesterDryAirControl", fTemp_Set->cbEnableTesterDryAirControl); filerw::ELKeep("TfTemp_Set", "cbEveryFirstDeviceUseInitialDelay", fTemp_Set->cbEveryFirstDeviceUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbFFCOffsetEnable", fTemp_Set->cbFFCOffsetEnable); filerw::ELKeep("TfTemp_Set", "cbHeaterGun", fTemp_Set->cbHeaterGun); filerw::ELKeep("TfTemp_Set", "cbLBTempHighAlarmEnable", fTemp_Set->cbLBTempHighAlarmEnable); filerw::ELKeep("TfTemp_Set", "cbLBTempLowAlarmEnable", fTemp_Set->cbLBTempLowAlarmEnable); filerw::ELKeep("TfTemp_Set", "cbMultiSensorEnable1", fTemp_Set->cbMultiSensorEnable1); filerw::ELKeep("TfTemp_Set", "cbMultiSensorEnable2", fTemp_Set->cbMultiSensorEnable2); filerw::ELKeep("TfTemp_Set", "cbMultiSensorEnable3", fTemp_Set->cbMultiSensorEnable3); filerw::ELKeep("TfTemp_Set", "cbMultiSensorEnable4", fTemp_Set->cbMultiSensorEnable4); filerw::ELKeep("TfTemp_Set", "cbMultiSensorOffsetEnable", fTemp_Set->cbMultiSensorOffsetEnable); filerw::ELKeep("TfTemp_Set", "cbMultiZoneFunction", fTemp_Set->cbMultiZoneFunction); filerw::ELKeep("TfTemp_Set", "cbOTDUnlockDelay", fTemp_Set->cbOTDUnlockDelay); filerw::ELKeep("TfTemp_Set", "cbPowerFollow_Enable", fTemp_Set->cbPowerFollow_Enable); filerw::ELKeep("TfTemp_Set", "cbSelectDevice", fTemp_Set->cbSelectDevice); filerw::ELKeep("TfTemp_Set", "cbShowFixTemp", fTemp_Set->cbShowFixTemp); filerw::ELKeep("TfTemp_Set", "cbTempOffset", fTemp_Set->cbTempOffset); filerw::ELKeep("TfTemp_Set", "cbTestFinishToNextTestOver", fTemp_Set->cbTestFinishToNextTestOver); filerw::ELKeep("TfTemp_Set", "cbTestStartToNextTestStart", fTemp_Set->cbTestStartToNextTestStart); filerw::ELKeep("TfTemp_Set", "cbTjWatchdog", fTemp_Set->cbTjWatchdog); filerw::ELKeep("TfTemp_Set", "cbUseAbitCHK", fTemp_Set->cbUseAbitCHK); filerw::ELKeep("TfTemp_Set", "cbUseAmbGuard", fTemp_Set->cbUseAmbGuard); filerw::ELKeep("TfTemp_Set", "cbUseFixTemp", fTemp_Set->cbUseFixTemp); filerw::ELKeep("TfTemp_Set", "cbUseOtherArmToTestAfterInitialDelay", fTemp_Set->cbUseOtherArmToTestAfterInitialDelay); filerw::ELKeep("TfTemp_Set", "cbUseTC2Offset", fTemp_Set->cbUseTC2Offset); filerw::ELKeep("TfTemp_Set", "cbWaitDewPoint", fTemp_Set->cbWaitDewPoint); filerw::ELKeep("TfTemp_Set", "cbWhenHappenTestedTimeBelowUseInitialDelay", fTemp_Set->cbWhenHappenTestedTimeBelowUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbWhenNoFullSiteUseInitialDelay", fTemp_Set->cbWhenNoFullSiteUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbWhenPressStopOverUseInitialDelay", fTemp_Set->cbWhenPressStopOverUseInitialDelay); filerw::ELKeep("TfTemp_Set", "cbZoneEnable1", fTemp_Set->cbZoneEnable1); filerw::ELKeep("TfTemp_Set", "cbZoneEnable2", fTemp_Set->cbZoneEnable2); filerw::ELKeep("TfTemp_Set", "cbZoneEnable3", fTemp_Set->cbZoneEnable3); filerw::ELKeep("TfTemp_Set", "cbZoneEnable4", fTemp_Set->cbZoneEnable4); filerw::ELKeep("TfTemp_Set", "cb_EnableAirMachineSocket", fTemp_Set->cb_EnableAirMachineSocket); filerw::ELKeep("TfTemp_Set", "cbbATC_RecipeFile", fTemp_Set->cbbATC_RecipeFile); filerw::ELKeep("TfTemp_Set", "cdATCUsePIDCtrl", fTemp_Set->cdATCUsePIDCtrl); filerw::ELKeep("TfTemp_Set", "chkAmbUsingAFan", fTemp_Set->chkAmbUsingAFan); filerw::ELKeep("TfTemp_Set", "chkBoostFunction", fTemp_Set->chkBoostFunction); filerw::ELKeep("TfTemp_Set", "chkBoostOffset", fTemp_Set->chkBoostOffset); filerw::ELKeep("TfTemp_Set", "chkChamberBoost", fTemp_Set->chkChamberBoost); filerw::ELKeep("TfTemp_Set", "chkLBInitOffset", fTemp_Set->chkLBInitOffset); filerw::ELKeep("TfTemp_Set", "chkLBTempFunction", fTemp_Set->chkLBTempFunction); filerw::ELKeep("TfTemp_Set", "chkSLKNoHeatUp", fTemp_Set->chkSLKNoHeatUp); filerw::ELKeep("TfTemp_Set", "chkShuttleNoHeatUp", fTemp_Set->chkShuttleNoHeatUp); filerw::ELKeep("TfTemp_Set", "chkSoakTimeReduce", fTemp_Set->chkSoakTimeReduce); filerw::ELKeep("TfTemp_Set", "chkTempCalByRecipe", fTemp_Set->chkTempCalByRecipe); filerw::ELKeep("TfTemp_Set", "chkTempErrorAction", fTemp_Set->chkTempErrorAction); filerw::ELKeep("TfTemp_Set", "chkTesterDock", fTemp_Set->chkTesterDock); filerw::ELKeep("TfTemp_Set", "chkTriTempEnableBase", fTemp_Set->chkTriTempEnableBase); filerw::ELKeep("TfTemp_Set", "chkTriTempEnableIndex", fTemp_Set->chkTriTempEnableIndex); filerw::ELKeep("TfTemp_Set", "chkTriTempEnableOutSht", fTemp_Set->chkTriTempEnableOutSht); filerw::ELKeep("TfTemp_Set", "chkUseCDAOnly", fTemp_Set->chkUseCDAOnly); filerw::ELKeep("TfTemp_Set", "chk_EnableArm_1_Air", fTemp_Set->chk_EnableArm_1_Air); filerw::ELKeep("TfTemp_Set", "chk_EnableArm_2_Air", fTemp_Set->chk_EnableArm_2_Air); filerw::ELKeep("TfTemp_Set", "chk_EnableSocket_Air", fTemp_Set->chk_EnableSocket_Air); filerw::ELKeep("TfTemp_Set", "chk_IndexHeaterDoor", fTemp_Set->chk_IndexHeaterDoor); filerw::ELKeep("TfTemp_Set", "chk_UseTj", fTemp_Set->chk_UseTj); filerw::ELKeep("TfTemp_Set", "edATCAmbTemp", fTemp_Set->edATCAmbTemp); filerw::ELKeep("TfTemp_Set", "edATCContFailOffset1", fTemp_Set->edATCContFailOffset1); filerw::ELKeep("TfTemp_Set", "edATCContFailOffset2", fTemp_Set->edATCContFailOffset2); filerw::ELKeep("TfTemp_Set", "edATCContFailOffset3", fTemp_Set->edATCContFailOffset3); filerw::ELKeep("TfTemp_Set", "edATCContFailOffset4", fTemp_Set->edATCContFailOffset4); filerw::ELKeep("TfTemp_Set", "edATCContFailOffsetCnt", fTemp_Set->edATCContFailOffsetCnt); filerw::ELKeep("TfTemp_Set", "edATCInitialOffset1", fTemp_Set->edATCInitialOffset1); filerw::ELKeep("TfTemp_Set", "edATCInitialOffset2", fTemp_Set->edATCInitialOffset2); filerw::ELKeep("TfTemp_Set", "edATCInitialOffset3", fTemp_Set->edATCInitialOffset3); filerw::ELKeep("TfTemp_Set", "edATCInitialOffset4", fTemp_Set->edATCInitialOffset4); filerw::ELKeep("TfTemp_Set", "edATCOfsTime", fTemp_Set->edATCOfsTime); filerw::ELKeep("TfTemp_Set", "edATCQAModeOffset1", fTemp_Set->edATCQAModeOffset1); filerw::ELKeep("TfTemp_Set", "edATCQAModeOffset2", fTemp_Set->edATCQAModeOffset2); filerw::ELKeep("TfTemp_Set", "edATCQAModeOffset3", fTemp_Set->edATCQAModeOffset3); filerw::ELKeep("TfTemp_Set", "edATCQAModeOffset4", fTemp_Set->edATCQAModeOffset4); filerw::ELKeep("TfTemp_Set", "edATCTestTimeOffset", fTemp_Set->edATCTestTimeOffset); filerw::ELKeep("TfTemp_Set", "edATCTestTimeOffset1", fTemp_Set->edATCTestTimeOffset1); filerw::ELKeep("TfTemp_Set", "edATCTestTimeOffset2", fTemp_Set->edATCTestTimeOffset2); filerw::ELKeep("TfTemp_Set", "edATCTestTimeOffset3", fTemp_Set->edATCTestTimeOffset3); filerw::ELKeep("TfTemp_Set", "edATCTestTimeOffset4", fTemp_Set->edATCTestTimeOffset4); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S1_TC1", fTemp_Set->edATC_Arm1_S1_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S1_TC2", fTemp_Set->edATC_Arm1_S1_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S1_TC3", fTemp_Set->edATC_Arm1_S1_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S1_TC4", fTemp_Set->edATC_Arm1_S1_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S2_TC1", fTemp_Set->edATC_Arm1_S2_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S2_TC2", fTemp_Set->edATC_Arm1_S2_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S2_TC3", fTemp_Set->edATC_Arm1_S2_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S2_TC4", fTemp_Set->edATC_Arm1_S2_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S3_TC1", fTemp_Set->edATC_Arm1_S3_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S3_TC2", fTemp_Set->edATC_Arm1_S3_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S3_TC3", fTemp_Set->edATC_Arm1_S3_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S3_TC4", fTemp_Set->edATC_Arm1_S3_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S4_TC1", fTemp_Set->edATC_Arm1_S4_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S4_TC2", fTemp_Set->edATC_Arm1_S4_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S4_TC3", fTemp_Set->edATC_Arm1_S4_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm1_S4_TC4", fTemp_Set->edATC_Arm1_S4_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S1_TC1", fTemp_Set->edATC_Arm2_S1_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S1_TC2", fTemp_Set->edATC_Arm2_S1_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S1_TC3", fTemp_Set->edATC_Arm2_S1_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S1_TC4", fTemp_Set->edATC_Arm2_S1_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S2_TC1", fTemp_Set->edATC_Arm2_S2_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S2_TC2", fTemp_Set->edATC_Arm2_S2_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S2_TC3", fTemp_Set->edATC_Arm2_S2_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S2_TC4", fTemp_Set->edATC_Arm2_S2_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S3_TC1", fTemp_Set->edATC_Arm2_S3_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S3_TC2", fTemp_Set->edATC_Arm2_S3_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S3_TC3", fTemp_Set->edATC_Arm2_S3_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S3_TC4", fTemp_Set->edATC_Arm2_S3_TC4); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S4_TC1", fTemp_Set->edATC_Arm2_S4_TC1); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S4_TC2", fTemp_Set->edATC_Arm2_S4_TC2); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S4_TC3", fTemp_Set->edATC_Arm2_S4_TC3); filerw::ELKeep("TfTemp_Set", "edATC_Arm2_S4_TC4", fTemp_Set->edATC_Arm2_S4_TC4); filerw::ELKeep("TfTemp_Set", "edAbLow", fTemp_Set->edAbLow); filerw::ELKeep("TfTemp_Set", "edAbMid", fTemp_Set->edAbMid); filerw::ELKeep("TfTemp_Set", "edAbitColdTime", fTemp_Set->edAbitColdTime); filerw::ELKeep("TfTemp_Set", "edAbitInitWaitTime", fTemp_Set->edAbitInitWaitTime); filerw::ELKeep("TfTemp_Set", "edAmbGuardband", fTemp_Set->edAmbGuardband); filerw::ELKeep("TfTemp_Set", "edAmbHotGuartbent", fTemp_Set->edAmbHotGuartbent); filerw::ELKeep("TfTemp_Set", "edAmbTemp", fTemp_Set->edAmbTemp); filerw::ELKeep("TfTemp_Set", "edArm1NoFullsiteOffset_1", fTemp_Set->edArm1NoFullsiteOffset_1); filerw::ELKeep("TfTemp_Set", "edArm1NoFullsiteOffset_2", fTemp_Set->edArm1NoFullsiteOffset_2); filerw::ELKeep("TfTemp_Set", "edArm1NoFullsiteOffset_3", fTemp_Set->edArm1NoFullsiteOffset_3); filerw::ELKeep("TfTemp_Set", "edArm1NoFullsiteOffset_4", fTemp_Set->edArm1NoFullsiteOffset_4); filerw::ELKeep("TfTemp_Set", "edArm1NoFullsiteOffset_5", fTemp_Set->edArm1NoFullsiteOffset_5); filerw::ELKeep("TfTemp_Set", "edArm1Offset", fTemp_Set->edArm1Offset); filerw::ELKeep("TfTemp_Set", "edArm2NoFullsiteOffset_1", fTemp_Set->edArm2NoFullsiteOffset_1); filerw::ELKeep("TfTemp_Set", "edArm2NoFullsiteOffset_2", fTemp_Set->edArm2NoFullsiteOffset_2); filerw::ELKeep("TfTemp_Set", "edArm2NoFullsiteOffset_3", fTemp_Set->edArm2NoFullsiteOffset_3); filerw::ELKeep("TfTemp_Set", "edArm2NoFullsiteOffset_4", fTemp_Set->edArm2NoFullsiteOffset_4); filerw::ELKeep("TfTemp_Set", "edArm2NoFullsiteOffset_5", fTemp_Set->edArm2NoFullsiteOffset_5); filerw::ELKeep("TfTemp_Set", "edArm2Offset", fTemp_Set->edArm2Offset); filerw::ELKeep("TfTemp_Set", "edAtcFileName", fTemp_Set->edAtcFileName); filerw::ELKeep("TfTemp_Set", "edBelowSec", fTemp_Set->edBelowSec); filerw::ELKeep("TfTemp_Set", "edChamberCoolTemp", fTemp_Set->edChamberCoolTemp); filerw::ELKeep("TfTemp_Set", "edChillerTemp", fTemp_Set->edChillerTemp); filerw::ELKeep("TfTemp_Set", "edContinuousSec", fTemp_Set->edContinuousSec); filerw::ELKeep("TfTemp_Set", "edDewPointAlarmInterval", fTemp_Set->edDewPointAlarmInterval); filerw::ELKeep("TfTemp_Set", "edDewPointRange", fTemp_Set->edDewPointRange); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_01", fTemp_Set->edFFC_Arm1Enable_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_02", fTemp_Set->edFFC_Arm1Enable_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_03", fTemp_Set->edFFC_Arm1Enable_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_04", fTemp_Set->edFFC_Arm1Enable_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_05", fTemp_Set->edFFC_Arm1Enable_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_06", fTemp_Set->edFFC_Arm1Enable_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_07", fTemp_Set->edFFC_Arm1Enable_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_08", fTemp_Set->edFFC_Arm1Enable_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_09", fTemp_Set->edFFC_Arm1Enable_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Enable_10", fTemp_Set->edFFC_Arm1Enable_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_01", fTemp_Set->edFFC_Arm1Offset_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_02", fTemp_Set->edFFC_Arm1Offset_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_03", fTemp_Set->edFFC_Arm1Offset_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_04", fTemp_Set->edFFC_Arm1Offset_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_05", fTemp_Set->edFFC_Arm1Offset_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_06", fTemp_Set->edFFC_Arm1Offset_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_07", fTemp_Set->edFFC_Arm1Offset_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_08", fTemp_Set->edFFC_Arm1Offset_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_09", fTemp_Set->edFFC_Arm1Offset_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1Offset_10", fTemp_Set->edFFC_Arm1Offset_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_01", fTemp_Set->edFFC_Arm1TimeOff_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_02", fTemp_Set->edFFC_Arm1TimeOff_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_03", fTemp_Set->edFFC_Arm1TimeOff_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_04", fTemp_Set->edFFC_Arm1TimeOff_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_05", fTemp_Set->edFFC_Arm1TimeOff_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_06", fTemp_Set->edFFC_Arm1TimeOff_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_07", fTemp_Set->edFFC_Arm1TimeOff_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_08", fTemp_Set->edFFC_Arm1TimeOff_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_09", fTemp_Set->edFFC_Arm1TimeOff_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOff_10", fTemp_Set->edFFC_Arm1TimeOff_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_01", fTemp_Set->edFFC_Arm1TimeOn_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_02", fTemp_Set->edFFC_Arm1TimeOn_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_03", fTemp_Set->edFFC_Arm1TimeOn_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_04", fTemp_Set->edFFC_Arm1TimeOn_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_05", fTemp_Set->edFFC_Arm1TimeOn_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_06", fTemp_Set->edFFC_Arm1TimeOn_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_07", fTemp_Set->edFFC_Arm1TimeOn_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_08", fTemp_Set->edFFC_Arm1TimeOn_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_09", fTemp_Set->edFFC_Arm1TimeOn_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm1TimeOn_10", fTemp_Set->edFFC_Arm1TimeOn_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_01", fTemp_Set->edFFC_Arm2Enable_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_02", fTemp_Set->edFFC_Arm2Enable_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_03", fTemp_Set->edFFC_Arm2Enable_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_04", fTemp_Set->edFFC_Arm2Enable_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_05", fTemp_Set->edFFC_Arm2Enable_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_06", fTemp_Set->edFFC_Arm2Enable_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_07", fTemp_Set->edFFC_Arm2Enable_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_08", fTemp_Set->edFFC_Arm2Enable_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_09", fTemp_Set->edFFC_Arm2Enable_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Enable_10", fTemp_Set->edFFC_Arm2Enable_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_01", fTemp_Set->edFFC_Arm2Offset_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_02", fTemp_Set->edFFC_Arm2Offset_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_03", fTemp_Set->edFFC_Arm2Offset_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_04", fTemp_Set->edFFC_Arm2Offset_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_05", fTemp_Set->edFFC_Arm2Offset_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_06", fTemp_Set->edFFC_Arm2Offset_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_07", fTemp_Set->edFFC_Arm2Offset_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_08", fTemp_Set->edFFC_Arm2Offset_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_09", fTemp_Set->edFFC_Arm2Offset_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2Offset_10", fTemp_Set->edFFC_Arm2Offset_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_01", fTemp_Set->edFFC_Arm2TimeOff_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_02", fTemp_Set->edFFC_Arm2TimeOff_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_03", fTemp_Set->edFFC_Arm2TimeOff_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_04", fTemp_Set->edFFC_Arm2TimeOff_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_05", fTemp_Set->edFFC_Arm2TimeOff_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_06", fTemp_Set->edFFC_Arm2TimeOff_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_07", fTemp_Set->edFFC_Arm2TimeOff_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_08", fTemp_Set->edFFC_Arm2TimeOff_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_09", fTemp_Set->edFFC_Arm2TimeOff_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOff_10", fTemp_Set->edFFC_Arm2TimeOff_10); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_01", fTemp_Set->edFFC_Arm2TimeOn_01); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_02", fTemp_Set->edFFC_Arm2TimeOn_02); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_03", fTemp_Set->edFFC_Arm2TimeOn_03); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_04", fTemp_Set->edFFC_Arm2TimeOn_04); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_05", fTemp_Set->edFFC_Arm2TimeOn_05); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_06", fTemp_Set->edFFC_Arm2TimeOn_06); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_07", fTemp_Set->edFFC_Arm2TimeOn_07); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_08", fTemp_Set->edFFC_Arm2TimeOn_08); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_09", fTemp_Set->edFFC_Arm2TimeOn_09); filerw::ELKeep("TfTemp_Set", "edFFC_Arm2TimeOn_10", fTemp_Set->edFFC_Arm2TimeOn_10); filerw::ELKeep("TfTemp_Set", "edFixedTemp", fTemp_Set->edFixedTemp); filerw::ELKeep("TfTemp_Set", "edHighBase", fTemp_Set->edHighBase); filerw::ELKeep("TfTemp_Set", "edIndexSoakTime", fTemp_Set->edIndexSoakTime); filerw::ELKeep("TfTemp_Set", "edInitialStart1Time", fTemp_Set->edInitialStart1Time); filerw::ELKeep("TfTemp_Set", "edInitialStart2Time", fTemp_Set->edInitialStart2Time); filerw::ELKeep("TfTemp_Set", "edInitialWaitTime", fTemp_Set->edInitialWaitTime); filerw::ELKeep("TfTemp_Set", "edJamSoakTime", fTemp_Set->edJamSoakTime); filerw::ELKeep("TfTemp_Set", "edLBTempAlarmTime", fTemp_Set->edLBTempAlarmTime); filerw::ELKeep("TfTemp_Set", "edLBTempHighSettingValue", fTemp_Set->edLBTempHighSettingValue); filerw::ELKeep("TfTemp_Set", "edLBTempLowSettingValue", fTemp_Set->edLBTempLowSettingValue); filerw::ELKeep("TfTemp_Set", "edLowBase", fTemp_Set->edLowBase); filerw::ELKeep("TfTemp_Set", "edMidBase", fTemp_Set->edMidBase); filerw::ELKeep("TfTemp_Set", "edOSTime", fTemp_Set->edOSTime); filerw::ELKeep("TfTemp_Set", "edOverSec", fTemp_Set->edOverSec); filerw::ELKeep("TfTemp_Set", "edSHighBase", fTemp_Set->edSHighBase); filerw::ELKeep("TfTemp_Set", "edSoakTime", fTemp_Set->edSoakTime); filerw::ELKeep("TfTemp_Set", "edSocketAirCoolingOff", fTemp_Set->edSocketAirCoolingOff); filerw::ELKeep("TfTemp_Set", "edSocketAirCoolingOn", fTemp_Set->edSocketAirCoolingOn); filerw::ELKeep("TfTemp_Set", "edTJTempRange_High", fTemp_Set->edTJTempRange_High); filerw::ELKeep("TfTemp_Set", "edTJTempRange_Low", fTemp_Set->edTJTempRange_Low); filerw::ELKeep("TfTemp_Set", "edTSDTimeOut", fTemp_Set->edTSDTimeOut); filerw::ELKeep("TfTemp_Set", "edTempAlwaysSameAlarm", fTemp_Set->edTempAlwaysSameAlarm); filerw::ELKeep("TfTemp_Set", "edTempDownContactDelay", fTemp_Set->edTempDownContactDelay); filerw::ELKeep("TfTemp_Set", "edTempOffsetCount", fTemp_Set->edTempOffsetCount); filerw::ELKeep("TfTemp_Set", "edTempReadyRange", fTemp_Set->edTempReadyRange); filerw::ELKeep("TfTemp_Set", "edWorkTemp", fTemp_Set->edWorkTemp); filerw::ELKeep("TfTemp_Set", "edZoneSetting1", fTemp_Set->edZoneSetting1); filerw::ELKeep("TfTemp_Set", "edZoneSetting2", fTemp_Set->edZoneSetting2); filerw::ELKeep("TfTemp_Set", "edZoneSetting3", fTemp_Set->edZoneSetting3); filerw::ELKeep("TfTemp_Set", "edZoneSetting4", fTemp_Set->edZoneSetting4); filerw::ELKeep("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue", fTemp_Set->edt3SigmaTempMonitior_Set3xSigmaValue); filerw::ELKeep("TfTemp_Set", "edt3SigmaTempMonitior_SetCount", fTemp_Set->edt3SigmaTempMonitior_SetCount); filerw::ELKeep("TfTemp_Set", "edtATCInPC1", fTemp_Set->edtATCInPC1); filerw::ELKeep("TfTemp_Set", "edtATCInPC2", fTemp_Set->edtATCInPC2); filerw::ELKeep("TfTemp_Set", "edtATCInPC3", fTemp_Set->edtATCInPC3); filerw::ELKeep("TfTemp_Set", "edtATCInPC4", fTemp_Set->edtATCInPC4); filerw::ELKeep("TfTemp_Set", "edtATCOffset_01", fTemp_Set->edtATCOffset_01); filerw::ELKeep("TfTemp_Set", "edtATCOffset_02", fTemp_Set->edtATCOffset_02); filerw::ELKeep("TfTemp_Set", "edtATCOffset_03", fTemp_Set->edtATCOffset_03); filerw::ELKeep("TfTemp_Set", "edtATCOffset_04", fTemp_Set->edtATCOffset_04); filerw::ELKeep("TfTemp_Set", "edtATCOffset_05", fTemp_Set->edtATCOffset_05); filerw::ELKeep("TfTemp_Set", "edtATCOffset_06", fTemp_Set->edtATCOffset_06); filerw::ELKeep("TfTemp_Set", "edtATCOffset_07", fTemp_Set->edtATCOffset_07); filerw::ELKeep("TfTemp_Set", "edtATCOffset_08", fTemp_Set->edtATCOffset_08); filerw::ELKeep("TfTemp_Set", "edtATCOffset_09", fTemp_Set->edtATCOffset_09); filerw::ELKeep("TfTemp_Set", "edtATCOffset_10", fTemp_Set->edtATCOffset_10); filerw::ELKeep("TfTemp_Set", "edtATCOffset_11", fTemp_Set->edtATCOffset_11); filerw::ELKeep("TfTemp_Set", "edtATCOffset_12", fTemp_Set->edtATCOffset_12); filerw::ELKeep("TfTemp_Set", "edtATCOffset_13", fTemp_Set->edtATCOffset_13); filerw::ELKeep("TfTemp_Set", "edtATCOffset_14", fTemp_Set->edtATCOffset_14); filerw::ELKeep("TfTemp_Set", "edtATCOffset_15", fTemp_Set->edtATCOffset_15); filerw::ELKeep("TfTemp_Set", "edtATCOffset_16", fTemp_Set->edtATCOffset_16); filerw::ELKeep("TfTemp_Set", "edtATCOffset_17", fTemp_Set->edtATCOffset_17); filerw::ELKeep("TfTemp_Set", "edtATCOffset_18", fTemp_Set->edtATCOffset_18); filerw::ELKeep("TfTemp_Set", "edtATCOffset_19", fTemp_Set->edtATCOffset_19); filerw::ELKeep("TfTemp_Set", "edtATCOffset_20", fTemp_Set->edtATCOffset_20); filerw::ELKeep("TfTemp_Set", "edtATCOffset_21", fTemp_Set->edtATCOffset_21); filerw::ELKeep("TfTemp_Set", "edtATCOffset_22", fTemp_Set->edtATCOffset_22); filerw::ELKeep("TfTemp_Set", "edtATCOffset_23", fTemp_Set->edtATCOffset_23); filerw::ELKeep("TfTemp_Set", "edtATCOffset_24", fTemp_Set->edtATCOffset_24); filerw::ELKeep("TfTemp_Set", "edtATCOffset_25", fTemp_Set->edtATCOffset_25); filerw::ELKeep("TfTemp_Set", "edtATCOffset_26", fTemp_Set->edtATCOffset_26); filerw::ELKeep("TfTemp_Set", "edtATCOffset_27", fTemp_Set->edtATCOffset_27); filerw::ELKeep("TfTemp_Set", "edtATCOffset_28", fTemp_Set->edtATCOffset_28); filerw::ELKeep("TfTemp_Set", "edtATCOffset_29", fTemp_Set->edtATCOffset_29); filerw::ELKeep("TfTemp_Set", "edtATCOffset_30", fTemp_Set->edtATCOffset_30); filerw::ELKeep("TfTemp_Set", "edtATCOffset_31", fTemp_Set->edtATCOffset_31); filerw::ELKeep("TfTemp_Set", "edtATCOffset_32", fTemp_Set->edtATCOffset_32); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MaxD", fTemp_Set->edtATCPIDOffset_MaxD); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MaxI", fTemp_Set->edtATCPIDOffset_MaxI); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MaxP", fTemp_Set->edtATCPIDOffset_MaxP); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MinD", fTemp_Set->edtATCPIDOffset_MinD); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MinI", fTemp_Set->edtATCPIDOffset_MinI); filerw::ELKeep("TfTemp_Set", "edtATCPIDOffset_MinP", fTemp_Set->edtATCPIDOffset_MinP); filerw::ELKeep("TfTemp_Set", "edtATCSP2", fTemp_Set->edtATCSP2); filerw::ELKeep("TfTemp_Set", "edtATC_HotGunTemp", fTemp_Set->edtATC_HotGunTemp); filerw::ELKeep("TfTemp_Set", "edtATC_HotGunTime", fTemp_Set->edtATC_HotGunTime); filerw::ELKeep("TfTemp_Set", "edtATC_PackageOffset_01", fTemp_Set->edtATC_PackageOffset_01); filerw::ELKeep("TfTemp_Set", "edtATC_PackageOffset_02", fTemp_Set->edtATC_PackageOffset_02); filerw::ELKeep("TfTemp_Set", "edtATC_PackageOffset_03", fTemp_Set->edtATC_PackageOffset_03); filerw::ELKeep("TfTemp_Set", "edtATC_PackageTemp_01", fTemp_Set->edtATC_PackageTemp_01); filerw::ELKeep("TfTemp_Set", "edtATC_PackageTemp_02", fTemp_Set->edtATC_PackageTemp_02); filerw::ELKeep("TfTemp_Set", "edtATC_PackageTemp_03", fTemp_Set->edtATC_PackageTemp_03); filerw::ELKeep("TfTemp_Set", "edtATC_Type", fTemp_Set->edtATC_Type); filerw::ELKeep("TfTemp_Set", "edtBoostDuration_LB", fTemp_Set->edtBoostDuration_LB); filerw::ELKeep("TfTemp_Set", "edtBoostDuration_Long", fTemp_Set->edtBoostDuration_Long); filerw::ELKeep("TfTemp_Set", "edtBoostDuration_Mid", fTemp_Set->edtBoostDuration_Mid); filerw::ELKeep("TfTemp_Set", "edtBoostDuration_Short", fTemp_Set->edtBoostDuration_Short); filerw::ELKeep("TfTemp_Set", "edtBoostOffset", fTemp_Set->edtBoostOffset); filerw::ELKeep("TfTemp_Set", "edtBoostOffset_LB", fTemp_Set->edtBoostOffset_LB); filerw::ELKeep("TfTemp_Set", "edtBoostOffset_Long", fTemp_Set->edtBoostOffset_Long); filerw::ELKeep("TfTemp_Set", "edtBoostOffset_Mid", fTemp_Set->edtBoostOffset_Mid); filerw::ELKeep("TfTemp_Set", "edtBoostOffset_Short", fTemp_Set->edtBoostOffset_Short); filerw::ELKeep("TfTemp_Set", "edtBoostTempMin", fTemp_Set->edtBoostTempMin); filerw::ELKeep("TfTemp_Set", "edtChamberBoostOffset", fTemp_Set->edtChamberBoostOffset); filerw::ELKeep("TfTemp_Set", "edtChamberBoostTime", fTemp_Set->edtChamberBoostTime); filerw::ELKeep("TfTemp_Set", "edtCurrTemp", fTemp_Set->edtCurrTemp); filerw::ELKeep("TfTemp_Set", "edtDelayAfterSOT", fTemp_Set->edtDelayAfterSOT); filerw::ELKeep("TfTemp_Set", "edtHeatGunTempATC", fTemp_Set->edtHeatGunTempATC); filerw::ELKeep("TfTemp_Set", "edtHotGunFLowLimit_H", fTemp_Set->edtHotGunFLowLimit_H); filerw::ELKeep("TfTemp_Set", "edtHotGunFLowLimit_L", fTemp_Set->edtHotGunFLowLimit_L); filerw::ELKeep("TfTemp_Set", "edtIdleTime_LB", fTemp_Set->edtIdleTime_LB); filerw::ELKeep("TfTemp_Set", "edtIdleTime_Long", fTemp_Set->edtIdleTime_Long); filerw::ELKeep("TfTemp_Set", "edtIdleTime_Mid", fTemp_Set->edtIdleTime_Mid); filerw::ELKeep("TfTemp_Set", "edtIdleTime_Short", fTemp_Set->edtIdleTime_Short); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_1", fTemp_Set->edtInitialDelay_1); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_10", fTemp_Set->edtInitialDelay_10); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_10_RT", fTemp_Set->edtInitialDelay_10_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_1_RT", fTemp_Set->edtInitialDelay_1_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_2", fTemp_Set->edtInitialDelay_2); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_2_RT", fTemp_Set->edtInitialDelay_2_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_3", fTemp_Set->edtInitialDelay_3); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_3_RT", fTemp_Set->edtInitialDelay_3_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_4", fTemp_Set->edtInitialDelay_4); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_4_RT", fTemp_Set->edtInitialDelay_4_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_5", fTemp_Set->edtInitialDelay_5); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_5_RT", fTemp_Set->edtInitialDelay_5_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_6", fTemp_Set->edtInitialDelay_6); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_6_RT", fTemp_Set->edtInitialDelay_6_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_7", fTemp_Set->edtInitialDelay_7); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_7_RT", fTemp_Set->edtInitialDelay_7_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_8", fTemp_Set->edtInitialDelay_8); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_8_RT", fTemp_Set->edtInitialDelay_8_RT); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_9", fTemp_Set->edtInitialDelay_9); filerw::ELKeep("TfTemp_Set", "edtInitialDelay_9_RT", fTemp_Set->edtInitialDelay_9_RT); filerw::ELKeep("TfTemp_Set", "edtInputVHigh", fTemp_Set->edtInputVHigh); filerw::ELKeep("TfTemp_Set", "edtInputVLow", fTemp_Set->edtInputVLow); filerw::ELKeep("TfTemp_Set", "edtLBAirOnTemp", fTemp_Set->edtLBAirOnTemp); filerw::ELKeep("TfTemp_Set", "edtLBTempMin", fTemp_Set->edtLBTempMin); filerw::ELKeep("TfTemp_Set", "edtLBTempOffset", fTemp_Set->edtLBTempOffset); filerw::ELKeep("TfTemp_Set", "edtLBTimeOut", fTemp_Set->edtLBTimeOut); filerw::ELKeep("TfTemp_Set", "edtPostBoost_LB", fTemp_Set->edtPostBoost_LB); filerw::ELKeep("TfTemp_Set", "edtPostBoost_Long", fTemp_Set->edtPostBoost_Long); filerw::ELKeep("TfTemp_Set", "edtPostBoost_Mid", fTemp_Set->edtPostBoost_Mid); filerw::ELKeep("TfTemp_Set", "edtPostBoost_Short", fTemp_Set->edtPostBoost_Short); filerw::ELKeep("TfTemp_Set", "edtPowerFollow_FullPower", fTemp_Set->edtPowerFollow_FullPower); filerw::ELKeep("TfTemp_Set", "edtPowerFollower_Many2one", fTemp_Set->edtPowerFollower_Many2one); filerw::ELKeep("TfTemp_Set", "edtPowerFollower_PFSlope", fTemp_Set->edtPowerFollower_PFSlope); filerw::ELKeep("TfTemp_Set", "edtPowerFollower_WGain", fTemp_Set->edtPowerFollower_WGain); filerw::ELKeep("TfTemp_Set", "edtSetTJ_Offset", fTemp_Set->edtSetTJ_Offset); filerw::ELKeep("TfTemp_Set", "edtSetTJ_Slope", fTemp_Set->edtSetTJ_Slope); filerw::ELKeep("TfTemp_Set", "edtSetTempature2AirMachine", fTemp_Set->edtSetTempature2AirMachine); filerw::ELKeep("TfTemp_Set", "edtTestDock", fTemp_Set->edtTestDock); filerw::ELKeep("TfTemp_Set", "edtThreshold", fTemp_Set->edtThreshold); filerw::ELKeep("TfTemp_Set", "edt_AirVolumeLmt_Index", fTemp_Set->edt_AirVolumeLmt_Index); filerw::ELKeep("TfTemp_Set", "edt_AirVolumeLmt_Socket", fTemp_Set->edt_AirVolumeLmt_Socket); filerw::ELKeep("TfTemp_Set", "edt_Defrost_Time_Too_Lower", fTemp_Set->edt_Defrost_Time_Too_Lower); filerw::ELKeep("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index", fTemp_Set->edt_SetAirstreamTemperatureRang_Index); filerw::ELKeep("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket", fTemp_Set->edt_SetAirstreamTemperatureRang_Socket); filerw::ELKeep("TfTemp_Set", "edt_SetIndexAirstreamTemp", fTemp_Set->edt_SetIndexAirstreamTemp); filerw::ELKeep("TfTemp_Set", "edt_TempRiseTemp", fTemp_Set->edt_TempRiseTemp); filerw::ELKeep("TfTemp_Set", "edt_TempRise_Delay", fTemp_Set->edt_TempRise_Delay); filerw::ELKeep("TfTemp_Set", "edt_TjAvgTimes", fTemp_Set->edt_TjAvgTimes); filerw::ELKeep("TfTemp_Set", "gbATC70", fTemp_Set->gbATC70); filerw::ELKeep("TfTemp_Set", "gbATCAmbientTemp", fTemp_Set->gbATCAmbientTemp); filerw::ELKeep("TfTemp_Set", "gbATCConsFail", fTemp_Set->gbATCConsFail); filerw::ELKeep("TfTemp_Set", "gbATCInitialOffset", fTemp_Set->gbATCInitialOffset); filerw::ELKeep("TfTemp_Set", "gbATCPackageOffset", fTemp_Set->gbATCPackageOffset); filerw::ELKeep("TfTemp_Set", "gbATCQAModeOffset", fTemp_Set->gbATCQAModeOffset); filerw::ELKeep("TfTemp_Set", "gbATCTestTimeOffset", fTemp_Set->gbATCTestTimeOffset); filerw::ELKeep("TfTemp_Set", "gbATC_Type", fTemp_Set->gbATC_Type); filerw::ELKeep("TfTemp_Set", "gbAmbientModeSetting", fTemp_Set->gbAmbientModeSetting); filerw::ELKeep("TfTemp_Set", "gbBasePoint", fTemp_Set->gbBasePoint); filerw::ELKeep("TfTemp_Set", "gbChillerTemp", fTemp_Set->gbChillerTemp); filerw::ELKeep("TfTemp_Set", "gbDefrostFunction", fTemp_Set->gbDefrostFunction); filerw::ELKeep("TfTemp_Set", "gbFileName", fTemp_Set->gbFileName); filerw::ELKeep("TfTemp_Set", "gbHeatGunTempATC", fTemp_Set->gbHeatGunTempATC); filerw::ELKeep("TfTemp_Set", "gbHotModeSetting", fTemp_Set->gbHotModeSetting); filerw::ELKeep("TfTemp_Set", "gbMultiZone", fTemp_Set->gbMultiZone); filerw::ELKeep("TfTemp_Set", "gbNoFullsiteOffset", fTemp_Set->gbNoFullsiteOffset); filerw::ELKeep("TfTemp_Set", "gbPowerFollowFuncAct", fTemp_Set->gbPowerFollowFuncAct); filerw::ELKeep("TfTemp_Set", "gbSocketAirCooling", fTemp_Set->gbSocketAirCooling); filerw::ELKeep("TfTemp_Set", "gbTSD", fTemp_Set->gbTSD); filerw::ELKeep("TfTemp_Set", "gbTempOffset", fTemp_Set->gbTempOffset); filerw::ELKeep("TfTemp_Set", "gb_HotGunFlow", fTemp_Set->gb_HotGunFlow); filerw::ELKeep("TfTemp_Set", "gb_TjFunction", fTemp_Set->gb_TjFunction); filerw::ELKeep("TfTemp_Set", "gbcb3SigmaTempMonitior", fTemp_Set->gbcb3SigmaTempMonitior); filerw::ELKeep("TfTemp_Set", "grbATCTempAlarm", fTemp_Set->grbATCTempAlarm); filerw::ELKeep("TfTemp_Set", "grpATCTempOffset", fTemp_Set->grpATCTempOffset); filerw::ELKeep("TfTemp_Set", "grpATC_HotAir", fTemp_Set->grpATC_HotAir); filerw::ELKeep("TfTemp_Set", "grpBoostMode", fTemp_Set->grpBoostMode); filerw::ELKeep("TfTemp_Set", "grpBoostOffset", fTemp_Set->grpBoostOffset); filerw::ELKeep("TfTemp_Set", "grpChamberBoost", fTemp_Set->grpChamberBoost); filerw::ELKeep("TfTemp_Set", "grpInPC", fTemp_Set->grpInPC); filerw::ELKeep("TfTemp_Set", "grpIndexAirStream", fTemp_Set->grpIndexAirStream); filerw::ELKeep("TfTemp_Set", "grpLBInitialOffset", fTemp_Set->grpLBInitialOffset); filerw::ELKeep("TfTemp_Set", "grpLBPre", fTemp_Set->grpLBPre); filerw::ELKeep("TfTemp_Set", "grpLongPrecondition", fTemp_Set->grpLongPrecondition); filerw::ELKeep("TfTemp_Set", "grpMidPrecondition", fTemp_Set->grpMidPrecondition); filerw::ELKeep("TfTemp_Set", "grpRS232", fTemp_Set->grpRS232); filerw::ELKeep("TfTemp_Set", "grpShortPrecondition", fTemp_Set->grpShortPrecondition); filerw::ELKeep("TfTemp_Set", "grpSocketAirStream", fTemp_Set->grpSocketAirStream); filerw::ELKeep("TfTemp_Set", "grpTJFunction", fTemp_Set->grpTJFunction); filerw::ELKeep("TfTemp_Set", "grpTJTempRange", fTemp_Set->grpTJTempRange); filerw::ELKeep("TfTemp_Set", "grpTempAlwaysSameTime", fTemp_Set->grpTempAlwaysSameTime); filerw::ELKeep("TfTemp_Set", "grpTempCtrl", fTemp_Set->grpTempCtrl); filerw::ELKeep("TfTemp_Set", "grpTemperatureCheck", fTemp_Set->grpTemperatureCheck); filerw::ELKeep("TfTemp_Set", "grpTemperatureRise", fTemp_Set->grpTemperatureRise); filerw::ELKeep("TfTemp_Set", "iTestFinishToNextTestOver", fTemp_Set->iTestFinishToNextTestOver); filerw::ELKeep("TfTemp_Set", "iTeststartToNextTestStart", fTemp_Set->iTeststartToNextTestStart); filerw::ELKeep("TfTemp_Set", "labATCContFailOffset1", fTemp_Set->labATCContFailOffset1); filerw::ELKeep("TfTemp_Set", "labATCContFailOffset2", fTemp_Set->labATCContFailOffset2); filerw::ELKeep("TfTemp_Set", "labATCContFailOffset3", fTemp_Set->labATCContFailOffset3); filerw::ELKeep("TfTemp_Set", "labATCContFailOffset4", fTemp_Set->labATCContFailOffset4); filerw::ELKeep("TfTemp_Set", "labATCInitialOffset1", fTemp_Set->labATCInitialOffset1); filerw::ELKeep("TfTemp_Set", "labATCInitialOffset2", fTemp_Set->labATCInitialOffset2); filerw::ELKeep("TfTemp_Set", "labATCInitialOffset3", fTemp_Set->labATCInitialOffset3); filerw::ELKeep("TfTemp_Set", "labATCInitialOffset4", fTemp_Set->labATCInitialOffset4); filerw::ELKeep("TfTemp_Set", "labATCQAModeOffset1", fTemp_Set->labATCQAModeOffset1); filerw::ELKeep("TfTemp_Set", "labATCQAModeOffset2", fTemp_Set->labATCQAModeOffset2); filerw::ELKeep("TfTemp_Set", "labATCQAModeOffset3", fTemp_Set->labATCQAModeOffset3); filerw::ELKeep("TfTemp_Set", "labATCQAModeOffset4", fTemp_Set->labATCQAModeOffset4); filerw::ELKeep("TfTemp_Set", "labATCTestTimeOffset", fTemp_Set->labATCTestTimeOffset); filerw::ELKeep("TfTemp_Set", "labATCTestTimeOffset1", fTemp_Set->labATCTestTimeOffset1); filerw::ELKeep("TfTemp_Set", "labATCTestTimeOffset2", fTemp_Set->labATCTestTimeOffset2); filerw::ELKeep("TfTemp_Set", "labATCTestTimeOffset3", fTemp_Set->labATCTestTimeOffset3); filerw::ELKeep("TfTemp_Set", "labATCTestTimeOffset4", fTemp_Set->labATCTestTimeOffset4); filerw::ELKeep("TfTemp_Set", "labContctCount", fTemp_Set->labContctCount); filerw::ELKeep("TfTemp_Set", "labLB1", fTemp_Set->labLB1); filerw::ELKeep("TfTemp_Set", "labLB2", fTemp_Set->labLB2); filerw::ELKeep("TfTemp_Set", "labLB3", fTemp_Set->labLB3); filerw::ELKeep("TfTemp_Set", "labLB4", fTemp_Set->labLB4); filerw::ELKeep("TfTemp_Set", "labReadyTempRange", fTemp_Set->labReadyTempRange); filerw::ELKeep("TfTemp_Set", "labTempDownContactDelay", fTemp_Set->labTempDownContactDelay); filerw::ELKeep("TfTemp_Set", "lb1", fTemp_Set->lb1); filerw::ELKeep("TfTemp_Set", "lb2", fTemp_Set->lb2); filerw::ELKeep("TfTemp_Set", "lb3", fTemp_Set->lb3); filerw::ELKeep("TfTemp_Set", "lb4", fTemp_Set->lb4); filerw::ELKeep("TfTemp_Set", "lb5", fTemp_Set->lb5); filerw::ELKeep("TfTemp_Set", "lb6", fTemp_Set->lb6); filerw::ELKeep("TfTemp_Set", "lb7", fTemp_Set->lb7); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm1Enable", fTemp_Set->lbIndiVidualTemperArm1Enable); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm1Offset", fTemp_Set->lbIndiVidualTemperArm1Offset); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm1TimeOff", fTemp_Set->lbIndiVidualTemperArm1TimeOff); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm1TimeOn", fTemp_Set->lbIndiVidualTemperArm1TimeOn); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm2Enable", fTemp_Set->lbIndiVidualTemperArm2Enable); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm2Offset", fTemp_Set->lbIndiVidualTemperArm2Offset); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm2TimeOff", fTemp_Set->lbIndiVidualTemperArm2TimeOff); filerw::ELKeep("TfTemp_Set", "lbIndiVidualTemperArm2TimeOn", fTemp_Set->lbIndiVidualTemperArm2TimeOn); filerw::ELKeep("TfTemp_Set", "lbTJTempRange_High", fTemp_Set->lbTJTempRange_High); filerw::ELKeep("TfTemp_Set", "lbTJTempRange_Low", fTemp_Set->lbTJTempRange_Low); filerw::ELKeep("TfTemp_Set", "lblATCInPC1", fTemp_Set->lblATCInPC1); filerw::ELKeep("TfTemp_Set", "lblATCInPC2", fTemp_Set->lblATCInPC2); filerw::ELKeep("TfTemp_Set", "lblATCInPC3", fTemp_Set->lblATCInPC3); filerw::ELKeep("TfTemp_Set", "lblATCInPC4", fTemp_Set->lblATCInPC4); filerw::ELKeep("TfTemp_Set", "lblAmbGuardBand", fTemp_Set->lblAmbGuardBand); filerw::ELKeep("TfTemp_Set", "lblAmbGuardBandDeg", fTemp_Set->lblAmbGuardBandDeg); filerw::ELKeep("TfTemp_Set", "lblArm1Offset", fTemp_Set->lblArm1Offset); filerw::ELKeep("TfTemp_Set", "lblArm2Offset", fTemp_Set->lblArm2Offset); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_LB", fTemp_Set->lblBoostDuration_LB); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_LB_1", fTemp_Set->lblBoostDuration_LB_1); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_Long", fTemp_Set->lblBoostDuration_Long); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_Long1", fTemp_Set->lblBoostDuration_Long1); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_Mid", fTemp_Set->lblBoostDuration_Mid); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_Short", fTemp_Set->lblBoostDuration_Short); filerw::ELKeep("TfTemp_Set", "lblBoostDuration_Short1", fTemp_Set->lblBoostDuration_Short1); filerw::ELKeep("TfTemp_Set", "lblBoostOffset", fTemp_Set->lblBoostOffset); filerw::ELKeep("TfTemp_Set", "lblBoostOffset1", fTemp_Set->lblBoostOffset1); filerw::ELKeep("TfTemp_Set", "lblBoostOffset2", fTemp_Set->lblBoostOffset2); filerw::ELKeep("TfTemp_Set", "lblBoostOffset3", fTemp_Set->lblBoostOffset3); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_LB", fTemp_Set->lblBoostOffset_LB); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_LB_1", fTemp_Set->lblBoostOffset_LB_1); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_Long", fTemp_Set->lblBoostOffset_Long); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_Long1", fTemp_Set->lblBoostOffset_Long1); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_Mid", fTemp_Set->lblBoostOffset_Mid); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_Short", fTemp_Set->lblBoostOffset_Short); filerw::ELKeep("TfTemp_Set", "lblBoostOffset_Short1", fTemp_Set->lblBoostOffset_Short1); filerw::ELKeep("TfTemp_Set", "lblChamberBoostOffset", fTemp_Set->lblChamberBoostOffset); filerw::ELKeep("TfTemp_Set", "lblChamberBoostTime", fTemp_Set->lblChamberBoostTime); filerw::ELKeep("TfTemp_Set", "lblCurr", fTemp_Set->lblCurr); filerw::ELKeep("TfTemp_Set", "lblDelayTime", fTemp_Set->lblDelayTime); filerw::ELKeep("TfTemp_Set", "lblFT", fTemp_Set->lblFT); filerw::ELKeep("TfTemp_Set", "lblIdleTime_LB", fTemp_Set->lblIdleTime_LB); filerw::ELKeep("TfTemp_Set", "lblIdleTime_LB_1", fTemp_Set->lblIdleTime_LB_1); filerw::ELKeep("TfTemp_Set", "lblIdleTime_Long", fTemp_Set->lblIdleTime_Long); filerw::ELKeep("TfTemp_Set", "lblIdleTime_Long1", fTemp_Set->lblIdleTime_Long1); filerw::ELKeep("TfTemp_Set", "lblIdleTime_Mid", fTemp_Set->lblIdleTime_Mid); filerw::ELKeep("TfTemp_Set", "lblIdleTime_Short", fTemp_Set->lblIdleTime_Short); filerw::ELKeep("TfTemp_Set", "lblIdleTime_Short1", fTemp_Set->lblIdleTime_Short1); filerw::ELKeep("TfTemp_Set", "lblLBTempMin", fTemp_Set->lblLBTempMin); filerw::ELKeep("TfTemp_Set", "lblLBTempMin_1", fTemp_Set->lblLBTempMin_1); filerw::ELKeep("TfTemp_Set", "lblLBTempOffset", fTemp_Set->lblLBTempOffset); filerw::ELKeep("TfTemp_Set", "lblLBTempOffset_1", fTemp_Set->lblLBTempOffset_1); filerw::ELKeep("TfTemp_Set", "lblNotice", fTemp_Set->lblNotice); filerw::ELKeep("TfTemp_Set", "lblPostBoost_LB", fTemp_Set->lblPostBoost_LB); filerw::ELKeep("TfTemp_Set", "lblPostBoost_LB_1", fTemp_Set->lblPostBoost_LB_1); filerw::ELKeep("TfTemp_Set", "lblPostBoost_Long", fTemp_Set->lblPostBoost_Long); filerw::ELKeep("TfTemp_Set", "lblPostBoost_Long1", fTemp_Set->lblPostBoost_Long1); filerw::ELKeep("TfTemp_Set", "lblPostBoost_Mid", fTemp_Set->lblPostBoost_Mid); filerw::ELKeep("TfTemp_Set", "lblPostBoost_Short", fTemp_Set->lblPostBoost_Short); filerw::ELKeep("TfTemp_Set", "lblPostBoost_Short1", fTemp_Set->lblPostBoost_Short1); filerw::ELKeep("TfTemp_Set", "lblRT", fTemp_Set->lblRT); filerw::ELKeep("TfTemp_Set", "lblShtSoakTime", fTemp_Set->lblShtSoakTime); filerw::ELKeep("TfTemp_Set", "lblTempBase", fTemp_Set->lblTempBase); filerw::ELKeep("TfTemp_Set", "lblTriggerTime", fTemp_Set->lblTriggerTime); filerw::ELKeep("TfTemp_Set", "lbllblShtSoakTimeSec", fTemp_Set->lbllblShtSoakTimeSec); filerw::ELKeep("TfTemp_Set", "lblthreshold", fTemp_Set->lblthreshold); filerw::ELKeep("TfTemp_Set", "lblthreshold_1", fTemp_Set->lblthreshold_1); filerw::ELKeep("TfTemp_Set", "memoInitialTempOffset", fTemp_Set->memoInitialTempOffset); filerw::ELKeep("TfTemp_Set", "palATC", fTemp_Set->palATC); filerw::ELKeep("TfTemp_Set", "palIndiTemp", fTemp_Set->palIndiTemp); filerw::ELKeep("TfTemp_Set", "palLBAir", fTemp_Set->palLBAir); filerw::ELKeep("TfTemp_Set", "pgcOtherFunc", fTemp_Set->pgcOtherFunc); filerw::ELKeep("TfTemp_Set", "pgcTempOffset", fTemp_Set->pgcTempOffset); filerw::ELKeep("TfTemp_Set", "pgcTempSetting", fTemp_Set->pgcTempSetting); filerw::ELKeep("TfTemp_Set", "pnl1stDevice", fTemp_Set->pnl1stDevice); filerw::ELKeep("TfTemp_Set", "pnlATC_PIDOffset", fTemp_Set->pnlATC_PIDOffset); filerw::ELKeep("TfTemp_Set", "pnlATC_PIDOffsetTop", fTemp_Set->pnlATC_PIDOffsetTop); filerw::ELKeep("TfTemp_Set", "pnlAlarmMsg", fTemp_Set->pnlAlarmMsg); filerw::ELKeep("TfTemp_Set", "pnlArm1Offset", fTemp_Set->pnlArm1Offset); filerw::ELKeep("TfTemp_Set", "pnlArm2Offset", fTemp_Set->pnlArm2Offset); filerw::ELKeep("TfTemp_Set", "pnlAutoClean", fTemp_Set->pnlAutoClean); filerw::ELKeep("TfTemp_Set", "pnlBoostDuration_Long", fTemp_Set->pnlBoostDuration_Long); filerw::ELKeep("TfTemp_Set", "pnlBoostDuration_Mid", fTemp_Set->pnlBoostDuration_Mid); filerw::ELKeep("TfTemp_Set", "pnlBoostDuration_Short", fTemp_Set->pnlBoostDuration_Short); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset", fTemp_Set->pnlBoostOffset); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset1", fTemp_Set->pnlBoostOffset1); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset2", fTemp_Set->pnlBoostOffset2); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset_Long", fTemp_Set->pnlBoostOffset_Long); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset_Mid", fTemp_Set->pnlBoostOffset_Mid); filerw::ELKeep("TfTemp_Set", "pnlBoostOffset_Short", fTemp_Set->pnlBoostOffset_Short); filerw::ELKeep("TfTemp_Set", "pnlCover", fTemp_Set->pnlCover); filerw::ELKeep("TfTemp_Set", "pnlEOTTime", fTemp_Set->pnlEOTTime); filerw::ELKeep("TfTemp_Set", "pnlHeatDoor", fTemp_Set->pnlHeatDoor); filerw::ELKeep("TfTemp_Set", "pnlHeatMode", fTemp_Set->pnlHeatMode); filerw::ELKeep("TfTemp_Set", "pnlIdleTime_Long", fTemp_Set->pnlIdleTime_Long); filerw::ELKeep("TfTemp_Set", "pnlIdleTime_Mid", fTemp_Set->pnlIdleTime_Mid); filerw::ELKeep("TfTemp_Set", "pnlIdleTime_Short", fTemp_Set->pnlIdleTime_Short); filerw::ELKeep("TfTemp_Set", "pnlLBInitialOffset", fTemp_Set->pnlLBInitialOffset); filerw::ELKeep("TfTemp_Set", "pnlLBInitialOffset1", fTemp_Set->pnlLBInitialOffset1); filerw::ELKeep("TfTemp_Set", "pnlLBInitialOffset2", fTemp_Set->pnlLBInitialOffset2); filerw::ELKeep("TfTemp_Set", "pnlLBInitialOffset3", fTemp_Set->pnlLBInitialOffset3); filerw::ELKeep("TfTemp_Set", "pnlLBInitialOffset4", fTemp_Set->pnlLBInitialOffset4); filerw::ELKeep("TfTemp_Set", "pnlLowTestTime", fTemp_Set->pnlLowTestTime); filerw::ELKeep("TfTemp_Set", "pnlNoFullsite", fTemp_Set->pnlNoFullsite); filerw::ELKeep("TfTemp_Set", "pnlOTD", fTemp_Set->pnlOTD); filerw::ELKeep("TfTemp_Set", "pnlOffsetBase", fTemp_Set->pnlOffsetBase); filerw::ELKeep("TfTemp_Set", "pnlPause", fTemp_Set->pnlPause); filerw::ELKeep("TfTemp_Set", "pnlPostBoost_Long", fTemp_Set->pnlPostBoost_Long); filerw::ELKeep("TfTemp_Set", "pnlPostBoost_Mid", fTemp_Set->pnlPostBoost_Mid); filerw::ELKeep("TfTemp_Set", "pnlPostBoost_Short", fTemp_Set->pnlPostBoost_Short); filerw::ELKeep("TfTemp_Set", "pnlTitle", fTemp_Set->pnlTitle); filerw::ELKeep("TfTemp_Set", "pnlUseOtherArmToTestAfterInitialDelay", fTemp_Set->pnlUseOtherArmToTestAfterInitialDelay); filerw::ELKeep("TfTemp_Set", "pnlthreshold", fTemp_Set->pnlthreshold); filerw::ELKeep("TfTemp_Set", "rb1Point", fTemp_Set->rb1Point); filerw::ELKeep("TfTemp_Set", "rb2Point", fTemp_Set->rb2Point); filerw::ELKeep("TfTemp_Set", "rb3Point", fTemp_Set->rb3Point); filerw::ELKeep("TfTemp_Set", "rb5Point", fTemp_Set->rb5Point); filerw::ELKeep("TfTemp_Set", "rb6Point", fTemp_Set->rb6Point); filerw::ELKeep("TfTemp_Set", "rbATC70ActiveOn", fTemp_Set->rbATC70ActiveOn); filerw::ELKeep("TfTemp_Set", "rbATC7TSDFunction", fTemp_Set->rbATC7TSDFunction); filerw::ELKeep("TfTemp_Set", "rbATCActiveOn", fTemp_Set->rbATCActiveOn); filerw::ELKeep("TfTemp_Set", "rbHandlerStartOn", fTemp_Set->rbHandlerStartOn); filerw::ELKeep("TfTemp_Set", "rbTemp", fTemp_Set->rbTemp); filerw::ELKeep("TfTemp_Set", "rbTemperatureSetOn", fTemp_Set->rbTemperatureSetOn); filerw::ELKeep("TfTemp_Set", "rbTestStartOn", fTemp_Set->rbTestStartOn); filerw::ELKeep("TfTemp_Set", "rgATC60AirOn", fTemp_Set->rgATC60AirOn); filerw::ELKeep("TfTemp_Set", "rgAmbCheck", fTemp_Set->rgAmbCheck); filerw::ELKeep("TfTemp_Set", "rgAmbientBrand", fTemp_Set->rgAmbientBrand); filerw::ELKeep("TfTemp_Set", "rgBasePoint", fTemp_Set->rgBasePoint); filerw::ELKeep("TfTemp_Set", "rgBoostMode", fTemp_Set->rgBoostMode); filerw::ELKeep("TfTemp_Set", "rgIndexHeatMode", fTemp_Set->rgIndexHeatMode); filerw::ELKeep("TfTemp_Set", "rgIndexHeatMode1", fTemp_Set->rgIndexHeatMode1); filerw::ELKeep("TfTemp_Set", "rgShuttleSoakTime", fTemp_Set->rgShuttleSoakTime); filerw::ELKeep("TfTemp_Set", "rgTemperatureMode", fTemp_Set->rgTemperatureMode); filerw::ELKeep("TfTemp_Set", "rgTemperatureMode1", fTemp_Set->rgTemperatureMode1); filerw::ELKeep("TfTemp_Set", "rgTjMapType", fTemp_Set->rgTjMapType); filerw::ELKeep("TfTemp_Set", "rgZ1Down", fTemp_Set->rgZ1Down); filerw::ELKeep("TfTemp_Set", "rg_TjMode", fTemp_Set->rg_TjMode); filerw::ELKeep("TfTemp_Set", "sbSafeTestATC", fTemp_Set->sbSafeTestATC); filerw::ELKeep("TfTemp_Set", "sbtExit", fTemp_Set->sbtExit); filerw::ELKeep("TfTemp_Set", "spbSave", fTemp_Set->spbSave); filerw::ELKeep("TfTemp_Set", "thNormal", fTemp_Set->thNormal); filerw::ELKeep("TfTemp_Set", "tsATC", fTemp_Set->tsATC); filerw::ELKeep("TfTemp_Set", "tsATC70", fTemp_Set->tsATC70); filerw::ELKeep("TfTemp_Set", "tsATCMultiSensor", fTemp_Set->tsATCMultiSensor); filerw::ELKeep("TfTemp_Set", "tsATC_FFC", fTemp_Set->tsATC_FFC); filerw::ELKeep("TfTemp_Set", "tsATC_PID", fTemp_Set->tsATC_PID); filerw::ELKeep("TfTemp_Set", "tsAmbCtr", fTemp_Set->tsAmbCtr); filerw::ELKeep("TfTemp_Set", "tsAmbient", fTemp_Set->tsAmbient); filerw::ELKeep("TfTemp_Set", "tsArm1", fTemp_Set->tsArm1); filerw::ELKeep("TfTemp_Set", "tsArm2", fTemp_Set->tsArm2); filerw::ELKeep("TfTemp_Set", "tsBoostFunc", fTemp_Set->tsBoostFunc); filerw::ELKeep("TfTemp_Set", "tsDUT", fTemp_Set->tsDUT); filerw::ELKeep("TfTemp_Set", "tsHeatGun", fTemp_Set->tsHeatGun); filerw::ELKeep("TfTemp_Set", "tsHot", fTemp_Set->tsHot); filerw::ELKeep("TfTemp_Set", "tsInitOffset", fTemp_Set->tsInitOffset); filerw::ELKeep("TfTemp_Set", "tsLBCtr", fTemp_Set->tsLBCtr); filerw::ELKeep("TfTemp_Set", "tsOther", fTemp_Set->tsOther); filerw::ELKeep("TfTemp_Set", "tsOthers", fTemp_Set->tsOthers); filerw::ELKeep("TfTemp_Set", "tsSigmaForTemp", fTemp_Set->tsSigmaForTemp); filerw::ELKeep("TfTemp_Set", "tsStandard", fTemp_Set->tsStandard); filerw::ELKeep("TfTemp_Set", "tsTriTemp", fTemp_Set->tsTriTemp); filerw::ELKeep("TfTemp_Set", "tsTriTempSet", fTemp_Set->tsTriTempSet); filerw::ELKeep("TfTemp_Set", "tsUnUse", fTemp_Set->tsUnUse); }   // 852 個（forms/fTemp_Set.h ↔ golden uTemp_Set.h 同名同型別）
static void TS_AdoptPanels() { AnsiString n; for(int i=0; i<tcTotalCount; i++) { if(fTemp_Set->myTempPal[i]==NULL) continue; if(TS_myTempPal[i]==NULL) TS_myTempPal[i]=new TS_TMyTempPanel(fTemp_Set->myTempPal[i]); n.sprintf("myTempPal%d_edLow", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edLow); n.sprintf("myTempPal%d_edMid", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edMid); n.sprintf("myTempPal%d_edLowbase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edLowbase); n.sprintf("myTempPal%d_edBase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edBase); n.sprintf("myTempPal%d_edHighBase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edHighBase); n.sprintf("myTempPal%d_edSHighBase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edSHighBase); n.sprintf("myTempPal%d_edPreOffset", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edPreOffset); n.sprintf("myTempPal%d_edPreOfsTime", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edPreOfsTime); n.sprintf("myTempPal%d_edAfterOfs", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edAfterOfs); n.sprintf("myTempPal%d_edKit_Low", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edKit_Low); n.sprintf("myTempPal%d_edKit_Mid", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edKit_Mid); n.sprintf("myTempPal%d_edKit_Lowbase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edKit_Lowbase); n.sprintf("myTempPal%d_edKit_Base", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edKit_Base); n.sprintf("myTempPal%d_edKit_HighBase", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edKit_HighBase); n.sprintf("myTempPal%d_edOffset", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edOffset); n.sprintf("myTempPal%d_edSingleLimit", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edSingleLimit); n.sprintf("myTempPal%d_edIndiTemp", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edIndiTemp); n.sprintf("myTempPal%d_edInitTempOffset", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edInitTempOffset); n.sprintf("myTempPal%d_edEOTTempOffset", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->edEOTTempOffset); n.sprintf("myTempPal%d_palTemp", i); filerw::ELKeep("TfTemp_Set", n.c_str(), fTemp_Set->myTempPal[i]->palTemp); AnsiString pal; pal.sprintf("myTempPal%d_palTemp", i); { n.sprintf("myTempPal%d_edLow", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edMid", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edLowbase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edBase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edHighBase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edSHighBase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edPreOffset", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edPreOfsTime", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edAfterOfs", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edKit_Low", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edKit_Mid", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edKit_Lowbase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edKit_Base", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edKit_HighBase", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edOffset", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edSingleLimit", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edIndiTemp", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edInitTempOffset", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } { n.sprintf("myTempPal%d_edEOTTempOffset", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents("TfTemp_Set", pr, 1); } } }   // golden uTemp_Set.h:1019 myTempPal[tcTotalCount]；頁面 id 合約 myTempPal<i>_<成員>（i＝tc* 通道索引）
static bool TS_PortPanelsBuilt() { return fTemp_Set!=NULL && fTemp_Set->myTempPal[0]!=NULL; }   // FileRW/Temperature.cpp 用（myTempPal 巨集之前）

// golden uTemp_Set.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
#define myTempPal TS_myTempPal                 // golden uTemp_Set.h:1019（替身包裝，指向移植樹 fTemp_Set->myTempPal）
static bool fShow=false;   // golden uTemp_Set.h:1017 —— 本 TU 自己的（不是 fTemp_Set->fShow：那是 WebStart.cpp:1945／Command.cpp:10247 的 START 互鎖訊號，網頁開著不能擋 START；同 ArmSpeed_File 等 C 路）
#define iTempMoldSet (fTemp_Set->iTempMoldSet)  // golden uTemp_Set.h:1018
#define bNeedChange (fTemp_Set->bNeedChange)    // golden uTemp_Set.h:1043
#define MaxTempSetting() (fTemp_Set->MaxTempSetting())   // golden :5692（移植樹 uTemp_Set.cpp，純計算）
#define MinTempSetting() (fTemp_Set->MinTempSetting())   // golden :5765（同上）
#define ControlATC60AirFlow(...) (fTemp_Set->ControlATC60AirFlow(__VA_ARGS__))   // golden :6240 ⚠ 硬體（ATC6.0 氣流）；移植樹本體 SAFETY GATE (S13) 全擋
#define FormHS TS_FormHS()                      // golden HS_Function.h FormHS（移植樹 FormHS 名字被 SCK_ART_Remainder.h 的 stub 佔走）

using filerw::EL;

// 前置宣告
static void TS_FormShow();
static void TS_SetTempPanelCaption();
static void TS_ReadTempFile(bool bUpdateAll);
static void TS_DoIniDataToForm(bool bUpdateAll);
static void TS_UpDateEdit();
static void TS_cbATCReferTempSensorClick();
static void TS_rbATCActiveOnClick();
static void TS_rbATC70ActiveOnClick();
static void TS_FormClose();
static void TS_spbSaveClick();
static void TS_SaveSetupFile(AnsiString szDir, AnsiString S);
static bool TS_CheckTempSettingChange();
static void TS_DisplayTargetTempEdit(bool Visible);

// golden uTemp_Set.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void TS_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfTemp_Set", "cbSelectDevice")->Items;
      s->Add("IndexArm1、IndexArm2");
      s->Add("Shuttle1、HotPlate1");
      s->Add("Shuttle2、HotPlate2");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->Items;
      s->Add("Enable");
      s->Add("Disable");
      s->Add("By L/B monitoring function");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgAmbCheck")->Items;
      s->Add("Start");
      s->Add("Running");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->Items;
      s->Add("System Temp. Offset Setting");
      s->Add("Kit Temp. Offset Setting");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgBoostMode")->Items;
      s->Add("1 No heater wait");
      s->Add("2 Wait Heat up and down");
      s->Add("3 Wait Heat up only");
      s->Add("4 Wait Heat down only");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Items;
      s->Add("[0] Head Only");
      s->Add("[1] Chamber Only");
      s->Add("[2] Head + Chamber");
      s->Add("[3] Chamber + Socket");
      s->Add("[4] Head + Socket");
      s->Add("[5] Head + Chamber + Socket");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->Items;
      s->Add("In");
      s->Add("Out");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items;
      s->Add("High");
      s->Add("Ambient");
      s->Add("Ambient/High/Temp control");
      s->Add("Temp control");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgTjMapType")->Items;
      s->Add("Not send");
      s->Add("Auto Map");
      s->Add("Define");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rgZ1Down")->Items;
      s->Add("Off");
      s->Add("On");
    }
    { TStringList* s = EL<TRadioGroup>("TfTemp_Set", "rg_TjMode")->Items;
      s->Add("By arm");
      s->Add("Real time");
      s->Add("Average              times");
    }
}

// golden uTemp_Set.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kTS_ParentOf[][2] = {
    {"GroupBox1", "tsInitOffset"},
    {"GroupBox2", "tsATCMultiSensor"},
    {"GroupBox6", "gbDefrostFunction"},
    {"Image2", "tsDUT"},
    {"Label44", "gbHotModeSetting"},
    {"Label45", "gbHotModeSetting"},
    {"PageControl1", "rgIndexHeatMode1"},
    {"Panel129", "tsATC_FFC"},
    {"Panel14", "tsBoostFunc"},
    {"Panel17", "gbTempOffset"},
    {"Panel18", "palATC"},
    {"Panel19", "palATC"},
    {"Panel20", "palATC"},
    {"Panel22", "tsLBCtr"},
    {"Panel24", "grpLBPre"},
    {"Panel26", "grpLBPre"},
    {"Panel30", "tsATC"},
    {"Panel33", "grpLBPre"},
    {"Panel38", "GroupBox1"},
    {"PnlDewPointAlarm", "gbHotModeSetting"},
    {"PnlIndexSoakTime", "gbHotModeSetting"},
    {"RachoiceATCFile", "gbFileName"},
    {"bgTempOffset", "tsInitOffset"},
    {"btClearAll", "pnlOffsetBase"},
    {"btnSort", "Panel17"},
    {"cb3SigmaTempMonitior_Enable", "gbcb3SigmaTempMonitior"},
    {"cbATCReferTempSensor", "Panel30"},
    {"cbATCTempRise", "grpTemperatureRise"},
    {"cbATCTestTimeOffset", "gbATCTestTimeOffset"},
    {"cbAfterAutoCleanFunctionUseInitialDelay", "pnlAutoClean"},
    {"cbAfterOpenHeatDoorUseInitialDelay", "pnlHeatDoor"},
    {"cbAfterShowAlarmMessageUseInitialDelay", "pnlAlarmMsg"},
    {"cbBlowLB", "palLBAir"},
    {"cbByPassChamber", "gbAmbientModeSetting"},
    {"cbEnableATCConsFailOffset", "gbATCConsFail"},
    {"cbEnableATCQAModeOffset", "gbATCQAModeOffset"},
    {"cbEnableIndividualMode", "gbHotModeSetting"},
    {"cbEnableTJFunction", "grpTJFunction"},
    {"cbEnableTesterDryAirControl", "GroupBox6"},
    {"cbEveryFirstDeviceUseInitialDelay", "pnl1stDevice"},
    {"cbFFCOffsetEnable", "Panel129"},
    {"cbHeaterGun", "tsHeatGun"},
    {"cbLBTempHighAlarmEnable", "tsOthers"},
    {"cbLBTempLowAlarmEnable", "tsOthers"},
    {"cbMultiSensorOffsetEnable", "GroupBox2"},
    {"cbMultiZoneFunction", "gbMultiZone"},
    {"cbOTDUnlockDelay", "pnlOTD"},
    {"cbPowerFollow_Enable", "gbPowerFollowFuncAct"},
    {"cbShowFixTemp", "tsDUT"},
    {"cbTempOffset", "bgTempOffset"},
    {"cbTestFinishToNextTestOver", "pnlEOTTime"},
    {"cbTestStartToNextTestStart", "Panel38"},
    {"cbTjWatchdog", "grpTJFunction"},
    {"cbUseAbitCHK", "gbAmbientModeSetting"},
    {"cbUseAmbGuard", "rgAmbientBrand"},
    {"cbUseFixTemp", "tsDUT"},
    {"cbUseOtherArmToTestAfterInitialDelay", "pnlUseOtherArmToTestAfterInitialDelay"},
    {"cbUseTC2Offset", "Panel30"},
    {"cbWaitDewPoint", "PnlDewPointAlarm"},
    {"cbWhenHappenTestedTimeBelowUseInitialDelay", "pnlLowTestTime"},
    {"cbWhenNoFullSiteUseInitialDelay", "pnlNoFullsite"},
    {"cbWhenPressStopOverUseInitialDelay", "pnlPause"},
    {"cbbATC_RecipeFile", "gbFileName"},
    {"cdATCUsePIDCtrl", "pnlATC_PIDOffset"},
    {"chkAmbUsingAFan", "gbAmbientModeSetting"},
    {"chkBoostFunction", "Panel14"},
    {"chkBoostOffset", "pnlBoostOffset"},
    {"chkChamberBoost", "grpChamberBoost"},
    {"chkLBInitOffset", "pnlLBInitialOffset"},
    {"chkLBTempFunction", "Panel22"},
    {"chkSLKNoHeatUp", "grpTempCtrl"},
    {"chkShuttleNoHeatUp", "grpTempCtrl"},
    {"chkSoakTimeReduce", "gbHotModeSetting"},
    {"chkTempCalByRecipe", "Panel17"},
    {"chkTempErrorAction", "gbHotModeSetting"},
    {"chkTesterDock", "gbHotModeSetting"},
    {"chkTriTempEnableBase", "tsTriTemp"},
    {"chkTriTempEnableIndex", "tsTriTemp"},
    {"chkTriTempEnableOutSht", "tsTriTemp"},
    {"chkUseCDAOnly", "tsHeatGun"},
    {"chk_EnableArm_1_Air", "grpIndexAirStream"},
    {"chk_EnableArm_2_Air", "grpIndexAirStream"},
    {"chk_EnableSocket_Air", "grpSocketAirStream"},
    {"chk_IndexHeaterDoor", "tsTriTemp"},
    {"chk_UseTj", "gb_TjFunction"},
    {"edATCAmbTemp", "gbATCAmbientTemp"},
    {"edATCContFailOffset1", "gbATCConsFail"},
    {"edATCContFailOffset2", "gbATCConsFail"},
    {"edATCContFailOffset3", "gbATCConsFail"},
    {"edATCContFailOffset4", "gbATCConsFail"},
    {"edATCContFailOffsetCnt", "gbATCConsFail"},
    {"edATCInitialOffset1", "gbATCInitialOffset"},
    {"edATCInitialOffset2", "gbATCInitialOffset"},
    {"edATCInitialOffset3", "gbATCInitialOffset"},
    {"edATCInitialOffset4", "gbATCInitialOffset"},
    {"edATCOfsTime", "grpATCTempOffset"},
    {"edATCQAModeOffset1", "gbATCQAModeOffset"},
    {"edATCQAModeOffset2", "gbATCQAModeOffset"},
    {"edATCQAModeOffset3", "gbATCQAModeOffset"},
    {"edATCQAModeOffset4", "gbATCQAModeOffset"},
    {"edATCTestTimeOffset", "gbATCTestTimeOffset"},
    {"edATCTestTimeOffset1", "gbATCTestTimeOffset"},
    {"edATCTestTimeOffset2", "gbATCTestTimeOffset"},
    {"edATCTestTimeOffset3", "gbATCTestTimeOffset"},
    {"edATCTestTimeOffset4", "gbATCTestTimeOffset"},
    {"edAbLow", "pnlOffsetBase"},
    {"edAbMid", "pnlOffsetBase"},
    {"edAbitColdTime", "gbAmbientModeSetting"},
    {"edAbitInitWaitTime", "gbAmbientModeSetting"},
    {"edAmbGuardband", "rgAmbientBrand"},
    {"edAmbHotGuartbent", "grpTempCtrl"},
    {"edAmbTemp", "gbAmbientModeSetting"},
    {"edArm1NoFullsiteOffset_1", "gbNoFullsiteOffset"},
    {"edArm1NoFullsiteOffset_2", "gbNoFullsiteOffset"},
    {"edArm1NoFullsiteOffset_3", "gbNoFullsiteOffset"},
    {"edArm1NoFullsiteOffset_4", "gbNoFullsiteOffset"},
    {"edArm1NoFullsiteOffset_5", "gbNoFullsiteOffset"},
    {"edArm1Offset", "pnlArm1Offset"},
    {"edArm2NoFullsiteOffset_1", "gbNoFullsiteOffset"},
    {"edArm2NoFullsiteOffset_2", "gbNoFullsiteOffset"},
    {"edArm2NoFullsiteOffset_3", "gbNoFullsiteOffset"},
    {"edArm2NoFullsiteOffset_4", "gbNoFullsiteOffset"},
    {"edArm2NoFullsiteOffset_5", "gbNoFullsiteOffset"},
    {"edArm2Offset", "pnlArm2Offset"},
    {"edAtcFileName", "gbFileName"},
    {"edBelowSec", "pnlLowTestTime"},
    {"edChamberCoolTemp", "gbHotModeSetting"},
    {"edChillerTemp", "gbChillerTemp"},
    {"edContinuousSec", "grpTJFunction"},
    {"edDewPointAlarmInterval", "PnlDewPointAlarm"},
    {"edDewPointRange", "PnlDewPointAlarm"},
    {"edFixedTemp", "tsDUT"},
    {"edHighBase", "pnlOffsetBase"},
    {"edIndexSoakTime", "PnlIndexSoakTime"},
    {"edInitialStart1Time", "gbHotModeSetting"},
    {"edInitialStart2Time", "gbHotModeSetting"},
    {"edInitialWaitTime", "gbHotModeSetting"},
    {"edJamSoakTime", "gbHotModeSetting"},
    {"edLBTempAlarmTime", "palLBAir"},
    {"edLBTempHighSettingValue", "tsOthers"},
    {"edLBTempLowSettingValue", "tsOthers"},
    {"edLowBase", "pnlOffsetBase"},
    {"edMidBase", "pnlOffsetBase"},
    {"edOSTime", "PnlIndexSoakTime"},
    {"edOverSec", "pnlPause"},
    {"edSHighBase", "pnlOffsetBase"},
    {"edSoakTime", "gbHotModeSetting"},
    {"edSocketAirCoolingOff", "gbSocketAirCooling"},
    {"edSocketAirCoolingOn", "gbSocketAirCooling"},
    {"edTJTempRange_High", "grpTJTempRange"},
    {"edTJTempRange_Low", "grpTJTempRange"},
    {"edTSDTimeOut", "gbTSD"},
    {"edTempAlwaysSameAlarm", "grpTempAlwaysSameTime"},
    {"edTempDownContactDelay", "bgTempOffset"},
    {"edTempOffsetCount", "bgTempOffset"},
    {"edTempReadyRange", "bgTempOffset"},
    {"edWorkTemp", "pnlOffsetBase"},
    {"edt3SigmaTempMonitior_Set3xSigmaValue", "gbcb3SigmaTempMonitior"},
    {"edt3SigmaTempMonitior_SetCount", "gbcb3SigmaTempMonitior"},
    {"edtATCInPC1", "grpInPC"},
    {"edtATCInPC2", "grpInPC"},
    {"edtATCInPC3", "grpInPC"},
    {"edtATCInPC4", "grpInPC"},
    {"edtATCPIDOffset_MaxD", "pnlATC_PIDOffset"},
    {"edtATCPIDOffset_MaxI", "pnlATC_PIDOffset"},
    {"edtATCPIDOffset_MaxP", "pnlATC_PIDOffset"},
    {"edtATCPIDOffset_MinD", "pnlATC_PIDOffset"},
    {"edtATCPIDOffset_MinI", "pnlATC_PIDOffset"},
    {"edtATCPIDOffset_MinP", "pnlATC_PIDOffset"},
    {"edtATCSP2", "grbATCTempAlarm"},
    {"edtATC_HotGunTemp", "grpATC_HotAir"},
    {"edtATC_HotGunTime", "grpATC_HotAir"},
    {"edtATC_Type", "gbATC_Type"},
    {"edtBoostDuration_LB", "pnlLBInitialOffset3"},
    {"edtBoostDuration_Long", "pnlBoostDuration_Long"},
    {"edtBoostDuration_Mid", "pnlBoostDuration_Mid"},
    {"edtBoostDuration_Short", "pnlBoostDuration_Short"},
    {"edtBoostOffset", "pnlBoostOffset2"},
    {"edtBoostOffset_LB", "pnlLBInitialOffset2"},
    {"edtBoostOffset_Long", "pnlBoostOffset_Long"},
    {"edtBoostOffset_Mid", "pnlBoostOffset_Mid"},
    {"edtBoostOffset_Short", "pnlBoostOffset_Short"},
    {"edtBoostTempMin", "pnlBoostOffset1"},
    {"edtChamberBoostOffset", "grpChamberBoost"},
    {"edtChamberBoostTime", "grpChamberBoost"},
    {"edtDelayAfterSOT", "grpTJFunction"},
    {"edtHeatGunTempATC", "gbHeatGunTempATC"},
    {"edtHotGunFLowLimit_H", "gb_HotGunFlow"},
    {"edtHotGunFLowLimit_L", "gb_HotGunFlow"},
    {"edtIdleTime_LB", "pnlLBInitialOffset1"},
    {"edtIdleTime_Long", "pnlIdleTime_Long"},
    {"edtIdleTime_Mid", "pnlIdleTime_Mid"},
    {"edtIdleTime_Short", "pnlIdleTime_Short"},
    {"edtInitialDelay_1", "pnl1stDevice"},
    {"edtInitialDelay_10", "Panel38"},
    {"edtInitialDelay_10_RT", "Panel38"},
    {"edtInitialDelay_1_RT", "pnl1stDevice"},
    {"edtInitialDelay_2", "pnlAlarmMsg"},
    {"edtInitialDelay_2_RT", "pnlAlarmMsg"},
    {"edtInitialDelay_3", "pnlAutoClean"},
    {"edtInitialDelay_3_RT", "pnlAutoClean"},
    {"edtInitialDelay_4", "pnlLowTestTime"},
    {"edtInitialDelay_4_RT", "pnlLowTestTime"},
    {"edtInitialDelay_5", "pnlHeatDoor"},
    {"edtInitialDelay_5_RT", "pnlHeatDoor"},
    {"edtInitialDelay_6", "pnlPause"},
    {"edtInitialDelay_6_RT", "pnlPause"},
    {"edtInitialDelay_7", "pnlNoFullsite"},
    {"edtInitialDelay_7_RT", "pnlNoFullsite"},
    {"edtInitialDelay_8", "pnlEOTTime"},
    {"edtInitialDelay_8_RT", "pnlEOTTime"},
    {"edtInitialDelay_9", "pnlOTD"},
    {"edtInitialDelay_9_RT", "pnlOTD"},
    {"edtInputVHigh", "grpTJFunction"},
    {"edtInputVLow", "grpTJFunction"},
    {"edtLBAirOnTemp", "palLBAir"},
    {"edtLBTempMin", "Panel24"},
    {"edtLBTempOffset", "Panel26"},
    {"edtLBTimeOut", "Panel33"},
    {"edtPostBoost_LB", "pnlLBInitialOffset4"},
    {"edtPostBoost_Long", "pnlPostBoost_Long"},
    {"edtPostBoost_Mid", "pnlPostBoost_Mid"},
    {"edtPostBoost_Short", "pnlPostBoost_Short"},
    {"edtPowerFollow_FullPower", "gbPowerFollowFuncAct"},
    {"edtPowerFollower_Many2one", "gbPowerFollowFuncAct"},
    {"edtPowerFollower_PFSlope", "gbPowerFollowFuncAct"},
    {"edtPowerFollower_WGain", "gbPowerFollowFuncAct"},
    {"edtSetTJ_Offset", "grpTJFunction"},
    {"edtSetTJ_Slope", "grpTJFunction"},
    {"edtSetTempature2AirMachine", "grpSocketAirStream"},
    {"edtTestDock", "gbHotModeSetting"},
    {"edtThreshold", "pnlthreshold"},
    {"edt_AirVolumeLmt_Index", "grpIndexAirStream"},
    {"edt_AirVolumeLmt_Socket", "grpSocketAirStream"},
    {"edt_Defrost_Time_Too_Lower", "GroupBox6"},
    {"edt_SetAirstreamTemperatureRang_Index", "grpIndexAirStream"},
    {"edt_SetAirstreamTemperatureRang_Socket", "grpSocketAirStream"},
    {"edt_SetIndexAirstreamTemp", "grpIndexAirStream"},
    {"edt_TempRiseTemp", "grpTemperatureRise"},
    {"edt_TempRise_Delay", "grpTemperatureRise"},
    {"edt_TjAvgTimes", "gb_TjFunction"},
    {"gbATC70", "tsATC70"},
    {"gbATCAmbientTemp", "Panel19"},
    {"gbATCConsFail", "tsATC70"},
    {"gbATCInitialOffset", "tsATC70"},
    {"gbATCPackageOffset", "Panel19"},
    {"gbATCQAModeOffset", "tsATC70"},
    {"gbATCTestTimeOffset", "tsATC70"},
    {"gbATC_Type", "Panel19"},
    {"gbAmbientModeSetting", "tsAmbient"},
    {"gbBasePoint", "pnlHeatMode"},
    {"gbChillerTemp", "Panel19"},
    {"gbDefrostFunction", "tsTriTempSet"},
    {"gbFileName", "Panel30"},
    {"gbHeatGunTempATC", "Panel19"},
    {"gbHotModeSetting", "tsHot"},
    {"gbMultiZone", "tsATC"},
    {"gbNoFullsiteOffset", "gbHotModeSetting"},
    {"gbPowerFollowFuncAct", "Panel20"},
    {"gbSocketAirCooling", "gbAmbientModeSetting"},
    {"gbTSD", "tsATC70"},
    {"gbTempOffset", "Panel1"},
    {"gb_HotGunFlow", "tsHeatGun"},
    {"gb_TjFunction", "Panel18"},
    {"gbcb3SigmaTempMonitior", "tsSigmaForTemp"},
    {"grbATCTempAlarm", "Panel18"},
    {"grpATCTempOffset", "Panel20"},
    {"grpATC_HotAir", "tsHeatGun"},
    {"grpBoostMode", "tsBoostFunc"},
    {"grpBoostOffset", "tsLBCtr"},
    {"grpChamberBoost", "tsInitOffset"},
    {"grpInPC", "Panel20"},
    {"grpIndexAirStream", "tsHeatGun"},
    {"grpLBInitialOffset", "tsLBCtr"},
    {"grpLBPre", "tsLBCtr"},
    {"grpLongPrecondition", "tsBoostFunc"},
    {"grpMidPrecondition", "tsBoostFunc"},
    {"grpRS232", "Panel19"},
    {"grpShortPrecondition", "tsBoostFunc"},
    {"grpSocketAirStream", "tsHeatGun"},
    {"grpTJFunction", "Panel20"},
    {"grpTJTempRange", "Panel18"},
    {"grpTempAlwaysSameTime", "Panel19"},
    {"grpTempCtrl", "tsAmbCtr"},
    {"grpTemperatureCheck", "Panel18"},
    {"grpTemperatureRise", "Panel18"},
    {"iTestFinishToNextTestOver", "pnlEOTTime"},
    {"iTeststartToNextTestStart", "Panel38"},
    {"labReadyTempRange", "bgTempOffset"},
    {"lblFT", "pnlTitle"},
    {"lblNotice", "Panel17"},
    {"lblRT", "pnlTitle"},
    {"lblShtSoakTime", "gbHotModeSetting"},
    {"lblTempBase", "pnlOffsetBase"},
    {"lbllblShtSoakTimeSec", "gbHotModeSetting"},
    {"palATC", "tsATC"},
    {"palIndiTemp", "Panel17"},
    {"palLBAir", "gbHotModeSetting"},
    {"pgcOtherFunc", "tsOther"},
    {"pgcTempOffset", "gbTempOffset"},
    {"pgcTempSetting", "Panel8"},
    {"pnl1stDevice", "GroupBox1"},
    {"pnlATC_PIDOffset", "tsATC_PID"},
    {"pnlAlarmMsg", "GroupBox1"},
    {"pnlArm1Offset", "pnlOffsetBase"},
    {"pnlArm2Offset", "pnlOffsetBase"},
    {"pnlAutoClean", "GroupBox1"},
    {"pnlBoostDuration_Long", "grpLongPrecondition"},
    {"pnlBoostDuration_Mid", "grpMidPrecondition"},
    {"pnlBoostDuration_Short", "grpShortPrecondition"},
    {"pnlBoostOffset", "grpBoostOffset"},
    {"pnlBoostOffset1", "grpBoostOffset"},
    {"pnlBoostOffset2", "grpBoostOffset"},
    {"pnlBoostOffset_Long", "grpLongPrecondition"},
    {"pnlBoostOffset_Mid", "grpMidPrecondition"},
    {"pnlBoostOffset_Short", "grpShortPrecondition"},
    {"pnlCover", "rgTemperatureMode1"},
    {"pnlEOTTime", "GroupBox1"},
    {"pnlHeatDoor", "GroupBox1"},
    {"pnlHeatMode", "Panel1"},
    {"pnlIdleTime_Long", "grpLongPrecondition"},
    {"pnlIdleTime_Mid", "grpMidPrecondition"},
    {"pnlIdleTime_Short", "grpShortPrecondition"},
    {"pnlLBInitialOffset", "grpLBInitialOffset"},
    {"pnlLBInitialOffset1", "grpLBInitialOffset"},
    {"pnlLBInitialOffset2", "grpLBInitialOffset"},
    {"pnlLBInitialOffset3", "grpLBInitialOffset"},
    {"pnlLBInitialOffset4", "grpLBInitialOffset"},
    {"pnlLowTestTime", "GroupBox1"},
    {"pnlNoFullsite", "GroupBox1"},
    {"pnlOTD", "GroupBox1"},
    {"pnlOffsetBase", "gbTempOffset"},
    {"pnlPause", "GroupBox1"},
    {"pnlPostBoost_Long", "grpLongPrecondition"},
    {"pnlPostBoost_Mid", "grpMidPrecondition"},
    {"pnlPostBoost_Short", "grpShortPrecondition"},
    {"pnlTitle", "GroupBox1"},
    {"pnlUseOtherArmToTestAfterInitialDelay", "tsInitOffset"},
    {"pnlthreshold", "grpLBPre"},
    {"rb1Point", "gbBasePoint"},
    {"rb2Point", "gbBasePoint"},
    {"rb3Point", "gbBasePoint"},
    {"rb5Point", "gbBasePoint"},
    {"rb6Point", "gbBasePoint"},
    {"rbATC70ActiveOn", "gbATC70"},
    {"rbATC7TSDFunction", "gbTSD"},
    {"rbATCActiveOn", "Panel30"},
    {"rbHandlerStartOn", "grpTemperatureCheck"},
    {"rbTemp", "Panel7"},
    {"rbTemperatureSetOn", "grpRS232"},
    {"rbTestStartOn", "grpTemperatureCheck"},
    {"rgATC60AirOn", "Panel18"},
    {"rgAmbCheck", "gbAmbientModeSetting"},
    {"rgAmbientBrand", "gbAmbientModeSetting"},
    {"rgBasePoint", "Panel17"},
    {"rgBoostMode", "grpBoostMode"},
    {"rgIndexHeatMode", "tsStandard"},
    {"rgIndexHeatMode1", "pnlHeatMode"},
    {"rgShuttleSoakTime", "gbHotModeSetting"},
    {"rgTemperatureMode", "rgTemperatureMode1"},
    {"rgTemperatureMode1", "Panel8"},
    {"rgTjMapType", "grpTJFunction"},
    {"rgZ1Down", "gbHotModeSetting"},
    {"rg_TjMode", "gb_TjFunction"},
    {"sbSafeTestATC", "Panel30"},
    {"sgTjMap", "grpTJFunction"},
    {"spbSave", "Panel7"},
    {"tsATC", "pgcTempOffset"},
    {"tsATC70", "pgcTempOffset"},
    {"tsATCMultiSensor", "pgcTempOffset"},
    {"tsATC_FFC", "pgcTempOffset"},
    {"tsATC_PID", "pgcTempOffset"},
    {"tsAmbCtr", "pgcTempSetting"},
    {"tsAmbient", "pgcTempSetting"},
    {"tsArm1", "pgcTempOffset"},
    {"tsArm2", "pgcTempOffset"},
    {"tsBoostFunc", "pgcOtherFunc"},
    {"tsDUT", "pgcTempOffset"},
    {"tsHeatGun", "pgcTempOffset"},
    {"tsHot", "pgcTempSetting"},
    {"tsInitOffset", "pgcOtherFunc"},
    {"tsLBCtr", "pgcOtherFunc"},
    {"tsOther", "pgcTempSetting"},
    {"tsOthers", "pgcOtherFunc"},
    {"tsSigmaForTemp", "pgcOtherFunc"},
    {"tsStandard", "PageControl1"},
    {"tsTriTemp", "PageControl1"},
    {"tsTriTempSet", "pgcTempSetting"},
    {"tsUnUse", "pgcTempOffset"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void TS_CreateContainerProxies()
{
    EL<TGroupBox>("TfTemp_Set", "GroupBox1");
    EL<TGroupBox>("TfTemp_Set", "GroupBox2");
    EL<TGroupBox>("TfTemp_Set", "GroupBox6");
    EL<TPageControl>("TfTemp_Set", "PageControl1");
    EL<TPanel>("TfTemp_Set", "Panel1");
    EL<TPanel>("TfTemp_Set", "Panel129");
    EL<TPanel>("TfTemp_Set", "Panel14");
    EL<TPanel>("TfTemp_Set", "Panel17");
    EL<TPanel>("TfTemp_Set", "Panel18");
    EL<TPanel>("TfTemp_Set", "Panel19");
    EL<TPanel>("TfTemp_Set", "Panel20");
    EL<TPanel>("TfTemp_Set", "Panel22");
    EL<TPanel>("TfTemp_Set", "Panel24");
    EL<TPanel>("TfTemp_Set", "Panel26");
    EL<TPanel>("TfTemp_Set", "Panel30");
    EL<TPanel>("TfTemp_Set", "Panel33");
    EL<TPanel>("TfTemp_Set", "Panel38");
    EL<TPanel>("TfTemp_Set", "Panel7");
    EL<TPanel>("TfTemp_Set", "Panel8");
    EL<TPanel>("TfTemp_Set", "PnlDewPointAlarm");
    EL<TPanel>("TfTemp_Set", "PnlIndexSoakTime");
    EL<TGroupBox>("TfTemp_Set", "bgTempOffset");
    EL<TGroupBox>("TfTemp_Set", "gbATC70");
    EL<TGroupBox>("TfTemp_Set", "gbATCAmbientTemp");
    EL<TGroupBox>("TfTemp_Set", "gbATCConsFail");
    EL<TGroupBox>("TfTemp_Set", "gbATCInitialOffset");
    EL<TGroupBox>("TfTemp_Set", "gbATCQAModeOffset");
    EL<TGroupBox>("TfTemp_Set", "gbATCTestTimeOffset");
    EL<TGroupBox>("TfTemp_Set", "gbATC_Type");
    EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting");
    EL<TGroupBox>("TfTemp_Set", "gbBasePoint");
    EL<TGroupBox>("TfTemp_Set", "gbChillerTemp");
    EL<TGroupBox>("TfTemp_Set", "gbDefrostFunction");
    EL<TGroupBox>("TfTemp_Set", "gbFileName");
    EL<TGroupBox>("TfTemp_Set", "gbHeatGunTempATC");
    EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting");
    EL<TGroupBox>("TfTemp_Set", "gbMultiZone");
    EL<TGroupBox>("TfTemp_Set", "gbNoFullsiteOffset");
    EL<TGroupBox>("TfTemp_Set", "gbPowerFollowFuncAct");
    EL<TGroupBox>("TfTemp_Set", "gbSocketAirCooling");
    EL<TGroupBox>("TfTemp_Set", "gbTSD");
    EL<TGroupBox>("TfTemp_Set", "gbTempOffset");
    EL<TGroupBox>("TfTemp_Set", "gb_HotGunFlow");
    EL<TGroupBox>("TfTemp_Set", "gb_TjFunction");
    EL<TGroupBox>("TfTemp_Set", "gbcb3SigmaTempMonitior");
    EL<TGroupBox>("TfTemp_Set", "grbATCTempAlarm");
    EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset");
    EL<TGroupBox>("TfTemp_Set", "grpATC_HotAir");
    EL<TGroupBox>("TfTemp_Set", "grpBoostMode");
    EL<TGroupBox>("TfTemp_Set", "grpBoostOffset");
    EL<TGroupBox>("TfTemp_Set", "grpChamberBoost");
    EL<TGroupBox>("TfTemp_Set", "grpInPC");
    EL<TGroupBox>("TfTemp_Set", "grpIndexAirStream");
    EL<TGroupBox>("TfTemp_Set", "grpLBInitialOffset");
    EL<TGroupBox>("TfTemp_Set", "grpLBPre");
    EL<TGroupBox>("TfTemp_Set", "grpLongPrecondition");
    EL<TGroupBox>("TfTemp_Set", "grpMidPrecondition");
    EL<TGroupBox>("TfTemp_Set", "grpRS232");
    EL<TGroupBox>("TfTemp_Set", "grpShortPrecondition");
    EL<TGroupBox>("TfTemp_Set", "grpSocketAirStream");
    EL<TGroupBox>("TfTemp_Set", "grpTJFunction");
    EL<TGroupBox>("TfTemp_Set", "grpTJTempRange");
    EL<TGroupBox>("TfTemp_Set", "grpTempAlwaysSameTime");
    EL<TGroupBox>("TfTemp_Set", "grpTempCtrl");
    EL<TGroupBox>("TfTemp_Set", "grpTemperatureCheck");
    EL<TGroupBox>("TfTemp_Set", "grpTemperatureRise");
    EL<TPanel>("TfTemp_Set", "palATC");
    EL<TPanel>("TfTemp_Set", "palLBAir");
    EL<TPageControl>("TfTemp_Set", "pgcOtherFunc");
    EL<TPageControl>("TfTemp_Set", "pgcTempOffset");
    EL<TPageControl>("TfTemp_Set", "pgcTempSetting");
    EL<TPanel>("TfTemp_Set", "pnl1stDevice");
    EL<TPanel>("TfTemp_Set", "pnlATC_PIDOffset");
    EL<TPanel>("TfTemp_Set", "pnlAlarmMsg");
    EL<TPanel>("TfTemp_Set", "pnlArm1Offset");
    EL<TPanel>("TfTemp_Set", "pnlArm2Offset");
    EL<TPanel>("TfTemp_Set", "pnlAutoClean");
    EL<TPanel>("TfTemp_Set", "pnlBoostDuration_Long");
    EL<TPanel>("TfTemp_Set", "pnlBoostDuration_Mid");
    EL<TPanel>("TfTemp_Set", "pnlBoostDuration_Short");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset1");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset2");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset_Long");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset_Mid");
    EL<TPanel>("TfTemp_Set", "pnlBoostOffset_Short");
    EL<TPanel>("TfTemp_Set", "pnlEOTTime");
    EL<TPanel>("TfTemp_Set", "pnlHeatDoor");
    EL<TPanel>("TfTemp_Set", "pnlHeatMode");
    EL<TPanel>("TfTemp_Set", "pnlIdleTime_Long");
    EL<TPanel>("TfTemp_Set", "pnlIdleTime_Mid");
    EL<TPanel>("TfTemp_Set", "pnlIdleTime_Short");
    EL<TPanel>("TfTemp_Set", "pnlLBInitialOffset");
    EL<TPanel>("TfTemp_Set", "pnlLBInitialOffset1");
    EL<TPanel>("TfTemp_Set", "pnlLBInitialOffset2");
    EL<TPanel>("TfTemp_Set", "pnlLBInitialOffset3");
    EL<TPanel>("TfTemp_Set", "pnlLBInitialOffset4");
    EL<TPanel>("TfTemp_Set", "pnlLowTestTime");
    EL<TPanel>("TfTemp_Set", "pnlNoFullsite");
    EL<TPanel>("TfTemp_Set", "pnlOTD");
    EL<TPanel>("TfTemp_Set", "pnlOffsetBase");
    EL<TPanel>("TfTemp_Set", "pnlPause");
    EL<TPanel>("TfTemp_Set", "pnlPostBoost_Long");
    EL<TPanel>("TfTemp_Set", "pnlPostBoost_Mid");
    EL<TPanel>("TfTemp_Set", "pnlPostBoost_Short");
    EL<TPanel>("TfTemp_Set", "pnlTitle");
    EL<TPanel>("TfTemp_Set", "pnlUseOtherArmToTestAfterInitialDelay");
    EL<TPanel>("TfTemp_Set", "pnlthreshold");
    EL<TGroupBox>("TfTemp_Set", "rgAmbientBrand");
    EL<TGroupBox>("TfTemp_Set", "rgIndexHeatMode1");
    EL<TGroupBox>("TfTemp_Set", "rgTemperatureMode1");
    EL<TTabSheet>("TfTemp_Set", "tsATC");
    EL<TTabSheet>("TfTemp_Set", "tsATC70");
    EL<TTabSheet>("TfTemp_Set", "tsATCMultiSensor");
    EL<TTabSheet>("TfTemp_Set", "tsATC_FFC");
    EL<TTabSheet>("TfTemp_Set", "tsATC_PID");
    EL<TTabSheet>("TfTemp_Set", "tsAmbCtr");
    EL<TTabSheet>("TfTemp_Set", "tsAmbient");
    EL<TTabSheet>("TfTemp_Set", "tsBoostFunc");
    EL<TTabSheet>("TfTemp_Set", "tsDUT");
    EL<TTabSheet>("TfTemp_Set", "tsHeatGun");
    EL<TTabSheet>("TfTemp_Set", "tsHot");
    EL<TTabSheet>("TfTemp_Set", "tsInitOffset");
    EL<TTabSheet>("TfTemp_Set", "tsLBCtr");
    EL<TTabSheet>("TfTemp_Set", "tsOther");
    EL<TTabSheet>("TfTemp_Set", "tsOthers");
    EL<TTabSheet>("TfTemp_Set", "tsSigmaForTemp");
    EL<TTabSheet>("TfTemp_Set", "tsStandard");
    EL<TTabSheet>("TfTemp_Set", "tsTriTemp");
    EL<TTabSheet>("TfTemp_Set", "tsTriTempSet");
    filerw::ELSetParents("TfTemp_Set", kTS_ParentOf, (int)(sizeof(kTS_ParentOf) / sizeof(kTS_ParentOf[0])));
}

// golden uTemp_Set.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void TS_DfmState()
{
    EL<TSpeedButton>("TfTemp_Set", "btnSort")->Visible = false;
    EL<TCheckBox>("TfTemp_Set", "cbShowFixTemp")->Visible = false;
    EL<TCheckBox>("TfTemp_Set", "cbUseFixTemp")->Visible = false;
    EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Visible = false;
    EL<TCheckBox>("TfTemp_Set", "chkLBInitOffset")->Enabled = false;
    EL<TEdit>("TfTemp_Set", "edAbitColdTime")->Visible = false;
    EL<TEdit>("TfTemp_Set", "edAbitInitWaitTime")->Visible = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edContinuousSec");
    EL<TEdit>("TfTemp_Set", "edFixedTemp")->Visible = false;
    EL<TEdit>("TfTemp_Set", "edInitialStart2Time")->Visible = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue");
    filerw::ELSetReadOnly("TfTemp_Set", "edt3SigmaTempMonitior_SetCount");
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Mid")->Enabled = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edtBoostDuration_Mid");
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Mid")->Enabled = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edtBoostOffset_Mid");
    filerw::ELSetReadOnly("TfTemp_Set", "edtDelayAfterSOT");
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Mid")->Enabled = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edtIdleTime_Mid");
    filerw::ELSetReadOnly("TfTemp_Set", "edtInputVHigh");
    filerw::ELSetReadOnly("TfTemp_Set", "edtInputVLow");
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Mid")->Enabled = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edtPostBoost_Mid");
    filerw::ELSetReadOnly("TfTemp_Set", "edtPowerFollow_FullPower");
    filerw::ELSetReadOnly("TfTemp_Set", "edtPowerFollower_Many2one");
    filerw::ELSetReadOnly("TfTemp_Set", "edtPowerFollower_PFSlope");
    filerw::ELSetReadOnly("TfTemp_Set", "edtPowerFollower_WGain");
    filerw::ELSetReadOnly("TfTemp_Set", "edtSetTJ_Offset");
    filerw::ELSetReadOnly("TfTemp_Set", "edtSetTJ_Slope");
    filerw::ELSetReadOnly("TfTemp_Set", "edtSetTempature2AirMachine");
    filerw::ELSetReadOnly("TfTemp_Set", "edt_AirVolumeLmt_Index");
    EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Socket")->Enabled = false;
    filerw::ELSetReadOnly("TfTemp_Set", "edt_AirVolumeLmt_Socket");
    filerw::ELSetReadOnly("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index");
    filerw::ELSetReadOnly("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket");
    filerw::ELSetReadOnly("TfTemp_Set", "edt_SetIndexAirstreamTemp");
    EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "gbFileName")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "gb_TjFunction")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "grpBoostOffset")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "grpInPC")->Visible = false;
    EL<TLabel>("TfTemp_Set", "lblShtSoakTime")->Visible = false;
    EL<TLabel>("TfTemp_Set", "lbllblShtSoakTimeSec")->Visible = false;
    EL<TPanel>("TfTemp_Set", "pnlCover")->Visible = false;
    EL<TPanel>("TfTemp_Set", "pnlUseOtherArmToTestAfterInitialDelay")->Visible = false;
    EL<TRadioGroup>("TfTemp_Set", "rgAmbCheck")->Visible = false;
    EL<TGroupBox>("TfTemp_Set", "rgAmbientBrand")->Visible = false;
    EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->Visible = false;
    EL<TSpeedButton>("TfTemp_Set", "sbSafeTestATC")->Enabled = false;
    EL<TSpeedButton>("TfTemp_Set", "sbSafeTestATC")->Visible = false;
    EL<TRadioButton>("TfTemp_Set", "RachoiceATCFile")->Checked = true;
    EL<TCheckBox>("TfTemp_Set", "cbTempOffset")->Checked = true;
    EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")->Checked = true;
    EL<TCheckBox>("TfTemp_Set", "chkChamberBoost")->Checked = true;
    EL<TCheckBox>("TfTemp_Set", "chkLBInitOffset")->Checked = true;
    EL<TCheckBox>("TfTemp_Set", "chkLBTempFunction")->Checked = true;
    EL<TEdit>("TfTemp_Set", "edATCAmbTemp")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset1")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset2")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset3")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset4")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCContFailOffsetCnt")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset1")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset2")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset3")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset4")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCOfsTime")->Text = "600000";
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset1")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset2")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset3")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset4")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset1")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset2")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset3")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset4")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edAbLow")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edAbMid")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edAbitColdTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edAbitInitWaitTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edAmbGuardband")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edAmbTemp")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edArm1Offset")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edArm2Offset")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edAtcFileName")->Text = "1";
    EL<TEdit>("TfTemp_Set", "edBelowSec")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edChamberCoolTemp")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text = "130.0";
    EL<TEdit>("TfTemp_Set", "edContinuousSec")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edDewPointAlarmInterval")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edDewPointRange")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edFixedTemp")->Text = "65";
    EL<TEdit>("TfTemp_Set", "edHighBase")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edIndexSoakTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edInitialStart1Time")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edInitialStart2Time")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edInitialWaitTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edJamSoakTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edLBTempAlarmTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edLBTempHighSettingValue")->Text = "60";
    EL<TEdit>("TfTemp_Set", "edLBTempLowSettingValue")->Text = "60";
    EL<TEdit>("TfTemp_Set", "edLowBase")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edMidBase")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edOSTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edOverSec")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edSHighBase")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edSoakTime")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOff")->Text = "10";
    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOn")->Text = "1";
    EL<TEdit>("TfTemp_Set", "edTJTempRange_High")->Text = "10.0";
    EL<TEdit>("TfTemp_Set", "edTJTempRange_Low")->Text = "30.0";
    EL<TEdit>("TfTemp_Set", "edTSDTimeOut")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Text = "130.0";
    EL<TEdit>("TfTemp_Set", "edTempDownContactDelay")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edTempOffsetCount")->Text = "2";
    EL<TEdit>("TfTemp_Set", "edTempReadyRange")->Text = "2";
    EL<TEdit>("TfTemp_Set", "edWorkTemp")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue")->Text = "100";
    EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_SetCount")->Text = "100";
    EL<TEdit>("TfTemp_Set", "edtATCInPC1")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtATCInPC2")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtATCInPC3")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtATCInPC4")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxD")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxI")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxP")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinD")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinI")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinP")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtATCSP2")->Text = "130.0";
    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTemp")->Text = "50";
    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTime")->Text = "30";
    EL<TEdit>("TfTemp_Set", "edtATC_Type")->Text = "Rogers";
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_LB")->Text = "40";
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Long")->Text = "300";
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Mid")->Text = "40";
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Short")->Text = "40";
    EL<TEdit>("TfTemp_Set", "edtBoostOffset")->Text = "2";
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_LB")->Text = "1";
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Long")->Text = "21";
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Mid")->Text = "21";
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Short")->Text = "21";
    EL<TEdit>("TfTemp_Set", "edtBoostTempMin")->Text = "60";
    EL<TEdit>("TfTemp_Set", "edtChamberBoostOffset")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edtChamberBoostTime")->Text = "2";
    EL<TEdit>("TfTemp_Set", "edtDelayAfterSOT")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtHeatGunTempATC")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_H")->Text = "50";
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_L")->Text = "30";
    EL<TEdit>("TfTemp_Set", "edtIdleTime_LB")->Text = "10";
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Long")->Text = "400";
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Mid")->Text = "9";
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Short")->Text = "8";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9_RT")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "edtInputVHigh")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtInputVLow")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtLBAirOnTemp")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtLBTempMin")->Text = "60";
    EL<TEdit>("TfTemp_Set", "edtLBTempOffset")->Text = "20";
    EL<TEdit>("TfTemp_Set", "edtLBTimeOut")->Text = "1200";
    EL<TEdit>("TfTemp_Set", "edtPostBoost_LB")->Text = "20";
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Long")->Text = "60";
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Mid")->Text = "20";
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Short")->Text = "20";
    EL<TEdit>("TfTemp_Set", "edtPowerFollow_FullPower")->Text = "1000";
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_Many2one")->Text = "1";
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_PFSlope")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_WGain")->Text = "0.0";
    EL<TEdit>("TfTemp_Set", "edtSetTJ_Offset")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtSetTJ_Slope")->Text = "1";
    EL<TEdit>("TfTemp_Set", "edtSetTempature2AirMachine")->Text = "25";
    EL<TEdit>("TfTemp_Set", "edtTestDock")->Text = "0";
    EL<TEdit>("TfTemp_Set", "edtThreshold")->Text = "20";
    EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Index")->Text = "400";
    EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Socket")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edt_Defrost_Time_Too_Lower")->Text = "600";
    EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edt_SetIndexAirstreamTemp")->Text = "25";
    EL<TEdit>("TfTemp_Set", "edt_TempRiseTemp")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edt_TempRise_Delay")->Text = "5";
    EL<TEdit>("TfTemp_Set", "edt_TjAvgTimes")->Text = "5";
    EL<TEdit>("TfTemp_Set", "iTestFinishToNextTestOver")->Text = "0.00";
    EL<TEdit>("TfTemp_Set", "iTeststartToNextTestStart")->Text = "0.00";
    EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked = true;
    EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->ItemIndex = 0;
    EL<TRadioGroup>("TfTemp_Set", "rgAmbCheck")->ItemIndex = 1;
    EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->ItemIndex = 0;
    EL<TRadioGroup>("TfTemp_Set", "rgBoostMode")->ItemIndex = 2;
    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex = 1;
    EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->ItemIndex = 1;
    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex = 2;
    EL<TRadioGroup>("TfTemp_Set", "rgTjMapType")->ItemIndex = 0;
    EL<TRadioGroup>("TfTemp_Set", "rgZ1Down")->ItemIndex = 1;
    EL<TRadioGroup>("TfTemp_Set", "rg_TjMode")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kTS_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox6", "Label44", "Label45", "PageControl1",
    "Panel1", "Panel129", "Panel14", "Panel17", "Panel18", "Panel19",
    "Panel20", "Panel22", "Panel24", "Panel26", "Panel30", "Panel33",
    "Panel38", "Panel7", "Panel8", "PnlDewPointAlarm", "PnlIndexSoakTime", "RachoiceATCFile",
    "bgTempOffset", "btClearAll", "btnSort", "cb3SigmaTempMonitior_Enable", "cbATCReferTempSensor", "cbATCTempRise",
    "cbATCTestTimeOffset", "cbAfterAutoCleanFunctionUseInitialDelay", "cbAfterOpenHeatDoorUseInitialDelay", "cbAfterShowAlarmMessageUseInitialDelay", "cbBlowLB", "cbByPassChamber",
    "cbEnableATCConsFailOffset", "cbEnableATCQAModeOffset", "cbEnableIndividualMode", "cbEnableTJFunction", "cbEnableTesterDryAirControl", "cbEveryFirstDeviceUseInitialDelay",
    "cbFFCOffsetEnable", "cbHeaterGun", "cbLBTempHighAlarmEnable", "cbLBTempLowAlarmEnable", "cbMultiSensorOffsetEnable", "cbMultiZoneFunction",
    "cbOTDUnlockDelay", "cbPowerFollow_Enable", "cbShowFixTemp", "cbTempOffset", "cbTestFinishToNextTestOver", "cbTestStartToNextTestStart",
    "cbTjWatchdog", "cbUseAbitCHK", "cbUseAmbGuard", "cbUseFixTemp", "cbUseOtherArmToTestAfterInitialDelay", "cbUseTC2Offset",
    "cbWaitDewPoint", "cbWhenHappenTestedTimeBelowUseInitialDelay", "cbWhenNoFullSiteUseInitialDelay", "cbWhenPressStopOverUseInitialDelay", "cdATCUsePIDCtrl", "chkAmbUsingAFan",
    "chkBoostFunction", "chkBoostOffset", "chkChamberBoost", "chkLBInitOffset", "chkLBTempFunction", "chkSLKNoHeatUp",
    "chkShuttleNoHeatUp", "chkSoakTimeReduce", "chkTempCalByRecipe", "chkTempErrorAction", "chkTesterDock", "chkTriTempEnableBase",
    "chkTriTempEnableIndex", "chkTriTempEnableOutSht", "chkUseCDAOnly", "chk_EnableArm_1_Air", "chk_EnableArm_2_Air", "chk_EnableSocket_Air",
    "chk_IndexHeaterDoor", "chk_UseTj", "gbATC70", "gbATCAmbientTemp", "gbATCConsFail", "gbATCInitialOffset",
    "gbATCPackageOffset", "gbATCQAModeOffset", "gbATCTestTimeOffset", "gbATC_Type", "gbAmbientModeSetting", "gbBasePoint",
    "gbChillerTemp", "gbDefrostFunction", "gbFileName", "gbHeatGunTempATC", "gbHotModeSetting", "gbMultiZone",
    "gbNoFullsiteOffset", "gbPowerFollowFuncAct", "gbSocketAirCooling", "gbTSD", "gbTempOffset", "gb_HotGunFlow",
    "gb_TjFunction", "gbcb3SigmaTempMonitior", "grbATCTempAlarm", "grpATCTempOffset", "grpATC_HotAir", "grpBoostMode",
    "grpBoostOffset", "grpChamberBoost", "grpInPC", "grpIndexAirStream", "grpLBInitialOffset", "grpLBPre",
    "grpLongPrecondition", "grpMidPrecondition", "grpRS232", "grpShortPrecondition", "grpSocketAirStream", "grpTJFunction",
    "grpTJTempRange", "grpTempAlwaysSameTime", "grpTempCtrl", "grpTemperatureCheck", "grpTemperatureRise", "labReadyTempRange",
    "lblFT", "lblNotice", "lblRT", "lblShtSoakTime", "lblTempBase", "lbllblShtSoakTimeSec",
    "palATC", "palLBAir", "pgcOtherFunc", "pgcTempOffset", "pgcTempSetting", "pnl1stDevice",
    "pnlATC_PIDOffset", "pnlAlarmMsg", "pnlArm1Offset", "pnlArm2Offset", "pnlAutoClean", "pnlBoostDuration_Long",
    "pnlBoostDuration_Mid", "pnlBoostDuration_Short", "pnlBoostOffset", "pnlBoostOffset1", "pnlBoostOffset2", "pnlBoostOffset_Long",
    "pnlBoostOffset_Mid", "pnlBoostOffset_Short", "pnlCover", "pnlEOTTime", "pnlHeatDoor", "pnlHeatMode",
    "pnlIdleTime_Long", "pnlIdleTime_Mid", "pnlIdleTime_Short", "pnlLBInitialOffset", "pnlLBInitialOffset1", "pnlLBInitialOffset2",
    "pnlLBInitialOffset3", "pnlLBInitialOffset4", "pnlLowTestTime", "pnlNoFullsite", "pnlOTD", "pnlOffsetBase",
    "pnlPause", "pnlPostBoost_Long", "pnlPostBoost_Mid", "pnlPostBoost_Short", "pnlTitle", "pnlUseOtherArmToTestAfterInitialDelay",
    "pnlthreshold", "rb1Point", "rb2Point", "rb3Point", "rb5Point", "rb6Point",
    "rbATC70ActiveOn", "rbATC7TSDFunction", "rbATCActiveOn", "rbHandlerStartOn", "rbTemp", "rbTemperatureSetOn",
    "rbTestStartOn", "rgATC60AirOn", "rgAmbCheck", "rgAmbientBrand", "rgBasePoint", "rgBoostMode",
    "rgIndexHeatMode", "rgIndexHeatMode1", "rgShuttleSoakTime", "rgTemperatureMode", "rgTemperatureMode1", "rgTjMapType",
    "rgZ1Down", "rg_TjMode", "sbSafeTestATC", "spbSave", "tsATC", "tsATC70",
    "tsATCMultiSensor", "tsATC_FFC", "tsATC_PID", "tsAmbCtr", "tsAmbient", "tsArm1",
    "tsArm2", "tsBoostFunc", "tsDUT", "tsHeatGun", "tsHot", "tsInitOffset",
    "tsLBCtr", "tsOther", "tsOthers", "tsSigmaForTemp", "tsStandard", "tsTriTemp",
    "tsTriTempSet", "tsUnUse",
};

// 存檔流程（spbSaveClick、SaveSetupFile、CheckTempSettingChange）讀的替身 232 個 —— FileRW/Temperature.cpp 用它決定頁面必須送哪些值
static const char* const kTS_SaveReads[] = {
    "RachoiceATCFile", "cb3SigmaTempMonitior_Enable", "cbATCReferTempSensor", "cbATCTempRise", "cbATCTestTimeOffset", "cbAfterAutoCleanFunctionUseInitialDelay",
    "cbAfterOpenHeatDoorUseInitialDelay", "cbAfterShowAlarmMessageUseInitialDelay", "cbBlowLB", "cbByPassChamber", "cbEnableATCConsFailOffset", "cbEnableATCQAModeOffset",
    "cbEnableIndividualMode", "cbEnableTJFunction", "cbEnableTesterDryAirControl", "cbEveryFirstDeviceUseInitialDelay", "cbFFCOffsetEnable", "cbHeaterGun",
    "cbLBTempHighAlarmEnable", "cbLBTempLowAlarmEnable", "cbMultiSensorOffsetEnable", "cbMultiZoneFunction", "cbOTDUnlockDelay", "cbPowerFollow_Enable",
    "cbShowFixTemp", "cbTempOffset", "cbTestFinishToNextTestOver", "cbTestStartToNextTestStart", "cbTjWatchdog", "cbUseAbitCHK",
    "cbUseAmbGuard", "cbUseFixTemp", "cbUseOtherArmToTestAfterInitialDelay", "cbUseTC2Offset", "cbWaitDewPoint", "cbWhenHappenTestedTimeBelowUseInitialDelay",
    "cbWhenNoFullSiteUseInitialDelay", "cbWhenPressStopOverUseInitialDelay", "cbbATC_RecipeFile", "cdATCUsePIDCtrl", "chkAmbUsingAFan", "chkBoostFunction",
    "chkBoostOffset", "chkChamberBoost", "chkLBTempFunction", "chkSLKNoHeatUp", "chkShuttleNoHeatUp", "chkSoakTimeReduce",
    "chkTempCalByRecipe", "chkTempErrorAction", "chkTesterDock", "chkTriTempEnableBase", "chkTriTempEnableIndex", "chkTriTempEnableOutSht",
    "chkUseCDAOnly", "chk_EnableArm_1_Air", "chk_EnableArm_2_Air", "chk_EnableSocket_Air", "chk_IndexHeaterDoor", "chk_UseTj",
    "edATCAmbTemp", "edATCContFailOffset1", "edATCContFailOffset2", "edATCContFailOffset3", "edATCContFailOffset4", "edATCContFailOffsetCnt",
    "edATCInitialOffset1", "edATCInitialOffset2", "edATCInitialOffset3", "edATCInitialOffset4", "edATCOfsTime", "edATCQAModeOffset1",
    "edATCQAModeOffset2", "edATCQAModeOffset3", "edATCQAModeOffset4", "edATCTestTimeOffset", "edATCTestTimeOffset1", "edATCTestTimeOffset2",
    "edATCTestTimeOffset3", "edATCTestTimeOffset4", "edAbLow", "edAbMid", "edAbitColdTime", "edAbitInitWaitTime",
    "edAmbGuardband", "edAmbHotGuartbent", "edAmbTemp", "edArm1NoFullsiteOffset_1", "edArm1NoFullsiteOffset_2", "edArm1NoFullsiteOffset_3",
    "edArm1NoFullsiteOffset_4", "edArm1NoFullsiteOffset_5", "edArm1Offset", "edArm2NoFullsiteOffset_1", "edArm2NoFullsiteOffset_2", "edArm2NoFullsiteOffset_3",
    "edArm2NoFullsiteOffset_4", "edArm2NoFullsiteOffset_5", "edArm2Offset", "edAtcFileName", "edBelowSec", "edChamberCoolTemp",
    "edChillerTemp", "edContinuousSec", "edDewPointAlarmInterval", "edDewPointRange", "edFixedTemp", "edHighBase",
    "edIndexSoakTime", "edInitialStart1Time", "edInitialStart2Time", "edInitialWaitTime", "edJamSoakTime", "edLBTempAlarmTime",
    "edLBTempHighSettingValue", "edLBTempLowSettingValue", "edLowBase", "edMidBase", "edOSTime", "edOverSec",
    "edSHighBase", "edSoakTime", "edSocketAirCoolingOff", "edSocketAirCoolingOn", "edTJTempRange_High", "edTJTempRange_Low",
    "edTSDTimeOut", "edTempAlwaysSameAlarm", "edTempDownContactDelay", "edTempOffsetCount", "edTempReadyRange", "edWorkTemp",
    "edt3SigmaTempMonitior_Set3xSigmaValue", "edt3SigmaTempMonitior_SetCount", "edtATCInPC1", "edtATCInPC2", "edtATCInPC3", "edtATCInPC4",
    "edtATCPIDOffset_MaxD", "edtATCPIDOffset_MaxI", "edtATCPIDOffset_MaxP", "edtATCPIDOffset_MinD", "edtATCPIDOffset_MinI", "edtATCPIDOffset_MinP",
    "edtATCSP2", "edtATC_HotGunTemp", "edtATC_HotGunTime", "edtATC_Type", "edtBoostDuration_LB", "edtBoostDuration_Long",
    "edtBoostDuration_Short", "edtBoostOffset", "edtBoostOffset_LB", "edtBoostOffset_Long", "edtBoostOffset_Short", "edtBoostTempMin",
    "edtChamberBoostOffset", "edtChamberBoostTime", "edtDelayAfterSOT", "edtHeatGunTempATC", "edtHotGunFLowLimit_H", "edtHotGunFLowLimit_L",
    "edtIdleTime_LB", "edtIdleTime_Long", "edtIdleTime_Short", "edtInitialDelay_1", "edtInitialDelay_10", "edtInitialDelay_10_RT",
    "edtInitialDelay_1_RT", "edtInitialDelay_2", "edtInitialDelay_2_RT", "edtInitialDelay_3", "edtInitialDelay_3_RT", "edtInitialDelay_4",
    "edtInitialDelay_4_RT", "edtInitialDelay_5", "edtInitialDelay_5_RT", "edtInitialDelay_6", "edtInitialDelay_6_RT", "edtInitialDelay_7",
    "edtInitialDelay_7_RT", "edtInitialDelay_8", "edtInitialDelay_8_RT", "edtInitialDelay_9", "edtInitialDelay_9_RT", "edtInputVHigh",
    "edtInputVLow", "edtLBAirOnTemp", "edtLBTempMin", "edtLBTempOffset", "edtLBTimeOut", "edtPostBoost_LB",
    "edtPostBoost_Long", "edtPostBoost_Short", "edtPowerFollow_FullPower", "edtPowerFollower_Many2one", "edtPowerFollower_PFSlope", "edtPowerFollower_WGain",
    "edtSetTJ_Offset", "edtSetTJ_Slope", "edtSetTempature2AirMachine", "edtTestDock", "edtThreshold", "edt_AirVolumeLmt_Index",
    "edt_Defrost_Time_Too_Lower", "edt_SetAirstreamTemperatureRang_Index", "edt_SetAirstreamTemperatureRang_Socket", "edt_SetIndexAirstreamTemp", "edt_TempRiseTemp", "edt_TempRise_Delay",
    "edt_TjAvgTimes", "iTestFinishToNextTestOver", "iTeststartToNextTestStart", "rb1Point", "rb2Point", "rb3Point",
    "rb5Point", "rb6Point", "rbATC70ActiveOn", "rbATC7TSDFunction", "rbATCActiveOn", "rbHandlerStartOn",
    "rbTemperatureSetOn", "rbTestStartOn", "rgATC60AirOn", "rgBoostMode", "rgIndexHeatMode", "rgShuttleSoakTime",
    "rgTemperatureMode", "rgTjMapType", "rgZ1Down", "rg_TjMode",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void TS_CreateSaveProxies()
{
    EL<TRadioButton>("TfTemp_Set", "RachoiceATCFile");
    EL<TCheckBox>("TfTemp_Set", "cb3SigmaTempMonitior_Enable");
    EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor");
    EL<TCheckBox>("TfTemp_Set", "cbATCTempRise");
    EL<TCheckBox>("TfTemp_Set", "cbATCTestTimeOffset");
    EL<TCheckBox>("TfTemp_Set", "cbAfterAutoCleanFunctionUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbAfterOpenHeatDoorUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbAfterShowAlarmMessageUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbBlowLB");
    EL<TCheckBox>("TfTemp_Set", "cbByPassChamber");
    EL<TCheckBox>("TfTemp_Set", "cbEnableATCConsFailOffset");
    EL<TCheckBox>("TfTemp_Set", "cbEnableATCQAModeOffset");
    EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode");
    EL<TCheckBox>("TfTemp_Set", "cbEnableTJFunction");
    EL<TCheckBox>("TfTemp_Set", "cbEnableTesterDryAirControl");
    EL<TCheckBox>("TfTemp_Set", "cbEveryFirstDeviceUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbFFCOffsetEnable");
    EL<TCheckBox>("TfTemp_Set", "cbHeaterGun");
    EL<TCheckBox>("TfTemp_Set", "cbLBTempHighAlarmEnable");
    EL<TCheckBox>("TfTemp_Set", "cbLBTempLowAlarmEnable");
    EL<TCheckBox>("TfTemp_Set", "cbMultiSensorOffsetEnable");
    EL<TCheckBox>("TfTemp_Set", "cbMultiZoneFunction");
    EL<TCheckBox>("TfTemp_Set", "cbOTDUnlockDelay");
    EL<TCheckBox>("TfTemp_Set", "cbPowerFollow_Enable");
    EL<TCheckBox>("TfTemp_Set", "cbShowFixTemp");
    EL<TCheckBox>("TfTemp_Set", "cbTempOffset");
    EL<TCheckBox>("TfTemp_Set", "cbTestFinishToNextTestOver");
    EL<TCheckBox>("TfTemp_Set", "cbTestStartToNextTestStart");
    EL<TCheckBox>("TfTemp_Set", "cbTjWatchdog");
    EL<TCheckBox>("TfTemp_Set", "cbUseAbitCHK");
    EL<TCheckBox>("TfTemp_Set", "cbUseAmbGuard");
    EL<TCheckBox>("TfTemp_Set", "cbUseFixTemp");
    EL<TCheckBox>("TfTemp_Set", "cbUseOtherArmToTestAfterInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset");
    EL<TCheckBox>("TfTemp_Set", "cbWaitDewPoint");
    EL<TCheckBox>("TfTemp_Set", "cbWhenHappenTestedTimeBelowUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbWhenNoFullSiteUseInitialDelay");
    EL<TCheckBox>("TfTemp_Set", "cbWhenPressStopOverUseInitialDelay");
    EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile");
    EL<TCheckBox>("TfTemp_Set", "cdATCUsePIDCtrl");
    EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan");
    EL<TCheckBox>("TfTemp_Set", "chkBoostFunction");
    EL<TCheckBox>("TfTemp_Set", "chkBoostOffset");
    EL<TCheckBox>("TfTemp_Set", "chkChamberBoost");
    EL<TCheckBox>("TfTemp_Set", "chkLBTempFunction");
    EL<TCheckBox>("TfTemp_Set", "chkSLKNoHeatUp");
    EL<TCheckBox>("TfTemp_Set", "chkShuttleNoHeatUp");
    EL<TCheckBox>("TfTemp_Set", "chkSoakTimeReduce");
    EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe");
    EL<TCheckBox>("TfTemp_Set", "chkTempErrorAction");
    EL<TCheckBox>("TfTemp_Set", "chkTesterDock");
    EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableBase");
    EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableIndex");
    EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableOutSht");
    EL<TCheckBox>("TfTemp_Set", "chkUseCDAOnly");
    EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_1_Air");
    EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_2_Air");
    EL<TCheckBox>("TfTemp_Set", "chk_EnableSocket_Air");
    EL<TCheckBox>("TfTemp_Set", "chk_IndexHeaterDoor");
    EL<TCheckBox>("TfTemp_Set", "chk_UseTj");
    EL<TEdit>("TfTemp_Set", "edATCAmbTemp");
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset1");
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset2");
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset3");
    EL<TEdit>("TfTemp_Set", "edATCContFailOffset4");
    EL<TEdit>("TfTemp_Set", "edATCContFailOffsetCnt");
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset1");
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset2");
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset3");
    EL<TEdit>("TfTemp_Set", "edATCInitialOffset4");
    EL<TEdit>("TfTemp_Set", "edATCOfsTime");
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset1");
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset2");
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset3");
    EL<TEdit>("TfTemp_Set", "edATCQAModeOffset4");
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset");
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset1");
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset2");
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset3");
    EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset4");
    EL<TEdit>("TfTemp_Set", "edAbLow");
    EL<TEdit>("TfTemp_Set", "edAbMid");
    EL<TEdit>("TfTemp_Set", "edAbitColdTime");
    EL<TEdit>("TfTemp_Set", "edAbitInitWaitTime");
    EL<TEdit>("TfTemp_Set", "edAmbGuardband");
    EL<TEdit>("TfTemp_Set", "edAmbHotGuartbent");
    EL<TEdit>("TfTemp_Set", "edAmbTemp");
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_1");
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_2");
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_3");
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_4");
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_5");
    EL<TEdit>("TfTemp_Set", "edArm1Offset");
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_1");
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_2");
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_3");
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_4");
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_5");
    EL<TEdit>("TfTemp_Set", "edArm2Offset");
    EL<TEdit>("TfTemp_Set", "edAtcFileName");
    EL<TEdit>("TfTemp_Set", "edBelowSec");
    EL<TEdit>("TfTemp_Set", "edChamberCoolTemp");
    EL<TEdit>("TfTemp_Set", "edChillerTemp");
    EL<TEdit>("TfTemp_Set", "edContinuousSec");
    EL<TEdit>("TfTemp_Set", "edDewPointAlarmInterval");
    EL<TEdit>("TfTemp_Set", "edDewPointRange");
    EL<TEdit>("TfTemp_Set", "edFixedTemp");
    EL<TEdit>("TfTemp_Set", "edHighBase");
    EL<TEdit>("TfTemp_Set", "edIndexSoakTime");
    EL<TEdit>("TfTemp_Set", "edInitialStart1Time");
    EL<TEdit>("TfTemp_Set", "edInitialStart2Time");
    EL<TEdit>("TfTemp_Set", "edInitialWaitTime");
    EL<TEdit>("TfTemp_Set", "edJamSoakTime");
    EL<TEdit>("TfTemp_Set", "edLBTempAlarmTime");
    EL<TEdit>("TfTemp_Set", "edLBTempHighSettingValue");
    EL<TEdit>("TfTemp_Set", "edLBTempLowSettingValue");
    EL<TEdit>("TfTemp_Set", "edLowBase");
    EL<TEdit>("TfTemp_Set", "edMidBase");
    EL<TEdit>("TfTemp_Set", "edOSTime");
    EL<TEdit>("TfTemp_Set", "edOverSec");
    EL<TEdit>("TfTemp_Set", "edSHighBase");
    EL<TEdit>("TfTemp_Set", "edSoakTime");
    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOff");
    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOn");
    EL<TEdit>("TfTemp_Set", "edTJTempRange_High");
    EL<TEdit>("TfTemp_Set", "edTJTempRange_Low");
    EL<TEdit>("TfTemp_Set", "edTSDTimeOut");
    EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm");
    EL<TEdit>("TfTemp_Set", "edTempDownContactDelay");
    EL<TEdit>("TfTemp_Set", "edTempOffsetCount");
    EL<TEdit>("TfTemp_Set", "edTempReadyRange");
    EL<TEdit>("TfTemp_Set", "edWorkTemp");
    EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue");
    EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_SetCount");
    EL<TEdit>("TfTemp_Set", "edtATCInPC1");
    EL<TEdit>("TfTemp_Set", "edtATCInPC2");
    EL<TEdit>("TfTemp_Set", "edtATCInPC3");
    EL<TEdit>("TfTemp_Set", "edtATCInPC4");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxD");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxI");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxP");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinD");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinI");
    EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinP");
    EL<TEdit>("TfTemp_Set", "edtATCSP2");
    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTemp");
    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTime");
    EL<TEdit>("TfTemp_Set", "edtATC_Type");
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_LB");
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Long");
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Short");
    EL<TEdit>("TfTemp_Set", "edtBoostOffset");
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_LB");
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Long");
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Short");
    EL<TEdit>("TfTemp_Set", "edtBoostTempMin");
    EL<TEdit>("TfTemp_Set", "edtChamberBoostOffset");
    EL<TEdit>("TfTemp_Set", "edtChamberBoostTime");
    EL<TEdit>("TfTemp_Set", "edtDelayAfterSOT");
    EL<TEdit>("TfTemp_Set", "edtHeatGunTempATC");
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_H");
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_L");
    EL<TEdit>("TfTemp_Set", "edtIdleTime_LB");
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Long");
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Short");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8_RT");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9");
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9_RT");
    EL<TEdit>("TfTemp_Set", "edtInputVHigh");
    EL<TEdit>("TfTemp_Set", "edtInputVLow");
    EL<TEdit>("TfTemp_Set", "edtLBAirOnTemp");
    EL<TEdit>("TfTemp_Set", "edtLBTempMin");
    EL<TEdit>("TfTemp_Set", "edtLBTempOffset");
    EL<TEdit>("TfTemp_Set", "edtLBTimeOut");
    EL<TEdit>("TfTemp_Set", "edtPostBoost_LB");
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Long");
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Short");
    EL<TEdit>("TfTemp_Set", "edtPowerFollow_FullPower");
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_Many2one");
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_PFSlope");
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_WGain");
    EL<TEdit>("TfTemp_Set", "edtSetTJ_Offset");
    EL<TEdit>("TfTemp_Set", "edtSetTJ_Slope");
    EL<TEdit>("TfTemp_Set", "edtSetTempature2AirMachine");
    EL<TEdit>("TfTemp_Set", "edtTestDock");
    EL<TEdit>("TfTemp_Set", "edtThreshold");
    EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Index");
    EL<TEdit>("TfTemp_Set", "edt_Defrost_Time_Too_Lower");
    EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index");
    EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket");
    EL<TEdit>("TfTemp_Set", "edt_SetIndexAirstreamTemp");
    EL<TEdit>("TfTemp_Set", "edt_TempRiseTemp");
    EL<TEdit>("TfTemp_Set", "edt_TempRise_Delay");
    EL<TEdit>("TfTemp_Set", "edt_TjAvgTimes");
    EL<TEdit>("TfTemp_Set", "iTestFinishToNextTestOver");
    EL<TEdit>("TfTemp_Set", "iTeststartToNextTestStart");
    EL<TRadioButton>("TfTemp_Set", "rb1Point");
    EL<TRadioButton>("TfTemp_Set", "rb2Point");
    EL<TRadioButton>("TfTemp_Set", "rb3Point");
    EL<TRadioButton>("TfTemp_Set", "rb5Point");
    EL<TRadioButton>("TfTemp_Set", "rb6Point");
    EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn");
    EL<TCheckBox>("TfTemp_Set", "rbATC7TSDFunction");
    EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn");
    EL<TCheckBox>("TfTemp_Set", "rbHandlerStartOn");
    EL<TCheckBox>("TfTemp_Set", "rbTemperatureSetOn");
    EL<TCheckBox>("TfTemp_Set", "rbTestStartOn");
    EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn");
    EL<TRadioGroup>("TfTemp_Set", "rgBoostMode");
    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode");
    EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime");
    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode");
    EL<TRadioGroup>("TfTemp_Set", "rgTjMapType");
    EL<TRadioGroup>("TfTemp_Set", "rgZ1Down");
    EL<TRadioGroup>("TfTemp_Set", "rg_TjMode");
}

// golden uTemp_Set.cpp:433  TfTemp_Set::FormShow(TObject *Sender)
static void TS_FormShow()
{

    AnsiString S;

    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")       ->Enabled=true;                                     //Steven 20120804 Start: 先在最上面Enable全部畫面
    EL<TGroupBox>("TfTemp_Set", "gbBasePoint")             ->Enabled=true;
    EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")        ->Enabled=true;
    EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")    ->Enabled=true;
    EL<TTabSheet>("TfTemp_Set", "tsOther")                 ->Enabled=true;                                     //kevin 20150127
    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")         ->Enabled=true;
    //AI(mg899to910) 20260831: uTemp_Set.dfm 下方 base 溫度列改成與 offset 表格同欄距(Left=grid+5, Width=50)，因 2024.07.27 加第6欄時只改了表格未改 base 列，造成選 5/6 Point 時欄位錯位（CASE-GIGAS-20260831-001）
    EL<TEdit>("TfTemp_Set", "edLowBase")               ->Enabled=true;
    EL<TEdit>("TfTemp_Set", "edMidBase")               ->Enabled=true;
    EL<TEdit>("TfTemp_Set", "edHighBase")              ->Enabled=true;
    EL<TEdit>("TfTemp_Set", "edAbLow")                 ->Enabled=true;                                     //kevin 20140918
    EL<TEdit>("TfTemp_Set", "edAbMid")                 ->Enabled=true;                                     //kevin 20140918
    EL<TButton>("TfTemp_Set", "btClearAll")              ->Enabled=true;
    EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")           ->Enabled=fSecurity->Insufficient(158, false);      //JerryYang 20181214 : 新增ATC權限管控
    EL<TEdit>("TfTemp_Set", "edSoakTime")              ->Enabled=true;                                     //JerryYang 20220311 : ATP鎖定Critical parameter
    EL<TEdit>("TfTemp_Set", "edWorkTemp")              ->Enabled=true;
    EL<TEdit>("TfTemp_Set", "edSHighBase")             ->Enabled=true;                                     //Ztex 2024.07.27 Add 6 Point Temperature Offset

    bNeedChange=true;
    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08 )                                       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {
        switch(TestIF.iTestMode)
        {
            case SingleSite:
                if(ATC_SYSTEM>=eATC60 &&
                   Temperature.bATCActiveCooling==true)                         //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                {
                }
                else
                {
                    if(TestIF_File.b2CableLayoutKit)                            //JerryYang 20160826 single site兩條線版本(使用一支加熱棒)
                    {
                    }
                    else
                    {
                        bNeedChange=false;
                    }
                }
                break;
            case DualSite:                                                      //1x2
                if((ATC_SYSTEM==eNewATCSystem ||                                //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {
                    bNeedChange=false;
                }
                else if(TestIF_File.b2CableLayoutKit)                           //JerryYang 20160826 1x2 兩條線版本使用兩支加熱棒
                {
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //jou 2015-10-15  : 16溫控器 1條線版本
                {
                    bNeedChange=false;
                }
                else
                {
                    bNeedChange=false;
                }
                break;
            case TriSite1X3:
                break;
            case QualSite1X4:                                                   //1x4
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 修正 ATC 1*4 溫度顯示異常問題
                   Temperature.bATCActiveCooling==true)
                {
                }
                else if(TestIF_File.b2CableLayoutKit)                           //Steven 20150724 : 16溫控器 2條線版本
                {
                }
                else if(TestIF_File.bNS7000kit==true ||
                        TestIF_File.b1CableLayoutKit==true)
                {
                    bNeedChange=false;
                }
                else                                                            //Steven 20250821 : 1x4 三條線版本
                {
                    bNeedChange=false;
                }
                break;
            case QualSite2X2:                                                   //2x2
                if(TestIF_File.bSquare_OctalKit ||                              //Steven 20141224 : 2x2Site使用8Site Kit
                   TestIF_File.b2x2Use16SiteKit)                                //Steven 20191113 : 2x2Site使用16Site Kit
                {
                }
                else if(CUSTOMER_CODE==CC_ATEC &&
                        TestIF_File.b2CableLayoutKit &&
                        DeviceForm_File.iHeadDeviceCT==3)                       //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //JerryYang 20181207 2x2新增一條線版本
                {
                    bNeedChange=false;
                }
                else
                {
                    bNeedChange=false;
                }
                break;
        }
    }

    for(int i=tcHotPlate1; i<tcTotalCount; i++)                                 //Ifor 20191024 : Fix 權限切換後無法編輯Temp Offset問題
    {
        myTempPal[i]->edLow->Enabled             =true;
        myTempPal[i]->edMid->Enabled             =true;
        myTempPal[i]->edLowbase->Enabled         =true;
        myTempPal[i]->edBase->Enabled            =true;
        myTempPal[i]->edHighBase->Enabled        =true;
        myTempPal[i]->edSHighBase->Enabled       =true;                         //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CUSTOMER_CODE!=CC_KYEC_LEE)
        {
            #ifndef HiSilicon
            if(IniConfig.bVTESTFunction==true)                                  //jou 20231101 : 客戶要求 temp頁面 Single Limit 不使用
                myTempPal[i]->edSingleLimit->Enabled =false;
            else
                myTempPal[i]->edSingleLimit->Enabled =true;
            myTempPal[i]->edInitTempOffset->Enabled  =true;
            myTempPal[i]->edEOTTempOffset->Enabled   =true;
            #else
            myTempPal[i]->edSingleLimit->Enabled     =false;
            myTempPal[i]->edInitTempOffset->Enabled  =false;
            myTempPal[i]->edEOTTempOffset->Enabled   =false;
            EL<TEdit>("TfTemp_Set", "edWorkTemp")->Enabled   = false;                                      //kevin 20200102 add disable

            myTempPal[i]->edLowbase->Enabled         =false;                    //kevin 20200508 add HISI
            myTempPal[i]->edBase->Enabled            =false;
            myTempPal[i]->edHighBase->Enabled        =false;
            myTempPal[i]->edSHighBase->Enabled       =false;                    //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edLow->Enabled            =false;                     //kevin 20200508 add HISI
            myTempPal[i]->edMid->Enabled            =false;
            #endif
            myTempPal[i]->edOffset->Enabled          =true;                     //kevin 20200514 溫度offset 可以輸入
        }
        else                                                                    //Ifor 20200305 : fix OP權限開啟後Offset鎖死
        {
            myTempPal[i]->edOffset->Enabled             =true;
            myTempPal[i]->edSingleLimit->Enabled        =true;
            myTempPal[i]->edInitTempOffset->Enabled     =true;
            myTempPal[i]->edEOTTempOffset->Enabled      =true;
        }
    }

    if(ATC_SYSTEM==eNewATCSystem &&
       (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 ||       //JerryYang 20250729 : ATC 3.3+6.0
        ATC_MixMode==eMixATC60_ATC33))                          //Steven 20181102 : For QTI全球統一Offset
    {
        EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode")->Visible=false;
        EL<TTabSheet>("TfTemp_Set", "tsBoostFunc")->TabVisible=true;
        EL<TTabSheet>("TfTemp_Set", "tsLBCtr")->TabVisible=true;
        EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->Visible=true;                                             //Steven 20191025 : for ATC6.0
    }
    else
    {
        EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode")->Visible=CosFunction.bUseIndividulTempSet;       //Steven 20140924 : 各個加熱區獨立有自己的設定值
        EL<TTabSheet>("TfTemp_Set", "tsBoostFunc")->TabVisible=false;
        EL<TTabSheet>("TfTemp_Set", "tsLBCtr")->TabVisible=false;
        if(LB_TEMP==true && CosFunction.bNonATCSupportLBFunction==true)         //JerryYang 20220126: non ATC也支援LB function
        {
            EL<TTabSheet>("TfTemp_Set", "tsLBCtr")->TabVisible=true;
        }
        else
        {
            EL<TTabSheet>("TfTemp_Set", "tsLBCtr")->TabVisible=false;
        }
        EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->Visible=false;                                            //Steven 20191025 : for ATC6.0
    }

    EL<TTabSheet>("TfTemp_Set", "tsOthers")                ->TabVisible=LB_TEMP_UpDown;                        //Frank 20241231 : add
    EL<TMemo>("TfTemp_Set", "palIndiTemp")             ->Visible=(CosFunction.bUseIndividulTempSet || Tri_Temp_Machine==1);                        //Steven 20140924 : 各個加熱區獨立有自己的設定值  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TCheckBox>("TfTemp_Set", "chkSoakTimeReduce")       ->Visible=CosFunction.bUseInitialDelayAsSoakTime;   //Steven 20170329 (wei) : 使用initial delay當 Soak time
    EL<TCheckBox>("TfTemp_Set", "chkTempErrorAction")      ->Visible=CosFunction.bTempAlarmBinNeedToError;     //Steven 20251022 : Temp alarm need put to error bin
    EL<TCheckBox>("TfTemp_Set", "chkUseCDAOnly")           ->Visible=(INSTALL_HEAT_GUN>1);                     //Steven 20181012 : 使用熱風槍吹冷風
    EL<TPanel>("TfTemp_Set", "palLBAir")                ->Visible=(CosFunction.bNonATCSupportLBFunction);   //JerryYang 20220927 : add
    EL<TTabSheet>("TfTemp_Set", "tsUnUse")                 ->TabVisible=false;
    EL<TTabSheet>("TfTemp_Set", "tsTriTempSet")            ->TabVisible=(Tri_Temp_Machine==1 || (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));                //Ztex 2023.04.19 Add HT-1032 TriTemp Function

//    for(i=0; i<TempEditTotal; i++)
//    {
//        for(j=tcHotPlate1; j<tcTotalCount; j++)
//        {
//            TemperEdit[i][j]->Enabled=true;
//        }
//    }
    //Steven 20120804 End: 先在最上面Enable全部畫面

    EL<TRadioButton>("TfTemp_Set", "rbTemp")->SetFocus();
    GetLimitAuth();
//    Image1->Picture->LoadFromFile(BmpPath+"Tmode1.bmp");
    EL<TPanel>("TfTemp_Set", "pnlUseOtherArmToTestAfterInitialDelay")->Visible=CosFunction.bAfterInitialDelayUseOtherArm;                           //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail

    if(IniConfig.bI03AmbientTempControl)                                        //kevin 20140918
    {
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Clear();
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Hot");
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Ambient");
        if(IniConfig.bOnlyRoomOrHot)
        {
            EL<TPanel>("TfTemp_Set", "pnlCover")->Visible=true;                                             //Steven 20210816 : 隱藏Temp control的部分
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("");
        }
        else
        {
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Ambient/High/Temp control");
        }
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Temp control");
    }
    else if(IniConfig.bOnlyRoomOrHot)                                           //Steven 20110914
    {
        EL<TPanel>("TfTemp_Set", "pnlCover")->Visible=true;                                                 //Steven 20210816 : 隱藏Temp control的部分
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Clear();
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Hot");
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Items->Add("Ambient");
    }

    TS_ReadTempFile(true);

    S.sprintf("Temperature Parameter ''%s''  ", GetLastOpenFN());
    ;   // golden uTemp_Set.cpp:664-664 取代：Caption：視窗標題
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:664-664 -- Caption：視窗標題
    Caption=S;
#endif // GATE (S12-C save)
    TS_UpDateEdit();

    switch(Temperature.iTempMode)
    {
        case 1:
        case 2:
        case 4:
        case 8:                                                                 //kevin 20141006
    ;   // golden uTemp_Set.cpp:673-673 取代：SetBasePointIMG：基準點示意圖（TImage，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:673-673 -- SetBasePointIMG：基準點示意圖（TImage，純畫面）
            SetBasePointIMG(Temperature.iTempMode);
#endif // GATE (S12-C save)
            break;
        case 16:                                                                //Ztex 2024.07.27 Add 6 Point Temperature Offset
    ;   // golden uTemp_Set.cpp:676-676 取代：SetBasePointIMG：基準點示意圖（TImage，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:676-676 -- SetBasePointIMG：基準點示意圖（TImage，純畫面）
            SetBasePointIMG(Temperature.iTempMode);
#endif // GATE (S12-C save)
            break;
        default :
            EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked=true;
            Temperature.iTempMode=1;
    ;   // golden uTemp_Set.cpp:681-681 取代：SetBasePointIMG：基準點示意圖（TImage，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:681-681 -- SetBasePointIMG：基準點示意圖（TImage，純畫面）
            SetBasePointIMG(Temperature.iTempMode);
#endif // GATE (S12-C save)
    }

    //jou 2012-03-12 假如ATC模式只支援Head mode & Head + Socket mode
    if(ATC_SYSTEM<=eATC30)                                                      //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    {
        if(IniConfig.bNoHeadaddChamberOption)
        {
            EL<TButton>("TfTemp_Set", "btClearAll")->Visible=false;
//            if(rgIndexHeatMode->Items->Count==3)                              //Steven 20240201 : 重新整理Index Heat Mode
//                rgIndexHeatMode->Items->Delete(2);
        }
    }
    else
    {
//        if(rgIndexHeatMode->Items->Count==3)
//            rgIndexHeatMode->Items->Delete(2);
//        rgIndexHeatMode->Items->Strings[1]="Head + Socket";
        EL<TLabel>("TfTemp_Set", "Label44")->Visible=false;
        EL<TLabel>("TfTemp_Set", "Label45")->Visible=false;
        EL<TEdit>("TfTemp_Set", "edChamberCoolTemp")->Visible=false;
    }
                                                                                //Steven 20240201 : 重新整理Index Heat Mode
    EL<TGroupBox>("TfTemp_Set", "gbSocketAirCooling")->Visible=(IniConfig.bL03SocketAirCoolingCT);             //Steven 20160714

    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Enabled=(LastSet.iTemperature==Tempture_Hot)?false:true;   //Steven 20091102
    ;   // golden uTemp_Set.cpp:707-707 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:707-707 -- Left／Top：視窗位置
    Left=10;                                                                    //Steven 20091103
#endif // GATE (S12-C save)
    ;   // golden uTemp_Set.cpp:708-708 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:708-708 -- Left／Top：視窗位置
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    if(HasICUnderMachine()==false)                                              //jou 2010-01-11 start : 機台上有IC時，不能改變加熱狀態
    {
        #ifdef HiSilicon                                                        //kevin 20200508 add HISI no change
            EL<TButton>("TfTemp_Set", "btClearAll")->Visible         =false;
            EL<TEdit>("TfTemp_Set", "edAbLow")->Enabled            =false;
            EL<TEdit>("TfTemp_Set", "edAbMid")->Enabled            =false;
            EL<TEdit>("TfTemp_Set", "edLowBase")->Enabled          =false;
            EL<TEdit>("TfTemp_Set", "edMidBase")->Enabled          =false;
            EL<TEdit>("TfTemp_Set", "edHighBase")->Enabled         =false;
            EL<TGroupBox>("TfTemp_Set", "gbBasePoint")->Enabled        =false;
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled  =false;
            EL<TEdit>("TfTemp_Set", "edSHighBase")->Enabled        =false;                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset

        #else
            EL<TButton>("TfTemp_Set", "btClearAll")->Visible         =fSecurity->Insufficient(17, false);
            EL<TEdit>("TfTemp_Set", "edAbLow")->Enabled            =fSecurity->Insufficient(17, false);    //kevin 20140918 add
            EL<TEdit>("TfTemp_Set", "edAbMid")->Enabled            =fSecurity->Insufficient(17, false);    //kevin 20140918 add
            EL<TEdit>("TfTemp_Set", "edLowBase")->Enabled          =fSecurity->Insufficient(17, false);
            EL<TEdit>("TfTemp_Set", "edMidBase")->Enabled          =fSecurity->Insufficient(17, false);
            EL<TEdit>("TfTemp_Set", "edHighBase")->Enabled         =fSecurity->Insufficient(17, false);
            EL<TGroupBox>("TfTemp_Set", "gbBasePoint")->Enabled        =fSecurity->Insufficient(17, false);
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled  =fSecurity->Insufficient(17, false);
            EL<TEdit>("TfTemp_Set", "edSHighBase")->Enabled        =fSecurity->Insufficient(17, false);    //Ztex 2024.07.27 Add 6 Point Temperature Offset

            if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)      //JerryYang 20220311 : ATP鎖定Critical parameter
            {
                if(bAuthCriticalPara[13])
                {
                    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled=false;
                }
            }
        #endif

        if(IniConfig.bKoreaFunction==false)
        {
            for(int i=0; i<tcTotalCount; i++)
            {
                myTempPal[i]->edLow->Enabled        =fSecurity->Insufficient(17, false);                                //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled        =fSecurity->Insufficient(17, false);
                myTempPal[i]->edLowbase->Enabled    =fSecurity->Insufficient(17, false);
                myTempPal[i]->edBase->Enabled       =fSecurity->Insufficient(17, false);
                myTempPal[i]->edHighBase->Enabled   =fSecurity->Insufficient(17, false);
                myTempPal[i]->edSHighBase->Enabled  =fSecurity->Insufficient(17, false);                                //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }

        EL<TLabel>("TfTemp_Set", "lblNotice")->Visible=false;
    }
    else
    {
        EL<TButton>("TfTemp_Set", "btClearAll")->Visible =false;
        EL<TEdit>("TfTemp_Set", "edAbLow")->Enabled    =false;                                             //kevin 20140918
        EL<TEdit>("TfTemp_Set", "edAbMid")->Enabled    =false;                                             //kevin 20140918
        EL<TEdit>("TfTemp_Set", "edLowBase")->Enabled  =false;
        EL<TEdit>("TfTemp_Set", "edMidBase")->Enabled  =false;
        EL<TEdit>("TfTemp_Set", "edHighBase")->Enabled =false;
        EL<TGroupBox>("TfTemp_Set", "gbBasePoint")->Enabled=false;
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled=false;
        EL<TEdit>("TfTemp_Set", "edSHighBase")->Enabled=false;                                             //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(IniConfig.bKoreaFunction==true && ATC_SYSTEM!=eATC60 &&
           (Temperature.bBoostFuncttion || Temperature.bLBTempFunction))
        {
        }
        else if(IniConfig.bKoreaFunction==false)
        {
            for(int i=0; i<tcTotalCount; i++)
            {
                myTempPal[i]->edLow->Enabled=false;                             //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=false;
                myTempPal[i]->edLowbase->Enabled=false;
                myTempPal[i]->edBase->Enabled=false;
                myTempPal[i]->edHighBase->Enabled=false;
                myTempPal[i]->edSHighBase->Enabled=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }

        EL<TLabel>("TfTemp_Set", "lblNotice")->Visible=true;
    }
    //jou 2010-01-11 end

    //jou 2011-12-21
    if(IniConfig.bKoreaFunction==true ||
       CUSTOMER_CODE==CC_UTAC_TW)                                               //Sam 20240416 : 聯測東廷要求開放
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            if(AccessLevel>iDefSupervisorLevel)                                 //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
            {
                myTempPal[i]->edLow->Enabled=true;                              //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=true;
                myTempPal[i]->edLowbase->Enabled=true;
                myTempPal[i]->edBase->Enabled=true;
                myTempPal[i]->edHighBase->Enabled=true;
                myTempPal[i]->edSHighBase->Enabled=true;                        //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
            else
            {
                myTempPal[i]->edLow->Enabled=false;                             //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=false;
                myTempPal[i]->edLowbase->Enabled=false;
                myTempPal[i]->edBase->Enabled=false;
                myTempPal[i]->edHighBase->Enabled=false;
                myTempPal[i]->edSHighBase->Enabled=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }
    }

    EL<TPanel>("TfTemp_Set", "PnlDewPointAlarm")->Visible=false;                                            //(DewPoint_Hardware_Install>0);            //Steven 20191017 : 露點計

    if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)                          //Steven 20101221 //ChungHung 20130621 add SCK RMS
    {
        if(IniConfig.bEnableRms)                                                //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
//            if(AccessLevel<=iDefEngineerLevel)                                //Steven 20210730 : Mark
//            {
                EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")    ->Enabled=false;
                EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")->Enabled=false;
                EL<TGroupBox>("TfTemp_Set", "gbBasePoint")         ->Enabled=false;
                EL<TGroupBox>("TfTemp_Set", "gbTempOffset")        ->Enabled=(CUSTOMER_CODE==CC_SCK);          //JimmyChiu 20211008 : Add for JSCK
                EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")   ->Enabled=false;
                EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")     ->Enabled=false;
//            }
            EL<TPanel>("TfTemp_Set", "pnlOffsetBase")       ->Enabled=false;                                //Steven 20210616 : JSCK RMS enabled, can not changed "Site map, temperature & Bin map"
        }
        else
        {
            EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")    ->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbBasePoint")         ->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbTempOffset")        ->Enabled=true;
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")   ->Enabled=true;
            EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")     ->Enabled=true;
            EL<TPanel>("TfTemp_Set", "pnlOffsetBase")       ->Enabled=true;                                 //Steven 20210616 : JSCK RMS enabled, can not changed "Site map, temperature & Bin map"
        }
    }

    EL<TLabel>("TfTemp_Set", "lblTempBase") ->Visible=true;
    EL<TButton>("TfTemp_Set", "btClearAll")  ->Visible=true;
    EL<TEdit>("TfTemp_Set", "edAbLow")     ->Visible=true;                                                 //kevin 20140918
    EL<TEdit>("TfTemp_Set", "edAbMid")     ->Visible=true;                                                 //kevin 20140918
    EL<TEdit>("TfTemp_Set", "edLowBase")   ->Visible=true;
    EL<TEdit>("TfTemp_Set", "edMidBase")   ->Visible=true;
    EL<TEdit>("TfTemp_Set", "edHighBase")  ->Visible=true;
    EL<TEdit>("TfTemp_Set", "edWorkTemp")  ->Visible=true;
    EL<TEdit>("TfTemp_Set", "edSHighBase") ->Visible=true;                                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset

    //jou 20220627 : JSCS 需要開啟 Base-point 5
//    if(CUSTOMER_CODE==CC_SCS)                                                 //jou 2015-08-24 SCS remove Base-point 5
//    {
//        rb5Point->Visible=false;
//        edAbLow->Visible=false;
//        edAbMid->Visible=false;
//    }

    EL<TGroupBox>("TfTemp_Set", "grpChamberBoost")->Visible=CosFunction.bUseChamberBoostMode;

    if(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)      //Steven 20140924 : 各個加熱區獨立有自己的設定值
        EL<TEdit>("TfTemp_Set", "edWorkTemp")->Visible=false;

    EL<TLabel>("TfTemp_Set", "lblShtSoakTime")      ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    EL<TEdit>("TfTemp_Set", "edInitialStart2Time") ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    EL<TLabel>("TfTemp_Set", "lbllblShtSoakTimeSec")->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")   ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time

    EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")     ->Visible   =IniConfig.bAmbRunChamberFanCanStop;        //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    EL<TGroupBox>("TfTemp_Set", "grpRS232")            ->Visible   =(ATC_SYSTEM==eATCSiliconType);
    EL<TTabSheet>("TfTemp_Set", "tsATC")               ->TabVisible=(ATC_SYSTEM>=eATC30 &&                     //20141204 ChungHung add for ATC3.0
                                      ATC_SYSTEM!=eNonChamber);                 //2014-05-30    Dell    for ATC6.0
//                                      Tri_Temp_Machine==1);                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    if(CUSTOMER_CODE==CC_SCS)                                                   //jou 2015-12-10 SCS 要求把ATC PID頁面關掉不顯示
    {
        EL<TTabSheet>("TfTemp_Set", "tsATC_PID")->TabVisible=false;
        EL<TTabSheet>("TfTemp_Set", "tsATC70")->TabVisible=false;                                              //Ifor 20160711 SCS 要求把ATC 7.0頁面關掉不顯示
    }
    else
    {
        EL<TTabSheet>("TfTemp_Set", "tsATC_PID")           ->TabVisible=(ATC_SYSTEM==eATCHonPrecType ||        //wei 20151019 ATC PID顯示
                                         (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_20) ||
                                         (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61) ||
                                         (Tri_Temp_Machine==1));                //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        EL<TTabSheet>("TfTemp_Set", "tsATC70")->TabVisible =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70 //Ifor 20160412 add ATC7.0 加入NewATCIF
    }

    if(ATC_SYSTEM==eNewATCSystem && CosFunction.bUseFFCTempOffset==true)
    {
        EL<TTabSheet>("TfTemp_Set", "tsATC_FFC")->TabVisible=true;
    }
    else
    {
        EL<TTabSheet>("TfTemp_Set", "tsATC_FFC")->TabVisible=false;
    }

    EL<TTabSheet>("TfTemp_Set", "tsATC")               ->Enabled   =(AccessLevel>=LevelSet.AccessLevel[44]);   //Steven 20130122 : 加入權限
    EL<TGroupBox>("TfTemp_Set", "gbATCAmbientTemp")    ->Visible   =(ATC_SYSTEM>=eATC30 &&
                                      ATC_SYSTEM!=eNonChamber);                 //wei 20151013  by Setup File ATC Ambient Temp set

    EL<TGroupBox>("TfTemp_Set", "grpTemperatureCheck") ->Visible   =(ATC_SYSTEM==eATCSiliconType);             //Steven 20120611
    EL<TGroupBox>("TfTemp_Set", "grbATCTempAlarm")     ->Visible   =(ATC_SYSTEM==eATCSiliconType);             //Steven 20120611
    EL<TGroupBox>("TfTemp_Set", "grpATC_HotAir")       ->Visible   =(ATC_SYSTEM==eNewATCSystem &&
    ((ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36) ||   // golden uTemp_Set.cpp:911-911 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:911-911 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
                                      (ATC_InterfaceForm->IS_ATC33() ||
#endif // GATE (S12-C save)
                                       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));
    EL<TTabSheet>("TfTemp_Set", "tsHeatGun")           ->TabVisible=(INSTALL_HEAT_GUN ||                       //Steven 20120525 : 熱風槍
                                      ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61 ||
                                      INSTALL_ATC_HEAT_GUN);                    //Ifor 20250926 add:ATC_HEAT_GUN 顯示
    EL<TTabSheet>("TfTemp_Set", "tsArm1")              ->TabVisible=(USE_16_HEATER==eht16Heater    ||
                                      USE_16_HEATER==eht16HeaterEJ1N   ||
                                      USE_16_HEATER==eht32HeaterEJ1N   ||       //Steven 20140923 : Index使用EJ1N版32組加熱器
                                      USE_16_HEATER==eht32HeaterKT4H   ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      USE_16_HEATER==eht16HeaterDTME08 ||       //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                                      USE_16_HEATER==eht32HeaterDTME08);        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

    EL<TTabSheet>("TfTemp_Set", "tsArm2")              ->TabVisible=(USE_16_HEATER==eht16Heater   ||
                                      USE_16_HEATER==eht16HeaterEJ1N  ||
                                      USE_16_HEATER==eht32HeaterEJ1N  ||        //Steven 20140923 : Index使用EJ1N版32組加熱器
                                      USE_16_HEATER==eht32HeaterKT4H  ||        //Steven 20150211 : Index使用KT4H版32組加熱器
                                      USE_16_HEATER==eht16HeaterDTME08||        //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                                      USE_16_HEATER==eht32HeaterDTME08);        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

//    tsArm1_2            ->TabVisible=(USE_16_HEATER==eht32HeaterEJ1N ||       //Steven 20140923 : Index使用EJ1N版32組加熱器
//                                      USE_16_HEATER==eht32HeaterKT4H);        //Steven 20150211 : Index使用KT4H版32組加熱器
//    tsArm2_2            ->TabVisible=(USE_16_HEATER==eht32HeaterEJ1N ||       //Steven 20140923 : Index使用EJ1N版32組加熱器
//                                      USE_16_HEATER==eht32HeaterKT4H);        //Steven 20150211 : Index使用KT4H版32組加熱器

    EL<TTabSheet>("TfTemp_Set", "tsDUT")               ->TabVisible=(iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea);
//    labSOC              ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示
//    edLSOC              ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示
//    edMSOC              ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示
//    edHSOC              ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示
//    edOSOC              ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示
//    edLimitSOC          ->Visible   =(iSocketBaseTempCount==eDut1ea);         //Steven 20140624 : 溫度Offset修正顯示

    EL<TTabSheet>("TfTemp_Set", "tsAmbCtr")->TabVisible=IniConfig.bI03AmbientTempControl;                      //jou 2015-12-10 Ambient_control Guard Band 顯示依照 bI03AmbientTempControl Function打開關閉
    EL<TGroupBox>("TfTemp_Set", "rgAmbientBrand")->Visible=IniConfig.bL20AbientGuardBand;                      //kevin 20180115 (Steven) add Amient Guard Band
    EL<TTabSheet>("TfTemp_Set", "tsSigmaForTemp")->TabVisible=IniConfig.bL22Enable3SigmaTempMonitor;           //kevin 20200521 : ASEKH 3SIGMA  溫度統計

    TS_SetTempPanelCaption();

    ;   // golden uTemp_Set.cpp:949-949 取代：ShowLineOnTop：Panel->BringToFront（Z 序，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:949-949 -- ShowLineOnTop：Panel->BringToFront（Z 序，純畫面）
    ShowLineOnTop();
#endif // GATE (S12-C save)
    EL<TPageControl>("TfTemp_Set", "pgcTempOffset")->ActivePageIndex=0;
    ;   // golden uTemp_Set.cpp:951-951 取代：Repaint：重畫（純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:951-951 -- Repaint：重畫（純畫面）
    EL<TPageControl>("TfTemp_Set", "pgcTempOffset")->Repaint();
#endif // GATE (S12-C save)
    EL<TSpeedButton>("TfTemp_Set", "btnSort")->Visible=false;
    fShow=true;

    //Steven 20120804 Start: 在最下面決定權限能不能Enable
    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")       ->Enabled=(AccessLevel<LevelSet.AccessLevel[50])?false:EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")     ->Enabled;   // golden uTemp_Set.cpp:956-956 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:956-956 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")       ->Enabled=(AccessLevel<LevelSet.AccessLevel[50])?false:rgTemperatureMode     ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfTemp_Set", "gbBasePoint")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TGroupBox>("TfTemp_Set", "gbBasePoint")           ->Enabled;   // golden uTemp_Set.cpp:957-957 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:957-957 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TGroupBox>("TfTemp_Set", "gbBasePoint")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:gbBasePoint           ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")        ->Enabled=(AccessLevel<LevelSet.AccessLevel[55])?false:EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")      ->Enabled;   // golden uTemp_Set.cpp:958-958 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:958-958 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")        ->Enabled=(AccessLevel<LevelSet.AccessLevel[55])?false:gbHotModeSetting      ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")    ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")  ->Enabled;   // golden uTemp_Set.cpp:959-959 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:959-959 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")    ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:gbAmbientModeSetting  ->Enabled;
#endif // GATE (S12-C save)
    EL<TTabSheet>("TfTemp_Set", "tsOther")                 ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:EL<TTabSheet>("TfTemp_Set", "tsOther")               ->Enabled;   // golden uTemp_Set.cpp:960-960 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:960-960 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TTabSheet>("TfTemp_Set", "tsOther")                 ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:tsOther               ->Enabled;     //kevin 20150127 add
#endif // GATE (S12-C save)
    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")         ->Enabled=(AccessLevel<LevelSet.AccessLevel[57])?false:EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")       ->Enabled;   // golden uTemp_Set.cpp:961-961 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:961-961 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")         ->Enabled=(AccessLevel<LevelSet.AccessLevel[57])?false:rgIndexHeatMode       ->Enabled;
#endif // GATE (S12-C save)
    EL<TButton>("TfTemp_Set", "btClearAll")              ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TButton>("TfTemp_Set", "btClearAll")            ->Enabled;   // golden uTemp_Set.cpp:962-962 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:962-962 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
    EL<TButton>("TfTemp_Set", "btClearAll")              ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:btClearAll            ->Enabled;
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)
    {
    EL<TEdit>("TfTemp_Set", "edWorkTemp")          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edWorkTemp")                ->Enabled;   // golden uTemp_Set.cpp:966-966 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:966-966 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edWorkTemp")          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edWorkTemp                ->Enabled;                                         //20140312 wei Kyec
#endif // GATE (S12-C save)
    }
    else
    {
    EL<TEdit>("TfTemp_Set", "edAbLow")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edAbLow")->Enabled;   // golden uTemp_Set.cpp:970-970 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:970-970 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edAbLow")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edAbLow->Enabled;                    //kevin 20140918
#endif // GATE (S12-C save)
    EL<TEdit>("TfTemp_Set", "edAbMid")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edAbMid")->Enabled;   // golden uTemp_Set.cpp:971-971 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:971-971 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edAbMid")             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edAbMid->Enabled;                    //kevin 20140918
#endif // GATE (S12-C save)
    EL<TEdit>("TfTemp_Set", "edLowBase")           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edLowBase")->Enabled;   // golden uTemp_Set.cpp:972-972 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:972-972 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edLowBase")           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edLowBase->Enabled;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTemp_Set", "edMidBase")           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edMidBase")->Enabled;   // golden uTemp_Set.cpp:973-973 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:973-973 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edMidBase")           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edMidBase->Enabled;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTemp_Set", "edHighBase")          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edHighBase")->Enabled;   // golden uTemp_Set.cpp:974-974 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:974-974 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edHighBase")          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edHighBase->Enabled;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTemp_Set", "edSHighBase")         ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edSHighBase")->Enabled;   // golden uTemp_Set.cpp:975-975 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:975-975 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
        EL<TEdit>("TfTemp_Set", "edSHighBase")         ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edSHighBase->Enabled;                //Ztex 2024.07.27 Add 6 Point Temperature Offset
#endif // GATE (S12-C save)

        if(IniConfig.bKoreaFunction)                                            //ChungHung 20140609 add Korea 溫度要卡權限 包含 Temp_Set
    EL<TEdit>("TfTemp_Set", "edWorkTemp")      ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:EL<TEdit>("TfTemp_Set", "edWorkTemp")                ->Enabled;   // golden uTemp_Set.cpp:978-978 取代：三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:978-978 -- 三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫
            EL<TEdit>("TfTemp_Set", "edWorkTemp")      ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edWorkTemp                ->Enabled;
#endif // GATE (S12-C save)
    }

    for(int i=tcHotPlate1; i<tcTotalCount; i++)
    {
        myTempPal[i]->edLow->Enabled             =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edLow->Enabled;                                    //Steven 20200717 : 補上前兩點的權限控制
        myTempPal[i]->edMid->Enabled             =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edMid->Enabled;
        myTempPal[i]->edLowbase->Enabled         =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edLowbase->Enabled;
        myTempPal[i]->edBase->Enabled            =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edBase->Enabled;
        myTempPal[i]->edHighBase->Enabled        =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edHighBase->Enabled;
        myTempPal[i]->edOffset->Enabled          =(AccessLevel<LevelSet.AccessLevel[53])?false:myTempPal[i]->edOffset->Enabled;
        myTempPal[i]->edSingleLimit->Enabled     =(AccessLevel<LevelSet.AccessLevel[54])?false:myTempPal[i]->edSingleLimit->Enabled;
        myTempPal[i]->edInitTempOffset->Enabled  =(AccessLevel<LevelSet.AccessLevel[106])?false:myTempPal[i]->edInitTempOffset->Enabled;
        myTempPal[i]->edEOTTempOffset->Enabled   =(AccessLevel<LevelSet.AccessLevel[124])?false:myTempPal[i]->edEOTTempOffset->Enabled;
        myTempPal[i]->edSHighBase->Enabled       =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edSHighBase->Enabled;                              //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    //Steven 20120804 End: 在最下面決定權限能不能Enable

    //jou 2012-09-17 SPIL要修改常溫關閉chamber風扇,必須使用hontech權限
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Enabled=false;                                        //Ifor 20160518 客戶要求ATC 溫度 Offset 不可修改
        EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")->Enabled=false;                                         //Ifor 20160411 客戶要求反灰不可修改
    }

    if(CUSTOMER_CODE==CC_AMKOR_China ||                                         //jou 2013-04-24 客戶說常溫應該要disable
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 add QUALCOMM
    {
        bool bTempFlag=true;
        if(Temperature.iMachineTempMode==1)
            bTempFlag=false;
        else
            bTempFlag=true;

        for(int i=tcHotPlate1; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Enabled            =bTempFlag;                 //Steven 20200717 : 補上前兩點的權限控制
            myTempPal[i]->edMid->Enabled            =bTempFlag;                 //Steven 20200717 : 補上前兩點的權限控制
            myTempPal[i]->edLowbase->Enabled        =bTempFlag;
            myTempPal[i]->edBase->Enabled           =bTempFlag;
            myTempPal[i]->edHighBase->Enabled       =bTempFlag;
            myTempPal[i]->edOffset->Enabled         =bTempFlag;
            myTempPal[i]->edSingleLimit->Enabled    =bTempFlag;
            myTempPal[i]->edSHighBase->Enabled      =bTempFlag;                 //Ztex 2024.07.27 Add 6 Point Temperature Offset
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //20140320 wei   KYEC 低於權限顯示不能修改
    {
        if(AccessLevel<LevelSet.AccessLevel[17])
        {
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")   ->Enabled=false;
            EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")    ->Enabled=false;
            EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")->Enabled=false;
            EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")     ->Enabled=false;
            EL<TPanel>("TfTemp_Set", "pnlHeatMode")         ->Enabled=false;
            EL<TGroupBox>("TfTemp_Set", "gbTempOffset")        ->Enabled=false;
        }
        else
        {
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")   ->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbHotModeSetting")    ->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbAmbientModeSetting")->Enabled=true;
            EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")     ->Enabled=true;
            EL<TPanel>("TfTemp_Set", "pnlHeatMode")         ->Enabled=true;
            EL<TGroupBox>("TfTemp_Set", "gbTempOffset")        ->Enabled=true;
        }
    }

    EL<TCheckBox>("TfTemp_Set", "chkTesterDock")->Visible=IniConfig.bA05UseAutoDocking;
    EL<TEdit>("TfTemp_Set", "edtTestDock")->Visible  =IniConfig.bA05UseAutoDocking;

    if(CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20140714 only for SCK add CUSTOMER_CODE==CC_SCK
    {
        EL<TCheckBox>("TfTemp_Set", "chkTesterDock")->Visible=true;
        EL<TEdit>("TfTemp_Set", "edtTestDock")  ->Visible=true;
        EL<TCheckBox>("TfTemp_Set", "chkTesterDock")->Enabled=false;
    }

    if(IniConfig.bTemp25degControl==true)                                       //jou 2014-06-07 Temperature 25 deg. control
    {
        if((TestIF.iTestMode==_8Site2X4 || LastSet.iTemperature==Tempture_Hot) &&
            Temperature.iIndexHeatMode==HeadOnly && Temperature.fSoakTime==0 && Temperature.fWorkTemperBase<=25.0)
        {
    if(IndexHasIC() || ShuttleHasIC() || HasICUnderHotPlate())   // golden uTemp_Set.cpp:1062-1063 取代：InArmSuck／MOT[] 本 TU 看不到（aHotPlateSubstrate.h 與 HTEditList.h 衝突）；HasICUnderHotPlate() 是超集（多 OutArmSuck／RotateKit），保守鎖定
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:1062-1063 -- InArmSuck／MOT[] 本 TU 看不到（aHotPlateSubstrate.h 與 HTEditList.h 衝突）；HasICUnderHotPlate() 是超集（多 OutArmSuck／RotateKit），保守鎖定
            if(IndexHasIC() || ShuttleHasIC() || InArmSuck.HasIC() ||
               MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())
#endif // GATE (S12-C save)
            {
                EL<TEdit>("TfTemp_Set", "edSoakTime")->Enabled=false;
                EL<TEdit>("TfTemp_Set", "edWorkTemp")->Enabled=false;
            }
            else
            {
                EL<TEdit>("TfTemp_Set", "edSoakTime")->Enabled=true;
                EL<TEdit>("TfTemp_Set", "edWorkTemp")->Enabled=true;
            }
        }
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[0])
        {
            EL<TEdit>("TfTemp_Set", "edWorkTemp")->Enabled=false;
        }

        if(bAuthCriticalPara[1])
        {
            EL<TEdit>("TfTemp_Set", "edSoakTime")->Enabled=false;
        }
    }

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    EL<TLabel>("TfTemp_Set", "lblRT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TLabel>("TfTemp_Set", "lblFT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    EL<TPanel>("TfTemp_Set", "pnlArm1Offset")->Visible    =false;                                           //wei 20151230
    EL<TPanel>("TfTemp_Set", "pnlArm1Offset")->Enabled    =fSecurity->Insufficient(17,false);               //wei 20151230
    EL<TPanel>("TfTemp_Set", "pnlArm2Offset")->Visible    =false;                                           //wei 20151230
    EL<TPanel>("TfTemp_Set", "pnlArm2Offset")->Enabled    =fSecurity->Insufficient(17,false);               //wei 20151230

    EL<TGroupBox>("TfTemp_Set", "gbNoFullsiteOffset")->Visible =(CUSTOMER_CODE==CC_TSMC_TAINAN && IniConfig.bL18NofullsiteaddTemperatureoffset);       //wei 20160107 No FullSite Add Offset

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160101 海思專用版本 顯示不可修改溫度
    {
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled      =false;
        EL<TTabSheet>("TfTemp_Set", "tsHot")->Enabled                  =false;
        EL<TTabSheet>("TfTemp_Set", "tsAmbient")->Enabled              =false;
//        tsOther->Enabled                =false;
        EL<TTabSheet>("TfTemp_Set", "tsAmbCtr")->Enabled               =false;                                 //JerryYang 20181002 (Steven) : fix頁面完全鎖定住
        EL<TTabSheet>("TfTemp_Set", "tsInitOffset")->Enabled           =false;
        EL<TTabSheet>("TfTemp_Set", "tsBoostFunc")->Enabled            =false;
        EL<TGroupBox>("TfTemp_Set", "gbBasePoint")->Enabled            =false;
        EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Enabled        =false;
        //tsHeatGun->Enabled              =false;                               //kevin 20200513 HSIS 可以改OFFSET
        EL<TCheckBox>("TfTemp_Set", "cbHeaterGun")->Enabled              =false;                               //kevin 20200513 HSIS 可以改OFFSET
        //tsATC->Enabled=false;//Ifor 20160720 tsATC->Enabled 改成 rbATCActiveOn->Enabled & palATC->Enable
        EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Enabled          =false;
        EL<TPanel>("TfTemp_Set", "palATC")->Enabled                 =false;
        EL<TTabSheet>("TfTemp_Set", "tsATC70")->Enabled                =false;
        EL<TTabSheet>("TfTemp_Set", "tsATC_PID")->Enabled              =false;
        EL<TPanel>("TfTemp_Set", "pnlArm1Offset")->Enabled          =false;
        EL<TPanel>("TfTemp_Set", "pnlArm2Offset")->Enabled          =false;
        EL<TEdit>("TfTemp_Set", "edATCAmbTemp")->Enabled           =false;
        EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Enabled  =false;
        EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Enabled      =false;
        EL<TEdit>("TfTemp_Set", "edChillerTemp")->Enabled          =false;
        EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Enabled       =false;
        EL<TGroupBox>("TfTemp_Set", "grpInPC")->Enabled                =false;
    }
    else if(bEnablePEModel==true)                                               //Ifor 20160824 避免PE工程模式無法開啟
    {
        EL<TPanel>("TfTemp_Set", "pnlArm1Offset")->Enabled          =true;
        EL<TPanel>("TfTemp_Set", "pnlArm2Offset")->Enabled          =true;
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled      =true;
        EL<TTabSheet>("TfTemp_Set", "tsHot")->Enabled                  =true;
        EL<TTabSheet>("TfTemp_Set", "tsAmbient")->Enabled              =true;
        EL<TTabSheet>("TfTemp_Set", "tsOther")->Enabled                =true;                                  //Ifor 20160825 Temp_Set Other選擇頁面
        EL<TGroupBox>("TfTemp_Set", "gbBasePoint")->Enabled            =true;                                  //Ifor 20160825 Base Point選擇頁面
        EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Enabled        =true;                                  //Ifor 20160825 Heat Mode 選擇頁面
        EL<TTabSheet>("TfTemp_Set", "tsHeatGun")->Enabled              =true;                                  //Ifor 20160825 Heat Gun 頁面控制
        EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Enabled          =true;                                  //Ifor 20160825 ATC Active ATC Cooling 選項控制
        EL<TPanel>("TfTemp_Set", "palATC")->Enabled                 =true;                                  //Ifor 20160825 ATC 下方控制
        EL<TTabSheet>("TfTemp_Set", "tsATC70")->Enabled                =true;                                  //Ifor 20160825 ATC 7.0 頁面控制
//        if(CosFunction.bHiSiliconFunction==true)
//        {
//            grpATCTempOffset->Enabled   =false;                               //Ifor 20160825 ATC Temp Offset
//            grpInPC->Enabled            =false;                               //Ifor 20160825 Temp offset
//            tsATC_PID->Enabled          =true;                                //Ifor 20160825 ATC_PID
//        }
//        else
//        {
        EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Enabled       =true;                                  //Ifor 20160825 ATC Temp Offset
        EL<TGroupBox>("TfTemp_Set", "grpInPC")->Enabled                =true;                                  //Ifor 20160825 Temp offset
        EL<TTabSheet>("TfTemp_Set", "tsATC_PID")->Enabled              =true;                                  //Ifor 20160825 ATC_PID
//        }
        EL<TEdit>("TfTemp_Set", "edATCAmbTemp")->Enabled           =true;
        EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Enabled  =true;
        EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Enabled      =true;
        EL<TEdit>("TfTemp_Set", "edChillerTemp")->Enabled          =true;
    }

    if(CosFunction.bHiSiliconFunction==false && ATC_SYSTEM==eNewATCSystem)
    {
        if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                                     //kevin 20181219 (Steven) :  add write ATC FILE NAME
            EL<TEdit>("TfTemp_Set", "edAtcFileName")->Visible=false;
        EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Visible=true;
    }

    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)                          //Ifor 20160420 ATC Temp Offset設定
    {
        EL<TGroupBox>("TfTemp_Set", "grpInPC")->Visible=true;
    }
    else if(ATC_SYSTEM==eNewATCSystem)
    {
        if(CosFunction.bUseOldATCTempOffset==true)                              //JerryYang 20190123 ATC Offset UI使用舊版
        {
            EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Visible=true;
        }
        else
        {
            EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Visible=CosFunction.bUseSecondATCTempOffset;      //Ifor 20241118 : 測試中變溫
        }
    }
    else if(ATC_SYSTEM==eATCHonPrecType)
    {
        EL<TGroupBox>("TfTemp_Set", "grpInPC")->Visible=true;
        EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Enabled=false;                                           //Ifor 20161103 ATC2.0 Chiller 溫度固定20度不可修改
    }

    if((CUSTOMER_CODE==CC_KYEC_LEE && bUseATC_SelfTestFunction==true) ||        //Alick 20170117 add 京元要求一般版也要有//Ifor 20170124 : add KYEC 要求由General.ini開關使用ATC Self Test功能
        CosFunction.bHiSiliconFunction==true)                                   //Ifor 20160907 Modify ATC Slef Test 按鍵 僅顯示於海思版本
        EL<TSpeedButton>("TfTemp_Set", "sbSafeTestATC")->Visible=true;
    else
        EL<TSpeedButton>("TfTemp_Set", "sbSafeTestATC")->Visible=false;

    //Ifor 20170426 整合UpDateEdit資料至FormShow 處理
    //==>
//    labCHA->Visible         =(ATC_SYSTEM<=eATC60);                            //20141204 ChungHung add for ATC3.0  //Steven 20120606 : ATC不需要顯示Chamber Offset
    if(IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false)   //Sam 20231214 : Temp offset use ready temp range
    {
        EL<TLabel>("TfTemp_Set", "labReadyTempRange")->Visible=true;
        EL<TEdit>("TfTemp_Set", "edTempReadyRange")->Visible=true;
    }
    else
    {
        EL<TLabel>("TfTemp_Set", "labReadyTempRange")->Visible=false;
        EL<TEdit>("TfTemp_Set", "edTempReadyRange")->Visible=false;
    }

    EL<TGroupBox>("TfTemp_Set", "gb_TjFunction")->Visible  =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);
    EL<TGroupBox>("TfTemp_Set", "gbATC70")->Visible        =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70
    EL<TGroupBox>("TfTemp_Set", "gbTSD")->Visible          =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70
    if(CosFunction.bHiSiliconFunction==true || TestIF_File.i2DIDFormat==eAMD || CosFunction.bATCUseHandlerRecipeName==true)                                     //Ifor 20191126 : add ATC Use Handler Recipe Name  //JerryYang 20200422 2DID format選項改用下拉選單
    {
        EL<TGroupBox>("TfTemp_Set", "gbFileName")->Visible=false;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //KEVIN 20181121 (Steven) : atc file name
        {
            EL<TGroupBox>("TfTemp_Set", "gbFileName")->Visible =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);               //Steven 20150810 : add for ATC //Ifor 20160111 Add For New Atc System
            EL<TEdit>("TfTemp_Set", "edAtcFileName")->Enabled=false;
            EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Enabled=false;
        }
    }
    else
    {
        EL<TGroupBox>("TfTemp_Set", "gbFileName")->Visible =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);                   //Steven 20150810 : add for ATC //Ifor 20160111 Add For New Atc System
    }

    EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible  =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eATCHonPrecType || ATC_SYSTEM==eNewATCSystem);                            //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0

    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)
        EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible=false;

    if(CosFunction.bUseHandlerSetChillerTemp==true)                             //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    {
        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 || ATC_SYSTEM==eATCHonPrecType)
        {
            EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible=false;
        }
        else
        {
            EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible=true;
        }
    }
    else
    {
        EL<TGroupBox>("TfTemp_Set", "gbChillerTemp")->Visible=false;
    }

    EL<TRadioButton>("TfTemp_Set", "RachoiceATCFile")->Checked=true;                                              //kevin 20181219 (Steven) :  選ATC工作檔還是輸入
    EL<TGroupBox>("TfTemp_Set", "gbATCPackageOffset")->Visible=(CosFunction.bATCUsePackageOffset);             //Ifor 20190215 : add ATC 使用 Package Offset 功能
    EL<TGroupBox>("TfTemp_Set", "grpTJTempRange")->Visible=(CosFunction.bUseATCTJControlMode || Temperature.bEnableTJFunction);                        //Ifor 20190328 : add TJ Temp Over Range  //JerryYang 20251124 : Tj control吃不同的溫度range
    if(CosFunction.bATC32UseTJMode==true)
    {
        EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Caption="Display TJ Temperature";
    }
    //<==
    //Ifor 20170426 整合UpDateEdit資料至FormShow 處理
    //Ifor 20170905 (wei) add HT9045AT 不顯示Heat頁面
    //==>
     //kevin 20181016 add
    if(CosFunction.bUseOldATCTempOffset==true)                                  //JerryYang 20190123 ATC Offset UI使用舊版
    {
        if(ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16)                // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180817 : Boost Function
        {
            EL<TTabSheet>("TfTemp_Set", "tsArm1")      ->TabVisible=false;
            EL<TTabSheet>("TfTemp_Set", "tsArm2")      ->TabVisible=false;
        }
    }
    else
    {
        EL<TGroupBox>("TfTemp_Set", "grpATCTempOffset")->Visible=CosFunction.bUseSecondATCTempOffset;          //Ifor 20241014 Use Second ATC Temp Offset
    }
    //<==
    //Ifor 20170905 (wei) add HT9045AT 不顯示Heat頁面
    if(CUSTOMER_CODE==CC_ASE_M && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_32)
    {
        EL<TGroupBox>("TfTemp_Set", "gbTSD")->Visible                  =true;
        EL<TGroupBox>("TfTemp_Set", "gbATC70")->Visible                =false;
        EL<TGroupBox>("TfTemp_Set", "gbATCInitialOffset")->Visible     =false;
        EL<TGroupBox>("TfTemp_Set", "gbATCConsFail")->Visible          =false;
        EL<TGroupBox>("TfTemp_Set", "gbATCQAModeOffset")->Visible      =false;
        EL<TGroupBox>("TfTemp_Set", "gbATCTestTimeOffset")->Visible    =false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        EL<TPanel>("TfTemp_Set", "PnlIndexSoakTime")->Visible=false;                                        //kevin 20210511 ASE KH  JL 說關閉
    }

    EL<TGroupBox>("TfTemp_Set", "gbHeatGunTempATC")->Visible=(INSTALL_ATC_HEAT_GUN>0);                         //JerryYang 20220408 : add for ATC3.5
    EL<TGroupBox>("TfTemp_Set", "gb_HotGunFlow")->Visible=HotGunFlowEnable;                                    //KaiChen 20190729 ：Hot Gun Flow
    if(ATC_SYSTEM==eNewATCSystem &&
    (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36))   // golden uTemp_Set.cpp:1303-1303 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:1303-1303 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
       ATC_InterfaceForm->IS_ATC33())
#endif // GATE (S12-C save)
    {
        EL<TGroupBox>("TfTemp_Set", "grpATC_HotAir")->Visible=true;
        EL<TGroupBox>("TfTemp_Set", "grpTemperatureRise")->Visible=true;
    }
    else
    {
        EL<TGroupBox>("TfTemp_Set", "grpATC_HotAir")->Visible=false;
        EL<TGroupBox>("TfTemp_Set", "grpTemperatureRise")->Visible=false;
    }
    EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Visible=CosFunction.bTempCalByRecipe;                   //jou 20220725 : Temperature calibration by recipe

    EL<TCheckBox>("TfTemp_Set", "cbByPassChamber")->Visible=(CUSTOMER_CODE==CC_HONPREC_QC);                    //KenHsieh 20230301 : By Pass Chamber
    EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Cells[0][0]="Define";                                              //JerryYang 20231128 : 傳送ATC site map
    EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Cells[0][1]="TC";
    for(int i=0; i<16; i++)
    {
        S.sprintf("TJ%02d", i+1);
        EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Cells[i+1][0]=S;
    }

    EL<TGroupBox>("TfTemp_Set", "gbPowerFollowFuncAct")->Visible=IniConfig.bL43EnableATCPowerFollow;           //Hmy 20240207 Add Send Pf Par to Atc

    EL<TTabSheet>("TfTemp_Set", "tsATCMultiSensor")->TabVisible=(ATC_SYSTEM==eNewATCSystem &&
                                 (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                                  ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36) );
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden uTemp_Set.cpp:1333  TfTemp_Set::SetTempPanelCaption()
static void TS_SetTempPanelCaption()
{

    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08 )                                       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {                                                                           //Steven 20140620 : Jovie說畫面要有溫控器對應位置
        myTempPal[tcAa1]->Caption="A1";
        myTempPal[tcBa1]->Caption="B1";
        myTempPal[tcAb1]->Caption="C1";
        myTempPal[tcBb1]->Caption="D1";
        myTempPal[tcAc1]->Caption="E1";
        myTempPal[tcBc1]->Caption="F1";
        myTempPal[tcAd1]->Caption="G1";
        myTempPal[tcBd1]->Caption="H1";
        myTempPal[tcAe1]->Caption="I1";
        myTempPal[tcBe1]->Caption="J1";
        myTempPal[tcAf1]->Caption="K1";
        myTempPal[tcBf1]->Caption="L1";
        myTempPal[tcAg1]->Caption="M1";
        myTempPal[tcBg1]->Caption="N1";
        myTempPal[tcAh1]->Caption="O1";
        myTempPal[tcBh1]->Caption="P1";

        myTempPal[tcAa2]->Caption="A2";
        myTempPal[tcBa2]->Caption="B2";
        myTempPal[tcAb2]->Caption="C2";
        myTempPal[tcBb2]->Caption="D2";
        myTempPal[tcAc2]->Caption="E2";
        myTempPal[tcBc2]->Caption="F2";
        myTempPal[tcAd2]->Caption="G2";
        myTempPal[tcBd2]->Caption="H2";
        myTempPal[tcAe2]->Caption="I2";
        myTempPal[tcBe2]->Caption="J2";
        myTempPal[tcAf2]->Caption="K2";
        myTempPal[tcBf2]->Caption="L2";
        myTempPal[tcAg2]->Caption="M2";
        myTempPal[tcBg2]->Caption="N2";
        myTempPal[tcAh2]->Caption="O2";
        myTempPal[tcBh2]->Caption="P2";

        if(CosFunction.bTempSetDisenableNotUseHeater)
        {
            for(int i=tcAa1; i<tcBd2+1; i++)
            {
                myTempPal[i]->SetEnable(bUT150Install[i]);
            }
            for(int i=tcAe1; i<tcBh2+1; i++)
            {
                myTempPal[i]->SetEnable(bUT150Install[i]);
            }
        }

        switch(TestIF.iTestMode)
        {
            case SingleSite:
                if(ATC_SYSTEM>=eATC60 && Temperature.bATCActiveCooling==true)   //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    if(Temperature.bMultiZoneEnable==true)                      //Ifor 20240821 add:ATC多區溫控模式Offset補償
                    {
                        if(Temperature.bZoneTempEnable[1]==true)
                        {
                            myTempPal[tcAb1]->Caption="C1 (Ab1)";
                            myTempPal[tcAb2]->Caption="C2 (Ab2)";
                        }

                        if(Temperature.bZoneTempEnable[2]==true)
                        {
                            myTempPal[tcAc1]->Caption="E1 (Ac1)";
                            myTempPal[tcAc2]->Caption="E2 (Ac2)";
                        }

                        if(Temperature.bZoneTempEnable[3]==true)
                        {
                            myTempPal[tcAd1]->Caption="G1 (Ad1)";
                            myTempPal[tcAd2]->Caption="G2 (Ad2)";
                        }
                    }
                }
                else
                {
                    if(TestIF_File.b2CableLayoutKit)                            //JerryYang 20160826 single site兩條線版本(使用一支加熱棒)
                    {
                        myTempPal[tcAa1]->Caption="A1 (Aa1)";
                        myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    }
                    else
                    {
                        myTempPal[tcAa1]->Caption="A1 (Aa1)";
                        myTempPal[tcBa1]->Caption="B1 (Aa1)";
                        myTempPal[tcAa2]->Caption="A2 (Aa2)";
                        myTempPal[tcBa2]->Caption="B2 (Aa2)";
                    }
                }
                break;
            case DualSite:                                                      //1x2
                if(TestIF_File.bUse1x3SiteKit==true &&                          //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                   Temperature.bATCActiveCooling==true)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                }
                else if((ATC_SYSTEM==eNewATCSystem ||                           //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ab1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ab2)";
                }
                else if(TestIF_File.b2CableLayoutKit)                           //JerryYang 20160826 1x2 兩條線版本使用兩支加熱棒
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //jou 2015-10-15  : 16溫控器 1條線版本
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ab1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ab2)";
                }
                else if(TestIF_File.bUse1x3SiteKit==true)                       //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Aa1)";
                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Aa2)";

                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Ab1)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Ab2)";
                }
                break;
            case TriSite1X3:
                myTempPal[tcAa1]->Caption="A1 (Aa1)";
                myTempPal[tcAb1]->Caption="C1 (Ab1)";
                myTempPal[tcAc1]->Caption="E1 (Ac1)";

                myTempPal[tcAa2]->Caption="A2 (Aa2)";
                myTempPal[tcAb2]->Caption="C2 (Ab2)";
                myTempPal[tcAc2]->Caption="E2 (Ac2)";
                break;
            case QualSite1X4:                                                   //1x4
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 修正 ATC 1*4 溫度顯示異常問題
                   Temperature.bATCActiveCooling==true)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                }
                else if(TestIF_File.b2CableLayoutKit)                           //Steven 20150724 : 16溫控器 2條線版本
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                }
                else if(TestIF_File.bNS7000kit==true ||
                        TestIF_File.b1CableLayoutKit==true)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ab1)";
                    myTempPal[tcAb1]->Caption="C1 (Ac1)";
                    myTempPal[tcBb1]->Caption="D1 (Ad1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ab2)";
                    myTempPal[tcAb2]->Caption="C2 (Ac2)";
                    myTempPal[tcBb2]->Caption="D2 (Ad2)";
                }
                else                                                            //Steven 20250821 : 1x4 三條線版本
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ab1)";
                    myTempPal[tcAb1]->Caption="C1 (Ac1)";
                    myTempPal[tcBb1]->Caption="D1 (Ad1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ab2)";
                    myTempPal[tcAb2]->Caption="C2 (Ac2)";
                    myTempPal[tcBb2]->Caption="D2 (Ad2)";
                }
                break;
            case DualSite2x1:
                myTempPal[tcAa1]->Caption="A1 (Aa1)";
                myTempPal[tcBa1]->Caption="B1 (Aa1)";
                myTempPal[tcAa2]->Caption="A2 (Aa2)";                           //Ifor 20191015 : A1 => A2
                myTempPal[tcBa2]->Caption="B2 (Aa2)";                           //Ifor 20191015 : B1 => B2

                myTempPal[tcAb1]->Caption="C1 (Ba1)";
                myTempPal[tcBb1]->Caption="D1 (Ba1)";
                myTempPal[tcAb2]->Caption="C2 (Ba2)";                           //Ifor 20191015 : C1 => C2
                myTempPal[tcBb2]->Caption="D2 (Ba2)";                           //Ifor 20191015 : D1 => D2
                break;
            case QualSite2X2:                                                   //2x2
                if(TestIF_File.bSquare_OctalKit ||                              //Steven 20141224 : 2x2Site使用8Site Kit
                   TestIF_File.b2x2Use16SiteKit)                                //Steven 20191113 : 2x2Site使用16Site Kit
                {
                    myTempPal[tcAb1]->Caption="C1 (Aa1)";
                    myTempPal[tcBb1]->Caption="D1 (Ba1)";
                    myTempPal[tcAc1]->Caption="E1 (Ab1)";
                    myTempPal[tcBc1]->Caption="F1 (Bb1)";

                    myTempPal[tcAa2]->Caption="C2 (Aa2)";
                    myTempPal[tcBa2]->Caption="D2 (Ba2)";
                    myTempPal[tcAb2]->Caption="E2 (Ab2)";
                    myTempPal[tcBb2]->Caption="F2 (Bb2)";
                }
                else if(CUSTOMER_CODE==CC_ATEC && TestIF_File.b2CableLayoutKit && DeviceForm_File.iHeadDeviceCT==3)     //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1, Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1, Bb1)";
                    myTempPal[tcAa2]->Caption="A2 (Aa2, Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2, Bb2)";
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //JerryYang 20181207 2x2新增一條線版本
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1, Ba1)";
                    myTempPal[tcBa1]->Caption="B1 (Ab1, Bb1)";
                    myTempPal[tcAa2]->Caption="A2 (Aa2, Ba2)";
                    myTempPal[tcBa2]->Caption="B2 (Ab2, Bb2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                }
                break;
            case QualSite2X2N:                                                  //Frank 20200520 2X2NN Mode
                if(TestIF_File.bUse1x3SiteKit==true)                            //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Ba)";
                    myTempPal[tcBa1]->Caption="B1 (Bb)";
                    myTempPal[tcAa2]->Caption="A2 (Aa)";
                    myTempPal[tcBa2]->Caption="B2 (Ab)";
                }
                break;
            case _6Site2X3:                                                     //ChungHung 20140115 add for 2x3_6
                myTempPal[tcAa1]->Caption="A1 (Aa1)";
                myTempPal[tcBa1]->Caption="B1 (Ba1)";
                myTempPal[tcAb1]->Caption="C1 (Ab1)";
                myTempPal[tcBb1]->Caption="D1 (Bb1)";
                myTempPal[tcAc1]->Caption="E1 (Ac1)";
                myTempPal[tcBc1]->Caption="F1 (Bc1)";

                myTempPal[tcAa2]->Caption="A2 (Aa2)";
                myTempPal[tcBa2]->Caption="B2 (Ba2)";
                myTempPal[tcAb2]->Caption="C2 (Ab2)";
                myTempPal[tcBb2]->Caption="D2 (Bb2)";
                myTempPal[tcAc2]->Caption="E2 (Ac2)";
                myTempPal[tcBc2]->Caption="F2 (Bc2)";
                break;
            case _6Site2X3N:                                                    //Steven 20220425 : 2X3NN Mode
                myTempPal[tcAa1]->Caption="A1 (Ba)";
                myTempPal[tcAb1]->Caption="C1 (Bb)";
                myTempPal[tcAc1]->Caption="E1 (Bc)";

                myTempPal[tcAa2]->Caption="A2 (Aa)";
                myTempPal[tcAb2]->Caption="C2 (Ab)";
                myTempPal[tcAc2]->Caption="E2 (Ac)";
                break;
            case _8Site2X4:                                                     //2x4
                if(TestIF_File.bOctal_12Kit==true)                              //JerryYang 20230204 : 修正8 site mode使用12site SLK溫度offset異常
                {
                    myTempPal[tcAb1]->Caption="C1 (Aa1)";
                    myTempPal[tcBb1]->Caption="D1 (Ba1)";
                    myTempPal[tcAc1]->Caption="E1 (Ab1)";
                    myTempPal[tcBc1]->Caption="F1 (Bb1)";
                    myTempPal[tcAd1]->Caption="G1 (Ac1)";
                    myTempPal[tcBd1]->Caption="H1 (Bc1)";
                    myTempPal[tcAe1]->Caption="I1 (Ad1)";
                    myTempPal[tcBe1]->Caption="J1 (Bd1)";

                    myTempPal[tcAb2]->Caption="C2 (Aa2)";
                    myTempPal[tcBb2]->Caption="D2 (Ba2)";
                    myTempPal[tcAc2]->Caption="E2 (Ab2)";
                    myTempPal[tcBc2]->Caption="F2 (Bb2)";
                    myTempPal[tcAd2]->Caption="G2 (Ac2)";
                    myTempPal[tcBd2]->Caption="H2 (Bc2)";
                    myTempPal[tcAe2]->Caption="I2 (Ad2)";
                    myTempPal[tcBe2]->Caption="J2 (Bd2)";
                }
                else if(TestIF_File.bOctal_16Kit==true)
                {
                    myTempPal[tcAc1]->Caption="E1 (Aa1)";
                    myTempPal[tcBc1]->Caption="F1 (Ba1)";
                    myTempPal[tcAd1]->Caption="G1 (Ab1)";
                    myTempPal[tcBd1]->Caption="H1 (Bb1)";
                    myTempPal[tcAe1]->Caption="I1 (Ac1)";
                    myTempPal[tcBe1]->Caption="J1 (Bc1)";
                    myTempPal[tcAf1]->Caption="K1 (Ad1)";
                    myTempPal[tcBf1]->Caption="L1 (Bd1)";

                    myTempPal[tcAc2]->Caption="E2 (Aa2)";
                    myTempPal[tcBc2]->Caption="F2 (Ba2)";
                    myTempPal[tcAd2]->Caption="G2 (Ab2)";
                    myTempPal[tcBd2]->Caption="H2 (Bb2)";
                    myTempPal[tcAe2]->Caption="I2 (Ac2)";
                    myTempPal[tcBe2]->Caption="J2 (Bc2)";
                    myTempPal[tcAf2]->Caption="K2 (Ad2)";
                    myTempPal[tcBf2]->Caption="L2 (Bd2)";
                }
                else if(TestIF_File.bNS8000CS==true)                            //Steven 20250826 : fixed for NS8000cs display
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1, Ab1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1, Bb1)";
                    myTempPal[tcAb1]->Caption="C1 (Ac1, Ad1)";
                    myTempPal[tcBb1]->Caption="D1 (Bc1, Bd1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2, Ab2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2, Bb2)";
                    myTempPal[tcAb2]->Caption="C2 (Ac2, Ad2)";
                    myTempPal[tcBb2]->Caption="D2 (Bc2, Bd2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcBc1]->Caption="F1 (Bc1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";
                    myTempPal[tcBd1]->Caption="H1 (Bd1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcBc2]->Caption="F2 (Bc2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                    myTempPal[tcBd2]->Caption="H2 (Bd2)";
                }
                break;
            case _8Site2X4N:                                                    //Wei 20231211 : 2X4NN Mode
                if(TestIF_File.bNS7000kit==true ||
                   TestIF_File.b2CableLayoutKit==true ||                        //Steven 20250613 : 修正顯示 for 1x4 3 cable SLK
                   TestIF_File.b1CableLayoutKit==true)
                {
                    myTempPal[tcAa1]->Caption="A1 (Ba)";
                    myTempPal[tcBa1]->Caption="B1 (Bb)";
                    myTempPal[tcAb1]->Caption="C1 (Bc)";
                    myTempPal[tcBb1]->Caption="D1 (Bd)";

                    myTempPal[tcAa2]->Caption="A2 (Aa)";
                    myTempPal[tcBa2]->Caption="B2 (Ab)";
                    myTempPal[tcAb2]->Caption="C2 (Ac)";
                    myTempPal[tcBb2]->Caption="D2 (Ad)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Ba)";
                    myTempPal[tcAb1]->Caption="C1 (Bb)";
                    myTempPal[tcAc1]->Caption="E1 (Bc)";
                    myTempPal[tcAd1]->Caption="G1 (Bd)";

                    myTempPal[tcAa2]->Caption="A2 (Aa)";
                    myTempPal[tcAb2]->Caption="C2 (Ab)";
                    myTempPal[tcAc2]->Caption="E2 (Ac)";
                    myTempPal[tcAd2]->Caption="G2 (Ad)";
                }
                break;
            case _10Site2X5:                                                    //wei 20190614 10 site
                if(TestIF_File.bUse32Heater || TestIF_File.b12SiteUse10Heater)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcBc1]->Caption="F1 (Bc1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";
                    myTempPal[tcBd1]->Caption="H1 (Bd1)";
                    myTempPal[tcAe1]->Caption="I1 (Ae1)";
                    myTempPal[tcBe1]->Caption="J1 (Be1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcBc2]->Caption="F2 (Bc2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                    myTempPal[tcBd2]->Caption="H2 (Bd2)";
                    myTempPal[tcAe2]->Caption="I2 (Ae2)";
                    myTempPal[tcBe2]->Caption="J2 (Be2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1, Ad1)";
                    myTempPal[tcBc1]->Caption="F1 (Bc1, Bd1)";
                    myTempPal[tcAd1]->Caption="G1 (Ae1)";
                    myTempPal[tcBd1]->Caption="H1 (Be1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2, Ad2)";
                    myTempPal[tcBc2]->Caption="F2 (Bc2, Bd2)";
                    myTempPal[tcAd2]->Caption="G2 (Ae2)";
                    myTempPal[tcBd2]->Caption="H2 (Be2)";
                }
                break;
            case _12Site2X6:
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcBc1]->Caption="F1 (Bc1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";
                    myTempPal[tcBd1]->Caption="H1 (Bd1)";
                    myTempPal[tcAe1]->Caption="I1 (Ae1)";
                    myTempPal[tcBe1]->Caption="J1 (Be1)";
                    myTempPal[tcAf1]->Caption="K1 (Af1)";
                    myTempPal[tcBf1]->Caption="L1 (Bf1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcBc2]->Caption="F2 (Bc2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                    myTempPal[tcBd2]->Caption="H2 (Bd2)";
                    myTempPal[tcAe2]->Caption="I2 (Ae2)";
                    myTempPal[tcBe2]->Caption="J2 (Be2)";
                    myTempPal[tcAf2]->Caption="K2 (Af2)";
                    myTempPal[tcBf2]->Caption="L2 (Bf2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1, Ab1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1, Bb1)";
                    myTempPal[tcAb1]->Caption="C1 (Ac1, Ad1)";
                    myTempPal[tcBb1]->Caption="D1 (Bc1, Bd1)";
                    myTempPal[tcAc1]->Caption="E1 (Ae1, Af1)";
                    myTempPal[tcBc1]->Caption="F1 (Be1, Bf1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2, Ab2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2, Bb2)";
                    myTempPal[tcAb2]->Caption="C2 (Ac2, Ad2)";
                    myTempPal[tcBb2]->Caption="D2 (Bc2, Bd2)";
                    myTempPal[tcAc2]->Caption="E2 (Ae2, Af2)";
                    myTempPal[tcBc2]->Caption="F2 (Be2, Bf2)";
                }
                break;
            case _16Site2X8:                                                    //2x8
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1)";
                    myTempPal[tcAb1]->Caption="C1 (Ab1)";
                    myTempPal[tcBb1]->Caption="D1 (Bb1)";
                    myTempPal[tcAc1]->Caption="E1 (Ac1)";
                    myTempPal[tcBc1]->Caption="F1 (Bc1)";
                    myTempPal[tcAd1]->Caption="G1 (Ad1)";
                    myTempPal[tcBd1]->Caption="H1 (Bd1)";
                    myTempPal[tcAe1]->Caption="I1 (Ae1)";
                    myTempPal[tcBe1]->Caption="J1 (Be1)";
                    myTempPal[tcAf1]->Caption="K1 (Af1)";
                    myTempPal[tcBf1]->Caption="L1 (Bf1)";
                    myTempPal[tcAg1]->Caption="M1 (Ag1)";
                    myTempPal[tcBg1]->Caption="N1 (Bg1)";
                    myTempPal[tcAh1]->Caption="O1 (Ah1)";
                    myTempPal[tcBh1]->Caption="P1 (Bh1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2)";
                    myTempPal[tcAb2]->Caption="C2 (Ab2)";
                    myTempPal[tcBb2]->Caption="D2 (Bb2)";
                    myTempPal[tcAc2]->Caption="E2 (Ac2)";
                    myTempPal[tcBc2]->Caption="F2 (Bc2)";
                    myTempPal[tcAd2]->Caption="G2 (Ad2)";
                    myTempPal[tcBd2]->Caption="H2 (Bd2)";
                    myTempPal[tcAe2]->Caption="I2 (Ae2)";
                    myTempPal[tcBe2]->Caption="J2 (Be2)";
                    myTempPal[tcAf2]->Caption="K2 (Af2)";
                    myTempPal[tcBf2]->Caption="L2 (Bf2)";
                    myTempPal[tcAg2]->Caption="M2 (Ag2)";
                    myTempPal[tcBg2]->Caption="N2 (Bg2)";
                    myTempPal[tcAh2]->Caption="O2 (Ah2)";
                    myTempPal[tcBh2]->Caption="P2 (Bh2)";
                }
                else
                {
                    myTempPal[tcAa1]->Caption="A1 (Aa1, Ab1)";
                    myTempPal[tcBa1]->Caption="B1 (Ba1, Bb1)";
                    myTempPal[tcAb1]->Caption="C1 (Ac1, Ad1)";
                    myTempPal[tcBb1]->Caption="D1 (Bc1, Bd1)";
                    myTempPal[tcAc1]->Caption="E1 (Ae1, Af1)";
                    myTempPal[tcBc1]->Caption="F1 (Be1, Bf1)";
                    myTempPal[tcAd1]->Caption="G1 (Ag1, Ah1)";
                    myTempPal[tcBd1]->Caption="H1 (Bg1, Bh1)";

                    myTempPal[tcAa2]->Caption="A2 (Aa2, Ab2)";
                    myTempPal[tcBa2]->Caption="B2 (Ba2, Bb2)";
                    myTempPal[tcAb2]->Caption="C2 (Ac2, Ad2)";
                    myTempPal[tcBb2]->Caption="D2 (Bc2, Bd2)";
                    myTempPal[tcAc2]->Caption="E2 (Ae2, Af2)";
                    myTempPal[tcBc2]->Caption="F2 (Be2, Bf2)";
                    myTempPal[tcAd2]->Caption="G2 (Ag2, Ah2)";
                    myTempPal[tcBd2]->Caption="H2 (Bg2, Bh2)";
                }
                break;
            case _32Site4X8N:
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->Caption="A1 (Ca)";
                    myTempPal[tcBa1]->Caption="B1 (Da)";
                    myTempPal[tcAb1]->Caption="C1 (Cb)";
                    myTempPal[tcBb1]->Caption="D1 (Db)";
                    myTempPal[tcAc1]->Caption="E1 (Cc)";
                    myTempPal[tcBc1]->Caption="F1 (Dc)";
                    myTempPal[tcAd1]->Caption="G1 (Cd)";
                    myTempPal[tcBd1]->Caption="H1 (Dd)";
                    myTempPal[tcAe1]->Caption="I1 (Ce)";
                    myTempPal[tcBe1]->Caption="J1 (De)";
                    myTempPal[tcAf1]->Caption="K1 (Cf)";
                    myTempPal[tcBf1]->Caption="L1 (Df)";
                    myTempPal[tcAg1]->Caption="M1 (Cg)";
                    myTempPal[tcBg1]->Caption="N1 (Dg)";
                    myTempPal[tcAh1]->Caption="O1 (Ch)";
                    myTempPal[tcBh1]->Caption="P1 (Dh)";

                    myTempPal[tcAa2]->Caption="A2 (Aa)";
                    myTempPal[tcBa2]->Caption="B2 (Ba)";
                    myTempPal[tcAb2]->Caption="C2 (Ab)";
                    myTempPal[tcBb2]->Caption="D2 (Bb)";
                    myTempPal[tcAc2]->Caption="E2 (Ac)";
                    myTempPal[tcBc2]->Caption="F2 (Bc)";
                    myTempPal[tcAd2]->Caption="G2 (Ad)";
                    myTempPal[tcBd2]->Caption="H2 (Bd)";
                    myTempPal[tcAe2]->Caption="I2 (Ae)";
                    myTempPal[tcBe2]->Caption="J2 (Be)";
                    myTempPal[tcAf2]->Caption="K2 (Af)";
                    myTempPal[tcBf2]->Caption="L2 (Bf)";
                    myTempPal[tcAg2]->Caption="M2 (Ag)";
                    myTempPal[tcBg2]->Caption="N2 (Bg)";
                    myTempPal[tcAh2]->Caption="O2 (Ah)";
                    myTempPal[tcBh2]->Caption="P2 (Bh)";
                }
                else
                {
                    if(TestIF_File.dSiteYPitch<50.0)
                    {
                        myTempPal[tcAa1]->Caption="A1 (Ca, Da)";
                        myTempPal[tcBa1]->Caption="B1 (Cb, Db)";
                        myTempPal[tcAb1]->Caption="C1 (Cc, Dc)";
                        myTempPal[tcBb1]->Caption="D1 (Cd, Dd)";
                        myTempPal[tcAc1]->Caption="E1 (Ce, De)";
                        myTempPal[tcBc1]->Caption="F1 (Cf, Df)";
                        myTempPal[tcAd1]->Caption="G1 (Cg, Dg)";
                        myTempPal[tcBd1]->Caption="H1 (Ch, Dh)";

                        myTempPal[tcAa2]->Caption="A2 (Aa, Ba)";
                        myTempPal[tcBa2]->Caption="B2 (Ab, Bb)";
                        myTempPal[tcAb2]->Caption="C2 (Ac, Bc)";
                        myTempPal[tcBb2]->Caption="D2 (Ad, Bd)";
                        myTempPal[tcAc2]->Caption="E2 (Ae, Be)";
                        myTempPal[tcBc2]->Caption="F2 (Af, Bf)";
                        myTempPal[tcAd2]->Caption="G2 (Ag, Bg)";
                        myTempPal[tcBd2]->Caption="H2 (Ah, Bh)";
                    }
                    else
                    {
                        myTempPal[tcAa1]->Caption="A1 (Ca, Cb)";
                        myTempPal[tcBa1]->Caption="B1 (Da, Db)";
                        myTempPal[tcAb1]->Caption="C1 (Cc, Cd)";
                        myTempPal[tcBb1]->Caption="D1 (Dc, Dd)";
                        myTempPal[tcAc1]->Caption="E1 (Ce, Cf)";
                        myTempPal[tcBc1]->Caption="F1 (De, Df)";
                        myTempPal[tcAd1]->Caption="G1 (Cg, Ch)";
                        myTempPal[tcBd1]->Caption="H1 (Dg, Dh)";

                        myTempPal[tcAa2]->Caption="A2 (Aa, Ab)";
                        myTempPal[tcBa2]->Caption="B2 (Ba, Bb)";
                        myTempPal[tcAb2]->Caption="C2 (Ac, Ad)";
                        myTempPal[tcBb2]->Caption="D2 (Bc, Bd)";
                        myTempPal[tcAc2]->Caption="E2 (Ae, Af)";
                        myTempPal[tcBc2]->Caption="F2 (Be, Bf)";
                        myTempPal[tcAd2]->Caption="G2 (Ag, Ah)";
                        myTempPal[tcBd2]->Caption="H2 (Bg, Bh)";
                    }
                }
                break;
            case _32Site4X8M:
                break;
        }
    }

}

// golden uTemp_Set.cpp:1986  TfTemp_Set::ReadTempFile(bool bUpdateAll)
static void TS_ReadTempFile(bool bUpdateAll)
{

    int i;
    unsigned int iAtc;
    bool bReadType=false;
    AnsiString S="", str="";
    S=GetLastOpenFN();
    AnsiString szDir="", szDir2="";
    AnsiString str1, sTemp="";                                                  //kevin 20151023

    if(CosFunction.bSaveTemperatureByMachine &&
       IniConfig.bA57_2SaveTemperatureByMachine)                                //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s\\", DataPath, S);
    }
    MyForceDirectories(szDir);
    szDir+="Temperature.Data";

//    szDir.sprintf("%s%s\\Temperature.Data", DataPath, S);
    double  bufferfWorkTemperBase=0.0;                                          //kevin 20160531
    double  bufferffSoakTime=0.0;                                               //kevin 20160531
    int iTemperatureDataVer=0;                                                  //Sam 20210106 : TemperatureData 新增版號來相容

    if(FileExists(szDir)==false)                                                //kevin 20160531 add
    {
        iSendChangeTempError=1;
        return;
    }

    if(bUpdateAll==true)
    {
        if(CosFunction.bUseIndividulTempSet==false ||                           //Steven 20181102 : For QTI全球統一Offset
           (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60))
            Temperature.bUseIndividualTemp=false;
        else
            Temperature.bUseIndividualTemp=ReadIniData(szDir, "Mode",    "UseIndividualTemp",         false);           //Steven 20140924 : 各個加熱區獨立有自己的設定值

        if(CosFunction.bUseInitialDelayAsSoakTime)                              //Steven 20170329 (wei) : 使用initial delay當 Soak time
            Temperature.bUseInitialDelayAsSoakTime=ReadIniData(szDir, "Mode", "bUseInitialDelayAsSoakTime", false);
        else
            Temperature.bUseInitialDelayAsSoakTime=false;

        if(CosFunction.bTempAlarmBinNeedToError)                                //Steven 20251022 : Temp alarm need put to error bin
            Temperature.bTempAlarmBinNeedToError  =ReadIniData(szDir, "Mode",       "bTempAlarmBinNeedToError",  false);
        else
            Temperature.bTempAlarmBinNeedToError  =false;

    ;   // golden uTemp_Set.cpp:2037-2037 取代：fMain->palIndivisual：主畫面面板，移植樹 fMain 沒有（純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2037-2037 -- fMain->palIndivisual：主畫面面板，移植樹 fMain 沒有（純畫面）
        fMain->palIndivisual->Visible=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp);
#endif // GATE (S12-C save)
        if(LastSet.iTemperature==Tempture_Hot ||
           LastSet.iTemperature==Tempture_AmbientHot)                           //kevin 20140918 恆溫控制
    fMain->edWorkTemperBase->Visible=!(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp);   // golden uTemp_Set.cpp:2040-2040 取代：fMain->palIndivisual->Visible 就是上一句 :2037 設的值 → 等價展開
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2040-2040 -- fMain->palIndivisual->Visible 就是上一句 :2037 設的值 → 等價展開
            fMain->edWorkTemperBase->Visible=!fMain->palIndivisual->Visible;
#endif // GATE (S12-C save)

        if(IniConfig.bOnlyRoomOrHot)                                            //Steven 20110914
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 1);
        else if(IniConfig.bI03AmbientTempControl)                               //kevin 20140918 恆溫控制
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 3);
        else
            EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 2);

        Temperature.iMachineTempMode=ReadIniData(szDir, "Mode",    "Mode",         0);
        bufferfWorkTemperBase=Temperature.fWorkTemperBase ;                     //kevin 20160531

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20160614
            Temperature.fWorkTemperBase=ReadIniData(szDir, "Mode",    "Temperature",  10.0);
        else
            Temperature.fWorkTemperBase=ReadIniData(szDir, "Mode",    "Temperature",  0.0);

        if(Temperature.iMachineTempMode==0)
        {
            LastSet.iTemperature=Tempture_Hot;
        }
        else if(Temperature.iMachineTempMode==1)
        {
            LastSet.iTemperature=Tempture_Ambient;
        }
        else if(Temperature.iMachineTempMode==3)                                //Steven 20200421 : 補上Ambient Temp
        {
            LastSet.iTemperature=Tempture_AmbientHot;
        }
        else
        {
            if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)               //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
                LastSet.iTemperature=ReadIniData(szDir, "Mode",    "WorkTempMode",  LastSet.iTemperature);
        }

        if(fDynamicTemp!=NULL)
        {
            fDynamicTemp->edUpper->Text=AnsiString(Temperature.fWorkTemperBase+1);
            fDynamicTemp->edLower->Text=AnsiString(Temperature.fWorkTemperBase-1);
            fDynamicTemp->edMax  ->Text=AnsiString(Temperature.fWorkTemperBase+5);
            fDynamicTemp->edMin  ->Text=AnsiString(Temperature.fWorkTemperBase-5);
        }

        if(IniConfig.bVTESTFunction==false)                                     //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
            Temperature.bUseAbitCHK      =CheckAndReadIniData(szDir, "Mode",    "AmbCheck",     0.0);
    //    Temperature.iAbitCHKStatus   =ReadIniData(szDir, "Ambient", "Check",        0.0);
        Temperature.iAbitCHKStatus   = 1;
        if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                    //Steven 20230410 : 南茂鍾永生要求修改
            Temperature.fAbitTemp    =CheckRange(CheckAndReadIniData(szDir, "Ambient", "Temperature",  23.0), 23.0, 30.0);                                      //Sam 20240424 : 修改南茂鍾永生要求修改 25 > 23
        else
            Temperature.fAbitTemp    =CheckRange(CheckAndReadIniData(szDir, "Ambient", "Temperature",  25.0), 50.0, 10.0);                                      //Jimmychiu 20220901 預設常溫為25

        if(LastSet.iTemperature==Tempture_Ambient &&
           IniConfig.bL20AbientGuardBand)                                       //kevin 20180115 (Steven) add Amient Guard Band
        {
        }
        else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                               //Sam 20240424 : 修改南茂鍾永生要求修改 25 > 23
        {
            if(Temperature.fAbitTemp<23 || Temperature.fAbitTemp>40)
                Temperature.fAbitTemp=0;                                        //強制設定為0度請人員確認
        }
        else if(Temperature.fAbitTemp<25 || Temperature.fAbitTemp>40)           //Ifor 20170605 (wei) add 常溫溫度讀取卡控避免人員修改檔案造成異常
        {
            Temperature.fAbitTemp=0;                                            //強制設定為0度請人員確認
        }

        if(DewPoint_Hardware_Install>0)                                         //Steven 20191017 : 露點計
        {
            Temperature.bWaitDewPoint         =ReadIniData(szDir, "Mode",    "WaitDewPoint",              false);       //Isaac 20180828 : wait dewpoint
            Temperature.dDewPointRange        =CheckRange(ReadIniData(szDir ,"Mode" ,"DewPointRange" ,10.0), 1.0, 30.0);                                        //Isaac 20181217 : 露點計報警範圍可設定
            Temperature.iDewPointAlarmInterval=CheckRange(ReadIniData(szDir ,"Mode" ,"DewPointAlarmInterval" ,10), 1, 30);                                      //Isaac 20181217 : 露點計報警範圍可設定
        }
        else
        {
            Temperature.bWaitDewPoint=false;
        }

        if(LB_TEMP>0)                                                           //JerryYang 20220126: non ATC也支援LB function
        {
            Temperature.bLBCoolingAirOn         =ReadIniData(szDir, "Mode",    "bLBCoolingAirOn",              false);  //Isaac 20180828 : wait dewpoint
            Temperature.dLBAirOnTemp            =CheckRange(ReadIniData(szDir ,"Mode" ,"dLBAirOnTemp" ,55.0), 40.0, 100.0);                                     //Isaac 20181217 : 露點計報警範圍可設定
            Temperature.iLBTempAlmInterval      =CheckRange(ReadIniData(szDir ,"Mode" ,"iLBTempAlmInterval" ,30), 30, 300);                                     //Isaac 20181217 : 露點計報警範圍可設定
        }
        else
        {
            Temperature.bLBCoolingAirOn=false;
        }

        Temperature.fAmbientHotGuartbent = CheckRange(ReadIniData(szDir, "Ambient", "fAmbientHotGuartbent",  3.0), 1.0, 30.0);                                  //JerryYang 20181001 (Steven) : 20->30   //kevin 20180906 add 10->20 //kevin 20141219 恆溫控制
        if(IniConfig.bI03AmbientTempControl && LastSet.iTemperature==Tempture_AmbientHot)
        {
            Temperature.bShuttleNoHeatUp=ReadIniData(szDir, "Ambient",    "bShuttleNoHeatUp", false);                   //Steven 20180815 : Amb Ctr mode, shuttle no heat up
            Temperature.bSLKNoHeatUp    =ReadIniData(szDir, "Ambient",    "bSLKNoHeatUp", false);;                      //Steven 20230221 : Amb Ctr mode, SLK no heat up
        }
        else
        {
            Temperature.bShuttleNoHeatUp=false;
            Temperature.bSLKNoHeatUp    =false;                                 //Steven 20230221 : Amb Ctr mode, SLK no heat up
        }

        bufferffSoakTime=Temperature.fSoakTime;                                 //kevin 20160531
        Temperature.fSoakTime        =CheckAndReadIniData(szDir, "Time",    "Soak",         999.0);                     //kevin 20160614 0.0->999
        Temperature.fJamSoakTime     =CheckAndReadIniData(szDir, "Time",    "Jam Soak",     0.0);
        Temperature.fInitialWaitTime =CheckAndReadIniData(szDir, "Time",    "H.Initial",    0.0);
        Temperature.fAbitInitWaitTime=CheckAndReadIniData(szDir, "Time",    "A.Initial",    0.0);
        Temperature.fAbitColdTime    =CheckAndReadIniData(szDir, "Time",    "Cool Time",    0.0);
        Temperature.iIndexHeatMode   =CheckAndReadIniData(szDir, "Index",   "Heating Mode", HeadOnly);                  //kevin 20160614 檔案寫入參數名稱

        if(CUSTOMER_CODE!=CC_HONPREC_QC)                                        //JerryYang 20160810 搬到讀取完iIndexHeatMode之後,避免Chamber加熱超過175度
        {
            dTempMax=MaxTempSetting();                                          //Steven 20170427 : 回傳機台可以用的最大溫度值
            dTempMin=MinTempSetting();
            if(bSendChangeTemp==false)                                          //kevin 20160531 起動auto temp offset 成功
                Temperature.fWorkTemperBase=CheckRange(Temperature.fWorkTemperBase, dTempMax, dTempMin);                //Steven 20110412

            if(ATC_SYSTEM==eNewATCSystem &&
    (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36))   // golden uTemp_Set.cpp:2156-2156 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2156-2156 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
               ATC_InterfaceForm->IS_ATC33())
#endif // GATE (S12-C save)
            {
                IniConfig.dATCAmbientTemperature=CheckRange(IniConfig.dATCAmbientTemperature, 30.0, -5.0);
            }
            else
            {
                IniConfig.dATCAmbientTemperature=CheckRange(IniConfig.dATCAmbientTemperature, 30.0, 25.0);              //wei 20151013  by Setup File ATC Ambient Temp set
            }
        }
        IniConfig.dHeatGunTempATC=CheckRange(IniConfig.dHeatGunTempATC, 70.0, 30.0);                                    //JerryYang 20220408 : add for ATC3.5

        //Ifor 20160411 客戶要求強制開啟
        if(IniConfig.bSPILFunction==true ||                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
           IniConfig.bAmbRunChamberFanCanStop==false)                           //Steven 20250806 : add protection
            Temperature.bAmbUsingAFan=true;
        else
            Temperature.bAmbUsingAFan=CheckAndReadIniData(szDir, "Ambient", "bAmbUsingAFan", true);                     //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動

        if(IniConfig.bL20AbientGuardBand)                                       //kevin 20180115 (Steven) add Amient Guard Band
        {
            Temperature.bAmbientGuardbandCheck=CheckAndReadIniData(szDir, "Ambient", "bAmbientGuardbandCheck", true);
            Temperature.iAmbGuardband=CheckAndReadIniData(szDir, "Ambient", "iAmbGuardband", 1);                        //JerryYang 20190520 false->1, fix AmbGuardband參數讀取異常問題
        }
        else
        {
            Temperature.bAmbientGuardbandCheck=false;
            Temperature.iAmbGuardband=0;
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE ||                                        //Ifor 20160318 add 客戶要求強制打開 常溫確認
           CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //KEVIN 20190529 Telix 要求有mail
        {
            Temperature.bUseAbitCHK=true;
        }

        if(Temperature.iIndexHeatMode<EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Items->Count)            //Steven 20240206 : 根據Temperature.iIndexHeatMode顯示狀態決定是否要調整Index Heat Mode
        {
    if(TS_IndexHeatModeItemVisible(Temperature.iIndexHeatMode)==false)   // golden uTemp_Set.cpp:2193-2193 取代：rgIndexHeatMode->Controls[i]->Visible：vclcompat 沒有 Controls[]；golden 唯一設定點 cprod.cpp:3818-3823 的公式（TS_IndexHeatModeItemVisible）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2193-2193 -- rgIndexHeatMode->Controls[i]->Visible：vclcompat 沒有 Controls[]；golden 唯一設定點 cprod.cpp:3818-3823 的公式（TS_IndexHeatModeItemVisible）
            if(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Controls[Temperature.iIndexHeatMode]->Visible==false)
#endif // GATE (S12-C save)
                Temperature.iIndexHeatMode=HeadOnly;
        }
        else
        {
            Temperature.iIndexHeatMode=HeadOnly;
        }

        //jou 981030 start
        Temperature.iInitialStart1Time=ReadIniData(szDir, "Time", "iInitialStart1Time", 0.0);
        Temperature.iInitialStart2Time=ReadIniData(szDir, "Time", "iInitialStart2Time", 0.0);
        //jou 981030 end
        Temperature.iIndexSoakTime=ReadIniData(szDir, "Time", "iIndexSoakTime", 0.0);                                   //2013-11-27   Dell    需要做Index soak time
        Temperature.iOSTime=ReadIniData(szDir, "Time", "iOSTime", 0.0);         //2013-11-27   Dell    需要做Index soak time
        if(CUSTOMER_CODE == CC_ASE_KaohSiung)
        {
            Temperature.iIndexSoakTime = 0;
            Temperature.iOSTime = 0;
        }                                                                       //kevin 20210511 ASE KH  JL 說關閉
        Temperature.iShuttleSoakTimeMode=ReadIniData(szDir, "Time", "In Shuttle Soak Time Mode", 1);                    //jou 2012-06-29 Index Pick up need wait Soak Time
        Temperature.bZ2DownSocket=ReadIniData(szDir, "Time", "bZ2DownSocket",0);                                        //kevin 20131211

        if(CosFunction.bOffsetTempByRecipeMinMaxLimit)                          //StevenHong 20260119 : Add Offset By Recipe Max Limit
        {
            Temperature.iOffsetByRecipeMaxLimit = ReadWriteIni(szDir, "User OffSet", "MaxLimit", Temperature.iOffsetByRecipeMaxLimit, 60,  bReadFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
            Temperature.iOffsetByRecipeMinLimit = ReadWriteIni(szDir, "User OffSet", "MinLimit", Temperature.iOffsetByRecipeMinLimit, -60, bReadFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
        }
        for(i=0; i<tcTotalCount; i++)
        {
            S.printf("CH%d", i+1);
            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&bSendChangeTemp)               //kevin 20161011 ASE調整溫度過大 需設最大值
            {
                dTempOffset[0][i]=ReadIniData(szDir, "User OffSet", S, 0.1);
            }
            else
            {
                if(IniConfig.bL49UseBySetupOffsetRange==true)                   //Ifor 20231219 add Use By Setup Offset Range
                    dTempOffset[0][i]=ReadWriteIni(szDir, "User OffSet", S, dTempOffset[0][i], 0.1, bReadFile, true, (double)IniConfig.iL49UseBySetupOffsetRange, (double)-IniConfig.iL49UseBySetupOffsetRange);    //Steven 20160906 : 溫度讀檔保護
                else
                    dTempOffset[0][i]=ReadWriteIni(szDir, "User OffSet", S, dTempOffset[0][i], 0.1, bReadFile, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);  //Steven 20160906 : 溫度讀檔保護
            }

            if(CosFunction.bTemp5PointKitOffset)                                //JerryYang 20221003 Analog要求新增Kit三點offset
            {
                S.printf("CH%d", i+1);
                Temperature.fTempOffSet[KitLowBase][i]      =ReadIniData(szDir, "Kit Low OffSet",  S, 0.0);
                Temperature.fTempOffSet[KitMidBase][i]      =ReadIniData(szDir, "Kit Mid. OffSet", S, 0.0);
                Temperature.fTempOffSet[KitHigBase][i]      =ReadIniData(szDir, "Kit High OffSet", S, 0.0);
                Temperature.fTempOffSet[KitAmbientHotLow][i]=ReadIniData(szDir, "Kit AmbientHotLowOffSet", S, 0.0);     //kevin 20141006
                Temperature.fTempOffSet[KitAmbientHotMid][i]=ReadIniData(szDir, "Kit AmbientHotMidOffSet", S, 0.0);     //kevin 20141006
            }

            if(CosFunction.bHiSiliconFunction)                                  //kevin 20191227 add
            {
               if(dTempOffset[0][i]>=30)
                    dTempOffset[0][i]=30;
            }

            if(bSendChangeTemp)                                                 //起動auto temp offset
            {
                if(abs(dTempOffset[0][i]-dTempOffset[1][i])<=10 &&              //kevin 20151204 生產中offset 溫度<=10 度 才可以改變溫度
                  (dTempOffset[0][i]<=InputLimit.iTempHigh &&
                   dTempOffset[0][i]>=InputLimit.iTempLow))                     //kevin 20161011
                {
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                }
                else
                {
                    if(dTempOffset[0][i]>=InputLimit.iTempHigh)
                    {
                        dTempOffset[1][i]= InputLimit.iTempHigh;
                        str1.printf("ASE_Tmperature_offset_OverHigh: %d", dTempOffset[0][i]);
                        RecordProcess(str1);                                    //kevin 20161011
                    }
                    else if(dTempOffset[0][i]<=InputLimit.iTempLow)             //kevin 20161011 //kevin 20151204 生產中offset 溫度<=10 度 才可以改變溫度
                    {
                        dTempOffset[1][i]=InputLimit.iTempLow;
                        str1.printf("ASE_Tmperature_offset_OverLow: %d", dTempOffset[0][i]);
                        RecordProcess(str1);                                    //kevin 20161011
                    }
                    str1="";
                    str1.printf("CH%d", i+1);
                    WriteIniData(szDir, "User OffSet", str1, dTempOffset[1][i]);
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[1][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                    bSendChangeTempError=true;
                }
            }
            else
            {
                dTempOffset[1][i]=dTempOffset[0][i];
                Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];       //kevin 20151023 紀錄讀取溫度offset 0:最新資料
            }

            if(ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                             //Steven 20181102 : For QTI全球統一Offset
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "Cal Temp", S, 0.0);
            else if(ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61)
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "IndividualTempSetting", S, 40.0);
            else
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "IndividualTempSetting", S, 30.0);                    //Steven 20140924 : 各個加熱區獨立有自己的設定值

            tATCData.dOriginalUseOffset[i]=Temperature.fTempOffSet[UserOffSet][i];                                      //2014-05-30    Dell    for ATC6.0

            if(CosFunction.bInitTempOffsetByWorkFile==true)                     //jou 2015-06-13 Initial Temperture Offset by WorkFile
                Temperature.fTempOffSet[InitTempOffset][i]  =ReadIniData(szDir, "Init Temp OffSet", S, 0.0);            //Steven 201    41117 : 起測時溫度要補Offset
        }

        if(bSendChangeTemp)                                                     //kevin 20160119 起動auto temp offset 成功
        {
            if(bSendChangeTempError)
            {                                                                   //溫度異常   恢復
                for(i=0; i<tcTotalCount; i++)
                {
                    str1.printf("CH%d", i+1);
                    WriteIniData(szDir, "User OffSet", str1, dTempOffset[1][i]);
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[1][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                }
            }
            else
            {
                for(i=0; i<tcTotalCount; i++)                                   //ok  sample 更新資料
                {
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                    dTempOffset[1][i]=dTempOffset[0][i];                        //kevin 20160119    sample
                }
            }

            if(Temperature.fWorkTemperBase==10)                                 //溫度資料異常
            {
                iSendChangeTempError=2;
                Temperature.fWorkTemperBase=bufferfWorkTemperBase;              //kevin 20160614
                if(bSendChangeTemp==false)
                {
                    ShowErrorMessage("WAR16107", K_RETRY, MMSystem);            //20160531
                }
            }

            if(Temperature.fSoakTime==999)
            {
                Temperature.fSoakTime=bufferffSoakTime;                         //kevin 20160614
                iSendChangeTempError=3;
                if(bSendChangeTemp==false)
                {
                    ShowErrorMessage("WAR16108", K_RETRY, MMSystem);            //20160531
                }
            }
        }

        bSendChangeTemp=false;                                                  //kevin 20151023
        Temperature.bActiveHeatGun  =ReadIniData(szDir, "Mode", "Active_Heat_Gun", false);                              //Steven 20120525 : 熱風槍
        if(INSTALL_HEAT_GUN>1)
        {
            Temperature.bUseCDAOnly =ReadIniData(szDir, "Mode", "Use CDA Only", false);;                                //Steven 20181012 : 使用熱風槍吹冷風
            if(Temperature.bUseCDAOnly)
            {
                Temperature.bActiveHeatGun=false;
            }
        }
        else
        {
            Temperature.bUseCDAOnly =false;
        }
        Temperature.iHotGunFLowLimit_H  =ReadIniData(szDir, "Mode", "iHotGunFLowLimit_H", 0);                           //KaiChen 20190729 ：Hot Gun Flow
        Temperature.iHotGunFLowLimit_L  =ReadIniData(szDir, "Mode", "iHotGunFLowLimit_L", 0);                           //KaiChen 20190729 ：Hot Gun Flow

        Temperature.bUseFixTemp     =ReadIniData(szDir, "DUT Setting",  "bUseFixTemp",    false);                       //Steven 20131025 : 使4固定的DUT溫度
        Temperature.dFixedTemp      =ReadIniData(szDir, "DUT Setting",  "dFixedTemp",     40.0);                        //Steven 20131025 : 固定的DUT溫度值
        Temperature.bShowFixedTemp  =ReadIniData(szDir, "DUT Setting",  "bShowFixedTemp", false);                       //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值
        Temperature.bActiveATCHeatGun  =ReadIniData(szDir, "Mode", "Active_ATC_Heat_Gun", false);                       //JerryYang 20220408 : add for ATC3.5

        if(CosFunction.bTempCalByRecipe==true)                                  //jou 20220725 : Temperature calibration by recipe
            Temperature.bTempCalByRecipe=ReadIniData(szDir, "Mode", "bTempCalByRecipe", false);
    }

    if(Temperature.iMachineTempMode==0)
    {
        LastSet.iTemperature=Tempture_Hot;
    }
    else if(Temperature.iMachineTempMode==1)
    {
        LastSet.iTemperature=Tempture_Ambient;
    }
    else if(Temperature.iMachineTempMode==3)                                    //Steven 20200421 : 補上Ambient Temp
    {
        LastSet.iTemperature=Tempture_AmbientHot;
    }
    else
    {
        if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)                   //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
            LastSet.iTemperature=ReadIniData(szDir, "Mode",    "WorkTempMode",  LastSet.iTemperature);
    }

    if(ATC_SYSTEM==eNewATCSystem &&                                             //Steven 20180817 : Boost Function
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
    {
        Temperature.bBoostFuncttion                     =CheckAndReadIniData(szDir, "Boost Function", "bBoostFuncttion",        false);
        Temperature.bLBTempFunction                     =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempFunction",        false);
    }
    else
    {
        Temperature.bBoostFuncttion=false;
        if(LB_TEMP==true && CosFunction.bNonATCSupportLBFunction==true)         //JerryYang 20220126: non ATC也支援LB function
        {
            Temperature.bLBTempFunction                     =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempFunction",        false);
        }
        else
        {
            Temperature.bLBTempFunction=false;
        }
    }
    Temperature.bEnableBoostOffset[Temperature.eBMax]   =Temperature.bBoostFuncttion;
    Temperature.dBoostIdleTime[Temperature.eBMax]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostIdleTime[0]",      15.0);
    Temperature.dBoostOffset[Temperature.eBMax]         =CheckAndReadIniData(szDir, "Boost Function", "dBoostOffset[0]",        0.0);
    Temperature.dBoostDuration[Temperature.eBMax]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostDuration[0]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBMax]   =CheckAndReadIniData(szDir, "Boost Function", "dPostBoostDuration[0]",  0.0);
    Temperature.bEnableBoostOffset[Temperature.eBMid]   =Temperature.bBoostFuncttion;
//    Temperature.dBoostIdleTime[Temperature.eBMid]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostIdleTime[1]",      10.0);
//    Temperature.dBoostOffset[Temperature.eBMid]         =CheckAndReadIniData(szDir, "Boost Function", "dBoostOffset[1]",        0.0);
//    Temperature.dBoostDuration[Temperature.eBMid]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostDuration[1]",      0.0);
//    Temperature.dPostBoostDuration[Temperature.eBMid]   =CheckAndReadIniData(szDir, "Boost Function", "dPostBoostDuration[1]",  0.0);
    Temperature.bEnableBoostOffset[Temperature.eBMin]   =Temperature.bBoostFuncttion;
    Temperature.dBoostIdleTime[Temperature.eBMin]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostIdleTime[2]",      5.0);
    Temperature.dBoostOffset[Temperature.eBMin]         =CheckAndReadIniData(szDir, "Boost Function", "dBoostOffset[2]",        0.0);
    Temperature.dBoostDuration[Temperature.eBMin]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostDuration[2]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBMin]   =CheckAndReadIniData(szDir, "Boost Function", "dPostBoostDuration[2]",  0.0);

//    Temperature.bEnableBoostOffset[Temperature.eBLBI]   =CheckAndReadIniData(szDir, "LB Temp Function", "bEnableBoostOffset[3]",  false);
    Temperature.bEnableBoostOffset[Temperature.eBLBI]   =true;
    Temperature.dBoostIdleTime[Temperature.eBLBI]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[3]",      0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostOffset[Temperature.eBLBI]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[3]",        0.0);
    Temperature.dBoostDuration[Temperature.eBLBI]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostDuration[3]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBLBI]   =CheckAndReadIniData(szDir, "LB Temp Function", "dPostBoostDuration[3]",  0.0);

    Temperature.bEnableBoostOffset[Temperature.eBLBL]   =Temperature.bLBTempFunction;
    Temperature.dBoostIdleTime[Temperature.eBLBL]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[4]",      0.0);
    Temperature.dBoostOffset[Temperature.eBLBL]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[4]",        0.0);
    Temperature.dThreshold                              =CheckAndReadIniData(szDir, "LB Temp Function", "Threshold",              0.0);                         //Steven 20190928 : L/B升溫的門檻值
    Temperature.dBoostDuration[Temperature.eBLBL]       =0.0;
    Temperature.dPostBoostDuration[Temperature.eBLBL]   =0.0;

    Temperature.bEnableBoostOffset[Temperature.eBLBB]   =CheckAndReadIniData(szDir, "LB Temp Function", "bEnableBoostOffset[5]",  false);
    Temperature.dBoostIdleTime[Temperature.eBLBB]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[5]",      0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostOffset[Temperature.eBLBB]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[5]",        0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostDuration[Temperature.eBLBB]       =10.0;
    Temperature.dPostBoostDuration[Temperature.eBLBB]   =0.0;
    Temperature.dBoostTimeOut                           =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostTimeOut",          1200.0);                      //Steven 20181222 : Add LB升溫的Time out

    Temperature.iBoostFunctionMode      =CheckAndReadIniData(szDir, "Boost Function", "iBoostFunctionMode",  0);

    Temperature.dLBTempHighSettingValue                          =CheckAndReadIniData(szDir, "LB Temp Function", "LB_Temp_High_Setting",            0.0);       //Frank 20241231 : add
    Temperature.dLBTempLowSettingValue                           =CheckAndReadIniData(szDir, "LB Temp Function", "LB_Temp_Low_Setting",             0.0);
    Temperature.bLBTempHighAlarm_Enable                          =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempHighAlarm_Enable",         false);     //Frank 20241231 : add
    Temperature.bLBTempLowAlarm_Enable                           =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempLowAlarm_Enable",          false);

    if(Temperature.dBoostIdleTime[Temperature.eBMin]>Temperature.dBoostIdleTime[Temperature.eBMax])
        Temperature.dBoostIdleTime[Temperature.eBMin]=Temperature.dBoostIdleTime[Temperature.eBMax];

    if(LastSet.iTemperature==Tempture_Ambient)
        Temperature.bBoostFuncttion=false;
    //<==
    //Steven 20180817 : Boost Function

    if(Temperature.bBoostFuncttion==false)                                      //Steven 20180817 : Boost Function啟用時, initial offset就關閉
        Temperature.bEnableTempOffsetForInitial         =ReadIniData(szDir, "InitialMode",       "EnableTemperatureOffsetforInitial",         false);           //Steven 20141117 : 起測時溫度要補Offset
    else
        Temperature.bEnableTempOffsetForInitial         =false;

    if(CosFunction.bUseChamberBoostMode)                                        //Steven 20191128 : Chamber Boost Function
    {
        Temperature.bEnableChamberBoost                 =ReadIniData(szDir, "ChamberBoostMode",       "bEnableChamberBoost",        false);
        Temperature.iChamberBoostTime                   =CheckRange(ReadIniData(szDir, "ChamberBoostMode",       "iChamberBoostTime",          10), 1, 30);
        Temperature.iChamberBoostOffset                 =CheckRange(ReadIniData(szDir, "ChamberBoostMode",       "iChamberBoostOffset",        10), 0, 30);
        fLotInfo->tsChamberBoost->TabVisible            =Temperature.bEnableChamberBoost;
    }
    else
    {
        Temperature.bEnableChamberBoost     =false;
        fLotInfo->tsChamberBoost->TabVisible=false;
    }

    if((LastSet.iTemperature==Tempture_Hot ||
        LastSet.iTemperature==Tempture_AmbientHot ||
        CUSTOMER_CODE==CC_KYEC_LEE) &&
       Temperature.bEnableTempOffsetForInitial)                                 //Steven 20180820 : 簡化init temp offset判斷式
    {
        bUseInitTempOffset=true;
    }
    else
    {
        bUseInitTempOffset=false;
    }

    if(LastSet.iTemperature==Tempture_Hot ||
       LastSet.iTemperature==Tempture_AmbientHot ||
       CUSTOMER_CODE==CC_KYEC_LEE)                                              //Steven 20180820 : 簡化init temp offset判斷式
    {
        bUseInitDelay=true;
    }
    else
    {
        bUseInitDelay=false;
    }

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing && fContact->fShow)                     //Sam 20240118 : contact 介面下關閉 InitTempOffset 加熱功能
    {
        bUseInitTempOffset=false;
    }

    Temperature.iCintactCntForTempOffsetAtInitial       =ReadIniData(szDir, "InitialMode",       "iCintactCntForTempOffsetAtInitial",         1);               //Steven 20141117 : 起測時溫度要補Offset
    Temperature.iCintactDelayCntForInitTempOffset       =ReadIniData(szDir, "InitialMode",       "iCintactDelayCntForInitTempOffset",         10);              //Steven 20141117 : 起測時溫度要補Offset

    if(IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false)   //Sam 20231214 : Temp offset use ready temp range
        Temperature.iTempReadyRange                         =CheckRange(ReadIniData(szDir, "InitialMode", "iTempReadyRange", 0), 10, 0);
    else
        Temperature.iTempReadyRange                         =0;

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160621 海思版本PID 設定Function 強制設為false
        Temperature.bUsePIDControl=false;
    else
        Temperature.bUsePIDControl                      =ReadIniData(szDir, "InitialMode",       "UsePIDControl",                             false);           //Ifor 20150910 : ATC PID Control

    for(int i=0; i<3; i++)
    {
        if(CosFunction.bHiSiliconFunction==true)                                //wei 20160506 ID_Min ID_Max 強制為0
        {                                                                       //Ifor 20160621 海思版本不讀取設定強制為0
            Temperature.iATC_PID_Min_Offset[i]          =0;
            Temperature.iATC_PID_Max_Offset[i]          =0;
        }
        else
        {
            Temperature.iATC_PID_Min_Offset[i]          =ReadIniData(szDir, "InitialMode",       "iATC_PID_Min_Offset_"+ AnsiString(i),        1000);           //Ifor 20150910 : ATC PID Min Offset
            Temperature.iATC_PID_Max_Offset[i]          =ReadIniData(szDir, "InitialMode",       "iATC_PID_Max_Offset_"+ AnsiString(i),        2000);           //Ifor 20150910 : ATC PID Max Offset
        }
    }
    Temperature.b3SigmaTempMonitior_Enable              =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_Enable",                             false);      //kevin 20200521 add 3 Sigma 溫度記錄比對
    Temperature.i3SigmaTempMonitior_SetCoun             =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_SetCount",                             0);        //kevin 20200521 add 3 Sigma 溫度記錄比對          //kevin 20200521 add 3 Sigma Index Contract time
    Temperature.d3SigmaTempMonitior_Set3xSigmaValue     =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_Set3xSigmaValue",                      0.0);      //kevin 20200521 add 3 Sigma 溫度記錄比對  //kevin 20200521 add 3 Sigma 客戶比對誤差值

    Temperature.bEnableTJFunction                       =ReadIniData(szDir, "ATC",       "bEnableTJFunction",             false);                               //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    Temperature.iTjSiteMapType                          =ReadIniData(szDir, "ATC",       "iTjSiteMapType",                0);                                   //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    Temperature.fSetTJ_Offset                           =ReadIniData(szDir, "ATC",       "dTjOffset",                     0.0);
    Temperature.fSetTJ_Slope                            =ReadIniData(szDir, "ATC",       "dTjSlope",                      1.0);

    Temperature.fTjInputVLow                            =ReadIniData(szDir, "ATC",       "dTjInputVLow",                  0.00);
    Temperature.fTjInputVHigh                           =ReadIniData(szDir, "ATC",       "dTjInputVHigh",                 10.0);

    Temperature.dDelayAfterSOT                          =ReadIniData(szDir, "ATC",       "dDelayAfterSOT",                0.0);
    Temperature.dDelayAfterSOTContinue                  =ReadIniData(szDir, "ATC",       "dDelayAfterSOTContinue",                0.0);                         //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    Temperature.bTjWatchdog                             =ReadIniData(szDir, "ATC",       "bTjWatchdog",                   false);                               //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    sATC_CH_Tj->CommaText=ReadIniData(szDir, "ATC",       "sATC_CH_Tj",                   AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));                       //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    for(int i=0; i<16; i++)
    {
        Temperature.iATCCH_Tj[i]=atoi(sATC_CH_Tj->Strings[i].c_str());
    }

    if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                            //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    {
        if(ATC_SYSTEM==eATCSiliconType)
        {
            Temperature.bATCTemperatureSet  =ReadIniData(szDir, "ATC", "Temperature Set", false);
        }
        else if(ATC_SYSTEM==eATCHonPrecType)
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                Temperature.iATCChillerTemp=15;                                 //Ifor 20161103 ATC 2.0 Chiller 溫度寫死20度   //JerryYang 20161128 守志說矽品SPEC固定15度
            else
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", 15), 40, 5);           //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
        }
        else if(ATC_SYSTEM==eNewATCSystem)
        {
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -20);        //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }
            else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36)   //JerryYang 20220408 : add for ATC3.5
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -40);        //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }
            else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61)
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -55);
            }
            else
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", 15), 40, 5);           //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }

            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {
            }
            else
            {
                if(CosFunction.bUseHandlerSetChillerTemp==true ||               //Ifor 20161103 (Jou)矽品要求依據工作擋修改Chiller溫度
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)
                {
                }
                else
                {
                    if(Temperature.iATCChillerTemp<15)
                    {
                        if(LastSet.iTemperature==Tempture_Hot)
                        {
                            if(Temperature.fWorkTemperBase<=25)
                                Temperature.iATCChillerTemp=15;
                            else if(Temperature.fWorkTemperBase<=80)
                                Temperature.iATCChillerTemp=20;
                            else
                                Temperature.iATCChillerTemp=25;
                        }
                        else
                        {
                            Temperature.iATCChillerTemp=15;
                        }
                    }
                }
            }
            EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text=IntToStr(Temperature.iATCChillerTemp);
            WriteIniData(szDir, "ATC", "Chiller Temp", EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text);
        }

        Temperature.bATCHandlerStart        =ReadIniData(szDir, "ATC", "Handler Start"  , true);
        Temperature.bATCTestStrat           =ReadIniData(szDir, "ATC", "Test Strat"     , true);
        Temperature.dATCSP2                 =CheckRange(ReadIniData(szDir, "ATC", "SP2", 130.0), 135.0, 20.0);
        Temperature.bATCActiveCooling       =ReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.dATCInPC[0]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[0]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[1]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[1]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[2]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[2]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[3]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[3]", 0.0), 30.0, -30.0);

        if(CUSTOMER_CODE==CC_SCS)                                               //Ifor 20160711 避免SCS要求ATC7.0頁面關閉造成異常問題
            Temperature.bATC70Active        =false;
        else
            Temperature.bATC70Active        =ReadIniData(szDir, "ATC", "ATC7", false);                                  //Eliot 2015_0105

        Temperature.bATC7ChannelEnabled[0]  =ReadIniData(szDir, "ATC", "ATC7CH1Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[1]  =ReadIniData(szDir, "ATC", "ATC7CH2Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[2]  =ReadIniData(szDir, "ATC", "ATC7CH3Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[3]  =ReadIniData(szDir, "ATC", "ATC7CH4Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7TSDFunction        =ReadIniData(szDir, "ATC", "TSDFunction",   false);                         //Eliot 2015_0105
        Temperature.dATC7TSDTimeOut         =ReadIniData(szDir, "ATC", "TSDTimeOut",    0.0);                           //Steven 20160604 : by site TSD
        Temperature.ATCInitialOffset[0]     =ReadIniData(szDir, "ATC", "Initial_0", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[1]     =ReadIniData(szDir, "ATC", "Initial_1", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[2]     =ReadIniData(szDir, "ATC", "Initial_2", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[3]     =ReadIniData(szDir, "ATC", "Initial_3", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC

        Temperature.dATCConFailOffset[0]    =ReadIniData(szDir, "ATC", "ConsFail_0", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[1]    =ReadIniData(szDir, "ATC", "ConsFail_1", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[2]    =ReadIniData(szDir, "ATC", "ConsFail_2", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[3]    =ReadIniData(szDir, "ATC", "ConsFail_3", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.bEnableATCConFailOffset =ReadIniData(szDir, "ATC", "bEnableATCConFailOffset", false);               //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.iATCConFailOffsetCount  =ReadIniData(szDir, "ATC", "iATCConFailOffsetCount",  0);                   //Steven 20151123 : Continue Fail Temp Offset for ATC

        Temperature.dATCQAModeOffset[0]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset1", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[1]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset2", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[2]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset3", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[3]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset4", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.bEnableATCQAModeOffset  =ReadIniData(szDir, "ATC", "bEnableATCQAModeOffset", false);                //Steven 20151125 : QA Mode Temp Offset for ATC

        Temperature.bEnableATCTestTimeOffset=ReadIniData(szDir, "ATC", "bEnableATCTestTimeOffset", false);              //Steven 20160216 : 測試時間太短也要Offset
        Temperature.iATCTestTimeOffsetTime  =ReadIniData(szDir, "ATC", "iATCTestTimeOffsetTime", 15);                   //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[0]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset1", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[1]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset2", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[2]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset3", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[3]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset4", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset

        Temperature.iATC_OFS_ST            =ReadIniData(szDir, "ATC", "ATC_OFS_ST",   0.0);                             //Ifor 20241118 : 測試中變溫

        for(int i=0; i<32; i++)                                                 //Ifor 20160419 add ATC 32 Site Offset
        {
            str1.sprintf("ATCTempOffset[%d]", i);
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20180215 (Steven) ADD atc
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 60.0, -60.0);
            else
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
            if(CosFunction.bUseSecondATCTempOffset)                             //Ifor 20241118 : 測試中變溫
            {
                str1.sprintf("ATCSecondTempOffset[%d]", i);
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20180215 (Steven) ADD atc
                    Temperature.dATCSecondTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 60.0, -60.0);
                else
                    Temperature.dATCSecondTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
            }
        }

        Temperature.bUseReferTempSensor=CheckAndReadIniData(szDir, "ATC", "UseReferTempSensor", (Temperature.bATCActiveCooling));                               //wei 20160506 第二點溫度預設修改為true //Steven 20251003 : 預設值根據ATC有沒有開來決定
        Temperature.bUseTC2Offset           =CheckAndReadIniData(szDir, "ATC", "UseTC2Offset",       false);            //KenHsieh 20240311 : add Tc2 Offset

        for(int i=0; i<3; i++)                                                  //Ifor 20190306 : add Package Offset 三點校正
        {
            str1.sprintf("ATCPackageOffset[%d]", i);
            Temperature.dATCPackageOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 20.0, -20.0);

            str1.sprintf("ATCPackagetemp[%d]", i);
            if(ATC_SYSTEM==eNewATCSystem &&
    (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36))   // golden uTemp_Set.cpp:2690-2690 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2690-2690 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
               ATC_InterfaceForm->IS_ATC33())
#endif // GATE (S12-C save)
            {
                Temperature.dATCPackageTemp[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 150.0, -5.0);
            }
            else
            {
                Temperature.dATCPackageTemp[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 150.0, 20.0);
            }
        }

        for(int i=0; i<2; i++)                                                  //Ifor 20190730 : add FFC Temperature Offset
        {
            for(int j=0; j<10; j++)
            {
                str1.sprintf("iArm%d_FFCTimeOn[%d]", i+1, j);
                Temperature.iATC_FFCTimeOn[i][j]   =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCTimeOff[%d]", i+1, j);
                Temperature.iATC_FFCTimeOff[i][j]  =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCOffset[%d]", i+1, j);
                Temperature.dATC_FFCOffset[i][j]   =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCPointUse[%d]", i+1, j);
                Temperature.bATC_FFCPointUse[i][j] =ReadIniData(szDir, "ATC", str1, false);
            }
        }
        Temperature.bATC_FFCEnable =ReadIniData(szDir, "ATC", "bATC_FFCEnable", false);

        Temperature.bMultiZoneEnable=ReadIniData(szDir, "ATC", "Multi Zone Enable", false);                             //wei 20240617 Multi Zone
        for(int i=0; i<4; i++)
        {
            str1.sprintf("Zone Temp %d Use", i+1);
            Temperature.bZoneTempEnable[i]=ReadIniData(szDir, "ATC", str1, false);

            str1.sprintf("Zone Temp %d Setting", i+1);
            Temperature.dZoneTempSetting[i]=ReadIniData(szDir, "ATC", str1, 0.0);
        }

        fLotInfo->ShowATCTempPanel();                                           //Steven 20241112 : 調整ATC溫度顯示

        InitialHeaterDoor();

        if(ATC_SYSTEM==eATCHonPrecType)                                         //Steven 20120410 End: Hontech ATC
        {
            if(LastSet.iTemperature==Tempture_Hot ||
               LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20180709 (wei) add  Tempture_AmbientHot
            {
                for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
                {
                    ATCInterfaceForm->SetWorkTemperature(iAtc, Temperature.fWorkTemperBase);

                    if(bUseInitTempOffset &&                                    //Steven 20141117 : 起測時溫度要補Offset
                       iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)                                 //Steven 20141125 : <= --> <
                    {
                        ATCInterfaceForm->SetOffsetTemperature(iAtc, Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc]);
                    }
                    else
                    {
                        ATCInterfaceForm->SetOffsetTemperature(iAtc, Temperature.dATCInPC[iAtc]);
                    }
                }
            }
            else
            {
                for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
                {
                    ATCInterfaceForm->SetWorkTemperature(iAtc, IniConfig.dATCAmbientTemperature);                       //Steven 20131007 : 啟用ATC時,強制使用 [L11] ATC常溫的溫度
                }
            }
            ATCInterfaceForm->SetChillerTemperature(Temperature.iATCChillerTemp);
        }

        if(Temperature.bUseTC2Offset)                                           //KenHsieh 20240311 : add Tc2 Offset
        {
            IniConfig.bL11_6ATCUseTemperatureOutsideAlarm=false;
            IniConfig.bL11_7ATCUseMaxSurgeAlarm=false;
            IniConfig.bL11_8ATCUseTemperatureCompare=false;
        }

        if(ATC_SYSTEM==eNewATCSystem && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36) )
            Temperature.bATC_MultiSensorEnable=ReadIniData(szDir, "ATC", "Multi Sensor Offset Enable", false);          //Ifor 20251203 add:ATC Multi Sensor Offset
        else
            Temperature.bATC_MultiSensorEnable=false;
        for(int i=0; i<4; i++)
        {
            str1.sprintf("Multi Sensor Offset %d Use", i+1);
            Temperature.bATC_MultiSensorUse[i]=ReadIniData(szDir, "ATC", str1, false);                                  //Ifor 20251203 add:ATC Multi Sensor Offset
        }

        for(int i=0; i<32; i++)                                                 //Ifor 20190730 : add FFC Temperature Offset
        {
            str1.sprintf("Multi Sensor Offset[%d]", i+1);
            Temperature.dATC_MultiSensorOfs[i]=ReadIniData(szDir, "ATC", str1, 0.0);
        }
    }
    else
    {
        Temperature.bUseReferTempSensor =false;                                 //Steven 20251003 : add default value
    }

    Temperature.ATCTypeName             =CheckAndReadIniData(szDir, "ATC", "Type Name", AnsiString("Rogers"));          //Ifor 20230828 add:送ATC Type 資料給GPIB
    if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                //20141204 ChungHung add for ATC3.0   //2014-05-30    Dell    for ATC6.0
    {
        ATCInterfaceForm->ATC_60_SYS.SetATCRun(LastSet.iTemperature>0?true:false);
        ATCInterfaceForm->ATC_60_SYS.SetTargetTemperature(Temperature.fWorkTemperBase);
        Temperature.bATCActiveCooling   =CheckAndReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.bUseTjFunction      =CheckAndReadIniData(szDir, "ATC", "USE Tj Function", false);
        Temperature.iTjMode             =CheckAndReadIniData(szDir, "ATC", "Tj Mode", 0);
        Temperature.iTjAvgTimes         =CheckAndReadIniData(szDir, "ATC", "Tj Avg Times", 5);
        Temperature.ATCFileName         =CheckAndReadIniData(szDir, "ATC", "File Name", AnsiString("1"));               //Steven 20150810 : add for ATC

        ATCInterfaceForm->ATC_60_SYS.LoadRecipe(Temperature.ATCFileName);       //Steven 20150810 : add for ATC
    }
    else if(ATC_SYSTEM==eNewATCSystem)                                          //Ifor 20151230 :add New ATC Interface HandlerArm
    {                                                                           //Ifor 20160429強制關閉ATC7.0 舊的啟動選項
        Temperature.bATC70Active=false;
        Temperature.bATCActiveCooling   =CheckAndReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.bUseTjFunction      =CheckAndReadIniData(szDir, "ATC", "USE Tj Function", false);
        Temperature.iTjMode             =CheckAndReadIniData(szDir, "ATC", "Tj Mode", 0);
        Temperature.iTjAvgTimes         =CheckAndReadIniData(szDir, "ATC", "Tj Avg Times", 5);
        Temperature.ATCFileName         =CheckAndReadIniData(szDir, "ATC", "File Name",AnsiString("1"));                //Steven 20150810 : add for ATC
        Temperature.iATC60Air           =CheckAndReadIniData(szDir, "ATC", "iATC60Air", 0);
        filerw::ELComboText(EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile"), Temperature.ATCFileName);

        ControlATC60AirFlow();

        //Ifor 20190328 : add TJ Temp Over Range
        //==>
        Temperature.dTJTempRange_High   =CheckRange(ReadIniData(szDir, "ATC", "TJTempRange_High", 0.0), 20.0, 1.0);     //JerryYang 20251201 : 放寬到20
        Temperature.dTJTempRange_Low    =CheckRange(ReadIniData(szDir, "ATC", "TJTempRange_Low", 0.0) , 30.0, 1.0);
        //<==
        //Ifor 20190328 : add TJ Temp Over Range
        if(ATC_SYSTEM==eNewATCSystem &&
    (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36))   // golden uTemp_Set.cpp:2824-2824 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:2824-2824 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
           ATC_InterfaceForm->IS_ATC33())                                       //Ifor 20230418 add:Index 回溫功能
#endif // GATE (S12-C save)
            Temperature.bEnableTempRise     =CheckAndReadIniData(szDir, "ATC", "Enable Temp Rise", false);
        else
            Temperature.bEnableTempRise=false;
        Temperature.dTempRiseTemp       =CheckRange(ReadIniData(szDir, "ATC", "dTempRiseTemp", 25.0), 100.0, 1.0);
        Temperature.dTempRiseDelay      =CheckRange(ReadIniData(szDir, "ATC", "dTempRiseDelay", 1.0), 1000.0, 1.0);
        if(Tri_Temp_Machine==1 ||
           (ATC_SYSTEM==eNewATCSystem &&
            ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
        {
            Temperature.EnableAirMachineSocket              = ReadIniData(szDir,"InitialMode","EnableAirMachineSocket",false);
//            Temperature.fSetTempature2AirMachine            = ReadIniData(szDir,"InitialMode","SetTempature2AirMachine",0.0);
            Temperature.dSetAirstreamTemperatureRang_Socket = ReadIniData(szDir,"InitialMode","dSetAirstreamTemperatureRang_Socket",0.0);
            Temperature.bUseOutShuttleDesoakTime            = ReadIniData(szDir,"InitialMode","UseOutShuttleDesoakTime",false);
            Temperature.iOutShuttleDesoakTime               = ReadIniData(szDir,"InitialMode","OutShuttleDesoakTime", 0);
            Temperature.bEnableTesterDryAirControl          = ReadIniData(szDir,"InitialMode","EnableTesterDryAirControl",false);
//            Temperature.dSetIndexAirstreamTemp            = ReadIniData(szDir,"InitialMode","dSetIndexAirstreamTemp", 0);
            Temperature.SetAirstreamTemperatureRang_Index   = ReadIniData(szDir,"InitialMode","SetAirstreamTemperatureRang_Index",0.0);
            Temperature.bEnableArm_1_Air                    = ReadIniData(szDir,"InitialMode","bEnableArm_1_Air",false);
            Temperature.bEnableArm_2_Air                    = ReadIniData(szDir,"InitialMode","bEnableArm_2_Air",false);
            Temperature.bEnableSocket_Air                   = ReadIniData(szDir,"InitialMode","bEnableSocket_Air",false);
            Temperature.iAirVolumeLmt                       = ReadIniData(szDir,"InitialMode","iAirVolumeLmt",0);
            Temperature.dAirStreamSocket_Offset             = ReadIniData(szDir,"InitialMode","AirStreamSocket_Offset", 0);                                     //Ztex 2023.12.19 Add Air Stream Offset
            Temperature.dAirStreamIndex_Offset              = ReadIniData(szDir,"InitialMode","AirStreamIndex_Offset", 0);                                      //Ztex 2023.12.19 Add Air Stream Offset
            Temperature.iDefrostTimeTooLower                = ReadIniData(szDir,"InitialMode","Defrost_Time_Too_Lower", 600);                                   //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
        }
    }

    Temperature.fChamberCoolTemp =CheckAndReadIniData(szDir, "Cooling", "ChamberCoolTemp", 40.0);                       //Steven 20111209 : Chamber降溫溫度
    Temperature.bUseTesterDocking=CheckAndReadIniData(szDir, "Time", "bUseTesterDocking", false);                       //jou 2014-04-25 Tester Docking Wait delay time
    Temperature.iUseTesterDocking=CheckAndReadIniData(szDir, "Time", "iUseTesterDocking", 20);                          //jou 2014-04-25 Tester Docking Wait delay time

    IniConfig.dATCAmbientTemperature=CheckRange(ReadIniData(szDir, "ATC", "ATCAmbientTemperature", 25.0), 30.0, -5.0);  //Steven 20150108 : [L11-5] For海思使用兩組感溫;     //wei 20151013  by Setup File ATC Ambient Temp set  //JerryYang 20220927 : ATC常溫range限制範圍
    IniConfig.dHeatGunTempATC=CheckRange(ReadIniData(szDir, "Index", "ATC_HeatGunTemp", 30.0), 70.0, 30.0);             //JerryYang 20220408 : add for ATC3.5
    Temperature.dATC_HotGunTemp     =CheckAndReadIniData(szDir, "ATC", "dATC_HotGunTemp", 25.0);                        //Ifor 20220124 add: ATC Hot Air
    Temperature.dATC_HotGunTime     =CheckAndReadIniData(szDir, "ATC", "dATC_HotGunTime", 0.0);                         //Ifor 20220124 add: ATC Hot Air

    Temperature.Arm1Offset  =CheckAndReadIniData(szDir, "Index", "Arm1 Offset", 0.0);
    Temperature.Arm2Offset  =CheckAndReadIniData(szDir, "Index", "Arm2 Offset", 0.0);

    Temperature.Arm1NoFullsiteOffset_1  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset1", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_2  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset2", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_3  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset3", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_4  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset4", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_5  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset5", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_1  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset1", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_2  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset2", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_3  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset3", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_4  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset4", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_5  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset5", 0.0);          //wei 20160107 No FullSite Add Offset

    ATCInterfaceForm->iCheckSameTempTime=CheckAndReadIniData("D:\\HT9045\\Config\\ATC.ini", "Setup", "iCheckSameTempTime", 300);                                //Ifor 20160308 Add 開啟ATC功能時讀取ATC設定檔

    Temperature.dSocketAirCoolingOnTimer=CheckAndReadIniData(szDir, "Index", "dSocketAirCoolingOnTimer", 5.0);          //jou 2016-04-28 Socket Air Cooling contact count trun on
    Temperature.dSocketAirCoolingOffTimer=CheckAndReadIniData(szDir, "Index", "dSocketAirCoolingOffTimer", 5.0);        //jou 2016-04-28 Socket Air Cooling contact count trun on

    if(CUSTOMER_CODE==CC_HONPREC_QC)                                            //KenHsieh 20230301 : By Pass Chamber
        Temperature.bByPassChamber          =CheckAndReadIniData(szDir, "Ambient",  "ByPassChamber",     false);
    else
        Temperature.bByPassChamber=false;

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        Temperature.bUseTriTempHeater_Ini[0]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterOutShuttle",           false);
        Temperature.bUseTriTempHeater_Ini[1]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterIndex",                false);
        Temperature.bUseTriTempHeater_Ini[2]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterBase",                 false);
        Temperature.bUseTriTempHeater_Ini[3]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterDoor",                 false);                                     //Ztex 2023.10.23 Add Index Door Heater
    }                                                                           //Heater開關設定

    Temperature.bPowerFollower_Enable   =CheckAndReadIniData(szDir, "ATC",   "bPowerFollow_Enable", false);             //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.iPowerFollower_FullPower=CheckAndReadIniData(szDir, "ATC",   "iPowerFollow_FullPower", 1000);           //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.dPowerFollower_PFSlope  =CheckAndReadIniData(szDir, "ATC",   "dPowerFollower_PFSlope", 0.0);            //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.dPowerFollower_WGain    =CheckAndReadIniData(szDir, "ATC",   "dPowerFollower_WGain", 0.0);              //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.iPowerFollower_Many2one =CheckAndReadIniData(szDir, "ATC",   "iPowerFollower_Many2one",1);              //KenHsieh 20240216 : add ATC Power Follow Function

//------------------------我是分隔線:以下是寫到DefineTemp-----------------------
    if(bUpdateAll==true)                                                        //jou 2010-01-11 start : 效正溫度改固定一組
    {
        bReadType=(Temperature.iIndexHeatMode==HeadChamber)?true:false;
    }
    else
    {
        bReadType=(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex==HeadChamber)?true:false;
    }

    szDir.sprintf("%sDefineTemp\\", DefaultPath);                               //JerryYang 20170123 避免路徑不存在出現錯誤
    MyForceDirectories(szDir);

    if(bReadType)                                                               //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
                WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                              //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常。//Sam 20210106 : TemperatureData 新增版號來相容
            }
            else                                                                //Sam 20210106 : TemperatureData 新增版號來相容
            {
                if(INDEX_PRESS_TYPE==e500KG)
                {
                    iTemperatureDataVer=ReadIniData(szDir, "Mode","Version",0);
                    if(iTemperatureDataVer<atoi(AnsiString(TemperatureDatavision).c_str()))                             //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    {
                        DeleteFile(szDir);
                        str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                        CopyFile(str.c_str(), szDir.c_str(), false);
                        WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                      //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    }
                }
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber_ATC.Data", DefaultPath);
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }
    else
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\Temperature60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
                WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                              //Sam 20210106 : TemperatureData 新增版號來相容
            }
            else                                                                //Sam 20210106 : TemperatureData 新增版號來相容
            {
                if(INDEX_PRESS_TYPE==e500KG)
                {
                    iTemperatureDataVer=ReadIniData(szDir, "Mode","Version",0);
                    if(iTemperatureDataVer<atoi(AnsiString(TemperatureDatavision).c_str()))                             //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    {
                        DeleteFile(szDir);
                        str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                        CopyFile(str.c_str(), szDir.c_str(), false);
                        WriteIniData(szDir, "Mode", "Version", (AnsiString)TemperatureDatavision);                      //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    }
                }
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC.Data", DefaultPath);
                }
                else
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC_Cold.Data", DefaultPath);
                }

                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20151204 ase kaohsiung 林佳青只 使用一組  temp offset
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }
    else if(CosFunction.bTempCalByRecipe==true && Temperature.bTempCalByRecipe==true)                                   //jou 20220725 : Temperature calibration by recipe
    {
        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\DefineTemperature.Data", DataPath, S);
    }

    szDir2.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);

    Temperature.iTempMode           =ReadIniData(szDir, "Mode",                 "Points",   1);                         //Jimmychiu 20230926 : fixed for iTempMode initial value is 1
    Temperature.fLowBase            =ReadIniData(szDir, "Low OffSet",           "Base",     0.0);
    Temperature.fMiddBase           =ReadIniData(szDir, "Mid. OffSet",          "Base",     0.0);
    Temperature.fHighBase           =ReadIniData(szDir, "High OffSet",          "Base",     0.0);

    Temperature.fAmbientHotLowBase  =ReadIniData(szDir, "AmbientHotLowOffSet",  "Base",     0.0);                       //kevin 20141006
    Temperature.fAmbientHotMiddBase =ReadIniData(szDir, "AmbientHotMidOffSet",  "Base",     0.0);                       //kevin 20141006
    Temperature.fSHighBase          =ReadIniData(szDir, "SHigh OffSet",         "Base",     0.0);                       //Ztex 2024.07.27 Add 6 Point Temperature Offset

    for(i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);
        Temperature.fTempOffSet[LowBase][i]                     =ReadIniData(szDir, "Low OffSet",  S, 0.0);
        Temperature.fTempOffSet[MidBase][i]                     =ReadIniData(szDir, "Mid. OffSet", S, 0.0);
        Temperature.fTempOffSet[HigBase][i]                     =ReadIniData(szDir, "High OffSet", S, 0.0);

        if(CosFunction.bInitTempOffsetByWorkFile==false)                        //jou 2015-06-13 Initial Temperture Offset by WorkFile
            Temperature.fTempOffSet[InitTempOffset][i]          =ReadIniData(szDir, "Init Temp OffSet", S, 0.0);        //Steven 201    41117 : 起測時溫度要補Offset

        Temperature.fTempOffSet[AmbientHotLow][i]               =ReadIniData(szDir, "AmbientHotLowOffSet", S, 0.0);     //kevin 20141006
        Temperature.fTempOffSet[AmbientHotMid][i]               =ReadIniData(szDir, "AmbientHotMidOffSet", S, 0.0);     //kevin 20141006
        Temperature.fTempOffSet[TestOverTimeTempOffset][i]      =ReadIniData(szDir, "TestOverTime Temp OffSet", S, 0.0);                                        //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        Temperature.fTempOffSet[SHigBase][i]                    =ReadIniData(szDir, "SHigh OffSet", S, 0.0);            //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    //jou 2010-01-11 end

    if(IniConfig.bL13HotPlateAndShuttleUseOneTempOffset)                        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    {
        for(i=tcHotPlate1; i<=tcShuttle2; i++)
        {
            S.printf("CH%d", i+1);
            Temperature.fTempOffSet[LowBase][i]                 =ReadIniData(szDir2, "Low OffSet",  S, 0.0);
            Temperature.fTempOffSet[MidBase][i]                 =ReadIniData(szDir2, "Mid. OffSet", S, 0.0);
            Temperature.fTempOffSet[HigBase][i]                 =ReadIniData(szDir2, "High OffSet", S, 0.0);

            if(CosFunction.bInitTempOffsetByWorkFile==false)                    //jou 2015-06-13 Initial Temperture Offset by WorkFile
                Temperature.fTempOffSet[InitTempOffset][i]      =ReadIniData(szDir2, "Init Temp OffSet", S, 0.0);       //Steven 20141117 : 起測時溫度要補Offset

            Temperature.fTempOffSet[AmbientHotLow][i]           =ReadIniData(szDir2, "AmbientHotLowOffSet", S, 0.0);    //kevin 20141006
            Temperature.fTempOffSet[AmbientHotMid][i]           =ReadIniData(szDir2, "AmbientHotMidOffSet", S, 0.0);    //kevin 20141006
            Temperature.fTempOffSet[TestOverTimeTempOffset][i]  =ReadIniData(szDir2, "TestOverTime Temp OffSet", S, 0.0);                                       //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
            Temperature.fTempOffSet[SHigBase][i]                =ReadIniData(szDir2, "SHigh OffSet", S, 0.0);           //Ztex 2024.07.27 Add 6 Point Temperature Offset
        }
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        Temperature.bUseAbitCHK      =CheckAndReadIniData(szDir2, "Mode",    "AmbCheck",     true);

    if(TC401HeaterControl==NoHeater)                                            //Steven 20171227 (Wei) : Add for HT-9045L
    {
        Temperature.iMachineTempMode=1;
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex=1;
        Temperature.bUseAbitCHK=false;
    }

    //Steven 20181203 : fTempOffSet --> fIndividualTemp
    Temperature.dIndexATCSecondTempOffset[ 0]=Temperature.fIndividualTemp[tcAa1];
    Temperature.dIndexATCSecondTempOffset[ 1]=Temperature.fIndividualTemp[tcAb1];
    Temperature.dIndexATCSecondTempOffset[ 2]=Temperature.fIndividualTemp[tcAc1];
    Temperature.dIndexATCSecondTempOffset[ 3]=Temperature.fIndividualTemp[tcAd1];
    Temperature.dIndexATCSecondTempOffset[ 4]=Temperature.fIndividualTemp[tcAe1];
    Temperature.dIndexATCSecondTempOffset[ 5]=Temperature.fIndividualTemp[tcAf1];
    Temperature.dIndexATCSecondTempOffset[ 6]=Temperature.fIndividualTemp[tcAg1];
    Temperature.dIndexATCSecondTempOffset[ 7]=Temperature.fIndividualTemp[tcAh1];
    Temperature.dIndexATCSecondTempOffset[ 8]=Temperature.fIndividualTemp[tcBa1];
    Temperature.dIndexATCSecondTempOffset[ 9]=Temperature.fIndividualTemp[tcBb1];
    Temperature.dIndexATCSecondTempOffset[10]=Temperature.fIndividualTemp[tcBc1];
    Temperature.dIndexATCSecondTempOffset[11]=Temperature.fIndividualTemp[tcBd1];
    Temperature.dIndexATCSecondTempOffset[12]=Temperature.fIndividualTemp[tcBe1];
    Temperature.dIndexATCSecondTempOffset[13]=Temperature.fIndividualTemp[tcBf1];
    Temperature.dIndexATCSecondTempOffset[14]=Temperature.fIndividualTemp[tcBg1];
    Temperature.dIndexATCSecondTempOffset[15]=Temperature.fIndividualTemp[tcBh1];
    Temperature.dIndexATCSecondTempOffset[16]=Temperature.fIndividualTemp[tcAa2];
    Temperature.dIndexATCSecondTempOffset[17]=Temperature.fIndividualTemp[tcAb2];
    Temperature.dIndexATCSecondTempOffset[18]=Temperature.fIndividualTemp[tcAc2];
    Temperature.dIndexATCSecondTempOffset[19]=Temperature.fIndividualTemp[tcAd2];
    Temperature.dIndexATCSecondTempOffset[20]=Temperature.fIndividualTemp[tcAe2];
    Temperature.dIndexATCSecondTempOffset[21]=Temperature.fIndividualTemp[tcAf2];
    Temperature.dIndexATCSecondTempOffset[22]=Temperature.fIndividualTemp[tcAg2];
    Temperature.dIndexATCSecondTempOffset[23]=Temperature.fIndividualTemp[tcAh2];
    Temperature.dIndexATCSecondTempOffset[24]=Temperature.fIndividualTemp[tcBa2];
    Temperature.dIndexATCSecondTempOffset[25]=Temperature.fIndividualTemp[tcBb2];
    Temperature.dIndexATCSecondTempOffset[26]=Temperature.fIndividualTemp[tcBc2];
    Temperature.dIndexATCSecondTempOffset[27]=Temperature.fIndividualTemp[tcBd2];
    Temperature.dIndexATCSecondTempOffset[28]=Temperature.fIndividualTemp[tcBe2];
    Temperature.dIndexATCSecondTempOffset[29]=Temperature.fIndividualTemp[tcBf2];
    Temperature.dIndexATCSecondTempOffset[30]=Temperature.fIndividualTemp[tcBg2];
    Temperature.dIndexATCSecondTempOffset[31]=Temperature.fIndividualTemp[tcBh2];

    Temperature.dIndexATCInitTempOffset[ 0]=Temperature.fTempOffSet[InitTempOffset][tcAa1];
    Temperature.dIndexATCInitTempOffset[ 1]=Temperature.fTempOffSet[InitTempOffset][tcAb1];
    Temperature.dIndexATCInitTempOffset[ 2]=Temperature.fTempOffSet[InitTempOffset][tcAc1];
    Temperature.dIndexATCInitTempOffset[ 3]=Temperature.fTempOffSet[InitTempOffset][tcAd1];
    Temperature.dIndexATCInitTempOffset[ 4]=Temperature.fTempOffSet[InitTempOffset][tcAe1];
    Temperature.dIndexATCInitTempOffset[ 5]=Temperature.fTempOffSet[InitTempOffset][tcAf1];
    Temperature.dIndexATCInitTempOffset[ 6]=Temperature.fTempOffSet[InitTempOffset][tcAg1];
    Temperature.dIndexATCInitTempOffset[ 7]=Temperature.fTempOffSet[InitTempOffset][tcAh1];
    Temperature.dIndexATCInitTempOffset[ 8]=Temperature.fTempOffSet[InitTempOffset][tcBa1];
    Temperature.dIndexATCInitTempOffset[ 9]=Temperature.fTempOffSet[InitTempOffset][tcBb1];
    Temperature.dIndexATCInitTempOffset[10]=Temperature.fTempOffSet[InitTempOffset][tcBc1];
    Temperature.dIndexATCInitTempOffset[11]=Temperature.fTempOffSet[InitTempOffset][tcBd1];
    Temperature.dIndexATCInitTempOffset[12]=Temperature.fTempOffSet[InitTempOffset][tcBe1];
    Temperature.dIndexATCInitTempOffset[13]=Temperature.fTempOffSet[InitTempOffset][tcBf1];
    Temperature.dIndexATCInitTempOffset[14]=Temperature.fTempOffSet[InitTempOffset][tcBg1];
    Temperature.dIndexATCInitTempOffset[15]=Temperature.fTempOffSet[InitTempOffset][tcBh1];
    Temperature.dIndexATCInitTempOffset[16]=Temperature.fTempOffSet[InitTempOffset][tcAa2];
    Temperature.dIndexATCInitTempOffset[17]=Temperature.fTempOffSet[InitTempOffset][tcAb2];
    Temperature.dIndexATCInitTempOffset[18]=Temperature.fTempOffSet[InitTempOffset][tcAc2];
    Temperature.dIndexATCInitTempOffset[19]=Temperature.fTempOffSet[InitTempOffset][tcAd2];
    Temperature.dIndexATCInitTempOffset[20]=Temperature.fTempOffSet[InitTempOffset][tcAe2];
    Temperature.dIndexATCInitTempOffset[21]=Temperature.fTempOffSet[InitTempOffset][tcAf2];
    Temperature.dIndexATCInitTempOffset[22]=Temperature.fTempOffSet[InitTempOffset][tcAg2];
    Temperature.dIndexATCInitTempOffset[23]=Temperature.fTempOffSet[InitTempOffset][tcAh2];
    Temperature.dIndexATCInitTempOffset[24]=Temperature.fTempOffSet[InitTempOffset][tcBa2];
    Temperature.dIndexATCInitTempOffset[25]=Temperature.fTempOffSet[InitTempOffset][tcBb2];
    Temperature.dIndexATCInitTempOffset[26]=Temperature.fTempOffSet[InitTempOffset][tcBc2];
    Temperature.dIndexATCInitTempOffset[27]=Temperature.fTempOffSet[InitTempOffset][tcBd2];
    Temperature.dIndexATCInitTempOffset[28]=Temperature.fTempOffSet[InitTempOffset][tcBe2];
    Temperature.dIndexATCInitTempOffset[29]=Temperature.fTempOffSet[InitTempOffset][tcBf2];
    Temperature.dIndexATCInitTempOffset[30]=Temperature.fTempOffSet[InitTempOffset][tcBg2];
    Temperature.dIndexATCInitTempOffset[31]=Temperature.fTempOffSet[InitTempOffset][tcBh2];
   //---------------------------我是分隔線-----------------------------------------
    TS_DoIniDataToForm(bUpdateAll);

    //Steven 20110324 : 加熱膨脹係數 (比例)-------------------------------------
    //材料鋁合金(A6061)的熱膨脹係數約0.000023
    //熱脹量=(升溫溫度-常溫)*材料膨脹係數*材料寬度(mm)
    //HT9045 HotPlate X=220; Y=380;
    if(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)      //Steven 20140924 : 各個加熱區獨立有自己的設定值
    {
        fHotPlateExpansionCoefficient=1;
    }
    else if(Temperature.fWorkTemperBase>30)                                     //要大於常溫才可以設定
    {
        fHotPlateExpansionCoefficient=(Temperature.fWorkTemperBase-30.0)*0.000023+1.0;
    }
    else
    {
        fHotPlateExpansionCoefficient=1;
    }
    //Steven 20110324 End------------------------------------------------------

    fLotInfo->SetATCFormVisible();                                              //Steven 20160217 : For ATC7.0
    if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true && (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite)) filerw::ELTodo("golden rs232.cpp:4260 COM2->ATCInitialTask (ATC initial-task flags, eATCSiliconType) not ported");   // golden uTemp_Set.cpp:3178-3178 取代：COM2->ATCInitialTask：TCOM2 移植樹沒有（golden rs232.cpp:4260，只在 eATCSiliconType 生效）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:3178-3178 -- COM2->ATCInitialTask：TCOM2 移植樹沒有（golden rs232.cpp:4260，只在 eATCSiliconType 生效）
    COM2->ATCInitialTask();                                                     //jou 2012-03-14 ATC
#endif // GATE (S12-C save)
    ATKRecipeInfo->SaveFile();                                                  //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案

}

// golden uTemp_Set.cpp:3182  TfTemp_Set::DoIniDataToForm(bool bUpdateAll)
static void TS_DoIniDataToForm(bool bUpdateAll)
{

    int iSel=0;
    AnsiString S="";
    TS_ReadTestIFFile_InitialMode(); filerw::ELTodo("golden cTesterIF.cpp:563 FTestIF->ReadTestIFFile only the [InitialMode] block (:713-779) re-read -- rest gated F-5 (porting-gaps 14)");   // golden uTemp_Set.cpp:3186-3186 取代：FTestIF->ReadTestIFFile 移植樹 GATE (F-5)：只跑 golden cTesterIF.cpp:713-779 的 [InitialMode] 段（本表單顯示／寫的欄位）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:3186-3186 -- FTestIF->ReadTestIFFile 移植樹 GATE (F-5)：只跑 golden cTesterIF.cpp:713-779 的 [InitialMode] 段（本表單顯示／寫的欄位）
    FTestIF->ReadTestIFFile();                                                  //Steven 20180815 : Initial Temp Offset
#endif // GATE (S12-C save)

    if(bUpdateAll==true)
    {
        EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode")->Checked=Temperature.bUseIndividualTemp;         //Steven 20140924 : 各個加熱區獨立有自己的設定值
        EL<TCheckBox>("TfTemp_Set", "chkSoakTimeReduce")     ->Checked=Temperature.bUseInitialDelayAsSoakTime;                                         //Steven 20170329 (wei) : 使用initial delay當 Soak time
        EL<TCheckBox>("TfTemp_Set", "chkTempErrorAction")    ->Checked=Temperature.bTempAlarmBinNeedToError;   //Steven 20251022 : Temp alarm need put to error bin

        if(ATC_SYSTEM<=eATC60)                                                  //2014-05-30    Dell    for ATC6.0
        {
            if(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Items->Count==5)                                //JerryYang 20161222 (jou) 中壢日月光先不開Head + Chamber + Socket, 怕電流超過20A
                EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->Items->Add("Head + Chamber + Socket");         //kevin 20131209
            EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked  =Temperature.bATCActiveCooling;
            EL<TCheckBox>("TfTemp_Set", "chk_UseTj")->Checked      =Temperature.bUseTjFunction;
            EL<TRadioGroup>("TfTemp_Set", "rg_TjMode")->ItemIndex    =Temperature.iTjMode;
            EL<TEdit>("TfTemp_Set", "edt_TjAvgTimes")->Text    =Temperature.iTjAvgTimes;
        }

        if(IniConfig.bOnlyRoomOrHot)                                            //Steven 20110914
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 1);
        else
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 2);

        if(IniConfig.bI03AmbientTempControl)                                    //kevin 20140918 恆溫控制
        {
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 3);
        }
        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex=iSel;
        iTempMoldSet=iSel;                                                      //20140217  wei  Barcode Reader

        EL<TEdit>("TfTemp_Set", "edWorkTemp")->Text        =FormatFloat("0.0", Temperature.fWorkTemperBase);
        EL<TCheckBox>("TfTemp_Set", "cbUseAbitCHK")->Checked   =Temperature.bUseAbitCHK;
        EL<TRadioGroup>("TfTemp_Set", "rgAmbCheck")->ItemIndex   =Temperature.iAbitCHKStatus;
        EL<TEdit>("TfTemp_Set", "edAmbTemp")->Text         =FormatFloat("0.0", Temperature.fAbitTemp);
        EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")->Checked=Temperature.bAmbUsingAFan;
        EL<TEdit>("TfTemp_Set", "edSoakTime")->Text        =FormatFloat("0.0", Temperature.fSoakTime);
        EL<TEdit>("TfTemp_Set", "edJamSoakTime")->Text     =FormatFloat("0.0", Temperature.fJamSoakTime);
        EL<TEdit>("TfTemp_Set", "edInitialWaitTime")->Text =FormatFloat("0.0", Temperature.fInitialWaitTime);
        EL<TEdit>("TfTemp_Set", "edAbitInitWaitTime")->Text=FormatFloat("0.0", Temperature.fAbitInitWaitTime);
        EL<TEdit>("TfTemp_Set", "edAbitColdTime")->Text    =FormatFloat("0.0", Temperature.fAbitColdTime);
        EL<TEdit>("TfTemp_Set", "edATCAmbTemp")->Text      =FormatFloat("0.0", IniConfig.dATCAmbientTemperature);                                  //wei 20151013  by Setup File ATC Ambient Temp set
        EL<TEdit>("TfTemp_Set", "edtHeatGunTempATC")->Text =FormatFloat("0.0", IniConfig.dHeatGunTempATC);                                         //JerryYang 20220408 : add for ATC3.5
        EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->ItemIndex=Temperature.iShuttleSoakTimeMode;

        EL<TCheckBox>("TfTemp_Set", "cbWaitDewPoint")->Checked         =Temperature.bWaitDewPoint;             //Steven 20191017 : 露點計
        EL<TEdit>("TfTemp_Set", "edDewPointRange")->Text           =Temperature.dDewPointRange;
        EL<TEdit>("TfTemp_Set", "edDewPointAlarmInterval")->Text   =Temperature.iDewPointAlarmInterval;

        EL<TCheckBox>("TfTemp_Set", "cbBlowLB")->Checked         =Temperature.bLBCoolingAirOn;                 //JerryYang 20220923 : LB吹氣function
        EL<TEdit>("TfTemp_Set", "edtLBAirOnTemp")->Text      =Temperature.dLBAirOnTemp;
        EL<TEdit>("TfTemp_Set", "edLBTempAlarmTime")->Text   =Temperature.iLBTempAlmInterval;

        for(int i=0; i<tcTotalCount; i++)                                       // 將檔案讀到的限制值存放進各頻道的容器中
        {
            myTempPal[i]->iOffsetByRecipeMaxLimit = Temperature.iOffsetByRecipeMaxLimit;
            myTempPal[i]->iOffsetByRecipeMinLimit = Temperature.iOffsetByRecipeMinLimit;
            myTempPal[i]->edOffset->Text=FormatFloat("0.0", Temperature.fTempOffSet[UserOffSet][i]);
            myTempPal[i]->edIndiTemp->Text=FormatFloat("0.0", Temperature.fIndividualTemp[i]);                          //Steven 20140924 : 各個加熱區獨立有自己的設定值
        }

        if(ATC_SYSTEM>eATC60)                                                   //2014-05-30    Dell    for ATC6.0
        {
            if(Temperature.iIndexHeatMode!=HeadOnly)                            //Steven 20240426 : unmark
                EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex=HeadSocket;
            else
                EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex=HeadOnly;

            if(ATC_SYSTEM!=eNonChamber && ATC_SYSTEM!=eATC30)                   //20141204 ChungHung add for ATC3.0   //Steven 20140314 : For HT9045WA
            {
                if(ATC_SYSTEM!=eATCSiliconType)
                {
                    EL<TCheckBox>("TfTemp_Set", "rbTemperatureSetOn")->Checked=Temperature.bATCTemperatureSet;
                }
                EL<TCheckBox>("TfTemp_Set", "rbHandlerStartOn")->Checked=Temperature.bATCHandlerStart;
                EL<TCheckBox>("TfTemp_Set", "rbTestStartOn")->Checked=Temperature.bATCTestStrat;
                EL<TEdit>("TfTemp_Set", "edtATCSP2")->Text=Temperature.dATCSP2;
                EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked=Temperature.bATCActiveCooling;
                EL<TEdit>("TfTemp_Set", "edtATCInPC1")->Text=Temperature.dATCInPC[0];
                EL<TEdit>("TfTemp_Set", "edtATCInPC2")->Text=Temperature.dATCInPC[1];
                EL<TEdit>("TfTemp_Set", "edtATCInPC3")->Text=Temperature.dATCInPC[2];
                EL<TEdit>("TfTemp_Set", "edtATCInPC4")->Text=Temperature.dATCInPC[3];
                EL<TEdit>("TfTemp_Set", "edATCInitialOffset1")->Text=Temperature.ATCInitialOffset[0];      //Steven 20151006 : Initial Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCInitialOffset2")->Text=Temperature.ATCInitialOffset[1];      //Steven 20151006 : Initial Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCInitialOffset3")->Text=Temperature.ATCInitialOffset[2];      //Steven 20151006 : Initial Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCInitialOffset4")->Text=Temperature.ATCInitialOffset[3];      //Steven 20151006 : Initial Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCContFailOffset1")->Text=Temperature.dATCConFailOffset[0];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCContFailOffset2")->Text=Temperature.dATCConFailOffset[1];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCContFailOffset3")->Text=Temperature.dATCConFailOffset[2];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCContFailOffset4")->Text=Temperature.dATCConFailOffset[3];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TCheckBox>("TfTemp_Set", "cbEnableATCConsFailOffset")->Checked=Temperature.bEnableATCConFailOffset;                                 //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCContFailOffsetCnt")->Text=Temperature.iATCConFailOffsetCount;                                        //Steven 20151123 : Continue Fail Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCQAModeOffset1")->Text=Temperature.dATCQAModeOffset[0];       //Steven 20151125 : QA Mode Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCQAModeOffset2")->Text=Temperature.dATCQAModeOffset[1];       //Steven 20151125 : QA Mode Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCQAModeOffset3")->Text=Temperature.dATCQAModeOffset[2];       //Steven 20151125 : QA Mode Temp Offset for ATC
                EL<TEdit>("TfTemp_Set", "edATCQAModeOffset4")->Text=Temperature.dATCQAModeOffset[3];       //Steven 20151125 : QA Mode Temp Offset for ATC
                EL<TCheckBox>("TfTemp_Set", "cbEnableATCQAModeOffset")->Checked=Temperature.bEnableATCQAModeOffset;                                    //Steven 20151125 : QA Mode Temp Offset for ATC

                EL<TCheckBox>("TfTemp_Set", "cbATCTestTimeOffset")->Checked=Temperature.bEnableATCTestTimeOffset;                                      //Steven 20160216 : 測試時間太短也要Offset
                EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset")->Text=Temperature.iATCTestTimeOffsetTime;   //Steven 20160216 : 測試時間太短也要Offset
                EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset1")->Text=Temperature.dATCTestTimeOffset[0];   //Steven 20160216 : 測試時間太短也要Offset
                EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset2")->Text=Temperature.dATCTestTimeOffset[1];   //Steven 20160216 : 測試時間太短也要Offset
                EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset3")->Text=Temperature.dATCTestTimeOffset[2];   //Steven 20160216 : 測試時間太短也要Offset
                EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset4")->Text=Temperature.dATCTestTimeOffset[3];   //Steven 20160216 : 測試時間太短也要Offset

                EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text=Temperature.iATCChillerTemp;
                EL<TEdit>("TfTemp_Set", "edAtcFileName")->Text=Temperature.ATCFileName;                    //Ifor 20160111 顯示目前設定receipe 檔案名稱
                filerw::ELComboText(EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile"), Temperature.ATCFileName);                //Ifor 20160111 顯示目前設定receipe 檔案名稱
                EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->ItemIndex =Temperature.iATC60Air;

                if(CosFunction.bUseOldATCTempOffset==true)                      //JerryYang 20190123 ATC Offset UI使用舊版
                {
                    for(int i=0; i<32; i++)
                    {
                        ATCOffsetEdit[i]->Text=Temperature.dATCTempOffset[i];
                    }
                }
                else
                {
                    if(ATC_SYSTEM==eNewATCSystem &&                             //Ifor 20190930 ATC 功能開啟才使用ATC Temp Offset檔案
                       Temperature.bATCActiveCooling==true)                     // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180814
                    {
                        for(int i=0; i<tcTotalCount; i++)
                        {
                            for(int j=0; j<32; j++)
                            {
                                if(myTempPal[i]->iIndexTag==j)
                                {
                                    myTempPal[i]->edOffset->Text=Temperature.dATCTempOffset[j];
                                    Temperature.fTempOffSet[UserOffSet][i]=Temperature.dATCTempOffset[j];               //Ifor 20200106 fix ATC offset
                                }
                            }
                        }
                    }

                    if(CosFunction.bUseSecondATCTempOffset==true)               //Ifor 20241014 Use Second ATC Temp Offset
                    {
                        for(int i=0; i<32; i++)
                        {
                            ATCOffsetEdit[i]->Text=Temperature.dATCSecondTempOffset[i];
                        }
                    }
                }

                //Ifor 20160421 第二點溫度Sensor設定改至工作檔
                EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked=Temperature.bUseReferTempSensor;
                //Ifor 20190306 : add Package Offset 三點校正
                //==>
                for(int i=0; i<3; i++)
                {
                    ATCPackageOffsetEdit[i] ->Text=Temperature.dATCPackageOffset[i];
                    ATCPackageTempEdit[i]   ->Text=Temperature.dATCPackageTemp[i];
                }
                //<==
                //Ifor 20190306 : add Package Offset 三點校正

                //Ifor 20190328 : add TJ Temp Over Range
                //==>
                 EL<TEdit>("TfTemp_Set", "edTJTempRange_High") ->Text=Temperature.dTJTempRange_High;
                 EL<TEdit>("TfTemp_Set", "edTJTempRange_Low")  ->Text=Temperature.dTJTempRange_Low;
                //<==
                //Ifor 20190328 : add TJ Temp Over Range
                EL<TCheckBox>("TfTemp_Set", "cbATCTempRise")       ->Checked=Temperature.bEnableTempRise;
                EL<TEdit>("TfTemp_Set", "edt_TempRiseTemp")    ->Text=Temperature.dTempRiseTemp;
                EL<TEdit>("TfTemp_Set", "edt_TempRise_Delay")  ->Text=Temperature.dTempRiseDelay;
                EL<TEdit>("TfTemp_Set", "edtATC_Type")->Text=Temperature.ATCTypeName;                      //Ifor 20230828 add:送ATC Type 資料給GPIB
                EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset")->Checked=Temperature.bUseTC2Offset;              //KenHsieh 20240311 : add Tc2 Offset
            }
        }
        else
        {
            EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex  =Temperature.iIndexHeatMode;            //Steven 20090926
        }

        EL<TEdit>("TfTemp_Set", "edInitialStart1Time")->Text=Temperature.iInitialStart1Time;
        EL<TEdit>("TfTemp_Set", "edInitialStart2Time")->Text=Temperature.iInitialStart2Time;

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            Temperature.iIndexSoakTime = 0;
            Temperature.iOSTime = 0;
        }                                                                       //kevin 20210511 ASE KH  JL 說關閉
        EL<TEdit>("TfTemp_Set", "edIndexSoakTime")->Text   =Temperature.iIndexSoakTime;                    //2013-11-27   Dell    需要做Index soak time
        EL<TEdit>("TfTemp_Set", "edOSTime")->Text          =Temperature.iOSTime;                           //2013-11-27   Dell    需要做Index soak time
        EL<TCheckBox>("TfTemp_Set", "cbUseFixTemp")->Checked   =Temperature.bUseFixTemp;                       //Steven 20131025 : 使4固定的DUT溫度
        EL<TEdit>("TfTemp_Set", "edFixedTemp")->Text       =Temperature.dFixedTemp;                        //Steven 20131025 : 固定的DUT溫度值
        EL<TCheckBox>("TfTemp_Set", "cbShowFixTemp")->Checked  =Temperature.bShowFixedTemp;                    //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值

        if(CosFunction.bTempCalByRecipe==true)                                  //jou 20220725 : Temperature calibration by recipe
            EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked=Temperature.bTempCalByRecipe;
    }

    if(Tri_Temp_Machine==1 ||
       (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
    {
//        edtSetTempature2AirMachine->Text                    = AnsiString(Temperature.fSetTempature2AirMachine);
        EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket")->Text        = AnsiString(Temperature.dSetAirstreamTemperatureRang_Socket);
//        edt_SetIndexAirstreamTemp->Text                     = Temperature.dSetIndexAirstreamTemp;
        EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index")->Text         = AnsiString(Temperature.SetAirstreamTemperatureRang_Index);
//        cbUseOutShuttleDesoakTime->Checked                  = Temperature.bUseOutShuttleDesoakTime;
//        edtOutShuttleDesoakTime->Text                       = AnsiString(Temperature.iOutShuttleDesoakTime);
        EL<TCheckBox>("TfTemp_Set", "cbEnableTesterDryAirControl")->Checked                = Temperature.bEnableTesterDryAirControl;
        EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_1_Air")->Checked                        = Temperature.bEnableArm_1_Air;
        EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_2_Air")->Checked                        = Temperature.bEnableArm_2_Air;
        EL<TCheckBox>("TfTemp_Set", "chk_EnableSocket_Air")->Checked                       = Temperature.bEnableSocket_Air;
        EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Index")->Text                        = Temperature.iAirVolumeLmt;
        EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Socket")->Text                       = EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Index")->Text;
        EL<TEdit>("TfTemp_Set", "edtSetTempature2AirMachine")->Text                    = AnsiString(Temperature.dAirStreamSocket_Offset);          //Ztex 2023.12.19 Add Air Stream Offset
        EL<TEdit>("TfTemp_Set", "edt_SetIndexAirstreamTemp")->Text                     = AnsiString(Temperature.dAirStreamIndex_Offset);           //Ztex 2023.12.19 Add Air Stream Offset
        EL<TEdit>("TfTemp_Set", "edt_Defrost_Time_Too_Lower")->Text                    = AnsiString(Temperature.iDefrostTimeTooLower);             //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
    }

    //Steven 20180817 : Boost Function
    //==>
    EL<TCheckBox>("TfTemp_Set", "chkBoostFunction")->Checked   =Temperature.bBoostFuncttion;
    EL<TCheckBox>("TfTemp_Set", "chkLBTempFunction")->Checked  =Temperature.bLBTempFunction;
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Long")->Text      =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBMax]);
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Long")->Text   =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBMax]);
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Long")->Text =FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBMax]);
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Long")->Text     =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBMax]);

    if(iTriggerBoostFunction!=-1)
    {
        EL<TEdit>("TfTemp_Set", "edtIdleTime_Mid")->Text       =FormatFloat("0.0", iBoostEotToSotTime);
        EL<TEdit>("TfTemp_Set", "edtBoostOffset_Mid")->Text    =FormatFloat("0.0", Temperature.dBoostOffset[iTriggerBoostFunction]);
        EL<TEdit>("TfTemp_Set", "edtBoostDuration_Mid")->Text  =FormatFloat("0.0", Temperature.dBoostDuration[iTriggerBoostFunction]);
        EL<TEdit>("TfTemp_Set", "edtPostBoost_Mid")->Text      =FormatFloat("0.0", Temperature.dPostBoostDuration[iTriggerBoostFunction]);
    }
    else if(iTriggerBoostFuncBack!=-1)
    {
        EL<TEdit>("TfTemp_Set", "edtIdleTime_Mid")->Text       =FormatFloat("0.0", iBoostEotToSotTime);
        EL<TEdit>("TfTemp_Set", "edtBoostOffset_Mid")->Text    =FormatFloat("0.0", Temperature.dBoostOffset[iTriggerBoostFuncBack]);
        EL<TEdit>("TfTemp_Set", "edtBoostDuration_Mid")->Text  =FormatFloat("0.0", Temperature.dBoostDuration[iTriggerBoostFuncBack]);
        EL<TEdit>("TfTemp_Set", "edtPostBoost_Mid")->Text      =FormatFloat("0.0", Temperature.dPostBoostDuration[iTriggerBoostFuncBack]);
    }
    else
    {
        EL<TEdit>("TfTemp_Set", "edtIdleTime_Mid")->Text       =0;
        EL<TEdit>("TfTemp_Set", "edtBoostOffset_Mid")->Text    =0;
        EL<TEdit>("TfTemp_Set", "edtBoostDuration_Mid")->Text  =0;
        EL<TEdit>("TfTemp_Set", "edtPostBoost_Mid")->Text      =0;
    }
    EL<TEdit>("TfTemp_Set", "edtIdleTime_Short")->Text     =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBMin]);
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_Short")->Text  =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBMin]);
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_Short")->Text=FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBMin]);
    EL<TEdit>("TfTemp_Set", "edtPostBoost_Short")->Text    =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBMin]);
    EL<TRadioGroup>("TfTemp_Set", "rgBoostMode")->ItemIndex      =Temperature.iBoostFunctionMode;

    EL<TCheckBox>("TfTemp_Set", "chkLBInitOffset")->Checked  =Temperature.bEnableBoostOffset[Temperature.eBLBI];
    EL<TEdit>("TfTemp_Set", "edtIdleTime_LB")->Text      =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBI]);                       //Steven 20181023 : LB溫度
    EL<TEdit>("TfTemp_Set", "edtBoostOffset_LB")->Text   =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBI]);
    EL<TEdit>("TfTemp_Set", "edtBoostDuration_LB")->Text =FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBLBI]);
    EL<TEdit>("TfTemp_Set", "edtPostBoost_LB")->Text     =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBLBI]);

    EL<TCheckBox>("TfTemp_Set", "chkBoostOffset")->Checked   =Temperature.bEnableBoostOffset[Temperature.eBLBL];
    EL<TEdit>("TfTemp_Set", "edtLBTempMin")->Text        =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBL]);
    EL<TEdit>("TfTemp_Set", "edtLBTempOffset")->Text     =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBL]);
    EL<TEdit>("TfTemp_Set", "edtThreshold")->Text        =FormatFloat("0.0", Temperature.dThreshold);      //Steven 20190928 : L/B升溫的門檻值
    EL<TEdit>("TfTemp_Set", "edtLBTimeOut")->Text        =FormatFloat("0.0", Temperature.dBoostTimeOut);   //Steven 20181222 : Add LB升溫的Time out

    EL<TCheckBox>("TfTemp_Set", "chkBoostOffset")->Checked   =Temperature.bEnableBoostOffset[Temperature.eBLBB];
    EL<TEdit>("TfTemp_Set", "edtBoostTempMin")->Text     =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBB]);                       //Steven 20181023 : LB溫度
    EL<TEdit>("TfTemp_Set", "edtBoostOffset")->Text      =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBB]);
    //<==
    //Steven 20180817 : Boost Function

    EL<TEdit>("TfTemp_Set", "edChamberCoolTemp")->Text=FormatFloat("0.0", Temperature.fChamberCoolTemp);   //Steven 20111209 : Chamber降溫溫度
    EL<TCheckBox>("TfTemp_Set", "chkTesterDock")->Checked=Temperature.bUseTesterDocking;                       //jou 2014-04-25 Tester Docking Wait delay time
    EL<TEdit>("TfTemp_Set", "edtTestDock")->Text=Temperature.iUseTesterDocking;                            //jou 2014-04-25 Tester Docking Wait delay time    //Richard 20230322 : limit修改

    EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked   =Temperature.iTempMode & 0x01;
    EL<TRadioButton>("TfTemp_Set", "rb2Point")->Checked   =Temperature.iTempMode & 0x02;
    EL<TRadioButton>("TfTemp_Set", "rb3Point")->Checked   =Temperature.iTempMode & 0x04;
    EL<TRadioButton>("TfTemp_Set", "rb5Point")->Checked   =Temperature.iTempMode & 0x08;                          //kevin 20140918
    EL<TRadioButton>("TfTemp_Set", "rb6Point")->Checked   =Temperature.iTempMode & 0x10;                          //Ztex 2024.07.27 Add 6 Point Temperature Offset    //Ifor 20250704 Fix 0x16 => 0x10

    EL<TEdit>("TfTemp_Set", "edAbLow")->Text           =FormatFloat("0.0",Temperature.fAmbientHotLowBase);                                         //kevin 20140918
    EL<TEdit>("TfTemp_Set", "edAbMid")->Text           =FormatFloat("0.0",Temperature.fAmbientHotMiddBase);                                        //kevin 20140918
    EL<TEdit>("TfTemp_Set", "edLowBase")->Text         =FormatFloat("0.0",Temperature.fLowBase);
    EL<TEdit>("TfTemp_Set", "edMidBase")->Text         =FormatFloat("0.0",Temperature.fMiddBase);
    EL<TEdit>("TfTemp_Set", "edHighBase")->Text        =FormatFloat("0.0",Temperature.fHighBase);
    EL<TEdit>("TfTemp_Set", "edSHighBase")->Text       =FormatFloat("0.0",Temperature.fSHighBase);         //Ztex 2024.07.27 Add 6 Point Temperature Offset
    EL<TRadioGroup>("TfTemp_Set", "rgZ1Down")->ItemIndex     =Temperature.bZ2DownSocket;                         //kevin 201312111 使用加熱z2在下
    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edLowbase->Text        =FormatFloat("0.0", Temperature.fTempOffSet[LowBase][i]);
        myTempPal[i]->edBase->Text           =FormatFloat("0.0", Temperature.fTempOffSet[MidBase][i]);
        myTempPal[i]->edHighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[HigBase][i]);
        myTempPal[i]->edInitTempOffset->Text =FormatFloat("0.0", Temperature.fTempOffSet[InitTempOffset][i]);           //Steven 20141117 : 起測時溫度要補Offset
        myTempPal[i]->edLow->Text  =FormatFloat("0.0", Temperature.fTempOffSet[AmbientHotLow][i]);                      //kevin 20140918 恆溫控制 低 offset
        myTempPal[i]->edMid->Text  =FormatFloat("0.0", Temperature.fTempOffSet[AmbientHotMid][i]);                      //kevin 20140918 恆溫控制 中 offset
        myTempPal[i]->edEOTTempOffset->Text  =FormatFloat("0.0", Temperature.fTempOffSet[TestOverTimeTempOffset][i]);   //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        myTempPal[i]->edSHighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[SHigBase][i]);                //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            myTempPal[i]->edKit_Lowbase->Text        =FormatFloat("0.0", Temperature.fTempOffSet[KitLowBase][i]);
            myTempPal[i]->edKit_Base->Text           =FormatFloat("0.0", Temperature.fTempOffSet[KitMidBase][i]);
            myTempPal[i]->edKit_HighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[KitHigBase][i]);
            myTempPal[i]->edKit_Low->Text  =FormatFloat("0.0", Temperature.fTempOffSet[KitAmbientHotLow][i]);           //kevin 20140918 恆溫控制 低 offset
            myTempPal[i]->edKit_Mid->Text  =FormatFloat("0.0", Temperature.fTempOffSet[KitAmbientHotMid][i]);           //kevin 20140918 恆溫控制 中 offset
        }
    }

    EL<TCheckBox>("TfTemp_Set", "cbTempOffset")->Checked           =Temperature.bEnableTempOffsetForInitial;   //Steven 20141117 : 起測時溫度要補Offset
    EL<TEdit>("TfTemp_Set", "edTempOffsetCount")->Text         =Temperature.iCintactCntForTempOffsetAtInitial;                                     //Steven 20141117 : 起測時溫度要補Offset
    EL<TEdit>("TfTemp_Set", "edTempDownContactDelay")->Text    =Temperature.iCintactDelayCntForInitTempOffset;                                     //Steven 20141117 : 起測時溫度要補Offset
    EL<TEdit>("TfTemp_Set", "edTempReadyRange")->Text          =Temperature.iTempReadyRange;               //Sam 20231214 : Temp offset use ready temp range

    EL<TCheckBox>("TfTemp_Set", "chkChamberBoost")->Checked        =Temperature.bEnableChamberBoost;           //Steven 20191128 : Chamber Boost Function
    EL<TEdit>("TfTemp_Set", "edtChamberBoostTime")->Text       =Temperature.iChamberBoostTime;
    EL<TEdit>("TfTemp_Set", "edtChamberBoostOffset")->Text     =Temperature.iChamberBoostOffset;

    if(CUSTOMER_CODE==CC_SCS)                                                   //jou 2015-12-10 SCS 要求把ATC PID頁面關掉不顯示
    {
        EL<TCheckBox>("TfTemp_Set", "cdATCUsePIDCtrl")->Checked=false;
        Temperature.bUsePIDControl=false;
    }
    else
    {
        //Ifor 20150910 :ATC PID 顯示前轉換
        EL<TCheckBox>("TfTemp_Set", "cdATCUsePIDCtrl")->Checked  = Temperature.bUsePIDControl;
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinP")->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[0]/100.0);
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinI")->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[1]/100.0);
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinD")->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[2]/100.0);
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxP")->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[0]/100.0);
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxI")->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[1]/100.0);
        EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxD")->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[2]/100.0);
    }

    EL<TCheckBox>("TfTemp_Set", "cbHeaterGun")->Checked=Temperature.bActiveHeatGun;                            //Steven 20120525 : 熱風槍
    EL<TCheckBox>("TfTemp_Set", "chkUseCDAOnly")->Checked=Temperature.bUseCDAOnly;                             //Steven 20181012 : 使用熱風槍吹冷風
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_H")->Text     =FormatFloat("0", Temperature.iHotGunFLowLimit_H);                                   //KaiChen 20190729 ：Hot Gun Flow
    EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_L")->Text     =FormatFloat("0", Temperature.iHotGunFLowLimit_L);                                   //KaiChen 20190729 ：Hot Gun Flow

    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edSingleLimit->Text=FormatFloat("0.0", IniConfig.dSingleTempLimit[i]);
    }
    EL<TEdit>("TfTemp_Set", "edAmbHotGuartbent")->Text     =FormatFloat("0.0", Temperature.fAmbientHotGuartbent);                                  //kevin 20140918 恆溫控制溫度
    EL<TCheckBox>("TfTemp_Set", "chkShuttleNoHeatUp")->Checked =Temperature.bShuttleNoHeatUp;                  //Steven 20180815 : Amb Ctr mode, shuttle no heat up
    EL<TCheckBox>("TfTemp_Set", "chkSLKNoHeatUp")->Checked     =Temperature.bSLKNoHeatUp;                      //Steven 20230221 : Amb Ctr mode, SLK no heat up
    EL<TCheckBox>("TfTemp_Set", "cbUseAmbGuard")->Checked      =Temperature.bAmbientGuardbandCheck;            //kevin 20180115 (Steven) add ambient grand bient check
    EL<TEdit>("TfTemp_Set", "edAmbGuardband")->Text        =FormatFloat("0", Temperature.iAmbGuardband);   //kevin 20180115 (Steven) add ambient grand bient check

    EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked    =Temperature.bATC70Active;                      //Eliot 2015_0105
    EL<TCheckBox>("TfTemp_Set", "rbATC7TSDFunction")->Checked  =Temperature.bATC7TSDFunction;                  //Eliot 2015_0105
    EL<TEdit>("TfTemp_Set", "edTSDTimeOut")->Text          =Temperature.dATC7TSDTimeOut;                   //Steven 20160604 : by site TSD

    EL<TEdit>("TfTemp_Set", "edArm1Offset")->Text= Temperature.Arm1Offset;
    EL<TEdit>("TfTemp_Set", "edArm2Offset")->Text= Temperature.Arm2Offset;

    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_1")->Text=Temperature.Arm1NoFullsiteOffset_1;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_2")->Text=Temperature.Arm1NoFullsiteOffset_2;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_3")->Text=Temperature.Arm1NoFullsiteOffset_3;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_4")->Text=Temperature.Arm1NoFullsiteOffset_4;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_5")->Text=Temperature.Arm1NoFullsiteOffset_5;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_1")->Text=Temperature.Arm2NoFullsiteOffset_1;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_2")->Text=Temperature.Arm2NoFullsiteOffset_2;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_3")->Text=Temperature.Arm2NoFullsiteOffset_3;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_4")->Text=Temperature.Arm2NoFullsiteOffset_4;          //wei 20160107 No FullSite Add Offset
    EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_5")->Text=Temperature.Arm2NoFullsiteOffset_5;          //wei 20160107 No FullSite Add Offset

    EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Text=ATCInterfaceForm->iCheckSameTempTime;           //Ifor 20160308 設定溫度相同連續時間 Alarm

    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOn")->Text =Temperature.dSocketAirCoolingOnTimer;           //jou 2016-04-28 Socket Air Cooling contact count trun on
    EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOff")->Text=Temperature.dSocketAirCoolingOffTimer;          //jou 2016-04-28 Socket Air Cooling contact count trun on

    EL<TCheckBox>("TfTemp_Set", "cbByPassChamber")->Checked   =Temperature.bByPassChamber;                     //KenHsieh 20230301 : By Pass Chamber
    EL<TEdit>("TfTemp_Set", "edATCOfsTime")->Text=Temperature.iATC_OFS_ST;                                 //Ifor 20160308 設定溫度相同連續時間 Alarm

    EL<TCheckBox>("TfTemp_Set", "cbPowerFollow_Enable")->Checked  =Temperature.bPowerFollower_Enable;          //KenHsieh 20240216 : add ATC Power Follow Function
    EL<TEdit>("TfTemp_Set", "edtPowerFollow_FullPower")->Text =Temperature.iPowerFollower_FullPower;       //KenHsieh 20240216 : add ATC Power Follow Function
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_PFSlope")->Text =FormatFloat("0.000", Temperature.dPowerFollower_PFSlope);                           //KenHsieh 20240216 : add ATC Power Follow Function
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_WGain")->Text   =FormatFloat("0.000", Temperature.dPowerFollower_WGain);                             //KenHsieh 20240216 : add ATC Power Follow Function
    EL<TEdit>("TfTemp_Set", "edtPowerFollower_Many2one")->Text=Temperature.iPowerFollower_Many2one;        //KenHsieh 20240216 : add ATC Power Follow Function
//Steven 20180815 : Initial Temp Offset-----------------------------------------
//==>
    EL<TCheckBox>("TfTemp_Set", "cbEveryFirstDeviceUseInitialDelay")->Checked          =TestIF_File.bEveryFirstDeviceUseInitialDelay;
    EL<TCheckBox>("TfTemp_Set", "cbUseOtherArmToTestAfterInitialDelay")->Checked       =TestIF_File.bUseOtherArmToTestAfterInitialDelay;               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    EL<TCheckBox>("TfTemp_Set", "cbAfterShowAlarmMessageUseInitialDelay")->Checked     =TestIF_File.bAfterShowAlarmMessageUseInitialDelay;
    EL<TCheckBox>("TfTemp_Set", "cbWhenHappenTestedTimeBelowUseInitialDelay")->Checked =TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay;
    EL<TCheckBox>("TfTemp_Set", "cbAfterAutoCleanFunctionUseInitialDelay")->Checked    =TestIF_File.bAfterAutoCleanFunctionUseInitialDelay;            //ChungHung 20141017 add for SCK Add for TSMC Device
    EL<TCheckBox>("TfTemp_Set", "cbAfterOpenHeatDoorUseInitialDelay")->Checked         =TestIF_File.bAfterOpenHeatDoorUseInitialDelay;                 //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    EL<TEdit>("TfTemp_Set", "edBelowSec")->Text                                    =FormatFloat("0.00", TestIF_File.iEveryFirstDeviceUseInitialDelay);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay);                //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_2);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_3);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_4);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_5);              //ChungHung 20141210 add for SCK want to every event have delay

    EL<TCheckBox>("TfTemp_Set", "cbWhenPressStopOverUseInitialDelay")->Checked         =TestIF_File.bWhenPressStopOverUseInitialDelay;                 //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    EL<TEdit>("TfTemp_Set", "edOverSec")->Text                                     =FormatFloat("0.00", TestIF_File.iWhenPressStopOver);           //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_6);              //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    EL<TCheckBox>("TfTemp_Set", "cbWhenNoFullSiteUseInitialDelay")->Checked            =TestIF_File.bWhenNoFullSiteUseInitialDelay;                    //wei 20151228 No FullSite delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_7);              //wei 20151228 No FullSite delay

    EL<TCheckBox>("TfTemp_Set", "cbTestFinishToNextTestOver")->Checked                 =TestIF_File.bTestFinishToNextTestOver;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    EL<TEdit>("TfTemp_Set", "iTestFinishToNextTestOver")->Text                     =FormatFloat("0.00", TestIF_File.iTestFinishToNextTestOver);    //kevin 20160310
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_8);              //kevin 20160310

    EL<TCheckBox>("TfTemp_Set", "cbOTDUnlockDelay")->Checked                           =TestIF_File.bOTDUnlockDelay;                                   //Steven 20160818 : OTD打開Delay
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_9);              //Steven 20160818 : OTD打開Delay
    EL<TCheckBox>("TfTemp_Set", "cbTestStartToNextTestStart")->Checked                 =TestIF_File.bTestStartToNextTestStart;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    EL<TEdit>("TfTemp_Set", "iTeststartToNextTestStart")->Text                     =FormatFloat("0.00", TestIF_File.dTeststartToNextTestStart);    //kevin 20160310
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10")->Text                            =FormatFloat("0.00", TestIF_File.dInitialDelay_10);             //kevin 20181031 : SOT 超過時間Delay

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_1_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_1_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_2_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_2_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_3_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_3_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_4_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_4_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_5_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_5_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_6_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_6_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_7_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_7_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_8_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_8_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_9_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_9_RT);
    EL<TEdit>("TfTemp_Set", "edtInitialDelay_10_RT")->Text                         =FormatFloat("0.00", TestIF_File.dInitialDelay_10_RT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    EL<TCheckBox>("TfTemp_Set", "cbEnableTJFunction")->Checked=Temperature.bEnableTJFunction;                  //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    EL<TRadioGroup>("TfTemp_Set", "rgTjMapType")->ItemIndex=Temperature.iTjSiteMapType;                          //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    if(Temperature.iTjSiteMapType==2)
    {
        EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Visible=true;
    }
    else
    {
        EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Visible=false;
    }

    EL<TEdit>("TfTemp_Set", "edtSetTJ_Offset")->Text=FormatFloat("0.00", Temperature.fSetTJ_Offset);
    EL<TEdit>("TfTemp_Set", "edtSetTJ_Slope")->Text=FormatFloat("0.00", Temperature.fSetTJ_Slope);

    EL<TEdit>("TfTemp_Set", "edtInputVLow")->Text=FormatFloat("0.00", Temperature.fTjInputVLow);
    EL<TEdit>("TfTemp_Set", "edtInputVHigh")->Text=FormatFloat("0.00", Temperature.fTjInputVHigh);
    EL<TEdit>("TfTemp_Set", "edtDelayAfterSOT")->Text=FormatFloat("0.00",Temperature.dDelayAfterSOT);
    EL<TEdit>("TfTemp_Set", "edContinuousSec")->Text=FormatFloat("0.00",Temperature.dDelayAfterSOTContinue);                                       //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    EL<TCheckBox>("TfTemp_Set", "cbTjWatchdog")->Checked= Temperature.bTjWatchdog;

    for(int i=0; i<16; i++)
    {
        EL<filerw::ELStringGrid>("TfTemp_Set", "sgTjMap")->Cells[i+1][1]=sATC_CH_Tj->Strings[i].c_str();
    }

    //Ifor 20190730 : add FFC Temperature Offset
    //==>
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<10; j++)
        {
            ATC_FFCOffsetOnTimeEdit[i][j]->Text   =Temperature.iATC_FFCTimeOn[i][j];
            ATC_FFCOffsetOffTimeEdit[i][j]->Text  =Temperature.iATC_FFCTimeOff[i][j];
            ATC_FFCOffsetEdit[i][j]->Text         =Temperature.dATC_FFCOffset[i][j];
            ATC_FFCPointUse[i][j]->Checked        =Temperature.bATC_FFCPointUse[i][j];
        }
    }
    EL<TCheckBox>("TfTemp_Set", "cbFFCOffsetEnable")->Checked  =Temperature.bATC_FFCEnable;

    EL<TCheckBox>("TfTemp_Set", "cbMultiZoneFunction")->Checked=Temperature.bMultiZoneEnable;                  //wei 20240617 Multi Zone
    for(int i=0; i<4; i++)
    {
        ZoneTempUse[i]->Checked=Temperature.bZoneTempEnable[i];
        ZoneTempSetting[i]->Text=Temperature.dZoneTempSetting[i];
    }

    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTemp")->Text     =FormatFloat("0.0", Temperature.dATC_HotGunTemp);                                       //Ifor 20220124 add: ATC Hot Air
    EL<TEdit>("TfTemp_Set", "edtATC_HotGunTime")->Text     =FormatFloat("0.0", Temperature.dATC_HotGunTime);                                       //Ifor 20220124 add: ATC Hot Air

    if(IniConfig.bL22Enable3SigmaTempMonitor ==true)
    {
        EL<TGroupBox>("TfTemp_Set", "gbcb3SigmaTempMonitior")->Visible = true;
        EL<TCheckBox>("TfTemp_Set", "cb3SigmaTempMonitior_Enable")->Checked  = Temperature.b3SigmaTempMonitior_Enable;                                 //kevin 20200521 add 3 Sigma 溫度記錄比對
        if(Temperature.i3SigmaTempMonitior_SetCoun<5)
        {
            Temperature.i3SigmaTempMonitior_SetCoun = 5;
        }
        EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_SetCount")->Text            = IntToStr(Temperature.i3SigmaTempMonitior_SetCoun);            //kevin 20200521 add 3 Sigma Index Contract time
        //edt3SigmaTempMonitior_NowCount->Text          = IntToStr(Temperature.i3SigmaTempMonitior_NowCount);  //kevin 20200629 mark //kevin 20200521 add 3 Sigma Index 目前 Contract time
        EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue")->Text     = FloatToStr(Temperature.d3SigmaTempMonitior_Set3xSigmaValue);  //kevin 20200521 add 3 Sigma 客戶比對誤差值
    }
    else
    {
        EL<TGroupBox>("TfTemp_Set", "gbcb3SigmaTempMonitior")->Visible = false;
        Temperature.b3SigmaTempMonitior_Enable = false;
    }
    //Hmy 20200515 Copy//Hmy 20200510 Add Enable 3 Sigma Temp Monitor//Hmy 20200510 Add Enable 3 Sigma Temp Monitor <-
    //<==
    //Ifor 20190730 : add FFC Temperature Offset

    EL<TEdit>("TfTemp_Set", "edLBTempHighSettingValue")->Text         =FormatFloat("0.0", Temperature.dLBTempHighSettingValue);                    //Frank 20241231 : add
    EL<TEdit>("TfTemp_Set", "edLBTempLowSettingValue")->Text          =FormatFloat("0.0", Temperature.dLBTempLowSettingValue);
    EL<TCheckBox>("TfTemp_Set", "cbLBTempHighAlarmEnable")->Checked       =Temperature.bLBTempHighAlarm_Enable;
    EL<TCheckBox>("TfTemp_Set", "cbLBTempLowAlarmEnable")->Checked        =Temperature.bLBTempLowAlarm_Enable;

//<==
//Steven 20180815 : Initial Temp Offset-----------------------------------------

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableOutSht")->Checked=Temperature.bUseTriTempHeater_Ini[0];
        EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableIndex") ->Checked=Temperature.bUseTriTempHeater_Ini[1];
        EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableBase")  ->Checked=Temperature.bUseTriTempHeater_Ini[2];
    }                                                                           //Heater開關設定

    EL<TCheckBox>("TfTemp_Set", "cbMultiSensorOffsetEnable")->Checked  =Temperature.bATC_MultiSensorEnable;    //Ifor 20251203 add:ATC Multi Sensor Offset

    for(int i=0; i<4; i++)                                                      //Ifor 20251203 add:ATC Multi Sensor Offset
    {
        MultiSensorOffsetUse[i]->Checked=Temperature.bATC_MultiSensorUse[i];
    }

    for(int i=0; i<32; i++)
    {                                                                           //Ifor 20251203 add:ATC Multi Sensor Offset
        ATC_MultiSensorOffsetEdit[i]->Text=Temperature.dATC_MultiSensorOfs[i];
    }

}

// golden uTemp_Set.cpp:3708  TfTemp_Set::UpDateEdit()
static void TS_UpDateEdit()
{

    int x, y;
//    int iRowPos[16]={  6,  30,  54,  78, 102, 126, 150, 174,
//                     198, 222, 246, 270, 294, 318, 342, 366};

    int iShowNum[2][16]={{-1, -1, -1, -1, -1, -1, -1, -1,
                          -1, -1, -1, -1, -1, -1, -1, -1},
                         {-1, -1, -1, -1, -1, -1, -1, -1,
                          -1, -1, -1, -1, -1, -1, -1, -1}};

    int iAddr[2][16]={{tcAa1, tcBa1, tcAb1, tcBb1, tcAc1, tcBc1, tcAd1, tcBd1,
                       tcAe1, tcBe1, tcAf1, tcBf1, tcAg1, tcBg1, tcAh1, tcBh1},
                      {tcAa2, tcBa2, tcAb2, tcBb2, tcAc2, tcBc2, tcAd2, tcBd2,
                       tcAe2, tcBe2, tcAf2, tcBf2, tcAg2, tcBg2, tcAh2, tcBh2}};

    if(CosFunction.bTemp5PointKitOffset)                                        //JerryYang 20221003 Analog要求新增Kit三點offset
    {
        EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->Visible=true;
        if(EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->ItemIndex==0)
        {
            ;   // UI-only（HTML 端處理）：EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->Color=(TColor)0x00C2B8A6;
        }
        else
        {
            ;   // UI-only（HTML 端處理）：EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->Color=clMedGray;
        }
    }
    else
    {
        EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->Visible=false;
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edInitTempOffset->Visible=true;
        myTempPal[i]->edEOTTempOffset->Visible=true;
        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            if(EL<TRadioGroup>("TfTemp_Set", "rgBasePoint")->ItemIndex==1)
            {
                myTempPal[i]->edKit_Low->Visible=true;
                myTempPal[i]->edKit_Mid->Visible=true;
                myTempPal[i]->edKit_Lowbase->Visible=true;
                myTempPal[i]->edKit_HighBase->Visible=true;
                myTempPal[i]->edKit_Base->Visible=true;

                myTempPal[i]->edKit_Low->Enabled=fSecurity->Insufficient(175, false);                                   //JerryYang 20230822 : 174->175
                myTempPal[i]->edKit_Mid->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_Lowbase->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_HighBase->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_Base->Enabled=fSecurity->Insufficient(175, false);

                myTempPal[i]->edLowbase->Visible=false;
                myTempPal[i]->edHighBase->Visible=false;
                myTempPal[i]->edBase->Visible=false;

                myTempPal[i]->edOffset->Visible=false;
                myTempPal[i]->edSingleLimit->Visible=false;
                myTempPal[i]->edIndiTemp->Visible=false;
                myTempPal[i]->edInitTempOffset->Visible=false;
                myTempPal[i]->edEOTTempOffset->Visible=false;
                myTempPal[i]->edSHighBase->Visible=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
            else
            {
                myTempPal[i]->edKit_Low->Visible=false;
                myTempPal[i]->edKit_Mid->Visible=false;
                myTempPal[i]->edKit_Lowbase->Visible=false;
                myTempPal[i]->edKit_HighBase->Visible=false;
                myTempPal[i]->edKit_Base->Visible=false;
                myTempPal[i]->edLow->Visible=true;
                myTempPal[i]->edMid->Visible=true;
                myTempPal[i]->edLowbase->Visible=true;
                myTempPal[i]->edHighBase->Visible=true;
                myTempPal[i]->edBase->Visible=true;

                myTempPal[i]->edOffset->Visible=true;
                myTempPal[i]->edSingleLimit->Visible=true;
                myTempPal[i]->edIndiTemp->Visible=true;
                myTempPal[i]->edInitTempOffset->Visible=true;
                myTempPal[i]->edEOTTempOffset->Visible=true;
                myTempPal[i]->edSHighBase->Visible=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }
        else
        {
            myTempPal[i]->edLow->Visible=true;
            myTempPal[i]->edMid->Visible=true;
            myTempPal[i]->edLowbase->Visible=true;
            myTempPal[i]->edHighBase->Visible=true;
            myTempPal[i]->edBase->Visible=true;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
            myTempPal[i]->edKit_Lowbase->Visible=false;
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Base->Visible=false;
        }

        for(int j=0; j<32; j++)                                                 //JerryYang 20190926 修正溫度Offset不見的問題
        {
            if(myTempPal[i]->iIndexTag==j)
            {
                myTempPal[i]->palTemp->Visible=true;
            }
        }
    }

    myTempPal[tcHotPlate3]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcHotPlate4]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcShuttle3 ]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcShuttle4 ]->palTemp->Visible=(Tri_Temp_Machine==1);

    if(EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edBase->Visible=false;
            myTempPal[i]->edHighBase->Visible=false;
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Base->Visible=false;                            //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(EL<TRadioButton>("TfTemp_Set", "rb2Point")->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edBase->Visible=false;
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Base->Visible=false;                            //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(EL<TRadioButton>("TfTemp_Set", "rb3Point")->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Low->Visible=false;                             //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(EL<TRadioButton>("TfTemp_Set", "rb6Point")->Checked)                                                  //Ztex 2024.07.27 Add 6 Point Temperature Offset ==>
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Visible=true;
            myTempPal[i]->edMid->Visible=true;
            myTempPal[i]->edLowbase->Visible=true;
            myTempPal[i]->edHighBase->Visible=true;
            myTempPal[i]->edBase->Visible=true;
            myTempPal[i]->edSHighBase->Visible=true;

            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
            myTempPal[i]->edKit_Lowbase->Visible=false;
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Base->Visible=false;
        }
    }                                                                           //Ztex 2024.07.27 Add 6 Point Temperature Offset <==

    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08)                                        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {
        for(int j=tcHead1; j<=tcHead4; j++)
        {
            myTempPal[j]->palTemp->Visible=false;
        }
    }

    if(USE_16_HEATER==eht16Heater       ||
       USE_16_HEATER==eht16HeaterEJ1N   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08)                                        //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
    {
        for(int j=tcAe1; j<=tcBh2; j++)
        {
            myTempPal[j]->palTemp->Visible=false;
        }
    }

    myTempPal[tcChamber]->palTemp->Visible=(myTempPal[tcChamber]->palTemp->Visible && ATC_SYSTEM<=eATC60);
    myTempPal[tcCCD]->palTemp->Visible=false;

    if(REAL_TIME_CCD==false)                                                    //kevin 20210803
    {
        myTempPal[tcCCD_2]->palTemp->Visible=false;
        myTempPal[tcCCD_2]->edIndiTemp->Visible=false;
    }

    if(REAL_TIME_CCD==true && RTC_TemperNumber==2)                              //Isaac 20201217 : RTC CCD增加第二組感溫
    {
        myTempPal[tcCCD_2]->palTemp->Visible=false;
    }
    myTempPal[tc2D]->palTemp->Visible=false;

    if(iSocketBaseTempCount==eDut2ea)                                           //Steven 20140116 : Socket數量,改用數字設定
    {
        myTempPal[tcDUT4]->palTemp->Visible=false;
        myTempPal[tcDUT3]->palTemp->Visible=false;
        myTempPal[tcSocket]->palTemp->Visible=false;
        EL<TControl>("TfTemp_Set", "Image2")->Visible=false;
    }
    else if(iSocketBaseTempCount==eDut4ea)                                      //Steven 20141120 : Fixed 溫度設定
    {
        myTempPal[tcSocket]->palTemp->Visible=false;
    }

    if(Index_ESDAir ==false)                                                    //kevin 20200210 add index ESD temp
       myTempPal[tcIndexESD]->palTemp->Visible=false;

    if(CosFunction.bUseIndividulTempSet)
        TS_DisplayTargetTempEdit(Temperature.bUseIndividualTemp);
    else
        TS_DisplayTargetTempEdit(false);

    if(CosFunction.bUseOldATCTempOffset==false &&                               //JerryYang 20190123 ATC Offset UI使用舊版
       bNeedChange==true)
    {
        if(ATC_SYSTEM==eNewATCSystem &&
           Temperature.bATCActiveCooling)                                       //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
        {
            for(int i=tcAa1; i<=tcBd2; i++)
            {
                if(CosFunction.bATCUseTempAdjustment==true &&
                   bUT150Install[i]==true)                                      //Ifor 20190215 : add ATC 使用 三點校正功能
                {
                }
                else
                {
                    myTempPal[i]->edLow->Visible=false;
                    myTempPal[i]->edMid->Visible=false;
                    myTempPal[i]->edLowbase->Visible=false;
                    myTempPal[i]->edBase->Visible=false;
                    myTempPal[i]->edHighBase->Visible=false;
                    myTempPal[i]->edSHighBase->Visible=false;                   //Ztex 2024.07.27 Add 6 Point Temperature Offset
                }
            }

            for(int i=tcAe1; i<=tcBh2; i++)
            {
                if(CosFunction.bATCUseTempAdjustment==true &&
                   bUT150Install[i]==true)                                      //Ifor 20190215 : add ATC 使用 三點校正功能
                {
                }
                else
                {
                    myTempPal[i]->edLow->Visible=false;
                    myTempPal[i]->edMid->Visible=false;
                    myTempPal[i]->edLowbase->Visible=false;
                    myTempPal[i]->edBase->Visible=false;
                    myTempPal[i]->edHighBase->Visible=false;
                    myTempPal[i]->edSHighBase->Visible=false;                   //Ztex 2024.07.27 Add 6 Point Temperature Offset
                }
            }
        }

        for(int i=0; i<tcTotalCount; i++)
        {
            for(int j=0; j<32; j++)
            {
                if(myTempPal[i]->iIndexTag==j)
                {
    ;   // golden uTemp_Set.cpp:3989-3989 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:3989-3989 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                    myTempPal[i]->palTemp->Align=alBottom;
#endif // GATE (S12-C save)
                }
            }
        }

        if(EL<TSpeedButton>("TfTemp_Set", "btnSort")->Tag<2)                                                      //Steven 20181119 : 新增溫度補償值可以排序
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<16; j++)
                {
                    if(EL<TSpeedButton>("TfTemp_Set", "btnSort")->Tag==0)
                    {
                        x=iAddr[i][j];
                        if(i==0)
                        {
    ;   // golden uTemp_Set.cpp:4005-4005 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4005-4005 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                            myTempPal[x]->palTemp->Align=alTop;
#endif // GATE (S12-C save)
                        }
                        else
                        {
    ;   // golden uTemp_Set.cpp:4009-4009 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4009-4009 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                            myTempPal[x]->palTemp->Align=alTop;
#endif // GATE (S12-C save)
                        }
                    }
                    else if(EL<TSpeedButton>("TfTemp_Set", "btnSort")->Tag==1)
                    {
                        x=iAddr[i][j];
                        iShowNum[i][j]=j/2+j%2*8;
                    }
                }
            }

            if(EL<TSpeedButton>("TfTemp_Set", "btnSort")->Tag==1)
            {
                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
    ;   // golden uTemp_Set.cpp:4031-4031 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4031-4031 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                                myTempPal[x]->palTemp->Align=alTop;
#endif // GATE (S12-C save)
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(TestIF.iTestMode==_32Site4X8N)
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<8; j++)
                    {
                        x=i+j*2;
                        if(TestIF_File.iSiteMap[i][j]>0)
                        {
                            iShowNum[0][x]=TestIF_File.iSiteMap[i][j]-1;
                        }
                        else
                        {
                            iShowNum[0][x]=-1;
                        }

                        if(TestIF_File.iSiteMap[i+2][j]>0)
                        {
                            iShowNum[1][x]=TestIF_File.iSiteMap[i+2][j]-1;
                        }
                        else
                        {
                            iShowNum[1][x]=-1;
                        }
                    }
                }

                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
    ;   // golden uTemp_Set.cpp:4076-4076 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4076-4076 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                                myTempPal[x]->palTemp->Align=alTop;
#endif // GATE (S12-C save)
                            }
                            else if(iShowNum[i][j]==-1)
                            {
                                myTempPal[x]->palTemp->Visible=false;
                            }
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<8; j++)
                    {
                        y=i+j*2;
                        for(int k=0; k<2; k++)
                        {
                            x=iAddr[k][y];
                            if(TestIF_File.iSiteMap[i][j]>0)
                            {
                                iShowNum[k][y]=TestIF_File.iSiteMap[i][j]-1;
                            }
                            else
                            {
                                iShowNum[k][y]=-1;
                            }
                        }
                    }
                }

                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
    ;   // golden uTemp_Set.cpp:4117-4117 取代：palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4117-4117 -- palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）
                                myTempPal[x]->palTemp->Align=alTop;
#endif // GATE (S12-C save)
                            }
                            else if(iShowNum[i][j]==-1)
                            {
                                myTempPal[x]->palTemp->Visible=false;
                            }
                        }
                    }
                }
            }
        }
    //<==
    //Steven 20181119 : 新增溫度補償值可以排序
    }
    //Ifor 20161220 (Steven) add 初始化 溫度設定 For Even Log
    //==>
    dOldWorkTemp    =Temperature.fWorkTemperBase;
    dOldAmbWorkTemp =Temperature.fAbitTemp;
    dOldSockTime    =Temperature.fSoakTime;
    //<==
    //Ifor 20161220 (Steven) add 初始化 溫度設定 For Even Log

}

// golden uTemp_Set.cpp:7073  TfTemp_Set::cbATCReferTempSensorClick(TObject *Sender)
static void TS_cbATCReferTempSensorClick()
{

    if(EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked)
    {
        EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset")->Visible=false;
        EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset")->Checked=false;
    }

}

// golden uTemp_Set.cpp:5430  TfTemp_Set::rbATCActiveOnClick(TObject *Sender)
static void TS_rbATCActiveOnClick()
{

    //Ifor 20160412 New ATC System 整合ATC7.0
    if(ATC_SYSTEM==eNewATCSystem)
    {
        EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Visible    =false;
        EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked    = false;
    }
    else if(EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked ==true)
    {
        EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked    = false;
        EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Enabled    = false;
        EL<TCheckBox>("TfTemp_Set", "rbATC7TSDFunction")->Enabled  = false;
        EL<TEdit>("TfTemp_Set", "edTSDTimeOut")->Enabled       = false;
    }
    else
    {
        EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Enabled    = true;
        EL<TCheckBox>("TfTemp_Set", "rbATC7TSDFunction")->Enabled  = true;
        EL<TEdit>("TfTemp_Set", "edTSDTimeOut")->Enabled       = true;
    }

}

// golden uTemp_Set.cpp:5407  TfTemp_Set::rbATC70ActiveOnClick(TObject *Sender)
static void TS_rbATC70ActiveOnClick()
{

    if( EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked ==true)
    {
        EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked = false;
        EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Enabled = false;
        EL<TEdit>("TfTemp_Set", "edChillerTemp")->Enabled = false;
        EL<TEdit>("TfTemp_Set", "edtATCInPC1")->Enabled = false;
        EL<TEdit>("TfTemp_Set", "edtATCInPC2")->Enabled = false;
        EL<TEdit>("TfTemp_Set", "edtATCInPC3")->Enabled = false;
        EL<TEdit>("TfTemp_Set", "edtATCInPC4")->Enabled = false;
    }
    else
    {
        EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Enabled = true;
        EL<TEdit>("TfTemp_Set", "edChillerTemp")->Enabled = true;
        EL<TEdit>("TfTemp_Set", "edtATCInPC1")->Enabled = true;
        EL<TEdit>("TfTemp_Set", "edtATCInPC2")->Enabled = true;
        EL<TEdit>("TfTemp_Set", "edtATCInPC3")->Enabled = true;
        EL<TEdit>("TfTemp_Set", "edtATCInPC4")->Enabled = true;
    }

}

// golden uTemp_Set.cpp:4196  TfTemp_Set::FormClose(TObject *Sender, TCloseAction &Action)
static void TS_FormClose()
{

//    bool bHaveChange=false;
    fShow=false;

    EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->Enabled=true;                                            //jou 2012-05-04 沒有把Enabled打開,會出現無法force的錯誤

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden uTemp_Set.cpp:4238  TfTemp_Set::spbSaveClick(TObject *Sender)
static void TS_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden uTemp_Set.cpp:4244-4244 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4244-4244 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S="";
    int iSum=0;
    S=GetLastOpenFN();
    AnsiString szDir="", str="", szDir2="";
    if(CosFunction.bSaveTemperatureByMachine && IniConfig.bA57_2SaveTemperatureByMachine)                               //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir=DataPath+S;
    }
    //JerryYang 20170213 (Steven) 矽品騰清要求修改Run check觸發條件
    //==>
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        if(TS_CheckTempSettingChange()==true)
        {
            bHasSaveSet=true;
        }
        EventReport(SECS_EVENT.SaveRecipe);
    }
    //<==
    //JerryYang 20170213 (Steven) 矽品騰清要求修改Run check觸發條件

    //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
    //==>
    double dbSetTemp=0;
    bool bHasErrorSet=false;
    AnsiString asErrorAddr="";
//    if(CosFunction.bHiSiliconFunction==true || CUSTOMER_CODE==CC_KYEC_LEE)    //Ifor 20220216 add:開放 Check the temperature Offset limit
    {
        if(LastSet.iTemperature==Tempture_Ambient && Temperature.bATCActiveCooling)
        {
            dbSetTemp=atof(fMain->edATCAmbientTemper->Text.c_str());
        }
        else if(LastSet.iTemperature==Tempture_Ambient &&
                Temperature.bATCActiveCooling==false)                           //Sam 20220527 : 常溫下不需要檢查溫度極限
        {
            dbSetTemp=25;
        }
        else
        {
            dbSetTemp=atof(fMain->edWorkTemperBase->Text.c_str());
        }

        for(int i=0; i<tcTotalCount; i++)
        {
            if(bUT150Install[i]==true)
            {
                if(FormHS->CheckTempOffset(i, atof(myTempPal[i]->edLowbase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(FormHS->CheckTempOffset(i, atof(myTempPal[i]->edBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(FormHS->CheckTempOffset(i, atof(myTempPal[i]->edHighBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(FormHS->CheckTempOffset(i, atof(myTempPal[i]->edSHighBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))         //Ztex 2024.07.27 Add 6 Point Temperature Offset
                {
                    bHasErrorSet=true;
                }
            }
        }

        if(bHasErrorSet==true)
        {
    FormHS->CheckTempOffset(0, 0, true); filerw::ELMark("WAR15194 CheckTempOffset over limit -- not saved");   // golden uTemp_Set.cpp:4319-4319 取代：CheckTempOffset(0,0,true) 發 WAR15194（golden），另記 trace 讓頁面知道為什麼沒存
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4319-4319 -- CheckTempOffset(0,0,true) 發 WAR15194（golden），另記 trace 讓頁面知道為什麼沒存
            FormHS->CheckTempOffset(0, 0, true);
#endif // GATE (S12-C save)
            return;
        }
    }
    //<==
    //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20

    if(ATC_SYSTEM==eNewATCSystem &&
    (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36))   // golden uTemp_Set.cpp:4327-4327 取代：IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4327-4327 -- IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法
       ATC_InterfaceForm->IS_ATC33())                                           //Ifor 20230504 add:回溫中不可關閉回溫功能
#endif // GATE (S12-C save)
    {
        if(Temperature.bEnableTempRise==true && EL<TCheckBox>("TfTemp_Set", "cbATCTempRise")->Checked==false && bDoATCTempRise==true)
        {
            filerw::ELMessage("During the warm up process , the temperature warm up function cannot be disabled.", "執行回溫中不可關閉回溫功能");
            EL<TCheckBox>("TfTemp_Set", "cbATCTempRise")->Checked=true;
            return;
        }
    }
    //Ifor 20190730 : add FFC Temperature Offset
    //==>
    int iPrevious=0;
    bHasErrorSet=false;
    asErrorAddr="";
    str="";
    for(int i=0; i<2; i++)
    {
        iPrevious=0;
        for(int j=0; j<10; j++)
        {
            if(ATC_FFCPointUse[i][j]->Checked==true)
            {
                if(j>0)
                {
                    if(atoi(ATC_FFCOffsetOnTimeEdit[i][j]->Text.c_str())<=atoi(ATC_FFCOffsetOffTimeEdit[i][iPrevious]->Text.c_str()))                           //下一個Time On 須大於上一個 Time Off時間點
                    {
                        bHasErrorSet=true;
                        str.sprintf("Arm%d_P%d,", i+1, j+1);
                        asErrorAddr=asErrorAddr+str;
                    }
                }

                if(atoi(ATC_FFCOffsetOffTimeEdit[i][j]->Text.c_str())<=atoi(ATC_FFCOffsetOnTimeEdit[i][j]->Text.c_str()))                                       //下一個Time off 須大於 Time On時間點
                {
                    bHasErrorSet=true;
                    str.sprintf("Arm%d_P%d,", i+1, j+1);
                    asErrorAddr=asErrorAddr+str;
                }
                iPrevious=j;
            }
        }
    }

    if(bHasErrorSet==true)
    {
        str.sprintf("FFC Offset Setting error!!");
        filerw::ELMessage(str, asErrorAddr);
        return;
    }
    //<==
    //Ifor 20190730 : add FFC Temperature Offset
    TS_SaveSetupFile(szDir, S);                                                    //kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(S,"JOBFILE");
    #endif

    if(     EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked)  iSum+=1;
    else if(EL<TRadioButton>("TfTemp_Set", "rb2Point")->Checked)  iSum+=2;
    else if(EL<TRadioButton>("TfTemp_Set", "rb3Point")->Checked)  iSum+=4;
    else if(EL<TRadioButton>("TfTemp_Set", "rb5Point")->Checked)  iSum+=8;                                        //kevin 20141006
    else if(EL<TRadioButton>("TfTemp_Set", "rb6Point")->Checked)  iSum+=16;                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
    //-----------------------------------------------------------

    szDir.sprintf("%sDefineTemp\\", DefaultPath);                               //JerryYang 20170123 避免路徑不存在出現錯誤
    MyForceDirectories(szDir);
    //jou 2010-01-11 start : 效正溫度改固定一組
    if(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex==2)                                           //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
    }
    else
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }

    bool bReadType=(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex==HeadChamber)?true:false;        //Steven 20111102

    if(bReadType)                                                               //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber_ATC.Data", DefaultPath);
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }
    else
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\Temperature60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC.Data", DefaultPath);
                }
                else
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC_Cold.Data", DefaultPath);
                }

                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20161103 ase kaohsiung 林佳青只 使用一組  temp offset
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }
    else if(CosFunction.bTempCalByRecipe==true &&
            EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked==true)                                  //jou 20220725 : Temperature calibration by recipe
    {
        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\DefineTemperature.Data", DataPath, S);
    }

    szDir2.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);

    WriteIniData(szDir, "Mode",       "Points",      iSum);
    WriteIniData(szDir, "Low OffSet",  "Base",       EL<TEdit>("TfTemp_Set", "edLowBase")->Text);
    WriteIniData(szDir, "High OffSet", "Base",       EL<TEdit>("TfTemp_Set", "edHighBase")->Text);
    WriteIniData(szDir, "Mid. OffSet", "Base",       EL<TEdit>("TfTemp_Set", "edMidBase")->Text);
    WriteIniData(szDir, "SHigh OffSet","Base",       EL<TEdit>("TfTemp_Set", "edSHighBase")->Text);        //Ztex 2024.07.27 Add 6 Point Temperature Offset

    WriteIniData(szDir, "AmbientHotLowOffSet", "Base",       EL<TEdit>("TfTemp_Set", "edAbLow")->Text);    //kevin 20140918 恆溫控制
    WriteIniData(szDir, "AmbientHotMidOffSet", "Base",       EL<TEdit>("TfTemp_Set", "edAbMid")->Text);    //kevin 20140918 恆溫控制

    for(int i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);
        WriteIniData(szDir, "Low OffSet",           S, myTempPal[i]->edLowbase->Text);
        WriteIniData(szDir, "Mid. OffSet",          S, myTempPal[i]->edBase->Text);
        WriteIniData(szDir, "High OffSet",          S, myTempPal[i]->edHighBase->Text);
        WriteIniData(szDir, "SHigh OffSet",         S, myTempPal[i]->edSHighBase->Text);                                //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CosFunction.bInitTempOffsetByWorkFile==false)                        //jou 2015-06-13 Initial Temperture Offset by WorkFile
            WriteIniData(szDir, "Init Temp OffSet",     S, myTempPal[i]->edInitTempOffset->Text);                       //Steven 20141117 : 起測時溫度要補Offset

        WriteIniData(szDir, "AmbientHotLowOffSet",  S, myTempPal[i]->edLow->Text);                                      //kevin 20140918 恆溫控制
        WriteIniData(szDir, "AmbientHotMidOffSet",  S, myTempPal[i]->edMid->Text);                                      //kevin 20140918 恆溫控制
        WriteIniData(szDir, "TestOverTime Temp OffSet",  S, myTempPal[i]->edEOTTempOffset->Text);                       //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
    }
    //jou 2010-01-11 end

    if(IniConfig.bL13HotPlateAndShuttleUseOneTempOffset)                        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    {
        for(int i=tcHotPlate1; i<=tcShuttle2; i++)
        {
            S.printf("CH%d", i+1);
            WriteIniData(szDir2, "Low OffSet",          S, myTempPal[i]->edLowbase->Text);
            WriteIniData(szDir2, "Mid. OffSet",         S, myTempPal[i]->edBase->Text);
            WriteIniData(szDir2, "High OffSet",         S, myTempPal[i]->edHighBase->Text);
            WriteIniData(szDir2, "SHigh OffSet",        S, myTempPal[i]->edSHighBase->Text);                            //Ztex 2024.07.27 Add 6 Point Temperature Offset

            if(CosFunction.bInitTempOffsetByWorkFile==false)                    //jou 2015-06-13 Initial Temperture Offset by WorkFile
                WriteIniData(szDir2, "Init Temp OffSet",    S, myTempPal[i]->edInitTempOffset->Text);                   //Steven 20141117 : 起測時溫度要補Offset

            WriteIniData(szDir2, "AmbientHotLowOffSet", S, myTempPal[i]->edLow->Text);                                  //kevin 20140918 恆溫控制
            WriteIniData(szDir2, "AmbientHotMidOffSet", S, myTempPal[i]->edMid->Text);                                  //kevin 20140918 恆溫控制
            WriteIniData(szDir2, "TestOverTime Temp OffSet",  S, myTempPal[i]->edEOTTempOffset->Text);                  //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        }
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        WriteIniData(szDir2, "Mode",       "AmbCheck",    EL<TCheckBox>("TfTemp_Set", "cbUseAbitCHK")->Checked);

    //Steven 20120525 Start: 取消TempMonitor, 改到Temp_Set
    for(int i=0; i<tcTotalCount; i++)
    {
        IniConfig.dSingleTempLimit[i]=atof(myTempPal[i]->edSingleLimit->Text.c_str());
    }

    SaveLastSetIni();                                                           //20111212  Dell 修正single temp limit range未存檔
    //Steven 20120525 End
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        if(LastSet.iTemperature==Tempture_Ambient)
        {
    filerw::ELTodo("golden uTemp_Set.cpp SetTempShiftOffsetPeriod (CC_ASE_CL) not in the port");   // golden uTemp_Set.cpp:4558-4558 取代：SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有本體（同 FileRW/Offset_File.cpp:725）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4558-4558 -- SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有本體（同 FileRW/Offset_File.cpp:725）
            SetTempShiftOffsetPeriod(25,fMain->cbSetupFileName->Text);
#endif // GATE (S12-C save)
        }
        else
        {
    filerw::ELTodo("golden uTemp_Set.cpp SetTempShiftOffsetPeriod (CC_ASE_CL) not in the port");   // golden uTemp_Set.cpp:4562-4562 取代：SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有本體（同 FileRW/Offset_File.cpp:725）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:4562-4562 -- SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有本體（同 FileRW/Offset_File.cpp:725）
            SetTempShiftOffsetPeriod(Temperature.fWorkTemperBase, fMain->cbSetupFileName->Text);
#endif // GATE (S12-C save)
        }
    }
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    bNeedInitialTestDelay=true;                                                 //Ztex 2024.02.21 Add Save Temp Offset And Send Offset Value

  //---------------------------我是分隔線-----------------------------------------
    AddSpace(szDir);
    EL<TSpeedButton>("TfTemp_Set", "spbSave")->Down=false;
//    fTemperFrom->TempOverCannotRunShowAlarmClear();
    TS_ReadTempFile(true);
    TS_DoIniDataToForm(true);
//    bSetTempChange=true;                //Ifor 20160425 工作檔切換需重新設定ATC參數   //Ifor 20190215 移至離開頁面執行

}

// golden uTemp_Set.cpp:4606  TfTemp_Set::SaveSetupFile(AnsiString szDir, AnsiString S)
static void TS_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    int iSum=0;
    int data=0;
    int i=0, j=0;
    AnsiString szDir3, Str;
    AnsiString S1="";

    if(CosFunction.bSaveTemperatureByMachine && IniConfig.bA57_2SaveTemperatureByMachine)                               //JimmyChiu 20220618 : save by machine
    {
        szDir3=sSaveByMachine;
    }
    else
    {
        S1=GetLastOpenFN();
        szDir3.sprintf("%s%s\\", DataPath, S1);
    }
    szDir3+="Tester.Data";
    MyForceDirectories(szDir);
    if(     EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked)  iSum+=1;
    else if(EL<TRadioButton>("TfTemp_Set", "rb2Point")->Checked)  iSum+=2;
    else if(EL<TRadioButton>("TfTemp_Set", "rb3Point")->Checked)  iSum+=4;
    else if(EL<TRadioButton>("TfTemp_Set", "rb5Point")->Checked)  iSum+=8;                                        //kevin 20141006
    else if(EL<TRadioButton>("TfTemp_Set", "rb6Point")->Checked)  iSum+=16;                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
    if(EL<TCheckBox>("TfTemp_Set", "cbUseAbitCHK")->Checked)
        data=1;

    szDir+="\\Temperature.Data";
    WriteIniData(szDir, "Mode",       "Mode",        EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex);
    WriteIniData(szDir, "Mode",       "Temperature", FormatFloat("0.0", EL<TEdit>("TfTemp_Set", "edWorkTemp")->Text.ToDouble()));                  //wei 20150422 存檔為小數點後一位，因為SPIL比對資料

    if(IniConfig.bVTESTFunction==false)                                         //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        WriteIniData(szDir, "Mode",       "AmbCheck",    data);

    WriteIniData(szDir, "Mode",       "UseIndividualTemp",         EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode")->Checked);                    //Steven 20140924 : 各個加熱區獨立有自己的設定值
    WriteIniData(szDir, "Mode",       "bUseInitialDelayAsSoakTime",EL<TCheckBox>("TfTemp_Set", "chkSoakTimeReduce")->Checked);                         //Steven 20170329 (wei) : 使用initial delay當 Soak time
    WriteIniData(szDir, "Mode",       "bTempAlarmBinNeedToError",  EL<TCheckBox>("TfTemp_Set", "chkTempErrorAction")->Checked);                        //Steven 20251022 : Temp alarm need put to error bin

    //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
    if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)
    {
        WriteIniData(szDir, "Mode",       "WorkTempMode", LastSet.iTemperature);
    }
//    WriteIniData(szDir, "Ambient",    "Check",       rgAmbCheck->ItemIndex);

    if(CosFunction.bTempCalByRecipe==true)                                      //jou 20220725 : Temperature calibration by recipe
        WriteIniData(szDir, "Mode", "bTempCalByRecipe", EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked);

    WriteIniData(szDir, "Ambient",    "Check",       1);

//    if(ATC_SYSTEM==eATCHonPrecType && rbATCActiveOn->Checked==true)     //Steven 20131007 : 啟用ATC時,強制使用 [L11] ATC常溫的溫度
//    {
//        WriteIniData(szDir, "Ambient",    "Temperature", IniConfig.dATCAmbientTemperature);
//    }
//    else
    {
        WriteIniData(szDir, "Ambient",    "Temperature", EL<TEdit>("TfTemp_Set", "edAmbTemp")->Text);
    }

    if(DewPoint_Hardware_Install>0)                                             //Steven 20191017 : 露點計
    {
        WriteIniData(szDir, "Mode", "WaitDewPoint",          EL<TCheckBox>("TfTemp_Set", "cbWaitDewPoint")->Checked);                                  //Isaac 20180828 : wait dewpoint
        WriteIniData(szDir, "Mode", "DewPointRange",         EL<TEdit>("TfTemp_Set", "edDewPointRange")->Text);                                    //Isaac 20181217 : 露點計報警範圍可設定
        WriteIniData(szDir, "Mode", "DewPointAlarmInterval", EL<TEdit>("TfTemp_Set", "edDewPointAlarmInterval")->Text);                            //Isaac 20181217 : 露點計報警範圍可設定
    }

    if(LB_TEMP==true)                                                           //JerryYang 20220923 : LB吹氣function
    {
        WriteIniData(szDir, "Mode", "bLBCoolingAirOn",          EL<TCheckBox>("TfTemp_Set", "cbBlowLB")->Checked);                                     //Isaac 20180828 : wait dewpoint
        WriteIniData(szDir, "Mode", "dLBAirOnTemp",         EL<TEdit>("TfTemp_Set", "edtLBAirOnTemp")->Text);                                      //Isaac 20181217 : 露點計報警範圍可設定
        WriteIniData(szDir, "Mode", "iLBTempAlmInterval", EL<TEdit>("TfTemp_Set", "edLBTempAlarmTime")->Text);                                     //Isaac 20181217 : 露點計報警範圍可設定
    }

    WriteIniData(szDir, "Ambient",    "bAmbUsingAFan", EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")->Checked);                                       //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    WriteIniData(szDir, "Ambient",    "fAmbientHotGuartbent", EL<TEdit>("TfTemp_Set", "edAmbHotGuartbent")->Text);                                 //kevin 20140918 恆溫控制
    WriteIniData(szDir, "Ambient",    "bShuttleNoHeatUp", EL<TCheckBox>("TfTemp_Set", "chkShuttleNoHeatUp")->Checked);                                 //Steven 20180815 : Amb Ctr mode, shuttle no heat up
    WriteIniData(szDir, "Ambient",    "bSLKNoHeatUp", EL<TCheckBox>("TfTemp_Set", "chkSLKNoHeatUp")->Checked);                                         //Steven 20230221 : Amb Ctr mode, SLK no heat up
    WriteIniData(szDir, "Ambient",    "bAmbientGuardbandCheck", EL<TCheckBox>("TfTemp_Set", "cbUseAmbGuard")->Checked);                                //kevin 20180115 (Steven) add ambient grand bient check
    WriteIniData(szDir, "Ambient",    "iAmbGuardband", EL<TEdit>("TfTemp_Set", "edAmbGuardband")->Text);   //kevin 20180115 (Steven) add ambient grand bient check

    WriteIniData(szDir, "Time",       "Soak",        FormatFloat("0.0", EL<TEdit>("TfTemp_Set", "edSoakTime")->Text.ToDouble()));                  //wei 20150422 存檔為小數點後一位，因為SPIL比對資料
    WriteIniData(szDir, "Time",       "Jam Soak",    EL<TEdit>("TfTemp_Set", "edJamSoakTime")->Text);
    WriteIniData(szDir, "Time",       "H.Initial",   EL<TEdit>("TfTemp_Set", "edInitialWaitTime")->Text);
    WriteIniData(szDir, "Time",       "A.Initial",   EL<TEdit>("TfTemp_Set", "edAbitInitWaitTime")->Text);
    WriteIniData(szDir, "Time",       "Cool Time",   EL<TEdit>("TfTemp_Set", "edAbitColdTime")->Text);
//jou 981030 start
    WriteIniData(szDir, "Time",       "iInitialStart1Time",   EL<TEdit>("TfTemp_Set", "edInitialStart1Time")->Text);
    WriteIniData(szDir, "Time",       "iInitialStart2Time",   EL<TEdit>("TfTemp_Set", "edInitialStart2Time")->Text);
//jou 981030 end
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        EL<TEdit>("TfTemp_Set", "edIndexSoakTime")->Text = "0";
        EL<TEdit>("TfTemp_Set", "edOSTime")->Text = "0";
    }                                                                           //kevin 20210511 ASE KH  JL 說關閉
    WriteIniData(szDir, "Time",       "iIndexSoakTime",   EL<TEdit>("TfTemp_Set", "edIndexSoakTime")->Text);                                       //2013-11-27   Dell    增加Index從Shuttle吸起IC時的Soak Time
    WriteIniData(szDir, "Time",       "iOSTime",          EL<TEdit>("TfTemp_Set", "edOSTime")->Text);      //2013-11-27   Dell    當發生o/s 也是要做Index soak time
    WriteIniData(szDir, "Time",       "bZ2DownSocket",    EL<TRadioGroup>("TfTemp_Set", "rgZ1Down")->ItemIndex);                                         //kevin 20131211

    WriteIniData(szDir, "Mode",       "Active_Heat_Gun",  EL<TCheckBox>("TfTemp_Set", "cbHeaterGun")->Checked);                                        //Steven 20120525 : 熱風槍
    WriteIniData(szDir, "Mode",       "Use CDA Only",     EL<TCheckBox>("TfTemp_Set", "chkUseCDAOnly")->Checked);                                      //Steven 20181012 : 使用熱風槍吹冷風
    WriteIniData(szDir, "Mode",       "iHotGunFLowLimit_H",         EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_H")->Text);                        //KaiChen 20190729 ：Hot Gun Flow
    WriteIniData(szDir, "Mode",       "iHotGunFLowLimit_L",         EL<TEdit>("TfTemp_Set", "edtHotGunFLowLimit_L")->Text);                        //KaiChen 20190729 ：Hot Gun Flow
    WriteIniData(szDir, "DUT Setting",  "bUseFixTemp",    EL<TCheckBox>("TfTemp_Set", "cbUseFixTemp")->Checked);                                       //Steven 20131025 : 使4固定的DUT溫度
    WriteIniData(szDir, "DUT Setting",  "dFixedTemp",     EL<TEdit>("TfTemp_Set", "edFixedTemp")->Text);   //Steven 20131025 : 固定的DUT溫度值
    WriteIniData(szDir, "DUT Setting",  "bShowFixedTemp", EL<TCheckBox>("TfTemp_Set", "cbShowFixTemp")->Checked);                                      //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值

    WriteIniData(szDir, "InitialMode",  "EnableTemperatureOffsetforInitial",         EL<TCheckBox>("TfTemp_Set", "cbTempOffset")->Checked);            //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iCintactCntForTempOffsetAtInitial",         EL<TEdit>("TfTemp_Set", "edTempOffsetCount")->Text);          //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iCintactDelayCntForInitTempOffset",         EL<TEdit>("TfTemp_Set", "edTempDownContactDelay")->Text);     //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iTempReadyRange",                          EL<TEdit>("TfTemp_Set", "edTempReadyRange")->Text);            //Sam 20231214 : Temp offset use ready temp range
    WriteIniData(szDir, "ChamberBoostMode",       "bEnableChamberBoost",        EL<TCheckBox>("TfTemp_Set", "chkChamberBoost")->Checked);              //Steven 20191128 : Chamber Boost Function
    WriteIniData(szDir, "ChamberBoostMode",       "iChamberBoostTime",          EL<TEdit>("TfTemp_Set", "edtChamberBoostTime")->Text);
    WriteIniData(szDir, "ChamberBoostMode",       "iChamberBoostOffset",        EL<TEdit>("TfTemp_Set", "edtChamberBoostOffset")->Text);

    if(CosFunction.bHiSiliconFunction==false)                                   //Ifor 20160622 海思版本不存檔，避免蓋掉設定
    {
        //Ifor 20150910 :ATC PID 存檔前轉換
        Temperature.bUsePIDControl = EL<TCheckBox>("TfTemp_Set", "cdATCUsePIDCtrl")->Checked;
        Temperature.iATC_PID_Min_Offset[0] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinP")->Text.c_str())*100;
        Temperature.iATC_PID_Min_Offset[1] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinI")->Text.c_str())*100;
        Temperature.iATC_PID_Min_Offset[2] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MinD")->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[0] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxP")->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[1] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxI")->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[2] =  atof(EL<TEdit>("TfTemp_Set", "edtATCPIDOffset_MaxD")->Text.c_str())*100;

        //Ifor 20150910 :ATC PID Control
        WriteIniData(szDir, "InitialMode",  "UsePIDControl",         Temperature.bUsePIDControl);
        //Ifor 20150910 : ATC PID Offset
        for(int i=0; i<3; i++)
        {
            WriteIniData(szDir, "InitialMode", "iATC_PID_Min_Offset_"+ AnsiString(i)     , Temperature.iATC_PID_Min_Offset[i]);
            WriteIniData(szDir, "InitialMode", "iATC_PID_Max_Offset_"+ AnsiString(i)     , Temperature.iATC_PID_Max_Offset[i]);
        }
    }

    WriteIniData(szDir, "Time", "In Shuttle Soak Time Mode", EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->ItemIndex);                             //jou 2012-06-29 Index Pick up need wait Soak Time
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_Enable"   , EL<TCheckBox>("TfTemp_Set", "cb3SigmaTempMonitior_Enable")->Checked);                  //kevin 20200521 add 3 Sigma 溫度記錄比對
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_SetCount" , EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_SetCount")->Text);                  //kevin 20200521 add 3 Sigma Index Contract time
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_Set3xSigmaValue"  , EL<TEdit>("TfTemp_Set", "edt3SigmaTempMonitior_Set3xSigmaValue")->Text);   //kevin 20200521 add 3 Sigma 客戶比對誤差值

    WriteIniData(szDir, "ATC", "bEnableTJFunction"  , EL<TCheckBox>("TfTemp_Set", "cbEnableTJFunction")->Checked);                                     //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    WriteIniData(szDir, "ATC", "iTjSiteMapType"  ,    EL<TRadioGroup>("TfTemp_Set", "rgTjMapType")->ItemIndex);  //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    WriteIniData(szDir, "ATC", "dTjInputVLow"  , atof(EL<TEdit>("TfTemp_Set", "edtInputVLow")->Text.c_str()));
    WriteIniData(szDir, "ATC", "dTjInputVHigh"  ,  atof(EL<TEdit>("TfTemp_Set", "edtInputVHigh")->Text.c_str()));

    WriteIniData(szDir, "ATC", "dDelayAfterSOT"  , atof(EL<TEdit>("TfTemp_Set", "edtDelayAfterSOT")->Text.c_str()));
    WriteIniData(szDir, "ATC", "dDelayAfterSOTContinue"  , atof(EL<TEdit>("TfTemp_Set", "edContinuousSec")->Text.c_str()));                        //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    WriteIniData(szDir, "ATC", "bTjWatchdog"  , EL<TCheckBox>("TfTemp_Set", "cbTjWatchdog")->Checked);         //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    WriteIniData(szDir, "ATC", "dTjOffset"  , atof(EL<TEdit>("TfTemp_Set", "edtSetTJ_Offset")->Text.c_str()));
    WriteIniData(szDir, "ATC", "dTjSlope"  ,  atof(EL<TEdit>("TfTemp_Set", "edtSetTJ_Slope")->Text.c_str()));

    WriteIniData(szDir, "ATC", "sATC_CH_Tj", sATC_CH_Tj->CommaText);

    if(ATC_SYSTEM>eATC60)                                                       //ChungHung 20141124 add
    {
        if(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex!=0)
            WriteIniData(szDir, "Index",       "Heating Mode",  HeadSocket);    //Steven `20090923
        else
            WriteIniData(szDir, "Index",       "Heating Mode",  HeadOnly);      //Steven `20090923

        if(ATC_SYSTEM==eATCSiliconType)
        {
            WriteIniData(szDir, "ATC", "Temperature Set", EL<TCheckBox>("TfTemp_Set", "rbTemperatureSetOn")->Checked);
        }
        else if(ATC_SYSTEM==eATCHonPrecType)                                    //Steven 20120410 : Hontech ATC 2.0
        {
//            WriteIniData(szDir, "ATC", "Chiller Temp", edChillerTemp->Text);
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                WriteIniData(szDir, "ATC", "Chiller Temp", IntToStr(15));       //Ifor 20161103 ATC 2.0 Chiller 溫度寫死20度   //JerryYang 20161128 (Jou)守志說矽品SPEC固定15度
            else
                WriteIniData(szDir, "ATC", "Chiller Temp", EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text);
        }
        else if(ATC_SYSTEM==eNewATCSystem)                                      //Ifor 20161101 Fix New ATC Chiller Temp Not Save
        {
            WriteIniData(szDir, "ATC", "Chiller Temp", EL<TEdit>("TfTemp_Set", "edChillerTemp")->Text);
        }

        if(ATC_SYSTEM!=eNonChamber)                                             //Steven 20140314 : For HT9045WA
        {
            WriteIniData(szDir, "ATC", "Active Cooling" , EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked);
            WriteIniData(szDir, "ATC", "Handler Start"  , EL<TCheckBox>("TfTemp_Set", "rbHandlerStartOn")->Checked);
            WriteIniData(szDir, "ATC", "Test Strat"     , EL<TCheckBox>("TfTemp_Set", "rbTestStartOn")->Checked);
            WriteIniData(szDir, "ATC", "SP2"            , EL<TEdit>("TfTemp_Set", "edtATCSP2")->Text);
            WriteIniData(szDir, "ATC", "InPC[0]"        , EL<TEdit>("TfTemp_Set", "edtATCInPC1")->Text);
            WriteIniData(szDir, "ATC", "InPC[1]"        , EL<TEdit>("TfTemp_Set", "edtATCInPC2")->Text);
            WriteIniData(szDir, "ATC", "InPC[2]"        , EL<TEdit>("TfTemp_Set", "edtATCInPC3")->Text);
            WriteIniData(szDir, "ATC", "InPC[3]"        , EL<TEdit>("TfTemp_Set", "edtATCInPC4")->Text);
            WriteIniData(szDir, "ATC", "Initial_0", EL<TEdit>("TfTemp_Set", "edATCInitialOffset1")->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_1", EL<TEdit>("TfTemp_Set", "edATCInitialOffset2")->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_2", EL<TEdit>("TfTemp_Set", "edATCInitialOffset3")->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_3", EL<TEdit>("TfTemp_Set", "edATCInitialOffset4")->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC

            WriteIniData(szDir, "ATC", "ConsFail_0", EL<TEdit>("TfTemp_Set", "edATCContFailOffset1")->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_1", EL<TEdit>("TfTemp_Set", "edATCContFailOffset2")->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_2", EL<TEdit>("TfTemp_Set", "edATCContFailOffset3")->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_3", EL<TEdit>("TfTemp_Set", "edATCContFailOffset4")->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "bEnableATCConFailOffset", EL<TCheckBox>("TfTemp_Set", "cbEnableATCConsFailOffset")->Checked);                  //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "iATCConFailOffsetCount",  EL<TEdit>("TfTemp_Set", "edATCContFailOffsetCnt")->Text);                        //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset1", EL<TEdit>("TfTemp_Set", "edATCQAModeOffset1")->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset2", EL<TEdit>("TfTemp_Set", "edATCQAModeOffset2")->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset3", EL<TEdit>("TfTemp_Set", "edATCQAModeOffset3")->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset4", EL<TEdit>("TfTemp_Set", "edATCQAModeOffset4")->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "bEnableATCQAModeOffset", EL<TCheckBox>("TfTemp_Set", "cbEnableATCQAModeOffset")->Checked);                     //Steven 20151125 : QA Mode Temp Offset for ATC

            WriteIniData(szDir, "ATC", "bEnableATCTestTimeOffset", EL<TCheckBox>("TfTemp_Set", "cbATCTestTimeOffset")->Checked);                       //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "iATCTestTimeOffsetTime", EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset")->Text);                            //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset1", EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset1")->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset2", EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset2")->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset3", EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset3")->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset4", EL<TEdit>("TfTemp_Set", "edATCTestTimeOffset4")->Text);                              //Steven 20160216 : 測試時間太短也要Offset

            WriteIniData(szDir, "ATC", "USE Tj Function" , EL<TCheckBox>("TfTemp_Set", "chk_UseTj")->Checked);                                         //2014-05-30    Dell    for ATC6.0
            WriteIniData(szDir, "ATC", "Tj Mode" , EL<TRadioGroup>("TfTemp_Set", "rg_TjMode")->ItemIndex);       //2014-05-30    Dell    for ATC6.0
            WriteIniData(szDir, "ATC", " Tj Avg Times" , EL<TEdit>("TfTemp_Set", "edt_TjAvgTimes")->Text);                                         //2014-05-30    Dell    for ATC6.0

            WriteIniData(szDir, "ATC", "ATC7"           , EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked);                                    //Eliot 2015_0105
            WriteIniData(szDir, "ATC", "TSDFunction"    , EL<TCheckBox>("TfTemp_Set", "rbATC7TSDFunction")->Checked);                                  //Eliot 2015_0105
            WriteIniData(szDir, "ATC", "TSDTimeOut"     , EL<TEdit>("TfTemp_Set", "edTSDTimeOut")->Text);  //Steven 20160604 : by site TSD
            WriteIniData(szDir, "ATC", "iATC60Air"      , EL<TRadioGroup>("TfTemp_Set", "rgATC60AirOn")->ItemIndex);
            WriteIniData(szDir, "ATC", "ATC_OFS_ST"    , EL<TEdit>("TfTemp_Set", "edATCOfsTime")->Text);   //Ifor 20241118 : 測試中變溫

            if(CosFunction.bUseOldATCTempOffset==true)                          //JerryYang 20190123 ATC Offset UI使用舊版
            {
                for(i=0; i<32; i++)
                {
                    Str.sprintf("ATCTempOffset[%d]", i);                        //JerryYang 20190910 fix ATC offset存檔問題
                    WriteIniData(szDir, "ATC", Str, ATCOffsetEdit[i]->Text);
                }
            }
            else
            {
                //Ifor 20190930 ATC 功能開啟才使用ATC Temp Offset檔案
                if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)                                    // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180817 : Boost Function
                {
                    for(i=0; i<tcTotalCount; i++)
                    {
                        for(j=0; j<32; j++)
                        {
                            if(myTempPal[i]->iIndexTag==j)
                            {
                                Str.sprintf("ATCTempOffset[%d]", j);            //JerryYang 20190910 fix ATC offset存檔問題
                                WriteIniData(szDir, "ATC", Str, myTempPal[i]->edOffset->Text);
                            }
                        }
                    }
                }

                if(CosFunction.bUseSecondATCTempOffset==true)                   //Ifor 20241014 Use Second ATC Temp Offset
                {
                    for(i=0; i<32; i++)
                    {
                        Str.sprintf("ATCSecondTempOffset[%d]", i);
                        WriteIniData(szDir, "ATC", Str, ATCOffsetEdit[i]->Text);
                    }
                }
            }
            //Ifor 20160421 第二點溫度Sensor設定改至工作檔
            WriteIniData(szDir, "ATC", "UseReferTempSensor" , EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked);

            //Ifor 20190306 : add Package Offset 三點校正
            //==>
            for(int i=0; i<3; i++)
            {
                Str.sprintf("ATCPackageOffset[%d]", i);
                WriteIniData(szDir, "ATC", Str, ATCPackageOffsetEdit[i]->Text);

                Str.sprintf("ATCPackagetemp[%d]", i);
                WriteIniData(szDir, "ATC", Str, ATCPackageTempEdit[i]->Text);
            }
            //<==
            //Ifor 20190306 : add Package Offset 三點校正

            //Ifor 20190328 : add TJ Temp Over Range
            //==>
             WriteIniData(szDir, "ATC", "TJTempRange_High"  , EL<TEdit>("TfTemp_Set", "edTJTempRange_High")->Text);
             WriteIniData(szDir, "ATC", "TJTempRange_Low"   , EL<TEdit>("TfTemp_Set", "edTJTempRange_Low")->Text);
            //<==
            //Ifor 20190328 : add TJ Temp Over Range

            for(int i=0; i<2; i++)                                              //Ifor 20190730 : add FFC Temperature Offset
            {
                for(int j=0; j<10; j++)
                {
                    Str.sprintf("iArm%d_FFCTimeOn[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetOnTimeEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCTimeOff[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetOffTimeEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCOffset[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCPointUse[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCPointUse[i][j]->Checked);
                }
            }
            WriteIniData(szDir, "ATC", "bATC_FFCEnable" , EL<TCheckBox>("TfTemp_Set", "cbFFCOffsetEnable")->Checked);
            WriteIniData(szDir, "ATC", "UseTC2Offset",    EL<TCheckBox>("TfTemp_Set", "cbUseTC2Offset")->Checked);                                     //KenHsieh 20240311 : add Tc2 Offset

            WriteIniData(szDir, "ATC", "Multi Zone Enable", EL<TCheckBox>("TfTemp_Set", "cbMultiZoneFunction")->Checked);                              //wei 20240617 Multi Zone
            for(int i=0; i<4; i++)
            {
                Str.sprintf("Zone Temp %d Use", i+1);
                WriteIniData(szDir, "ATC", Str , ZoneTempUse[i]->Checked);

                Str.sprintf("Zone Temp %d Setting", i+1);
                WriteIniData(szDir, "ATC", Str , ZoneTempSetting[i]->Text);
            }
            WriteIniData(szDir, "ATC", "Enable Temp Rise" , EL<TCheckBox>("TfTemp_Set", "cbATCTempRise")->Checked);                                    //Ifor 20230418 add:Index 回溫功能
            WriteIniData(szDir, "ATC", "dTempRiseTemp", EL<TEdit>("TfTemp_Set", "edt_TempRiseTemp")->Text);
            WriteIniData(szDir, "ATC", "dTempRiseDelay", EL<TEdit>("TfTemp_Set", "edt_TempRise_Delay")->Text);

            if(ATC_SYSTEM==eNewATCSystem)
            {
                WriteIniData(szDir, "ATC", "Multi Sensor Offset Enable", EL<TCheckBox>("TfTemp_Set", "cbMultiSensorOffsetEnable")->Checked);           //Ifor 20251203 add:ATC Multi Sensor Offset
                for(int i=0; i<4; i++)                                          //Ifor 20251203 add:ATC Multi Sensor Offset
                {
                    Str.sprintf("Multi Sensor Offset %d Use", i+1);
                    WriteIniData(szDir, "ATC", Str , MultiSensorOffsetUse[i]->Checked);
                }
                for(int i=0; i<32; i++)                                         //Ifor 20251203 add:ATC Multi Sensor Offset
                {
                    Str.sprintf("Multi Sensor Offset[%d]", i+1);
                    WriteIniData(szDir, "ATC", Str , ATC_MultiSensorOffsetEdit[i]->Text);
                }
            }
        }
    }
    else
    {
        WriteIniData(szDir, "Index",       "Heating Mode",  EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex);                                //Steven `20090923
    }

    if(Tri_Temp_Machine==1 ||
       (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
    {
//        WriteIniData(szDir, "InitialMode", "SetTempature2AirMachine"             , edtSetTempature2AirMachine->Text);
        WriteIniData(szDir, "InitialMode", "dSetAirstreamTemperatureRang_Socket" , EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Socket")->Text);
//        WriteIniData(szDir, "InitialMode", "EnableWaitTemperatureStableTime"     , cbWaitTemperatureStableTime->Checked);
//        WriteIniData(szDir, "InitialMode", "WaitTemperatureStableTime"           , edtWaitTemperatureStableTime->Text);
//        WriteIniData(szDir, "InitialMode", "UseOutShuttleDesoakTime"             , cbUseOutShuttleDesoakTime->Checked);
//        WriteIniData(szDir, "InitialMode", "OutShuttleDesoakTime"                , edtOutShuttleDesoakTime->Text);
        WriteIniData(szDir, "InitialMode", "EnableTesterDryAirControl"           , EL<TCheckBox>("TfTemp_Set", "cbEnableTesterDryAirControl")->Checked);
//        WriteIniData(szDir, "InitialMode", "dSetIndexAirstreamTemp"              , edt_SetIndexAirstreamTemp->Text);
        WriteIniData(szDir, "InitialMode", "SetAirstreamTemperatureRang_Index"   , EL<TEdit>("TfTemp_Set", "edt_SetAirstreamTemperatureRang_Index")->Text);
        WriteIniData(szDir, "InitialMode", "bEnableArm_1_Air"                    , EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_1_Air")->Checked);
        WriteIniData(szDir, "InitialMode", "bEnableArm_2_Air"                    , EL<TCheckBox>("TfTemp_Set", "chk_EnableArm_2_Air")->Checked);
        WriteIniData(szDir, "InitialMode", "bEnableSocket_Air"                   , EL<TCheckBox>("TfTemp_Set", "chk_EnableSocket_Air")->Checked);
        WriteIniData(szDir, "InitialMode", "iAirVolumeLmt"                       , EL<TEdit>("TfTemp_Set", "edt_AirVolumeLmt_Index")->Text);
        WriteIniData(szDir, "InitialMode", "AirStreamSocket_Offset"             , EL<TEdit>("TfTemp_Set", "edtSetTempature2AirMachine")->Text);    //Ztex 2023.12.19 Add Air Stream Offset
        WriteIniData(szDir, "InitialMode", "AirStreamIndex_Offset"              , EL<TEdit>("TfTemp_Set", "edt_SetIndexAirstreamTemp")->Text);     //Ztex 2023.12.19 Add Air Stream Offset
        WriteIniData(szDir, "InitialMode", "Defrost_Time_Too_Lower"             , EL<TEdit>("TfTemp_Set", "edt_Defrost_Time_Too_Lower")->Text);    //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
    }

    AnsiString sATCFILENAME="";
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20181219 (Steven) :  add write ATC FILE NAME
    {
         if(EL<TRadioButton>("TfTemp_Set", "RachoiceATCFile")->Checked)
            sATCFILENAME = EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Text;
         else
            sATCFILENAME = EL<TEdit>("TfTemp_Set", "edAtcFileName")->Text;
         WriteIniData(szDir, "ATC", "File Name", sATCFILENAME);                 //Steven 20150810 : add for ATC
    }
    else
    {
        if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)
        {
            WriteIniData(szDir, "ATC", "File Name", EL<TEdit>("TfTemp_Set", "edAtcFileName")->Text);       //Steven 20150810 : add for ATC
        }
        else if(ATC_SYSTEM==eNewATCSystem)                                      //Ifor 20160111 add for New ATC System
        {
            WriteIniData(szDir, "ATC", "File Name", EL<TComboBox>("TfTemp_Set", "cbbATC_RecipeFile")->Text);   //Ifor 20160111 : add for New ATC System
        }
    }
    WriteIniData(szDir, "ATC", "Type Name", EL<TEdit>("TfTemp_Set", "edtATC_Type")->Text);                 //Ifor 20230828 add:送ATC Type 資料給GPIB
    if(CosFunction.bOffsetTempByRecipeMinMaxLimit)                              //StevenHong 20260119 : Add Offset By Recipe Max Limit
    {
        Temperature.iOffsetByRecipeMaxLimit = ReadWriteIni(szDir, "User OffSet", "MaxLimit", Temperature.iOffsetByRecipeMaxLimit, 60,  bWriteFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
        Temperature.iOffsetByRecipeMinLimit = ReadWriteIni(szDir, "User OffSet", "MinLimit", Temperature.iOffsetByRecipeMinLimit, -60, bWriteFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
    }
    for(int i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);

        if(((i>=tcAa1 && i<=tcBd2) || (i>=tcAe1 && i<=tcBh2)) &&
           Temperature.bATCActiveCooling==true)                                 //Ifor 20191111 : Fix ATC 開啟時Offset無法修改問題
        {
        }
        else
        {
            WriteIniData(szDir, "User OffSet", S, myTempPal[i]->edOffset->Text);
        }

        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            WriteIniData(szDir, "Kit Low OffSet",           S, myTempPal[i]->edKit_Lowbase->Text);
            WriteIniData(szDir, "Kit Mid. OffSet",          S, myTempPal[i]->edKit_Base->Text);
            WriteIniData(szDir, "Kit High OffSet",          S, myTempPal[i]->edKit_HighBase->Text);
            WriteIniData(szDir, "Kit AmbientHotLowOffSet",  S, myTempPal[i]->edKit_Low->Text);
            WriteIniData(szDir, "Kit AmbientHotMidOffSet",  S, myTempPal[i]->edKit_Mid->Text);
        }

        if(ATC_SYSTEM==eNewATCSystem &&
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                      //Steven 20181102 : For QTI全球統一Offset
            WriteIniData(szDir, "Cal Temp", S, myTempPal[i]->edIndiTemp->Text);
        else
            WriteIniData(szDir, "IndividualTempSetting", S, myTempPal[i]->edIndiTemp->Text);                            //Steven 20140924 : 各個加熱區獨立有自己的設定值

        if(CosFunction.bInitTempOffsetByWorkFile==true)                         //jou 2015-06-13 Initial Temperture Offset by WorkFile
            WriteIniData(szDir, "Init Temp OffSet",     S, myTempPal[i]->edInitTempOffset->Text);                       //Steven 20141117 : 起測時溫度要補Offset
    }

//    Temperature.bATC7TSDFunction    = ReadIniData(szDir, "ATC", "TSDFunction", false);              //Eliot 2015_0105
//    i = Temperature.bATC7TSDFunction;                                                               //Eliot 2015_0105
//    ATCInterfaceForm->SendCommToATC7(ATC_USE_TSD, i, "");                                           //Eilot 2015_0105

    WriteIniData(szDir, "Cooling", "ChamberCoolTemp",   EL<TEdit>("TfTemp_Set", "edChamberCoolTemp")->Text);                                       //Steven 20111209 : Chamber降溫溫度
    WriteIniData(szDir, "Time",    "bUseTesterDocking", EL<TCheckBox>("TfTemp_Set", "chkTesterDock")->Checked);                                        //jou 2014-04-25 Tester Docking Wait delay time
    WriteIniData(szDir, "Time",    "iUseTesterDocking", atoi(EL<TEdit>("TfTemp_Set", "edtTestDock")->Text.c_str()));                               //jou 2014-04-25 Tester Docking Wait delay time
    WriteIniData(szDir, "ATC", "ATCAmbientTemperature", EL<TEdit>("TfTemp_Set", "edATCAmbTemp")->Text);    //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "ATC_HeatGunTemp", EL<TEdit>("TfTemp_Set", "edtHeatGunTempATC")->Text);   //JerryYang 20220408 : add for ATC3.5
    WriteIniData(szDir, "ATC", "dATC_HotGunTemp", EL<TEdit>("TfTemp_Set", "edtATC_HotGunTemp")->Text);     //Ifor 20220124 add: ATC Hot Air
    WriteIniData(szDir, "ATC", "dATC_HotGunTime", EL<TEdit>("TfTemp_Set", "edtATC_HotGunTime")->Text);     //Ifor 20220124 add: ATC Hot Air

    WriteIniData(szDir, "Index", "Arm1 Offset",   EL<TEdit>("TfTemp_Set", "edArm1Offset")->Text);          //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "Arm2 Offset",   EL<TEdit>("TfTemp_Set", "edArm2Offset")->Text);          //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset1", EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_1")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset2", EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_2")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset3", EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_3")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset4", EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_4")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset5", EL<TEdit>("TfTemp_Set", "edArm1NoFullsiteOffset_5")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset1", EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_1")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset2", EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_2")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset3", EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_3")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset4", EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_4")->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset5", EL<TEdit>("TfTemp_Set", "edArm2NoFullsiteOffset_5")->Text);                           //wei 20160107 No FullSite Add Offset

    //Ifor 20160308 add Temp
    double dAlarmCount=StrToFloat(EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Text);
    if(dAlarmCount<20)
    {
        EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Text = "20";
    }

    WriteIniData("D:\\HT9045\\Config\\ATC.ini", "Setup", "iCheckSameTempTime", EL<TEdit>("TfTemp_Set", "edTempAlwaysSameAlarm")->Text);

    WriteIniData(szDir, "Index", "dSocketAirCoolingOnTimer", EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOn")->Text);                               //jou 2016-04-28 Socket Air Cooling contact count trun on
    WriteIniData(szDir, "Index", "dSocketAirCoolingOffTimer", EL<TEdit>("TfTemp_Set", "edSocketAirCoolingOff")->Text);                             //jou 2016-04-28 Socket Air Cooling contact count trun on

    //Steven 20180817 : Boost Function
    //==>
    WriteIniData(szDir, "Boost Function", "bBoostFuncttion",            EL<TCheckBox>("TfTemp_Set", "chkBoostFunction")->Checked);
    WriteIniData(szDir, "LB Temp Function", "bLBTempFunction",          EL<TCheckBox>("TfTemp_Set", "chkLBTempFunction")->Checked);
    WriteIniData(szDir, "Boost Function", "dBoostIdleTime[0]",          atof(EL<TEdit>("TfTemp_Set", "edtIdleTime_Long")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostOffset[0]",            atof(EL<TEdit>("TfTemp_Set", "edtBoostOffset_Long")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostDuration[0]",          atof(EL<TEdit>("TfTemp_Set", "edtBoostDuration_Long")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dPostBoostDuration[0]",      atof(EL<TEdit>("TfTemp_Set", "edtPostBoost_Long")->Text.c_str()));
//    WriteIniData(szDir, "Boost Function", "dBoostIdleTime[1]",      atof(edtIdleTime_Mid->Text.c_str()));
//    WriteIniData(szDir, "Boost Function", "dBoostOffset[1]",        atof(edtBoostOffset_Mid->Text.c_str()));
//    WriteIniData(szDir, "Boost Function", "dBoostDuration[1]",      atof(edtBoostDuration_Mid->Text.c_str()));
//    WriteIniData(szDir, "Boost Function", "dPostBoostDuration[1]",  atof(edtPostBoost_Mid->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostIdleTime[2]",          atof(EL<TEdit>("TfTemp_Set", "edtIdleTime_Short")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostOffset[2]",            atof(EL<TEdit>("TfTemp_Set", "edtBoostOffset_Short")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostDuration[2]",          atof(EL<TEdit>("TfTemp_Set", "edtBoostDuration_Short")->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dPostBoostDuration[2]",      atof(EL<TEdit>("TfTemp_Set", "edtPostBoost_Short")->Text.c_str()));

//    WriteIniData(szDir, "LB Temp Function", "bEnableBoostOffset[3]",    chkLBInitOffset->Checked);
    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[3]",        atof(EL<TEdit>("TfTemp_Set", "edtIdleTime_LB")->Text.c_str()));            //Steven 20181023 : LB溫度
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[3]",          atof(EL<TEdit>("TfTemp_Set", "edtBoostOffset_LB")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostDuration[3]",        atof(EL<TEdit>("TfTemp_Set", "edtBoostDuration_LB")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dPostBoostDuration[3]",    atof(EL<TEdit>("TfTemp_Set", "edtPostBoost_LB")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[4]",        atof(EL<TEdit>("TfTemp_Set", "edtLBTempMin")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[4]",          atof(EL<TEdit>("TfTemp_Set", "edtLBTempOffset")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "Threshold",                atof(EL<TEdit>("TfTemp_Set", "edtThreshold")->Text.c_str()));              //Steven 20190928 : L/B升溫的門檻值
    WriteIniData(szDir, "LB Temp Function", "bEnableBoostOffset[5]",    EL<TCheckBox>("TfTemp_Set", "chkBoostOffset")->Checked);
    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[5]",        atof(EL<TEdit>("TfTemp_Set", "edtBoostTempMin")->Text.c_str()));           //Steven 20181023 : LB溫度
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[5]",          atof(EL<TEdit>("TfTemp_Set", "edtBoostOffset")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostTimeOut",            atof(EL<TEdit>("TfTemp_Set", "edtLBTimeOut")->Text.c_str()));              //Steven 20181222 : Add LB升溫的Time out
    WriteIniData(szDir, "Boost Function",   "iBoostFunctionMode",       EL<TRadioGroup>("TfTemp_Set", "rgBoostMode")->ItemIndex);
    //<==
    //Steven 20180817 : Boost Function

    //KenHsieh 20240216 : add ATC Power Follow Function
    //==>
    WriteIniData(szDir, "ATC",     "bPowerFollow_Enable",     EL<TCheckBox>("TfTemp_Set", "cbPowerFollow_Enable")->Checked);
    WriteIniData(szDir, "ATC",     "iPowerFollow_FullPower",  EL<TEdit>("TfTemp_Set", "edtPowerFollow_FullPower")->Text);
    WriteIniData(szDir, "ATC",     "dPowerFollower_PFSlope",  atof(EL<TEdit>("TfTemp_Set", "edtPowerFollower_PFSlope")->Text.c_str()));
    WriteIniData(szDir, "ATC",     "dPowerFollower_WGain",    atof(EL<TEdit>("TfTemp_Set", "edtPowerFollower_WGain")->Text.c_str()));
    WriteIniData(szDir, "ATC",     "iPowerFollower_Many2one", EL<TEdit>("TfTemp_Set", "edtPowerFollower_Many2one")->Text);
    //<==
    //KenHsieh 20240216 : add ATC Power Follow Function

    //Steven 20180815 : Initial Temp Offset-------------------------------------
    //==>
    WriteIniData(szDir3, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",            EL<TCheckBox>("TfTemp_Set", "cbEveryFirstDeviceUseInitialDelay")->Checked);

    if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        WriteIniData(szDir3, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         EL<TCheckBox>("TfTemp_Set", "cbUseOtherArmToTestAfterInitialDelay")->Checked);
    else
        WriteIniData(szDir3, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         false);

    WriteIniData(szDir3, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",       EL<TCheckBox>("TfTemp_Set", "cbAfterShowAlarmMessageUseInitialDelay")->Checked);
    WriteIniData(szDir3, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay",   EL<TCheckBox>("TfTemp_Set", "cbWhenHappenTestedTimeBelowUseInitialDelay")->Checked);
    WriteIniData(szDir3, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",            EL<TEdit>("TfTemp_Set", "edBelowSec")->Text);
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay",                               EL<TEdit>("TfTemp_Set", "edtInitialDelay_1")->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_2",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_2")->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_3",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_3")->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_4",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_4")->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_5",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_5")->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_6",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_6")->Text);  //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir3, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           EL<TCheckBox>("TfTemp_Set", "cbWhenPressStopOverUseInitialDelay")->Checked);                      //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    WriteIniData(szDir3, "InitialMode",       "iWhenPressStopOver",    EL<TEdit>("TfTemp_Set", "edOverSec")->Text);                                //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir3, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              EL<TCheckBox>("TfTemp_Set", "cbWhenNoFullSiteUseInitialDelay")->Checked);                         //wei 20151228 No FullSite delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_7",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_7")->Text);  //wei 20151228 No FullSite delay

    //ChungHung 20141017 add for SCK Add for TSMC Device
    WriteIniData(szDir3, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      EL<TCheckBox>("TfTemp_Set", "cbAfterAutoCleanFunctionUseInitialDelay")->Checked);
    //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    WriteIniData(szDir3, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           EL<TCheckBox>("TfTemp_Set", "cbAfterOpenHeatDoorUseInitialDelay")->Checked);

    WriteIniData(szDir3, "InitialMode",       "bTestFinishToNextTestOver",                   EL<TCheckBox>("TfTemp_Set", "cbTestFinishToNextTestOver")->Checked);                              //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir3, "InitialMode",       "iTestFinishToNextTestOver",                   EL<TEdit>("TfTemp_Set", "iTestFinishToNextTestOver")->Text);                                  //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_8",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_8")->Text);  //kevin 20160310 測試完成到下一次完成超過所設時間須啟動

    WriteIniData(szDir3, "InitialMode",       "bOTDUnlockDelay",                             EL<TCheckBox>("TfTemp_Set", "cbOTDUnlockDelay")->Checked);                                        //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_9",                             EL<TEdit>("TfTemp_Set", "edtInitialDelay_9")->Text);  //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir3, "InitialMode",       "bTestStartToNextTestStart",                   EL<TCheckBox>("TfTemp_Set", "cbTestStartToNextTestStart")->Checked);                              //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir3, "InitialMode",       "dTeststartToNextTestStart",                   EL<TEdit>("TfTemp_Set", "iTeststartToNextTestStart")->Text);                                  //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_10",                            EL<TEdit>("TfTemp_Set", "edtInitialDelay_10")->Text);                                         //kevin 20181031 : SOT 超過時間Delay

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_1_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_1_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_2_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_2_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_3_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_3_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_4_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_4_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_5_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_5_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_6_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_6_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_7_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_7_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_8_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_8_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_9_RT",             EL<TEdit>("TfTemp_Set", "edtInitialDelay_9_RT")->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_10_RT",            EL<TEdit>("TfTemp_Set", "edtInitialDelay_10_RT")->Text);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    WriteIniData(szDir, "Ambient",            "ByPassChamber",                  EL<TCheckBox>("TfTemp_Set", "cbByPassChamber")->Checked);              //KenHsieh 20230301 : By Pass Chamber

    WriteIniData(szDir, "LB Temp Function", "LB_Temp_High_Setting",      atof(EL<TEdit>("TfTemp_Set", "edLBTempHighSettingValue")->Text.c_str()));                                         //Frank 20241231 : add
    WriteIniData(szDir, "LB Temp Function", "LB_Temp_Low_Setting",       atof(EL<TEdit>("TfTemp_Set", "edLBTempLowSettingValue")->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "bLBTempHighAlarm_Enable",   EL<TCheckBox>("TfTemp_Set", "cbLBTempHighAlarmEnable")->Checked);
    WriteIniData(szDir, "LB Temp Function", "bLBTempLowAlarm_Enable",    EL<TCheckBox>("TfTemp_Set", "cbLBTempLowAlarmEnable")->Checked);
    TS_ReadTestIFFile_InitialMode(); filerw::ELTodo("golden cTesterIF.cpp:563 FTestIF->ReadTestIFFile only the [InitialMode] block (:713-779) re-read -- rest gated F-5 (porting-gaps 14)");   // golden uTemp_Set.cpp:5162-5162 取代：FTestIF->ReadTestIFFile 移植樹 GATE (F-5)：只跑 golden cTesterIF.cpp:713-779 的 [InitialMode] 段（本表單顯示／寫的欄位）
#if 0 // GATE (S12-C save) golden uTemp_Set.cpp:5162-5162 -- FTestIF->ReadTestIFFile 移植樹 GATE (F-5)：只跑 golden cTesterIF.cpp:713-779 的 [InitialMode] 段（本表單顯示／寫的欄位）
    FTestIF->ReadTestIFFile();
#endif // GATE (S12-C save)
    //<==
    //Steven 20180815 : Initial Temp Offset-------------------------------------

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        WriteIniData(szDir, "TriTempSet",   "UseHeaterOutShuttle",              EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableOutSht")->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterIndex",                   EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableIndex")->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterBase",                    EL<TCheckBox>("TfTemp_Set", "chkTriTempEnableBase")->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterDoor",                    EL<TCheckBox>("TfTemp_Set", "chk_IndexHeaterDoor")->Checked);          //Ztex 2023.10.23 Add Index Door Heater
    }                                                                           //Heater開關設定

}

// golden uTemp_Set.cpp:5628  TfTemp_Set::CheckTempSettingChange()
static bool TS_CheckTempSettingChange()
{
    filerw::ELMark("CheckTempSettingChange");   // FileRW：存檔流程 trace（ack.trace）

    //==============Temperature=======================
    if(Temperature.iMachineTempMode!=EL<TRadioGroup>("TfTemp_Set", "rgTemperatureMode")->ItemIndex)
        return true;
    //==============High Mode Setting=================
    else if(EL<TEdit>("TfTemp_Set", "edSoakTime")->Text!=FormatFloat("0.0", Temperature.fSoakTime))
        return true;
    else if(EL<TEdit>("TfTemp_Set", "edJamSoakTime")->Text!=FormatFloat("0.0", Temperature.fJamSoakTime))
        return true;
    else if(EL<TEdit>("TfTemp_Set", "edInitialWaitTime")->Text!=FormatFloat("0.0", Temperature.fInitialWaitTime))
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edInitialStart1Time")->Text.c_str())!=Temperature.iInitialStart1Time)
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edInitialStart2Time")->Text.c_str())!=Temperature.iInitialStart2Time)
        return true;
    else if(EL<TEdit>("TfTemp_Set", "edChamberCoolTemp")->Text!=FormatFloat("0.0", Temperature.fChamberCoolTemp))
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edIndexSoakTime")->Text.c_str())!=Temperature.iIndexSoakTime)
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edOSTime")->Text.c_str())!=Temperature.iOSTime)
        return true;
    else if(EL<TRadioGroup>("TfTemp_Set", "rgZ1Down")->ItemIndex!=Temperature.bZ2DownSocket)
        return true;
    else if(EL<TRadioGroup>("TfTemp_Set", "rgShuttleSoakTime")->ItemIndex!=Temperature.iShuttleSoakTimeMode)
        return true;
    else if(EL<TCheckBox>("TfTemp_Set", "cbEnableIndividualMode")->Checked!=Temperature.bUseIndividualTemp)
        return true;
    //==============Ambient Mode Setting==============
    else if(EL<TCheckBox>("TfTemp_Set", "cbUseAbitCHK")->Checked!=Temperature.bUseAbitCHK)
        return true;
    else if(EL<TEdit>("TfTemp_Set", "edAmbTemp")->Text!=FormatFloat("0.0", Temperature.fAbitTemp))
        return true;
    else if(EL<TCheckBox>("TfTemp_Set", "chkAmbUsingAFan")->Checked!=Temperature.bAmbUsingAFan)
        return true;
    //==============Temperature offset function=======
    else if(EL<TCheckBox>("TfTemp_Set", "cbTempOffset")->Checked!=Temperature.bEnableTempOffsetForInitial)
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edTempOffsetCount")->Text.c_str())!=Temperature.iCintactCntForTempOffsetAtInitial)
        return true;
    else if(atoi(EL<TEdit>("TfTemp_Set", "edTempDownContactDelay")->Text.c_str())!=Temperature.iCintactDelayCntForInitTempOffset)
        return true;
    //==============Base-point========================
    else if((Temperature.iTempMode==1)!=EL<TRadioButton>("TfTemp_Set", "rb1Point")->Checked)
        return true;
    else if((Temperature.iTempMode==2)!=EL<TRadioButton>("TfTemp_Set", "rb2Point")->Checked)
        return true;
    else if((Temperature.iTempMode==4)!=EL<TRadioButton>("TfTemp_Set", "rb3Point")->Checked)
        return true;
    else if((Temperature.iTempMode==8)!=EL<TRadioButton>("TfTemp_Set", "rb5Point")->Checked)
        return true;
    else if((Temperature.iTempMode==16)!=EL<TRadioButton>("TfTemp_Set", "rb6Point")->Checked)                     //Ztex 2024.07.27 Add 6 Point Temperature Offset
        return true;
    //==============Index Heating Mode================
    else if(EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex!=Temperature.iIndexHeatMode)
        return true;
    else if(EL<TEdit>("TfTemp_Set", "edWorkTemp")->Text!=FormatFloat("0.0", Temperature.fWorkTemperBase))
        return true;
    else
        return false;

}

// golden uTemp_Set.cpp:5307  TfTemp_Set::DisplayTargetTempEdit(bool Visible)
static void TS_DisplayTargetTempEdit(bool Visible)
{

    if(ATC_SYSTEM==eNewATCSystem &&
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                          //Steven 20181102 : For QTI全球統一Offset
    {
        EL<TMemo>("TfTemp_Set", "palIndiTemp")->Visible=true;
        EL<TMemo>("TfTemp_Set", "palIndiTemp")->Text   ="Cal Offset";
        for(int j=0; j<tcTotalCount; j++)
        {
            if((j>=tcAa1 && j<=tcBd2) || (j>=tcAe1 && j<=tcBh2))
                myTempPal[j]->edIndiTemp->Visible=true;
            else
                myTempPal[j]->edIndiTemp->Visible=false;
        }
    }
    else
    {
        for(int j=0; j<tcTotalCount; j++)
        {
            myTempPal[j]->edIndiTemp->Visible=Visible;
        }

        if(USE_16_HEATER==eht16Heater       ||                                  //Steven 20120606 : 16溫控器 8Site使用Hontech頭
           USE_16_HEATER==eht16HeaterEJ1N   ||
           USE_16_HEATER==eht32HeaterEJ1N   ||                                  //Steven 20140923 : Index使用EJ1N版32組加熱器
           USE_16_HEATER==eht32HeaterKT4H   ||                                  //Steven 20150211 : Index使用KT4H版32組加熱器
           USE_16_HEATER==eht16HeaterDTME08 ||                                  //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
           USE_16_HEATER==eht32HeaterDTME08)                                    //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {
            for(int j=tcHead1; j<=tcHead4; j++)
            {
                myTempPal[j]->edIndiTemp->Visible=false;
            }
        }

        if(iSocketBaseTempCount==eDut2ea)                                       //Steven 20140116 : Socket數量,改用數字設定
        {
            myTempPal[tcDUT4]->edIndiTemp->Visible=false;
            myTempPal[tcDUT3]->edIndiTemp->Visible=false;
            myTempPal[tcSocket]->edIndiTemp->Visible=false;
        }
        else if(iSocketBaseTempCount==eDut4ea)                                  //Steven 20141120 : Fixed 溫度設定
        {
            myTempPal[tcSocket]->edIndiTemp->Visible=false;
        }

        if(ATC_SYSTEM==eATCUninstall || ATC_SYSTEM==eNonChamber)                //20141204 ChungHung ????
            myTempPal[tcChamber]->edIndiTemp->Visible=false;
        myTempPal[tcCCD]->edIndiTemp    ->Visible=false;

        if(REAL_TIME_CCD==true && RTC_TemperNumber==2)                          //Isaac 20201217 : RTC CCD增加第二組感溫
        {
            myTempPal[tcCCD_2]->edIndiTemp    ->Visible=false;
        }
        myTempPal[tc2D]->edIndiTemp     ->Visible=false;                        //wei 20160524 2D溫度

        EL<TEdit>("TfTemp_Set", "edWorkTemp")->Visible=!Visible;

        myTempPal[tcOutSht1]->edIndiTemp->Visible=(Tri_Temp_Machine==1);
        myTempPal[tcOutSht2]->edIndiTemp->Visible=(Tri_Temp_Machine==1);
        if(Tri_Temp_Machine==1)
        {
            for(int i=0; i<4; i++)
                myTempPal[tcDUT1+i]->edIndiTemp->Visible=true;
        }

        for(int i=0; i<6; i++)
            myTempPal[tcBase1+i]->edIndiTemp->Visible=(Tri_Temp_Machine==1);

        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            EL<TMemo>("TfTemp_Set", "palIndiTemp")->Text="Set Temp";

        if(INDEXDOORHEATER==1)                                                  //Ztex 2023.10.23 Add Index Door Heater
        {
            myTempPal[tcDoor1]->edIndiTemp->Visible=true;
            myTempPal[tcDoor2]->edIndiTemp->Visible=true;
        }
    }

}

