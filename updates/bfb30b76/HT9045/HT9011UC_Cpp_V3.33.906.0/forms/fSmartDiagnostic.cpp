// =============================================================================
//  forms/fSmartDiagnostic.cpp  --  definitions for the fSmartDiagnostic facade
//
//  AI(W906-FW3-SD1) 20260827: new file. Translated from golden
//  HT9011UC_Code_V3.33.906.0_20260618/SmartDiagnostic.cpp (966 span lines / 36
//  bodies = 35 TfSmartDiagnostic:: members + 1 file-scope SplitStrByDotSpaceOnly,
//  measured by tools/census/wave_preflight.py SmartDiagnostic.cpp, 20260827).
//
//  See forms/fSmartDiagnostic.h for: the full GATE REGISTER (SD-1..SD-7), the
//  EXCLUDED-ENTIRELY ledger (12 bodies), and the PREDETERMINED-NAME contract
//  (13 names already referenced by #if 0 call sites in mycylin.cpp / csystem.cpp
//  that this facade's names/types must match exactly).
//
//  golden includes -> this file's includes:
//    MachineDefine.h/myQwertyKeyBoard.h/SmartDiagnostic.h/cmydef.h/mycylin.h/
//    HTimer.h/INPUT.h
//    -> forms/fSmartDiagnostic.h (own header, replaces SmartDiagnostic.h +
//       pulls in mycylin.h/myTimer.h already), cmydef.h (N_INTEGER),
//       Config.h (IniConfig), CosFunction.h (CosFunction.bCylinderOnOffTimeLog),
//       forms/fQwertyKey.h (fQwertyKey->ShowQwertyKey, real since FW-QWKEY1).
//    golden's "HTimer.h" -> this tree's myTimer.h (already the established
//    substitute other golden mycylin.h consumers use for TQPF_Timer; pulled
//    in transitively by forms/fSmartDiagnostic.h -> mycylin.h -> myTimer.h).
//
//  Steven 20260925 (Data.SmartDiagnostic): write direction un-gated against golden
//  V912 SmartDiagnostic.cpp (HT9011UC_Code_V3.33.912.0_20260908_Jimmy, cp950; the
//  bodies are identical to the 906 text this file was first cut from). See the
//  header's 20260925 block for the list. Files this TU can now write, all under
//  D:\HT9045\system: SmartDiagnosticRecord.txt, SmartDiagnosticRecordReset.txt,
//  SmartDiagnosticPara.ini (and CylinderPreAlm.ini via SaveCylinderData, which
//  golden never calls).
// =============================================================================
#include "forms/fSmartDiagnostic.h"
#include "vclcompat/BtnPanelCore.h"   // vclcompat::clRed (TColor already `using`'d by the header)
#include "cmydef.h"                  // N_INTEGER (cmydef.cpp, ht9045_globals)
#include "Config.h"                  // IniConfig (Config.cpp, ht9045_globals)
#include "CosFunction.h"             // CosFunction.bCylinderOnOffTimeLog (CosFunction.cpp, ht9045_globals)
#include "forms/fQwertyKey.h"        // fQwertyKey->ShowQwertyKey (real, forms/fQwertyKey.cpp, ht9045_forms)
#include <cstdlib>                   // atoi
#include <cstring>                   // strcpy

using vclcompat::clRed;

// Steven 20260925: VCL constants the un-gated bodies use and vclcompat does not carry.
//   mrYes/mrNo = Controls.hpp values (6/7). clWindow = system-colour reference
//   COLOR_WINDOW (Graphics.hpp clWindow = clSystemColor|COLOR_WINDOW = 0x80000005),
//   stored unresolved exactly like vclcompat's own clBtnFace (BtnPanelCore.h);
//   WebSmartDiag.cpp resolves system colours with GetSysColor when it paints.
//   Same TU-local shape as forms/fMotorTest.cpp's clWindow (its D-3 note).
static const int    mrYes    = 6;
static const int    mrNo     = 7;
static const TColor clWindow = TColor(0x80000005);

//---------------------------------------------------------------------------
TfSmartDiagnostic *fSmartDiagnostic = new TfSmartDiagnostic();
//---------------------------------------------------------------------------

