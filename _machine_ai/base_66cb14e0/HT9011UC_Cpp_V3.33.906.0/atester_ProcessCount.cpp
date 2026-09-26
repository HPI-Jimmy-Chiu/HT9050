//---------------------------------------------------------------------------
//  atester_ProcessCount.cpp  --  TESTER/INDEX count/yield-alarm bookkeeping (leaf subset)
//
//  Translation wave: W5-ProcessCount-Translate (atester_ProcessCount partial leaf extraction)
//  Translator: AI(W5-ProcessCount-Translate) 20260710
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/atester_ProcessCount.cpp (2161 lines, cp950)
//
//  SCOPE (narrow, per translation-plan hand-off -- do NOT expand).  14 functions,
//  faithfully transcribed, golden line ranges (re-verified against golden before
//  writing this file):
//    RecordPiggyBackStartEnd            golden :61-92
//    ProcessPiggyBackFunction            golden :94-140
//    InitialPiggyBackFunction            golden :142-217
//    ProcessSocketPurgeCount              golden :219-223
//    DoLowYieldAlarm                      golden :258-355
//    DoLowYieldAlarmFirst                 golden :357-394
//    CheckContinuoussFail                 golden :396-1018   (largest -- 623 lines)
//    ProcessContinuoussFailForATC         golden :1020-1050
//    GetMultiplierNum                     golden :1052-1055
//    CheckContinuoussPassBySocket          golden :1057-1129
//    CheckContinuoussPass                 golden :1131-1183
//    ProcessHeadContactCount              golden :1185-1232
//    ProcessSocketCount                    golden :1234-1307
//    ProcessAutoloadcellMeasureCount       golden :1730-1747
//
//  Deliberately OUT OF SCOPE this wave (leave for the brand-new cSocket.cpp
//  module + 7 new forms per the translation-plan hand-off; every OTHER golden
//  export -- ProcessShowTestStatus(:225-256)/ProcessCount/RecordHistroy/
//  ProcessStartTestData/DoLowYieldAlarm's sibling ProcessAutoCleanCount/
//  ProcessQASampling/... -- stays served by the existing atester_shims.h/.cpp
//  offline stubs exactly as today; NOT re-declared/re-defined here (would be a
//  duplicate-definition ODR clash at link -- the integrate agent removes the
//  now-superseded stub bodies for the 14 symbols THIS file defines for real,
//  mirroring the aTester_Front.cpp/aTester_Rear.cpp W64b integrate precedent).
//
//  TRANSLATION RULES (mirrors atester.cpp / aTester_Front.cpp conventions)
//  --------------------------------------------------------------------
//    * Off Borland: __fastcall removed; AnsiString/TQPF_Timer via vclcompat;
//      numeric semantics EXACT (no int/float rewrites; the ContinuousPassBinCount
//      256-element array zeroed only over its first 30 slots in Initial/Process
//      PiggyBackFunction is a FAITHFUL golden quirk -- golden :126/:205 literally
//      loop `i<30`, NOT `i<256` -- preserved verbatim, not "fixed").
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8. ZERO U+FFFD.
//    * bContinuoussPass is GLOBAL (cmydef.h) and used as such in
//      CheckContinuoussPassBySocket (no local shadow, golden :1057-1129 has none);
//      CheckContinuoussPass DOES locally shadow it with `bool bContinuoussPass=
//      false;` (golden :1134) -- BOTH forms preserved exactly as golden, a
//      faithful same-named global/local split across two sibling functions.
//
//  GATED DEPENDENCIES (verified ABSENT from the already-translated substrate;
//  each gated individually with the golden default reproduced, per the
//  established "#if 0 // TODO(...)" convention -- see aTester_Front.cpp for the
//  precedent this follows):
//    1. TastCategory (TEST_CATEGORY, golden cSocket.h:144-174) -- cSocket.cpp is
//       a future ~1300-line module + 7 new forms (explicitly out of THIS wave's
//       scope per the hand-off).  Gates the two golden
//       `if(CosFunction.bBySiteByBinPercentCompare==true){...}` blocks inside
//       CheckContinuoussFail (golden :588-591 and :764-936) -- the by-site/by-bin
//       and by-arm-per-site percent-compare low-yield alarms (WAR07335/36/37).
//       Default: skip (golden CosFunction.bBySiteByBinPercentCompare defaults
//       false anyway, so this is the observed baseline behavior).
//    2. InitialAutoCleanAllTask() (golden AutoClean/AutoClean.h:7, AutoClean.cpp)
//       -- the AutoClean engine module is not translated (separate wave, see
//       ht9045-autoclean-flow skill).  Gates 3 call sites inside
//       CheckContinuoussFail (golden :970/:981/:994).  Default: no-op (the
//       auto-clean trigger itself is skipped; the surrounding low-yield
//       ignore-count-clear / alarm logic that does NOT depend on AutoClean is
//       untouched).
//    3. fCleaning->CheckSmartAutoClean(AnsiString,AnsiString) (golden
//       AutoClean/uCleaning.h:356, TfCleaning : public TForm) -- gated via the
//       PC_CHECKSMARTAUTOCLEAN() macro below (2 call sites in DoLowYieldAlarm,
//       golden :265/:287).  Default: false (both golden call sites have an
//       EMPTY if-body anyway -- see the macro comment for why false is exactly
//       equivalent to "feature not present").
//    4. fMain->slLowYieldAlarm (TStringList*, golden main.h) -- FormsFacade.h's
//       TfMain (already included) does not have this member yet.  Gated as a
//       verbatim block (golden :295-298) inside DoLowYieldAlarm's bOneCycle
//       branch; pure log/bookkeeping side effect, no bearing on the ret code or
//       on the BtnOneCycleClick() call immediately after it (which is NOT gated
//       -- FormsFacade.h's TfMain::BtnOneCycleClick/BtnOneCycle->Down already
//       exist and are used directly).
//    5. fYieldMonitoring->ClearYieldCount() (golden uYieldMonitoring.h) --
//       aHotPlateSubstrate.h's TfYieldMonitoring_2x4_16 (the ALREADY-DECLARED
//       offline mirror of the golden TfYieldMonitoring form) only exposes
//       DoAutoCloseSite() today; ClearYieldCount() has no member yet.  Gated via
//       the PC_YIELDMON_CLEARCOUNT() macro below (6 call sites in
//       CheckContinuoussFail).  Default: no-op.
//    6. fProductionInfo->cDynaThres->GetMultiplierNum()/AddThresholdNum() (golden
//       ProductionInfo.h:547/587-588, cDynamicMultiContinualPassBinBySocket) --
//       gated in GetMultiplierNum() and CheckContinuoussPassBySocket (golden
//       :1054/:1126).  Default: GetMultiplierNum()->1 (golden
//       ProductionInfo.cpp:5955-5961's own default when
//       IniConfig.bN14_24_DynaMultiContinuPassSocket==false, i.e. no dynamic
//       multiplier scaling); AddThresholdNum() no-op (has zero observable effect
//       while GetMultiplierNum() is gated to always return 1).  NOT invented as
//       a new global here -- see this unit's translate report for why (the tree
//       already carries multiple mutually-incompatible per-file
//       `extern <Stub>* fProductionInfo;` declarations with no actual
//       definition anywhere, a pre-existing cross-unit landmine, not introduced
//       by this file).
//  Every gate is individually re-cited at its call site below.
//---------------------------------------------------------------------------
#include "atester_ProcessCount.h"

#include "MachineType.h"            // ePiggyBackTrugger (pbtNone..), CUSTOMER_CODE consts, NN_2Row, MAX_SOCKET_ROW/COL
#include "cmydef.h"                 // global scalar universe (Continuous*Count arrays, bCheckIndex, iOneCycle, ...)
#include "cprod.h"                  // Prod / TestIF / TestIF_File / RunInfo / Temperature
#include "Config.h"                 // IniConfig (HT9045_CONFIG)
#include "CosFunction.h"            // CosFunction (HT9045_COUSTOMER_FUNCTION)
#include "canary_support.h"         // LastSet / ShowErrorMessage / RecordProcess
#include "aHotPlateSubstrate.h"     // TestSocket / FTestSuck / MyDBIProcess / fYieldMonitoring
#include "cSocket.h"                // AI(W906-StaleGates) 20260820: TastCategory (real home landed; gate #1 retired)
#include "csystem.h"                // InitOneCycle
#include "atester_shims.h"          // IsNNMode() / fContact (TfContactShim)
#include "FormsFacade.h"            // fMain (BtnOneCycleClick / BtnOneCycle->Down already present)

//---------------------------------------------------------------------------
//  Gate macros (see file-header "GATED DEPENDENCIES" list above for full
//  rationale).  Each mirrors the golden default so the surrounding logic
//  compiles + behaves identically to a handler with that specific feature
//  disabled/absent, matching the W64B_* macro-gate convention established in
//  aTester_Front.cpp.
//---------------------------------------------------------------------------
// AI(W5-ProcessCount-Translate) 20260710: fCleaning->CheckSmartAutoClean(...) --
// see file-header gate #3.  Both golden call sites (:265/:287) have an EMPTY
// if-body ("//DoSmartAutoClean", no code) -- so whether this predicate is true
// or false has NO effect on ret/control-flow beyond which branch of the
// if/else-if ladder is (harmlessly) skipped.  false is the conservative,
// behaviorally-safe default (identical to "no smart-auto-clean gate active").
#define PC_CHECKSMARTAUTOCLEAN(AlarmCode, ErrPart)  (false)

// AI(W5-ProcessCount-Translate) 20260710: fYieldMonitoring->ClearYieldCount() --
// see file-header gate #5.  6 call sites below.
#define PC_YIELDMON_CLEARCOUNT()   ((void)0)

