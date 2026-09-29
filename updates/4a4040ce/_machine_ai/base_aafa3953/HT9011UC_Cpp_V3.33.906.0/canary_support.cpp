// =============================================================================
//  canary_support.cpp  --  definitions for the W6.1 canary-support scaffold
//
//  Translation wave: W6.0 (canary-support scaffold for asendic_Empty)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  Defines the minimal LastSet shim + sim bodies for the free functions the
//  Empty-tray canary calls.  See canary_support.h for the per-symbol rationale
//  and golden provenance.
// =============================================================================
#include "canary_support.h"
#include "csystem.h"    // ReadWriteTrayID decl (golden csystem.h:245)
#include "cmydef.h"     // K_RETRY (extern const int)
#include "cpublic.h"    // RespondASECom decl (gated body in cpublic.cpp)
#include "mymessbox_web.h"  //AI(W906-SMM) 20260925: MyMessageBox 家族的網頁宿主 hook（定義在本檔，安裝在 tools/wb_serve.cpp）
#include <cstdio>
//AI(W906-W7-L2) 20260803: <vector> backs the alarm FIFO at the bottom of this
// file.  .cpp-only cost on purpose -- canary_support.h is included by 123 TUs
// and stays free of new includes.
#include <vector>

// ---------------------------------------------------------------------------
//  LastSet  (MINIMAL shim)
//  Zero-initialised -> iRealDummy == 0 == DUMMY.  The smoke test sets it
//  explicitly (DUMMY for the dummy-feed path); production LastSet lands W6.x.
// ---------------------------------------------------------------------------
// AI(W906-GA1-B1) 20260804: shim global RETIRED -- the real definition
// (plus Tech/CmdData/AlignTeach) now lives in LastSet.cpp (golden LastSet.cpp).

// ---------------------------------------------------------------------------
//  bIsCatchingFromBuffer / bIsPlacingToBuffer
//  Declared `extern bool` in cmydef.h, but their DEFINITIONS (cmydef.cpp:5946-
//  5947) fall inside the cmydef.cpp `#if 0 // TODO(W6)` gate (5806-6016), so the
//  ht9045_globals archive does not provide them yet.  They are the two newest
//  TrayArm-buffer flags (JerryYang 20250828 / RogerYang 20260225) and the canary
//  reads both (DoAutoEmpty case 0 + case 100).  Provide the definitions here as
//  a W6.1 scaffold; when the cmydef.cpp gate is opened (W6.x) remove these two.
// ---------------------------------------------------------------------------
bool bIsPlacingToBuffer   = false;      //JerryYang 20250828 (golden cmydef.cpp:5946)
bool bIsCatchingFromBuffer = false;     //RogerYang 20260225 (golden cmydef.cpp:5947)

// ---------------------------------------------------------------------------
//  AI(W906-W7-L1-Wave0) 20260801: OBSERVABILITY SEAM STATE for ShowErrorMessage
//  and ShowMyMessage (see canary_support.h for the full rationale).  The default
//  return stays K_RETRY, so this is a PURE ADDITION: every existing caller
//  behaves exactly as before until a test sets W906_ShowErrorMessage_SimReturn.
//
//  STATIC-INITIALISATION NOTE: K_RETRY is `extern const int` DEFINED in
//  cmydef.cpp:337, i.e. it is dynamically initialised, so seeding the seam with
//  the symbol here would depend on unspecified cross-TU initialisation order.
//  The literal 0x0001 is used instead -- K_RETRY's value verbatim from
//  cmydef.cpp:337 -- while W906_ShowErrorMessage_Reset() below uses the real
//  K_RETRY symbol, which is safe because a reset only ever runs from test code,
//  long after initialisation is complete.
// ---------------------------------------------------------------------------
int        W906_ShowErrorMessage_SimReturn = 0x0001;  // == K_RETRY (cmydef.cpp:337); see note above
AnsiString W906_ShowErrorMessage_LastCode;            // default-constructs to ""
int        W906_ShowErrorMessage_LastKCode = 0;  bool W906_ShowErrorMessage_LastDuplicate = false;   // AI(W906-J2) 20260926: see canary_support.h
int        W906_ShowErrorMessage_Count     = 0;
void W906_ShowErrorMessage_Reset()
{
    W906_ShowErrorMessage_SimReturn = K_RETRY;
    W906_ShowErrorMessage_LastCode  = "";
    W906_ShowErrorMessage_LastKCode = 0;  W906_ShowErrorMessage_LastDuplicate = false;
    W906_ShowErrorMessage_Count     = 0;
}
AnsiString W906_ShowMyMessage_LastS1;                 // default-constructs to ""
int        W906_ShowMyMessage_Count = 0;
void W906_ShowMyMessage_Reset()
{
    W906_ShowMyMessage_LastS1 = "";
    W906_ShowMyMessage_Count  = 0;
}