// =============================================================================
//  ctor -- golden SmartDiagnostic.cpp:14-25.
//
//  RESTRUCTURED, not a literal transcription -- static-init ctor safety rule
//  (this tree's established idiom, forms/fCleaning.cpp TfCleaning() precedent;
//  see also docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL 全域", which records
//  the 88/134 ctest SEGFAULT episode this rule exists to prevent). This global
//  (`fSmartDiagnostic` above) is constructed by a static initializer that runs
//  BEFORE main(); at that point every OTHER global in the program (including,
//  transiently, this very `fSmartDiagnostic` pointer itself, mid-assignment) is
//  in an unspecified order relative to this one, so the ctor body below may
//  ONLY touch this object's OWN fields and `new` its OWN widget stand-ins --
//  never dereference another global.
//
//  Consequently golden's ctor body is NOT reproduced verbatim here. What golden
//  does at :19-24 (InitialVariable(); LoadSDSummaryData(); LoadSDResetSummaryData();
//  SearchChangePageButton(fSmartDiagnostic); LoadSmartDiagnosticParameter();
//  SmartDiagnosticTimer->Enabled=bStartRecord;) is handled as follows:
//    * InitialVariable()/LoadSDSummaryData()/LoadSDResetSummaryData()/
//      LoadSmartDiagnosticParameter() are all translated below as real,
//      standalone, callable methods (see each for its own notes) -- just not
//      invoked automatically from here, matching the fCleaning.cpp precedent.
//      InitialVariable()'s FIELD-only side effects (the ones with no widget/
//      list dependency) are flattened directly into this ctor below instead,
//      so this object's state matches what golden's ctor chain would have
//      produced for those specific fields.
//    * SearchChangePageButton(fSmartDiagnostic) is doubly excluded: (a) GATE
//      SD-1 (no TWinControl port) makes it undefinable regardless, and (b) even
//      if it existed, passing the GLOBAL `fSmartDiagnostic` from inside this
//      object's own ctor is EXACTLY the SIOF hazard the ctor-safety rule bans
//      -- at this point in construction `fSmartDiagnostic` (the global above)
//      has not yet been assigned the result of `new TfSmartDiagnostic()`, so it
//      would read as the pre-initialization value, not `this`.
//    * `SmartDiagnosticTimer->Enabled=bStartRecord;` (golden :24) IS reproduced
//      below, literally -- it only touches this object's own two fields
//      (SmartDiagnosticTimer, bStartRecord), no other global. Its OBSERVABLE
//      VALUE differs from golden's runtime behaviour, though: golden's ctor
//      reaches this line AFTER LoadSDSummaryData() has possibly flipped
//      bStartRecord to true (file exists) or confirmed it false; here
//      LoadSDSummaryData() never runs during construction, so bStartRecord is
//      always the InitialVariable()-default `false` at this point, and this
//      line is therefore always a no-op (Enabled stays at its own false
//      default). Documented rather than silently different.
// =============================================================================
TfSmartDiagnostic::TfSmartDiagnostic()
{
    // ---- golden ctor's own TForm-inherited geometry writes (:17-18) --------
    Height = 660;
    Width  = 1300;
    Left   = 0;   // golden ZeroInitVclFields -- ctor itself never sets these two;
    Top    = 0;   // FormShow (:279-280) overwrites both before anything could observe them

    // ---- own-widget `new` (ctor-safety rule: own stand-ins only) -----------
    pn_SmartDiagnosticTitle                 = new TPanel();
    sb_SmartDiagnostic_Setup                = new TSpeedButton();
    sb_SmartDiagnostic_Summary              = new TSpeedButton();
    sb_SmartDiagnostic_Exit                 = new TSpeedButton();
    sb_SmartDiagnostic_Save                 = new TSpeedButton();
    pc_SmartDiagnostic                      = new TPageControl();
    ts_SmartDiagnostic_Setup                = new TTabSheet();
    ts_SmartDiagnostic_Summary              = new TTabSheet();
    pn_SmartDiagnostic_Summary              = new TPanel();
    sg_SmartDiagnostic_Summary              = new TStringGrid(6, 2);    // Steven 20260925: dfm ColCount=6 RowCount=2 (SmartDiagnostic.dfm:137/140); was the 5x5 VCL default
    pn_SmartDiagnostic_Setup                = new TPanel();
    sb_SmarDiagnostic_CreateCyliderName     = new TSpeedButton();
    pn_SmartDiagnostic_Time                 = new TPanel();
    lb_SmartDiagnostic_SystemTime           = new TLabel();
    pn_SmartDiagnostic_CyliderManagement    = new TPanel();
    pn_SmartDiagnostic_CyliderManagementTop = new TPanel();
    cob_SmartDiagnostic_CyliderName         = new TComboBox();
    sb_SmartDiagnostic_ResetRecordCount     = new TSpeedButton();
    sg_SmartDiagnostic_CyliderManagement    = new TStringGrid(2, 2);    // Steven 20260925: dfm ColCount=2 RowCount=2 (:318/:321)
    pn_SmartDiagnostic_Parameter            = new TPanel();
    pn_SmartDiagnostic_ParameterTop         = new TPanel();
    lb_SmartDiagnostic_LimitCountValue      = new TLabel();
    ed_SmartDiagnostic_LimitCountValue      = new TEdit();
    SmartDiagnosticTimer                    = new TfSmartDiagnosticTimer();
    lb_SmartDiagnostic_LimitCheckTime       = new TLabel();
    ed_SmartDiagnostic_LimitCheckTime       = new TEdit();
    sgCylinder                              = new TStringGrid(6, 2);    // Steven 20260925: dfm ColCount=6 RowCount=2 (:427/:430); FormShow resizes to 9x11
    edTemp                                  = new TEdit();
    MyPageChangeList                        = new vclcompat::TList();

    // ---- Steven 20260925: dfm values the un-gated bodies read (own widgets only) --
    //   Tag: FormButtonClick dispatches on P->Tag (golden :167/:172); dfm Setup Tag=1
    //   (:471), Exit Tag=10 (:772), Save Tag=9 (:923), Summary no Tag (=0).
    //   ItemIndex: VCL TComboBox starts at -1 (vclcompat's TComboBox starts at 0);
    //   golden ResetRecordCountClick :718 reads it.
    sb_SmartDiagnostic_Setup->Tag   = 1;
    sb_SmartDiagnostic_Exit->Tag    = 10;
    sb_SmartDiagnostic_Save->Tag    = 9;
    sb_SmartDiagnostic_Summary->Tag = 0;
    cob_SmartDiagnostic_CyliderName->ItemIndex = -1;
    edTemp->Visible = false;                                            // dfm :456
    bW906Created       = false;
    iW906GridColorRows = 0;
    iW906DlgAnswer     = mrNo;
    bW906DlgReached    = false;
    bW906InputHas      = false;
    iW906InputValue    = 0;

    // ---- golden InitialVariable() (:79-115) field-only effects, flattened --
    aSmartDiagnosticFilePath      = "d:\\HT9045\\system\\SmartDiagnosticRecord.txt";
    aSmartDiagnosticResetFilePath = "d:\\HT9045\\system\\SmartDiagnosticRecordReset.txt";
    aSmartDiagnosticParaPath      = "d:\\HT9045\\system\\SmartDiagnosticPara.ini";
    aCylinderPreAlm               = "d:\\HT9045\\system\\CylinderPreAlm.ini";     //JerryYang 20250120 : add
    iLimitCountValue = 0;
    bStartRecord     = false;
    for(int i=0; i<MaxCylinderItem; i++)
        bRecordCyStatus[i] = false;

    // ---- ZeroInitVclFields: golden never sets these anywhere reachable from
    //      the ctor chain (only conditionally, inside LoadSDSummaryData /
    //      sb_SmarDiagnostic_CreateCyliderNameClick / ReadCylinderData -- none
    //      of which run at construction time here). Zeroed for determinism,
    //      this tree's established DEFAULT-VALUE RULE (vclcompat/Controls.h). --
    iLimitCheckTime = 0;
    for(int i=0; i<MaxCylinderItem; i++)
    {
        iRecordCyliderOnCount[i]  = 0;
        iRecordCyliderOffCount[i] = 0;
    }
    for(int i=0; i<10; i++)
    {
        iPush_UpLimitTime[i]  = 0;
        iPush_LowLimitTime[i] = 0;
        iPop_UpLimitTime[i]   = 0;
        iPop_LowLimitTime[i]  = 0;
        iPushAvgTime[i]       = 0;
        iPopAvgTime[i]        = 0;
        iWarPercent[i]        = 0;
    }
    GridColor         = NULL;   // golden: allocated only by InitialStringGridColor (called from FormCreate)
    CylinderGridColor = NULL;   // golden: allocated only by InitialCylinderSGColor (called from FormCreate)

    // ---- golden ctor's literal last line (:24) -- own fields only, safe ----
    SmartDiagnosticTimer->Enabled = bStartRecord;   // always false here -- see banner note above
}

// =============================================================================
//  W906_Create -- Steven 20260925. PORT-ONLY entry, NOT in golden as a member.
//
//  What golden does ONCE, at boot (HT9045.cpp:271 Application->CreateForm):
//    ctor body :17-24   InitialVariable; LoadSDSummaryData; LoadSDResetSummaryData;
//                       SearchChangePageButton(fSmartDiagnostic);
//                       LoadSmartDiagnosticParameter; SmartDiagnosticTimer->Enabled=bStartRecord;
//    OnCreate           FormCreate :268-272 (dfm :14 OldCreateOrder=False -> VCL fires
//                       OnCreate from AfterConstruction, i.e. AFTER the ctor body, so
//                       InitialStringGridColor sees the RowCount LoadSDSummaryData set).
//  The static-init ctor above may not do file I/O or touch other globals, so the
//  same sequence runs here, lazily, the first time anyone needs the form:
//  WebSmartDiag.cpp (the page) and GetCyliderOnCount/OffCount (mycylin.cpp's live
//  counts -- see the SD-6 note there). Idempotent.
//
//  DEVIATIONS: (1) SearchChangePageButton stays excluded (GATE SD-1): it only binds
//  OnClick=FormButtonClick on the four title buttons; WebSmartDiag.cpp calls
//  FormButtonClick directly for Save. (2) The ctor pre-allocated MyPageChangeList
//  (ctor-safety); InitialVariable :85 allocates its own, so the empty pre-allocated
//  list is freed first instead of leaked.
// =============================================================================
void TfSmartDiagnostic::W906_Create()
{
    if(bW906Created)
        return;
    bW906Created=true;

    Height          = 660;                                          // golden :17
    Width           = 1300;                                         // golden :18
    delete MyPageChangeList;                                        // DEVIATION (2)
    MyPageChangeList=NULL;
    InitialVariable();                                              // golden :19
    LoadSDSummaryData();                                            // golden :20
    LoadSDResetSummaryData();                                       // golden :21
    // SearchChangePageButton(fSmartDiagnostic);                    // golden :22 -- GATE SD-1, DEVIATION (1)
    LoadSmartDiagnosticParameter();                                 // golden :23
    SmartDiagnosticTimer->Enabled   = bStartRecord;                 // golden :24

    FormCreate(NULL);                                               // dfm OnCreate (OldCreateOrder=False)
}

// ---------------------------------------------------------------------------
//  W906_MessageDlg -- Steven 20260925. Stands in for golden
//  `MessageDlg(text, mtConfirmation, TMsgDlgButtons()<<mbYes<<mbNo, 0)` at :174,
//  :597 and :698. There is no modal box in wb_serve and its dispatch cannot wait
//  for one (single tick thread), so the browser asks first (two-phase,
//  WebSmartDiag.cpp): phase 1 runs the handler with the answer preset to mrNo and
//  learns from bW906DlgReached that golden got as far as the question; phase 2
//  re-runs it with mrYes. In all three golden handlers nothing is changed before
//  the question, so phase 1 has no side effect.
// ---------------------------------------------------------------------------
int TfSmartDiagnostic::W906_MessageDlg(const AnsiString &text)
{
    bW906DlgReached=true;
    asW906DlgPrompt=text;
    return iW906DlgAnswer;
}

// ---------------------------------------------------------------------------
//  W906_MyInputBox -- Steven 20260925. golden INPUT.cpp:84-102 MyInputBox(TEdit*):
//  `fInput->Result=atoi(Ptr->Text.c_str()); ... fInput->ShowModal(); Ptr->Text=fInput->Result;`
//  -- an integer box. The value the operator typed comes from the browser
//  (bW906InputHas/iW906InputValue); without one, Result keeps atoi(Text), which is
//  what golden writes back when the box is closed unchanged.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::W906_MyInputBox(TEdit *Ptr)
{
    int Result=atoi(Ptr->Text.c_str());
    if(bW906InputHas)
        Result=iW906InputValue;
    Ptr->Text=Result;
}

