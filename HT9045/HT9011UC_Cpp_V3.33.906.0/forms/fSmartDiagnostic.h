// =============================================================================
//  forms/fSmartDiagnostic.h  --  non-VCL stand-in for golden's fSmartDiagnostic
//                                 form pointer (cylinder Push/Pop On/Off-time
//                                 diagnostic panel)
//
//  AI(W906-FW3-SD1) 20260827: new file. Wave FW3-SD1 (golden SmartDiagnostic.cpp,
//  HT9011UC_Code_V3.33.906.0_20260618). 36 bodies total (35 TfSmartDiagnostic::
//  members + 1 file-scope SplitStrByDotSpaceOnly), 966 span lines (measured by
//  tools/census/wave_preflight.py SmartDiagnostic.cpp, 20260827).
//
//  WHY THIS FILE MATTERS BEYOND ITS OWN LOGIC: TfSmartDiagnostic/fSmartDiagnostic
//  were BOTH completely absent from the port before this wave (class name +
//  global name both "clear" per wave_preflight's shim-occupation scan,
//  20260827). 74 `fSmartDiagnostic->` call sites tree-wide sit inside `#if 0`
//  waiting on exactly this facade -- 3 of them are named/dated GATE H5-G4/
//  H5-G5/H5-G6 in csystem.cpp (see that file's own banner at :27568-27582,
//  27593-27601, 28182-28209) which explicitly say "RETIRED BY: the wave that
//  lands a TfSmartDiagnostic facade" -- THIS wave. Landing this file resolves
//  those three gates' STATED PREMISE (class/object now exist); un-gating them
//  is explicitly a SEPARATE, safety-relevant decision left to a future wave
//  (H5-G6's own DELTA note: bHasErr driving fMain->Pause("DoInitialCylinderCheck")
//  is safety-relevant and MUST NOT be silently re-enabled by this translation
//  pass). This file does not un-gate anything outside itself.
//
//  GATE REGISTER (full detail + golden line numbers in forms/fSmartDiagnostic.cpp;
//  this is the index)
//  -----------------------------------------------------------------------
//    SD-1  No `class TWinControl` (recursive-control-tree-walk shape) anywhere
//          reachable from this facade -- SAME tree-wide absence documented by
//          forms/fHandlerSys.h/fIoSetView.h/fSpeed.h's own GATE S4 etc.
//          Affects: SetVCLSizePosition (golden :29-35, TWinControl* param +
//          ->Width/->Height, neither of which exists on vclcompat::TControl
//          either -- 0 hits, `grep -n "Width\|Height" vclcompat/Controls.h`,
//          20260827), SearchChangePageButton (golden :132-156, recurses
//          ->ControlCount/->Controls[i]). BOTH fully excluded (not declared).
//    SD-2  FormButtonClick Tag==9 branch (golden :172-180): MessageDlg/
//          mtConfirmation/TMsgDlgButtons/mbYes/mbNo/mrNo have zero vclcompat
//          port tree-wide (same absent family as forms/fContact.h's (X-24)/
//          (X-25)); the 3 Save*() calls it guards are also independently
//          write-path-excluded (see EXCLUDED below). Gated as a block, rest of
//          FormButtonClick (Tag==10 Exit, else page-switch) stays ACTIVE.
//    SD-3  vclcompat::TStringGrid has no ->DefaultDrawing property (0 hits,
//          `grep -n "DefaultDrawing" vclcompat/StringGrid.h`, 20260827).
//          Affects: InitialStringGridColor (golden :224), InitialCylinderSGColor
//          (golden :245) -- one line each gated, rest ACTIVE.
//    SD-4  vclcompat::TStringGrid has no ->Refresh() method (0 hits, same grep
//          sweep). Affects: SetCellColor (golden :253), SetCylinderCellColor
//          (golden :263) -- one line each gated, rest ACTIVE.
//    SD-5  FormShow (golden :274-357): the 2 SetVCLSizePosition calls (:282-283,
//          transitively gated via SD-1) and `sgCylinder->Width=1300;` (:290, no
//          ->Width on TStringGrid, same SD-1/SD-3 style gap) are gated; every
//          other line (Left/Top self-geometry, sgCylinder Cells/RowCount/
//          ColCount/ColWidths setup, ReadCylinderData() call, IniConfig branch)
//          stays ACTIVE.
//    SD-6  GetCyliderOnCount (golden :627-657) / GetCyliderOffCount (golden
//          :661-690): trailing `SaveSDSummaryData();` call is a disk write
//          (SaveSDSummaryData itself is independently write-path-excluded --
//          see below); gated as the one line, rest (the actual on/off count
//          bookkeeping into sg_SmartDiagnostic_Summary, which is what the 4
//          mycylin.cpp #if 0 call sites at :278/:295/:320/:337 actually need)
//          stays ACTIVE.
//    SD-7  sgCylinderSelectCell (golden :854-865): `MyInputBox(edTemp);` --
//          MyInputBox has zero vclcompat port tree-wide (confirmed by
//          forms/fContact.h's own (X-24)/(X-25)/(X-31) recon, re-checked here
//          20260827: 0 hits outside forms/fContact.h's own prose). One line
//          gated, the Cells<->edTemp->Text copy in/out around it stays ACTIVE
//          (net effect without MyInputBox: a harmless round-trip, not a
//          functioning edit box -- see reply banner).
//
//  EXCLUDED ENTIRELY (not declared as class members -- nothing here calls
//  them, and none is on the predetermined-name list below, so there is no
//  future-linking contract to honour by declaring an undefined member):
//    * SetVCLToParameter (golden :39-54)          -- IniF->WriteInteger, write path
//    * SaveSmartDiagnosticParameter (golden :470-479) -- calls SetVCLToParameter x2, write path
//    * SaveSDSummaryData (golden :546-567)         -- P->SaveToFile(...), write path
//    * SaveSDResetSummaryData (golden :571-592)    -- P->SaveToFile(...), write path
//    * SaveCylinderData (golden :799-845)          -- P->SaveToFile(aCylinderPreAlm), write path
//    * sb_SmartDiagnostic_SaveMouseDown (golden :847-852) -- calls SaveCylinderData, transitively excluded
//    * sb_SmarDiagnostic_CreateCyliderNameClick (golden :594-623) -- reads
//      `Cylinder[i].CylinderName`; Cylinder[]'s DEFINITION lives in
//      mycylin.cpp, which CMakeLists.txt:1044-1062 puts in `ht9045_io`;
//      `ht9045_forms` links ONLY vclcompat+ht9045_globals+ht9045_core
//      (CMakeLists.txt:795-802) -- ht9045_io is NOT reachable, so this body
//      would compile but fail to LINK. Also needs MessageDlg (SD-2 family).
//    * sb_SmartDiagnostic_ResetRecordCountClick (golden :692-724) -- resets
//      the diagnostic on/off counters + persists the reset count; task brief
//      explicitly calls this out as "changes production/diagnostic state".
//      Also needs MessageDlg (SD-2 family) and `clWindow` (no shared
//      vclcompat::clWindow -- only a TU-local const in forms/fMotorTest.cpp,
//      see that file's own D-3 note; not usable from here).
//    * sg_SmartDiagnostic_SummaryDrawCell (golden :366-384) / sgCylinderDrawCell
//      (golden :1036-1053) -- need Canvas/FillRect/TextOut/DrawText/TRect/
//      TGridDrawState, none of which vclcompat ports (task's own known-gap
//      list; re-confirmed 20260827, 0 hits each under vclcompat/).
//
//  PREDETERMINED-NAME CONTRACT (13 names the task brief identified as already
//  referenced by #if 0 call sites elsewhere -- name+type must match EXACTLY):
//  GetCyliderOffCount/GetCyliderOnCount (mycylin.cpp), ReadCylinderData/
//  SetCylinderCellColor/ShowModal/iPopAvgTime/iPop_LowLimitTime/
//  iPop_UpLimitTime/iPushAvgTime/iPush_LowLimitTime/iPush_UpLimitTime/
//  iWarPercent/sgCylinder (csystem.cpp GATE H5-G4/G5/G6). ShowModal is not a
//  golden TfSmartDiagnostic:: member at all (it is inherited TForm::ShowModal,
//  called via csystem.cpp:28280) -- added below as the same permanent offline
//  no-op every other facade form already carries (forms/fContact.h:1603,
//  forms/fMotorTest.h:977, forms/fPassword.h:337 precedent).
//
//  NOTE ON `Cylinder[]`/TMyCylinder: mycylin.h is included below ONLY for the
//  `MaxCylinderItem` macro (=295, sizes iRecordCyliderOnCount[]/
//  iRecordCyliderOffCount[]/bRecordCyStatus[]) and the TMyCylinder type name.
//  This file NEVER references the `Cylinder[]` global array itself (that is
//  exactly what sb_SmarDiagnostic_CreateCyliderNameClick needed and is
//  excluded for -- see above) -- so including the header does not create an
//  ht9045_io link dependency; only USING the `Cylinder` symbol would.
//
//  =============================================================================
//  Steven 20260925 (Data.SmartDiagnostic web page, golden V912
//  HT9011UC_Code_V3.33.912.0_20260908_Jimmy/SmartDiagnostic.cpp, cp950):
//  WRITE DIRECTION UN-GATED (Steven 20260925 授權：Save/Reset 照 golden 翻，寫的檔都在
//  D:\HT9045\system). The ledger above is kept as history; what changed:
//    * SD-2  RETIRED. FormButtonClick Tag==9 runs golden :172-180 again. The
//            MessageDlg (:174) is W906_MessageDlg below: the web asks the question
//            itself (two-phase, WebSmartDiag.cpp) and presets the answer.
//    * SD-6  RETIRED. GetCyliderOnCount/OffCount call SaveSDSummaryData again
//            (:656/:689). ⚠ They first call W906_Create() -- see that member: the
//            golden form is created at boot (HT9045.cpp:271 Application->CreateForm)
//            so its grid ALWAYS mirrors SmartDiagnosticRecord.txt before a count;
//            here the facade is a static object whose ctor may not read files, so
//            without W906_Create a count would SaveToFile an EMPTY grid over the
//            record file.
//    * SD-7  RETIRED. sgCylinderSelectCell's MyInputBox (:862) is W906_MyInputBox
//            (golden INPUT.cpp:84-102: integer box, Ptr->Text = Result).
//    * EXCLUDED -> ACTIVE: SetVCLToParameter (:39-54), SaveSmartDiagnosticParameter
//            (:470-479), SaveSDSummaryData (:546-567), SaveSDResetSummaryData
//            (:571-592), SaveCylinderData (:799-845), sb_SmartDiagnostic_SaveMouseDown
//            (:847-852), sb_SmartDiagnostic_ResetRecordCountClick (:692-724), and
//            sb_SmarDiagnostic_CreateCyliderNameClick (:594-623) -- the last one's
//            BODY is in the repo-root SmartDiagnostic.cpp (library ht9045_sm, which
//            links ht9045_io), NOT in forms/fSmartDiagnostic.cpp: it reads
//            Cylinder[] and ht9045_forms may not reach ht9045_io (same split as
//            forms/fOffSet.h + cOffSet.cpp).
//    * ⚠ SaveCylinderData / sb_SmartDiagnostic_SaveMouseDown are translated but
//            UNREACHABLE, exactly as in golden: golden SmartDiagnostic.dfm binds NO
//            OnMouseDown on sb_SmartDiagnostic_Save (dfm :922-1061, only Tag=9), and
//            nothing else calls SaveCylinderData -> golden never writes
//            CylinderPreAlm.ini. The web page does not call it either.
//    * SD-1 (TWinControl walk), SD-3 (DefaultDrawing), SD-4 (Refresh), SD-5
//            (SetVCLSizePosition / ->Width) and the two DrawCell bodies stay gated:
//            they are paint-only; the web renders the grids from JSON.
//  =============================================================================
// =============================================================================
#ifndef FORMS_FSMARTDIAGNOSTIC_H
#define FORMS_FSMARTDIAGNOSTIC_H