// ---------------------------------------------------------------------------
//  ShowErrorMessage -- golden note.h:466.
//  In the offline sim there is no operator to press Retry/Skip/Home, and the
//  Empty-tray SM uses the return value to choose its recovery branch.  Returning
//  K_RETRY keeps the SM on the "retry" arm (it re-attempts rather than skipping
//  or homing), which is the safe, faithful default for an unattended sim.
//  Logs the alarm so the smoke test trace shows which error path was taken.
//  AI(W906-W7-L1-Wave0) 20260801: K_RETRY is still the DEFAULT but is no longer
//  UNCONDITIONAL -- the return now comes from W906_ShowErrorMessage_SimReturn so
//  a test can drive the K_SKIP / K_CLEAN_OUT arms.  See the seam block above.
// ---------------------------------------------------------------------------
// AI(W906-FW-W5b) 20260819: answer hook, null by default (see canary_support.h).
int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos) = 0;
//AI(W906-P6b-C) 20260921: 預設 NULL —— 沒有宿主安裝時，`Command.cpp` 的
// `Bit4_HandlerDiagnostics` 維持今天的行為（恆 0）。理由與執行緒前提見標頭。
bool (*W906_DiagnosticsWindowOpen_Hook)(bool systemStart, int contactMode) = 0;

int ShowErrorMessage(AnsiString Code, int KCode, int Pos,
                     bool bDuplicateErr, AnsiString /*errPart*/)
{
    std::printf("  [ShowErrorMessage] Code=%s KCode=%d Pos=%d\n",
                Code.c_str(), KCode, Pos);
    // AI(W906-W7-L1-Wave0) 20260801: record the call, and return the settable
    // seam instead of a hardwired K_RETRY, so the K_SKIP / K_CLEAN_OUT recovery
    // arms reached through this function stop being structurally unreachable
    // (and therefore unfalsifiable).  See canary_support.h for the full note.
    W906_ShowErrorMessage_LastCode  = Code;
    W906_ShowErrorMessage_LastKCode = KCode;  W906_ShowErrorMessage_LastDuplicate = bDuplicateErr;   // AI(W906-J2) 20260926: golden note.cpp:802-803 -- recorded on every call, before the fShow check (:814), like golden
    W906_ShowErrorMessage_Count++;
    // AI(W906-FW-W5b) 20260819: record FIRST, then offer the question to a
    // live operator; hook returning 0 (or no hook) keeps the sim answer.
    if (W906_ShowErrorMessage_Hook)
    {
        int k = W906_ShowErrorMessage_Hook(Code.c_str(), KCode, Pos);
        if (k != 0) return k;
    }
    return W906_ShowErrorMessage_SimReturn;   // defaults to K_RETRY (unattended-sim posture, unchanged)
}

