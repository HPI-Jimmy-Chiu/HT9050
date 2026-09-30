// =============================================================================
//  MyVacuumPanel.h  --  TMyVacuumPanel, a custom VCL panel component showing
//                        one EtherCAT-VC8 vacuum channel (current pressure /
//                        threshold / on-off latch / event text over a painted
//                        TImage, plus a TGroupBox frame, a numeric-entry
//                        TEdit+TSpeedButton pair, a TMyLed vacuum-sensor lamp
//                        and a TBtnPanelLane on/off switch pair).
//
//  Faithful translation of golden VacuumUnit/MyVacuumPanel.h (122 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  First file to land.
//
//  ZERO CONSUMERS: grepped the ENTIRE golden 906 tree for `MyVacuumPanel`/
//  `TMyVacuumPanel` outside VacuumUnit/ itself -- the only real consumer is
//  VacuumUnit.cpp (this same wave's file #2), which builds 8+8+4+4 instances
//  of it into fVacuumUnit's four panel grids.  In THIS port tree, grepped
//  clean (tools/dfm2rc/layout_out/VacuumUnit/*.gen.cpp and one comment line
//  in Public/MyProductionRecord.h are the only other hits, neither a real
//  compiled consumer) -- confirmed before writing a single line here.
//
//  ROLE: a self-contained custom TComponent (NOT a TControl -- mirrors
//  golden's own class shape) that owns a TGroupBox/TEdit/TSpeedButton trio as
//  public members. Its ctor computes, from `_iKind` (0..5: HT9046LS
//  IndexArm1/IndexArm2, InArm, OutArm, HT9045 IndexArm1/IndexArm2) and the
//  `_iCol`/`_iRow` grid position, the EtherCAT-VC8 Ring/IP/Port/VCNo/Type/
//  ISABase address triplet for On (vacuum-on solenoid), Off (destroy/blow-off
//  solenoid) and Sen (vacuum sensor) -- all real int arithmetic, unaffected by
//  any gate below. 8 public Refresh*/Read*/Write* methods drive the actual
//  EtherCAT PCI1203 vendor calls (Acm_DaqDoSetBitEx / Acm_DaqDoGetBitEx /
//  Acm_DaqDiGetBitEx / Acm_DevWriteSDOData, all REAL substrate, see
//  MyLaneIo.cpp / EtherCAT/vendor/AdvMotApi.h) through the address triplet.
//
//  WAVE SCOPE -- ACTIVE vs GATED (golden line numbers):
//   ACTIVE (faithful, verbatim data-flow):
//     ctor's iKind ladder (address-triplet arithmetic)   golden :16-149
//     ctor's real-substrate widget alloc + Caption/Visible/Text copies
//     ~TMyVacuumPanel (golden body IS empty -- see below)  golden :274-283
//     ShowCurectVal/ShowThreshold/SetEvent's Visible guard golden :285-371 (guard only)
//     SetBG                                                golden :373-378 (100% gated, see below)
//     edSVClick                                            golden :380-383 (un-gated 20260824, fQwertyKey real since FW-QWKEY1)
//     btnSVClick                                           golden :385-389 -- 100% ACTIVE
//     btnVaccumOnOffOnClick                                golden :391-416 (gated, see GATE 3)
//     RefreshCurrectVal/RefreshThresholdVal                golden :418-445 -- 100% ACTIVE
//     RefreshVaccumIO                                      golden :447-453 (state write gated, HW read active)
//     RefreshEvent                                         golden :455-465 -- 100% ACTIVE
//     RefreshDOIO                                          golden :467-493 (HW read active, LED write gated)
//     ChangeBtnPanelLane                                   golden :495-519 -- GATE 3, whole method omitted
//     ReadVaccumCurrect/ReadVaccumThreshold/ReadVaccumIO   golden :521-550 -- 100% ACTIVE
//     WriteVaccumThreshold/InitialThresholdMode/Reset      golden :552-580 -- 100% ACTIVE
//
//  GATE REGISTER -- 3 gates, all grounded in a tree-wide grep (not guessed):
//   (1) `TImage *ImgVacuumPanel` (golden :33 in .cpp's alloc, declared golden
//       .h :115) and every ->Canvas->Font/->Brush/->Pen/->Rectangle/
//       ->TextOutA/->TextWidth/->TextHeight call through it (golden
//       ShowCurectVal/ShowThreshold/SetEvent/SetBG bodies, :285-378) -- there
//       is NO vclcompat TImage/TCanvas type anywhere in this port (grepped:
//       zero `class TImage`/`class TCanvas` hits outside comments; same
//       finding EJ1N/MyOmronPanel.h's GATE (1) already recorded for the
//       identical golden idiom). Member OMITTED from the class entirely
//       (matches MyOmronPanel precedent). ACTIVE arm of each Set*/Show*
//       method keeps ONLY its `if(GroupBox->Visible==false) return;` guard
//       (GroupBox->Visible is real, vclcompat::TControl); the paint itself is
//       gated. Two call-site consequences, both recorded at their own site in
//       the .cpp: (a) `_fVacuumUnit->pnl*->Font->Color` arguments cannot be
//       evaluated (TPanel/TLabel carry no `Font` member in this port's
//       Controls.h) -- substituted with a same-signature literal `TColor(0)`
//       at each such call, inert either way because the paint that would
//       consume it is itself gated; (b) SetBG's own body is 100% Canvas paint
//       with no other state, so it is 100% gated (empty ACTIVE arm).
//       BEHAVIOUR DELTA: no on-screen digits/outline for this panel on a real
//       machine until a future W7-C2-style HDC renderer replays this state;
//       zero consumers today so nothing downstream observes it.
//   (2) `TMyLed *myld1` (golden .h :118) and `TBtnPanelLane *bplOn`/`*bplOff`
//       (golden .h :119-120) -- grepped tree-wide: this port has ONLY the
//       framework-free STATE cores MyLedCore / BtnPanelLaneCore
//       (vclcompat/LedCore.h, vclcompat/BtnPanelCore.h), never the widget
//       classes themselves (no ctor-less `new TMyLed(this)` target, no
//       ->Parent/->Top/->Left/->Width/->Height/->Hint/->ShowHint/->Style/
//       ->TrueColor/->FalseColor/->TrueFontColor/->FalseFontColor/->Caption/
//       ->Down member surface anywhere). Same class of gap as
//       MyOmronPanel.h's GATE (1)/(2): the golden TYPE has no port, so the
//       MEMBER is omitted from the class entirely, and every function whose
//       BODY needs it (ChangeBtnPanelLane, whose own PARAMETER type
//       `TBtnPanelLane*` cannot even be spelled -- GATE(2)-class exactly like
//       MyOmronPanel's GroupBox1MouseUp/Down/Move) is either omitted
//       (ChangeBtnPanelLane) or reduced to its real remainder (RefreshDOIO
//       keeps the real Acm_DaqDoGetBitEx HW reads, drops only the
//       bplOn/bplOff->Down write + the two ChangeBtnPanelLane calls;
//       RefreshVaccumIO keeps the real ReadVaccumIO() call for its
//       side-effect on sEvent, drops only the myld1->Value write).
//       BEHAVIOUR DELTA: no on-screen LED/switch state; the underlying
//       EtherCAT reads still happen and sEvent still reflects a real sensor
//       read failure, so downstream alarm-text logic (RefreshEvent) is
//       unaffected.
//   (3) `fQwertyKey->ShowQwertyKey(...)` (golden edSVClick :382) and the
//       ENTIRE golden `btnVaccumOnOffOnClick` body (golden :391-416, which
//       casts `Sender` to `TBtnPanelLane*` -- GATE(2)'s type, unspellable
//       here) -- fQwertyKey real since FW-QWKEY1 (fc08e09); GATE (3)
//       OPENED 20260824 (FW-QWKEY2) for edSVClick, whose call is now live
//       (latent until HTEdit GATE (6) wires the keyboard instance).
//       btnVaccumOnOffOnClick is unchanged (still gated on GATE(2)'s type):
//       ACTIVE arm of btnVaccumOnOffOnClick: no-op -- it is wired only as
//       bplOn/bplOff's OnClick in golden, and those members do not exist
//       here (GATE 2), so it is unreachable in this build either way; kept
//       declared (not omitted) because its SIGNATURE `(TObject*)` is fully
//       spellable and a future wave that lands BtnPanelCore's widget half
//       may want to re-wire it.
//
//  `ChangeBtnPanelLane` (golden .h :87, private) is OMITTED from the class
//  entirely -- its sole parameter type `TBtnPanelLane*` cannot be spelled
//  (GATE 2), the identical class of omission MyOmronPanel.h's GATE (2)
//  already applied to GroupBox1MouseUp/Down/Move.
//
//  ~TMyVacuumPanel (golden :274-283): UNLIKE MyOmronPanel's dtor, golden's OWN
//  body here is *already* fully commented out (`//delete GroupBox;` etc, all
//  7 lines) -- this is not a case of "relies on VCL auto-free" needing a
//  correctness fix, it is golden's own deliberate no-op (the class is a
//  create-once, live-for-process-lifetime UI panel; golden's author chose not
//  to free it). Preserved EXACTLY as empty, matching golden's actual
//  intent -- not "improved" into a real free the way MyOmronPanel's dtor was
//  (that one had a documented substrate-limitation reason to diverge; this
//  one has none).
//
//  VCL/Borland conversions: `__fastcall` dropped from every method. `default=NULL`
//  clauses on golden's own `__property` declarations are a language-level
//  Borland-ism (nothing enforces them; they document IDE-Inspector defaults
//  only) and have no C++ equivalent -- dropped, matching vcl_compat.h's own
//  neutralisation of `__property`. The 18 `__property int OnRing={read=iOnRing,
//  write=iOnRing,...}`-shaped declarations (golden .h :17-36) are each a
//  TRANSPARENT passthrough (read and write name the SAME backing field) --
//  the identical BCB6 idiom vclcompat/Comm.h's own banner already resolves by
//  collapsing to "plain public data members" ("Since vcl_compat.h neutralizes
//  __property/__fastcall/__closure, the VCL property syntax becomes plain
//  public data members"). Followed verbatim here: each pair collapses to ONE
//  public field spelled with the PROPERTY name (OnRing, not iOnRing) --
//  matching how every external consumer (VacuumUnit.cpp's
//  VaccumCopyFormSuck/VaccumCopyToSuck/ctor) actually spells it, and how this
//  unit's OWN ctor body spells it internally too (no second, differently-
//  spelled backing field exists to diverge from).  `SuckerName` (golden .h
//  :15, same read/write=sSuckerName shape) collapses the same way.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL header (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #include "MachineDefine.h"
//   #ifndef MyVacuumPanelH / #define MyVacuumPanelH
//   #include <Classes.hpp>
//   #include "BtnPanelLane.h"
//   using namespace std;
// =============================================================================

