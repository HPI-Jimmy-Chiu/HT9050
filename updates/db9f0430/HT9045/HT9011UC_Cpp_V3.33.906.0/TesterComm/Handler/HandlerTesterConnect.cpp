// ===========================================================================
//  TesterComm/Handler/HandlerTesterConnect.cpp -- the gated pieces of golden TfMain::ChangeTesterConnect (GB P2d TODOs
//  D1 / D2 / D3 / D6 / D7) as hook bodies.  AI(W906-D1D7) 20260928 (St02-E).
//
//  Golden 906_0625_Steven main.cpp:12064-12257 TfMain::ChangeTesterConnect (912 main.cpp:12581-12778; the port at
//  forms/fMain.cpp is the 912 body, docs/ST02_GOLDEN906_AUDIT.md row 6).  The P2d ruling (20260926) listed these as
//  "先列待辦"; Steven 20260926 20:37 「待辦裡面，你可以做的就先做吧」.
//
//  Why hooks: forms/fMain.cpp is ht9045_forms (the bottom layer; Jimmy's file).  Each item's golden text stays gated in
//  fMain.cpp as the reference, and one same-line call per item reaches the body here.  The seats are at LogObjects.cpp EOF
//  (ht9045_db, in every link that has ht9045_forms), so this file and its seats build without the fMain.cpp lines.  This
//  file is ht9045_testercomm_handler (wb_serve only); W906_TesterConnectRulesInstall() is called by W906_TesterCommInit
//  BEFORE its HT9045_TESTERCOMM=0 opt-out -- these are Handler rules, not the bridge.  Not installed (every ctest but
//  TesterConnect_Rules) = seat 0 = the port's behaviour before D1-D7.
//
//  Translation: golden text, TfMain members as fMain->X where golden writes them unqualified (none here: golden already
//  writes fMain->ModifyTester / fMain->cbRunStartMode / fMain->SetMainRunStartMode), golden `return 1;` -> `return true;`
//  of the hook (the caller returns 1).  Line numbers below are 906_0625_Steven main.cpp.
//
//    D1 :12072-12074  access check                    W906_CtcD1Access(bRemote)       fMain.cpp: the if around the body
//    D2 :12076-12087  IC-in-machine refusal MES1646   W906_CtcD2IcRefuse(Mode, Msg)   fMain.cpp: first statement inside it
//    D3 :12093-12097  I27 Manual Sort                 W906_CtcD3ManualSort()          fMain.cpp: the if in front of the On-Line else
//    D6 :12144-12152  ON_LINE -> 2D_SORT              W906_CtcD6To2DSort()            fMain.cpp: inside the kept else-if
//    D7 :12212-12237  ASM On-Line arm                 W906_CtcD7AsmOnLine()           fMain.cpp: inside the kept if
//  D4 :12104-12131 is WebLogin.cpp W906_WebLoginForceOperator (the login state lives there).
//
//  Deviation: D7 carries 912's RecordProcess("Silent run mode change ...") (912 :12741-12742, not in 906), as the live
//  Off-Line arm of the same function already does (ST02_GOLDEN906_AUDIT.md row 6: 照 912 live).
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterConnect.h"

#include "LogObjects.h"           // the seats (W906_Ctc*Hook)
#include "MachineType.h"          // SOFT_SIMULTE, CC_*
#include "cmydef.h"               // AccessLevel, iDefEngineerLevel, bOneCycleOperateChangeON_line, bRunManualSortMode, OFF_LINE,
                                  // _2D_SORT, MMSystem, iAutoSiteMapRunStartMode, StartModeName[], rsm*
#include "cprod.h"                // LevelSet, TestIF_File
#include "LastSet.h"              // LastSet.iTester / iRunStartMode
#include "Config.h"               // IniConfig
#include "CosFunction.h"          // CosFunction.bOffLineBin
#include "csystem.h"              // HasICUnderMachine / HasAnyICInMachine / sHasICUnderMachine / sHasAnyICInMachine
#include "canary_support.h"       // ShowErrorMessage, RecordProcess
#include "acatchtray_shims.h"     // NewRecordProcess (with canary_support.h, as forms/fMain.cpp :690-691; cMyDB.h would redeclare RecordProcess's default)
#include "forms/fMain.h"          // fMain->ModifyTester / cbRunStartMode / SetMainRunStartMode
#include "forms/fBinSel.h"        // fBinSel->ReadParam / ReadFile

// ---------------------------------------------------------------------------
// D1  golden main.cpp:12072-12074
// ---------------------------------------------------------------------------
bool W906_CtcD1Access(bool bRemote)
{
    return AccessLevel>=LevelSet.AccessLevel[8] || bRemote==true ||
       ((IniConfig.bI40_bStartProductOnLine && LastSet.iTester==OFF_LINE) &&
       (AccessLevel<iDefEngineerLevel || bOneCycleOperateChangeON_line));       //kevin 20140407 operater 只有ON LINE //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
}