// ---------------------------------------------------------------------------
//  RecordProcess -- golden cMyDB.h:63.  AI(W906-CMYDB-P4) 20260927 (St02-E): the stdout stand-in that was here
//  is DELETED (P4, one commit with the other four); the golden body is live in cMyDB.cpp (golden 906_0625_Steven
//  cMyDB.cpp:1564-1573).  Declaration unchanged (canary_support.h:70).  Lines kept blank: line numbers do not move.
// ---------------------------------------------------------------------------






// ---------------------------------------------------------------------------
//  ShowUnloaderTrayMessage -- golden mymessbox.h:60.  Sim: log (no UI).
// ---------------------------------------------------------------------------
//AI(W906-SMM) 20260925: 宿主 hook（NULL = 沒有宿主，行為不變）。golden 本體是
//  MyMessageBox->Show()（mymessbox.cpp:930-955，非阻塞）；網頁版的顯示與 FormShow／FormClose
//  副作用在 tools/wb_serve.cpp 檔尾的 W906MbShowUnloaderTray。
void (*W906_ShowUnloaderTrayMessage_Hook)(const char* S1, const char* S2) = 0;
void ShowUnloaderTrayMessage(AnsiString S1, AnsiString S2)
{
    std::printf("  [ShowUnloaderTrayMessage] %s | %s\n", S1.c_str(), S2.c_str());
    if (W906_ShowUnloaderTrayMessage_Hook)                                      //AI(W906-SMM) 20260925
        W906_ShowUnloaderTrayMessage_Hook(S1.c_str(), S2.c_str());
}

// ---------------------------------------------------------------------------
//  WhichAutoNeedTray -- golden acatchtray.cpp:410.  W7: the REAL definition now
//  lives in the translated acatchtray.cpp (the TrayArm engine OWNS it).  The
//  W6.1 sim stub that used to live here (return 0) was REMOVED to avoid an ODR /
//  link collision (mirrors how the in-arm engine removed pitch-helper stubs from
//  aHotPlateSubstrate.cpp).  The decl remains in canary_support.h / acatchtray.h.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  ShowMyMessage -- golden mymessbox.h:58.  Sim: log (no modal dialog).
// ---------------------------------------------------------------------------
// AI(W906-FW-W5a) 20260819: forward hook, null by default (see canary_support.h).
void (*W906_ShowMyMessage_Hook)(const char* S1, const char* S2) = 0;

//AI(W906-SMM) 20260925: 完整參數的宿主 hook（mymessbox_web.h 說明為什麼另開一支而不改舊的）。
void (*W906_ShowMyMessageEx_Hook)(const char* S1, const char* S2, const char* S3,
                                  bool Ok, bool bServoOff) = 0;

void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString S3,
                   bool Ok, bool bServoOff)
{
    // AI(W906-W7-L1-Wave0) 20260801: observability seam -- golden's ShowMyMessage
    // returns void, so there is nothing to make settable; capture + count only.
    W906_ShowMyMessage_LastS1 = S1;
    W906_ShowMyMessage_Count++;
    if(S2.Length() > 0)
        std::printf("  [ShowMyMessage] %s | %s\n", S1.c_str(), S2.c_str());
    else
        std::printf("  [ShowMyMessage] %s\n", S1.c_str());
    // AI(W906-FW-W5a) 20260819: forward AFTER recording, so hook failures can
    // never lose the capture the tests assert on.
    if (W906_ShowMyMessage_Hook)
        W906_ShowMyMessage_Hook(S1.c_str(), S2.c_str());
    // AI(W906-SMM) 20260925: 然後交給網頁宿主照 golden 顯示並等回答（mymessbox.cpp:780-883
    //   的 ShowModal）。放在舊 hook **之後**：舊 hook 可能是 WebBuilder／WebSmartDiag 這一次
    //   請求的收集器，宿主看得出來（tools/wb_serve.cpp W906MbShowMyMessage 的 capture 判斷），
    //   那種情況訊息已經進了 ack 的 messages，宿主不再另外跳框。
    if (W906_ShowMyMessageEx_Hook)
        W906_ShowMyMessageEx_Hook(S1.c_str(), S2.c_str(), S3.c_str(), Ok, bServoOff);
}