//---------------------------------------------------------------------------
#ifndef MyVacuumPanelH
#define MyVacuumPanelH

#include "vclcompat/vcl_compat.h"   // AnsiString; TComponent (via Comm.h, in the umbrella)
#include "vclcompat/Controls.h"     // TGroupBox / TEdit / TSpeedButton (real substrate)
#include "vclcompat/LedCore.h"      // TColor + clBlack/clGreen/clRed/clYellow (shared
                                    //   HT9045_W7C1_TCOLOR_SHIM guard block)

using vclcompat::TColor;
using vclcompat::clBlack;
using vclcompat::clGreen;
using vclcompat::clRed;
using vclcompat::clYellow;

//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: golden ctor calls `SetBG(clGray, ...)` (golden
//  .cpp :172). The shared HT9045_W7C1_TCOLOR_SHIM block does not define
//  clGray -- added here, the real Win32 COLORREF for VCL's clGray, scoped to
//  this header only (same fix EJ1N/MyOmronPanel.h already made for the same
//  missing constant).
const TColor clGray = TColor(0x00808080);

//---------------------------------------------------------------------------
class TMyVacuumPanel : public TComponent
{
    public:
        //AI(W906-PT-W3) 20260807: golden __published block (golden .h :14-36)
        //  -- 18 transparent-passthrough __property ints + SuckerName,
        //  collapsed to plain public data members named after the PROPERTY
        //  (not the golden backing-field) spelling; see this file's own
        //  banner for why. golden's `default=...` clauses have no C++
        //  equivalent and are dropped (IDE-Inspector metadata only).
        AnsiString SuckerName;

