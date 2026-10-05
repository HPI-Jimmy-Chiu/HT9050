// =============================================================================
//  forms/fDTME08.cpp  --  definitions for the frmDTME08 facade
//
//  AI(W906-FW3-DTL1) 20260828: new file, FW wave FW3-DTL1.
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/EJ1N/fDTME08.cpp
//  (765 lines) + EJ1N/fDTME08.h (206 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 55 golden `TfrmDTME08::` member bodies, 693 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 55-member denominator:
//    22 ACTIVE           173 of 693 golden span lines (25.0%)
//     1 ACTIVE-PARTIAL   the ctor -- golden :19-27 is GATE (E-1); the
//                        delivered body is empty
//    32 GATED-WITH-BODY  507 golden span lines carried as `#if 0` transcript
//
//  See forms/fDTME08.h for the full GATE REGISTER (E-1)..(E-33), the LINK
//  BOUNDARY measurement that 30 of the 32 gates rest on, the ACTIVE evidence,
//  DEVIATIONS D-1..D-10, the FIELD LIST and -- most importantly -- the
//  ZERO-WRITER section explaining why `myPalGroup` is always empty and what
//  that means for anyone un-gating bthermo.cpp:5396-5399.
//
//  ⚠ AI(W906-I03) 20261002 (Ifor01): I-03 PHASE 2 OPENED EVERY GATE BELOW
//  (`#if 1 // was: #if 0 ... // GATE (E-n)`); they compile and ctest
//  DTME08Cycle runs them against a fake DTM.  The two paragraphs that follow
//  are the 20260828 record (what was true while they were closed).  See
//  forms/fDTME08.h's I-03 note for what replaced each blocker and for why
//  W906_DTME08_FormShowArm (end of this file) is not called yet.
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall` and applying DEVIATION D-4's
//  `TScrollBox` -> `TfrmDTME08ScrollBox` rename), so it is a faithful
//  TRANSCRIPT -- not verified code.  Many identifiers it names (edAddress,
//  edPort, rgSensorType, ledEJ1N1, btnCtrl, chkHeater, SW[], ATC_InterfaceForm,
//  HeaterSVLog, TColor/clBlack/clYellow, ...) are NOT members of this facade
//  and/or have no reachable definition; un-gating requires supplying them
//  first.
// =============================================================================
#include "forms/fDTME08.h"
#include "cmydef.h"             // USE_16_HEATER / N_PORT / N_DOUBLE / dTempMax (cmydef.cpp, ht9045_globals)
#include "MachineType.h"        // eht16HeaterDTME08/eht32HeaterDTME08 (:661-662) + ChangeToFloatNonPcnt (:1601, header template)
#include "forms/fQwertyKey.h"   // fQwertyKey->ShowQwertyKey (forms/fQwertyKey.cpp, ht9045_forms)
#include "vclcompat/SysUtils.h" // IntToStr (vclcompat)
#include "vclcompat/TDateTime.h" // FormatDateTime / Now (vclcompat)
// AI(W906-I03) 20261002 (Ifor01): I-03 phase 2 -- what the opened bodies use
#include "EJ1N/DtmChannelMap.h"  // AI(W906-I03b) 20261005 (Ifor01): DtmMap_Active / DtmMap_ChannelCount / DtmMap_SensorOf (ht9045_globals)
#include "EJ1N/uDTME08Control.h" // dtme08 (ht9045_comms; resolved by the RESCAN groups, fDTME08.h note)
#include "cpublic.h"             // HeaterSVLog (SetSettingSV; live since I-03 phase 1, cpublic.cpp:711)
#include "cprod.h"               // Temperature.bATCActiveCooling (TimerUpdateTimer)
#include "acarry_shims.h"        // ATC_InterfaceForm->iATC_MODE_TYPE (TimerUpdateTimer; the same include forms/fLotInfo.cpp uses)
#include "myswitch.h"            // SW[] (chkHeaterClick)
#include <windows.h>             // GetTickCount (W906_DTME08TimerTick)
#ifndef ATC_TYPE_60
#define ATC_TYPE_60 60           // golden ATC_Handler_Side.h; no shared-header home in the port (csystem.cpp:16963 note), TU-local as uHeaterThread.cpp
#endif

// AI(W906-FW3-DTL1) 20260828: TfrmDTME08/frmDTME08 were FREE tree-wide --
// same idiom as forms/fPrecaution.cpp:14 / forms/fHotPlate.cpp:24 /
// forms/fTesterTCP.cpp:49.  The DELIVERED ctor body is EMPTY (golden's whole
// conditional is GATE (E-1)), so this static-init `new` touches no global --
// no SIOF risk (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL 全域"; the
// fLaserSensor incident that rule comes from turned 88 of 134 ctest binaries
// into SEGFAULTs, and TfMonitor's ctor read D:\HT9045\system\MVData.ini
// before main()).  Golden's ctor would call Reload(), i.e. read
// DTME08_Control.ini before main() -- exactly that shape.
//
// NOTE, disclosed rather than avoided: the two by-value `TQPF_Timer` members
// DO run their own ctor here, and TQPF_Timer's ctor (myTimer.cpp:14-17) calls
// QueryPerformanceCounter 1001 times to calibrate its overhead.  That is a
// Win32 call touching none of this port's globals, so it is SIOF-safe; it
// costs ~2x1001 QPC calls at process start for every binary that links
// ht9045_forms.
TfrmDTME08 *frmDTME08 = new TfrmDTME08();

// ---------------------------------------------------------------------------
//  golden :16-28.  ACTIVE-PARTIAL: the delivered body is EMPTY.  Golden's
//  whole `if(USE_16_HEATER==eht16HeaterDTME08 || USE_16_HEATER==
//  eht32HeaterDTME08){ ... }` block is GATE (E-1) and appears as transcript
//  further down this file.
// ---------------------------------------------------------------------------
TfrmDTME08::TfrmDTME08()
{
    // GATE (E-1) -- golden :19-27.  Two independent reasons: (a) three of its
    // five callees are gated on the ht9045_comms link boundary
    // (NewuDTME08Control E-2 / Reload E-6 / AddTempGUI E-3), and (b) this
    // ctor runs BEFORE main(), so even the two ACTIVE callees must not run
    // here.  Scalar fields are NSDMI-initialised in the header; widget fields
    // are NSDMI-`new`'d there.
}

// ---------------------------------------------------------------------------
void TfrmDTME08::InitialData()   // golden :54-72
{
    iDoInitialCommandStatus=DoInitialNone;
    brecordmsgLock=bTimerUpdateLock=false;
    TargetCh=TargetStation=TargetNo=iSingleStationTask=iCycleRunNumber=iNowActiveStation=0;
    TimerUpdate->Enabled=false;

//AI(W906-I03b) 20261005 (Ifor01): DTM channel map active -> its stations x 8 (HT9050: 3 stations = 24); the station loop
//  (DoDTME08Cycle case 9000, golden :659) then stops after the last mapped station instead of golden's 2 or 4.
    if(DtmMap_Active())
        SetChannelNumber(DtmMap_ChannelCount());
    else
    if(USE_16_HEATER==eht16HeaterDTME08)                                        //Steven 20241113 : add for DTM 32CH
        SetChannelNumber(16);
    else if(USE_16_HEATER==eht32HeaterDTME08)
        SetChannelNumber(32);

    bShow=false;
    ClearAllCommandStatus();
    bCycleFirstRun=true;
    iTaskDTME08Cycle=1;
    iDoManualCommandStatus=DoNone;
    bCtrlSwitch=true;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::ShowMSG(AnsiString asStr)   // golden :88-98
{
    if(brecordmsgLock==false)
    {
        brecordmsgLock=true;
        if(meLogs->Lines->Count>50)
            meLogs->Lines->Clear();
        meLogs->Lines->Add(asStr);
        brecordmsgLock=false;
    }
}

// ---------------------------------------------------------------------------
void TfrmDTME08::RecordMsg(AnsiString asMsg)   // golden :100-104
{
    asMsg=FormatDateTime("yyyy-mm-dd HH:MM:ss ", Now())+asMsg;
    ShowMSG(asMsg);
}

// ---------------------------------------------------------------------------
void TfrmDTME08::RecordErrorMsg(AnsiString asMsg)   // golden :106-110
{
    asErrorDTME08=asMsg;
    RecordMsg("[Error]"+asMsg);
}

// ---------------------------------------------------------------------------
void TfrmDTME08::AddTempPanel(TMyOmronPanel* tmop)   // golden :131-134
{
    myPalGroup[GetSize()]=tmop;
}

// ---------------------------------------------------------------------------
//  golden :136-140.  ACTIVE, PRE-COMMITTED (bthermo.cpp:5398).
//  ⚠ ZERO-WRITER: `myPalGroup` is only ever filled through the GATED
//  AddSingleView (E-7), so as delivered the map is EMPTY, `index` always
//  clamps to 0, and `myPalGroup[0]` INSERTS a null entry and returns nullptr.
//  That is golden's own std::map::operator[] semantics carried faithfully.
//  See forms/fDTME08.h's ZERO-WRITER section before un-gating bthermo.
// ---------------------------------------------------------------------------
TMyOmronPanel* TfrmDTME08::GetPalGroup(int index)
{
    index=(index<0 || index>=GetSize())?0:index;
    return myPalGroup[index];
}

// ---------------------------------------------------------------------------
void TfrmDTME08::InitialComboList()   // golden :147-155
{
    cbCh->Clear();
    for(int i=0; i<GetChannelNumber(); i++)
    {
        cbCh->Items->Add("CH"+IntToStr(i+1));
    }
    cbCh->ItemIndex=0;
}

// ---------------------------------------------------------------------------
//  golden :178-191.  ACTIVE.  `DTMTimeout` is a TQPF_Timer (myTimer.h /
//  myTimer.cpp = ht9045_globals, which ht9045_forms links).
//  ⚠ Golden's own oddity, kept: the message says "time out 10 Sec" while the
//  timeout actually in force is whatever the last SetSecAndOn() set.
// ---------------------------------------------------------------------------
bool TfrmDTME08::GetErrorMsg()
{
    if(asErrorDTME08!="")
    {
        RecordErrorMsg(asErrorDTME08);
        return true;
    }
    else if(DTMTimeout.Off())
    {
        RecordErrorMsg("time out 10 Sec");
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::btSetThermoTypeClick(TObject *Sender)   // golden :272-275
{
    (void)Sender;
    iDoManualCommandStatus=DoSetSeonsorType;
}

// ---------------------------------------------------------------------------
//  golden :330-334.  DEVIATION D-3: golden's (TObject*, TCloseAction&)
//  parameters are dropped -- TCloseAction has no port and the body reads
//  neither.
// ---------------------------------------------------------------------------
void TfrmDTME08::FormClose()
{
    bShow=false;
}

// ---------------------------------------------------------------------------
//  golden :443-450.  ACTIVE.
//  ⚠ Golden's own shape, kept verbatim: the `if(bCycleFirstRun==false)` has
//  NO braces, so it guards ONLY the `iDoInitialCommandStatus=DoInitialNone;`
//  line -- the other three assignments run unconditionally.  Golden's
//  indentation (the guarded line is NOT indented) suggests a missing brace,
//  but changing it would change behaviour, which is a user decision.
// ---------------------------------------------------------------------------
void TfrmDTME08::ClearAllCommandStatus()
{
    if(bCycleFirstRun==false)
    iDoInitialCommandStatus=DoInitialNone;
    iDoCommandStatus=DoNone;
    iDoAutoCommandStatus=DoNone;
    iNowActiveStation=0;
}

// ---------------------------------------------------------------------------
//  golden :462-472.  ACTIVE: `GetSettingSV()` / `GetSV()` are BOTH INLINE on
//  TMyOmronPanel (EJ1N/MyOmronPanel.h), so no ht9045_comms symbol is
//  referenced.  ⚠ Returns false always as delivered -- GetSize() is 0, see
//  ZERO-WRITER.
// ---------------------------------------------------------------------------
bool TfrmDTME08::IsNeedSetSV()
{
    for(int i=0; i<GetSize(); i++)
    {
        if(i>=GetChannelNumber())
            return false;
        if(GetPalGroup(i)->GetSettingSV()!=GetPalGroup(i)->GetSV())
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//  golden :474.  File-scope const (internal linkage) -- outside census's
//  55-member denominator.  Only the GATED DoDTME08Cycle reads it; translated
//  anyway so the transcript below stays literal.  See DEVIATION (D-8) for why
//  golden's sibling `bool bFirstTime;` (:157) is deliberately NOT translated.
// ---------------------------------------------------------------------------
const int iAutoCycleCount=10;

// ---------------------------------------------------------------------------
//  golden :678-685.  ACTIVE: `SetAT(true)` resolves to TMyOmronPanel's
//  INLINE `SetAT(bool)` overload -- contrast GATE (E-24), whose
//  `SetAT("AT", clYellow)` resolves to the OUT-OF-LINE
//  `SetAT(AnsiString,TColor)`.
// ---------------------------------------------------------------------------
void TfrmDTME08::btATClick(TObject *Sender)
{
    (void)Sender;
    for(int i=0; i<GetSize(); i++)
    {
        GetPalGroup(i)->SetAT(true);
    }
    iDoManualCommandStatus=DoSetAT;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::btSetSVClick(TObject *Sender)   // golden :687-690
{
    (void)Sender;
    iDoManualCommandStatus=DoSetSV;
}

// ---------------------------------------------------------------------------
//  golden :692-695.  ⚠ Golden's body is identical to btATClick's tail -- the
//  "1 by 1" in the name has no distinct behaviour in golden.  Verbatim.
// ---------------------------------------------------------------------------
void TfrmDTME08::btAT1by1Click(TObject *Sender)
{
    (void)Sender;
    iDoManualCommandStatus=DoSetAT;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::btATOffClick(TObject *Sender)   // golden :697-704
{
    (void)Sender;
    for(int i=0; i<GetSize(); i++)
    {
        GetPalGroup(i)->SetAT(false);
    }
    iDoManualCommandStatus=DoSetAT;
}

// ---------------------------------------------------------------------------
//  golden :706-709.  ⚠ Writes `iDoAutoCommandStatus`, whereas the visually
//  parallel btSetThermoTypeClick (:272-275) writes `iDoManualCommandStatus`
//  for the same command.  Golden's asymmetry, kept.
// ---------------------------------------------------------------------------
void TfrmDTME08::btnSetSensorClick(TObject *Sender)
{
    (void)Sender;
    iDoAutoCommandStatus=DoSetSeonsorType;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::btnCtrlClick(TObject *Sender)   // golden :711-714
{
    (void)Sender;
    bCtrlSwitch=!bCtrlSwitch;
}

// ---------------------------------------------------------------------------
//  golden :716-723.  ACTIVE: `edSV` is a public `vclcompat::TEdit*` FIELD of
//  TMyOmronPanel and `Text` is a plain AnsiString member, so no
//  ht9045_comms symbol is referenced.  Zero iterations as delivered.
// ---------------------------------------------------------------------------
void TfrmDTME08::btnSetSVFollowCh1Click(TObject *Sender)
{
    (void)Sender;
    for(int i=0; i<GetSize(); i++)
    {
        GetPalGroup(i)->edSV->Text=GetPalGroup(0)->edSV->Text;
    }
    iDoManualCommandStatus=DoSetSV;
}

// ---------------------------------------------------------------------------
void TfrmDTME08::edPortClick(TObject *Sender)   // golden :730-733
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_PORT);
}

// ---------------------------------------------------------------------------
//  golden :735-738.  ⚠ Golden passes `dTempMax` as ShowQwertyKey's `min`
//  argument and `0.0` as its `max` (forms/fQwertyKey.h:370 declares
//  `..., double min=0, double max=0`), i.e. the range reads inverted.
//  Transcribed verbatim; flagged rather than "corrected".
// ---------------------------------------------------------------------------
void TfrmDTME08::edSVClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 1, true, dTempMax, 0.0);
}

// ---------------------------------------------------------------------------
//  golden :740-764.  ACTIVE.  `ScrollBox1->Width` is the facade-local
//  stand-in (DEVIATION D-4, value 859 from the .dfm -- read that note, a
//  0 there would make `i%numOfCol` a division by zero);
//  `GetPalGroup(iMem)->GroupBox->Left/Top` are public
//  `vclcompat::TGroupBox*` fields; `ChangeToFloatNonPcnt` is a header
//  template (MachineType.h:1601), so no link edge is involved.
//  Zero iterations as delivered (GetSize()==0, see ZERO-WRITER).
// ---------------------------------------------------------------------------
void TfrmDTME08::SetPanelPos()                                                  //Jimmychiu 20230628 : DTME08物件排列
{
    int numOfCol=ScrollBox1->Width/210;
    //排列方式:
    // 1-1, 1-3, 2-1, 2-3
    // 1-2, 1-4, 2-2, 2-4
    int iPanelSort[32]=
    {
        0,  2,  4,  6,
        1,  3,  5,  7,
        8, 10, 12, 14,
        9, 11, 13, 15,
       16, 18, 20, 22,
       17, 19, 21, 23,
       24, 26, 28, 30,
       25, 27, 29, 31,
    };
    int iMem=0;
    for(int i=0; i<GetSize(); i++)
    {
        iMem=iPanelSort[i];
        GetPalGroup(iMem)->GroupBox->Left     = 210*(i%numOfCol);
        GetPalGroup(iMem)->GroupBox->Top      = 124*(ChangeToFloatNonPcnt((double)(i), (double)(numOfCol)));
    }
}

// ---------------------------------------------------------------------------
//  AI(W906-I03) 20261002 (Ifor01): golden passes `dtme08->DoGetPV` (a Borland
//  __closure = the object + the member) to SetSingleCommand.  W906_DoCmd binds
//  the same pair; dtme08 never changes after NewuDTME08Control, so binding the
//  pointer value is what golden's closure holds.
// ---------------------------------------------------------------------------
static std::function<bool(bool,int)> W906_DoCmd(uDTME08Control* pCtl, bool (uDTME08Control::*pfn)(bool,int))
{
    return [pCtl, pfn](bool bIsFirst, int iStation){ return (pCtl->*pfn)(bIsFirst, iStation); };
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  AI(W906-I03) 20261002 (Ifor01): ALL OPEN since I-03 phase 2 -- the
//  per-gate notes below are the 20260828 reasons, kept as history.
//  See forms/fDTME08.h for the per-entry reasoning, grouped by cause.
// ===========================================================================

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-1) -- golden :19-27, the ctor's conditional.
      // (a) three of its five callees are gated on the ht9045_comms link
      //     boundary (NewuDTME08Control E-2 / Reload E-6 / AddTempGUI E-3);
      // (b) static-init safety -- this facade is created by a file-scope
      //     `new`, so this would run before main().  Reload() reads
      //     DTME08_Control.ini; InitialComboList()/InitialData() are ACTIVE
      //     but must not run at that point either.
      // AI(W906-I03) 20261002 (Ifor01): (a) answered by the RESCAN link, (b) by
      // running this body from W906_DTME08_CreateFormBoot (end of this file),
      // which wb_serve calls at golden CreateForm(TfrmDTME08) HT9045.cpp:272.
void TfrmDTME08::W906_CreateFormBody()
{
    if(USE_16_HEATER==eht16HeaterDTME08  ||                                     //Steven 20221201 : DTM08沒用到不要進來
       USE_16_HEATER==eht32HeaterDTME08  ||
       DtmMap_Active())                                                         //AI(W906-I03b) 20261005 (Ifor01): DTM channel map (HT9050: every zone on DTM)
    {
        NewuDTME08Control();
        Reload();
        InitialData();
        AddTempGUI();
        InitialComboList();
//AI(W906-I03b) 20261005 (Ifor01): with the map, every panel boots with setting SV 0 instead of golden's edSV "30"
//  (AddSingleView golden :120): the first cycle writes the panels' SV to the controller (InitialDoSetAllSV), and on an HT9050
//  that reaches Hotplate / Chamber / the unconfirmed SLK heaters, not golden's Index sockets.  bthermo's DoThermoReal pushes
//  the real setpoints (W906_DtmMapExchange) one channel per visit.
        if(DtmMap_Active())
            for(int i=0; i<GetChannelNumber(); i++)
                GetPalGroup(i)->SetSettingSV(0.0);
    }
}
#endif // GATE (E-1)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-2) NewuDTME08Control -- golden :30-36.  TWO GATES:
      // (a) LINK BOUNDARY -- `new uDTME08Control()` and every member of it
      //     live in EJ1N/uDTME08Control.cpp, add_library(ht9045_comms);
      //     ht9045_forms links only vclcompat + ht9045_globals + ht9045_core.
      // (b) MISSING TYPE -- `dtme08->RecordErrorMsg=RecordErrorMsg;` assigns
      //     a BOUND MEMBER to a Borland `__closure` field; DEVIATION D-6's
      //     plain function pointer cannot express that.
void TfrmDTME08::NewuDTME08Control()
{
    if(dtme08==NULL)
        dtme08=new uDTME08Control();
    dtme08->RecordErrorMsg=[this](AnsiString asMsg){ RecordErrorMsg(asMsg); };   //AI(W906-I03) 20261002 (Ifor01): golden assigns the bound member (__closure); TPointVoidShowMsg is std::function (EJ1N/uDTME08Control.h)
    dtme08->RecordMsg=[this](AnsiString asMsg){ RecordMsg(asMsg); };             //AI(W906-I03) 20261002 (Ifor01): same
}
#endif // GATE (E-2)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-3) AddTempGUI -- golden :38-44.  LINK BOUNDARY (dtme08).
      // Also TRANSITIVE on AddSingleView (E-7).
void TfrmDTME08::AddTempGUI()
{
    for(int i=0; i<dtme08->GetMaxStationNumber()*dtme08->GetChannelNumberPerStation(); i++)
    {
        AddSingleView(ScrollBox1);
    }
}
#endif // GATE (E-3)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-4) DrawTempGUI -- golden :46-52.  LINK BOUNDARY:
      // `TMyOmronPanel::SetVisible(bool)` is OUT-OF-LINE
      // (EJ1N/MyOmronPanel.cpp, ht9045_comms).  Everything else here --
      // GetSize(), GetPalGroup(), GetChannelNumber() -- is this wave's own
      // ACTIVE code, which makes this the cheapest un-gate in the file after
      // the comms boundary is solved.
void TfrmDTME08::DrawTempGUI()
{
    for(int i=0; i<GetSize(); i++)
    {
        GetPalGroup(i)->SetVisible((i<GetChannelNumber()));
    }
}
#endif // GATE (E-4)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-5) SaveData -- golden :74-79.  TWO GATES: LINK BOUNDARY
      // (dtme08, ht9045_comms) and WRITES DISK
      // (`WriteSettingFile()` persists DTME08_Control.ini -- see
      // EJ1N/uDTME08Control.h:144 for the filename).
void TfrmDTME08::SaveData()
{
    dtme08->SetSocketAddress(edAddress->Text);
    dtme08->SetSocketPort(edPort->Text);
    dtme08->WriteSettingFile();
}
#endif // GATE (E-5)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-6) Reload -- golden :81-86.  LINK BOUNDARY (dtme08).
      // ⚠ Golden calls this from the ctor, i.e. it reads a settings file at
      // form-construction time; in this port that would be BEFORE main().
      // See GATE (E-1)(b).
void TfrmDTME08::Reload()
{
    dtme08->ReadSettingFile();
    edAddress->Text=dtme08->GetSocketAddress();
    edPort->Text=dtme08->GetSocketPort();
}
#endif // GATE (E-6)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-7) AddSingleView -- golden :112-129.  THREE GATES:
      // (a) LINK BOUNDARY -- `new TMyOmronPanel(this)` is the OUT-OF-LINE
      //     ctor, and SetSP/SetEvent/SetPV/SetRunStop/SetDegree are all
      //     out-of-line members (EJ1N/MyOmronPanel.cpp, ht9045_comms).
      // (b) DEVIATION D-2 -- `this` cannot be passed as a `TComponent*`
      //     owner; this facade has no base class.
      // (c) DEVIATION D-4 -- golden's parameter is `TScrollBox*`; renamed
      //     here to the facade-local stand-in.
      // Also needs TColor/clBlack/clYellow (vclcompat/LedCore.h, reachable)
      // and IntToStr (reachable) -- neither is the blocker.
void TfrmDTME08::AddSingleView(TfrmDTME08ScrollBox *ScrollBox1)
{
    TMyOmronPanel* panel      = new TMyOmronPanel(NULL);   //AI(W906-I03) 20261002 (Ifor01): golden owner `this` (the form frees it at exit); the facade has no TComponent base (D-2) -- the 32 panels live as long as the process, as golden's form does
    int numOfCol=ScrollBox1->Width/210;
    //panel->GroupBox->Parent   = ScrollBox1;   //AI(W906-I03) 20261002 (Ifor01): display parent only -- vclcompat TControl has no Parent (no window); Caption / Visible / Left / Top below are kept
    panel->GroupBox->Caption  = "CH"+IntToStr(GetSize()+1);
    panel->GroupBox->Visible  = true;
    panel->iMyTag             = GetSize();
    panel->edSV->Text         = AnsiString(30);
    panel->GroupBox->Left     = 210*(GetSize()%numOfCol);
    panel->GroupBox->Top      = 124*(ChangeToFloatNonPcnt((double)(GetSize()), (double)(numOfCol)));
    panel->SetSP("          ", TColor(0x004080FF));
    panel->SetEvent("Event", clBlack);                                          //警報事件
    panel->SetPV("          ", clYellow);
    panel->SetRunStop("STOP", clBlack);
    panel->SetDegree("℃", clYellow);                                           //顯示量測單位
    AddTempPanel(panel);
}
#endif // GATE (E-7)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-8) btnSetSaveClick -- golden :142-145.  TRANSITIVE on
      // SaveData (E-5).
void TfrmDTME08::btnSetSaveClick(TObject *Sender)
{
    SaveData();
}
#endif // GATE (E-8)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-9) TimerUpdateTimer -- golden :158-176.  TWO GATES:
      // (a) LINK BOUNDARY -- `ATC_InterfaceForm->iATC_MODE_TYPE`;
      //     ATC/ATCInterface.cpp is add_library(ht9045_sm).
      // (b) TRANSITIVE on DoDTME08Cycle (E-32).
      // Everything else it reads IS reachable: InitialOK (cmydef.cpp:285),
      // Temperature.bATCActiveCooling (cprod.cpp:27), ATC_SYSTEM
      // (cmydef.cpp:3567), eATC60/eNewATCSystem/ATC_TYPE_60.
      // ⚠ golden's file-scope `bool bFirstTime;` sat immediately above this
      // body (:157) and is DEAD in golden -- deliberately not translated,
      // see DEVIATION (D-8).
void TfrmDTME08::TimerUpdateTimer(TObject *Sender)
{
    if(InitialOK==false)
        return;

    if(Temperature.bATCActiveCooling ||                                                 //Jimmychiu 20230628 : ATC6.0不需要跑Omron DTME08
       ATC_SYSTEM==eATC60 ||
       (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60))   //Steven 20201120 : 修正ATC6.0不需要跑Omron EJ1N
        return;

    if(bTimerUpdateLock)
        return;
    else
        bTimerUpdateLock=true;
    //
    DoDTME08Cycle();
    //
    bTimerUpdateLock=false;
}
#endif // GATE (E-9)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-10) btnGetPVClick -- golden :193-197.  LINK BOUNDARY
      // (dtme08->DoGetPV, ht9045_comms).  Also TRANSITIVE on SetSingleCommand
      // (E-17), whose parameter type is the __closure typedef (D-6).
void TfrmDTME08::btnGetPVClick(TObject *Sender)
{
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetPV), GetChannelIndex());   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetPV (__closure)
    iDoCommandStatus=DoGetPV;
}
#endif // GATE (E-10)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-11) btnSetATClick -- golden :199-205.  LINK BOUNDARY
      // (dtme08->DoSetAT / InitialmapDTMInfoList / mapDTMInfo).
      // `GetPalGroup(TargetCh)->cbEnableAT->Checked` alone would be fine
      // (public vclcompat field).
void TfrmDTME08::btnSetATClick(TObject *Sender)
{
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetAT), GetChannelIndex());   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetAT (__closure)
    dtme08->InitialmapDTMInfoList();
    dtme08->mapDTMInfo[TargetNo].RunAutoTunning=GetPalGroup(TargetCh)->cbEnableAT->Checked;
    iDoCommandStatus=DoSetAT;
}
#endif // GATE (E-11)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-12) btnGetSVClick -- golden :207-211.  LINK BOUNDARY (dtme08).
void TfrmDTME08::btnGetSVClick(TObject *Sender)
{
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetSV), GetChannelIndex());   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetSV (__closure)
    iDoCommandStatus=DoGetSV;
}
#endif // GATE (E-12)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-13) btnSetSVClick -- golden :213-221.  LINK BOUNDARY (dtme08).
      // ⚠ SAFETY NOTE for a future write-path wave: this is one of the two
      // manual paths that ultimately WRITE a setpoint to the controller (the
      // other is the auto path through DoDTME08Cycle/InitialDoSetAllSV).
