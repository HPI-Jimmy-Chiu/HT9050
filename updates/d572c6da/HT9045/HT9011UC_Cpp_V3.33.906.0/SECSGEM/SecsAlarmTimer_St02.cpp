// =============================================================================
//  SecsAlarmTimer_St02.cpp -- POOL-15 (SECS-S10F3-913): the SECS/GEM host terminal messages shown to the operator per golden 913.
//  AI(W906-POOL15) 20261010 (St02).  Claim: docs/handoff/ST02_POOL15_CLAIM_20261010.md (v906/steven-handoff 55adb599).
//  golden 913 = D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven (cp950).  Laptop TO_STEVEN s4 20261010 16:3x / POOL.md POOL-15:
//  follow 913 (RULINGS_20261008 #3), not 0618's ShowMyMessage; the window is a screen whose open / closed state belongs to the
//  browser (no C++ fShow stub); the queue-draining half is C++; the display goes through the existing web message box and never
//  waits on the tick thread; L12 (no X, Alt+F4 refused) is done too.
//
//  WHAT IT DOES.  The host's terminal messages are queued in HSys.MyGem->SecsAlarmMessage by S10F3 (SECSGEM/uHGemClass.cpp:3516 /
//  :3521), S10F5 (:3634), S2F41 HOST_ALARM_DESCRIPTION (SECSGEM/uHGemHT9045.cpp:4797) and the RunCheck failure (:7849); nobody took
//  them out.  Once a second (golden TTimer) the drain takes one, locks the panel keys (bSECSGEMAlarm), stops the machine and shows
//  it; OK on the box releases it (EventReport CEID 73, keys unlocked).
//
//  ⚠ NO MACHINE CHANGE TODAY.  wb_serve has no HT9045Gem (HSys.MyGem is built only by SystemModularInitial, database.cpp:189, which
//  wb_serve never calls -- BinDisplay/BinDispBringUp_St02.cpp:593-594), and the S10F3 / S10F5 handlers queue only when
//  HGemPtr->TerminalDisplayIndex != 0 (uHGemClass.cpp:3489 / :3614), which only the untranslated TFSECS::FormCreate SetTerminalWindows
//  sets (golden 913 UsecegemMainFrom.cpp:626).  The drain returns at once on a NULL HSys.MyGem; ctest drives it with its own HTGem.
//  HT9050 runs with SECS off.
//
//  GOLDEN MAP (golden 913)
//    SECSGEM/UsecegemMainFrom.cpp:1023-1110  TFSECS::TimerSecsAlarmTimer            -> W906_TimerSecsAlarmTimer_St02
//    SECSGEM/UsecegemMainFrom.dfm:10977-10982 TimerSecsAlarm (Enabled False, 1000 ms) -> timer table entry "FSECS.TimerSecsAlarm"
//    SECSGEM/UsecegemMainFrom.cpp:150 / :967  FormCreate on / FormDestroy off        -> W906_SecsAlarmHostInstall_St02 (post != 0 / 0)
//    mymessbox.cpp:1565-1630                  ShowSecsAlarmMessage                   -> W906_ShowSecsAlarmMessage_St02
//    mymessbox.cpp:1455-1508                  TSecsAlarmForm::btnOKClick             -> W906_SecsAlarmAnswer_St02 (OK / PAUSE)
//    mymessbox.cpp:1510-1553                  TSecsAlarmForm::DoReleaseAndHide       -> DoReleaseAndHide below
//    mymessbox.cpp:1415-1416 / :1560-1563     L12: BorderIcons empty, CanClose=false -> only W906_SecsAlarmAnswer_St02's OK releases
//                                                                                       (the web box has no X and swallows Esc:
//                                                                                       web/page/dialog-bridge.js:139-142 / :173-175,
//                                                                                       web/page/ht9045_modal.js:160-161 / :179-182)
//  0618 (UsecegemMainFrom.cpp:1023-1103) differs: MyMessageBox->fShow stood for "window open" and ShowMyMessage(str) showed it; 913
//  adds the MyMessageBox guard (:1039-1043, RogerYang 0923 "restore 910's guard") and the separate window.
//
//  [W906] PORT NOTES
//    P1  `HSys.MyGem==NULL` (or its queue NULL) -> return, right after the MyMessageBox guard.  golden always has the object; the
//        port's wb_serve does not (banner above).
//    P2  golden `fSecsAlarm && fSecsAlarm->Visible` -> W906_SecsAlarmShown_St02(): a SECS box is on the mailbox and not answered.
//        No page-table row in this card (claim s7 Q2 = A); the token is not written in live code (tests/test_h013_terms.cpp K-pins).
//    P3  the window = the web message box.  post() puts the memo text (golden moSecs->Lines joined with CRLF) and lblChinese (S2) on
//        tools/wb_serve.cpp's show-my-message mailbox as a non-blocking, machine-stopping box with OK and Alarm Reset; with
//        bSECSGEMAlarm already true the page draws the SECS look (web/page/dialog-page.js:159-171).  "BringToFront" (an appended
//        line) = post again.
//    P4  REPOST (not golden).  The mailbox has one slot: a ShowMyMessage / YES-NO / Unloader box replaces ours and the page drops
//        the SECS box while C++ still holds the key lock -- golden 913's modeless window stays underneath.  Every tick, before the
//        golden body: the SECS box is up, no MyMessageBox is shown and the mailbox no longer holds our request (isOurs) -> post
//        again with a NEW requestId (the page remembers the ones it has seen).  A failed post is retried the same way.  An answer
//        to an earlier requestId of the same window is refused with "no query pending" so the page drops that stale box.
//    P5  retire only while the mailbox still holds our request -- an unconditional retire would wipe another box the operator is
//        looking at (a blocking ShowMyMessage would then wait for an answer nobody can give).
//    P6  customer branches whose dependencies are not on the port are kept verbatim under #if 0 (RULINGS_20260925 #25):
//        OEE ACM_WriteMsgAndCallExe (no TfProductionInfo member), PANTHER machineTime and RENESAS FTCTManStartUnlock (not on the
//        fMain facade), TfProductionInfo::bFTPError (as forms/fNote_ShowError.cpp:1072), btnOKClick's RENESAS tail (bRenesasFTCTAlarm
//        is defined only inside cmydef.cpp's #if 0, :6286-6300), and btnOKClick's N07_5 password /
//        employee-ID / KYEC barcode checks (as WebLogin.cpp:3000-3001; St01 docs/D034_PLAN_20261002.md is where they belong).
//        MTI / PTI's fAutomation->DoCommandBuffer stays live (atester_shims.h TfAutomationShim, a no-op today).
//    P7  left out: bNeedBigMsg (:1513 -- its definition is still inside cmydef.cpp's #if 0, :6377-6380), iValue (:1500 -- the
//        MyMessageBox's, tools/wb_serve.cpp s_iValue), palWaitEAP (the KYEC employee-ID EAP wait, D-034), CloseSecsAlarmForm
//        (:1632-1638, called only from that EAP code), and Alarm Reset (golden :1555-1558 -> pnlAlarmResetClick): the page posts
//        HT_DIALOG_EVENT for it and nothing forwards that, so only OK reaches C++ (claim s7 Q4).
// =============================================================================
#include "MachineType.h"              // first: vclcompat + windows.h (techniques s5); CC_MAXIM_THAILAND / CC_KYEC_LEE / CC_MTI / CC_PTI
#include "cmydef.h"                   // InitialOK, SystemInitialOK, SystemStart, SoftStart, CUSTOMER_CODE, iUnLoaderCount, bSECSGEMAlarm, ...
#include "Config.h"                   // IniConfig.bEnable_SECS_GEM
#include "cprod.h"                    // TestIF_File, Prod
#include "cMyDB.h"                    // MyDBIProcess / RecordProcess -- NOT canary_support.h in the same TU (cMyDB.h:82-89)
#include "mysensor.h"                 // Sen[] (SnSafeDoor3)
#include "mymessbox_shim.h"           // MyMessageBox (fShow)
#include "W906FormShowing.h"          // W906_FormShowing
#include "BarcodeReader.h"            // FormBarcodeReader (bShow / Close)
#include "database.h"                 // HSys (SYSTEM_MODULAR::MyGem)
#include "SECSGEM/uHGemClass.h"       // HTGem::SecsAlarmMessage
#include "SECSGEM/SecsEventType.h"    // SECS_EVENT.MymessboxOK / DoPause
#include "SECSGEM/SecsEventReport.h"  // EventReport
#include "Interface/InterfaceSYS.h"   // SendCommand_ESD / ESD_SYSTEM_STOP
#include "Motor/myGALILmotor.h"       // StopAllMotor(bool bIndexCanStop=true) -- golden mymessbox.cpp's StopAllMotor()
#include "atester_shims.h"            // fAutomation (TfAutomationShim::DoCommandBuffer)
#include "TimerTable.h"               // ht9045::TTimerEntry
#include "SECSGEM/SecsAlarmTimer_St02.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

