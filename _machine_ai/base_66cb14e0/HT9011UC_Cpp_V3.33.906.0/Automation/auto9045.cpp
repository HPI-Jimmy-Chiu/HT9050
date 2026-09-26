// =============================================================================
//  Automation/auto9045.cpp  --  OLP/GPIB host-command config accessor layer
//
//  Translation wave: W5-Final-Auto9045
//  Translator: AI(W5-Final-Auto9045) 20260711
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/Automation/auto9045.cpp
//                 (3018 lines, cp950) + auto9045.h (190 lines).
//
//  SCOPE: the WHOLE file, confirmed a plain free-function module (auto9045.h
//  has no TForm) -- all 148 golden functions translated 1:1, faithfully,
//  preserving control flow/off-by-one quirks verbatim (see per-function notes
//  below for the handful of pre-existing golden oddities, e.g. the golden
//  authors' own `//QQQ` markers on GetFixTrayDefine/GetLotTotal).
//
//  TU-LOCAL GATES (see the "GATED DEPENDENCIES" block just below the includes
//  for the full rationale of each): this file reaches a wide set of VCL forms
//  that have NO translated home yet (fBinSel/fShowBinSelect/fTrayAssignment/
//  fTestCategory/fContactCT/fCounterClear/fMonitor/fProductionInfo --
//  zero pre-existing shims anywhere in the tree), PLUS additional member
//  surface on forms that DO already have a partial shim but not these specific
//  members (fMain/fLotInfo in FormsFacade.h; fAutomation/fObserver/fContact in
//  atester_shims.h -- both FormsFacade.h and atester_shims.h are HARD-BOUNDARY
//  files this wave may not edit). Every one of these is given a narrowly-named
//  TU-local companion object/function (prefix `W5FA_`) with an offline-safe
//  conservative default, and is reported to the integrate agent as a
//  candidate for folding into the real shared shim once that is safe -- see
//  the translate report's "FormsFacade / atester_shims extension candidates"
//  section for the exact member list per class.
//  fTemp_Set is NO LONGER on this list -- AI(W906-FW-TEMP3) 20260820 retired
//  its `W5FA_FTemp_Set` stand-in in favour of the real forms/fTemp_Set.h
//  facade (commit c60e9f4), see the "GATED DEPENDENCIES" block's own note.
//
//  Big5: Chinese comments decoded via cp950 (python open(...,encoding='cp950')),
//  reproduced here as real UTF-8. ZERO U+FFFD (checked before hand-off).
// =============================================================================
#include "auto9045.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <utility>
#include <algorithm>

#include "MachineDefine.h"
#include "MachineType.h"            // eRunStartMode, CC_* customer codes, MAX_SOCKET_ROW/COL, MAX_SOCKET_TOTAL, MAX_Index_Row, Tempture_Hot, TestMode site enums
#include "cprod.h"                  // Prod, TestIF, TestIF_File, DeviceForm, DeviceForm_File, Offset, Offset_File, Temperature, RunInfo, ArmSpeed, BinSelect, HotPlateForm(_File), UserDefForm(_File), UN150Read
#include "cmydef.h"                 // CUSTOMER_CODE, bTestSiteUse, iTestRunMode, iTestBinCount, bUseTwoArm32Site, bOLPSetCategroy, MachineTypeChoice, USE_ROTATE_KIT, NEW_MAX_Index_Col, K_RETRY/K_SKIP/K_TRAY_FEED, MMSystem/MMInterface, bHomeByStart, PC_NAME, bFTPDownloadSetupFile, NULL_IC/HAS_NULL_IC/TEST_PASS
#include "Config.h"                 // IniConfig
#include "common.h"                 // DataPath/OffsetPath/AuthPath/LastDataPath, WriteIniData/ReadIniData/CheckAndReadIniData family (all REAL, already translated)
#include "canary_support.h"         // LastSet (minimal shim -- extended below via TU-local companion for the 2 missing fields), ShowErrorMessage, RecordProcess, ShowMyMessage
#include "csystem.h"                // HasICUnderMachine (REAL), InitOneCycle (REAL)
#include "aHotPlateSubstrate.h"     // TestSocket (TMyKitSuck: iShtRow/iShtCol/iShtCnt/Item[][])
#include "FormsFacade.h"            // fMain (REAL: ShowTestHeadComp), fLotInfo (REAL: cbRunMode/edtSysLotID/SetLotID/SetLotStart)
#include "forms/fTemp_Set.h"        // AI(W906-FW-TEMP3) 20260820: TfTemp_Set + extern fTemp_Set (REAL: edSoakTime/edWorkTemp) -- retires the TU-local W5FA_FTemp_Set stand-in below, see its own note

// =============================================================================
//  GATED DEPENDENCIES -- TU-local stand-ins for substrate/forms not yet in the
//  translated tree, or not yet on an existing shared shim's surface. Every
//  gate is local to THIS TU (companion objects/macros just below), matches the
//  project's established "#if 0 // TODO(...)" / local-macro convention (see
//  Automation/SCK_ART.cpp's own gate list for the precedent this follows), and
//  is re-cited at its call site further down.
// =============================================================================

// ---- Gate #1: common.h's GetLastOpenFN() / MyForceDirectories() ------------
//   Both declared in common.h but their real BODIES are gated `#if 0 //
//   TODO(wave-path)` / `TODO(wave-file)` in common.cpp (out of THIS unit's
//   scope -- common.cpp belongs to a different unit). TU-local duplicates,
//   same pattern already used by atester_shims.cpp's GetSiteCount (a real,
//   self-contained golden body copied locally because its real home is
//   gated) -- delete these duplicates once common.cpp's gates lift (would
//   otherwise ODR-conflict with the real definitions).
//
//   GetLastOpenFN: faithful copy of golden common.cpp:1252-1279 (reads the
//   first line of the LastData.inf-style file at LastDataPath via the already-
//   real vclcompat TStringList::LoadFromFile).
static AnsiString W5FA_GetLastOpenFN()
{
    AnsiString Str="Fail Open";
    TStringList MyList;
    if(FileExists(LastDataPath))
    {
        MyList.LoadFromFile(LastDataPath);
        if(MyList.Count==0)                                                    //Jimmychiu 20250426 : fixed for the content of setup is empty
        {
            ShowMyMessage("The content of setup.inf is empty!");
        }
        else if(AnsiString(MyList.Strings[0]).Trim()=="")
        {
            ShowMyMessage("The content of setup.inf is NULL!");
        }
        else
        {
            Str=MyList.Strings[0];
        }
    }
    else
    {
        ShowMyMessage("LastData does not exist");
    }
    return Str;
}
#define GetLastOpenFN W5FA_GetLastOpenFN

//   MyForceDirectories: faithful copy of golden common.cpp:1667-1717's CORE
//   action (if the directory doesn't exist, ForceDirectories it), reusing the
//   already-real vclcompat DirectoryExists/ForceDirectories + canary_support.h
//   RecordProcess. JUDGMENT CALL (flagged for review): the golden auto-detect-
//   trailing-filename branch (ExtractFilePath/ExtractFileExt + GetFileAttributes,
//   common.cpp:1676-1689) is OMITTED here -- verified dead for every call site
//   in THIS file (all pass a bare directory path, never a path with a trailing
//   filename component), so the omission is behaviorally inert for auto9045.cpp
//   but would diverge from golden if some future caller ever passed a filename.
//   The try/catch around ForceDirectories (golden :1704-1716) is also omitted
//   (vclcompat's ForceDirectories does not throw the same VCL Exception type).
static int W5FA_MyForceDirectories(AnsiString Directory, AnsiString Function="")
{
    if(Directory=="")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }
    if(DirectoryExists(Directory)==false)
        ForceDirectories(Directory);
    return 1;
}
#define MyForceDirectories W5FA_MyForceDirectories

// ---- Gate #2: MySleep(DWORD) -------------------------------------------------
//   golden common.h:261; common.cpp's own body is gated (`#if 0 // TODO(wave-
//   timing)`), but a REAL offline no-op definition ALREADY EXISTS in
//   acarry_shims.cpp (golden common.h:261, same signature) -- reuse it via
//   extern rather than duplicating a third copy.
extern void MySleep(DWORD dwMilliseconds);   // REAL definition: acarry_shims.cpp

// ---- Gate #3: ChangeSite() (golden cinitial.h:40 / cinitial.cpp:12074) ------
//   cinitial.cpp (W3 config/DB engine) is not translated this wave. Offline
//   no-op stand-in (golden body re-applies the just-written Site-Map config to
//   the live Prod/TestIF runtime state + refreshes UI; SetMapping already wrote
//   TestIF_File.iSiteMap[][] + persisted it via WriteIniData just above this
//   call, so the only thing genuinely lost offline is the live-state re-apply,
//   which has no consumer yet in this offline build).
static void W5FA_ChangeSite() {}
#define ChangeSite W5FA_ChangeSite

// ---- Gate #3b: DoStructUnitConvert() (golden cUnitConvert.h/.cpp) -----------
//   cUnitConvert.cpp's Do*Convert glue (including the DoStructUnitConvert
//   orchestrator) is a documented TODO(W6+W7) -- see cUnitConvert.h:105 / .cpp:145
//   -- not translated this wave. Offline no-op stand-in (golden body re-derives
//   ARM_OFFSET/speed-table structures from the just-changed Site-Map; the
//   underlying TestIF_File.iSiteMap[][] this file already wrote is unaffected).
static void W5FA_DoStructUnitConvert() {}
#define DoStructUnitConvert W5FA_DoStructUnitConvert

// ---- Gate #4: ArmData[]->ArmSKET[][]->Get*() (golden cSocket.h:41/91/180) ---
//   Per-arm/per-socket pass/bin/IF-error counters (TArm/TMySocket, cSocket.h/
//   .cpp) are not translated at all yet -- same gap already flagged in
//   Automation/SCK_ART.cpp's own gate #4 (GetPCA) for the identical family.
//   TU-local stand-ins all return 0 (conservative: "no test result recorded
//   yet", matching a freshly-idle offline handler with zero completed units).
static int W5FA_ArmData_GetTotal(int /*iArm*/, int /*iRow*/, int /*iCol*/)                 { return 0; }
static int W5FA_ArmData_GetSelBinCT(int /*iArm*/, int /*iRow*/, int /*iCol*/, int /*iBin*/){ return 0; }
static int W5FA_ArmData_GetIFError(int /*iArm*/, int /*iRow*/, int /*iCol*/)               { return 0; }
static int W5FA_ArmData_GetPassCT(int /*iArm*/, int /*iRow*/, int /*iCol*/)                { return 0; }

// ---- Gate #5: LastSet.iJamCount[3] / LastSet.BinCT[4][256] ------------------
//   golden LastSet.h:182 / :106. Not yet on canary_support.h's minimal
//   LAST_GENERAL_SET shim (a hard-boundary file this wave may not edit).
//   TU-local zero-initialized stand-ins (matches the shim's own all-fields-
//   default-0 posture).
static int          W5FA_LS_iJamCount[3]     = {0,0,0};      // golden LastSet.h:182
static unsigned int  W5FA_LS_BinCT[4][256]   = {{0}};        // golden LastSet.h:106

// ---- Gate #6: additional TfMain (golden main.h) surface not yet on the real
//   FormsFacade.h TfMain class. golden citations per member below; body
//   (main.cpp) is untranslated for all of these -- see report for exact
//   candidate signatures to fold into the real class.
namespace {

// AI(W906-W7-F2) 20260729: W5FA_TextField / W5FA_PanelCaption / W5FA_ComboBox RETIRED
// -- vclcompat/Controls.h now owns the unified stock-widget stand-ins (plan D4), so
// every member below is typed TEdit / TPanel / TComboBox directly.  Golden classes
// re-read for this change, one per member (they are cited at each declaration):
// every W5FA_TextField member is a golden TEdit*, and every W5FA_PanelCaption member
// is a golden TPanel* -- including the fObserver labels, whose old "golden TLabel*"
// comment was WRONG (golden cObserver.h:356-361/:374 declares labPowerOnTime/
// labRunningTime/labProductTime/labLoadingCount/labMUBA/labMTBA/labModel as TPanel*).
// All instances are heap-allocated through pointers -- no by-value copy, no aggregate
// initialisation -- so the unified types' vtable disturbs nothing.
//
// ONE NON-DEFAULT VALUE CARRIED OVER EXPLICITLY: the retired W5FA_ComboBox defaulted
// ItemIndex to **-1**, while vclcompat::TComboBox defaults it to 0.  That difference
// is preserved verbatim by an explicit `cbSetupFileName->ItemIndex = -1;` in
// W5FA_TfMainExt's constructor below (same technique the W7-F0 wave used for
// fLotInfo->palRemoveTray, see vclcompat/Controls.h's DEFAULT-VALUE RULE note), so
// this retirement is a zero-behaviour-change refactor and not a silent default flip.

struct W5FA_TfMainExt
{
    // -- golden main.h:1325 CanChangeSite(bNoIncludeHotplate=false) --
    bool CanChangeSite()
    {
        // JUDGMENT CALL (flagged for review): golden main.cpp body unavailable
        // this wave. Offline default TRUE (idle handler -> site map changeable).
        return true;
    }
    // -- golden main.h TfMain::CheckCanChangeRealDummy() --
    bool CheckCanChangeRealDummy()
    {
        // JUDGMENT CALL (flagged for review): golden main.cpp body unavailable.
        // Offline default TRUE ("safe to switch Real/Dummy now") -- matches a
        // freshly-idle Sim-HAL handler with nothing running, and lets
        // CheckNeedCleanOut()'s OR-condition fall through to the real, already-
        // translated HasICUnderMachine() check rather than unconditionally
        // denying every OLP/host Set* command offline (which FALSE would do).
        return true;
    }
    TComboBox *cbSetupFileName;                      // golden main.h:875 (TComboBox*)
    void ChangeSetUpFile(AnsiString /*sFileName*/) {}// golden main.h -- offline: recipe-switch UI refresh no-op
    TEdit     *edSoakTime;                           // golden main.h:733 (TEdit*)
    TEdit     *edWorkTemperBase;                     // golden main.h:732 (TEdit*)
    int ChangeTempMode(int /*iMode*/, bool /*bManual*/, bool /*bRefresh*/) { return 0; }   // golden main.h -- offline: 0=OK
    int ChangeTesterConnect(int /*iTester*/, bool /*bManual*/)             { return 0; }   // golden main.h -- offline: 0=OK
    TPanel    *palMainStatus;                        // golden main.h:669 (TPanel*)
    void ShowOLPState(int /*iState*/) {}              // golden main.h -- offline: OLP status UI no-op
    void DoFTRTClick(bool /*bRT*/, bool /*bManual*/) {} // golden main.h -- offline: FT/RT switch UI no-op