#include "forms/FormWidgets.h"      // AnsiString/TStringList/TIniFile/TDateTime/
                                     // TPanel/TSpeedButton/TPageControl/TTabSheet/
                                     // TComboBox/TLabel/TEdit/TStringGrid (all
                                     // brought to global scope by Controls.h/
                                     // StringGrid.h via this umbrella)
#include "vclcompat/TList.h"        // vclcompat::TList -- MyPageChangeList. Deliberately
                                     // NOT `using`'d here: vclcompat/vcl_compat.h's own
                                     // note says a global `using vclcompat::TList;` would
                                     // collide with the UNRELATED global `class TList` in
                                     // aHotPlateSubstrate.h (a real tree-wide gotcha, see
                                     // forms/fTemp_Set.h:344-359) -- always spelled
                                     // `vclcompat::TList` explicitly in this file instead.
#include "vclcompat/BtnPanelCore.h" // vclcompat::TColor / clRed -- GridColor/CylinderGridColor
                                     // cell colors + SmartDiagnosticTimerTimer's alarm paint
#include "vclcompat/ShiftState.h"   // TMouseButton / TShiftState -- the 2 *MouseDown handlers'
                                     // full golden signature (forms/fCleaning.h precedent)
#include "myTimer.h"                // TQPF_Timer -- file-scope `hDelay` in the .cpp (golden :727)
#include "mycylin.h"                // MaxCylinderItem #define (295) ONLY -- see file-head note
                                     // above; `Cylinder[]` itself is never referenced here.