// AI(W906-SMM) 20260925 → 合併 main 時（Steven 20260925 整合計畫 A）拿掉 Steven 這邊的 ShowMyMessageBox_YES_NO：
//   保留 Jimmy ee5de164 的版本（下方 AI(W906-YESNO)），兩份同名定義不能並存。

// ---------------------------------------------------------------------------
//  AI(W906-YESNO) 20260925: ShowMyMessageBox_YES_NO -- golden mymessbox.h:55 /
//  mymessbox.cpp:1009-1062.  取代五個 TU 各自的 `return 0;` 替身（使用者 20260925
//  裁決第 10 條：照 golden 跳網頁對話框等操作員回答）。語意與 hook 契約見標頭。
//
//  這裡**只做「記錄＋問」**，golden 本體其餘的事（StopAllMotor、FormShow 的停機、
//  FormClose 的收尾、MyDBIProcess("Message", S1+"  Yes"/"  No", S3)）由真的去問
//  操作員的那一方做（tools/wb_serve.cpp 的 ForwardShowMyMessageBoxYesNo）——
//  與 ShowErrorMessage／W906_AlarmStopLikeGolden 同一個分工：沒有人問，就不停機、
//  不記錄，行為與替身時代相同。
// ---------------------------------------------------------------------------
int        W906_ShowMyMessageBoxYesNo_SimReturn = 0;   // 替身時代的值；見標頭「SimReturn 的重設值刻意是 0」
AnsiString W906_ShowMyMessageBoxYesNo_LastS1;
AnsiString W906_ShowMyMessageBoxYesNo_LastS2;
int        W906_ShowMyMessageBoxYesNo_Count = 0;
int (*W906_ShowMyMessageBoxYesNo_Hook)(const char* S1, const char* S2, const char* S3) = 0;

void W906_ShowMyMessageBoxYesNo_Reset()
{
    W906_ShowMyMessageBoxYesNo_SimReturn = 0;
    W906_ShowMyMessageBoxYesNo_LastS1    = "";
    W906_ShowMyMessageBoxYesNo_LastS2    = "";
    W906_ShowMyMessageBoxYesNo_Count     = 0;
}

int ShowMyMessageBox_YES_NO(AnsiString S1, AnsiString S2, AnsiString S3)
{
    std::printf("  [ShowMyMessageBox_YES_NO] %s | %s\n", S1.c_str(), S2.c_str());
    // 先記錄再問 —— hook 出任何事都不會丟掉測試要斷言的 capture（同 ShowMyMessage）。
    W906_ShowMyMessageBoxYesNo_LastS1 = S1;
    W906_ShowMyMessageBoxYesNo_LastS2 = S2;
    W906_ShowMyMessageBoxYesNo_Count++;
    if (W906_ShowMyMessageBoxYesNo_Hook)
    {
        const int v = W906_ShowMyMessageBoxYesNo_Hook(S1.c_str(), S2.c_str(), S3.c_str());
        if (v != 0) return v;                  // 1=Yes 2=No 3=框已開著（golden mymessbox.cpp:1014）
    }
    return W906_ShowMyMessageBoxYesNo_SimReturn;   // 沒有操作員：預設 0（替身時代的值）
}

// ---------------------------------------------------------------------------
//  ReadWriteTrayID -- golden csystem.h:245.  Sim: no-op (no RFID/2D reader).
//  Declared in csystem.h; defined here for the canary so the SM links.
// ---------------------------------------------------------------------------
#if 0   // PT-W5f RETIRED (ReadWriteTrayID)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error, measured in build_0810_w5f.
void ReadWriteTrayID(bool /*bRead*/)
{
    // TODO(W6.x): wire to TrayID[][] read/write when the tray-ID subsystem lands.
}
#endif