void TfrmDTME08::btnSetSVClick(TObject *Sender)
{
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetSV), GetChannelIndex());   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetSV (__closure)
    dtme08->InitialmapDTMInfoList();
    AnsiString Str=GetPalGroup(TargetCh)->edSV->Text;
    double dTargetSV=atof(Str.c_str());
    dtme08->mapDTMInfo[TargetNo].SetSettingSV(dTargetSV);
    iDoCommandStatus=DoSetSV;
}
#endif // GATE (E-13)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-14) btnGetStatusClick -- golden :223-227.  LINK BOUNDARY.
void TfrmDTME08::btnGetStatusClick(TObject *Sender)
{
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetStatus), GetChannelIndex());   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetStatus (__closure)
    iDoCommandStatus=DoGetStatus;
}
#endif // GATE (E-14)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-15) DoSendCommandByManual -- golden :229-250.  LINK BOUNDARY
      // (`dtme08->DoCommuncation()` -- golden's spelling of "Communication",
      // kept) plus the `tpvDoCommand(...)` closure call (D-6).
bool TfrmDTME08::DoSendCommandByManual(bool bIsFirst, int iStation)
{
    if(dtme08->DoCommuncation())
    {
        if(bIsFirst)
        {
            asErrorDTME08="";
            tpvDoCommand(true, iStation);
            return true;
        }
        else if(tpvDoCommand(false, iStation))
        {
            return true;
        }
    }

    if(GetErrorMsg())
    {
        return true;
    }
    return false;
}
#endif // GATE (E-15)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-16) GetTargetCh -- golden :252-256.  TRANSITIVE on
      // GetStationAndNumber (E-18).  Its own first line is an own field.
