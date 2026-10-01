// =============================================================================
//  forms/fTesterTCP.h  --  non-VCL facade for golden's TfTesterTCP
//                          (Interface/TesterTCP.h)
//
//  AI(W906-FW3-TIF1) 20260828: new file, FW wave FW3-TIF1 (facade wave 1 of
//  Interface/TesterTCP -- read-only direction, no write path). Golden source
//  of truth: HT9011UC_Code_V3.33.906.0_20260618/Interface/TesterTCP.h
//  (267 lines) + Interface/TesterTCP.cpp (1117 lines). Decoded this wave with
//  `io.open(p, encoding='cp950')` (0 U+FFFD asserted over the whole file
//  before any quoting); every GATED body below is SLICED PROGRAMMATICALLY out
//  of that decode rather than retyped, so the transcript cannot drift.
//  Written out UTF-8 / bare-LF.
//
//  ============================================================================
//  *** READ THIS FIRST: 16 OF THE 23 GOLDEN MEMBERS ARE ALREADY TRANSLATED
//  ELSEWHERE IN THIS TREE, UNDER A DIFFERENT CLASS. ***
//  ============================================================================
//  `tools/census/wave_preflight.py Interface/TesterTCP.cpp` reports shapes
//  (a) class name / (c) global name / (d) TU-local seam all CLEAR -- and that
//  is TRUE but MISLEADING, because the existing port deliberately did NOT
//  reproduce golden's class. It split the form into two non-form units, both
//  in target `ht9045_sm`:
//
//    Interface/TesterTCP_Socket.{h,cpp}  (CMakeLists.txt:1736) -- 12 members.
//      They are FREE FUNCTIONS named `TesterTCPSocket_*`, operating on ONE
//      file-scope object `TesterTCPSocketState TesterTCPSocket;`
//      (TesterTCP_Socket.cpp:181; the struct is TesterTCP_Socket.h:164).
//      They are NOT methods of that struct -- so a search for
//      `TesterTCPSocketState::` finds only its ctor/dtor (:149/:175) and
//      reports the other 12 as missing. Verified body-by-body this wave:
//        golden ClientSocket_TCPIPConnect    :166-174  -> TesterTCPSocket_OnConnect                 :201
//        golden ClientSocket_TCPIPDisconnect :176-183  -> TesterTCPSocket_OnDisconnect              :216
//        golden TimerTCPIPConnectTimer       :185-239  -> TesterTCPSocket_TimerTCPIPConnectTimer    :275
//        golden SendTCPIPCommand             :241-274  -> TesterTCPSocket_SendTCPIPCommand          :331
//        golden AddTCPIPCommunicationLog     :276-300  -> TesterTCPSocket_AddTCPIPCommunicationLog  :374
//        golden btTCPIP_TriggerClick         :302-307  -> TesterTCPSocket_btTCPIP_TriggerClick      :773
//        golden btTCPIP_ConnectClick         :309-313  -> TesterTCPSocket_btTCPIP_ConnectClick      :779
//        golden btTCPIP_DisconnectClick      :315-320  -> TesterTCPSocket_btTCPIP_DisconnectClick   :786
//        golden ClientSocket_TCPIPRead       :322-345  -> TesterTCPSocket_OnRead                    :251
//        golden TimerProcessTCPDataTimer     :349-552  -> TesterTCPSocket_TimerProcessTCPDataTimer  :423
//        golden SimulateBin                  :554-613  -> TesterTCPSocket_SimulateBin               :708
//        golden ClientSocket_TCPIPError      :615-633  -> TesterTCPSocket_OnError                   :228
//      (`TesterTCPSocket_Init` :185 has no golden counterpart -- it is the
//      port's own extraction of the ctor's widget-array wiring.)
//      ⚠ FOUR of the 12 were RENAMED on the way over (Connect/Disconnect/
//      Error/Read -> On*), so name-only searches under-report the coverage.
//    Interface/TesterTCP.{h,cpp}      (CMakeLists.txt:1944) -- 4 members, also
//      FREE FUNCTIONS, with the widget reads promoted to parameters:
//        golden CopyOSTestResult      :660-697   -> TesterTCP_CopyOSTestResult      :455
//        golden CopyRecipeToTester    :1058-1077 -> TesterTCP_CopyRecipeToTester    :163
//        golden CopyRecipeFromTester  :1079-1097 -> TesterTCP_CopyRecipeFromTester  :220
//        golden btnSaveClick          :1099-1115 -> TesterTCP_btnSaveClick          :262
//
//  THIS WAVE THEREFORE DOES NOT RE-TRANSLATE THOSE 16. Copying their bodies
//  into a second file would be a duplicate transcript of the same golden text
//  in the same tree -- the exact hazard docs/KNOWLEDGE.md records as
//  "golden 的類別被拆到別的 port 類別；只搜同類別會把已翻的讀成缺". The 4
//  PRE-COMMITTED names that fall inside that set are DECLARED here (so the
//  waiting `#if 0` sites bind) and left DEFINITION-FREE, with a pointer to
//  the real body -- see GATE REGISTER (T-P1)..(T-P2).
//
//  ROLE
//  ----
//  TfTesterTCP is the Tester "OS / Open-Short" TCP/IP channel, used only when
//  CUSTOMER_CODE==CC_JSCC_OS (wei 20211027 "open short TCP/IP"; Steven
//  20230116 "OS測試機傳送工作檔名"). It owns the client socket, the inbound
//  protocol decoder, a 32-site simulate/bin mirror, and the OS test-report
//  generation family (per-tray TMemo -> .TXT on disk).
//
//  `python tools/census/wave_preflight.py Interface/TesterTCP.cpp`
//  (20260828, re-run at closeout): 23 bodies = 23 `TfTesterTCP::` members +
//  0 file-scope, 1068 span lines, 0 brace-swallow short-fall, 0 overload
//  collapse.
//  THIS WAVE, against the 23-member denominator:
//     1 ACTIVE-PARTIAL   (the ctor; 8 of its 86 golden lines are live)
//     6 GATED-WITH-BODY  (golden text translated into `#if 0` in the .cpp)
//    16 NOT RE-TRANSLATED (already ported elsewhere -- see the block above)
//  BY LINES: 8 of 1068 golden span lines are live (0.7%); 448 of 1068 (41.9%)
//  are carried as `#if 0` transcript; the remaining 612 (57.3%) already have
//  a port in ht9045_sm. Quote all three or none -- "6/23 translated" alone
//  would misrepresent both directions.
//
//  ============================================================================
//  *** THE GLOBALS `TfTesterTCP` / `fTesterTCP` ARE FREE -- THIS WAVE CLAIMS
//  THEM *** Measured 20260828 05:14 over the 1,199 git-tracked .cpp/.h/.hpp
//  files (`git ls-files` + per-file regex; build_*/ is untracked):
//    `TfTesterTCP` -- 40+ hits, ALL of them banner prose in
//      Interface/TesterTCP.{h,cpp} / Interface/TesterTCP_Socket.{h,cpp}
//      describing golden, plus one generated layout row
//      (tools/dfm2rc/layout_out/Interface/TesterTCP_layout.gen.cpp:12).
//      0 `class TfTesterTCP` definitions.
//    `fTesterTCP`  -- 40+ hits, ALL of them banner prose, `#define`d gate
//      stand-ins (Automation/SCK_ART_Remainder.cpp:243), or one of the 4
//      PRE-COMMITTED `#if 0` sites below. 0 global definitions.
//  `.h` declares `extern TfTesterTCP *fTesterTCP;`, `.cpp` defines
//  `TfTesterTCP *fTesterTCP = new TfTesterTCP();`.
//
//
//  CMakeLists.txt LINE NUMBERS WERE RE-MEASURED AFTER REGISTRATION.
//  Every `CMakeLists.txt:N` / `ht9045_xxx:N` citation in this file was first
//  taken at 20260828 05:10, then the integrating wave registered this facade's
//  .cpp into `add_library(ht9045_forms ...)` -- a 53-line insertion (46 lines, plus a later 7-line
//  correction by the integrator) at
//  CMakeLists.txt:924-976 -- which shifted EVERY citation from :935 onward by
//  +53 in total. They were re-measured TWICE -- after the registration, and
//  again after that 7-line correction -- each time READ OFF the file rather
//  than derived by adding an offset, because deriving is how off-by-N gets in.
//  All were
//  corrected. Citations at :272 / :485 / :699 sit ABOVE the insertion point and
//  are unchanged. Adding sources to ht9045_forms moves these numbers again --
//  re-measure, do not trust them blind.
//  ============================================================================
//  PRE-COMMITTED `#if 0` CALL SITES ELSEWHERE (4 names / 6 sites), each
//  verified by walking UP to its enclosing `#if` this wave -- none is live.
//  ============================================================================
//    SendTCPIPCommand(int,AnsiString,AnsiString)
//                          SECSGEM/uHGemHT9045.cpp:3399, :3400, :3401
//                          (enclosing `#if 0` at :3390, GATE [E2]) and :3417
//                          (GATE [E3]).  3 args at :3399/:3401, 3 at :3400 --
//                          all bind to `(int, AnsiString, AnsiString="")`.
//    Show()                forms/fLotInfo.cpp:2002 (enclosing `#if 0` at
//                          :1998, GATE WA-9).  Zero arguments.
//    rgUnloader            forms/fLotInfo.cpp:3479, read as
//                          `fTesterTCP->rgUnloader->ItemIndex` (enclosing
//                          `#if 0` at :3465) -- so it must be a POINTER to
//                          something carrying `ItemIndex`; golden
//                          TesterTCP.h:204 says `TRadioGroup *rgUnloader;`.
//    bOutArmVariAuto_OS    aoutarm.cpp:2956, read as
//                          `fTesterTCP->bOutArmVariAuto_OS[iAuto]` (enclosing
//                          `#if 0 // GATE G11` at :2955) -- golden
//                          TesterTCP.h:261 `bool bOutArmVariAuto_OS[3];`.
//  All 4 stay `#if 0` after this wave. Declaring the members does NOT un-gate
//  them; that is each owning file's own separate, deliberate edit and is
//  outside this wave's write boundary.
//
//  ============================================================================
//  GATE REGISTER (8)
//  ============================================================================
//  (T-P1) SendTCPIPCommand   golden :241-274 -- DECLARED, NOT DEFINED, AND
//        DELIBERATELY NOT RE-TRANSLATED. Two independent reasons.
//        (a) SAFETY POLICY: :271
//            `ClientSocket_TCPIP->Socket->SendText(Msg2+"\r\n");` puts bytes
//            on the wire. This wave is read-only direction.
//        (b) ALREADY PORTED: the body lives at
//            Interface/TesterTCP_Socket.cpp:331 as the FREE FUNCTION
//            `TesterTCPSocket_SendTCPIPCommand(int, AnsiString, AnsiString)`,
//            target ht9045_sm -- NOT as a method of TesterTCPSocketState.
//        Only the DECLARATION is supplied here, so the 4 waiting
//        uHGemHT9045.cpp sites have a matching signature. No second copy of
//        the body exists in this file -- on purpose.
//  (T-P2) Show()              golden: INHERITED `TForm::Show`, no golden body
//        in TesterTCP.cpp at all. Supplied as a PORT-ONLY no-op (DEVIATION
//        D-2), matching forms/fPrecaution.h D-3's precedent for exactly this
//        situation (a pre-committed call site that needs a real symbol and a
//        golden method that is pure VCL window management).
//  (T-1) ctor's widget-wiring blocks   golden :28-59 and :69-108 -- gated
//        INSIDE an otherwise-ACTIVE ctor (statement-level gating, same shape
//        as forms/fMonitor's `LoadTCPIPParament()` precedent). Three reasons:
//        (a) STATIC-INIT SAFETY (docs/KNOWLEDGE.md "static-init ctor 不可碰
//            NULL 全域", and the fLaserSensor incident: 88 of 134 ctest
//            SEGFAULTs). :73 reads `CUSTOMER_CODE` and :71/:81 read
//            `s6TrayName[]` -- both are ht9045_globals objects whose
//            initialisation order relative to this `new` is not guaranteed.
//        (b) MISSING MEMBERS: :80-87 and :91-97 set `->Name`, `->Parent`,
//            `->Height`, `->Align`, `->Font->Name`, `->Font->Color`,
//            `->Color`, `->ScrollBars`. vclcompat::TGroupBox carries only
//            `Caption` + TControl's Visible/Enabled/Left/Top/Tag
//            (vclcompat/Controls.h:308); vclcompat::TMemo carries only
//            `Lines`/`Text`/`Clear` (:367). None of Name/Parent/Height/Align/
//            Font/ScrollBars exists anywhere in vclcompat/Controls.h, and
//            adding them is a shared-file edit this wave is not allowed.
//        (c) 128 UNDECLARED WIDGETS: :29-47 name cbSite01..32, plSite01..32,
//            cbSiteOn01..32, labOcr01..32. Interface/TesterTCP.h's own banner
//            already classifies this as the "~140 widget-member construction
//            cluster ... no FormsFacade home, near-zero unique logic" and
//            leaves it out; this wave agrees rather than inventing 128 fields
//            no live body reads.
//  (T-2) FormDestroy          golden :111-123 -- TRANSITIVE ON (T-1). Its
//        whole body is `delete` of objects the GATED ctor block never
//        allocates: `RichEdit[iAuto]` / `GroupBox[iAuto]` (:120-121). Running
//        it as written would free nothing on the first loop (the arrays are
//        never populated) while `delete SocketTCPIPReceiveList` (:114) WOULD
//        free a list the ACTIVE ctor really made -- i.e. a half-live
//        destructor. Gating the whole body keeps golden's pairing intact.
//        NOT a link-boundary gate: nothing in it is unreachable.
//  (T-3) FormShow             golden :125-164 -- HIDDEN WRITE (x2).
//        (a) :147-149 `CheckAndReadIniData(szDir, "Output Arm",
//            "JSCC_OS_Use Auto1", 1)`. `CheckAndReadIniData` (common.cpp:603)
//            WRITES THE DEFAULT BACK when the key is missing
//            (common.cpp:1567-1569) -- it is NOT the pure-read `ReadIniData`
//            (common.cpp:692). Same finding forms/fHotPlate.h GATE (G-1)
//            records for ReadFile, re-verified independently this wave.
//        (b) :144 `MyForceDirectories(szDir);` CREATES DIRECTORIES on the
//            machine's data path.
//        Both would fire against `DataPath`/`sSaveByMachine`, i.e. the shared
//        production parameter tree -- exactly what the Gerneral.ini incident
//        of 20260817 cost. Not split: `pgcTcpIp->ActivePageIndex` (:132/:156)
//        and `fShow=true` (:163) would be safe alone, but the write is
//        unconditional inside the CC_JSCC_OS arm.
//  (T-4) rgUnloaderClick      golden :635-658 -- MISSING MEMBERS. Every loop
//        body sets `GroupBox[i]->Height` / `->Align` (and `alTop`/`alNone`/
//        `alClient`), none of which exist on vclcompat::TGroupBox or anywhere
//        in vclcompat/Controls.h. Also depends on the GATED (T-1) ctor to
//        have populated `GroupBox[]` at all, and on `Prod.iTrayType[]` /
//        `tNotUse`. Its NAME is nonetheless pre-committed indirectly: the
//        FIELD `rgUnloader` is read by forms/fLotInfo.cpp:3479, so the field
//        is declared even though this handler is not defined.
//  (T-5) PlaceOSTestResultToTray  golden :699-739 -- WRITES DISK. :738
//        `RichEdit[iAuto]->Lines->SaveToFile(sTargetFileName);` plus :718
//        `DeleteFile(sSourceFileName);` (which DELETES a file) and :713
//        MyForceDirectories. Interface/TesterTCP.h's own banner already
//        flagged this one as explicitly out of scope for the same reason
//        (SourceMemo/RichEdit[] are real on-screen widgets).
//  (T-6) ProcessOSPrint       golden :741-962 (222L, the largest single body
//        in the file) -- WRITES DISK. :957
//        `redtSummary->Lines->SaveToFile(asFileName);` under
//        `if(bViewOnly==false)`, plus :757 MyForceDirectories, plus
//        TRANSITIVE on (T-7) at :960. Note :953-958 means a `bViewOnly==true`
//        call would NOT write -- but the transitive (T-7) call at :960 passes
//        bViewOnly straight through and (T-7) has the same shape, so
//        "just gate the write" is not available without inventing a split.
//        `redtSummary` is a golden `TRichEdit*` and `class TRichEdit` has
//        ZERO port in this tree (scan over the 1,199 tracked sources,
//        20260828: 0 hits) -- an independent second gate.
//  (T-7) ProcessOSTrayData    golden :964-1056 -- WRITES DISK. :1035/:1039/
//        :1045/:1054 are four separate `SaveToFile` calls, :969
//        MyForceDirectories. Also depends on `RichEdit[]`/`SourceMemo` from
//        the GATED (T-1) ctor.
//
//  ============================================================================
//  ACTIVE (1, partial) -- reachability evidence
//  ============================================================================
//   TfTesterTCP()  golden :24-109. LIVE portion = golden :27 and :61-67,
//        8 lines:
//            OSRecipe="";                                     (:27)
//            SocketTCPIPReceiveList = new TStringList;        (:61)
//            SummaryHead            = new TStringList;        (:62)
//            bEnableTCPIPChannelConnect = true;               (:63)
//            SimulateStart              = false;              (:64)
//            bTCPError                  = false;              (:65)
//            SocketTCPIPReceiveList->Clear();                 (:66)
//            SummaryHead->Clear();                            (:67)
//        Every one is a write to this object's OWN field, or an allocation of
//        a vclcompat::TStringList (vclcompat/TStringList.cpp, target
//        `vclcompat`, trivially-constructible, touches no global). No global
//        is read, no pointer global is dereferenced, no function outside this
//        object is called -- so the static-init `new TfTesterTCP()` in the
//        .cpp is SIOF-safe. The rest of golden's ctor is (T-1).
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//      Inside the `#if 0` transcripts it is dropped too (the ONLY edit made
//      to the sliced golden text), so the blocks read as this port would
//      write them.
//  (D-2) `Show()` is a PORT-ONLY no-op (`{}`) -- golden's own is
//      `TForm::Show`, and this facade has no TForm base (D-3). Same call as
//      forms/fPrecaution.h D-3, for the same reason (a pre-committed
//      zero-argument site at forms/fLotInfo.cpp:2002).
//  (D-3) `TfTesterTCP` has NO base class (matches forms/fPrecaution.h D-4 /
//      forms/fHotPlate.h D-2).
//  (D-4) `FormDestroy(TObject*)` / `FormShow(TObject*)` / `rgUnloaderClick(
//      TObject*)` keep their parameters (no unportable type involved).
//      `ProcessOSPrint(bool bViewOnly=false)` /
//      `ProcessOSTrayData(bool bViewOnly=false)` keep golden's defaults
//      (TesterTCP.h:256-257).
//
//  ============================================================================
//  FIELD LIST (dfm leaf names / golden data members) -- and what is NOT here
//  ============================================================================
//  DECLARED:
//    rgUnloader              (TRadioGroup*)  golden :204 -- PRE-COMMITTED
//                                            (forms/fLotInfo.cpp:3479)
//    edTCPIP_Address         (TEdit*)        golden :30  -- PRE-COMMITTED
//    edTCPIP_Port            (TEdit*)        golden :31  -- PRE-COMMITTED
//      ⚠ These two became pre-committed DURING this same wave, by this wave's
//      OWN sibling file: forms/fTesterIF.cpp's GATE (F-5) transcript writes
//      `fTesterTCP->edTCPIP_Address->Text` / `->edTCPIP_Port->Text` (golden
//      cTesterIF.cpp:783-784). Caught only by re-running
//      tools/census/wave_preflight.py at CLOSEOUT (20260828 05:31), which is
//      exactly what trap #2 exists for -- the 05:14 run could not have seen
//      them because the file that creates the obligation did not exist yet.
//    bOutArmVariAuto_OS[3]   (bool)          golden :261 -- PRE-COMMITTED
//                                            (aoutarm.cpp:2956)
//    OSRecipe                (AnsiString)    golden :260 -- ACTIVE ctor
//    SocketTCPIPReceiveList  (TStringList*)  golden :235 -- ACTIVE ctor
//    SummaryHead             (TStringList*)  golden :246 -- ACTIVE ctor
//    bEnableTCPIPChannelConnect (bool)       golden :236 -- ACTIVE ctor
//    SimulateStart           (bool)          golden :243 -- ACTIVE ctor
//    bTCPError               (bool)          golden :253 -- ACTIVE ctor
//    bConnectOK / bConnect / fShow (bool)    golden :237/:238/:239
//    ErrorMessage / LastFileName (AnsiString) golden :252/:262
//  NOT DECLARED, deliberately (nothing LIVE reads them, and each would be a
//  claim this wave cannot back):
//    * the 128 per-site widgets cbSite01..32 / plSite01..32 / cbSiteOn01..32 /
//      labOcr01..32 and their 4 mirror arrays -- see GATE (T-1)(c).
//    * `ClientSocket_TCPIP` (TClientSocket*), `TimerTCPIPConnect` /
//      `TimerProcessTCPData` (TTimer*). These three are ALSO named by
//      forms/fTesterIF.cpp's GATE (F-5) transcript (:829-830, :835-836,
//      :858-860) and so are pre-committed in the same accidental way as the
//      two TEdits above -- but unlike them they CANNOT be declared faithfully
//      here, so they are left out ON PURPOSE and the obligation is recorded
//      instead. A real `Scktcomp::TClientSocket`
//      EXISTS (vclcompat/ClientSocket.h) -- but it is a live socket, and
//      `class TTimer` has NO vclcompat port at all (the three that exist,
//      ATC/ATCInterface.h:189 / Automation/uRENESAS_Server.h:168 /
//      BinDisplay/MyBinDisp.h, are TU-LOCAL stand-ins with incompatible
//      member sets -- reusing one from here would be exactly the
//      "TU-local stand-in" archive-extraction shape trap #1 warns about).
//    * `mmTCPIPCommLog` / `SourceMemo` / `redtSummary` / `scrlbxSummary` /
//      `RichEdit[]` / `GroupBox[]` / `bFlag[]` -- all touched only by GATED
//      bodies. `TRichEdit` and `TScrollBox` have no port anywhere
//      (0 hits, 20260828), so `redtSummary`/`scrlbxSummary` could not be
//      declared faithfully even if wanted.
//  A future wave that un-gates (T-1)..(T-7) must add these FIRST; the gate
//  text says so at each block.
//
//  ============================================================================
//  ZERO-WRITER FIELDS -- observable consequence, flagged per task rule 7
//  ============================================================================
//  `bOutArmVariAuto_OS[]` is WRITTEN only by the GATED FormShow (T-3) and
//  READ only by the still-gated aoutarm.cpp:2956. It is initialised here to
//  {false,false,false} (C++ default for a bool member left unmentioned by the
//  ACTIVE ctor). Golden's own default, had FormShow run, would be
//  {true,false,false} -- the `1` at golden :147 ("default auto 1 enable").
//  So un-gating aoutarm.cpp's G11 WITHOUT also supplying that value would
//  silently flip Auto1 off. Recorded here because it is the kind of value-
//  provenance trap docs/KNOWLEDGE.md "解 gate 前先查值從哪來" describes.
//  `rgUnloader->ItemIndex` is likewise 0 here; golden's ctor sets it to 1
//  (:101) -- inside GATED (T-1).
//
//  ============================================================================
//  ABSENCE-CLAIM PROVENANCE (trap #2: these expire)
//  ============================================================================
//  Every "0 hits" above was measured 20260828 05:14 with
//      git ls-files -- '*.cpp' '*.h' '*.hpp'   (1,199 files)
//  plus a per-file Python regex scan. Re-run at closeout.
// =============================================================================
#ifndef FORMS_FTESTERTCP_H
#define FORMS_FTESTERTCP_H

