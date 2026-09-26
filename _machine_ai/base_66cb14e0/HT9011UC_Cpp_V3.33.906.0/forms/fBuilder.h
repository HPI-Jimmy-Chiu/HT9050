// =============================================================================
//  forms/fBuilder.h  --  non-VCL facade for golden's TfBuilder (cBuilder.h)
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (1 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cBuilder.h (67 lines)
//  + cBuilder.cpp (573 lines), read with `io.open(p, encoding='cp950')`
//  (0 U+FFFD), written out UTF-8 / bare-LF.
//
//  ROLE
//  ----
//  TfBuilder is golden's "setup-file Builder": it creates a new recipe folder
//  by COPYING an existing one (DataPath + OffsetPath, via SHFileOperation),
//  deletes recipe folders, and imports/exports recipe folders to and from a
//  browsed directory.  Nine of its 22 members exist to move files on disk.
//  It also owns the two "save the whole recipe out" entry points the rest of
//  the tree calls into (`bSaveAsJobFile`, `bSaveAllFillOrFile`).
//
//  `python tools/census/wave_preflight.py cBuilder.cpp` (20260828, re-run at
//  closeout): 22 bodies = 22 `TfBuilder::` members + 0 file-scope, 531 span
//  lines, 0 brace-swallow short-fall, 0 overload collapse.
//
//  THIS WAVE, against the 22-member denominator:
//     13 ACTIVE           123 of 531 golden span lines (23.2%)
//      9 GATED-WITH-BODY  408 golden span lines carried as `#if 0` transcript
//                         in forms/fBuilder.cpp
//  Every single one of the 9 gates is the READ-ONLY-DIRECTION safety rule --
//  not one of them is a link-boundary gate.  Quote both numbers or neither.
//
//  ============================================================================
//  *** THE NAMES `TfBuilder` / `fBuilder` ARE FREE -- THIS WAVE CLAIMS THEM ***
//  ============================================================================
//  Measured 20260828 with `python tools/census/wave_preflight.py cBuilder.cpp`
//  (section [3] SHIM OCCUPATION, 1203 port files scanned, build_*/vendor/
//  third_party excluded): (a) class name `clear`, (c) global name `clear`,
//  (d) TU-local seam / #define `clear`.  (b) reported 14 same-NAME members of
//  OTHER classes (FormShow/FormClose/spbExportClick/spbImportClick on
//  TFormBarcodeReader / TfVacuumUnit / TfSecurity / TfLd_ULd / TFTestIF) --
//  those are different classes' members, not an occupation of this one.
//  Cross-checked the same day with
//    `rg -c -g '!build_*' -g '*.h' -g '*.cpp' "\bTfBuilder\b" .`
//  -> the ONLY hits are tools/dfm2rc/layout_out/cBuilder_layout.gen.cpp:12 and
//  its _b1d_regen twin, where "TfBuilder" is a STRING LITERAL in a generated
//  .dfm widget table -- and `rg -n "layout_out" CMakeLists.txt` returns 0, i.e.
//  those files are in NO add_library and are never compiled.
//
//  Every non-comment `fBuilder->` in the tree is already inside `#if 0`; see
//  the PRE-COMMITTED list below.
//
//  ============================================================================
//  CONDITION-TWO CHECK (does a port of these bodies ALREADY exist somewhere,
//  possibly as a RENAMED FREE FUNCTION outside the class?)
//  ============================================================================
//  Searched 20260828, whole port tree, build_* excluded, `-g '*.cpp' -g '*.h'`.
//  ⚠ MEASUREMENT CONVENTION: every count below is AS OF THE PRE-WRITE SCAN and
//  EXCLUDES this wave's own forms/fBuilder.{h,cpp}.  Re-running the same
//  command after this wave returns higher numbers purely because these banners
//  mention the names in prose -- a comment is not a port.  Stated because a
//  wave silently falsifying its OWN absence claims is a recorded trap
//  (docs/KNOWLEDGE.md "波次agent的「不存在」宣稱會過期"), and it did happen
//  here: the closeout pre-flight re-run reports shapes (a) and (c) as 1 and 2
//  hits respectively, and all of those hits are this facade itself.
//    * `Create2DCodeWorkFile`     0 files
//    * `DeleteSetupFile`          0 files
//    * `InitCompData`             0 files
//    * `CopySourTarget`           0 files
//    * `NeedExport`               0 files
//    * `ShowDirBoxPath`           0 files
//    * `btCreateSetupFileClick`   0 files
//    * `bSaveAsJobFile`           7 files -- ALL of them CALL SITES or prose
//      (forms/fLd_ULd.cpp:342, forms/fTesterIF.cpp:1567, uTemp_Set.cpp:4656,
//      VacuumUnit/VacuumUnit.cpp:575, plus banner prose in forms/fHotPlate.h,
//      forms/fLd_ULd.h, forms/fTrayAssignment.h).  0 definitions.
//    * `bSaveAllFillOrFile`       1 file -- SECSGEM/uHGemHT9045.cpp:3710, a
//      call site inside that file's own `#if 0`.  0 definitions.
//  Renaming shapes probed (the TfTesterTCP -> TesterTCPSocket_* trap):
//    * `rg "\b(Builder_|fBuilder_|Bld)\w+"`                        0 hits
//    * `rg "SetupFileBuilder|RecipeBuilder|MakeSetupFile"`         0 hits
//  Conclusion: nothing to re-use, nothing re-translated twice.
//
//  ============================================================================
//  PRE-COMMITTED `#if 0` CALL SITES ELSEWHERE (3 names, 6 sites)
//  ============================================================================
//    fShow               Command.cpp:10248, Command.cpp:15036
//                        -- matches this facade's `bool fShow;`.
//    bSaveAsJobFile      uTemp_Set.cpp:4656, forms/fLd_ULd.cpp:342,
//                        forms/fTesterIF.cpp:1567   (all spelled
//                        `fBuilder->bSaveAsJobFile(S, "JOBFILE")`, i.e.
//                        (AnsiString, AnsiString) -- matches the declaration
//                        below), and VacuumUnit/VacuumUnit.cpp:575.
//    bSaveAllFillOrFile  SECSGEM/uHGemHT9045.cpp:3710, spelled
//                        `fBuilder->bSaveAllFillOrFile("Temperature", 0)`,
//                        i.e. (AnsiString, int) -- matches.
//  ⚠ THE PRE-FLIGHT'S "CONDITIONAL" ROW FOR VacuumUnit.cpp:575 IS A FALSE LIVE.
//  wave_preflight classifies that site as CONDITIONAL, not GATED, because
//  gate_depth_map does not evaluate macros (docs/KNOWLEDGE.md #22).  The line
//  sits inside `#ifdef ASE_KaohSiung`, and `ASE_KaohSiung` is NOT defined
//  anywhere in CMakeLists.txt (0 hits, 20260828), so the compiler never sees
//  it.  `bSaveAsJobFile` is declared below anyway -- it is a RESERVED NAME --
//  but NOT because the tree would fail to build without it.
//  All six sites stay `#if 0` / `#ifdef`-dead after this wave.
//
//  ============================================================================
//  GATE REGISTER (9) -- ALL NINE ARE THE READ-ONLY-DIRECTION SAFETY RULE
//  ============================================================================
//  Judgement applied to every body: "if this ran, would it change a file on
//  disk, send a packet, or move hardware?"  These nine would all change files.
//  NONE of them is gated for reachability: DataPath/OffsetPath (common.cpp,
//  ht9045_core), MyForceDirectories/FileExists/DirectoryExists (common.cpp /
//  vclcompat) and ShowMyMessage (canary_support.cpp, one of the four sanctioned
//  forms->sm exceptions) are ALL linkable from ht9045_forms today.  That is the
//  point: un-gating any of these would really delete or overwrite recipe
//  folders under D:\HT9045\, on a tree whose experimental and production data
//  are not yet separated (see the wave brief's `--dry` note).
//
//  (B-1) Create2DCodeWorkFile()   golden :51-70 (20L) -- WRITES DISK.
//        `CopyFile(str2.c_str(), str1.c_str(), false)` at :66, into
//        `D:\HT9045\Barcode_File\<port>\<recipe>.cfg`.  Guarded in its CALLER
//        by CC_KYEC_XILINX, but the body itself is unconditional.
//  (B-2) btCreateSetupFileClick(TObject*)  golden :72-159 (88L) -- WRITES DISK,
//        the biggest body in the file.  `MyForceDirectories(NewPath)` at :106
//        CREATES DIRECTORIES and `SHFileOperation(&oFile)` with
//        `wFunc=FO_COPY` at :122 recursively copies a whole recipe folder,
//        twice (DataPath and OffsetPath).  Secondary blockers recorded so a
//        future wave does not re-derive them: `oFile` (a file-scope
//        SHFILEOPSTRUCT in golden) has no port; `Handle` does not exist on a
//        facade with no base class; `Application->MessageBox` has no port; and
//        `fMain->cbSetupFileName` / `fMain->LookForFile()` -- fMain IS real and
//        in this same ht9045_forms target, but `rg "LookForFile" forms/fMain.h`
//        returns 0 hits (20260828).
//  (B-3) bSaveAsJobFile(AnsiString,AnsiString)  golden :163-192 (30L)
//        -- WRITES DISK.  MyForceDirectories + FO_COPY at :176-183.  RESERVED
//        NAME, so it is DECLARED here even though every one of its four call
//        sites is dead today (see the false-LIVE note above).  ⚠ Note that
//        golden's whole copy block sits inside `#ifdef ASE_KaohSiung`, so on a
//        non-ASE build this body only executes :190-191 (`CheckListBox1->
//        Items->Add(FileName); asBackupCreate=FileName;`).  It is STILL gated
//        whole, because splitting on a macro this port does not define would
//        be inventing a build variant, not translating one.
//  (B-4) btDeleteSetupFileClick(TObject*)  golden :202-230 (29L) -- DELETES
//        FILES, transitively: :220 calls DeleteSetupFile (GATE B-5).  Also
//        Application->MessageBox and fMain->cbSetupFileName/LookForFile as in
//        (B-2).
//  (B-5) DeleteSetupFile(AnsiString)  golden :232-260 (29L) -- DELETES FILES.
//        `oFile.wFunc=FO_DELETE` at :249 + SHFileOperation at :253, on
//        `DataPath\<name>` and `OffsetPath\<name>`.  This is the single most
//        destructive body in the wave.  Also needs `IsFileInUse`
//        (common.h:346, common.cpp, ht9045_core -- REACHABLE, i.e. not a
//        blocker) and `MySleep`.
//        ⚠ GOLDEN BUG PRESERVED VERBATIM, NOT "FIXED": :253 assigns
//        `bool bDelOK=SHFileOperation(&oFile);` and then reports an ERROR when
//        `bDelOK==true`.  SHFileOperation returns 0 on SUCCESS, so the flag's
//        name is inverted relative to its meaning -- the message fires on
//        failure, which happens to be the intended behaviour, via a variable
//        whose name says the opposite.  Transcribed byte-for-byte
//        (改行為要留給使用者決定).
//  (B-6) CopySourTarget(AnsiString)  golden :274-335 (62L) -- WRITES DISK.
//        FO_COPY into `DataPath` at :296 and :324.  Needs `CosFunction
//        .bBuilderImportSingleFolder` (CosFunction.cpp, ht9045_globals --
//        REACHABLE, not a blocker) and TSearchRec/FindFirst/FindNext.
//  (B-7) spbExportClick(TObject*)  golden :396-473 (78L) -- WRITES **AND
//        DELETES** OUTSIDE D:\HT9045.  :448 FO_DELETEs the destination folder
//        under the browsed `DirectoryListBox1->Directory` before :462 FO_COPYs
//        over it.  A wrong browse target here destroys unrelated data.
//  (B-8) spbImportClick(TObject*)  golden :475-482 (8L) -- WRITES DISK
//        transitively via CopySourTarget (GATE B-6) at :481; also
//        Application->MessageBox.
//  (B-9) bSaveAllFillOrFile(AnsiString,int)  golden :509-572 (64L) -- WRITES
//        DISK, the widest blast radius of the nine: it calls
//        `SaveSetupFile(...)` on ELEVEN other forms (fSpeed, fBinSel, fContact,
//        fSetup, fHotPlate, fTemp_Set, FTestIF, fYieldMonitoring, fTrayForm,
//        fTrayAssignment, fLd_ULd, plus fCCLink) -- i.e. it rewrites the entire
//        recipe on disk.  RESERVED NAME, so DECLARED here.  Independently it
//        also has real reachability blockers (fSpeed/fBinSel/fTrayForm/
//        fCCLink/fSetup/fYieldMonitoring have no facade with these methods, and
//        `SaveTestMode()` is cprod.h:3289 -> cprod.cpp, ht9045_globals, which
//        IS reachable) -- but SAFETY is the operative gate, and it would remain
//        the operative gate even if every collaborator existed.
//
//  ============================================================================
//  ACTIVE (13) -- reachability evidence
//  ============================================================================
//   TfBuilder()               golden :17-21.  `asBackupCreate="";` only -- own
//        field, no global deref, no call.  SIOF-safe (docs/KNOWLEDGE.md
//        "static-init ctor 不可碰 NULL 全域"; the fLaserSensor incident that
//        rule comes from turned 88 of 134 ctest binaries into SEGFAULTs).
//   FormShow(TObject*)        golden :23-33.  Calls the two ACTIVE private
//        helpers below, then writes four own fields.  NOTE it is ACTIVE only
//        BECAUSE InitCompData is: golden's FormShow is a pure display refresh.
//   cbSourceFileChange        golden :35-41.  TComboBox->Text / TEdit->Enabled.
//   edNewFileNameChange       golden :43-49.  TEdit->Text / TButton->Enabled.
//   cbDeleteFileChange        golden :194-200. Same shape.
//   DirectoryListBox1Change   golden :262-265. One call to ShowDirBoxPath.
//   ShowDirBoxPath()          golden :267-272. `str.sprintf("Path:%s",
//        DirectoryListBox1->Directory)` -- vclcompat's AnsiString::sprintf is
//        the variadic template that explicitly accepts a raw AnsiString for
//        `%s` (vclcompat/AnsiString.h:147-150), the same construct already live
//        at forms/fTesterIF.cpp:347.
//   NeedExport(AnsiString)    golden :337-348. Pure read of CheckListBox1.
//   InitCompData()            golden :350-394. READ-ONLY directory enumeration
//        (FindFirstFile/FindNextFile/FindClose from <windows.h>, the same
//        include forms/fMesSystem.cpp:24 already uses in this target) plus
//        `ShowMyMessage` (canary_support.h:80, body canary_support.cpp:143 =
//        ht9045_sm -- one of the four sanctioned forms->sm exceptions; that
//        definition is NOT inside any `#if 0`, checked 20260828).  It opens no
//        file, creates nothing and writes nothing.
//        ⚠ WHAT IT ACTUALLY TOUCHES, stated precisely because an earlier draft
//        of this banner got it wrong: `DataPath` is NOT empty offline.  It is
//        a HARD-CODED initialiser -- common.cpp:104
//        `AnsiString DataPath = "D:\\HT9045\\IniData\\Data\\";` (ht9045_core,
//        not inside any `#if 0`) -- and that directory EXISTS on this machine
//        (checked 20260828).  So this body really does enumerate the shared
//        recipe folder and fills the three list widgets with the real recipe
//        names.  It stays ACTIVE because enumeration is a READ: no handle is
//        opened for writing, no directory is created, nothing is deleted.
//        The "File Path Lost" branch fires only if that directory is missing.
//        ⚠ This is also why the gating judgement for the OTHER nine bodies is
//        about writes and not about "touching D:\HT9045\ at all" -- the two
//        are different questions, and only the first one gates.
//   spbExitClick(TObject*)    golden :484-487. `Close();` (DEVIATION D-3).
//   edNewFileNameMouseDown    golden :489-493. `fQwertyKey->ShowQwertyKey(
//        (TEdit*)Sender, N_NO_SYMBOL)` -- fQwertyKey is forms/fQwertyKey.cpp,
//        SAME ht9045_forms target, and its signature
//        (forms/fQwertyKey.h:370) defaults the last four parameters, so the
//        2-argument golden spelling binds.  N_NO_SYMBOL is
//        `extern const int` cmydef.h:291 -> cmydef.cpp, ht9045_globals.
//   edNewFileNameKeyPress     golden :495-500. `OnlyMakeFileDataInPut(Key)` --
//        common.h:402, defined common.cpp:1437, ht9045_core.
//   FormClose()               golden :502-505. `fShow=false;`.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) `TfBuilder` has NO base class (golden: `: public TForm`).  Matches
//      forms/fCounterSel.h (D-2) / forms/fCleaning.h / forms/fHotPlate.h.
//      CONSEQUENCE: golden's inherited `Left`/`Top`/`Handle`/`Close()` are not
//      inherited here.  Left/Top are re-declared as plain ints (D-5); Close()
//      is a no-op (D-3); `Handle` is NOT re-declared and appears only inside
//      `#if 0`.
//  (D-3) `Close()` is a PORT-ONLY no-op (`{}`) -- needed by ACTIVE
//      spbExitClick.  Same as forms/fCounterSel.h (D-3).
//  (D-4) `FormClose(TObject*, TCloseAction&)` -> `void FormClose();`.
//      `TCloseAction` has no port anywhere in this tree and golden's body reads
//      NEITHER parameter.  Precedent forms/fCounterSel.h (D-4) /
//      forms/fMesSystem.h D-5 / forms/fPrecaution.h (D-5).
//  (D-5) `int Left = 0; int Top = 0;` declared on the facade, value-only, no
//      layout.  Written by ACTIVE FormShow (75 / 10).  Exactly the idiom at
//      forms/fLd_ULd.h:348-349 and vclcompat/Controls.h:271-272 ("只存值").
//  (D-6) `edNewFileNameMouseDown(TObject*, TMouseButton, TShiftState, int, int)`
//      -> `edNewFileNameMouseDown(TObject *Sender)`.  TMouseButton and
//      TShiftState have no port in this tree; golden's body reads only
//      `Sender`.  Same reduction forms/fLd_ULd.h and forms/fQAMode.h apply.
//  (D-7) `edNewFileNameKeyPress`'s golden `Key=NULL;` on a `char&` is written
//      `Key=0;` -- identical value, and `NULL` on a char is a C++11-onward
//      warning magnet.  Same call, same file, as forms/fLd_ULd.cpp:71.
//  (D-8) TWO FACADE-LOCAL WIDGET STAND-INS are declared below,
//      `TfBuilderCheckListBox` (golden TCheckListBox) and
//      `TfBuilderDirectoryListBox` (golden TDirectoryListBox).  Neither type
//      exists in vclcompat/Controls.h, and forms/FormWidgets.h's own banner
//      says a wave should NOT edit that shared header.  Declaring the stand-in
//      in the consuming form's OWN header is the tree's established answer --
//      ATC/ATCInterface.h (TATCBitBtn/TATCButton/TATCCheckBox),
//      forms/fDynamicTemp.h (TfDynamicTempGroupBox), forms/fTemp_Set.h
//      (TfTemp_SetTagEdit/TfTemp_SetTagButton), and historically
//      forms/fObserver.h's TfObserverGrid + forms/fConfiguration.h's
//      TfConfigurationGrid (which were later collapsed INTO vclcompat -- the
//      same path is open to these two if a second consumer ever appears).
//      Each stand-in carries ONLY the members an ACTIVE body touches.
//      ⚠ NOT MODELLED, deliberately: `TDirectoryListBox` really browses the
//      filesystem in VCL and its `Directory` is driven by the user clicking.
//      Here it is a plain AnsiString that nothing writes -- see ZERO-WRITER.
//  (D-9) Golden's other 8 `__published` widgets (GroupBox5/6/7, Label78,
//      Label79, Label81, DriveComboBox1, spbImport, spbExport, spbExit) are NOT
//      declared: no ACTIVE body and no pre-committed site touches them
//      (minimal-declaration idiom -- forms/fSetup.h:20-25's explicit refusal to
//      grow a facade past measured need).  Their names all appear only inside
//      the `#if 0` transcripts, which never compile.
//
//  ============================================================================
//  FIELD LIST (8 widgets + 2 bools/strings + 2 ints)
//  ============================================================================
//    asBackupCreate (AnsiString)   golden cBuilder.h:61, ACTIVE ctor + FormShow
//    fShow          (bool)         golden cBuilder.h:62, PRE-COMMITTED (2 sites)
//    Left / Top     (int)          DEVIATION D-5, ACTIVE FormShow
//    cbSourceFile      (TComboBox*)                 golden cBuilder.h:21
//    edNewFileName     (TEdit*)                     golden cBuilder.h:22
//    btCreateSetupFile (TButton*)                   golden cBuilder.h:23
//    cbDeleteFile      (TComboBox*)                 golden cBuilder.h:27
//    btDeleteSetupFile (TButton*)                   golden cBuilder.h:26
//    labDir            (TLabel*)                    golden cBuilder.h:29
//    CheckListBox1     (TfBuilderCheckListBox*)     golden cBuilder.h:30
//    DirectoryListBox1 (TfBuilderDirectoryListBox*) golden cBuilder.h:31
//  NOT declared: see DEVIATION (D-9).
//
//  ============================================================================
//  ZERO-WRITER / ZERO-READER FIELDS (task rule: state the consequence)
//  ============================================================================
//  * `fShow` is written `true` by ACTIVE FormShow and `false` by ACTIVE
//    FormClose, so it is a REAL flag in this port -- unlike fCounterSel's,
//    which is permanently false.  Its two pre-committed readers stay `#if 0`
//    until Command.cpp's own gate is lifted (NOT this wave's job).
//  * `asBackupCreate` has NO reader anywhere in the port
//    (`rg -g '!build_*' "\basBackupCreate\b" .` outside this pair -> 0 hits,
//    20260828).  Golden's readers are elsewhere in its own tree; recorded so a
//    future wave does not read the silence as "dead field".
//  * `DirectoryListBox1->Directory` has NO WRITER at all here (D-8): in golden
//    the VCL control sets it from the user's browse action.  CONSEQUENCE:
//    ACTIVE ShowDirBoxPath always renders `labDir->Caption == "Path:"`.  That
//    is a faithful consequence of not porting a filesystem-browsing control,
//    not a translation error -- flagged per task rule.
//  * `CheckListBox1->Checked[]` likewise has no writer (golden's writer is the
//    user ticking a box), so ACTIVE NeedExport always returns false.  Its only
//    caller, spbExportClick, is GATE (B-7) anyway.
//
//  ============================================================================
//  DISCLOSURE (task rule 8 -- what was NOT read to completion / NOT verified)
//  ============================================================================
//  - `oFile` (golden's file-scope SHFILEOPSTRUCT, used by five gated bodies)
//    was NOT traced to its golden declaring TU.  Irrelevant: every body that
//    touches it is gated, and it is not declared here.
//  - `FileInfo().PathCombin(...)` (golden :61-65, :104-105, :124, :173-175,
//    :240) was NOT resolved to a port.  Appears ONLY inside gated bodies.
//  - `MySleep` / `IsFileInUse` were resolved to a declaration (common.h:346 for
//    IsFileInUse) but their bodies were NOT read.  Both appear only in gated
//    (B-5).
//  - golden `cBuilder.dfm` was NOT opened.  ACTIVE FormShow writes Left/Top/
//    btCreateSetupFile->Top itself, so no design-time geometry is needed by any
//    ACTIVE body -- but every widget default here is vclcompat's (Visible and
//    Enabled both FALSE, per vclcompat/Controls.h's DEFAULT-VALUE RULE), not
//    golden's .dfm value.  A wave that un-gates display logic must check that.
//  - `Application->MessageBox` (golden :95, :218, :407, :472, :480) was NOT
//    searched for a port; all five sites are inside gated bodies.
// =============================================================================
#ifndef FORMS_FBUILDER_H
#define FORMS_FBUILDER_H

#include "forms/FormWidgets.h"   // AnsiString / TObject / TComboBox / TEdit /
                                 // TButton / TLabel / TStringList (vclcompat)
#include <deque>                 // TfBuilderCheckListBox::Checked backing store

// ===========================================================================
//  DEVIATION D-8 -- facade-local stand-in for golden's TCheckListBox
//  (golden cBuilder.h:30 `TCheckListBox *CheckListBox1;`).
//
//  Members carried are exactly the ones ACTIVE bodies touch:
//    Items->Add / Items->Clear / Items->Strings[i] / Items->Count  (InitCompData,
//                                                                   NeedExport)
//    Checked[i]                                                    (NeedExport)
//    Clear()                                                       (InitCompData)
//
//  `std::deque<bool>` -- NOT `std::vector<bool>` -- backs Checked[] on purpose:
//  vector<bool> is the proxy specialisation and cannot return `bool&`, so
//  golden's assignable `Checked[i]` spelling would not survive.  deque<bool> is
//  not specialised and yields a real reference.  Growth-on-write mirrors
//  vclcompat/StringGrid.h's ColWidthsProxy verbatim.
// ===========================================================================
class TfBuilderCheckListBox : public vclcompat::TControl
{
public:
    TStringList *Items;

    class CheckedProxy {
    public:
        explicit CheckedProxy(TfBuilderCheckListBox *o) : owner_(o) {}
        bool &operator[](int idx)
        {
            if (idx >= static_cast<int>(owner_->checked_.size()))
                owner_->checked_.resize(idx + 1, false);
            return owner_->checked_[idx];
        }
    private:
        TfBuilderCheckListBox *owner_;
    };
    CheckedProxy Checked{this};

    TfBuilderCheckListBox() { Items = new TStringList(); }
    virtual ~TfBuilderCheckListBox() { delete Items; }

    void Clear()   // golden TCheckListBox::Clear -- wipes items AND tick state
    {
        if (Items) Items->Clear();
        checked_.clear();
    }

private:
    std::deque<bool> checked_;
};

// ===========================================================================
//  DEVIATION D-8 -- facade-local stand-in for golden's TDirectoryListBox
//  (golden cBuilder.h:31 `TDirectoryListBox *DirectoryListBox1;`).
//  ⚠ This does NOT browse anything.  `Directory` is a plain stored string with
//  no writer in this port; see the ZERO-WRITER note in the banner.
// ===========================================================================
class TfBuilderDirectoryListBox : public vclcompat::TControl
{
public:
    AnsiString Directory;      // golden TDirectoryListBox::Directory
    virtual ~TfBuilderDirectoryListBox() {}
};

// ===========================================================================
//  TfBuilder -- non-VCL facade (golden cBuilder.h:15-63,
//  `class TfBuilder : public TForm`; NO base class here, DEVIATION D-2).
// ===========================================================================
class TfBuilder
{
public:
    TfBuilder();

    // -- golden public: User declarations ------------------------------------
    AnsiString asBackupCreate;                          // golden cBuilder.h:61
    bool       fShow = false;                           // golden cBuilder.h:62, PRE-COMMITTED (Command.cpp:10248, Command.cpp:15036)

    // -- DEVIATION D-5: golden's inherited TControl geometry, value-only ------
    int Left = 0;                                       // ACTIVE FormShow writes 75 (golden :27)
    int Top  = 0;                                       // ACTIVE FormShow writes 10 (golden :28)

    // -- golden __published widgets touched by an ACTIVE body ----------------
    TComboBox                 *cbSourceFile      = new TComboBox();                 // golden cBuilder.h:21
    TEdit                     *edNewFileName     = new TEdit();                     // golden cBuilder.h:22
    TButton                   *btCreateSetupFile = new TButton();                   // golden cBuilder.h:23
    TButton                   *btDeleteSetupFile = new TButton();                   // golden cBuilder.h:26
    TComboBox                 *cbDeleteFile      = new TComboBox();                 // golden cBuilder.h:27
    TLabel                    *labDir            = new TLabel();                    // golden cBuilder.h:29
    TfBuilderCheckListBox     *CheckListBox1     = new TfBuilderCheckListBox();     // golden cBuilder.h:30, DEVIATION D-8
    TfBuilderDirectoryListBox *DirectoryListBox1 = new TfBuilderDirectoryListBox(); // golden cBuilder.h:31, DEVIATION D-8

    // -- ACTIVE this wave (defined in forms/fBuilder.cpp) --------------------
    void FormShow(TObject *Sender);                     // golden :23-33     ACTIVE
    void cbSourceFileChange(TObject *Sender);           // golden :35-41     ACTIVE
    void edNewFileNameChange(TObject *Sender);          // golden :43-49     ACTIVE
    void cbDeleteFileChange(TObject *Sender);           // golden :194-200   ACTIVE
    void DirectoryListBox1Change(TObject *Sender);      // golden :262-265   ACTIVE
    void ShowDirBoxPath();                              // golden :267-272   ACTIVE
    bool NeedExport(AnsiString sName);                  // golden :337-348   ACTIVE
    void InitCompData();                                // golden :350-394   ACTIVE
    void spbExitClick(TObject *Sender);                 // golden :484-487   ACTIVE
    void edNewFileNameMouseDown(TObject *Sender);       // golden :489-493   ACTIVE, DEVIATION D-6
    void edNewFileNameKeyPress(TObject *Sender, char &Key); // golden :495-500 ACTIVE, DEVIATION D-7
    void FormClose();                                   // golden :502-505   ACTIVE, DEVIATION D-4
    void Close() {}                                     // DEVIATION D-3, port-only no-op

    // -- GATED this wave -- declared here, golden body translated in
    //    forms/fBuilder.cpp inside `#if 0`.  Calling one is a LINK ERROR by
    //    design (see GATE REGISTER above).  ALL NINE ARE SAFETY GATES. -------
    void Create2DCodeWorkFile();                                            // golden :51-70    GATE (B-1)
    void btCreateSetupFileClick(TObject *Sender);                           // golden :72-159   GATE (B-2)
    void bSaveAsJobFile(AnsiString SourceFileFileName, AnsiString FileName); // golden :163-192 GATE (B-3)  RESERVED NAME
    void btDeleteSetupFileClick(TObject *Sender);                           // golden :202-230  GATE (B-4)
    void DeleteSetupFile(AnsiString DeleteFileName);                        // golden :232-260  GATE (B-5)
    void CopySourTarget(AnsiString Sour);                                   // golden :274-335  GATE (B-6)
    void spbExportClick(TObject *Sender);                                   // golden :396-473  GATE (B-7)
    void spbImportClick(TObject *Sender);                                   // golden :475-482  GATE (B-8)
    void bSaveAllFillOrFile(AnsiString SourceFileFileName, int iFile);      // golden :509-572  GATE (B-9)  RESERVED NAME
};

// AI(W906-FW3-BTQ1) 20260828: TfBuilder/fBuilder were FREE tree-wide (see the
// banner's measured provenance) -- declared here so the six pre-committed
// `#if 0` / `#ifdef`-dead sites (Command.cpp:10248, Command.cpp:15036,
// uTemp_Set.cpp:4656, forms/fLd_ULd.cpp:342, forms/fTesterIF.cpp:1567,
// VacuumUnit/VacuumUnit.cpp:575) have something real to bind to once THEIR own
// gates are separately lifted (NOT done by this wave).
extern TfBuilder *fBuilder;

#endif // FORMS_FBUILDER_H