//---------------------------------------------------------------------------
TQPF_Timer PiggyBackTimer;                                                      //golden :60
void RecordPiggyBackStartEnd(bool bStart)                                       //jou 2011-11-14 start : 記錄piggyback時間
{
    bPiggyBackIndexCheck=false;

    if(IniConfig.bRecordPiggyBackStartEnd==false)
        return;

    AnsiString AStr;

    if(bStart==true)
    {
        if(bRecordPiggyBackStart==true)
        {
            PiggyBackTimer.LatchCycleTimeSec(true);
        }
    }
    else
    {
        if(bRecordPiggyBackEnd==true && iOneCycle==false)
        {
            bRecordPiggyBackEnd=false;
            bRecordPiggyBackStart=false;
            iPiggyBackTime[1]=MyTickCount();                                    //jou test delay time

            AStr.sprintf("PiggyBack Funtion record time : %d Sec", PiggyBackTimer.LatchCycleTimeSec());
            MyDBIProcess("Message", AStr);

            AStr.sprintf("Piggy Back Pass Finish");                             //JerryYang 20220614 松諭要求新增
            MyDBIProcess("Message", AStr);
        }
    }
}
//---------------------------------------------------------------------------
void ProcessPiggyBackFunction()                                                 //Steven 20110725 : 將所有的Piggy Back整合到這裡
{
    if(CosFunction.bIndexCheckCanTurnOff &&                                     //Isaac 20211019 : 可選擇做index check的時機，符合條件則不檢查
       ((IniConfig.iD71IndexCheckOnOffMode==0 && bLotStartEndNeedIndexCheck==false) ||
        (IniConfig.iD71IndexCheckOnOffMode==1 && bIndexJamNeedIndexcheck==false) ||
         IniConfig.iD71IndexCheckOnOffMode==2))
    {                                                                           //不檢查
    }
    else
    {
        if(bCheckIndex==false)
        {
            bRecordPiggyBackStart=true;
        }

        bCheckIndex=true;

        if(fMain->BtnOneCycle->Down==false)
        {
            InitOneCycle("Process Piggy Back Function");
            fMain->BtnOneCycle->Down=true;
            LastSet.SendCT[3]=0;
            LastSet.SendCT[2]=0;
            LastSet.iIndexCount=0;                                              //wei 20141201 Low Yield Auto Clean
            if((USE_AUTO_RETEST==eartInstall &&
                (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||              //kevin 20150615
                CosFunction.bUseARTSortCount)                                   //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
            {
               LastSet.SendCT_ART[3]=0;
               LastSet.SendCT_ART[2]=0;
            }

            for(int i=0; i<30; i++)
            {
                ContinuousPassBinCount[i]=0;
            }

            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    ContinuousPassBinBySocket[i][j]=0;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void InitialPiggyBackFunction()                                                 //Steven 20110725 : 重置Piggy Back的狀態
{
    AnsiString str;
    if(bCheckIndex==true)
    {
        switch(iWhoTriggerPiggyBack)                                            //Steven 20111207 : 誰觸發了Piggy Back
        {
            case pbtNone:
                str.sprintf("PiggyBack Function Start");
                break;
            case pbtContinualPassBin:
                str.sprintf("PiggyBack Function Start - Continual Pass Bin");
                break;
            case pbtContinualPassBinBySocket:
                str.sprintf("PiggyBack Function Start - Continual Pass Bin By Socket");
                break;
            case pbtContinualLoader:
                str.sprintf("PiggyBack Function Start - Continual Loader");
                break;
            case pbtContinualContactCount:
                str.sprintf("PiggyBack Function Start - Continual Contact Count");
                break;
            case pbtHome:
                str.sprintf("PiggyBack Function Start - Home");
                break;
            case pbtOneCycle:
                str.sprintf("PiggyBack Function Start - One Cycle");
                break;
            case pbtCleanout:
                str.sprintf("PiggyBack Function Start - Clean Out");
                break;
            case pbtReset:
                str.sprintf("PiggyBack Function Start - Reset");
                break;
            case pbtOutShuttleLose:
                str.sprintf("PiggyBack Function Start - Out Shuttle Lose IC");
                break;
            case pbtIndexArmVacOffErr:                                          //JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back
                str.sprintf("PiggyBack Function Start - Index Arm Vacuum Off Error");
                break;
            case pbtRTCAlarm:
                str.sprintf("PiggyBack Function Start - RTC Alarm");
                break;
            case pbtOneCycleRemainSht:                                          //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                str.sprintf("PiggyBack Function Start - [I09] one cycle remain shuttle device");
                break;
        }
        iWhoTriggerPiggyBack=pbtNone;
        bRecordPiggyBackEnd=true;
        MyDBIProcess("Message", str);
    }

    bCheckIndex=false;
    LastSet.SendCT[3]=0;
    LastSet.SendCT[2]=0;
    LastSet.iIndexCount=0;                                                      //wei 20141201 Low Yield Auto Clean
    if(USE_AUTO_RETEST==eartInstall &&                                          //kevin 20150615
       (bAutoReTest_ART || IniConfig.bA10_AutoReTest))                          //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    {
        LastSet.SendCT_ART[3]=0;
        LastSet.SendCT_ART[2]=0;
    }

    for(int i=0; i<30; i++)
    {
        ContinuousPassBinCount[i]=0;
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            ContinuousPassBinBySocket[i][j]=0;
        }
    }
}
//---------------------------------------------------------------------------
void ProcessSocketPurgeCount()
{
    if(IniConfig.bD47EnableSocketPurgeFunction)
        LastSet.iD47SocketTestedCount++;
}
//------------------------------------------------------------------------------
int DoLowYieldAlarm(AnsiString AlarmCode, AnsiString ErrPart)                   //Steven 20180627 (wei) : 整合Low Yield Alarm
{
    AnsiString Str;
    bool bOneCycle=false;
    int ret=K_ONECYCLE;

    if(CosFunction.bSmartAutoClean &&
       PC_CHECKSMARTAUTOCLEAN(AlarmCode, ErrPart))                              //Sam 20250916 : Alarm後需要清除資料才能Start //Sam 20230111 : Smart Auto Clean
    {
        //DoSmartAutoClean
    }
    else if(CosFunction.bYieldAlmNeedOneCycle)
    {
        bOneCycle=true;
        if(CosFunction.bContinueFailNeedAlarmDirectly)                          //Steven 20230831 : 當使用bYieldAlmNeedOneCycle, 連續Fail要直接Alarm
        {
            if(AlarmCode=="WAR07301" ||                                         //Socket consecutive failureAlarmCode
               AlarmCode=="WAR07321" ||                                         //Arm1: consecutive failure
               AlarmCode=="WAR07322" ||                                         //Consecutive Pass Error!
               AlarmCode=="WAR07329" ||                                         //Arm2: consecutive failure
               AlarmCode=="WAR07331" ||                                         //Special Bin Socket Consecutive Failure
               AlarmCode=="WAR07332" ||                                         //Arm1: Special Bin Consecutive Failure
               AlarmCode=="WAR07333")                                           //Arm2: Special Bin Consecutive Failure
            {
                bOneCycle=false;
            }
        }
    }

    if(PC_CHECKSMARTAUTOCLEAN(AlarmCode, ErrPart))                              //Sam 20230111 : Smart Auto Clean
    {
        //DoSmartAutoClean
    }
    else if(bOneCycle)                                                          //JerryYang 20161006 Yeild Alarm 時要做完Onecycle才能跳出Alarm視窗
    {
        bNeedOneCycleByYieldAlm=true;
        Str.sprintf("%s,%s", AlarmCode, ErrPart);
        // AI(W5-ProcessCount-Translate) 20260710: fMain->slLowYieldAlarm (TStringList*)
        // -- see file-header gate #4.  Golden verbatim, gated whole (pure log/
        // bookkeeping side effect; the BtnOneCycleClick() call right after this
        // block is NOT gated -- it is unconditional in golden too).
#if 0 // TODO(W6.4b-integrate): ADD `TStringList *slLowYieldAlarm;` (golden main.h) to FormsFacade.h's TfMain, initialized in TfMain's ctor.
        if(fMain->slLowYieldAlarm->Text.AnsiPos(AlarmCode)==0)
        {
            fMain->slLowYieldAlarm->Add(Str);
        }
#endif
        fMain->BtnOneCycleClick(fMain);
    }
    else if(CosFunction.bLowYieldDoOneCycle)                                    //Steven 20170308 (wei) : 良率相關的都做One Cycle
    {
        if(CUSTOMER_CODE==CC_JCET)                                              //JerryYang 20190709 JCET黃剛要求low yield不要強制one cycle
        {
            if(IniConfig.bI05LowYieldForcedOneCycle)
            {
                ret=ShowErrorMessage(AlarmCode, K_ONECYCLE, MMInterface, false, ErrPart);
            }
            else
            {
                ret=ShowErrorMessage(AlarmCode, K_RETRY|K_ONECYCLE, MMInterface, false, ErrPart);
            }
        }
        else
        {
            ret=ShowErrorMessage(AlarmCode, K_ONECYCLE, MMInterface, false, ErrPart);
        }
    }
    else if(CUSTOMER_CODE==CC_Greatek ||
            CUSTOMER_CODE==CC_AMKOR_China ||
            CUSTOMER_CODE==CC_QUALCOMM ||
            IniConfig.bKoreaFunction==true)
    {
        if(CosFunction.bYieldAlmNeedOneCycle)                                   //Steven 20240131 : 修正連續fail做 auto clean 跟 alarm互卡的問題
        {
            if(iOneCycle==0)
                ret=ShowErrorMessage(AlarmCode, K_RETRY, MMInterface, false, ErrPart);
        }
        else
        {
            ret=ShowErrorMessage(AlarmCode, K_RETRY, MMInterface, false, ErrPart);
        }
    }
    else if(CosFunction.bSpecailLowYeild && Prod.bFailAlarmLowYieldSpecial &&
            (bLowYeildAlarmSpecial || bLowYeildAlarmSpecial1stPass))            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        ret=ShowErrorMessage(AlarmCode, 0, MMInterface, false, ErrPart);
    }
    else
    {
        ret=ShowErrorMessage(AlarmCode, K_RETRY|K_ONECYCLE, MMInterface, false, ErrPart);
    }

    if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                      //Sam 20230728 : 發生 LowYield 後重新 Start 清除 Sort Count 和 contact counterKinds
    {
        if(AlarmCode=="WAR0702" || AlarmCode=="WAR0702" || AlarmCode=="WAR0705" || AlarmCode=="WAR07321" || AlarmCode=="WAR07329")
            bInitailQtyByLowYield=true;
    }

    if(ret==K_ONECYCLE)
    {
        fMain->BtnOneCycleClick(fMain);
    }
    return ret;
}
//------------------------------------------------------------------------------
int DoLowYieldAlarmFirst(AnsiString AlarmCode, AnsiString ErrPart)              //Steven 20180627 (wei) : 整合Low Yield Alarm
{
    int ret=K_ONECYCLE;
    if(CosFunction.bLowYieldDoOneCycle)                                         //Steven 20170308 (wei) : 良率相關的都做One Cycle
    {
        if(CUSTOMER_CODE==CC_JCET)                                              //JerryYang 20190709 JCET黃剛要求low yield不要強制one cycle
        {
            if(IniConfig.bI05LowYieldForcedOneCycle)
            {
                ret=ShowErrorMessage(AlarmCode, K_ONECYCLE, MMInterface, false, ErrPart);
            }
            else
            {
                ret=ShowErrorMessage(AlarmCode, K_RETRY|K_ONECYCLE, MMInterface, false, ErrPart);
            }
        }
        else
        {
            ret=ShowErrorMessage(AlarmCode, K_ONECYCLE, MMInterface, false, ErrPart);
        }
    }
    else if(CUSTOMER_CODE==CC_Greatek ||
            CUSTOMER_CODE==CC_AMKOR_China ||
            CUSTOMER_CODE==CC_QUALCOMM)
    {
        ret=ShowErrorMessage(AlarmCode, K_RETRY, MMInterface, false, ErrPart);
    }
    else
    {
        ret=ShowErrorMessage(AlarmCode, K_RETRY|K_ONECYCLE, MMInterface, false, ErrPart);
    }

    if(ret==K_ONECYCLE)
    {
        fMain->BtnOneCycleClick(fMain);
    }
    return ret;
}
//------------------------------------------------------------------------------
void CheckContinuoussFail(int Index)                                            //Steven 20110603 : 整合只叫一次
{
    bool bConseFailureBySocket_AutoClean;
    unsigned int  iConseFailureCountBySocket_AutoClean;
    bool bConseFailureByHead_AutoClean;
    unsigned int  iConseFailureCountByHead_AutoClean;
    if(iRunStartMode==FT)
    {
        bConseFailureBySocket_AutoClean     =TestIF.bAutoClean_ConseFailureBySocket_Normal;
        iConseFailureCountBySocket_AutoClean=TestIF.iAutoClean_ConseFailureCountBySocket_Normal;
        bConseFailureByHead_AutoClean       =TestIF.bAutoClean_ConseFailureByHead_Normal;
        iConseFailureCountByHead_AutoClean  =TestIF.iAutoClean_ConseFailureCountByHead_Normal;
    }
    else
    {
        bConseFailureBySocket_AutoClean     =TestIF.bAutoClean_ConseFailureBySocket_Retest;
        iConseFailureCountBySocket_AutoClean=TestIF.iAutoClean_ConseFailureCountBySocket_Retest;
        bConseFailureByHead_AutoClean       =TestIF.bAutoClean_ConseFailureByHead_Retest;
        iConseFailureCountByHead_AutoClean  =TestIF.iAutoClean_ConseFailureCountByHead_Retest;
    }
    //ChungHung 20131223 add for SCK request end

    bool bSocketConsecutiveFailure=false, bConsecutiveFailure=false, bSocketAllSiteFail=false;                          //kevin 20170825 (Steven) add all site fail
    bool bSocketConsecutiveFailure_AutoClean=false, bConsecutiveFailure_AutoClean=false;                                //ChungHung 20131223 add for SCK request
    bool bDoAutoCleanByBinCount=false;                                          //Steven 20160308 : By Bin count do auto clean
    bool bDoAutoCleanBySiteCount=false;                                         //Steven 20160308 : By Bin pre site count do auto clean
    bool bSpecailBinContinuousFailByArm=false,bSpecialBinContinuousFailBySocket=false;                                  //ChungHung 20140730 add ContinuousFailHaveOneCycle
    AnsiString SocketErrPart="", ErrPart="", AutoCleanErrPart="";
    int iGetBin=0, iBinCount=0, iSite=0, iTray[TEST_MAX_BIN]={0};               //JerryYang 20221215 : 33->TEST_MAX_BIN
    int iAllSiteFailCount[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    int iSiteCount=0;

    // AI(W5-ProcessCount-Translate) 20260710: TastCategory-dependent locals (golden
    // :428-443) -- ONLY consumed inside the gated bBySiteByBinPercentCompare block
    // below (file-header gate #1).  Declared here verbatim (matching golden's
    // top-of-function layout for easy diffing against golden) even though they are
    // presently unused outside that gate.
    double dSiteByBinYield=0.0;                                                 //JerryYang 20170712 (Steven) by site by bin compare percent
    double dSiteByBinYield1[32]={0.0};
    double dArm1ByBinYield=0.0;
    double dArm2ByBinYield=0.0;
    double dSiteByBinYieldMax=0.0;
    double dSiteByBinYieldMin=100.0;
    double dArm1ByBinYieldMax=0.0;
    double dArm1ByBinYieldMin=100.0;
    double dArm2ByBinYieldMax=0.0;
    double dArm2ByBinYieldMin=100.0;
    AnsiString sWhichSiteByBinYieldMax="";
    AnsiString sWhichSiteByBinYieldMin="";
    AnsiString sArm1WhichSiteByBinYieldMax="";
    AnsiString sArm1WhichSiteByBinYieldMin="";
    AnsiString sArm2WhichSiteByBinYieldMax="";
    AnsiString sArm2WhichSiteByBinYieldMin="", sSocketAllSiteErrPart="";

    int Z=0, iTotalSite=0, iSocketFailIc=0;

    if(TestIF.iShuttleMode==1)                                                  //kevin 20170825 (Steven) use arm open/close site
    {
        if(TestIF.iShuttle_Sel==1)
        {
            Z=1;
        }
        else
        {
            Z=0;
        }
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(bTestSiteUse[Z][i][j])
            {
                iSite++;
                if(IniConfig.bA09_ByArmCloseSite &&                             //Steven 20220818 : By Arm Continue fail, auto site off
                   IsNNMode()==0)                                               //Sam 20250401 : 修正 NN 模式計數錯誤
                {
                    iSiteCount+=2;
                }
                else
                {
                    iSiteCount++;
                }
            }

            iAllSiteFailCount[i][j]=0;                                          //kevin 20170825 (Steven) all site fail
            if(TestSocket.Item[i][j]>=TEST_PASS)                                //ChungHung 20140516 fix 記憶體溢位
            {
                iGetBin=TestSocket.Item[i][j]-TEST_PASS;

                if(iGetBin>(iTestBinCount-1))                                   //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1
                    continue;

                if(IniConfig.bEnableAutoCleanFunction &&
                   TestIF.iAutoClean_Function==true &&
                   bIsAutoOneCycle==false)                                      //確保不要重複計算
                {
                    iAutoCleanByBinCount[iGetBin]++;
                    iAutoCleanBySiteCount[i][j][iGetBin]++;
                    if(Prod.iAutoCleanByBin[iGetBin]>0 &&                       //Steven 20160308 : By Bin count do auto clean
                       iAutoCleanByBinCount[iGetBin]>=Prod.iAutoCleanByBin[iGetBin])
                    {
                        bDoAutoCleanByBinCount=true;
                        AutoCleanErrPart+=IndexSuckName[i][j];                  //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                    }

                    if(Prod.iAutoCleanBySite[iGetBin]>0 &&                      //Steven 20160308 : By Bin pre site count do auto clean
                       iAutoCleanBySiteCount[i][j][iGetBin]>=Prod.iAutoCleanBySite[iGetBin])
                    {
                        bDoAutoCleanBySiteCount=true;
                        AutoCleanErrPart+=IndexSuckName[i][j];                  //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                    }
                }

                if(TestSocket.bPass[i][j])                                      //pass
                {
                    ContinuousFailSKTCount[i][j]=0;
                    ContinuousFailARMCount[Index][i][j]=0;
                    ContinuousFailSKTCount_AutoClean[i][j]=0;                   //ChungHung 20131223 add for SCK request
                    ContinuousFailARMCount_AutoClean[Index][i][j]=0;

                    if(Prod.iContsFailSocketAlarmCT>0)                          //Steven 20230529 : Spc Bin Couont改成連續錯誤
                    {
                        SpecialBinContinuousFailSKTCount[i][j]=0;
                        SpecialBinContinuousFailARMCount[Index][i][j]=0;
                    }
                }
                else if(TestSocket.Item[i][j]!=HAS_IC &&                        //ChungHung 20140730 add ContinuousFailHaveOneCycle start
                        TestSocket.Item[i][j]!=HAS_NULL_IC &&
                        TestSocket.bPass[i][j]==false)                          //Kevin 20161102 : 沒IC不Alarm連續fail
                {
                    if(Prod.bConsFail[iGetBin]==true ||
                       CUSTOMER_CODE==CC_Greatek ||                             //Sam 20200224 : 正賢要求只要是 Fail 就計數不管 Prod.bConsFail 設定
                       CUSTOMER_CODE==CC_SIGURD_HUKOU)                          //Sam 20210312 : 矽格湖口卓邵要求
                    {
                        if(fContact->fShow==false)
                        {
                            if(CUSTOMER_CODE==CC_KYEC_CHEN &&
                               Prod.bContsFailIgnore)                           //wei 20160115 銅鑼前幾顆不計算ContsFail
                            {
                                if(Prod.bContsFailBySocket==false &&
                                   Prod.bContsFailByHead==false)
                                    iContsFailIgnoreCount=0;
                                bContsFailIgnoreStart=true;
                                iContsFailIgnoreCount++;
                                if(iContsFailIgnoreCount>=Prod.iContsFailIgnore)
                                    bContsFailIgnoreStart=false;
                            }
                            else
                            {
                                bContsFailIgnoreStart=false;
                            }

                            if(bContsFailIgnoreStart==false)
                            {
                                ContinuousFailSKTCount[i][j]++;
                                ContinuousFailARMCount[Index][i][j]++;
                                ContinuousFailSKTCount_AutoClean[i][j]++;       //ChungHung 20131223 add for SCK request
                                ContinuousFailARMCount_AutoClean[Index][i][j]++;
                            }
                        }
                    }

                    if(Prod.bSpecialBinBySocket[iGetBin]==1)                    //ChungHung 20140730 add ContinuousFailHaveOneCycle start
                        SpecialBinContinuousFailSKTCount[i][j]++;

                    if(Prod.bSpecialBinByArm[iGetBin]==1)
                        SpecialBinContinuousFailARMCount[Index][i][j]++;

                    if(CosFunction.bAllSiteSameFailBinShowAlarm==true &&        //JerryYang 20170221 (wei) 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm,改成可複選bin
                       IniConfig.bI34AllSiteAreSameFailBinShowAlarm==true)
                    {
                        for(int k=0; k<iTestBinCount; k++)                      //JerryYang 20170221 (wei) 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm,改成可複選bin
                        {
                            iTray[k]=Prod.iT6CatData[k];
                            if(iTray[k]>=0 &&                                   //Steven 20230929 : 加入保護
                               Prod.iIsFailT6[iTray[k]]==1 &&                   //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
                               bSpecificBin[k]==true)
                            {
                                if(iGetBin==k)
                                {
                                    iBinCount++;
                                    if(bTestSiteUse[Z][i][j])                   //kevin 20170831 開關site
                                    {
                                        sSocketAllSiteErrPart+=IndexSuckName[i][j];                                     //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                                    }
                                }
                            }
                        }
                    }
                    iAllSiteFailCount[i][j]=1;                                  //kevin 20170825 (Steven) all site fail
                }
            }
        }
    }

    // AI(W906-StaleGates) 20260820: gate #1 RETIRED -- its premise ("TastCategory
    // has no translated home yet", W5-ProcessCount 20260710) died when
    // cSocket.cpp landed: nm on libht9045_sm.a shows cSocket.cpp.obj OWNS the
    // real `TEST_CATEGORY TastCategory` (UpdataCount body cSocket.cpp:1258)
    // and SCK_ART_Remainder.cpp's old stub was deleted at integration
    // (SCK_ART_Remainder.cpp:361 records the removal). Golden :588-591 live.
    if(CosFunction.bBySiteByBinPercentCompare==true)                            //JerryYang 20170712 (Steven) by site by bin compare percent
    {
        TastCategory.UpdataCount(true);                                         //Steven 20250514 : 統一計算數量
    }

    if(CosFunction.bAllSiteSameFailBinShowAlarm==true &&                        //kevin 20170825 (Steven) add all site fail
       IniConfig.bI34AllSiteAreSameFailBinShowAlarm==true)                      //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    {
        if(iBinCount>0 && iBinCount>=iSite)
        {
            DoLowYieldAlarmFirst("WAR07334", asErrPart);                        //Steven 20180627 (wei) : 整合Low Yield Alarm
        }
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(Prod.bAllSiteFail && bTestSiteUse[Z][i][j])                      //kevin 20170825 (Steven) all site fail  //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開
            {
                iTotalSite++;
                if(iAllSiteFailCount[i][j]>0)                                   //kevin 20170825 all site fail[i][j]>0)
                {
                    iSocketFailIc++;
                    sSocketAllSiteErrPart+=IndexSuckName[i][j];                 //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                }
            }

            if(Prod.bContsFailBySocket && Prod.iContsFailSocketAlarmCT!=0)
            {
                if(ContinuousFailSKTCount[i][j]>=Prod.iContsFailSocketAlarmCT)
                {
                    ContinuousFailSKTCount[i][j]=0;

                    if(bCanAutoCloseSite &&
                       TestIF_File.bLowYieldAutoSiteOffByContiFail)             //Steven 20200420 : Continue fail, auto site off
                    {
                        if(IniConfig.bA09_ByArmCloseSite &&                     //Steven 20220818 : By Arm Continue fail, auto site off
                           IsNNMode()==0)                                       //Sam 20250401 : 修正 NN 模式計數錯誤
                        {
                            iSiteCount-=2;
                        }
                        else
                        {
                            iSiteCount--;
                        }

                        if(iSiteCount<TestIF_File.iAlarmWhenSiteOnCountLess)
                        {
                            bSocketConsecutiveFailure=true;
                        }
                        else
                        {
                            bLowYieldCloseSite[0][i][j]=true;
                            bLowYieldCloseSite[1][i][j]=true;
                        }
                    }
                    else
                    {
                        bSocketConsecutiveFailure=true;
                    }
                    SocketErrPart+=IndexSuckName[i][j];                         //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                    asErrTemp+=IndexSuckName[i][j];                             //JerryYang 20160407 存error位置
                }
            }
            else
            {
                ContinuousFailSKTCount[i][j]=0;
            }

            if(Prod.bContsFailByHead && Prod.iContsFailHeadAlarmCT!=0)
            {
                if(ContinuousFailARMCount[Index][i][j]>=Prod.iContsFailHeadAlarmCT)
                {
                    ContinuousFailARMCount[Index][i][j]=0;
                    if(bCanAutoCloseSite &&
                       TestIF_File.bLowYieldAutoSiteOffArmContiFail &&
                       (IniConfig.bA09_ByArmCloseSite ||                        //Steven 20220818 : By Arm Continue fail, auto site off
                        IsNNMode()>0))                                          //Sam 20250401 : 修正 NN 模式計數錯誤
                    {
                        if(IsNNMode()>0)                                        //Sam 20250401 : 修正 NN 模式計數錯誤
                        {
                            if(bLowYieldCloseSite[Index][i][j]==false)
                                iSiteCount--;
                        }
                        else
                        {
                            iSiteCount--;
                        }

                        if(iSiteCount<TestIF_File.iAlarmWhenSiteOnCountLess)
                        {
                            bConsecutiveFailure=true;
                        }
                        else
                        {
                            bLowYieldCloseSite[Index][i][j]=true;
                        }
                    }
                    else
                    {
                        bConsecutiveFailure=true;
                    }
                    ErrPart+=IndexSuckName[i][j];                               //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                }
            }
            else
            {
                ContinuousFailARMCount[Index][i][j]=0;
            }

            if(bConseFailureBySocket_AutoClean &&                               //ChungHung 20131223 add for SCK request start
               iConseFailureCountBySocket_AutoClean!=0)
            {
                if(ContinuousFailSKTCount_AutoClean[i][j]>=iConseFailureCountBySocket_AutoClean)
                {
                    ContinuousFailSKTCount_AutoClean[i][j]=0;
                    bSocketConsecutiveFailure_AutoClean=true;
                    SocketErrPart+=IndexSuckName[i][j];                         //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                }
            }
            else
            {
                ContinuousFailSKTCount_AutoClean[i][j]=0;
            }

            if(bConseFailureByHead_AutoClean && iConseFailureCountByHead_AutoClean!=0)
            {
                if(ContinuousFailARMCount_AutoClean[Index][i][j]>=iConseFailureCountByHead_AutoClean)
                {
                    ContinuousFailARMCount_AutoClean[Index][i][j]=0;
                    bConsecutiveFailure_AutoClean=true;
                    ErrPart+=IndexSuckName[i][j];                               //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                }
            }
            else
            {
                ContinuousFailARMCount_AutoClean[Index][i][j]=0;
            }

            if(TestSocket.Item[i][j]>=TEST_PASS)                                //ChungHung 20140730 add ContinuousFailHaveOneCycle start
            {
                iGetBin=TestSocket.Item[i][j]-TEST_PASS;
                if(iGetBin>(iTestBinCount-1))
                    continue;

                if(Prod.bSpecialBinBySocket[iGetBin] &&
                   SpecialBinContinuousFailSKTCount[i][j]>=Prod.iSpecialBinCountBySocket[iGetBin])
                {
                    SpecialBinContinuousFailSKTCount[i][j]=0;
                    bSpecialBinContinuousFailBySocket=true;
                    ErrPart+=IndexSuckName[i][j];                               //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                }

                if(Prod.bSpecialBinByArm[iGetBin] &&
                   SpecialBinContinuousFailARMCount[Index][i][j]>=Prod.iSpecialBinCountByArm[iGetBin])
                {
                    SpecialBinContinuousFailARMCount[Index][i][j]=0;
                    bSpecailBinContinuousFailByArm=true;
                    ErrPart+=IndexSuckName[i][j];                               //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                    if(CUSTOMER_CODE==CC_SJ_Semiconductor)                      //RogerYang 20260123 : add for Alarm Message
                    {
                        asErrPart.sprintf("%s : BIN %d over limit count %d ", ErrPart, iGetBin, Prod.iSpecialBinCountByArm[iGetBin]);
                    }
                }
            }
        }
    }

    if(Prod.bAllSiteFail && (iSocketFailIc>=iTotalSite))                        //Ifor 20190611 all site fail Alarm By Ft/Rt Setting
    {
        iallSitCount++;
        if(iallSitCount>=Prod.iAllSiteFailCount)                                //kevin 20180720 (wei) add all site fail count      //Steven 20230118 : All site fail RT
            bSocketAllSiteFail=true;                                            //kevin 20170825 add
    }

    // AI(W5-ProcessCount-Translate) 20260710: gate #1 (TastCategory / cSocket.cpp,
    // golden :764-936, the "Site by Bin" + "Site by Arm" percent-compare low-yield
    // alarms WAR07335/07336/07337) -- see file header.  ENTIRE golden for(iCat...)
    // loop below is dormant offline (matches CosFunction.bBySiteByBinPercentCompare's
    // own default==false baseline).
#if 0 // AI(W906-StaleGates) 20260820: PREMISE UPDATED, GATE KEPT. The old
      // premise ("TastCategory has no translated home yet") is stale --
      // cSocket.cpp owns the real definition now. This block STAYS gated on a
      // different, still-true ground: it is the WAR07335/07336/07337 low-yield
      // ALARM ESCALATION loop (golden :764-936), the same safety class as
      // fTemperFrom GATE (T1) -- a real alarm channel starting to fire is an
      // operational behaviour change reserved for explicit sign-off, not a
      // missing-dependency question.
    if(CosFunction.bBySiteByBinPercentCompare==true)                            //JerryYang 20170712 (Steven) by site by bin compare percent
    {
        for(int iCat=0; iCat<iTestBinCount; iCat++)                             //Site by Bin
        {
            dSiteByBinYieldMax=0.0;
            dSiteByBinYieldMin=100.0;
            dArm1ByBinYieldMax=0.0;
            dArm1ByBinYieldMin=100.0;
            dArm2ByBinYieldMax=0.0;
            dArm2ByBinYieldMin=100.0;
            if(Prod.bSpecBinBySiteCompareEnable[iCat]==true &&
               Prod.dSpecBinBySiteComparePercent[iCat]>0 &&
               RunInfo.iUnloadCount-iYeildCT[5]>Prod.iSpecBinBySiteCompareIgnore[iCat])
            {
                if(CUSTOMER_CODE==CC_JCET)
                {
                    for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
                    {
                        for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                        {
                            if((LastSet.bUseTestSocket[0][iRow][iCol] ||        //jou 20171023 (wei) : 修正關site也會by bin site gap alarm
                                LastSet.bUseTestSocket[1][iRow][iCol]) &&
                               TastCategory.iCountSocketTotal[iRow][iCol]>0)
                            {
                                dSiteByBinYield=ChangeToFloat((double)(TastCategory.iCountCategory[0][iRow][iCol][iCat]+TastCategory.iCountCategory[1][iRow][iCol][iCat]), (double)TastCategory.iCountSocketTotal[iRow][iCol]);  //Steven 20250820 : 針對除以0加上保護
                            }
                            else
                            {
                                dSiteByBinYield=-1.0;
                                if(LastSet.bUseTestSocket[0][iRow][iCol]==false)                                        //jou 20171225 (Steven) : 修正被關掉的site再開site也會by bin site gap alarm
                                    TastCategory.iCountCategory[0][iRow][iCol][iCat]=0;
                                if(LastSet.bUseTestSocket[1][iRow][iCol]==false)
                                    TastCategory.iCountCategory[1][iRow][iCol][iCat]=0;
                            }

                            if(dSiteByBinYieldMax<dSiteByBinYield)
                            {
                                dSiteByBinYieldMax=dSiteByBinYield;
                                sWhichSiteByBinYieldMax=IndexSuckName[iRow][iCol];                                      //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                            }

                            if(dSiteByBinYieldMin>dSiteByBinYield && dSiteByBinYield!=-1.0)
                            {
                                dSiteByBinYieldMin=dSiteByBinYield;
                                sWhichSiteByBinYieldMin=IndexSuckName[iRow][iCol];                                      //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                            }
                        }
                    }

                    if((dSiteByBinYieldMax-dSiteByBinYieldMin)>Prod.dSpecBinBySiteComparePercent[iCat])
                    {
                        ErrPart=sWhichSiteByBinYieldMax+sWhichSiteByBinYieldMin;
                        DoLowYieldAlarm("WAR07335", ErrPart);                   //Steven 20180627 (wei) : 整合Low Yield Alarm
                        iYeildCT[5]=RunInfo.iUnloadCount;
                        PC_YIELDMON_CLEARCOUNT();
                    }
                }
                else
                {
                    for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
                    {
                        for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                        {
                            if((LastSet.bUseTestSocket[0][iRow][iCol] ||        //jou 20171023 (wei) : 修正關site也會by bin site gap alarm
                                LastSet.bUseTestSocket[1][iRow][iCol]) &&
                                TastCategory.iCountSocketTotal[iRow][iCol]>0)
                            {
                                dSiteByBinYield1[iRow*8+iCol]=ChangeToFloat((double)(TastCategory.iCountCategory[0][iRow][iCol][iCat]+TastCategory.iCountCategory[1][iRow][iCol][iCat]), (double)TastCategory.iCountSocketTotal[iRow][iCol]);
                            }
                            else
                            {
                                dSiteByBinYield1[iRow*8+iCol]=0.0;
                            }

                            if(dSiteByBinYieldMin>dSiteByBinYield1[iRow*8+iCol])
                            {
                                dSiteByBinYieldMin=dSiteByBinYield1[iRow*8+iCol];
                            }
                        }
                    }

                    for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
                    {
                        for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                        {
                            if((dSiteByBinYield1[iRow*8+iCol]-dSiteByBinYieldMin)>Prod.dSpecBinBySiteComparePercent[iCat] && bLowYeildAlarm==false)
                            {
                                bByBinPercentCompareAlarm=true;
                                ErrPart+=IndexSuckName[iRow][iCol];             //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                            }
                        }
                    }

                    if(bByBinPercentCompareAlarm && bLowYeildAlarm==false)
                    {
                        DoLowYieldAlarm("WAR07335", ErrPart);                   //Steven 20180627 (wei) : 整合Low Yield Alarm
                        iYeildCT[5]=RunInfo.iUnloadCount;
                        PC_YIELDMON_CLEARCOUNT();
                        bByBinPercentCompareAlarm=false;
                    }
                }
            }
            //Site by Arm
            if(Prod.bSpecBinByArmPerSiteCompareEnable[iCat]==true &&
               Prod.dSpecBinByArmPerSiteComparePercent[iCat]>0 &&
               RunInfo.iUnloadCount-iYeildCT[6]>Prod.iSpecBinByArmPerSiteCompareIgnore[iCat])
            {
                ErrPart="";
                for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
                {
                    for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                    {
                        if(TastCategory.iCountCategory[0][iRow][iCol][iCat]>0 && TastCategory.iCountHeadTotal[0][iRow][iCol]>0)
                            dArm1ByBinYield=ChangeToFloat((double)TastCategory.iCountCategory[0][iRow][iCol][iCat], (double)TastCategory.iCountHeadTotal[0][iRow][iCol]);
                        else
                            dArm1ByBinYield=0.0;
                        if(dArm1ByBinYieldMax<dArm1ByBinYield)
                        {
                            dArm1ByBinYieldMax=dArm1ByBinYield;
                            sArm1WhichSiteByBinYieldMax=IndexSuckName[iRow][iCol];                                      //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                        }

                        if(dArm1ByBinYieldMin>dArm1ByBinYield &&
                          (LastSet.bUseTestSocket[0][iRow][iCol] ||
                           LastSet.bUseTestSocket[1][iRow][iCol]))              //JerryYang 20250120 : modify
                        {
                            dArm1ByBinYieldMin=dArm1ByBinYield;
                            sArm1WhichSiteByBinYieldMin=IndexSuckName[iRow][iCol];                                      //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                        }

                        if(TastCategory.iCountCategory[0][iRow][iCol][iCat]>0 && TastCategory.iCountHeadTotal[0][iRow][iCol]>0)
                            dArm2ByBinYield=ChangeToFloat((double)TastCategory.iCountCategory[0][iRow][iCol][iCat], (double)TastCategory.iCountHeadTotal[0][iRow][iCol]);
                        else
                            dArm2ByBinYield=0.0;
                        if(dArm2ByBinYieldMax<dArm2ByBinYield)
                        {
                            dArm2ByBinYieldMax=dArm2ByBinYield;
                            sArm2WhichSiteByBinYieldMax=IndexSuckName[iRow][iCol];
                        }

                        if(dArm2ByBinYieldMin>dArm2ByBinYield)
                        {
                            dArm2ByBinYieldMin=dArm2ByBinYield;
                            sArm2WhichSiteByBinYieldMin=IndexSuckName[iRow][iCol];
                        }
                    }
                }

                if(Prod.bSpecBinByArmPerSiteCompareEnable[iCat]==true &&
                   Prod.dSpecBinByArmPerSiteComparePercent[iCat]>0 &&
                   RunInfo.iUnloadCount-iYeildCT[6]>Prod.iSpecBinByArmPerSiteCompareIgnore[iCat] &&
                   (dArm1ByBinYieldMax-dArm1ByBinYieldMin) > Prod.dSpecBinByArmPerSiteComparePercent[iCat])
                {
                    ErrPart=sArm1WhichSiteByBinYieldMax+sArm1WhichSiteByBinYieldMin;
                    DoLowYieldAlarm("WAR07336", ErrPart);                       //Steven 20180627 (wei) : 整合Low Yield Alarm
                    iYeildCT[6]=RunInfo.iUnloadCount;
                    PC_YIELDMON_CLEARCOUNT();
                }

                if(Prod.bSpecBinByArmPerSiteCompareEnable[iCat]==true &&
                    Prod.dSpecBinByArmPerSiteComparePercent[iCat]>0 &&
                    TastCategory.iTotalSocket>Prod.iSpecBinByArmPerSiteCompareIgnore[iCat] &&
                    RunInfo.iUnloadCount-iYeildCT[6]>Prod.iSpecBinBySiteCompareIgnore[iCat] &&
                  (dArm2ByBinYieldMax-dArm2ByBinYieldMin)>Prod.dSpecBinByArmPerSiteComparePercent[iCat])
                {
                    ErrPart=sArm2WhichSiteByBinYieldMax+sArm2WhichSiteByBinYieldMin;
                    DoLowYieldAlarm("WAR07337", ErrPart);                       //Steven 20180627 (wei) : 整合Low Yield Alarm
                    iYeildCT[6]=RunInfo.iUnloadCount;
                    PC_YIELDMON_CLEARCOUNT();
                }
            }
        }
    }
#endif

    if(bSocketAllSiteFail)                                                      //kevin 20170825 (Steven) all site fail
    {
        bSocketAllSiteFail=false;
        iallSitCount=0;                                                         //kevin 20180720 (wei) all site fail count
        DoLowYieldAlarmFirst("WAR07334", asErrPart);                            //Steven 20180627 (wei) : 整合Low Yield Alarm
    }

    if(bSpecialBinContinuousFailBySocket)                                       //ChungHung 20140730 add ContinuousFailHaveOneCycle start
    {
        PC_YIELDMON_CLEARCOUNT();                                               //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
        bSpecialBinContinuousFailBySocket=false;
        DoLowYieldAlarm("WAR07331", ErrPart);                                   //Steven 20180627 (wei) : 整合Low Yield Alarm
    }

    if(bSpecailBinContinuousFailByArm)
    {
        PC_YIELDMON_CLEARCOUNT();                                               //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
        bSpecailBinContinuousFailByArm=false;
        if(Index==0)
            DoLowYieldAlarm("WAR07332", asErrPart);                             //Steven 20180627 (wei) : 整合Low Yield Alarm
        else
            DoLowYieldAlarm("WAR07333", asErrPart);                             //Steven 20180627 (wei) : 整合Low Yield Alarm
    }

    AnsiString aLowYield;
    if(bSocketConsecutiveFailure_AutoClean)                                     //ChungHung 20131223 add for SCK request start
    {
        if(IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function==true &&
           (TestIF.iAutoClean_Mode & M_SOCKET_ALARM) && bConseFailureBySocket_AutoClean)
        {
            aLowYield = "AutoClean : Consecutive Failure by Socket : " + SocketErrPart + " Contact Count : " + iAutoClean_IndexContactCount;
            RecordProcess(aLowYield);
            // AI(W5-ProcessCount-Translate) 20260710: gate #2 (InitialAutoCleanAllTask,
            // AutoClean module not translated) -- see file header.
#if 0 // TODO(AutoClean-module): InitialAutoCleanAllTask (golden AutoClean/AutoClean.h:7) has no translated home yet.
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
#endif
        }
    }

    if(bConsecutiveFailure_AutoClean)
    {
        if(IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function==true &&
           (TestIF.iAutoClean_Mode & M_SOCKET_ALARM) && bConseFailureByHead_AutoClean)
        {
            aLowYield = "AutoClean : Consecutive Failure by Head : " + SocketErrPart + " Contact Count : " + iAutoClean_IndexContactCount;
            RecordProcess(aLowYield);
#if 0 // TODO(AutoClean-module): InitialAutoCleanAllTask (golden AutoClean/AutoClean.h:7) has no translated home yet.
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
#endif
        }
    }

    if(bDoAutoCleanByBinCount || bDoAutoCleanBySiteCount)                       //Steven 20160308 : By Bin count do auto clean
    {
        if(IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function==true)
        {
            if(bDoAutoCleanByBinCount)
                aLowYield = "AutoClean by Bin count : " + AutoCleanErrPart;
            else
                aLowYield = "AutoClean by site count: " + AutoCleanErrPart;
            RecordProcess(aLowYield);
#if 0 // TODO(AutoClean-module): InitialAutoCleanAllTask (golden AutoClean/AutoClean.h:7) has no translated home yet.
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
#endif
        }
    }

    if(bSocketConsecutiveFailure)                                               //ChungHung 20131223 add for SCK request end
    {
        DoLowYieldAlarm("WAR07301", SocketErrPart);                             //Steven 20180627 (wei) : 整合Low Yield Alarm
        PC_YIELDMON_CLEARCOUNT();                                               //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
    }

    if(bUseTwoArm32Site==true)
    {
        if(bSocketConsecutiveFailure)
            bConsecutiveFailure=false;
    }

    if(bConsecutiveFailure)
    {
        PC_YIELDMON_CLEARCOUNT();                                               //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
        if(Index==0)
            DoLowYieldAlarm("WAR07321", ErrPart);                               //Steven 20180627 (wei) : 整合Low Yield Alarm
        else
            DoLowYieldAlarm("WAR07329", ErrPart);                               //Steven 20180627 (wei) : 整合Low Yield Alarm
    }
}
//------------------------------------------------------------------------------
void ProcessContinuoussFailForATC(int Index)                                    //Steven 20151123 : Continue Fail Temp Offset for ATC
{
    int iGetBin=0;

    if(ATC_SYSTEM!=eATCHonPrecType || LastSet.iTemperature!=Tempture_Hot)
        return;

    for(int j=0; j<2; j++)                                                      //Steven 20151209 : Modify for ATC 7.0
    {
        if(TestSocket.Item[0][j]>=TEST_PASS)
        {
            iGetBin=TestSocket.Item[0][j]-TEST_PASS;
            if(iGetBin>(iTestBinCount-1))
            {
                Temperature.iATCCurrentFailCount[j]++;
                Temperature.iATCCurrentFailCount[j+2]++;
            }

            if(TestSocket.bPass[0][j])                                          //pass
            {
                Temperature.iATCCurrentFailCount[j]=0;
                Temperature.iATCCurrentFailCount[j+2]=0;
            }
            else
            {
                Temperature.iATCCurrentFailCount[j]++;
                Temperature.iATCCurrentFailCount[j+2]++;
            }
        }
    }
}
//---------------------------------------------------------------------------
int GetMultiplierNum()                                                          //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
{
    // AI(W5-ProcessCount-Translate) 20260710: gate #6 (fProductionInfo, golden
    // ProductionInfo.h:547/587) -- see file header.  Golden default when
    // IniConfig.bN14_24_DynaMultiContinuPassSocket==false (ProductionInfo.cpp:
    // 5955-5961) is 1 (no multiplier scaling); reproduced verbatim as the gate
    // default (this offline build never sets that INI flag, so 1 is exact).
#if 0 // TODO(W6.4b-integrate): fProductionInfo->cDynaThres->GetMultiplierNum() -- see file-header gate #6 for the exact shim + pre-existing cross-unit fProductionInfo naming conflict to resolve.
    return fProductionInfo->cDynaThres->GetMultiplierNum();
#else
    return 1;
#endif
}
//---------------------------------------------------------------------------
void CheckContinuoussPassBySocket(int Index)                                    //Steven 20110919
{
    unsigned int iGetBin=0;
    AnsiString ErrPart="";

    if(LastSet.iRunStartMode==rsmAutoSiteMap)                                   //Steven 20110506 : Auto Site Mapping時不啟動
    {
        return;
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]>=TEST_PASS)                                //ChungHung 20140516 fix 記憶體溢位
            {
                iGetBin=TestSocket.Item[i][j]-TEST_PASS;
                if(int(iGetBin)>(iTestBinCount-1))                              //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1
                    continue;

                if(Prod.bContinuousPassBySocket && Prod.iContinuousPassBinCountBySocket!=0 && TestSocket.bPass[i][j])   //pass
                {
                    ContinuousPassBinBySocket[i][j]++;
                    if(ContinuousPassBinBySocket[i][j]>=(Prod.iContinuousPassBinCountBySocket*GetMultiplierNum()))      //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
                    {
                        if(CosFunction.bPiggyBackShowMainForm && Prod.iCountAlarmAction==0)                             //kevin 20131101 強致 Enable Yield 裡面piggyback 功能    //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
                        {
                            bContinuoussPass=true;                              //秀警告不停機
                        }
                        else if(CosFunction.bPiggyBackShowMainForm && Prod.iCountAlarmAction==1)                        //kevin 20131101 強致 Enable Yield 裡面piggyback 功能   //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
                        {                                                       //index check
                            iWhoTriggerPiggyBack=pbtContinualPassBinBySocket;   //Steven 20111207 : 誰觸發了Piggy Back
                            ProcessPiggyBackFunction();                         //Steven 20110725 : 整合成function
                            bContinuoussPass=false;                             //  kevin 20131009改為全域  //Steven 20140923 : 移到上面
                        }
                        else if(Prod.iCountAlarmAction==0)
                        {
                            ContinuousPassBinBySocket[i][j]=0;
                            bContinuoussPass=true;
                            ErrPart+=IndexSuckName[i][j];                       //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                        }
                        else
                        {
                            bContinuoussPass=false;                             //  kevin 20131009改為全域
                            ContinuousPassBinBySocket[i][j]=0;
                            iWhoTriggerPiggyBack=pbtContinualPassBinBySocket;   //Steven 20111207 : 誰觸發了Piggy Back
                            ProcessPiggyBackFunction();                         //Steven 20110725 : 整合成function
                        }
                    }
                }
                else
                {
                    ContinuousPassBinBySocket[i][j]=0;
                }
            }
        }
    }

    if(bContinuoussPass)
    {
        if(IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function==true && (TestIF.iAutoClean_Mode & M_SOCKET_ALARM))
        {
#if 0 // TODO(AutoClean-module): InitialAutoCleanAllTask (golden AutoClean/AutoClean.h:7) has no translated home yet.
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
#endif
        }

        if(CosFunction.bPiggyBackShowMainForm==false)                           //kevin 20131009 高雄ASE不能停機只能畫面秀警告
        {
            DoLowYieldAlarm("WAR07322", ErrPart);                               //Steven 20180627 (wei) : 整合Low Yield Alarm
            bContinuoussPass=false;
            // AI(W5-ProcessCount-Translate) 20260710: gate #6 (fProductionInfo,
            // golden ProductionInfo.h:588) -- see file header + GetMultiplierNum()
            // above.  No-op offline: GetMultiplierNum() is gated to always return
            // 1, so skipping this threshold-bump has zero observable effect.
#if 0 // TODO(W6.4b-integrate): fProductionInfo->cDynaThres->AddThresholdNum() -- see file-header gate #6.
            fProductionInfo->cDynaThres->AddThresholdNum();                     //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
#endif
        }
    }
}
//---------------------------------------------------------------------------
void CheckContinuoussPass(int Index)
{
    unsigned int iGetBin=0;
    bool bContinuoussPass=false;
    AnsiString ErrPart="";

    if(LastSet.iRunStartMode==rsmAutoSiteMap)                                   //Steven 20110506 : Auto Site Mapping時不啟動
    {
        return;
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]>=TEST_PASS)                                //ChungHung 20140516 fix 記憶體溢位
            {
                iGetBin=TestSocket.Item[i][j]-TEST_PASS;
                if((int)iGetBin>(iTestBinCount-1))                              //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1
                    continue;

                if(Prod.bContinuousPass && ((Prod.iContinuousPassBin)==iGetBin) &&
                   Prod.iContinuousPassBinCount!=0 && TestSocket.bPass[i][j])   //pass    //Steven 20140922 : 因為沒有Bin 0,所以不用-1
                {
                    ContinuousPassBinCount[iGetBin]++;
                    if(ContinuousPassBinCount[iGetBin]>=Prod.iContinuousPassBinCount)
                    {
                        ContinuousPassBinCount[iGetBin]=0;
                        if(Prod.iCountAlarmAction==0)
                        {
                            bContinuoussPass=true;
                            ErrPart+=IndexSuckName[i][j];                       //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                        }
                        else
                        {
                            iWhoTriggerPiggyBack=pbtContinualPassBin;           //Steven 20111207 : 誰觸發了Piggy Back
                            ProcessPiggyBackFunction();                         //Steven 20110725 : 整合成function
                        }
                    }
                }
                else
                {
                    ContinuousPassBinCount[Prod.iContinuousPassBin]=0;          //Steven 20210812 : iGetBin --> Prod.iContinuousPassBin
                }
            }
        }
    }

    if(bContinuoussPass)
    {
        DoLowYieldAlarm("WAR07322", ErrPart);                                   //Steven 20180627 (wei) : 整合Low Yield Alarm
    }
}
//---------------------------------------------------------------------------
void ProcessHeadContactCount(int Index)                                         //Ifor 20160516 京元要求銦片 Life Time 功能
{
    bool bHaveUseArm=true;
    int iPos;

    if(CosFunction.bUseHeadContactCount)
    {
        if(TestIF_File.iShuttleMode==1)
        {
            if(TestIF_File.iShuttle_Sel==0 && Index==1)
            {
                bHaveUseArm=false;
            }
            else if(TestIF_File.iShuttle_Sel==1 && Index==0)
            {
                bHaveUseArm=false;
            }
        }
        else
        {
            bHaveUseArm=true;
        }

        //Ifor 20160513 修改Arm2 Life Time 異常問題
        for(int iA=0; iA<3 ; iA++)                                              //wei 20160509
        {
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[Index][i][j])
                    {
                        iPos=j*2+i;
                        if(bHaveUseArm==true)
                        {
                            IniConfig.HeadContactCount[iA][Index][iPos]++;
                            IniConfig.HeadContactCountHistory[iA][Index][iPos]++;

                            if(IniConfig.HeadContactCountHistory[iA][Index][iPos]>=1000000000)
                                IniConfig.HeadContactCountHistory[iA][Index][iPos]=0;
                        }
                        iSLT_HeadContactCount[Index][iPos]=IniConfig.HeadContactCount[0][Index][iPos];                  //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void ProcessSocketCount(int Index)
{
    //JerryYang 20170208 (wei) socket contact計數
    //==>
    if(CUSTOMER_CODE==CC_ASE_CL ||
       CUSTOMER_CODE==CC_PTI ||                                                 //Sam 20170526 (jou) PTI要求socket contact計數
       CUSTOMER_CODE==CC_TERAPOWER ||                                           //Sam 20190916 : 晶兆成也記錄測頭 Coun
       CUSTOMER_CODE==CC_ASE_JP ||                                              //Sam 20220706 : ASE_JP 也記錄測頭 Count
       CUSTOMER_CODE==CC_AMD_M ||
       CosFunction.bOEEFunction ||
       CosFunction.bUseSocketContactCount)                                      //Sam 20220720 : 新增一組 Socket Count
    {
        for(int i=0; i<FTestSuck.iShtRow; i++)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                if(IsNNMode()==NN_2Row)
                {
                    if(LastSet.bUseTestSocket[0][i][j] &&
                       TestSocket.Item[i][j]!=NULL_IC &&
                       TestSocket.Item[i][j]!=HAS_NULL_IC)                      //Steven 20221216 : Socket count加上有無IC判斷
                    {
                        LastSet.iSocketContactCount[i][j]++;
                        if(LastSet.iSocketContactCount[i][j]>=1000000000 ||
                           LastSet.iSocketContactCount[i][j]<0)
                            LastSet.iSocketContactCount[i][j]=0;

                       //Sam 20220720 : 新增一組 Socket Count
                        IniConfig.SocketContactCount[i][j]++;
                        IniConfig.SocketContactCountHistory[i][j]++;
                        if(IniConfig.SocketContactCountHistory[i][j]>=1000000000 || IniConfig.SocketContactCountHistory[i][j]<0)
                            IniConfig.SocketContactCountHistory[i][j]=0;
                    }

                    if(LastSet.bUseTestSocket[1][i+2][j] &&                     //Sam 20201021 : Fix 32Site contact count
                       TestSocket.Item[i+2][j]!=NULL_IC &&
                       TestSocket.Item[i+2][j]!=HAS_NULL_IC)                    //Steven 20221216 : Socket count加上有無IC判斷
                    {
                        LastSet.iSocketContactCount[i+2][j]++;
                        if(LastSet.iSocketContactCount[i+2][j]>=1000000000 || LastSet.iSocketContactCount[i][j]<0)
                            LastSet.iSocketContactCount[i+2][j]=0;

                        //Sam 20220720 : 新增一組 Socket Count
                        IniConfig.SocketContactCount[i+2][j]++;
                        IniConfig.SocketContactCountHistory[i+2][j]++;
                        if(IniConfig.SocketContactCountHistory[i+2][j]>=1000000000 || IniConfig.SocketContactCountHistory[i][j]<0)
                            IniConfig.SocketContactCountHistory[i+2][j]=0;
                    }

                    if(LastSet.iSocketContactCount[i][j]>=1000000000 || LastSet.iSocketContactCount[i][j]<0)
                        LastSet.iSocketContactCount[i][j]=0;
                }
                else
                {
                    if(LastSet.bUseTestSocket[Index][i][j] &&
                       TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC)                            //Steven 20221216 : Socket count加上有無IC判斷
                    {
                        LastSet.iSocketContactCount[i][j]++;
                        if(LastSet.iSocketContactCount[i][j]>=1000000000 || LastSet.iSocketContactCount[i][j]<0)
                            LastSet.iSocketContactCount[i][j]=0;

                        //Sam 20220720 : 新增一組 Socket Count
                        IniConfig.SocketContactCount[i][j]++;
                        IniConfig.SocketContactCountHistory[i][j]++;
                        if(IniConfig.SocketContactCountHistory[i][j]>=1000000000 || IniConfig.SocketContactCountHistory[i][j]<0)
                            IniConfig.SocketContactCountHistory[i][j]=0;
                    }
                }
            }
        }
    }
    //<==
    //JerryYang 20170208 (wei) socket contact計數
}
//---------------------------------------------------------------------------
// kevin 20190907 add load cell Measure                                         //kevin 20190907 Arm 測區次數道量測 功能;
//---------------------------------------------------------------------------
void ProcessAutoloadcellMeasureCount()
{
    if(IniConfig.bD67LoadCellMeasure && iOneCycle==0)
    {
        if(TestIF.bLoadCellMeasure)                                             //kevin 20190907 Arm 測區次數道量測 功能;
            iloadcellCount++;
        else
            iloadcellCount=0;

        if(iloadcellCount>= TestIF.iLoadCellCount)
        {
            bIsAutoOneCycle=true;
            fMain->BtnOneCycleClick(fMain);
            iloadcellCount=0;
            AsebLoadCellTest=true;
        }
    }
}

//===========================================================================
//  PT-W7f APPEND (AI(PT-W7f-ProcessCount2) 20260811) -- the 7 remaining
//  golden atester_ProcessCount.cpp exports.  APPEND-ONLY: nothing above this
//  line was rewritten, reordered or re-indented.
//
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/atester_ProcessCount.cpp
//  (2161 lines, read cp950 / written UTF-8, bare-LF preserved).
//
//  WAVE SCOPE (one line per golden function assigned to this append):
//    ProcessShowTestStatus   golden :225-256   ( 32 lines) ACTIVE  (GATE A)
//    ProcessAutoCleanCount   golden :1309-1572 (264 lines) ACTIVE  (GATE C, D)
//    ProcessArmCount         golden :1574-1726 (153 lines) ACTIVE  (GATE E, F, G)
//    ProcessCount            golden :1749-1997 (249 lines) ACTIVE  (GATE B, F, I)
//    ProcessStartTestData    golden :1999-2018 ( 20 lines) ACTIVE  (GATE A)
//    RecordHistroy           golden :2020-2108 ( 89 lines) ACTIVE  (GATE A, J)
//    ProcessQASampling       golden :2110-2160 ( 51 lines) ACTIVE  (no gate)
//  No function is wholly GATED; every one of the 7 has a real body here.
//  Golden's own relative order among these 7 is preserved.  Their absolute
//  position in golden interleaves with the 14 functions the earlier wave
//  already placed above (e.g. ProcessShowTestStatus sits at golden :225,
//  BEFORE DoLowYieldAlarm :258); append-only forbids re-inserting them there,
//  so they are grouped here instead -- the only ordering deviation, and it is
//  behaviour-neutral (all 7 are free functions with no static-init coupling).
//
//  RETIRES 5 one-line offline stubs in atester_shims.cpp (:147-:151) --
//  ProcessShowTestStatus / ProcessCount / ProcessStartTestData / RecordHistroy
//  / ProcessQASampling.  The integrate step MUST remove those 5 stub bodies or
//  the link is a duplicate-definition error.  ProcessAutoCleanCount and
//  ProcessArmCount have never had a stub anywhere in the tree (verified, see
//  GATE REGISTER preamble) and, exactly like golden, are declared in NO header
//  -- golden atester_ProcessCount.h does not declare them either; they are
//  reached only from ProcessCount, and are defined above their caller here.
//
//  ============================ GATE REGISTER =============================
//  Every absence claim below was re-verified from
//  D:/HT9045/HT9011UC_Cpp_V3.33.906.0 with
//     rg -c --no-heading -g '*.h' -g '*.hpp' -g '!Motor/vendor/**'
//        -g '!EtherCAT/vendor/**' -g '!third_party/**' "\b<SYMBOL>\b" .
//  run twice: first at 2026-08-11 11:27:21 (before writing) and AGAIN at
//  2026-08-11 11:44:51 (immediately before delivery), because a sibling agent's
//  file landing mid-wave can falsify an absence claim made at open-time.  Both
//  runs returned IDENTICAL counts, and 0 matching files for every symbol named
//  below except the three comment-only / unrelated hits called out in place:
//  fTestCategory (1 = the comment at atester_ProcessCount.h:21),
//  SaveGroundESDData (1 = the comment at Public/MyProductionRecord.h:26) and
//  GetTestResult (2 = Automation/auto9045.h:86, a different signature, plus
//  SECSGEM/SecsEventType.h:68, an enumerator).
//
//  GATE A -- fTestCategory (golden cTestCategory.h:20-46, TfTestCategory)
//    Absent: `fTestCategory` has 0 declarations in the tree (the single rg
//AI(W906-FW3-BTQ1) 20260828: SUPERSEDED -- FW3-BTQ1 新增了 forms/fTestCategory.{h,cpp}，
//    TfTestCategory 與 fTestCategory 現在都有宣告。下面的 gate 仍是 #if 0，
//    沒有任何東西因此開始編譯；要解閘另需自行判斷。
//    hit is a COMMENT in atester_ProcessCount.h:21).  SetTestCateCellINT /
//    SetTestCateCellAS / SetTestingCateCell / ShowTestCategory / GetTestResult
//    each return 0 header hits (GetTestResult's 2 hits are unrelated:
//    Automation/auto9045.h:86 `int GetTestResult(AnsiString*)` and
//    SECSGEM/SecsEventType.h:68 an enumerator).
//    Golden lines gated: :238, :242, :246, :251, :255 (ProcessShowTestStatus);
//                        :2008, :2012, :2017 (ProcessStartTestData);
//                        :2035, :2038, :2055, :2058, :2074 (RecordHistroy).
//    WHY THE OFFLINE DEFAULT IS FAITHFUL.  The four Set* entry points are pure
//    writes into TfTestCategory's own TestResult[2][4][8] / ColorPtr[2][4][8]
//    arrays plus a TStringGrid repaint (golden cTestCategory.cpp:361-463);
//    nothing outside the form reads them EXCEPT GetTestResult, which is gated
//    on the same object.  So no-op writes plus a read-back of the array's reset
//    state is exactly self-consistent.  That reset state is not invented:
//    golden TfTestCategory::InitCateCell (cTestCategory.cpp:496-509) fills
//    ColorPtr[i][j][k]=clWhite and TestResult[i][j][k]=-1 -- hence
//    PCW7_TESTCAT_GETRESULT yields (*CellColor=clWhite, -1), i.e. the value a
//    real machine returns for a cell nobody has painted yet.
//    REAL-MACHINE DIFFERENCE.  On iron the grid shows per-site bins in
//    green/red/silver/white, and RecordHistroy's HistroyBin[][][0] /
//    HistroyPassFail[][][0] receive the ACTUAL bin and pass flag.  Here they
//    receive -1 / false every cycle, so the history ring shifts real values out
//    and -1 in.  HistroyBin/HistroyPassFail have no other producer in this
//    tree, so nothing downstream is fed a mixture of real and degraded data.
//
//  GATE B -- fMain->ResetForESC(AnsiString) (golden main.h)
//    Absent: 0 header hits for `ResetForESC`.  Golden line gated: :1803.
//    FAITHFUL: golden's own guard is
//    `EmptySocketCheckModeBeUse() && IniConfig.bI41_5_RegularExecutionCycle`
//    -- an RFMD-only Empty-Socket-Check feature; offline the tree has no
//    empty-socket-check one-cycle to start, so skipping the trigger equals
//    "feature not installed", which is the observed baseline.
//    REAL-MACHINE DIFFERENCE + KNOWN CONSEQUENCE OF THE GATE: golden does NOT
//    zero iESC_IndexContactCount at this call site (ResetForESC does it deeper
//    in), so with the trigger gated the counter keeps climbing and the (no-op)
//    branch is taken on every contact past the threshold instead of once.  That
//    is a gate artefact, NOT golden behaviour -- flagged for GA-3.
//
//  GATE C -- fLotInfo members: edtAutoCleanContactCount (golden uLotInfo.h,
//    TEdit*), RefreshYieldMonitor(), lblAdaptiveLowYield (TLabel*),
//    AutoTempOfsByFTP()
//    Absent: 0 header hits for each of the four names (forms/fLotInfo.h's
//    TfLotInfo carries edtSysLotID/edtCusLotID/... and none of these).
//    Golden lines gated: :1344, :1348, :1381, :1502, :1504 and :1779.
//    FAITHFUL: the first three are display-only (an edit box, a label caption,
//    and a yield-monitor grid repaint) and feed no decision in this file.
//    AutoTempOfsByFTP() is NOT display-only -- it is the consumer of
//    LastSet.iAutoTempOfsTriggerCnt (incremented ACTIVE one line above, golden
//    :1778) and on iron pushes an FTP-derived temperature offset.  Offline
//    no-op == "the FTP auto-temp-offset feature is not configured", which is
//    the shipping default for every customer that does not enable it; the
//    counter still ticks, so a later wave that lands the real method sees a
//    correct count rather than a starved one.
//
//  GATE D -- fCleaning->ChangeACSmartInterval(int, AnsiString)
//    (golden AutoClean/uCleaning.h; note fCleaning ITSELF exists --
//    forms/fCleaning.h -- but its stand-in has no such method: 0 header hits
//    for `ChangeACSmartInterval`.)
//    Golden lines gated: :1419, :1438, :1457, :1477, :1494 (Smart Auto Clean
//    adaptive-interval up/down/normal ladder).
//    FAITHFUL: every one of the 5 call sites is inside
//    `CosFunction.bSmartAutoClean && TestIF.bACSmart && iRunACSmart==0` and
//    `CosFunction.bAdaptiveYield && Prod.bAdaptiveLowYield`; the method's only
//    job is to move iAdaptiveACInterval (read back at golden :1349) and log.
//    With it gated iAdaptiveACInterval stays where it is, i.e. the interval
//    never adapts -- identical to running with Smart Auto Clean's adaptive
//    tuning switched off, which is the default profile.  Everything else in
//    the ladder (iACUseParam, iAdaptiveLowYieldCntNor/Min,
//    dAdaptiveStardardYield, the WAR0729/WAR0730 alarms and the
//    Prod.iAutoClean_* parameter copy) is ACTIVE and untouched.
//    REAL-MACHINE DIFFERENCE: interval widens to 99 / narrows to -99 / steps
//    +/-1 as yield drifts; here it is constant.
//
//  GATE E -- TMyProductionRecord::SaveGroundESDData(AnsiString)
//    (golden Public/MyProductionRecord.cpp:660-1384 block)
//    Absent: 1 rg hit, and it is a COMMENT -- Public/MyProductionRecord.h:26
//    lists SaveGroundESDData among the methods that wave deliberately left
//    undeclared (it pulls ProductionInfo.h/VacuumUnit.h/..., 9 of 10 not in
//    the tree).  Golden line gated: :1628.
//    FAITHFUL: it is a per-IC file-append of the ground/ESD measurement into
//    the production log.  Nothing in the control path reads it back.  Offline
//    no-op == "production-record ground/ESD logging not enabled".
//    REAL-MACHINE DIFFERENCE: the ground/ESD column of the prod log is empty.
//
//  GATE F -- fYieldMonitoring interval counters + CalculateSiteYield()
//    (golden uYieldMonitoring.h)
//    NOT an absence claim about the OBJECT: fYieldMonitoring DOES exist
//    (aHotPlateSubstrate.h:1149, type TfYieldMonitoring_2x4_16, defined in
//    ainarm9045_2x4_16_shims.cpp) -- but that stand-in class declares exactly
//    ONE member, DoAutoCloseSite(bool).  0 header hits for each of
//    iFailAlarmSiteYieldIntervalCount / iPickerYieldIntervalCount /
//    iFailAlarmSiteMaxYieldIntervalCount /
//    iAutoClean_FailAlarmSiteYieldIntervalCount / iLowYieldContactCount /
//    iLowYieldByTotalContactCount / CalculateSiteYield.
//    Golden lines gated: :1658, :1662, :1666, :1670, :1687, :1693, :1698,
//    :1704, :1709, :1715 (ProcessArmCount) and :1994 (ProcessCount).
//    FAITHFUL: this is the SAME object and the SAME rationale as this file's
//    pre-existing gate #5 (PC_YIELDMON_CLEARCOUNT, 6 sites in
//    CheckContinuoussFail) -- consumers and producers of these counters both
//    live inside the untranslated TfYieldMonitoring form, so gating producer
//    AND consumer together is self-consistent: no half-fed alarm can fire.
//    REAL-MACHINE DIFFERENCE: the site-yield / picker-yield / low-yield
//    interval alarms (and their auto-close-site actions) never arm here.
//    NOTE the deliberate asymmetry preserved from golden: golden runs the SAME
//    four counters twice -- once per-IC inside the row/col loop when
//    CosFunction.bYieldControlUseEACount is TRUE (:1654-1671), once per-contact
//    outside it when that flag is FALSE (:1678-1718).  Both arms are
//    transcribed; the "//    if(bEnter)" / "//    }" commented-out guard at
//    golden :1676-1677/:1719 and the vestigial `bEnter_1` (set true at :1576,
//    tested at :1721, set false at :1723 and never read again -- so it is true
//    on EVERY call and the block always runs) are golden quirks kept verbatim,
//    NOT translation slips.
//
//  GATE G -- fShowBinSelect->iLowYieldBinSelectContactCount
//    (golden uShowBinSelect.h)  Absent: 0 header hits for `fShowBinSelect`
//    and 0 for `iLowYieldBinSelectContactCount`.  Golden line gated: :1724.
//    FAITHFUL: same shape as GATE F -- the counter's only reader is the
//    untranslated BinSelect low-yield form.  Offline no-op.
//
//  GATE H -- reserved; ResetForESC is registered above as GATE B.
//
//  GATE I -- fContactCT (golden cContactCT.h): ReturnSiteDataArray(bool,int,int)
//    and sgYield (TStringGrid*).  Absent: 0 header hits for `fContactCT`,
//    0 for `ReturnSiteDataArray`.  Golden lines gated: :1874, :1906 (reads)
//    and :1995 (grid repaint).
//    FAITHFUL DEFAULT = 0.  ReturnSiteDataArray returns the per-site yield the
//    Contact-CT form has accumulated; with that form absent, no site has
//    accumulated anything, and 0 is that state.  Follow the arithmetic: at
//    golden :1906 iStandardYield[i][j]=0, at :1874 iYield[i][j]=0, so
//    iDifferenceYield=0-0=0 at :1875 and the `iDifferenceYield[i][j]>0` guard
//    at :1880 is false -- bIsLowYield stays false, no site is closed, no
//    spurious WAR fires and no AutoClean is triggered.  That is precisely a
//    fresh machine that has not yet contacted anything.  This is NOT "the
//    degraded value happens to be equivalent" hand-waving: 0 is the literal
//    initial state of the thing being read.
//    REAL-MACHINE DIFFERENCE: the E53 Low-Yield-Auto-Clean site comparison
//    (Prod.bFailAlarmLowYield_AutoClean) can never trip here.
//
//  GATE J -- RecordHistroy's handler-result TCP block, golden :2082-2106,
//    kept VERBATIM inside `#if 0` (the aRotateKIT.cpp idiom) because it needs
//    THREE absent things at once: fObserver->TimeInfoGrid (0 header hits;
//    atester_shims.h's TfObserverShim has Memo1Lines/labModel/... and no grid),
//    fMain->HandlerTCPIPResultSendProcess and
//    fMain->HandlerTeraTResultSendProcess (0 header hits each).
//    fObserver->UpdateBin() (golden :2079, 0 header hits) is gated separately
//    one line above.
//    FAITHFUL: the whole block is guarded by
//    CosFunction.bEnableHandlerResultServer and its ONLY effect is composing a
//    local `String sMessage` and pushing it down two TCP sinks -- no global is
//    written, no return value, no control flow.  Compiling it out is therefore
//    EXACTLY equivalent to running with bEnableHandlerResultServer==false,
//    which is the default for every customer that has not bought the
//    handler-result server.
//    REAL-MACHINE DIFFERENCE: customers with the flag on stop receiving the
//    "HTSR,501,<test time>,<index time>,<bin per site>..." push.
//
//  NOT GATED -- two dependencies that an open-time reading of this file's
//  ORIGINAL banner would have gated, and that are now real.  Both of that
//  banner's claims have EXPIRED and are corrected here (the banner text above
//  is left untouched because this append may not rewrite it):
//    * InitialAutoCleanAllTask() -- original gate #2 said "the AutoClean engine
//      module is not translated".  It IS: AutoClean/AutoClean.h:103 declares it
//      and AutoClean/AutoClean.cpp:813 defines a real body, registered in
//      CMakeLists.txt under ht9045_sm (the same archive as this file).  Golden
//      :1933 therefore calls it FOR REAL here.  Likewise EnableAutoclean
//      (AutoClean.cpp:3643) at golden :1328, and InitialAutoCleanTask /
//      InitialShuttleAutoCleanTask / InitialIndexAutoCleanTask
//      (AutoClean.cpp:827/:855/:862) at golden :1541-1543 and :1558-1560.
//    * fTesterTCP->CopyOSTestResult(i,j) (golden :1629) -- the real body is
//      translated as the free function TesterTCP_CopyOSTestResult(int,int)
//      (Interface/TesterTCP.h:151, body Interface/TesterTCP.cpp:455, ht9045_sm).
//      Its own header notes it had NO production caller; ProcessArmCount is now
//      the FIRST one.  Its golden guard is intact and self-protecting:
//      TestIF_File.iTestType != TCP_IP_MODE (==3, cmydef.h:64) returns
//      immediately, and offline iTestType is 0 -- so no filesystem side effect
//      occurs unless a recipe genuinely selects the OS-Tester TCP mode.  This
//      is the trap-3 question asked and answered: it SHOULD NOT be gated,
//      because the guard golden already wrote is the correct gate.
//===========================================================================
#include <cmath>                    // ceil()   -- golden :2128 / :2142
#include <cstdlib>                  // abs(int) -- golden :1880/:1885/:1889/:1893.
                                    // int overload matters: golden concatenates the
                                    // result into an AnsiString, so a double abs()
                                    // would change the rendered text.
#include "cSocket.h"                // TArm + ArmData/ArmDataLot/ArmHistory/ArmData_AutoClean
#include "myswitch.h"               // SW[] (SwTesterAirCooling, golden :1818)
#include "AutoClean/AutoClean.h"    // EnableAutoclean / InitialAutoClean*Task / InitialAutoCleanAllTask
#include "Interface/TesterTCP.h"    // TesterTCP_CopyOSTestResult (REAL -- see GATE REGISTER)
// acatchtray_shims.h is the tree's canonical home for NewRecordProcess (real
// body acatchtray_shims.cpp:132) -- ainarm2.cpp:46 reaches it exactly this way.
// NOT cMyDB.h: cMyDB.h re-declares MyDBIProcessNew/RecordProcess with defaults
// that ambiguate against canary_support.h (already included above), which is
// precisely the clash ainarm2.cpp:63 documents.
// It also carries the guarded global TColor/clWhite/clYellow block
// (acatchtray_shims.h:123-128), which is why only clSilver/clGreen/clRed are
// imported from vclcompat below -- importing clWhite/clYellow as well would
// make every use of those two names ambiguous.
#include "acatchtray_shims.h"       // NewRecordProcess (golden :1522) + TColor/clWhite/clYellow
#include "vclcompat/BtnPanelCore.h" // vclcompat's guarded cl* block (clSilver/clGreen/clRed)

// AI(PT-W7f-ProcessCount2) 20260811: golden spells the VCL colour constants
// unqualified (Graphics.hpp puts them at global scope).  In this tree the three
// that acatchtray_shims.h does NOT provide live in namespace vclcompat
// (vclcompat/BtnPanelCore.h, guarded by HT9045_W7C1_TCOLOR_SHIM and shared
// verbatim with TrayCore.h/LedCore.h), so the established per-TU idiom is a
// using-declaration -- exactly what mykitsuck.h:107, ATC/ATCInterface.h:167-175
// and EJ1N/MyOmronPanel.h:169-171 already do.  TColor itself is NOT imported
// from vclcompat: cmydef.h:16 / acatchtray_shims.h:125 already provide the
// global `typedef int TColor` that this file's `TColor CellColor;` (golden
// :2022) binds to, and every spelling is the same underlying int.
using vclcompat::clSilver;   // golden :238
using vclcompat::clGreen;    // golden :242 / :2036 / :2039 / :2056 / :2059 / :2075
using vclcompat::clRed;      // golden :246
// clWhite (golden :251 / :2012) and clYellow (golden :2008) come from
// acatchtray_shims.h:126-127 -- same 0x00FFFFFF / 0x0000FFFF values.

//---------------------------------------------------------------------------
//  PT-W7f gate macros.  Same convention as this file's existing PC_* macros
//  (and aTester_Front.cpp's W64B_*): the golden call's ARGUMENTS stay named
//  and stay type-checked -- `(void)(x)` keeps every identifier and every
//  colour constant load-bearing, so a later rename or a wrong constant still
//  breaks the build instead of rotting silently inside a comment.
//  Letters match the GATE REGISTER above.
//---------------------------------------------------------------------------
// GATE A -- fTestCategory (golden cTestCategory.h:20-46).
#define PCW7_TESTCAT_SETINT(Arm, R, C, Bin, Color)   ((void)(Arm),(void)(R),(void)(C),(void)(Bin),(void)(Color))
#define PCW7_TESTCAT_SETAS(Arm, R, C, Bin, Color)    ((void)(Arm),(void)(R),(void)(C),(void)(Bin),(void)(Color))
#define PCW7_TESTCAT_SETTESTING(Arm, R, C, Color)    ((void)(Arm),(void)(R),(void)(C),(void)(Color))
#define PCW7_TESTCAT_SHOW(Arm)                       ((void)(Arm))
// Read-back default reproduces golden InitCateCell (cTestCategory.cpp:504-505):
// ColorPtr=clWhite, TestResult=-1.  See GATE A in the register.
#define PCW7_TESTCAT_GETRESULT(Arm, R, C, pColor)    ((void)(Arm),(void)(R),(void)(C), *(pColor)=clWhite, -1)
// GATE B -- fMain->ResetForESC (golden main.h).
#define PCW7_MAIN_RESETFORESC(S)                     ((void)(S))
// GATE C -- fLotInfo members (golden uLotInfo.h).
#define PCW7_LOTINFO_ACCONTACTCOUNT(V)               ((void)(V))
#define PCW7_LOTINFO_REFRESHYIELD()                  ((void)0)
#define PCW7_LOTINFO_ADAPTIVECAPTION(S)              ((void)(S))
#define PCW7_LOTINFO_AUTOTEMPOFSBYFTP()              ((void)0)
// GATE D -- fCleaning->ChangeACSmartInterval (golden AutoClean/uCleaning.h).
#define PCW7_CLEANING_CHANGEACSMARTINTERVAL(N, S)    ((void)(N),(void)(S))
// GATE E -- TMyProductionRecord::SaveGroundESDData (golden MyProductionRecord.cpp:660-1384 block).
#define PCW7_PORDREC_SAVEGROUNDESD(R, C, Lot)        ((void)(R),(void)(C),(void)(Lot))
// GATE F -- fYieldMonitoring counters / CalculateSiteYield (golden uYieldMonitoring.h).
//           The field name is NEVER expanded, so it does not have to exist.
#define PCW7_YIELDMON_INC(Field)                     ((void)0)
#define PCW7_YIELDMON_CALCSITEYIELD()                ((void)0)
// GATE G -- fShowBinSelect->iLowYieldBinSelectContactCount (golden uShowBinSelect.h).
#define PCW7_BINSELECT_LOWYIELDCT_INC()              ((void)0)
// GATE I -- fContactCT (golden cContactCT.h).  0 == "no site has accumulated
//           any contact yet", the literal initial state of the absent form.
#define PCW7_CONTACTCT_RETURNSITEDATAARRAY(B, R, C)  ((void)(B),(void)(R),(void)(C), 0)
#define PCW7_CONTACTCT_SGYIELD_REFRESH()             ((void)0)
// GATE J's companion -- fObserver->UpdateBin (golden cObserver.h).
#define PCW7_OBSERVER_UPDATEBIN()                    ((void)0)

//---------------------------------------------------------------------------
//  ProcessShowTestStatus -- golden atester_ProcessCount.cpp:225-256
//---------------------------------------------------------------------------
void ProcessShowTestStatus(int Index)
{
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC &&
               TestSocket.Item[i][j]!=HAS_NULL_IC)
            {
                if(TestIF_File.iCloseSiteOnHPDontTest==2 &&                     //Steven 20251022 : 關site的位置有IC不測試送指定 bin
//                   TestSocket.iBinData[i][j]==iTestBinCount &&
                   //AI(PT-W7f-ProcessCount2) 20260811: golden :236 reads
                   //`...asBuffer->Strings[eErrorCode].AnsiPos("NonTestToSettedBin")`.
                   //vclcompat's TStringList::Strings[] returns a StringsProxy (an
                   //lvalue proxy with operator AnsiString), not an AnsiString, so the
                   //member call needs the proxy materialised first.  Same idiom
                   //database.cpp:1947 documents.  Pure type plumbing: identical value,
                   //identical AnsiPos semantics, no behaviour change.
                   AnsiString(TestSocket.PordRec[i][j].asBuffer->Strings[eErrorCode]).AnsiPos("NonTestToSettedBin")!=0)
                {
                    PCW7_TESTCAT_SETINT(Index, i, j, TestIF_File.iCloseSiteBin, clSilver);
                }
                else if(TestSocket.bPass[i][j])                                 //kevin 20140322
                {
                    PCW7_TESTCAT_SETINT(Index, i, j, TestSocket.Item[i][j]-TEST_PASS, clGreen);
                }
                else
                {
                    PCW7_TESTCAT_SETINT(Index, i, j, TestSocket.Item[i][j]-TEST_PASS, clRed);
                }
            }
            else                                                                //kevin 20150119 沒有 ic sit不秀
            {
                PCW7_TESTCAT_SETINT(Index, i, j, -1, clWhite);
            }
        }
    }
    PCW7_TESTCAT_SHOW(Index);                                     //jou 981017 level down display loading
}

//---------------------------------------------------------------------------
//  ProcessAutoCleanCount -- golden atester_ProcessCount.cpp:1309-1572
//---------------------------------------------------------------------------
void ProcessAutoCleanCount()
{
    AnsiString s="";                                                            //Sam 20230620 : 優化 Smart Auto Clean
    int IntervalCount=0, sum=0, ipass=0;
    double dYield=0.0;

    AnsiString sYield1="", sYield2="";

    if(IniConfig.bEnableAutoCleanFunction)
    {
        if(CosFunction.bAutoCleanOnHotPlate &&
           bACInterval &&
           LastSet.iRunStartMode!=rsmAutoSiteMap)
        {
            if(TestIF.iAutoClean_Function)                                      //Auto Clean
                iAutoClean_IndexContactCount++;
            else
                iAutoClean_IndexContactCount=0;
            fMain->AutoCleanContactCountLabel->Caption=iAutoClean_IndexContactCount;
            EnableAutoclean(false);                                             //kevin 20120501 啟動autoclean
        }
        else
        {
            //Steven 20260427 : ATK P260427-ATK-H9-01 fix operator precedence
            //                  Original: A && B || C => (A && B) || C, lets SOCKET_ALARM bypass AutoSiteMap guard
            if(LastSet.iRunStartMode!=rsmAutoSiteMap &&
               ((TestIF_File.iAutoClean_Mode&M_INTERVAL) ||
                (TestIF_File.iAutoClean_Mode&M_SOCKET_ALARM)))                  //ChungHung 20131225 add
            {
                if(TestIF.iAutoClean_Function)                                  //Auto Clean
                    iAutoClean_IndexContactCount++;
                else
                    iAutoClean_IndexContactCount=0;

                fMain->AutoCleanContactCountLabel->Caption=iAutoClean_IndexContactCount;
                PCW7_LOTINFO_ACCONTACTCOUNT(iAutoClean_IndexContactCount);                                  //ChungHung 20131225 add

                if(CosFunction.bSmartAutoClean && TestIF.bACSmart)
                {
                    PCW7_LOTINFO_REFRESHYIELD();
                    IntervalCount=iAdaptiveACInterval;
                }
                else
                {
                    IntervalCount=TestIF.iAutoClean_IntervalContact;
                }

                if(CosFunction.bAdaptiveYield && Prod.bAdaptiveLowYield)        //Sam 20230914 : 自適應性良率監控
                {
                    sum=0;
                    ipass=0;
                    for(int i=0; i<6; i++)
                    {
                        sum+=LastSet.BinCT[0][i];
                        if(BinSelect[iTestRunMode].iStackDefFailCate[i]==0)
                            ipass+=LastSet.BinCT[0][i];
                    }

                    if(sum>0)
                    {
                        dYield=(double)ipass*100/sum;
                    }
                    else
                    {
                        dYield=0;
                    }

                    if(sum>Prod.iLowYieldCount &&
                       dAdaptiveStardardYield<0)                                // IC 數量到了計算良率當標準
                    {
                        dAdaptiveStardardYield=dYield;
                        s.sprintf("Adaptive Low Yield : %2.2f%s", dAdaptiveStardardYield, "%");
                        PCW7_LOTINFO_ADAPTIVECAPTION(s);
                    }
                }

                //Steven 20260427 : ATK P260427-ATK-H9-01 fix off-by-one
                //                  Original >=(IntervalCount-1) triggers one IC too early
                if(TestIF.iAutoClean_Function &&
                   IntervalCount!=0 &&
                   (TestIF_File.iAutoClean_Mode & M_INTERVAL) &&
                   iCleanOut==0 &&
                   iAutoClean_IndexContactCount>=IntervalCount)
                {
                    if(CosFunction.bSmartAutoClean &&
                       TestIF.bACSmart &&
                       iRunACSmart==0)                                          //Sam 20230620 : 優化 Smart Auto Clean
                    {
                        iRunACSmart=2;
                        if(dAdaptiveStardardYield<0)                            //若 Interval 到了 IC 數量未到就計算良率當標準
                            dAdaptiveStardardYield=dYield;

                        if(CosFunction.bAdaptiveYield &&
                           Prod.bAdaptiveLowYield)                              //Sam 20230914 : 自適應性良率監控
                        {                                                       //良率高於Max設定
                            if(dYield>(Prod.dLowYieldLimit+Prod.iAdaptiveYieldMax))
                            {
                                iAdaptiveLowYieldCntNor=0;
                                iAdaptiveLowYieldCntMin=0;
                                iACUseParam=1;
                                s.sprintf("Start smart auto clean use %d parameter. ↑%2.2f%s > %2.2f%s Max Area. Yield %2.2f%s change to %2.2f%s",
                                            iACUseParam,
                                            dYield,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMax,
                                            "%",
                                            dAdaptiveStardardYield,
                                            "%",
                                            dYield,
                                            "%");
                                PCW7_CLEANING_CHANGEACSMARTINTERVAL(99, s);        //良率大於MAX 設定無條件interval 升至最高Count
                            }
                            else
                            {
                                if(dYield<dAdaptiveStardardYield)               //低於自適應性良率
                                {                                               //良率低於Min設定
                                    if(dYield<(Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin))
                                    {
                                        iACUseParam=1;
                                        s.sprintf("Start smart auto clean use %d parameter. ↓%2.2f%s < %2.2f%s Min Area. Yield %2.2f%s change to %2.2f%s",
                                            iACUseParam,
                                            dYield,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin,
                                            "%",
                                            dAdaptiveStardardYield,
                                            "%",
                                            dYield,
                                            "%");
                                        PCW7_CLEANING_CHANGEACSMARTINTERVAL(-99, s);
                                        iAdaptiveLowYieldCntNor++;              //Min Area 無條件interval 降至最低Count
                                        iAdaptiveLowYieldCntMin++;
                                    }
                                    else                                        //良率介於Normal間設定
                                    {
                                        iACUseParam=1;
                                        s.sprintf("Start smart auto clean use %d parameter. ↘%2.2f%s = %2.2f%s - %2.2f%s Normal Area. Yield %2.2f%s change to %2.2f%s",
                                            iACUseParam,
                                            dYield,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMax,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin,
                                            "%",
                                            dAdaptiveStardardYield,
                                            "%",
                                            dYield,
                                            "%");
                                        PCW7_CLEANING_CHANGEACSMARTINTERVAL(-1, s);                                        //Normal Area比較前次良率較低執行AI clean 2.Adaptive Interval原參數遞減 X Count
                                        iAdaptiveLowYieldCntNor++;
                                    }
                                }
                                else
                                {                                               //良率低於Min設定
                                    if(dYield<(Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin))
                                    {
                                        //iACUseParam=2;
                                        iACUseParam=1;
                                        s.sprintf("Start smart auto clean use %d parameter. ↓%2.2f%s < %2.2f%s Min Area. Yield %2.2f%s change to %2.2f%s",
                                            iACUseParam,
                                            dYield,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin,
                                            "%",
                                            dAdaptiveStardardYield,
                                            "%",
                                            dYield,
                                            "%");
                                         PCW7_CLEANING_CHANGEACSMARTINTERVAL(-99, s);
                                    }                                           //Min Area 無條件interval 降至最低Count
                                    else                                        //良率介於Normal間設定
                                    {
                                        iACUseParam=1;
                                        s.sprintf("Start smart auto clean use %d parameter. ↗%2.2f%s = %2.2f%s - %2.2f%s Normal Area. Yield %2.2f%s change to %2.2f%s",
                                            iACUseParam,
                                            dYield,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMax,
                                            "%",
                                            Prod.dLowYieldLimit+Prod.iAdaptiveYieldMin,
                                            "%",
                                            dAdaptiveStardardYield,
                                            "%",
                                            dYield,
                                            "%");
                                        PCW7_CLEANING_CHANGEACSMARTINTERVAL(1, s);                                         //Min Area 無條件interval 降至最低Count
                                    }
                                    iAdaptiveLowYieldCntNor=0;
                                    iAdaptiveLowYieldCntMin=0;
                                }
                            }

                            dAdaptiveStardardYield=dYield;                      //以當前的良率作為自適應性良率
                            PCW7_LOTINFO_REFRESHYIELD();
                            s.sprintf("Adaptive Low Yield : %2.2f%s", dAdaptiveStardardYield, "%");
                            PCW7_LOTINFO_ADAPTIVECAPTION(s);

                            if(iAdaptiveLowYieldCntNor>=Prod.iAdaptiveContsLowerAlarmNor)
                            {
                                iAdaptiveLowYieldCntNor=0;
                                iAdaptiveLowYieldCntMin=0;
                                ShowErrorMessage("WAR0729", K_RETRY, MMSystem, false);
                                return;
                            }
                            else if(iAdaptiveLowYieldCntMin>=Prod.iAdaptiveContsLowerAlarmMin)
                            {
                                iAdaptiveLowYieldCntNor=0;
                                iAdaptiveLowYieldCntMin=0;
                                 ShowErrorMessage("WAR0730", K_RETRY, MMSystem, false);
                                 return;
                            }
                            else
                            {
                                NewRecordProcess("",s,IntToStr(iAutoClean_IndexContactCount));
                                if(iACUseParam==2)
                                {
                                    Prod.iAutoClean_ContactMode =TestIF_File.iACSmart_ContactMode;
                                    Prod.iAutoCleanDropHigh     =TestIF_File.iACSmart_DropHigh;
                                    Prod.iAutoClean_ContactTime =TestIF_File.iACSmart_ContactTime;
                                    Prod.iAutoClean_ContactCount=TestIF_File.iACSmart_ContactCount;
                                }
                                else
                                {
                                    Prod.iAutoClean_ContactMode =TestIF_File.iAutoClean_ContactMode;
                                    Prod.iAutoCleanDropHigh     =TestIF_File.iAutoCleanDropHigh;
                                    Prod.iAutoClean_ContactTime =TestIF_File.iAutoClean_ContactTime;
                                    Prod.iAutoClean_ContactCount=TestIF_File.iAutoClean_ContactCount;
                                }
                            }
                        }
                    }

                    InitialAutoCleanTask();
                    InitialShuttleAutoCleanTask();
                    InitialIndexAutoCleanTask();
                    bIsAutoOneCycle=true;
                    fMain->BtnOneCycleClick(fMain);
                }
            }

            if(CosFunction.bAutoCleanTimeCT==true &&
               LastSet.iRunStartMode!=rsmAutoSiteMap)                           //jou 20250102 : auto clean triger time count
            {
                if(TestIF_File.iAutoClean_Function==true &&
                   TestIF_File.bEnabledAutoCleanTimeCT==true &&
                   iCleanOut==0 &&
                   (lAutoClean_TimeCount/1000)>=(TestIF_File.iAutoCleanTimeCT*60))
                {
                    RecordProcess("Auto Clean by time count");
                    InitialAutoCleanTask();
                    InitialShuttleAutoCleanTask();
                    InitialIndexAutoCleanTask();
                    bIsAutoOneCycle=true;
                    fMain->BtnOneCycleClick(fMain);
                    lAutoClean_TimeCount=0;
                }
            }
            else
            {
                lAutoClean_TimeCount=0;
            }
        }
    }
}

//---------------------------------------------------------------------------
//  ProcessArmCount -- golden atester_ProcessCount.cpp:1574-1726
//---------------------------------------------------------------------------
void ProcessArmCount(int Index)
{
    bool bEnter_1=true;                                                         //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count
    int iBin;
    int iRow32, iCol32, iArm32;

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(IsNNMode()==NN_2Row)                                             //Steven 20210630 : 修正32site的socket資料顯示
            {
                if(i==2 || i==3)
                {
                    iArm32=0;
                    iRow32=i-2;
                }
                else
                {
                    iArm32=1;
                    iRow32=i;
                }
                iCol32=j;
            }
            else if(IsNNMode()==NN_1Row)
            {
                if(i==1)
                {
                    iArm32=0;
                    iRow32=0;
                }
                else if(i==0)
                {
                    iArm32=1;
                    iRow32=0;
                }
                else
                {
                    continue;
                }
                iCol32=j;
            }
            else
            {
                iArm32=Index;
                iRow32=i;
                iCol32=j;
            }

            if(TestSocket.Item[i][j]>=TEST_PASS)                                //ChungHung 20140516 fix 記憶體溢位
            {
                iBin=TestSocket.Item[i][j]-TEST_PASS;

                TestSocket.PordRec[i][j].AddOrderTestRecord(LastSet.iASEContact);                                       //Frank 20160505 add
                PCW7_PORDREC_SAVEGROUNDESD(i, j, RunInfo.LotNo);      //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
                TesterTCP_CopyOSTestResult(i, j);                             //Steven 20250515 : 整合Open Short測試報表

                ArmData[iArm32]->SetArmSKTData(iRow32, iCol32, iBin);
                ArmDataLot[iArm32]->SetArmSKTData(iRow32, iCol32, iBin);        //Steven 20250603 : by lot summary
                ArmHistory[iArm32]->SetArmSKTData(iRow32, iCol32, iBin);
                ArmData[2]->SetArmSKTData(i, j, iBin);
                ArmDataLot[2]->SetArmSKTData(i, j, iBin);                       //Steven 20250603 : by lot summary
                ArmHistory[2]->SetArmSKTData(i, j, iBin);

#ifdef SOFT_SIMULTE
                if(IniConfig.bVTESTFunction==true && IniConfig.bCheckFile==true)
#else
                if(IniConfig.bVTESTFunction==true && IniConfig.bCheckFile==true && LastSet.iTester==ON_LINE)
#endif
                {
                    if(iBin<iTestBinCount && iBin>=0)                           //Steven 20260316 : 修正測試Bin數量超過設定值導致記憶體溢位
                    {
                        LastSet.iSiteBinCTForAlways[i][j][iBin]++;
                        LastSet.iSiteTotalCTForAlways[i][j]++;
                    }
                }

                if(TestIF_File.iAutoClean_Mode & M_SOCKET_ALARM)                //ChungHung 20131223 add for SCK
                    ArmData_AutoClean[iArm32]->SetArmSKTData(iRow32, iCol32, iBin);                                     //Sam 20230104 : 修正 LowYield AutoClean //ChungHung 20131225 add

                if(CosFunction.bYieldControlUseEACount)                         //Steven 20190819 : 修正計算方式
                {
                    if(Prod.bFailAlarmSiteYieldDifferent &&
                       Prod.iFailAlarmSiteYieldDifferentCount!=0)               //Steven 20120423 : 換位置
                        PCW7_YIELDMON_INC(iFailAlarmSiteYieldIntervalCount);   //Eliot 2007_0524

                    if(Prod.bLowYieldByPicker &&
                       Prod.iLowYieldCountByPicker!=0)                          //Steven 20230223 : 根據Index吸嘴比較良率
                        PCW7_YIELDMON_INC(iPickerYieldIntervalCount);

                    if(Prod.bFailAlarmSiteYieldCmp &&
                       Prod.iFailAlarmSiteYieldCmpCount!=0)                     //jou 2014-08-14 Site Compare Low Yield alarm
                        PCW7_YIELDMON_INC(iFailAlarmSiteMaxYieldIntervalCount);

                    if(TestIF.bAutoClean_FailAlarmSiteYieldDifferent &&
                       TestIF.iAutoClean_FailAlarmSiteYieldDifferentCount!=0)
                        PCW7_YIELDMON_INC(iAutoClean_FailAlarmSiteYieldIntervalCount);
                }
            }
        }
    }

//    if(bEnter) //ChungHung 20131227 add 防止一次contact 計數多次
//    {
        if(CosFunction.bYieldControlUseEACount==false)                          //Steven 20190819 : 修正計算方式
        {
//            bEnter=false;
            if(TestIF_File.iShuttleMode==0 ||
              (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==Index))                                         //Sam 20230823 : 關Arm時不累加 contact count by Yield
            {
                if(Prod.bFailAlarmSiteYieldDifferent &&
                   Prod.iFailAlarmSiteYieldDifferentCount!=0)                   //Steven 20120423 : 換位置
                {
                    PCW7_YIELDMON_INC(iFailAlarmSiteYieldIntervalCount);       //Eliot 2007_0524
                }

                if(Prod.bFailAlarmSiteYieldCmp &&
                   Prod.iFailAlarmSiteYieldCmpCount!=0)                         //jou 2014-08-14 Site Compare Low Yield alarm
                {
                    PCW7_YIELDMON_INC(iFailAlarmSiteMaxYieldIntervalCount);
                }

                if(Prod.bLowYieldByPicker && Prod.iLowYieldCountByPicker!=0)    //Steven 20230223 : 根據Index吸嘴比較良率
                {
                    PCW7_YIELDMON_INC(iPickerYieldIntervalCount);
                }

                if(TestIF.bAutoClean_FailAlarmSiteYieldDifferent &&
                   TestIF.iAutoClean_FailAlarmSiteYieldDifferentCount!=0)
                {
                    PCW7_YIELDMON_INC(iAutoClean_FailAlarmSiteYieldIntervalCount);
                }

                if(Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0)           //JerryYang 20160530 LowYieldLimit要能設定到小數點
                {
                    PCW7_YIELDMON_INC(iLowYieldContactCount);                  //Steven 20141212 : Yield控制使用Contact Count
                }

                if(Prod.bFailAlarmLowYieldByTotal &&
                   Prod.dLowYieldLimitByTotal!=0)                               //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count
                {
                    PCW7_YIELDMON_INC(iLowYieldByTotalContactCount);
                }
            }
        }
//    }

    if(bEnter_1==true)                                                          //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count
    {
        bEnter_1=false;
        PCW7_BINSELECT_LOWYIELDCT_INC();                       //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count
    }
}

//---------------------------------------------------------------------------
//  ProcessCount -- golden atester_ProcessCount.cpp:1749-1997
//---------------------------------------------------------------------------
void ProcessCount(int Index, bool bHasIC)  //Eastsun 20260515 F022: D7
{
    bool bContinuousContactAlarm=false;
    ArmData[Index]->SetContactCT(1);
    ArmHistory[Index]->SetContactCT(1);

    ArmData[2]->SetContactCT(1);
    ArmHistory[2]->SetContactCT(1);

    //wei 20141201 Low Yield Auto Clean
    bool bIsLowYield=false;
    AnsiString SocketErrPart="";
    AnsiString aLowYield;
    AnsiString aStandardYield;
    //wei 20141201 Low Yield Auto Clean

    if(TestIF_File.iAutoClean_Function && (TestIF_File.iAutoClean_Mode & M_SOCKET_ALARM))                               //ChungHung 20131223 add for SCK
    {
        ArmData_AutoClean[Index]->SetContactCT(1);
        ArmData_AutoClean[2]->SetContactCT(1);
    }

    if(TestSocket.HasRealIC()==true)                                            //Steven 20200114 : 修改計算方式
    {
        ProcessHeadContactCount(Index);                                         //Ifor 20160516 京元要求銦片 Life Time 功能
        ProcessSocketCount(Index);                                              //JerryYang 20170208 (wei) socket contact計數
        LastSet.iContactCT[Index]++;                                            //jou 2012-06-05
        LastSet.SendCT[3]++;
        LastSet.iIndexCount++;                                                  //wei 20141201 Low Yield Auto Clean
        LastSet.iAutoTempOfsTriggerCnt++;                                       //Sam 20220406 : 溫度自動補償功能 By FTP
        PCW7_LOTINFO_AUTOTEMPOFSBYFTP();                                           //Sam 20220406 : 溫度自動補償功能 By FTP
        if(CosFunction.IntervalYieldCount && Prod.bFailAlarmIntervalLowYieldBySite)                                     //wei 20180606 Interval Low Yield By Site
        {
            if(iYieldSiteCount[Index]>=Prod.iIntervalLowYieldCountBySite-1)
            {
                iYieldSiteCount[Index]=0;
                bYieldSiteBin[Index]=true;
            }
            else
            {
                iYieldSiteCount[Index]++;                                       //wei 20180606 Interval Low Yield By Site
            }
        }

        if(bRunAutoClean==false)                                                //Steven 20160531 : modify
            LastSet.iASEContact++;                                              //kevin 20141020 ASE 記錄此批一CONTACT 次數

        if(EmptySocketCheckModeBeUse() &&
           IniConfig.bI41_5_RegularExecutionCycle)                              //Steven 20201022 : For RFMD Empty Socket Check Funstion.
        {
            iESC_IndexContactCount++;
            if(IniConfig.iI41_5_RegularExecutionCycleCount!=0 &&
               iESC_IndexContactCount>=IniConfig.iI41_5_RegularExecutionCycleCount)
            {
                PCW7_MAIN_RESETFORESC("Start Empty Socket OneCycle by Index Contact Count");
            }
        }
        else
        {
            iESC_IndexContactCount=0;
        }
    }

    //jou 2016-04-28 Socket Air Cooling contact count trun on
    if(IniConfig.bL03SocketAirCoolingCT==true)
    {
        iL03SocketAirCoolingCT++;                                               //jou 2016-04-28 Socket Air Cooling contact count trun on
        if(iL03SocketAirCoolingCT>IniConfig.iL03SocketAirCoolingCT)
        {
            SW[SwTesterAirCooling].On();
        }
    }

    //Steven 20110420
    if(iRunStartMode==FT)
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
            LastSet.SendCT[3]=0;
            if(USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest))                          //kevin 20150615   //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                LastSet.SendCT_ART[3]=0;
        }
        else
        {
            if(TestIF.bContinuousContact==true)
            {
                if(LastSet.SendCT[3]>=int(TestIF.iContinuousContactCount))
                {
                    LastSet.SendCT[3]=0;                                        //20141001 wei add
                    if((USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) || CosFunction.bUseARTSortCount)                        //kevin 20150615  //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                        LastSet.SendCT_ART[3]=0;

                    iWhoTriggerPiggyBack=pbtContinualContactCount;              //Steven 20111207 : 誰觸發了Piggy Back
                    if(CosFunction.bPiggyBackShowMainForm && Prod.iCountAlarmAction==0)                                 //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
                    {
                        bContactCounOven=true;                                  //kevin 20131009 CONTACT COUNT 超過多設定次數秀警告
                    }
                    else if(Prod.iCountAlarmAction==0)                          //20141001 wei add
                    {
                        bContinuousContactAlarm=true;
                    }
                    else
                    {
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }

        if(IniConfig.bE53LowYieldAutoClean)                                     //kevin 20160802   //wei 20141201 Low Yield Auto Clean(%) start
        {
            //參考值設定後,每次進行Site to Site比較時間點為產生參考值之前n個Cycle數之一半
            //If n = 30 Cycles, 每 15 cycles 比較一次
            //If n = 20 Cycles, 每 10 cycles 比較一次
            //P.S:Arm1 & Arm2 各contact一次 = 2 cycles.
            if(Prod.bFailAlarmLowYield_AutoClean && Prod.iLowYieldLimit_AutoClean!=0 && bLowYieldAutoCleanEnd==false)   //wei 20141216  LowYieldautoclean  避免重複進去
            {
                if(LastSet.iIndexCount>=Prod.iLowYieldCount_AutoClean/2 && bStandardYield==true)
                {
                    LastSet.iIndexCount=0;

                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            iYield[i][j]=PCW7_CONTACTCT_RETURNSITEDATAARRAY(false, i, j);
                            iDifferenceYield[i][j]=iStandardYield[i][j]-iYield[i][j];

                            //If 取樣點 Yield < 參考點 Yield 10% , 則啟動Auto Clean Function
                            //If 取樣點 Yield < 參考點 Yield 20% , 則Close Site (Close Site 之前至少要做過一次Auto Clean)
                            //Close Site數不得超過總Site數1/4,否則須Alarm停機
                            if(iDifferenceYield[i][j]>0 && abs(iDifferenceYield[i][j])>Prod.iLowYieldLimit_AutoClean && LastSet.bUseTestSocket[0][i][j]==true)
                            {
                                bIsLowYield=true;
                                iLowYieldSiteCount[i][j]++;

                                if(abs(iDifferenceYield[i][j])>Prod.iLowYieldLimit_AutoClean*2 && iLowYieldSiteCount[i][j]>1)
                                {
                                    bLowYieldCleanOut=true;
                                    bLowYieldCloseSite[0][i][j]=true;
                                    SocketErrPart+=IndexSuckName[i][j]+ " : " + abs(iDifferenceYield[i][j]) + " % Close ,";                                     //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                                }
                                else
                                {
                                    SocketErrPart+=IndexSuckName[i][j]+ " : " + abs(iDifferenceYield[i][j]) + "% ,";    //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                                }
                            }
                        }
                    }
                }
                else if(LastSet.iIndexCount>=Prod.iLowYieldCount_AutoClean && bStandardYield==false)
                {
                    LastSet.iIndexCount=0;
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            iStandardYield[i][j]=PCW7_CONTACTCT_RETURNSITEDATAARRAY(false, i, j);
                            SocketErrPart += IndexSuckName[i][j] + " : " + iStandardYield[i][j] + " % ,";               //JerryYang 20180815 (Steven) : 修正32 site Error part異常
                            bStandardYield=true;
                        }
                        aStandardYield = "Low Yield Start Standard Yield : " + AnsiString(SocketErrPart);
                        RecordProcess(aStandardYield);
                        SocketErrPart="";
                    }
                }
            }

            if(bIsLowYield==true)
            {
                if(bLowYieldCleanOut)
                {
                    aLowYield="Low Yield Close Site : "+AnsiString(SocketErrPart);
                }
                else
                {
                    aLowYield="Low Yield Site : "+AnsiString(SocketErrPart);
                }

                RecordProcess(aLowYield);
                bLowYieldAutoCleanEnd=true;                                     //wei 20141216  LowYieldautoclean  避免重複進去

                if(IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function==true && Prod.bFailAlarmLowYield_AutoClean)
                {
                    InitialAutoCleanAllTask();                                  //Sam 20230504 : 整理 InitialAutoCleanTask
                }
            }
        }                                                                       //wei 20141201 Low Yield Auto Clean(%) end
    }
    else if(iRunStartMode==RT)
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
            LastSet.SendCT[3]=0;
            if((USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) || CosFunction.bUseARTSortCount)                                //kevin 20150615 //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                LastSet.SendCT_ART[3]=0;
        }
        else
        {
            if(TestIF.bContinuousContact_RT==true)
            {
                if(LastSet.SendCT[3]>=int(TestIF.iContinuousContactCount_RT))
                {
                    LastSet.SendCT[3]=0;
                    if((USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) || CosFunction.bUseARTSortCount)                        //kevin 20150615    //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                       LastSet.SendCT_ART[3]=0;

                    iWhoTriggerPiggyBack=pbtContinualContactCount;              //Steven 20111207 : 誰觸發了Piggy Back
                    if(CosFunction.bPiggyBackShowMainForm &&
                       Prod.iCountAlarmAction==0)                               //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
                    {
                        bContactCounOven=true;                                  //kevin 20131009 CONTACT COUNT 超過多設定次數秀警告
                    }
                    else if(Prod.iCountAlarmAction==0)                          //20141001 wei add
                    {
                        bContinuousContactAlarm=true;
                    }
                    else
                    {
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }
    }

    if(bContinuousContactAlarm)                                                 //20141001 wei add
    {
        ShowErrorMessage("WAR07325", 0, MMInterface, false);
    }

    ProcessArmCount(Index);

    if((TestIF_File.iShuttleMode==0 ||
       (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==Index)) &&     //Ifor 20220419 add:Auto Clean
       bHasIC)                                                                   //Eastsun 20260515 F022: D7 fix AutoClean count
    {
        ProcessAutoCleanCount();
    }
    ProcessSocketPurgeCount();
    ProcessShowTestStatus(Index);
    CheckContinuoussFail(Index);
    CheckContinuoussPass(Index);                                                //Eliot 20100709
    CheckContinuoussPassBySocket(Index);                                        //Steven 20110919
    ProcessContinuoussFailForATC(Index);                                        //Steven 20151123 : Continue Fail Temp Offset for ATC
    PCW7_YIELDMON_CALCSITEYIELD();                                     //Steven 20170905 (wei) : Yield Alarm全部移動去fYieldMonitoring
    PCW7_CONTACTCT_SGYIELD_REFRESH();
    ProcessAutoloadcellMeasureCount();                                          //kevin 20190907 add load cell Measure
}

//---------------------------------------------------------------------------
//  ProcessStartTestData -- golden atester_ProcessCount.cpp:1999-2018
//---------------------------------------------------------------------------
void ProcessStartTestData(int iArm)
{
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC)
            {
                if(TestSocket.Item[i][j]<TEST_PASS && TestSocket.Item[i][j]!=HAS_NULL_IC)
                    PCW7_TESTCAT_SETTESTING(iArm, i, j, clYellow);
            }
            else
            {
                PCW7_TESTCAT_SETAS(iArm, i, j, "-1", clWhite);    //kevin 20150119 送給測試機 沒有IC清空
            }
        }
    }

    PCW7_TESTCAT_SHOW(iArm);                                      //jou 981017 level down display loading
}

//---------------------------------------------------------------------------
//  RecordHistroy -- golden atester_ProcessCount.cpp:2020-2108
//---------------------------------------------------------------------------
void RecordHistroy(int attr)
{
    TColor CellColor;
    int iHistroyKCnt=(CUSTOMER_CODE==CC_KYEC_LEE)?21:9;                         //Eastsun 20260526 #026-1.9 Ifor 20221026 add:KYEC 要求增加比數9 => 21
    if(IsNNMode()==NN_1Row)                                                     //Steven 20231023 : fixed for NN mode
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            for(int k=iHistroyKCnt; k>=1; k--)                                  //Eastsun 20260526 #026-1.9
            {
                HistroyBin[0][j][k]=HistroyBin[0][j][k-1];
                HistroyPassFail[0][j][k]=HistroyPassFail[0][j][k-1];
                HistroyBin[1][j][k]=HistroyBin[1][j][k-1];
                HistroyPassFail[1][j][k]=HistroyPassFail[1][j][k-1];
            }
            HistroyBin[0][j][0]=PCW7_TESTCAT_GETRESULT(attr, 0, j, &CellColor);
            HistroyPassFail[0][j][0]=(CellColor==clGreen)?true:false;

            HistroyBin[1][j][0]=PCW7_TESTCAT_GETRESULT(attr, 1, j, &CellColor);
            HistroyPassFail[1][j][0]=(CellColor==clGreen)?true:false;
        }
    }
    else if(IsNNMode()==NN_2Row)
    {
        for(int i=0; i<FTestSuck.iShtRow; i++)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                for(int k=iHistroyKCnt; k>=1; k--)                              //Eastsun 20260526 #026-1.10
                {
                    HistroyBin[i][j][k]=HistroyBin[i][j][k-1];
                    HistroyPassFail[i][j][k]=HistroyPassFail[i][j][k-1];
                    HistroyBin[i+2][j][k]=HistroyBin[i+2][j][k-1];
                    HistroyPassFail[i+2][j][k]=HistroyPassFail[i+2][j][k-1];
                }
                HistroyBin[i][j][0]=PCW7_TESTCAT_GETRESULT(attr, i, j, &CellColor);
                HistroyPassFail[i][j][0]=(CellColor==clGreen)?true:false;

                HistroyBin[i+2][j][0]=PCW7_TESTCAT_GETRESULT(attr, i+2, j, &CellColor);
                HistroyPassFail[i+2][j][0]=(CellColor==clGreen)?true:false;
            }
        }
    }
    else
    {
        for(int i=0; i<FTestSuck.iShtRow; i++)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                for(int k=iHistroyKCnt; k>=1; k--)                              //Eastsun 20260526 #026-1.11
                {
                    HistroyBin[i][j][k]=HistroyBin[i][j][k-1];
                    HistroyPassFail[i][j][k]=HistroyPassFail[i][j][k-1];
                }
                HistroyBin[i][j][0]=PCW7_TESTCAT_GETRESULT(attr, i, j, &CellColor);
                HistroyPassFail[i][j][0]=(CellColor==clGreen)?true:false;
            }
        }
    }
    PCW7_OBSERVER_UPDATEBIN();

    //==>>Isaac 20170809 (wei) TCPIP Reply test result