        int OnRing;
        int OnIP;
        int OnPort;
        int OnVCNo;
        int OnType;
        int OnISABase;

        int OffRing;
        int OffIP;
        int OffPort;
        int OffVCNo;
        int OffType;
        int OffISABase;

        int SenRing;
        int SenIP;
        int SenPort;
        int SenVCNo;
        int SenType;
        int SenISABase;

    private:    // User declarations
        void edSVClick(TObject *Sender);
        void btnSVClick(TObject *Sender);
        void btnVaccumOnOffOnClick(TObject *Sender);
        int iKind;      //0:IndexArm1 ; 1:IndexArm2 ; 2:InArm ; 3:OutArm
        int iCol;
        int iRow;
        int iVacuOnOffSwap;
        double dCurrectVal;
        double dOldCurrectVal;
        double dThresholdVal;
        double dWriteThresholdVal;
        bool bNeedReadVaccumThreshold;

        unsigned short DataSize;                //固定為 128

        unsigned short IO_Index;
        unsigned short IO_SubIndex;

        unsigned short CurValue_Index;
        unsigned short CurValue_SubIndex;

        unsigned short Threshold_Index;
        unsigned short Threshold_SubIndex;
        unsigned short ThresholdMode_SubIndex;

        //AI(W906-PT-W3) 20260807: GATE (2) -- golden `void ChangeBtnPanelLane
        //  (TBtnPanelLane *Ptr);` (golden .h :87) omitted: `TBtnPanelLane`
        //  names a widget type with no port anywhere in this tree (only the
        //  framework-free BtnPanelLaneCore STATE exists, vclcompat/
        //  BtnPanelCore.h -- see this file's banner GATE (2)). Golden
        //  signature kept here verbatim, in comment form, for provenance:
        //    void ChangeBtnPanelLane(TBtnPanelLane *Ptr);