// ---------------------------------------------------------------------------
//  RespondASECom -- declared in cpublic.h; its golden body (cpublic.cpp:729) is
//  GATED there (#if 0 // TODO(W6)) because it derefs ASESendMessage (god-stack).
//  Faithful to the golden: the real send only happens for CC_ASE_KaohSiung; for
//  every other customer (and offline) it returns false.  The sim has no ASE
//  Kaohsiung link, so this is the faithful false path.
//  When cpublic.cpp's gate is opened (W6.x), remove this definition.
// ---------------------------------------------------------------------------
bool RespondASECom(AnsiString /*S1*/)
{
#if 0   // TODO(W6.x): ASESendMessage->SendToASEData (god-stack); CC_ASE_KaohSiung
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        ASESendMessage->SendToASEData(S1);
        return true;
    }
#endif
    return false;   // offline / non-ASE-Kaohsiung: faithful false path
}

// =============================================================================
//  AI(W906-W7-L2) 20260803: SECTION 5 -- golden note.h free functions.
//  Rationale and per-function route (REAL vs RECORDING SIM) in canary_support.h
//  section 5.  note.cpp is not ported; these had no home anywhere in the tree.
// =============================================================================

// ---------------------------------------------------------------------------
//  MotorIndexToJamCode -- golden note.h:468, body golden note.cpp:4291-4296.
//  REAL TRANSLATION, verbatim: the golden body is exactly these three
//  statements, VCL-free and side-effect-free.
//  Yields e.g. MotNo=4 -> "WAR240004", which is the code the motor CCW/sensor
//  alarm family is known by.
// ---------------------------------------------------------------------------
AnsiString MotorIndexToJamCode(int MotNo)
{
    AnsiString S;
    S.sprintf("WAR24%03d", MotNo);
    return S;
}

