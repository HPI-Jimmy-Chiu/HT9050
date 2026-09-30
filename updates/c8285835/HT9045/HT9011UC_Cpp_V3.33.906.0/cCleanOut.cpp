// =============================================================================
//  cCleanOut.cpp  --  TfMain::BtnCleanOutClick／TfMain::CleanOut（CLEAN OUT：讓機台把料清空）
//
//  AI(W906-CLEANOUT) 20260924: 新檔。golden 對照：HT9011UC_Code_V3.33.906.0_20260618/main.cpp（cp950）
//    TfMain::BtnCleanOutClick  main.cpp:4264-4267（宣告 main.h:914）
//    TfMain::CleanOut          main.cpp:4269-4330（宣告 main.h:1257 `bool __fastcall CleanOut(AnsiString Func);`）
//  取代 forms/fMain.cpp:399／:406 兩個 offline no-op 空殼。本體由腳本從 golden 逐字複製，只改兩處簽名：
//  拿掉 __fastcall；BtnCleanOutClick 的 `TObject *Sender` 沿用本樹既有宣告的 `void *`（forms/fMain.h:231）。
//  兩支是同一條流程：golden BtnCleanOutClick 的全部內容就是 CleanOut("BtnCleanOutClick")。
//
//  為什麼不放 forms/fMain.cpp —— 同 cMainStatus.cpp／cTrayForm.cpp 檔頭的說明：forms/fMain.cpp 編在
//  ht9045_forms，只准連 vclcompat＋ht9045_globals＋ht9045_core；CleanOut 要 InitCleanOutFunction
//  （csystem.cpp:352，ht9045_sm）、MOT[]（ht9045_motor）、EventReport（ht9045_secsgem）。
//  兩支都從 virtual 改成非 virtual（forms/fMain.h:193／:231）：virtual 的本體放進 sm 會讓 forms 的
//  vtable 反過來依賴 sm（分層違規）。全樹沒有任何覆寫（Grep `CleanOut\s*\(\s*(AnsiString|const)` 與
//  `BtnCleanOutClick`，*.cpp/*.h：宣告與定義只在 forms/fMain.h／forms/fMain.cpp）。
//  CleanOut 的回傳型別照 golden 改回 bool —— golden 唯一讀回傳值的呼叫端是 ASE_K Socket/aseTest.cpp:305，
//  本樹沒有這個單元；本樹所有呼叫端都是 `fMain->CleanOut("...");` 敘述句，丟棄回傳值，不受影響。
//
//  ── 它做什麼 ───────────────────────────────────────────────────────────────────
//  只寫全域狀態；本檔與它呼叫的函式都不直接下任何馬達／IO／汽缸命令。真正的清料是之後各狀態機
//  讀到 iCleanOut==1 自己去做的（InArm 不再從 Loader 取料、TrayArm／Loader 把盤退掉…）。
//   本檔直接寫：
//     bCleanoutStart=true            golden :4271-4276（iCleanOut==0 且 one-cycle 進行中／one-cycle 備份）
//     TrayForm.bAutoFeed=true        golden :4278-4282（I40 生產前強制 On line，且 LastSet.iTester==OFF_LINE）
//     IniConfig.bBackUpAutoFeed=TrayForm.bAutoFeed、TrayForm.bAutoFeed=false
//                                    golden :4284-4291（E53 低良率 Auto Clean 且 bLowYieldCleanOut）
//     TrayForm.bAutoFeed=false       golden :4293-4294（bASECleanOutCloseSite）
//     —— 以下只在 iCleanOut==0 時（golden :4312-4328）——
//     InitCleanOutFunction()         → iHome=0、iReset=0、iCleanOut=1、iTrayFeed=0、bCleanoutStart=true
//                                      （csystem.cpp:380-384），加上 AutoSiteMap 分支（csystem.cpp:354-378）
//     iTrayFeedTask=1
//     NewRecordProcess("MES2114","CLEAN OUT pressed",Func) —— 本樹的活本體是空殼（acatchtray_shims.cpp:152），不寫檔
//     bCleanHotplate_ART=2（原本是 1）或 0
//     EventReport(SECS_EVENT.DoCleanOut)（IniConfig.bEnable_SECS_GEM 時）—— 本樹是 Sim 計數器
//                                      （SECSGEM/SecsEventReport.cpp:15-19），不送主機
//   不寫 bBackupCleanOut、SoftStart —— 那是呼叫端的事（例：asendic_Loader.cpp 的 "DoLoad 7" 之後 `bBackupCleanOut=true`）。
//
//  回傳 false（上面 iCleanOut==0 那段整段不做；但前面四段已經寫了）的條件，golden :4296-4310：
//    ((iOneCycle!=0 || BtnOneCycle->Down) && bOneCycle_BackUp==false) || !fAllMotorHome
//    而且 MOT[MMTrayY_Car]／MOT[MMTrayY] 任一 fHasTray，或 MOT[MMPlate1]／MOT[MMPlate2] 任一 HasIC()。
//  ⚠ 這是 golden 的順序（先寫旗標、後判斷要不要 return false），照翻，沒有「修正」。
//
//  零閘：golden 本體用到的每個名字在本樹都有活的定義（逐一以 Grep 量過，見下方 include 的行尾註）。
// =============================================================================
#include "forms/fMain.h"           // TfMain、BtnOneCycle（forms/fMain.h:244，TfMainSpeedButton::Down）
#include "vclcompat/vcl_compat.h"  // AnsiString
#include "cmydef.h"                // iCleanOut(:255)、iOneCycle(:253)、bOneCycle_BackUp(:273)、fAllMotorHome(:222)、
                                   // iTrayFeedTask(:2584)、bCleanoutStart(:3346)、bLowYieldCleanOut(:3671)、
                                   // bCleanHotplate_ART(:3756)、bASECleanOutCloseSite(:4138)、OFF_LINE(:85)、
                                   // MMTrayY(:2255)、MMTrayY_Car(:2256)、MMPlate1(:2257)／MMPlate2
