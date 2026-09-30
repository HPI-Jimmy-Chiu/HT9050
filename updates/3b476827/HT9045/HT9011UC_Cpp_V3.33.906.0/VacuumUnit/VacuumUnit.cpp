// =============================================================================
//  VacuumUnit.cpp  --  TfVacuumUnit definitions (see VacuumUnit.h for the full
//                       ROLE / ZERO CONSUMERS / CLASS SHAPE DECISION / WAVE
//                       SCOPE writeup -- re-cited briefly at each call site).
//
//  Faithful translation of golden VacuumUnit/VacuumUnit.cpp (602 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  Second file to land.
//
//  golden_fns = 20: ctor(1) Initial(2) SetPanelPos(3) ShowSuckMode(4)
//  FormShow(5) tmr1Timer(6) FormClose(7) FormDestroy(8) ReadFile(9)
//  DoIniDataToForm(10) SaveSetupFile(11) spbSaveClick(12) sbtExitClick(13)
//  sbResetClick(14) SetIOTableByECAT_VC8_Sucker(15) SetSuckISABase(16)
//  VaccumCopyFormSuck(17) VaccumCopyToSuck(18) btnSetInArmClick(19)
//  edSetInArmClick(20).
//  ported_fns = 20: every one gets a real definition below.  15 are ACTIVE
//  (several with an internal golden-verbatim `#if 0` span, per this file's
//  own per-function notes); 5 are ENTIRELY gated (empty ACTIVE body, full
//  golden body preserved in `#if 0`) -- SetIOTableByECAT_VC8_Sucker,
//  SetSuckISABase, VaccumCopyFormSuck, VaccumCopyToSuck (GATE 1) and   //AI(W906-MACH1001) 20261001: now -- SetIOTableByECAT_VC8_Sucker / SetSuckISABase / VaccumCopyFormSuck = the machine's cpp 0045 (behind MachineType.h W906_VC8_SUCKER_REMAP + the VC8 sucker gate, VacuumUnit/Vc8Route.h; it supersedes INBOX 115 B's lift of the same three, which had no gate); VaccumCopyToSuck = INBOX 115 B's lift (0045 left it alone; no live caller)
//  btnSetInArmClick (GATE 2) -- because their golden bodies do not have ANY
//  statement independent of a missing substrate member -- see the GATE
//  REGISTER below and VacuumUnit.h's own WAVE SCOPE section.
//
//  GATE REGISTER -- grounded in a tree-wide grep (not guessed), re-citing
//  VacuumUnit.h's own summary with the full reasoning:
//   (1) `SetIOTableByECAT_VC8_Sucker()` (golden :431-480), `SetSuckISABase`
//       (:482-488), `VaccumCopyFormSuck` (:490-522), `VaccumCopyToSuck`
//       (:524-548) -- every statement in all four bodies reads or writes one
//       of `TMySucker::{OnRing,OnIP,OnPort,OnBit,OnType,OnISABase,OffRing,
//       OffIP,OffPort,OffBit,OffType,OffISABase,SenRing,SenIP,SenPort,SenBit,
//       SenType,SenISABase,ISABase,SuckerName}`. This port's TMySucker
//       (aHotPlateSubstrate.h) is grepped clean of every one of those 20
//       names -- it is a MINIMAL mirror carrying only what other
//       already-translated engines (ckernel/ainarm/aoutarm/acatchtray) touch,
//       and none of them touch the EtherCAT-VC8 address fields (those exist
//       only inside golden's own MyKitSuck.h, which this port has never
//       translated -- confirmed by the earlier waves' own citations, e.g.
//       RotateKit/aRotateKIT.cpp's banner listing what aHotPlateSubstrate.h
//       DOES carry from golden MyKitSuck.h). With zero matching members,
//       there is no independent, member-free statement left in any of the
//       four bodies to keep ACTIVE -- unlike MyVacuumPanel.cpp's RefreshDOIO/
//       RefreshVaccumIO (which had a real HW-read/HAL call to preserve
//       alongside the gated display write), these four have NOTHING but the
//       missing-member accesses. ACTIVE arm of each: empty (SetIOTableBy
//       ECAT_VC8_Sucker/SetSuckISABase have no return value; VaccumCopyFormSuck/
//       VaccumCopyToSuck likewise return void). Golden bodies preserved
//       VERBATIM in `#if 0`. BEHAVIOUR DELTA: the vacuum-panel widgets'
//       On/Off/Sen Ring/IP/Port/VCNo/Type/ISABase fields (set once, in each
//       TMyVacuumPanel's OWN ctor from its `_iKind`/`_iCol`/`_iRow`, see
//       MyVacuumPanel.cpp) are simply never OVERWRITTEN a second time from a
//       `sucker.db`/`sucker_9046.db` row -- they keep the ctor's built-in
//       address table instead of whatever the DB might have customised it to.
//       fVacuumUnit->Initial() (golden :32-136, ACTIVE below) does not call
//       SetIOTableByECAT_VC8_Sucker itself -- golden wires that call from
//       OUTSIDE this unit (grepped: zero callers inside VacuumUnit.cpp
//       itself), so this delta is invisible to every function THIS wave
//       translates; it would only surface once a future wave lands golden's
//       own caller.
//   (2) `btnSetInArmClick(TObject *Sender)` (golden :550-595) -- every branch   [AI(W906-VACUNIT-1203) 20260930: OPENED -- the Tags now come from the golden DFM through one tested table; see the function]
//       dispatches on `((TButton*)Sender)->Tag` (golden :556/:568/:583).
//       vclcompat::TButton (and its TControl base) carries NO `Tag` member at
//       all (grepped vclcompat/Controls.h) -- there is no way to recover
//       which of InArm/IndexArm/OutArm the click came from without it, so
//       there is no member-free remainder to keep ACTIVE (unlike, say,
//       spbSaveClick, whose IniConfig/AccessLevel/ShowMyMessage preamble is
//       fully real and independent of the one gated `Close()` call). ACTIVE
//       arm: no-op. Golden body preserved VERBATIM in `#if 0`. Reachability
//       note: btnSetInArm/btnSetIndexArm/btnSetOutArm (the only golden
//       `.dfm`-side sources of a `Sender` here) are real vclcompat::TButton
//       members in this port (see VacuumUnit.h) with no OnClick delegate slot
//       to wire them to this handler anyway (vclcompat::TControl has no
//       OnClick member -- Click() is a parameterless virtual), so this
//       handler is unreachable from this build's own object graph either way.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "VacuumUnit.h"
//   #include "atester.h" / "cprod.h" / "cmydef.h" / "HTEditList.h" /
//     "common.h" / "main.h" / "UsecegemMainFrom.h" / "uHGemHT9045.h" /
//     "MyKitSuck.h" / "myQwertyKeyBoard.h" / "mymessbox.h"
//   #pragma package(smart_init) ; #pragma link "ALed" ; #pragma link
//     "BtnPanelLane" ; #pragma link "MyLedLane" ; #pragma link "MyLed" ;
//     #pragma resource "*.dfm"
//
//   NOT re-included here, and why: "atester.h" -- nothing in the translated
//   bodies below reaches an atester.h-only symbol (FTestSuck/TestIF/
//   TestIF_File all come from cprod.h/aHotPlateSubstrate.h instead, already
//   pulled in); "main.h" -- golden's own god-header for fMain, superseded in
//   this port by forms/fMain.h (the facade every prior wave already
//   converged on); "UsecegemMainFrom.h"/"uHGemHT9045.h" -- nothing here
//   reaches a SECS/GEM class member, only the free function EventReport() and
//   the SECS_EVENT struct, both declared by the narrower SECSGEM/
//   SecsEventReport.h + SecsEventType.h; "MyKitSuck.h" -- TMySucker's real
//   definition is not needed by this TU (every TMySucker-touching body is
//   GATE (1), whose `#if 0` spans need no live definition; the header-level
//   `class TMySucker &` parameters resolve via VacuumUnit.h's own forward
//   declaration); "myQwertyKeyBoard.h"/"mymessbox.h" -- fQwertyKey is real
//   since FW-QWKEY1 (fc08e09) via forms/fQwertyKey.h, included 20260824
//   (edSetInArmClick's GATE 3 OPENED, FW-QWKEY2), and ShowMyMessage's real
//   declaration lives in canary_support.h, already pulled in below.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella)
#include "VacuumUnit/VacuumUnit.h"        // this unit's own contract (also pulls MyVacuumPanel.h + Public/HTEditList.h)
#include "cprod.h"                  // TestIF/TestIF_File (iTestMode, iVaccumThrdIndexArm1/2, iVaccumThrdIn/OutArm)
#include "cmydef.h"                 // MachineTypeChoice, USE_46_SUCKER_DB, AccessLevel, InitialOK, bHasSaveSet
#include "MachineType.h"            // Type_HT9046/_HT9046_LS/_HT1032, eTestMode (SingleSite..._32Site4X8N)
#include "common.h"                 // GetLastOpenFN/GetRecipePath/MyForceDirectories/DataPath
#include "Config.h"                 // IniConfig (HT9045_CONFIG: bA02DisableSaveParsWhenSwitchToOp/bEnable_SECS_GEM)
#include "canary_support.h"         // ShowMyMessage
#include "forms/fMain.h"            // fMain->BackupSetupFile()
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT.SaveRecipe
#include "SECSGEM/SecsEventReport.h"// EventReport(unsigned)
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey site (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
#include "mykitsuck.h"         //AI(W906-VACUNIT-1203) 20260930: TMySucker's golden layout (A4-6 merged it here, mykitsuck.h:143) + FTestSuck / BTestSuck / InArmSuck / OutArmSuck -- the GATE (1) bodies below need them; co-includes cleanly with HTEditList.h (measured -fsyntax-only 20260930: the TList clash below is aHotPlateSubstrate.h's, not this header's). On the old blank line, so no line below moves
//AI(W906-PT-W3) 20260807: `aHotPlateSubstrate.h` is deliberately NOT #include-d
//  whole here (unlike MyVacuumPanel.cpp's sibling functions, which never
//  needed it) -- this file's own VacuumUnit.h already pulls in
//  Public/HTEditList.h (for HTEditList/elVacuumUnit, this dialog's whole
//  reason to exist), and that header's OWN banner documents, as INTEGRATION
//  HAZARD (A), that aHotPlateSubstrate.h defines its own, differently-shaped
//  `class TList` (same name) under a DELIBERATELY different guard macro so
//  co-including both is a hard "redefinition of class TList" compile error,
//  not a silent narrower-capability substitution -- confirmed by trying it:
//  co-including both here produces exactly that error, plus a second one for
//  `class uPlateInfo`. This exact situation, and its resolution, is ALREADY
//  established precedent: OmronLaser/LaserSensor.cpp (which also needs both
//  HTEditList's real elLaser AND aHotPlateSubstrate.h's MyDBIProcess)
//  resolves it the identical way -- forward-declare ONLY the one free
//  function needed, matching (per that file's own citation) golden's OWN
//  idiom for the same situation (golden LaserSensor.cpp:581's
//  `extern bool GetCOMPortStatus(AnsiString Com);`).
extern void MyDBIProcess(AnsiString S1, AnsiString S2);   // golden home aHotPlateSubstrate.h:924 -- see note above
//---------------------------------------------------------------------------
TfVacuumUnit *fVacuumUnit;
//---------------------------------------------------------------------------
TfVacuumUnit::TfVacuumUnit(TComponent* Owner) : TComponent(Owner)
{
    // golden: TForm(Owner) -- TForm has no substrate; TComponent(Owner) is
    // the real base this class inherits instead, see VacuumUnit.h CLASS
    // SHAPE DECISION (Owner is load-bearing: `this` is passed on to every
    // TMyVacuumPanel this ctor's sibling Initial() constructs).
    iCount=5;
}
//---------------------------------------------------------------------------
void TfVacuumUnit::Initial()
{
    bool bShow=true;//, bNoShow=false;
    bool bEnable=true;//, bDisable=false;
    bool bReadFromFile=true;//, bFixedValue=false;
    AnsiString s="";
    if(MachineTypeChoice==Type_HT9046      ||
       MachineTypeChoice==Type_HT9046_LS   ||
       MachineTypeChoice==Type_HT1032      || MachineTypeChoice==Type_HT9050 ||   //AI(W906-HT9050-FAM) 20260925: 加 Type_HT9050（index 真空單元 8 欄＋46 型位址表）-- golden 沒有 HT9050，照 HT9046 家族（使用者 20260925，RULINGS_20260925 第 5 條）
       USE_46_SUCKER_DB==1)                                                     //HT9046LS sucker_9046.db
    {
        iIndexColMax=TOTAL_VACUUM_UNIT;
    }
    else  //HT9045 sucker.db
    {
        iIndexColMax=TOTAL_VACUUM_UNIT/2;
    }
    iInOutColMax=TOTAL_VACUUM_UNIT/2;

    //AI(W906-PT-W3) 20260807: golden's grpIndexArm2/grpIndexArm1/grpInarm/
    //  grpOutarm ->Top/->Left/->Width/->Height writes (golden :52-60,91-99)
    //  have no substrate -- vclcompat::TGroupBox carries only Caption (see
    //  Controls.h). Preserved verbatim:
#if 0
    //initial IndexArm2/IndexArm1 Vacuum unit
    grpIndexArm2->Top       =-3;                                                //IndexArm2
    grpIndexArm2->Left      =0;
    grpIndexArm2->Width     =(VACUUM_UNIT_WIDTH+2)*iIndexColMax;
    grpIndexArm2->Height    =(VACUUM_UNIT_HEIGHT+12)*2;

    grpIndexArm1->Top       =grpIndexArm2->Height;                              //IndexArm1
    grpIndexArm1->Left      =0;
    grpIndexArm1->Width     =(VACUUM_UNIT_WIDTH+2)*iIndexColMax;
    grpIndexArm1->Height    =(VACUUM_UNIT_HEIGHT+12)*2;
#endif

    if(iIndexColMax==TOTAL_VACUUM_UNIT)                                         //HT9046LS sucker_9046.db
    {
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalArm1[iCol][iRow]                     = new TMyVacuumPanel(this,0,iCol,iRow);      //IndexArm1
                myPalArm2[iCol][iRow]                     = new TMyVacuumPanel(this,1,iCol,iRow);      //IndexArm2
                //AI(W906-PT-W3) 20260807: golden `->GroupBox->Parent=
                //  scrlbxIndexArm1/2;` (golden :69,72) has no substrate on
                //  EITHER side (no ->Parent member; no TScrollBox type at
                //  all, see VacuumUnit.h's own scrlbx GATE). Preserved
                //  verbatim:
#if 0
                myPalArm1[iCol][iRow]->GroupBox->Parent   = scrlbxIndexArm1;
                myPalArm2[iCol][iRow]->GroupBox->Parent   = scrlbxIndexArm2;
#endif
            }
        }
    }
    else    //HT9045 sucker.db
    {
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalArm1[iCol][iRow]                     = new TMyVacuumPanel(this,4,iCol,iRow);      //IndexArm1
                myPalArm2[iCol][iRow]                     = new TMyVacuumPanel(this,5,iCol,iRow);      //IndexArm2
#if 0
                myPalArm1[iCol][iRow]->GroupBox->Parent   = scrlbxIndexArm1;
                myPalArm2[iCol][iRow]->GroupBox->Parent   = scrlbxIndexArm2;
#endif
            }
        }
    }
