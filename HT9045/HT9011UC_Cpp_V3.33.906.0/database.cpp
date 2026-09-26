// ===========================================================================
//  database.cpp  -- PARTIAL translation (W3-cont2: config-table loaders batch)
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\database.cpp
//
//  BCB6 preamble (dropped per migration plan):
//    #include "MachineDefine.h" / #pragma hdrstop / #pragma package(smart_init)
//    / #pragma resource "*.dfm"  -- none carry portable meaning.
//
//  WHAT IS TRANSLATED (active code):
//    SYSTEM_MODULAR::LoadIoData()          (database.cpp:1548-1614)
//    SYSTEM_MODULAR::LoadMotData()         (database.cpp:1616-1682)
//    TIODATA::TIODATA(AnsiString)          (database.cpp:1756-1903)
//    TIOTABLENO::TIOTABLENO()              (database.cpp:1905-1923)
//    TIOTABLENO::SetIOTableNo(AnsiString)  (database.cpp:1925-2045)
//    TMOTNO::TMOTNO()                      (database.cpp:2047-2079)
//    TMOTNO::SetMOTTableNo(AnsiString)     (database.cpp:2081-2298)
//    TMOTDATA::TMOTDATA(AnsiString)        (database.cpp:2300-2874)
//    SYSTEM_MODULAR::SystemModularInitial()  (database.cpp:1539-1546, real as
//      of W906-SysModWire -- wires MyGem via SECSGEM/uHGemHT9045.h's
//      HT9045Gem class (permanent home as of W906-uHGemHT9045-Bucket0,
//      20260721; zero of golden's 22 overrides declared yet); NUMBER_PANEL_TYPE
//      branch stays gated, see below)
//    SYSTEM_MODULAR::ReadGeneralIni()      (database.cpp:301-1537, ~1240 lines,
//      real as of GA-1-B6, 20260804) -- reads Gerneral.ini into the full
//      SYSTEM_MODULAR member surface + cmydef.h globals.  A handful of
//      sub-blocks stay #if 0-gated (TTLRS232VerCheck / COMMSPEED_20M /
//      Application->MessageBox / CustomerFunctionSelect+ReadLastSetIni+
//      ReadEventLogAutoSaveInfo call-sites) -- see that function's own
//      file-head note and _ga1_b6_report.md for the full reconciliation.
//
//  WHAT IS GATED (#if 0 // TODO(wave) / TODO(GA1-B6)):
//    TDataModule1 ctor / DataModule1 global (database.cpp:24, 33-41)
//      -> BDE TTable / TDataModule; deferred to BDE wave.
//    SYSTEM_MODULAR ctor/dtor + InstallColorBinDisplay
//      (database.cpp:43-288, 1684-1729, 1731-1754)
//      -> ctor calls ReadGeneralIni + SystemModularInitial (both now real,
//         but the ctor ITSELF that would call them stays gated -- tests
//         invoke ReadGeneralIni/SystemModularInitial directly);
//         InstallColorBinDisplay is REAL since FW-BINDISP1 (20260824, Offline
//         DEVIATION per user ruling); its ctor-path CALL stays gated.
//    LogSoftwareOnTime / SoftwareExeTimer globals (database.cpp:28-31, 2876-2917)
//      -> #ifdef DEBUG_SOFTWARE_EXEC_TIME, effectively no-op; stub provided.
//    Inside ReadGeneralIni (GA-1-B6, see that function's own comment):
//      CustomerFunctionSelect()/ReadLastSetIni()/ReadEventLogAutoSaveInfo() call-sites
//      (bodies gated in cprod.cpp's own TODO(W6)); TTLRS232VerCheck (MessageDef.h/.cpp
//      untranslated); MOTIONNET_SPEED default COMMSPEED_20M (Motor/mn200.h untranslated);
//      Application->MessageBox interactive prompt (no vclcompat TApplication surface).
//
//  CROSS-HEADER DEPENDENCIES (active in this batch):
//    MachineType.h  -> enum eIOType {eMotionNet=0, eISABase=1, ePCI1735U=2,
//                      ePCI1203=3, ePLCbase=4}  (MachineType.h:717-722); also
//                      pulled in directly by database.h now (iTotalFunction).
//    common.h:67-68 -> extern AnsiString IoTablePath; MotTablePath;
//    common.h       -> asGeneralPath / CheckAndReadIniData / CheckAndReadIniDataGeneral /
//                      WriteIniDataGeneral / CheckIniData / MyForceDirectories (ReadGeneralIni)
//    cmydef.h:2977  -> extern int INDEX_MOTION_CARD; (plus the ~280 other
//                      System/TempCtrl/ATC/... globals ReadGeneralIni assigns)
//    cMyDB.h:20     -> void __fastcall MyDBIProcess(AnsiString,AnsiString,AnsiString="")
//    mymessbox.h:58 -> void ShowMyMessage(AnsiString,AnsiString="",...)
//    vclcompat       -> HexStrToInt(char*) and HexStrToInt(AnsiString) overloads
//
//  FAITHFULNESS NOTES:
//    * The copy-paste bug at database.cpp:1671 (LoadMotData catch block logs
//      "SYSTEM_MODULAR::LoadIoData" instead of "LoadMotData") is PRESERVED.
//    * SetMOTTableNo: the `//    else` comment-out at database.cpp:2091 that
//      causes the name-scan block to run unconditionally (even on Count mismatch)
//      is PRESERVED -- see comment at that location.
//    * Port HEX-parse rule for TIODATA: HexStrToInt(...c_str()) (database.cpp:1828).
//    * Port HEX-parse rule for TMOTDATA MC88X1: HexStrToInt(SL->Strings[...])
//      WITHOUT .c_str() (database.cpp:2419) -- different overload, preserved.
//    * TIODATA::iInType defaults to 0 (not -1) when empty (database.cpp:1845).
//      In the Count<14 fallback path iInType=-1 (database.cpp:1892).
//    * TMOTDATA bHasNullData -> iEnable=0 at end of valid-row path (database.cpp:2835-2838).
// ===========================================================================

#include "vclcompat/vcl_compat.h"   // replaces <vcl.h>; AnsiString/TStringList/FileExists/...
#include "database.h"
#include "MachineType.h"            // enum eIOType (eMotionNet/eISABase/ePCI1735U/ePLCbase)
#include "common.h"                 // extern AnsiString IoTablePath / MotTablePath (common.h:67-68)
#include "cmydef.h"                 // extern int INDEX_MOTION_CARD (cmydef.h:2977)
// AI(W906-uHGemHT9045-Bucket0) 20260721: SystemModularInitial's
// `new HT9045Gem(...)` needs the complete HT9045Gem type + extern HGem;
// swapped from the temporary uHGemHT9045_Shim.h (W906-SysModWire, deleted
// this wave) to the permanent SECSGEM/uHGemHT9045.h -- see that header's own
// file-head note for scope (still zero of golden's 22 overrides).
#include "SECSGEM/uHGemHT9045.h"
#include "BinDisplay/MyBinDisp.h"   // AI(W906-FW-BINDISP1) 20260824: TMyBinDispOffline for InstallColorBinDisplay

// ---------------------------------------------------------------------------
//  MyDBIProcess / ShowMyMessage forward declarations
//  cMyDB.h and mymessbox.h pull in VCL headers that are not yet portable.
//  For the loader batch we declare only the signatures we call, matching
//  cMyDB.h:20 and mymessbox.h:58 exactly.
// ---------------------------------------------------------------------------
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2="");
void ShowMyMessage(AnsiString S1, AnsiString S2="", AnsiString S3="", bool Ok=false, bool bServoOff=false);

// ---------------------------------------------------------------------------
//  Global instances  (database.cpp:27)
// ---------------------------------------------------------------------------
SYSTEM_MODULAR HSys;    // global system parameter aggregate

// ---------------------------------------------------------------------------
//  LogSoftwareOnTime stub  (database.cpp:2876-2917)
//  Original body is entirely inside #ifdef DEBUG_SOFTWARE_EXEC_TIME (off in
//  normal builds).  Provide an empty stub so the loader ctor-chain compiles.
// ---------------------------------------------------------------------------
void LogSoftwareOnTime(AnsiString /*Flag*/)
{
#ifdef DEBUG_SOFTWARE_EXEC_TIME
    // TODO(wave): translate TMyStringList / TQPF_Timer log path when needed.
#endif
}

// ---------------------------------------------------------------------------
//  #if 0: TDataModule1 global + ctor  (database.cpp:24, 33-41)
//  Needs TDataModule / TTable from BDE (<Db.hpp> / <DBTables.hpp>).
// ---------------------------------------------------------------------------
#if 0 // TODO(wave-BDE): TDataModule1 ctor -- database.cpp:33-41
TDataModule1 *DataModule1;

__fastcall TDataModule1::TDataModule1(TComponent* Owner)
    : TDataModule(Owner)
{
    CylinderTable->DatabaseName=ALIAS;
    SwitchTable->DatabaseName=ALIAS;
    MotorTable->DatabaseName=ALIAS;
    SensorTable->DatabaseName=ALIAS;
    SuckerTable->DatabaseName=ALIAS;
}
#endif // TODO(wave-BDE)

// ---------------------------------------------------------------------------
//  #if 0: SYSTEM_MODULAR ctor  (database.cpp:43-288)
//  Calls ReadGeneralIni() + SystemModularInitial() + mapATCAlarmCode seeding.
//  Deferred: ReadGeneralIni needs ~1240-line cmydef/cprod surface; ctor also
//  calls OpenGeneralIniFile / InitialMemory / InitCommonString (common.cpp).
// ---------------------------------------------------------------------------
#if 0 // TODO(wave): SYSTEM_MODULAR ctor -- database.cpp:43-288
_fastcall SYSTEM_MODULAR::SYSTEM_MODULAR()
{
    LogSoftwareOnTime("SYSTEM_MODULAR, Start");
    SysTimer.LatchCycleTime(true);
    // InitialMemory();
    // InitCommonString();
    // OpenGeneralIniFile();
    BinDisCtrl = NULL;
    MyGem      = NULL;
    // ATKRecipeInfo = new ATK_RECIPE_INFO();
    // ReadGeneralIni();
    // SystemModularInitial();   // AI(W906-SysModWire) 20260720: this callee is now real -- see below; this ctor itself stays gated
    // AddSpace(asGeneralPath);
    mapATCAlarmCode.clear();
    // ... mapATCAlarmCode["ALM001"] = "WAR15200"; ... (database.cpp:58-288)
}
#endif // TODO(wave)

// ---------------------------------------------------------------------------
//  #if 0: SYSTEM_MODULAR dtor  (database.cpp:1731-1754)
// ---------------------------------------------------------------------------
#if 0 // TODO(wave): SYSTEM_MODULAR dtor -- database.cpp:1731-1754
_fastcall SYSTEM_MODULAR::~SYSTEM_MODULAR()
{
    try
    {
        // delete BinDisCtrl; delete ATKRecipeInfo; CloseGeneralIniFile();
        // ...
    }
    catch(...)
    {
    }
}
#endif // TODO(wave)

// ---------------------------------------------------------------------------
//  SYSTEM_MODULAR::SystemModularInitial  (golden database.cpp:1539-1546)
//  Creates HT9045Gem (SECS) + InstallColorBinDisplay.
//
// AI(W906-SysModWire) 20260720: golden calls this once, from the
// (still-gated in this port) SYSTEM_MODULAR ctor (golden database.cpp:54);
// tests invoke it directly instead. Calling it twice leaks the first MyGem --
// golden has the identical property (single ctor call site, no guard, no
// re-entry protection) -- deliberately NOT guarded here either. Deletion
// lives in untranslated main.cpp:11480-11481 (`delete HSys.MyGem;
// HSys.MyGem=NULL;`) -- callers/tests must do the same in their own teardown.
// ---------------------------------------------------------------------------
void SYSTEM_MODULAR::SystemModularInitial()
{
    MyGem=new HT9045Gem("HT9045", HGem);                                        //20140213  wei   KYEC SECS/GEM
    // AI(W906-FW-BINDISP2) 20260824: UN-GATED (behavior wave). The body is
    // real since FW-BINDISP1 (Offline DEVIATION per user ruling 20260824).
    // Safety chain for opening this: NUMBER_PANEL_TYPE defaults 0 offline
    // (cmydef.cpp) so the branch is dead on this dev machine and in every
    // test that never runs ReadGeneralIni; when it IS 3/4 (real HT9050
    // config), the seed-writing CheckAndReadIniDataGeneral calls inside the
    // body are golden's own behavior, and INIFileGeneral is necessarily open
    // by then (only ReadGeneralIni can have set NUMBER_PANEL_TYPE to 3/4).
    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604: new BinDisplay TFT
        InstallColorBinDisplay(NUMBER_PANEL_TYPE);
}