#include "cprod.h"                 // TrayForm（cprod.h:1359，.bAutoFeed :1314）、Prod（cprod.h:1138，.bFailAlarmLowYield_AutoClean :649）
#include "LastSet.h"               // LastSet.iTester（LastSet.h:125）
#include "Config.h"                // IniConfig.bI40_bStartProductOnLine(:811)／bE53LowYieldAutoClean(:621)／bBackUpAutoFeed(:43)／bEnable_SECS_GEM(:92)
#include "csystem.h"               // InitCleanOutFunction（csystem.h:74，本體 csystem.cpp:352）
#include "Motor/mymotor.h"         // MOT[].fHasTray／MOT[].HasIC()
#include "cMyDB.h"                 // NewRecordProcess（cMyDB.h:129；活本體是 acatchtray_shims.cpp:152 的空殼）
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT.DoCleanOut（SecsEventType.h:46）
#include "SECSGEM/SecsEventReport.h" // EventReport(unsigned)（SecsEventReport.h:55）

// =============================================================================
//  golden main.cpp:4264-4330 —— 以下逐字（腳本複製），只改簽名（見檔頭）
// =============================================================================
void TfMain::BtnCleanOutClick(void * /*Sender*/)
{
    CleanOut("BtnCleanOutClick");
}
//------------------------------------------------------------------------------
bool TfMain::CleanOut(AnsiString Func)
{
    if(iCleanOut==0 &&                                                          //ChungHung 20150213 add fix 2x6 if open munt full issue hangup
       (iOneCycle!=0 ||                                                         //kevin 20140218 正在 ONEcycle 無法設定clean out
        bOneCycle_BackUp==true))                                                //JerryYang 20161129 iOneCycle_BackUp改成bool
    {
        bCleanoutStart=true;
    }

    if(IniConfig.bI40_bStartProductOnLine &&                                    //kevin 20180517 change config setup
       LastSet.iTester==OFF_LINE)                                               //kevin 20140407 生產前OP OFF_LINE 強制 On line
    {
        TrayForm.bAutoFeed=true;                                                //kevin 20140412 off_line  load 強制吸完IC
    }

    if(IniConfig.bE53LowYieldAutoClean && Prod.bFailAlarmLowYield_AutoClean)    //wei 20141201 Low Yield Auto Clean(%) start    //kevin 20160802
    {
        if(bLowYieldCleanOut)
        {
            IniConfig.bBackUpAutoFeed=TrayForm.bAutoFeed;                       //kevin 20150424 有改變才需記錄
            TrayForm.bAutoFeed=false;
        }
    }

    if(bASECleanOutCloseSite)                                                   //kevin 20160715 claen out 系統
        TrayForm.bAutoFeed=false;                                               //kevin 20140412 off_line  load 強制吸完IC

    if(((iOneCycle!=0 || BtnOneCycle->Down) &&                                  //ChungHung 20150515 add fix full shuttle hangup
        bOneCycle_BackUp==false) ||                                             //JerryYang 20161129 iOneCycle_BackUp改成bool
        !fAllMotorHome)
    {
        if(MOT[MMTrayY_Car].fHasTray==false &&                                  //Ifor 20251111 add:避免 DoLoad_800-5 一直報警無法消除問題
           MOT[MMTrayY].fHasTray==false &&                                      // no any tray
           MOT[MMPlate1].HasIC()==false &&                                      //KevinCheng 20260529 : Loader無盤時 CleanOut無法OneCycle
           MOT[MMPlate2].HasIC()==false)
        {
        }
        else
        {
            return false;                                                       //kevin 20141108
        }
    }

    if(iCleanOut==0)
    {
        InitCleanOutFunction();
        iTrayFeedTask=1;
        NewRecordProcess("MES2114", "CLEAN OUT pressed", Func);
        if(bCleanHotplate_ART==1)                                               //kevin 20150722
        {
            bCleanHotplate_ART=2;
        }
        else
        {
            bCleanHotplate_ART=0;                                               //kevin 20150722
        }

        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoCleanOut);
    }
    return true;                                                                //kevin 20141108
}
//------------------------------------------------------------------------------

