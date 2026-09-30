// =============================================================================
//  VacuumUnit.h  --  non-VCL stand-in for golden's fVacuumUnit form: the
//                     EtherCAT-VC8 vacuum-condition dialog (4 grids of
//                     TMyVacuumPanel over IndexArm1/IndexArm2/InArm/OutArm,
//                     plus a manual set-all-values tab).
//
//  Faithful translation of golden VacuumUnit/VacuumUnit.h (114 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  Second file to land (after
//  this wave's own MyVacuumPanel.h/.cpp, which TfVacuumUnit's 4 panel-grid
//  arrays depend on).
//
//  ZERO CONSUMERS: grepped the ENTIRE golden 906 tree for `fVacuumUnit`/
//  `TfVacuumUnit` outside VacuumUnit.{h,cpp} itself -- zero hits. In THIS
//  port tree, grepped clean too (tools/dfm2rc/layout_out/VacuumUnit/*.gen.cpp
//  and one comment line in Public/MyProductionRecord.h are the only other
//  hits, neither a real compiled consumer) -- confirmed before writing a
//  single line here, same as MyVacuumPanel.h's own finding.
//
//  CLASS SHAPE DECISION, PARTIALLY ON PRECEDENT, PARTIALLY DIVERGING FROM IT
//  FOR A STATED REASON: golden declares `class TfVacuumUnit : public TForm`.
//  This port's `TForm` (language.h) is a BARE stand-in with only inherited
//  Visible/Enabled -- no ctor taking a `TComponent* Owner`, no Close()/
//  Caption/Top/Left/Width/Height. Every existing `: public TForm` golden
//  class this tree has already landed (forms/fRotate.h's TFrmRotate,
//  forms/fSetup.h's TfSetup, forms/fHome.h, forms/fNote.h, ...) resolves the
//  ABSENT-TForm problem by dropping the base ENTIRELY, because in every one
//  of those classes golden's own ctor never actually USES its `Owner`
//  parameter beyond forwarding it to `TForm(Owner)` -- see forms/fRotate.cpp's
//  own citation: "Golden's ctor... assigns NONE of these members -- relies on
//  VCL zero-fill; written out explicitly here instead".
//
//  TfVacuumUnit's `Owner` is DIFFERENT: it is NOT decorative here. Golden's
//  ctor passes `this` as the `Owner` argument to `new TMyVacuumPanel(this,
//  ...)` (32 times, golden .cpp :68-107), and MyVacuumPanel.cpp's OWN ctor
//  immediately recovers it -- `_fVacuumUnit=(TfVacuumUnit *)Owner;` -- and
//  then dereferences it repeatedly for REAL, ACTIVE state (edSV->Text,
//  btnSV->Caption, pnlCurectVal->Color; see MyVacuumPanel.cpp). Dropping the
//  base entirely, as fRotate/fSetup do, would make `this` (a bare
//  `TfVacuumUnit*`) fail to bind to `TMyVacuumPanel`'s real
//  `TComponent* Owner` parameter at every one of those 32 call sites -- a
//  compile error, not a stylistic gap. So TfVacuumUnit inherits
//  `vclcompat::TComponent` (Comm.h) instead of `TForm`: a DIFFERENT, REAL,
//  already-existing substrate class (not the absent TForm), chosen
//  specifically because it is what makes the Owner-to-child-panel chain
//  typecheck and because TComponent's own ctor genuinely takes an
//  `AOwner` parameter that golden's own `Owner` parameter can be forwarded to
//  (`: TComponent(Owner)`), unlike TForm here which has no such ctor at all.
//  This is the SAME reasoning EJ1N/MyOmronPanel.h/.cpp and this wave's own
//  MyVacuumPanel.h/.cpp already apply one level down (TMyVacuumPanel/
//  TMyOmronPanel both inherit vclcompat::TComponent, not any TForm), simply
//  applied one level UP because THIS class is the one that plays Owner for
//  those panels.
//
//  golden's own inherited-from-TForm members it actually uses beyond the
//  Owner chain (Close(), Caption, Top/Left/Width/Height -- golden .cpp
//  FormShow/spbSaveClick/sbtExitClick) still have NO substrate (neither
//  TForm nor TComponent carries any of them) and are still GATED at each
//  call site in the .cpp (see that file's GATE REGISTER) rather than
//  invented fresh on this class -- inventing them here would diverge from
//  how OmronLaser/LaserSensor.h and SECSGEM/uHGemEquipment.h already resolved
//  the sibling golden idiom `FormClose(TObject*, TCloseAction&)` (both drop
//  it to a bare `FormClose()`, TCloseAction having no port either): the
//  tree's convention for "TForm capability golden uses but this substrate
//  does not model" is GATE, not invent.
//
//  WAVE SCOPE -- ACTIVE vs GATED (golden line numbers, all in VacuumUnit.cpp):
//   ACTIVE: ctor(golden :26-30), Initial (:32-136, minus geometry+Parent+
//     scrlbx/TImage half), SetPanelPos (:138-166, Visible half only),
//     ShowSuckMode (:168-208, fully -- the switch's cases are empty in golden
//     too), FormShow (:210-253, minus Caption/Top/Left/Width/Height/tmr1),
//     tmr1Timer (:255-302, fully), FormClose (:304-311, minus tmr1),
//     FormDestroy (:313-349, fully), ReadFile (:351-361, fully),
//     DoIniDataToForm (:363-367, fully), SaveSetupFile (:369-376, fully),
//     spbSaveClick (:378-400, minus Close()), sbtExitClick (:402-406, minus
//     Close()), sbResetClick (:408-429, fully), edSetInArmClick (:597-600,
//     GATE 3, no-op).
//   GATED (whole-body, no real remainder possible -- see .cpp GATE REGISTER
//     for the full citation of each):
//     SetIOTableByECAT_VC8_Sucker (:431-480), SetSuckISABase (:482-488),
//     VaccumCopyFormSuck (:490-522), VaccumCopyToSuck (:524-548) -- ALL FOUR
//     operate exclusively on TMySucker.OnRing/OnIP/OnPort/OnBit/OnType/
//     OnISABase/Off*/Sen*/ISABase/SuckerName, NONE of which exist on this
//     port's TMySucker (aHotPlateSubstrate.h; grepped, confirmed absent --
//     that class is a MINIMAL mirror carrying only the members other already-
//     translated engines touch, and none of them touch these).
//     btnSetInArmClick (:550-595) -- dispatches on `((TButton*)Sender)->Tag`;
//     vclcompat::TButton (and its TControl base) carries no `Tag` member at
//     all (grepped Controls.h), so the dispatch itself cannot be reproduced.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL header (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #ifndef VacuumUnitH / #define VacuumUnitH
//   #include "MyVacuumPanel.h"
//   #include <Classes.hpp> / <Controls.hpp> / <StdCtrls.hpp> / <Forms.hpp> /
//     <ComCtrls.hpp> / <ExtCtrls.hpp> / <Buttons.hpp>
//   #include "ALed.hpp" / "BtnPanelLane.h" / "MyLedLane.h" / "VacuumUnit.h" /
//     "MyLed.h"
// =============================================================================