// =============================================================================
//  SetVCLSizePosition -- golden :29-35. EXCLUDED (not declared/defined).
//  GATE SD-1: golden `TWinControl *PCtrl` param + ->Width/->Height, none of
//  which vclcompat ports (0 hits, `grep -n "Width\|Height" vclcompat/Controls.h`,
//  20260827). See forms/fSmartDiagnostic.h GATE REGISTER.
// =============================================================================

// ---------------------------------------------------------------------------
//  SetVCLToParameter -- golden :39-54. Steven 20260925: un-gated (was EXCLUDED:
//  IniF->WriteInteger is a disk write -- Steven authorised the Save path).
//  Only caller: SaveSmartDiagnosticParameter (system\SmartDiagnosticPara.ini).
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::SetVCLToParameter(TIniFile *IniF, AnsiString Section, AnsiString Name, TObject *PCtrl, int &iValue)
{
    TEdit        *PEdit     = dynamic_cast<TEdit *>(PCtrl);
    TLabeledEdit *LabEditPtr = dynamic_cast<TLabeledEdit *>(PCtrl);

    if(PEdit!=NULL)
    {
        iValue=atoi(PEdit->Text.c_str());
        IniF->WriteInteger(Section, Name, iValue);
    }
    else if(LabEditPtr!=NULL)
    {
        iValue=atoi(LabEditPtr->Text.c_str());
        IniF->WriteInteger(Section, Name, iValue);
    }
}

// ---------------------------------------------------------------------------
//  SetParameterToShow -- golden :58-73. READ only (IniF->ReadInteger). ACTIVE.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::SetParameterToShow(TIniFile *IniF, AnsiString Section, AnsiString Name, TObject *PCtrl, int &iValue)
{
    TEdit        *PEdit     = dynamic_cast<TEdit *>(PCtrl);
    TLabeledEdit *LabEditPtr = dynamic_cast<TLabeledEdit *>(PCtrl);

    if(PEdit!=NULL)
    {
        iValue      = IniF->ReadInteger(Section, Name, 0);
        PEdit->Text = iValue;
    }
    else if(LabEditPtr!=NULL)
    {
        iValue          = IniF->ReadInteger(Section, Name, 0);
        LabEditPtr->Text= iValue;
    }
}

// ---------------------------------------------------------------------------
//  InitialVariable -- golden :77-116. ACTIVE, but NOT called from the ctor
//  (see ctor banner above) -- a standalone callable method for a future wave
//  to wire in. No unsafe operation: own widgets + own list only, no other
//  global dereferenced, no file I/O.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::InitialVariable()
{
    aSmartDiagnosticFilePath        = "d:\\HT9045\\system\\SmartDiagnosticRecord.txt";
    aSmartDiagnosticResetFilePath   = "d:\\HT9045\\system\\SmartDiagnosticRecordReset.txt";
    aSmartDiagnosticParaPath        = "d:\\HT9045\\system\\SmartDiagnosticPara.ini";

    aCylinderPreAlm                 = "d:\\HT9045\\system\\CylinderPreAlm.ini";     //JerryYang 20250120 : add

    MyPageChangeList = new vclcompat::TList;
    MyPageChangeList->Clear();

    sg_SmartDiagnostic_Summary->ColWidths[0] = 50;
    sg_SmartDiagnostic_Summary->ColWidths[1] = 200;
    sg_SmartDiagnostic_Summary->ColWidths[2] = 100;
    sg_SmartDiagnostic_Summary->ColWidths[3] = 100;
    sg_SmartDiagnostic_Summary->ColWidths[4] = 100;
    sg_SmartDiagnostic_Summary->ColWidths[5] = 100;
    sg_SmartDiagnostic_Summary->Cells[0][0]  = "Item";
    sg_SmartDiagnostic_Summary->Cells[1][0]  = "Cylider_Name";
    sg_SmartDiagnostic_Summary->Cells[2][0]  = "ON_Count";
    sg_SmartDiagnostic_Summary->Cells[3][0]  = "OFF_Count";
    sg_SmartDiagnostic_Summary->Cells[4][0]  = "StartTime";
    sg_SmartDiagnostic_Summary->Cells[5][0]  = "EndTime";

    sg_SmartDiagnostic_CyliderManagement->ColWidths[0]  = 130;
    sg_SmartDiagnostic_CyliderManagement->ColWidths[1]  = 95;
    sg_SmartDiagnostic_CyliderManagement->Cells[0][0]   = "CyliderName";
    sg_SmartDiagnostic_CyliderManagement->Cells[1][0]   = "ResetCount";

    BackupChangePage(pc_SmartDiagnostic,sb_SmartDiagnostic_Summary);
    BackupChangePage(pc_SmartDiagnostic,sb_SmartDiagnostic_Setup);
    BackupChangePage(pc_SmartDiagnostic,sb_SmartDiagnostic_Save);
    BackupChangePage(pc_SmartDiagnostic,sb_SmartDiagnostic_Exit);

    for(int i=0; i<MaxCylinderItem; i++)
        bRecordCyStatus[i] = false;

    iLimitCountValue    = 0;
    bStartRecord        = false;
}

// ---------------------------------------------------------------------------
//  BackupChangePage -- golden :120-128. ACTIVE. Own list + own widgets only.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::BackupChangePage(TPageControl *pc, TSpeedButton *sb)
{
    MySDPageChange *P;
    P=new MySDPageChange;
    P->PCtrl    = pc;
    P->sbPtr    = sb;
    P->iTag     = sb->Tag;
    MyPageChangeList->Add((MySDPageChange*)P);
}

// =============================================================================
//  SearchChangePageButton -- golden :132-156. EXCLUDED (not declared/defined).
//  GATE SD-1: recurses `PCtrl->ControlCount`/`PCtrl->Controls[iP]` over a
//  `TWinControl *PCtrl` -- no TWinControl-shaped ancestor with ControlCount/
//  Controls[] exists anywhere reachable from this facade (same tree-wide
//  absence documented by forms/fHandlerSys.h/fIoSetView.h/fSpeed.h's own GATE
//  S4 family). See forms/fSmartDiagnostic.h GATE REGISTER.
// =============================================================================