using vclcompat::TColor;

// ===========================================================================
//  golden SmartDiagnostic.h:16-21 (file-scope typedef, unchanged shape).
//  Confirmed collision-free tree-wide, 20260827 (`grep -rn "MySDPageChange"`).
// ===========================================================================
typedef struct
{
    TPageControl *PCtrl;
    TSpeedButton *sbPtr;
    int          iTag;
} MySDPageChange;

// ===========================================================================
//  TfSmartDiagnosticTimer -- facade-local stand-in for golden `TTimer
//  *SmartDiagnosticTimer` (SmartDiagnostic.h:49). No vclcompat TTimer port
//  exists tree-wide (`grep -rn "class TTimer" vclcompat/` -- 0 hits, matches
//  forms/fShowBinSelect.h's identical finding for a different form's TTimer).
//  Only ->Enabled is ever touched anywhere in this file's 36 bodies (ctor
//  golden :24). Same minimal shape as forms/fLotInfo.h's TfLotInfoTimer.
//  Confirmed collision-free tree-wide, 20260827.
// ===========================================================================
struct TfSmartDiagnosticTimer
{
    bool Enabled;
    TfSmartDiagnosticTimer() : Enabled(false) {}
};

// ===========================================================================
//  TfSmartDiagnostic -- non-VCL stand-in for golden `class TfSmartDiagnostic :
//  public TForm` (SmartDiagnostic.h:23). "Form as junk drawer" idiom, same as
//  TfCleaning/TfMain/TfLotInfo (forms/fCleaning.h precedent) -- no TForm base
//  exists in this facade world (`grep -rn "class TForm" vclcompat/` -- 0 hits),
//  so Height/Width/Left/Top below are plain data fields, not inherited
//  properties (forms/fDynamicTemp.h / forms/fContactCT.h precedent for this
//  exact substitution).
//
//  ACCESS: golden splits private:/public:; collapsed to one `public:` block
//  per this tree's established D-6 convention (e.g. forms/fContact.h) -- no
//  behavioural difference, this facade has no encapsulation to enforce.
// ===========================================================================
class TfSmartDiagnostic
{
public:
    // ---- golden TForm-inherited geometry (SmartDiagnostic.cpp ctor :17-18,
    //      FormShow :279-280) -- plain fields, no TControl/TForm base ----
    int Height;
    int Width;
    int Left;
    int Top;