// =============================================================================
//  AI(W906-AMB-L2) 20260929: TfMain::InitialTrayFeedTask -- golden main.cpp:2317-2377
//    (declaration golden main.h:1245 `void InitialTrayFeedTask(AnsiString Func);`; port declaration forms/fMain.h:552).
//  RULINGS_20260929 #11.  It was translated nowhere in the port: csystem.cpp:4049's W7C2_FMAIN_INITIALTRAYFEED
//  was a no-op macro, so a one-cycle finish with bNeedTrayFeed never started the tray feed.
//  The body below is copied verbatim from golden by script (cp950 -> UTF-8, strict iconv clean); the signature is
//  golden's own (golden has no __fastcall on this one).
//
//  Why here and not forms/ or FileRW/Main*.cpp -- the same layering reason as TfMain::CleanOut above (file head):
//  forms/fMain.cpp is ht9045_forms, which cannot see InArmSuck / MOT[] / ShuttleHasIC / SetRunStartMode, and
//  FileRW/Main*.cpp is compiled into wb_serve only (CMakeLists.txt:3437, ${W906_FILERW_SRC}), while the caller
//  csystem.cpp is ht9045_sm (CMakeLists.txt:2451), which other executables link without FileRW (undefined reference).
//  This file is already the ht9045_sm home of the sibling operator action (CLEAN OUT), so no CMakeLists change.
//  Non-virtual for the same reason as CleanOut (forms/fMain.h:193); nothing in the tree overrides it.
//
//  What it does: it only writes global state -- no motor / IO / cylinder command.  With the guards passing it sets
//  iHome=0 iReset=0 iCleanOut=0 iTrayFeed=1 iTrayFeedTask=1 (or, for E39 "check hot plate after clean out", arms
//  the hot-plate check flags and switches to Continue Start instead).  MainProc's ladder then runs
//  DoTrayFeedProcess() -> DoTrayFeed() (csystem.cpp:31821-32040 -> csystem.cpp:11869) on the next ticks.
//  NewRecordProcess("MES2118", ...) is the golden body, live in cMyDB.cpp since W906-CMYDB-P4 (cMyDB.h:129).
//
//  Golden callers (906 golden) and where they are in the port:
//    csystem.cpp:13982/13984  DoOneCycleFinishCheck, if(bNeedTrayFeed)  -> port csystem.cpp:5421/5423 through
//                             W7C2_FMAIN_INITIALTRAYFEED, which now forwards to this member (csystem.cpp:4049).
//    main.cpp:2624            TfMain::ScanKey, panel key SnFKTrayFeed   -> TfMain::ScanKey is not translated in the
//                             port (no call site exists), so nothing to wire.
//    main.cpp:13946           TfMain::BtnTrayEndClick (operator TRAY FEED button) -> FLOW-4, deliberately NOT wired
//                             here: forms/fMain.cpp's BtnTrayEndClick is still a call-count seam.  Its golden body is
//                             exactly `InitialTrayFeedTask("BtnTrayEndClick");` (golden main.cpp:13944-13947); the web
//                             "trayFeed" control event (FileRW/MainClick.cpp:1636) should call that.
//
//  Zero gates: every name the golden body uses has a live definition in the port (measured with git grep):
// =============================================================================
#include "mykitsuck.h"            // InArmSuck / OutArmSuck (mykitsuck.h:452 / :460), TMyKitSuck::HasIC (:396)
#include "aHotPlateSubstrate.h"   // SetRunStartMode (aHotPlateSubstrate.h:945, golden signature; body RunStartMode.cpp)
#include "MachineType.h"          // rsmContinuStart / rsmAutoSiteMap (MachineType.h:724 / :728), CC_TSMC_TAINAN (:280)
                                  // from cmydef.h (included at the top): SystemStart (:221) fAllMotorHome (:222)
                                  //   iOneCycle (:253) iCleanOut (:255) iTrayFeed (:257) iHome (:260) iReset (:261)
                                  //   MTrayX (:2132) MInRotateKit / MOutRotateKit (:2144 / :2145) iTrayFeedTask (:2584)
                                  //   bNeedTrySuckHotPlate (:2704) bOneTimeHotPlateCheckAll (:2707)
                                  //   bTryPickFromHotPlateShowError (:2708) bHotPlateCheckNeedTrayFeed (:2709)
                                  //   Tempture_Hot (:2760) bCanUseHotPlateCheck (:3347) bASMFinishOneCycle (:3398)
                                  //   bHPCleanout (:4137) iHPShuttle (:4138) CUSTOMER_CODE
                                  // from csystem.h: ShuttleHasIC (:135) IndexHasIC (:140)
                                  // from Config.h: IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed (:603)
                                  // from LastSet.h: LastSet.iTemperature (:123) LastSet.iRunStartMode (:366)
                                  // from Motor/mymotor.h: MOT[].HasIC() / MOT[].fHasTray; from cMyDB.h: NewRecordProcess