// ---------------------------------------------------------------------------
//  FormButtonClick -- golden :160-194. ACTIVE.
//  Steven 20260925: GATE SD-2 RETIRED -- the Tag==9 (Save) branch runs golden
//  :174-179 again; MessageDlg -> W906_MessageDlg (two-phase from the browser).
//  Writes system\SmartDiagnosticRecord.txt, SmartDiagnosticRecordReset.txt and
//  SmartDiagnosticPara.ini (the last via SetVCLToParameter, then re-read).
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::FormButtonClick(TObject *Sender)
{
    int ret;
    TSpeedButton *P=(TSpeedButton *)Sender;
    MySDPageChange *MyP;

    if(P->Tag==10)                                                          //Exit
    {
        sb_SmartDiagnostic_Exit->Down=false;
        Close();
    }
    else if(P->Tag==9)                                                      //Save
    {
        ret=W906_MessageDlg("Sure To Save Report?");                        // golden :174 MessageDlg(..., mtConfirmation, mbYes|mbNo, 0)
        if(ret==mrNo)
            return;
        SaveSDSummaryData();
        SaveSDResetSummaryData();
        SaveSmartDiagnosticParameter();
    }
    else
    {
        for(int i=0; i<MyPageChangeList->Count; i++)
        {
            MyP=(MySDPageChange *)MyPageChangeList->Items[i];
            if(MyP->sbPtr==P)
            {
                MyP->PCtrl->ActivePageIndex=MyP->iTag;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  InitialStringGridColor -- golden :198-225. ACTIVE with GATE SD-3.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::InitialStringGridColor(TStringGrid *sg, int iRow, int iCol)
{
    int i, j;

    GridColor=new TColor*[iCol];

    for(i=0; i<iCol; i++)
    {
        GridColor[i]=new TColor[iRow];
    }
    iW906GridColorRows=iRow;                                        // Steven 20260925: port-only bookkeeping, see SetCellColor

    for(i=0; i<iCol; i++)
    {
        for(j=0; j<iRow; j++)
        {
            if(j==0)
            {
                GridColor[i][j]=TColor(0x00917B51);
            }
            else
            {
                GridColor[i][j]=TColor(0x00DFD9CC);
            }
            SetCellColor(sg, sg->Cells[i][j], i, j, GridColor[i][j]);
        }
    }
    //----------------------------------------------------------------
    //  GATE SD-3 -- golden :224 `sg->DefaultDrawing=false;`. No
    //  ->DefaultDrawing on vclcompat::TStringGrid (0 hits, 20260827).
    //----------------------------------------------------------------
    #if 0
    sg->DefaultDrawing=false;
    #endif
}
// ---------------------------------------------------------------------------
//  InitialCylinderSGColor -- golden :227-246. ACTIVE with GATE SD-3.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::InitialCylinderSGColor(TStringGrid *sg, int iRow, int iCol)
{
    int i, j;

    CylinderGridColor=new TColor*[iCol];

    for(i=0; i<iCol; i++)
    {
        CylinderGridColor[i]=new TColor[iRow];
    }

    for(i=0; i<iCol; i++)
    {
        for(j=0; j<iRow; j++)
        {
            CylinderGridColor[i][j]=TColor(0x00DFD9CC);
        }
    }
    //----------------------------------------------------------------
    //  GATE SD-3 -- golden :245 `sg->DefaultDrawing=false;`. Same gap as
    //  InitialStringGridColor above.
    //----------------------------------------------------------------
    #if 0
    sg->DefaultDrawing=false;
    #endif
}
// ---------------------------------------------------------------------------
//  SetCellColor -- golden :248-256. ACTIVE with GATE SD-4.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::SetCellColor(TStringGrid *sg, AnsiString aTitle, int iCol, int iRow, TColor Color)
{
    //----------------------------------------------------------------
    //  Steven 20260925: MEMORY-SAFETY DEVIATION. GridColor is sized once, by
    //  FormCreate, to the RowCount LoadSDSummaryData left (2 when no record file).
    //  golden then grows the grid (Create button :603 RowCount=MaxCylinderItem+1)
    //  and keeps calling this with rows past the allocation (Reset :721 with
    //  ItemIndex+1, Timer :775/:777 with any alarm row) -- a heap overwrite in
    //  BCB6 that happens to go unnoticed. Here the six column blocks grow to fit
    //  (new rows get golden's data-row colour 0x00DFD9CC, InitialStringGridColor
    //  :219); nothing else changes. iCol stays <6 at every golden call site.
    //----------------------------------------------------------------
    if(GridColor!=NULL && iRow>=iW906GridColorRows && iCol>=0 && iCol<6)
    {
        const int iNewRows=iRow+1;
        for(int c=0; c<6; c++)
        {
            TColor *p=new TColor[iNewRows];
            for(int r=0; r<iNewRows; r++)
                p[r]=(r<iW906GridColorRows) ? GridColor[c][r] : TColor(0x00DFD9CC);
            delete [] GridColor[c];
            GridColor[c]=p;
        }
        iW906GridColorRows=iNewRows;
    }
    if(GridColor==NULL)                                             // golden: AV (FormCreate always ran first there); W906_Create guarantees it here
        return;
    if(iCol>=0 && iRow>=0)
    {
        GridColor[iCol][iRow]=Color;
        //----------------------------------------------------------------
        //  GATE SD-4 -- golden :253 `sg->Refresh();`. No ->Refresh() on
        //  vclcompat::TStringGrid (0 hits, 20260827).
        //----------------------------------------------------------------
        #if 0
        sg->Refresh();
        #endif
        sg->Cells[iCol][iRow]=aTitle;
    }
}
// ---------------------------------------------------------------------------
//  SetCylinderCellColor -- golden :258-266. ACTIVE with GATE SD-4.
//  PREDETERMINED NAME -- csystem.cpp GATE H5-G5/H5-G6 call sites need this
//  exact name+signature to match on future un-gate.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::SetCylinderCellColor(TStringGrid *sg, AnsiString aTitle, int iCol, int iRow, TColor Color)
{
    if(CylinderGridColor==NULL || iCol>=9 || iRow>=11)              // Steven 20260925: FormCreate allocates [9][11] (:271); golden's callers stay inside it, this only refuses what would be a heap overwrite
        return;
    if(iCol>=0 && iRow>=0)
    {
        CylinderGridColor[iCol][iRow]=Color;
        //----------------------------------------------------------------
        //  GATE SD-4 -- golden :263 `sg->Refresh();`. Same gap as
        //  SetCellColor above.
        //----------------------------------------------------------------
        #if 0
        sg->Refresh();
        #endif
        sg->Cells[iCol][iRow]=aTitle;
    }
}
// ---------------------------------------------------------------------------
//  FormCreate -- golden :268-272. ACTIVE. Own widgets only.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::FormCreate(TObject *Sender)
{
    InitialStringGridColor(sg_SmartDiagnostic_Summary, sg_SmartDiagnostic_Summary->RowCount, 6);
    InitialCylinderSGColor(sgCylinder, 11, 9);                      //JerryYang 20250120 : add
}
// ---------------------------------------------------------------------------
//  FormShow -- golden :274-357. ACTIVE with GATE SD-5. IniConfig.
//  bC24InitialStartCheckCylinder read is a plain global-struct field read
//  (Config.cpp, ht9045_globals -- reachable), matches csystem.cpp:11753's own
//  read of the identical flag.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::FormShow(TObject *Sender)
{
    TDateTime TimeNow=Now();
    AnsiString S;

    Left=(1280-Width)/2;
    Top =(1024-Height)/2;

    //----------------------------------------------------------------
    //  GATE SD-5 -- golden :282-283 (text below is golden VERBATIM).
    //  SetVCLSizePosition is excluded outright (GATE SD-1).
    //----------------------------------------------------------------
    #if 0
    SetVCLSizePosition(pn_SmartDiagnosticTitle, 0, 0, 1300, 50);    //JerryYang 20250120 : add
    SetVCLSizePosition(pc_SmartDiagnostic, 25, 0, 1300, 600);
    #endif

    pc_SmartDiagnostic->ActivePageIndex=0;

    // golden `TimeNow.FormatString("YYYY")` etc. -- vclcompat has no
    // TDateTime::FormatString member; established substitution
    // FormatDateTime(fmt, dt) (forms/fProductionInfo.cpp:351-352 precedent).
    S.sprintf("%s年-%s月-%s日-%s時", FormatDateTime("YYYY", TimeNow), FormatDateTime("MM", TimeNow), FormatDateTime("DD", TimeNow), FormatDateTime("HH", TimeNow));
    pn_SmartDiagnostic_Time->Caption   =S;

    //----------------------------------------------------------------
    //  GATE SD-5 -- golden :290 `sgCylinder->Width=1300;`. No ->Width on
    //  vclcompat::TStringGrid (same class of gap as SD-1/SD-3/SD-4).
    //----------------------------------------------------------------
    #if 0
    sgCylinder->Width=1300;
    #endif
    sgCylinder->RowCount=11;
    sgCylinder->ColCount=9;

    for(int i=0; i<9; i++)
    {
        sgCylinder->ColWidths[i]=110;
    }

    sgCylinder->ColWidths[0]=120;

    sgCylinder->Cells[0][0]="Cylinder Name";
    sgCylinder->Cells[1][0]="Sensor QTY";
    sgCylinder->Cells[2][0]="On Time(sec)";
    sgCylinder->Cells[3][0]="Upper limit(On)";
    sgCylinder->Cells[4][0]="Lower limit(On)";
    sgCylinder->Cells[5][0]="Off Time(sec)";
    sgCylinder->Cells[6][0]="Upper limit(Off)";
    sgCylinder->Cells[7][0]="Lower limit(Off)";
    sgCylinder->Cells[8][0]="Waring %";

    sgCylinder->Cells[0][1]="C_LoaderEdgePush";
    sgCylinder->Cells[0][2]="C_TrayY_Fixer";
    sgCylinder->Cells[0][3]="C_Empty_Fix";
    sgCylinder->Cells[0][4]="C_Color_Fix";
    sgCylinder->Cells[0][5]="C_Auto1EdgePush";
    sgCylinder->Cells[0][6]="C_Auto1Side_Fixer";
    sgCylinder->Cells[0][7]="C_Auto2EdgePush";
    sgCylinder->Cells[0][8]="C_Auto2Side_Fixer";
    sgCylinder->Cells[0][9]="C_Auto3EdgePush";
    sgCylinder->Cells[0][10]="C_Auto3Side_Fixer";

    sgCylinder->Cells[1][1]="1";
    sgCylinder->Cells[1][2]="2";
    sgCylinder->Cells[1][3]="2";
    sgCylinder->Cells[1][4]="2";
    sgCylinder->Cells[1][5]="1";
    sgCylinder->Cells[1][6]="2";
    sgCylinder->Cells[1][7]="1";
    sgCylinder->Cells[1][8]="2";
    sgCylinder->Cells[1][9]="1";
    sgCylinder->Cells[1][10]="2";

    ReadCylinderData();

    sgCylinder->Cells[0][1]="LoaderEdgePush";
    sgCylinder->Cells[0][2]="LoaderTrayY_Fixer";
    sgCylinder->Cells[0][3]="Empty_Fix";
    sgCylinder->Cells[0][4]="Color_Fix";
    sgCylinder->Cells[0][5]="Auto1EdgePush";
    sgCylinder->Cells[0][6]="Auto1Side_Fixer";
    sgCylinder->Cells[0][7]="Auto2EdgePush";
    sgCylinder->Cells[0][8]="Auto2Side_Fixer";
    sgCylinder->Cells[0][9]="Auto3EdgePush";
    sgCylinder->Cells[0][10]="Auto3Side_Fixer";

    if(IniConfig.bC24InitialStartCheckCylinder)
    {
        pc_SmartDiagnostic->ActivePageIndex=1;
        pn_SmartDiagnostic_Parameter->Visible=false;
        pn_SmartDiagnostic_CyliderManagement->Visible=false;
    }
    else
    {
        pn_SmartDiagnostic_Parameter->Visible=true;
        pn_SmartDiagnostic_CyliderManagement->Visible=true;
    }
}
// ---------------------------------------------------------------------------
//  FormDestroy -- golden :359-362. ACTIVE. Own field only.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::FormDestroy(TObject *Sender)
{
    delete MyPageChangeList;
}

// =============================================================================
//  sg_SmartDiagnostic_SummaryDrawCell -- golden :366-384. EXCLUDED (not
//  declared/defined). Needs Canvas/FillRect/TextOut/DrawText/TRect/
//  TGridDrawState -- none ported (task's own known vclcompat-gap list;
//  re-confirmed 20260827, 0 hits each under vclcompat/). See forms/
//  fSmartDiagnostic.h ledger.
// =============================================================================

// =============================================================================
//  SplitStrByDotSpaceOnly -- golden :386-431 (file-scope, NOT a
//  TfSmartDiagnostic:: member). Ported here as `SD_SplitStrByDotSpaceOnly`
//  -- see the RENAME note below for why the name had to change.
//
//  ⚠ NAME COLLISION WITH AN ALREADY-LINKED GLOBAL, DIFFERENT BODY:
//  cpublic.h:25 already DECLARES (and cpublic.cpp:465 already DEFINES,
//  non-static, real, compiled into `ht9045_globals` which `ht9045_forms`
//  links) a global function called `SplitStrByDotSpaceOnly` with this EXACT
//  signature -- but a DIFFERENT delimiter set. cpublic.cpp's copy treats ' '
//  (space) as a field delimiter (cpublic.cpp:474 `str[ct1]!=' '`, hence the
//  name "DotSpaceOnly"); THIS golden file's own copy does NOT (verified
//  against the actual cp950 bytes: golden SmartDiagnostic.cpp:395/:408 check
//  only ',', '\t', '\r', ':' -- no ' '). Two BCB6 source files independently
//  drifted copies of a same-named utility, which is not uncommon in this
//  codebase; porting BOTH into one unified C++ static-link graph under the
//  identical name would collide.
//
//  RENAME, NOT JUST `static`: cmydef.h (included above) transitively includes
//  cpublic.h, so cpublic.h's `bool SplitStrByDotSpaceOnly(char*,char*,int);`
//  declaration (external linkage) is ALREADY VISIBLE in this translation
//  unit by the time this line is reached. A `static` definition of the exact
//  same name is therefore a hard compile error in C++ -- "was declared
//  'extern' and later 'static'" (caught by this wave's own -fsyntax-only
//  self-check, 20260827) -- linkage cannot be downgraded for a name that
//  already has a prior non-static declaration in scope. The name itself has
//  to differ, not just its linkage. Kept `static` (internal linkage) ANYWAY
//  once renamed, as a second, independent guard against ever re-colliding
//  with anything else tree-wide -- this is exactly the "static shadow" shape
//  this tree already names as one of the archive-extraction trap's five
//  shapes, just reached via a rename instead of linkage alone. Only this
//  file's own PasteStringGridAsTabFormat calls it, so neither the rename nor
//  the internal linkage changes anything observable from outside this TU.
// =============================================================================
static bool SD_SplitStrByDotSpaceOnly(char *str, char *dest, int Max)
{
    char Buffer[10240];
    int ct1=0, ct2=0;

    while(1)         // find first character
    {
        if(str[ct1]=='\x0')
            return false;
        if((str[ct1]!=',' && str[ct1]!='\t' && str[ct1]!='\r' && str[ct1]!=':'))
            break;
        ct1++;
    }
    while(1)
    {
        dest[ct2]=str[ct1];
        ct2++;
        ct1++;
        dest[ct2]='\x0';
        if((ct2+1)>=Max)
            break;

        if(str[ct1]!=',' && str[ct1]!='\t' && str[ct1]!='\0' && str[ct1]!='\r' && str[ct1]!=':')
        {
        }
        else
        {
            break;
        }
    }
    ct2=0;
    while(1)
    {
        Buffer[ct2]=str[ct1];
        if(str[ct1]=='\x0')
            break;
        ct1++;
        ct2++;

        if(ct2>=10240)
            break;
        Buffer[ct2]='\x0';
    }
    strcpy(str, Buffer);
    return true;
}

// ---------------------------------------------------------------------------
//  PasteStringGridAsTabFormat -- golden :435-466. ACTIVE. Pure in-memory
//  logic over the caller-supplied memoPtr/strGrd; no file I/O of its own.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::PasteStringGridAsTabFormat(int XSTART, TStringGrid *strGrd, TStringList *memoPtr)
{
    int x, y, StartX, StartY;
    char str[8192], str2[256];
    bool flag;

    for(y=0; y<strGrd->RowCount; y++)
        for(x=XSTART; x<strGrd->ColCount; x++)
            strGrd->Cells[x][y]="";

    for(y=0; y<memoPtr->Count; y++)
    {
        if(y>=strGrd->RowCount)
            break;
        strcpy(str, memoPtr->Strings[y].c_str());
        x=XSTART;
        do
        {
            flag=SD_SplitStrByDotSpaceOnly(str, str2, 256);
            if(flag==false)
                break;

            StartX=x;
            StartY=y;

            strGrd->Cells[StartX][StartY]=str2;
            x++;
            if(x>=strGrd->ColCount)
                break;
        }while(1);
    }
}

// ---------------------------------------------------------------------------
//  SaveSmartDiagnosticParameter -- golden :470-479. Steven 20260925: un-gated.
//  WRITES D:\HT9045\system\SmartDiagnosticPara.ini [Setup] LimitCount /
//  LimitCheckTime (atoi of the two edits), then re-reads it into the edits.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::SaveSmartDiagnosticParameter()
{
    TIniFile* IniFile=new TIniFile(aSmartDiagnosticParaPath);

    SetVCLToParameter(IniFile, "Setup", "LimitCount"                , ed_SmartDiagnostic_LimitCountValue                , iLimitCountValue);
    SetVCLToParameter(IniFile, "Setup", "LimitCheckTime"            , ed_SmartDiagnostic_LimitCheckTime                 , iLimitCheckTime);
    LoadSmartDiagnosticParameter();

    delete IniFile;
}

// ---------------------------------------------------------------------------
//  LoadSmartDiagnosticParameter -- golden :483-491. READ only
//  (TIniFile + SetParameterToShow, both pure-read). ACTIVE.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::LoadSmartDiagnosticParameter()
{
    TIniFile* IniFile=new TIniFile(aSmartDiagnosticParaPath);

    SetParameterToShow(IniFile, "Setup", "LimitCount"                , ed_SmartDiagnostic_LimitCountValue                , iLimitCountValue);
    SetParameterToShow(IniFile, "Setup", "LimitCheckTime"            , ed_SmartDiagnostic_LimitCheckTime                 , iLimitCheckTime);

    delete IniFile;
}

// ---------------------------------------------------------------------------
//  LoadSDSummaryData -- golden :495-522. READ only (FileExists +
//  TStringList::LoadFromFile). ACTIVE.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::LoadSDSummaryData()
{
    if(FileExists(aSmartDiagnosticFilePath.c_str())==false)
    {
        bStartRecord=false;
        return;
    }

    TStringList *slSummary;

    slSummary=new TStringList;

    slSummary->LoadFromFile(aSmartDiagnosticFilePath);
    sg_SmartDiagnostic_Summary->RowCount=slSummary->Count;
    PasteStringGridAsTabFormat(0, sg_SmartDiagnostic_Summary, slSummary);
    cob_SmartDiagnostic_CyliderName->Items->Clear();
    cob_SmartDiagnostic_CyliderName->ItemIndex=-1;                  // Steven 20260925: VCL Items->Clear = CB_RESETCONTENT -> no selection (stdctrls.pas:2261-2269); vclcompat's ItemIndex is a plain int
    for(int i=0; i<sg_SmartDiagnostic_Summary->RowCount; i++)
    {
        if(sg_SmartDiagnostic_Summary->Cells[1][i+1]!="")
        {
            iRecordCyliderOnCount[i]  = atoi(sg_SmartDiagnostic_Summary->Cells[2][i+1].c_str());
            iRecordCyliderOffCount[i] = atoi(sg_SmartDiagnostic_Summary->Cells[3][i+1].c_str());
            cob_SmartDiagnostic_CyliderName->Items->Add(sg_SmartDiagnostic_Summary->Cells[1][i+1]);
        }
    }
    bStartRecord=true;
    delete slSummary;
}

// ---------------------------------------------------------------------------
//  LoadSDResetSummaryData -- golden :526-542. READ only. ACTIVE.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::LoadSDResetSummaryData()
{
    if(FileExists(aSmartDiagnosticResetFilePath.c_str())==false)
    {
        return;
    }

    TStringList *slSummary;

    slSummary=new TStringList;

    slSummary->LoadFromFile(aSmartDiagnosticResetFilePath);
    sg_SmartDiagnostic_CyliderManagement->RowCount = slSummary->Count;
    PasteStringGridAsTabFormat(0, sg_SmartDiagnostic_CyliderManagement, slSummary);

    delete slSummary;
}

// ---------------------------------------------------------------------------
//  SaveSDSummaryData -- golden :546-567. Steven 20260925: un-gated.
//  WRITES D:\HT9045\system\SmartDiagnosticRecord.txt: every row x every column of
//  sg_SmartDiagnostic_Summary, each cell followed by "," (CRLF lines, vclcompat
//  TStringList::SaveToFile = BCB6).
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::SaveSDSummaryData()
{
    AnsiString S;
    int i, j;

    TStringList *P;
    P=new TStringList;
    P->Clear();

    for(i=0; i<sg_SmartDiagnostic_Summary->RowCount; i++)
    {
        S="";
        for(j=0; j<sg_SmartDiagnostic_Summary->ColCount; j++)
        {
            S+=sg_SmartDiagnostic_Summary->Cells[j][i];
            S+=",";
        }
        P->Add(S);
    }
    P->SaveToFile(aSmartDiagnosticFilePath.c_str());
    delete P;
}

// ---------------------------------------------------------------------------
//  SaveSDResetSummaryData -- golden :571-592. Steven 20260925: un-gated.
//  WRITES D:\HT9045\system\SmartDiagnosticRecordReset.txt (same shape, from
//  sg_SmartDiagnostic_CyliderManagement).
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::SaveSDResetSummaryData()
{
    AnsiString S;
    int i, j;

    TStringList *P;
    P=new TStringList;
    P->Clear();

    for(i=0; i<sg_SmartDiagnostic_CyliderManagement->RowCount; i++)
    {
        S="";
        for(j=0; j<sg_SmartDiagnostic_CyliderManagement->ColCount; j++)
        {
            S+=sg_SmartDiagnostic_CyliderManagement->Cells[j][i];
            S+=",";
        }
        P->Add(S);
    }
    P->SaveToFile(aSmartDiagnosticResetFilePath.c_str());
    delete P;
}

// =============================================================================
//  sb_SmarDiagnostic_CreateCyliderNameClick -- golden :594-623.
//  Steven 20260925: un-gated, but its BODY lives in the repo-root
//  SmartDiagnostic.cpp (library ht9045_sm), not here: it reads
//  `Cylinder[i].CylinderName`, whose definition (mycylin.cpp) is in ht9045_io, and
//  ht9045_forms links only vclcompat+ht9045_globals+ht9045_core. ht9045_sm links
//  ht9045_io PUBLIC, so the member binds there. Same facade/body split as
//  forms/fOffSet.h + cOffSet.cpp.
// =============================================================================

// ---------------------------------------------------------------------------
//  GetCyliderOnCount -- golden :627-657. ACTIVE with GATE SD-6.
//  PREDETERMINED NAME (golden spelling "Cylider" verbatim, NOT "Cylinder" --
//  do not "fix") -- mycylin.cpp:278/:320 need this exact name+signature.
//
//  ⚠ NOTED, NOT FIXED (golden bug, translated faithfully): the `for(int i=0;
//  i<MaxCylinderItem; i++)` loop below indexes `sg_SmartDiagnostic_Summary->
//  Cells[1][i+1]` up to i=320 regardless of the grid's actual ->RowCount
//  (which LoadSDSummaryData sets to whatever the record file's line count
//  happens to be, and may be far smaller than MaxCylinderItem+1 = 322).   [AI(W906-F9050-BD) 20261004: MaxCylinderItem 295 -> 321, was i=294 / 296]
//  [AI(W906-W3-6b) 20260925: corrected -- BCB6 grids.pas is {$R-}: outside RowCount a read gives ""; vclcompat now matches]
//  (history: vclcompat used to throw std::out_of_range here and this note wrongly said real VCL does too for the
//  identical out-of-bounds access, so this is a genuine pre-existing golden
//  risk (present in real BCB6 too), not something this translation
//  introduces. Left verbatim per "忠實優先於寫得更好".
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::GetCyliderOnCount(AnsiString CyName)
{
    if(CosFunction.bCylinderOnOffTimeLog==false)
    {
        return;
    }

    if(FileExists(aSmartDiagnosticFilePath.c_str())==false)
    {
        return;
    }

    W906_Create();                                                  // Steven 20260925: see the SD-6 note at the end of this body

    TDateTime TimeNow=Now();

    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(CyName==sg_SmartDiagnostic_Summary->Cells[1][i+1])
        {
            if(bRecordCyStatus[i]==true)
                break;

            iRecordCyliderOnCount[i]=atoi(sg_SmartDiagnostic_Summary->Cells[2][i+1].c_str());
            iRecordCyliderOnCount[i]++;
            sg_SmartDiagnostic_Summary->Cells[2][i+1]   = iRecordCyliderOnCount[i];
            sg_SmartDiagnostic_Summary->Cells[5][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);
            bRecordCyStatus[i]=true;
            break;
        }
    }
    //----------------------------------------------------------------
    //  Steven 20260925: GATE SD-6 RETIRED -- golden :656 `SaveSDSummaryData();`.
    //  Every counted actuation rewrites system\SmartDiagnosticRecord.txt, as in
    //  golden -- but only when the file already exists (the FileExists return
    //  above) and CosFunction.bCylinderOnOffTimeLog is on.
    //  ⚠ The W906_Create() call above is what makes this safe: golden's grid is
    //  loaded from this same file at boot (ctor :20), so saving writes back what
    //  was read plus the count. Without it the grid here would still be the empty
    //  static-init one and this line would overwrite the record file with blanks.
    //----------------------------------------------------------------
    SaveSDSummaryData();
}

// ---------------------------------------------------------------------------
//  GetCyliderOffCount -- golden :661-690. ACTIVE with GATE SD-6.
//  PREDETERMINED NAME (golden spelling "Cylider" verbatim) --
//  mycylin.cpp:295/:337 need this exact name+signature. Same loop-bound note
//  as GetCyliderOnCount above applies here too.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::GetCyliderOffCount(AnsiString CyName)
{
    if(CosFunction.bCylinderOnOffTimeLog==false)
    {
        return;
    }

    if(FileExists(aSmartDiagnosticFilePath.c_str())==false)
    {
        return;
    }
    W906_Create();                                                  // Steven 20260925: same reason as GetCyliderOnCount
    TDateTime TimeNow=Now();

    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(CyName==sg_SmartDiagnostic_Summary->Cells[1][i+1])
        {
            if(bRecordCyStatus[i]==false)
                break;

            iRecordCyliderOffCount[i]=atoi(sg_SmartDiagnostic_Summary->Cells[3][i+1].c_str());
            iRecordCyliderOffCount[i]++;
            sg_SmartDiagnostic_Summary->Cells[3][i+1]   = iRecordCyliderOffCount[i];
            sg_SmartDiagnostic_Summary->Cells[5][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);
            bRecordCyStatus[i] = false;
            break;
        }
    }
    //----------------------------------------------------------------
    //  Steven 20260925: GATE SD-6 RETIRED -- golden :689 `SaveSDSummaryData();`.
    //  Same conditions and the same W906_Create() reason as GetCyliderOnCount.
    //----------------------------------------------------------------
    SaveSDSummaryData();
}

// ---------------------------------------------------------------------------
//  sb_SmartDiagnostic_ResetRecordCountClick -- golden :692-724.
//  Steven 20260925: un-gated (was EXCLUDED). Changes the two grids in MEMORY
//  only -- golden writes nothing here; the operator's Save (FormButtonClick
//  Tag==9) or the next counted actuation (SaveSDSummaryData) persists it.
//  MessageDlg -> W906_MessageDlg; clWindow is the TU-local constant at the top.
//
//  ⚠ GOLDEN QUIRK, TRANSLATED AS IS (改行為要留給使用者決定): the loop compares
//  row i+1 with the i-th COMBO ITEM, not with the selected one. The combo is
//  filled in row order from the same rows (LoadSDSummaryData :511-518 /
//  Create :606-621), so whenever row 1 is named the loop stops at i=0 and resets
//  row 1 whatever the operator picked; the selection (:718) only decides which
//  row gets repainted clWindow.
//  ⚠ PAST THE COMBO'S Count, golden's `Items->Strings[i]` is "" -- NOT an
//  exception: the combo's Items are TCustomComboBoxStrings, whose Get is
//  `Len := SendMessage(CB_GETLBTEXT, Index, ...); if Len = CB_ERR then Len := 0;`
//  (BCB6 Source\vcl\stdctrls.pas:2251-2259). vclcompat's TStringList::GetString
//  also returns "" out of range, so the loop below behaves like golden without
//  help: if no named row matched first, it "resets" the first row whose name is
//  empty (e.g. an unnamed row left by Create).
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::sb_SmartDiagnostic_ResetRecordCountClick(
      TObject *Sender)
{
    TDateTime TimeNow=Now();
    int iCT=0, iIndex=0;

    int ret=W906_MessageDlg("Sure To Reset Report?");               // golden :698 MessageDlg(..., mtConfirmation, mbYes|mbNo, 0)
    if(ret==mrNo)
        return;

    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(sg_SmartDiagnostic_Summary->Cells[1][i+1]==cob_SmartDiagnostic_CyliderName->Items->Strings[i])
        {
            sg_SmartDiagnostic_Summary->Cells[2][i+1]   = "0";
            sg_SmartDiagnostic_Summary->Cells[3][i+1]   = "0";
            sg_SmartDiagnostic_Summary->Cells[4][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);
            sg_SmartDiagnostic_Summary->Cells[5][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);
            iRecordCyliderOnCount[i]                    = 0;
            iRecordCyliderOffCount[i]                   = 0;
            iCT     = atoi(sg_SmartDiagnostic_CyliderManagement->Cells[1][i+1].c_str());
            iCT++;
            sg_SmartDiagnostic_CyliderManagement->Cells[1][i+1]=iCT;
            break;
        }
    }
    iIndex=cob_SmartDiagnostic_CyliderName->ItemIndex+1;
    for(int i=0; i<6; i++)
    {
        SetCellColor(sg_SmartDiagnostic_Summary, sg_SmartDiagnostic_Summary->Cells[i][iIndex], i, iIndex, clWindow);
    }
    cob_SmartDiagnostic_CyliderName->ItemIndex=-1;
}

//------------------------------------------------------------------------------
//  golden SmartDiagnostic.cpp:727 -- file-scope (NOT a class member).
//------------------------------------------------------------------------------
TQPF_Timer hDelay;
//==============================================================================
//  SmartDiagnosticTimerTimer -- golden :729-783. ACTIVE. Own widgets + own
//  file-scope hDelay only, no writes.
//==============================================================================
void __fastcall TfSmartDiagnostic::SmartDiagnosticTimerTimer(TObject *Sender)
{
    static int Task=1, iAlarmOnIndex=0, iAlarmOffIndex=0;
    int i, iOnCount, iOffCount;
    static bool bAlarm=false;

    switch(Task)
    {
        case 1:
            bAlarm          = false;
            iAlarmOnIndex   = 0;
            iAlarmOffIndex  = 0;
            for(i=1; i<sg_SmartDiagnostic_Summary->RowCount; i++)
            {
                iOnCount    = atoi(sg_SmartDiagnostic_Summary->Cells[2][i].c_str());
                iOffCount   = atoi(sg_SmartDiagnostic_Summary->Cells[3][i].c_str());

                if(iOnCount>=iLimitCountValue)
                {
                    bAlarm=true;
                    iAlarmOnIndex=i;
                    break;
                }

                if(iOffCount>=iLimitCountValue)
                {
                    bAlarm=true;
                    iAlarmOffIndex=i;
                    break;
                }
            }

            if(iAlarmOffIndex==0 && iAlarmOnIndex==0)
            {
            }
            hDelay.SetMSAndOn(iLimitCheckTime*10);
            Task=10;
            break;
        case 10:
            if(hDelay.Off())
            {
                for(i=0; i<6; i++)
                {
                    if(iAlarmOnIndex>0)
                        SetCellColor(sg_SmartDiagnostic_Summary, sg_SmartDiagnostic_Summary->Cells[i][iAlarmOnIndex], i, iAlarmOnIndex,clRed);
                    if(iAlarmOffIndex>0)
                        SetCellColor(sg_SmartDiagnostic_Summary, sg_SmartDiagnostic_Summary->Cells[i][iAlarmOffIndex], i, iAlarmOffIndex,clRed);
                }
                Task=1;
            }
            break;
    }
}
// ---------------------------------------------------------------------------
//  ed_SmartDiagnostic_LimitCountValueMouseDown -- golden :785-790. ACTIVE.
//  fQwertyKey is real (forms/fQwertyKey.cpp, ht9045_forms).
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::ed_SmartDiagnostic_LimitCountValueMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 999999, 1);
}
// ---------------------------------------------------------------------------
//  ed_SmartDiagnostic_LimitCheckTimeMouseDown -- golden :792-797. ACTIVE.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::ed_SmartDiagnostic_LimitCheckTimeMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 999999, 1);
}