// ---------------------------------------------------------------------------
//  SYSTEM_MODULAR::InstallColorBinDisplay  (golden database.cpp:1684-1729)
//
//  AI(W906-FW-BINDISP1) 20260824: translation wave FW-BINDISP1. DEVIATION
//  (user ruling 20260824): golden :1686 news TMyBinDispHT9046 -- the hardware
//  protocol subclass is untranslated, and the user confirmed the experimental
//  machine HAS the color bin display panel but ruled the port instantiates
//  TMyBinDispOffline (the no-op concrete subclass, BinDisplay/MyBinDisp.h:403)
//  for now so the "BinDisCtrl is a real instance" runtime premise holds
//  (MN200 GATE c/e and the NUMBER_PANEL_TYPE 3/4 deref family wait on it).
//  TMyBinDispHT9046 translation + real-panel bring-up are future waves.
//  Everything below the new is golden verbatim.
//
//  GOLDEN ODDITY (ICBD-1): golden allocates BEFORE the iType check -- a
//  non-3/4 caller would leak the instance via the early return (:1694-1697),
//  and the `BinDisCtrl==NULL` test right after `new` never fires in standard
//  C++ (operator new throws). Both are unreachable in practice: the only
//  call site (:1543-1545) is itself guarded by NUMBER_PANEL_TYPE==3||4.
//  Preserved verbatim, not fixed.
// ---------------------------------------------------------------------------
void SYSTEM_MODULAR::InstallColorBinDisplay(int iType)
{
    BinDisCtrl=new TMyBinDispOffline;                                           //AI(W906-FW-BINDISP1) 20260824: DEVIATION, golden news TMyBinDispHT9046 (see banner)

    if(BinDisCtrl==NULL)                                                        //Sam 20240604 : 新增 BinDisplay TFT
        return;

    if(iType==3 || iType==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {
    }
    else
    {
        return;
    }
    BinDisCtrl->SetComPort(sNumberPanelComPort);
    BinDisCtrl->SetComPort2(sNumberPanelComPort2);

    for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal  //GG Steven
    {
        if(AUTO3_IS_MAGAZINE==0 && i>=eBinDispMag1 && i<=eBinDispMag14)         //JerryYang 20230515 : 沒裝Magazine就不要顯示
        {
        }
        else if(AUTO_EMPTY_COLOR<3 && i>=eBinDispAuto4 && i<=eBinDispFix12)
        {
        }
        else if(i==eBinDispBulkBox)
        {
        }
        else
        {
            BinDisCtrl->InstalledUnit(i);
        }
        BinDisCtrl->Alias[i]=asTrayForBinDisp[i];
    }

    int autoempty =CheckAndReadIniDataGeneral("System", "AUTO_EMPTY_COLOR", 1);
    int EmptyEmpty=CheckAndReadIniDataGeneral("System", "SUPPORT_2_EMPTY_EMPTY", 0);

    if(autoempty==0 && EmptyEmpty==0)
    {
        BinDisCtrl->CloseUnit(1);
        BinDisCtrl->CloseUnit(2);
    }

    BinDisCtrl->SetDelayTime(dNumberPanelDelay);                                //Sam 20240604 : 顯示器輪巡時間改為 double
}

// ---------------------------------------------------------------------------
//  SYSTEM_MODULAR::ReadGeneralIni()  (golden database.cpp:301-1537, 1237 lines)
//
//  ACTIVE as of GA-1-B6.  Faithful 1:1 translation: every key name, group
//  name, default value and read order is preserved verbatim; only the
//  Chinese prose in comments is translated to English (matches this file's
//  own established convention -- see file-head banner).
//
//  GATED SUB-BLOCKS (range-clause: do not add fields to shared headers;
//  gate instead, report to the main loop):
//    1. CustomerFunctionSelect() / ReadLastSetIni() / ReadEventLogAutoSaveInfo()
//       call-sites -- all three are DECLARED in cprod.h (active) but their
//       BODIES sit inside cprod.cpp's own file-wide
//       "#if 0 // TODO(W6): function bodies depend on untranslated state
//       machines + globals" gate (cprod.cpp:184-4036).  Calling them here
//       would compile but fail to LINK.  Golden's call order preserved as
//       gated statements (not deleted) so a future wave can un-gate by
//       simply removing the #if 0/#endif once cprod.cpp's W6 wave lands.
//    2. TTLRS232VerCheck (golden database.cpp:1097) -- declared in golden's
//       MessageDef.h/.cpp, which has NO ported counterpart at all yet (the
//       whole module is untranslated, not merely gated inside an existing
//       ported file).  Not a "shared header" per the range clause (Config.h/
//       cmydef.h/cpublic.h/cprod.h/CosFunction.h/canary_support.h) -- it is
//       simply missing.  Gated; TTLRS232VerCheck stays whatever it already
//       is (uninitialized until MessageDef wave lands elsewhere).
//    3. MOTIONNET_SPEED's default value COMMSPEED_20M (golden database.cpp:1083)
//       -- #define lives in golden's Motor/mn200.h, a driver header on the
//       AGENTS.md read-only "definition file" list that has NOT been ported
//       into this tree at all (no Motor/mn200.h here yet).  Gated rather than
//       hardcoding the literal 3 (which would silently desync from golden if
//       mn200.h's value ever changed) or adding a new file (out of this
//       batch's write authorization: database.cpp/h + tests only).
//    4. The interactive Application->MessageBox(...) Yes/No prompt inside the
//       AUTO_EMPTY_COLOR missing-key seeding block (golden database.cpp:439) --
//       vclcompat has no TApplication/MB_YESNO/IDYES Win32-dialog surface.
//       Gated; the surrounding iTemp=1 fallback (matches golden's own
//       pre-prompt value) is preserved so the missing-key seed path still
//       produces a defined (if not user-confirmed) AUTO_EMPTY_COLOR.
//
//  All other identifiers (globals in cmydef.h; enums/#defines in MachineType.h,
//  both already included by this TU) were verified present before writing this
//  translation -- see _ga1_b6_report.md for the full key/field reconciliation.
// ---------------------------------------------------------------------------
void SYSTEM_MODULAR::ReadGeneralIni()                                           // Steven 20190320: Handler config read split into its own function
{
    AnsiString str="", MachName, sIndEPCnt;
    MyForceDirectories("D:\\GPIB9045\\system");
    str="D:\\GPIB9045\\system\\general.ini";
    MachName=CheckAndReadIniData(str, "Version", "Model", AnsiString("ModelNG"));  W906_GpibModel = MachName;  /* AI(W906-HT9050-ID) 20260924: 留下讀到的 Model 給 machine.gpibModel tag（原本是區域變數，用完就丟） */                                       //jou 20200601: GPIB model read failure should alarm, must not write back model
    if(MachName=="9045GPIB" ||
       MachName=="9046GPIB" ||
       MachName=="9046_32GPIB" ||                                               //Steven 20120505: if not 9045/9046, fall back to default 9045
       MachName=="9045GPIB_12Site" ||
       MachName=="502GPIB" ||                                                   //ChungHung 20130507: HT9045 12-site 517 update
       MachName=="1032GPIB" ||
       MachName=="7080GPIB" ||
       //AI(W906-BU-H1) 20260907, ported into the A tree 20260924 (user: "目前的第一個
       //  目標就是讓 9050 可以動起來"): HT9050 is a NEW machine in this product line,
       //  not a variant of the seven above.  Source of this entry, the arm below and
       //  Type_HT9050=800 is D:\HT9045\HT9050\HT9011UC_Cpp_V3.33.906.0, where all three
       //  were settled by user ruling; they are copied here rather than re-derived.
       //
       //  ⚠⚠ THE WHITELIST ENTRY AND THE ARM BELOW MUST STAY IN ONE COMMIT.
       //  Falling out of this `if` takes the `else` + return a few lines below, which
       //  skips the ENTIRE rest of ReadGeneralIni: measured 20260907, 1,279 lines / 422
       //  CheckAndReadIniDataGeneral calls, including the whole MachineTypeChoice
       //  chain, CustomerFunctionSelect() / ReadLastSetIni() /
       //  ReadEventLogAutoSaveInfo() and SubMachineType.  Adding only one half gives a
       //  machine that reads its config but has no identity, or an identity no config
       //  ever reaches.
       //
       //  ⓘ WHY IT MATTERS NOW, and why golden does NOT have this entry: golden V912
       //  database.cpp:308-316 lists the same seven models because HT9050 postdates it.
       //  This is a DELIBERATE DEVIATION from golden, on the user's ruling, not a
       //  translation error -- do not "restore" it to seven.
       MachName=="9050GPIB")
    {
        bHandlerModel=true;
    }
    else
    {
//        WriteIniData(str, "Version", "Model", "9046_32GPIB");                 //jou 20200601: GPIB model read failure should alarm, must not write back model (disabled in golden)
        bHandlerModel=false;
        return;
    }
    #ifdef ASE_KaohSiung                                                        //kevin 20141222
        WriteIniDataGeneral("System", "CUSTOMER_CODE", CC_ASE_KaohSiung);
    #endif

    CUSTOMER_CODE=CheckAndReadIniDataGeneral("System", "CUSTOMER_CODE", 0);
    #ifdef ASE_KaohSiung                                                        //Steven 20110520: ASE-KH cross-alarm requirement
        if(CUSTOMER_CODE!=CC_HONPREC_QC && CUSTOMER_CODE!=CC_ASE_KaohSiung_K12)                                         //Steven 20131101: add ASE-K12
            CUSTOMER_CODE=CC_ASE_KaohSiung;
    #else
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            CUSTOMER_CODE=CC_HONPREC_QC;
    #endif

    // AI(W906-WebBridge) 20260805: UNGATED.  The GA-1-B6 reason -- "bodies are
    // inside cprod.cpp's own file-wide TODO(W6) gate; calling them would not
    // link" -- stopped being true later the SAME DAY it was written: commit
    // 7ecc9dc (GA-1 B2+B3, cprod/cpublic ungate) removed that gate.  All four
    // functions in the chain are at preprocessor gate-depth 0 and DEFINED
    // (nm type T) in ht9045_globals/cprod.cpp.obj:
    //     CustomerFunctionSelect()   cprod.cpp:3828
    //     ReadLastSetIni()           cprod.cpp:3092
    //     ReadEventLogAutoSaveInfo()
    //     ReadLastDataFile()         cprod.cpp:1682  (called by ReadLastSetIni)
    //
    // MEASURED, WITH THE ISOLATION EXPERIMENT THAT PRODUCED IT.  Ungating these
    // was first blamed for a segfault; that attribution was WRONG, and the real
    // finding is more useful:
    //
    //   * Calling ReadLastSetIni() standalone segfaults -- gdb puts the fault in
    //     TIniStore::findSection, reached from CheckAndReadIniDataGeneral, which
    //     dereferences the global INIFileGeneral with NO NULL check (common.cpp,
    //     faithful BCB6 behaviour, documented at that function).
    //   * INIFileGeneral is only ever set by OpenGeneralIniFile(), whose only
    //     production caller is the SYSTEM_MODULAR constructor -- and that ctor is
    //     entirely #if 0 in this same file (see "TODO(wave): SYSTEM_MODULAR ctor"
    //     above).  So in the ported tree that pointer is permanently NULL.
    //   * CONTROL EXPERIMENT: with these three calls STILL GATED,
    //     HSys.ReadGeneralIni() segfaults exactly the same way when called
    //     without an open.  ReadGeneralIni's own body calls
    //     CheckAndReadIniDataGeneral at :292.  So the ungate adds NO new hazard;
    //     the function was already unusable without an open.
    //   * WITH OpenGeneralIniFile() called first (against a scratch copy, per the
    //     DO-NOT-MODIFY-REAL-CONFIG discipline in
    //     tests/test_ga1_readgeneralini.cpp -- ReadGeneralIni WRITES to
    //     asGeneralPath), the whole ungated chain runs and FILLS the data layer:
    //         IniConfig.sMachineType   = "HT-9046LS"
    //         IniConfig.sGPIBMachineID = "GLY320"
    //         IniConfig.RMSTesterID    = "HT9046"
    //
    // WHY THIS MATTERS.  Every machine-state global in this tree is DEFINED but
    // never POPULATED (measured in tests/test_wb_datalayer.cpp).  ReadLastSetIni()
    // is the single call that fills it: config.ini into IniConfig,
    // CustomerFunctionSelect() for the CosFunction customer-profile flags (140
    // distinct flags read at 1,091 sites, nothing sets them), and
    // ReadLastDataFile() for LastSet.  A reader that cannot tell "not loaded"
    // from a real zero is, for a UI, a screen that lies.
    //
    // STILL OPEN: nothing in the APPLICATION calls LoadMachineConfig() yet --
    // ht9045_app links only ht9045_public, so HSys is not even reachable from
    // it until GA-3 lands the god-stack in the exe. That is the last blocker.
    CustomerFunctionSelect();                                                   //customer function selection area
    ReadLastSetIni();
    ReadEventLogAutoSaveInfo();                                                 //Steven 20110603
    //Temperature related-----------------------------
    USE_NEW_TEMPCTRL_FUNCTION   =false;                                         //Steven 20100707: force old architecture for now

    TEMPCTRL_NEED_UNDER_20A     =CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_NEED_UNDER_20A", false);              //default: HotPlate staged heating
    bUseHotGunCheck             =CheckAndReadIniDataGeneral("TempCtrl", "UseHotGunCheck", false);                       //kevin 20190621: hot gun flow shortage detection
    bUseHotGunFlowCheck         =CheckAndReadIniDataGeneral("TempCtrl", "UseHotGunFlowCheck", false);                   //KaiChen 20190729: Hot Gun Flow

    //Caption=MachName;
    if(MachName=="9045GPIB")
    {
        TEMPCTRL_HOTPLATE_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_HOTPLATE_TOGTHER", false);           //jou 2010-08-16: SCC Hotplate non-staged heating
        MachineTypeChoice=Type_HT9045;
        NEW_MAX_Index_Col =8;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }
    else if(MachName=="9045GPIB_12Site")                                        //ChungHung 20130507: HT9045 12-site 517 Index Col define
    {
        TEMPCTRL_HOTPLATE_TOGTHER=false;
        MachineTypeChoice=Type_HT9045_12Site;
        NEW_MAX_Index_Col =6;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }
    else if(MachName=="9046GPIB")
    {
        //TEMPCTRL_HOTPLATE_TOGTHER=false;
        TEMPCTRL_HOTPLATE_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_HOTPLATE_TOGTHER", false);           //kevin 20150508 add //jou 2010-08-16: SCC Hotplate non-staged heating
        MachineTypeChoice=Type_HT9046;
        NEW_MAX_Index_Col=8;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }
    else if(MachName=="9046_32GPIB")
    {
        bTEMPCTRL_Shuttle_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);            //kevin 20150508 non-staged heating //Steven 20150909: cancel shrimp-head staged heating
        TEMPCTRL_HOTPLATE_TOGTHER=false;
        MachineTypeChoice=Type_HT9046_LS;
        NEW_MAX_Index_Col=8;
    }
    else if(MachName=="502GPIB")
    {
        //TEMPCTRL_HOTPLATE_TOGTHER=false;
        TEMPCTRL_HOTPLATE_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_HOTPLATE_TOGTHER", false);           //kevin 20150508 add //jou 2010-08-16: SCC Hotplate non-staged heating
        MachineTypeChoice=Type_HT502;
        NEW_MAX_Index_Col=4;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }
    else if(MachName=="7080GPIB")
    {
        //TEMPCTRL_HOTPLATE_TOGTHER=false;
        TEMPCTRL_HOTPLATE_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_HOTPLATE_TOGTHER", false);           //kevin 20150508 add //jou 2010-08-16: SCC Hotplate non-staged heating
        MachineTypeChoice=Type_HT7080;
        NEW_MAX_Index_Col=4;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }
    else if(MachName=="1032GPIB")
    {
        //TEMPCTRL_HOTPLATE_TOGTHER=false;
        TEMPCTRL_HOTPLATE_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "TEMPCTRL_HOTPLATE_TOGTHER", false);           //kevin 20150508 add //jou 2010-08-16: SCC Hotplate non-staged heating
        MachineTypeChoice=Type_HT1032;
        NEW_MAX_Index_Col=8;

        WriteIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);     //Steven 20150909: cancel shrimp-head staged heating
        bTEMPCTRL_Shuttle_TOGTHER=true;
    }

    else if(MachName=="9050GPIB")
    {
        //AI(W906-BU-H1) 20260907: HT9050 arm, user ruling, NEW_MAX_Index_Col=8.
        //  Shape copied from the 9046_32GPIB arm above (golden database.cpp:374-380)
        //  -- the ONLY one of golden's seven arms that does NOT write.  The other six
        //  call WriteIniDataGeneral("TempCtrl","bTEMPCTRL_Shuttle_TOGTHER",true), an
        //  unconditional INIFileGeneral->WriteBool (common.cpp:1548-1551) onto
        //  asGeneralPath = "D:\HT9045\system\Gerneral.ini" (common.cpp:89, bound at
        //  common.cpp:1459) -- on a live machine that overwrites whatever the operator
        //  set.  CheckAndReadIniDataGeneral (common.cpp:1530-1537) only seeds a key
        //  that is ABSENT, and this key is present (D:\HT9045\system\Gerneral.ini
        //  [TempCtrl] at :255, bTEMPCTRL_Shuttle_TOGTHER=1 at :317, measured
        //  20260907), so THIS ARM WRITES NOTHING.
        //  ZERO BEHAVIOUR CHANGE, two independent reasons: (1) this machine's
        //  D:\GPIB9045\system\general.ini has [Version] Model=9046_32GPIB (measured
        //  20260907 -- "Model=ModelNG" in D:\HT9045\CLAUDE.md was stale), so the
        //  chain leaves at the 9046_32GPIB arm and never reaches here; (2) nothing in
        //  the tree compares against Type_HT9050 -- the only code hit is the enum
        //  itself, MachineType.h:459 == 800 (100/200/300/400/500/600/700/800, no
        //  collision).  AI(W906-HT9050-FAM) 20260925: reason (2) NO LONGER HOLDS -- bd90b948 (SetMyKitSuckItemAmount) plus W906-HT9050-FAM (11 sites: VacuumUnit, MyEtherCAT, this file x2, cinitial InitShuttleThreadParameter + ChangeSite x6) now test Type_HT9050 as HT9046 family (user 20260925, RULINGS_20260925 #5); reason (1) still decides whether any of it runs.
        //  User ruling 7's "IO and motors all via 1203" is a DATA decision, not an
        //  enum decision (Mot_Table.csv CardModel -> cinitial.cpp:3904; IO_CARD_TYPE
        //  and IO_Table.csv ISABase), queued as BU-H2 -- deliberately not set here.
        bTEMPCTRL_Shuttle_TOGTHER=CheckAndReadIniDataGeneral("TempCtrl", "bTEMPCTRL_Shuttle_TOGTHER", true);            //non-writing read, per the 9046_32GPIB arm (golden database.cpp:376)
        TEMPCTRL_HOTPLATE_TOGTHER=false;
        MachineTypeChoice=Type_HT9050;
        NEW_MAX_Index_Col=8;
    }

    HotGunFlowEnable            =CheckAndReadIniDataGeneral("System",       "HotGunFlowEnable", 0);                     //KaiChen 20190729: Hot Gun Flow
    HotGunFlow_LineNo           =CheckAndReadIniDataGeneral("System",       "HotGunFlow_LineNo", 0);                    //KaiChen 20190729: Hot Gun Flow
    HotGunFlow_DevNo            =CheckAndReadIniDataGeneral("System",       "HotGunFlow_DevNo", 0);                     //KaiChen 20190729: Hot Gun Flow
    HotGunFlow_Gun1_ChannelNo   =CheckAndReadIniDataGeneral("System",       "HotGunFlow_Gun1_ChannelNo", 0);            //KaiChen 20190729: Hot Gun Flow
    HotGunFlow_Gun2_ChannelNo   =CheckAndReadIniDataGeneral("System",       "HotGunFlow_Gun2_ChannelNo", 0);            //KaiChen 20190729: Hot Gun Flow

    if(MachineTypeChoice==Type_HT9045 ||
       MachineTypeChoice==Type_HT502)                                           //Steven 20190307: add for HT-9046LA
        SubMachineType=Type_None;
    else
        SubMachineType=CheckAndReadIniDataGeneral("Version", "SubModel", Type_None);

    SHUTTLE_COOLING     =CheckAndReadIniDataGeneral("TempCtrl", "SHUTTLE_COOLING", 0.0);                                //cold-air vortex tube offset
    VORTEX_COOLING      =CheckAndReadIniDataGeneral("TempCtrl", "VORTEX_COOLING",  0.5);                                //cold-air vortex tube start offset
    SOCKET_OFFSET       =CheckAndReadIniDataGeneral("TempCtrl", "SOCKET_OFFSET",  10.0);                                //Socket offset
    bUseSocketTemp      =CheckAndReadIniDataGeneral("TempCtrl", "Socket",            0);                                //9th-axis heater enable
    bUseAuto2Empty      =CheckAndReadIniDataGeneral("System",   "bUseAuto2Empty",    0);                                //kevin 20120718: Auto2 has a splittable cylinder to feed material in
    SHUTTLE_COOLING     =CheckRange(SHUTTLE_COOLING, 10.0, 0.0);
    VORTEX_COOLING      =CheckRange(VORTEX_COOLING,  10.0, 0.0);
    SOCKET_OFFSET       =CheckRange(SOCKET_OFFSET,   30.0, 0.0);
    TC401HeaterControl  =CheckAndReadIniDataGeneral("TempCtrl", "HEATER_CTRL_TYPE", KT4H);                              //Steven 20141030: add OMRON E5DC temp controller
    OFFLINE_ALARM       =CheckAndReadIniDataGeneral("System",   "OFFLINE_ALARM",    1);                                 //Steven 20140805

    USE_AUTO_RETEST     =CheckAndReadIniDataGeneral("System", "USE_AUTO_RETEST",     (int)eartUninstall);               //ChungHung 20140317: add Auto Retest

    //RogerYang 20250823: executes earlier here, moved to this spot
    SUPPORT_2_EMPTY_EMPTY  =CheckAndReadIniDataGeneral("System", "SUPPORT_2_EMPTY_EMPTY", 0);                           //Steven 20230907: For HT-9011UC
    int iTemp;
    if(CheckIniData(asGeneralPath, "System", "AUTO_EMPTY_COLOR")==false)
    {
        if(SUPPORT_2_EMPTY_EMPTY==true)                                         //Steven 20230907: For HT-9011UC
        {
            iTemp=2;
        }
        else
        {
            iTemp=1;
            // AI(W906-GA1-B6) 20260804: gated -- vclcompat has no TApplication/
            // MB_YESNO/IDYES Win32-dialog surface.  golden prompts the operator
            // "Machine Empty load and Color load is Auto?" and downgrades
            // iTemp to 0 on any answer other than Yes; without the prompt we
            // keep golden's own pre-prompt value (iTemp=1) so the missing-key
            // seed path still writes a defined value.  See _ga1_b6_report.md.
#if 0 // TODO(GA1-B6): Application->MessageBox(...) interactive Yes/No prompt -- no TApplication/MB_YESNO/IDYES in vclcompat
            if(Application->MessageBox("Machine Empty load and Color load is Auto?", NULL, MB_YESNO | MB_TOPMOST) != IDYES)
                iTemp=0;
#endif
        }
        WriteIniDataGeneral("System","AUTO_EMPTY_COLOR", iTemp);
        AUTO_EMPTY_COLOR=iTemp;
    }
    else
    {
        AUTO_EMPTY_COLOR=CheckAndReadIniDataGeneral("System", "AUTO_EMPTY_COLOR", 0);
    }

    if(USE_AUTO_RETEST==eartInstall)
    {
        UNLOADER_ART[eAuto1] =CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO1_ART",     (int)eartInstall);         //Steven 20161221 (jou): for SCK only Auto 2 has ART
        UNLOADER_ART[eAuto2] =CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO2_ART",     (int)eartInstall);         //Steven 20161221 (jou): for SCK only Auto 2 has ART
        UNLOADER_ART[eAuto3] =CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO3_ART",     (int)eartInstall);         //Steven 20161221 (jou): for SCK only Auto 2 has ART

        if(AUTO_EMPTY_COLOR>=3)                                                 //Steven 20230907: For HT-9011UC
        {
            UNLOADER_ART[eAuto4]    =CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO4_ART",     (int)eartInstall);
            UNLOADER_ART[eAuto5]    =CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO5_ART",     (int)eartInstall);
            if(AUTO_EMPTY_COLOR>=4)
                UNLOADER_ART[eAuto6]=CheckAndReadIniDataGeneral("System", "UNLOADER_AUTO6_ART",     (int)eartInstall);
            else
                UNLOADER_ART[eAuto6]=(int)eartUninstall;
        }
        else
        {
            UNLOADER_ART[eAuto4]    =(int)eartUninstall;
            UNLOADER_ART[eAuto5]    =(int)eartUninstall;
            UNLOADER_ART[eAuto6]    =(int)eartUninstall;
        }
    }
    else
    {
        UNLOADER_ART[eAuto1] =(int)eartUninstall;
        UNLOADER_ART[eAuto2] =(int)eartUninstall;
        UNLOADER_ART[eAuto3] =(int)eartUninstall;
        UNLOADER_ART[eAuto4] =(int)eartUninstall;
        UNLOADER_ART[eAuto5] =(int)eartUninstall;
        UNLOADER_ART[eAuto6] =(int)eartUninstall;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        bNewCatchTrayblock = CheckAndReadIniDataGeneral("System" , "UseCatchTrayBlock"   , 1);                          //kevin 20200512: catch-tray shield trimmed short
    else
        bNewCatchTrayblock = CheckAndReadIniDataGeneral("System" , "UseCatchTrayBlock"   , 0);                          //kevin 20200512: catch-tray shield trimmed short

    MOTOR_DRIVER_TYPE   =CheckAndReadIniDataGeneral("MotorDriver", "Type", Panasonic_DRIVER);                           //Steven 20110321: distinguish other-axis motor brand
    USE_COLOR_TRAY_SENSOR=CheckAndReadIniDataGeneral("System",   "USE_COLOR_TRAY_SENSOR", 0);                           //20140903 wei: color Tray
    USE_SOCKET_SENSOR=CheckAndReadIniDataGeneral("System",   "USE_SOCKET_SENSOR", 999);                                 //JerryYang 20200327: Socket sensor switched to hardware-option decision

    CANBUS_METHOD=CheckAndReadIniDataGeneral("System",   "Canbus_Method", 0);   //Sam 20210518: new CanBus software configuration

    for(int i=0; i<tcTotalCount; i++)                                           //Steven 20100831: ambient-check position
    {
        str.sprintf("AMBIENT_TEMP_CHECK%02d", i+1);
        if(i<2)                                                                 //HotPlate default: no check
            AMBIENT_TEMP_CHECK[i]=CheckAndReadIniDataGeneral("TempCtrl", str, false);
        else
            AMBIENT_TEMP_CHECK[i]=CheckAndReadIniDataGeneral("TempCtrl", str, true);
    }

    //Number display----------------------------------------------
    NUMBER_PANEL_TYPE   =CheckAndReadIniDataGeneral("System",          "NUMBER_PANEL_TYPE",    2);
    dNumberPanelDelay   =CheckAndReadIniDataGeneral("NUMBER_PANEL",    "NUMBER_PANEL_DELAY",   1.0);                    //Sam 20240604: display polling period changed to double
    //Steven 20120217: Com Port made configurable
    sNumberPanelComPort =CheckAndReadIniDataGeneral("NUMBER_PANEL",    "COM_PORT",         AnsiString("COM4"));
    sNumberPanelComPort2=CheckAndReadIniDataGeneral("NUMBER_PANEL2",    "COM_PORT",        AnsiString("COM4"));

    sTorqueComPort      =CheckAndReadIniDataGeneral("IndexDriver",     "COM_PORT",         AnsiString("COM1"));
    sTempComPort        =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT",         AnsiString("COM2"));
    sTempOmronComPort   =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT_OMRON",   AnsiString("COM7"));
    TrayStepMotor_ComPort=CheckAndReadIniDataGeneral("TrayY", "COM PORT",  AnsiString("COM18"));
    sTempDynamicComPort =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT_DYNAMIC", AnsiString("COM6"));
    sRTCComPort         =CheckAndReadIniDataGeneral("RealTimeCCD",     "Port",             AnsiString("COM3"));

    asLASER_COM[0]      =CheckAndReadIniDataGeneral("Laser",        "COM_Laser_1",     AnsiString("COM16"));            //Steven 20140228: laser distance-measurement function
    asLASER_COM[1]      =CheckAndReadIniDataGeneral("Laser",        "COM_Laser_2",     AnsiString("COM17"));            //Steven 20140228: laser distance-measurement function
    asLASER_COM[2]      =CheckAndReadIniDataGeneral("Laser",        "COM_Laser_InArm", AnsiString("COM18"));            //Steven 20140228: laser distance-measurement function
    asLASER_COM[3]      =CheckAndReadIniDataGeneral("Laser",        "COM_Laser_OutArm",AnsiString("COM19"));            //==> Eastsun 20260525 laser integration MissF-4: +asLASER_COM[3] COM_Laser_OutArm

    sFinePitchComPort   =CheckAndReadIniDataGeneral("FinePitch",       "COM_PORT",           AnsiString("COM3"));
    sFinePitchAdjustmentComPort =CheckAndReadIniDataGeneral("FinePitch",       "COM_PORT_Adjustment",   AnsiString("COM3"));

    if(sNumberPanelComPort!="" && sNumberPanelComPort.Pos("COM")!=1)
    {
        sNumberPanelComPort="COM"+sNumberPanelComPort;
    }

    if(sNumberPanelComPort2!="" && sNumberPanelComPort2.Pos("COM")!=1)
    {
        sNumberPanelComPort2="COM"+sNumberPanelComPort2;
    }

    if(sTorqueComPort!="" && sTorqueComPort.Pos("COM")!=1)
    {
        sTorqueComPort="COM"+sTorqueComPort;
    }

    if(sTempComPort!="" && sTempComPort.Pos("COM")!=1)
    {
        sTempComPort="COM"+sTempComPort;
    }

    if(sTempOmronComPort!="" && sTempOmronComPort.Pos("COM")!=1)
    {
        sTempOmronComPort="COM"+sTempOmronComPort;
    }

    if(sRTCComPort!="" && sRTCComPort.Pos("COM")!=1)
    {
        sRTCComPort="COM"+sRTCComPort;
    }

    if(sTempDynamicComPort!="" && sTempDynamicComPort.Pos("COM")!=1)
    {
        sTempDynamicComPort="COM"+sTempDynamicComPort;
    }

    WEIGHT_CALIBRATION  =CheckAndReadIniDataGeneral("System",    "WEIGHT_CALIBRATION",      0);                         //Steven 20111108
    //Shuttle Sensor-----------------------------
    SHUTTLE_SENSOR_TYPE=CheckAndReadIniDataGeneral("System", "SHUTTLE_SENSOR_TYPE", 0);
    NUEC_TYPE=CheckAndReadIniDataGeneral("System", "NUEC_TYPE", 0);             //Sam 20230707: EtherCAT Shuttle sensor
    ENABLE_OUT_SHUTTLE_SENEOR=CheckAndReadIniDataGeneral("System", "ENABLE_OUT_SHUTTLE_SENEOR", true);                  //Steven 20120531: bypass out shuttle sensor
    ENABLE_OUT_SHUTTLEY_LATCH=CheckAndReadIniDataGeneral("System", "ENABLE_OUT_SHUTTLEY_LATCH", false);                 //Steven 20170518 (jou): Out shuttle Y sensor uses LATCH
    Use_AxisY_Sensor_2x3mode =CheckAndReadIniDataGeneral("System", "Use_AxisY_Sensor_2x3mode", false);                  //Isaac 20180402 (Steven): 2x3 mode uses Y sensor to detect Outshuttle residual
    Bias_Mode_Use_Y_Sensor   =CheckAndReadIniDataGeneral("System", "Bias_Mode_Use_Y_Sensor", false);                    //Steven 20240805: 1x2 Bias mode uses Y sensor to detect Outshuttle residual

    if(MachineTypeChoice==Type_HT9046 ||
       MachineTypeChoice==Type_HT9046_LS ||
       MachineTypeChoice==Type_HT1032 || MachineTypeChoice==Type_HT9050)       //AI(W906-HT9050-FAM) 20260925: 加 Type_HT9050（出料飛梭 pocket sensor 強制 bypass，不寫檔）-- golden 沒有 HT9050，照 HT9046 家族（使用者 20260925，RULINGS_20260925 第 5 條）
    {
        ENABLE_OUT_SHUTTLE_SENEOR=false;
    }

    if(MachineTypeChoice==Type_HT9046_LS ||                                     //Steven 20170518 (jou): Out shuttle Y sensor uses LATCH
       MachineTypeChoice==Type_HT1032)
    {
        ENABLE_OUT_SHUTTLEY_LATCH=true;
    }
    USE_RFID_READER=CheckAndReadIniDataGeneral("System", "USE_RFID_READER", 0);                                         //Steven 20220713: RFID Reader for SJSEMI

    //Safety door----------------------------------
    SAFE_DOOR_AMOUNT=CheckAndReadIniDataGeneral("System", "SAFE_DOOR_AMOUNT", 2);

    //Tray knock--------------------------------
    TRAY_VIBRATION=CheckAndReadIniDataGeneral("System", "TRAY_VIBRATION", 0);   //JerryYang 20170531 (wei) bool->int
    TRAY_ARM_MODE =CheckAndReadIniDataGeneral("System", "TRAY_ARM_MODE", 0);    //Frank 20230419

    USE_LOADER_VIBRATION=CheckAndReadIniDataGeneral("System", "LOADER_VIBRATION", false);                               //JerryYang 20191001: loader vibration motor

    USE_TRAY_ROBOT=CheckAndReadIniDataGeneral("System", "USE_TRAY_ROBOT", 0);   //Steven 20170330 (Wei): For HT-9046LM
//Steven 20260924 (S12-C)：golden V912 有、移植樹沒有；FileRW/IniConfig.gen.inc（golden cConfiguration.cpp）要用到，逐字補上（golden database.cpp:604-605）
    USE_TRAY_ARM_TYPE=CheckAndReadIniDataGeneral("System", "USE_TRAY_ARM_TYPE", 0);                                    //Ifor 20260824 : Tray Arm hardware type
    TRAY_ARM_STANDBY_OFFSET=CheckAndReadIniDataGeneral("System", "TrayArmStandbyEmptyOffset", 15000);                    //Ifor 20260824 : new Tray Arm standby offset from Empty toward Color
    USE_LOADER_HINGE=CheckAndReadIniDataGeneral("System", "USE_LOADER_HINGE",   0);                                     //Steven 20170330 (Wei): For TSMC

    //Ifor 20150708: CKD FCM Clean Air--------------------------------
    USE_CKD_FCM_CleanAir=CheckAndReadIniDataGeneral("System", "USE_CKD_FCM_CleanAir", false);

    if(USE_AUTO_RETEST==eartInstall)                                            //wei 20150807 Catch Tray Model
    {
        USE_CATCH_TRAY_MODEL=CheckAndReadIniDataGeneral("System", "USE_CATCH_TRAY_MODEL", 2);
    }
    else
    {
        USE_CATCH_TRAY_MODEL=CheckAndReadIniDataGeneral("System", "USE_CATCH_TRAY_MODEL", 0);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        bNewCatchTrayblock = CheckAndReadIniDataGeneral("System" , "UseCatchTrayBlock"   , 1);                          //kevin 20200512: catch-tray shield trimmed short
    else
        bNewCatchTrayblock = CheckAndReadIniDataGeneral("System" , "UseCatchTrayBlock"   , 0);                          //kevin 20200512: catch-tray shield trimmed short

    USE_16_HEATER=CheckAndReadIniDataGeneral("System", "USE_16_HEATER", eht4Heater);                                    //Steven 20111011: use 16 heaters
    if(USE_16_HEATER==eht16HeaterEJ1N)                                          //Steven 20200318: fix EJ1N read-abnormal issue
        EJ1N_Count=4;
    else if(USE_16_HEATER==eht32HeaterEJ1N)
        EJ1N_Count=8;
    else
        EJ1N_Count=0;

    //Real Time CCD--------------------------------
    REAL_TIME_CCD   =CheckAndReadIniDataGeneral("System", "REAL_TIME_CCD", false);
    RTC_TemperNumber=CheckAndReadIniDataGeneral("System", "RTC_TemperNumber", 1);                                       //Isaac 20201217: RTC CCD adds a second temp-sense group
    CCD2_TEMPER     =CheckAndReadIniDataGeneral("System", "CCD2_TEMPER", false);
    LB_TEMP         =CheckAndReadIniDataGeneral("System", "LB_TEMP", false);    //Steven 20181023: LB temperature
    LB_TEMP_UpDown  =CheckAndReadIniDataGeneral("System", "LB_TEMP_UpDown", false);                                     //Frank 20241231: add
    Index_ESDAir    =CheckAndReadIniDataGeneral("System", "Index_ESDAir", false);                                       //kevin 20200207: add index ESD temp

    //OCR------------------------------------------
    INSTALL_OCR=CheckAndReadIniDataGeneral("System", "INSTALL_OCR",  eocrUninstal);                                     //Steven 20120716: OCR
    asOCRComPort=CheckAndReadIniDataGeneral("OCR",   "OCR_COM_PORT", AnsiString("COM18"));
    asOCRwithTesterComPort=CheckAndReadIniDataGeneral("OCR",   "OCRwithTester_COM_PORT", AnsiString("COM19"));

    INSTALL_OCR_YMot=CheckAndReadIniDataGeneral("System", "INSTALL_OCR_YMot",  eocrYMotUninstal);                       //Frank 20250214 add

    SAFE_DOOR_LOCK  =CheckAndReadIniDataGeneral("System", "SAFE_DOOR_LOCK", false);
    CLEAN_AIR       =CheckAndReadIniDataGeneral("IndexDriver", "CLEAN_AIR", false);                                     //ChungHung 20120821: Clean Air
//    HighTemperatureSet150=CheckAndReadIniDataGeneral("System",  "HighTemperatureSet150", false); //wei 20150617: retrofit max temp 150C
//    HighTemperatureSet155=CheckAndReadIniDataGeneral("System",  "HighTemperatureSet155", false); //Sam 20240118: new 155C mode
//    HighTemperatureSet175=CheckAndReadIniDataGeneral("System",  "HighTemperatureSet175", false); //JerryYang 20160810: retrofit max temp 175C

    int iTemp175=CheckAndReadIniDataGeneral("System", "HighTemperatureSet175", 0);                                      //Frank 20160705 add: retrofit max temp 175C
    int iTemp155=CheckAndReadIniDataGeneral("System", "HighTemperatureSet155", 0);                                      //Sam 20240118: new 155C mode
    int iTemp150=CheckAndReadIniDataGeneral("System", "HighTemperatureSet150", 0);                                      //wei 20150617: retrofit max temp 150C

    if(iTemp175!=0)                                                             //Steven 20250701: unify machine temperature setting
    {
        iTempLimitation=CheckAndReadIniDataGeneral("System", "HighTempLimit", tTemp175);
        iTempLimitation=tTemp175;
    }
    else if(iTemp155!=0)
    {
        iTempLimitation=CheckAndReadIniDataGeneral("System", "HighTempLimit", tTemp155);
        iTempLimitation=tTemp155;
    }
    else if(iTemp150!=0)
    {
        iTempLimitation=CheckAndReadIniDataGeneral("System", "HighTempLimit", tTemp150);
        iTempLimitation=tTemp150;
    }
    else
    {
        iTempLimitation=CheckAndReadIniDataGeneral("System", "HighTempLimit", tTemp130);
    }

    DewPoint_Hardware_Install=CheckAndReadIniDataGeneral("System", "DewPoint_Hardware_Install", 0);                     //Steven 20191017: dew-point meter
//    USE_FINGER_PRINT     =CheckAndReadIniDataGeneral("System",  "USE_FINGER_PRINT", 0);//Steven 20190503: fingerprint auth
    USE_FINGER_PRINT=0;                                                         //Steven 20240920: fingerprint auth removed
    MAGAZINE_BIN_DISP_TYPE=CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", 0);                           //JerryYang 20220909: add magazine

    //ATC
    ATC_SYSTEM                  =CheckAndReadIniDataGeneral("ATC", "USE_ATC_MODE",  eATCUninstall);                     //jou 2012-03-08: Enable ATC mode
    asATC1ComPort               =CheckAndReadIniDataGeneral("ATC", "ATC1_COM_PORT", AnsiString("COM15"));
    asATC2ComPort               =CheckAndReadIniDataGeneral("ATC", "ATC2_COM_PORT", AnsiString("COM16"));
    asATC3ComPort               =CheckAndReadIniDataGeneral("ATC", "ATC3_COM_PORT", AnsiString("COM17"));
    asATC4ComPort               =CheckAndReadIniDataGeneral("ATC", "ATC4_COM_PORT", AnsiString("COM18"));
    asATCSYSTEMIP               =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_IP", AnsiString("172.16.8.90"));
    asATCSYSTEMPORT             =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_PORT", 1234);
    asATCSYSTEMUSEHEAT          =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_USEHEAT", 4);                            //Ifor 20160506: add New ATC Interface Use Heat Count

    ATC_MixMode                 =CheckAndReadIniDataGeneral("ATC", "ATC_Mix_Mode",  eATCUninstall);                     //jou 2012-03-08: Enable ATC mode

    if(ATC_SYSTEM==eATCHonPrecType)
        iATC_Use_Heat_Count     =4;
    else
        iATC_Use_Heat_Count     =asATCSYSTEMUSEHEAT;                            //Ifor 20160506 add ATC Heat Count
    bUseATC_SelfTestFunction    =CheckAndReadIniDataGeneral("ATC", "USE_ATC_SELFTEST", 0);                              //Ifor 20170120 (Steven): Add ATC Self Test Function Switch (General.ini only)
    INDEX_PRESS_TYPE            =(eIndexPressType)CheckAndReadIniDataGeneral("System",       "INDEX_PRESS_TYPE",         0);                                    //Steven 20110310: 240KG
    USE_IN_OUT_ARM_Y_PITCH      =CheckAndReadIniDataGeneral("System", "USE_IN_OUT_ARM_Y_PITCH", iXPitch60);             //jou 2012-05-15: choose Y Pitch mechanism mode
    USE_OUT_ARM_Y_PITCH         =CheckAndReadIniDataGeneral("System", "USE_OUT_ARM_Y_PITCH", USE_IN_OUT_ARM_Y_PITCH);   //JerryYang 20251218: IN/OUT ARM support different modules

    IN_OUT_ARM_Y_PITCH_MIN      =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_Y_PITCH_MIN", 1500);
    IN_OUT_ARM_Y_PITCH_MAX      =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_Y_PITCH_MAX", 7500);
    BASE_X_TO_HP                =CheckAndReadIniDataGeneral("System", "BASE_X_TO_HP",           6800);                  //Steven 20230826: reference-axis X to HotPlate edge distance
    bUserDefMaxContactHeight    =CheckAndReadIniDataGeneral("Index", "EnableUserDefMaxContactHeight",   false);         //Steven 20180412: For new HT-9045 taller variant
    dUserDefMaxContactHeight    =CheckAndReadIniDataGeneral("Index", "UserDefMaxContactHeight",   -135.0);              //Steven 20180412: For new HT-9045 taller variant

    USE_OUT_SORT_ARM            =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_ARM", eartUninstall);           //Steven 20240822: For HT-9046AU
    USE_OUT_SORT_X_PITCH_MIN    =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_X_PITCH_MIN", 1333);            //RogerYang 20250609: Add for 9046AU
    USE_OUT_SORT_X_PITCH_MAX    =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_X_PITCH_MAX", 4000);
    USE_2nd_LOADER              =CheckAndReadIniDataGeneral("System",     "USE_2nd_LOADER",     0);                     //RogerYang 20250401: For HT-9046AU

    USE_IN_Y_IS_AUTO_PITCH      =(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||   //Steven 20230602: unify the Y-variable-pitch flag  //JerryYang 20251218: IN/OUT ARM support different modules
                                  USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
                                  USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb       ||
                                  USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be  ||   //Steven for HT7080 //Ztex 2023.12.06 Add HT-1032
                                  USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc);                                        //Ztex 2024.02.24 Add HT-1132

    USE_OUT_Y_IS_AUTO_PITCH     =(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||      //Steven 20230602: unify the Y-variable-pitch flag  //JerryYang 20251218: IN/OUT ARM support different modules
                                  USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
                                  USE_OUT_ARM_Y_PITCH==iXYPitchBb       ||
                                  USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be  ||      //Steven for HT7080 //Ztex 2023.12.06 Add HT-1032
                                  USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc);   //Ztex 2024.02.24 Add HT-1132

    USE_16PICKER_TYPE           =(USE_PICKER_COUNT==ep16Picker ||
                                  USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)?1:0;                                         //Ztex 2023.12.06 Add HT-1032

    #ifdef FOR_QLE
        if(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU)
        {
            SPIL_FOR_QLE=1;
            WriteIniDataGeneral("System", "SPIL_FOR_QLE",  SPIL_FOR_QLE);       //Steven 20230110: For Qu Liang
        }
    #else
        if(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU)
            SPIL_FOR_QLE        =CheckAndReadIniDataGeneral("System", "SPIL_FOR_QLE", 0);
        else
            SPIL_FOR_QLE        =0;
    #endif

    //AI(ht9045-v899) 20260417: read AutoDeviceEjection flag from Gerneral.ini
    bAutoDeviceEjection=CheckAndReadIniDataGeneral("System", "bAutoDeviceEjection", 0);

    if(SPIL_FOR_QLE==1 && ATC_SYSTEM==eATCUninstall && MachName=="9045GPIB")    //Steven 20230110: Index safe-height default-enabled for QuLiang's Chamber-equipped 9045
    {
        bUser_Define_IndexZ_SafePos =CheckAndReadIniDataGeneral("Index", "EnableUser_Define_IndexZ_SafePos",   1);      //Richard 20230107: SPIL customer-defined install height (200~800)
        dUserDefineIndexZSafePos    =CheckAndReadIniDataGeneral("Index", "UserDefineIndexZSafePos",   800);             //Richard 20230107: SPIL customer-defined install height (200~800)
    }
    else
    {
        bUser_Define_IndexZ_SafePos =CheckAndReadIniDataGeneral("Index", "EnableUser_Define_IndexZ_SafePos",   false);  //Richard 20230107: SPIL customer-defined install height (200~800)
        dUserDefineIndexZSafePos    =CheckAndReadIniDataGeneral("Index", "UserDefineIndexZSafePos",   200);             //Richard 20230107: SPIL customer-defined install height (200~800)
    }

    if(dUserDefineIndexZSafePos<200)
        dUserDefineIndexZSafePos=200;
    else if(dUserDefineIndexZSafePos>800)
        dUserDefineIndexZSafePos=800;

    USE_PICKER_COUNT            =CheckAndReadIniDataGeneral("System",       "USE_PICKER_COUNT",     1);                 //Steven 20161117: for HT-9045S

    LOAD_Z_USE_MOTOR[0]=CheckAndReadIniDataGeneral("TrayZ", "LOAD_Z_USE_MOTOR", false);                                 //Steven 20190813: switch loader Tray to stepper motor
    LOAD_Z_USE_MOTOR[1]=CheckAndReadIniDataGeneral("TrayZ", "EMPTY_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[2]=CheckAndReadIniDataGeneral("TrayZ", "COLOR_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[3]=CheckAndReadIniDataGeneral("TrayZ", "AUTO1_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[4]=CheckAndReadIniDataGeneral("TrayZ", "AUTO2_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[5]=CheckAndReadIniDataGeneral("TrayZ", "AUTO3_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[6]=CheckAndReadIniDataGeneral("TrayZ", "AUTO4_Z_USE_MOTOR", false);                                //Steven 20230907: For HT-9011UC
    LOAD_Z_USE_MOTOR[7]=CheckAndReadIniDataGeneral("TrayZ", "AUTO5_Z_USE_MOTOR", false);
    LOAD_Z_USE_MOTOR[8]=CheckAndReadIniDataGeneral("TrayZ", "AUTO6_Z_USE_MOTOR", false);
    LOAD_Y_USE_MOTOR[0]=(CheckAndReadIniDataGeneral("TrayY", "LOAD_Y_USE_MOTOR", false) || (INSTALL_OCR>eocrUninstal));         //Jimmychiu 20240307: switch Loader Tray to stepper motor
    LOAD_Y_USE_MOTOR[1]=CheckAndReadIniDataGeneral("TrayY", "EMPTY_Y_USE_MOTOR", false); //AI(ht9045-v899) 20260423: enable empty y stepper
    LOAD_Y_USE_MOTOR[2]=CheckAndReadIniDataGeneral("TrayY", "COLOR_Y_USE_MOTOR", false); //AI(ht9045-v899) 20260423: enable color y stepper
    LOAD_Y_USE_MOTOR[3]=CheckAndReadIniDataGeneral("TrayY", "AUTO1_Y_USE_MOTOR", false); //AI(ht9045-v899) 20260423: enable auto1 y stepper
    LOAD_Y_USE_MOTOR[4]=CheckAndReadIniDataGeneral("TrayY", "AUTO2_Y_USE_MOTOR", false); //AI(ht9045-v899) 20260423: enable auto2 y stepper
    LOAD_Y_USE_MOTOR[5]=CheckAndReadIniDataGeneral("TrayY", "AUTO3_Y_USE_MOTOR", false); //AI(ht9045-v899) 20260423: enable auto3 y stepper

    LOADUNLOAD_USE_CASSETTE[0]=CheckAndReadIniDataGeneral("TrayCassette", "LOAD_USE_Cassette",  false);
    LOADUNLOAD_USE_CASSETTE[1]=CheckAndReadIniDataGeneral("TrayCassette", "EMPTY_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[2]=CheckAndReadIniDataGeneral("TrayCassette", "COLOR_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[3]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO1_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[4]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO2_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[5]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO3_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[6]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO4_USE_Cassette", false);                 //Steven 20230907: For HT-9011UC
    LOADUNLOAD_USE_CASSETTE[7]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO5_USE_Cassette", false);
    LOADUNLOAD_USE_CASSETTE[8]=CheckAndReadIniDataGeneral("TrayCassette", "AUTO6_USE_Cassette", false);

    sIndEPCnt=CheckAndReadIniDataGeneral("System", "Individual_EP_COUNT", AnsiString("16"));

    if(sIndEPCnt=="16")
    {
        iIndEPCnt=16;
    }
    else if(sIndEPCnt=="8")
    {
        iIndEPCnt=8;                                                            //RogerYang 20260603: Add 8EP
    }
    else
    {
        iIndEPCnt=4;                                                            //JerryYang 20210413: Add 4-independent-EP variant
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        iInArmXBase=2;
        iInArmYBase=0;
        iOutArmXBase=2;
        iOutArmYBase=0;

        IN_OUT_ARM_Y_PITCH_MIN=1500;
        IN_OUT_ARM_Y_PITCH_MAX=7500;

        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;

        iInArmShtXCenterPos=-2000;
        iInArmShtYCenterPos=-3000;
        iOutArmShtXCenterPos=-2000;
        iOutArmShtYCenterPos=-3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                           //Steven 20141029: XY-Pitch
    {
        iInArmXBase=2;
        iInArmYBase=1;
        iOutArmXBase=1;
        iOutArmYBase=1;
        if(IN_OUT_ARM_Y_PITCH_MIN>IN_OUT_ARM_Y_PITCH_MAX ||
           IN_OUT_ARM_Y_PITCH_MIN<1500 ||
           IN_OUT_ARM_Y_PITCH_MIN>5000)
            IN_OUT_ARM_Y_PITCH_MIN=1500;

        if(IN_OUT_ARM_Y_PITCH_MAX<IN_OUT_ARM_Y_PITCH_MIN ||
           IN_OUT_ARM_Y_PITCH_MAX>8000 ||
           IN_OUT_ARM_Y_PITCH_MAX<5000)
            IN_OUT_ARM_Y_PITCH_MAX=8000;                                        //kevin 20190322 change
        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;    //Jimmychiu 20221205 add y pitch minus
        iInArmShtXCenterPos=-2000;
        iInArmShtYCenterPos=3000;
        iOutArmShtXCenterPos=2000;
        iOutArmShtYCenterPos=3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchRowA)                               //Steven 20170424 (wei): new XY-variable-pitch
    {
        iInArmXBase=2;
        iInArmYBase=0;
        iOutArmXBase=1;
        iOutArmYBase=0;

        iInArmShtXCenterPos=-2000;
        iInArmShtYCenterPos=3000;
        iOutArmShtXCenterPos=2000;
        iOutArmShtYCenterPos=3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)
    {
        if(IN_OUT_ARM_Y_PITCH_MIN>IN_OUT_ARM_Y_PITCH_MAX ||
           IN_OUT_ARM_Y_PITCH_MIN<2500 ||
           IN_OUT_ARM_Y_PITCH_MIN>5000)
            IN_OUT_ARM_Y_PITCH_MIN=2500;

        if(IN_OUT_ARM_Y_PITCH_MAX<IN_OUT_ARM_Y_PITCH_MIN ||
           IN_OUT_ARM_Y_PITCH_MAX>6500 ||
           IN_OUT_ARM_Y_PITCH_MAX<5000)
            IN_OUT_ARM_Y_PITCH_MAX=6500;

        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;    //Jimmychiu 20221205 add y pitch minus

        iInArmXBase=3;
        iInArmYBase=1;
        iOutArmXBase=3;
        iOutArmYBase=1;
        iInArmShtXCenterPos =1100;
        iInArmShtYCenterPos =3000;
        iOutArmShtXCenterPos=1100;
        iOutArmShtYCenterPos=3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb)                                 //Steven for HT7080
    {
        if(IN_OUT_ARM_Y_PITCH_MIN>IN_OUT_ARM_Y_PITCH_MAX ||
           IN_OUT_ARM_Y_PITCH_MIN<1900 ||
           IN_OUT_ARM_Y_PITCH_MIN>5000)
            IN_OUT_ARM_Y_PITCH_MIN=2000;

        if(IN_OUT_ARM_Y_PITCH_MAX<IN_OUT_ARM_Y_PITCH_MIN ||
           IN_OUT_ARM_Y_PITCH_MAX>6500 ||
           IN_OUT_ARM_Y_PITCH_MAX<5000)
            IN_OUT_ARM_Y_PITCH_MAX=6000;

        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;    //Jimmychiu 20221205 add y pitch minus

        iInArmXBase=1;
        iInArmYBase=1;
        iOutArmXBase=2;
        iOutArmYBase=1;
        iInArmShtXCenterPos =2000;
        iInArmShtYCenterPos =3000;
        iOutArmShtXCenterPos=-2000;
        iOutArmShtYCenterPos=3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        if(IN_OUT_ARM_Y_PITCH_MIN>IN_OUT_ARM_Y_PITCH_MAX ||
           IN_OUT_ARM_Y_PITCH_MIN<2500 ||
           IN_OUT_ARM_Y_PITCH_MIN>5000)
            IN_OUT_ARM_Y_PITCH_MIN=2500;

        if(IN_OUT_ARM_Y_PITCH_MAX<IN_OUT_ARM_Y_PITCH_MIN ||
           IN_OUT_ARM_Y_PITCH_MAX>6500 ||
           IN_OUT_ARM_Y_PITCH_MAX<5000)
            IN_OUT_ARM_Y_PITCH_MAX=6500;

        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;    //Jimmychiu 20221205 add y pitch minus

        iInArmXBase=3;
        iInArmYBase=1;
        iOutArmXBase=3;
        iOutArmYBase=1;
        iInArmShtXCenterPos =1100;
        iInArmShtYCenterPos =3000;
        iOutArmShtXCenterPos=1100;
        iOutArmShtYCenterPos=3000;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                       //Ztex 2024.02.24 Add HT-1132
    {
        if(IN_OUT_ARM_Y_PITCH_MIN>IN_OUT_ARM_Y_PITCH_MAX ||
           IN_OUT_ARM_Y_PITCH_MIN<2500 ||
           IN_OUT_ARM_Y_PITCH_MIN>5000)
            IN_OUT_ARM_Y_PITCH_MIN=2500;

        if(IN_OUT_ARM_Y_PITCH_MAX<IN_OUT_ARM_Y_PITCH_MIN ||
           IN_OUT_ARM_Y_PITCH_MAX>6500 ||
           IN_OUT_ARM_Y_PITCH_MAX<5000)
            IN_OUT_ARM_Y_PITCH_MAX=6500;

        iPitchY_Max_minus_Min=IN_OUT_ARM_Y_PITCH_MAX-IN_OUT_ARM_Y_PITCH_MIN;    //Jimmychiu 20221205 add y pitch minus

        iInArmXBase=1;
        iInArmYBase=1;
        iOutArmXBase=2;
        iOutArmYBase=1;
        iInArmShtXCenterPos =1100;
        iInArmShtYCenterPos =3250;
        iOutArmShtXCenterPos=1100;
        iOutArmShtYCenterPos=3250;
    }
    else
    {
        IN_OUT_ARM_Y_PITCH_MIN=0;
    }

    if(USE_OUT_ARM_Y_PITCH==iXPitch60 || USE_OUT_ARM_Y_PITCH==iXPitchManual635)                                         //JerryYang 20251218: IN/OUT ARM support different modules
    {
        iOutArmXBase=2;
        iOutArmYBase=0;

        iOutArmShtXCenterPos=-2000;
        iOutArmShtYCenterPos=-3000;
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be  ||                              //Ztex 2023.12.06 Add HT-1032
       iInArmXBase==2)
    {
        if(BASE_X_TO_HP<6800)                                                   //Steven 20230826: reference-axis X to HotPlate edge distance
            BASE_X_TO_HP=6800;                                                  // >7000 : HT-9xxx, HT-1032AT
    }
    else
    {
        if(BASE_X_TO_HP>4000)                                                   // <0    : HT1040, HT-7080B
            BASE_X_TO_HP=4000;                                                  // <4000 : HT1028
    }

    //Isaac 20171204 (Steven): Xpitch40->50mm selectable X Pitch mechanism mode
    //==>
    USE_IN_OUT_ARM_X_PITCH      =CheckAndReadIniDataGeneral("System", "USE_IN_OUT_ARM_X_PITCH", iXPitch40mm);
    if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)
    {
        IN_OUT_ARM_X_PITCH_MIN=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MIN", 4000);
        IN_OUT_ARM_X_PITCH_MAX=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MAX", 12000);
        iXpitchMin          =1333;                                              //Steven 20180522: X-Pitch minimum
        iXpitchMinX2        =2666;
        iXpitchMinX3        =4000;
        iXpitchMax          =4000;
        iXpitchMaxX2        =8000;
        iXpitchMaxX3        =12000;
        iPitch_Max_minus_Min=iXpitchMaxX3-iXpitchMinX3;
    }
    else if(USE_IN_OUT_ARM_X_PITCH==iXPitch50mm)
    {
        IN_OUT_ARM_X_PITCH_MIN=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MIN", 4000);
        IN_OUT_ARM_X_PITCH_MAX=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MAX", 15000);
        iXpitchMin          =1333;                                              //Steven 20180522: X-Pitch minimum
        iXpitchMinX2        =2666;
        iXpitchMinX3        =4000;
        iXpitchMax          =5000;
        iXpitchMaxX2        =10000;
        iXpitchMaxX3        =15000;
        iPitch_Max_minus_Min=iXpitchMaxX3-iXpitchMinX3;
    }
    else if(USE_IN_OUT_ARM_X_PITCH==iXPitch16Pick)
    {
        IN_OUT_ARM_X_PITCH_MIN=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MIN", 7700);
        IN_OUT_ARM_X_PITCH_MAX=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MAX", 15400);
        iXpitchMin          =1100;                                              //Steven 20180522: X-Pitch minimum
        iXpitchMinX2        =2200;
        iXpitchMinX3        =3300;
        iXpitchMinX4        =4400;
        iXpitchMinX5        =5500;
        iXpitchMinX6        =6600;
        iXpitchMinX7        =7700;
        iXpitchMax          =2200;
        iXpitchMaxX2        =4400;
        iXpitchMaxX3        =6600;
        iXpitchMaxX4        =8800;
        iXpitchMaxX5        =11000;
        iXpitchMaxX6        =13200;
        iXpitchMaxX7        =15400;
        iPitch_Max_minus_Min=iXpitchMaxX7-iXpitchMinX7;
    }
    else                                                                        //JimmyChiu 20220708: add Auto X pitch
    {
        IN_OUT_ARM_X_PITCH_MIN=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MIN", 6000);                    //JimmyChiu 20220708: add Auto X pitch
        IN_OUT_ARM_X_PITCH_MAX=CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MAX", 12000);                   //JimmyChiu 20220708: add Auto X pitch
        if(IN_OUT_ARM_X_PITCH_MIN>IN_OUT_ARM_X_PITCH_MAX)
        {
            int itemp=IN_OUT_ARM_X_PITCH_MIN;
            IN_OUT_ARM_X_PITCH_MIN=IN_OUT_ARM_X_PITCH_MAX;
            IN_OUT_ARM_X_PITCH_MAX=itemp;
        }
        iXpitchMin          =IN_OUT_ARM_X_PITCH_MIN/3;                          //Steven 20180522: X-Pitch minimum
        iXpitchMinX2        =double(IN_OUT_ARM_X_PITCH_MIN)/3.0*2.0;
        iXpitchMinX3        =IN_OUT_ARM_X_PITCH_MIN;
        iXpitchMax          =IN_OUT_ARM_X_PITCH_MAX/3;
        iXpitchMaxX2        =double(IN_OUT_ARM_X_PITCH_MAX)/3.0*2.0;
        iXpitchMaxX3        =IN_OUT_ARM_X_PITCH_MAX;
        iPitch_Max_minus_Min=iXpitchMaxX3-iXpitchMinX3;
    }

    iXpitchMaxX1_MM=iXpitchMax/100.0;
    iXpitchMaxX2_MM=iXpitchMaxX2/100.0;
    iXpitchMaxX3_MM=iXpitchMaxX3/100.0;
    iXpitchMaxX4_MM=iXpitchMaxX4/100.0;
    iXpitchMaxX5_MM=iXpitchMaxX5/100.0;
    iXpitchMaxX6_MM=iXpitchMaxX6/100.0;
    iXpitchMaxX7_MM=iXpitchMaxX7/100.0;
    iXpitchMinX1_MM=iXpitchMin/100.0;
    iXpitchMinX2_MM=iXpitchMinX2/100.0;
    iXpitchMinX3_MM=iXpitchMinX3/100.0;
    iXpitchMinX4_MM=iXpitchMinX4/100.0;
    iXpitchMinX5_MM=iXpitchMinX5/100.0;
    iXpitchMinX6_MM=iXpitchMinX6/100.0;
    iXpitchMinX7_MM=iXpitchMinX7/100.0;
    //<==
    //Isaac 20171204 (Steven): Xpitch40->50mm selectable X Pitch mechanism mode

    AnsiString sVer=CheckAndReadIniDataGeneral("Version", "Ver", AnsiString(""));

    #ifdef ASE_KaohSiung
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20191225 (wei): version record fetchable remotely by customer
    {
        #ifdef HiSilicon
            if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&
               TestIF.bEnableAutoAlignment==true)                               //KenHsieh 20211015: ASE requires AOA appended to version string
                sVer=AnsiString(HISI_VERSION)+AnsiString(ASE_AOA);
            else
                sVer=AnsiString(HISI_VERSION)+AnsiString(HISI_ASE);             //kevin 20180514
        #else                                                                   //kevin 20190820: drop HT9046 version number
             if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&
                TestIF.bEnableAutoAlignment==true)                              //KenHsieh 20211015: ASE requires AOA appended to version string
                sVer=AnsiString(MainVersion)+AnsiString(ASE_AOA);
            else
                sVer=AnsiString(MainVersion)+AnsiString(HISI_ASE);
        #endif
        WriteIniDataGeneral("Version", "Ver",       sVer);                      //2014-05-30 Dell: for ATC6.0
        WriteIniDataGeneral("Version", "DefineVer", AnsiString(Record_Ver));    //2014-05-30 Dell: for ATC6.0
    }
    #endif

    //------------------------------------------
    INSTALL_HEAT_GUN        =CheckAndReadIniDataGeneral("System",       "INSTALL_HEAT_GUN",     0);                     //kevin 20120523: choose AirGUN mechanism mode
    INSTALL_ATC_HEAT_GUN    =CheckAndReadIniDataGeneral("System",       "INSTALL_ATC_HEAT_GUN", 0);                     //JerryYang 20220408: add for ATC3.5
    USE_HOTPLATE_TYPE       =CheckAndReadIniDataGeneral("System",       "USE_HOTPLATE_TYPE",    0);                     //jou 2012-05-15: choose Hotplate Type
    USE_MAGNETIC_SCALE      =CheckAndReadIniDataGeneral("System",       "USE_MAGNETIC_SCALE",   0);                     //Steven 20160426: magnetic scale
    USE_IO_CHANGE_TOQUE     =CheckAndReadIniDataGeneral("IndexDriver",  "USE_IO_CHANGE_TOQUE",  0);                     //jou 2012-06-21: Enable index I/O Change Toque
    MOTION_CARD_TYPE        =CheckAndReadIniDataGeneral("System",       "MOTION_CARD_TYPE",     0);                     //Brian 20121015: choose Motion Card mode
    // AI(W906-GA1-B6) 20260804: gated -- COMMSPEED_20M (#define, golden's
    // Motor/mn200.h) has no ported header at all yet.  See file-head note.
#if 0 // TODO(GA1-B6): MOTIONNET_SPEED default COMMSPEED_20M -- not yet in any ported header (Motor/mn200.h untranslated driver header)
    MOTIONNET_SPEED         =CheckAndReadIniDataGeneral("System",       "MOTIONNET_SPEED",      COMMSPEED_20M);         //Steven 20181122: MNet speed needs to be editable
#endif
    IO_CARD_TYPE            =CheckAndReadIniDataGeneral("System",       "IO_CARD_TYPE",         0);                     //Brian 20121015: choose I/O Card mode
    USE_ReadIndex_TOQUE     =CheckAndReadIniDataGeneral("IndexDriver",  "USE_ReadIndex_TOQUE",  0);                     //kevin 20201027
    USE_INDEX_ARM_AXES      =CheckAndReadIniDataGeneral("IndexDriver",  "USE_INDEX_ARM_AXES",   0);                     //JimmyChiu 20220708: add Index Arm Axis

    if(IO_CARD_TYPE==NewIO_MN200 ||
       IO_CARD_TYPE==PCI_P64C64 || IO_CARD_TYPE==PCI1203_IO )                                               //Steven 20231218 HT7080B  //AI(W906-IOWEB-P4) 20260925: +PCI1203_IO（Jimmy 20260925 裁決 A；來源機台 wip/field-20260924 的 AI(W906-1203-IO1) 20260903）
    {
        LoadIoData();
    }

    TTL_CARD_TYPE           =CheckAndReadIniDataGeneral("System",       "TTL_CARD_TYPE",        0);                     //Steven 20121122: choose TTL Card mode
    if(TTL_CARD_TYPE==2)                                                        //Isaac 20210511: TTLRS232 board version check, 1 board
    {
        // AI(W906-GA1-B6) 20260804: gated -- TTLRS232VerCheck is declared/defined
        // in golden's MessageDef.h/.cpp, which has NO ported file at all yet
        // (not merely a gate inside an existing ported file). See file-head note.
#if 0 // TODO(GA1-B6): TTLRS232VerCheck -- not yet in any ported header (MessageDef.h/.cpp untranslated)
        TTLRS232VerCheck=7071601;                                               //07071601
#endif
    }
    TTL_CARD_USE_ADDRESS    =CheckAndReadIniDataGeneral("System",       "TTL_CARD_USE_ADDRESS",        0);              //Isaac 20210922: choose whether TTL board carries site number

    INSTALL_SOCKET_CLAMP    =CheckAndReadIniDataGeneral("System",       "INSTALL_SOCKET_CLAMP", 0);                     //JerryYang 20160523: retrofit to split type
    INSTALL_DOUBLE_EP       =CheckAndReadIniDataGeneral("System",       "INSTALL_DOUBLE_EP", 0);                        //Ifor 20190104: Add Double EP Control
    CHECK_EP_SETTING        =CheckAndReadIniDataGeneral("System",       "CHECK_EP_SETTING", 1);                         //Steven 20240701: EP-check feature gets an on/off switch
    bAutoTrackCanGoRear     =CheckAndReadIniDataGeneral("System",       "bAutoTrackCanGoRear",   0);                    //Isaac 20180109 (Steven): auto123 can go forward/back
    bNoAutoZSelect          =CheckAndReadIniDataGeneral("System",       "bAutoZNoUseART",   0);                         //kevin 20181023 add ART Auto 1 2 3: use single-stage cylinder k21 false use 2-stage cylinder
    IN_SHT_LAST_SENSOR      =CheckAndReadIniDataGeneral("System",       "IN_SHT_LAST_SENSOR",   0);                     //Steven 20181203: In Shuttle last-sensor definition
    USE_PRECISER            =CheckAndReadIniDataGeneral("System",       "USE_PRECISER",         0);                     //Frank 20180410 (Steven): InArm Preciser Station
    USE_InPlacement         =CheckAndReadIniDataGeneral("System",       "USE_InPlacement", eartUninstall);              //JimmyChiu 20220908 add Pickup Error Placement
    iPreciserInstallArea    =CheckAndReadIniDataGeneral("System", "iPreciserInstallArea", 0);                           //Ifor 20191008: add Preciser Install Area
    //Barcode_2D
    BOTTOM_2DID             =CheckAndReadIniDataGeneral("2D_BarCode", "BOTTOM_2DID",       ebctUninstall);              //Steven 20190308: Bottom 2D
    if(BOTTOM_2DID==1)
        BAR_CODE_INSTALL    =ebctUseCCDMode;
    else
        BAR_CODE_INSTALL    =CheckRange(CheckAndReadIniDataGeneral("2D_BarCode", "BAR_CODE_INSTALL", ebctUninstall), (int)ebctUninstall, (int)ebcUseOCR);       //Steven 20120921: Barcode_2D //Ifor 20190129: add Cognex EtherNet comm
    SHT_FLOATING_CHK        =CheckAndReadIniDataGeneral("2D_BarCode", "SHT_FLOATING_CHK",  ebctUninstall);              //Steven 20160920: IC displacement check
    BOTTOM_2DID_CCD         =CheckAndReadIniDataGeneral("2D_BarCode", "BOTTOM_2DID_CCD",   ebctUninstall);              //KaiChen 20200513: Bottom 2D 8CCD
    asBarCodeComPort[0]     =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode1_COM_PORT"     , AnsiString("COM15"));
    asBarCodeComPort[1]     =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode2_COM_PORT"     , AnsiString("COM16"));
    asBarCodeComPort[2]     =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode3_COM_PORT"     , AnsiString("COM17"));
    asBarCodeComPort[3]     =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode4_COM_PORT"     , AnsiString("COM18"));

//----------------------------------
    MACHINE_HAS_AUTO_ALIGNMENT_CCD  =CheckAndReadIniDataGeneral("System", "MACHINE_HAS_AUTO_ALIGNMENT_CCD", 0);         //ChungHung 20210113 add for Alignment CCD //KenHsieh 20210813: add CCD AUTO ALIGNMENT
//----------------------------------

    BAR_CODE_USECOUNT       =CheckRange(CheckAndReadIniDataGeneral("2D_BarCode", "BAR_CODE_USECOUNT", 4), 2, 4);        //Ifor 20190129: add COGNEX SYSTEM CCD Count //Steven 20190904: default changed to 4
    if(BAR_CODE_INSTALL==ebctEtherNetCCD)                                       //Ifor 20180104: add Cognex EtherNet comm
    {
        asCCDBarCodeIP[0]       =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1A_IP"     , AnsiString("172.16.8.210"));                           //Ifor 20151224 CCD 2D
        asCCDBarCodeIP[1]       =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1B_IP"     , AnsiString("172.16.8.211"));                           //Ifor 20151224 CCD 2D
        asCCDBarCodeIP[2]       =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2A_IP"     , AnsiString("172.16.8.212"));                           //Ifor 20151224 CCD 2D
        asCCDBarCodeIP[3]       =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2B_IP"     , AnsiString("172.16.8.213"));                           //Ifor 20151224 CCD 2D

        asCCDBarCodePort[0]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1A_Port"   , AnsiString("5001"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[1]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1B_Port"   , AnsiString("5001"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[2]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2A_Port"   , AnsiString("5001"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[3]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2B_Port"   , AnsiString("5001"));                                   //Ifor 20151224 CCD 2D
    }
    else
    {
        if(BAR_CODE_INSTALL==ebcUseOCR)                                         //Ifor 20210407 add: self-made OCR
        {
            asCCDBarCodeIP[0]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1A_IP"     , AnsiString("172.16.8.70"));                            //Ifor 20210407 add: self-made OCR
            asCCDBarCodeIP[1]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1B_IP"     , AnsiString("172.16.8.70"));                            //Ifor 20210407 add: self-made OCR
            asCCDBarCodeIP[2]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2A_IP"     , AnsiString("172.16.8.70"));                            //Ifor 20210407 add: self-made OCR
            asCCDBarCodeIP[3]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2B_IP"     , AnsiString("172.16.8.70"));                            //Ifor 20210407 add: self-made OCR
        }
        else
        {
            asCCDBarCodeIP[0]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1A_IP"     , AnsiString("172.16.8.200"));                           //Ifor 20151224 CCD 2D
            asCCDBarCodeIP[1]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1B_IP"     , AnsiString("172.16.8.200"));                           //Ifor 20151224 CCD 2D
            asCCDBarCodeIP[2]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2A_IP"     , AnsiString("172.16.8.200"));                           //Ifor 20151224 CCD 2D
            asCCDBarCodeIP[3]   =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2B_IP"     , AnsiString("172.16.8.200"));                           //Ifor 20151224 CCD 2D
        }
        asCCDBarCodePort[0]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1A_Port"   , AnsiString("5001"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[1]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_1B_Port"   , AnsiString("5002"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[2]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2A_Port"   , AnsiString("5003"));                                   //Ifor 20151224 CCD 2D
        asCCDBarCodePort[3]     =CheckAndReadIniDataGeneral("2D_BarCode",   "Use_CCDShuttle_2B_Port"   , AnsiString("5004"));                                   //Ifor 20151224 CCD 2D
    }
    //RogerYang 20180901 add Sun-Chip HuKou Demo AI CCD Function
    //==>
    asFix2BGAAICCDIP[0]     =CheckAndReadIniDataGeneral("Fix_AI_CCD",    "Fix_BGA_AI_CCD_IP"           , AnsiString("172.16.8.210"));
    asFix2BGAAICCDPort[0]   =CheckAndReadIniDataGeneral("Fix_AI_CCD",    "Fix_BGA_AI_CCD_Port"         , AnsiString("8000"));
    asFix2BGAAICCDIP[1]     =CheckAndReadIniDataGeneral("Fix_AI_CCD",    "Fix_BGA_AI_CCD_IP2"          , AnsiString("172.16.8.210"));
    asFix2BGAAICCDPort[1]   =CheckAndReadIniDataGeneral("Fix_AI_CCD",    "Fix_BGA_AI_CCD_Port2"        , AnsiString("8001"));
    asVisionLightPort       =CheckAndReadIniDataGeneral("Fix_AI_CCD",    "Vision_Light_PORT"           , AnsiString("COM8"));
    //<==
    //RogerYang 20180901 add Sun-Chip HuKou Demo AI CCD Function

    BarcodeBaudRate         =CheckAndReadIniDataGeneral("2D_BarCode",   "BaudRate"              , 9600);
    InBarcodeBaudRate       =CheckAndReadIniDataGeneral("2D_BarCode",   "InBaudRate"            , 9600);
    BarcodeByteSize         =CheckAndReadIniDataGeneral("2D_BarCode",   "ByteSize"              , 8);
    BarcodeStopBit          =CheckAndReadIniDataGeneral("2D_BarCode",   "StopBit"               , 1);
    BarcodeParity           =CheckAndReadIniDataGeneral("2D_BarCode",   "Parity"                , AnsiString("None"));

    asRFIDCom               =CheckAndReadIniDataGeneral("RFID",   "RFIDReader_PORT"       , AnsiString("COM15"));
    iRFIDBaudRate           =CheckAndReadIniDataGeneral("RFID",   "BaudRate"              , 9600);                      //Steven 20220713: RFID Reader for SJSEMI
    iRFIDByteSize           =CheckAndReadIniDataGeneral("RFID",   "ByteSize"              , 8);
    iRFIDStopBit            =CheckAndReadIniDataGeneral("RFID",   "StopBit"               , 1);
    sRFIDParity             =CheckAndReadIniDataGeneral("RFID",   "Parity"                , AnsiString("None"));

    asCCDTrayIP[0]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_ID_IP"           , AnsiString("172.16.8.200"));                            //wei 20161219 Tray Mapping
    asCCDTrayIP[1]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_Map_IP"          , AnsiString("172.16.8.200"));                            //wei 20161219 Tray Mapping
    asCCDTrayIP[4]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt1_IP"   , AnsiString("172.16.8.201"));                            //Sam 20190405: Tray Decive Count
    asCCDTrayIP[5]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt2_IP"   , AnsiString("172.16.8.201"));                            //Sam 20190405: Tray Decive Count
    asCCDTrayIP[6]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt3_IP"   , AnsiString("172.16.8.201"));                            //Sam 20190405: Tray Decive Count

    asCCDTrayIP[7]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_LoaderCarIP"   , AnsiString("172.16.8.151"));                      //JerryYang 20250220: change default IP
    asCCDTrayIP[8]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_Loader"        , AnsiString("172.16.8.150"));
    asCCDTrayIP[9]          =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO1Car_IP"   , AnsiString("172.16.8.152"));
    asCCDTrayIP[10]         =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO2Car_IP"   , AnsiString("172.16.8.153"));
    asCCDTrayIP[11]         =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO3Car_IP"   , AnsiString("172.16.8.154"));

    asCCDTrayPort[0]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_ID_Port"         , AnsiString("5101"));                                    //wei 20161219 Tray Mapping
    asCCDTrayPort[1]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_Map_Port"        , AnsiString("5102"));                                    //wei 20161219 Tray Mapping
    asCCDTrayPort[4]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt1_Port" , AnsiString("5104"));                                    //Sam 20190405: Tray Decive Count
    asCCDTrayPort[5]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt2_Port" , AnsiString("5105"));                                    //Sam 20190405: Tray Decive Count
    asCCDTrayPort[6]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_Tray_DeciveCnt3_Port" , AnsiString("5106"));                                    //Sam 20190405: Tray Decive Count
    asCCDTrayPort[7]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_LoaderCar_Port"  , AnsiString("23"));
    asCCDTrayPort[8]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_Loader_Port"     , AnsiString("23"));
    asCCDTrayPort[9]        =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO1Car_Port"   , AnsiString("23"));
    asCCDTrayPort[10]       =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO2Car_Port"   , AnsiString("23"));
    asCCDTrayPort[11]       =CheckAndReadIniDataGeneral("Tray_Mapping",    "Use_CoverTray_ID_AUTO3Car_Port"   , AnsiString("23"));

    //Sam 20181201: AutoAlignment
    //==>
    asCCDAlignIP[0]          =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_InTop_IP"              , AnsiString("172.16.110.201"));
    asCCDAlignIP[1]          =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_InBottom_IP"           , AnsiString("172.16.110.201"));
    asCCDAlignIP[2]          =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_OutTop_IP"             , AnsiString("172.16.110.201"));
    asCCDAlignIP[3]          =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_OutBottom_IP"          , AnsiString("172.16.110.201"));
    asCCDAlignPort[0]        =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_InTop_Port"            , AnsiString("5110"));
    asCCDAlignPort[1]        =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_InBottom_Port"         , AnsiString("5111"));
    asCCDAlignPort[2]        =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_OutTop_Port"           , AnsiString("5112"));
    asCCDAlignPort[3]        =CheckAndReadIniDataGeneral("Auto_Alignment",    "Use_AutoAlign_OutBottom_Port"        , AnsiString("5113"));
    //<==
    //Sam 20181201: AutoAlignment

    USE_TRAY_MAPPING        =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_TRAY_MAPPING",  etmUninstall), (int)etmUninstall, (int)etmDeviceRemain);    //RogerYang 20250906 range max 1->2   //wei 20161219 Tray Mapping
    USE_DIE_CLEAN           =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_DIE_CLEAN",  0), 0, 1);             //wei 20170418

    USE_LOADER_HINGE        =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_LOADER_HINGE",  0), 0, 1);          //wei 20170418
    USE_MR_SYSTEM           =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_MR_SYSTEM",  0), 0, 2);             ////Sam 20190112 LM //wei 20180702 MR
    USE_RFID_SYSTEM         =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_RFID_SYSTEM",  0), 0, 1);           //wei 20180702 MR
    USE_AUTO_ALIGNMENT      =CheckAndReadIniDataGeneral("System", "USE_AUTO_ALIGNMENT",    0);                          //Sam 20181201: AutoAlignment
    USE_COLORSENSOR_MUN     =CheckAndReadIniDataGeneral("System", "USE_COLORSENSOR_MUN",   eCSMUN_Uninstall);           //Jimmychiu 20230630: add color sensor MU-N in Loader
    LoaderUnload_StepMotor  =CheckAndReadIniDataGeneral("TrayY", "LoaderUnload_StepMotor", 0);                          //Steven 20200529: Loader Tray-in switched to stepper

    asRFIDComPort[0]        =CheckAndReadIniDataGeneral("RFID",   "RFID1_COM_PORT"        , AnsiString("COM15"));       //wei 20180726 RFID
    asRFIDComPort[1]        =CheckAndReadIniDataGeneral("RFID",   "RFID2_COM_PORT"        , AnsiString("COM16"));       //wei 20180726 RFID
    RFIDBaudRate            =CheckAndReadIniDataGeneral("RFID",   "BaudRate"              , 9600);                      //wei 20180726 RFID
    RFIDByteSize            =CheckAndReadIniDataGeneral("RFID",   "ByteSize"              , 8);                         //wei 20180726 RFID
    RFIDStopBit             =CheckAndReadIniDataGeneral("RFID",   "StopBit"               , 1);                         //wei 20180726 RFID
    RFIDParity              =CheckAndReadIniDataGeneral("RFID",   "Parity"                , AnsiString("None"));        //wei 20180726 RFID

    HOT_PLATE_POSITION      =CheckAndReadIniDataGeneral("System",       "HOT_PLATE_POSITION"    , 0);                   //Steven 20140222: HotPlate Pin position
    HOT_PLATE_LIMITATION    =CheckAndReadIniDataGeneral("System",       "HOT_PLATE_LIMITATION"  , 0);                   //Steven 20151117: HotPlate Pin limit position, can run 2x2 8-sucker
    USE_LASER_DISTANCE      =CheckAndReadIniDataGeneral("System",       "USE_LASER_DISTANCE"    , 0);                   //Steven 20140228: laser distance-measurement function
    USE_DEVICE_FLIPPER      =CheckAndReadIniDataGeneral("System",       "USE_DEVICE_FLIPPER"    , 0);                   //Frank 20210612: Flipper Function

    ION_PULSE_COUNT         =CheckRange(CheckAndReadIniDataGeneral("System",       "ION_PULSE_COUNT", 3000), 3000, 100000);                                     //Steven 20220107: alarm should fire sooner for ESD     //Steven 20230322: ION_PULSE_COUNT min raised to 3000
    FIX3_FULL_PLACE         =CheckRange(CheckAndReadIniDataGeneral("System",       "FIX3_FULL_PLACE",  0), 0, 5);       //Steven 20130126: Fix3 full-tray feature  //ChungHung 20140722 add for HT9046LA 2--->3 //JimmyChiu 20220927 4->5
    USE_ROTATE_KIT          =CheckRange(CheckAndReadIniDataGeneral("ROTATE_KIT",   "USE_ROTATE_KIT",           0), 0, 1);                                       //Steven 20121001: rotate kit
    iRotate_In_Index        =CheckRange(CheckAndReadIniDataGeneral("ROTATE_KIT",   "iRotate_In_Index",         0), -1, 1);                                      //Steven 20121001: rotate kit
    iRotate_Out_Tray6       =CheckRange(CheckAndReadIniDataGeneral("ROTATE_KIT",   "iRotate_Out_Index",        2), 0, 2);                                       //Fix tray must not be placed!
    iRotate_Type            =CheckRange(CheckAndReadIniDataGeneral("ROTATE_KIT",   "RotateKit_Type",           e1MotRotate), 0, eRotateTypeTotal-1);            //2013-04-12 Dell: rotate station; motor version //Frank 20180512 (Steven): modify 1 Dut 1 Motor 3 -> 5
    iSpecialSequence        =CheckRange(CheckAndReadIniDataGeneral("ROTATE_KIT",   "SpecialSequence",          0), 0, 1);                                       //2013-04-12 Dell: rotate station; motor version
    FIX3_INSTALL            =CheckRange(CheckAndReadIniDataGeneral("System",   "FIX3_INSTALL",  1), 0, 1);              //2013-07-16 Dell: 9046LS first unit has no Fix3; 2nd+ Fix3 is standard
    CROSS_SENSOR_INSTALL    =CheckRange(CheckAndReadIniDataGeneral("System",   "CROSS_SENSOR_INSTALL",  0), 0, 1);      //2013-07-16 Dell: Shuttle cross sensor
    AUTO_SENSOR_INSTALL     =CheckRange(CheckAndReadIniDataGeneral("System",   "AUTO_SENSOR_INSTALL",  0), 0, 1);       //wei 20160914 Auto Shuttle Sensor
    ShuttleVibration_INSTALL=CheckRange(CheckAndReadIniDataGeneral("System",   "ShuttleVibration",  0), 0, 1);          //JerryYang 20171006 (wei) Shuttle vibration motor
    //already loaded above:  USE_TRAY_MAPPING        =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_TRAY_MAPPING",  0), 0, 2);         //wei 20161219 Tray Mapping
    ESD_Monitor             =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_ESD_Monior",  0), 0, 1);            //2013-07-16 Dell: use ESD monitor
    USE_NOVX3360            =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_NOVX3360",    0), 0, 1);            //Steven 20131127: use Simco ION fan
    USE_AutoCleanIonFan     =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_AutoCleanIonFan",    0), 0, 1);     //Isaac 20210609: IO-triggered IonFan needle-clean
    USE_KASUGA              =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_KASUGA",    0), 0, 1);              //Ifor 20150731: use Kasuga ION fan
    USE_OTD                 =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_OTD",  0), 0, 2);                   //Steven 20140312: OTD  //ChungHung 20140709 add 1--->2 distinguish software vs external control
    USE_PULSE_TYPE          =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_PULSE_TYPE",  0), 0, 1);            //Ifor 20180316: add Simco Use Pulse Type
    bHT9045S_USE2x4         =CheckRange(CheckAndReadIniDataGeneral("System",   "bHT9045S_USE2x4",    0), 0, 1);         //Ifor 20170308 (wei) add HT9045S 2x4-mode lock
    bEnable_KLT_Function    =CheckRange(CheckAndReadIniDataGeneral("System",   "bBarCodeRules",    0), 0, 1);           //Ifor 20180517: add BarCode encoding rule 0:ROC 1:CE
    i24V_PULSE_COUNT        =CheckRange(CheckAndReadIniDataGeneral("System",   "I24V_PULSE_COUNT", 500), 500, 1000);    //JerryYang 20190813 count minimum protection to avoid false alarm //Ifor 20190311: add 24V Monitor Use Pulse Type
    CHAMBER_USE_PULSE_TYPE  =CheckRange(CheckAndReadIniDataGeneral("System",   "CHAMBER_USE_PULSE_TYPE",  0), 0, 1);    //Ifor 20190422: add Chamber Use Pulse Type

    USE_DIE_CLEAN           =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_DIE_CLEAN",  0), 0, 1);             //wei 20170418
    if(CUSTOMER_CODE==CC_AMD_M)
    {
        #ifdef TFAMD_SUZHOU
            iAMD_Function       =CheckRange(CheckAndReadIniDataGeneral("System",   "iEnable_Function",    1), 0, 2);    //Ifor 20231222 add TFAMD Function 0:AMD(Singapore/US) 1:TFAMD(Suzhou) 2:TFAMD(Malaysia)
        #else
            #ifdef TFAMD_MALAYSIA
            iAMD_Function       =CheckRange(CheckAndReadIniDataGeneral("System",   "iEnable_Function",    2), 0, 2);    //Ifor 20231222 add TFAMD Function 0:AMD(Singapore/US) 1:TFAMD(Suzhou) 2:TFAMD(Malaysia)
            #else
            iAMD_Function       =CheckRange(CheckAndReadIniDataGeneral("System",   "iEnable_Function",    0), 0, 2);    //Ifor 20231222 add TFAMD Function 0:AMD(Singapore/US) 1:TFAMD(Suzhou) 2:TFAMD(Malaysia)
            #endif
        #endif
    }
    else
    {
        iAMD_Function=-1;
    }

    if(USE_SOCKET_SENSOR==0)                                                    //JerryYang 20200327: reorganize the 3rd comm module
    {
        SOCKET_AMP_QTY=0;
    }
    else
    {
        if(USE_COLOR_TRAY_SENSOR)
        {
            SOCKET_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty",  4), 0, iSnSocketCnt);  //Color sensor installed defaults to 4 ea
        }
        else
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                SOCKET_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty",  4), 0, iSnSocketCnt);                                      //kevin 20210907 default: Socket sensor switched to count
            else
                SOCKET_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty",  8), 0, iSnSocketCnt);                                      //Steven 20200610: Socket sensor switched to 16
        }
    }

    if(USE_ROTATE_KIT)
    {
        ROTATE_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "RotateSenAmpQty",  4), 0, 4);
    }
    else
    {
        ROTATE_AMP_QTY=0;
    }

    SOCKET_AMP_QTY_2nd=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty2nd",  0), 0, 16);
    SOCKET_AMP_QTY_3rd=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty3rd",  0), 0, 16);

    VibrationMotorCount=CheckRange(CheckAndReadIniDataGeneral("System",   "VibrationCardQty",  2), 0, 2);               //JerryYang 20230814: add vibration-motor comm speed-adjust version

    COLOR_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "ColorSenAmpQty",  6), 0, 8);                       //JerryYang 20200812: color sensor max count raised to 8
    //Sam 20210518: new CanBus software configuration
    //==>>
    NUDN1_QTY             =CheckRange(CheckAndReadIniDataGeneral("System",   "CanBusNudn1Qty",    2), 0, 4);
    NUDN1_MACID11_AMP_QTY =CheckRange(CheckAndReadIniDataGeneral("System",   "Nudn1Macid11Qty",  16), 0, 16);
    NUDN1_MACID12_AMP_QTY =CheckRange(CheckAndReadIniDataGeneral("System",   "Nudn1Macid12Qty",   4), 0, 16);
    NUDN1_MACID13_AMP_QTY =CheckRange(CheckAndReadIniDataGeneral("System",   "Nudn1Macid13Qty",   0), 0, 16);
    NUDN1_MACID14_AMP_QTY =CheckRange(CheckAndReadIniDataGeneral("System",   "Nudn1Macid14Qty",   0), 0, 16);
    //<==
    //Sam 20210518: new CanBus software configuration

    USE_OHT_SYSTEM          =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_OHT_SYSTEM",  0), 0, 1);            //KaiChen 20200716: OHT
    USE_Multile_Empty       =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_Multile_Empty",  0), 0, 1);         //KaiChen 20200716: OHT

    USE_KEYENCE_LOADER      =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_KEYENCE_LOADER",  0), 0, 1);        //wei 20161219 Tray Mapping
    USE_KEYENCE_EMPTY       =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_KEYENCE_EMPTY",   0), 0, 2);        //wei 20161219 Tray Mapping
    USE_MultileEmptyTrayID_Keyence=CheckRange(CheckAndReadIniDataGeneral("System",   "USE_MultileEmptyTrayID_Keyence",   0), 0, 1);

    if(CheckIniData(asGeneralPath, "System", "OTDRecord")==false)               //kevin 20150827 OTD ON_OFF RECORD
    {
        WriteIniDataGeneral("System", "OTDRecord", 0);
        bOTDRecord=false;
    }
    else
    {
        bOTDRecord            =CheckAndReadIniDataGeneral("System",   "OTDRecord",  0);                                 //kevin 20150827 OTD ON_OFF RECORD
    }
    USE_BARCODE_AS_KEYBOARD =CheckAndReadIniDataGeneral("System",  "USE_BARCODE_AS_KEYBOARD" , 0);                      //Steven 20140108: input only via Bar Code Reader
    USE_46_SUCKER_DB        =CheckAndReadIniDataGeneral("System",  "USE_46_SUCKER_DB",      0);                         //Steven 20131008: HT9045 uses 46-way pneumatics
    USE_46_SENSOR_DB        =CheckAndReadIniDataGeneral("System",  "USE_46_SENSOR_DB",      0);                         //Steven 20131008: HT9045 uses 46-way electrical distribution
    INDEX_MOTION_CARD       =CheckAndReadIniDataGeneral("System",  "INDEX_MOTION_CARD",     0);                         //Steven 20210623: Index uses Galil
    GALI_PROTECT_MODE       =CheckRange(CheckAndReadIniDataGeneral("Index", "GALI_PROTECT_MODE", 0), 0, 2);             //Steven 20260504: Index in-motion protect (0=Original, 2=TE)
    USE_FINE_PITCH          =CheckAndReadIniDataGeneral("System",  "USE_FINE_PITCH",        0);                         //Steven 20210624: Fine Pitch
    USE_OUT_SHT_MOT         =CheckAndReadIniDataGeneral("System",  "USE_OUT_SHT_MOT",       0);                         //Steven 20210624: Out shuttle independent motor
    USE_AIR_CONDITIONER     =CheckAndReadIniDataGeneral("AIR_CON", "USE_AIR_CONDITIONER",   0);                         //Steven 20131011: air conditioner
    asAirConPort            =CheckAndReadIniDataGeneral("AIR_CON", "AIR_CON_PORT",  AnsiString("COM15"));               //Steven 20131011: air conditioner
    USE_AOI_Inspection      =CheckRange(CheckAndReadIniDataGeneral("System", "AOI", 0), 0, 1);                          //2014-03-04 Dell: for SPIL WLP Add 5S Inspection
    USE_VibrationCommunication      =CheckRange(CheckAndReadIniDataGeneral("Vibration",   "VibrationCommunication",  0), 0, 1);                                 //JerryYang 20230814: add vibration-motor comm speed-adjust version

    USE_Scanner_AOI_Inspection =CheckRange(CheckAndReadIniDataGeneral("System",   "Scanner_AOI",  eBtnAOI_Uninstall), (int)eBtnAOI_Uninstall, (int)eBtnAOI_TopBottomInstall);     //Ifor 20190725: add Scanner AOI
    USE_Scanner_AOI_Inspection_FixLight_Z_Axis =CheckRange(CheckAndReadIniDataGeneral("System",   "Scanner_AOI_FixLight_Z_Axis",  0), 0, 1);  //Eastsun 20260410: AOI Fix Light Z Axis Mode
    USE_Fix_AI_CCD          =CheckRange(CheckAndReadIniDataGeneral("System",   "Fix_AI_CCD",  0), 0, 1);                //RogerYang 20180901 add Sun-Chip HuKou Demo AI CCD Function
    USE_Top_Scanner_AOI_Inspection =CheckRange(CheckAndReadIniDataGeneral("System",   "Top_Scanner_AOI",  0), 0, 1);    //Ifor 20200902 add: TFAMD Top AOI
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20180305 (wei): add KYEC/Qiaozhi forced-on requirement
    {
        USE_BARCODE_AS_KEYBOARD=1;
    }

    if(iRotate_Type!=eCynRotate)                                                //kevin 20130528 Rotate uses motor version
    {
        iRotate_In_Index=-1;                                                    //must be contended for
        iRotate_Out_Tray6=-1;
    }

    if(USE_ROTATE_KIT==1)
    {
        if(iRotate_Type==eCynRotate)
            USE_ROTATE=eCynRot;                                                 //Steven 20230930: unify the cylinder-version rotate-kit flag
        else
            USE_ROTATE=eMotRot;
    }
    else
    {
        USE_ROTATE=eNonRot;
    }

    T_MODE_SPEED            =CheckRange(CheckAndReadIniDataGeneral("System", "T_MODE_SPEED", 0.9), 0.5, 1.0);           //jou 2012-05-30 T Mode Speed
    SHUTTLE_Z_TYPE          =CheckAndReadIniDataGeneral("System", "SHUTTLE_Z_TYPE", 0);                                 //jou 2013-07-26: choose Shuttle Z Sensor Type
    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
       MachineTypeChoice==Type_HT9046_LS)                                       //kevin 20210616
    {
        SHUTTLE_Z_TYPE=1;
    }

    dFixTrayDataCleanTime   =CheckRange(CheckAndReadIniDataGeneral("System", "FixTrayDataCleanTime", 1.5), 0.5, 3.0);   //Ifor 20170525 (wei) add Fix Tray Data Clean Time
    iSocketBaseTempCount    =CheckAndReadIniDataGeneral("System", "SocketBasedAdd4Temp", 0);
    bHasEnteredPEModel      =CheckAndReadIniDataGeneral("System", "bHasEnteredPEModel", false);                         //Ifor 20160825 add entered-PE-mode-before flag
    if(MachineTypeChoice==Type_HT9046_LS &&
       (SubMachineType==Type_None ||                                            //Steven 20190307: Add for HT-9046LA
        SubMachineType==Type_HT9046AU ||                                        //Steven 20240822: For HT-9046AU
        SubMachineType==Type_HT9046CR))                                         //Steven 20241001: For HT-9046CR
    {
        iSocketBaseTempCount=eDut4ea;
    }

    if(MachineTypeChoice==Type_HT9046 ||
       MachineTypeChoice==Type_HT9046_LS ||
       MachineTypeChoice==Type_HT1032 || MachineTypeChoice==Type_HT9050)       //AI(W906-HT9050-FAM) 20260925: 加 Type_HT9050 -- golden 沒有 HT9050，照 HT9046 家族（使用者 20260925，RULINGS_20260925 第 5 條）。⚠ 照 golden 寫回 Gerneral.ini [System] USE_46_SUCKER_DB=0／USE_46_SENSOR_DB=0（每次 ReadGeneralIni 都寫，Model=9050GPIB 才進得來）
    {
        USE_46_SUCKER_DB=0;
        WriteIniDataGeneral("System", "USE_46_SUCKER_DB",      0);
        USE_46_SENSOR_DB=0;
        WriteIniDataGeneral("System", "USE_46_SENSOR_DB",      0);
    }

    iInArmAutoYTeachOffset   =CheckAndReadIniDataGeneral("AutoTeach",    "iInArmAutoYTeachOffset",      0);             //kevin 20161124 AutoTeach offset Y
    iOutArmAutoYTeachOffset  =CheckAndReadIniDataGeneral("AutoTeach",    "iOutArmAutoYTeachOffset",     0);             //kevin 20161124 AutoTeach offset Y

    iInArmTeachZ   =CheckAndReadIniDataGeneral("AutoTeach",    "iInArmTeachZ",      -1500);                             //kevin 20170502 (wei) auto teach Z check pos
    ioutArmTeachZ  =CheckAndReadIniDataGeneral("AutoTeach",    "ioutArmTeachZ",     -1500);                             //kevin 20170502 (wei) auto teach Z check pos

    USE_GROUND_MAN          =CheckRange(CheckAndReadIniDataGeneral("Ground_Man",   "USE_GROUND_MAN",  0), 0, 1);        //Steven 20190828: communication-based GroundMan
    asGroundManComPort      =CheckAndReadIniDataGeneral("Ground_Man", "Ground_Man_COM_PORT", AnsiString("COM18"));
    iGroundManScanPoint     =CheckRange(CheckAndReadIniDataGeneral("Ground_Man",   "Ground_Man_ScanPoint",  0), 0, 2);  //KaiChen 20191005: communication-based GroundMan //JerryYang 20210428: Add 28-point ground detection
    iGroundManAlarmOhm      =CheckRange(CheckAndReadIniDataGeneral("Ground_Man",   "Ground_Man_AlarmOhm",  0), 10, 1);  //KaiChen 20191005: communication-based GroundMan
    USE_KASUGA_Fan          =CheckRange(CheckAndReadIniDataGeneral("System",   "USE_KASUGA_Fan",  0), 0, 1);            //KaiChen 20191225: KASUGA Fan communication
    iUseHTIonBarFunction    =CheckRange(CheckAndReadIniDataGeneral("System",   "HTIonBarFunction", 0), 0, 3);           //RogerYang 20250825: Unloader adds 3 IonBars, replacing 4/5/8 ion fans
    iDBQueryDays            =CheckAndReadIniDataGeneral("System", "iDBQueryDays", 3);                                   //Ifor 20200324: add KYEC-requested DB Query day count

    AUTO3_IS_MAGAZINE        =CheckRange(CheckAndReadIniDataGeneral("System",   "AUTO3_IS_MAGAZINE",  0), 0, 1);        //JerryYang 20220909: add magazine
    iMagazineCheckZPos       =CheckRange(CheckAndReadIniDataGeneral("System",   "iMagazineCheckZPos",    400), 100, 1000);
    #ifdef FOR_EVAN                                                             //Steven 20210304: JCET's "Ru Chun" variant (customer codename)
        if(CUSTOMER_CODE==CC_JCET)
        {
            JCET_FOR_EVAN           =1;
            USE_BARCODE_AS_KEYBOARD =1;
            WriteIniDataGeneral("System", "USE_BARCODE_AS_KEYBOARD",  USE_BARCODE_AS_KEYBOARD);
            WriteIniDataGeneral("System", "JCET_FOR_EVAN",  JCET_FOR_EVAN);
        }
        else
        {
            JCET_FOR_EVAN           =0;
        }
    #else
        if(CUSTOMER_CODE==CC_JCET)
            JCET_FOR_EVAN           =CheckAndReadIniDataGeneral("System", "JCET_FOR_EVAN", 0);                          //Steven 20210304: JCET's "Ru Chun" variant (customer codename)
        else
            JCET_FOR_EVAN           =0;
    #endif

    //KenHsieh 20210830: add CCD AUTO ALIGNMENT for Offset
    //==>
    iAOA_InArm_Loader_X         =CheckAndReadIniDataGeneral("System", "AOA_InArm_Loader_X", 0);
    iAOA_InArm_Loader_Y         =CheckAndReadIniDataGeneral("System", "AOA_InArm_Loader_Y", 0);
    iAOA_InArm_Shuttle1_X       =CheckAndReadIniDataGeneral("System", "AOA_InArm_Shuttle1_X", 0);
    iAOA_InArm_Shuttle1_Y       =CheckAndReadIniDataGeneral("System", "AOA_InArm_Shuttle1_Y", 0);
    iAOA_InArm_Shuttle2_X       =CheckAndReadIniDataGeneral("System", "AOA_InArm_Shuttle2_X", 0);
    iAOA_InArm_Shuttle2_Y       =CheckAndReadIniDataGeneral("System", "AOA_InArm_Shuttle2_Y", 0);
    iAOA_InArm_Hotplate1_X      =CheckAndReadIniDataGeneral("System", "AOA_InArm_Hotplate1_X", 0);
    iAOA_InArm_Hotplate1_Y      =CheckAndReadIniDataGeneral("System", "AOA_InArm_Hotplate1_Y", 0);
    iAOA_InArm_Hotplate2_X      =CheckAndReadIniDataGeneral("System", "AOA_InArm_Hotplate2_X", 0);
    iAOA_InArm_Hotplate2_Y      =CheckAndReadIniDataGeneral("System", "AOA_InArm_Hotplate2_Y", 0);
    //<==
    //KenHsieh 20210830: add CCD AUTO ALIGNMENT for Offset
    //KenHsieh 20210918: CCD AUTO ALIGNMENT offset additions
    //==>
    iAOA_OutArm_Auto1_X         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto1_X", 0);
    iAOA_OutArm_Auto1_Y         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto1_Y", 0);
    iAOA_OutArm_Auto2_X         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto2_X", 0);
    iAOA_OutArm_Auto2_Y         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto2_Y", 0);
    iAOA_OutArm_Auto3_X         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto3_X", 0);
    iAOA_OutArm_Auto3_Y         =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto3_Y", 0);
    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20240428: Add for HT9011 AOA
    {
        iAOA_OutArm_Auto4_X     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto4_X", 0);
        iAOA_OutArm_Auto4_Y     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto4_Y", 0);
        iAOA_OutArm_Auto5_X     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto5_X", 0);
        iAOA_OutArm_Auto5_Y     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto5_Y", 0);
        iAOA_OutArm_Auto6_X     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto6_X", 0);
        iAOA_OutArm_Auto6_Y     =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Auto6_Y", 0);
    }
    iAOA_OutArm_Fix1_X          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix1_X", 0);
    iAOA_OutArm_Fix1_Y          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix1_Y", 0);
    iAOA_OutArm_Fix2_X          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix2_X", 0);
    iAOA_OutArm_Fix2_Y          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix2_Y", 0);
    iAOA_OutArm_Fix3_X          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix3_X", 0);
    iAOA_OutArm_Fix3_Y          =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix3_Y", 0);
    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20240428: Add for HT9011 AOA
    {
        iAOA_OutArm_Fix4_X      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix4_X", 0);
        iAOA_OutArm_Fix4_Y      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix4_Y", 0);
        iAOA_OutArm_Fix5_X      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix5_X", 0);
        iAOA_OutArm_Fix5_Y      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix5_Y", 0);
        iAOA_OutArm_Fix6_X      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix6_X", 0);
        iAOA_OutArm_Fix6_Y      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Fix6_Y", 0);
    }
    iAOA_OutArm_Shuttle1_X      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Shuttle1_X", 0);
    iAOA_OutArm_Shuttle1_Y      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Shuttle1_Y", 0);
    iAOA_OutArm_Shuttle2_X      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Shuttle2_X", 0);
    iAOA_OutArm_Shuttle2_Y      =CheckAndReadIniDataGeneral("System", "AOA_OutArm_Shuttle2_Y", 0);
    //<==
    //KenHsieh 20210918: CCD AUTO ALIGNMENT offset additions

    iControlPanelMode           =CheckAndReadIniDataGeneral("System", "ControlPanelMode", 0);                           //Ken 20210702 AddPadInterface
    VCCU_UNIT_TYPE              =CheckAndReadIniDataGeneral("System", "VacuUnitType", 0);                               //Sam 20230210: new VacuumUnit comm module
    USE_ATC_RS232_Check         =CheckRange(CheckAndReadIniDataGeneral("System", "USE_ATC_RS232_Check", 1), 0, 1);      //Ifor 20211116 add: ATC RS232 Check Function
    TRAY_MAPPING_GRAB           =CheckAndReadIniDataGeneral("System", "TRAY_MAPPING_GRAB", 0);                          //KenHsieh 20220923: add Tray Map Throw IC Function
    if(CUSTOMER_CODE ==CC_ASE_KaohSiung || CUSTOMER_CODE ==CC_AMD_M)            //kevin 20220901 ASE_KH close //Ifor 20240603 add
        USE_ARM_PROTECTION      =CheckAndReadIniDataGeneral("System", "USE_ARM_PROTECTION", false);                     //Steven 20220314: In/Out Arm Z Sensor protection gets a switch
    else
        USE_ARM_PROTECTION      =CheckAndReadIniDataGeneral("System", "USE_ARM_PROTECTION", true);                      //Steven 20220314: In/Out Arm Z Sensor protection gets a switch

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function ==>
    BASE_HEATER                 =CheckAndReadIniDataGeneral("System", "BASE_HEATER_COUNT",              0);
    SHUTTLE_FLOODGATE           =CheckAndReadIniDataGeneral("System", "SHUTTLE_FLOODGATE",              0);
    Tri_Temp_Machine            =CheckAndReadIniDataGeneral("System", "Tri_Temp_Machine",               0);
    AirStream_Select            =CheckAndReadIniDataGeneral("System", "AirStream_Select",               0);
    TriTemperature_TotalChannel =CheckAndReadIniDataGeneral("System", "TriTemperature_TotalChannel",    0);
    Tri_Temperature_MaxDegree   =CheckAndReadIniDataGeneral("System", "Tri_Temperature_MaxDegree",      175);
    Tri_Temperature_MinDegree   =CheckAndReadIniDataGeneral("System", "Tri_Temperature_MinDegree",      -55);
    SetHeaterTemp_MaxOutSht     =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxOutSht",        60);
    SetHeaterTemp_MaxIndex      =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxIndex",         60);
    SetHeaterTemp_MaxBase       =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxBase",          60);
    Total_Compressor            =CheckAndReadIniDataGeneral("System", "Total_Compressor",                3);
    INDEXDOORHEATER             =CheckAndReadIniDataGeneral("System", "IndexDoorHeater",                0);
    //Ztex 2023.04.19 Add HT-1032 TriTemp Function <==
    iAutoFormSize               =CheckAndReadIniDataGeneral("System", "iAutoFormSize",      0);                         //Jimmychiu 20231130: switch to allow Auto form size
    Enable_PLCSafety_IO         =CheckAndReadIniDataGeneral("System",  "SafePlcIO",         0);                         //Jason 20230619 add PLC_IO module + safety-related IO
    DOUBLE_BELT_MODE            =CheckAndReadIniDataGeneral("System",  "DOUBLE_BELT_MODE",  0);                         //Ztex 2023.12.13 Add Double Belt Mode
    USE_COVER_TRAYID            =CheckAndReadIniDataGeneral("System",  "USE_COVER_TRAYID",  tCIDNotUse);
    USE_LD_Rot_Arm              =CheckAndReadIniDataGeneral("System", "USE_LD_Rot_Arm",     0);                         //RogerYang 20250828: add for Loader Rotate Arm
    USE_E84_Sensor              =CheckAndReadIniDataGeneral("System", "AGVModal",     0);                               //Frank 20251204 add E84

    USE_LdUldCassetteMode       =CheckAndReadIniDataGeneral("System", "USE_LdUldCassetteMode",     0);                  //Frank 20251217 add
    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.09.14 Add Tri_Temp_Machine Do't Check
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            AMBIENT_TEMP_CHECK[i]=false;
        }
    }
    In_Shuttle_Auto_Latch      =CheckRange(CheckAndReadIniDataGeneral("System",   "In_Shuttle_Auto_Latch",  0), 0, 1);  //KenHsieh 20250722: InSht sensor changed to 2, using Latch to distinguish stacked vs airborne material
}