// ---------------------------------------------------------------------------
//  CylinderIndexToJamCode -- golden note.h:470, body golden note.cpp:4156-4284.
//  //Steven 20231127 : 氣缸Alarm改成自動生成, 分類31
//  REAL TRANSLATION of the mapper.  All 20 branches, the C_*/MM* constants and
//  the final `S.sprintf("JAM%d", Code)` are golden's verbatim.
//
//  ONE DOCUMENTED OMISSION -- golden note.cpp:4161-4162 opens with
//      for(int i=0; i<6; i++)
//          fNote->Select[i]=false;
//  The ported TfNote (forms/fNote.h) carries no `Select[]` member and this front
//  does not own that file.  The field is the note DIALOG's button pre-clear
//  (golden note.h:396 `bool Select[10];`) -- note golden's own loop bound is 6
//  while the array is 10.  Nothing in the caller reads it: golden ProcessAlarm
//  (ckernel.cpp:2522-2523) consumes ONLY the returned string and *Pos, and both
//  are faithful here.  Re-add the loop in one line if forms/fNote.h ever gains
//  Select[]; see this front's report.
//
//  *Pos is unconditionally written -- golden's terminal else (:4277-4280,
//  //ChungHung 20140731 add fix Cylinder make UnknowMessage) defaults it to
//  MMSystem -- so callers may pass the address of an uninitialised int, as
//  golden ckernel.cpp:2505/2522 does.
// ---------------------------------------------------------------------------
AnsiString CylinderIndexToJamCode(int Code, int *Pos)
{
    int CylinderIndex=Code-31000;
    AnsiString S;

    if(CylinderIndex==C_TrayX_UpDown        ||
       CylinderIndex==C_CatchTray_FixOn     ||                                  //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
       CylinderIndex==C_TurnTrayArm         ||                                  //ChungHung 20140701 add AutoRetest Turn Tray Arm
       CylinderIndex==C_CatchTray_FixOff    ||
       CylinderIndex==C_CatchTray_Fix)
    {
        *Pos=MTrayX;
    }
    else if(CylinderIndex==C_TrayY_Fixer    ||
            CylinderIndex==C_LoaderEdgePush)
    {
        *Pos=MMTrayY;
    }
    else if(CylinderIndex==C_Load_Up        ||
            CylinderIndex==C_Load_Middle    ||
            CylinderIndex==C_TrayZ_Selector)
    {
        *Pos=MMTrayY_Car;
    }
    else if(CylinderIndex==C_Empty_Fix)
    {
        *Pos=MMEmpty;
    }
    else if(CylinderIndex==C_Empty_Up           ||
            CylinderIndex==C_Empty_Middle       ||
            CylinderIndex==C_EmptyLoaderZ_Select)
    {
        *Pos=MMEmpty_Car;
    }
    else if(CylinderIndex==C_Auto1EdgePush  ||
            CylinderIndex==C_Auto1Side_Fixer)
    {
        *Pos=MMAuto1;
    }
    else if(CylinderIndex==C_Auto1_Selector     ||
            CylinderIndex==C_Auto1_Up           ||                              //Steven 20140409 : Auto Retest
            CylinderIndex==C_Auto1LoaderZ_Select)                               //Steven 20140409 : Auto Retest
    {
        *Pos=MMAuto1_Car;
    }
    else if(CylinderIndex==C_Auto2Side_Fixer    ||
            CylinderIndex==C_Auto2EdgePush      )
    {
        *Pos=MMAuto2;
    }
    else if(CylinderIndex==C_Auto2_Selector     ||
            CylinderIndex==C_Auto2_Up           ||                              //Steven 20140409 : Auto Retest
            CylinderIndex==C_Auto2LoaderZ_Select)                               //Steven 20140409 : Auto Retest
    {
        *Pos=MMAuto2_Car;
    }
    else if(CylinderIndex==C_Auto3Side_Fixer    ||
            CylinderIndex==C_Auto3EdgePush      )
    {
        *Pos=MMAuto3;
    }
    else if(CylinderIndex==C_Auto3_Selector     ||
            CylinderIndex==C_Auto3_Up           ||                              //Steven 20140409 : Auto Retest
            CylinderIndex==C_Auto3LoaderZ_Select)                               //Steven 20140409 : Auto Retest
    {
        *Pos=MMAuto3_Car;
    }
    else if(CylinderIndex==C_Auto4EdgePush  ||
            CylinderIndex==C_Auto4Side_Fixer)
    {
        *Pos=MMAuto4;
    }
    else if(CylinderIndex==C_Auto4_Selector     ||
            CylinderIndex==C_Auto4_Up           ||
            CylinderIndex==C_Auto4LoaderZ_Select)
    {
        *Pos=MMAuto4_Car;
    }
    else if(CylinderIndex==C_Auto5EdgePush  ||
            CylinderIndex==C_Auto5Side_Fixer)
    {
        *Pos=MMAuto5;
    }
    else if(CylinderIndex==C_Auto5_Selector     ||
            CylinderIndex==C_Auto5_Up           ||
            CylinderIndex==C_Auto5LoaderZ_Select)
    {
        *Pos=MMAuto5_Car;
    }
    else if(CylinderIndex==C_Auto6EdgePush  ||
            CylinderIndex==C_Auto6Side_Fixer)
    {
        *Pos=MMAuto6;
    }
    else if(CylinderIndex==C_Auto6_Selector     ||
            CylinderIndex==C_Auto6_Up           ||
            CylinderIndex==C_Auto6LoaderZ_Select)
    {
        *Pos=MMAuto6_Car;
    }
    else if(CylinderIndex==C_Color_Fix)
    {
        *Pos=MMColor;
    }
    else if(CylinderIndex==C_Color_Up           ||
            CylinderIndex==C_Color_Middle       ||
            CylinderIndex==C_ColorLoaderZ_Select)
    {
        *Pos=MMColor_Car;
    }
    else if(CylinderIndex==C_TesterSidePush)                                    //Richard 20220321 : 測試Side Push
    {
        *Pos=MMSystem;
    }
    else if(CylinderIndex==C_FixTray_FullPlace)
    {
        *Pos=MManualTray3;
    }
    else                                                                        //ChungHung 20140731 add fix Cylinder make UnknowMessage
    {
        *Pos=MMSystem;
    }

    S.sprintf("JAM%d", Code);
    return S;
}