extern bool       bHangTimePause;          // atester.cpp:183 (golden mymessbox.cpp:48 extern)
extern bool       bSupplyNewICTrayPause;   // asendic_Loader.cpp:326 (golden mymessbox.cpp:49)
extern TQPF_Timer tGalilTwoYMoveDelay;     // Motor/myGALILmotor.cpp:5432 (golden mymessbox.cpp:384 extern)
extern TQPF_Timer hAutoCleanHangUp;        // csystem.h:291

namespace {
W906SecsPostFn_St02   g_post   = 0;
W906SecsIsOursFn_St02 g_isOurs = 0;
W906SecsRetireFn_St02 g_retire = 0;
ht9045::TTimerEntry*  g_entry  = 0;
bool                     g_visible = false;              // golden 913 fSecsAlarm->Visible (P2)
std::string              g_qid;                          // the request the mailbox should hold; "" = not posted (P4)
std::vector<std::string> g_oldQids;                      // earlier requests of this same window (P4)
std::vector<AnsiString>  g_lines;                        // golden 913 moSecs->Lines
AnsiString               g_chinese;                      // golden 913 lblChinese->Caption
bool                     g_bDisableAlarmBuzzer = false;  // golden 913 TSecsAlarmForm::bDisableAlarmBuzzer (ctor :1451; nothing sets it)

AnsiString MemoText()
{
    AnsiString s;
    for (size_t i = 0; i < g_lines.size(); ++i)
    {
        if (i) s += "\r\n";
        s += g_lines[i];
    }
    return s;
}

// P3 / P4: put the window on the mailbox (a new requestId every time).
void Post()
{
    if (g_post == 0)
        return;
    const std::string q = g_post(MemoText().c_str(), g_chinese.c_str());
    if (q.empty())
    {
        std::printf("  [SECS host message] mailbox post failed -- retried on the next tick\n");
        return;
    }
    if (!g_qid.empty() && g_qid != q)
    {
        g_oldQids.push_back(g_qid);
        if (g_oldQids.size() > 32) g_oldQids.erase(g_oldQids.begin());
    }
    g_qid = q;
}

// P5: golden fSecsAlarm->Hide() -- off the screen; the mailbox goes idle only while it still holds ours.
void Hide()
{
    if (!g_qid.empty())
    {
        if (g_retire != 0 && (g_isOurs == 0 || g_isOurs(g_qid.c_str())))
            g_retire(g_qid.c_str());
        g_oldQids.push_back(g_qid);
        if (g_oldQids.size() > 32) g_oldQids.erase(g_oldQids.begin());
        g_qid.clear();
    }
    g_visible = false;
}

// P4: the web upkeep, every tick before the golden body.
void KeepShown()
{
    if (!g_visible || g_post == 0)
        return;
    if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true)   // a MyMessageBox owns the mailbox; ours comes back after it
        return;
    if (!g_qid.empty() && (g_isOurs == 0 || g_isOurs(g_qid.c_str())))
        return;                                                           // still on the page (or no way to tell)
    std::printf("  [SECS host message] %s no longer on the mailbox -- posted again\n", g_qid.empty() ? "(not posted)" : g_qid.c_str());
    Post();
}