// ---------------------------------------------------------------------------
//  #if 0: InstallColorBinDisplay  (database.cpp:1684-1729)
// ---------------------------------------------------------------------------
#if 0 // TODO(wave-UI): InstallColorBinDisplay -- database.cpp:1684-1729
void SYSTEM_MODULAR::InstallColorBinDisplay(int iType)
{
    // TMyBinDispHT9046 / BinDisCtrl / eBinDispTotal / etc.
}
#endif // TODO(wave-UI)

// ===========================================================================
//  SYSTEM_MODULAR::LoadIoData()  (database.cpp:1548-1614)
//
//  Loads IO_Table.csv from IoTablePath (common.h extern).
//  Steps:
//    1. Set IoTablePath.
//    2. FileExists guard; ShowMyMessage if not found.
//    3. Allocate TStringList, delete + clear existing IOTable / mapIOTable
//       (memory-leak fix: Steven 20260612).
//    4. LoadFromFile.
//    5. Count<=1 -> ShowMyMessage data-lose.
//    6. SetIOTableNo(header row) -> iResult.
//    7. Gate: iResult==eioTotal(15) exactly.
//    8. For each data row i=1..Count-1:
//         push_back(new TIODATA(row)) then dedup map keyed by Alias.
//         Index arithmetic: IOTable[i-1] is the just-pushed row.
//    9. catch(...) -> MyDBIProcess + ShowMyMessage "opened by other software".
//   10. delete StrList.
// ===========================================================================
void SYSTEM_MODULAR::LoadIoData()
{
    int iResult;
    AnsiString Str;
    IoTablePath = getenv("W906_IOTABLE_PATH") ? AnsiString(getenv("W906_IOTABLE_PATH")) : AnsiString("D:\\HT9045\\System\\IO_Table.csv");      // database.cpp:1552 -- AI(W906-IOTABLE-SEAM) 20260924: 唯讀接縫，比照 :1775 的 W906_MOTTABLE_PATH；沒設 => golden 原字面，量產不變。使用者 20260924 裁決：路徑照 golden 不分機種，機台專屬的表放 machines/<機台>/，開發機要切機種才用這兩個接縫
    if(FileExists(IoTablePath))
    {
        TStringList *StrList=new TStringList();

        mapIOTable.clear();
        for(int i=0; i<(int)IOTable.size(); i++)         // Steven 20260612: fix memory leak
            delete IOTable[i];
        IOTable.clear();
        try
        {
            StrList->LoadFromFile(IoTablePath);

            if(StrList->Count<=1)
            {
                Str.sprintf("File %s data is lose!", IoTablePath);
                ShowMyMessage(Str, "注意！IO檔案的資料錯誤!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1568 原文（移植初期改成英文）
            }
            else
            {
                iResult=HSys.IoNo.SetIOTableNo(StrList->Strings[0]);
                if(iResult==HSys.IoNo.eioTotal)
                {
                    for(int i=1; i<StrList->Count; i++)
                    {
                        IOTable.push_back(new TIODATA(StrList->Strings[i]));
                        mapIOTableIter=mapIOTable.find(IOTable[i-1]->Alias);
                        Str=IOTable[i-1]->Alias;
                        if(Str!="")
                        {
                            if(mapIOTableIter==mapIOTable.end())
                            {
                                mapIOTable[IOTable[i-1]->Alias]=i-1;
                            }
                            else
                            {
                                Str.sprintf("IO %s alias is duplicated!", IOTable[i-1]->Alias);
                                ShowMyMessage(Str, "注意！IO名稱重複!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1589 原文（移植初期改成英文）
                            }
                        }
                    }
                }
                else
                {
                    Str.sprintf("File %s data is mistake! (%d)", IoTablePath, iResult);
                    ShowMyMessage(Str, "注意！IO檔案的資料錯誤!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1597 原文（移植初期改成英文）
                }
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "SYSTEM_MODULAR::LoadIoData");
            Str.sprintf("File %s is opened by other software!", IoTablePath);
            ShowMyMessage(Str, "注意！IO檔案被其他軟體開啟!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1605 原文（移植初期改成英文）
        }
        delete StrList;
    }
    else
    {
        Str.sprintf("File %s is not exist!", IoTablePath);
        ShowMyMessage(Str, "注意！IO檔案遺失!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1612 原文（移植初期改成英文）
    }
}

// ===========================================================================
//  SYSTEM_MODULAR::LoadMotData()  (database.cpp:1616-1682)
//
//  Symmetric to LoadIoData.  Key difference: gate uses >=emotTotal-1 (>=28)
//  instead of exact ==eioTotal.  (Steven 20231218 HT7080B -- tolerates one
//  missing column, e.g. SimulateSpeed.)
//  FAITHFULNESS: catch block logs "SYSTEM_MODULAR::LoadIoData" (copy-paste bug
//  at database.cpp:1671) -- preserved exactly.
// ===========================================================================
void SYSTEM_MODULAR::LoadMotData()
{
    AnsiString Str;
    int iResult;
    MotTablePath = getenv("W906_MOTTABLE_PATH") ? AnsiString(getenv("W906_MOTTABLE_PATH")) : AnsiString("D:\\HT9045\\System\\Mot_Table.csv");  // database.cpp:1620 -- AI(W906-BU-C2b) 20260917: read-only test seam (plan book option 甲); unset => golden literal, production unchanged
    if(FileExists(MotTablePath))
    {
        TStringList *StrList=new TStringList();

        mapMotTable.clear();
        for(int i=0; i<(int)MotTable.size(); i++)        // Steven 20260612: fix memory leak
            delete MotTable[i];
        MotTable.clear();
        try
        {
            StrList->LoadFromFile(MotTablePath);

            if(StrList->Count<=1)
            {
                Str.sprintf("File %s data is lose!", MotTablePath);
                ShowMyMessage(Str, "注意！Motor檔案的資料錯誤!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1636 原文（移植初期改成英文）
            }
            else
            {
                iResult=HSys.MotNo.SetMOTTableNo(StrList->Strings[0]);
                if(iResult>=HSys.MotNo.emotTotal-1)      // Steven 20231218 HT7080B: >= not ==
                {
                    for(int i=1; i<StrList->Count; i++)
                    {
                        MotTable.push_back(new TMOTDATA(StrList->Strings[i]));
                        mapMotTableIter=mapMotTable.find(MotTable[i-1]->No);
                        Str=MotTable[i-1]->No;
                        if(Str!="")
                        {
                            if(mapMotTableIter==mapMotTable.end())
                            {
                                mapMotTable[MotTable[i-1]->No]=i-1;
                            }
                            else
                            {
                                Str.sprintf("Motor %s alias is duplicated!", MotTable[i-1]->No);
                                ShowMyMessage(Str, "注意！Motor名稱重複!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1657 原文（移植初期改成英文）
                            }
                        }
                    }
                }
                else
                {
                    Str.sprintf("File %s data is mistake! (%d)", MotTablePath, iResult);
                    ShowMyMessage(Str, "注意！Motor檔案的資料錯誤!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1665 原文（移植初期改成英文）
                }
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "SYSTEM_MODULAR::LoadIoData"); // NOTE: copy-paste bug from database.cpp:1671 -- preserved faithfully
            Str.sprintf("File %s is opened by other software!", MotTablePath);
            ShowMyMessage(Str, "注意！Motor檔案被其他軟體開啟!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1673 原文（移植初期改成英文）
        }
        delete StrList;
    }
    else
    {
        Str.sprintf("File %s is not exist!", MotTablePath);
        ShowMyMessage(Str, "注意！Motor檔案遺失!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:1680 原文（移植初期改成英文）
    }
}

// ===========================================================================
//  TIODATA::TIODATA(AnsiString Str)  (database.cpp:1756-1903)
//
//  Row constructor.  SL->CommaText=Str does CSV parse (quote-aware).
//  VALID path: SL->Count >= 14 (note: >=14, not ==15; eioNote col is optional).
//  INVALID path (Count<14): all string fields="", all int fields=-1.
//    Exception: iInType=-1 in invalid path (vs. 0 in valid-empty path).
//
//  Column reads use name-resolved indices from HSys.IoNo (SetIOTableNo must
//  have been called before the first TIODATA ctor is invoked by LoadIoData).
//
//  Field parse order (matches database.cpp:1778-1880):
//    ISABase  FIRST (KenHsieh 20260421: ISABase needs to come first)
//    Lane     (empty -> -1; if eMotionNet -> bHasNullData)
//    ModuleType
//    IP       (empty -> -1; if eMotionNet -> bHasNullData)
//    Port     (empty -> -1 + bHasNullData;
//              non-empty: if ISABase in {1,2,4} -> HexStrToInt(.c_str())
//                         else atoi)
//    Bit      (empty -> -1 + bHasNullData)
//    InType   (empty -> 0, NOT -1)
//    Enable   (empty OR bHasNullData -> 0)
//    OnAlarmTime / OffAlarmTime / OnDelayTime / OffDelayTime  (empty -> -1)
//
//  Alias branches (database.cpp:1767-1776): all five branches assign the same
//  column index (eioAlias) -- the Cylinder_On/Off / Sucker_On/Off AnsiPos
//  checks are semantically dead but MUST be reproduced faithfully.
//
//  Tag and bDelete are always set (database.cpp:1900-1901).
// ===========================================================================
TIODATA::TIODATA(AnsiString Str)
{
    bool bHasNullData=false;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    if(SL->Count>=14)
    {
        _CommaText=Str;
        Type=SL->Strings[HSys.IoNo.eioType];

        // Alias -- all five branches assign the same index (dead-equal, preserved faithfully)
        // database.cpp:1767-1776
        if(Type.AnsiPos(AnsiString("Cylinder_On"))!=0)
            Alias=SL->Strings[HSys.IoNo.eioAlias];
        else if(Type.AnsiPos(AnsiString("Cylinder_Off"))!=0)
            Alias=SL->Strings[HSys.IoNo.eioAlias];
        else if(Type.AnsiPos(AnsiString("Sucker_On"))!=0)
            Alias=SL->Strings[HSys.IoNo.eioAlias];
        else if(Type.AnsiPos(AnsiString("Sucker_Off"))!=0)
            Alias=SL->Strings[HSys.IoNo.eioAlias];
        else
            Alias=SL->Strings[HSys.IoNo.eioAlias];

        // ISABase FIRST (KenHsieh 20260421) -- database.cpp:1778-1785
        if(AnsiString(SL->Strings[HSys.IoNo.eioISABase])==AnsiString(""))
        {
            iISABase=eMotionNet;
        }
        else
        {
            iISABase=atoi(AnsiString(SL->Strings[HSys.IoNo.eioISABase]).c_str());
        }

        // Lane -- database.cpp:1787-1796
        if(AnsiString(SL->Strings[HSys.IoNo.eioLane])==AnsiString(""))
        {
            iLane=-1;
            if(iISABase==eMotionNet)            // Steven 20231218 HT7080B
                bHasNullData=true;
        }
        else
        {
            iLane=atoi(AnsiString(SL->Strings[HSys.IoNo.eioLane]).c_str());
        }

        // ModuleType -- database.cpp:1798-1805
        if(AnsiString(SL->Strings[HSys.IoNo.eioModuleType])==AnsiString(""))
        {
            iModuleType=-1;
        }
        else
        {
            iModuleType=atoi(AnsiString(SL->Strings[HSys.IoNo.eioModuleType]).c_str());
        }

        // IP -- database.cpp:1807-1816
        if(AnsiString(SL->Strings[HSys.IoNo.eioIP])==AnsiString(""))
        {
            iIP=-1;
            if(iISABase==eMotionNet)            // Steven 20231218 HT7080B
                bHasNullData=true;
        }
        else
        {
            iIP=atoi(AnsiString(SL->Strings[HSys.IoNo.eioIP]).c_str());
        }

        // Port -- database.cpp:1818-1831
        // HEX rule: if ISABase in {eISABase(1), ePCI1735U(2), ePLCbase(4)} use HexStrToInt(.c_str())
        // else atoi.  Note: TIODATA uses HexStrToInt(.c_str()) -- the char* overload.
        if(AnsiString(SL->Strings[HSys.IoNo.eioPort])==AnsiString(""))
        {
            iPort=-1;
            bHasNullData=true;
        }
        else
        {
            if(iISABase==eISABase   ||          // Nickliu 20230310: add Suck ISABase Type
               iISABase==ePCI1735U  ||
               iISABase==ePLCbase)
                iPort=HexStrToInt(AnsiString(SL->Strings[HSys.IoNo.eioPort]).c_str());
            else
                iPort=atoi(AnsiString(SL->Strings[HSys.IoNo.eioPort]).c_str());
        }

        // Bit -- database.cpp:1833-1841
        if(AnsiString(SL->Strings[HSys.IoNo.eioBit])==AnsiString(""))
        {
            iBit=-1;
            bHasNullData=true;
        }
        else
        {
            iBit=atoi(AnsiString(SL->Strings[HSys.IoNo.eioBit]).c_str());
        }

        // InType -- database.cpp:1843-1850  NOTE: empty -> 0, NOT -1
        if(AnsiString(SL->Strings[HSys.IoNo.eioInType])==AnsiString(""))
        {
            iInType=0;
        }
        else
        {
            iInType=atoi(AnsiString(SL->Strings[HSys.IoNo.eioInType]).c_str());
        }

        // Enable -- database.cpp:1852-1859  empty OR bHasNullData -> 0
        if(AnsiString(SL->Strings[HSys.IoNo.eioEnable])==AnsiString("") || bHasNullData)
        {
            iEnable=0;
        }
        else
        {
            iEnable=atoi(AnsiString(SL->Strings[HSys.IoNo.eioEnable]).c_str());
        }

        // OnAlarmTime -- database.cpp:1861-1864
        if(AnsiString(SL->Strings[HSys.IoNo.eioOnAlarmTime])==AnsiString(""))
            iOnAlarmTime=-1;
        else
            iOnAlarmTime=atoi(AnsiString(SL->Strings[HSys.IoNo.eioOnAlarmTime]).c_str());

        // OffAlarmTime -- database.cpp:1866-1869
        if(AnsiString(SL->Strings[HSys.IoNo.eioOffAlarmTime])==AnsiString(""))
            iOffAlarmTime=-1;
        else
            iOffAlarmTime=atoi(AnsiString(SL->Strings[HSys.IoNo.eioOffAlarmTime]).c_str());

        // OnDelayTime -- database.cpp:1871-1875  (blank line in original preserved as intent)
        if(AnsiString(SL->Strings[HSys.IoNo.eioOnDelayTime])==AnsiString(""))

            iOnDelayTime=-1;
        else
            iOnDelayTime=atoi(AnsiString(SL->Strings[HSys.IoNo.eioOnDelayTime]).c_str());

        // OffDelayTime -- database.cpp:1877-1880
        if(AnsiString(SL->Strings[HSys.IoNo.eioOffDelayTime])==AnsiString(""))
            iOffDelayTime=-1;
        else
            iOffDelayTime=atoi(AnsiString(SL->Strings[HSys.IoNo.eioOffDelayTime]).c_str());
    }
    else
    {
        // Count < 14: all fields blank / -1  (database.cpp:1883-1898)
        // NOTE: iInType=-1 here, contrasting iInType=0 in the empty-cell case above.
        _CommaText      ="";
        Type            ="";
        Alias           ="";
        iLane           =-1;
        iModuleType     =-1;
        iIP             =-1;
        iPort           =-1;
        iBit            =-1;
        iInType         =-1;
        iISABase        =-1;
        iEnable         =-1;
        iOnAlarmTime    =-1;
        iOffAlarmTime   =-1;
        iOnDelayTime    =-1;
        iOffDelayTime   =-1;
    }
    // Always set (database.cpp:1900-1901)
    Tag=HSys.IOTable.size();
    bDelete=false;
    delete SL;
}

// ===========================================================================
//  TIOTABLENO::TIOTABLENO()  (database.cpp:1905-1923)
//  Sets positional fallback indices.  Overwritten by SetIOTableNo name-scan.
// ===========================================================================
TIOTABLENO::TIOTABLENO()
{
    eioType             =0;
    eioAlias            =1;
    eioLane             =2;
    eioModuleType       =3;
    eioIP               =4;
    eioPort             =5;
    eioBit              =6;
    eioInType           =7;
    eioISABase          =8;
    eioEnable           =9;
    eioOnAlarmTime      =10;
    eioOffAlarmTime     =11;
    eioOnDelayTime      =12;
    eioOffDelayTime     =13;
    eioNote             =14;
    eioTotal            =15;
}

// ===========================================================================
//  TIOTABLENO::SetIOTableNo(AnsiString Str)  (database.cpp:1925-2045)
//
//  Name-based column resolver.  SL->CommaText=Str (header row CSV split).
//  Pattern per column:
//    field = -1;
//    for i in 0..Count-1: if AnsiString(SL->Strings[i]).AnsiPos(token)!=0 -> field=i  // LAST wins
//    if field==-1 -> bResult=<positional fallback index>
//
//  Guard: scan runs ONLY if SL->Count==eioTotal(15).
//    If Count!=eioTotal: bResult=false(0), skip entire scan.
//  Success: returns eioTotal(15) when all 15 columns resolve.
//
//  AnsiPos is 1-based substring; !=0 means "token found anywhere in cell".
//  No break in the inner loop -> LAST matching column wins.
// ===========================================================================
int TIOTABLENO::SetIOTableNo(AnsiString Str)
{
    int bResult=eioTotal;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    if(SL->Count!=eioTotal)
    {
        bResult=false;
    }
    else
    {
        eioType=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("IOType"))!=0)
                eioType=i;
        if(eioType==-1)
            bResult=0;

        eioAlias=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Alias"))!=0)
                eioAlias=i;
        if(eioAlias==-1)
            bResult=1;

        eioLane=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Lane"))!=0)
                eioLane=i;
        if(eioLane==-1)
            bResult=2;

        eioModuleType=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("ModuleType"))!=0)
                eioModuleType=i;
        if(eioModuleType==-1)
            bResult=3;

        eioIP=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("IP"))!=0)
                eioIP=i;
        if(eioIP==-1)
            bResult=4;

        eioPort=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Port"))!=0)
                eioPort=i;
        if(eioPort==-1)
            bResult=5;

        eioBit=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Bit"))!=0)
                eioBit=i;
        if(eioBit==-1)
            bResult=6;

        eioInType=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("InType"))!=0)
                eioInType=i;
        if(eioInType==-1)
            bResult=7;

        eioISABase=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("ISABase"))!=0)
                eioISABase=i;
        if(eioISABase==-1)
            bResult=8;

        eioEnable=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Enable"))!=0)
                eioEnable=i;
        if(eioEnable==-1)
            bResult=9;

        eioOnAlarmTime=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("OnAlarmTime"))!=0)
                eioOnAlarmTime=i;
        if(eioOnAlarmTime==-1)
            bResult=10;

        eioOffAlarmTime=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("OffAlarmTime"))!=0)
                eioOffAlarmTime=i;
        if(eioOffAlarmTime==-1)
            bResult=11;

        eioOnDelayTime=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("OnDelayTime"))!=0)
                eioOnDelayTime=i;
        if(eioOnDelayTime==-1)
            bResult=12;

        eioOffDelayTime=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("OffDelayTime"))!=0)
                eioOffDelayTime=i;
        if(eioOffDelayTime==-1)
            bResult=13;

        eioNote=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Note"))!=0)
                eioNote=i;
        if(eioNote==-1)
            bResult=14;
    }

    delete SL;
    return bResult;
}