void TfrmDTME08::GetTargetCh(int ich)
{
    TargetCh=ich;
    GetStationAndNumber(TargetCh, TargetStation, TargetNo);
}
#endif // GATE (E-16)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-17) SetSingleCommand -- golden :258-264.  TWO GATES:
      // (a) MISSING TYPE -- the `TPointVoidDoCommand` parameter is golden's
      //     `__closure` typedef (DEVIATION D-6);
      // (b) TRANSITIVE on GetTargetCh (E-16).
      // `DTMTimeout.SetSecAndOn(10)` alone would be reachable (TQPF_Timer is
      // ht9045_globals).
void TfrmDTME08::SetSingleCommand(TPointVoidDoCommand tcdc, int ich)
{
    tpvDoCommand=tcdc;
    iTaskCommand=1;
    DTMTimeout.SetSecAndOn(10);
    GetTargetCh(ich);
}
#endif // GATE (E-17)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-18) GetStationAndNumber -- golden :266-270.  LINK BOUNDARY
      // (dtme08->GetChannelNumberPerStation).  ChangeToFloatNonPcnt is a
      // header template and is NOT the blocker.
void TfrmDTME08::GetStationAndNumber(int iCh, int &iStation, int &iNumber)
{
    iStation=ChangeToFloatNonPcnt((double)(iCh), (double)(dtme08->GetChannelNumberPerStation()));
    iNumber =iCh%dtme08->GetChannelNumberPerStation();
}
#endif // GATE (E-18)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-19) btnReloadClick -- golden :277-280.  TRANSITIVE on
      // Reload (E-6).