    // ---- __published widgets (SmartDiagnostic.h:26-53) ----
    TPanel        *pn_SmartDiagnosticTitle;
    TSpeedButton  *sb_SmartDiagnostic_Setup;
    TSpeedButton  *sb_SmartDiagnostic_Summary;
    TSpeedButton  *sb_SmartDiagnostic_Exit;
    TSpeedButton  *sb_SmartDiagnostic_Save;
    TPageControl  *pc_SmartDiagnostic;
    TTabSheet     *ts_SmartDiagnostic_Setup;
    TTabSheet     *ts_SmartDiagnostic_Summary;
    TPanel        *pn_SmartDiagnostic_Summary;
    TStringGrid   *sg_SmartDiagnostic_Summary;
    TPanel        *pn_SmartDiagnostic_Setup;
    TSpeedButton  *sb_SmarDiagnostic_CreateCyliderName;   // golden spelling verbatim (missing 'e' in "Smar")
    TPanel        *pn_SmartDiagnostic_Time;
    TLabel        *lb_SmartDiagnostic_SystemTime;
    TPanel        *pn_SmartDiagnostic_CyliderManagement;
    TPanel        *pn_SmartDiagnostic_CyliderManagementTop;
    TComboBox     *cob_SmartDiagnostic_CyliderName;
    TSpeedButton  *sb_SmartDiagnostic_ResetRecordCount;
    TStringGrid   *sg_SmartDiagnostic_CyliderManagement;
    TPanel        *pn_SmartDiagnostic_Parameter;
    TPanel        *pn_SmartDiagnostic_ParameterTop;
    TLabel        *lb_SmartDiagnostic_LimitCountValue;
    TEdit         *ed_SmartDiagnostic_LimitCountValue;
    TfSmartDiagnosticTimer *SmartDiagnosticTimer;         // golden TTimer* -- see stand-in above
    TLabel        *lb_SmartDiagnostic_LimitCheckTime;
    TEdit         *ed_SmartDiagnostic_LimitCheckTime;
    TStringGrid   *sgCylinder;             // PREDETERMINED NAME -- csystem.cpp GATE H5-G4/G5/G6
    TEdit         *edTemp;