// ===========================================================================
//  TMOTNO::TMOTNO()  (database.cpp:2047-2079)
//  Sets positional fallback indices.  Overwritten by SetMOTTableNo name-scan.
// ===========================================================================
TMOTNO::TMOTNO()
{
    emotNo              =0;
    emotAlias           =1;
    emotEnable          =2;
    emotBoardID         =3;
    emotPort            =4;
    emotIP              =5;
    emotGearRatio       =6;
    emotSoftLimitN      =7;
    emotSoftLimitP      =8;
    emotCardModel       =9;
    emotDirection       =10;
    emotHomeDirectior   =11;
    emotAcc             =12;
    emotDec             =13;
    emotInitSpeed       =14;
    emotHomeHighSpeed   =15;
    emotHomeLowSpeed    =16;
    emotJogHighSpeed    =17;
    emotJogLowSpeed     =18;
    emotRange           =19;
    emotRate            =20;
    emotServoAlarmOn    =21;
    emot1P2P            =22;
    emotSensorType      =23;
    emotEncodeType      =24;
    emotPickLimit       =25;
    emotLimitLogic      =26;
    emotIn1Logic        =27;
    emotSimulateSpeed   =28;
    emotTotal           =29;
}

// ===========================================================================
//  TMOTNO::SetMOTTableNo(AnsiString Str)  (database.cpp:2081-2298)
//
//  Name-based column resolver for Mot_Table.csv.
//  Differences from SetIOTableNo:
//    1. Init: bResult=emotTotal(29).
//    2. Count check: Count!=HSys.MotNo.emotTotal -> bResult=-2.
//       KEY: the `else` before the scan block is COMMENTED OUT at database.cpp:2091
//       (`//    else`), so the name-scan runs UNCONDITIONALLY even when Count
//       mismatches.  A real 29-col header resolves all columns and overwrites
//       bResult=-2; LoadMotData's gate (>=emotTotal-1 == >=28) then passes.
//    3. 29 logical columns instead of 15.
//    4. Token "HomeDirectior" (sic, misspelling in BCB6 -- preserved).
//    5. Success: returns emotTotal(29) when all 29 columns resolve.
//
//  AnsiPos substring, LAST-wins, no break -- same as SetIOTableNo.
// ===========================================================================
int TMOTNO::SetMOTTableNo(AnsiString Str)
{
    int bResult=emotTotal;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    if(SL->Count!=HSys.MotNo.emotTotal)
    {
        bResult=-2;
    }
//    else                    // database.cpp:2091 -- this `else` is COMMENTED OUT;
                              // scan runs unconditionally.  PRESERVED FAITHFULLY.
    {
        emotNo=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Motorname"))!=0)
                emotNo=i;
        if(emotNo==-1)
            bResult=0;

        emotAlias=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Alias"))!=0)
                emotAlias=i;
        if(emotAlias==-1)
            bResult=1;

        emotEnable=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Enable"))!=0)
                emotEnable=i;
        if(emotEnable==-1)
            bResult=2;

        emotBoardID=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("BoardID"))!=0)
                emotBoardID=i;
        if(emotBoardID==-1)
            bResult=3;

        emotPort=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Port"))!=0)
                emotPort=i;
        if(emotPort==-1)
            bResult=4;

        emotIP=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("IP"))!=0)
                emotIP=i;
        if(emotIP==-1)
            bResult=5;

        emotGearRatio=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("GearRatio"))!=0)
                emotGearRatio=i;
        if(emotGearRatio==-1)
            bResult=6;

        emotSoftLimitN=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("SoftLimitN"))!=0)
                emotSoftLimitN=i;
        if(emotSoftLimitN==-1)
            bResult=7;

        emotSoftLimitP=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("SoftLimitP"))!=0)
                emotSoftLimitP=i;
        if(emotSoftLimitP==-1)
            bResult=8;

        emotCardModel=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("CardModel"))!=0)
                emotCardModel=i;
        if(emotCardModel==-1)
            bResult=9;

        emotDirection=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Direction"))!=0)
                emotDirection=i;
        if(emotDirection==-1)
            bResult=10;

        emotHomeDirectior=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("HomeDirectior"))!=0) // sic: misspelling preserved
                emotHomeDirectior=i;
        if(emotHomeDirectior==-1)
            bResult=11;

        emotAcc=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Acc"))!=0)
                emotAcc=i;
        if(emotAcc==-1)
            bResult=12;

        emotDec=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Dec"))!=0)
                emotDec=i;
        if(emotDec==-1)
            bResult=13;

        emotInitSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("InitSpeed"))!=0)
                emotInitSpeed=i;
        if(emotInitSpeed==-1)
            bResult=14;

        emotHomeHighSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("HomeHighSpeed"))!=0)
                emotHomeHighSpeed=i;
        if(emotHomeHighSpeed==-1)
            bResult=15;

        emotHomeLowSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("HomeLowSpeed"))!=0)
                emotHomeLowSpeed=i;
        if(emotHomeLowSpeed==-1)
            bResult=16;

        emotJogHighSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("JogHighSpeed"))!=0)
                emotJogHighSpeed=i;
        if(emotJogHighSpeed==-1)
            bResult=17;

        emotJogLowSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("JogLowSpeed"))!=0)
                emotJogLowSpeed=i;
        if(emotJogLowSpeed==-1)
            bResult=18;

        emotRange=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Range"))!=0)
                emotRange=i;
        if(emotRange==-1)
            bResult=19;

        emotRate=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("Rate"))!=0)
                emotRate=i;
        if(emotRate==-1)
            bResult=20;

        emotServoAlarmOn=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("ServoAlarmOn"))!=0)
                emotServoAlarmOn=i;
        if(emotServoAlarmOn==-1)
            bResult=21;

        emot1P2P=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("1P2P"))!=0)
                emot1P2P=i;
        if(emot1P2P==-1)
            bResult=22;

        emotSensorType=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("SensorType"))!=0)
                emotSensorType=i;
        if(emotSensorType==-1)
            bResult=23;

        emotEncodeType=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("EncodeType"))!=0)
                emotEncodeType=i;
        if(emotEncodeType==-1)
            bResult=24;

        emotPickLimit=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("PickLimit"))!=0)
                emotPickLimit=i;
        if(emotPickLimit==-1)
            bResult=25;

        emotLimitLogic=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("LimitLogic"))!=0)
                emotLimitLogic=i;
        if(emotLimitLogic==-1)
            bResult=26;

        emotIn1Logic=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("In1Logic"))!=0)
                emotIn1Logic=i;
        if(emotIn1Logic==-1)
            bResult=27;

        emotSimulateSpeed=-1;
        for(int i=0; i<SL->Count; i++)
            if(AnsiString(SL->Strings[i]).AnsiPos(AnsiString("SimulateSpeed"))!=0)
                emotSimulateSpeed=i;
        if(emotSimulateSpeed==-1)
            bResult=28;
    }
    delete SL;
    return bResult;
}