// ---------------------------------------------------------------------------
// D2  golden main.cpp:12076-12087 (golden :12066 `AnsiString S=GrapicPath;` is only this message's buffer: a local here)
// ---------------------------------------------------------------------------
bool W906_CtcD2IcRefuse(int Mode, bool Msg)
{
    #ifndef SOFT_SIMULTE                                                    //Steven 20171214 : 軟體模擬可以任意關Tester
    if(bOneCycleOperateChangeON_line==false &&                              //kevin 20140411
       (HasICUnderMachine() || HasAnyICInMachine()))                        //Steven 20120517 : 有IC不能切換連線模式!!
    {
        if(Mode==10 && Msg==true)
        {
            AnsiString S=AnsiString("ChangeTesterConnect :")+sHasICUnderMachine()+sHasAnyICInMachine();                        //Steven 20250110 : 顯示哪個位置還有IC
            ShowErrorMessage("MES1646", 0, MMSystem, false, S);             //Must finish [Clean out]!!
        }
        return true;                                                        // golden: return 1;
    }
    #else
    (void)Mode;
    (void)Msg;
    #endif
    return false;
}

// ---------------------------------------------------------------------------
// D3  golden main.cpp:12093-12097 (the `else` :12098 is the caller's)
// ---------------------------------------------------------------------------
bool W906_CtcD3ManualSort()
{
    if(IniConfig.bI27_ManualSortMode && bRunManualSortMode==false)
    {
        bRunManualSortMode=true;
        NewRecordProcess("MES2156", "Change To Manual Sort Mode by iTester Button");                        //Steven 20150915 : For TSMC 手動整盤功能
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// D6  golden main.cpp:12144-12152
// ---------------------------------------------------------------------------
void W906_CtcD6To2DSort()
{
    bRunManualSortMode=false;                                       //Steven 20180915 : For TSMC 手動整盤功能
    fMain->ModifyTester(_2D_SORT);                                  //Steven 20191218 : 整合修改LastSet.iTester
    NewRecordProcess("MES2155", "Change to 2D_SORT", "by iTester Button");                                  //ChungHung 20140722 add add record

    if(CosFunction.bOffLineBin)
    {
        fBinSel->ReadParam();
        fBinSel->ReadFile(true, false, "");
    }
}

// ---------------------------------------------------------------------------
// D7  golden main.cpp:12212-12237 (+ 912 :12741-12742, see the banner)
// ---------------------------------------------------------------------------
void W906_CtcD7AsmOnLine()
{
    if(TestIF_File.iTestMode==SingleSite)                       //Steven 20130610 : Single Site不做Auto Site Mapping
    {
        if(iAutoSiteMapRunStartMode==0)                         //Steven 20230410 : Add for Auto site map
        {
            LastSet.iRunStartMode=rsmContinuStart;
            fMain->cbRunStartMode->Text=StartModeName[rsmContinuStart];
        }
        else
        {
            LastSet.iRunStartMode=rsmContinuRetest;
            fMain->cbRunStartMode->Text=StartModeName[rsmContinuRetest];
        }
        //AI(ht9045-v912) 20260921: On/Off Line 切換會靜默改 Start Mode 且不發 MES2107, 補記以利追查 (CASE-FOREHOPE_NINGBO-20260920-001)
        RecordProcess("Silent run mode change by iTester On/Off Line : "+fMain->cbRunStartMode->Text, "ChangeTesterConnect");  //#20 exception (Steven 1003 standing rule): golden 906_0625 main.cpp:12212-12237 / :12239-12257 change the Start Mode on On / Off-Line silently (no record, no MES2107); V912 main.cpp:12741-12742 / :12770-12771 (CASE-FOREHOPE_NINGBO-20260920-001) records it for traceability -- V912 kept (St02-E 1003: trace only, no behaviour change)
    }
    else
    {
        //jou 20200701 : VTEST for auto site mapping cable mount
        if(IniConfig.bVTESTFunction==true)
        {
            if(TestIF_File.bAutoSiteMappingOpenSite)
                fMain->SetMainRunStartMode(rsmAutoSiteMap);
        }
        else
        {
            fMain->SetMainRunStartMode(rsmAutoSiteMap);
        }
    }
}

// ---------------------------------------------------------------------------
void W906_TesterConnectRulesInstall()
{
    W906_CtcD1AccessHook     = &W906_CtcD1Access;
    W906_CtcD2IcRefuseHook   = &W906_CtcD2IcRefuse;
    W906_CtcD3ManualSortHook = &W906_CtcD3ManualSort;
    W906_CtcD6To2DSortHook   = &W906_CtcD6To2DSort;
    W906_CtcD7AsmOnLineHook  = &W906_CtcD7AsmOnLine;
}