// ---------------------------------------------------------------------------
//  ShowMotorErrorMessage recorder state (golden note.h:467).
// ---------------------------------------------------------------------------
AnsiString W906_ShowMotorErrorMessage_LastCode;         // default-constructs to ""
int        W906_ShowMotorErrorMessage_LastMotorAlarmNo = 0;
AnsiString W906_ShowMotorErrorMessage_LastErrPart;      // default-constructs to ""
int        W906_ShowMotorErrorMessage_Count            = 0;
void W906_ShowMotorErrorMessage_Reset()
{
    W906_ShowMotorErrorMessage_LastCode         = "";
    W906_ShowMotorErrorMessage_LastMotorAlarmNo = 0;
    W906_ShowMotorErrorMessage_LastErrPart      = "";
    W906_ShowMotorErrorMessage_Count            = 0;
}

// ---------------------------------------------------------------------------
//  ShowMotorErrorMessage -- golden note.h:467, body golden note.cpp:1052+.
//  RECORDING SIM, but NOT a bare no-op.  Three things are reproduced faithfully
//  because they are cheap globals this tree already has (all four declared in
//  cmydef.h, which this file already includes):
//
//    * SoftStop=false     -- golden note.cpp:1054
//    * SoftStart=false    -- golden note.cpp:1055
//    * fAllMotorHome=false-- golden note.cpp:1059
//
//  These MATTER and dropping them would have been a real divergence: golden's
//  own caller sets SoftStop=true immediately before calling (golden
//  ckernel.cpp:2476), and it is THIS function that clears it again, so a no-op
//  sim would have left SoftStop stuck true for every downstream SM in the test.
//
//  NOT reproduced (the hardware/VCL half -- this is why it is a sim):
//    * StopAllMotor()                        golden note.cpp:1056
//    * MOT[MTestY1].Gali_Command("ST", ...)  golden note.cpp:1057
//    * IndexMotorBreakerOFF()                golden note.cpp:1058
//    * everything from golden note.cpp:1073 on -- MyDBIEvent, fNote->sJamArea /
//      sJamCode / ErrShowToForm / KeyCode, ProductionLog, ShowErrorUnit,
//      fAutomation->DoCommandBuffer, fFTPClient->SaveJamCodeFile, TStringList.
//
//  CONTROL FLOW IS FAITHFUL for the two early-return arms:
//
//    1. `if(InitialOK==false)` -- golden note.cpp:1061, //Steven 20250310 : Add
//       protection.  OFFLINE THIS ARM IS TAKEN: InitialOK is defined false at
//       cmydef.cpp:285 and nothing in an offline facade sets it.  That is the
//       correct selection, not a convenient one -- golden added the guard for
//       exactly the "machine has not finished initialising" state, which is what
//       an offline facade permanently is.  Consequence a test must know: while
//       InitialOK is false this function NEVER reaches arm 2, so it never calls
//       ShowErrorMessage.  Set InitialOK=true to exercise arm 2.
//       Golden's body for this arm is MyDBIProcess("Exception", Code,
//       AnsiString(MotorAlarmNo)) (:1063).  NOT forwarded: golden's MyDBIProcess
//       takes three parameters (golden cMyDB.h:20) but the only MyDBIProcess in
//       this tree takes two (aHotPlateSubstrate.h:727) and its body is an empty
//       stub (aHotPlateSubstrate.cpp:772), so forwarding would buy nothing
//       observable while adding a link edge.  The row is recorded and traced
//       instead.
//
//    2. `if(Code=="WAR")` -- golden note.cpp:1067, //Steven 20100830.  Raises
//       WAR16101 ("Motor error !!") through ShowErrorMessage and returns.
//       Reachable only once InitialOK is true.  Note ckernel never triggers it:
//       ckernel passes MotorIndexToJamCode's "WAR24nnn" (golden
//       ckernel.cpp:2470/2490), never the bare "WAR".
// ---------------------------------------------------------------------------
void ShowMotorErrorMessage(AnsiString Code, int MotorAlarmNo, AnsiString errPart)
{
    //AI(W906-W7-L2) 20260803: record first, so the W7-L2 ckernel test can assert
    // "fired exactly once with this JamCode" regardless of which arm runs below.
    W906_ShowMotorErrorMessage_LastCode         = Code;
    W906_ShowMotorErrorMessage_LastMotorAlarmNo = MotorAlarmNo;
    W906_ShowMotorErrorMessage_LastErrPart      = errPart;
    W906_ShowMotorErrorMessage_Count++;
    std::printf("  [ShowMotorErrorMessage] Code=%s MotorAlarmNo=%d errPart=%s\n",
                Code.c_str(), MotorAlarmNo, errPart.c_str());

    SoftStop=false;                                                             //golden note.cpp:1054
    SoftStart=false;                                                            //golden note.cpp:1055
    //  golden note.cpp:1056-1058 (StopAllMotor / Galil "ST" / IndexMotorBreakerOFF)
    //  omitted -- hardware, see the header block above.
    fAllMotorHome=false;                                                        //golden note.cpp:1059

    if(InitialOK==false)                                                        //golden note.cpp:1061 //Steven 20250310 : Add protection
    {
        //  golden note.cpp:1063 MyDBIProcess("Exception", Code, AnsiString(MotorAlarmNo))
        //  -- arity divergence, recorded not forwarded (see header block).
        std::printf("  [ShowMotorErrorMessage] (InitialOK==false) Exception | %s | %d\n",
                    Code.c_str(), MotorAlarmNo);
        return;
    }

    if(Code=="WAR")                                                             //golden note.cpp:1067 //Steven 20100830
    {
        ShowErrorMessage("WAR16101", 0, MMSystem, false, AnsiString(MotorAlarmNo));   //golden note.cpp:1069  Motor error !!
        return;
    }

    //  golden note.cpp:1073 onward -- note-form / EventLog DB / FTP / OLP
    //  display half.  Not reproducible offline; the recorder above is the
    //  observable substitute.
}