    W5FA_TfMainExt()
    {
        cbSetupFileName  = new TComboBox();
        // AI(W906-W7-F2) 20260729: PRESERVED VERBATIM from the retired W5FA_ComboBox,
        // whose own ctor was `: ItemIndex(-1)`.  vclcompat::TComboBox defaults
        // ItemIndex to 0, so without this line the retirement would silently flip a
        // live default from -1 ("nothing selected", the real VCL TComboBox value) to 0
        // ("first item selected").  See the block comment above this struct.
        cbSetupFileName->ItemIndex = -1;
        edSoakTime       = new TEdit();
        edWorkTemperBase = new TEdit();
        palMainStatus    = new TPanel();
    }
};
W5FA_TfMainExt W5FA_FMain;

// -- additional TfObserver (golden cObserver.h) surface -----------------------
struct W5FA_MemoLines0 { AnsiString Strings0; };   // golden TMemo*->Lines->Strings[0] (only index used here)
struct W5FA_TfObserverExt
{
    W5FA_MemoLines0   *Memo1Lines;                  // golden cObserver.h:380 (TMemo* Memo1)
    // AI(W906-W7-F2) 20260729: golden cObserver.h declares these as **TPanel*** --
    // :374 labModel, :356 labPowerOnTime, :357 labRunningTime, :358 labProductTime,
    // :359 labLoadingCount, :360 labMUBA, :361 labMTBA.  The old comment here said
    // "TLabel*", which was wrong; corrected while retiring W5FA_PanelCaption.
    TPanel *labModel, *labPowerOnTime, *labRunningTime, *labProductTime,
           *labLoadingCount, *labMUBA, *labMTBA;    // golden cObserver.h:374/:356-361 (TPanel*)
    W5FA_TfObserverExt()
    {
        Memo1Lines      = new W5FA_MemoLines0();
        labModel        = new TPanel();
        labPowerOnTime  = new TPanel();
        labRunningTime  = new TPanel();
        labProductTime  = new TPanel();
        labLoadingCount = new TPanel();
        labMUBA         = new TPanel();
        labMTBA         = new TPanel();
    }
};
W5FA_TfObserverExt W5FA_FObserver;

// -- additional TfAutomation (golden Automation/automation.h) surface --------
//   AI(W5-Final-Auto9045) 20260711: AmkorUDPGetSiteAndTemp/GetSite/GetTemp
//   (below) only POPULATE these fields -- they do NOT touch the UDP transport
//   directly (that happens in automation.cpp, out of scope this wave, per the
//   hand-off prompt). TestMode (int) is also read by GetTesterMode()/set by
//   SetTesterMode(). Report candidate: fold these 3 members onto the real
//   TfAutomationShim (atester_shims.h) once safe.
struct W5FA_TfAutomationExt
{
    int  TestMode;                      // golden automation.h -- host-selected tester mode echo
    char sATKSendMessage[256];          // golden automation.h -- ATK UDP site/temp packet buffer
    int  sATKSendMessageCount;          // golden automation.h -- current packet length
    W5FA_TfAutomationExt() : TestMode(0), sATKSendMessageCount(0) { for(int i=0;i<256;i++) sATKSendMessage[i]=0; }
};
W5FA_TfAutomationExt W5FA_FAutomation;

// -- additional TfLotInfo (golden uLotInfo.h) surface -------------------------
struct W5FA_TfLotInfoExt
{
    TEdit *edtSysOperatorID;                                 // golden uLotInfo.h:306 (TEdit*)
    void SetLotComponents(bool /*bClear*/) {}                // golden uLotInfo.h -- offline no-op
    void SetFirstTrayCheckOnUnloader() {}                    // golden uLotInfo.h -- offline no-op
    W5FA_TfLotInfoExt() { edtSysOperatorID = new TEdit(); }
};
W5FA_TfLotInfoExt W5FA_FLotInfo;

// -- additional TfContact (golden cContact.h) surface -------------------------
//   Read-only report fields (GetArmTotalForce/GetClinderForce/GetForcePerDevice/
//   GetSetNoPerPin/GetForcePerPin/GetContactForcen/GetContactForceInfo). Offline
//   default "" / 0 (no Contact-form widget content available offline).
struct W5FA_TfContactExt
{
    // golden cContact.h:72 edAirForceN, :228 edAirKPA, :208 edSetKg, :73
    // edForcePerDeviceKG, :74 edForcePerDeviceN, :199 edPinCount, :201
    // edForcePerPinG, :200 edForcePerPinN -- all TEdit*.
    TEdit *edAirForceN, *edAirKPA, *edSetKg, *edForcePerDeviceKG,
          *edForcePerDeviceN, *edPinCount, *edForcePerPinG, *edForcePerPinN;
    int rgOutKitDiameter_ItemIndex;
    W5FA_TfContactExt() : rgOutKitDiameter_ItemIndex(0)
    {
        edAirForceN       = new TEdit();
        edAirKPA          = new TEdit();
        edSetKg           = new TEdit();
        edForcePerDeviceKG= new TEdit();
        edForcePerDeviceN = new TEdit();
        edPinCount        = new TEdit();
        edForcePerPinG    = new TEdit();
        edForcePerPinN    = new TEdit();
    }
};
W5FA_TfContactExt W5FA_FContact;

// -- fBinSel (golden cBinSel.h, TfBinSel) -- zero pre-existing shim ----------
struct W5FA_TfBinSelExt
{
    void ChangeActivePageIndex() {}                 // golden cBinSel.h -- offline no-op (UI page refresh)
    void spbSaveClick(void* /*Sender*/) {}           // golden cBinSel.h -- offline no-op (UI save-button click)
};
W5FA_TfBinSelExt W5FA_FBinSel;

// -- fShowBinSelect (golden cShowBinSelect.h, TfShowBinSelect) ---------------
struct W5FA_TfShowBinSelectExt
{
    void ShowBinSel() {}                             // golden cShowBinSelect.h -- offline no-op (UI refresh)
};
W5FA_TfShowBinSelectExt W5FA_FShowBinSelect;

// -- fTrayAssignment (golden cTrayForm.h, TfTrayAssignment) ------------------
// AI(W906-W7-F2) 20260729: W5FA_CheckBox / W5FA_RadioGroup RETIRED in favour of
// vclcompat/Controls.h's TCheckBox / TRadioGroup (plan D4).  Defaults match exactly
// (Checked=false, ItemIndex=0), all instances are heap-allocated through pointers.
// The declaring golden header is **cTrayAssignment.h**, not cTrayForm.h as the
// surrounding comments say: :163 rgLoaderType and :179 rgFixTrayMode are TRadioGroup*,
// :168/:173/:178 ckUseFix1/2/3 are TCheckBox*.  Comment corrected at the members only;
// the older cTrayForm.h references around this block are left as found.
struct W5FA_TfTrayAssignmentExt
{
    bool fShow;                                      // golden cTrayForm.h -- offline: form not shown
    TRadioGroup *rgLoaderType, *rgFixTrayMode;       // golden cTrayAssignment.h:163 / :179 (TRadioGroup*)
    TCheckBox   *ckUseFix1, *ckUseFix2, *ckUseFix3;  // golden cTrayAssignment.h:168/:173/:178 (TCheckBox*)
    void ReadFile() {}                                // golden cTrayForm.h -- offline no-op (re-read Tray.Data)
    W5FA_TfTrayAssignmentExt() : fShow(false)
    {
        rgLoaderType  = new TRadioGroup();
        rgFixTrayMode = new TRadioGroup();
        ckUseFix1 = new TCheckBox();
        ckUseFix2 = new TCheckBox();
        ckUseFix3 = new TCheckBox();
    }
};
W5FA_TfTrayAssignmentExt W5FA_FTrayAssignment;

// -- fTestCategory (golden cTestCategory.h, TfTestCategory) ------------------
struct W5FA_TfTestCategoryExt
{
    void AdjFormData() {}                            // golden cTestCategory.h -- offline no-op (UI refresh)
};
W5FA_TfTestCategoryExt W5FA_FTestCategory;

// -- fContactCT (golden cContactCT.h, TfContactCT) ---------------------------
struct W5FA_TfContactCTExt
{
    void ShowFormComp() {}                           // golden cContactCT.h -- offline no-op (UI refresh)
};
W5FA_TfContactCTExt W5FA_FContactCT;

// -- fCounterClear (golden cCounterClear.h, TfCounterClear) ------------------
struct W5FA_TfCounterClearExt
{
    void ClearCount(int /*iType*/) {}                // golden cCounterClear.h -- offline no-op (counter reset UI)
};
W5FA_TfCounterClearExt W5FA_FCounterClear;

// -- fMonitor (golden MonitorInterface.h, TfMonitor) -------------------------
struct W5FA_TfMonitorExt
{
    bool MVCtrl_IsConnect() { return false; }        // golden MonitorInterface.h -- offline: no Monitor View connection
};
W5FA_TfMonitorExt W5FA_FMonitor;

// -- fProductionInfo (golden ProductionInfo.h, TfProductionInfo) -------------
struct W5FA_TfProductionInfoExt
{
    bool OEE_StartLot(bool /*bSilent*/, AnsiString* /*pErrMsg*/)
    {
        // golden ProductionInfo.h -- offline: no OEE/MO cloud backend wired ->
        // fail closed (matches SetLotInfoGreatekOEE's own fallback path, which
        // fills a default error message when the callee leaves asErrorMsg empty).
        return false;
    }
};
W5FA_TfProductionInfoExt W5FA_FProductionInfo;

// -- fTemp_Set (golden uTemp_Set.h, TfTemp_Set) ------------------------------
// AI(W906-FW-TEMP3) 20260820: RETIRED -- the TU-local `W5FA_TfTemp_SetExt
// W5FA_FTemp_Set` stand-in (edSoakTime/edWorkTemp only) that used to live here
// is deleted. Premise dead: forms/fTemp_Set.h + uTemp_Set.cpp landed commit
// c60e9f4 (`TfTemp_Set *fTemp_Set;` global, extern'd at forms/fTemp_Set.h:1544,
// defined uTemp_Set.cpp:220) -- the REAL facade carries both members with the
// exact golden shape (`TEdit *edSoakTime` forms/fTemp_Set.h:799, `TEdit
// *edWorkTemp` forms/fTemp_Set.h:762), grep-confirmed this wave before
// retiring the stand-in. The 2 call sites below (SetSoakTime/SetTemperature)
// now write through the real global with a NULL guard (this port never `new`s
// fTemp_Set -- no VCL CreateForm-equivalent runs offline, unlike golden where
// the form-creation list guarantees non-NULL). `auto9045.cpp` and
// `uTemp_Set.cpp` are both compiled into the SAME archive (ht9045_sm,
// CMakeLists.txt), so this introduces no new cross-archive link edge --
// test_auto9045 (tests/CMakeLists.txt:2086) already links ht9045_sm whole.
// Other files' banner comments (Command.cpp:10368-10369, MyTempPanel.cpp:
// 751-753, ProductionInfo/uPAT_Function.cpp:308-309, TempCtrl/TriTemp.cpp:
// 131-133) still describe the now-retired stand-in as "the only thing that
// exists" -- those files are outside this wave's write boundary (uHeaterThread.
// cpp/cprod.cpp/Automation/auto9045.cpp/tests/ only), so their text is now
// stale and a follow-up wave should refresh it; none of them reference the
// `W5FA_FTemp_Set` SYMBOL (grep-verified, see this wave's report), only cite it
// in prose, so nothing fails to compile or link.

} // namespace

