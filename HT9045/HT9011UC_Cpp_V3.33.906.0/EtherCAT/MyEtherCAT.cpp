// =============================================================================
//  EtherCAT/MyEtherCAT.cpp  --  TMyEtherCAT implementation + PCI1203 card mgmt
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/EtherCAT/MyEtherCAT.cpp (489 lines)
//  Translation wave: W906-PT-W2 fieldbus (CanBus + EtherCAT group)
//  Translator: AI(W906-PT-W2) 20260807
//
//  WAVE SCOPE -- ACTIVE (faithful, whole file, all 7 golden functions):
//    TMyEtherCAT::TMyEtherCAT          golden :18-51
//    TMyEtherCAT::~TMyEtherCAT         golden :53-56
//    TMyEtherCAT::Start                golden :58-61
//    TMyEtherCAT::Pause                golden :63-66
//    TMyEtherCAT::TaskClear             golden :68-73
//    TMyEtherCAT::tmrReadInputDataTimer golden :75-232
//    CheckPCI_EtherCatState (free fn)   golden :236-407
//    INSTALL_ETHETCAT (free fn)         golden :409-419
//    OpenEtherCatMastCard (free fn)     golden :421-488
//  SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//    TMyNUEC1 (EtherCAT/MyNUEC1.h/.cpp, whole class incl. iGetAmplifier()/
//    iGetDesMACID()/ReadCurrentValue()/ReadCurrentIO()/WriteSettingValue()/
//    ReadSettingValue()/ClearError()/RetWriteSettingValue[]/CheckAmplifierExist()),
//    uiDevhand/m_dwDevNum/iEtherCatRing/TOTAL_MOTOR/MMSystem/SHUTTLE_SENSOR_TYPE/
//    VCCU_UNIT_TYPE/MachineTypeChoice/USE_46_SUCKER_DB/SystemHour|Min|Sec
//    (all cmydef.h), Type_HT9046/Type_HT9046_LS/Type_HT1032/eSensorEtherCAT(3)
//    (MachineType.h), MOT[]/TTrayMotor/HTMotor::Enable/ResetAxisOpen()/
//    InitMotor() (Motor/mymotor.h, Motor/HTMotor.h), ShowErrorMessage
//    (canary_support.h), NewRecordProcess (acatchtray_shims.h -- declared but
//    UNUSED by this file: golden's own 2 call sites at :389/:391 are
//    `//`-commented dead code, preserved as comments below, not activated),
//    AnsiString::sprintf/printf, IntToStr/IntToHex (vclcompat).
//
//  GATE REGISTER -- 2 entries, both re-cited at their call sites below:
//   (1) fCCLink (TfCCLink -- the CC-Link config FORM, golden
//       CCLink/MyCCLinkSensor.h, __published field iTotalDevices/
//       iDesMACIDList[16]).  W7-UI, NOT translated anywhere in this tree:
//       grepped the whole port tree (every .cpp/.h outside build*/) on
//       20260807 for `class TfCCLink` and `*fCCLink` -- zero hits outside
//       comments (CCLink/MyCCLink.h's own banner independently documents
//       the SAME form as W7-UI-deferred, for the SAME reason). Two golden
//       touches, both inside CheckPCI_EtherCatState:
//         golden :266  `iTotalDeviceRing1+=fCCLink->iTotalDevices;`
//         golden :343-364  `for(i=0;i<fCCLink->iTotalDevices;i++) { ...
//                            fCCLink->iDesMACIDList[i] ... }` (CC-Link per-
//                            slave EC_SLAVE_STATE_OP health poll)
//       Gated `#if 0`/`#else` at both sites (Hard Rule 6 idiom).
//       WHY 0-DEVICES IS THE FAITHFUL DEFAULT: with fCCLink unavailable there
//       is no way to read its live device count/MACID list; treating the
//       CC-Link ring as contributing 0 slaves is the same "nothing attached"
//       shape golden itself falls into whenever `SHUTTLE_SENSOR_TYPE` is
//       neither eSensorEtherCAT nor eSensorEtherCAT3 (the whole block is
//       skipped) -- i.e. this collapses to golden's own "CC-Link ring not in
//       play" branch rather than inventing a new state.
//       BEHAVIOUR DELTA, STATED PLAINLY: on a real machine with
//       SHUTTLE_SENSOR_TYPE==eSensorEtherCAT(3) AND a live CC-Link ring
//       attached, this port under-counts iTotalDeviceRing1 by
//       fCCLink->iTotalDevices (so a genuine Ring1-IO-disconnect event can
//       be misjudged at the iTotalDeviceRing1!=iCheckIoCnt compare, golden
//       :312) and never raises WAR16151 for a CC-Link slave that drops out
//       of EC_SLAVE_STATE_OP (golden :367-371), until CCLink/MyCCLinkSensor.h's
//       TfCCLink form is translated.  Retire this gate the moment that
//       lands: delete the two macro pairs, restore the `#if 0` arms.
//   (2) The Acm_Dev*/Acm_Daq*/DEVLIST/U32/U16/ULONG/EC_SLAVE_STATE_OP/
//       EVT_DEV_DISCONNET/EVT_DEV_IO_DISCONNET/FT_MasCyclicCnt_R0|R1/
//       FT_DevAxesCount vendor family (EtherCAT/AdvMotApi.h and siblings) is
//       gated behind `#if HAVE_PCI1203`, REUSING the flag this tree's W4-IO
//       wave already established for this EXACT vendor header + uiDevhand
//       master handle (see IOBackend.cpp's TPci1203Backend, MyLaneIo.cpp's
//       "EtherCAT analog trio", and this file's own sibling
//       EtherCAT/MyNUEC1.cpp -- all gate the SAME functions under the SAME
//       macro). Not a new gate invented by this unit; flagged here because
//       Hard Rule 6 technically applies -- these are real vendor-DLL-exported
//       declarations with ZERO compiled bodies anywhere in this tree by
//       design (they resolve only against the PCI1203 vendor .lib at link
//       time). Covers:
//         CheckPCI_EtherCatState's entire non-SOFT_SIMULTE body (golden
//           :253-406, everything between `#else` and the trailing `#endif`)
//         OpenEtherCatMastCard's card-open sequence (golden :421-475, EVERY
//           statement except the trailing MOT[] reinit loop at :476-485,
//           which only touches already-real HTMotor/TTrayMotor and needs no
//           gate)
//       DEFAULT (HAVE_PCI1203 undefined): CheckPCI_EtherCatState() no-ops --
//       `bCheckPCI_EtherCatStateRun=false; return 0;` -- mirroring golden's
//       own early-return shape for "not ready" states (golden :238-241 /
//       :243-246). OpenEtherCatMastCard() skips straight to the
//       unconditionally-real MOT[] reinit loop and reports SUCCESS (0),
//       matching golden's own "card opened cleanly" terminal return
//       (golden :487).
//       BEHAVIOUR DELTA: with no PCI1203 vendor SDK linked, this build never
//       actually detects an EtherCAT master/IO disconnect and never
//       physically reopens the card -- identical in spirit to golden's own
//       `#ifdef SOFT_SIMULTE` early-return branch (golden :248-252), just
//       triggered by a different (build-time, not source-time) switch.
// =============================================================================
#include "EtherCAT/MyEtherCAT.h"