// =============================================================================
//  AI(W906-W7-L2) 20260803: SECTION 6 -- MyDBIProcessNew (golden cMyDB.h:21).
//  AI(W906-CMYDB-P4) 20260927 (St02-E): the record + stdout stand-in and its recorder W906_MyDBIProcessNew_*
//  (no reader anywhere, grep 20260927) are DELETED (P4); the golden body is live in cMyDB.cpp (golden 906_0625_Steven
//  cMyDB.cpp:724-787).  Declaration + the atester_32Site.cpp collision note: canary_support.h:282-300.
//  Lines kept blank below: line numbers do not move.
// =============================================================================















// =============================================================================
//  AI(W906-W7-L2) 20260803: SECTION 7 -- THE ALARM-QUEUE SEAM.
//  AI(W906-HALARM) 20260926: RETIRED.  PopUpAlarm / ClearAllAlarm (and the
//  W906_PopUpAlarm_Push / W906_Alarm_* test stand-ins, zero users) are replaced
//  by the whole golden component translated in halarm.h / HAlarm.cpp
//  (ht9045_globals) -- RULINGS_20260926 #16.  Keeping these definitions would be
//  a multiple-definition error the moment HAlarm.o is pulled from the archive.
//  SystemNG moved with them (golden HAlarm.cpp:14; acarry_shims.cpp now only
//  declares it).  See canary_support.h section 7 for what changed.
// =============================================================================