//---------------------------------------------------------------------------
int SetCategory(AnsiString *Data) //OK
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;

    int PosIndex[TEST_MAX_BIN];

    //Error Category 0~8
    //0=Auto1
    //1=Auto2
    //2=Auto3
    //3=Fix1
    //4=Fix2
    //5=Fix3
    //6=Fix4
    //7=Fix5
    //8=Fix6
    //檢查 IfError SetCategory 資料
    PosIndex[0] = atoi(Data[0].c_str());
    if(PosIndex[0]<0 || PosIndex[0]>8)                                          //Sam 20240227 : 修正 SetCategory 問題
        return 3;

    if(MachineTypeChoice==Type_HT9046_LS && USE_ROTATE_KIT==1)
    {
        if(PosIndex[0]==3 || PosIndex[0]==6)  //不能使用 Fix1 & Fix4
            return 3;
    }

    if(PosIndex[0]>=6 && PosIndex[0]<=8)
    {
        if(TrayForm.LodareType==0 || TrayForm.iFixTrayMode==0)
            return 3;
        else if(PosIndex[0]==6 && TrayForm.bTrayUpDownSet[eFix1]==false)
            return 3;
        else if(PosIndex[0]==7 && TrayForm.bTrayUpDownSet[eFix2]==false)
            return 3;
        else if(PosIndex[0]==8 && TrayForm.bTrayUpDownSet[eFix3]==false)
            return 3;
    }

   //Bin Category 0~9
    //0=Not Use
    //1=Auto1
    //2=Auto2
    //3=Auto3
    //4=Fix1
    //5=Fix2
    //6=Fix3
    //7=Fix4
    //8=Fix5
    //9=Fix6
    //檢查 Bin SetCategory 資料
    for(int i=1; i<=iTestBinCount; i++) //取得資料 檢查資料範圍                 //Sam 20240227 : 修正 SetCategory 問題
    {
        PosIndex[i] = atoi(Data[i].c_str());

        if(PosIndex[i]<0 || PosIndex[i]>=9)
            return 3;

        if(MachineTypeChoice==Type_HT9046_LS && USE_ROTATE_KIT==1)
        {
            if(PosIndex[i]==4 || PosIndex[i]==7)  //不能使用 Fix1 & Fix4
                return 3;
        }

        if(PosIndex[i]>=7 && PosIndex[i]<=9)
        {
            if(TrayForm.LodareType==0 || TrayForm.iFixTrayMode==0) //QQQ (pre-existing golden marker, preserved verbatim -- golden authors' own second-look flag, not resolved here)
                return 3;
            else if(PosIndex[i]==7 && TrayForm.bTrayUpDownSet[eFix1]==false)
                return 3;
            else if(PosIndex[i]==8 && TrayForm.bTrayUpDownSet[eFix2]==false)
                return 3;
            else if(PosIndex[i]==9 && TrayForm.bTrayUpDownSet[eFix3]==false)
                return 3;
        }
    }

    BinSelect[iTestRunMode].IfErrorT3 = PosIndex[0];
    BinSelect[iTestRunMode].iStackDefFailCate[PosIndex[0]] = 1;

    for(int i=0; i<iTestBinCount; i++)
    {
        //Bin0.....Bin15
        BinSelect[iTestRunMode].iCatDataT3Pos[i]=PosIndex[i+1];
    }

    bOLPSetCategroy=true;
    W5FA_FBinSel.ChangeActivePageIndex();               // golden fBinSel->ChangeActivePageIndex() -- TU-local gate (no fBinSel shim yet)
    W5FA_FBinSel.spbSaveClick(0);                        // golden fBinSel->spbSaveClick(fBinSel) -- TU-local gate
    W5FA_FShowBinSelect.ShowBinSel();                    // golden fShowBinSelect->ShowBinSel() -- TU-local gate
    bOLPSetCategroy=false;
    return 0;
}
//---------------------------------------------------------------------------
int SetBinDefine(AnsiString *Data)      //Sam 20230803 : 新增OLP指令
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;

    int iBinPassFail[TEST_MAX_BIN];
    int iPassFail=0;
    int iStackDefFail[e3TrayCount];
    int iStack=0;

    for(int i=0; i<e3TrayCount; i++)
    {
        iStackDefFail[i]=-1;
    }

    for(int i=0; i<=iTestBinCount; i++) //取得資料 檢查資料範圍
    {
        iPassFail=atoi(Data[i].c_str());
        if(iPassFail==1)            //客戶定義 0:Fail ; 1:Pass 。 機台定義 1:Fail ; 0:Pass
            iBinPassFail[i]=0;
        else if(iPassFail==0)
            iBinPassFail[i]=1;
        else
            return 3;               //Undefine
    }

    //if(iBinPassFail[0]!=0)  //iBinPassFail[0] Error 一定是 Fail 不用特別去設定
    //{
    //    return 3;
    //}

    iStackDefFail[Prod.iIfErrorT6]=1;  //設定 Error Stack Fail

    for(int i=0; i<iTestBinCount; i++)
    {
        if(Prod.iT6PosCate[i]>0)
            iStack=Prod.iT6PosCate[i]-1;         //取得 Bin Stack
        else
            iStack=-1;

         if(iStack>=0)
         {
             if(iStackDefFail[iStack]==-1)                          //-1 Stack 沒有被設定過
             {
                iStackDefFail[iStack]=iBinPassFail[i+1];            //Stack 設定 PassFail
             }
             else
             {
                if(iStackDefFail[iStack]!=iBinPassFail[i+1])        //Stack 已經被設定過且 Bin Pass Fail 錯誤重疊
                {
                    return 4;
                }
             }
         }
    }

    for(int i=0; i<e3TrayCount; i++)
    {
        if(iStackDefFail[i]==-1)
        {
            Prod.iIsPassT6[i]=1;
            Prod.iIsFailT6[i]=0;                                    //沒用到的 Stack 塞 0
        }
        else
        {
            Prod.iIsFailT6[i]=iStackDefFail[i];                     //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
            Prod.iIsPassT6[i]=(Prod.iIsFailT6[i]==0)?1:0;
        }
    }

    bOLPSetCategroy=true;
    W5FA_FBinSel.ChangeActivePageIndex();
    W5FA_FBinSel.spbSaveClick(0);
    W5FA_FShowBinSelect.ShowBinSel();
    bOLPSetCategroy=false;
    return 0;
}
//---------------------------------------------------------------------------
int SetFixTrayDefine(AnsiString *Data)  //Sam 20230921 : 新增 FixTray 指令
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    int iLoaderType=atoi(Data[0].c_str());      //0:Same ; 1:Different
    int iFixTrayMode=atoi(Data[1].c_str());     //0:Full Bin ; 1:Up Down
    int iFixUpDwon1=atoi(Data[2].c_str());      //0:Not use ; 1:Up Down
    int iFixUpDwon2=atoi(Data[3].c_str());
    int iFixUpDwon3=atoi(Data[4].c_str());

    if(iLoaderType==0)
    {
        if(iFixTrayMode>0 || iFixUpDwon1>0 || iFixUpDwon2>0 || iFixUpDwon3>0)
            return 3;
    }

    if(iFixTrayMode==0)
    {
        if(iFixUpDwon1>0 || iFixUpDwon2>0 || iFixUpDwon3>0)
            return 3;
    }

    if(MachineTypeChoice==Type_HT9046_LS && USE_ROTATE_KIT==1)
    {
        if(iFixUpDwon1>0)
            return 3;  //不能使用 Fix1
    }

    AnsiString S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);
    szDir+="\\Tray.Data";
    if(W5FA_FTrayAssignment.fShow)                                             // golden fTrayAssignment->fShow -- TU-local gate (offline: form not shown)
    {
        W5FA_FTrayAssignment.rgLoaderType->ItemIndex=iLoaderType;
        if(iLoaderType==1)
        {
            W5FA_FTrayAssignment.rgFixTrayMode->ItemIndex=iFixTrayMode;
            if(iFixTrayMode==1)
            {
                W5FA_FTrayAssignment.ckUseFix1->Checked=(iFixUpDwon1==1)?true:false;
                W5FA_FTrayAssignment.ckUseFix2->Checked=(iFixUpDwon2==1)?true:false;
                W5FA_FTrayAssignment.ckUseFix3->Checked=(iFixUpDwon3==1)?true:false;
            }
        }
    }
    WriteIniData(szDir, "Flag", "Loader Type",iLoaderType);
    WriteIniData(szDir, "Flag", "Fix Tray Mode",iFixTrayMode);
    WriteIniData(szDir, "Flag", "Use Fix1 Tray",(iFixUpDwon1==1)?true:false);
    WriteIniData(szDir, "Flag", "Use Fix2 Tray",(iFixUpDwon2==1)?true:false);
    WriteIniData(szDir, "Flag", "Use Fix3 Tray",(iFixUpDwon3==1)?true:false);
    W5FA_FTrayAssignment.ReadFile();                                           // golden fTrayAssignment->ReadFile() -- TU-local gate
    return 0;
}
//---------------------------------------------------------------------------
int SetDutOnOff(AnsiString *Data) //OK
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;

    //根據模式選擇
    //TestIF.iTestMode
    int MaxI=0;
    int MaxJ=0;
    switch(TestIF.iTestMode)
    {
      case DualSite://1x2
           MaxI=1;
           MaxJ=2;
           break;
      case SingleSite://1x1
           MaxI=1;
           MaxJ=1;
           break;
      case QualSite1X4://1x4
      case _8Site1X4:  //ChungHung 20150528 add for 海思 _8Site1x4
           MaxI=1;
           MaxJ=4;
           break;
      case QualSite2X2://2x2
           MaxI=2;
           MaxJ=2;
           break;
      case _16Site4X4:  //Sam 20190226 : 16Site4X4
      case _8Site2X4://2x4
           MaxI=2;
           MaxJ=4;
           break;
      case _16Site2X8://2x8
           MaxI=2;
           MaxJ=8;
           break;
    };

    if(W5FA_FMain.CanChangeSite()==false)                                      // golden fMain->CanChangeSite() -- TU-local gate (offline default true)
    {
        return 3;
    }

    //AnsiString KeyWord[16]={ "Dut1=" ,"Dut2=" ,"Dut3=" ,"Dut4=" ,"Dut5=" ,"Dut6=" ,"Dut7=" ,"Dut8=" ,
    //                         "Dut9=" ,"Dut10=" ,"Dut11=" ,"Dut12=" ,"Dut13=" ,"Dut14=" ,"Dut15=" ,"Dut16=" };
    //AnsiString Result[2]={ "Close","Open"};

    int index;
    int value;
    bool bDutOnOff[MAX_SOCKET_TOTAL]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
                                      false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
            index = i*MaxJ+j;
            if(index<8)
            {
                if(TestIF.iSiteMap[index/4][index%4]==0)
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bTestSiteUse[0][index/4][index%4]=false;
                    LastSet.bUseTestSocket[0][index/4][index%4]=false;

                    bTestSiteUse[1][index/4][index%4]=false;
                    LastSet.bUseTestSocket[1][index/4][index%4]=false;
                }
                else
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bTestSiteUse[0][index/4][index%4]=!bTestSiteUse[0][index/4][index%4];
                    LastSet.bUseTestSocket[0][index/4][index%4]=!LastSet.bUseTestSocket[0][index/4][index%4];

                    bTestSiteUse[1][index/4][index%4]=!bTestSiteUse[1][index/4][index%4];
                    LastSet.bUseTestSocket[1][index/4][index%4]=!LastSet.bUseTestSocket[1][index/4][index%4];
                }
            }
            else
            {
                if(TestIF.iSiteMap[index/4-2][index%4+4]==0)
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bTestSiteUse[0][index/4-2][index%4+4]=false;
                    LastSet.bUseTestSocket[0][index/4-2][index%4+4]=false;

                    bTestSiteUse[1][index/4-2][index%4+4]=false;
                    LastSet.bUseTestSocket[1][index/4-2][index%4+4]=false;
                }
                else
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bTestSiteUse[0][index/4-2][index%4+4]=!bTestSiteUse[0][index/4-2][index%4+4];
                    LastSet.bUseTestSocket[0][index/4-2][index%4+4]=!LastSet.bUseTestSocket[0][index/4-2][index%4+4];

                    bTestSiteUse[1][index/4-2][index%4+4]=!bTestSiteUse[1][index/4-2][index%4+4];
                    LastSet.bUseTestSocket[1][index/4-2][index%4+4]=!LastSet.bUseTestSocket[1][index/4-2][index%4+4];
                }
            }

            value =  atoi(Data[index].c_str());
            bDutOnOff[index] = (value==1)? true:false;
        }
    }

    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
          index = i*MaxJ+j;
          //ChungHung 20130910 alter for SCK can close site by Index
          LastSet.bUseTestSocket[0][i][j] = bDutOnOff[index];
          bTestSiteUse[0][i][j] = bDutOnOff[index];

          LastSet.bUseTestSocket[1][i][j] = bDutOnOff[index];
          bTestSiteUse[1][i][j] = bDutOnOff[index];
        }
    }

    fMain->ShowTestHeadComp(false);                                           // REAL FormsFacade.h member
    return 0;
}
//---------------------------------------------------------------------------
int SetMapping(AnsiString *Data) //ok
{
    if(CheckSystemStart())
      return 2;
    if(CheckNeedCleanOut())
      return 1;

    //根據模式選擇
    //TestIF.iTestMode
    int MaxI=0;
    int MaxJ=0;
    switch(TestIF.iTestMode)
    {
      case DualSite://1x2
           MaxI=1;
           MaxJ=2;
           break;
      case SingleSite://1x1
           MaxI=1;
           MaxJ=1;
           break;
      case QualSite1X4://1x4
      case _8Site1X4:  //ChungHung 20150528 add for 海思 _8Site1x4
           MaxI=1;
           MaxJ=4;
           break;
      case QualSite2X2://2x2
           MaxI=2;
           MaxJ=2;
           break;
      case _6Site2X3:  //ChungHung 20140115 add for 2x3_6
           MaxI=2;
           MaxJ=3;
           break;
      case _16Site4X4:  //Sam 20190226 : 16Site4X4
      case _8Site2X4://2x4
           MaxI=2;
           MaxJ=4;
           break;
      case _10Site2X5://2x5 //wei 20190614 10 site
      case _12Site2X6://2x8
           MaxI=2;
           MaxJ=8;
           break;
    };

    //AnsiString KeyWord[16]={ "Dut1=" ,"Dut2=" ,"Dut3=" ,"Dut4=" ,"Dut5=" ,"Dut6=" ,"Dut7=" ,"Dut8=" ,
    //                         "Dut9=" ,"Dut10=" ,"Dut11=" ,"Dut12=" ,"Dut13=" ,"Dut14=" ,"Dut15=" ,"Dut16=" };
    //AnsiString Position[16]={ "Ch1" ,"Ch2" , "Ch3", "Ch4", "Ch5", "Ch6", "Ch7", "Ch8",
    //                          "Ch9", "Ch10", "Ch11", "Ch12", "Ch13", "Ch14", "Ch15", "Ch16" };

    AnsiString value;
    int index=0;
    int PosIndex[16]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
                      -1,-1,-1,-1,-1,-1};
    bool bSet[17]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
            index = i*MaxJ+j;
            PosIndex[index] = atoi(Data[index].c_str());

            if(PosIndex[index]<1 || PosIndex[index]>MaxI*MaxJ)
              return 3;

            if(bSet[PosIndex[index]] == false)
                bSet[PosIndex[index]]=true;
            else
                return 3;
        }
    }

    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
            index = i*MaxJ+j;
            TestIF_File.iSiteMap[i][j] = PosIndex[index];
        }
    }

    AnsiString S=GetLastOpenFN();
    AnsiString szDir="";
    AnsiString str;

    szDir.sprintf("%s%s", DataPath, S);
    MyForceDirectories(szDir, "SetMapping");
    szDir+="\\HandlerCondition.Data";

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            str.sprintf("Site %c%c", i+'A', j+'a');//kevin 20180312 j+'A' ==> j+'a'
            WriteIniData(szDir, "Configuration", str, TestIF_File.iSiteMap[i][j]);
        }
    }

    ChangeSite();                                                             // golden cinitial.cpp:12074 -- TU-local gate (offline no-op)

    DoStructUnitConvert();
    fMain->ShowTestHeadComp(false);                                           // REAL FormsFacade.h member
    W5FA_FTestCategory.AdjFormData();                                         // golden fTestCategory->AdjFormData() -- TU-local gate
    W5FA_FContactCT.ShowFormComp();                                           // golden fContactCT->ShowFormComp() -- TU-local gate

    return 0;
}
//---------------------------------------------------------------------------
int SetSetUpFileName(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;

    int index = W5FA_FMain.cbSetupFileName->Items->IndexOf(Data[0]);           // golden fMain->cbSetupFileName->Items->IndexOf(Data[0]) -- TU-local gate
    if(index!=-1)
    {
        RecordProcess("SetSetUpFileName");
        W5FA_FMain.cbSetupFileName->ItemIndex = index;
        W5FA_FMain.ChangeSetUpFile(W5FA_FMain.cbSetupFileName->Text);
        return 0;
    }
    return 3;
}
//---------------------------------------------------------------------------
int SetSoakTime(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    Temperature.fSoakTime=atof(Data[0].c_str());
    W5FA_FMain.edSoakTime->Text     =Temperature.fSoakTime;                    // golden fMain->edSoakTime->Text -- TU-local gate
    if(fTemp_Set)                                                              // AI(W906-FW-TEMP3) 20260820: real fTemp_Set (see GATED DEPENDENCIES note above), NULL-guarded -- retires W5FA_FTemp_Set
        fTemp_Set->edSoakTime->Text  =Temperature.fSoakTime;                   // golden fTemp_Set->edSoakTime->Text

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);
    MyForceDirectories(szDir, "SetSoakTime");
    szDir+="\\Temperature.Data";

    WriteIniData(szDir, "Time",       "Soak",       Temperature.fSoakTime);

    return 0;
}
//---------------------------------------------------------------------------
int SetTemperature(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    Temperature.fWorkTemperBase=atof(Data[0].c_str());
    W5FA_FMain.edWorkTemperBase->Text   = Temperature.fWorkTemperBase;         // golden fMain->edWorkTemperBase->Text -- TU-local gate
    if(fTemp_Set)                                                              // AI(W906-FW-TEMP3) 20260820: real fTemp_Set (see GATED DEPENDENCIES note above), NULL-guarded -- retires W5FA_FTemp_Set
        fTemp_Set->edWorkTemp->Text     = Temperature.fWorkTemperBase;         // golden fTemp_Set->edWorkTemp->Text

    AnsiString S="";

    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);
    MyForceDirectories(szDir, "SetTemperature");
    szDir+="\\Temperature.Data";

    WriteIniData(szDir, "Mode",       "Temperature", Temperature.fWorkTemperBase);

    return 0;
}
//---------------------------------------------------------------------------
int SetTempMode(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    LastSet.iTemperature=atoi(Data[0].c_str());
    return W5FA_FMain.ChangeTempMode(LastSet.iTemperature, false, bRefreshFunction);  // golden fMain->ChangeTempMode(...) -- TU-local gate
}
//---------------------------------------------------------------------------
int SetTesterConnect(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    LastSet.iTester=atoi(Data[0].c_str());
    return W5FA_FMain.ChangeTesterConnect(LastSet.iTester,false);             // golden fMain->ChangeTesterConnect(...) -- TU-local gate
}
//---------------------------------------------------------------------------
int SetTesterMode(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    W5FA_FAutomation.TestMode=atoi(Data[0].c_str());                          // golden fAutomation->TestMode -- TU-local gate (see report: fold onto TfAutomationShim)
    return 0;
}
//---------------------------------------------------------------------------
int SetLowYield(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.bFailAlarmLowYield  =atoi(Data[0].c_str());
    TestIF_File.iLowYieldLimit      =atof(Data[1].c_str());
    TestIF_File.iLowYieldCount      =atoi(Data[2].c_str());

    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Low Yield Alarm", "Enable",            int(TestIF_File.bFailAlarmLowYield));
    WriteIniData(szDir, "Low Yield Alarm", "Limit",             TestIF_File.iLowYieldLimit);
    WriteIniData(szDir, "Low Yield Alarm", "Count",             int(TestIF_File.iLowYieldCount));
    return 0;
}
//---------------------------------------------------------------------------
int SetByArmPerSiteDiffYield(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.bFailAlarmSiteYieldDifferent        =atoi(Data[0].c_str());
    TestIF_File.iFailAlarmSiteYield                 =atof(Data[1].c_str());
    TestIF_File.iFailAlarmSiteYieldDifferentCount   =atoi(Data[2].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different",          int(TestIF_File.bFailAlarmSiteYieldDifferent));
    WriteIniData(szDir, "Site Yield Alarm","Site Yield",             TestIF_File.iFailAlarmSiteYield);
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different Count",    int(TestIF_File.iFailAlarmSiteYieldDifferentCount));
    return 0;
}
//---------------------------------------------------------------------------
int SetConsecutiveFailureAlarmByHead(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.bContsFailByHead        =atoi(Data[0].c_str());
    TestIF_File.iContsFailHeadAlarmCT   =atoi(Data[1].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Alarm",     "HeadEnable",      int(TestIF_File.bContsFailByHead));
    WriteIniData(szDir, "Alarm",     "HeadCT",          int(TestIF_File.iContsFailHeadAlarmCT));
    return 0;
}
//---------------------------------------------------------------------------
int SetConsecutiveFailureAlarmBySocket(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.bContsFailBySocket      =atoi(Data[0].c_str());
    TestIF_File.iContsFailSocketAlarmCT =atoi(Data[1].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Alarm",     "SocketEnable",    int(TestIF_File.bContsFailBySocket));
    WriteIniData(szDir, "Alarm",     "SocketCT",        int(TestIF_File.iContsFailSocketAlarmCT));
    return 0;
}
//---------------------------------------------------------------------------
int SetAllSiteFailFor9045(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.bAllSiteFail=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Alarm4", "AllSiteFail",int(TestIF_File.bAllSiteFail));
    return 0;
}
//---------------------------------------------------------------------------
int SetContactModeFor9045(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.ContactMode=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Contact",        int(DeviceForm_File.ContactMode));
    return 0;
}
//---------------------------------------------------------------------------
int SetContactVacuumMode(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.VacuumMode=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Vacuum",          int(DeviceForm_File.VacuumMode));
    return 0;
}
//---------------------------------------------------------------------------
int SetContactDropWait(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.DropWait=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Wait Time", "Drop Wait",   DeviceForm_File.DropWait);
    return 0;
}
//---------------------------------------------------------------------------
int SetSlowContactSpeed(AnsiString *Data)
{
    AnsiString szDir="";
    if(CheckSystemStart())
        return 2;

    DeviceForm_File.DropSpeed=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Wait Time", "Drop Speed",  DeviceForm_File.DropSpeed);
    return 0;
}
//---------------------------------------------------------------------------
int SetShuttleWaitOutSideCamber(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.bShuttleWaitingOutSiteChamber=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Shuttle Waiting Out Site Chamber",  int(DeviceForm_File.bShuttleWaitingOutSiteChamber));
    return 0;
}
//---------------------------------------------------------------------------
int SetPickShuttleDeviceAfterTested(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.bSuckShuttleDeviceAfterTested=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Suck Shuttle Device After Tested",  int(DeviceForm_File.bSuckShuttleDeviceAfterTested));
    return 0;
}
//---------------------------------------------------------------------------
int SetPickShuttleDeviceThenWaitOnShuttle(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Suck Shuttle Device Wait On Shuttle",  int(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle));
    return 0;
}
//---------------------------------------------------------------------------
int SetPickShuttleDeviceTogetherFor32SiteN(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.bPickShuttleDeviceTogether=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "Pick Shuttle Device Together",  int(DeviceForm_File.bPickShuttleDeviceTogether));
    return 0;
}
//---------------------------------------------------------------------------
int SetIndexArm1Height(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    if(atof(Data[1].c_str())<atof(Data[0].c_str()))     //PickUp 必須高於 Release
        return 3;
    DeviceForm_File.IndexPlace[0]   =atof(Data[0].c_str());
    DeviceForm_File.IndexArmPick[0] =atof(Data[1].c_str());
    DeviceForm_File.IndexDrop[0]    =atof(Data[2].c_str());
    DeviceForm_File.IndexContact[0] =atof(Data[3].c_str());

    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Test Arm1", "Place",      DeviceForm_File.IndexPlace[0]);
    WriteIniData(szDir, "Test Arm1", "Pick Up",    DeviceForm_File.IndexArmPick[0]);
    WriteIniData(szDir, "Test Arm1", "Drop",       DeviceForm_File.IndexDrop[0]);
    WriteIniData(szDir, "Test Arm1", "Contact",    DeviceForm_File.IndexContact[0]);
    return 0;
}
//---------------------------------------------------------------------------
int SetIndexArm2Height(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    if(atof(Data[1].c_str())<atof(Data[0].c_str()))     //PickUp 必須高於 Release
        return 3;
    DeviceForm_File.IndexPlace[1]   =atof(Data[0].c_str());
    DeviceForm_File.IndexArmPick[1] =atof(Data[1].c_str());
    DeviceForm_File.IndexDrop[1]    =atof(Data[2].c_str());
    DeviceForm_File.IndexContact[1] =atof(Data[3].c_str());

    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Test Arm2", "Place",      DeviceForm_File.IndexPlace[1]);
    WriteIniData(szDir, "Test Arm2", "Pick Up",    DeviceForm_File.IndexArmPick[1]);
    WriteIniData(szDir, "Test Arm2", "Drop",       DeviceForm_File.IndexDrop[1]);
    WriteIniData(szDir, "Test Arm2", "Contact",    DeviceForm_File.IndexContact[1]);
    return 0;
}
//---------------------------------------------------------------------------
int SetTestICCheckMode(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    DeviceForm_File.iSocketInitialICCheckPosition=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "iSocketInitialICCheckPosition",  int(DeviceForm_File.iSocketInitialICCheckPosition));
    return 0;
}
//---------------------------------------------------------------------------
int SetAboveSocket(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    if(DeviceForm_File.iSocketInitialICCheckPosition!=1)
        return 3;
    DeviceForm_File.fSocketInitialICCheckPositionOffset=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Contact.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "fSocketInitialICCheckPositionOffset",  DeviceForm_File.fSocketInitialICCheckPositionOffset);
    return 0;
}
//---------------------------------------------------------------------------
int SetHotPlate1(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    AnsiString szDir="";
    if(atoi(Data[0].c_str())==0)
    {
        if(HotPlateForm_File.iPlateSelect==1)//HP1 要關閉，不能全關
            return 3;
        else
            HotPlateForm_File.iPlateSelect=2;
        }
    else
    {
        if(HotPlateForm_File.iPlateSelect==2)
            HotPlateForm_File.iPlateSelect=3;
        }
    szDir.sprintf("%s%s\\HotPlate.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Hotplate Form", "Using Flag",  int(HotPlateForm_File.iPlateSelect));
    return 0;
}
//---------------------------------------------------------------------------
int SetHotPlate2(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    AnsiString szDir="";
    if(atoi(Data[0].c_str())==0)
    {
        if(HotPlateForm_File.iPlateSelect==2)//HP1 要關閉，不能全關
            return 3;
        else
            HotPlateForm_File.iPlateSelect=1;
        }
    else
    {
        if(HotPlateForm_File.iPlateSelect==1)
            HotPlateForm_File.iPlateSelect=3;
        }
    szDir.sprintf("%s%s\\HotPlate.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Hotplate Form", "Using Flag",  int(HotPlateForm_File.iPlateSelect));
    return 0;
}
//---------------------------------------------------------------------------
int SetTesterInitialMaximumTest(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.iInitialMaxTime=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time",      "Initial MAX Time",    TestIF_File.iInitialMaxTime);
    return 0;
}
//---------------------------------------------------------------------------
int SetTesterMaximumTest(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.iMaxTime=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time",      "MAX Time",    TestIF_File.iMaxTime);
    return 0;
}
//---------------------------------------------------------------------------
int SetTesterDummyTest(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;
    if(CheckNeedCleanOut())
        return 1;
    AnsiString szDir="";
    TestIF_File.iDummyTime=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time",      "Dummy Time",    TestIF_File.iDummyTime);
    return 0;
}
//---------------------------------------------------------------------------
int SetTesterStartDelay(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    TestIF_File.dStartDelayTime=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Tester.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time",      "Stary Delay",    TestIF_File.dStartDelayTime);
    return 0;
}
//---------------------------------------------------------------------------
int SetHotZ1Down(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.bZ2DownSocket=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time", "bZ2DownSocket",int(Temperature.bZ2DownSocket));
    return 0;
}
//---------------------------------------------------------------------------
int SetHotShuttleSoakMode(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.iShuttleSoakTimeMode=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Time", "In Shuttle Soak Time Mode",int(Temperature.iShuttleSoakTimeMode));
    return 0;
}
//---------------------------------------------------------------------------
int SetAmbientCheck(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.bUseAbitCHK=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Mode", "AmbCheck",int(Temperature.bUseAbitCHK));
    return 0;
}
//---------------------------------------------------------------------------
int SetAmbientCheckTemp(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.fAbitTemp=atof(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "Ambient",      "Temperature",    Temperature.fAbitTemp);
    return 0;
}
//---------------------------------------------------------------------------
int SetTemperatureOffset(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.bEnableTempOffsetForInitial=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "InitialMode",      "EnableTemperatureOffsetforInitial",    int(Temperature.bEnableTempOffsetForInitial));
    return 0;
}
//---------------------------------------------------------------------------
int SetContactCountForOffsetPeriod(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.iCintactCntForTempOffsetAtInitial=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "InitialMode",      "iCintactCntForTempOffsetAtInitial",    int(Temperature.iCintactCntForTempOffsetAtInitial));
    return 0;
}
//---------------------------------------------------------------------------
int SetContactCountForCoolDown(AnsiString *Data)
{
    if(CheckSystemStart())
        return 2;

    AnsiString szDir="";
    Temperature.iCintactDelayCntForInitTempOffset=atoi(Data[0].c_str());
    szDir.sprintf("%s%s\\Temperature.Data", DataPath, GetLastOpenFN());
    WriteIniData(szDir, "InitialMode",      "iCintactDelayCntForInitTempOffset",    int(Temperature.iCintactDelayCntForInitTempOffset));
    return 0;
}
//---------------------------------------------------------------------------
int SetLotInfo(AnsiString *Data)
{
    if(W5FA_FMain.CheckCanChangeRealDummy())                                   // golden fMain->CheckCanChangeRealDummy() -- TU-local gate
    {
        W5FA_FLotInfo.edtSysOperatorID->Text=Data[1];                          // golden fLotInfo->edtSysOperatorID->Text -- TU-local gate
        fLotInfo->SetLotID(Data[0], false);                                    // REAL FormsFacade.h member
        fLotInfo->SetLotStart(__FUNC__);                                       // REAL FormsFacade.h member
        if(Data[2]=="0")
        {
            W5FA_FMain.DoFTRTClick(false, false);                             // golden fMain->DoFTRTClick(...) -- TU-local gate
        }
        else
        {
            W5FA_FMain.DoFTRTClick(true, false);
        }
        return 0;
    }
    else
    {
        return 2;
    }
}
//---------------------------------------------------------------------------
//AI(ht9045-v899) 20260515: F899-008 OEE_StartLot 失敗 errMsg 關鍵字分流（對齊 SPEC §6.3 / §8.5）
// 實際對盤 OEE_StartLot 6 條 errMsg：
//   - "MO or Machine ID is empty"                                  → 62
//   - "... Set MO Information Fail!" / "Auto Download Setup File by MO ..." /
//     "... AutoDownloadSetupFileByMO() Function" / "... CheckLoadMOInformation() Function" → 61。
// 90 為防禦性 fallback（理論不觸發）。
static int GreatekMapOEEStartLotError(AnsiString asErrorMsg)
{
    if(asErrorMsg.Pos("Machine ID")>0)
        return 62;
    if(asErrorMsg.Pos("MO")>0)
        return 61;
    return 90;
}
//---------------------------------------------------------------------------
//AI(JimmyChiu) 20260515: F899-008 超豐 LotInfo_REQUEST 內嵌 OEE Start Lot（v3）
// 設計依據：SPEC F899-008 §6.3 v3。
// 2026-05-15 客戶二次確認：超豐 Agent 不送 LotID/OPID/RunMode（全部由雲端 MO file 帶入），
//   故本函式忽略 Data[0..2]，僅執行：預檢查 → OEE_StartLot → SetLotID → SetLotStart →
//   SetLotComponents(false) → SetFirstTrayCheckOnUnloader。
// 不重構 sbSECSLotStartClick（見 SPEC §6.1）。
int SetLotInfoGreatekOEE(AnsiString *Data, AnsiString &asErrorMsg)
{
    (void)Data;     // 客戶決定不使用，保留參數僅為呼叫端 signature 一致
    asErrorMsg="";

    // ── (1) 預檢查──
    if(W5FA_FMain.CheckCanChangeRealDummy()==false)                            // golden fMain->CheckCanChangeRealDummy() -- TU-local gate
    {
        asErrorMsg="Machine running or has residual IC";
        return 2;
    }

    // Monitor 連線預檢（對齊 uLotInfo.cpp L8023 手動路徑）。
    if(IniConfig.bC11UseMonitorView && W5FA_FMonitor.MVCtrl_IsConnect()==false) // golden fMonitor->MVCtrl->IsConnect() -- TU-local gate
    {
        asErrorMsg="Please Check Monitor Connect!!!";
        return 60;
    }

    // ── (2) OEE_StartLot（silent，使用 ProductionInfo.cpp 既有介面）──
    //   超豐由雲端 MO file 帶入 LotID/OPID 等資訊，本函式不再寫 UI 欄位。
    if(W5FA_FProductionInfo.OEE_StartLot(true, &asErrorMsg)==false)            // golden fProductionInfo->OEE_StartLot(...) -- TU-local gate
    {
        if(asErrorMsg=="")
            asErrorMsg="Production Start Lot Fail";
        return GreatekMapOEEStartLotError(asErrorMsg);
    }

    // ── (3) SetLotID + SetLotStart（對齊 §1.2 step 12/13）──
    //   edtSysLotID->Text 由 OEE 雲端下載階段透過 MO 對應流程更新（OEE_StartLot 後讀當下值）。
    fLotInfo->SetLotID(fLotInfo->edtSysLotID->Text, false);                    // REAL FormsFacade.h members
    fLotInfo->SetLotStart(__FUNC__);

    // ── (4) 強制 lock：補 SetLotStart 對空 LotID 走 SetLotComponents(true) 的缺口──
    W5FA_FLotInfo.SetLotComponents(false);                                     // golden fLotInfo->SetLotComponents(false) -- TU-local gate

    // ── (5) First Tray Check（對齊 §1.2 step 15）──
    W5FA_FLotInfo.SetFirstTrayCheckOnUnloader();                               // golden fLotInfo->SetFirstTrayCheckOnUnloader() -- TU-local gate

    return 0;
}
//---------------------------------------------------------------------------
//extern void SetRunStartMode(eRunStartMode Mode=rsmNull, AnsiString ModeText="");
int SetStartMode(AnsiString *Data)                                              //Sam 20221212 : 新增 StartMode 指令
{
    int iStartMode=0;
    //if(RunInfo.bLotStart==false)
    if(W5FA_FMain.CheckCanChangeRealDummy())                                   // golden fMain->CheckCanChangeRealDummy() -- TU-local gate
    {
        if(Data[0]=="0" || Data[0]=="1" || Data[0]=="2" || Data[0]=="3")
        {
            iStartMode=atoi(Data[0].c_str());
            SetRunStartMode((eRunStartMode)iStartMode);                        // REAL: aHotPlateSubstrate.h (SetRunStartMode(int))
            return 0;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        return 2;
    }
}
//---------------------------------------------------------------------------
bool CheckInitialOK()
{
    return InitialOK;
}
//---------------------------------------------------------------------------
bool CheckNeedCleanOut()
{
    if(CheckCanChangeRealDummy()==false || HasICUnderMachine())                //Steven 20240719 : && --> ||
        return true; //需要cleanout
    else
        return false;
}
//---------------------------------------------------------------------------
bool CheckSystemStart()
{
    return SystemStart;
}
//---------------------------------------------------------------------------
int ShowOLPState(int iState)
{
    W5FA_FMain.ShowOLPState(iState);                                           // golden fMain->ShowOLPState(iState) -- TU-local gate
    return 0;
}
//---------------------------------------------------------------------------
AnsiString GetMainStatus()
{
    AnsiString S=W5FA_FMain.palMainStatus->Caption.UpperCase();                // golden fMain->palMainStatus->Caption -- TU-local gate
    if(S=="RUNNING" ||
       S=="PAUSE" ||
       S=="HALT"  ||
       S=="LOCK")
    {
        if(S=="RUNNING")
        {
            S="RUN";
        }

//        if(S=="LOCK")
//        {
//            S="HALT";
//        }
        return S;
    }
    else
    {
        return "RUN";
    }
}
//---------------------------------------------------------------------------
AnsiString GetDataPath()
{
    return DataPath;
}
//---------------------------------------------------------------------------
AnsiString GetSoftwareVersion()
{
    return W5FA_FObserver.Memo1Lines->Strings0;                                // golden fObserver->Memo1->Lines->Strings[0] -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetActTemp(int i)
{
    return UN150Read[i];
}
//---------------------------------------------------------------------------
AnsiString GetHandlerType()
{
    return "HT9045";
}
//---------------------------------------------------------------------------
AnsiString GetSetUpFileName()
{
    return W5FA_FMain.cbSetupFileName->Text;                                   // golden fMain->cbSetupFileName->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetJamCount()
{
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)  //Sam 20190429 : Add CC_PTI_NEWWORK
    {
        return AnsiString(W5FA_LS_iJamCount[0]);                               // golden LastSet.iJamCount[0] -- TU-local gate (see gate #5)
    }
    else
    {
        return AnsiString(W5FA_LS_iJamCount[1]);                               // golden LastSet.iJamCount[1] -- TU-local gate
    }
}
//---------------------------------------------------------------------------
AnsiString GetSoakTime()
{
    return AnsiString(Temperature.fSoakTime);
}
//---------------------------------------------------------------------------
AnsiString GetTemperature()
{
    return AnsiString(Temperature.fWorkTemperBase);
}
//---------------------------------------------------------------------------
AnsiString GetTempMode()
{
    return AnsiString(LastSet.iTemperature);
}
//---------------------------------------------------------------------------
AnsiString GetTesterConnect()
{
    return AnsiString(LastSet.iTester);
}
//---------------------------------------------------------------------------
AnsiString GetTesterMode()
{
    return AnsiString(W5FA_FAutomation.TestMode);   //未完成 配合測試流程          // golden fAutomation->TestMode -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetLoadCount()
{
    return LastSet.SendCT[1];    // 2009.09.04 , Joye
}
//---------------------------------------------------------------------------
AnsiString GetAutoHeight(int index)
{
    return DeviceForm_File.IndexContact[index];
}
//---------------------------------------------------------------------------
AnsiString GetContactOffset(int index)
{
    return Offset.iIndexArmContact[index];
}
//---------------------------------------------------------------------------
AnsiString GetContactTestMode()
{
    return DeviceForm.ContactMode;
}
//---------------------------------------------------------------------------
AnsiString GetSecondSpeed()
{
    return DeviceForm_File.DropSpeed;
}
//---------------------------------------------------------------------------
AnsiString GetContactWaitHeight()
{
    return 0;
}
//---------------------------------------------------------------------------
AnsiString GetDropHeight(int index)
{
    return DeviceForm_File.IndexDrop[index];
}
//---------------------------------------------------------------------------
AnsiString GetReleaseWait()
{
    return DeviceForm_File.DropWait;
}
//---------------------------------------------------------------------------
AnsiString GetShuttlePickOffset(int index)
{
    return Offset_File.iIndexArmPickUp[index];
}
//---------------------------------------------------------------------------
AnsiString GetShuttlePickHeight(int index)
{
    return DeviceForm_File.IndexArmPick[index];
}
//---------------------------------------------------------------------------
AnsiString GetShuttleReleaseHeight(int index)
{
    return DeviceForm_File.IndexPlace[index];
}
//---------------------------------------------------------------------------
AnsiString GetArmTotalForce(int index)
{
    if(index==0)
        return DeviceForm.dPress;
    else
        return W5FA_FContact.edAirForceN->Text;                                // golden fContact->edAirForceN->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetClinderForce(int index)
{
    if(index==0)
        return W5FA_FContact.edForcePerDeviceKG->Text;                         // golden fContact->edForcePerDeviceKG->Text -- TU-local gate
    else
        return W5FA_FContact.edForcePerDeviceN->Text;                          // golden fContact->edForcePerDeviceN->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetForcePerDevice(int index)
{
    if(index==0)
        return W5FA_FContact.edForcePerDeviceKG->Text;
    else
        return W5FA_FContact.edForcePerDeviceN->Text;
}
//---------------------------------------------------------------------------
AnsiString GetSetNoPerPin()
{
    return W5FA_FContact.edPinCount->Text;                                     // golden fContact->edPinCount->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetForcePerPin()
{
    return W5FA_FContact.edForcePerPinG->Text;                                 // golden fContact->edForcePerPinG->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetContactForcen()
{
    return W5FA_FContact.edForcePerPinN->Text;                                 // golden fContact->edForcePerPinN->Text -- TU-local gate
}
//---------------------------------------------------------------------------
AnsiString GetStartPos(int index)
{
    if(index==0)
        return UserDefForm_File[0].XStart;
    else
        return UserDefForm_File[0].YStart;
}
//---------------------------------------------------------------------------
AnsiString GetDivision(int index)
{
    if(index==0)
        return UserDefForm_File[0].XDivision;
    else
        return UserDefForm_File[0].YDivision;
}
//---------------------------------------------------------------------------
int GetDimemsion(AnsiString *Data)
{
    Data[0]=FormatFloat("0.00",DeviceForm_File.XDimension);
    Data[1]=FormatFloat("0.00",DeviceForm_File.YDimension);
    return 2;
}
//---------------------------------------------------------------------------
AnsiString GetZThickness()
{
    return UserDefForm_File[0].ZDepth;
}
//---------------------------------------------------------------------------
AnsiString GetTrayType()
{
    return UserDefForm_File[0].Alias;
}
//---------------------------------------------------------------------------
AnsiString GetPitch(int index)
{
    if(index==0)
        return UserDefForm_File[0].XPitch;
    else
        return UserDefForm_File[0].YPitch;
}
//---------------------------------------------------------------------------
//AnsiString GetIP()
//{
//}
//---------------------------------------------------------------------------
//AnsiString GetPort()
//{
//}
//---------------------------------------------------------------------------
//AnsiString GetCusCode()
//{
//}
//---------------------------------------------------------------------------
bool CheckCanChangeRealDummy()
{
    return W5FA_FMain.CheckCanChangeRealDummy();                               // golden fMain->CheckCanChangeRealDummy() -- TU-local gate
}
//---------------------------------------------------------------------------
void DoClearReportRequest(int iSwitchCase)
{
    int iCase=0;
    if(iSwitchCase==1111)   //全清
    {
        W5FA_FCounterClear.ClearCount(ctTimeData);                             // golden fCounterClear->ClearCount(...) -- TU-local gate
        W5FA_FCounterClear.ClearCount(ctLoadingCounts);
        W5FA_FCounterClear.ClearCount(ctTraySortCount);
        W5FA_FCounterClear.ClearCount(ctContactCounts);
        W5FA_FCounterClear.ClearCount(ctTesterCategory);
    }
    else
    {
        switch(iSwitchCase)
        {
            case 2: iCase=ctTimeData; break;
            case 3: iCase=ctLoadingCounts; break;
            case 4: iCase=ctTraySortCount; break;
            case 5: iCase=ctContactCounts; break;
            case 6: iCase=ctTesterCategory; break;
//          case 9: iCase=0; break;       //GPIB
        }
        W5FA_FCounterClear.ClearCount(iCase);
    }
}
//---------------------------------------------------------------------------
int GetCategory(AnsiString *Data)
{
    Data[0]= BinSelect[iTestRunMode].IfErrorT3;
    for(int i=0; i<=iTestBinCount; i++)
    {
        Data[i+1]=BinSelect[iTestRunMode].iCatDataT3Pos[i];
    }
    return 17;
}
//---------------------------------------------------------------------------
int GetBindefine(AnsiString *Data)   //Sam 20230803 : 新增OLP指令
{
//    int iStack=0;
    Data[0]="0";
    for(int i=0; i<iTestBinCount; i++)
    {
        if(Prod.iT6CatData[i]>=0)
        {
            if(Prod.bIsPassBin[i]==1)
                Data[i+1]="1";
             else
                Data[i+1]="0";
        }
        else
        {
            Data[i+1]="0";
        }
    }
    return 17;
}
//---------------------------------------------------------------------------
int GetFixTrayDefine(AnsiString *Data)  //Sam 20230921 : 新增 FixTray 指令  //QQQ (pre-existing golden marker, preserved verbatim -- golden authors' own second-look flag, not resolved here)
{
    Data[0]=IntToStr(TrayForm.LodareType);
    Data[1]=IntToStr(TrayForm.iFixTrayMode);
    Data[2]=(TrayForm.bTrayUpDownSet[eFix1])?"1":"0";
    Data[3]=(TrayForm.bTrayUpDownSet[eFix2])?"1":"0";
    Data[4]=(TrayForm.bTrayUpDownSet[eFix3])?"1":"0";
    return 5;
}
//---------------------------------------------------------------------------
int GetMapping(AnsiString *Data)
{
    //根據模式選擇
    //TestIF.iTestMode
    int MaxI=0;
    int MaxJ=0;
    /*
    AnsiString asY1Site[]={"Aa:","Ba:","Ca:","Da:","Ea:","Fa:","Ga:","Ha:"};
    AnsiString asY2Site[]={"Aa:","Ba:",
                           "Ab:","Bb:",
                           "Ac:","Bc:",
                           "Ad:","Bd:",
                           "Ae:","Be:",
                           "Af:","Bf:",
                           "Ag:","Bg:",
                           "Ah:","Bh:"};
    AnsiString asY4Site[]={"Aa:","Ba:","Ca:","Da:",
                           "Ab:","Bb:","Cb:","Db:",
                           "Ac:","Bc:","Cc:","Dc:",
                           "Ad:","Bd:","Cd:","Dd:",
                           "Ae:","Be:","Ce:","De:",
                           "Af:","Bf:","Cf:","Df:",
                           "Ag:","Bg:","Cg:","Dg:",
                           "Ah:","Bh:","Ch:","Dh:"};
    */
    AnsiString asSite[]={"A:","B:","C:","D:",
                         "E:","F:","G:","H:",
                         "I:","J:","K:","L:",
                         "M:","N:","O:","P:",
                         "Q:","R:","S:","T:",
                         "U:","V:","W:","X:",
                         "Y:","Z:","0:","1:",
                         "2:","3:","4:","5:"};
    switch(TestIF.iTestMode)
    {
        case SingleSite://1x1
             MaxI=1;
             MaxJ=1;
             break;
        case DualSite://1x2
             MaxI=1;
             MaxJ=2;
             break;
        case TriSite1X3:
             MaxI=1;
             MaxJ=3;
             break;
        case QualSite1X4://1x4
        case _8Site1X4:         //ChungHung 20150528 add for 海思 _8Site1x4
             MaxI=1;
             MaxJ=4;
             break;
        case DualSite2x1:
             MaxI=2;
             MaxJ=1;
             break;
        case QualSite2X2://2x2
        case QualSite2X2N:      //Frank 20200520 2X2NN Mode
             MaxI=2;
             MaxJ=2;
             break;
        case _6Site2X3:
        case _6Site2X3N:        //Steven 20220425 : 2X3NN Mode
             MaxI=2;
             MaxJ=3;
             break;
        case _8Site2X4://2x4
        case _8Site2X4N:        //Wei 20231211 : 2X4NN Mode
             MaxI=2;
             MaxJ=4;
             break;
        case _10Site2X5:        //wei 20190614 10 site
             MaxI=2;
             MaxJ=5;
             break;
        case _12Site2X6:
             MaxI=2;
             MaxJ=6;
             break;
        case _16Site2X8://2x8
             MaxI=2;
             MaxJ=8;
             break;
        case _16Site4X4:
             MaxI=4;
             MaxJ=4;
             break;
        case _32Site4X8N:
             MaxI=4;
             MaxJ=8;
             break;
        case _32Site4X8M:
             MaxI=4;
             MaxJ=8;
             break;
    };

    //AnsiString KeyWord[16]={ "Dut1=" ,"Dut2=" ,"Dut3=" ,"Dut4=" ,"Dut5=" ,"Dut6=" ,"Dut7=" ,"Dut8=" ,
    //                         "Dut9=" ,"Dut10=" ,"Dut11=" ,"Dut12=" ,"Dut13=" ,"Dut14=" ,"Dut15=" ,"Dut16=" };
    //AnsiString Position[17]={ "---","Ch1" ,"Ch2" , "Ch3", "Ch4", "Ch5", "Ch6", "Ch7", "Ch8",
    //                          "Ch9", "Ch10", "Ch11", "Ch12", "Ch13", "Ch14", "Ch15", "Ch16" };

    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
    {
        Data[0]=IntToStr(MaxJ);  //Sam 20200414 : 加入 MaxI & MaxJ
        Data[1]=IntToStr(MaxI);  //Sam 20200414 : 加入 MaxI & MaxJ
        for(int j=0; j<MaxJ; j++)
        {
            for(int i=0; i<MaxI; i++)
            {
                /*
                if(MaxI==1)
                {
                    Data[j*MaxI+i]=asY1Site[j*MaxI+i]+TestIF.iSiteMap[i][j];
                }
                else if(MaxI==2)
                {
                    Data[j*MaxI+i]=asY2Site[j*MaxI+i]+TestIF.iSiteMap[i][j];
                }
                else
                {
                    Data[j*MaxI+i]=asY4Site[j*MaxI+i]+TestIF.iSiteMap[i][j];
                }*/
                Data[2+j*MaxI+i]=asSite[j*MaxI+i]+TestIF.iSiteMap[i][j]; //Sam 20200414 : 加入 MaxI & MaxJ
            }
        }
        return 2+(MaxI*MaxJ);    //Sam 20200414 : 加入 MaxI & MaxJ
    }
    else
    {
        for(int i=0; i<MaxI; i++)
        {
            for(int j=0; j<MaxJ; j++)
            {
                Data[i*MaxJ+j] = TestIF.iSiteMap[i][j];
            }
        }
        return MaxI*MaxJ;
    }
}
//---------------------------------------------------------------------------
int GetDutOnOff(AnsiString *Data)
{
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(bUseTwoArm32Site==true)
                Data[i*TestSocket.iShtCol+j]=(LastSet.bUseTestSocket[0][i][j]==true)?1:0;
            else
                Data[i*TestSocket.iShtCol+j]=(LastSet.bUseTestSocket[0][i][j]==true || LastSet.bUseTestSocket[1][i][j]==true)?1:0;
        }
    }
    return TestSocket.iShtCnt;
}
//---------------------------------------------------------------------------
int GetProductivity(AnsiString *Data)
{
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
    {
        Data[0]=W5FA_FObserver.labModel->Caption;                              // golden fObserver->labModel->Caption -- TU-local gate
        Data[1]=W5FA_FObserver.labPowerOnTime->Caption;
        Data[2]=W5FA_FObserver.labRunningTime->Caption;
        Data[3]=W5FA_FObserver.labProductTime->Caption;
        Data[4]=W5FA_FObserver.labLoadingCount->Caption;
        Data[5]=W5FA_FObserver.labMUBA->Caption;                                //MUBF
        Data[6]=W5FA_FObserver.labMTBA->Caption;                                //MTBF
    }
    else
    {
        Data[0]="0";                                                            //Down Time Rate
        Data[1]=W5FA_FObserver.labMUBA->Caption;                                //MUBF
        Data[2]=W5FA_FObserver.labMTBA->Caption;                                //MTBF
        Data[3]="0";
    }
    return 0;                                                                   //MTTR
}
//---------------------------------------------------------------------------
int GetSortingCount(AnsiString *Data)
{
    Data[0]=AnsiString(W5FA_LS_BinCT[0][e3Auto1]);                             // golden LastSet.BinCT[0][e3Auto1] -- TU-local gate (see gate #5)
    Data[1]=AnsiString(W5FA_LS_BinCT[0][e3Auto2]);
    Data[2]=AnsiString(W5FA_LS_BinCT[0][e3Auto3]);
    Data[3]=AnsiString(W5FA_LS_BinCT[0][e3Fix1 ]);
    Data[4]=AnsiString(W5FA_LS_BinCT[0][e3Fix2 ]);
    Data[5]=AnsiString(W5FA_LS_BinCT[0][e3Fix3 ]);
    if(TrayForm.iFixTrayMode==0)
    {
        Data[6]=0;
        Data[7]=0;
        Data[8]=0;
    }
    else
    {
        Data[6]=AnsiString(W5FA_LS_BinCT[0][e3Fix4]);
        Data[7]=AnsiString(W5FA_LS_BinCT[0][e3Fix5]);
        Data[8]=AnsiString(W5FA_LS_BinCT[0][e3Fix6]);
    }
    return 0;
}
//---------------------------------------------------------------------------
int GetSocketCount(AnsiString *Data)
{
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                Data[j+i*TestSocket.iShtCol]=W5FA_ArmData_GetTotal(0,i,j)+W5FA_ArmData_GetTotal(1,i,j);  // golden ArmData[k]->ArmSKET[i][j]->GetTotal() -- TU-local gate (see gate #4)
            }
        }
        return TestSocket.iShtCnt;
    }
    else
    {
        //HT9045,最多8個Socket
        // Data Format : Bin0,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,Bin10,Bin11,Bin12,Bin13,Bin14,Bin15,SKIP,TOTAL,PASS
        // 順序: Aa, Ab, Ac, Ad, Ba, Bb, Bc, Bd
        int iIFErrTotal=0, iGategoryTotal[TEST_MAX_BIN]={0}, i_Total=0, i_Pass_Total=0;   //Steven 20121112 : RS232支援32Bin 15 --> TEST_MAX_BIN
        int DutCount=MAX_SOCKET_TOTAL;
        for(int i=0; i<DutCount; i++)
        {
            int j=i/NEW_MAX_Index_Col, k=i%NEW_MAX_Index_Col;

            // Bin 0 Data
            Data[i]="0,";

            // Bin 1 ~ Bin15
            for(int l=0; l<iTestBinCount; l++)                                  //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
            {
                Data[i]+=AnsiString(W5FA_ArmData_GetSelBinCT(0,j,k,l)+W5FA_ArmData_GetSelBinCT(1,j,k,l))+",";   // golden ArmData[k]->ArmSKET[j][k]->GetSelBinCT(l) -- TU-local gate
                iGategoryTotal[l]+=(W5FA_ArmData_GetSelBinCT(0,j,k,l)+W5FA_ArmData_GetSelBinCT(1,j,k,l));
            }

            //I/F Error
            Data[i]+=AnsiString(W5FA_ArmData_GetIFError(0,j,k)+W5FA_ArmData_GetIFError(1,j,k))+",";              // golden ArmData[k]->ArmSKET[j][k]->GetIFError() -- TU-local gate
            //Total
            Data[i]+=AnsiString(W5FA_ArmData_GetTotal(0,j,k)+W5FA_ArmData_GetTotal(1,j,k))+",";
            //Pass
            Data[i]+=AnsiString(W5FA_ArmData_GetPassCT(0,j,k)+W5FA_ArmData_GetPassCT(0,j,k));                    // golden ArmData[0]->ArmSKET[j][k]->GetPassCT() twice (verbatim golden quirk: 2nd term also reads Arm0, not Arm1)
            i_Pass_Total+=(W5FA_ArmData_GetPassCT(0,j,k)+W5FA_ArmData_GetPassCT(1,j,k));
            iIFErrTotal+=(W5FA_ArmData_GetIFError(0,j,k)+W5FA_ArmData_GetIFError(1,j,k));
        }
        Data[DutCount]="0";

        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
        {
            Data[DutCount]+=","+AnsiString(iGategoryTotal[i]);
            i_Total+=iGategoryTotal[i];
        }
        Data[DutCount]+=","+AnsiString(iIFErrTotal)+","+AnsiString(i_Total)+","+AnsiString(i_Pass_Total);

        return DutCount+1;
    }
}
//---------------------------------------------------------------------------
int GetHeadCount(AnsiString *Data)
{
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                Data[j+i*TestSocket.iShtCol]                                      =W5FA_ArmData_GetTotal(0,i,j);
                Data[j+i*TestSocket.iShtCol+TestSocket.iShtCnt]=W5FA_ArmData_GetTotal(1,i,j);
            }
        }
        return (TestSocket.iShtCnt)*2;
    }
    else
    {
        int iIFErrTotal=0, iGategoryTotal[TEST_MAX_BIN]={0}, i_Total=0, i_Pass_Total=0;       //Steven 20121112 : RS232支援32Bin 15 --> TEST_MAX_BIN
        // Data Format : Bin0,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,Bin10,Bin11,Bin12,Bin13,Bin14,Bin15,SKIP,TOTAL,PASS
        // 順序: Aa, Ab, Ac, Ad, Ba, Bb, Bc, Bd 先Arm1再Arm2
        // 尚未加入9046的支援
        for(int x=0; x<2; x++)
        {
            for(int i=0; i<NEW_MAX_Index_Col; i++)
            {
                int j=i/NEW_MAX_Index_Col, k=i%NEW_MAX_Index_Col, y=x*8+i;

                // Bin 0 Data
                Data[y]="0,";
                iIFErrTotal+=W5FA_ArmData_GetIFError(x,j,k);//category total->IFErr

                // Bin 1 ~ Bin15
                for(int l=0; l<iTestBinCount; l++)     //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                {
                    Data[y]+=AnsiString(W5FA_ArmData_GetSelBinCT(x,j,k,l))+",";
                    iGategoryTotal[l]+=W5FA_ArmData_GetSelBinCT(x,j,k,l);
                }

                //I/F Error
                Data[y]+=AnsiString(W5FA_ArmData_GetIFError(x,j,k))+",";
                //Total
                Data[y]+=AnsiString(W5FA_ArmData_GetTotal(x,j,k))+",";
                //Pass
                Data[y]+=AnsiString(W5FA_ArmData_GetPassCT(x,j,k));
                i_Pass_Total+=W5FA_ArmData_GetPassCT(x,j,k);
            }
        }
        Data[iTestBinCount]="0";
        for(int i=0; i<iTestBinCount; i++)
        {
            Data[iTestBinCount]+=","+AnsiString(iGategoryTotal[i]);
            i_Total+=iGategoryTotal[i];
        }
        Data[iTestBinCount]+=","+AnsiString(iIFErrTotal)+","+AnsiString(i_Total)+","+AnsiString(i_Pass_Total);

        return iTestBinCount+2;
    }
}
//---------------------------------------------------------------------------
int GetTestResult(AnsiString *Data)
{
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)
    {
        for(int i=0; i<TestSocket.iShtCnt; i++)
        {
            Data[i]=TestIF.testBin[i];
        }
        return TestSocket.iShtCnt;
    }
    else
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC)
                    Data[i*NEW_MAX_Index_Col+j].sprintf("Dut%d=Bin%d", TestIF.iSiteMap[i][j], (TestSocket.Item[i][j]-TEST_PASS+1));
                else
                    Data[i*NEW_MAX_Index_Col+j].sprintf("Dut%d=Bin%d", TestIF.iSiteMap[i][j], 0);
            }
        }
        return MAX_Index_Row*NEW_MAX_Index_Col;
    }
}
//---------------------------------------------------------------------------
int GetHandMode(AnsiString *Data)
{
    Data[0]=TestIF.iTestMode;

    return 1;
}
//---------------------------------------------------------------------------
int GetTrayForm(AnsiString *Data)
{
    int iType=0;

    Data[0]=AnsiString(UserDefForm[iType].XDivision);
    Data[1]=AnsiString(UserDefForm[iType].YDivision);
    Data[2]=AnsiString(UserDefForm[iType].XStart);
    Data[3]=AnsiString(UserDefForm[iType].YStart);
    Data[4]=AnsiString(UserDefForm[iType].XPitch);
    Data[5]=AnsiString(UserDefForm[iType].YPitch);
    return 6;
}
//---------------------------------------------------------------------------
int GetPlateForm(AnsiString *Data)
{
    Data[0]=AnsiString(HotPlateForm.XDivision);
    Data[1]=AnsiString(HotPlateForm.YDivision);
    Data[2]=AnsiString(HotPlateForm.XStart);
    Data[3]=AnsiString(HotPlateForm.YStart);
    Data[4]=AnsiString(HotPlateForm.XPitch);
    Data[5]=AnsiString(HotPlateForm.YPitch);

    return 6;
}
//---------------------------------------------------------------------------
void DoOnLineReply(AnsiString *Data)
{
    Data[0]=IniConfig.sMachineType;
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Steven 20110902
    {
        Data[1]=AnsiString(PC_NAME);
    }
    else
    {
        Data[1]=IniConfig.SocketHandlerID;
    }

    Data[2]=W5FA_FObserver.Memo1Lines->Strings0;                                //軟體版本                    // golden fObserver->Memo1->Lines->Strings[0] -- TU-local gate
    Data[3]="Ready";
    Data[4]="0";
}
//---------------------------------------------------------------------------
void AlarmMessage(AnsiString enMess, AnsiString chMess)
{
    ShowMyMessage(enMess, chMess);
}
//---------------------------------------------------------------------------
void DoULRequest(AnsiString *Data)                                              //上傳檔案到Host
{
    // AI(W5-Final-Auto9045) 20260711: FAITHFUL translation of golden DoULRequest
    // (auto9045.cpp:1855-1922) preserved verbatim, including its real `system()`
    // shell-outs (del / 7z.exe) and CopyFile between the system 7-Zip install
    // path and "d:\HT9045\7z.exe". JUDGMENT CALL (flagged for review): this
    // function has REAL, uncontrolled OS side effects (deletes a real .zip,
    // shells out to a real 7z.exe, reads/writes real files under DataPath) --
    // the translate report explicitly recommends the test harness NOT call
    // this function, to avoid mutating a real dev machine's filesystem.
    SetCurrentDirectory("D://");                                               // golden SetCurrentDirectory(_T("D://")) -- _T() neutralized (ANSI build, _T(x)==x)

    AnsiString str1;
    AnsiString sULFileName=W5FA_FMain.cbSetupFileName->Text;                    // golden fMain->cbSetupFileName->Text -- TU-local gate
    char ddd[10];
    FILE *fs;

    //先刪除舊的壓縮檔
    str1.sprintf("del %s%s.zip", DataPath, sULFileName);
    system(str1.c_str());
    MySleep(50);

    //把7Zip準備好
    if(FileExists( "d:\\HT9045\\7z.exe" ) == false)
    {
        CopyFile( "C:\\Program Files\\7-Zip\\7z.exe" , "d:\\HT9045\\7z.exe" , false);
    }

    //把工作檔壓縮
    str1.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s%s.zip\" \"%s%s\\*.*\"", DataPath, sULFileName, DataPath, sULFileName);
    system(str1.c_str());

    str1="";
    int j=0;
    int size=0;
    char ccc;
    str1.sprintf("%s%s.zip", DataPath, sULFileName);
    fs=fopen(str1.c_str(), "rb");
    //記錄檔案大小
    if(fs!=NULL)    // 2009.07.30 , Joye
    {
        fseek(fs, 0, SEEK_END);
        size=ftell(fs);
    }
    fclose(fs);

    //將工作檔的壓縮檔進行轉換成要傳輸的格式
    fs=fopen(str1.c_str(), "rb");
    if(fs!=NULL)    // 2009.07.30 , Joye
    {
        str1="";
        while(j<=size)
        {
            j++;
            if(fscanf(fs,"%1c",&ccc))
            {
                sprintf(ddd,"%02X",ccc);
                if(ddd[0]=='F' && ddd[1]=='F' && ddd[2]=='F')
                {
                    ddd[0]=ddd[6];
                    ddd[1]=ddd[7];
                    ddd[2]='\0';
                }
                str1=str1+ddd;
            }
            //str1=str1+s;
        }
    }
    fclose(fs); // 2009.08.04

    str1.Delete(str1.Length()-2, 2);
    Data[0]=sULFileName+".zip";
    Data[1]=str1.Length();
    Data[2]=str1;
    Data[3]="0";
}
//---------------------------------------------------------------------------
int DoDLRequest(AnsiString sDLFileName)     //下載檔案,並切換工作檔
{
    // AI(W5-Final-Auto9045) 20260711: FAITHFUL translation of golden DoDLRequest
    // (auto9045.cpp:1924-2230). Same real-OS-side-effect caveat as DoULRequest
    // above (system() del/7z.exe/xcopy-style unzip, real CopyFile/ReadIniData/
    // WriteIniData against real DataPath/OffsetPath files) -- test harness does
    // NOT call this function, see translate report.
    SetCurrentDirectory("D://");                                               // _T() neutralized, see DoULRequest note above

    bool bNeedCover=false;

    AnsiString str1, str2;
    AnsiString sConfigPath=AuthPath+"Security_new.def";
    AnsiString SPath[2]={DataPath, OffsetPath};
    int ret=0;//, ret2=0;
    bool bHasSetUpFile=false;

    //----------------------
    //首先判斷工作檔有沒有
    //----------------------
    if(!DirectoryExists(SPath[0]+sDLFileName))
    {
        MyForceDirectories(SPath[0]+sDLFileName);
        bHasSetUpFile=false;
    }
    else
    {
        bHasSetUpFile=true;
    }

    if(FileExists("d:\\HT9045\\7z.exe")==false)
    {
        CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false);
    }

    if(bHasSetUpFile==false)                                                    //沒有的話,就先建立新的資料夾
    {
        for(int i=1; i<2; i++)
        {
            MyForceDirectories(SPath[i]+sDLFileName);
        }
        W5FA_FMain.cbSetupFileName->Items->Add(sDLFileName);                    // golden fMain->cbSetupFileName->Items->Add(...) -- TU-local gate
    }

    //----------------------
        //檢查Offset檔
    //----------------------
    str1.sprintf("%s%s\\Position Offset.Data", OffsetPath, sDLFileName);
    if(!FileExists(str1))    //如果沒有就從當下的工作檔複製過來
    {
        str2.sprintf("%s%s\\Position Offset.Data", OffsetPath, GetLastOpenFN());
        CopyFile(str2.c_str(), str1.c_str(), false);
    }

    str1.sprintf("\"%s%s\\Position Offset Hot.Data\"", OffsetPath, sDLFileName);
    if(!FileExists(str1))
    {
        str2.sprintf("\"%s%s\\Position Offset Hot.Data\"",  OffsetPath, GetLastOpenFN());
        CopyFile(str2.c_str(), str1.c_str(), false);
    }
    MySleep(50);

    //----------------------
    //把要還原的資料先備份
    //----------------------
    double fTempUserOffset[10], fContactHeight[19];
    int iShuttleMode[2];
    int iIndexHeatingMode;

    if(bHasSetUpFile)   //前提是該資料夾已經有資料了
    {
        //----------------------
        //把溫度Offset資料備份
        //----------------------
        str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName);
        for(int i=0; i<10; i++)
        {
            str2.printf("CH%d", i+1);
            fTempUserOffset[i]=ReadIniData(str1, "User OffSet", str2, 0.0);
        }
        iIndexHeatingMode=CheckAndReadIniData(str1, "Index",   "Heating Mode",   0);    //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式
        MySleep(50);

        //----------------------
        //把HotPlate資料備份
        //----------------------
        str1.sprintf("%s%sHotPlate.Data", DataPath, sDLFileName);
        str2.sprintf("%sHotPlate.Data", DataPath);
        ret=CopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(50);

        //----------------------
        //把Contact高度備份
        //----------------------
        str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
        fContactHeight[0]=ReadIniData(str1, "Test Arm1", "Pick Up", 0.0);
        fContactHeight[1]=ReadIniData(str1, "Test Arm1", "Contact", 0.0);
        fContactHeight[2]=ReadIniData(str1, "Test Arm1", "Drop",    0.0);
        fContactHeight[3]=ReadIniData(str1, "Test Arm1", "Place",   0.0);
        fContactHeight[4]=ReadIniData(str1, "Test Arm2", "Pick Up", 0.0);
        fContactHeight[5]=ReadIniData(str1, "Test Arm2", "Contact", 0.0);
        fContactHeight[6]=ReadIniData(str1, "Test Arm2", "Drop",    0.0);
        fContactHeight[7]=ReadIniData(str1, "Test Arm2", "Place",   0.0);
        fContactHeight[8]=ReadIniData(str1, "Torque Control", "Pin Number",    0.0);
        fContactHeight[9]=ReadIniData(str1, "Torque Control", "Force Per Pin", 0.0);

        fContactHeight[10]=ReadIniData(str1, "Wait Time", "Drop Wait",   1.0);
        fContactHeight[11]=ReadIniData(str1, "Wait Time", "Drop Speed",  1.0);
        fContactHeight[12]=ReadIniData(str1, "Mode", "Contact",                          0.0);
        fContactHeight[13]=ReadIniData(str1, "Mode", "Vacuum",                           0.0);
        fContactHeight[14]=ReadIniData(str1, "Mode", "Dummy Contact",                    0.0);
        fContactHeight[15]=ReadIniData(str1, "Mode", "Head Device Mode",                 0.0);
        fContactHeight[16]=ReadIniData(str1, "Mode", "Kit Diameter",                     3.0);
        fContactHeight[17]=ReadIniData(str1, "Mode", "Suck Shuttle Device After Tested", 0.0);
        fContactHeight[18]=ReadIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber", 0.0);

        //----------------------
        //把Ld/Uld速度備份
        //----------------------
        str1.sprintf("%s%s\\UdUld.Data", DataPath, sDLFileName);
        str2.sprintf("%sUdUld.Data", DataPath);
        ret=CopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(50);

        //----------------------
        //把速度設定備份
        //----------------------
        str1.sprintf("%s%s\\ArmCondition.Data", DataPath, sDLFileName);
        str2.sprintf("%s\\ArmCondition.Data", DataPath);
        ret=CopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(50);

        //----------------------
        //備份開啟單一Shuttle選項
        //----------------------
        str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName);
        iShuttleMode[0]=ReadIniData(str1, "Configuration", "Shuttle Mode",    0);
        iShuttleMode[1]=ReadIniData(str1, "Configuration", "Shuttle1 Cancel", 0);
    }

    //----------------------
    //解壓縮檔案並覆蓋到原本的資料夾
    //----------------------
    if(FileExists(DataPath+sDLFileName+".zip"))     //Steven 20110603
    {
        str1="d:\\HT9045\\7z.exe e \""+DataPath+sDLFileName+".zip\" -o\""+DataPath+sDLFileName+"\\\" -y";
        ret=system(str1.c_str());
        MySleep(100);
    }
    else
    {
        ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, sDLFileName);     //下載 %s.zip 失敗
        return false;
    }

    if(ret==0)  //解壓縮成功的話
    {
        //----------------------
        //把壓縮檔砍掉
        //----------------------
        str1.sprintf("del \"%s%s.zip\"", DataPath, sDLFileName);
        system(str1.c_str());
        MySleep(50);

        if(bHasSetUpFile) //如果不是新的工作檔就要進行資料還原
        {
            //----------------------
            //不覆蓋就是必須要還原
            // 0 : 不覆蓋
            // 1 : 要覆蓋
            //----------------------

            //----------------------
            //把溫度Offset資料還原
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Temp OffSet", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName);
                for(int i=0; i<10; i++)
                {
                    str2.printf("CH%d", i+1);
                    WriteIniData(str1, "User OffSet", str2, fTempUserOffset[i]);
                }
            }

            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Index Heat Mode", true);  //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式
            if(bNeedCover==false)
            {
                str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName);
                WriteIniData(str1, "Index",   "Heating Mode",   iIndexHeatingMode);
            }

            //----------------------
            //把Contact高度還原
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Test Arm", false);  //預設不覆蓋
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
                WriteIniData(str1, "Test Arm1", "Pick Up", fContactHeight[0]);
                WriteIniData(str1, "Test Arm1", "Contact", fContactHeight[1]);
                WriteIniData(str1, "Test Arm1", "Drop",    fContactHeight[2]);
                WriteIniData(str1, "Test Arm1", "Place",   fContactHeight[3]);
                WriteIniData(str1, "Test Arm2", "Pick Up", fContactHeight[4]);
                WriteIniData(str1, "Test Arm2", "Contact", fContactHeight[5]);
                WriteIniData(str1, "Test Arm2", "Drop",    fContactHeight[6]);
                WriteIniData(str1, "Test Arm2", "Place",   fContactHeight[7]);
            }

            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Torque Control", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
                WriteIniData(str1, "Torque Control", "Pin Number",    fContactHeight[8]);
                WriteIniData(str1, "Torque Control", "Force Per Pin", fContactHeight[9]);
            }

            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact Mode", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
                WriteIniData(str1, "Wait Time", "Drop Wait",                    fContactHeight[10]);
                WriteIniData(str1, "Wait Time", "Drop Speed",                   fContactHeight[11]);
                WriteIniData(str1, "Mode", "Contact",                           fContactHeight[12]);
                WriteIniData(str1, "Mode", "Vacuum",                            fContactHeight[13]);
                WriteIniData(str1, "Mode", "Dummy Contact",                     fContactHeight[14]);
                WriteIniData(str1, "Mode", "Head Device Mode",                  fContactHeight[15]);
                WriteIniData(str1, "Mode", "Kit Diameter",                      fContactHeight[16]);
                WriteIniData(str1, "Mode", "Suck Shuttle Device After Tested",  fContactHeight[17]);
                WriteIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber",  fContactHeight[18]);
            }

            //----------------------
            //把HotPlate資料還原
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "HotPlate", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%sHotPlate.Data", DataPath);
                str2.sprintf("%s%s\\HotPlate.Data", DataPath, sDLFileName);
                ret=CopyFile(str1.c_str(), str2.c_str(), false);
                MySleep(50);
            }

            str1.sprintf("del \"%sHotPlate.Data\"", DataPath);    //刪除備份
            system(str1.c_str());
            MySleep(50);

            //----------------------
            //把Ld/Uld速度還原
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Load Unload", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%sUdUld.Data", DataPath);
                str2.sprintf("%s%s\\UdUld.Data", DataPath, sDLFileName);
                ret=CopyFile(str1.c_str(), str2.c_str(), false);
                MySleep(50);
            }

            str1.sprintf("del \"%sUdUld.Data\"", DataPath);    //刪除備份
            system(str1.c_str());
            MySleep(50);

            //----------------------
            //把速度設定備份
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "ArmCondition", true);
            if(bNeedCover==false)   //不覆蓋就要還原
            {
                str1.sprintf("%sArmCondition.Data", DataPath);
                str2.sprintf("%s%s\\ArmCondition.Data", DataPath, sDLFileName);
                ret=CopyFile(str1.c_str(), str2.c_str(), false);
                MySleep(50);
            }

            str1.sprintf("del \"%sArmCondition.Data\"", DataPath); //刪除備份
            system(str1.c_str());
            MySleep(50);

            //----------------------
            //備份開啟單一Shuttle選項
            //----------------------
            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Shuttle Mode", true);
            if(bNeedCover==false)    //不覆蓋就要還原
            {
                str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName);
                WriteIniData(str1, "Configuration", "Shuttle Mode",    iShuttleMode[0]);
                WriteIniData(str1, "Configuration", "Shuttle1 Cancel", iShuttleMode[1]);
            }
        }