#include "forms/FormWidgets.h"   // AnsiString / TStringList / TRadioGroup / TEdit

// ===========================================================================
//  TfTesterTCP -- non-VCL facade (golden Interface/TesterTCP.h:17-263,
//  `class TfTesterTCP : public TForm`; NO base class here, DEVIATION D-3).
// ===========================================================================
class TfTesterTCP
{
public:
    TfTesterTCP();

    // -- golden __published widgets, PRE-COMMITTED ------------------------
    TRadioGroup *rgUnloader      = new TRadioGroup();       // golden :204, forms/fLotInfo.cpp:3479
    TEdit       *edTCPIP_Address = new TEdit();             // golden :30,  forms/fTesterIF.cpp:1028 (this wave's own GATE (F-5) transcript)
    TEdit       *edTCPIP_Port    = new TEdit();             // golden :31,  forms/fTesterIF.cpp:1029 (ditto)

    // -- golden public: User declarations ----------------------------------
    TStringList *SocketTCPIPReceiveList = 0;                // golden :235, allocated by the ACTIVE ctor (golden :61)
    TStringList *SummaryHead            = 0;                // golden :246, allocated by the ACTIVE ctor (golden :62)
    bool bEnableTCPIPChannelConnect = false;                // golden :236
    bool bConnectOK                 = false;                // golden :237
    bool bConnect                   = false;                // golden :238
    bool fShow                      = false;                // golden :239
    bool SimulateStart              = false;                // golden :243
    bool bTCPError                  = false;                // golden :253
    AnsiString ErrorMessage;                                // golden :252
    AnsiString OSRecipe;                                    // golden :260, Steven 20230116 : OS測試機傳送工作檔名
    AnsiString LastFileName;                                // golden :262
    // golden :261, RogerYang 20260210 -- PRE-COMMITTED (aoutarm.cpp:2956).
    // See ZERO-WRITER FIELDS: golden's real default for [0] is TRUE and comes
    // from the GATED FormShow (T-3), not from any initialiser.
    bool bOutArmVariAuto_OS[3] = {false, false, false};