// =============================================================================
//  golden main.cpp:2317-2377 -- verbatim (script copy) below
// =============================================================================
void TfMain::InitialTrayFeedTask(AnsiString Func)
{
    if(SystemStart && bASMFinishOneCycle==false)                                //Steven 20120907 : Auto Site Mapping的時候要跳過
    {
        return;
    }

    if(iOneCycle!=0 || !fAllMotorHome)
    {                                                                           //jou 981125 start : Clean Out時,按tray feed,hang up error
        return;
    }

    if(InArmSuck.HasIC() || OutArmSuck.HasIC()  ||
       ShuttleHasIC()    || IndexHasIC()  ||
       MOT[MInRotateKit].HasIC() || MOT[MOutRotateKit].HasIC() ||               //Steven 20140107
       MOT[MTrayX].fHasTray==true ||                                            //Steven 20260204 : Tray Arm need to finish then can tray feed
       iCleanOut==1)
    {
        return;
    }

    NewRecordProcess("MES2118", "TRAY FEED pressed", Func);

    if(LastSet.iRunStartMode==rsmAutoSiteMap)                                   //Steven 20140107 : Auto Site Mapping "Remove Loader Tray Manually"不勾會Hang Up
    {
        iHome=0;
        iReset=0;
        iCleanOut=0;
        iTrayFeed=1;
        iTrayFeedTask=1;
    }
    else if(IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed &&        //ChungHung 20120206 Hotplate check  是否開啟Tray Feed後 HotPlate Check
            LastSet.iTemperature==Tempture_Hot &&
            bCanUseHotPlateCheck==true   &&
            MOT[MMPlate1].HasIC()==false &&
            MOT[MMPlate2].HasIC()==false)                                       //Steven 20140107 : 更換
    {
        bNeedTrySuckHotPlate            =true;                                  //ChungHung 20120206 Hotplate check
        bOneTimeHotPlateCheckAll        =true;                                  //ChungHung 20120206 Hotplate check
        bTryPickFromHotPlateShowError   =false;                                 //ChungHung 20120206 Hotplate check
        bHotPlateCheckNeedTrayFeed      =true;
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //wei 20160624 Hotplate clean out
        {
            bHPCleanout                 =true;
            iHPShuttle                  =0;
        }

        if(LastSet.iRunStartMode!=rsmContinuStart)
        {
            SetRunStartMode(rsmContinuStart);
        }
    }
    else
    {
        iHome=0;
        iReset=0;
        iCleanOut=0;
        iTrayFeed=1;
        iTrayFeedTask=1;
    }
}