//        ShowErrorMessage("MES1687", 0, MMSystem, 0, sDLFileName); //UnZip %s.zip OK.
//        return true;
    }
    else
    {
        ShowErrorMessage("WAR1686", 0, MMSystem, 0, sDLFileName); //UnZip %s.zip Fail.
        return false;
    }

    bFTPDownloadSetupFile = true;       // 2013.12.03 , Joye , KYEC FTP Download

    //ChungHung 20110906 add
    if(ret==0)
        ret=1;
    else
        ret=0;

    return ret;
}
//---------------------------------------------------------------------------
#ifdef DEBUG_DUTONOFF
void DoHomeAndStart()
{
    bHomeByStart=true;
    fMain->Home("DoHomeAndStart");
}
#endif
//---------------------------------------------------------------------------
void DoOneCycle()
{
    InitOneCycle("DoOneCycle");
    fMain->BtnOneCycle->Down=true;                                             // REAL FormsFacade.h member
}
//---------------------------------------------------------------------------
void AmkorUDPGetSiteAndTemp()         //Steven 20120330 : ATK Site Map Monitorning
{
//1Byte         1Byte        1Byte          4Byte      N     1Byte
//Packet Count, Packet Type, Handler Model, Data Size, Data, Check Sum
//Packet Type Define : SITE MAP=0x01, TEMPERATURE=0x02
//Handler Model :
//Check Sum : Packet Count ~ Data N 使用 XOR

//Site MAP Data: Aa Site Num, Aa Enable/Disable, Ab Site Num... Ba Site Num, Ba Enable/Disable...
//Temperature Info Data: Temperature Mode(1Byte), Temperature(1Byte), Soak Time(2Byte)
//Temperature Mode: High=0x00, Ambient=0x01

    AnsiString SendString;
//    char DataTempMode;
    int BCC;

    int MaxI=0;
    int MaxJ=0;
    switch(TestIF.iTestMode)
    {
      case DualSite://1x2
           MaxI=1;
           MaxJ=2;
           break;
      case SingleSite://1x1
           MaxI=1;
           MaxJ=1;
           break;
      case QualSite1X4://1x4
      case _8Site1X4:  //ChungHung 20150528 add for 海思 _8Site1x4
           MaxI=1;
           MaxJ=4;
           break;
      case QualSite2X2://2x2
           MaxI=2;
           MaxJ=2;
           break;
      case _16Site4X4:  //Sam 20190226 : 16Site4X4
      case _8Site2X4://2x4
           MaxI=2;
           MaxJ=4;
           break;
      case _16Site2X8://2x8
           MaxI=2;
           MaxJ=8;
           break;
    };

//    if(LastSet.iTemperature==Tempture_Hot)
//        DataTempMode=0;
//    else
//        DataTempMode=1;

    W5FA_FAutomation.sATKSendMessage[ 0]=2;                            //count     =2       // golden fAutomation->sATKSendMessage[] -- TU-local gate
    W5FA_FAutomation.sATKSendMessage[ 1]=2;                            //Type      =Temperature
    W5FA_FAutomation.sATKSendMessage[ 2]=10;                           //Model     =HT9045WH
    W5FA_FAutomation.sATKSendMessage[ 3]=4;                            //DataSize  =4
    W5FA_FAutomation.sATKSendMessage[ 4]=0;
    W5FA_FAutomation.sATKSendMessage[ 5]=0;
    W5FA_FAutomation.sATKSendMessage[ 6]=0;
    W5FA_FAutomation.sATKSendMessage[ 7]=(LastSet.iTemperature==Tempture_Hot)?0:1;
    W5FA_FAutomation.sATKSendMessage[ 8]=(char)int(Temperature.fWorkTemperBase);
    W5FA_FAutomation.sATKSendMessage[ 9]=(char)(int(Temperature.fSoakTime)%256);
    W5FA_FAutomation.sATKSendMessage[10]=(char)(int(Temperature.fSoakTime)/256);
    W5FA_FAutomation.sATKSendMessage[11]=1;                            //Type      =SiteMap
    W5FA_FAutomation.sATKSendMessage[12]=10;                           //Model     =HT9045WH
    W5FA_FAutomation.sATKSendMessage[13]=(char)(MaxI*MaxJ*2);
    W5FA_FAutomation.sATKSendMessage[14]=0;
    W5FA_FAutomation.sATKSendMessage[15]=0;
    W5FA_FAutomation.sATKSendMessage[16]=0;
    W5FA_FAutomation.sATKSendMessageCount=17;
    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
            W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)TestIF_File.iSiteMap[i][j];
            W5FA_FAutomation.sATKSendMessageCount++;
            W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)LastSet.bUseTestSocket[0][i][j];
            W5FA_FAutomation.sATKSendMessageCount++;
        }
    }
    BCC=W5FA_FAutomation.sATKSendMessage[0];

    for(int i=1; i<W5FA_FAutomation.sATKSendMessageCount; i++)
    {
        BCC^=W5FA_FAutomation.sATKSendMessage[i];
    }
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)BCC;
    W5FA_FAutomation.sATKSendMessageCount++;
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]='\0';
}
//---------------------------------------------------------------------------
void AmkorUDPGetSite()         //Steven 20120330 : ATK Site Map Monitorning
{
//1Byte         1Byte        1Byte          4Byte      N     1Byte
//Packet Count, Packet Type, Handler Model, Data Size, Data, Check Sum
//Packet Type Define : SITE MAP=0x01, TEMPERATURE=0x02
//Handler Model :
//Check Sum : Packet Count ~ Data N 使用 XOR

//Site MAP Data: Aa Site Num, Aa Enable/Disable, Ab Site Num... Ba Site Num, Ba Enable/Disable...
//Temperature Info Data: Temperature Mode(1Byte), Temperature(1Byte), Soak Time(2Byte)
//Temperature Mode: High=0x00, Ambient=0x01

    AnsiString SendString;
//    char DataTempMode;
    int BCC;

    int MaxI=2;
    int MaxJ=8; //ChungHung 20140514 alter 4--->8
    /*
    switch(TestIF.iTestMode)
    {
      case DualSite://1x2
           MaxI=1;
           MaxJ=2;
           break;
      case SingleSite://1x1
           MaxI=1;
           MaxJ=1;
           break;
      case QualSite1X4://1x4
           MaxI=1;
           MaxJ=4;
           break;
      case QualSite2X2://2x2
           MaxI=2;
           MaxJ=2;
           break;
      case _8Site2X4://2x4
           MaxI=2;
           MaxJ=4;
           break;
      case _16Site2X8://2x8
           MaxI=2;
           MaxJ=8;
           break;
    };
    */
//    if(LastSet.iTemperature==Tempture_Hot)
//        DataTempMode=0;
//    else
//        DataTempMode=1;

    W5FA_FAutomation.sATKSendMessage[0]=1;                            //count     =1
    W5FA_FAutomation.sATKSendMessage[1]=1;                            //Type      =SiteMap
    W5FA_FAutomation.sATKSendMessage[2]=10;                           //Model     =HT9045WH
    W5FA_FAutomation.sATKSendMessage[3]=(char)(MaxI*MaxJ*2);
    W5FA_FAutomation.sATKSendMessage[4]=0;
    W5FA_FAutomation.sATKSendMessage[5]=0;
    W5FA_FAutomation.sATKSendMessage[6]=0;
    W5FA_FAutomation.sATKSendMessageCount=7;
    for(int i=0; i<MaxI; i++)
    {
        for(int j=0; j<MaxJ; j++)
        {
            if(TestIF_File.iSiteMap[i][j]!=-1)
            {
                W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)TestIF_File.iSiteMap[i][j];
                W5FA_FAutomation.sATKSendMessageCount++;
                W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)LastSet.bUseTestSocket[0][i][j];
                W5FA_FAutomation.sATKSendMessageCount++;
            }
            else
            {
                W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=0;
                W5FA_FAutomation.sATKSendMessageCount++;
                W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=0;
                W5FA_FAutomation.sATKSendMessageCount++;
            }
        }
    }
    BCC=W5FA_FAutomation.sATKSendMessage[0];

    for(int i=1; i<W5FA_FAutomation.sATKSendMessageCount; i++)
    {
        BCC^=W5FA_FAutomation.sATKSendMessage[i];
    }
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)BCC;
    W5FA_FAutomation.sATKSendMessageCount++;
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]='\0';
}
//---------------------------------------------------------------------------
void AmkorUDPGetTemp()         //Steven 20120330 : ATK Site Map Monitorning
{
//1Byte         1Byte        1Byte          4Byte      N     1Byte
//Packet Count, Packet Type, Handler Model, Data Size, Data, Check Sum
//Packet Type Define : SITE MAP=0x01, TEMPERATURE=0x02
//Handler Model :
//Check Sum : Packet Count ~ Data N 使用 XOR

//Site MAP Data: Aa Site Num, Aa Enable/Disable, Ab Site Num... Ba Site Num, Ba Enable/Disable...
//Temperature Info Data: Temperature Mode(1Byte), Temperature(1Byte), Soak Time(2Byte)
//Temperature Mode: High=0x00, Ambient=0x01

    AnsiString SendString;
//    char DataTempMode;
    int BCC;

//    if(LastSet.iTemperature==Tempture_Hot)
//        DataTempMode=0;
//    else
//        DataTempMode=1;

    W5FA_FAutomation.sATKSendMessage[ 0]=1;                            //count     =1
    W5FA_FAutomation.sATKSendMessage[ 1]=2;                            //Type      =Temperature
    W5FA_FAutomation.sATKSendMessage[ 2]=10;                           //Model     =HT9045WH
    W5FA_FAutomation.sATKSendMessage[ 3]=4;                            //DataSize  =4
    W5FA_FAutomation.sATKSendMessage[ 4]=0;
    W5FA_FAutomation.sATKSendMessage[ 5]=0;
    W5FA_FAutomation.sATKSendMessage[ 6]=0;
    W5FA_FAutomation.sATKSendMessage[ 7]=(LastSet.iTemperature==Tempture_Hot)?0:1;

    if(LastSet.iTemperature==Tempture_Hot)
    {
        W5FA_FAutomation.sATKSendMessage[ 8]=(char)int(Temperature.fWorkTemperBase);
        W5FA_FAutomation.sATKSendMessage[ 9]=(char)(int(Temperature.fSoakTime)%256);
        W5FA_FAutomation.sATKSendMessage[10]=(char)(int(Temperature.fSoakTime)/256);
        W5FA_FAutomation.sATKSendMessageCount=11;
    }
    else
    {
        W5FA_FAutomation.sATKSendMessage[ 8]=(char)int(Temperature.fAbitTemp);
        W5FA_FAutomation.sATKSendMessage[ 9]=0x00;
        W5FA_FAutomation.sATKSendMessage[10]=0x00;
        W5FA_FAutomation.sATKSendMessageCount=11;
    }

    BCC=W5FA_FAutomation.sATKSendMessage[0];

    for(int i=1; i<W5FA_FAutomation.sATKSendMessageCount; i++)
    {
        BCC^=W5FA_FAutomation.sATKSendMessage[i];
    }
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]=(char)BCC;
    W5FA_FAutomation.sATKSendMessageCount++;
    W5FA_FAutomation.sATKSendMessage[W5FA_FAutomation.sATKSendMessageCount]='\0';
}
//---------------------------------------------------------------------------
typedef std::pair<int,int> dPair;
// AI(W5-Final-Auto9045) 20260711: golden declares this comparator with
// external linkage (not `static`) -- preserved verbatim per the "faithful,
// don't fix" rule. FLAGGED (report Section 6): a generic non-static global
// named `SiteSort` is a symbol-collision risk if any other translated unit
// ever defines the same name; recommend reviewing whether to make it
// `static` when this is folded into the shared build.
bool SiteSort(dPair a,dPair b)
{
    return a.first<b.first;
}
//---------------------------------------------------------------------------
AnsiString GetTestResult_ASE_KR(int row,int col) //ChungHung 20120104 add
{
    dPair temp;
    std::vector<dPair> vBinData;
    vBinData.clear();
    for(int i=0; i<row; i++)
    {
        for(int j=0; j<col; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC)
            {
                temp.first=TestIF.iSiteMap[i][j];
                temp.second=TestSocket.Item[i][j]-TEST_PASS+1;
            }
            else
            {
                temp.first=TestIF.iSiteMap[i][j];
                temp.second=0;
            }

            vBinData.push_back(temp);
        }
    }
    sort(vBinData.begin(),vBinData.end(),SiteSort);
    AnsiString Data="",SubData="";
    Data.sprintf("%c%c",0x01,row*col);
    AnsiString aSiteNum;
    AnsiString aBinResult;
    for(int i=0; i<row*col; i++)
    {
        temp=vBinData[i];
        aSiteNum=temp.first;
        aBinResult=temp.second;
        SubData.sprintf("%c%c%c%c%s%c%c%s",0x01,0x02,0x41,aSiteNum.Length(),aSiteNum,0x41,aBinResult.Length(),aBinResult);
        Data+=SubData;
    }
    return Data;
}
//---------------------------------------------------------------------------
int GetTotalSite_ASE_KR(int &row,int &col) //ChungHung 20120104 add
{
    int ret=0;
    switch(TestIF.iTestMode)
    {
        case SingleSite:
            row=1;
            col=1;
            ret=1;
            break;
        case DualSite:
            row=1;
            col=2;
            ret=2;
            break;
//        case DualSiteBS:
//            row=1;
//            col=2;
//            ret=2;
//            break;
        case TriSite1X3:
            row=1;
            col=3;
            ret=3;
            break;
        case QualSite1X4:
        case _8Site1X4: //ChungHung 20150528 add for 海思 _8Site1x4
            row=1;
            col=4;
            ret=4;
            break;
        case DualSite2x1:
            row=2;
            col=1;
            ret=2;
            break;
        case QualSite2X2:
        case QualSite2X2N:     //Frank 20200520 2X2NN Mode
            row=2;
            col=2;
            ret=4;
            break;
//        case QualSite2X2BS:
//            row=2;
//            col=2;
//            ret=4;
//            break;
        case _6Site2X3:  //ChungHung 20140115 add for 2x3_6
        case _6Site2X3N: //Steven 20220425 : 2X3NN Mode
            row=2;
            col=3;
            ret=6;
            break;
        case _8Site2X4:
        case _8Site2X4N: //Wei 20231211 : 2X4NN Mode
            row=2;
            col=4;
            ret=8;
            break;
        case _10Site2X5:    //wei 20190614 10 site
            row=2;
            col=5;
            ret=10;
            break;
        case _12Site2X6:
            row=2;
            col=6;
            ret=12;
            break;
        case _16Site2X8:
            row=2;
            col=8;
            ret=16;
            break;
        case _16Site4X4:  //Sam 20190226 : 16Site4X4
            row=4;
            col=4;
            ret=16;
            break;
        case _32Site4X8N:    //單Arm 32 Site
            row=4;
            col=8;
            ret=32;
            break;
        case _32Site4X8M:    //雙Arm 32 Site
            row=4;
            col=8;
            ret=32;
            break;
    }
    return ret;
}
//---------------------------------------------------------------------------
int GetLowYield(AnsiString *Data)
{
    Data[0]=(TestIF_File.bFailAlarmLowYield)?1:0;
    Data[1]=AnsiString(TestIF_File.iLowYieldLimit);
    Data[2]=AnsiString(TestIF_File.iLowYieldCount);
    return 3;
}
//---------------------------------------------------------------------------
int GetByArmPerSiteDiffYield(AnsiString *Data)
{
    Data[0]=(TestIF_File.bFailAlarmSiteYieldDifferent)?1:0;
    Data[1]=AnsiString(TestIF_File.iFailAlarmSiteYield);
    Data[2]=AnsiString(TestIF_File.iFailAlarmSiteYieldDifferentCount);
    return 3;
}
//---------------------------------------------------------------------------
int GetConsecutiveFailureAlarmByHead(AnsiString *Data)
{
    Data[0]=(TestIF_File.bContsFailByHead)?1:0;
    Data[1]=AnsiString(TestIF_File.iContsFailHeadAlarmCT);
    return 2;
}
//---------------------------------------------------------------------------
int GetConsecutiveFailureAlarmBySocket(AnsiString *Data)
{
    Data[0]=(TestIF_File.bContsFailBySocket)?1:0;
    Data[1]=AnsiString(TestIF_File.iContsFailSocketAlarmCT);
    return 2;
}
//---------------------------------------------------------------------------
int GetAllSiteFailFor9045(AnsiString *Data)
{
    Data[0]=(TestIF_File.bAllSiteFail)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetTrayFormTypeInfo(AnsiString *Data,int iType)
{
    if(iType>=1)
    {
        Data[0]=AnsiString(UserDefForm_File[iType-1].XDivision);
        Data[1]=AnsiString(UserDefForm_File[iType-1].YDivision);
        Data[2]=AnsiString(UserDefForm_File[iType-1].XStart);
        Data[3]=AnsiString(UserDefForm_File[iType-1].YStart);
        Data[4]=AnsiString(UserDefForm_File[iType-1].XPitch);
        Data[5]=AnsiString(UserDefForm_File[iType-1].YPitch);
        return 6;
    }
    return 0;
}
//---------------------------------------------------------------------------
int GetTrayFormTypeThickness(AnsiString *Data,int iType)
{
    if(iType>=1)
    {
        Data[0]=AnsiString(UserDefForm_File[iType-1].ZDepth);
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int GetTrayFormTypePickUp(AnsiString *Data,int iType)
{
    if(iType>=1)
    {
        Data[0]=AnsiString(UserDefForm_File[iType-1].iPickUp);
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int GetTrayFormTypeName(AnsiString *Data,int iType)
{
    if(iType>=1)
    {
        Data[0]=AnsiString(UserDefForm_File[iType-1].Alias);
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int GetContactModeFor9045(AnsiString *Data)
{
    Data[0]=DeviceForm_File.ContactMode;
    return 1;
}
//---------------------------------------------------------------------------
int GetContactVacuumMode(AnsiString *Data)
{
    Data[0]=DeviceForm_File.VacuumMode;
    return 1;
}
//---------------------------------------------------------------------------
int GetContactDropWait(AnsiString *Data)
{
    Data[0]=AnsiString(DeviceForm_File.DropWait);
    return 1;
}
//---------------------------------------------------------------------------
int GetSlowContactSpeed(AnsiString *Data)
{
    Data[0]=DeviceForm_File.DropSpeed;
    return 1;
}
//---------------------------------------------------------------------------
int GetShuttleWaitOutSideCamber(AnsiString *Data)
{
    Data[0]=(DeviceForm_File.bShuttleWaitingOutSiteChamber)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetPickShuttleDeviceAfterTested(AnsiString *Data)
{
    Data[0]=(DeviceForm_File.bSuckShuttleDeviceAfterTested)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetPickShuttleDeviceThenWaitOnShuttle(AnsiString *Data)
{
    Data[0]=(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetPickShuttleDeviceTogetherFor32SiteN(AnsiString *Data)
{
    Data[0]=(DeviceForm_File.bPickShuttleDeviceTogether)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetIndexArm1Height(AnsiString *Data)
{
    Data[0]=AnsiString(DeviceForm_File.IndexPlace[0]);
    Data[1]=AnsiString(DeviceForm_File.IndexArmPick[0]);
    Data[2]=AnsiString(DeviceForm_File.IndexDrop[0]);
    Data[3]=AnsiString(DeviceForm_File.IndexContact[0]);
    return 4;
}
//---------------------------------------------------------------------------
int GetIndexArm2Height(AnsiString *Data)
{
    Data[0]=AnsiString(DeviceForm_File.IndexPlace[1]);
    Data[1]=AnsiString(DeviceForm_File.IndexArmPick[1]);
    Data[2]=AnsiString(DeviceForm_File.IndexDrop[1]);
    Data[3]=AnsiString(DeviceForm_File.IndexContact[1]);
    return 4;
}
//---------------------------------------------------------------------------
int GetTestICCheckMode(AnsiString *Data)
{
    Data[0]=DeviceForm_File.iSocketInitialICCheckPosition;
    return 1;
}
//---------------------------------------------------------------------------
int GetAboveSocket(AnsiString *Data)
{
    Data[0]=AnsiString(DeviceForm_File.fSocketInitialICCheckPositionOffset);
    return 1;
}
//---------------------------------------------------------------------------
int GetContactForceInfo(AnsiString *Data)
{
    Data[ 0]=AnsiString(DeviceForm_File.dPress);
    Data[ 1]=W5FA_FContact.edAirForceN->Text;                                  // golden fContact->edAirForceN->Text -- TU-local gate
    Data[ 2]=W5FA_FContact.edAirKPA->Text;                                     // golden fContact->edAirKPA->Text -- TU-local gate
    Data[ 3]=W5FA_FContact.edSetKg->Text;                                      // golden fContact->edSetKg->Text -- TU-local gate
    Data[ 4]=DeviceForm_File.iHeadDeviceCT;
    Data[ 5]=W5FA_FContact.rgOutKitDiameter_ItemIndex;                         // golden fContact->rgOutKitDiameter->ItemIndex -- TU-local gate
    Data[ 6]=W5FA_FContact.edForcePerDeviceKG->Text;
    Data[ 7]=W5FA_FContact.edForcePerDeviceN->Text;
    Data[ 8]=DeviceForm_File.iPinCT;
    Data[ 9]=AnsiString(DeviceForm_File.ForcePerPinN);
    Data[10]=AnsiString(DeviceForm_File.ForcePerPinG);
    return 11;
}
//---------------------------------------------------------------------------
int GetHotPlateFormName(AnsiString *Data)
{
    Data[0]=HotPlateForm_File.Alias;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotPlate1(AnsiString *Data)
{
    Data[0]=(HotPlateForm_File.iPlateSelect&0x01)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotPlate2(AnsiString *Data)
{
    Data[0]=(HotPlateForm_File.iPlateSelect&0x02)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetInterfaceType(AnsiString *Data)
{
    Data[0]=TestIF_File.iTestType;
    return 1;
}
//---------------------------------------------------------------------------
int GetTesterInitialMaximumTest(AnsiString *Data)
{
    Data[0]=AnsiString(TestIF_File.iInitialMaxTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetTesterMaximumTest(AnsiString *Data)
{
    Data[0]=AnsiString(TestIF_File.iMaxTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetTesterDummyTest(AnsiString *Data)
{
    Data[0]=AnsiString(TestIF_File.iDummyTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetTesterStartDelay(AnsiString *Data)
{
    Data[0]=AnsiString(TestIF_File.dStartDelayTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetHotSoakTime(AnsiString *Data)
{
    Data[0]=AnsiString(Temperature.fSoakTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetHotJamSoakTime(AnsiString *Data)
{
    Data[0]=AnsiString(Temperature.fJamSoakTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetHotInitialWaitTime(AnsiString *Data)
{
    Data[0]=AnsiString(Temperature.fInitialWaitTime);
    return 1;
}
//---------------------------------------------------------------------------
int GetHotInitialStart1Time(AnsiString *Data)
{
    Data[0]=Temperature.iInitialStart1Time;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotShuttleSoakTime(AnsiString *Data)
{
    Data[0]=Temperature.iInitialStart2Time;
    return 1;
}
//---------------------------------------------------------------------------
int GetChamberCoolingTemp(AnsiString *Data)
{
    Data[0]=AnsiString(Temperature.fChamberCoolTemp);
    return 1;
}
//---------------------------------------------------------------------------
int GetHotIndexSoakTime(AnsiString *Data)
{
    Data[0]=Temperature.iIndexSoakTime;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotOpenShortTime(AnsiString *Data)
{
    Data[0]=Temperature.iOSTime;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotZ1Down(AnsiString *Data)
{
    Data[0]=(Temperature.bZ2DownSocket)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetHotShuttleSoakMode(AnsiString *Data)
{
    Data[0]=Temperature.iShuttleSoakTimeMode;
    return 1;
}
//---------------------------------------------------------------------------
int GetMachineTempMode(AnsiString *Data)
{
    Data[0]=Temperature.iMachineTempMode;
    return 1;
}
//---------------------------------------------------------------------------
int GetAmbientCheck(AnsiString *Data)
{
    Data[0]=(Temperature.bUseAbitCHK)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetAmbientCheckTemp(AnsiString *Data)
{
    Data[0]=AnsiString(FormatFloat("0.0", Temperature.fAbitTemp));
    return 1;
}
//---------------------------------------------------------------------------
int GetTemperatureOffset(AnsiString *Data)
{
    Data[0]=(Temperature.bEnableTempOffsetForInitial)?1:0;
    return 1;
}
//---------------------------------------------------------------------------
int GetContactCountForOffsetPeriod(AnsiString *Data)
{
    Data[0]=Temperature.iCintactCntForTempOffsetAtInitial;
    return 1;
}
//---------------------------------------------------------------------------
int GetContactCountForCoolDown(AnsiString *Data)
{
    Data[0]=Temperature.iCintactDelayCntForInitTempOffset;
    return 1;
}
//---------------------------------------------------------------------------
int GetInterfaceDIOInfo(AnsiString *Data)
{
    Data[0]=TestIF_File.iDioMode;
    return 1;
}
//---------------------------------------------------------------------------
int GetInterfaceGPIBInfo(AnsiString *Data)
{
    Data[0]=TestIF_File.iGpibMode;
    Data[1]=TestIF_File.iGpibAddress;
    return 2;
}
//---------------------------------------------------------------------------
int GetInterfaceRS232Info(AnsiString *Data)
{
    Data[0]=TestIF_File.iRs232Mode;
    //Data[1]=(TestIF_File.bRS232NeedSendVSOT)?1:0;
    Data[1]=0;
    Data[2]=TestIF_File.Rs232_Data.Baud_Rate;
    Data[3]=TestIF_File.Rs232_Data.Bit_Length;
    Data[4]=TestIF_File.Rs232_Data.Parity;
    Data[5]=TestIF_File.Rs232_Data.Stop_Bit;

    return 6;
}
//---------------------------------------------------------------------------
int GetLotInfo(AnsiString *Data)
{
    Data[0]=fLotInfo->edtSysLotID->Text.c_str();                              // REAL FormsFacade.h member
    Data[1]=W5FA_FLotInfo.edtSysOperatorID->Text.c_str();                     // golden fLotInfo->edtSysOperatorID->Text -- TU-local gate
    if(fLotInfo->cbRunMode->Text=="Normal")                                   // REAL FormsFacade.h member
    {
        Data[2]="0";
    }
    else
    {
        Data[2]="1";
    }
    return 3;
}
//---------------------------------------------------------------------------
int GetIP(AnsiString *Data)
{
    Data[0]=IniConfig.sN08OlpIP;
    return 1;
}
//---------------------------------------------------------------------------
int GetPort(AnsiString *Data)
{
    Data[0]=IniConfig.sN08OlpPort;
    return 1;
}
//---------------------------------------------------------------------------
int GetCusCode(AnsiString *Data)
{
    Data[0]=IntToStr(CUSTOMER_CODE);
    return 1;
}
//---------------------------------------------------------------------------
int GetStartMode(AnsiString *Data)  //Sam 20221212 : 新增 StartMode 指令
{
    Data[0]=IntToStr(LastSet.iRunStartMode);
    return 1;
}
//---------------------------------------------------------------------------
int GetLotTotal(AnsiString *Data)  //Sam 20230803 : 新增OLP指令     //QQQ (pre-existing golden marker, preserved verbatim -- golden authors' own second-look flag, not resolved here)
{
    Data[0]=IntToStr(LastSet.SendCT[0]);
    Data[1]=IntToStr(W5FA_LS_iJamCount[1]);                                    // golden LastSet.iJamCount[1] -- TU-local gate (see gate #5)
    Data[2]=IntToStr(RunInfo.iUnloadCount);
    for(int i=0; i<iTestBinCount; i++)
    {
        Data[3+i]=IntToStr(LastSet.iBinData32[0][i]);
    }
    return 3+iTestBinCount;
}