#if 0   // GATE J (fObserver->TimeInfoGrid / fMain->HandlerTCPIPResultSendProcess /
        //          HandlerTeraTResultSendProcess) -- golden :2082-2106.  Register: file banner.
        //          Golden lines below are VERBATIM; only the #if 0/#endif frame is added.
    if(CosFunction.bEnableHandlerResultServer)
    {
        String sMessage="HTSR,501,";
        sMessage+=fObserver->TimeInfoGrid->Cells[3][11];                        //test time
        sMessage+=",";
        sMessage+=fObserver->TimeInfoGrid->Cells[5][11];                        //index time
        sMessage+=",";
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(HistroyBin[i][j][0]>0)
                {
                    sMessage+=IntToStr(HistroyBin[i][j][0])+",";                //20150130, Nickliu,TCP Reply Result
                }
                else
                {
                    sMessage+="-1," ;
                }
            }
        }

        fMain->HandlerTCPIPResultSendProcess(sMessage);
        fMain->HandlerTeraTResultSendProcess(sMessage);                         //Sam 20230421 : 整理 TCP Log
    }
#endif  // GATE J (golden :2082-2106)
    //<<==Isaac 20170809 (wei) TCPIP Reply test result
}

//---------------------------------------------------------------------------
//  ProcessQASampling -- golden atester_ProcessCount.cpp:2110-2160
//---------------------------------------------------------------------------
void ProcessQASampling(int Index)                                               //Steven 20190326 : QA Sampling
{
    int iPercent;
    int iCount=0;
    int iSiteCount=0;
    if(TestIF_File.bEnableQASampling && TestIF_File.bSCKART_EnableART && fSCKART->iLotCount!=0)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(bTestSiteUse[Index][i][j])
                    iSiteCount++;
            }
        }

        if(iSiteCount!=0)
        {
            iPercent=ceil(double(fSCKART->iLotCount)/double(iSiteCount));       //Steven 20250820 : 針對除以0加上保護
        }
        else
        {
            iPercent=0;
        }

        if(iPercent>TestIF_File.iQASamplingCnt ||
           iPercent==0)
        {
            iPercent=1;
        }
        else
        {
            iPercent=ceil(double(TestIF_File.iQASamplingCnt)/double(iPercent));
        }

        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(iCount<iPercent &&
                   bQASampleCnt<TestIF_File.iQASamplingCnt &&
                   TestSocket.Item[i][j]-TEST_PASS==TestIF_File.iQASamplingBin)
                {
                    TestSocket.bQATray[i][j]=true;
                    iCount++;
                    //AI(PT-W7f-ProcessCount2) 20260811: golden :2155 is literally
                    //`bQASampleCnt++;` and bQASampleCnt is a `bool` (golden
                    //cmydef.h:4824, cmydef.cpp:4874 `bool bQASampleCnt=0;` -- the
                    //port tree mirrors that exact type at cmydef.h:4837).  ++ on a
                    //bool was deprecated in C++98 and REMOVED in C++17, so it will
                    //not compile here; BCB6's semantics for it are "set the operand
                    //to true" (the result is the old value, discarded here), which
                    //`=true` reproduces EXACTLY.  Not a fix, a transcription.
                    //GOLDEN BUG PRESERVED: because the counter is a bool and not an
                    //int, it saturates at 1 -- the guard three lines up,
                    //`bQASampleCnt<TestIF_File.iQASamplingCnt`, therefore stops
                    //admitting samples after the FIRST one for any iQASamplingCnt>1,
                    //and never re-arms within a lot.  Golden behaves the same way on
                    //iron; do NOT "repair" it to an int here.
                    bQASampleCnt=true;
                }
            }
        }
    }
}