void St02OnTimerSecsAlarm()
{
    KeepShown();
    W906_TimerSecsAlarmTimer_St02();
}

// golden 913 mymessbox.cpp:1510 `void __fastcall TSecsAlarmForm::DoReleaseAndHide()`
void DoReleaseAndHide()
{
    // ---- from FormClose 388-442 ----
    // bNeedBigMsg = false;                             // 388   [W906] P7: definition still inside cmydef.cpp's #if 0 (:6377-6380)
    if(!iUnLoaderCount)                                 // 389-400   // golden 913 mymessbox.cpp:1514-1523
    {
        if(Sen[SnSafeDoor3].Enable==false)
        {
            bIsTestSitICFallDown = false;
            bContactCTOverCHK    = false;
        }
    }
    else
        iUnLoaderCount = 0;

#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1525-1526 -- TfProductionInfo has no bFTPError (forms/fProductionInfo.h; bFTPError is FormHS's, forms/fHS.h:665), as forms/fNote_ShowError.cpp:1072
    if(CosFunction.bOEEFunction)                     // 404-407
        fProductionInfo->bFTPError = false;
#endif

    if(IniConfig.bEnable_SECS_GEM==true)                // 411-423   // golden 913 mymessbox.cpp:1528-1538
    {
        if(bSECSGEMAlarm)
        {
            EventReport(SECS_EVENT.MymessboxOK);        // 73
            bSECSGEMAlarm = false;
            g_lines.clear();                            // moSecs->Clear();
        }
        else
            EventReport(SECS_EVENT.DoPause);            // 2
    }

    SendCommand_ESD(ESD_SYSTEM_STOP);                   // 430   // golden 913 mymessbox.cpp:1540-1547
    bEnterTestIF = true;                                // 431
    bAlarmReset  = false;                               // 432
    lHandlerStopTime.LatchCycleTime(true);              // 434
    bHasQwertyKeyForm = false;                          // 439
    bHasPasswordForm  = false;                          // 440
    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);  // 441
    tGalilTwoYMoveDelay.SetSecAndOn(60);                // 442

    // palWaitEAP->Visible = false;                     // (取代舊 fShow=false)   [W906] P7: no EAP panel   // golden 913 mymessbox.cpp:1549
    Hide();                                             // this->Hide();   [W906] P5   // golden 913 mymessbox.cpp:1550
    bAlarmBuzzer = false;                               // golden 913 mymessbox.cpp:1551
    RecordProcess("SECS alarm window released");        //AI(ht9045-secs-sem) 20260923 (RogerYang) : 原本兩條關閉路徑都不留 log, 事後無法分辨是按 OK 還是被繞過   // golden 913 mymessbox.cpp:1552
}
}  // namespace