    public:
        TMyVacuumPanel(TComponent* Owner,int _iKind,int _iCol,int _iRow);
        void SetBG           (TColor OutLine, TColor BackGround);        //設定背景
        void SetEvent        (AnsiString Event,     TColor Color);       //顯示事件
        void ShowCurectVal   (AnsiString CurectVal, TColor Color);       //顯示目前壓力值
        void ShowThreshold   (AnsiString ThresholdVal, TColor Color);    //顯示目前壓力值下限值
        void RefreshCurrectVal();        //更新顯示壓力值
        void RefreshThresholdVal();      //更新顯示壓力下限值
        void RefreshVaccumIO();          //更新顯示真空建立狀態
        void RefreshDOIO();              //更新顯示 DO 狀態
        void RefreshEvent();             //更新顯示 Event
        double ReadVaccumCurrect();
        double ReadVaccumThreshold();
        bool ReadVaccumIO();

        void WriteVaccumThreshold(double dKpa);
        void InitialThresholdMode();
        bool bInitialThresholdModeOK;
        ~TMyVacuumPanel();
        void Reset();
        AnsiString sEvent;
        AnsiString sOldEvent;

        TGroupBox       *GroupBox;
        //AI(W906-PT-W3) 20260807: GATE (1) -- golden `TImage *ImgVacuumPanel;`
        //  (golden .h :115) omitted: no vclcompat TImage/TCanvas type exists
        //  anywhere in this port (grepped). See this header's own banner
        //  GATE (1). Every Show*/Set* method below that painted through it is
        //  reduced to its real (Visible-guard-only) remainder in the .cpp.
        TEdit           *edSV;
        TSpeedButton    *btnSV;
        //AI(W906-PT-W3) 20260807: GATE (2) -- golden `TMyLed *myld1;`
        //  (golden .h :118) and `TBtnPanelLane *bplOn;`/`*bplOff;` (golden .h
        //  :119-120) omitted: no widget-level port for either golden type
        //  exists anywhere in this tree (only the framework-free MyLedCore /
        //  BtnPanelLaneCore STATE, see this file's banner GATE (2)).
};
#endif