#include "MachineType.h"   // SHUTTLE_SENSOR_TYPE, eSensorEtherCAT/eSensorEtherCAT3,
                            // VCCU_UNIT_TYPE, MachineTypeChoice, Type_HT9046/_LS, Type_HT1032
#include "cmydef.h"         // uiDevhand, m_dwDevNum, iEtherCatRing, TOTAL_MOTOR, MMSystem,
                            // USE_46_SUCKER_DB, SystemHour/SystemMin/SystemSec
#include "Motor/mymotor.h"  // MOT[], TTrayMotor, MAX_DEVICES
#include "canary_support.h" // ShowErrorMessage
#include "acatchtray_shims.h" // NewRecordProcess

#include <cstdlib>          // atoi
#include <windows.h>        // Sleep()

#if HAVE_PCI1203
//AI(W906-PT-W2-integrate) 20260807: restored to golden's bare form (golden
//  EtherCAT/MyEtherCAT.cpp:10 is `#include "AdvMotApi.h"`).  The wave had
//  written it as "EtherCAT/AdvMotApi.h", which is both a deviation from golden
//  and unresolvable here: this tree stages the read-only Advantech headers under
//  EtherCAT/vendor/ to keep them visibly separate from ported code, and that
//  directory is on the include path (root CMakeLists.txt, ht9045_motor).
#include "EtherCAT/AdvMotCompat.h"        // AI(W906-1203HAL-1) 20260820: ADVCMNAPI shim -> vendor AdvMotApi.h (Acm_Dev*/Acm_Daq*, SUCCESS,
                                 // DEVLIST, U32/U16/ULONG, EC_SLAVE_STATE_OP,
                                 // EVT_DEV_DISCONNET/EVT_DEV_IO_DISCONNET,
                                 // FT_MasCyclicCnt_R0/R1, FT_DevAxesCount,
                                 // EC_OpenMasterDevFailed -- transitively via
                                 // AdvMotDev.h/AdvMotDrv.h/AdvMotPropID.h/AdvMotErr.h)