// ===========================================================================
//  TMOTDATA::TMOTDATA(AnsiString Str)  (database.cpp:2300-2874)
//
//  Row constructor.  VALID path: SL->Count >= HSys.MotNo.emotTotal (29).
//  All field reads guard `HSys.MotNo.emotXxx < 0 || SL->Strings[emotXxx]==""`.
//  The <0 guard (Steven 20231218/20240103) treats a name-unresolved column
//  the same as an empty cell.
//
//  SPECIAL CASE (database.cpp:2353-2363):
//    INDEX_MOTION_CARD==0 AND Alias in {MTestY1, MTestZ1, MTestZ2, MTestY2}:
//    force CardModel="SMC", iBoardID=-1, iPort=-1, iIP=-1,
//    dAcc=1.0, dDec=1.0, iLimitLogic=0, iIn1Logic=0.
//    (Skip all those field reads.)
//
//  Port HEX rule for MC88X1 (database.cpp:2419):
//    HexStrToInt(SL->Strings[HSys.MotNo.emotPort])  -- AnsiString overload,
//    NOT .c_str().  Contrast TIODATA which uses .c_str().
//
//  IP only read if CardModel=="SYNTEK" (database.cpp:2424-2445); else iIP=-1.
//
//  PickLimit only read if Alias in {MTestZ1, MTestZ2} (database.cpp:2792-2810);
//  warn always (no iEnable guard here) -- preserved faithfully.
//
//  bHasNullData -> iEnable=0 at end of valid path (database.cpp:2835-2838).
//
//  INVALID path (Count<29): full set of safe defaults (database.cpp:2842-2872).
// ===========================================================================
TMOTDATA::TMOTDATA(AnsiString Str)
{
    AnsiString Msg;
    bool bHasNullData=false;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    if(SL->Count>=HSys.MotNo.emotTotal)
    {
        _CommaText=Str;

        // Enable (database.cpp:2311-2319)
        if(HSys.MotNo.emotEnable<0 ||           // Steven 20240103: add guard
           AnsiString(SL->Strings[HSys.MotNo.emotEnable])==AnsiString(""))
        {
            iEnable=0;
        }
        else
        {
            iEnable=atoi(AnsiString(SL->Strings[HSys.MotNo.emotEnable]).c_str());
        }

        // Motorname -> No (database.cpp:2321-2335)
        if(HSys.MotNo.emotNo<0 ||               // Steven 20240103: add guard
           AnsiString(SL->Strings[HSys.MotNo.emotNo])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("MotorName is NULL in %s", MotTablePath);
                ShowMyMessage(Msg, "注意！MotorName為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2327 原文（移植初期改成英文）
            }
            bHasNullData=true;
            No="";
        }
        else
        {
            No=SL->Strings[HSys.MotNo.emotNo];
        }

        // Alias (database.cpp:2337-2351)
        if(HSys.MotNo.emotAlias<0 ||            // Steven 20240103: add guard
           AnsiString(SL->Strings[HSys.MotNo.emotAlias])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("Alias of %s is NULL in %s", No, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達Alias為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2343 原文（移植初期改成英文）
            }
            bHasNullData=true;
            Alias="";
        }
        else
        {
            Alias=SL->Strings[HSys.MotNo.emotAlias];
        }

        // Special case: INDEX_MOTION_CARD==0 and Alias is an index motor
        // (database.cpp:2353-2363)
        if(INDEX_MOTION_CARD==0 && (Alias=="MTestY1" || Alias=="MTestZ1" || Alias=="MTestZ2" || Alias=="MTestY2"))
        {
            CardModel="SMC";
            iBoardID=-1;
            iPort=-1;
            iIP=-1;
            dAcc=1.0;
            dDec=1.0;
            iLimitLogic=0;
            iIn1Logic=0;
        }
        else
        {
            // CardModel (database.cpp:2366-2380)
            if(HSys.MotNo.emotCardModel<0 ||    // Steven 20231218 HT7080B
               AnsiString(SL->Strings[HSys.MotNo.emotCardModel])==AnsiString(""))
            {
                if(iEnable)
                {
                    Msg.sprintf("CardModel of %s is NULL in %s", Alias, MotTablePath);
                    ShowMyMessage(Msg, "注意！馬達CardModel為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2372 原文（移植初期改成英文）
                }
                bHasNullData=true;
                CardModel="";
            }
            else
            {
                CardModel=SL->Strings[HSys.MotNo.emotCardModel];
            }

            // BoardID (database.cpp:2382-2403)
            if(CardModel=="MC88X1")             // Steven 20231218 HT7080B
            {
                iBoardID=0;
            }
            else
            {
                if(HSys.MotNo.emotBoardID<0 ||
                   AnsiString(SL->Strings[HSys.MotNo.emotBoardID])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("Alias of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達Alias為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2394 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    iBoardID=-1;
                }
                else
                {
                    iBoardID=atoi(AnsiString(SL->Strings[HSys.MotNo.emotBoardID]).c_str());
                }
            }

            // Port (database.cpp:2405-2422)
            // MC88X1: HexStrToInt(AnsiString overload) -- NOT .c_str() (database.cpp:2419)
            if(HSys.MotNo.emotPort<0 ||         // Steven 20231218 HT7080B
               AnsiString(SL->Strings[HSys.MotNo.emotPort])==AnsiString(""))
            {
                if(iEnable)
                {
                    Msg.sprintf("Port of %s is NULL in %s", Alias, MotTablePath);
                    ShowMyMessage(Msg, "注意！馬達Port為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2411 原文（移植初期改成英文）
                }
                bHasNullData=true;
                iPort=-1;
            }
            else
            {
                if(CardModel=="MC88X1")         // Steven 20231218 HT7080B
                    iPort=HexStrToInt(SL->Strings[HSys.MotNo.emotPort]); // AnsiString overload
                else
                    iPort=atoi(AnsiString(SL->Strings[HSys.MotNo.emotPort]).c_str());
            }

            // IP: only read for SYNTEK (database.cpp:2424-2445)
            if(CardModel=="SYNTEK")
            {
                if(HSys.MotNo.emotIP<0 ||       // Steven 20240103: add guard
                   AnsiString(SL->Strings[HSys.MotNo.emotIP])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("IP of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達IP為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2432 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    iIP=-1;
                }
                else
                {
                    iIP=atoi(AnsiString(SL->Strings[HSys.MotNo.emotIP]).c_str());
                }
            }
            else
            {
                iIP=-1;
            }

            // Acc / Dec / LimitLogic / In1Logic: MC88X1 gets fixed values (database.cpp:2447-2519)
            if(CardModel=="MC88X1")             // Steven 20231218 HT7080B
            {
                dAcc=1.0;
                dDec=1.0;
                iLimitLogic=0;
                iIn1Logic=0;
            }
            else
            {
                // Acc (database.cpp:2456-2470)
                if(HSys.MotNo.emotAcc<0 ||
                   AnsiString(SL->Strings[HSys.MotNo.emotAcc])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("ACC of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達ACC為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2462 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    dAcc=1.0;
                }
                else
                {
                    dAcc=atof(AnsiString(SL->Strings[HSys.MotNo.emotAcc]).c_str());
                }

                // Dec (database.cpp:2472-2486)
                if(HSys.MotNo.emotDec<0 ||
                   AnsiString(SL->Strings[HSys.MotNo.emotDec])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("DEC of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達DEC為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2478 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    dDec=1.0;
                }
                else
                {
                    dDec=atof(AnsiString(SL->Strings[HSys.MotNo.emotDec]).c_str());
                }

                // LimitLogic (database.cpp:2488-2502)
                if(HSys.MotNo.emotLimitLogic<0 ||
                   AnsiString(SL->Strings[HSys.MotNo.emotLimitLogic])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("LimitLogic of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達LimitLogic為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2494 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    iLimitLogic=0;
                }
                else
                {
                    iLimitLogic=atoi(AnsiString(SL->Strings[HSys.MotNo.emotLimitLogic]).c_str());
                }

                // In1Logic (database.cpp:2504-2518)
                if(HSys.MotNo.emotIn1Logic<0 ||
                   AnsiString(SL->Strings[HSys.MotNo.emotIn1Logic])==AnsiString(""))
                {
                    if(iEnable)
                    {
                        Msg.sprintf("In1Logic of %s is NULL in %s", Alias, MotTablePath);
                        ShowMyMessage(Msg, "注意！馬達In1Logic為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2510 原文（移植初期改成英文）
                    }
                    bHasNullData=true;
                    iIn1Logic=0;
                }
                else
                {
                    iIn1Logic=atoi(AnsiString(SL->Strings[HSys.MotNo.emotIn1Logic]).c_str());
                }
            }
        } // end of not-INDEX_MOTION_CARD-index-motor else block

        // GearRatio (database.cpp:2522-2536)
        if(HSys.MotNo.emotGearRatio<0 ||        // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotGearRatio])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("GearRatio of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達GearRatio為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2528 原文（移植初期改成英文）
            }
            bHasNullData=true;
            dGearRatio=1.0;
        }
        else
        {
            dGearRatio=atof(AnsiString(SL->Strings[HSys.MotNo.emotGearRatio]).c_str());
        }

        // Direction (database.cpp:2538-2552)
        if(HSys.MotNo.emotDirection<0 ||        // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotDirection])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("Direction of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達Direction為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2544 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iDirection=0;
        }
        else
        {
            iDirection=atoi(AnsiString(SL->Strings[HSys.MotNo.emotDirection]).c_str());
        }

        // HomeDirectior (sic: misspelling preserved) (database.cpp:2554-2568)
        if(HSys.MotNo.emotHomeDirectior<0 ||    // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotHomeDirectior])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("HomeDirectior of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達HomeDirectior為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2560 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iHomeDirectior=0;
        }
        else
        {
            iHomeDirectior=atoi(AnsiString(SL->Strings[HSys.MotNo.emotHomeDirectior]).c_str());
        }

        // SoftLimitN (database.cpp:2570-2584)
        if(HSys.MotNo.emotSoftLimitN<0 ||       // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotSoftLimitN])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("SoftLimitN of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達SoftLimitN為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2576 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iSoftLimitN=-999999;
        }
        else
        {
            iSoftLimitN=atoi(AnsiString(SL->Strings[HSys.MotNo.emotSoftLimitN]).c_str());
        }

        // SoftLimitP (database.cpp:2586-2600)
        if(HSys.MotNo.emotSoftLimitP<0 ||       // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotSoftLimitP])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("SoftLimitP of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達SoftLimitP為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2592 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iSoftLimitP=999999;
        }
        else
        {
            iSoftLimitP=atoi(AnsiString(SL->Strings[HSys.MotNo.emotSoftLimitP]).c_str());
        }

        // InitSpeed (database.cpp:2602-2616)
        if(HSys.MotNo.emotInitSpeed<0 ||        // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotInitSpeed])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("InitSpeed of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達InitSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2608 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iInitSpeed=100;
        }
        else
        {
            iInitSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotInitSpeed]).c_str());
        }

        // HomeHighSpeed (database.cpp:2618-2632)
        if(HSys.MotNo.emotHomeHighSpeed<0 ||    // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotHomeHighSpeed])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("HomeHighSpeed of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達HomeHighSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2624 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iHomeHighSpeed=100;
        }
        else
        {
            iHomeHighSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotHomeHighSpeed]).c_str());
        }

        // HomeLowSpeed (database.cpp:2634-2648)
        if(HSys.MotNo.emotHomeLowSpeed<0 ||     // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotHomeLowSpeed])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("HomeLowSpeed of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達HomeLowSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2640 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iHomeLowSpeed=100;
        }
        else
        {
            iHomeLowSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotHomeLowSpeed]).c_str());
        }

        // JogHighSpeed (database.cpp:2650-2664)
        if(HSys.MotNo.emotJogHighSpeed<0 ||     // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotJogHighSpeed])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("JogHighSpeed of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達JogHighSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2656 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iJogHighSpeed=100;
        }
        else
        {
            iJogHighSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotJogHighSpeed]).c_str());
        }

        // JogLowSpeed (database.cpp:2666-2680)
        if(HSys.MotNo.emotJogLowSpeed<0 ||      // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotJogLowSpeed])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("JogLowSpeed of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達JogLowSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2672 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iJogLowSpeed=100;
        }
        else
        {
            iJogLowSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotJogLowSpeed]).c_str());
        }

        // Range: MC88X1 -> 10, else read (database.cpp:2682-2703)
        if(CardModel=="MC88X1")                 // Steven 20231218 HT7080B
        {
            iRange=10;
        }
        else
        {
            if(HSys.MotNo.emotRange<0 ||
               AnsiString(SL->Strings[HSys.MotNo.emotRange])==AnsiString(""))
            {
                if(iEnable)
                {
                    Msg.sprintf("Range of %s is NULL in %s", Alias, MotTablePath);
                    ShowMyMessage(Msg, "注意！馬達Range為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2694 原文（移植初期改成英文）
                }
                bHasNullData=true;
                iRange=1;
            }
            else
            {
                iRange=atoi(AnsiString(SL->Strings[HSys.MotNo.emotRange]).c_str());
            }
        }

        // Rate (database.cpp:2705-2719)
        if(HSys.MotNo.emotRate<0 ||             // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotRate])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("Rate of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達Rate為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2711 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iRate=1;
        }
        else
        {
            iRate=atoi(AnsiString(SL->Strings[HSys.MotNo.emotRate]).c_str());
        }

        // ServoAlarmOn (database.cpp:2721-2735)
        if(HSys.MotNo.emotServoAlarmOn<0 ||     // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotServoAlarmOn])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("ServoAlarmOn of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達ServoAlarmOn為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2727 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iServoAlarmOn=0;
        }
        else
        {
            iServoAlarmOn=atoi(AnsiString(SL->Strings[HSys.MotNo.emotServoAlarmOn]).c_str());
        }

        // 1P2P (database.cpp:2737-2751)
        if(HSys.MotNo.emot1P2P<0 ||             // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emot1P2P])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("1P2P of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達1P2P為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2743 原文（移植初期改成英文）
            }
            bHasNullData=true;
            i1P2P=0;
        }
        else
        {
            i1P2P=atoi(AnsiString(SL->Strings[HSys.MotNo.emot1P2P]).c_str());
        }

        // SensorType (database.cpp:2753-2767)
        if(HSys.MotNo.emotSensorType<0 ||       // Steven 20231218 HT7080B
           AnsiString(SL->Strings[HSys.MotNo.emotSensorType])==AnsiString(""))
        {
            if(iEnable)
            {
                Msg.sprintf("SensorType of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！馬達SensorType為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2759 原文（移植初期改成英文）
            }
            bHasNullData=true;
            iSensorType=0;
        }
        else
        {
            iSensorType=atoi(AnsiString(SL->Strings[HSys.MotNo.emotSensorType]).c_str());
        }

        // EncodeType: MC88X1 -> 0, else read (database.cpp:2769-2790)
        if(CardModel=="MC88X1")                 // Steven 20231218 HT7080B
        {
            iEncodeType=0;
        }
        else
        {
            if(HSys.MotNo.emotEncodeType<0 ||   // Steven 20231218 HT7080B
               AnsiString(SL->Strings[HSys.MotNo.emotEncodeType])==AnsiString(""))
            {
                if(iEnable)
                {
                    Msg.sprintf("EncodeType of %s is NULL in %s", Alias, MotTablePath);
                    ShowMyMessage(Msg, "注意！馬達EncodeType為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2781 原文（移植初期改成英文）
                }
                bHasNullData=true;
                iEncodeType=0;
            }
            else
            {
                iEncodeType=atoi(AnsiString(SL->Strings[HSys.MotNo.emotEncodeType]).c_str());
            }
        }

        // PickLimit: only for MTestZ1/MTestZ2 (database.cpp:2792-2810)
        // NOTE: warn without iEnable guard (no `if(iEnable)`) -- preserved faithfully.
        if(Alias=="MTestZ1" || Alias=="MTestZ2")
        {
            if(HSys.MotNo.emotPickLimit<0 ||    // Steven 20240103: add guard
               AnsiString(SL->Strings[HSys.MotNo.emotPickLimit])==AnsiString(""))
            {
                Msg.sprintf("Motor PickLimit of %s is NULL in %s", Alias, MotTablePath);
                ShowMyMessage(Msg, "注意！Motor PickLimit為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2798 原文（移植初期改成英文）
                iPickLimit=0;
                bHasNullData=true;
            }
            else
            {
                iPickLimit=atoi(AnsiString(SL->Strings[HSys.MotNo.emotPickLimit]).c_str());
            }
        }
        else
        {
            iPickLimit=0;
        }

        // SimulateSpeed: MC88X1 -> 1000, else read (database.cpp:2812-2833)
        if(CardModel=="MC88X1")                 // Steven 20231218 HT7080B
        {
            iSimulateSpeed=1000;
        }
        else
        {
            if(HSys.MotNo.emotSimulateSpeed<0 || // Steven 20231218 HT7080B
               AnsiString(SL->Strings[HSys.MotNo.emotSimulateSpeed])==AnsiString(""))
            {
                if(iEnable)
                {
                    Msg.sprintf("SimulateSpeed of %s is NULL in %s", Alias, MotTablePath);
                    ShowMyMessage(Msg, "注意！馬達SimulateSpeed為空!");   // AI(W906-DBMSG) 20260926: 標題照 golden database.cpp:2824 原文（移植初期改成英文）
                }
                bHasNullData=true;
                iSimulateSpeed=10000;
            }
            else
            {
                iSimulateSpeed=atoi(AnsiString(SL->Strings[HSys.MotNo.emotSimulateSpeed]).c_str());
            }
        }

        // Any missing field disables the motor (database.cpp:2835-2838)
        if(bHasNullData)
        {
            iEnable=0;
        }
    }
    else
    {
        // Count < 29: full set of safe defaults (database.cpp:2842-2872)
        _CommaText      ="";
        No              ="";
        Alias           ="";
        iEnable         =-1;
        iBoardID        =-1;
        iPort           =-1;
        iIP             =-1;
        dGearRatio      =1.0;
        iSoftLimitN     =-999999;
        iSoftLimitP     =999999;
        CardModel       ="";
        iDirection      =0;
        iHomeDirectior  =0;
        dAcc            =1.0;           // Steven 20231218 HT7080B
        dDec            =1.0;           // Steven 20231218 HT7080B
        iInitSpeed      =100;
        iHomeHighSpeed  =100;
        iHomeLowSpeed   =100;
        iJogHighSpeed   =100;
        iJogLowSpeed    =100;
        iRange          =1;
        iRate           =1;
        iServoAlarmOn   =0;
        i1P2P           =0;
        iSensorType     =0;
        iEncodeType     =0;
        iPickLimit      =0;
        iLimitLogic     =0;
        iIn1Logic       =0;
        iSimulateSpeed  =10000;
    }
    delete SL;
}

// ===========================================================================
//  LoadMachineConfig  --  AI(W906-WebBridge) 20260806.  NOT in golden.
//  Contract, and the reason this is not done from a constructor, are in
//  database.h at the declaration. Read that before changing anything here.
// ===========================================================================
bool LoadMachineConfig()
{
    // OpenGeneralIniFile() unconditionally closes first, so calling this twice
    // is safe; it is not, however, cheap -- it reparses the whole ini.
    OpenGeneralIniFile();

    if (INIFileGeneral == 0)
    {
        // Nothing below can run: every CheckAndReadIniDataGeneral would
        // dereference this. Fail loudly at the one place that can still
        // report it, rather than faulting deep inside ReadGeneralIni.
        return false;
    }

    // Fills HSys's own members from Gerneral.ini, sets CUSTOMER_CODE, and --
    // since the 20260806 ungate -- runs CustomerFunctionSelect() /
    // ReadLastSetIni() / ReadEventLogAutoSaveInfo(), which is what populates
    // IniConfig, the CosFunction profile flags, and LastSet (ReadLastDataFile
    // reads system\lastdata.dat as a raw struct blob -- verified working:
    // 12,358 non-zero bytes land in LastSet; see tests/test_wb_datalayer.cpp).
    HSys.ReadGeneralIni();

    return true;
}

// AI(W906-HT9050-ID) 20260924: machine.typeChoice／typeName／gpibModel 三個 tag 的來源（Steven c56af0b 定的介面，
//   規格 .claude/skills/ht9050-hw/references/machine-type-entry.md；宣告在 database.h 檔尾）。
//   W906_GpibModel：ReadGeneralIni() 在 :318 讀到的 D:\GPIB9045\system\general.ini [Version] Model。
//   不在白名單時也照實保留讀到的字串（例如 "ModelNG"），tag 反映的是「讀到什麼」，不是「認不認得」。
//   刻意沒有加進 IniConfig（Steven 信裡的建議形狀）：改那個結構會牽動所有 include 它的單元，
//   而它有沒有整塊進出二進位檔沒有量過；一個獨立的全域變數沒有這個風險。
AnsiString W906_GpibModel;

const char* W906_MachineTypeName(int t)
{
    switch (t) {
    case Type_HT9045:        return "Type_HT9045";
    case Type_HT9046:        return "Type_HT9046";
    case Type_HT9046_LS:     return "Type_HT9046_LS";
    case Type_HT9045_12Site: return "Type_HT9045_12Site";
    case Type_HT502:         return "Type_HT502";
    case Type_HT1032:        return "Type_HT1032";
    case Type_HT7080:        return "Type_HT7080";
    case Type_HT9050:        return "Type_HT9050";
    default:                 return "";
    }
}