#if 0
    //initial InArm/OutArm Vacuum unit
    grpInarm->Top       =-3;                                                     //InArm
    grpInarm->Left      =0;
    grpInarm->Width     =(VACUUM_UNIT_WIDTH+2)*iInOutColMax;
    grpInarm->Height    =(VACUUM_UNIT_HEIGHT+12)*2;

    grpOutarm->Top      =-3;                                                     //OutArm
    grpOutarm->Left     =0;
    grpOutarm->Width    =(VACUUM_UNIT_WIDTH+2)*iInOutColMax;
    grpOutarm->Height   =(VACUUM_UNIT_HEIGHT+12)*2;
#endif
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]                    = new TMyVacuumPanel(this,2,iCol,iRow);     //InArm
            myPalOutArm[iCol][iRow]                   = new TMyVacuumPanel(this,3,iCol,iRow);     //OutArm
#if 0
            myPalInArm[iCol][iRow]->GroupBox->Parent  = scrlbxInArm;
            myPalOutArm[iCol][iRow]->GroupBox->Parent = scrlbxOutArm;
#endif
        }
    }

    //VacuumUnit
    elVacuumUnit->Add(CheckBox1,        &bTest1,          ECBool,           "Tray",    "CheckBox1",      bShow, bEnable, bReadFromFile, 0);
    elVacuumUnit->Add(Edit1,            &iTest1,          ECInteger,        "Tray",    "Edit1",          bShow, bEnable, bReadFromFile, iTest1,          true,  0,   300);

    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            s.sprintf("IndexArm1_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalArm1[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdIndexArm1[iRow][iCol], ECInteger,   "Vacuum Threshold",  s,    bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdIndexArm1[iRow][iCol],    true,  -116.0,   148);
            s.sprintf("IndexArm2_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalArm2[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdIndexArm2[iRow][iCol], ECInteger,   "Vacuum Threshold",  s,    bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdIndexArm2[iRow][iCol],    true,  -116.0,   148);
        }
    }
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            s.sprintf("InArm_%d_%d", iRow, iCol);
            elVacuumUnit->Add(myPalInArm[iCol][iRow]->edSV,  &TestIF_File.iVaccumThrdInArm[iRow][iCol], ECInteger,   "Vacuum Threshold",  s, bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdInArm[iRow][iCol],    true,  -116.0,   148);
            s.sprintf("OutArm_%d_%d",iRow,iCol);
            elVacuumUnit->Add(myPalOutArm[iCol][iRow]->edSV, &TestIF_File.iVaccumThrdOutArm[iRow][iCol], ECInteger,  "Vacuum Threshold",  s, bShow, bEnable, bReadFromFile, TestIF_File.iVaccumThrdOutArm[iRow][iCol],    true,  -116.0,   148);
        }
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::SetPanelPos(bool bInit)
{
    //AI(W906-PT-W3) 20260807: golden's ->Left/->Top writes (golden :145-149,
    //  158-159,162-163) have no substrate (see Initial()'s identical note).
    //  The ->Visible writes on myPalInArm/myPalOutArm (golden :157,161) ARE
    //  real (vclcompat::TControl::Visible) and stay ACTIVE.
    //Arm2/Arm1
    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
#if 0
            myPalArm2[iCol][iRow]->GroupBox->Left         =VACUUM_UNIT_WIDTH*iCol;
            myPalArm2[iCol][iRow]->GroupBox->Top          =VACUUM_UNIT_HEIGHT*iRow;

            myPalArm1[iCol][iRow]->GroupBox->Left         =VACUUM_UNIT_WIDTH*iCol;
            myPalArm1[iCol][iRow]->GroupBox->Top          =VACUUM_UNIT_HEIGHT*iRow;
#endif
        }
    }
    //In/OutArm
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]->GroupBox->Visible     =bInit;
#if 0
            myPalInArm[iCol][iRow]->GroupBox->Left        =VACUUM_UNIT_WIDTH*iCol;
            myPalInArm[iCol][iRow]->GroupBox->Top         =VACUUM_UNIT_HEIGHT*iRow;