// =============================================================================
//  AI(W906-FLOW-4) 20260930: the main screen's ONE CYCLE / TRAY FEED / ALARM RESET buttons (INBOX 109 FLOW-4).
//    golden 906 (HT9011UC_Code_V3.33.906.0_20260618, cp950):
//      TfMain::BtnOneCycleClick    main.cpp:4332-4380    (main.h:915; main.dfm:10694 BtnOneCycle OnClick)
//      TfMain::BtnTrayEndClick     main.cpp:13944-13947  (main.h:925; main.dfm:10759 BtnTrayEnd OnClick)
//      TfMain::BtnAlarmResetClick  main.cpp:22159-22166  (main.h:934; main.dfm:10737 BtnAlarmReset OnClick)
//    The bodies below are copied by script (cp950 strict -> UTF-8); only the signatures change (no __fastcall, `void *`
//    Sender like the rest of forms/fMain.h), plus the one gate and the one [W906] counter line marked in place.
//    Same file and layering reason as CleanOut (file head): forms/fMain.cpp is ht9045_forms and cannot see InitOneCycle
//    (csystem.cpp:391), InitialTrayFeedTask (above) or EventReport.  The three are non-virtual (forms/fMain.h:229 / :551),
//    as CleanOut is; nothing in the tree overrides them.
//
//  Callers:
//    * the web buttons -- FileRW/MainClick.cpp (St01, S169) queues one event per click (act.main.ctlButton; a disabled
//      click is refused there by its CtlBtnEnabled = golden ProcessKeyFlush); WebMainCtlButtons.cpp takes them on every
//      pass of wb_serve's main loop and calls these members.
//    * the engines that used to call the offline no-ops: BtnOneCycleClick has ~20 call sites (acarry.cpp, atester*.cpp,
//      atester_ProcessCount.cpp, AutoClean/AutoClean.cpp, asendic_Loader.cpp, SECSGEM/uHGemHT9045.cpp S2F41 ONE_CYCLE,
//      TesterComm/Handler/HandlerGpibMsg.cpp GPIB ONECYCLE); BtnTrayEndClick the two S2F41 TRAY_FEED sites
//      (SECSGEM/uHGemHT9045.cpp:5521 / :5537).  They now run golden's body, as golden does.
//
//  No wait on the tick thread (checked before wiring -- a body run from wb_serve's loop must not block it):
//    BtnOneCycleClick: flags, InitOneCycle (flags + RecordProcess), NewRecordProcess (a cMyDB log row), EventReport
//      (SECSGEM/SecsEventReport.cpp, a counter).  No dialog, no sleep, no motion command.
//    BtnTrayEndClick -> InitialTrayFeedTask (above): flags; its E39 hot-plate branch calls SetRunStartMode(rsmContinuStart)
//      (RunStartMode.cpp:114-928), whose only dialog (:526 ShowMyMessage "No Run ART Mode") sits on the rsmInitial_ART +
//      CC_KYEC_LEE branch that rsmContinuStart never takes.
//    BtnAlarmResetClick: NewRecordProcess, EventReport, RespondASECom (live body canary_support.cpp:243 returns false -- its
//      ASE send is still #if 0; the cpublic.cpp:768 copy is inside that file's #if 0 gate opened at :709).
//
//  906 vs V912 (V912 main.cpp): V912 puts an ATK AMR "Fix-Full swap running" refusal at the head of BtnOneCycleClick
//    (:4471-4475) and of InitialTrayFeedTask (:2387-2391, RogerYang 20260823).  906 has neither; this tree follows 906.
//    BtnTrayEndClick (V912 :14467-14470) and BtnAlarmResetClick (V912 :22867-22874) are the same in both.
// =============================================================================
#include "cpublic.h"               // RespondASECom (cpublic.h:41; live body canary_support.cpp:243)
                                   // from the includes at the top of this file: bSECSGEMAlarm (cmydef.h:3761)
                                   //   bEnableEmployeeIDCheck (:3892) bCanUseHotPlateCheck (:3347) bNeedTrySuckHotPlate (:2704)
                                   //   bOneTimeHotPlateCheckAll (:2707) bNeedOneCycleByYieldAlm (:4086) bDoEmptySocketOneCycle (:4981)
                                   //   bIsAutoOneCycle (:2801) bManualOneCycle (:2803) bIsAvoidIndexShiftOneCycle (:2804);
                                   //   TestIF_File.bAutoOnecycleHomStart (cprod.h:2404); IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed
                                   //   (Config.h:603) / bO01_ResetNeedClearAndCheckHP (:1327); InitOneCycle (csystem.h:88);
                                   //   SECS_EVENT.DoOneCycle / DoAlarmReset (SECSGEM/SecsEventType.h:45 / :72); RecordProcess /
                                   //   NewRecordProcess (cMyDB.h:130 / :129)

