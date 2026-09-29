// =============================================================================
//  forms/fProductionInfo.cpp  --  definitions for the fProductionInfo facade
//
//  AI(W906-W7-L1-Wave0) 20260801: NEW FILE.  See forms/fProductionInfo.h for the
//  golden provenance, the two-requests merge, and the pre-existing out-arm
//  variant extern situation this definition resolves.
// =============================================================================
#include "forms/fProductionInfo.h"
// AI(W906-FW3-PI1) 20260827: includes added for the FW3-PI1 read-only batch.
// All are pre-existing ports registered in ht9045_globals (Config.cpp's
// IniConfig home is actually cprod.cpp; cmydef.cpp; LastSet.cpp) or, for
// forms/fMain.h, ht9045_forms itself (the SAME target as this file, so no
// link-boundary concern) -- verified against CMakeLists.txt before use.
// MachineType.h/cprod.h/aHotPlateSubstrate.h/cSocket.h/Motor/mymotor.h/
// canary_support.h are DELIBERATELY NOT included here even though the
// suggested batch's golden functions reference symbols from them
// (ShowMyMessage/RecordProcess/ArmData/TestSocket/MOT[]/...) -- those symbols
// live only in ht9045_sm/ht9045_motor, which this file's target
// (ht9045_forms) does not and, per CMakeLists.txt:2249-2295, cannot link
// (target cycle). See forms/fProductionInfo.h's LINK-BOUNDARY EXCLUSIONS note
// for the full list of functions this cut, and the wave report for the
// per-symbol trace.
#include "forms/fMain.h"    // GetSetUpName(): fMain->cbSetupFileName->Text (same target, ht9045_forms)
#include "Config.h"          // IniConfig (CheckContactForceExist/bIntegerTimeCheck/bTimerCheck) -- defined cprod.cpp:50, ht9045_globals
#include "cmydef.h"          // iTo3Unload (ClearUnloaderTrayQty) -- cmydef.cpp, ht9045_globals
#include "LastSet.h"         // LastSet (BinCT / iN14_9_* tray counters) -- LastSet.cpp, ht9045_globals
#include "MachineType.h"     // e3Auto1..e3Fix5 (GetBin0_8List) -- header-only constants, no link cost
#include <cmath>              // pow() (cDynamicMultiContinualPassBinBySocket::GetMultiplierNum)
// AI(W906-FW3-PI2) 20260827: 1 include added for the FW3-PI2 read-only batch.
// CosFunction.h -- CosFunction (bEnableRPLog) -- defined cprod.cpp:52, ht9045_globals
// (same file/target as IniConfig above). SystemStart/InitialOK (TimeCount/bEnableRPLog)
// are already visible via cmydef.h, included above. Same link-boundary discipline as
// FW3-PI1: verified against CMakeLists.txt before use -- see forms/fProductionInfo.h's
// FW3-PI2 block comment for the per-function trace of what this wave did NOT translate.
#include "CosFunction.h"
// AI(W906-FW3-PIOEE) 20260828: 2 個 include。
// ⚠ 上面 FW3-PI1 那段寫著「cprod.h ... 是 DELIBERATELY NOT included」，
//   那在當時是實情，**不改寫**；但它的前提（本檔屬 ht9045_forms）
//   已於 commit ba3683e 失效：本檔現在是 **ht9045_sm** 的來源
//   （CMakeLists.txt:2722），而 ht9045_sm link 了 io/motor/globals/vclcompat/
//   secsgem/forms，所以那道牆不存在了。見 DEVLOG 20260828 XII/XIII。
#include "cprod.h"           // Prod.iIsPassT6[] (CalculateOEEReport) -- cprod.cpp, ht9045_globals
#include "forms/fObserver.h" // fObserver->sTestReceiveTime/sTestIndexZTime/dOEEIndexCycleTime
// AI(W906-FW3-PICTL) 20260829: 4 個 include，全部因本檔自 ba3683e 起屬
// ht9045_sm 而合法（舊的 ht9045_forms 連結牆已不存在）。
#include "cSocket.h"             // ArmData[]/ArmSKET[][]、TEST_CATEGORY、
                                  //   Now/OldControlBinCategory（定義在 cSocket.cpp:230-231，
                                  //   註冊於 CMakeLists.txt:2567 的 ht9045_sm；
                                  //   nm 在 libht9045_sm.a 看到 B _Now/_OldControlBinCategory）
#include "aHotPlateSubstrate.h"  // TestSocket -- ⚠ 陣阱 #5：`TestSocket` 同時出現於
                                  //   mykitsuck.h 與本檔，且佈局不同。選本檔是**照本樹既有慣例**
                                  //   （cSocket.cpp:165 明文："TestSocket (TMyKitSuck, golden
                                  //   MyKitSuck.h -> this tree's substrate)"），不自己判斷。
#include "Motor/mymotor.h"       // MOT[]、TMyMotor::HowManyDevice/fHasTray (CalICCountInHandler)
#include "canary_support.h"      // RecordProcess (bEnableIPSC) -- 四個既有例外之一，canary_support.cpp
                              //   -- cObserver.cpp, ht9045_sm (same target as this file since ba3683e)

TfProductionInfo::TfProductionInfo() : sLoadMO_TestFlow("") {
    // AI(W906-FW3-PI1) 20260827: widget pointer for the FW3-PI1 batch, same
    // ctor-body `new` idiom as every other facade (e.g. forms/fLotInfo.cpp's
    // edtSysLotID).  _bOEEStartLotSuccess/h/n/s/z and the array fields all use
    // in-class default member initializers instead (forms/fBinSel.h
    // precedent), so only the pointer member needs a line here.
    edInsertOPID_HALT = new TfLotInfoEdit();
    // AI(W906-FW3-PIGSV) 20260829: 同一個 ctor-body `new` 慣例。
    pn_ErrorMsg_HALTStatus  = new TfProductionInfoPanel();
    pn_ErrorMsg_PauseStatus = new TfProductionInfoPanel();
}
// Offline: the real body refreshes the production-info form's per-tray IC-count
// display (golden ProductionInfo.h:377); there is no headless equivalent, so this
// is a no-op sink -- the same idiom every other UI-refresh facade method uses.
void TfProductionInfo::CalTrayICCount(int /*iWitchTray*/) {}
TfProductionInfo *fProductionInfo = new TfProductionInfo();

// ===========================================================================
// AI(W906-FW3-PI1) 20260827: FW3-PI1 wave -- read-only batch, faithful
// translation of golden ProductionInfo/ProductionInfo.cpp.  Each function's
// golden line span is cited on its own opening comment; see forms/
// fProductionInfo.h for the matching declaration-site citations.
// ===========================================================================