#endif

// AI(W906-PT-W2) 20260807: golden's `SUCCESS` (vendor AdvMotErr.h:16,
// ==0x00000000) is compared against OpenEtherCatMastCard()'s return value at
// tmrReadInputDataTimer's case 500 (golden :96) -- a call site that runs
// regardless of HAVE_PCI1203 (the state machine itself is not vendor-gated,
// only the card-open internals are). Provide the SAME numeric fallback when
// the vendor header was not included above, so that comparison keeps
// compiling AND keeps its golden meaning (0 == success) either way.
#ifndef SUCCESS
#define SUCCESS 0
#endif

// AI(W906-PT-W2) 20260807: golden declares MySleep in common.h (BUSY-poll
// sleep helper, real body lives in acarry_shims.cpp per this tree's
// established convention -- see CCLink/MyCCLink.cpp's identical local
// extern for the SAME reason: common.h's own declaration is gated
// `#if 0 // TODO(wave-timing)`).
extern void MySleep(DWORD dwMilliseconds);

// golden extern decl, kept local exactly as golden has it (MyEtherCAT.cpp:233)
// -- MNetLog's real body is declared/defined elsewhere in this tree (e.g.
// Motor/mymotor.cpp/MyLaneIo.cpp already carry the SAME local extern); not
// this unit's symbol to define.
extern bool MNetLog(AnsiString Message);   // Steven 20110406