//---------------------------------------------------------------------------
#ifndef VacuumUnitH
#define VacuumUnitH
//---------------------------------------------------------------------------
#include "VacuumUnit/MyVacuumPanel.h"
#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "Public/HTEditList.h"      // HTEditList + extern elVacuumUnit (golden HTEditList.h)

//---------------------------------------------------------------------------
const int TOTAL_VACUUM_UNIT=8;
//AI(W906-PT-W3) 20260807: golden's `const int VACUUM_UNIT_WIDTH=81;` /
//  `VACUUM_UNIT_HEIGHT=177;` (golden .h :22-23) are consumed ONLY by golden's
//  own GroupBox->Width/->Height writes and the ImgVacuumPanel geometry --
//  every one of those sites is GATE (1) in MyVacuumPanel.h/.cpp (no
//  Width/Height member on vclcompat::TGroupBox, no TImage type at all).  With
//  zero remaining ACTIVE reader anywhere in this wave's two files, defining
//  them here would be dead constants no compiled statement ever touches --
//  kept as comments for provenance instead of live (unused) declarations:
//    const int VACUUM_UNIT_WIDTH=81;
//    const int VACUUM_UNIT_HEIGHT=177;

class TfVacuumUnit : public TComponent
{
    public:    // IDE-managed Components (golden __published)
        TPageControl *pgcVacuumUnit;
        TTabSheet *tsInArm;
        TTabSheet *tsIndex;
        TGroupBox *grpInarm;
        //AI(W906-PT-W3) 20260807: GATE -- golden `TScrollBox *scrlbxInArm;`
        //  (and its 3 siblings below) omitted: no vclcompat TScrollBox type
        //  exists anywhere in this port (grepped). Every live use is
        //  `->GroupBox->Parent=scrlbxXxx;` and `->Parent` itself has no
        //  substrate either (vclcompat::TControl models only Visible/
        //  Enabled), so omitting the target changes nothing observable.
        //  Golden field kept here in comment form for provenance:
        //    TScrollBox *scrlbxInArm;
        TGroupBox *grpIndexArm2;
        //    TScrollBox *scrlbxIndexArm2;
        TGroupBox *grpIndexArm1;
        //    TScrollBox *scrlbxIndexArm1;
        //AI(W906-PT-W3) 20260807: GATE -- golden `TTimer *tmr1;` omitted: no
        //  vclcompat TTimer type exists anywhere in this port (grepped).
        //  tmr1Timer() itself is STILL translated below as a normal callable
        //  method (its own signature needs no TTimer) -- only the timer
        //  OBJECT that would drive it, and the two ->Enabled writes
        //  (FormShow/FormClose), are gated. Golden field kept in comment form:
        //    TTimer *tmr1;
        TTabSheet *tsManual;
        TGroupBox *GroupBox1;
        TSpeedButton *btnSV;
        TPanel *pnlCurectVal;
        TPanel *pnlThreshold;
        TPanel *pnlEvent;
        TPanel *Panel1;
        TSpeedButton *spbSave;
        TSpeedButton *sbtExit;
        TCheckBox *CheckBox1;
        TEdit *Edit1;
        TEdit *edSV;
        //AI(W906-PT-W3) 20260807: GATE -- golden `TMyLed *myld1;` and
        //  `TBtnPanelLane *bplOn;`/`*bplOff;` omitted: no widget-level port
        //  for either golden type exists anywhere in this tree (only the
        //  framework-free MyLedCore/BtnPanelLaneCore STATE, vclcompat/
        //  LedCore.h / BtnPanelCore.h -- same finding as MyVacuumPanel.h's
        //  own GATE (2)). Golden fields kept in comment form:
        //    TMyLed *myld1;
        //    TBtnPanelLane *bplOn;
        //    TBtnPanelLane *bplOff;
        TSpeedButton *sbReset;
        TTabSheet *tsOutArm;
        TGroupBox *grpOutarm;
        //    TScrollBox *scrlbxOutArm;
        TGroupBox *grpSetAllValue;
        TButton *btnSetInArm;
        TEdit *edSetInArm;
        TEdit *edSetIndexArm;
        TButton *btnSetIndexArm;
        TButton *btnSetOutArm;
        TEdit *edSetOutArm;
        TLabel *lblInArm;
        TLabel *lblIndexArm;
        TLabel *lblOutArm;
        void FormShow(TObject *Sender);
        void tmr1Timer(TObject *Sender);
        //AI(W906-PT-W3) 20260807: golden `FormClose(TObject *Sender,
        //  TCloseAction &Action)` trimmed to a bare `FormClose()` -- same
        //  precedent as OmronLaser/LaserSensor.h and SECSGEM/
        //  uHGemEquipment.h (both cited in this file's banner): TCloseAction
        //  has no port anywhere in this tree.
        void FormClose();
        void FormDestroy(TObject *Sender);
        void spbSaveClick(TObject *Sender);
        void sbtExitClick(TObject *Sender);
        void sbResetClick(TObject *Sender);
        void btnSetInArmClick(TObject *Sender);
        void edSetInArmClick(TObject *Sender);
    private:    // User declarations
        void SetPanelPos(bool bInitial=true);
        AnsiString LastFileName;