// golden ProductionInfo.cpp:1592-1598
AnsiString TfProductionInfo::MyBoolToString(bool b)
{
    if(b==true)
        return "Use";
    else
        return "No Use";
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5839-5843
AnsiString TfProductionInfo::GetNowTime()
{
    TDateTime _now=utimetool.GetNow();
    return utimetool.TDatetmeToAnsiLog(_now);
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5119-5123
int TfProductionInfo::GetNowTimeSec()
{
    DecodeTime(Now(), h, n, s, z);
    return h*3600+n*60+s;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5845-5848
void TfProductionInfo::SetPISTime()
{
    sPI_STime=GetNowTime();
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5850-5853
AnsiString TfProductionInfo::GetPISTime()
{
    return sPI_STime;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:369-372
AnsiString TfProductionInfo::OEE_GetMO()
{
    return _sOEE_MO;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:1175-1178
bool TfProductionInfo::IsOEEStartLotSuccess()
{
    return _bOEEStartLotSuccess;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5382-5385
AnsiString TfProductionInfo::GetSetUpName()
{
    return fMain->cbSetupFileName->Text;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5834-5837
AnsiString TfProductionInfo::GetScheduleName()
{
    return sLoadMO_MO+"_"+sLoadMO_TestFlow;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5917-5920
void TfProductionInfo::SetInsertOPIDStr(const AnsiString& s)
{
    edInsertOPID_HALT->Text=FilterAlphanumeric(s);
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5922-5925
AnsiString TfProductionInfo::GetInsertOPIDStr()
{
    return FilterAlphanumeric(edInsertOPID_HALT->Text);
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5927-5939
AnsiString TfProductionInfo::FilterAlphanumeric(const AnsiString& s)
{
    AnsiString result;
    for (int i = 1; i <= s.Length(); i++)
    {
        char c = s[i];
        if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') )
        {
            result += c;
        }
    }
    return result;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4931-4934
void TfProductionInfo::CheckNewDayAndSubtract(int &iTimeSec)
{
    iTimeSec=iTimeSec%86400;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4936-4947
bool TfProductionInfo::CheckContactForceExist()
{
    if( sDevice_Pin_Force==""  ||
        sDevice_Pin_Force=="0" ||
        sDevice_Pin_Count==""  ||
        sDevice_Pin_Count=="0" ||
        IniConfig.bD06ContactOffsetDefaultValue==false
    )
        return false;
    else
        return true;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4506-4520
bool TfProductionInfo::bIntegerTimeCheck()
{
    DecodeTime(Now(), h, n, s, z);
    if(IniConfig.iN14_16_IPSCInterval==120)                                     //2Hour
    {
        if(h%2==0)                                                              //0 2 4 6 8 10 12 14 16 18 20 22 24 偶數整點。
            return true;
    }
    else
    {
        if(n==0)                                                                //整點
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4522-4538
bool TfProductionInfo::bTimerCheck(int iTimeMode)
{
    static int iOldHour=-1;
    static int iOldmin=-1;
    DecodeTime(Now(), h, n, s, z);
    if((iTimeMode==1 && n==0) ||                                                //整點
       (iTimeMode==2 && n==30))                                                 //半小時
    {
        if(iOldHour!=h || iOldmin!=n)
        {
            iOldHour=h;
            iOldmin=n;
            return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5773-5783
bool TfProductionInfo::IsContinueFailAlarm()
{
    if(sLoadMO_TestFlow.SubString(1, 1)=="R")
    {
        return false;
    }
    else
    {
        return true;
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:495-515
AnsiString TfProductionInfo::GetCSVLineData(int iDataNum, AnsiString sCSVLineStr)
{
    AnsiString sRes="";
    for(int i=0; i<iDataNum-1; i++ )
    {
        if(sCSVLineStr.Pos(",")>0)
        {
            sCSVLineStr=sCSVLineStr.SubString((sCSVLineStr.Pos(",")+1), (sCSVLineStr.Length()-sCSVLineStr.Pos(",")));
        }
        else
        {
            sCSVLineStr="";
        }
    }
    sRes=sCSVLineStr;
    if( sCSVLineStr.Pos(",")>0)
    {
        sRes=sCSVLineStr.SubString(1, sCSVLineStr.Pos(",")-1);
    }
    return sRes;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:6065-6077
std::vector<int> TfProductionInfo::GetBin0_8List()
{
    int iBin0_8[8]= {
                     e3Auto1,  e3Auto2,  e3Auto3,   e3Fix1,
                     e3Fix2,   e3Fix3,   e3Fix4,    e3Fix5,
                    };
    std::vector<int> list;
    for(int iarea=0; iarea<8; iarea++)
    {
        list.push_back(iBin0_8[iarea]);
    }
    return list;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4393-4405
void TfProductionInfo::CalculateNowUnloaderTrayQty(bool bIsClear)
{
    std::vector<int> lsBin0_8=GetBin0_8List();
    for(int iBin=0; iBin<6; iBin++)                                                      //Auto1~3，Fix1~3
    {
        iNowUnloaderTrayQty[iBin]+=LastSet.BinCT[0][lsBin0_8[iBin]]-iLastUnloaderTrayQty[iBin];
        iLastUnloaderTrayQty[iBin]=LastSet.BinCT[0][lsBin0_8[iBin]];
        if(bIsClear)
        {
            iLastUnloaderTrayQty[iBin]=0;
        }
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4429-4440
int TfProductionInfo::CalculateUnloadTotalICQty()                               //放到 Unloader 的總數
{
    int iTotalLoadQty;
    CalculateNowUnloaderTrayQty();

    iTotalLoadQty=0;
    for(int i=0; i<8; i++)
    {
        iTotalLoadQty=iTotalLoadQty+iNowUnloaderTrayQty[i];                     //Bin1 ~ Bin8  == Auto1~Auto3 Fix1~Fix3
    }
    return iTotalLoadQty;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4460-4467
void TfProductionInfo::ClearUnloaderTrayQty()
{
    for(int i=0; i<8; i++)
    {
        iNowUnloaderTrayQty[i] =0;
        iLastUnloaderTrayQty[i]=LastSet.BinCT[0][iTo3Unload[i]];
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:3329-3339
void TfProductionInfo::ClearTrayCnt()
{
    for(int i=0; i<256; i++)
    {
//        LastSet.iN14_9_TrayICCnt[i]         =0;                                 //Tested IC
        LastSet.iN14_9_FullTrayCnt[i]       =0;                                 //Full Tray Tray Qty
        LastSet.iN14_9_PartialTrayCnt[i]    =0;                                 //Not Full Tray Tray Qty
        LastSet.iN14_9_PartialTrayICCnt[i]  =0;                                 //Not Full Tray IC Qty
        LastSet.iN14_9_InHandlerICCnt[i]    =0;                                 //In Machine Tray IC Qty
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5955-5963
int cDynamicMultiContinualPassBinBySocket::GetMultiplierNum()
{
    int iBack=1;
    if(IniConfig.bN14_24_DynaMultiContinuPassSocket)
    {
        iBack=pow(2,iDynamicThresholdNum);
    }
    return iBack;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5965-5969
void cDynamicMultiContinualPassBinBySocket::AddThresholdNum()
{
    if(iDynamicThresholdNum<IniConfig.iN14_24_DyMultiPassPower)//max value =5
        iDynamicThresholdNum++;
}
//---------------------------------------------------------------------------

// ===========================================================================
// AI(W906-FW3-PI2) 20260827: FW3-PI2 wave -- second read-only batch.  Each
// function's golden line span is cited on its own opening comment; see
// forms/fProductionInfo.h's FW3-PI2 block for the matching declaration-site
// citations and the per-function exclusion trace for the 7 priority names
// that were NOT translated this wave.
// ===========================================================================

// golden ProductionInfo.cpp:364-367
void TfProductionInfo::OEE_SetMO(AnsiString sMO)
{
    _sOEE_MO=sMO;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:374-377
void TfProductionInfo::OEE_SetHandlerID(AnsiString sHDID)
{
    _sOEE_HandlerID=sHDID;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:920-940
void TfProductionInfo::TimeCount()
{
    if(IniConfig.bN14_1_EnableOEEFunction==false ||
       IniConfig.iN14_1_OEERecordCycleTime==0)
    {
        return;
    }
    static int iLastSecond=0;
    int iSecond=atoi(FormatDateTime("ss", Now()).c_str());   // golden: Now().FormatString("ss") -- vclcompat has no
                                                               // TDateTime::FormatString member, established substitution
                                                               // (ainarm9045.cpp:916 and others)
    iSecond=(iSecond+500)*1000;
    dtNowDateTime=Now();
    if(iSecond!=iLastSecond)
    {
        iLastSecond=iSecond;
        //Power On 開始時間
        if(SystemStart==true)
        {
            _iOEE_RunTime++;
        }
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:1126-1134
void TfProductionInfo::SetNextRecordDateTime()
{
    int iRecordCycleTime=IniConfig.iN14_1_OEERecordCycleTime;
    iNextDateTime=GetNowTimeSec();
    if(IniConfig.iN14_1_OEERecordCycleTime!=0)
    {
        iNextDateTime=iNextDateTime-iNextDateTime%iRecordCycleTime+iRecordCycleTime;
    }
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:2818-2822
void TfProductionInfo::SetStartStatus()
{
    _sOEE_ActivityID="";
    _sOEE_Status="Auto Run";
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:3958-3974
bool TfProductionInfo::bEnableRPLog()
{
    if(CosFunction.bRecipeParameterDefault   ==false ||
       CosFunction.bRecipeParameterDefaultLog==false ||
       InitialOK==false)
    {
        return false;
    }

    if(IniConfig.bN14_20_DefaultRecipeChangeLog             ==false ||
       IniConfig.asN14_20_ChangeLogPath                     ==""    ||
       IniConfig.bN14_20_DefaultRecipeChangeLogCycleRecord  ==false)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:4949-4954
void TfProductionInfo::SetOffsetContactForce(AnsiString asValue)
{
    if(asValue=="")
        asValue="0";
    dOffsetContactForce=atof(asValue.c_str());
}
//---------------------------------------------------------------------------
// GATE (PI2-G1) -- AI(W906-FW3-PI2-fix) 20260827: EnableInArmAutoCalSuckZ
// (golden ProductionInfo.cpp:5907-5910) 與 EnableOutArmAutoCalSuckZ
// (golden :5912-5915) 本波交付後被主迴圈移除，**理由是連結期缺符號**：
//
//   undefined reference to `bEnableInarmSuckZAuto'
//   undefined reference to `bEnableOutarmSuckZAuto'
//
// 兩個全域在 `cmydef.cpp:6102-6103` **確實有定義**，但那一段整個在
// `cmydef.cpp:6011` 開始的 `#if 0 // TODO(W6): function bodies depend on
// untranslated globals/state machines` 內——**文字存在，編譯器看不到**。
// `cmydef.h:5806-5807` 只有 extern 宣告，所以 `-fsyntax-only` 完全過關，
// `nm --undefined-only` 也只會列出它們而不會說「沒人提供」。
// 是全新 build dir 的連結期把它抓出來的（gate pi2 兩側 build 都因此失敗）。
//
// **要解它必須先解 cmydef.cpp:6011 那個 #if 0**，那是行為變更、要單獨一波。
// 這裡刻意不留退化版本（例如恆回 false）——**缺符號比錯答案好**，
// 一個恆假的 EnableAutoCalSuckZ 會讓未來的呼叫者靜默走錯分支。
//---------------------------------------------------------------------------
// AI(W906-FRW-S70) 20260926: 上面 GATE (PI2-G1) 解除 —— 它寫的前提（cmydef.cpp TODO(W6) 關著 bEnableIn/OutarmSuckZAuto）
// 本次已不成立（cmydef.cpp 解出那兩個定義；見 forms/fProductionInfo.h 同編號註記）。照 golden 補回兩支，不是退化版本：
// 值是 FileRW/AutoCalSuckZ.cpp 照 golden 讀進來的（只有 CosFunction.bOEEFunction 的機台會讀；其他機台維持建構子的 false，
// 與 golden 相同）。
// golden ProductionInfo.cpp:6071-6074（V912）
bool TfProductionInfo::EnableInArmAutoCalSuckZ()
{
    return bEnableInarmSuckZAuto;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:6076-6079（V912）
bool TfProductionInfo::EnableOutArmAutoCalSuckZ()
{
    return bEnableOutarmSuckZAuto;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5949-5953 -- golden's OWN body is fully commented
// out (dead/no-op in golden itself); translated verbatim as an empty body.
void TfProductionInfo::InitialStringGrid(TStringGrid * /*sg*/)
{
//    sg->Colcount();
//    int iCol=sg->Colcount;
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5821-5832
void TfProductionInfo::GetFTP_Setting(AnsiString &asUserID, AnsiString &asPassword, AnsiString &asHost)
{
    #ifdef SOFT_SIMULTE
    asUserID="HONPREC";
    asPassword="27025312";
    asHost="127.0.0.1";
    #else
    asUserID=IniConfig.asN14_3_OEEFTPUserName;
    asPassword=IniConfig.asN14_3_OEEFTPPassword;
    asHost=IniConfig.asN14_3_OEEFTPHost;
    #endif
}
//---------------------------------------------------------------------------
// golden ProductionInfo.cpp:5254-5278 -- self-contained helper (no golden-global
// references); its only golden caller, GetBinTraySetting, is NOT translated this
// wave -- see forms/fProductionInfo.h's FW3-PI2 block comment.
bool TfProductionInfo::SetBinTraySetting(TStringList* tlBinTray,AnsiString asSource,AnsiString asBin)
{
    bool bGetE=false;
    if(asSource.Trim()=="")
        return bGetE;
    int iTemp=0;
    asSource=asSource.Trim();
    TStringList* tempTL=new TStringList();
    tempTL->CommaText=asSource;
    AnsiString asString="";
    for(int i=0; i<tempTL->Count; i++)
    {
        asString=tempTL->Strings[i];
        if(asString=="E")
        {bGetE=true;}
        else
        {
            iTemp=atoi(tempTL->Strings[i].c_str());
            iTemp=iTemp>15?0:iTemp;
            tlBinTray->Strings[iTemp]=asBin;
        }
    }
    delete tempTL;
    return bGetE;
}
//---------------------------------------------------------------------------

// =============================================================================
//  AI(W906-FW3-PIOEE) 20260828: OEE 報表三支，逐字翻自 golden
//  ProductionInfo/ProductionInfo.cpp。三支都是純計算＋組字串，
//  **零 widget 存取、零機台動作、零檔案 I/O、零對外命令**
//  （tools/wavescan/screen_methods.py 實測：三支全部「乾淨」）。
//
//  ⚠ 同檔其他 OEE 方法**仍然不翻**，而且不是因為連結：
//    * LoadMOInformation (golden :309 行) -- **寫檔/寫 ini**
//    * CheckOEE_WhenStart (83 行)        -- **送命令/上傳**
//    安全軸與可達軸是兩個獨立的軸；搬進 ht9045_sm 只解開了後者。
//    見 DEVLOG 20260828 IX / XII。
//
//  ⚠ DEVIATION -- golden :634 的 `DateSeparator='/';`（Sam 20210225：增加保護）
//    **未翻**，照本樹既有慣例：見 cObserver.cpp:54-56「i`DateSeparator` 的
//    指派已捨棄（inert）」與 cMyDB.cpp:897 的同型註記。
//    本 port 的 TDateTime::FormatString 不讀那個全域，所以不翻就是零差異；
//    不是「省略一行」而是「那一行在這棵樹上沒有可觀察效果」。
//
//  ⚠ 三個 golden 自帶的瑕疵，**逐字重現，沒有靈修**：
//    (1) golden :654 `if(sJamCNT<=0)` 拿 **AnsiString 跟 0 比**（作者顯然想寫
//        `_iOEE_JamCount<=0`）。BCB6 走非 explicit 的 AnsiString(int) -> "0"，
//        所以實際是**字串比較**。本 port 一樣：vclcompat/AnsiString.h:67 非 explicit，
//        而 :233 的 operator<= **只有 (AnsiString, AnsiString) 一個版本**（沒有 const char*
//        多載，所以不會被當成空指標）——**逐字照抄就重現相同行為**。
//        ⚠ 但要說清楚：**這不是一個會出錯的 bug。** 實測逐值驗過
//        （scratchpad 探針）：IntToStr(0)="0" -> true、IntToStr(-5)="-5" 因 '-'(0x2D)<'0'(0x30)
//        -> true、IntToStr(1)="1" -> false、IntToStr(100)="100" 因 '1'>'0' -> false。
//        IntToStr 的輸出永遠不帶前導零，所以**字串比較對每一個 int 值都與
//        整數比較同結果**。它是寫法瑕疵，沒有可觀察效果。
//    (2) golden :765/:777 `sprintf("%0.2f%", ...)` 結尾是殘缺的格式規格。照翻。
//    (3) golden :746/:753 的錯誤訊息寫完下一行馬上被覆寫成 "0"/"0.00%"，
//        那些訊息永遠不可觀察。照翻（它們是 golden 的意圖記錄）。
//
//  ⚠ MEASURED BEHAVIOUR NOTE -- `fObserver->sTestReceiveTime` 在本 port
//    **沒有任何寫入點**（見 forms/fObserver.h 同日註記），
//    所以輸出 CSV 的那一欄恆為 ""。這是忠實的，不是缺陷。
//
//  ⚠ 本波**沒有任何呼叫者**：golden 的呼叫點在未翻的 OEE 流程裡。
//    所以這三支落地後**不改變任何執行期行為**，也不可觀察。
// =============================================================================

void TfProductionInfo::SetOEEReportMessage()
{
    _sOEE_FLOW=sLoadMO_TestFlow;
    //IN Q'ty
    int iNowINQty=LastSet.SendCT[0];
    if(iNowINQty<iLastINQty)
    {
        iLastINQty=LastSet.SendCT[0];
    }
    _iOEE_sINQty=iNowINQty-iLastINQty; //IN Q'ty
    iLastINQty=iNowINQty;
    int iActuralOut=0;
    int iNowBIN=0;
    std::vector<int> lsBin0_8=GetBin0_8List();
    //BIN0~8
    for(unsigned int i=0; i<lsBin0_8.size(); i++)                                //Sam 20171018 : OEE Bug 修正
    {
        iNowBIN=LastSet.BinCT[0][lsBin0_8[i]];                                   //Sam 20171018 : OEE Bug 修正
        if(iNowBIN<iLastBIN[i])
        {
            iLastBIN[i]=LastSet.BinCT[0][lsBin0_8[i]];                           //Sam 20171018 : OEE Bug 修正
        }
        _iOEE_sBIN[i]=iNowBIN-iLastBIN[i];
        iLastBIN[i]=iNowBIN;
        iActuralOut=iActuralOut+iNowBIN;
    }
    //Actural Out
    if(iActuralOut<iLastActuralOut)
    {
        iLastActuralOut=iActuralOut;
    }
    _iOEE_ActuralOut=iActuralOut-iLastActuralOut; //Actural Out
    iLastActuralOut=iActuralOut;
}
//---------------------------------------------------------------------------

void TfProductionInfo::CalculateOEEReport(AnsiString &sResultOEEReport,bool bSaveNow)//Jimmychiu 20231023 : 非整點製作報表，時間準確到秒
{
    AnsiString sOEEReportMsg = "";
    SetOEEReportMessage();
    _dtOEE_StartDateTime    =Now(); //Start Lot時間
    // golden :634 `DateSeparator='/';` -- DEVIATION, 見檔頭說明（inert）。
    // AI(W906-FW3-PIOEE) 20260828: golden 這四行用 `<TDateTime>.FormatString(fmt)`，
    // 而 vclcompat 的 TDateTime **沒有這個成員**（vclcompat/TDateTime.h:33-53）。
    // 本樹既有的對應是自由函式 `FormatDateTime(fmt, dt)`
    //（vclcompat/TDateTime.h:67）——先例與理由記在 ainarm9045.cpp:7219-7223，
    // 用例在 ainarm9045.cpp:916。**不自創也不擴 vclcompat**。
    // 語意已驗：該實作將 token 小寫化後比對（所以大寫 "YYYYMMDD" 一樣），
    // 且「其他字元一律字面輸出」（TDateTime.cpp:255-257），
    // 所以 "_" 與 "HHNN00" 裡的字面 "00" 都與 golden 相同。
    AnsiString sStartDate   =FormatDateTime("yyyy/mm/dd", _dtOEE_StartDateTime); //Start Date
    AnsiString sStartTime   =FormatDateTime("hh:nn:ss", _dtOEE_StartDateTime); //Start Time
    if(bSaveNow==true)                                                          //Jimmychiu 20231023 : 非整點製作報表，時間準確到秒
    {
        sStartDateTime          =FormatDateTime("YYYYMMDD_HHNNSS", _dtOEE_StartDateTime); //Jimmychiu 20230824 : 符合Greatek資訊部抓資料條件，秒數必須為0
    }
    else
    {
        sStartDateTime          =FormatDateTime("YYYYMMDD_HHNN00", _dtOEE_StartDateTime); //Jimmychiu 20230824 : 符合Greatek資訊部抓資料條件，秒數必須為0
    }
    AnsiString sPowerTime   =IntToStr(_iOEE_PowerTime); //Power Time(Sec)
    AnsiString sRunTime     =IntToStr(_iOEE_RunTime-_iOEE_SoakTime); //Run Time
    AnsiString sTempTime    =IntToStr(0); //Temp Time
    AnsiString sSoakTime    =IntToStr(_iOEE_SoakTime); //Soak Time
    AnsiString sStopTime    =IntToStr(_iOEE_StopTime); //Stop Time
    AnsiString sPauseTime   =IntToStr(_iOEE_PauseTime);
    AnsiString sJamCNT      =IntToStr(_iOEE_JamCount); //Jam CNT
    AnsiString sServiceCNT  =IntToStr(_iOEE_ServiceCount); //Service CNT
    AnsiString sMTBA        =""; //MTBA
    if(sJamCNT<=0)   // golden :654 逐字保留：AnsiString vs 0 -> AnsiString("0") 字串比較，實測與整數比較等價（見檔頭 (1)）
    {
        sMTBA=sRunTime;
    }
    else
    {
        if(_iOEE_JamCount>0)
        {
            sMTBA=IntToStr(_iOEE_RunTime/_iOEE_JamCount);
        }
        else
        {
            sMTBA="_iOEE_JamCount = 0 Error";
            sMTBA="0";
        }
    }

    AnsiString sMTBF=""; //MTBF
    if(_iOEE_ServiceCount<=0)
    {
        sMTBF=sRunTime;
    }
    else
    {
        if(_iOEE_ServiceCount>0)
        {
            sMTBF=IntToStr(_iOEE_RunTime/_iOEE_ServiceCount);
        }
        else
        {
            sMTBF="_iOEE_ServiceCount = 0 Error";
            sMTBF="0";
        }
    }

    AnsiString sHOEEPercent=""; //HOEE%
    if(_iOEE_PowerTime>0)
    {
        sHOEEPercent=ChangeToPercentage((double)_iOEE_RunTime-_iOEE_SoakTime, (double)_iOEE_PowerTime); //Steven 20250820 : 針對除以0加上保護
    }
    else
    {
        sHOEEPercent="_iOEE_PowerTime = 0 Error";
        sHOEEPercent="0.00%";
    }

    AnsiString sFLOW    =sLoadMO_TestFlow;
    AnsiString sStatus  =_sOEE_Status;
    AnsiString sJamCode =_sOEE_JamCode;
    AnsiString sINQty   =IntToStr(_iOEE_sINQty); //IN Q'ty
    AnsiString sBIN[8]; //Sam 20171018 : OEE Bug 修正
    int iPassBin=0;
    for(int i=0; i<8; i++)    //Sam 20171018 : OEE Bug 修正                     //QQQ
    {
        sBIN[i]=IntToStr(_iOEE_sBIN[i]);                                        //BIN1
        if(Prod.iIsPassT6[i]==1)                                                //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6 //Steven 20240701 : 0 --> 1
        {
            iPassBin=iPassBin+_iOEE_sBIN[i];
        }
    }
    AnsiString sYIELD=""; //YIELD
    if(_iOEE_ActuralOut>0)
    {
        sYIELD=ChangeToPercentage((double)iPassBin, (double)_iOEE_ActuralOut);  //Steven 20250820 : 針對除以0加上保護
    }
    else
    {
        sYIELD="_iOEE_sINQty = 0 Error";
        sYIELD="0.00%";
    }
    AnsiString sActuralOut=IntToStr(_iOEE_ActuralOut); //Actural Out
    AnsiString splanout=""; //IntToStr(_iOEE_planout); //plan out
    if((fLastIndexTime+fLastTestTime)>0.0)
    {
        splanout=IntToStr((int)(ChangeToFloatNonPcnt((double)(atof(sPowerTime.c_str())), (double)(fLastTestTime))));       //Mylin 20170517 Modify OEE Clear Count Issue
    }
    else
    {
        splanout=sActuralOut;
    }
    _iOEE_planout=atoi(splanout.c_str());

    AnsiString sMOEEPercent=""; //MOEE%
    if(_iOEE_planout>0)
    {
        sMOEEPercent=ChangeToPercentage((double)_iOEE_ActuralOut, (double)_iOEE_planout*100.0);
        sMOEEPercent.sprintf("%0.2f%", atof(sMOEEPercent.c_str()));   // golden :765 殘缺格式規格，逐字保留（見檔頭 (2)）
    }
    else
    {
        sMOEEPercent="_iOEE_planout = 0 Error";
        sMOEEPercent="0.00%";
    }

    AnsiString sOEEEPercent=""; //OEEE%
    if(_iOEE_ActuralOut>0 && _iOEE_planout>0)
    {
        sOEEEPercent=FloatToStr((double)iPassBin/(double)_iOEE_ActuralOut*100.0*(double)_iOEE_ActuralOut/(double)_iOEE_planout);
        sOEEEPercent.sprintf("%0.2f%",atof(sOEEEPercent.c_str()));    // golden :777 同上
    }
    else
    {
        sOEEEPercent="_iOEE_sINQty = 0 or _iOEE_planout = 0 Error";
        sOEEEPercent="0.00%";
    }
    AnsiString sMO=sLoadMO_MO;
    AnsiString sTestTime="0.00";
    if(atof(sLoadMO_TestTime.c_str())!=0)
    {
        sTestTime=sLoadMO_TestTime;
    }

    AnsiString sActivityID=_sOEE_ActivityID;
    int iSiteUse=0;
    for(int i=0; i<4; i++)                                                      //Sam 20170731 HT-7045 變數改為 HT-9045 變數
    {
        for(int j=0; j<8; j++)
        {
            if(LastSet.bUseTestSocket[0][i][j]==true)
            {
                iSiteUse++;
            }
        }
    }
    AnsiString sTestSite=IntToStr(iSiteUse);

    sOEEReportMsg  =sStartDate  +","+
                    sStartTime  +","+
                    sPowerTime  +","+
                    sRunTime    +","+
                    sTempTime   +","+
                    sSoakTime   +","+
                    sStopTime   +","+
                    sPauseTime  +","+
                    sJamCNT     +","+
                    sServiceCNT +","+
                    sMTBA       +","+
                    sMTBF       +","+
                    sHOEEPercent+","+
                    sFLOW       +","+
                    sStatus     +","+
                    sJamCode    +","+
                    sINQty      +",";

    for(int i=0; i<8; i++)    //Sam 20171018 : OEE Bug 修正
    {
        sOEEReportMsg=sOEEReportMsg+sBIN[i]+",";
    }

    //KaiChen 20171127 ：超豐 OEE 新增 Test Time、Index Time
    //==>
    AnsiString sTestRecevieTime =fObserver->sTestReceiveTime;
    AnsiString sIndexTime       =fObserver->sTestIndexZTime;
    //<==
    //KaiChen 20171127 ：超豐 OEE 新增 Test Time、Index Time

    if(bUseTwoArm32Site==true   &&
       sIndexTime!="")                                                          //Sam 20180802 (wei) : OEE 32Site 修正
    {
        sIndexTime=FloatToStr(StrToFloat(sIndexTime)+fObserver->dOEEIndexCycleTime);
    }

    sOEEReportMsg=sOEEReportMsg+sYIELD          +","+
                                sActuralOut     +","+
                                splanout        +","+
                                sMOEEPercent    +","+
                                sOEEEPercent    +","+
                                sMO             +","+
                                sTestTime       +","+
                                sTestRecevieTime+","+      //KaiChen 20171127 ：超豐 OEE 新增 Test Time、Index Time
                                sIndexTime      +","+      //KaiChen 20171127 ：超豐 OEE 新增 Test Time、Index Time
                                sActivityID     +","+
                                sTestSite;
    sResultOEEReport=sOEEReportMsg;
    ClearOEECount();
}
//---------------------------------------------------------------------------

void TfProductionInfo::ClearOEECount()
{
    _dtOEE_StartDateTime=Now(); //Start Lot時間
    _iOEE_PowerTime     =0;
    _iOEE_RunTime       =0;
    _iOEE_SoakTime      =0; //Soak時間
    _iOEE_StopTime      =0;

    iLastINQty = LastSet.SendCT[0];
    //Actural Out
    int iActuralOut=0;
    int iNowBIN=0;
    std::vector<int> lsBin0_8=GetBin0_8List();
    for(int iBin=0; iBin<8; iBin++)                                             //Sam 20171018 : OEE Bug 修正
    {
        iNowBIN=LastSet.BinCT[0][lsBin0_8[iBin]];                               //Sam 20171018 : OEE Bug 修正
        iLastBIN[iBin]=iNowBIN;
        iActuralOut=iActuralOut+iNowBIN;
    }
    iLastActuralOut=iActuralOut;

    //plan out
    iplanout            =0;
    _iOEE_JamCount      =0; //Jam次數
    _iOEE_ServiceCount  =0; //Error次數

    _sOEE_Status    ="";
    _sOEE_JamCode   ="";
    _sOEE_ActivityID="";
    _sOEE_TestSite  ="";
    _iOEE_PauseTime =0;
}
//---------------------------------------------------------------------------

// =============================================================================
//  AI(W906-FW3-PICTL) 20260829: Control-Bin / IPSC 八支，逐字翻自 golden
//  ProductionInfo/ProductionInfo.cpp（合計 191 行）。
//
//  兩個軸都篩過才開工（今日付過代價的教訓：安全軸 != 可達軸）：
//    * 安全軸 -- tools/wavescan/screen_methods.py：八支全部「乾淨」。
//      （同批的 GetStringBySeparatedValues 因 ShowMyMessage 被標警報/對話框，
//       **本波不翻**，留給下一波逐案判。）
//    * 可達軸 -- 八支合計 **零個跨模組 `->` 解參考**；
//      外部呼叫只有 RecordProcess / GetTotal / GetSelTrayCT / GetSelBinCT /
//      GetIFError / HowManyDevice / ClearCount / FileExists，全在 ht9045_sm
//      或 sm 連得到的 target。
//
//  ⚠ 本波仍然**沒有接線**：這八支在本 port 沒有呼叫者（golden 的呼叫點
//    在未翻的 OEE / Control-Bin 流程裡），所以**不改變任何執行期行為**。
//
//  ⚠ golden 行為逐字保留的兩處（**不是缺陷，是 golden 的意圖**）：
//    (1) bEnableIPSC 的縮排在 golden 就是歪的（:3934 那個 `if` 比兄弟少縮），照翻。
//    (2) UpdateControlBinCount 的 `memcpy(..., sizeof(NowControlBinCategory))`
//        拿**整個結構的大小**去複製一個成員陣列的起始位址。
//        這在 golden 是有意的（一次拷貝整個快照），因為兩個物件同型且
//        iCountCategory 是第一個成員；逐字照翻，**不改寫成 `=` 費購**。
// =============================================================================

void TfProductionInfo::CalICCountInHandler()
{
    for(int i=0; i<eTrayCount; i++)
    {
        if(MOT[iMMAuto[i]].fHasTray==true)
        {
            LastSet.iN14_9_InHandlerICCnt[i]=MOT[iMMAuto[i]].HowManyDevice();   //In Machine Tray IC Qty
        }
    }
}
//---------------------------------------------------------------------------

bool TfProductionInfo::bEnableIPSC()
{
    AnsiString sMsg="";
    if(CosFunction.bOEEFunction==false ||
       IniConfig.bN14_16_EnableIPSC==false ||
       InitialOK==false)
    {
        return false;
    }

        if(IniConfig.asN14_16_ExecutFilePath=="" ||
           IniConfig.asN14_16_FlagFilePath==""   ||
           FileExists(IniConfig.asN14_16_ExecutFilePath)==false ||
           IniConfig.iN14_16_IPSCInterval<=0)
        {
            sMsg="";
            if(IniConfig.asN14_16_ExecutFilePath=="")
            {
                sMsg.sprintf("IPSC Execute File Path Name is NULL!!");
                RecordProcess(sMsg);
            }

            if(FileExists(IniConfig.asN14_16_ExecutFilePath)==false)
            {
                sMsg.sprintf("IPSC Can Not Find Execute File => %s", IniConfig.asN14_16_ExecutFilePath);
                #ifndef SOFT_SIMULTE
                RecordProcess(sMsg);
                #endif
            }

            if(IniConfig.asN14_16_FlagFilePath=="")
            {
                sMsg.sprintf("IPSC Flag Ini File Path Name is NULL!!");
                RecordProcess(sMsg);
            }

        if(IniConfig.iN14_16_IPSCInterval<=0)
            {
                sMsg.sprintf("IPSC Record Cycle Time is less than 1 min");
                RecordProcess(sMsg);
            }
            return false;
        }
    return true;
}
//---------------------------------------------------------------------------

int TfProductionInfo::GetArmBySiteFor32Site(int iSite)
{
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestIF_File.iSiteMap[i][j]==iSite)
            {
                return i;
            }
        }
    }
    return -1;
}
//---------------------------------------------------------------------------

void TfProductionInfo::CalculateNowArmSiteBinQty(bool bIsClear)
{
    if(CUSTOMER_CODE==CC_Greatek && IniConfig.bN14_16_EnableIPSC)
    {
        int iSelTrayCT=0, iTarValue=0;
        for(int iCat=0;iCat<TEST_MAX_BIN;iCat++)                                //每個category
        {
            iNowSiteBinTotalQty[iCat]=0;
        }
        for(int iArm=0; iArm<2; iArm++)
        {
            for(int iRow=0; iRow<MAX_SOCKET_ROW; iRow++)
            {
                for(int iCol=0; iCol<MAX_SOCKET_COL; iCol++)
                {
                    for(int iAuto=0; iAuto<9; iAuto++)                          //Auto1~3 Fix1~3
                    {
                        iSelTrayCT=static_cast<int>(ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelTrayCT(iAuto));   //Jimmychiu 20240123 : 防呆-IPSC上傳的資料有負值
                        iTarValue=iSelTrayCT-iLastArmSiteBinQty[iArm][iRow][iCol][iAuto];
                        if(iTarValue<=0)
                        {
                            iTarValue=0;
                        }
                        iNowArmSiteBinQty[iArm][iRow][iCol][iAuto]+=iTarValue;
                        iLastArmSiteBinQty[iArm][iRow][iCol][iAuto]=iSelTrayCT;
                        iNowSiteBinTotalQty[iAuto]+=iNowArmSiteBinQty[iArm][iRow][iCol][iAuto];
                        if(bIsClear)
                        {
                            iLastArmSiteBinQty[iArm][iRow][iCol][iAuto]=0;
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

int TfProductionInfo::CalculateNowTotalICQty()                                  //測試機測出來的總數
{
    int iArm,iRow,iCol,iBin,iTotalLoadQty;
    CalculateNowArmSiteBinQty();

    iTotalLoadQty=0;
    for(iArm=0; iArm<2; iArm++)
    {
        for(iRow=0; iRow<MAX_SOCKET_ROW; iRow++)
        {
            for(iCol=0; iCol<MAX_SOCKET_COL; iCol++)
            {
                for(iBin=0; iBin<TEST_MAX_BIN; iBin++)
                {
                    iTotalLoadQty=iTotalLoadQty+iNowArmSiteBinQty[iArm][iRow][iCol][iBin];
                }
            }
        }
    }
    return iTotalLoadQty;
}
//---------------------------------------------------------------------------

void TfProductionInfo::ClearArmSiteBinQty()
{
    for(int iArm=0; iArm<2; iArm++)
    {
        for(int iRow=0; iRow<MAX_SOCKET_ROW; iRow++)
        {
            for(int iCol=0; iCol<MAX_SOCKET_COL; iCol++)
            {
                for(int iAuto=0; iAuto<9; iAuto++)                              //Auto1~3 Fix1~3
                {
                     iNowArmSiteBinQty[iArm][iRow][iCol][iAuto]=0;
                     iLastArmSiteBinQty[iArm][iRow][iCol][iAuto]=ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelTrayCT(iAuto);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

bool TfProductionInfo::bIsNeedCheckControlBin()                                 //Sam 20201216 : Add 數量監控
{
    bool bRet=false;
    int iTotal=0;
    UpdateControlBinCount(false);

    for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
    {
        for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
        {
            iTotal=NowControlBinCategory.iCountSocketTotal[iRow][iCol]-OldControlBinCategory.iCountSocketTotal[iRow][iCol];
            if(iTotal>iControlBinCheckCount)                                    //其中一個 Site 數量到了需要檢查 Control Bin
                bRet=true;
        }
    }
    return bRet;
}
//---------------------------------------------------------------------------

void TfProductionInfo::UpdateControlBinCount(bool bClear)                       //Sam 20200525 : Control Bin
{
    if(CUSTOMER_CODE==CC_Greatek && IniConfig.bN14_1_EnableOEEFunction)
    {
        NowControlBinCategory.ClearCount();
        for(int iRow=0; iRow<MAX_SOCKET_ROW; iRow++)                            //Sam 20201209 : 修正資料
        {
            for(int iCol=0; iCol<MAX_SOCKET_COL; iCol++)
            {
                for(int iArm=0; iArm<2; iArm++)
                {
                    NowControlBinCategory.iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmData[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                    NowControlBinCategory.iTotalCategory[iTestBinCount]                   +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                    for(int iCat=0; iCat<iTestBinCount; iCat++)                 //每個category
                    {
                        NowControlBinCategory.iCountCategory[iArm][iRow][iCol][iCat]  =ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                        NowControlBinCategory.iTotalCategory[iCat]                   +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                    }
                    NowControlBinCategory.iCountSocketTotal[iRow][iCol]      +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                    NowControlBinCategory.iCountHeadTotal[iArm][iRow][iCol]  +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                    NowControlBinCategory.iTotalSocket                       +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                }
            }
        }

        if(bClear)
        {
            memcpy(&OldControlBinCategory.iCountCategory[0][0][0][0], &NowControlBinCategory.iCountCategory[0][0][0][0], sizeof(NowControlBinCategory));
        }
    }
}
//---------------------------------------------------------------------------

// =============================================================================
//  AI(W906-FW3-PIGSV) 20260829: GetStringBySeparatedValues（golden :2733-2770）。
//  這是 FW3-PICTL 那一批裡剩下的第九支，**從來不是連結問題**：
//  它卡在 `ShowMyMessage`（警報/對話框）需要逐案判斷。本次判定可翻：
//
//   * 安全軸 -- 它**只讀檔**（`LoadFromFile`），不寫檔、不動機台、
//     不送對外命令。同檔的 bEnableIPSC 已經在用 FileExists，同一等級。
//   * `ShowMyMessage` 在本 port **不是彈窗**，是觀測接縫
//     （canary_support.cpp:143-157：記錄 S1、計數、printf、呼叫選用 hook）。
//     這一點是查過才用的——**背景批次跑的程式不可有 modal 彈窗**，
//     否則會卡死 ctest 且 log 完全沒紀錄。它也是四個既有例外之一，
//     本檔自 ba3683e 起就在 ht9045_sm，連得到。
//
//  ⚠ MEASURED BEHAVIOUR NOTE -- `bShow` 在本 port 恆為 false（見 .h 的欄位註解），
//    因為 golden 只在 **FormShow**（.cpp:164/:169）把它設為 true，而 FW 戰役
//    刷意不接線 event handler。所以**寫 Caption 的那條分支目前不可達**，
//    錯誤路徑一律走 ShowMyMessage。這與 golden 的 ctor 初始值一致，是忠實的；
//    但它是**主動選了一條分支**，不是「無作用」，所以寫在這裡而不藏起來。
//
//  ⚠ 本支仍然**沒有呼叫者**（golden 呼叫點在未翻的 OEE 流程）。
// =============================================================================

AnsiString TfProductionInfo::GetStringBySeparatedValues(AnsiString sFullFileName,AnsiString sItemValue,AnsiString sSeparatedValues)
{
    TStringList *slSourceFile;
    AnsiString sDataList="";
    AnsiString sGetString="";

    if(sFullFileName=="" || FileExists(sFullFileName)==false)
    {
        if(bShow)  //Sam 20200225 : OEE 功能防呆保護提示顯示在畫面，不要用 MessageBox。
        {
            pn_ErrorMsg_HALTStatus->Caption = "File Name Error!";
            pn_ErrorMsg_PauseStatus->Caption = "File Name Error!";
        }else
        {
            ShowMyMessage("File Name Error!");
        }
        return sGetString;
    }

    slSourceFile=new TStringList();
    slSourceFile->LoadFromFile(sFullFileName);
    for(int i=0; i<slSourceFile->Count; i++)
    {
        int iPos=-1;
        int iStart=-1;

        sDataList=slSourceFile->Strings[i];
        iPos=sDataList.Pos(sItemValue+sSeparatedValues);
        iStart=sItemValue.Length()+sSeparatedValues.Length()+1;
        if(iPos==1)
        {
            sGetString=sDataList.SubString(iStart,sDataList.Length());
            break;
        }
    }
    delete slSourceFile;
    return sGetString;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//AI(W906-CMYDB-P4-D4) 20260927 (St02-E, Steven D4 = B): golden TfProductionInfo::SaveMessageHistroy, 906_0625_Steven
//  ProductionInfo/ProductionInfo.cpp:1136-1173 (declared ProductionInfo.h:218).  Caller: cMyDB.cpp NewRecordProcess
//  (golden cMyDB.cpp:1547-1550, CC_Greatek && IniConfig.bN14_1_EnableOEEFunction only).  Writes
//  <sProductionInfoFilePath>\<sLoadMO_MO>\<_sOEE_DirectoryName>\<_sOEE_MO>_History.csv: load, append one row, save.
//  Only change: vclcompat TStringList has no Append -> Add (same effect).  _sOEE_DirectoryName stays "" in V906 (see .h).
#include "common.h"          // sProductionInfoFilePath, MyForceDirectories
void TfProductionInfo::SaveMessageHistroy(AnsiString sAlarmCode,AnsiString sAlarmMessage,int iType,int iSkipLevel)
{
    AnsiString sFilePath    ="";
    AnsiString sFileName    ="";
    AnsiString sFullFileName="";
    AnsiString sMsgString   ="";
    AnsiString sSaveTime    ="";
    AnsiString sFileTitle   ="AlarmCode,AlarmDate,AlarmTime,ErrMessage,AlarmType,SkipLevel";
    TStringList *slMsgHistroy=new TStringList();

    //Sam 20190128 : 檢查資料夾並自動產生
    MyForceDirectories(sProductionInfoFilePath, "TfProductionInfo::SaveMessageHistroy_1");
    sFilePath       =sProductionInfoFilePath+"\\"+sLoadMO_MO+"\\"+_sOEE_DirectoryName; //Mylin 20170515 Modify OEE Function
    sFileName       =_sOEE_MO+"_History.csv"; //Handler ID_MO_OEE.csv
    sFullFileName   =sFilePath+"\\"+sFileName;

    MyForceDirectories(sFilePath, "TfProductionInfo::SaveMessageHistroy_2");
    slMsgHistroy->Clear();

    if(FileExists(sFullFileName)==true)
    {
        slMsgHistroy->LoadFromFile(sFullFileName);
    }
    else
    {
        slMsgHistroy->Add(sFileTitle);                                          // golden Append
    }

    sSaveTime.sprintf("%04d/%02d/%02d,%02d:%02d:%02d",SystemYear,SystemMonth,SystemDate,SystemHour,SystemMin,SystemSec);
    sMsgString =sAlarmCode          +","+
                sSaveTime           +","+
                sAlarmMessage       +","+
                AnsiString(iType)   +","+
                AnsiString(iSkipLevel);
    slMsgHistroy->Add(sMsgString);                                              // golden Append
    slMsgHistroy->SaveToFile(sFullFileName);
    delete slMsgHistroy;
}