// ---------------------------------------------------------------------------
//  SaveCylinderData -- golden :799-845. Steven 20260925: un-gated (was EXCLUDED),
//  WRITES D:\HT9045\system\CylinderPreAlm.ini -- ⚠ but UNREACHABLE, as in golden:
//  its only caller is sb_SmartDiagnostic_SaveMouseDown below, and golden
//  SmartDiagnostic.dfm binds that handler to nothing (sb_SmartDiagnostic_Save,
//  dfm :922-1061, carries Tag=9 and no OnMouseDown; the button's click goes to
//  FormButtonClick via SearchChangePageButton). Translated so the golden text is
//  here; not called by WebSmartDiag.cpp. Consequence carried over from golden:
//  edits made through sgCylinderSelectCell are never persisted.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::SaveCylinderData()
{
    AnsiString S;
    int i, j;

    TStringList *P;
    P=new TStringList;
    P->Clear();

    sgCylinder->Cells[0][1]="C_LoaderEdgePush";
    sgCylinder->Cells[0][2]="C_TrayY_Fixer";
    sgCylinder->Cells[0][3]="C_Empty_Fix";
    sgCylinder->Cells[0][4]="C_Color_Fix";
    sgCylinder->Cells[0][5]="C_Auto1EdgePush";
    sgCylinder->Cells[0][6]="C_Auto1Side_Fixer";
    sgCylinder->Cells[0][7]="C_Auto2EdgePush";
    sgCylinder->Cells[0][8]="C_Auto2Side_Fixer";
    sgCylinder->Cells[0][9]="C_Auto3EdgePush";
    sgCylinder->Cells[0][10]="C_Auto3Side_Fixer";

    for(i=0; i<sgCylinder->RowCount; i++)
    {
        S="";
        for(j=0; j<sgCylinder->ColCount; j++)
        {
            if(j==0 ||j==3 || j==4 || j==6 || j==7 || j==8)
            {
                S+=sgCylinder->Cells[j][i];
                S+=",";
            }
        }
        P->Add(S);
    }
    P->SaveToFile(aCylinderPreAlm.c_str());
    delete P;

    sgCylinder->Cells[0][1]="LoaderEdgePush";
    sgCylinder->Cells[0][2]="LoaderTrayY_Fixer";
    sgCylinder->Cells[0][3]="Empty_Fix";
    sgCylinder->Cells[0][4]="Color_Fix";
    sgCylinder->Cells[0][5]="Auto1EdgePush";
    sgCylinder->Cells[0][6]="Auto1Side_Fixer";
    sgCylinder->Cells[0][7]="Auto2EdgePush";
    sgCylinder->Cells[0][8]="Auto2Side_Fixer";
    sgCylinder->Cells[0][9]="Auto3EdgePush";
    sgCylinder->Cells[0][10]="Auto3Side_Fixer";
}