// =============================================================================
//  golden main.cpp:4332-4380 -- verbatim (script copy) below, with the one GATE marked in place
// =============================================================================
void TfMain::BtnOneCycleClick(void * /*Sender*/)
{
    if(fAllMotorHome==false || iOneCycle)
        return;
    if(bSECSGEMAlarm==true)                                                     //Ifor 20151210 鎖定面板
        return;
    if(bEnableEmployeeIDCheck==true)                                            //Ifor 20180911 (Steven) : Add 啟動 Employee ID Check
        return;

    if((IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed ||
        IniConfig.bO01_ResetNeedClearAndCheckHP) &&                             //jou 2012-05-08 在做Hotplate check的時候不給做one cycle,會出現index error
       LastSet.iTemperature==Tempture_Hot && bCanUseHotPlateCheck &&            //ChungHung 20120813 add Reset Hotplate Check 不給one cycle
       bNeedTrySuckHotPlate==true && bOneTimeHotPlateCheckAll==true)            //ChungHung 20120206 Hotplate check
    {
        return;
    }

    // GATE (W906-FLOW-4-LOWYIELD): missing dependency -- TfMain has no `TStringList *slLowYieldAlarm` (golden main.h:1487).
    //   Same member as the gate at atester_ProcessCount.cpp:343 (TODO(W6.4b-integrate): the list's only golden writer,
    //   golden atester_ProcessCount.cpp:295-298) and csystem.cpp:4016 (a TU-local stand-in that nothing fills).  Nothing
    //   in the port can add to the list, so it is empty, Count==0 and this if is never taken -- which is what the gate does.
    //   UN-GATE together with those two once the member exists.
#if 0 // GATE (W906-FLOW-4-LOWYIELD)
    if(CosFunction.bYieldAlmNeedOneCycle &&
       fMain->slLowYieldAlarm->Count!=0)
    {
        bNeedOneCycleByYieldAlm=true;
    }
#endif

    if(BtnOneCycle->Down==false)
    {
        if(bDoEmptySocketOneCycle)
        {
            InitOneCycle("Empty Socket Check...");
        }
        else if(bIsAutoOneCycle)
        {
            InitOneCycle("Auto Clean...");                                      //Steven 20130618 : 紀錄Auto Clean動作
        }
        else
        {
            InitOneCycle("BtnOneCycleClick");                                   //Steven 20220325 : 一定要執行這個
        }

        if(TestIF_File.bAutoOnecycleHomStart && bIsAvoidIndexShiftOneCycle)     //Isaac 20210821 : 全智要求定時onecycle回home,start
        {
            RecordProcess("Start ONE CYCLE by Auto OneCycle and Home and Start...");
        }
        BtnOneCycle->Down=true;
        NewRecordProcess("MES2115", "ONE CYCLE pressed", "BtnOneCycleClick");
        bManualOneCycle=true;                                                   //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 未完成時又觸發 AutoClean 動作，AutoClean 做完並不會提示 OneCycle Finish
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoOneCycle);
    }
}
//------------------------------------------------------------------------------

// =============================================================================
//  golden main.cpp:13944-13947 -- verbatim (script copy) below, plus the [W906] counter line
// =============================================================================
void TfMain::BtnTrayEndClick(void * /*Sender*/)
{
    W906_BtnTrayEndClickCallCount++;                                            // [W906] port-only call counter (forms/fMain.h:556), kept from the old seam
    InitialTrayFeedTask("BtnTrayEndClick");
}
//------------------------------------------------------------------------------

// =============================================================================
//  golden main.cpp:22159-22166 -- verbatim (script copy) below
// =============================================================================
void TfMain::BtnAlarmResetClick(void * /*Sender*/)
{
    NewRecordProcess("MES2116", "ALARM RESET pressed", "BtnAlarmResetClick");
    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.DoAlarmReset);

    RespondASECom("@e02111Done");                                               //kevin 20150415 回應 ase Alarmreset finish
}