void TfrmDTME08::btnReloadClick(TObject *Sender)
{
    Reload();
}
#endif // GATE (E-19)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-20) DoManualProcess -- golden :282-320.  LINK BOUNDARY
      // (dtme08->mapDTMInfo) plus TRANSITIVE on DoSendCommandByManual (E-15),
      // SetSP (E-22) and SetPV (E-23).
bool TfrmDTME08::DoManualProcess()
{
    if(iDoCommandStatus>0)
    {
        switch(iTaskCommand)
        {
            case 1:
                if(DoSendCommandByManual(true, TargetStation))
                {
                    iTaskCommand=10;
                }
                break;
            case 10:
                if(DoSendCommandByManual(false, TargetStation))
                {
                    if(iDoCommandStatus==DoGetSV)
                    {
                        SetSP(TargetCh, dtme08->mapDTMInfo[TargetNo].SV, dtme08->mapDTMInfo[TargetNo].Event);
                    }
                    else if(iDoCommandStatus==DoGetPV)
                    {
                        SetPV(TargetCh, dtme08->mapDTMInfo[TargetNo].PV, dtme08->mapDTMInfo[TargetNo].Event);
                    }
                    else if(iDoCommandStatus==DoGetStatus)
                    {
                        //
                    }
                    iTaskCommand=20;
                }
                break;
            case 20://finish
                iDoCommandStatus=DoNone;
                iTaskCommand=1;
                return false;
        }
        return true;
    }
    return false;
}
#endif // GATE (E-20)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-21) FormShow -- golden :322-328.  TRANSITIVE on DrawTempGUI
      // (E-4) ONLY: `bShow=true;`, `InitialComboList()` and `SetPanelPos()`
      // are all this wave's own ACTIVE deliveries.  Cheapest future un-gate
      // in the file.