    // -- PORT-ONLY no-op, pre-committed (forms/fLotInfo.cpp:2002) -----------
    void Show() {}                                          // DEVIATION D-2, GATE (T-P2)

    // -- DECLARED, NOT DEFINED, AND NOT RE-TRANSLATED HERE ------------------
    //    The real body is Interface/TesterTCP_Socket.cpp:330
    //    (TesterTCPSocketState::SendTCPIPCommand, ht9045_sm). See (T-P1).
    void SendTCPIPCommand(int index, AnsiString Msg, AnsiString Msg2="");  // golden :241-274  GATE (T-P1)

    // -- GATED this wave -- declared here, golden body translated in
    //    forms/fTesterTCP.cpp inside `#if 0`. Calling one is a LINK ERROR by
    //    design (see GATE REGISTER above). ----------------------------------
    void FormDestroy(TObject *Sender);                       // golden :111-123  GATE (T-2)
    void FormShow(TObject *Sender);                          // golden :125-164  GATE (T-3)
    void rgUnloaderClick(TObject *Sender);                   // golden :635-658  GATE (T-4)
    void PlaceOSTestResultToTray(int iSuckRow, int iSuckCol,
                                 int iTrayRow, int iTrayCol, int iAuto);   // golden :699-739  GATE (T-5)
    void ProcessOSPrint(bool bViewOnly=false);               // golden :741-962  GATE (T-6)
    void ProcessOSTrayData(bool bViewOnly=false);            // golden :964-1056 GATE (T-7)

    // NOT declared here because they ALREADY HAVE A PORT in ht9045_sm (see
    // the "READ THIS FIRST" block): ClientSocket_TCPIPConnect/Disconnect/
    // Read/Error, TimerTCPIPConnectTimer, TimerProcessTCPDataTimer,
    // AddTCPIPCommunicationLog, SimulateBin, btTCPIP_Trigger/Connect/
    // DisconnectClick, CopyOSTestResult, CopyRecipeToTester,
    // CopyRecipeFromTester, btnSaveClick.
};

// AI(W906-FW3-TIF1) 20260828: TfTesterTCP/fTesterTCP were FREE tree-wide (see
// banner's measured provenance) -- declared here so the 6 pre-committed
// `#if 0` sites (uHGemHT9045.cpp:3399/3400/3401/3417, forms/fLotInfo.cpp:2002
// and :3479, aoutarm.cpp:2956) have something real to bind to once each
// owning file's OWN gate is separately lifted (NOT done by this wave).
extern TfTesterTCP *fTesterTCP;

#endif // FORMS_FTESTERTCP_H