// ---------------------------------------------------------------------------
//  sb_SmartDiagnostic_SaveMouseDown -- golden :847-852. Steven 20260925:
//  un-gated; bound to no event in golden's dfm (see SaveCylinderData).
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::sb_SmartDiagnostic_SaveMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    SaveCylinderData();
}

// ---------------------------------------------------------------------------
//  sgCylinderSelectCell -- golden :854-865. ACTIVE.
//  Steven 20260925: GATE SD-7 RETIRED -- golden :862 `MyInputBox(edTemp);` is
//  W906_MyInputBox (integer box; the typed value comes from the browser). The
//  change stays in sgCylinder's memory: golden never saves it (SaveCylinderData
//  is unreachable) and csystem's pre-alarm reads iPush_*/iPop_* from
//  ReadCylinderData, not from these cells.
// ---------------------------------------------------------------------------
void __fastcall TfSmartDiagnostic::sgCylinderSelectCell(TObject *Sender,
      int ACol, int ARow, bool &CanSelect)
{
    if(ACol==0 || ARow==0)
        return;
    if(ACol==3 || ACol==4 || ACol==6 || ACol==7 || ACol==8)
    {
        edTemp->Text=sgCylinder->Cells[ACol][ARow];
        W906_MyInputBox(edTemp);                                    // golden :862 MyInputBox(edTemp)
        sgCylinder->Cells[ACol][ARow]=edTemp->Text;
    }
}