void TfrmDTME08::FormShow(TObject *Sender)
{
    bShow=true;
    DrawTempGUI();
    InitialComboList();
    SetPanelPos();                                                              //Jimmychiu 20230628 : DTME08物件排列
}
#endif // GATE (E-21)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-22) SetSP -- golden :336-339.  LINK BOUNDARY:
      // `TMyOmronPanel::SetSV(double)` is OUT-OF-LINE (ht9045_comms).
      // ⚠ golden ignores its own `Event` parameter here (SetPV, E-23, does
      // use it).  Verbatim.
void TfrmDTME08::SetSP(int iCh,double SV,AnsiString Event)
{
    GetPalGroup(iCh)->SetSV(SV);
}
#endif // GATE (E-22)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-23) SetPV -- golden :341-345.  LINK BOUNDARY:
      // `SetPV(AnsiString,TColor)` and `SetEvent(AnsiString,TColor)` are
      // OUT-OF-LINE (ht9045_comms).
void TfrmDTME08::SetPV(int iCh,double PV,AnsiString Event)
{
    GetPalGroup(iCh)->SetPV(AnsiString().sprintf("%5.1f", PV), clYellow);
    GetPalGroup(iCh)->SetEvent(Event,(Event=="Event")?clBlack:vclcompat::clRed);           //警報事件   //AI(W906-I03) 20261002 (Ifor01): clRed qualified -- EJ1N/MyOmronPanel.h:171-172 brings in only clBlack / clYellow
}
#endif // GATE (E-23)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-24) SetATStatus -- golden :347-350.  LINK BOUNDARY:
      // `SetAT(AnsiString, TColor)` is the OUT-OF-LINE overload.
      // ⚠ Contrast the ACTIVE btATClick/btATOffClick above, which call the
      // INLINE `SetAT(bool)` overload.  Same name, opposite verdict -- this
      // is the subtlest classification in the file.