//---------------------------------------------------------------------------
TMyEtherCAT *MyEtherCAT;
//---------------------------------------------------------------------------
//  ctor / dtor
//  golden EtherCAT/MyEtherCAT.cpp:18-56
//---------------------------------------------------------------------------
// AI(W906-PT-W2) 20260807: golden's `: TComponent(Owner)` base
// mem-initializer is dropped along with the TComponent base itself (see
// MyEtherCAT.h header comment for the full "Owner" ctor-bug diagnosis and
// the in-wave precedent this follows, EtherCAT/MyNUEC1.h). No base ctor call
// remains here because there is no base left to initialize.
TMyEtherCAT::TMyEtherCAT(unsigned short _iTotalDevices, unsigned char *_iDesMACIDList, unsigned char *_iAmplifierList)
{
    // AI(W906-PT-W2) 20260807: golden's `if(NUEC1Item[i]!=NULL) delete
    // NUEC1Item[i];` guard below (golden :37-38) relies on VCL's
    // NewInstance zero-filling the whole instance before the ctor body runs
    // (same reasoning EtherCAT/MyNUEC1.cpp's ctor banner documents for its
    // own array members). Plain C++ `new TMyEtherCAT(...)` gives no such
    // guarantee, so NUEC1Item[] is explicitly zeroed here first -- ADDED
    // (not present in golden's text) to reproduce golden's ACTUAL runtime
    // behavior rather than golden's literal absence of code.
    for(int i=0; i<MaxNUEC1; i++)
        NUEC1Item[i]=NULL;

    iTotalDevices=_iTotalDevices;                                               //使用幾顆 NU-EC1
    for(int i=0; i<MaxNUEC1; i++)
    {
        if(i<iTotalDevices)
        {
            iDesMACIDList[i]=_iDesMACIDList[i];
            iAmplifierList[i]=_iAmplifierList[i];
        }
        else
        {
            iDesMACIDList[i]=0;
        }
    }
    iRing=iEtherCatRing;

    for(int i=0; i<iTotalDevices; i++)
    {
        if(NUEC1Item[i]!=NULL)
            delete NUEC1Item[i];

        NUEC1Item[i]=new TMyNUEC1(iRing,
                                  iDesMACIDList[i],
                                  iAmplifierList[i]);
    }
    bReOpenPCI1203=false;

    // AI(W906-PT-W2) 20260807: substitutes for golden's `tmrReadInputData=new
    // TTimer(this); ...->OnTimer=tmrReadInputDataTimer; ...->Interval=30;
    // ...->Enabled=false;` (see .h banner note -- bTimerEnabled stands in for
    // tmrReadInputData->Enabled; nothing here drives the 30ms cadence yet).
    bTimerEnabled=false;
    iEtherCATTask=1;
}
//---------------------------------------------------------------------------
TMyEtherCAT::~TMyEtherCAT()
{
    bTimerEnabled=false;   // golden: tmrReadInputData->Enabled=false;
}
//---------------------------------------------------------------------------
void TMyEtherCAT::Start()
{
     bTimerEnabled=true;   // golden: tmrReadInputData->Enabled=true;
}
//---------------------------------------------------------------------------
void TMyEtherCAT::Pause()
{
     bTimerEnabled=false;   // golden: tmrReadInputData->Enabled=false;
}
//---------------------------------------------------------------------------
void TMyEtherCAT::TaskClear()
{
    bHasErr=false;
    iEtherCATTask=1;
    bReOpenPCI1203=true;
}
//---------------------------------------------------------------------------
//  tmrReadInputDataTimer -- switch(Task)-stepped FS-N12 fleet poll.
//  golden EtherCAT/MyEtherCAT.cpp:75-232.  __fastcall dropped (see .h note).
//---------------------------------------------------------------------------
void TMyEtherCAT::tmrReadInputDataTimer(TObject * /*Sender*/)
{
    static bool bIn=false;
    static int iNUEC1No=0;
    static int iErrCount=0;

    // AI(W906-PT-W2) 20260807: substitutes for the VCL TTimer's own Enabled
    // gate (a real TTimer with Enabled==false simply never fires OnTimer) --
    // see .h note. golden itself only checks bInitialOK/bHasErr/bIn here
    // (the TTimer's Enabled gate was implicit); bTimerEnabled is ADDED to
    // reproduce that implicit gate now that there is no real VCL TTimer.
    if(bTimerEnabled==false ||
       bInitialOK==false ||
       bHasErr==true ||
       bIn==true)                                                               //Steven 20250915 : EtherCAT Timer加上保護
    {
        return;
    }

    bool bRet=false;
    int ret=0;
    int &Task=iEtherCATTask;
    AnsiString sErr="";
    AnsiString StrEng="";
    bIn=true;

    switch (Task)
    {
        case 1:
            if(bReOpenPCI1203)                                                  //PCI1203 重新開卡
            {
                Task=500;
                break;
            }
            iNUEC1No=0;
            Task=1000;
            break;
        case 500:                                                               //PCI1203 重新開卡
            ret=OpenEtherCatMastCard();
            if(ret!=SUCCESS)
            {
                ShowErrorMessage("WAR16150", 0, MMSystem, 0, IntToStr(ret));
            }
            bReOpenPCI1203=false;
            Task=1;
            break;
        case 1000:                                                              //讀取 FS-N12 Sensor 數值
            if(NUEC1Item[iNUEC1No]->iGetAmplifier()==0)                         //沒有接 FS-N12 就跳下醫科
            {
                iNUEC1No++;
                if(iNUEC1No>=iTotalDevices)                                     //讀寫完全部 NU-EC1
                {
                    Task=1;
                }
                break;
            }

            SysMess="Read Currect Value";
            bRet=NUEC1Item[iNUEC1No]->ReadCurrentValue();
            if(bRet)
            {
                iErrCount=0;
                Task=2000;
            }
            else
            {
                iErrCount++;
                if(iErrCount>10)
                {
                    Task=1;
                    bHasErr=true;
                    ErrMess.printf("NUEC1=%d, EtherCAT Read CurrentValue error", NUEC1Item[iNUEC1No]->iGetDesMACID());
                    NewRecordProcess("", "EtherCAT Read CurrentValue error", ErrMess);
                }
                else
                {
                    Task=1000;
                }
            }
            break;
        case 2000:                                                              //讀取 FS-N12 Sensor IO
            SysMess="Read I/O Value";
            bRet=NUEC1Item[iNUEC1No]->ReadCurrentIO();
            if(bRet)
            {
                iErrCount=0;
                Task=3000;
            }
            else
            {
                iErrCount++;
                if(iErrCount>10)
                {
                    Task=1;
                    bHasErr=true;
                    ErrMess.printf("NUEC1=%d, EtherCAT Read I/O error", NUEC1Item[iNUEC1No]->iGetDesMACID());
                    NewRecordProcess("", "EtherCAT Read I/O error", ErrMess);
                }
                else
                {
                    Task=2000;
                }
            }
            break;
        case 3000:                                                              //設定 FS-N12 Sensor IO Value 閥值
            bRet=NUEC1Item[iNUEC1No]->WriteSettingValue();
            if(bRet)
            {
                iErrCount=0;
                Task=4000;
            }
            else
            {
                iErrCount++;
                if(iErrCount>10)
                {
                    Task=1;
                    bHasErr=true;
                    for(int i=0;i<MaxFSN12;i++)
                       sErr+=AnsiString(NUEC1Item[iNUEC1No]->RetWriteSettingValue[i]);

                    ErrMess.printf("NUEC1=%d, EtherCAT Setting I/O Value error %s", NUEC1Item[iNUEC1No]->iGetDesMACID(), sErr);
                    NewRecordProcess("", "EtherCAT Setting I/O Value error", ErrMess);
                    NUEC1Item[iNUEC1No]->ClearError();
                }
                else
                {
                    Task=3000;
                }
            }
            break;
        case 4000:                                                              //讀取 FS-N12 IO Value 閥值
            bRet=NUEC1Item[iNUEC1No]->ReadSettingValue();
            if(bRet)
            {
                iErrCount=0;
                Task=1000;
                iNUEC1No++;
                if(iNUEC1No>=iTotalDevices)                                     //讀寫完全部 NU-EC1
                {
                    Task=1;
                    break;
                }
            }
            else
            {
                iErrCount++;
                if(iErrCount>10)
                {
                    Task=1;
                    bHasErr=true;
                    ErrMess.printf("NUEC1=%d, EtherCAT Read I/O Value error", NUEC1Item[iNUEC1No]->iGetDesMACID());
                    NewRecordProcess("", "EtherCAT Read I/O Value error", ErrMess);
                }
                else
                {
                    Task=4000;
                }
            }
            break;
    }

    bIn=false;
}
//------------------------------------------------------------------------------
//  CheckPCI_EtherCatState -- Sam 20230811 : 新增 EtherCAT 檢查功能
//  golden EtherCAT/MyEtherCAT.cpp:236-407.  See file-banner GATE REGISTER
//  entries (1) [fCCLink] and (2) [HAVE_PCI1203] for both gates below.
//------------------------------------------------------------------------------
bool bCheckPCI_EtherCatStateRun=false;
int CheckPCI_EtherCatState()
{
    if(MyEtherCAT->bInitialOK==false)                                           //RogerYang 20250731 fix flag; golden: MyEtherCAT->InitialOK
    {
        return 0;
    }

    if(bCheckPCI_EtherCatStateRun==true)
    {
        return 0;
    }
    bCheckPCI_EtherCatStateRun=true;
    #ifdef SOFT_SIMULTE
    {
        bCheckPCI_EtherCatStateRun=false;
        return 1;
    }
    #else
    #if HAVE_PCI1203
    U32 Result, MasCheckEvt=0, iCheckAxisCnt=0, iCheckIoCnt=0;
    ULONG  AxesPerDev, buffLen=64;
    AnsiString S="", Str1="", sRet="";
    int iIORing=1;
    unsigned int iTotalDeviceRing0=0, iTotalDeviceRing1=0;
    U16 SlaveState=0;
    bool bResetEtherCAT=false, bCheckIOError=false;
    static int iFailCount=0;

    if(SHUTTLE_SENSOR_TYPE==eSensorEtherCAT  ||
       SHUTTLE_SENSOR_TYPE==eSensorEtherCAT3)
    {
#if 0
        // GATE REGISTER (1): fCCLink (TfCCLink) not translated -- see file
        // banner. golden EtherCAT/MyEtherCAT.cpp:266.
        iTotalDeviceRing1+=fCCLink->iTotalDevices;
#else
        // Faithful default: 0 additional Ring1 devices contributed by
        // CC-Link until TfCCLink lands (see GATE REGISTER (1) above).
#endif
    }

    if(VCCU_UNIT_TYPE==1)
    {
        if(MachineTypeChoice==Type_HT9046      ||
           MachineTypeChoice==Type_HT9046_LS   ||
           MachineTypeChoice==Type_HT1032      || MachineTypeChoice==Type_HT9050 ||   //AI(W906-HT9050-FAM) 20260925: 加 Type_HT9050（兩臂 golden 都是 +=1，行為不變）-- golden 沒有 HT9050，照 HT9046 家族（使用者 20260925，RULINGS_20260925 第 5 條）
           USE_46_SUCKER_DB==1)                                                 //HT9046LS sucker_9046.db
        {
            iTotalDeviceRing1+=1;                                               //目前只裝1個在模擬測試
        }
        else
        {
            iTotalDeviceRing1+=1;                                               //目前只裝1個在模擬測試
        }
    }

    Result=Acm_DevCheckEvent(uiDevhand, &MasCheckEvt, 10);                      //檢查 Ring
    if(Result!=SUCCESS ||
       (MasCheckEvt&EVT_DEV_DISCONNET) ||
       (MasCheckEvt&EVT_DEV_IO_DISCONNET))
    {
        if(MasCheckEvt&EVT_DEV_DISCONNET &&
           iTotalDeviceRing0>0)                                                 //雖然事件已經抓到是 Ring0 斷線，可能是誤判，再抓 Device 數量來雙重判斷
        {
            Result=Acm_GetU32Property(uiDevhand, FT_MasCyclicCnt_R0, &iCheckAxisCnt);
            if(Result==SUCCESS)
            {
                if(iTotalDeviceRing0!=iCheckAxisCnt)
                {
                    // NOTE(golden quirk, preserved verbatim -- not "fixed"):
                    // "%[d]" is not a valid printf conversion; golden's own
                    // format string literally contains this. Only reachable
                    // when a real PCI1203 card observes iTotalDeviceRing0>0
                    // AND a Ring0 disconnect event; harmless bystander
                    // (the resulting mis-formatted S is only used inside the
                    // WAR16152 message text below).
                    S.sprintf("Ring0 Error Device %[d] [%d]", iTotalDeviceRing0 ,iCheckAxisCnt);
                    bResetEtherCAT=true;
                }
            }
            else
            {
                S="Ring0 Error";
                bResetEtherCAT=true;
            }
        }
        else if(MasCheckEvt & EVT_DEV_IO_DISCONNET && iTotalDeviceRing1>0)
        {
            Result=Acm_GetU32Property(uiDevhand, FT_MasCyclicCnt_R1, &iCheckIoCnt);
            if(Result==SUCCESS)
            {
                if(iTotalDeviceRing1!=iCheckIoCnt)
                {
                    bResetEtherCAT=true;
                    S="Ring1 Error";                                            //RgoerYang 20250411 add
                }
            }
            else
            {
                S="Ring1 Error";
                bResetEtherCAT=true;
            }
        }
        else
        {
            S="Ring Error";
            bResetEtherCAT=true;
        }
    }

    if(bResetEtherCAT)
    {
        ShowErrorMessage("WAR16152", 0, MMSystem, 0, S);
    }

    if(SHUTTLE_SENSOR_TYPE==eSensorEtherCAT  ||
       SHUTTLE_SENSOR_TYPE==eSensorEtherCAT3)
    {                                                                           //檢查 NU-EC1 站別
        Result=Acm_GetProperty(uiDevhand, FT_DevAxesCount, &AxesPerDev, &buffLen);
        if(Result==SUCCESS)
        {
            Str1="";
#if 0
            // GATE REGISTER (1): fCCLink (TfCCLink) not translated -- see
            // file banner. golden EtherCAT/MyEtherCAT.cpp:343-364 (per-slave
            // CC-Link EC_SLAVE_STATE_OP poll).
            for(int i=0; i<fCCLink->iTotalDevices; i++)
            {
                SlaveState=0;
                Result=Acm_DevGetSlaveStates(uiDevhand, iIORing, fCCLink->iDesMACIDList[i], &SlaveState);
                if(Result==SUCCESS)
                {
                    if(SlaveState!=EC_SLAVE_STATE_OP)
                    {
                        bCheckIOError=true;
                        Str1+=S.sprintf("[%d]=X ",fCCLink->iDesMACIDList[i]);
                    }
                    else
                    {
                        Str1+=S.sprintf("[%d]=O ",fCCLink->iDesMACIDList[i]);
                    }
                }
                else
                {
                    Str1+=S.sprintf("[%d]=X ",fCCLink->iDesMACIDList[i]);
                    bCheckIOError=true;
                }
            }
#else
            // Faithful default: 0 CC-Link slaves to poll (see GATE REGISTER
            // (1) above) -- bCheckIOError stays false, matching golden's own
            // "no error" fallthrough.
            (void)iIORing; (void)SlaveState;
#endif

            if(bCheckIOError)
            {
                ShowErrorMessage("WAR16151", 0, MMSystem, 0, Str1);
                bResetEtherCAT=true;
            }
        }

        if(SystemHour==0 && SystemMin==0 &&
           (0<=SystemSec && SystemSec<20))                                      //JerryYang 20250723 : 抓數量錯誤先不alarm
        {
            iFailCount=0;
        }

        for(int i=0; i<3; i++)                                                  //檢查各 NU-EC1 Amplifier 數量是否正確  //JerryYang 20250723 : Mark掉
        {
            sRet=MyEtherCAT->NUEC1Item[i]->CheckAmplifierExist();
            if(sRet!="OK")
            {
                iFailCount++;
                if(iFailCount<10)
                {
                    Str1.sprintf("EtherCAT AMP count error, %s", sRet);
                    MNetLog(Str1);
//                    NewRecordProcess("", Str1);
                }
//                ShowErrorMessage("WAR16153", 0, MMSystem, 0, sRet);
            }
        }
    }

    if(bResetEtherCAT)
    {
        int ret=OpenEtherCatMastCard();
        if(ret!=SUCCESS)
        {
            ShowErrorMessage("WAR16150", 0, MMSystem, 0, IntToStr(ret));
        }
    }
    bCheckPCI_EtherCatStateRun=false;
    return 0;
    #else
    // GATE REGISTER (2): HAVE_PCI1203 not defined -- no EtherCAT master
    // vendor SDK linked; nothing to check. Mirrors golden's own
    // "not ready" early-return shape (golden :238-241/:243-246).
    bCheckPCI_EtherCatStateRun=false;
    return 0;
    #endif
    #endif
}
//------------------------------------------------------------------------------
bool INSTALL_ETHETCAT()                                                         //Sam 20230210 : 新增 VacuumUnit 通訊模組
{
    bool bRet=false;
    if(SHUTTLE_SENSOR_TYPE==eSensorEtherCAT  ||
       SHUTTLE_SENSOR_TYPE==eSensorEtherCAT3 ||
       VCCU_UNIT_TYPE==1)
    {
        bRet=true;
    }
    return bRet;
}
//------------------------------------------------------------------------------
//  OpenEtherCatMastCard -- Sam 20230707 : EtherCAT Shuttle sensor
//  golden EtherCAT/MyEtherCAT.cpp:421-488.  See file-banner GATE REGISTER
//  entry (2) [HAVE_PCI1203].
//------------------------------------------------------------------------------
int OpenEtherCatMastCard()
{
#if HAVE_PCI1203
    DEVLIST m_avaDevs[MAX_DEVICES];
    U32 Result, deviceCounter=0;
    AnsiString sErrorCode=0;
    Acm_DevClose(&uiDevhand);
    Result=Acm_GetAvailableDevs(m_avaDevs, MAX_DEVICES, &deviceCounter);
    if(Result!=SUCCESS)
    {
        sErrorCode=IntToHex((int)Result, 8);
        return atoi(sErrorCode.c_str());
    }

    m_dwDevNum=m_avaDevs[0].dwDeviceNum;
    int iOpenCardFailTimes=0;

    while(true)
    {
        if(iOpenCardFailTimes>2)                                                //開卡失敗
        {
            sErrorCode=IntToHex((int)Result, 8);
            return atoi(sErrorCode.c_str());
        }
        else
        {
            Result=Acm_DevOpen(m_dwDevNum, &uiDevhand);
            if(Result==SUCCESS)
            {
                break;
            }
            else
            {
                ReOpen:
                Result=Acm_DevReOpen(uiDevhand);
                if(Result!=EC_OpenMasterDevFailed)
                {
                    goto ReOpen;
                }
                Acm_DevClose(&uiDevhand);
                Sleep(2000);
                iOpenCardFailTimes++;
            }
        }
    }
    MySleep(500);
    U32 MasEnableEvt=0;
    MasEnableEvt|=EVT_DEV_DISCONNET;
    MasEnableEvt|=EVT_DEV_IO_DISCONNET;
    Result=Acm_DevEnableEvent(uiDevhand, MasEnableEvt);                         //啟用檢查事件
    if(Result!=SUCCESS)
    {
        sErrorCode=IntToHex((int)Result, 8);
        return atoi(sErrorCode.c_str());
    }
#else
    // GATE REGISTER (2): HAVE_PCI1203 not defined -- no EtherCAT master
    // vendor SDK linked. Faithful default: nothing to open; fall straight
    // through to the (always-real) MOT[] reinit loop below and report
    // SUCCESS(0), matching golden's own "card opened cleanly" terminal
    // return (golden :487).
#endif

    for(int i=0; i<TOTAL_MOTOR; i++)                                            //Rogeryang 20250411 EtherCAT Motor Reinitiate here
    {
        if(MOT[i].Motor!=NULL         &&
         MOT[i].CardType=="PCI1203" &&
         MOT[i].Motor->Enable)
        {
          MOT[i].Motor->ResetAxisOpen();
          MOT[i].Motor->InitMotor(0); //No Need Addr here
        }
    }

    return 0;
}
//------------------------------------------------------------------------------