// ---------------------------------------------------------------------------
//  ReadCylinderData -- golden :867-1034. ACTIVE. READ only (FileExists +
//  TStringList::LoadFromFile + in-memory CommaText parse -- CommaText is a
//  TStringList PROPERTY that reparses the list's OWN in-memory Strings[] from
//  a comma-delimited AnsiString; it never touches disk). No write anywhere in
//  this body. PREDETERMINED NAME -- csystem.cpp GATE H5-G4 needs this exact
//  name+signature (golden :24847, csystem.cpp:27584-27588).
//
//  ⚠ NOTED, NOT FIXED (golden quirk, translated faithfully): the final
//  `if(slTemp->Count<6)` block (golden :1020) checks slTemp's state as left
//  over by the LAST line the main loop happened to parse, not a "was any
//  matching cylinder found" flag -- reads like a bug (the intent looks like
//  it wants a not-found fallback) but is golden's own literal control flow;
//  left verbatim.
// ---------------------------------------------------------------------------
void TfSmartDiagnostic::ReadCylinderData()
{
    if(FileExists(aCylinderPreAlm.c_str())==false)
    {
        return;
    }
    AnsiString str="", sCylinderName;

    TStringList *slSummary;
    slSummary=new TStringList;

    TStringList *slTemp;
    slTemp=new TStringList;
    slTemp->Clear();

    slSummary->LoadFromFile(aCylinderPreAlm);

//    PasteStringGridAsTabFormat(0, sg_SmartDiagnostic_Summary, slSummary);
//    cob_SmartDiagnostic_CyliderName->Items->Clear();

    for(int y=0; y<slSummary->Count; y++)
    {
        if(y>=sgCylinder->RowCount)
            break;

        str=slSummary->Strings[y];

//        slTemp->Add(str);
        slTemp->CommaText=str;

        sCylinderName=slTemp->Strings[0];

        if(sCylinderName=="C_LoaderEdgePush" ||
           sCylinderName=="C_TrayY_Fixer" ||
           sCylinderName=="C_Empty_Fix" ||
           sCylinderName=="C_Color_Fix" ||
           sCylinderName=="C_Auto1EdgePush" ||
           sCylinderName=="C_Auto1Side_Fixer" ||
           sCylinderName=="C_Auto2EdgePush" ||
           sCylinderName=="C_Auto2Side_Fixer" ||
           sCylinderName=="C_Auto3EdgePush" ||
           sCylinderName=="C_Auto3Side_Fixer")
        {
            if(slTemp->Count>=6)
            {
                if(sCylinderName=="C_LoaderEdgePush")
                {
                    iPush_UpLimitTime[0]= StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[0]=StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[0]=  StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[0]= StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[0]=       StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_TrayY_Fixer")
                {
                    iPush_UpLimitTime[1]= StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[1]=StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[1]=  StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[1]= StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[1]=       StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Empty_Fix")
                {
                    iPush_UpLimitTime[2]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[2]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[2]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[2]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[2]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Color_Fix")
                {
                    iPush_UpLimitTime[3]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[3]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[3]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[3]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[3]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto1EdgePush")
                {
                    iPush_UpLimitTime[4]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[4]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[4]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[4]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[4]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto1Side_Fixer")
                {
                    iPush_UpLimitTime[5]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[5]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[5]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[5]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[5]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto2EdgePush")
                {
                    iPush_UpLimitTime[6]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[6]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[6]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[6]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[6]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto2Side_Fixer")
                {
                    iPush_UpLimitTime[7]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[7]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[7]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[7]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[7]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto3EdgePush")
                {
                    iPush_UpLimitTime[8]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[8]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[8]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[8]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[8]=         StrToIntDef(slTemp->Strings[5], 20);
                }
                else if(sCylinderName=="C_Auto3Side_Fixer")
                {
                    iPush_UpLimitTime[9]=   StrToIntDef(slTemp->Strings[1], 3000);
                    iPush_LowLimitTime[9]=  StrToIntDef(slTemp->Strings[2], 3000);
                    iPop_UpLimitTime[9]=    StrToIntDef(slTemp->Strings[3], 3000);
                    iPop_LowLimitTime[9]=   StrToIntDef(slTemp->Strings[4], 3000);
                    iWarPercent[9]=         StrToIntDef(slTemp->Strings[5], 20);
                }

                for(int i=1; i<sgCylinder->RowCount; i++)
                {
                    if(sgCylinder->Cells[0][i]=="C_LoaderEdgePush" ||
                       sgCylinder->Cells[0][i]=="C_TrayY_Fixer" ||
                       sgCylinder->Cells[0][i]=="C_Empty_Fix" ||
                       sgCylinder->Cells[0][i]=="C_Color_Fix" ||
                       sgCylinder->Cells[0][i]=="C_Auto1EdgePush" ||
                       sgCylinder->Cells[0][i]=="C_Auto1Side_Fixer" ||
                       sgCylinder->Cells[0][i]=="C_Auto2EdgePush" ||
                       sgCylinder->Cells[0][i]=="C_Auto2Side_Fixer" ||
                       sgCylinder->Cells[0][i]=="C_Auto3EdgePush" ||
                       sgCylinder->Cells[0][i]=="C_Auto3Side_Fixer")
                    {
                        if(slTemp->Strings[0]==sgCylinder->Cells[0][i])
                        {
                            sgCylinder->Cells[3][i]=StrToIntDef(slTemp->Strings[1], 3000);
                            sgCylinder->Cells[4][i]=StrToIntDef(slTemp->Strings[2], 3000);
                            sgCylinder->Cells[6][i]=StrToIntDef(slTemp->Strings[3], 3000);
                            sgCylinder->Cells[7][i]=StrToIntDef(slTemp->Strings[4], 3000);
                            sgCylinder->Cells[8][i]=StrToIntDef(slTemp->Strings[5], 20);
                        }
                    }
                }
            }
        }
    }

    if(slTemp->Count<6)
    {
        for(int i=1; i<sgCylinder->RowCount; i++)
        {
            sgCylinder->Cells[3][i]=3000;
            sgCylinder->Cells[4][i]=3000;
            sgCylinder->Cells[6][i]=3000;
            sgCylinder->Cells[7][i]=3000;
            sgCylinder->Cells[8][i]=50;
        }
    }

    delete slSummary;
    delete slTemp;
}

// =============================================================================
//  sgCylinderDrawCell -- golden :1036-1053. EXCLUDED (not declared/defined).
//  Same Canvas/FillRect/TextOut/DrawText/TRect/TGridDrawState gap as
//  sg_SmartDiagnostic_SummaryDrawCell above. See forms/fSmartDiagnostic.h
//  ledger.
// =============================================================================