void TfrmDTME08::SetATStatus(int iCh,bool bOn)
{
    GetPalGroup(iCh)->SetAT("AT", (bOn)?clYellow:clBlack);                      //顯示AT是否啟動
}
#endif // GATE (E-24)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-25) InitialDoSetSensorType -- golden :352-369.  LINK BOUNDARY
      // (dtme08->mapDTMInfo / esstPT100 / esstKType / DoSetSensorType).
      // ⚠ golden's own bug, kept: the loop writes `TargetCh` from `iStation`
      // but indexes `mapDTMInfo[i]`, not `mapDTMInfo[TargetCh]` -- and
      // `TargetCh` is then thrown away by the SetSingleCommand() call below.
      // SAFETY: this WRITES the sensor type into the controller.
void TfrmDTME08::InitialDoSetSensorType(int iStation)
{
    for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
    {
        TargetCh=dtme08->GetChannelNumberPerStation()*iStation+i;
        dtme08->mapDTMInfo[i].SensorType=dtme08->esstPT100;
        if(rgSensorType->ItemIndex==0)                                          // PT Type
        {
            dtme08->mapDTMInfo[i].SensorType=dtme08->esstPT100;
        }
        else
        {
            dtme08->mapDTMInfo[i].SensorType=dtme08->esstKType;
        }
//AI(W906-I03b) 20261005 (Ifor01): DTM channel map -> the type of THIS channel (golden's one radio group sets the whole
//  panel; HT9050 station 2 is a DTMN08 with Chamber PT100 + Hot Air 1/2 K-type).  Empty channels: PT100.
        if(DtmMap_Active())
            dtme08->mapDTMInfo[i].SensorType=(DtmMap_SensorOf(TargetCh)==edmsKType)?dtme08->esstKType:dtme08->esstPT100;
    }
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetSensorType));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetSensorType (__closure)
    DTMTimeout.SetSecAndOn(GetDTMTimeout());
}
#endif // GATE (E-25)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-26) InitialDoSetAllAT -- golden :371-380.  LINK BOUNDARY.
      // SAFETY: starts auto-tuning on the controller.
void TfrmDTME08::InitialDoSetAllAT(int iStation)
{
    for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
    {
        TargetCh=dtme08->GetChannelNumberPerStation()*iStation+i;
        dtme08->mapDTMInfo[i].RunAutoTunning=GetPalGroup(TargetCh)->cbEnableAT->Checked;
    }
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetAT));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetAT (__closure)
    DTMTimeout.SetSecAndOn(GetDTMTimeout());
}
#endif // GATE (E-26)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-27) InitialDoSetAllSV -- golden :382-395.  LINK BOUNDARY.
      // SAFETY: this is the auto path that WRITES SETPOINTS to every channel
      // of a station.
void TfrmDTME08::InitialDoSetAllSV(int iStation)
{
    AnsiString Str="";
    double dTargetSV=0;
    for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
    {
        TargetCh=dtme08->GetChannelNumberPerStation()*iStation+i;
        Str=GetPalGroup(TargetCh)->edSV->Text;
        dTargetSV=atof(Str.c_str());
        dtme08->mapDTMInfo[i].SetSettingSV(dTargetSV);
    }
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetSV));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetSV (__closure)
    DTMTimeout.SetSecAndOn(GetDTMTimeout());
}
#endif // GATE (E-27)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-28) InitialDoSetCycleTime -- golden :397-406.  LINK BOUNDARY.
void TfrmDTME08::InitialDoSetCycleTime(int iStation)
{
    for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
    {
        TargetCh=dtme08->GetChannelNumberPerStation()*iStation+i;
        dtme08->mapDTMInfo[i].SetCycleTime(GetCycleTime());
    }
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetCycleTime));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetCycleTime (__closure)
    DTMTimeout.SetSecAndOn(GetDTMTimeout());
}
#endif // GATE (E-28)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-29) InitialDoSetControlActionOut2 -- golden :408-417.
      // LINK BOUNDARY (dtme08->ecaDisable / DoSetControlActionOut2).
void TfrmDTME08::InitialDoSetControlActionOut2(int iStation)
{
    for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
    {
        TargetCh=dtme08->GetChannelNumberPerStation()*iStation+i;
        dtme08->mapDTMInfo[i].ControlActionOut2=dtme08->ecaDisable;
    }
    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoSetControlActionOut2));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoSetControlActionOut2 (__closure)
    DTMTimeout.SetSecAndOn(GetDTMTimeout());
}
#endif // GATE (E-29)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-30) InitialSetting -- golden :419-441.  TRANSITIVE on the
      // five InitialDoSet* bodies (E-25..E-29).
void TfrmDTME08::InitialSetting(int iStation)
{
    if(iDoAutoCommandStatus==DoSetAT)
    {
        InitialDoSetAllAT(iStation);
    }
    else if(iDoAutoCommandStatus==DoSetSV)
    {
        InitialDoSetAllSV(iStation);
    }
    else if(iDoAutoCommandStatus==DoSetSeonsorType)
    {
        InitialDoSetSensorType(iStation);
    }
    else if(iDoAutoCommandStatus==DoSetCycleTime)
    {
        InitialDoSetCycleTime(iStation);
    }
    else if(iDoAutoCommandStatus==DoSetControlActionOut2)
    {
        InitialDoSetControlActionOut2(iStation);
    }
}
#endif // GATE (E-30)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-31) SetSettingSV -- golden :452-460.  TWO INDEPENDENT GATES:
      // (a) `TMyOmronPanel::SetSettingSV(double)` is OUT-OF-LINE
      //     (EJ1N/MyOmronPanel.cpp, ht9045_comms).  Note `GetSV()` on the
      //     same line IS inline and would have been fine.
      // (b) `HeaterSVLog(int,double)` has NO REACHABLE DEFINITION: its only
      //     definition, cpublic.cpp:711, sits inside that file's own `#if 0`
      //     (opened at cpublic.cpp:709), whose note at cpublic.cpp:694 says
      //     "HeaterSVLog just after stays gated: nothing references it."
      //     A definition inside `#if 0` is not a definition.  (b) survives
      //     even if forms ever links ht9045_comms.
      // `iTempCode[]` IS reachable (cmydef.cpp:111).
      // PRE-COMMITTED name and signature (bthermo.cpp:5397).