    // ---- golden private: section (SmartDiagnostic.h:76-86) ----
    AnsiString aSmartDiagnosticFilePath;
    AnsiString aSmartDiagnosticResetFilePath;
    AnsiString aSmartDiagnosticParaPath;
    int        iLimitCountValue;
    int        iLimitCheckTime;
    TColor     **GridColor;            // allocated only by InitialStringGridColor (FormCreate), NULL until then
    TColor     **CylinderGridColor;    // allocated only by InitialCylinderSGColor (FormCreate), NULL until then
    AnsiString aCylinderPreAlm;

    // ---- golden public: data section (SmartDiagnostic.h:91-95, 121-127) ----
    vclcompat::TList *MyPageChangeList;
    int  iRecordCyliderOnCount[MaxCylinderItem];
    int  iRecordCyliderOffCount[MaxCylinderItem];
    bool bRecordCyStatus[MaxCylinderItem];
    bool bStartRecord;
    int  iPush_UpLimitTime[10];        // PREDETERMINED NAME
    int  iPush_LowLimitTime[10];       // PREDETERMINED NAME
    int  iPop_UpLimitTime[10];         // PREDETERMINED NAME
    int  iPop_LowLimitTime[10];        // PREDETERMINED NAME
    int  iPushAvgTime[10];             // PREDETERMINED NAME
    int  iPopAvgTime[10];              // PREDETERMINED NAME
    int  iWarPercent[10];              // PREDETERMINED NAME

    // ---- ctor / dtor ----
    TfSmartDiagnostic();                 // golden __fastcall TfSmartDiagnostic(TComponent* Owner)
                                          // -- no-arg per this tree's established facade-ctor
                                          // convention (forms/fCleaning.h TfCleaning() precedent);
                                          // see forms/fSmartDiagnostic.cpp for the ctor-safety
                                          // restructuring (does NOT call InitialVariable/
                                          // LoadSDSummaryData/LoadSDResetSummaryData/
                                          // SearchChangePageButton/LoadSmartDiagnosticParameter).
    virtual ~TfSmartDiagnostic() {}       // F0-e convention (forms/fCleaning.h:96 precedent)

    // ---- golden TForm-inherited methods, permanent offline no-ops (same
    //      idiom as forms/fContact.h:1601-1603 / forms/fMotorTest.h:976-977 /
    //      forms/fPassword.h:337-338) ----
    void Close()     {}                // golden TForm::Close -- FormButtonClick Tag==10 (golden :170)
    void ShowModal() {}                // golden TForm::ShowModal -- PREDETERMINED NAME, csystem.cpp:28280 (GATE H5-G6)

    // ---- Steven 20260925 (Data.SmartDiagnostic): port-only state for the web
    //      page (WebSmartDiag.cpp). None of these exists in golden. ----
    bool       bW906Created;       // W906_Create() has run (golden: the form exists from boot)
    int        iW906GridColorRows; // rows allocated in GridColor[] (golden writes past it -- see SetCellColor)
    int        iW906DlgAnswer;     // preset answer for W906_MessageDlg (mrYes/mrNo, VCL values 6/7)
    bool       bW906DlgReached;    // W906_MessageDlg was reached during the last handler run
    AnsiString asW906DlgPrompt;    // the golden question text it was reached with
    bool       bW906InputHas;      // W906_MyInputBox: the operator typed a value ...
    int        iW906InputValue;    // ... and this is it (otherwise golden's Result=atoi(Text) stays)