    public:     // User declarations
        TfVacuumUnit(TComponent* Owner);
        TMyVacuumPanel *myPalArm2   [TOTAL_VACUUM_UNIT][2];
        TMyVacuumPanel *myPalArm1   [TOTAL_VACUUM_UNIT][2];
        TMyVacuumPanel *myPalInArm  [TOTAL_VACUUM_UNIT/2][2];
        TMyVacuumPanel *myPalOutArm [TOTAL_VACUUM_UNIT/2][2];

        double dIndexArm1[TOTAL_VACUUM_UNIT][2];
        double dIndexArm2[TOTAL_VACUUM_UNIT][2];

        double dInArm[TOTAL_VACUUM_UNIT/2][2];
        double dOutArm[TOTAL_VACUUM_UNIT/2][2];

        bool fShow;
        int iCount;
        void ShowSuckMode(int iSel);
        void ReadFile();
        void DoIniDataToForm();
        void SaveSetupFile(AnsiString szDir, AnsiString S);
        void Initial();
        int iIndexColMax;
        int iInOutColMax;
        int iTest1;
        bool bTest1;

        int iTest33;
};
extern TfVacuumUnit *fVacuumUnit;
extern void SetIOTableByECAT_VC8_Sucker();
extern void VaccumCopyFormSuck(class TMyVacuumPanel &Target, class TMySucker &Source);
extern void VaccumCopyToSuck(class TMyVacuumPanel &Source, class TMySucker &Target);
extern void SetSuckISABase(class TMySucker &Source, int iISABase);
//---------------------------------------------------------------------------
#endif