// =============================================================================
//  golden 913 SECSGEM/UsecegemMainFrom.cpp:1023-1110 (POOL-15)
// =============================================================================
void W906_TimerSecsAlarmTimer_St02()                                            //Steven 20150519   // golden 913 UsecegemMainFrom.cpp:1023 TFSECS::TimerSecsAlarmTimer
{
    static bool bRun=false;

    AnsiString str;
    if(InitialOK==false)
    {
        return;
    }

    if(bRun==true)
    {
        return;
    }
    bRun=true;

    if(W906_FormShowing("MyMessageBox", MyMessageBox->fShow)==true)            //AI(ht9045-secs-sem) 20260923 (RogerYang) : 回復 V3.33.910.0 的守門; MyMessageBox 是 ShowModal, 此時不 drain, 訊息留在佇列等它關掉   // golden 913 :1039
    {
        bRun=false;
        return;
    }

    if(HSys.MyGem==NULL || HSys.MyGem->SecsAlarmMessage==NULL)                  // [W906] P1: wb_serve has no HT9045Gem today
    {
        bRun=false;
        return;
    }

    if(W906_SecsAlarmShown_St02())                                              //RogerYang 20260724 : Add for new S10F3 SECS alarm   // golden 913 :1045 ([W906] P2)
    {
        if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                                    //Ifor 20251018 有新資料需更新
        {
        }
        else if(CUSTOMER_CODE==CC_KYEC_LEE)                                     //Ifor 20170616 (wei) KYEC add  PreAlarm時不Return
        {
            if(iUnLoaderCount==0)
            {
                bRun=false;
                return;
            }
        }
        else
        {
            bRun=false;
            return;
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20170425 add KYEC 要求 SECS GEM Message 要在最上層   // golden 913 :1065
    {
        if(HSys.MyGem->SecsAlarmMessage->Count==0)
        {
            bRun=false;
            return;
        }
    }
//    else                                                                      //JerryYang 20170907 (wei) Mark掉,為了讓note及message能同時show
//    {
//        if(fNote->fShow==true)                                                //JerryYang 20161213 先處理完note才show，避免Note及message同時showmodal可能會畫面卡死
//        {
//            return;
//        }
//    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && W906_FormShowing("FormBarcodeReader", FormBarcodeReader->bShow))   //Ifor 20170616 (wei) KYEC add 避免BarCode From開啟時 Show SECSGEM Alrm 卡死   // golden 913 :1081
    {
        FormBarcodeReader->Close();
        bRun=false;
        return;
    }

    if(HSys.MyGem->SecsAlarmMessage->Count!=0)                                  // golden 913 :1088
    {
        if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                                    //Ifor 20251018 add:收到新的SECS Message 需關閉後重新顯示訊息
        {
            if(W906_SecsAlarmShown_St02())                                      //RogerYang 20260724 : SECS alarm   ([W906] P2)
            {
                Hide();                                                         //RogerYang 20260724 : SECS alarm   ([W906] P5)
                bRun=false;
                return;
            }
        }

        if(bAlarmAfterPreAlarm==false)                                          //Ifor 20170906 (wei) add 避免 PreAlarm -> Alarm -> SECS GEM Alarm 同時發生造成當機問題
        {
            bSECSGEMAlarm=true;                                                 //wei 20150817 S10F3 Alarm Reset畫面
            str=HSys.MyGem->SecsAlarmMessage->Strings[0];                       //Ifor 20251018 add:取出資料並刪除避免Secs Alarm Message Count 異常
            HSys.MyGem->SecsAlarmMessage->Delete(0);
            //ShowMyMessage(str);                                                 //HSys.MyGem->SecsAlarmMessage->Strings[0]);     //Steven 20251204 : fixed for SECS GEM
            W906_ShowSecsAlarmMessage_St02(str);                                //RogerYang 20260724 : Add for new S10F3 SECS alarm
        }
    }
    bRun=false;
}

// =============================================================================
//  golden 913 mymessbox.cpp:1565-1630 (POOL-15)
// =============================================================================
void W906_ShowSecsAlarmMessage_St02(AnsiString S1, AnsiString S2)              // golden 913 mymessbox.cpp:1565 ShowSecsAlarmMessage
{
    if(InitialOK==false) { MyDBIProcess("Exception", S1); return; }

    // ---- machine-state on show (ShowMyMessage 792/827 + FormShow 303-343) ----
    bHandlerPause      = true;                       // SMM 792
    iHandlerStartCount = 0;                          // SMM 793
    if(!iUnLoaderCount)                              // FS 303-311
    {
        SystemStart = false;
        SoftStart   = false;
        bSupplyNewICTrayPause = true;
        bHangTimePause        = true;
    }
    if(SystemInitialOK==true) StopAllMotor();        // SMM 827
    for(int i=0; i<7; i++)                           // FS 339-343
    {
        bLifterPause[i] = true;
        bAuto2Pause[i]  = true;
    }

    // if(!fSecsAlarm) fSecsAlarm = new TSecsAlarmForm(Application);   [W906] P3: no C++ form; the window is the web box   // golden 913 :1586

    bAlarmBuzzer = !g_bDisableAlarmBuzzer;           // FS 337 (buzzer honor disable)
    MyDBIProcess("Message", S1);

    // ---- automation / OEE / CCD / PANTHER (FS 347-382) : customer-guarded ----
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
        fAutomation->DoCommandBuffer("MESSAGE_REQUEST", "", S1, 0, "");
#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1594-1599 -- TfProductionInfo has no ACM_WriteMsgAndCallExe (forms/fProductionInfo.h)
    if(CosFunction.bOEEFunction && IniConfig.bN14_14_AlarmCtrlMachine)
    {
        AnsiString sMessage;
        sMessage.sprintf("Alarm,Message,%s", S1.c_str());
        fProductionInfo->ACM_WriteMsgAndCallExe(sMessage);
    }
#endif
    bSendRealCCDSendStart = true;
#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1601-1602 -- the fMain facade has no machineTime (forms/fMain.h)
    if(CUSTOMER_CODE==CC_PANTHER)
        fMain->machineTime.Pause();
#endif

#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1605-1609 -- the facade's RENESAS_Server has no FTCTManStartUnlock (forms/fMain.h:124-140)
    // ---- RENESAS FTCT unlock (SMM 870-875) : RENESAS only ----
    if(TestIF_File.bRENESAS_EnableFTCT==true)
    {
        if(S1!=sFTCTAlarmStr) bAutoRestartAfterFTCTAlarm=false;
        fMain->RENESAS_Server->FTCTManStartUnlock(S1);
    }
#endif

    // ---- display (modeless) ----   [W906] P3
    if(W906_SecsAlarmShown_St02())                                              // golden 913 :1612
    {
        if(g_lines.empty() || g_lines.back()!=S1)                               //AI(ht9045-secs-sem) 20260923 (RogerYang) : 連續同一句不重複堆疊
        {
            while(g_lines.size()>=200)                                          //AI(ht9045-secs-sem) 20260923 (RogerYang) : 原本無上限, 一天可堆上百行
                g_lines.erase(g_lines.begin());
            g_lines.push_back(S1);
        }
        Post();                                                                 // fSecsAlarm->BringToFront();
    }
    else
    {
        g_lines.clear();                                                        // moSecs->Clear();
        g_lines.push_back(S1);
        g_chinese = S2;                                                         // lblChinese->Caption = S2;
        g_visible = true;                                                       // Show();
        Post();
    }
}

// =============================================================================
//  golden 913 mymessbox.cpp:1455-1508 (POOL-15) -- the browser's answer
// =============================================================================
int W906_SecsAlarmAnswer_St02(const char* tag, const char* action, std::string* reply)
{
    const std::string t = tag ? tag : "";
    if (reply) reply->clear();
    if (t.empty())
        return 0;
    if (!g_visible || g_qid.empty() || t != g_qid)
    {
        if (std::find(g_oldQids.begin(), g_oldQids.end(), t) == g_oldQids.end())
            return 0;                                                           // not this window's: wb_serve goes on as before
        if (reply) *reply = "no query pending (SECS host message " + t + " superseded by " + (g_qid.empty() ? std::string("none") : g_qid) + ")";
        return -1;                                                              // P4: the page drops the stale box
    }
    const std::string a = action ? action : "";
    if (a != "OK" && a != "PAUSE")                                              // L12 (golden 913 :1415-1416 / :1560-1563): only OK closes it
    {
        if (reply) *reply = "not an offered option: only OK closes the SECS host message (golden 913 L12)";
        return -1;
    }

    // ---- golden 913 mymessbox.cpp:1455 `void __fastcall TSecsAlarmForm::btnOKClick(TObject *Sender)` ----
    if(bWaitSecsGemReply==true)                      // pnl 447
    {
        if (reply) *reply = "bWaitSecsGemReply: golden btnOKClick ignores the key while waiting for EAP";
        return -1;
    }

    bStartMoveSpeed = false;                         // pnl 452

    // if(bMBoxNeedPassword) { Top = 0; if(MyMessageBox->DoPassword_MBox()==false) return; else bMBoxNeedPassword = false; }   // pnl 454-466
    //   [W906] golden 913 :1461-1466: nothing in golden 913 sets TSecsAlarmForm::bMBoxNeedPassword true (ctor :1450 false)

#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1468-1498 -- customer-specific close checks (RULINGS_20260925 #25; N07_5 = KYEC_LEE / JSCC_OS / SCC, Barcode_Reader = KYEC), as WebLogin.cpp:3000-3001; St01 docs/D034_PLAN_20261002.md
    if(bSECSGEMAlarm)                                // pnl 468-523
    {
        if(CosFunction.bUseN07_5==true)
        {
            if(iSECSMessageCanCloseByOperator!=1)
            {
                if(IniConfig.bN07_EnableEmployeeIdCheak==true)
                {
                    if(iSECSMessageCanCloseByOperator==0)
                    { if(MyMessageBox->DoPassword_MBox()==false) return; }
                    else if(iSECSMessageCanCloseByOperator==2)
                    {
                        bEnableEmployeeIDCheck=true;
                        CheckEmployeeID("工號檢查:請輸入工號密碼");
                        return;
                    }
                }
                else
                { if(MyMessageBox->DoPassword_MBox()==false) return; }
            }
            else
            {
                if(fNote->fShow)       fNote->Close();
                if(fPassword->Visible) fPassword->Close();
            }
        }
        else
        {
            if(Barcode_Reader(bcSECSGEM)==0) return;
        }
    }
#endif
    if(bSECSGEMAlarm)
        std::printf("  [SECS host message] golden 913 mymessbox.cpp:1468-1498 (N07_5 password / employee ID, KYEC barcode) not run -- "
                    "customer-specific, RULINGS_20260925 #25\n");

    // iValue = 0;                                   // pnl 525   [W906] P7: the MyMessageBox's (tools/wb_serve.cpp s_iValue)
    if(TestIF_File.bIndexCycleTimeMonitor==true && bResetflag==false)  // pnl 526-529
        bResetflag = true;

    DoReleaseAndHide();                              // pnl 530 (= Close + FormClose)

#if 0 // GATE(W906-POOL15) P6: golden 913 mymessbox.cpp:1506-1507 -- bRenesasFTCTAlarm's definition is still inside cmydef.cpp's #if 0 (:6286-6300; nothing links it)
    if(TestIF_File.bRENESAS_EnableFTCT==true)        // pnl 532-534
        bRenesasFTCTAlarm = false;
#endif

    std::printf("  [SECS host message] %s answered %s -> released (golden 913 mymessbox.cpp:1504 DoReleaseAndHide)\n", t.c_str(), a.c_str());
    return 1;
}

// =============================================================================
//  golden 913 SECSGEM/UsecegemMainFrom.cpp:147-151 FormCreate / :959-967 FormDestroy (POOL-15) -- the web host and the timer
// =============================================================================
void W906_SecsAlarmHostInstall_St02(W906SecsPostFn_St02 post, W906SecsIsOursFn_St02 isOurs, W906SecsRetireFn_St02 retire)
{
    g_post   = post;
    g_isOurs = isOurs;
    g_retire = retire;
    if (g_entry == 0)
    {
        g_entry = new ht9045::TTimerEntry("FSECS.TimerSecsAlarm", false, 1000,
                                          "SECSGEM/UsecegemMainFrom.cpp:1023-1110 (golden 913)", &St02OnTimerSecsAlarm);   // golden 913 UsecegemMainFrom.dfm:10977-10982
        g_entry->Status = "partial";                                            // P6 / P7
    }
    g_entry->Enabled = (post != 0);                  // TimerSecsAlarm->Enabled=true (FormCreate :150) / =false (FormDestroy :967)
}

ht9045::TTimerEntry* W906_SecsAlarmTimerEntry_St02() { return g_entry; }
bool        W906_SecsAlarmShown_St02()     { return g_visible; }
std::string W906_SecsAlarmQid_St02()       { return g_qid; }
std::string W906_SecsAlarmText_St02()      { return std::string(MemoText().c_str()); }
int         W906_SecsAlarmLineCount_St02() { return (int)g_lines.size(); }
void        W906_SecsAlarmReset_St02()
{
    g_visible = false;
    g_qid.clear();
    g_oldQids.clear();
    g_lines.clear();
    g_chinese = "";
}