    // golden ctor body :19-24 + OnCreate FormCreate :268-272 (dfm OldCreateOrder=False
    // -> OnCreate fires after the ctor). Runs once; every later call is a no-op.
    void W906_Create();
    // golden MessageDlg(text, mtConfirmation, mbYes|mbNo, 0): records the question,
    // returns iW906DlgAnswer (the web asked the operator before calling).
    int  W906_MessageDlg(const AnsiString &text);
    // golden MyInputBox(TEdit*) (INPUT.cpp:84-102): Ptr->Text = fInput->Result (int).
    void W906_MyInputBox(TEdit *Ptr);

    // ---- translated methods (see forms/fSmartDiagnostic.cpp for bodies,
    //      GATE markers, and the full exclusion ledger above) ----
    void SetVCLToParameter(TIniFile *IniF, AnsiString Section, AnsiString Name, TObject *PCtrl, int &iValue);   // golden :39-54   Steven 20260925 un-gated
    void SetParameterToShow(TIniFile *IniF, AnsiString Section, AnsiString Name, TObject *PCtrl, int &iValue);
    void InitialVariable();
    void BackupChangePage(TPageControl *pc, TSpeedButton *sb);
    void __fastcall FormButtonClick(TObject *Sender);                 // golden :160-194 (SD-2 retired 20260925)
    void __fastcall SaveSmartDiagnosticParameter();                    // golden :470-479  Steven 20260925 un-gated (writes system\SmartDiagnosticPara.ini)
    void SaveSDSummaryData();                                          // golden :546-567  Steven 20260925 un-gated (writes system\SmartDiagnosticRecord.txt)
    void SaveSDResetSummaryData();                                     // golden :571-592  Steven 20260925 un-gated (writes system\SmartDiagnosticRecordReset.txt)
    void __fastcall sb_SmarDiagnostic_CreateCyliderNameClick(TObject *Sender);   // golden :594-623  BODY IN repo-root SmartDiagnostic.cpp (reads Cylinder[], ht9045_io)
    void __fastcall sb_SmartDiagnostic_ResetRecordCountClick(TObject *Sender);   // golden :692-724  Steven 20260925 un-gated (memory only; Save persists)
    void SaveCylinderData();                                           // golden :799-845  Steven 20260925 un-gated -- UNREACHABLE in golden (see banner)
    void __fastcall sb_SmartDiagnostic_SaveMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :847-852 -- dfm binds it to nothing
    void __fastcall InitialStringGridColor(TStringGrid *sg, int iRow, int iCol);  // GATE SD-3
    void __fastcall InitialCylinderSGColor(TStringGrid *sg, int iRow, int iCol);  // GATE SD-3
    void __fastcall SetCellColor(TStringGrid *sg, AnsiString aTitle, int iCol, int iRow, TColor Color);           // GATE SD-4
    void __fastcall SetCylinderCellColor(TStringGrid *sg, AnsiString aTitle, int iCol, int iRow, TColor Color);   // PREDETERMINED NAME, GATE SD-4
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);                        // GATE SD-5
    void __fastcall FormDestroy(TObject *Sender);
    void PasteStringGridAsTabFormat(int XSTART, TStringGrid *strGrd, TStringList *memoPtr);
    void __fastcall LoadSmartDiagnosticParameter();
    void LoadSDSummaryData();
    void LoadSDResetSummaryData();
    void GetCyliderOnCount(AnsiString CyName);     // golden spelling verbatim ("Cylider"). PREDETERMINED NAME, GATE SD-6
    void GetCyliderOffCount(AnsiString CyName);    // golden spelling verbatim ("Cylider"). PREDETERMINED NAME, GATE SD-6
    void __fastcall SmartDiagnosticTimerTimer(TObject *Sender);
    void __fastcall ed_SmartDiagnostic_LimitCountValueMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ed_SmartDiagnostic_LimitCheckTimeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall sgCylinderSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);  // golden :854-865 (SD-7 retired 20260925)
    void ReadCylinderData();               // PREDETERMINED NAME
};
//---------------------------------------------------------------------------
extern TfSmartDiagnostic *fSmartDiagnostic;
//---------------------------------------------------------------------------
#endif