void TfrmDTME08::SetSettingSV(int iCh, double dTemp)
{
    bool bdiff=!(GetPalGroup(iCh)->GetSV()==dTemp);
    if(bdiff)
    {
        HeaterSVLog(iTempCode[iCh], dTemp);                                     //Steven 20250116 : add SV change log
        GetPalGroup(iCh)->SetSettingSV(dTemp);
    }
}
#endif // GATE (E-31)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-32) DoDTME08Cycle -- golden :475-676 (202L, 29% of this file).
      // LINK BOUNDARY on dtme08 throughout, plus `ledEJ1N1` (golden
      // `TMyLed*`, no vclcompat port -- three unrelated `class TMyLed`
      // definitions exist elsewhere, DEVIATION D-7) and `btnCtrl->Caption`
      // (widget not declared, DEVIATION D-10).  TRANSITIVE on
      // DoManualProcess (E-20), InitialSetting (E-30),
      // DoSendCommandByManual (E-15), SetSP/SetPV/SetATStatus (E-22..E-24).
      // SAFETY: this is the whole autonomous polling+setpoint loop.
void TfrmDTME08::DoDTME08Cycle()
{
    int &Task=iTaskDTME08Cycle;
    //ctrl
    if(bCtrlSwitch)
    {
        btnCtrl->Caption="Running";
    }
    else
    {
        btnCtrl->Caption="Stop";
        return;
    }
    //
    if(DTMReconnectTime.Off())
    {
        if(dtme08->DoCommuncation())
        {
            if(ledEJ1N1->Value==false)
                ledEJ1N1->Value=true;
        }
        else
        {
            if(ledEJ1N1->Value==true)
                ledEJ1N1->Value=false;
            DTMReconnectTime.SetSecAndOn(GetReconnectTime());
            return;
        }
    }
    else
    {
        return;
    }
    //
    switch(Task)
    {
        case 1:
            if(bCycleFirstRun)
            {
                if(iDoInitialCommandStatus>=DoInitialTotal)
                {
                    DTMTimeout.SetSecAndOn(GetDTMTimeout());
                    bCycleFirstRun=false;
                }
                else if(iDoInitialCommandStatus==DoInitialSetTypeSeonsor)
                {
                    iDoAutoCommandStatus=DoSetSeonsorType;
                    Task=100;
                }
                else if(iDoInitialCommandStatus==DoInitialSetCycleTime)
                {
                    iDoAutoCommandStatus=DoSetCycleTime;
                    Task=100;
                }
                else if(iDoInitialCommandStatus==DoInitialSetControlActionOut2)
                {
                    iDoAutoCommandStatus=DoSetControlActionOut2;
                    Task=100;
                }
                else if(iDoInitialCommandStatus==DoInitialSetSV)
                {
                    iDoAutoCommandStatus=DoSetSV;
                    Task=100;
                }
                else
                {
                    DTMTimeout.SetSecAndOn(GetDTMTimeout());
                }
                iDoInitialCommandStatus++;
            }
            else if(DoManualProcess()) //single ctrl
            {
                Task=9000;//finish
            }
            else if(iDoManualCommandStatus!=DoNone)//Manual
            {
                if(iDoManualCommandStatus==DoSetSV)
                {
                    iDoAutoCommandStatus=DoSetSV;
                    Task=100;
                }
                else if(iDoManualCommandStatus==DoSetAT)
                {
                    iDoAutoCommandStatus=DoSetAT;
                    Task=100;
                }
                else if(iDoManualCommandStatus==DoSetSeonsorType)
                {
                    iDoAutoCommandStatus=DoSetSeonsorType;
                    Task=100;
                }
                iDoManualCommandStatus=DoNone;
            }
            else //Cycle Run
            {
                if(iCycleRunNumber%iAutoCycleCount==1)  //getstatus
                {
                    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetStatus));   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetStatus (__closure)
                    iDoAutoCommandStatus=DoGetStatus;
                }
                else if(iCycleRunNumber%iAutoCycleCount==2)  //get sv
                {
                    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetSV),0);   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetSV (__closure)
                    iDoAutoCommandStatus=DoGetSV;
                }
                else if(iCycleRunNumber%iAutoCycleCount==3)  //set sv
                {
                    if(IsNeedSetSV())
                    {
                        iDoAutoCommandStatus=DoSetSV;
                    }
                    else
                    {
                        SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetPV),0);   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetPV (__closure)
                        iDoAutoCommandStatus=DoGetPV;
                    }
                }
                else //bDoGetPV
                {
                    SetSingleCommand(W906_DoCmd(dtme08, &uDTME08Control::DoGetPV),0);   //AI(W906-I03) 20261002 (Ifor01): golden dtme08->DoGetPV (__closure)
                    iDoAutoCommandStatus=DoGetPV;
                }
                Task=100;
                DTMTimeout.SetSecAndOn(GetDTMTimeout());
                if(iCycleRunNumber>65530)
                    iCycleRunNumber=0;
                else
                    iCycleRunNumber++;
            }
            break;
        case 100://station
            InitialSetting(iNowActiveStation);
            if(tpvDoCommand!=nullptr)   //AI(W906-I03) 20261002 (Ifor01): golden !=NULL; std::function compares with nullptr
            {
                if(DoSendCommandByManual(true, iNowActiveStation))
                {
                    Task=200;
                }
            }
            else
            {
                Task=9000;
            }
            break;
        case 200:
            if(tpvDoCommand!=nullptr)   //AI(W906-I03) 20261002 (Ifor01): golden !=NULL; std::function compares with nullptr
            {
                if(DoSendCommandByManual(false, iNowActiveStation))
                {
                    if(iDoAutoCommandStatus==DoGetSV)
                    {
                        for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
                        {
                            TargetCh=dtme08->GetChannelNumberPerStation()*iNowActiveStation+i;
                            SetSP(TargetCh, dtme08->mapDTMInfo[i].SV, dtme08->mapDTMInfo[i].Event);
                        }
                    }
                    else if(iDoAutoCommandStatus==DoGetPV)
                    {
                        for(int i=0; i<dtme08->GetChannelNumberPerStation(); i++)
                        {
                            TargetCh=dtme08->GetChannelNumberPerStation()*iNowActiveStation+i;
                            SetPV(TargetCh, dtme08->mapDTMInfo[i].PV, dtme08->mapDTMInfo[i].Event);
                        }
                    }
                    else if(iDoAutoCommandStatus==DoGetStatus)
                    {
                        for(int i=0;i<dtme08->GetChannelNumberPerStation();i++)
                        {
                            TargetCh=dtme08->GetChannelNumberPerStation()*iNowActiveStation+i;
                            SetATStatus(TargetCh,dtme08->mapDTMInfo[i].IsRunAutoTuning);
                        }
                    }
                    Task=9000;
                }
            }
            else
            {
                Task=9000;
            }
            break;
        case 9000://finish
            iNowActiveStation++;
            DTMTimeout.SetSecAndOn(GetDTMTimeout());
            if(GetChannelNumber()>=(iNowActiveStation+1)*dtme08->GetChannelNumberPerStation())//continue
            {
                Task=100;
            }
            else  //finish
            {
                ClearAllCommandStatus();
                Task=1;
            }
            break;
    }

    if(DTMTimeout.Off())
    {
        RecordErrorMsg(AnsiString().sprintf("DoDTME08Cycle Time Out %d Sec", GetDTMTimeout()));
        Task=9000;                                                              //finish
    }
}
#endif // GATE (E-32)