#endif

            myPalOutArm[iCol][iRow]->GroupBox->Visible    =bInit;
#if 0
            myPalOutArm[iCol][iRow]->GroupBox->Left       =VACUUM_UNIT_WIDTH*iCol;
            myPalOutArm[iCol][iRow]->GroupBox->Top        =VACUUM_UNIT_HEIGHT*iRow;
#endif
        }
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::ShowSuckMode(int iSel)
{
    //AI(W906-PT-W3) 20260807: golden's `FTestSuck.iMaxRow/iMaxCol/iShtRow/
    //  iShtCol` loop (golden :170-186) needs TMyKitSuck's FULL class layout
    //  (aHotPlateSubstrate.h), which cannot be #include-d in this TU -- see
    //  this file's include-block note (the SAME documented TList/uPlateInfo
    //  conflict with Public/HTEditList.h, which this file needs for real
    //  elVacuumUnit functionality elsewhere). Unlike MyDBIProcess (a single
    //  free function, forward-declarable in one line), TMyKitSuck's members
    //  cannot be reached without its complete definition, so there is no
    //  narrower forward-declaration escape here -- this is a NEW instance of
    //  the same class of documented cross-file conflict, not a missing-
    //  substrate gap. Golden body preserved verbatim:
#if 0
    for(int iRow=0; iRow<FTestSuck.iMaxRow; iRow++)
    {
        for(int iCol=0; iCol<FTestSuck.iMaxCol; iCol++)
        {
            if(iRow<FTestSuck.iShtRow &&
               iCol<FTestSuck.iShtCol)
            {
                myPalArm2[iCol][iRow]->GroupBox->Visible=true;
                myPalArm1[iCol][iRow]->GroupBox->Visible=true;
            }
            else
            {
                myPalArm2[iCol][iRow]->GroupBox->Visible=false;
                myPalArm1[iCol][iRow]->GroupBox->Visible=false;
            }
        }
    }
#endif

    switch(iSel)
    {
        case SingleSite:        //1x1 BusyShuttle
        case DualSite:          //1x2
        case QualSite2X2N:      //Frank 20200520 2X2NN Mode
        case TriSite1X3:        //Frank 20160329 add for 1x3_4
        case _6Site2X3N:        //Steven 20220425 : 2X3NN Mode
        case QualSite1X4:       //1x4
        case _8Site1X4:         //ChungHung 20150528 add for 海思 _8Site1x4
        case DualSite2x1:       //2x1
        case QualSite2X2:       //2x2
        case _6Site2X3:         //ChungHung 20140115 add for 2x3_6
        case _16Site4X4:        //Sam 20190226 : 16Site4X4
        case _8Site2X4:         //2x4
        case _16Site2X8:        //2x8  //Steven 20110520 : 換位置
        case _10Site2X5:        //wei 20190614 10 site
        case _12Site2X6:
        case _32Site4X8N:       //2x8
            break;
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::FormShow(TObject *Sender)
{
    (void)Sender;
    AnsiString S="";
    LastFileName=GetLastOpenFN();
    ReadFile();
    S.sprintf("Vacuum Unit Condition  ''%s''  ",LastFileName);
    //AI(W906-PT-W3) 20260807: golden `Caption=S;` / `this->Top/Left/Width/
    //  Height=...;` (golden :216-220) have no substrate -- TfVacuumUnit is a
    //  standalone class (VacuumUnit.h CLASS SHAPE DECISION), not a TForm with
    //  Caption/geometry. `S` is still computed above for provenance (harmless
    //  local AnsiString), just never displayed. Preserved verbatim:
#if 0
    Caption=S;
    this->Top=0;
    this->Left=110;
    this->Width=690;
    this->Height=1020;
#endif
    fShow=true;
    //AI(W906-PT-W3) 20260807: golden `tmr1->Enabled=true;` (golden :222) has
    //  no substrate (no TTimer type, see VacuumUnit.h's own tmr1 GATE).
#if 0
    tmr1->Enabled=true;
#endif
    SetPanelPos(fShow);     //設定畫面
    iCount=2;
    ShowSuckMode(TestIF.iTestMode);

    //IndexArm12
    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalArm1[iCol][iRow]->RefreshDOIO();

            myPalArm2[iCol][iRow]->RefreshDOIO();
        }
    }

    //In/OutArm
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]->RefreshDOIO();

            myPalOutArm[iCol][iRow]->RefreshDOIO();
        }
    }
   /*
   edSetInArm->Text     =(int)myPalInArm[0][0]->ReadVaccumThreshold();      //以第一個位置閥值來當初始值。
   edSetIndexArm->Text  =(int)myPalArm1[0][0]->ReadVaccumThreshold();
   edSetOutArm->Text    =(int)myPalOutArm[0][0]->ReadVaccumThreshold();
   */
}
//---------------------------------------------------------------------------
void TfVacuumUnit::tmr1Timer(TObject *Sender)
{
    (void)Sender;
    if(InitialOK==false || fShow==false)
        return;
    if(iCount<=0)
    {
        //Arm2/Arm1
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalArm1[iCol][iRow]->RefreshCurrectVal();
                myPalArm1[iCol][iRow]->RefreshThresholdVal();
                myPalArm1[iCol][iRow]->RefreshVaccumIO();
                myPalArm1[iCol][iRow]->RefreshEvent();
                myPalArm1[iCol][iRow]->InitialThresholdMode();

                myPalArm2[iCol][iRow]->RefreshCurrectVal();
                myPalArm2[iCol][iRow]->RefreshThresholdVal();
                myPalArm2[iCol][iRow]->RefreshVaccumIO();
                myPalArm2[iCol][iRow]->RefreshEvent();
                myPalArm2[iCol][iRow]->InitialThresholdMode();
            }
        }
        //In/OutArm
        for(int iCol=0; iCol<iInOutColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                myPalInArm[iCol][iRow]->RefreshCurrectVal();
                myPalInArm[iCol][iRow]->RefreshThresholdVal();
                myPalInArm[iCol][iRow]->RefreshVaccumIO();
                myPalInArm[iCol][iRow]->InitialThresholdMode();
                myPalInArm[iCol][iRow]->RefreshEvent();

                myPalOutArm[iCol][iRow]->RefreshCurrectVal();
                myPalOutArm[iCol][iRow]->RefreshThresholdVal();
                myPalOutArm[iCol][iRow]->RefreshVaccumIO();
                myPalOutArm[iCol][iRow]->InitialThresholdMode();
                myPalOutArm[iCol][iRow]->RefreshEvent();
            }
        }
    }
    else
    {
        iCount--;
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::FormClose()
{
    ReadFile();
    //AI(W906-PT-W3) 20260807: golden `tmr1->Enabled=false;` (golden :308) has
    //  no substrate, see FormShow's identical note.
#if 0
    tmr1->Enabled=false;
#endif
    fShow=false;
    DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
}
//---------------------------------------------------------------------------
void TfVacuumUnit::FormDestroy(TObject *Sender)
{
    (void)Sender;
    try
    {
        for(int iCol=0; iCol<iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                if(myPalArm2[iCol][iRow]!=NULL)
                    delete myPalArm2[iCol][iRow];
                myPalArm2[iCol][iRow]=NULL;

                if(myPalArm1[iCol][iRow]!=NULL)
                    delete myPalArm1[iCol][iRow];
                myPalArm1[iCol][iRow]=NULL;
            }
        }

        for(int iCol=0; iCol<iInOutColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                if(myPalInArm[iCol][iRow]!=NULL)
                    delete myPalInArm[iCol][iRow];
                myPalInArm[iCol][iRow]=NULL;
                if(myPalOutArm[iCol][iRow]!=NULL)
                    delete myPalOutArm[iCol][iRow];
                myPalOutArm[iCol][iRow]=NULL;
            }
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfVacuumUnit::FormDestroy");
    }
    //LogSoftwareOffTime("TfVacuumUnit, FormDestroy"); //Steven 20210526 : 紀錄軟體執行時間
}
//---------------------------------------------------------------------------
void TfVacuumUnit::ReadFile()
{
    AnsiString szDir=GetRecipePath();
    LastFileName=GetLastOpenFN();

    if(elVacuumUnit!=NULL)
    {
        elVacuumUnit->ReadEditTextFromFile(szDir, "HandlerCondition.Data");
    }
    DoIniDataToForm();
}
//---------------------------------------------------------------------------
void TfVacuumUnit::DoIniDataToForm()
{
    if(elVacuumUnit!=NULL)
        elVacuumUnit->InitialDataToEdit();
}
//---------------------------------------------------------------------------
void TfVacuumUnit::SaveSetupFile(AnsiString szDir, AnsiString S)
{
    (void)S;
    MyForceDirectories(szDir);
    if(elVacuumUnit!=NULL)
        elVacuumUnit->SaveEditTextToFile(szDir, "HandlerCondition.Data");

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
}
//---------------------------------------------------------------------------
void TfVacuumUnit::spbSaveClick(TObject *Sender)
{
    (void)Sender;
    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        ShowMyMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        //AI(W906-PT-W3) 20260807: golden `Close();` (golden :384) has no
        //  substrate -- TfVacuumUnit is standalone (VacuumUnit.h CLASS SHAPE
        //  DECISION), not a TForm with a Close() method. golden itself falls
        //  through and saves anyway regardless of this branch (no `return`
        //  after Close()) -- that fall-through is golden's OWN behaviour,
        //  preserved as-is below; only the window-close side effect is gated.
#if 0
        Close();
#endif
    }

    AnsiString szDir="";
    szDir.sprintf("%s%s\\", DataPath, LastFileName);
    SaveSetupFile(szDir, LastFileName);
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(LastFileName, "JOBFILE");
    #endif
    ReadFile();
    spbSave->Down=false;
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::sbtExitClick(TObject *Sender)
{
    (void)Sender;
    sbtExit->Down=false;
    //AI(W906-PT-W3) 20260807: golden `Close();` (golden :405) -- same GATE as
    //  spbSaveClick above.
#if 0
    Close();
#endif
}
//---------------------------------------------------------------------------
void TfVacuumUnit::sbResetClick(TObject *Sender)
{
    (void)Sender;
    //Arm2/Arm1
    for(int iCol=0; iCol<iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalArm1[iCol][iRow]->Reset();
            myPalArm2[iCol][iRow]->Reset();
        }
    }

    //In/OutArm
    for(int iCol=0; iCol<iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            myPalInArm[iCol][iRow]->Reset();
            myPalOutArm[iCol][iRow]->Reset();
        }
    }
}
//---------------------------------------------------------------------------
// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- SetIOTableByECAT_VC8_Sucker
//  golden HT9011UC_Code_V3.33.906.0_20260618/VacuumUnit/VacuumUnit.cpp:431-480  (50 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN SetIOTableByECAT_VC8_Sucker VacuumUnit/VacuumUnit.cpp:431-480
void SetIOTableByECAT_VC8_Sucker()
{
    if(VCCU_UNIT_TYPE==1)
    {
        //由軟體自動編排 2
        /*
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
            {
                VaccumCopyToSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);
                VaccumCopyToSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
            }
        }

        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                VaccumCopyToSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);
                VaccumCopyToSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
            }
        }
        */

        //讀 DB 檔 2
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
            {
                SetSuckISABase(FTestSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);

                SetSuckISABase(BTestSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
            }
        }
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                SetSuckISABase(InArmSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);

                SetSuckISABase(OutArmSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
            }
        }
    }
}
#endif // AI-W6C-GOLDEN-END SetIOTableByECAT_VC8_Sucker VacuumUnit/VacuumUnit.cpp:431-480