#if 1 // was: #if 0 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2 // GATE (E-33) chkHeaterClick -- golden :725-728.  TWO INDEPENDENT
      // GATES:
      // (a) SAFETY / MACHINE OUTPUT -- `SW[SwHeaterRelay].OnOff(...)`
      //     switches the HEATER RELAY.  Read-only-direction rule forbids it
      //     outright, regardless of (b).
      // (b) LINK BOUNDARY -- `SW[]` (myswitch.h:43) is defined in
      //     myswitch.cpp, add_library(ht9045_io); ht9045_forms does not link
      //     it.  (`SwHeaterRelay` itself IS reachable, cmydef.cpp:1917.)
      // `chkHeater` is also not a declared field here (DEVIATION D-10).
void TfrmDTME08::chkHeaterClick(TObject *Sender)
{
    SW[SwHeaterRelay].OnOff(chkHeater->Checked);
}
#endif // GATE (E-33)

// -- DEVIATION (D-9): golden declares `bool DoGetAllPV_DTME08();`
//    (EJ1N/fDTME08.h:192) and defines it NOWHERE in the entire golden tree
//    (verified 20260828: `rg -l "DoGetAllPV_DTME08"` over
//    HT9011UC_Code_V3.33.906.0_20260618/ returns exactly one file, that
//    header, with one hit).  The facade mirrors golden exactly: declared,
//    never defined.  Calling it is a LINK ERROR here for the same reason it
//    would be in golden.

// ===========================================================================
//  AI(W906-I03) 20261002 (Ifor01): I-03 phase 2 -- the port's entry points.
//  golden runs this form as a VCL TForm: Application->CreateForm runs the ctor
//  (HT9045.cpp:272), TfMain::FormShow arms TimerUpdate on a ship build (906
//  main.cpp:10523-10531), and the VCL message loop delivers the socket events
//  (TClientSocket ctNonBlocking) and fires TimerUpdate every 100 ms.
// ===========================================================================

// golden ctor :16-28 (its :19-27 conditional), at golden's CreateForm position.
// wb_serve calls it after LoadMachineConfig (USE_16_HEATER is read) -- tools/wb_serve.cpp:4064.
// On a DTME08 machine: new uDTME08Control, Reload() READS DTME08_Control.ini
// (EJ1N/uDTME08Control.h GetSettingFilePath; read only), 32 panels.  Otherwise nothing.
void W906_DTME08_CreateFormBoot()
{
    static bool bDone=false;            // golden CreateForm runs the ctor once; a second AddTempGUI would add 32 more panels
    if(bDone)
        return;
    bDone=true;
    frmDTME08->W906_CreateFormBody();
}

// AI(W906-I03b) 20261005 (Ifor01): bthermo W906_DtmMapExchange sets a mapped channel's setting SV through here on every
// visit.  Unlike SetSettingSV (golden :452-460) it writes no SV log -- bthermo already logged the change with the right
// eTempControll (golden's SetSettingSV logs iTempCode[iCh], the Index code, which is wrong for a mapped channel).
void TfrmDTME08::W906_DtmMapSetSV(int iCh, double dTemp)
{
    if(iCh<0 || iCh>=GetChannelNumber())
        return;
    if(GetPalGroup(iCh)->GetSettingSV()!=dTemp)
        GetPalGroup(iCh)->SetSettingSV(dTemp);
}

bool TfrmDTME08::W906_DtmConnected()
{
    return dtme08!=NULL && dtme08->IsConnected();
}

void TfrmDTME08::W906_UseRealSocket()
{
    if(dtme08!=NULL)
        dtme08->W906_UsePolledRealSocket();
}

void TfrmDTME08::W906_PollSocket()
{
    if(dtme08!=NULL)
        dtme08->W906_Poll();
}

// golden TfMain::FormShow 906 main.cpp:10499-10531 -- the heater-controller arm
// chain; this card owns the DTME08 branch.  Kept as golden's chain so the
// ATC 6.0 branch and the EJ1N branch still win first.  ⚠ golden :10523-10525
// reads `A && B || C` (no parentheses): (TC401HeaterControl!=NoHeater &&
// USE_16_HEATER==eht16HeaterDTME08) || USE_16_HEATER==eht32HeaterDTME08 --
// carried verbatim.
// ⚠ NOT CALLED YET (FROM_IFOR s1 1002 14:0x): once armed, the first cycle
// writes sensor type / cycle time / Out2 / SV to every channel.
void W906_DTME08_FormShowArm()
{
    if(ATC_SYSTEM==eNewATCSystem &&
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                          //JerryYang 20210218 : ATC6.0沒有用EJ1N, 不用啟動執行緒
    {
    }
    else if(TC401HeaterControl!=NoHeater &&                                     //Steven 20171227 (Wei) : Add for HT-9045L
           (USE_16_HEATER==eht16HeaterEJ1N ||                                   //Steven 20120220 : Omron EJ1N溫控器
            USE_16_HEATER==eht32HeaterEJ1N))                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
    {
        // golden :10506-10522 fOmron (EJ1N) -- bthermo G26, not this card
    }
    else if(TC401HeaterControl!=NoHeater &&
            USE_16_HEATER==eht16HeaterDTME08 ||                                 //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
            USE_16_HEATER==eht32HeaterDTME08 )                                  //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {
        //sbOmron->Caption="DTM Temp.";   // golden :10527, the main form's button caption (no web element)
        #ifndef SOFT_SIMULTE
        frmDTME08->W906_UseRealSocket();       // port: golden's socket is real on a ship build (vclcompat's default is SIM)
        frmDTME08->TimerUpdate->Enabled=true;
        #endif
    }
    else if(TC401HeaterControl!=NoHeater && DtmMap_Active())                   //AI(W906-I03b) 20261005 (Ifor01): DTM channel map, same arm
    {
        #ifndef SOFT_SIMULTE
        frmDTME08->W906_UseRealSocket();
        frmDTME08->TimerUpdate->Enabled=true;
        #endif
    }
}

// golden TTimer TimerUpdate (fDTME08.dfm:725-729: Interval 100, Enabled False
// until W906_DTME08_FormShowArm).  Called every PumpTick (WebBridgeTags.cpp:601,
// about 500 ms until the shared fast clock of RULINGS_20261002 #7 lands); VCL
// deadline style as MainTimersSt02.cpp: the first fire one Interval after
// the enable, a late tick does not catch up.  The socket is polled on every
// call while armed -- golden's message loop delivers socket events between
// timer fires -- so the connect / read / close complete on this thread and
// nothing here waits for the controller.
// In namespace ht9045 because its caller, PumpTick, is (the block-scope extern
// there names ht9045::W906_DTME08TimerTick) -- the same as forms/fMain_Heater.cpp's
// ht9045::W906_Timer2HeaterTick on the same line.
namespace ht9045 {
void W906_DTME08TimerTick()
{
    static unsigned long ulNext=0;
    if(frmDTME08==NULL || frmDTME08->TimerUpdate->Enabled==false || bSystemClose)
    {
        ulNext=0;
        return;
    }
    frmDTME08->W906_PollSocket();
    const unsigned long ulNow=::GetTickCount();
    if(ulNext==0)
    {
        ulNext=ulNow+100;
        return;
    }
    if((long)(ulNow-ulNext)<0)
        return;
    ulNext=((ulNow-ulNext)>=100)?(ulNow+100):(ulNext+100);
    frmDTME08->TimerUpdateTimer(NULL);
}
} // namespace ht9045