//AI(W906-PT-W3) 20260807: GATE (1) -- see this file's own banner GATE
//  REGISTER entry (1). Golden body preserved verbatim, in comment form:
//
//  void SetIOTableByECAT_VC8_Sucker()
//  {
//      if(VCCU_UNIT_TYPE==1)
//      {
//          //由軟體自動編排 2
//          /*
//          for(int iRow=0; iRow<2; iRow++)
//          {
//              for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
//              {
//                  VaccumCopyToSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);
//                  VaccumCopyToSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
//              }
//          }
//
//          for(int iRow=0; iRow<2; iRow++)
//          {
//              for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
//              {
//                  VaccumCopyToSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);
//                  VaccumCopyToSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
//              }
//          }
//          */
//
//          //讀 DB 檔 2
//          for(int iRow=0; iRow<2; iRow++)
//          {
//              for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
//              {
//                  SetSuckISABase(FTestSuck.Suck[iRow][iCol],ePCI1203);
//                  VaccumCopyFormSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);
//
//                  SetSuckISABase(BTestSuck.Suck[iRow][iCol],ePCI1203);
//                  VaccumCopyFormSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
//              }
//          }
//          for(int iRow=0; iRow<2; iRow++)
//          {
//              for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
//              {
//                  SetSuckISABase(InArmSuck.Suck[iRow][iCol],ePCI1203);
//                  VaccumCopyFormSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);
//
//                  SetSuckISABase(OutArmSuck.Suck[iRow][iCol],ePCI1203);
//                  VaccumCopyFormSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
//              }
//          }
//      }
//  }
//---------------------------------------------------------------------------
void SetIOTableByECAT_VC8_Sucker()
{
    //AI(W906-PT-W3) 20260807: GATE (1) -- see banner. ACTIVE arm: empty.
    //AI(W906-VACUNIT-1203) 20260930: OPENED behind MachineType.h W906_VC8_SUCKER_REMAP (EastSun ruling R3
    //  20260930: follow golden). GATE (1)'s premise is gone: TMySucker carries every one of the 20 fields since
    //  A4-6 (mykitsuck.h:143), and InitSucker binds them from IO_Table.csv. Called once at boot by
    //  FileRW_VacuumUnit_Boot, right after the live panels exist -- golden main.cpp:10497-10498
    //  (fVacuumUnit->Initial(); SetIOTableByECAT_VC8_Sucker();), after InitialHandler's InitSucker, as golden.
    //  The body is golden's (:431-480) verbatim. What it does on THIS machine today (VacuUnitType=1, no VC8):
    //    * SetSuckISABase: every FTestSuck / BTestSuck (2 x 8) and InArmSuck / OutArmSuck (2 x 4) sucker gets
    //      On/Off/Sen/ISABase = ePCI1203 (3) -- which IO_Table.csv already gives all 144 of their rows, so the
    //      engine's IO dispatch does not change; all 144 rows are Enable=0, so no sucker is switched or sensed.
    //    * VaccumCopyFormSuck: each golden panel takes its sucker's IO_Table address. Same as golden's ctor table
    //      for IndexArm1 / InArm / OutArm; DIFFERENT for IndexArm2 (e.g. myPalArm2_0_0: ctor VC3 on 22/23,
    //      IO_Table BTestSuckAA VC7 on 31/30) -- the DB wins, as in golden.
    //    * + the W906 sucker gate (VacuumUnit/Vc8Route.h): each remapped sucker's three aliases are registered,
    //      so MyLaneIO switches / senses them only when their station IS an ECAT-VC8. BTestSuck*'s DO channels
    //      16..19 on stations 0x50 / 0x51 are the ENABLED loader / auto1 cylinders' channels (C_Load_Up,
    //      C_LoaderDrawerLock, C_Auto1_Up, C_Auto1DrawerLock) -- without the gate an enabled BTestSuck row would
    //      pulse those coils.
    //  With the #define commented out this function is the empty stub again (today's behaviour).
#ifdef W906_VC8_SUCKER_REMAP
    if (fVacuumUnit == 0) return;                                               // golden: Initial() ran first (main.cpp:10497)
    if(VCCU_UNIT_TYPE==1)
    {
        //由軟體自動編排 2
        /*
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
            {
                VaccumCopyToSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);
                VaccumCopyToSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
            }
        }

        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                VaccumCopyToSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);
                VaccumCopyToSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
            }
        }
        */

        //讀 DB 檔 2
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
            {
                SetSuckISABase(FTestSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalArm1[iCol][iRow],FTestSuck.Suck[iRow][iCol]);

                SetSuckISABase(BTestSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalArm2[iCol][iRow],BTestSuck.Suck[iRow][iCol]);
            }
        }
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                SetSuckISABase(InArmSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalInArm[iCol][iRow],InArmSuck.Suck[iRow][iCol]);

                SetSuckISABase(OutArmSuck.Suck[iRow][iCol],ePCI1203);
                VaccumCopyFormSuck(*fVacuumUnit->myPalOutArm[iCol][iRow],OutArmSuck.Suck[iRow][iCol]);
            }
        }
        std::printf("VacuumUnit: golden SetIOTableByECAT_VC8_Sucker (VacuUnitType=1): %d sucker aliases now switch / sense "
                    "only through an identity-checked ECAT-VC8 (W906_VC8_SUCKER_REMAP)\n", W906_Vc8GuardedSuckers());
    }
#endif
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: GATE (1) -- golden body preserved verbatim:
//
//  void SetSuckISABase(class TMySucker &Source, int iISABase)
//  {
//      Source.OnISABase    =iISABase;
//      Source.OffISABase   =iISABase;
//      Source.SenISABase   =iISABase;
//      Source.ISABase      =iISABase;
//  }
//---------------------------------------------------------------------------
void SetSuckISABase(class TMySucker &Source, int iISABase)
{
    (void)Source; (void)iISABase;
    //AI(W906-PT-W3) 20260807: GATE (1) -- see banner. ACTIVE arm: empty.
    //AI(W906-VACUNIT-1203) 20260930: OPENED behind W906_VC8_SUCKER_REMAP (R3, see SetIOTableByECAT_VC8_Sucker). golden :482-488 verbatim.
#ifdef W906_VC8_SUCKER_REMAP
    Source.OnISABase    =iISABase;
    Source.OffISABase   =iISABase;
    Source.SenISABase   =iISABase;
    Source.ISABase      =iISABase;
#endif
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: GATE (1) -- golden body preserved verbatim:
//
//  void VaccumCopyFormSuck(class TMyVacuumPanel &Target, class TMySucker &Source)
//  {
//      //將 VaccumUnit 讀取 Suck Database
//      AnsiString str="";
//      Target.SuckerName=Source.SuckerName;
//      Target.OnRing   =Source.OnRing;
//      Target.OnIP     =Source.OnIP;
//      Target.OnPort   =Source.OnPort;
//      Target.OnVCNo   =Source.OnBit;
//      Target.OnType   =Source.OnType;
//      Target.OnISABase=Source.OnISABase;
//
//      str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s", Target.OnRing, Target.OnIP, Target.OnPort, Target.OnVCNo, Target.SuckerName);
//      Target.bplOn->Hint=str;
//
//      Target.OffRing  =Source.OffRing;
//      Target.OffIP    =Source.OffIP;
//      Target.OffPort  =Source.OffPort;
//      Target.OffVCNo  =Source.OffBit;
//      Target.OffType  =Source.OffType;
//      Target.OffISABase=Source.OffISABase;
//      str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s", Target.OffRing, Target.OffIP, Target.OffPort, Target.OffVCNo, Target.SuckerName);
//      Target.bplOff->Hint=str;
//
//      Target.SenRing  =Source.SenRing;
//      Target.SenIP    =Source.SenIP;
//      Target.SenPort  =Source.SenPort;
//      Target.SenVCNo  =Source.SenBit;
//      Target.SenType  =Source.SenType;
//      Target.SenISABase=Source.SenISABase;
//      str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s VCNo%d", Target.SenRing,Target.SenIP, Target.SenPort, Target.SenVCNo, Target.SuckerName, Target.SenVCNo);
//      Target.myld1->Hint=str;
//  }
//---------------------------------------------------------------------------
void VaccumCopyFormSuck(class TMyVacuumPanel &Target, class TMySucker &Source)
{
    (void)Target; (void)Source;
    //AI(W906-PT-W3) 20260807: GATE (1) -- see banner. ACTIVE arm: empty.
    //AI(W906-VACUNIT-1203) 20260930: OPENED behind W906_VC8_SUCKER_REMAP (R3, see SetIOTableByECAT_VC8_Sucker).
    //  golden :490-522 verbatim; only the three widget Hints (bplOn / bplOff / myld1 -- MyVacuumPanel.h GATE (2))
    //  stay gated. Then (W906) the sucker's three aliases are registered with the sucker gate at the address
    //  golden now gives them (VacuumUnit/Vc8Route.h): MyLaneIO switches / senses them only on an ECAT-VC8.
    //  ⓘ golden oddities kept, not fixed: the VCNo fields take the IO row's BIT (golden :498/:508/:517), which on
    //  IO_Table.csv IS the VC number (FTestSuckAA Port 128 Bit 0, BTestSuckAA Port 135 Bit 7); and TMySucker::
    //  GetIOValue (mykitsuck.cpp:2904) later passes SenPort (128+VC), not the VC, as GetIOValue's Port -- so the
    //  sucker's own analog read asks DI bytes 256/257 (the route refuses them: 999.0, as golden's vendor error).
#ifdef W906_VC8_SUCKER_REMAP
    //將 VaccumUnit 讀取 Suck Database
    AnsiString str="";
    Target.SuckerName=Source.SuckerName;
    Target.OnRing   =Source.OnRing;
    Target.OnIP     =Source.OnIP;
    Target.OnPort   =Source.OnPort;
    Target.OnVCNo   =Source.OnBit;
    Target.OnType   =Source.OnType;
    Target.OnISABase=Source.OnISABase;

#if 0   // GATE (2): bplOn has no port (MyVacuumPanel.h)
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s", Target.OnRing, Target.OnIP, Target.OnPort, Target.OnVCNo, Target.SuckerName);
    Target.bplOn->Hint=str;
#endif

    Target.OffRing  =Source.OffRing;
    Target.OffIP    =Source.OffIP;
    Target.OffPort  =Source.OffPort;
    Target.OffVCNo  =Source.OffBit;
    Target.OffType  =Source.OffType;
    Target.OffISABase=Source.OffISABase;
#if 0   // GATE (2): bplOff has no port
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s", Target.OffRing, Target.OffIP, Target.OffPort, Target.OffVCNo, Target.SuckerName);
    Target.bplOff->Hint=str;
#endif

    Target.SenRing  =Source.SenRing;
    Target.SenIP    =Source.SenIP;
    Target.SenPort  =Source.SenPort;
    Target.SenVCNo  =Source.SenBit;
    Target.SenType  =Source.SenType;
    Target.SenISABase=Source.SenISABase;
#if 0   // GATE (2): myld1 has no port
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) %s VCNo%d", Target.SenRing,Target.SenIP, Target.SenPort, Target.SenVCNo, Target.SuckerName, Target.SenVCNo);
    Target.myld1->Hint=str;
#endif
    (void)str;
    W906_Vc8GuardSucker(Source.OnRing,  Source.OnIP,  Source.OnPortName.c_str());    //AI(W906-VACUNIT-1203) 20260930: the sucker gate
    W906_Vc8GuardSucker(Source.OffRing, Source.OffIP, Source.OffPortName.c_str());
    W906_Vc8GuardSucker(Source.SenRing, Source.SenIP, Source.SensorName.c_str());
#endif
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: GATE (1) -- golden body preserved verbatim:   //AI(W906-I115B) 20260930: LIFTED -- the golden text below is uncommented and live
//
void VaccumCopyToSuck(class TMyVacuumPanel &Source, class TMySucker &Target)
{
    //將 VaccumUnit 寫到 Suck Database
    Target.OnRing   =Source.OnRing;
    Target.OnIP     =Source.OnIP;
    Target.OnPort   =Source.OnPort;
    Target.OnBit    =Source.OnVCNo;
    Target.OnType   =Source.OnType;
    Target.OnISABase=Source.OnISABase;

    Target.OnRing   =Source.OffRing;   //AI(W906-I115B) 20260930: GOLDEN BUG KEPT (faithful): OnRing is overwritten with the panel OffRing and Target.OffRing is never written
    Target.OffIP    =Source.OffIP;
    Target.OffPort  =Source.OffPort;
    Target.OffBit   =Source.OffVCNo;
    Target.OffType  =Source.OffType;
    Target.OffISABase=Source.OffISABase;

    Target.SenRing  =Source.SenRing;
    Target.SenIP    =Source.SenIP;
    Target.SenPort  =Source.SenPort;
    Target.SenBit   =Source.SenVCNo;
    Target.SenType  =Source.SenType;
    Target.SenISABase=Source.SenISABase;
    Target.ISABase  =ePCI1203;
}
//  NOTE (provenance only, not fixed): golden's 2nd statement writes
//  `Target.OnRing=Source.OffRing;` -- reusing OnRing where OffRing is surely
//  meant (every other line in this function pairs Source.XXX with
//  Target.XXX of the SAME prefix). Left exactly as golden wrote it in this
//  comment for the record.  //AI(W906-I115B) 20260930: the body is LIVE now, so golden's OnRing=OffRing overwrite runs as written (:814)
//---------------------------------------------------------------------------
//void VaccumCopyToSuck(class TMyVacuumPanel &Source, class TMySucker &Target)   //AI(W906-I115B) 20260930: port stub RETIRED -- the golden body at :804-828 is the definition now
//{
//    (void)Source; (void)Target;
//    //AI(W906-PT-W3) 20260807: GATE (1) -- see banner. ACTIVE arm: empty.
//}
//---------------------------------------------------------------------------
void TfVacuumUnit::btnSetInArmClick(TObject *Sender)
{
    (void)Sender;
    //AI(W906-PT-W3) 20260807: GATE (2) -- see this file's own banner GATE
    //  REGISTER entry (2). Golden body preserved verbatim:
    // AI(W906-FW-TAG1) 20260825: STAYS SHUT, and the reason is now much
    //   stronger than "no substrate". Tag compiles since this wave, but the
    //   VALUES come from the designer: golden VacuumUnit/VacuumUnit.dfm gives
    //   btnSetInArm no Tag line (=0, :223), btnSetIndexArm `Tag = 1` (:251)
    //   and btnSetOutArm `Tag = 2` (:261), and all three share
    //   `OnClick = btnSetInArmClick` (:230/:258/:268). This port has no
    //   .dfm->C++ Tag loader, so all three would read 0 and ALL THREE BUTTONS
    //   WOULD TAKE THE InArm ARM below -- i.e. pressing "set Index" or "set
    //   OutArm" would write the InArm threshold into the InArm vacuum units
    //   via WriteVaccumThreshold(). That is a machine-control defect, not a
    //   cosmetic one. It is latent only because nothing wires OnClick yet;
    //   the write-path wave MUST supply these Tags (or dispatch another way)
    //   before this block is opened.
    //AI(W906-VACUNIT-1203) 20260930: OPENED -- the write-path wave is this one (EastSun 20260930
    //   「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」), and the Tag problem above is solved
    //   the way that note asks, BOTH halves:
    //     (1) the Tags ARE supplied, from the golden DFM, not left at 0: the live host
    //         (VacuumUnit/VacuumUnitLive.inc W906_VuLiveBuild_) creates btnSetInArm / btnSetIndexArm /
    //         btnSetOutArm with Tag = 0 / 1 / 2 from ONE table, kVuSetAllArms (VacuumUnit/VacuumUnitLive.h),
    //         cited to VacuumUnit.dfm :223 (no Tag line = 0) / :251 `Tag = 1` / :261 `Tag = 2`, and
    //         tests/test_vacuum_vc8.cpp asserts that table against tools/dfm2rc/ir_out/VacuumUnit/
    //         VacuumUnit.dfm.ir.json every run;
    //     (2) the dispatch no longer depends on WHICH object a click came from: the web page sends the
    //         arm by name ({arm:"in"|"index"|"out"}), C++ maps the name through that table to the button,
    //         and an unknown name is refused before this function runs.
    //   So the three buttons can no longer all read Tag 0 and all take the InArm arm. The body is golden's,
    //   unchanged; its hardware writes go through WriteVaccumThreshold -> MyLaneIO.SetIOValueThread -> the
    //   ECAT-VC8 route (fail closed, VacuumUnit/Vc8Route.h). The only caller is W906_VacuumLiveSetAll.
    TButton *Ptr;
    double dValue=0.0;

    Ptr=(TButton *)Sender;
    if(Ptr->Tag==0)     //InArm
    {
        dValue=atof(edSetInArm->Text.c_str());
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                fVacuumUnit->myPalInArm[iCol][iRow]->edSV->Text=(int)dValue;
                fVacuumUnit->myPalInArm[iCol][iRow]->WriteVaccumThreshold(dValue);
            }
        }
    }
    else if(Ptr->Tag==1)    //Index Arm1/2
    {
        dValue=atof(edSetIndexArm->Text.c_str());
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iIndexColMax; iCol++)
            {
                fVacuumUnit->myPalArm1[iCol][iRow]->edSV->Text=(int)dValue;
                fVacuumUnit->myPalArm2[iCol][iRow]->edSV->Text=(int)dValue;
                fVacuumUnit->myPalArm1[iCol][iRow]->WriteVaccumThreshold(dValue);
                fVacuumUnit->myPalArm2[iCol][iRow]->WriteVaccumThreshold(dValue);
            }
        }
    }

    if(Ptr->Tag==2)     //OutArm
    {
        dValue=atof(edSetOutArm->Text.c_str());
        for(int iRow=0; iRow<2; iRow++)
        {
            for(int iCol=0; iCol<fVacuumUnit->iInOutColMax; iCol++)
            {
                fVacuumUnit->myPalOutArm[iCol][iRow]->edSV->Text=(int)dValue;
                fVacuumUnit->myPalOutArm[iCol][iRow]->WriteVaccumThreshold(dValue);
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfVacuumUnit::edSetInArmClick(TObject *Sender)
{
    //AI(W906-PT-W3) 20260807: GATE (3) -- golden `fQwertyKey->ShowQwertyKey(
    //  (TEdit*)Sender, N_DOUBLE, 0, true, -116.0, 148.0);` (golden :599).
    //  fQwertyKey real since FW-QWKEY1 (fc08e09) -- GATE (3) OPENED 20260824
    //  (FW-QWKEY2); call live below, headless no-op until HTEdit GATE (6)
    //  wires the runtime keyboard instance (see MyVacuumPanel.h GATE (3)).
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 0, true, -116.0, 148.0); //小鍵盤
    (void)Sender;
}
//---------------------------------------------------------------------------
//AI(W906-VACUNIT-1203) 20260930: the LIVE half of this form for HW.VacuumUnit (EastSun 20260930 「vacuunit 所有
//  功能按鈕和內部功能要有所對應，我需要實際上有功能」): golden's own objects and handlers, driven by the page
//  service FileRW/TestIF_File_VacuumUnit.cpp. An .inc (no new .cpp); see its banner for the deviations.
#include "VacuumUnit/VacuumUnitLive.inc"
