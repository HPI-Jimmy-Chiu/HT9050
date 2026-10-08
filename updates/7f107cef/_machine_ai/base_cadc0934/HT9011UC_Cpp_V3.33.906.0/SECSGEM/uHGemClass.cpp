//---------------------------------------------------------------------------
//  SECSGEM/uHGemClass.cpp  --  HTGem abstract base (SECS/GEM S,F-handler layer)
//
//  Translation wave: W5 (SECSGEM base layer)
//  Translator: AI(W5-SECSGEM-Translate) 20260710
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemClass.cpp (3625 lines, cp950)
//
//  DISCREPANCY FROM THE ASSIGNED UNIT SCOPING -- READ THIS FIRST
//  ---------------------------------------------------------------------------
//  The unit brief characterized uHGemClass.cpp as "confirmed 0% VCL... only
//  needs vclcompat + a couple of default handler bodies that reference
//  csystem.h's predicate API".  On actually reading golden, that is NOT what
//  is here: HTGem itself never touches VCL directly (true, 0% VCL on ITS OWN
//  members), but 57 of its ~90 methods are near-single-purpose thin wrappers
//  that dereference `HGemPtr` (a `THGem*`) for almost every statement --
//  `HGemPtr->InitLocalHead(...)`, `HGemPtr->DataItemOut(...)`,
//  `HGemPtr->SendLocalData()`, and in a handful of cases direct VCL widget
//  reads THROUGH that pointer (`HGemPtr->strGrdCEID->Cells[...]`,
//  `HGemPtr->chkMoreMessageAbortProcess->Checked`).  `THGem` is defined in
//  uHGemEquipment.h/.cpp (~6200 lines combined) -- explicitly OUT OF SCOPE
//  for this unit (it is the actual byte-level SECS engine + VCL form, not the
//  "site-specific override layer" the brief meant to exclude -- that is
//  uHGemHT9045.h/.cpp, a DIFFERENT, also out-of-scope file).  None of the 57
//  methods reference csystem.h predicates at all (grep confirms zero
//  IndexHasIC/ShuttleHasIc/TestSocketHasIC hits in this file).
//
//  Golden reality: this file is a ~90%-by-body-volume adapter onto a
//  not-yet-translated subsystem.  Faithfully hand-transcribing every method
//  body here would require also fabricating THGem's ~101-distinct-member
//  interface from zero first-hand analysis of uHGemEquipment.h/.cpp -- i.e.
//  effectively also translating (or guessing at) that 6200-line VCL form,
//  which is explicitly out of scope and would not be a faithful translation
//  (it would be invented behavior, not golden-derived).
//
//  RESOLUTION (follows this project's established convention -- see
//  atester.cpp's GetTesterResult/ProcessTestResult, csystem.cpp's
//  DoOneCycleFinishCheck gated branches, etc.): every method whose real body
//  needs `HGemPtr`/`THGem` is GATED (`#if 0 // TODO(...) -- golden :L1-L2`,
//  body omitted -- golden already holds the citable source, no need to
//  duplicate 3000+ lines here) with an ACTIVE minimal stub that compiles
//  clean and returns a conservative/no-op default, so:
//    (a) HTGem is a real, instantiable, link-complete abstract base today
//        (every virtual has SOME body, matching its declared signature),
//    (b) the header's already-fully-inline virtuals (AddSV/AddEC/S7F4_.../
//        S7F24_.../S14F4_.../S110F*/S120F*/S125F4_...) carry over verbatim,
//        UNCHANGED, with zero gating (they were already 0%-coupled in
//        golden), and
//    (c) a future dedicated wave that translates uHGemEquipment.h (THGem)
//        can un-gate each block here mechanically, one at a time, citing the
//        golden line range already recorded in the `#if 0` comment.
//
//  ACTIVE in this file (faithful, zero gating -- genuinely 0%-VCL/0%-THGem):
//    * HTGem::HTGem() / HTGem(THGem*) / HTGem(AnsiString) / ~HTGem()
//      (SecsAlarmMessage / FMessageList TStringList lifetime only).
//    * HTGem::UpdateDataPath(AnsiString) (DataPath=Path;  one-liner).
//    * IsCorrectDateFormat(int,int,int) (free helper; pure leap-year check;
//      golden's only caller, S2F32_DateAndTimeAcknowledge, is itself gated --
//      kept active anyway per this project's "keep pure leaves active even
//      if their only caller is gated" precedent).
//    * SYS_ECChangeID / SYS_ECChangeIDOriginaValue / SYS_ECChangeIDNewValue
//      global AnsiString definitions (never read by ACTIVE code in this
//      file, but harmless, cheap, and matches golden 1:1).
//
//  GATED (23 of the original 57 methods remain gated as of INTEGRATE WAVE 4
//  below, `#if 0` + cited golden range + ACTIVE default stub) -- see each
//  stub's comment for its exact golden line range and (post integrate-wave)
//  an honest note on exactly which missing piece still blocks it.
//  AI(W906-SysModWire) 20260720: refreshed from "44" (INTEGRATE WAVE 2's own
//  count) -- INTEGRATE WAVE 3 un-gated 8 more methods (44-8=36, matching that
//  wave's own note below); grep `^#if 0` count re-verified at 36.
//  AI(W906-SvEcDataItem) 20260720: INTEGRATE WAVE 4 un-gated 13 more methods
//  (36-13=23, matching that wave's own note below); grep `^#if 0` count
//  re-verified at 23.
//
//  TRANSLATION RULES
//  ------------------
//    * __fastcall removed from S7F18's out-of-line definition (golden:
//      `void __fastcall HTGem::S7F18_...`) -- neutralized by vcl_compat.h
//      anyway; dropped per project convention (see .h file-head note).
//    * AnsiString / TStringList via vclcompat.
//    * MyDBIProcess: declared `extern` locally below (project convention --
//      see aHotPlateSubstrate.h:576 for the identical prior-wave forward
//      declaration; not `#include`d here to avoid pulling HotPlate-specific
//      globals into a file that otherwise needs none of them).
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8.  ZERO
//      U+FFFD (verified; see translate-agent report).
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE (AI(W906-uHGemClass-Unlock) 20260713) -- uHGemClass gate-unlock
//  ---------------------------------------------------------------------------
//  Two prerequisite THGem slices now exist as real, standalone, tested
//  classes: `SecsWireCodec` (byte-level SECS-II item codec -- InitLocalHead/
//  DataItemOut/DataItemIn/GetDataItemLenAndType/StringOut/LocalAcknowledge/
//  SendLocalData[gated]/...) and `SecsSvEcRegistration` (SV/EC registration
//  bookkeeping -- EC_ID/EC_TYPE/EC_Ptr_Max/EC_Ptr_Min/EC_Ptr_Max_Value/
//  EC_Ptr_Min_Value/...). HTGem now embeds one instance of EACH (`WireCodec`/
//  `SvEcReg`, see uHGemClass.h's own design-choice comment on those members).
//
//  Every one of the 57 gated methods was re-read against golden IN FULL (not
//  just its cited line range -- also golden uHGemEquipment.h's THGem member
//  list, to identify every field/method it touches) to determine whether its
//  ENTIRE dependency chain now resolves through WireCodec/SvEcReg alone.
//  REAL RESULT: 10 of 57 do (a much smaller fraction than "most" -- the
//  other 47 need at least one thing neither engine models: a StringGrid-
//  backed CEID/Report-ID/Alarm database, a VCL widget type not in vclcompat
//  [TPanel/TCustomEdit/TComboBox/TLabel/TCheckBox/TRadioGroup/TCheckListBox],
//  a THGem-only data member with no engine home [bOnLine, GemMDLN/GemSOFTREV/
//  GemClock, TraceData[10]/bTraceData[10]/iTRID[10]/..., SReceiveDataBackup,
//  MoveCheckCallBack, UpLoadPath, SV_70_UNT1_ReceipeStruct, ...], a THGem
//  method with no engine home [CheckSFFormatOnlyHead, OnLine/OffLine/
//  GetOnLineMode, GetTimeInfo, DataItemOutSV/-NameList/-NameListWithValue/
//  DataItemOutEC/-NameList, DefineReportAcknowledge*/LinkReportAcknowledge*/
//  EnableDisable*, IsValidSVID, SetReceipeDirectoryAndGlobalName, ...], the
//  as-yet-untranslated `GetDataItemLenAndTypeAndDelete` (SIBLING of the
//  peek-only `GetDataItemLenAndType` SecsWireCodec already has -- NOT the
//  same method; by far the single most common blocker, hit by ~20 of the 47),
//  or Borland dos.h `settime()`/`setdate()`/`struct time`/`struct date`
//  (S2F32) -- none of which SecsWireCodec/SecsSvEcRegistration were ever
//  scoped to cover, and none of which this wave invents a stand-in for.
//
//  UN-GATED (10, now calling WireCodec./SvEcReg. instead of HGemPtr->/HGem->):
//    S9F1_UnrecognizedDeviceID, S9F3_Unrecognized_Stream_Function_Type,
//    S9F5_UnrecognizedFunctionType, S9F7_IllegalData,
//    S9F9_TransactionTimerTimeout (golden :2222-2269, all 5 an identical
//      4-call StringOut+InitLocalHead+DataItemOut+SendLocalData shape);
//    S2F26_DiagnosticLoopbackData (golden :1006-1032);
//    S2F42_Host_Command_Acknowledge (golden :1523-1574, incl. its own golden
//      quirk -- the error path's DataItemOut calls with no preceding
//      InitLocalHead, preserved verbatim);
//    S2F44_ResetSpoolingAcknowledge (golden :1576-1584);
//    S2F24_TraceInitializeAcknowledge (golden :992-1000 -- a thin wrapper:
//      calls its own still-GATED …Sub() sibling + LocalAcknowledge, needs
//      nothing else, so it un-gates independently of its Sub);
//    CheckECValue (golden :3192-3560 -- the ONLY one of the 4
//      SetECValue/S2F15_Update/S2F15_Check/CheckECValue "EC value" cluster
//      with zero VCL-widget-cast branch and zero GetDataItemLenAndTypeAndDelete
//      call at the time of THIS wave; its 3 siblings stayed gated for exactly
//      those reasons back then -- see INTEGRATE WAVE 2 below for 2 of the 3
//      un-gating once their sole remaining blocker was translated).
//
//  Every un-gated method's `HGemPtr->X(...)`/`HGem->X(...)` call became
//  `WireCodec.X(...)` (wire-codec ops) or `SvEcReg.X(...)` (EC/SV lists);
//  `HType`/`HTypeStruct` (shared global, already visible via
//  SecsWireCodec.h) needed no change. Nothing else in these 10 bodies
//  changed vs. golden.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 2 (AI(W906-uHGemClass-Unlock2) 20260716) -- SecsWireCodec
//  "WAVE 3" unlocks S7F2 + the S2F15 pair
//  ---------------------------------------------------------------------------
//  SecsWireCodec gained 3 more real methods this wave (see SecsWireCodec.h/.cpp
//  own "WAVE 3" file-head notes): `GetDataItemLenAndTypeAndDeleteSub`/
//  `GetDataItemLenAndTypeAndDelete` (the destructive sibling pair of the
//  peek-only `GetDataItemLenAndType(Sub)` this file already had -- confirmed
//  by direct golden read to be pure SReceiveData bookkeeping, zero new THGem
//  member, zero VCL) and `SendInvalidDataMessageToHost` (a 3-call InitLocalHead
//  +DataItemOut+SendLocalData composer, same shape as the already-un-gated
//  S9F7_IllegalData). Re-reading every still-gated method's cited blocker list
//  against this delta found exactly 3 whose ENTIRE remaining dependency chain
//  now resolves through WireCodec (+ same-class virtual calls, which are not
//  an HGemPtr/HGem dependency and were already permitted for CheckECValue in
//  the prior wave):
//
//  UN-GATED (3 more, 13/57 total now):
//    S7F2_ProcessProgramLoadGrant (golden uHGemClass.cpp:2081-2113 -- sole
//      blocker was GetDataItemLenAndTypeAndDelete; error path was already
//      un-gated S9F7_IllegalData);
//    S2F15_UpdateNewEquipmentConstant (golden uHGemClass.cpp:2884-3024 -- both
//      GetDataItemLenAndTypeAndDelete and SendInvalidDataMessageToHost were its
//      only 2 recorded blockers; calls its own still-gated SetECValue, which
//      simply no-ops for now, exactly as CheckECValue's un-gating already
//      established as an acceptable interim state for this cluster);
//    S2F15_CheckNewEquipmentConstant (golden uHGemClass.cpp:3026-3190 -- same
//      2 blockers, same resolution; calls its own already-un-gated
//      CheckECValue).
//
//  STILL GATED, checked and confirmed NOT resolvable by this wave's delta
//  alone (each needs at least one more thing beyond WireCodec/SvEcReg, so
//  landing them would be forcing a partial translation through):
//    SetECValue itself (needs TPanel/TCustomEdit/TComboBox/TLabel/TCheckBox/
//      TRadioGroup dynamic_cast targets, none in vclcompat -- see its own
//      comment, unchanged reasoning);
//    S7F18_DeleteProcessProgramAcknowledge / Process_S7F20_CurrentEPPIDData /
//      S10F4/S10F6 TerminalDisplay* (need GetDataItemLenAndTypeAndDelete --
//      now available -- PLUS a VCL widget/THGem-only-member this wave's
//      SendInvalidDataMessageToHost addition does nothing for: UpLoadPath/
//      DeleteDirectory, TCheckListBox GemRemoteReceipeList,
//      TerminalDisplayIndex/TerminalMemoPtr/... respectively);
//    S2F13/.../S2F30/S2F34/S2F36/S2F38 SV/EC-registration-DB family (need
//      DataItemOutSV/-NameList/-EC/-NameList/-NameListWithValue, none of which
//      this wave translates);
//    S125F2_EnableDisableECDataAcknowledge (needs EnableDisableECDataAll/
//      EnableDisableECData, both StringGrid-backed -- sgSECSECData, confirmed
//      by direct golden read this wave (see stretch-goal recon below), out of
//      this file's scope).
//
//  STRETCH-GOAL RECON (AI(W906-uHGemClass-Unlock2) 20260716): the hand-off
//  also flagged 8 THGem methods as "worth a dedicated look" --
//  CheckSFFormatOnlyHead, IsValidSVID, GetTimeInfo,
//  SendInvalidDataMessageToHost, DeleteAllHostDefineReportID,
//  DeleteAllHostDefineCeid, GetAlarmIndex, EnableDisableAlarmAll/
//  EnableDisableAlarm, EnableDisableECDataAll/EnableDisableECData. Each was
//  read against golden uHGemEquipment.cpp in full:
//    SendInvalidDataMessageToHost: CLEAN (translated above into SecsWireCodec,
//      see its own citation).
//    CheckSFFormatOnlyHead: reads `chkMoreMessageAbortProcess->Checked` (VCL
//      TCheckBox, golden uHGemEquipment.cpp:8684) before ever reaching
//      GetDataItemLenAndTypeAndDelete -- a real VCL dependency this wave's
//      scope (SecsWireCodec.h/.cpp + uHGemClass.h/.cpp only) has no home for;
//      stays gated.
//    IsValidSVID: `SV_ID->IndexOf(SVID)` (golden uHGemEquipment.cpp:3114) --
//      SV_ID IS already a real SecsSvEcRegistration member (SvEcReg.SV_ID),
//      but SecsSvEcRegistration.h/.cpp is OUTSIDE this wave's allowed-file
//      list, so this method cannot be placed anywhere from here; moot anyway
//      since its only caller in this file (golden uHGemClass.cpp:810-985) is
//      blocked by 7 OTHER THGem-only members with no engine home regardless.
//    GetTimeInfo: writes THGem-only members (SystemYear/Month/Date/Hour/Min/
//      Sec/MSec, TimeString, GemClock, iTimeFormat, plus disk-space/memory-
//      status globals -- golden uHGemEquipment.cpp:315-346) that belong to
//      neither WireCodec nor SvEcReg's modeled scope and would be new,
//      unrelated state bolted onto one or the other; its only caller in this
//      file is ALSO blocked by CheckSFFormatOnlyHead above regardless --
//      stays gated.
//    DeleteAllHostDefineReportID / DeleteAllHostDefineCeid / GetAlarmIndex /
//      EnableDisableAlarmAll / EnableDisableAlarm / EnableDisableECDataAll /
//      EnableDisableECData: every one of these 7 iterates a live VCL
//      TStringGrid (stdGridReportID / strGrdCEID / strGrdAlarm / sgSECSECData
//      respectively, confirmed by direct golden read) -- exactly the
//      "StringGrid-backed CEID/Report-ID/Alarm database" category the prior
//      wave's note already named as out of WireCodec/SvEcReg's scope; none
//      translated this wave.
//  Net stretch-goal yield: 1 of 8 (SendInvalidDataMessageToHost) was
//  genuinely clean; the other 7 all hit a real, confirmed blocker (6 VCL
//  StringGrid, 1 VCL TCheckBox + THGem-only members) -- reported honestly
//  rather than forced through.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 3 (AI(W906-SysModWire) 20260720) -- SystemModularInitial
//  wiring wave: ActiveWire indirection + 8 more S,F handlers unlocked
//  ---------------------------------------------------------------------------
//  Design D (see design brief): HTGem gained a `SecsWireCodec *ActiveWire`
//  member (uHGemClass.h), defaulting to `&WireCodec` in all three ctors.
//  MECHANICAL RENAME: every occurrence of `WireCodec.` in the (now) 13
//  previously-un-gated ACTIVE method bodies below became `ActiveWire->`
//  (90 call sites; verified none touch `SvEcReg.`, which is untouched --
//  EC/SV registration state is HTGem's own, not a wire-codec concept, so it
//  does not route through the indirection). Standalone callers (this file's
//  own test, test_uHGemClass.cpp) are unaffected: ActiveWire defaults to
//  &WireCodec, so `hgem.WireCodec.*` pokes remain read by the SAME object
//  ActiveWire dereferences. Historical comments elsewhere in this file below
//  that say "now calling WireCodec." describe THAT wave's own before/after
//  (HGemPtr-> -> WireCodec.) and are left as their own historical record --
//  not rewritten to "ActiveWire->" -- since the code they narrate has, in
//  turn, been renamed uniformly by this note's own mechanical pass.
//
//  UN-GATED (8 more, 13->21/57 total now) -- golden SECSGEM/uHGemClass.cpp
//  line ranges cited at each definition below: S1F1_AreYouThereRequest,
//  S1F2_OnLineData, S1F13_EstablishCommunicationsRequest,
//  S1F14_ConnectRequestAcknowledge, Process_S1F14_ConnectRequestAcknowledge,
//  S1F16_OFFLINEAcknowledge, S1F18_ONLINEAcknowledge, S2F18_DateandTimeData.
//  Every one of their `HGemPtr->InitLocalHead/DataItemOut/DataItemIn/
//  SendLocalData/GetDataItemLenAndTypeAndDelete/StringOut` calls became
//  `ActiveWire->...` (wire-codec ops, same rule as the mechanical rename
//  above); `HGemPtr->GemMDLN/GemSOFTREV/bS1F2_OnLineData/bOnLine/
//  bReceiveEstablishCommunicationsRequest/bWaitEstablishCommunications.../
//  GemClock` (real THGem DATA members) and `HGemPtr->CheckSFFormatOnlyHead(...)/
//  OffLine()/OnLine(...)/GetOnLineMode()/GetTimeInfo()` (real THGem OUT-OF-LINE
//  METHODS, now defined in uHGemEquipment.cpp -- this is what makes this file
//  #include "SECSGEM/uHGemEquipment.h" for the FIRST time, see uHGemClass.h's
//  own updated forward-declaration note) all stay `HGemPtr->`, unchanged --
//  real THGem state/behavior, not a wire-codec concept. HGemPtr==NULL at call
//  time is undefined behavior, same as golden's own pre-AddSV construction
//  window (uHGemHT9045_SV.cpp:61 re-points HGemPtr=HGem there) -- not guarded,
//  by design (see design brief risk R8).
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 4 (AI(W906-SvEcDataItem) 20260720) -- SECSGEM closing-waves
//  Wave 1 (design doc DESIGN_SECSGEM_closing_waves.md): SvEcReg embed + SV/EC
//  DataItem family unlocks 13 more S,F handlers
//  ---------------------------------------------------------------------------
//  uHGemEquipment.h's THGem now embeds `SecsSvEcRegistration SvEcReg;` (by
//  value, mirroring HTGem's own already-proven WireCodec/SvEcReg precedent)
//  and gained 5 new real methods (DataItemOutSV/DataItemOutSVNameList/
//  DataItemOutSVNameListWithValue/DataItemOutEC/DataItemOutECNameList) +
//  IsValidSVID + the Report/CEID composer family (SendRepoerID/
//  SendAnnotatedRepoerID/SendCeid/SendAnnotatedCeid) + FormCreate (system SV/
//  EC registration). This resolves every remaining blocker for 8 methods in
//  the §2-category-② "small increment" bucket (S1F4, S1F12, S2F14, S2F30,
//  S6F16, S6F18, S6F20, S103F12) PLUS the 5 §2-category-① "no defect"
//  methods that were always resolvable through WireCodec/strGrdCEID/
//  strGrdAlarm/stdGridReportID alone and simply hadn't been visited yet
//  (S1F24, S5F8, S100F4, S101F6, S101F8) -- landed together in this one wave
//  per the design doc's own bucketing (§3 Wave 1 scope item 4).
//
//  UN-GATED (13 more, 21->34/57 total now; 36->23 remaining-gated) -- golden
//  SECSGEM/uHGemClass.cpp line ranges cited at each definition below:
//    S1F4_SelectedStatusReply (:119-264), S1F12_StatusVariableNamelistReply
//    (:266-324), S1F24_CollectionEventNamelist (:450-587),
//    S2F14_EquipmentConstanData (:609-719), S2F30_EquipmentConstantNamelistReply
//    (:1037-1080), S5F8_ListEnableAlarmAcknowledge (:1892-1923),
//    S6F16_EventReportData (:1925-1960), S6F18_AnnotatedEventReportData
//    (:1962-1997), S6F20_IndividualReportData (:1999-2052),
//    S100F4_ReportAllAlarm (:2437-2450), S101F6 (:2505-2513), S101F8
//    (:2583-2595), S103F12_StatusVariableNamelistReply (:3562-3625).
//
//  MECHANICAL RENAME RULE (same golden-derived split as prior waves): every
//  golden `HGemPtr->InitLocalHead/DataItemOut/DataItemIn/GetDataItemLenAndType/
//  GetDataItemLenAndTypeAndDelete/SendLocalData/LocalAcknowledge/
//  bDisableBinaryShow` (wire-codec primitives/state) became `ActiveWire->...`;
//  golden `HGemPtr->SV_ID`/`HGemPtr->EC_ID` (SV/EC bookkeeping, now living
//  one level down inside THGem's embedded SvEcReg rather than as THGem's own
//  direct members) became `HGemPtr->SvEcReg.SV_ID`/`HGemPtr->SvEcReg.EC_ID`
//  (still routes through HGemPtr -- SvEcReg IS THGem's own state, just
//  nested); golden `HGemPtr->DataItemOutSV/DataItemOutSVNameList/
//  DataItemOutSVNameListWithValue/DataItemOutEC/DataItemOutECNameList/
//  SendCeid/SendAnnotatedCeid/CheckCEIDExist/strGrdCEID/strGrdAlarm/
//  stdGridReportID/MoveCheckCallBack/bReceiveS101F5/bReceiveS101F7` (real
//  THGem methods/data members) stayed `HGemPtr->`, unchanged.
//
//  GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM (flag for review, found via
//  direct golden read while translating -- not previously catalogued):
//  S6F18_AnnotatedEventReportData's "CEID not exist" error path
//  (uHGemClass.cpp:1988) sends `InitLocalHead(6, 16, 0)` -- S,F 6,16, NOT
//  6,18 -- an evident copy-paste bug from S6F16_EventReportData's own,
//  textually-identical error path right above it in golden. NOT corrected
//  here, per this project's faithful-translation mandate; see that method's
//  own inline comment.
//
//  STILL GATED (23 remain; unaffected by this wave's delta, each needs at
//  least one thing beyond SvEcReg/WireCodec/StringGrid): S2F24Sub (Trace
//  member arrays), S2F32 (Borland dos.h clock), S2F34 family x5 (temp-list
//  members), S2F36 family x2 (ditto), S2F38 (ditto), S6F24/S7F18/S7F20/
//  Process_S7F20/S101F2/S101F4/S101F6_StoreHostUploadFile/
//  S101F8_StoreHostUploadFile (spool/upload/recipe subsystem members --
//  Wave 3/3b territory), S10F4/S10F6 (Terminal widget stand-ins),
//  S125F2/SetECValue (VCL widget dynamic_cast cluster) -- see §2 DEFERRED
//  in the design doc for the full breakdown.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 5 (AI(W906-AlarmReportAck) 20260721) -- SECSGEM closing-waves
//  Wave 2: THGem's Alarm/Report-Ack family unlocks the 8 remaining S2F34/
//  S2F36/S2F38/S5F4/S5F6 methods
//  ---------------------------------------------------------------------------
//  uHGemEquipment.h's THGem gained: 2 wire-codec forwarders (InitLocalHead/
//  DataItemOut, pointer overload only), 4 new members (slTempReportID/
//  lTempReportIDContent/slTempCeID/lTempCeIDContent), 3 new Alarm methods
//  (EnableDisableAlarm/EnableDisableAlarmAll/GetAlarmIndex), the Report/Link
//  Acknowledge composer family (ProcessHostSendReportLinkID/
//  ReportAcknowledge+5 wrappers/ReportLinkAcknowledgeError+4 wrappers), and a
//  REAL body for EnableDisableEventReportAcknowledgeError (+ its 2 wrappers)
//  -- replacing the earlier no-op stub. This resolves every remaining
//  blocker for the last 8 of the original 57 gated methods.
//
//  UN-GATED (8 more, 34->42/57 total now; 23->15 remaining-gated) -- golden
//  SECSGEM/uHGemClass.cpp line ranges cited at each definition below:
//    S2F34_DefineReportAcknowledgeSub (:1218-1291), S2F34_ProcessHostSendReportID
//    (:1294-1361), S2F34_DefineReportAcknowledge (:1363-1393),
//    S2F36_LinkEventReportAcknowledgeSub (:1395-1457),
//    S2F36_LinkEventReportAcknowledge (:1460-1477),
//    S2F38_EnableDisableEventReportAcknowledge (:1480-1521),
//    S5F4_EnableDisableAlarmAcknowledge (:1593-1624), S5F6_ListAlarmData
//    (:1626-1887).
//
//  MECHANICAL RENAME RULE (same golden-derived split as prior waves): every
//  golden `HGemPtr->DataItemIn/GetDataItemLenAndType/
//  GetDataItemLenAndTypeAndDelete/InitLocalHead/DataItemOut/SendLocalData/
//  LocalAcknowledge` (wire-codec primitives) became `ActiveWire->...`; golden
//  `HGemPtr->slTempReportID/lTempReportIDContent/slTempCeID/
//  lTempCeIDContent/strGrdAlarm/CheckReportAlreadyDefine/IsValidSVID/
//  DefineReportAcknowledge*/DeleteReportID/DeleteReportIDOfCeid/
//  SetReportIDContent/SaveEventReportData/DeleteAllHostDefineReportID/
//  DeleteAllHostDefineCeid/LinkReportAcknowledgeFormatError/
//  ProcessHostSendReportLinkID/CheckCEIDExist/
//  EnableDisableEventReportAcknowledgeCeidNotExist/...FormatError/
//  EnableDisableEventReport/EnableDisableAlarmAll/EnableDisableAlarm/
//  GetAlarmIndex` (real THGem-only state/methods, no wire-codec engine home)
//  stayed `HGemPtr->`, unchanged.
//
//  DEPENDENCY WIDENING (flagged explicitly): S2F34_DefineReportAcknowledgeSub's
//  golden body branches on `CUSTOMER_CODE==CC_ONSEMI_M` (golden :1241-1248,
//  an empty-bodied `if` -- preserved verbatim, not an omission on this port's
//  part). Neither symbol existed in this file before this wave; `cmydef.h`
//  is now `#include`d (below) for `CUSTOMER_CODE`/`CC_ONSEMI_M`, matching the
//  identical widening uHGemEquipment.cpp's own Bucket-B wave already made for
//  the same header -- zero new CMake link edge needed (uHGemClass.cpp and
//  uHGemEquipment.cpp already share the `ht9045_secsgem` target, which
//  already links `ht9045_core`/`ht9045_globals` for exactly this).
//
//  GOLDEN QUIRK PRESERVED VERBATIM (S5F6_ListAlarmData, golden :1751-1788,
//  the "alarm not found" branch): `DataItemOut(0, HType.BINARY_TYPE, &Mode)`
//  passes length 0 -- `Mode`'s value is never actually transmitted on the
//  wire. This is a genuine golden zero-length SECS-II item, not a
//  translation bug -- see that method's own inline comment. Also preserved:
//  golden `_atoi64` -> `strtoll`/`strtoull` (MinGW <cstdlib> substitution,
//  same precedent SecsSvEcRegistration.cpp/uHGemEquipment.cpp already
//  established) and the `(unsigned)strtoull(...)` 32-bit-truncating cast
//  golden itself applies before storing into a 64-bit `uint8SV` (golden
//  :1720 -- an existing golden quirk, not introduced here).
//
//  STILL GATED (15 remain; unaffected by this wave's delta): S2F24Sub (Trace
//  member arrays), S2F32 (Borland dos.h clock), S6F24/S7F18/S7F20/
//  Process_S7F20/S101F2/S101F4/S101F6_StoreHostUploadFile/
//  S101F8_StoreHostUploadFile (spool/upload/recipe subsystem members),
//  S10F4/S10F6 (Terminal widget stand-ins), S125F2/SetECValue (VCL widget
//  dynamic_cast cluster) -- unchanged from INTEGRATE WAVE 4's own list minus
//  the 8 resolved this wave.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 6 (AI(W906-uHGemClass-Micro5) 20260721) -- micro-slice #5 of
//  the "remaining 15 gated methods" cluster: recipe-upload-listing +
//  remote-recipe-checklist + EC-enable "StringGrid database"
//  ---------------------------------------------------------------------------
//  uHGemEquipment.h's THGem gained: TStringList *UploadFileString (new/delete
//  lifecycle mirroring HTGem's own SecsAlarmMessage/FMessageList pair),
//  THGemListBox *GemRemoteReceipeList (default-NULL/externally-assigned
//  lifecycle mirroring TerminalMemoPtr -- reuses the EXISTING THGemListBox
//  stand-in, already in use for SFCodeResponseList), and
//  TStringGrid *sgSECSECData + WriteECEnableData/EnableDisableECData/
//  EnableDisableECDataAll (a mechanical structural clone of the prior wave's
//  own strGrdAlarm/WriteAlamData/EnableDisableAlarm/EnableDisableAlarmAll
//  family, just for EC instead of Alarm). This resolves every remaining
//  blocker for 4 of the 15 gated methods.
//
//  UN-GATED (4 more, 42->46/57 total now; 15->11 remaining-gated) -- golden
//  SECSGEM/uHGemClass.cpp line ranges cited at each definition below:
//    Process_S7F20_CurrentEPPIDData (:2191-2220), S101F2_CurrentEPPDData
//    (:2456-2476), S101F4_CurrentEPPDData (:2482-2502),
//    S125F2_EnableDisableECDataAcknowledge (:2643-2680).
//
//  MECHANICAL RENAME RULE (same golden-derived split as prior waves): every
//  golden `HGemPtr->InitLocalHead/DataItemOut/SendLocalData/DataItemIn/
//  GetDataItemLenAndType/GetDataItemLenAndTypeAndDelete` (wire-codec
//  primitives) became `ActiveWire->...` -- including golden S125F2's OWN
//  cosmetic mix of `HGemPtr->` and the bare global `HGem->` for these same
//  primitives (both spellings unified to `ActiveWire->`, see that method's
//  own comment); golden `HGemPtr->UploadFileString/GemRemoteReceipeList/
//  SV_70_UNT1_ReceipeStruct/SV_71_ASCII_FilenameExtened/
//  EnableDisableECDataAll/EnableDisableECData` (real THGem state/methods)
//  stayed `HGemPtr->`, unchanged.
//
//  GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM (same species as the
//  already-documented S6F16/S6F18 InitLocalHead(6,16,0) copy-paste bug, see
//  INTEGRATE WAVE 4's own note above): S101F4_CurrentEPPDData's BOTH the
//  early-return path (golden :2487) and the success path (golden :2493) call
//  InitLocalHead(101, 2, 0) -- the S101F2 header, never (101, 4, 0) -- an
//  evident copy-paste artifact from S101F2_CurrentEPPDData immediately above
//  it in golden. NOT corrected here; see that method's own inline comment.
//
//  STILL GATED (11 remain; unaffected by this wave's delta): S2F16 (needs
//  THGem::SReceiveDataBackup + csystem predicates HasICUnderMachine()/
//  HasAnyICInMachine() -- a new ht9045_secsgem->ht9045_sm link edge,
//  deliberately out of scope; grep-confirmed among the "15" count above but
//  not previously named in this file-head note's own prose), S2F24Sub (Trace
//  member arrays), S2F32 (Borland dos.h clock), S6F24/S7F18/S7F20 (spool/
//  upload/recipe subsystem members -- HTGem::S7F20_CurrentEPPDData, DISTINCT
//  from Process_S7F20_CurrentEPPIDData resolved this wave), S10F4/S10F6
//  (Terminal widget stand-ins), S101F6_StoreHostUploadFile/
//  S101F8_StoreHostUploadFile (still need UpLoadPath/CurrentDirectory +
//  bFinishDownloadFile, none in this wave's scope), SetECValue (VCL widget
//  dynamic_cast cluster) -- unchanged from INTEGRATE WAVE 5's own list minus
//  the 4 resolved this wave.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 7 (AI(W906-uHGemClass-Micro6) 20260721) -- micro-slice #6 of
//  the "remaining 11 gated methods" cluster: S6F24 spool-ack + S7F18
//  delete-process-program
//  ---------------------------------------------------------------------------
//  uHGemEquipment.h's THGem gained 4 new plain scalar members --
//  bSpoolActive/bBeginTransferSpool/GemSpoolPath (S6F24's own gate/latch/
//  target-dir) and UpLoadPath (S7F18's recipe-upload base dir) -- plus a new
//  free function `DeleteDirectory` (Public/ExternFunction.h/.cpp; the "W3
//  DEFERRED, needs FindFirst/FindNext shim" note on it was stale -- vclcompat/
//  SysUtils.h already carries FindFirst/FindNext/FindClose/TSearchRec/
//  RemoveDir/DeleteFile/DirectoryExists, added by a later wave than when that
//  deferred note was written). `GetDataItemLenAndTypeAndDelete` (already a
//  real ActiveWire/WireCodec method since INTEGRATE WAVE 2) and the free
//  `GetLastOpenFN()` (already real, common.cpp, translated in an earlier
//  wave) needed no further work. This resolves every remaining blocker for
//  both methods.
//
//  UN-GATED (2 more, 46->48/57 total now; 11->9 remaining-gated) -- golden
//  SECSGEM/uHGemClass.cpp line ranges cited at each definition below:
//    S6F24_RequestSpooledDataAcknowledgementSend (:2053-2079),
//    S7F18_DeleteProcessProgramAcknowledge (:2115-2166).
//
//  MECHANICAL RENAME RULE (same golden-derived split as prior waves): golden
//  `HGemPtr->DataItemIn/GetDataItemLenAndTypeAndDelete/LocalAcknowledge` (wire-
//  codec primitives) became `ActiveWire->...`; golden `HGemPtr->bSpoolActive/
//  bBeginTransferSpool/GemSpoolPath/UpLoadPath` (real THGem data members)
//  stayed `HGemPtr->`, unchanged. `GetLastOpenFN()` and `DeleteDirectory(...)`
//  are golden GLOBAL free functions (not THGem members -- confirmed by
//  reading golden uHGemClass.h/uHGemEquipment.h, neither declares a member of
//  either name), so they are called bare, exactly as golden does, no `HGemPtr->`/
//  `ActiveWire->` prefix.
//
//  GOLDEN QUIRKS PRESERVED VERBATIM (flagged for review, not corrected):
//    S6F24: the spool-wipe command string is built as
//    `AnsiString("del ")+IncludeTrailingPathDelimiter(path)+AnsiString("*.*/q/f")`
//    -- note the missing spaces before `/q` and `/f`; as written this is a
//    single argument to `del` (`...\*.*/ q/f` has no delimiter cmd.exe would
//    split on), so the `/Q` (quiet) and `/F` (force) switches are almost
//    certainly never actually applied as switches -- a pre-existing golden
//    bug (same shape recurs verbatim at golden uHGemEquipment.cpp:4102/6144,
//    so it is not local to S6F24). Also: this `system()` call only executes
//    when `bSpoolActive==true` AND the host's RSDC byte==1 -- with this wave's
//    safe `bSpoolActive=false` ctor default (no "DoSpool subsystem" wave has
//    run yet), it is a no-op path, exactly as the design brief intended.
//    S7F18: `ret=DeleteDirectory(S); if(ret==0)` assigns `DeleteDirectory`'s
//    `bool` return into an `int` and compares with `==0` rather than
//    `==false` -- functionally identical (bool->int: true=1/false=0), just
//    golden's own literal C++ idiom, preserved rather than "cleaned up" to
//    `if(!ret)`/`bool ret`. Also preserved: the `asLastFileName==PPID`
//    "currently open recipe" guard short-circuits BEFORE the
//    `DirectoryExists`/`DeleteDirectory` calls for that one PPID entry (golden
//    :2143-2147) -- if the host lists the in-use PPID first in a multi-PPID
//    L,n request, later PPIDs in the same list are never even reached (golden
//    `return`s immediately) -- an existing golden behavior, not a translation
//    defect.
//---------------------------------------------------------------------------
//
//  INTEGRATE WAVE 8 (AI(W906-uHGemClass-Micro7) 20260721) -- micro-slice #7:
//  Terminal display Ack pair + the recipe-upload "Store Host Upload File"
//  Sub pair (S101F6/S101F8's own real bodies, not just their wrappers)
//  ---------------------------------------------------------------------------
//  uHGemEquipment.h's THGem gained: 9 new Terminal* members
//  (TerminalDisplayIndex/TerminalListboxPtr/TerminalEditPtr/TerminalPanelPtr
//  + the full "2"-suffixed mirror set -- TerminalMemoPtr already existed),
//  `bool bFinishDownloadFile`, and `AnsiString CurrentDirectory` (bare member
//  only -- golden's own SetCurrentDirectory setter/cascade stays out of
//  scope). `THGemListBox` (the existing GemRemoteReceipeList stand-in) gained
//  a new `Checked` member (`THGemCheckedArray`, write-only this wave --
//  auto-grows on `Checked[i]=false`, see that struct's own header comment for
//  the read-support-later design). This resolves every remaining blocker for
//  4 methods.
//
//  UN-GATED (4 more, 48->52/57 total now; 9->5 remaining-gated) -- golden
//  SECSGEM/uHGemClass.cpp line ranges cited at each definition below:
//    S10F4_TerminalDisplaySingleAcknowledge (:2274-2382),
//    S10F6_TerminalDisplayMultiBlockAcknowledge (:2387-2431),
//    S101F6_StoreHostUploadFile (:2519-2581),
//    S101F8_StoreHostUploadFile (:2597-2639).
//
//  MECHANICAL RENAME RULE (same golden-derived split as prior waves): golden
//  `HGemPtr->GetDataItemLenAndTypeAndDelete/GetDataItemLenAndType/DataItemIn/
//  Remote.W_Bit/LocalAcknowledge` (wire-codec primitives) became
//  `ActiveWire->...` -- INCLUDING S10F6's own cosmetic mix of `HGemPtr->` and
//  the bare global `HGem->` for these same primitives (both spellings
//  unified to `ActiveWire->`, same duality rule as INTEGRATE WAVE 5/6's own
//  notes, see uHGemClass.cpp:3234); golden `HGemPtr->TerminalDisplayIndex(2)/
//  TerminalMemoPtr(2)/TerminalListboxPtr(2)/TerminalEditPtr(2)/
//  TerminalPanelPtr(2)/GetTimeInfo()/TimeString/bFinishDownloadFile/
//  UpLoadPath/CurrentDirectory/GemRemoteReceipeList/SV_70_UNT1_ReceipeStruct/
//  SV_71_ASCII_FilenameExtened` (real THGem state/methods) stayed
//  `HGemPtr->`, unchanged. `ActiveWire->DownLoadFilePtr` is the wire codec's
//  own raw-binary-payload pointer (SecsWireCodec.h, already real), not a
//  THGem member. `FMessageList`/`SecsAlarmMessage` are HTGem's OWN members
//  (bare, no prefix, same as every other access to them in this file).
//  `MyForceDirectories`/`IncludeTrailingPathDelimiter` are golden GLOBAL/
//  TU-local free functions (not THGem members), called bare, exactly as
//  golden does. `Now().FormatString(fmt)` -> `FormatDateTime(fmt, Now())`,
//  same substitution already established at ainarm9045.cpp:890. `__FUNC__`
//  (BCB6 builtin) -> standard `__func__`, same substitution this project's
//  own canary_support.h/aArmHeader.h shims already establish elsewhere.
//
//  GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM (flag for review, found via
//  direct golden read while translating -- not previously catalogued):
//  S101F6_StoreHostUploadFile's own success path calls
//  `ActiveWire->LocalAcknowledge(101, 6, 0);` internally (golden :2579), and
//  its caller `HTGem::S101F6()` (golden :2778-2787) ALSO unconditionally
//  calls `ActiveWire->LocalAcknowledge(101, 6, 0);` right after -- a genuine
//  double-acknowledge: the host receives the SAME accept TWICE for one
//  successful S101F6 primary message. NOT corrected here, per this project's
//  faithful-translation mandate; see both methods' own inline comments.
//  ASYMMETRY (also flagged, NOT a bug to "fix"): `S101F8_StoreHostUploadFile`
//  itself never calls LocalAcknowledge anywhere in golden -- only its own
//  wrapper `HTGem::S101F8()` does, exactly once -- so S101F8's host reply is
//  single, never double like S101F6's. Confirmed by direct read of golden
//  (uHGemClass.cpp:2597-2639 vs :2519-2581); a genuine difference between the
//  two otherwise near-identical Subs, not a translation gap.
//
//  GOLDEN QUIRK, FLAGGED NOT FIXED (wire-supplied path, zero sanitization,
//  same risk category as this file's own established S7F18/DeleteDirectory
//  precedent, see INTEGRATE WAVE 7's own note above): both
//  S101F6_StoreHostUploadFile's `str` (200-byte wire-supplied ASCII filename,
//  concatenated onto `UpLoadPath+"HGem\\"`) and S101F8_StoreHostUploadFile's
//  `PathName`/`str` (both 200-byte wire-supplied ASCII fields, fed into
//  `MyForceDirectories` and the final `fopen` path) are used with NO
//  length/charset/".."/absolute-path sanitization of any kind, in golden or
//  here. NOT sanitized here, per this project's faithful-translation
//  mandate -- see each method's own inline comment.
//
//  STILL GATED (5 remain; unaffected by this wave's delta): S2F16 (needs
//  THGem::SReceiveDataBackup + csystem predicates HasICUnderMachine()/
//  HasAnyICInMachine()), S2F24Sub (Trace member arrays), S2F32 (Borland
//  dos.h clock), S7F20_CurrentEPPDData (DISTINCT from
//  Process_S7F20_CurrentEPPIDData, already un-gated), SetECValue (VCL widget
//  dynamic_cast cluster) -- unchanged from INTEGRATE WAVE 7's own list minus
//  the 4 resolved this wave.
//
//  AI(W906-VCW1) 20260721: refreshed from "5" above -- SetECValue is
//  UN-GATED this wave (vclcompat/Controls.h supplies the 6 missing widget
//  stand-ins; see SetECValue's own comment below). 4 REMAIN gated: S2F16,
//  S2F24Sub, S2F32, S7F20_CurrentEPPDData (unchanged, none of this wave's
//  scope). grep `^#if 0` re-verified at 4.
//
//  AI(W906-uHGemClass-Unlock3) 20260723: refreshed from "4" above -- S2F16
//  is UN-GATED this wave (SecsWireCodec gained SReceiveDataBackup, csystem.h
//  supplies the real HasICUnderMachine()/HasAnyICInMachine() predicates --
//  see S2F16's own comment below). 3 REMAIN gated: S2F24Sub, S2F32,
//  S7F20_CurrentEPPDData (unchanged, none of this wave's scope). grep
//  `^#if 0` re-verified at 3.
//
//  AI(W906-uHGemClass-TraceUnlock) 20260728: refreshed from "3" above -- 2 of
//  the 3 remaining are UN-GATED this wave:
//    * S2F24Sub -- uHGemEquipment.h gained the "Trace cluster" member set
//      (bTraceData[10]/iTRID[10]/DSPER[10]/iTOTSMP[10]/iREPGSZ[10]/
//      iTOTSMP_Count[10]/TraceData[10]/TraceDataResponseTask[10]); real body
//      translated, golden :810-990.
//    * S2F32 -- split into a pure ParseSECSDateTimeString(...) helper (this
//      file, zero OS side effects, fully unit-tested) + SetSystemDateTimeHook
//      (uHGemClass.h, an std::function seam defaulting EMPTY/no-op) standing
//      in for golden's only non-portable calls (Borland dos.h settime()/
//      setdate()); real body translated, golden :1095-1216.
//  1 REMAINS gated: S7F20_CurrentEPPDData -- INVESTIGATED this wave, NOT a
//  mechanical member-addition case: its real blocker is
//  THGem::SetReceipeDirectoryAndGlobalName's Type==2 branch, which needs a
//  live TDirectoryListBox-equivalent directory-tree-walking widget (no
//  vclcompat stand-in exists) -- see that method's own updated #if 0 comment
//  for the full re-investigation writeup (2 of its originally-cited 3
//  blockers, SV_70_UNT1_ReceipeStruct/UploadFileString, turned out to
//  already be real). grep `^#if 0` re-verified at 1.
//---------------------------------------------------------------------------

#include "vclcompat/vcl_compat.h"
// AI(W906-VCW1) 20260721: SetECValue's IsVCL==1 dynamic_cast cascade (below)
// needs the 6 VCL-widget stand-ins this header supplies (TPanel/TCustomEdit/
// TComboBox/TLabel/TCheckBox/TRadioGroup) -- see Controls.h's own file-head
// scope-boundary note (no real widget-backed EC/SV is made functional by
// this include).
#include "vclcompat/Controls.h"
#include "uHGemClass.h"
// AI(W906-SysModWire) 20260720: FIRST include of uHGemEquipment.h in this file
// -- see uHGemClass.h's own updated forward-declaration note for why: 8 newly
// un-gated methods below call real out-of-line THGem methods
// (CheckSFFormatOnlyHead/OffLine/OnLine/GetOnLineMode/GetTimeInfo), which
// require THGem's complete type (not just the bare forward declaration the
// header keeps). uHGemClass.h itself is UNCHANGED (still `class THGem;`) --
// this include stays confined to the .cpp, matching this project's "header
// stays minimal, .cpp pulls what its method BODIES need" convention.
#include "SECSGEM/uHGemEquipment.h"
// AI(W906-AlarmReportAck) 20260721: DEPENDENCY WIDENING -- S2F34_
// DefineReportAcknowledgeSub's golden body branches on
// `CUSTOMER_CODE==CC_ONSEMI_M` (golden uHGemClass.cpp:1241-1248). Neither
// symbol existed in this file before this wave; cmydef.h supplies both
// (CUSTOMER_CODE extern int + CC_ONSEMI_M macro, via its own MachineType.h
// include) -- same widening uHGemEquipment.cpp's own Bucket-B wave already
// made for the identical header, zero new CMake link edge needed (this file
// and uHGemEquipment.cpp already share the ht9045_secsgem target, which
// already links ht9045_core/ht9045_globals for exactly this).
#include "cmydef.h"
// AI(W906-uHGemClass-Unlock3) 20260723: S2F16_NewEquipmentConstantSendAcknowledge's
// golden body (uHGemClass.cpp:736/746) branches on the cross-arm IC-presence
// predicates `HasICUnderMachine()`/`HasAnyICInMachine()` -- declared in
// csystem.h (the frozen W6 predicate-API contract), real-implemented in
// csystem_predicates.cpp (part of the ht9045_sm library). This is the FIRST
// call from ht9045_secsgem into ht9045_sm's predicate surface -- a new
// link edge (ht9045_secsgem the CMake TARGET does not link ht9045_sm; only
// the final test/executable that links BOTH resolves it, same posture as
// every other still-unresolved extern already declared-but-not-linked
// elsewhere in this tree). See tests/CMakeLists.txt's test_uHGemClass entry
// for the RESCAN group update this requires.
#include "csystem.h"
#include <cstdlib>   // atoi (CheckECValue's Type/PMax_Value/PMin_Value decode); strtoll/strtoull (S5F6_ListAlarmData's golden _atoi64 substitution, same precedent as SecsSvEcRegistration.cpp/uHGemEquipment.cpp)
// AI(W906-uHGemClass-Micro6) 20260721: S6F24/S7F18's own dependencies --
// common.h declares the real (un-gated, AI(W906-CommonWaveFile) 20260721)
// global GetLastOpenFN() S7F18 calls bare, exactly as golden does (it is a
// free function, NOT a THGem member -- confirmed by reading golden
// uHGemClass.h/uHGemEquipment.h, neither declares one of that name).
// Public/ExternFunction.h declares the real (this same wave) global
// DeleteDirectory(AnsiString), likewise called bare.
#include "common.h"
#include "Public/ExternFunction.h"

// AI(W5-SECSGEM-Translate) 20260710: MyDBIProcess's real golden signature is
// `void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1,
// AnsiString S2="");` (cMyDB.h:20; defined cMyDB.cpp:789, not yet
// translated) -- golden call sites like this file's own
// `MyDBIProcess("Exception", "~HTGem")` rely on the S2="" default.  Declared
// locally here matching that REAL 3-arg signature (not #include-ing cMyDB.h,
// to keep this file's dependency surface to vclcompat only).  NOTE for the
// integrate agent: aHotPlateSubstrate.h:576 forward-declares a DIFFERENT
// (2-arg) `MyDBIProcess(AnsiString S1, AnsiString S2)` from a prior wave --
// that is a distinct overload, not a redeclaration, so both can coexist, but
// it means the prior wave's declaration does not match golden's real
// signature (flagged as a pre-existing discrepancy, not introduced here).
// AI(W906-uHGemEquipment-BucketC) 20260717: added the missing `__fastcall` --
// this declaration previously LACKED it while database.cpp:64 (and the test
// stubs satisfying it) carry it. On i686 MinGW `__fastcall` is a real ABI
// (`__attribute__((fastcall))`, @n-decorated symbol): the mismatch made ld
// "resolve" the plain reference against the decorated definition with only a
// warning, producing a call through the WRONG calling convention --
// empirically reproduced as a segfault by the Bucket C fidelity review
// (dormant only because the affected call sites are exception-path-only).
// Root cause is vcl_compat.h's `#ifndef __fastcall` neutralization never
// firing (`__fastcall` is a compiler-builtin macro on MinGW) -- systemic
// follow-up tracked in MIGRATION_ROADMAP; this line fixes the one live edge.
extern void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2 = "");

// ---------------------------------------------------------------------------
//  IncludeTrailingPathDelimiter -- golden calls this exact SysUtils name
//  (uHGemClass.cpp:2067, inside S6F24_RequestSpooledDataAcknowledgementSend).
//  vclcompat/SysUtils.h only exposes IncludeTrailingBackslash (semantically
//  identical: appends the platform path separator iff not already present).
//  SAME file-scope-only thin wrapper idiom as uHGemEquipment.cpp's own
//  identically-named static helper (that one is NOT visible here -- static
//  = TU-local linkage -- hence this file needs its own copy).
// ---------------------------------------------------------------------------
static AnsiString IncludeTrailingPathDelimiter(const AnsiString &path)
{
    return IncludeTrailingBackslash(path);
}

//---------------------------------------------------------------------------
AnsiString SYS_ECChangeID             = "";                                     //pig 2014.04.23 KYEC SECS
AnsiString SYS_ECChangeIDOriginaValue = "";                                     //pig 2014.04.23 KYEC SECS
AnsiString SYS_ECChangeIDNewValue     = "";                                     //pig 2014.04.23 KYEC SECS
//------------------------------------------------------------------------------
bool IsCorrectDateFormat(int y,int m,int d)
{
    int   month[]={31,0,31,30,31,30,31,31,30,31,30,31};
    if(y<=2000 || y>9999)
        return   false;
    if(m<1 || m>12)
        return   false;
    if(((y%4)==0 && (y%100)!=0) || (y%400==0))
        month[1]=29;
    else
        month[1]=28;
    if(d>=1 && d<=month[m-1])
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-TraceUnlock) 20260728: injectable OS-clock-write seam --
// definition of the extern declared in uHGemClass.h (see that header's own
// comment for the full design rationale). Default-constructed = an EMPTY
// std::function -- calling it is guarded at S2F32's own call site below, so
// this is a safe no-op until some future wave/test explicitly assigns it.
std::function<void(int year, int month, int day, int hour, int min, int sec, int hundredths)> SetSystemDateTimeHook;
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-TraceUnlock) 20260728: pure parse+validate extraction
// for S2,F32 (golden SECSGEM/uHGemClass.cpp:1095-1216) -- see uHGemClass.h's
// own comment on this function/struct for the full design rationale
// (mirrors this project's established "extract pure calc-core, gate the side
// effect" philosophy, e.g. MainCalcCore.h/.cpp, kept local to this SECSGEM
// file instead since golden's origin here is uHGemClass.cpp, not main.cpp).
// Every `atoi(S.SubString(a,b).c_str())` call below is a DIRECT, unmodified
// transcription of golden's own `atoi(S.SubString(a,b).c_str())` calls
// (uHGemClass.cpp:1114-1175) -- only the assignment TARGETS were renamed
// (golden's `reset.da_year`/`t.ti_hour`/etc, Borland dos.h `struct date`/
// `struct time` fields -- not portable -- became `out.year`/`out.hour`/etc,
// this function's own portable POD struct).
bool ParseSECSDateTimeString(const AnsiString &S, int len, SECSDateTimeFields &out,
                              bool &bRangeError, bool &bApplyClock)
{
    bRangeError=false;
    bApplyClock=false;

    if(len==12)                                                                // "030601134700"
    {
        out.year        =atoi(S.SubString(1, 2).c_str())+2000;
        out.month       =atoi(S.SubString(3, 2).c_str());
        out.day         =atoi(S.SubString(5, 2).c_str());
        out.hour        =atoi(S.SubString(7, 2).c_str());
        out.min         =atoi(S.SubString(9, 2).c_str());
        out.sec         =atoi(S.SubString(11, 2).c_str());
        out.hundredths  =0;
    }
    else if(len==14)                                                           // "20030602134700"
    {
        out.year        =atoi(S.SubString(1, 4).c_str());
        out.month       =atoi(S.SubString(5, 2).c_str());
        out.day         =atoi(S.SubString(7, 2).c_str());
        out.hour        =atoi(S.SubString(9, 2).c_str());
        out.min         =atoi(S.SubString(11, 2).c_str());
        out.sec         =atoi(S.SubString(13, 2).c_str());
        out.hundredths  =0;
    }
    else if(len==16)                                                           // "2003060313401000"
    {
        out.year        =atoi(S.SubString(1, 4).c_str());
        out.month       =atoi(S.SubString(5, 2).c_str());
        out.day         =atoi(S.SubString(7, 2).c_str());
        out.hour        =atoi(S.SubString(9, 2).c_str());
        out.min         =atoi(S.SubString(11, 2).c_str());
        out.sec         =atoi(S.SubString(13, 2).c_str());
        out.hundredths  =atoi(S.SubString(15, 2).c_str());
    }
    else if(len==19)                                                           //  "2003-06-04T13:01:01"
    {
        out.year        =atoi(S.SubString(1, 4).c_str());
        out.month       =atoi(S.SubString(6, 2).c_str());
        out.day         =atoi(S.SubString(9, 2).c_str());
        out.hour        =atoi(S.SubString(12, 2).c_str());
        out.min         =atoi(S.SubString(15, 2).c_str());
        out.sec         =atoi(S.SubString(18, 2).c_str());
        out.hundredths  =0;
    }
    else if(len==21)                                                           //  "2003-06-05T13:01:01.2"
    {
        out.year        =atoi(S.SubString(1, 4).c_str());
        out.month       =atoi(S.SubString(6, 2).c_str());
        out.day         =atoi(S.SubString(9, 2).c_str());
        out.hour        =atoi(S.SubString(12, 2).c_str());
        out.min         =atoi(S.SubString(15, 2).c_str());
        out.sec         =atoi(S.SubString(18, 2).c_str());
        out.hundredths  =atoi(S.SubString(21, 1).c_str())*10;
    }
    else if(len==22)                                                           //  "2003-06-06T13:01:01.25"
    {
        out.year        =atoi(S.SubString(1, 4).c_str());
        out.month       =atoi(S.SubString(6, 2).c_str());
        out.day         =atoi(S.SubString(9, 2).c_str());
        out.hour        =atoi(S.SubString(12, 2).c_str());
        out.min         =atoi(S.SubString(15, 2).c_str());
        out.sec         =atoi(S.SubString(18, 2).c_str());
        out.hundredths  =atoi(S.SubString(21, 2).c_str());
    }
    else
    {
        return false;                                                          // golden's own Error=true, uHGemClass.cpp:1177-1180
    }

    // golden uHGemClass.cpp:1182-1200 -- GOLDEN QUIRK: see this function's
    // own header comment (uHGemClass.h) for why an IsCorrectDateFormat()
    // failure here does NOT set bRangeError (preserved verbatim).
    if(IsCorrectDateFormat(out.year, out.month, out.day))
    {
        if(out.hour>23)                                                        //Steven 20140404 (golden): 避免Warning
            bRangeError=true;
        if(out.min>59)
            bRangeError=true;
        if(out.sec>59)
            bRangeError=true;
        if(out.hundredths>99)
            bRangeError=true;
        if(bRangeError==false)
            bApplyClock=true;
    }
    return true;
}
//---------------------------------------------------------------------------
HTGem::HTGem()
{
    HGemPtr=NULL;
    HandlerPath="";
    ActiveWire=&WireCodec;                                                      //AI(W906-SysModWire) 20260720: design D default -- standalone dispatch target
    SecsAlarmMessage=new TStringList;                                           //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
    FMessageList=new TStringList;                                               //Ifor 20251018 add:Secs Alarm List
};
//---------------------------------------------------------------------------
HTGem::HTGem(THGem *HGemTmp)
{
    HGemPtr=HGemTmp;
    HandlerPath="";
    ActiveWire=&WireCodec;                                                      //AI(W906-SysModWire) 20260720: design D default -- standalone dispatch target
    SecsAlarmMessage=new TStringList;                                           //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
    FMessageList=new TStringList;                                               //Ifor 20251018 add:Secs Alarm List
}
//---------------------------------------------------------------------------
HTGem::HTGem(AnsiString Path)
{
    HandlerPath=Path;
    ActiveWire=&WireCodec;                                                      //AI(W906-SysModWire) 20260720: design D default -- standalone dispatch target
    SecsAlarmMessage=new TStringList;                                           //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
    FMessageList=new TStringList;                                               //Ifor 20251018 add:Secs Alarm List
}
//---------------------------------------------------------------------------
HTGem::~HTGem()                                                                 //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
{
    try
    {
        if(SecsAlarmMessage!=NULL)
        {
            SecsAlarmMessage->Clear();                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
            delete SecsAlarmMessage;
        }
        SecsAlarmMessage=NULL;                                                  //kevin 20180907 add

        if(FMessageList!=NULL)                                                  //Ifor 20251018 add:Secs Alarm List
        {
            FMessageList->Clear();
            delete FMessageList;
        }
        FMessageList = NULL;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~HTGem");
    }
}
//---------------------------------------------------------------------------
void HTGem::UpdateDataPath(AnsiString Path)
{
    DataPath=Path;
}
//===========================================================================
//  Below: 57 out-of-line S,F handlers.  Every one of them needs `HGemPtr`
//  (THGem, out of scope -- see file-head note).  Real bodies are GATED with
//  a golden line-range citation; golden itself is the citable source, so the
//  body text is intentionally NOT duplicated here.  Each ACTIVE stub matches
//  its declared signature and returns a conservative/no-op default.
//===========================================================================

//---------------------------------------------------------------------------
// [S1,F1] Are You There Request.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:89-95).
// `HGemPtr->bS1F2_OnLineData` is a real THGem DATA member (unchanged);
// InitLocalHead/StringOut/SendLocalData -> ActiveWire-> (wire-codec ops).
// GOLDEN QUIRK preserved verbatim: InitLocalHead(1,1,0) sets W-Bit=0 even
// though S1,F1 is itself a request (W-Bit would normally be 1) -- not "fixed".
void HTGem::S1F1_AreYouThereRequest()
{
    HGemPtr->bS1F2_OnLineData=false;
    ActiveWire->InitLocalHead(1,1,0);
    ActiveWire->StringOut("[Send]    AreYouThereRequest");
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S1,F2] OnLineData : Data signifying that the equipment is alive.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:102-111).
// CheckSFFormatOnlyHead/GemMDLN/GemSOFTREV are real THGem method/data members
// (HGemPtr->, unchanged); InitLocalHead/DataItemOut/SendLocalData -> ActiveWire->.
void HTGem::S1F2_OnLineData()
{
    if(HGemPtr->CheckSFFormatOnlyHead("S1,F1 Format error !!!")==false)
        return;
    ActiveWire->InitLocalHead(1, 2, 0);
    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemMDLN);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemSOFTREV);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S1,F4] Selected Status Reply -- reports the value of each requested SVID.
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 119-264). Wire primitives (GetDataItemLenAndType/GetDataItemLenAndTypeAndDelete/
// InitLocalHead/DataItemOut/DataItemIn/SendLocalData) -> ActiveWire->;
// DataItemOutSV is THGem's own method (HGemPtr->, unchanged).
void HTGem::S1F4_SelectedStatusReply()
{
    int SVlen, len, i;
    unsigned char Type;
    AnsiString S;

    if(ActiveWire->GetDataItemLenAndType(SVlen, Type)==1)
    {
        if(Type==HType.LIST_TYPE || SVlen==0)                                   // new
        {
            ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, Type);
            if(SVlen==0)                                                        // report all svid
            {
                ActiveWire->InitLocalHead(1, 4, 0);
                len=HGemPtr->SvEcReg.SV_ID->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(i=0; i<len; i++)
                {
                    HGemPtr->DataItemOutSV(atoi(HGemPtr->SvEcReg.SV_ID->GetString(i).c_str()));
                }
                ActiveWire->SendLocalData();
            }
            else
            {
                ActiveWire->InitLocalHead(1, 4, 0);
                ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
                for(i=0; i<SVlen; i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len,Type)==1)
                        if(ActiveWire->DataItemIn(len, Type, S)==1)              // only for ascii,i1,i2,i4,i5 ,u1,u2,u4,u8
                            if(HGemPtr->DataItemOutSV(S)==false)
                                return;
                }
                ActiveWire->SendLocalData();
            }
        }
        else                                                                    // old type
        {
            ActiveWire->InitLocalHead(1, 4, 0);
            ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
            if(Type==HType.UINT_2_TYPE)
            {
                unsigned short *P;
                P=new unsigned short [SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        if(HGemPtr->DataItemOutSV(P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else if(Type==HType.UINT_4_TYPE)
            {
                unsigned *P;
                P=new unsigned[SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        if(HGemPtr->DataItemOutSV(P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else if(Type==HType.UINT_8_TYPE)                                    //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                unsigned long long *P;                                          // golden unsigned __int64
                P=new unsigned long long[SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        // AI(W906-SvEcDataItem) 20260720: explicit (long long)
                        // cast -- vclcompat::AnsiString has ctors for
                        // int/unsigned int/long/long long/double but none for
                        // unsigned long long, so an implicit conversion here is
                        // AMBIGUOUS (could go via the long long OR the double
                        // ctor). SVIDs are always small positive integers in
                        // practice (well within long long range), so this cast
                        // is value-preserving for every real call; disambiguates
                        // only, no behavior change.
                        if(HGemPtr->DataItemOutSV((long long)P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else if(Type==HType.INT_2_TYPE)
            {
                short *P;
                P=new short[SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        if(HGemPtr->DataItemOutSV(P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else if(Type==HType.INT_4_TYPE)
            {
                int *P;
                P=new int [SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        if(HGemPtr->DataItemOutSV(P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else if(Type==HType.INT_8_TYPE)                                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                long long *P;                                                   // golden __int64
                P=new long long [SVlen];
                if(ActiveWire->DataItemIn(SVlen, Type, P))
                {
                    for(i=0; i<SVlen; i++)
                        if(HGemPtr->DataItemOutSV(P[i])==false)
                        {
                            delete[] P;
                            return;
                        }
                }
                delete[] P;
            }
            else
            {
                S9F7_IllegalData("S1,F3 data format error");
                return;
            }
            ActiveWire->SendLocalData();
        }
    }
    else
    {
        S9F7_IllegalData("S1,F3 data format error");
    }
}
//---------------------------------------------------------------------------
// [S1,F12] Status Variable Namelist Reply.
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 266-324). Same wire-primitive -> ActiveWire-> / THGem-method -> HGemPtr->
// rule as S1F4 above.
void HTGem::S1F12_StatusVariableNamelistReply()
{
    int SVlen, i, len;
    unsigned char Type;
    AnsiString S;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,Type)==1)
    {
        if(Type==HType.LIST_TYPE || SVlen==0)
        {
            ActiveWire->InitLocalHead(1,12,0);
            if(SVlen==0)
            {
                len=HGemPtr->SvEcReg.SV_ID->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(i=0; i<len; i++)
                    HGemPtr->DataItemOutSVNameList(HGemPtr->SvEcReg.SV_ID->GetString(i));
            }
            else
            {
                ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
                for(i=0; i<SVlen; i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len,Type)==1)
                    {
                        if(ActiveWire->DataItemIn(len, Type, S)==1)
                        {
                            if(HGemPtr->DataItemOutSVNameList(S)==false)
                                return;
                        }
                        else
                        {
                            S9F7_IllegalData("S1,F11 data format error");
                            return;
                        }
                    }
                    else
                    {
                        S9F7_IllegalData("S1,F11 data format error");
                        return;
                    }
                }
            }
            ActiveWire->SendLocalData();
        }
        else
        {
            S9F7_IllegalData("S1,F11 data format error");
        }
    }
    else
    {
        S9F7_IllegalData("S1,F11 data format error");
    }
}
//---------------------------------------------------------------------------
// [S1,F13] Establish Communications Request.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:326-334).
// GemMDLN/GemSOFTREV real THGem data members (HGemPtr->, unchanged);
// InitLocalHead/DataItemOut/SendLocalData -> ActiveWire->.
void HTGem::S1F13_EstablishCommunicationsRequest()
{
    AnsiString S;
    ActiveWire->InitLocalHead(1, 13, 0);
    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemMDLN);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemSOFTREV);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S1,F14] Connect Request Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:341-368).
// chkMoreMessageAbortProcess (THGemCheckBox*) + GemMDLN/GemSOFTREV +
// bReceiveEstablishCommunicationsRequest are real THGem members (HGemPtr->,
// unchanged); GetDataItemLenAndTypeAndDelete/InitLocalHead/DataItemOut/
// SendLocalData -> ActiveWire->.
void HTGem::S1F14_ConnectRequestAcknowledge()
{
    unsigned char Command=0,Type;
    AnsiString S, ret;
    int len;

    // < L[0]
    // >.

    if(HGemPtr->chkMoreMessageAbortProcess->Checked)
    {
        ret=ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type);
        if(ret!=1 || len!=0 || Type!=HType.LIST_TYPE)
        {
            S9F7_IllegalData("S1,F13 data format error");
            return;
        }
    }
    ActiveWire->InitLocalHead(1,14,0);

    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &Command);
    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemMDLN);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemSOFTREV);
    ActiveWire->SendLocalData();
    HGemPtr->bReceiveEstablishCommunicationsRequest=true;
}
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:370-403).
// bWaitEstablishCommunicationsResponse/...Error are real THGem data members
// (HGemPtr->, unchanged); DataItemIn -> ActiveWire->.
void HTGem::Process_S1F14_ConnectRequestAcknowledge()
{
    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        unsigned char binarydata[1];
        if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, binarydata)==1)
        {
            if(ActiveWire->DataItemIn(0, HType.LIST_TYPE, NULL)!=1)
            {
                S9F7_IllegalData("S1,F14 data format error");
                return;
            }

            if(binarydata[0]==0x00)
            {
                HGemPtr->bWaitEstablishCommunicationsResponse=true;
                HGemPtr->bWaitEstablishCommunicationsResponseError=false;
                return;
            }
        }
        else
        {
            S9F7_IllegalData("S1,F14 data format error");
            return;
        }
    }
    else
    {
        S9F7_IllegalData("S1,F14 data format error");
        return;
    }
    HGemPtr->bWaitEstablishCommunicationsResponse=true;
    HGemPtr->bWaitEstablishCommunicationsResponseError=true;
}
//---------------------------------------------------------------------------
// [S1,F16] OFFLINE Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:408-417).
// CheckSFFormatOnlyHead/OffLine are real THGem out-of-line methods (HGemPtr->,
// unchanged); InitLocalHead/DataItemOut/SendLocalData -> ActiveWire->.
void HTGem::S1F16_OFFLINEAcknowledge()
{
    unsigned char Command=0;
    if(HGemPtr->CheckSFFormatOnlyHead("S1,F15 Format error !!!")==false)
        return;

    ActiveWire->InitLocalHead(1, 16, 0);
    ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &Command);
    ActiveWire->SendLocalData();
    HGemPtr->OffLine();
}
//---------------------------------------------------------------------------
// [S1,F18] ONLINE Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:423-447).
// CheckSFFormatOnlyHead/bOnLine/GemCheckBoxAcceptHostOnlineRequest/OnLine/
// GetOnLineMode are real THGem members (HGemPtr->, unchanged); InitLocalHead/
// DataItemOut/SendLocalData -> ActiveWire->.
void HTGem::S1F18_ONLINEAcknowledge()
{
    unsigned char Command=0;
    if(HGemPtr->CheckSFFormatOnlyHead("S1,F17 Format error !!!")==false)
        return;

    ActiveWire->InitLocalHead(1, 18, 0);
    if(HGemPtr->bOnLine)
    {
        Command=2;
    }
    else if(HGemPtr->GemCheckBoxAcceptHostOnlineRequest->Checked)
    {
        HGemPtr->bOnLine=true;
        Command=0;
    }
    else
    {
        Command=1;
    }
    ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &Command);
    ActiveWire->SendLocalData();
    if(Command==0)
        HGemPtr->OnLine(HGemPtr->GetOnLineMode());
}
//---------------------------------------------------------------------------
// [S1,F24] Collection Event Namelist.                          //2014/01/01 lee
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 450-587) -- "no defect" method (§2 category ①): strGrdCEID/stdGridReportID
// (THGem's own StringGrid members, HGemPtr->, unchanged) + wire primitives
// (-> ActiveWire->) is the WHOLE dependency chain -- zero new THGem surface.
void HTGem::S1F24_CollectionEventNamelist()
{
    int SVlen, len;
    unsigned char Type;

    int   pos;
    unsigned int   uint4SV;
    bool ret;
    AnsiString S;
    TStringList *CEIDList,*SVIDList,*BackCeid;

    CEIDList=new TStringList;
    SVIDList=new TStringList;
    BackCeid=new TStringList;

    CEIDList->Clear();
    BackCeid->Clear();

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, Type)==1)
    {
        if(Type==HType.LIST_TYPE || SVlen==0)
        {
            ActiveWire->InitLocalHead(1, 24, 0);
            if(SVlen==0)                                                        // report all CEID name and SVID
            {
                for(int i=1; i<HGemPtr->strGrdCEID->RowCount; i++)
                {
                    if(HGemPtr->strGrdCEID->Cells[0][i]!="" && HGemPtr->strGrdCEID->Cells[0][i]!=NULL)
                        CEIDList->Add(i);
                }
                len=CEIDList->Count;

                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(int j=0; j<len; j++)
                {
                    ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                    pos=atoi(CEIDList->GetString(j).c_str());
                    uint4SV=(unsigned)atoi(HGemPtr->strGrdCEID->Cells[0][pos].c_str());
                    ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
                    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdCEID->Cells[2][pos]);
                    SVIDList->Clear();
                    for(int k=3; k<HGemPtr->strGrdCEID->ColCount; k++)
                    {
                        S=HGemPtr->strGrdCEID->Cells[k][pos];
                        if(S!="")
                        {
                            for(int y=1; y<HGemPtr->stdGridReportID->RowCount; y++)
                            {
                                if(S==HGemPtr->stdGridReportID->Cells[0][y])
                                {
                                    for(int x=2; x<HGemPtr->stdGridReportID->ColCount; x++)
                                    {
                                        if(HGemPtr->stdGridReportID->Cells[x][y]!="")
                                            SVIDList->Add(HGemPtr->stdGridReportID->Cells[x][y]);
                                    }
                                }
                            }
                        }
                    }
                    ActiveWire->DataItemOut(SVIDList->Count, HType.LIST_TYPE, NULL);
                    for(int k=0; k<SVIDList->Count; k++)
                    {
                        uint4SV=(unsigned)atoi(SVIDList->GetString(k).c_str());
                        ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
                    }
                }
            }
            else
            {
                for(int j=0; j<SVlen; j++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len,Type)==1)
                    {
                        if(ActiveWire->DataItemIn(len, Type, S)==1)
                        {
                            ret=false;
                            for(int i=1; i<HGemPtr->strGrdCEID->RowCount; i++)
                            {
                                if(HGemPtr->strGrdCEID->Cells[0][i]==S)
                                {
                                    CEIDList->Add(i);
                                    ret=true;
                                    break;
                                }
                            }
                            BackCeid->Add(S);
                            if(ret==false)
                                CEIDList->Add(0);
                        }
                    }
                }
                len=CEIDList->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(int j=0; j<len; j++)
                {
                    ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                    pos=atoi(CEIDList->GetString(j).c_str());
                    if(pos==0)                                                  // no such CEID
                    {
                        uint4SV=(unsigned)atoi(BackCeid->GetString(j).c_str());
                        ActiveWire->DataItemOut(1, HType.UINT_4_TYPE,&uint4SV);
                        ActiveWire->DataItemOut(HType.ASCII_TYPE, AnsiString(""));
                        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
                    }
                    else
                    {
                        uint4SV=(unsigned)atoi(HGemPtr->strGrdCEID->Cells[0][pos].c_str());
                        ActiveWire->DataItemOut(1, HType.UINT_4_TYPE,&uint4SV);
                        ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdCEID->Cells[2][pos]);
                        SVIDList->Clear();
                        for(int k=3; k<HGemPtr->strGrdCEID->ColCount; k++)
                        {
                            S=HGemPtr->strGrdCEID->Cells[k][pos];
                            if(S!="")
                            {
                                for(int y=1; y<HGemPtr->stdGridReportID->RowCount; y++)
                                {
                                    if(S==HGemPtr->stdGridReportID->Cells[0][y])
                                    {
                                        for(int x=2; x<HGemPtr->stdGridReportID->ColCount; x++)
                                        {
                                            if(HGemPtr->stdGridReportID->Cells[x][y]!="")
                                                SVIDList->Add(HGemPtr->stdGridReportID->Cells[x][y]);
                                        }
                                    }
                                }
                            }
                        }
                        ActiveWire->DataItemOut(SVIDList->Count, HType.LIST_TYPE, NULL);
                        for(int k=0; k<SVIDList->Count; k++)
                        {
                            uint4SV=(unsigned)atoi(SVIDList->GetString(k).c_str());
                            ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
                        }
                    }
                }
            }
            ActiveWire->SendLocalData();
        }
        else
        {
            S9F7_IllegalData("S1,F11 data format error");
        }
    }
    else
    {
        S9F7_IllegalData("S1,F11 data format error");
    }
    CEIDList->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    SVIDList->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    BackCeid->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete CEIDList;
    delete SVIDList;
    delete BackCeid;
}
//---------------------------------------------------------------------------
// [S2,F14] Equipment Constant Data.
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 609-719). EC_ID -> HGemPtr->SvEcReg.EC_ID (SV/EC bookkeeping embed);
// DataItemOutEC is THGem's own method (HGemPtr->, unchanged); wire
// primitives -> ActiveWire->.
void HTGem::S2F14_EquipmentConstanData()
{
    int EClen, len;
    unsigned char Type;
    AnsiString S;

    if(ActiveWire->GetDataItemLenAndType(EClen, Type)==1)
    {
        if(Type==HType.LIST_TYPE || EClen==0)
        {
            ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type);
            ActiveWire->InitLocalHead(2, 14, 0);
            if(EClen==0)
            {
                len=HGemPtr->SvEcReg.EC_ID->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(int i=0; i<len; i++)
                    HGemPtr->DataItemOutEC(HGemPtr->SvEcReg.EC_ID->GetString(i));
            }
            else
            {
                ActiveWire->DataItemOut(EClen, HType.LIST_TYPE, NULL);
                for(int i=0; i<EClen; i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
                    {
                        if(ActiveWire->DataItemIn(len, Type, S)==1)
                            HGemPtr->DataItemOutEC(S);
                    }
                    else
                    {
                        S9F7_IllegalData("S2,F13 data format error");
                        return;
                    }
                }
            }
            ActiveWire->SendLocalData();
        }
        else
        {
            ActiveWire->InitLocalHead(2, 14, 0);
            ActiveWire->DataItemOut(EClen, HType.LIST_TYPE, NULL);
            if(Type==HType.UINT_2_TYPE)
            {
                unsigned short *P;
                P=new unsigned short [EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.UINT_4_TYPE)
            {
                unsigned *P;
                P=new unsigned [EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.INT_2_TYPE)
            {
                short *P;
                P=new short [EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.INT_4_TYPE)
            {
                int *P;
                P=new int[EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.INT_8_TYPE)                                     //Steven 20130730 ADD
            {
                long long *P;                                                   // golden __int64
                P=new long long[EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.FT_4_TYPE)                                      //Steven 20130730 ADD
            {
                float *P;
                P=new float[EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else if(Type==HType.FT_8_TYPE)                                      //Steven 20130730 ADD
            {
                double *P;
                P=new double[EClen];
                if(ActiveWire->DataItemIn(EClen, Type, P))
                    for(int i=0; i<EClen; i++)
                        HGemPtr->DataItemOutEC(P[i]);
                delete[] P;
            }
            else
            {
                S9F7_IllegalData("S21,F13 data format error");
                return;
            }
            ActiveWire->SendLocalData();
        }
    }
    else
    {
        S9F7_IllegalData("S2,F13 data format error");
    }
}
//---------------------------------------------------------------------------
// [S2,F16] New Equipment Constant Send Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Unlock3) 20260723: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 731-781). Both blockers this stub's prior gate comment cited are now real:
// `THGem::SReceiveDataBackup` -- added THIS wave, but NOT on THGem/
// uHGemEquipment.h as that comment (and the task brief handed down for this
// wave) assumed. Direct read of THIS tree (not just the golden header)
// showed golden's `SReceiveData` itself already migrated off THGem onto
// SecsWireCodec in an EARLIER wave (see SecsWireCodec.h's own file-head
// note) -- THGem/uHGemEquipment.h/.cpp have carried ZERO `SReceiveData`
// members since then (grepped uHGemEquipment.cpp in full to confirm: zero
// hits). Adding SReceiveDataBackup back onto THGem as the brief's plan
// literally described would have reintroduced a member the codec no longer
// needs there and left THIS method unable to reach it via `ActiveWire->`
// (this file's own established Design D dispatch idiom, see uHGemClass.h) --
// so it is added to SecsWireCodec instead, right next to SReceiveData,
// matching that member's own ctor/dtor lifecycle exactly (see
// SecsWireCodec.h/.cpp's own comments on the new member). The csystem
// predicates `HasICUnderMachine()`/`HasAnyICInMachine()` (golden :738/746)
// are real, already-implemented free functions (csystem_predicates.cpp,
// part of ht9045_sm) -- called bare, exactly as golden does (not THGem
// members, no ActiveWire/HGemPtr indirection needed). Linking them in is a
// new ht9045_secsgem(-consuming executable) -> ht9045_sm link edge -- see
// tests/CMakeLists.txt's test_uHGemClass entry for the RESCAN group update
// this requires (and the resulting MyDBIProcess stub collision that fix
// surfaces).
// MECHANICAL RENAME: golden `HGemPtr->SReceiveDataBackup`/`HGemPtr->
// SReceiveData` (wire-codec primitives, now SecsWireCodec members) ->
// `ActiveWire->...`; golden `HGemPtr->LocalAcknowledge` -> `ActiveWire->
// LocalAcknowledge` (same D1/D2/Design-D precedent used throughout this
// file); `S2F15_CheckNewEquipmentConstant`/`S2F15_UpdateNewEquipmentConstant`
// (already-real HTGem sibling methods) called bare, unchanged; `HGemPtr->
// MoveCheckCallBack` (a real THGem member, added the SysModWire wave) stays
// `HGemPtr->`, unchanged, GOLDEN QUIRK preserved verbatim: golden derefs
// HGemPtr here with NO NULL-guard of its own (only the function-pointer
// VALUE is NULL-checked) -- same as this file's own already-un-gated
// S101F6()/S101F8() (see each one's own `HGemPtr->MoveCheckCallBack` call),
// so a standalone HTGem with HGemPtr==NULL must not reach this tail; this
// method's own test seeds a real THGem instance for exactly that reason.
void HTGem::S2F16_NewEquipmentConstantSendAcknowledge()
{
    int ret;
    ActiveWire->SReceiveDataBackup->Assign(ActiveWire->SReceiveData);

    if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS)                                   //Steven 20230213 : For SJSemi OS Tester
    {
        if(HasICUnderMachine())                                                //kevin 20181127 add
        {
            ActiveWire->LocalAcknowledge(2, 16, 2);
            return ;
        }
    }
    else
    {
        if(HasICUnderMachine() || HasAnyICInMachine())                         //kevin 20181127 add
        {
            ActiveWire->LocalAcknowledge(2, 16, 2);
            return ;
        }
    }

    ret=S2F15_CheckNewEquipmentConstant();                                     //wei 20170417 (Steven) add S2F15
    if(ret!=0)
    {
        if(ret==-1)
        {
            S9F7_IllegalData("S2,F15 data format error");
            return ;
        }
        else
        {
            ActiveWire->LocalAcknowledge(2, 16, ret);
            return ;
        }
    }
    ActiveWire->SReceiveData->Assign(ActiveWire->SReceiveDataBackup);

    ret=S2F15_UpdateNewEquipmentConstant();                                    //JerryYang 20250120 : modify
    if(ret==0)
    {
        ActiveWire->LocalAcknowledge(2, 16, 0);
    }
    else
    {
        ActiveWire->LocalAcknowledge(2, 16, ret);
    }

    if(HGemPtr->MoveCheckCallBack!=NULL)
        HGemPtr->MoveCheckCallBack();
}
//---------------------------------------------------------------------------
// [S2,F18] Date and Time Data.
//---------------------------------------------------------------------------
// AI(W906-SysModWire) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:786-793).
// CheckSFFormatOnlyHead/GetTimeInfo/GemClock are real THGem method/data
// members (HGemPtr->, unchanged); InitLocalHead/DataItemOut/SendLocalData ->
// ActiveWire->.
void HTGem::S2F18_DateandTimeData()
{
    if(HGemPtr->CheckSFFormatOnlyHead("S2,F17 Format error !!!")==false)
        return;
    HGemPtr->GetTimeInfo();
    ActiveWire->InitLocalHead(2, 18, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemClock);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S2,F24] Trace Initialize -- ack-code sub (called by the void wrapper below).
// AI(W906-uHGemClass-TraceUnlock) 20260728 UN-GATED (golden SECSGEM/
// uHGemClass.cpp:810-990). uHGemEquipment.h gained the "Trace cluster" member
// set this method (and its DoTraceDataResponse sibling in uHGemEquipment.cpp,
// still its own separate gated stub, out of THIS wave's scope -- see that
// method's own updated comment) share: bTraceData[10]/iTRID[10]/DSPER[10]/
// iTOTSMP[10]/iREPGSZ[10]/iTOTSMP_Count[10]/TraceData[10]/
// TraceDataResponseTask[10] (golden uHGemEquipment.h:674/696/699/702-706).
// THGem::IsValidSVID was already real (added by an earlier wave for the
// SV/EC DataItem family). NOTE: golden's own blocker citation for this
// method (as inherited from an earlier wave's #if 0 comment) omitted
// iTOTSMP_Count[10] -- confirmed by direct re-read of golden :982 (`HGemPtr->
// iTOTSMP_Count[iIndex]=1;`), which is not reachable from any OTHER already-
// ported method; added here as part of the same coherent cluster.
//
// MECHANICAL RENAME (same rule as every prior integrate wave in this file):
// golden `HGemPtr->DataItemIn/GetDataItemLenAndType` (wire-codec primitives)
// -> `ActiveWire->...`; `HGemPtr->bTraceData/iTRID/DSPER/iTOTSMP/iREPGSZ/
// iTOTSMP_Count/TraceData/TraceDataResponseTask/IsValidSVID/StringOut` (real
// THGem state/methods) stay `HGemPtr->`, unchanged.
//
// GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM (found via direct golden read
// while translating -- not previously catalogued): in the `Type==
// HType.UINT_4_TYPE` SVID-array branch (golden :937-964), `P=new unsigned
// [len];` is `delete[]`d ONLY on the `DataItemIn(...)!=1` failure path
// (golden :961) -- the SUCCESS path (the entire `if(...==1){...}` body,
// golden :941-958) never frees `P` at all, a genuine memory leak on every
// successful UINT_4_TYPE trace-SVID-list request. NOT corrected here, per
// this project's faithful-translation mandate; preserved exactly (no `delete[]`
// added after the if/else below).
//
// GOLDEN QUIRK PRESERVED VERBATIM (asymmetric length-mismatch handling,
// confirmed by direct read, golden :886-900): the TOTSMP peek's `len!=1`
// check returns `1` (golden :889 -- NOT an error code; the wrapper's
// `ret==-1` test is false, so this ends up ACKing 1 via LocalAcknowledge,
// distinct from a true format error), while the REPGSZ peek's OWN `len!=1`
// check (golden :896, structurally identical shape) returns `-1` (a true
// format error, routes to S9F7_IllegalData in the wrapper instead). Golden's
// own inconsistency between two adjacent, near-identical blocks -- not
// "fixed" into a uniform return code here.
int HTGem::S2F24_TraceInitializeAcknowledgeSub()
{
    int len,ret;
    unsigned int i;
    unsigned char Type;
    char str[256],dsper[64];
    unsigned hh, mm, ss, cc=0;
    int iIndex;
    bool bSVIDError=false;
    AnsiString ID,S,SVID;

    if(ActiveWire->DataItemIn(5,HType.LIST_TYPE,NULL)==1)
    {
        ActiveWire->GetDataItemLenAndType(len,Type);
        if(ActiveWire->DataItemIn(len,Type,ID)!=1)                             // ID=TRID
            return -1;
        iIndex=-1;
        for(i=0; i<10; i++)
        {
            if(HGemPtr->bTraceData[i]==true && HGemPtr->iTRID[i]==ID)
            {
                iIndex=i;
                break;
            }
        }

        if(iIndex==-1)                                                          // new TRACE
        {
            for(i=0; i<10; i++)
            {
                if(HGemPtr->bTraceData[i]==false)
                {
                    iIndex=i;
                    break;
                }
            }
        }

        if(iIndex==-1)                                                          // No more trace can use
            return 2;
        HGemPtr->iTRID[iIndex]=ID;

        ret=ActiveWire->GetDataItemLenAndType(len,Type);
        if(ret!=1 || Type!=HType.ASCII_TYPE || (len!=6 && len!=8))
            return -1;

        if(ActiveWire->DataItemIn(len,HType.ASCII_TYPE,dsper)!=1)
            return -1;
        // Format 1: hhmmss, 6 bytes
        // Format 2: hhmmsscc, 8 bytes

        str[0]=dsper[0];
        str[1]=dsper[1];
        str[2]=0;
        hh=atoi(str);

        str[0]=dsper[2];
        str[1]=dsper[3];
        str[2]=0;
        mm=atoi(str);

        str[0]=dsper[4];
        str[1]=dsper[5];
        str[2]=0;
        ss=atoi(str);
        if(len==8)
        {
            str[0]=dsper[6];
            str[1]=dsper[7];
            str[2]=0;
            cc=atoi(str);
        }

        HGemPtr->DSPER[iIndex]=hh*60*60*1000+mm*60*1000+ss*1000+cc;             // 以 0.1 sec 為單位 (golden gloss)
        HGemPtr->TraceDataResponseTask[iIndex]=1;                               //2013/07/18 lee

        if(ActiveWire->GetDataItemLenAndType(len, Type)!=1)
            return -1;
        if(len!=1)
            return 1;
        if(ActiveWire->DataItemIn(1, Type, S)!=1)
            return -1;
        HGemPtr->iTOTSMP[iIndex]=atoi(S.c_str());

        if(ActiveWire->GetDataItemLenAndType(len, Type)!=1)
            return -1;
        if(len!=1)
            return -1;
        if(ActiveWire->DataItemIn(1,Type,S)!=1)
            return -1;
        HGemPtr->iREPGSZ[iIndex]=atoi(S.c_str());

        HGemPtr->TraceData[iIndex]->Clear();

        if(ActiveWire->GetDataItemLenAndType(len, Type)!=1)
            return -1;
        if(Type==HType.LIST_TYPE)
        {
            if(ActiveWire->DataItemIn(len, HType.LIST_TYPE, NULL)==1)
            {
                HGemPtr->iREPGSZ[iIndex]=len;
                for(i=0; i<HGemPtr->iREPGSZ[iIndex];i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
                    {
                        if(len!=1)
                            return -1;
                        if(ActiveWire->DataItemIn(1, Type, SVID)!=1)
                            return -1;
                        if(HGemPtr->IsValidSVID(SVID)==false)                   // invalid SVID
                        {
                            S="SVID:"+AnsiString(SVID)+" not exist!!!";
                            HGemPtr->StringOut(S);
                            bSVIDError=true;
                        }
                        else
                        {
                            HGemPtr->TraceData[iIndex]->Add(SVID);
                        }
                    }
                }
            }
            else
            {
                HGemPtr->iREPGSZ[iIndex]=0;
            }
        }
        else if(Type==HType.UINT_4_TYPE)
        {
            unsigned *P;
            P=new unsigned [len];
            if(ActiveWire->DataItemIn(len, Type, P)==1)
            {
                HGemPtr->iREPGSZ[iIndex]=len;
                for(i=0; i<HGemPtr->iREPGSZ[iIndex]; i++)
                {
                    SVID=P[i];
                    if(HGemPtr->IsValidSVID(SVID)==false)                       // invalid SVID
                    {
                        S="SVID:"+AnsiString(SVID)+" not exist!!!";
                        HGemPtr->StringOut(S);
                        bSVIDError=true;
                    }
                    else
                    {
                        HGemPtr->TraceData[iIndex]->Add(SVID);
                    }
                }
            }
            else
            {
                delete[] P;
                return -1;
            }
        }
        else
        {
            return -1;
        }

        if(bSVIDError==true)
            return 4;
        if(HGemPtr->iREPGSZ[iIndex]!=0)
        {
            HGemPtr->iTOTSMP[iIndex]/=HGemPtr->iREPGSZ[iIndex];
            HGemPtr->bTraceData[iIndex]=true;
        }
        else
        {
            HGemPtr->bTraceData[iIndex]=false;
        }

        HGemPtr->iTOTSMP_Count[iIndex]=1;
        HGemPtr->TraceDataResponseTask[iIndex]=1;
    }
    else
    {
        return -1;
    }
    return 0;
}
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:992-1000):
// a thin wrapper -- calls its own still-GATED Sub() sibling (compiles/links
// fine either way; a sibling method call needs no THGem) plus LocalAcknowledge
// (now WireCodec.LocalAcknowledge, real). Needs nothing else.
void HTGem::S2F24_TraceInitializeAcknowledge()
{
    int ret;
    ret=S2F24_TraceInitializeAcknowledgeSub();
    if(ret==-1)
        S9F7_IllegalData("S2,F23 Format error !!!");
    else
        ActiveWire->LocalAcknowledge(2,24,(unsigned char)ret);                     // int->uchar narrowing, matches golden's own implicit conversion
}
//---------------------------------------------------------------------------
// [S2,F26] Diagnostic Loopback Data.
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:1006-1032; fixed 20260713 by an independent fidelity review, was mis-cited 1006-1024):
// entirely GetDataItemLenAndType/DataItemIn/InitLocalHead/DataItemOut/
// SendLocalData -- all now WireCodec methods. `Temp` is allocated as
// `unsigned char[]` here (golden: `unsigned char *Temp; ... Temp=new
// char[len+100];` -- a char*->unsigned char* implicit pointer-signedness
// assignment BCB6 accepted and standard C++ rejects without a cast;
// allocating with the variable's own element type instead is the same
// zero-behavior-change accommodation already established elsewhere in this
// file/SecsWireCodec.cpp for analogous char/unsigned char pointer mismatches).
//---------------------------------------------------------------------------
void HTGem::S2F26_DiagnosticLoopbackData()
{
    int len, ret;
    unsigned char Type;
    unsigned char *Temp;
    ret=ActiveWire->GetDataItemLenAndType(len,Type);
    if(Type==HType.BINARY_TYPE && ret==1)
    {
        Temp=new unsigned char [len+100];
        if(ActiveWire->DataItemIn(len, Type, Temp)==1)
        {
            ActiveWire->InitLocalHead(2, 26, 0);
            ActiveWire->DataItemOut(len, HType.BINARY_TYPE, Temp);
            ActiveWire->SendLocalData();
        }
        else
        {
            S9F7_IllegalData("S2,F25 Format error !!!");
        }
        delete[] Temp;
        Temp=NULL;                                                              //kevin 20180517
    }
    else
    {
        S9F7_IllegalData("S2,F25 Format error !!!");
    }
}
//---------------------------------------------------------------------------
// [S2,F30] Equipment Constant Namelist Reply.
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1037-1080). Same EC_ID -> HGemPtr->SvEcReg.EC_ID / DataItemOutECNameList ->
// HGemPtr-> / wire -> ActiveWire-> rule as S2F14 above.
void HTGem::S2F30_EquipmentConstantNamelistReply()
{
    int EClen, i, len;                                                          //,ret;
    unsigned char Type;
    AnsiString ECID;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type)==1)
    {
        if(Type==HType.LIST_TYPE)
        {
            ActiveWire->InitLocalHead(2, 30, 0);
            if(EClen==0)
            {
                len=HGemPtr->SvEcReg.EC_ID->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(i=0; i<len; i++)
                    HGemPtr->DataItemOutECNameList(HGemPtr->SvEcReg.EC_ID->GetString(i));
            }
            else
            {
                ActiveWire->DataItemOut(EClen, HType.LIST_TYPE, NULL);
                for(i=0; i<EClen; i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
                    {
                        if(ActiveWire->DataItemIn(len, Type, ECID)==1)
                        {
                            HGemPtr->DataItemOutECNameList(ECID);
                        }
                        else
                        {
                            S9F7_IllegalData("S2,F29 Format error !!!");
                            return;
                        }
                    }
                    else
                    {
                        S9F7_IllegalData("S2,F29 Format error !!!");
                        return;
                    }
                }
            }
            ActiveWire->SendLocalData();
        }
        else
        {
            S9F7_IllegalData("S2,F29 Format error !!!");
        }
    }
    else
    {
        S9F7_IllegalData("S2,F29 Format error !!!");
    }
}
//---------------------------------------------------------------------------
// [S2,F32] Date And Time Acknowledge (uses IsCorrectDateFormat, kept ACTIVE above).
// AI(W906-uHGemClass-TraceUnlock) 20260728 UN-GATED (golden SECSGEM/
// uHGemClass.cpp:1095-1216). Split per this wave's brief: the pure decode+
// validate logic lives in ParseSECSDateTimeString (this file, see its own
// header-banner comment right above IsCorrectDateFormat) -- zero OS side
// effects, fully unit-tested (tests/test_uHGemClass.cpp). The ONLY
// non-portable operations golden had here (Borland dos.h settime()/setdate())
// are represented by SetSystemDateTimeHook (uHGemClass.h), an std::function
// seam that defaults EMPTY -- guarded below, so this method NEVER touches the
// real OS clock unless a caller has explicitly wired the hook (no such
// wiring exists anywhere in this ported tree yet, by design -- see the
// hook's own header comment).
//
// MECHANICAL RENAME: golden `HGemPtr->GetDataItemLenAndType/DataItemIn`
// (wire-codec primitives) -> `ActiveWire->...`; golden `HGemPtr->
// LocalAcknowledge` -> `ActiveWire->LocalAcknowledge` (same rule as every
// other un-gated method in this file).
//
// GOLDEN QUIRK PRESERVED VERBATIM (see ParseSECSDateTimeString's own header
// comment in uHGemClass.h for the full writeup): a structurally well-formed
// date/time string whose CALENDAR date is invalid (e.g. month=13, or a
// day that does not exist in that month) still yields Error==false here
// (DTACK=0, "accepted") even though NO clock write happens
// (`bApplyClock==false` in that case) -- golden's own asymmetry against the
// range-check-failure case (hour>23 etc, which DOES set Error=true ->
// DTACK=1), not "fixed" into a uniform decision here.
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- HTGem::S2F32_DateAndTimeAcknowledge
//  golden SECSGEM/uHGemClass.cpp:1095-1216 (122 golden lines), transcribed CHARACTER-FOR-CHARACTER
//  (Big5/cp950 -> UTF-8 only) and GATED OFF.  The block is INERT: the ACTIVE
//  HTGem::S2F32_DateAndTimeAcknowledge is the 44-line body immediately after the #endif, and that body is
//  UNCHANGED by this pair being here.  Net behaviour change: ZERO.
//
//  READ THIS FIRST -- THIS ONE IS A SIZE-HEURISTIC FALSE POSITIVE, NOT LOST
//  TEXT.  Unlike the other pairs added by this wave, golden's logic here is
//  ALREADY PRESENT in this tree; it is merely RELOCATED, which is exactly the
//  second way body_size_scan.py can mis-rank a function (see DEVLOG 20260810:
//  "某個區塊被正確搬到別的檔案也會讓本體變短").  Golden :1111-1200 (the whole
//  len==12/14/16/19/22 decode plus the IsCorrectDateFormat + hour/min/sec/hund
//  range checks) lives at ParseSECSDateTimeString, this file :764-852, field
//  for field and comment for comment; golden's settime()/setdate() pair
//  (:1196-1197) is the SetSystemDateTimeHook seam.  The live body is 44 lines
//  vs golden's 122 ONLY because ~78 lines were extracted into that helper.
//  This block is therefore a REFERENCE COPY of golden's original one-function
//  form (useful for auditing the split), NOT a restoration of missing text --
//  do NOT un-gate it, that would duplicate a definition.  Recorded here
//  because the wave brief listed it; the honest classification is in the
//  wave report.
//
//  NOTHING inside the gate is fixed, renamed, reflowed or reindented --
//  golden's own defects are preserved ON PURPOSE so a diff against golden
//  stays EMPTY (this wave's report lists them with their golden line
//  numbers).  Same shape as this tree's existing pairs: atester.cpp
//  GetTesterResult (golden 1,705 gated / 6-line live) and csystem.cpp
//  MainProc (2,390 / 26), DoTrayFeedProcess (1,235 / 6),
//  CheckContinusStartIsReady (700 / 9), DoAllProcess.  Being gated it needs
//  NO callee to exist -- only lexical validity; no stub, declaration or
//  header edit was added anywhere for its symbols.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden SECSGEM/uHGemClass.cpp:1095-1216.  INERT reference text; the live HTGem::S2F32_DateAndTimeAcknowledge follows the #endif.
void HTGem::S2F32_DateAndTimeAcknowledge()
{
    char str[256];
    int len,ret;
    unsigned char Type;
    bool Error=false;
    AnsiString S, S1;
//    unsigned YY,MM,DD,hh,mm,ss,cc;
    struct  time t;
    struct date reset;

    ret=HGemPtr->GetDataItemLenAndType(len, Type);
    if(Type==HType.ASCII_TYPE && ret==1)
    {
        if(HGemPtr->DataItemIn(len, Type, str)==1)
        {
            if(len==12)                                                         // "030601134700"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 2).c_str())+2000;
                reset.da_mon    =atoi(S.SubString(3, 2).c_str());
                reset.da_day    =atoi(S.SubString(5, 2).c_str());
                t.ti_hour       =atoi(S.SubString(7, 2).c_str());
                t.ti_min        =atoi(S.SubString(9, 2).c_str());
                t.ti_sec        =atoi(S.SubString(11, 2).c_str());
                t.ti_hund       =0;
            }
            else if(len==14)                                                    // "20030602134700"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 4).c_str());
                reset.da_mon    =atoi(S.SubString(5, 2).c_str());
                reset.da_day    =atoi(S.SubString(7, 2).c_str());
                t.ti_hour       =atoi(S.SubString(9, 2).c_str());
                t.ti_min        =atoi(S.SubString(11, 2).c_str());
                t.ti_sec        =atoi(S.SubString(13, 2).c_str());
                t.ti_hund       =0;
            }
            else if(len==16)                                                    // "2003060313401000"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 4).c_str());
                reset.da_mon    =atoi(S.SubString(5, 2).c_str());
                reset.da_day    =atoi(S.SubString(7, 2).c_str());
                t.ti_hour       =atoi(S.SubString(9, 2).c_str());
                t.ti_min        =atoi(S.SubString(11, 2).c_str());
                t.ti_sec        =atoi(S.SubString(13, 2).c_str());
                t.ti_hund       =atoi(S.SubString(15, 2).c_str());
            }
            else if(len==19)                                                    //  "2003-06-04T13:01:01"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 4).c_str());
                reset.da_mon    =atoi(S.SubString(6, 2).c_str());
                reset.da_day    =atoi(S.SubString(9, 2).c_str());
                t.ti_hour       =atoi(S.SubString(12, 2).c_str());
                t.ti_min        =atoi(S.SubString(15, 2).c_str());
                t.ti_sec        =atoi(S.SubString(18, 2).c_str());
                t.ti_hund       =0;
            }
            else if(len==21)                                                    //  "2003-06-05T13:01:01.2"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 4).c_str());
                reset.da_mon    =atoi(S.SubString(6, 2).c_str());
                reset.da_day    =atoi(S.SubString(9, 2).c_str());
                t.ti_hour       =atoi(S.SubString(12, 2).c_str());
                t.ti_min        =atoi(S.SubString(15, 2).c_str());
                t.ti_sec        =atoi(S.SubString(18, 2).c_str());
                t.ti_hund       =atoi(S.SubString(21, 1).c_str())*10;
            }
            else if(len==22)                                                    //  "2003-06-06T13:01:01.25"
            {
                S=str;
                reset.da_year   =atoi(S.SubString(1, 4).c_str());
                reset.da_mon    =atoi(S.SubString(6, 2).c_str());
                reset.da_day    =atoi(S.SubString(9, 2).c_str());
                t.ti_hour       =atoi(S.SubString(12, 2).c_str());
                t.ti_min        =atoi(S.SubString(15, 2).c_str());
                t.ti_sec        =atoi(S.SubString(18, 2).c_str());
                t.ti_hund       =atoi(S.SubString(21, 2).c_str());
            }
            else
            {
                Error=true;
            }

            if(Error==false)
            {
                if(IsCorrectDateFormat(reset.da_year, reset.da_mon, reset.da_day))
                {
                    if(t.ti_hour>23)                                            //Steven 20140404 : 避免Warning
                        Error=true;
                    if(t.ti_min>59)
                        Error=true;
                    if(t.ti_sec>59)
                        Error=true;
                    if(t.ti_hund>99)
                        Error=true;
                    if(Error==false)
                    {
                        settime(&t);
                        setdate(&reset);
                    }
                }
            }
        }
        else
        {
            Error=true;
        }
    }
    else
    {
        Error=true;
    }

    if(Error==true)
        HGemPtr->LocalAcknowledge(2, 32, 1);
    else
        HGemPtr->LocalAcknowledge(2, 32, 0);
}
#endif // GOLDEN VERBATIM -- golden SECSGEM/uHGemClass.cpp:1095-1216 (end)
void HTGem::S2F32_DateAndTimeAcknowledge()
{
    char str[256];
    int len,ret;
    unsigned char Type;
    bool Error=false;
    AnsiString S;
    SECSDateTimeFields parsed;
    bool bRangeError=false, bApplyClock=false;

    ret=ActiveWire->GetDataItemLenAndType(len, Type);
    if(Type==HType.ASCII_TYPE && ret==1)
    {
        if(ActiveWire->DataItemIn(len, Type, str)==1)
        {
            S=str;
            if(ParseSECSDateTimeString(S, len, parsed, bRangeError, bApplyClock))
            {
                if(bRangeError==true)
                    Error=true;
                if(bApplyClock==true && SetSystemDateTimeHook)
                    SetSystemDateTimeHook(parsed.year, parsed.month, parsed.day,
                                          parsed.hour, parsed.min, parsed.sec, parsed.hundredths);
            }
            else
            {
                Error=true;
            }
        }
        else
        {
            Error=true;
        }
    }
    else
    {
        Error=true;
    }

    if(Error==true)
        ActiveWire->LocalAcknowledge(2, 32, 1);
    else
        ActiveWire->LocalAcknowledge(2, 32, 0);
}
//---------------------------------------------------------------------------
// [S2,F34] Define Report -- ack-code sub (DRACK).
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1218-1291). DataItemIn/GetDataItemLenAndType/GetDataItemLenAndTypeAndDelete
// -> ActiveWire-> (wire-codec primitives); slTempReportID/lTempReportIDContent/
// DeleteAllHostDefineReportID stay HGemPtr-> (real THGem state/method, no
// wire-codec engine home). DEPENDENCY WIDENING: the CUSTOMER_CODE==
// CC_ONSEMI_M branch below needs cmydef.h, now included (see this file's own
// file-head note).
int HTGem::S2F34_DefineReportAcknowledgeSub()
{
    int len, svlen, ret, temp;
    unsigned char Type;
    AnsiString DATAID, ReportID, sSVID;
    TStringList *P;

    for(int i=0; i<HGemPtr->slTempReportID->Count; i++)                         // 若上一次的資料尚未清除則要清除
    {
        P=(TStringList *) HGemPtr->lTempReportIDContent->Items[i];
        P->Clear();                                                             //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete P;
    }
    HGemPtr->slTempReportID->Clear();
    HGemPtr->lTempReportIDContent->Clear();

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        return -1;

    ActiveWire->GetDataItemLenAndType(len, Type);
    if(ActiveWire->DataItemIn(len, Type, DATAID)!=1)
        return -1;

    if(CUSTOMER_CODE==CC_ONSEMI_M)
    {
    }
    else
    {
        if(DATAID=="")                                                          //JerryYang 20250120 : modify
            return -1;
    }

    ret=ActiveWire->GetDataItemLenAndType(len, Type);                              // 取得多少 report id 要被定義
    if(ret!=1 || Type!=HType.LIST_TYPE)
        return -1;
    ActiveWire->DataItemIn(len, HType.LIST_TYPE, NULL);
    if(len==0)
    {
        HGemPtr->DeleteAllHostDefineReportID();
        return 1;
    }

    for(int i=0; i<len; i++)
    {
        if(ActiveWire->DataItemIn(2, HType.LIST_TYPE,NULL)!=1)
            return -1;
        if(ActiveWire->GetDataItemLenAndType(temp, Type)!=1)
            return -1;
        if(ActiveWire->DataItemIn(temp, Type, ReportID)!=1)
            return -1;
        ret=ActiveWire->GetDataItemLenAndTypeAndDelete(svlen, Type);
        if(ret!=1 || Type!=HType.LIST_TYPE)
            return -1;

        P=new TStringList;
        P->Clear();
        for(int j=0; j<svlen; j++)
        {
            ActiveWire->GetDataItemLenAndType(temp, Type);
            if(ActiveWire->DataItemIn(temp, Type, sSVID)==1)
            {
                P->Add(sSVID);
            }
            else
            {
                P->Clear();                                                     //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                delete P;
                return -1;
            }
        }
        HGemPtr->slTempReportID->Add(ReportID);
        HGemPtr->lTempReportIDContent->Add(P);
    }
    return 1;
}
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1294-1361). No wire-codec primitives called directly in this method's own
// body -- every HGemPtr-> call stays HGemPtr-> (real THGem state/methods,
// all already real per this wave's own header/A4 additions).
void HTGem::S2F34_ProcessHostSendReportID()
{
    AnsiString ReportID, SVID;
    TStringList *strPtr;

    // 檢查是否已經定義了
    for(int i=0; i<HGemPtr->slTempReportID->Count; i++)
    {
        ReportID=HGemPtr->slTempReportID->Strings[i];
        strPtr  =(TStringList *)HGemPtr->lTempReportIDContent->Items[i];
        if(HGemPtr->CheckReportAlreadyDefine(ReportID) && strPtr->Count!=0)
        {
            HGemPtr->DefineReportAcknowledgeAlreadyDefined();
            return;
        }
    }
    // 檢查 SVID 是否有不存在的
    for(int i=0; i<HGemPtr->slTempReportID->Count; i++)
    {
        strPtr=(TStringList *)HGemPtr->lTempReportIDContent->Items[i];
        for(int j=0; j<strPtr->Count; j++)
        {
            SVID=strPtr->Strings[j];
            if(HGemPtr->IsValidSVID(SVID)==false)
            {
                HGemPtr->DefineReportAcknowledgeInvalidSVID();
                return;
            }
        }
    }
    unsigned SVIDBuffer[1024];
    for(int i=0; i<HGemPtr->slTempReportID->Count; i++)
    {
        ReportID=HGemPtr->slTempReportID->Strings[i];
        strPtr=(TStringList *)HGemPtr->lTempReportIDContent->Items[i];
        if(strPtr->Count==0)
        {
            HGemPtr->DeleteReportID(atoi(ReportID.c_str()), 0);                 //  把 report id 表內地 report id 全部 delete  2013/11/20
            HGemPtr->DeleteReportIDOfCeid(atoi(ReportID.c_str()));              // 把 ceid   id 表內地 report id 全部 delete
        }
        else
        {
            if(strPtr->Count>1024)
            {
                HGemPtr->DefineReportAcknowledgeInsufficientSpace();
                return;
            }

            for(int j=0; j<strPtr->Count; j++)
            {
                SVID=strPtr->Strings[j];
                SVIDBuffer[j]=atoi(SVID.c_str());
            }

            if(HGemPtr->SetReportIDContent(atoi(ReportID.c_str()), strPtr->Count, SVIDBuffer, 0)==false)
            {
                HGemPtr->DefineReportAcknowledgeInsufficientSpace();
                return;
            }
        }
        strPtr->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete strPtr;
    }
    HGemPtr->slTempReportID->Clear();
    HGemPtr->lTempReportIDContent->Clear();
    HGemPtr->DefineReportAcknowledgeAccept();
    HGemPtr->SaveEventReportData();
}
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1363-1393). No wire-codec primitives called directly (calls its own
// Sub() sibling + HGemPtr->DefineReportAcknowledgeFormatError, both real).
void HTGem::S2F34_DefineReportAcknowledge()
{
    int ret;
    ret=S2F34_DefineReportAcknowledgeSub();
    if(ret==-1)
        HGemPtr->DefineReportAcknowledgeFormatError();
    else
        S2F34_ProcessHostSendReportID();
}
//---------------------------------------------------------------------------
// [S2,F36] Link Event Report -- ack-code sub (LRACK).
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1395-1457). Same DataItemIn/GetDataItemLenAndType/
// GetDataItemLenAndTypeAndDelete -> ActiveWire-> rule as
// S2F34_DefineReportAcknowledgeSub above; slTempCeID/lTempCeIDContent/
// DeleteAllHostDefineCeid stay HGemPtr->.
int HTGem::S2F36_LinkEventReportAcknowledgeSub()
{
    int len, svlen, ret, temp;
    unsigned char Type;
    AnsiString DATAID,Ceid,sReportID;
    TStringList *P;

    for(int i=0; i<HGemPtr->slTempCeID->Count; i++)                             // 若上一次遞資料尚未清除記憶體則要清除
    {
        P=(TStringList *) HGemPtr->lTempCeIDContent->Items[i];
        P->Clear();                                                             //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete P;
    }
    HGemPtr->slTempCeID->Clear();
    HGemPtr->lTempCeIDContent->Clear();

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        return -1;

    ActiveWire->GetDataItemLenAndType(len, Type);
    if(ActiveWire->DataItemIn(len, Type, DATAID)!=1)
        return -1;
    ret=ActiveWire->GetDataItemLenAndType(len, Type);                              // 取得多少 report id 要被定義
    if(ret!=1 || Type!=HType.LIST_TYPE)
        return -1;

    ActiveWire->DataItemIn(len, HType.LIST_TYPE, NULL);
    if(len==0)
    {
        HGemPtr->DeleteAllHostDefineCeid();
        return 1;
    }

    for(int i=0; i<len; i++)
    {
        if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
            return -1;

        ActiveWire->GetDataItemLenAndType(temp, Type);
        if(ActiveWire->DataItemIn(temp, Type, Ceid)!=1)
            return -1;

        ret=ActiveWire->GetDataItemLenAndTypeAndDelete(svlen, Type);
        if(ret!=1 || Type!=HType.LIST_TYPE)
            return -1;

        P=new TStringList;
        P->Clear();
        for(int j=0; j<svlen; j++)
        {
            ActiveWire->GetDataItemLenAndType(temp, Type);
            if(ActiveWire->DataItemIn(temp, Type, sReportID)!=1)
            {
                P->Clear();                                                     //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                delete P;
                return -1;
            }
            P->Add(sReportID);
        }
        HGemPtr->slTempCeID->Add(Ceid);
        HGemPtr->lTempCeIDContent->Add(P);
    }
    return 1;
}
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1460-1477). No wire-codec primitives called directly (calls its own
// Sub() sibling + HGemPtr->LinkReportAcknowledgeFormatError/
// ProcessHostSendReportLinkID, both real).
void HTGem::S2F36_LinkEventReportAcknowledge()
{
    int ret;
    ret=S2F36_LinkEventReportAcknowledgeSub();
    if(ret==-1)
        HGemPtr->LinkReportAcknowledgeFormatError();
    else
        HGemPtr->ProcessHostSendReportLinkID();
}
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1480-1521). DataItemIn/GetDataItemLenAndTypeAndDelete/GetDataItemLenAndType
// -> ActiveWire->; CheckCEIDExist/EnableDisableEventReportAcknowledge
// CeidNotExist/...FormatError/EnableDisableEventReport stay HGemPtr-> (real
// THGem methods, all already real per this wave's own header/A4 additions).
void HTGem::S2F38_EnableDisableEventReportAcknowledge()
{
    int len, slen;
    unsigned char Type;
    bool CEED;
    unsigned CEID[1024];
    AnsiString sID;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE,NULL)==1)
    {
        if(ActiveWire->DataItemIn(1, HType.BOOLEAN_TYPE, &CEED))
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(slen, Type)==1)
            {
                if(Type==HType.LIST_TYPE)
                {
                    for(int i=0; i<slen; i++)
                    {
                        ActiveWire->GetDataItemLenAndType(len, Type);
                        if(ActiveWire->DataItemIn(len, Type, sID)==1)
                        {
                            CEID[i]=atoi(sID.c_str());
                            if(HGemPtr->CheckCEIDExist(sID)==false)
                            {
                                HGemPtr->EnableDisableEventReportAcknowledgeCeidNotExist();
                                return;
                            }
                        }
                        else
                        {
                            HGemPtr->EnableDisableEventReportAcknowledgeFormatError();
                            return;
                        }
                    }
                    HGemPtr->EnableDisableEventReport(CEED, slen, CEID);
                    return;
                }
            }
        }
    }
    HGemPtr->EnableDisableEventReportAcknowledgeFormatError();
}
//---------------------------------------------------------------------------
// [S2,F42] Host Command Acknowledge (HCACK).
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:1523-1574):
// entirely DataItemIn/GetDataItemLenAndType/InitLocalHead/DataItemOut/
// SendLocalData -- all now WireCodec methods. PRESERVED GOLDEN QUIRK (not
// fixed): the `else` (format-error) path emits DataItemOut calls WITHOUT a
// preceding InitLocalHead -- golden's own asymmetry vs. the `if` branch,
// confirmed by direct read, not a translation gap.
//---------------------------------------------------------------------------
int HTGem::S2F42_Host_Command_Acknowledge()
{
    unsigned char HCACK=1;
    char CommandStr[1024];
    AnsiString PPID;
    int ret,len;
    unsigned char Type;
    AnsiString S, S1;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)                       // 需要補充多重 Command
    {
        ActiveWire->GetDataItemLenAndType(len, Type);
        if(Type==HType.ASCII_TYPE)
        {
            ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
            if(ret==-1)
                S="";
        }
        else
        {
            S="";
        }
        S=CommandStr;
        S=S.UpperCase();
        HCACK=1;
        ActiveWire->InitLocalHead(2, 42, 0);

        if(HCACK==0)
        {
            ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
            ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        }
        else
        {
            ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
            ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        }

        ActiveWire->SendLocalData();
        return 1;
    }
    else
    {
        HCACK=3;
        ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
        ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
    }
    return HCACK;
}
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:1576-1584; fixed 20260713 by an independent fidelity review, was mis-cited 1576-1591):
// entirely InitLocalHead/DataItemOut/SendLocalData -- all now WireCodec.
//---------------------------------------------------------------------------
void HTGem::S2F44_ResetSpoolingAcknowledge()
{
    unsigned char C=0;
    ActiveWire->InitLocalHead(2, 44,0);
    ActiveWire->DataItemOut( 2,HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut( 1,HType.BINARY_TYPE,&C);
    ActiveWire->DataItemOut( 0,HType.LIST_TYPE, NULL);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1593-1624). DataItemIn/GetDataItemLenAndType/LocalAcknowledge ->
// ActiveWire-> (wire-codec primitives); EnableDisableAlarmAll/
// EnableDisableAlarm stay HGemPtr-> (real THGem methods, added this wave).
void HTGem::S5F4_EnableDisableAlarmAcknowledge()
{
    unsigned char ALED, Type;
    int len;
    AnsiString ID;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, &ALED))
        {
            ActiveWire->GetDataItemLenAndType(len, Type);
            if(len==0)
            {
                HGemPtr->EnableDisableAlarmAll(ALED);
                ActiveWire->LocalAcknowledge(5, 4, 0);
            }
            else
            {
                ActiveWire->DataItemIn(len, Type, ID);
                if(HGemPtr->EnableDisableAlarm(ID, ALED))
                    ActiveWire->LocalAcknowledge(5, 4, 0);
                else
                    ActiveWire->LocalAcknowledge(5, 4, 1);
            }
            return;
        }
    }
    S9F7_IllegalData("S5,F3 Data Format error !!!");
}
//---------------------------------------------------------------------------
// V 1.0
// S5,F6 This message contains the alarm data known to the equipment. There are “m” alarms in the list.
//---------------------------------------------------------------------------
// AI(W906-AlarmReportAck) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1626-1887). DataItemIn/GetDataItemLenAndType/InitLocalHead/DataItemOut/
// SendLocalData -> ActiveWire-> (wire-codec primitives); strGrdAlarm/
// GetAlarmIndex stay HGemPtr-> (real THGem StringGrid/method, no wire-codec
// engine home). golden `_atoi64` -> `strtoll`/`strtoull` (MinGW <cstdlib>
// substitution, see this file's own file-head note); the `(unsigned)
// strtoull(...)` 32-bit-truncating cast before storing into the 64-bit
// `uint8SV` is an EXISTING golden quirk (golden :1720), not introduced here.
void HTGem::S5F6_ListAlarmData()
{
    int SVlen,i, j,ret;
    unsigned char Type;

    __int64  int8SV;                                                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    int   int4SV;
    short int2SV;
    unsigned __int64  uint8SV;                                                  //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    unsigned int   uint4SV;
    unsigned short uint2SV;
    AnsiString str;
    AnsiString S;

    if(ActiveWire->GetDataItemLenAndType(SVlen,Type)==1)
    {
        if(SVlen!=0)
        {
            if(Type==HType.UINT_8_TYPE || Type==HType.UINT_4_TYPE || Type==HType.UINT_2_TYPE ||
               Type==HType.INT_8_TYPE  || Type==HType.INT_4_TYPE  || Type==HType.INT_2_TYPE)                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                unsigned __int64 *uint8Ptr;
                unsigned *uint4Ptr;
                unsigned short *uint2Ptr;
                __int64     *int8Ptr;
                int      *int4Ptr;
                short    *int2Ptr;
                unsigned char Mode=0x80;

                ret=ActiveWire->GetDataItemLenAndType(SVlen,Type);
                if(ret!=1)
                {
                    S9F7_IllegalData("S5,F5 Data Format error !!!");
                    return;
                }

                if(Type==HType.UINT_8_TYPE)                                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                {
                    uint8Ptr=new unsigned __int64 [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_8_TYPE,uint8Ptr);
                }
                else if(Type==HType.UINT_4_TYPE)
                {
                    uint4Ptr=new unsigned [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_4_TYPE,uint4Ptr);
                }
                else if(Type==HType.UINT_2_TYPE)
                {
                    uint2Ptr=new unsigned short [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_2_TYPE,uint2Ptr);
                }
                else if(Type==HType.INT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                {
                    int8Ptr=new __int64[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_8_TYPE,int8Ptr);
                }
                else if(Type==HType.INT_4_TYPE)
                {
                    int4Ptr=new int[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_4_TYPE,int4Ptr);
                }
                else if(Type==HType.INT_2_TYPE)
                {
                    int2Ptr=new short[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_2_TYPE,int2Ptr);
                }

                if(ret==1)
                {
                    ActiveWire->InitLocalHead(5,6,0);
                    ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
                    for(i=0; i<SVlen; i++)
                    {
                        if(Type==HType.UINT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            // AI(W906-AlarmReportAck) 20260721: cast to (long
                            // long) -- vclcompat AnsiString has no operator=
                            // matching `unsigned long long` exactly (ambiguous
                            // across int/unsigned int/long long/double), same
                            // established precedent as SecsWireCodec.cpp's own
                            // identical accommodation.
                            S=(long long)uint8Ptr[i];
                        else if(Type==HType.UINT_4_TYPE)
                            S=uint4Ptr[i];
                        else if(Type==HType.UINT_2_TYPE)
                            S=uint2Ptr[i];
                        else if(Type==HType.INT_8_TYPE)                         //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            S=int8Ptr[i];
                        else if(Type==HType.INT_4_TYPE)
                            S=int4Ptr[i];
                        else if(Type==HType.INT_2_TYPE)
                            S=int2Ptr[i];

                        j=HGemPtr->GetAlarmIndex(S);
                        if(j>=0)
                        {
                            ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                            Mode=atoi(HGemPtr->strGrdAlarm->Cells[2][j].c_str())+0x80;
                            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &Mode);  // mode
                            if(Type==HType.UINT_8_TYPE)                         //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            {
                                uint8SV=(unsigned)strtoull(HGemPtr->strGrdAlarm->Cells[1][j].c_str(), NULL, 10);   // golden: _atoi64 (MinGW substitution); (unsigned) truncation is an EXISTING golden quirk, see this method's own file-head-adjacent comment
                                ActiveWire->DataItemOut(1, HType.UINT_8_TYPE, &uint8SV);
                            }
                            else if(Type==HType.UINT_4_TYPE)
                            {
                                uint4SV=atoi(HGemPtr->strGrdAlarm->Cells[1][j].c_str());
                                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
                            }
                            else if(Type==HType.UINT_2_TYPE)
                            {
                                uint2SV=atoi(HGemPtr->strGrdAlarm->Cells[1][j].c_str());
                                ActiveWire->DataItemOut(1, HType.UINT_2_TYPE, &uint2SV);
                            }
                            else if(Type==HType.INT_8_TYPE)                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            {
                                int8SV=strtoll(HGemPtr->strGrdAlarm->Cells[1][j].c_str(), NULL, 10);   // golden: _atoi64 (MinGW substitution)
                                ActiveWire->DataItemOut(1, HType.INT_8_TYPE, &int8SV);
                            }
                            else if(Type==HType.INT_4_TYPE)
                            {
                                int4SV=atoi(HGemPtr->strGrdAlarm->Cells[1][j].c_str());
                                ActiveWire->DataItemOut(1, HType.INT_4_TYPE, &int4SV);
                            }
                            else if(Type==HType.INT_2_TYPE)
                            {
                                int2SV=atoi(HGemPtr->strGrdAlarm->Cells[1][j].c_str());
                                ActiveWire->DataItemOut(1, HType.INT_2_TYPE, &int2SV);
                            }

                            ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdAlarm->Cells[4][j]);                  // message
                        }
                        else
                        {
                            // AI(W906-AlarmReportAck) 20260721: golden quirk,
                            // preserved verbatim -- this DataItemOut call
                            // passes length 0, so `Mode`'s value (0x80 from
                            // its declaration above, never reassigned in THIS
                            // branch) is never actually transmitted. This is a
                            // genuine zero-length SECS-II item in golden
                            // itself (golden uHGemClass.cpp:1754), not a
                            // translation error -- do NOT "fix" it to a
                            // non-zero length.
                            ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                            ActiveWire->DataItemOut(0, HType.BINARY_TYPE,&Mode);   // mode -- zero length, see comment above
                            if(Type==HType.UINT_8_TYPE)                         //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            {
                                uint8SV=uint8Ptr[i];
                                ActiveWire->DataItemOut(1, HType.UINT_8_TYPE, &uint8SV);
                            }
                            else if(Type==HType.UINT_4_TYPE)
                            {
                                uint4SV=uint4Ptr[i];
                                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
                            }
                            else if(Type==HType.UINT_2_TYPE)
                            {
                                uint2SV=uint2Ptr[i];
                                ActiveWire->DataItemOut(1, HType.UINT_2_TYPE, &uint2SV);
                            }
                            else if(Type==HType.INT_8_TYPE)                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            {
                                int8SV=int8Ptr[i];
                                ActiveWire->DataItemOut(1, HType.INT_8_TYPE, &int8SV);
                            }
                            else if(Type==HType.INT_4_TYPE)
                            {
                                int4SV=int4Ptr[i];
                                ActiveWire->DataItemOut(1, HType.INT_4_TYPE, &int4SV);
                            }
                            else if(Type==HType.INT_2_TYPE)
                            {
                                int2SV=int2Ptr[i];
                                ActiveWire->DataItemOut(1, HType.INT_2_TYPE, &int2SV);
                            }

                            str="";
                            ActiveWire->DataItemOut(HType.ASCII_TYPE, str);
                        }
                    }
                    ActiveWire->SendLocalData();
                    if(Type==HType.UINT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] uint8Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint8Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        delete[] uint4Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint4Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        delete[] uint2Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint2Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] int8Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int8Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        delete[] int4Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int4Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        delete[] int2Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int2Ptr=NULL;                                           //kevin 20180517
                    }
                    return;
                }
                else
                {
                    S9F7_IllegalData("S5,F5 Data Format error !!!");
                    if(Type==HType.UINT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] uint8Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint8Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        delete[] uint4Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint4Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        delete[] uint2Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint2Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] int8Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int8Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        delete[] int4Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int4Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        delete[] int2Ptr;                                       //Ifor 20170603 修改陣列刪除方式 delete ==> delete[]
                        int2Ptr=NULL;                                           //kevin 20180517
                    }
                    return;
                }
            }
            else
            {
                S9F7_IllegalData("S5,F5 Data Format error !!!");
            }
        }
        else
        {
            ActiveWire->InitLocalHead(5, 6, 0);
            unsigned char Mode=0x80;
            ActiveWire->DataItemOut(HGemPtr->strGrdAlarm->RowCount-1, HType.LIST_TYPE, NULL);
            for(i=1; i<HGemPtr->strGrdAlarm->RowCount; i++)
            {
                ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                Mode=atoi(HGemPtr->strGrdAlarm->Cells[2][i].c_str())+0x80;
                ActiveWire->DataItemOut(1, HType.BINARY_TYPE,&Mode);               // mode
                uint4SV=atoi(HGemPtr->strGrdAlarm->Cells[1][i].c_str());
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);           // ID
                ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdAlarm->Cells[4][i]);
            }

            ActiveWire->SendLocalData();
            return;
        }
    }
    else
    {
        S9F7_IllegalData("S5,F5 Data Format error !!!");
    }
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1892-1923) -- "no defect" method (§2 category ①): strGrdAlarm (THGem's
// own StringGrid, HGemPtr->, unchanged) + wire -> ActiveWire-> is the WHOLE
// dependency chain -- zero new THGem surface.
void HTGem::S5F8_ListEnableAlarmAcknowledge()
{
    int SVlen, len;
    unsigned char Type;
    unsigned int   uint4SV;
    unsigned char ALT=0x80;
    if( ActiveWire->GetDataItemLenAndType(SVlen, Type)!=1)                       //pig 2014.08.01 SECS
    {
        S9F7_IllegalData("S5,F7 Data Format error !!!");
        return;
    }
    ActiveWire->InitLocalHead(5,8,0);
    len=0;
    for(int i=1; i<HGemPtr->strGrdAlarm->RowCount; i++)
        if(HGemPtr->strGrdAlarm->Cells[7][i]=="1")                              //wei 20180227 (Steven) S5F7 判斷位置錯誤 3-->7
            len++;

    ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);

    for(int i=1; i<HGemPtr->strGrdAlarm->RowCount; i++)
    {
        if(HGemPtr->strGrdAlarm->Cells[7][i]=="1")                              //wei 20180227 (Steven) S5F7 判斷位置錯誤 3-->7
        {
            ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &ALT);
            uint4SV=(unsigned)atoi(HGemPtr->strGrdAlarm->Cells[1][i].c_str());
            ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &uint4SV);
            ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdAlarm->Cells[4][i]);
        }
    }
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1925-1960). CheckCEIDExist/SendCeid are THGem's own methods (HGemPtr->,
// unchanged); wire -> ActiveWire->.
void HTGem::S6F16_EventReportData()
{
    int len, ret;
    unsigned char Type;
    AnsiString CEID;
    AnsiString S;
    unsigned iCeid,iDataID=1;

    ret=ActiveWire->GetDataItemLenAndType(len,Type);
    if(ret==1 && ActiveWire->DataItemIn(len, Type, CEID)==1)
    {
        iCeid=(unsigned)atoi(CEID.c_str());
        if(HGemPtr->CheckCEIDExist(CEID))
        {
            //if(IsEnableEvent(1,iCeid))                                        // 若 Host 要求 Equipment 送回某一個 CEID
            {                                                                   // 但此 CEID 並未被 enable 那麼依然回報嗎
                ActiveWire->InitLocalHead(6, 16, 0);                            // 目前先寫一定回報,若要依　enable 來決定
                ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);              // 則以下的　mark 打開就好
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &iDataID);
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &iCeid);
                HGemPtr->SendCeid(iCeid);
                ActiveWire->SendLocalData();
            }
        }
        else
        {
            ActiveWire->InitLocalHead(6, 16, 0);
            ActiveWire->DataItemOut(0, HType.UINT_4_TYPE, &iCeid);
            ActiveWire->SendLocalData();
        }
    }
    else
    {
        S9F7_IllegalData("S6,F15 Data Format error !!!");
    }
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1962-1997). Same rule as S6F16 above. GOLDEN BUG preserved verbatim: the
// "CEID not exist" error path below sends `InitLocalHead(6, 16, 0)` -- S,F
// 6,16, NOT 6,18 (a real golden copy-paste bug from S6F16's own error path
// right above it, confirmed by direct golden read, uHGemClass.cpp:1988) --
// not "fixed" to (6,18,0) here, per this project's faithful-translation
// mandate.
void HTGem::S6F18_AnnotatedEventReportData()
{
    int len, ret;
    unsigned char Type;
    AnsiString CEID;
    AnsiString S;
    unsigned iCeid,iDataID=1;

    ret=ActiveWire->GetDataItemLenAndType(len, Type);
    if(ret==1 && ActiveWire->DataItemIn(len, Type, CEID)==1)
    {
        iCeid=(unsigned)atoi(CEID.c_str());
        if(HGemPtr->CheckCEIDExist(CEID))
        {
            //if(IsEnableEvent(1,iCeid))                                        // 若 Host 要求 Equipment 送回某一個 CEID
            {                                                                   // 但此 CEID 並未被 enable 那麼依然回報嗎
                ActiveWire->InitLocalHead(6, 18, 0);                            // 目前先寫一定回報,若要依　enable 來決定
                ActiveWire->DataItemOut(3, HType.LIST_TYPE,NULL);               // 則以下的　mark 打開就好
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE,&iDataID);
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE,&iCeid);
                HGemPtr->SendAnnotatedCeid(iCeid);
                ActiveWire->SendLocalData();
            }
        }
        else
        {
            ActiveWire->InitLocalHead(6, 16, 0);                               // golden bug preserved -- see comment above
            ActiveWire->DataItemOut(0, HType.UINT_4_TYPE, &iCeid);
            ActiveWire->SendLocalData();
        }
    }
    else
    {
        S9F7_IllegalData("S6,F17 Data Format error !!!");
    }
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 1999-2052). stdGridReportID is THGem's own StringGrid (HGemPtr->,
// unchanged); SV_ID -> HGemPtr->SvEcReg.SV_ID (SV/EC bookkeeping embed);
// DataItemOutSV -> HGemPtr-> (THGem's own method); wire -> ActiveWire->.
void HTGem::S6F20_IndividualReportData()
{
    int ret, x, y, ict, len;
    AnsiString RptID,SVID;
    unsigned char Type;

    ret=ActiveWire->GetDataItemLenAndType(len, Type);

    if(ret==1 && ActiveWire->DataItemIn(len, Type, RptID)==1)
    {
        for(y=1; y<HGemPtr->stdGridReportID->RowCount; y++)
        {
            if(HGemPtr->stdGridReportID->Cells[0][y]==RptID)
            {
                ict=0;
                for(x=2; x<HGemPtr->stdGridReportID->ColCount; x++)
                {
                    if(HGemPtr->stdGridReportID->Cells[x][y]!="")
                    {
                        SVID=HGemPtr->stdGridReportID->Cells[x][y];
                        if(HGemPtr->SvEcReg.SV_ID->IndexOf(SVID)>=0)             //Ifor 20260402: fix IndexOf>=0 (was >0, would miss index 0)
                        {
                            ict++;
                        }
                        else
                        {
                            S9F7_IllegalData("S6,F19 Invalid SVID in Report ");
                            return;
                        }
                    }
                }
                ActiveWire->InitLocalHead(6, 20, 0);
                ActiveWire->DataItemOut(ict, HType.LIST_TYPE, NULL);
                for(x=2; x<HGemPtr->stdGridReportID->ColCount; x++)
                {
                    if(HGemPtr->stdGridReportID->Cells[x][y]!="")
                    {
                        SVID=HGemPtr->stdGridReportID->Cells[x][y];
                        HGemPtr->DataItemOutSV(SVID);
                    }
                }
                ActiveWire->SendLocalData();
                return;
            }
        }
    }
    S9F7_IllegalData("S6,F19 Data Format error !!!");
}
//---------------------------------------------------------------------------
// [S6,F24] Request Spooled Data Acknowledge (RSDA).
// AI(W906-uHGemClass-Micro6) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2053-2079): needed 3 new THGem plain-scalar members (bSpoolActive/
// bBeginTransferSpool/GemSpoolPath, uHGemEquipment.h, this same wave).
// `HGemPtr->DataItemIn`/`HGemPtr->LocalAcknowledge` (wire primitives) ->
// `ActiveWire->...`; `HGemPtr->bSpoolActive`/`bBeginTransferSpool`/
// `GemSpoolPath` (real THGem data) stay `HGemPtr->`, unchanged -- same split
// as every prior integrate wave. GOLDEN QUIRK preserved verbatim: the
// spool-wipe command string `"del "+path+"*.*/q/f"` is missing the spaces
// before `/q`/`/f` that would make those real DOS `del` switches (same shape
// recurs in golden uHGemEquipment.cpp:4102/6144, so this is a pre-existing
// golden bug, not introduced here) -- NOT corrected. With this wave's safe
// `bSpoolActive=false` ctor default, this `system()` call is a no-op path
// until some future "DoSpool subsystem" wave drives it true (not built this
// wave, per plan).
//---------------------------------------------------------------------------
void HTGem::S6F24_RequestSpooledDataAcknowledgementSend()
{
    unsigned char RSDC;
    AnsiString S;
    if(HGemPtr->bSpoolActive==false)
    {
        ActiveWire->LocalAcknowledge(6, 24, 0);
        return;
    }
    HGemPtr->bBeginTransferSpool=true;
    if(ActiveWire->DataItemIn(1, HType.UINT_1_TYPE, &RSDC)==1)
    {
        if(RSDC==1)
        {
            S=AnsiString("del ")+IncludeTrailingPathDelimiter(HGemPtr->GemSpoolPath)+AnsiString("*.*/q/f");  // golden quirk: missing spaces before /q /f, see comment above
            system(S.c_str());
        }
        ActiveWire->LocalAcknowledge(6, 24, 0);
    }
    else
    {
        ActiveWire->LocalAcknowledge(6, 24, 0);
    }
}
//---------------------------------------------------------------------------
// [S7,F2] Process Program Load Grant (PPGNT).
// AI(W906-uHGemClass-Unlock2) 20260716 UN-GATED (golden SECSGEM/uHGemClass.cpp:2081-2113):
// its sole recorded blocker, GetDataItemLenAndTypeAndDelete, is now a real
// WireCodec method (SecsWireCodec.h/.cpp "WAVE 3" addendum); everything else
// in this body was already WireCodec-only (DataItemIn/GetDataItemLenAndType/
// LocalAcknowledge), and its error path (S9F7_IllegalData) was un-gated back
// in the first integrate wave. `HGemPtr->` -> `WireCodec.`, nothing else
// changed vs. golden.
//---------------------------------------------------------------------------
int HTGem::S7F2_ProcessProgramLoadGrant()
{
// L,2
//    1. <PPID>          // ascii
//    2. <LENGTH>        // int ,uint
    int ret,len;
    unsigned char Type;
    AnsiString PPID;
    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        ret=ActiveWire->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            ActiveWire->DataItemIn(len, Type, PPID);
            ret=ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type);
            if(ret==1 && len==1 &&
                 (Type==HType.UINT_1_TYPE || Type==HType.UINT_2_TYPE ||
                  Type==HType.UINT_4_TYPE || Type==HType.UINT_8_TYPE ||
                  Type==HType.INT_1_TYPE  || Type==HType.INT_2_TYPE  ||
                  Type==HType.INT_4_TYPE  || Type==HType.INT_8_TYPE))
            {
                ActiveWire->LocalAcknowledge(7, 2, 0);
                return 1;
            }
        }
    }
    S9F7_IllegalData("S7,F1 Data Format error !!!");
    return 0;
}
//---------------------------------------------------------------------------
// [S7,F18] Delete Process Program Acknowledge (ACKC7).
// AI(W906-uHGemClass-Micro6) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2115-2166): its 2 recorded blockers are now both real -- THGem::UpLoadPath
// (new plain-scalar member, uHGemEquipment.h, this same wave) and the free
// function DeleteDirectory (Public/ExternFunction.h/.cpp, this same wave,
// translated from golden's own ExternFunction.cpp:249-280 -- the "W3
// DEFERRED, needs FindFirst/FindNext shim" note on it was stale, see that
// file's own updated comment). GetDataItemLenAndTypeAndDelete was already a
// real ActiveWire/WireCodec method since INTEGRATE WAVE 2. GetLastOpenFN() is
// the real GLOBAL free function (common.h/common.cpp, un-gated in an earlier
// wave) -- confirmed by reading golden uHGemClass.h/uHGemEquipment.h that
// neither declares a member of that name, so golden's own unqualified call
// here really is the global, not some THGem method reached implicitly.
// `HGemPtr->GetDataItemLenAndTypeAndDelete/GetDataItemLenAndType/DataItemIn/
// LocalAcknowledge` (wire primitives) -> `ActiveWire->...`; `HGemPtr->
// UpLoadPath` (real THGem data) stays `HGemPtr->`, unchanged -- same split as
// every prior integrate wave. GOLDEN QUIRKS preserved verbatim (see file-head
// note above for the full list): `ret=DeleteDirectory(S); if(ret==0)` is a
// bool->int assignment compared with `==0` rather than `==false`; the
// "currently open recipe" (`asLastFileName==PPID`) guard short-circuits with
// LocalAcknowledge(7,18,1) BEFORE DirectoryExists/DeleteDirectory even run for
// that PPID entry; a non-ASCII_TYPE list item is silently skipped (the `for`
// loop's `i` still advances, but neither GetDataItemLenAndTypeAndDelete nor
// DataItemIn consume that item's body) -- none of these three are corrected.
//---------------------------------------------------------------------------
void HTGem::S7F18_DeleteProcessProgramAcknowledge()                             // AI(W5-SECSGEM-Translate) 20260710: __fastcall dropped (see .h note)
{
    /*
    L,n     (Number of process programs to be deleted)
        1. <PPID1>
         .
         .
        n. <PPIDn>
    */
    char PPID[1024];
    int len, ret, slen;
    unsigned char Type;
    AnsiString S;
    AnsiString asLastFileName=GetLastOpenFN();                                  //JerryYang 20170626 (Steven) 使用中的工作檔不能被刪除

    ret=ActiveWire->GetDataItemLenAndTypeAndDelete(slen, Type);
    if(ret!=1 || Type!=HType.LIST_TYPE)
    {
        S9F7_IllegalData("S7,F17 Data Format error !!!");
        return;
    }

    for(int i=0; i<slen; i++)
    {
        ret=ActiveWire->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            ActiveWire->DataItemIn(len, Type, PPID);                            // PPID=filename;
            if(asLastFileName==PPID)                                            //JerryYang 20170626 (Steven) 使用中的工作檔不能被刪除
            {
                ActiveWire->LocalAcknowledge(7, 18, 1);
                return;
            }
            S=HGemPtr->UpLoadPath+"\\"+PPID;
            if(DirectoryExists(S)==false)                                       //JerryYang 20170626 (Steven) 不存在的工作檔要return 4
            {
                ActiveWire->LocalAcknowledge(7, 18, 4);
                return;
            }
            ret=DeleteDirectory(S);
            if(ret==0)
            {
                ActiveWire->LocalAcknowledge(7, 18, 1);
                return;
            }
        }
    }
    ActiveWire->LocalAcknowledge(7,18,0);
}
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-TraceUnlock) 20260728: INVESTIGATED, STILL GATED --
// refreshed blocker citation (2 of the original 3 named blockers are STALE,
// confirmed by direct re-grep of uHGemEquipment.h/.cpp this wave):
//   * SV_70_UNT1_ReceipeStruct already EXISTS as a real THGem member
//     (uHGemEquipment.h:995, golden :707/SV70) -- NOT a blocker.
//   * UploadFileString already EXISTS as a real THGem member, `new`'d in the
//     ctor / NULL-guarded Clear-then-delete in the dtor (uHGemEquipment.h:
//     ~1013, added by the W906-uHGemClass-Micro5 wave for S101F2/S101F4's
//     sake) -- NOT a blocker, exactly as this task's own brief anticipated
//     checking for.
// The GENUINE remaining blocker is `THGem::SetReceipeDirectoryAndGlobalName`
// itself (golden uHGemEquipment.cpp:6427-6489) -- confirmed absent (grepped
// uHGemEquipment.h/.cpp, zero hits besides comments). Golden S7F20 only
// reaches this call when SV_70_UNT1_ReceipeStruct==2 (golden uHGemClass.cpp:
// 2171-2177 early-returns with an empty-list reply otherwise) -- but every
// call it DOES make passes `Type==2` (golden uHGemClass.cpp:2179, the ONLY
// call site anywhere in golden that passes 2 -- SECSGEM.cpp:867/
// UsecegemMainFrom.cpp:695 both pass Type==2 as well), and golden's OWN
// Type==2 branch (uHGemEquipment.cpp:
// 6436-6460) is fundamentally different in kind from its Type==0/1 siblings:
// it does NOT just build a file-mask string -- it assigns
// `DirectoryListBox1->Directory=Path;` and then walks
// `DirectoryListBox1->Items` (a live VCL `TDirectoryListBox`'s already-
// populated, OS-driven directory-tree listing for that path) to compute
// `ct` (an index into that listing) and populate `UploadFileString` from the
// entries AFTER it. This is NOT a data member gap this wave can mechanically
// add (unlike UploadFileString/SV_70_UNT1_ReceipeStruct above) -- it needs a
// vclcompat stand-in that actually WALKS A REAL FILESYSTEM DIRECTORY TREE
// and reproduces TDirectoryListBox's own Items-population semantics
// (subdirectory listing at each path-nesting level, matching BCB6's own
// `TDirectoryListBox` behavior) -- confirmed NO such stand-in exists
// anywhere in vclcompat/ (grepped "DirectoryListBox"/"TDirectoryListBox" --
// zero hits outside docs/*.md). This is a genuine, non-trivial design task
// (a real directory-walking widget, not a thin seam or a plain data member),
// exactly the class of blocker this task's own brief anticipated when it
// said "if it's genuinely not available, this method likely cannot land
// this wave; confirm and document rather than force it" -- confirmed here;
// NOT attempted this wave. A future wave un-gating this method would need to
// either (a) build a real TDirectoryListBox-equivalent vclcompat stand-in
// (using <filesystem>/FindFirst-FindNext, matching this project's existing
// SysUtils.h precedent for other directory-walking needs), or (b) special-
// case SetReceipeDirectoryAndGlobalName's Type==2 branch specifically for
// S7F20's actual call pattern rather than modeling the full widget.
void HTGem::S7F20_CurrentEPPDData()
{
#if 0 // TODO(W906-uHGemClass-Unlock, needs THGem::SetReceipeDirectoryAndGlobalName's Type==2 branch -- a live TDirectoryListBox directory-tree-walking widget, NOT yet in vclcompat; SV_70_UNT1_ReceipeStruct/UploadFileString are ALREADY real, see this method's own comment above for the full re-investigation) -- golden SECSGEM/uHGemClass.cpp:2168-2189
#endif
}
//---------------------------------------------------------------------------
// [S7,F20 -> Process] Current EPPID Data (incoming remote-recipe checklist).
// AI(W906-uHGemClass-Micro5) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2191-2220): sole recorded blockers were THGem's own TCheckListBox
// GemRemoteReceipeList (now the THGemListBox stand-in, uHGemEquipment.h, this
// same wave) + GetDataItemLenAndTypeAndDelete (already real, ActiveWire->).
// `HGemPtr->GemRemoteReceipeList` (real THGem state) stays HGemPtr->;
// `HGemPtr->GetDataItemLenAndTypeAndDelete`/`HGemPtr->DataItemIn` (wire
// primitives) -> ActiveWire->, same mechanical-rename rule as every prior
// integrate wave. GOLDEN NULL-GUARD preserved verbatim (golden :2196-2197):
// if GemRemoteReceipeList==NULL, return immediately WITHOUT ever touching the
// wire -- do NOT pre-allocate this pointer "helpfully"; that would silently
// change behavior for any path relying on this short-circuit.
//---------------------------------------------------------------------------
void HTGem::Process_S7F20_CurrentEPPIDData()
{
    int count;
    unsigned char Type;
    char str[256];
    if(HGemPtr->GemRemoteReceipeList==NULL)
        return;

    HGemPtr->GemRemoteReceipeList->Clear();

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(count, Type)==1)
    {
        if(Type==HType.LIST_TYPE)
        {
            for(int i=0; i<count; i++)
            {
                if(ActiveWire->DataItemIn(256, HType.ASCII_TYPE, str))
                {
                    HGemPtr->GemRemoteReceipeList->Items->Add(str);
                }
                else
                {
                    return;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
// [S9,F1] Unrecognized Device ID.                                //Ifor 20260402
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:2222-2228):
// this and its 4 S9Fx siblings below share an identical 4-call shape --
// StringOut+InitLocalHead+DataItemOut(ASCII,S)+SendLocalData -- all now
// WireCodec methods, nothing else needed.
//---------------------------------------------------------------------------
void HTGem::S9F1_UnrecognizedDeviceID(AnsiString S)
{
    ActiveWire->StringOut(S);
    ActiveWire->InitLocalHead(9, 1, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, S);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S9,F3] Unrecognized Stream Function Type.                    //KenHsieh 20221006
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:2232-2238).
//---------------------------------------------------------------------------
void HTGem::S9F3_Unrecognized_Stream_Function_Type(AnsiString S)
{
    ActiveWire->StringOut(S);
    ActiveWire->InitLocalHead(9, 3, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, S);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S9,F5] Unrecognized Function Type.                            //Ifor 20260402
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:2242-2248).
//---------------------------------------------------------------------------
void HTGem::S9F5_UnrecognizedFunctionType(AnsiString S)
{
    ActiveWire->StringOut(S);
    ActiveWire->InitLocalHead(9, 5, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, S);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S9,F7] Illegal Data.
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:2253-2259).
// This is the most-called-internally of the 5 (nearly every other gated
// method in this file calls it on a format-error path) -- un-gating it means
// those still-gated callers now emit a REAL S9F7 reply instead of silently
// swallowing the error, even while the caller's own main body stays gated.
//---------------------------------------------------------------------------
void HTGem::S9F7_IllegalData(AnsiString S)
{
    ActiveWire->StringOut(S);
    ActiveWire->InitLocalHead(9, 7, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, S);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S9,F9] Transaction Timer Timeout.                             //Ifor 20260402
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:2263-2269).
//---------------------------------------------------------------------------
void HTGem::S9F9_TransactionTimerTimeout(AnsiString S)
{
    ActiveWire->StringOut(S);
    ActiveWire->InitLocalHead(9, 9, 0);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, S);
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S10,F4] Terminal Display Single Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro7) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2274-2382) -- sole recorded blockers were the 9 THGem Terminal* members
// (added this wave, uHGemEquipment.h: TerminalDisplayIndex/TerminalListboxPtr/
// TerminalEditPtr/TerminalPanelPtr + the "2"-suffixed mirror set;
// TerminalMemoPtr already existed). MECHANICAL RENAME: golden
// `HGemPtr->GetDataItemLenAndTypeAndDelete/DataItemIn/Remote.W_Bit/
// LocalAcknowledge` (wire-codec primitives) -> `ActiveWire->...`;
// `HGemPtr->TerminalDisplayIndex/TerminalMemoPtr/TerminalListboxPtr/
// TerminalEditPtr/TerminalPanelPtr(+2 each)/GetTimeInfo()/TimeString` (real
// THGem state/methods) stay `HGemPtr->`, unchanged. `FMessageList`/
// `SecsAlarmMessage` are HTGem's OWN members (bare, no prefix, same as every
// other access to them in this file, e.g. this file's ctor/dtor).
// `CUSTOMER_CODE`/`CC_MAXIM_THAILAND`/`iSECSMessageCanCloseByOperator` are
// real bare globals (cmydef.h, already #include'd). `Now().FormatString(...)`
// -> `FormatDateTime(...)`, same substitution pattern already established at
// ainarm9045.cpp:890 (verified against vclcompat/TDateTime.cpp's own
// tokenizer: "mm" between "yyyy" and "dd_hh" correctly resolves to MONTH, not
// minute, because its nearest token neighbour is 'y', not 'h' -- see that
// file's own token-disambiguation comment).
// GOLDEN INVARIANT preserved verbatim, NOT a NULL-guard added by this port:
// TerminalMemoPtr/TerminalListboxPtr/TerminalEditPtr/TerminalPanelPtr (and
// their "2" mirrors) are dereferenced with NO NULL check whenever
// TerminalDisplayIndex(2)==1..4 -- golden's own real invariant is "whoever
// sets TerminalDisplayIndex to a non-zero value must have already wired the
// matching pointer" (out of this wave's scope, some future FormCreate-style
// wave). A caller/test that sets TerminalDisplayIndex!=0 without wiring the
// matching pointer crashes here in BOTH golden and this port, by design.
//---------------------------------------------------------------------------
void HTGem::S10F4_TerminalDisplaySingleAcknowledge()
{
    int len;
    unsigned char Type;
    unsigned char  uint1EC=0;
    char str[1024];

    int iDisplay=1;
    AnsiString S,S1;
    AnsiString sStartTime;
    sStartTime=FormatDateTime("yyyy:mm:dd_hh:nn:ss ", Now());   // golden: Now().FormatString("yyyy:mm:dd_hh:nn:ss ")

    if(HGemPtr->TerminalDisplayIndex!=0)
    {
        if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)==1)
        {
            if(Type==HType.LIST_TYPE && len==2)
            {
                if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, &uint1EC)==1)
                {
                    iSECSMessageCanCloseByOperator=uint1EC;                     //Ifor 20171024 : add KYEC SECS GEM Can Close By Operator 0:一般流程 1:必須清除Alarm 2:啟動工站檢查流程
                    if(ActiveWire->DataItemIn(1024, HType.ASCII_TYPE, str)==1)
                    {
                        HGemPtr->GetTimeInfo();
                        if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                    //Ifor 20251018 add:Secs Alarm List
                        {
                            S=sStartTime+str;
                            FMessageList->Insert(0, S);
                            S="";
                            if(FMessageList->Count > 20)
                            {
                                FMessageList->Delete(20);
                            }

                            for(int i=0; i<FMessageList->Count; i++)
                            {
                                S1=IntToStr(i+1)+ ". ";
                                S=S+S1+FMessageList->Strings[(FMessageList->Count-1)-i]+"\r\n";
                            }
                            SecsAlarmMessage->Add(S);                           //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
                        }
                        else
                        {
                            S=str;
                            SecsAlarmMessage->Add(S);                           //Steven 20150519 : 修正SECS GEM使用ShowMyMessage會出現記憶體破壞
                        }

                        S=HGemPtr->TimeString+AnsiString(" [R] <<==")+S;
                        if(HGemPtr->TerminalDisplayIndex==1)
                        {
                            if(uint1EC==0)
                                HGemPtr->TerminalMemoPtr->Clear();
                            HGemPtr->TerminalMemoPtr->Lines->Add(S);
                            iDisplay=0;
                        }
                        else if(HGemPtr->TerminalDisplayIndex==2)
                        {
                            if(uint1EC==0)
                                HGemPtr->TerminalListboxPtr->Clear();
                            HGemPtr->TerminalListboxPtr->Items->Add(S);
                            iDisplay=0;
                        }
                        else if(HGemPtr->TerminalDisplayIndex==3)
                        {
                            HGemPtr->TerminalEditPtr->Text=S;
                            iDisplay=0;
                        }
                        else if(HGemPtr->TerminalDisplayIndex==4)
                        {
                            HGemPtr->TerminalPanelPtr->Caption=S;
                            iDisplay=0;
                        }
                    }
                }
            }
        }
    }
    else
    {
        iDisplay=2;
    }

    if(ActiveWire->Remote.W_Bit==1)
        ActiveWire->LocalAcknowledge(10, 4, iDisplay);

    if(iDisplay==0 && HGemPtr->TerminalDisplayIndex2!=0)
    {
        if(HGemPtr->TerminalDisplayIndex2==1)
        {
            if(uint1EC==0)
                HGemPtr->TerminalMemoPtr2->Clear();
            HGemPtr->TerminalMemoPtr2->Lines->Add(S);
        }
        else if(HGemPtr->TerminalDisplayIndex2==2)
        {
            if(uint1EC==0)
                HGemPtr->TerminalListboxPtr2->Clear();
            HGemPtr->TerminalListboxPtr2->Items->Add(S);
        }
        else if(HGemPtr->TerminalDisplayIndex2==3)
        {
            HGemPtr->TerminalEditPtr2->Text=S;
        }
        else if(HGemPtr->TerminalDisplayIndex2==4)
        {
            HGemPtr->TerminalPanelPtr2->Caption=S;
        }
    }
}
//---------------------------------------------------------------------------
// [S10,F6] Terminal Display Multi-Block Acknowledge.
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro7) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2387-2431) -- only needs `TerminalDisplayIndex` (shared with S10F4 above,
// added this wave) -- does NOT touch TerminalMemoPtr/TerminalListboxPtr/
// TerminalEditPtr/TerminalPanelPtr or any "2"-suffixed member at all.
// MECHANICAL RENAME: golden mixes `HGemPtr->`/bare `HGem->` for the SAME wire
// primitives within this one function (`HGemPtr->GetDataItemLenAndTypeAndDelete/
// DataItemIn/Remote.W_Bit/LocalAcknowledge` vs the loop body's bare
// `HGem->GetDataItemLenAndType/DataItemIn`) -- both spellings normalized to
// `ActiveWire->...` per this project's established duality rule (see this
// file's own INTEGRATE WAVE 6 note / uHGemClass.cpp:3234). `HGemPtr->
// TerminalDisplayIndex` (real THGem state) stays `HGemPtr->`, unchanged.
// `SecsAlarmMessage`/`iSECSMessageCanCloseByOperator` same bare-access
// reasoning as S10F4 immediately above.
//---------------------------------------------------------------------------
void HTGem::S10F6_TerminalDisplayMultiBlockAcknowledge()
{
    int len;
    unsigned char Type;

    unsigned char  uint1EC=0;
    char str[1024];
    int iDisplay=1;
    int iDataLen;
    AnsiString S;

    if(HGemPtr->TerminalDisplayIndex!=0)
    {
        if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)==1)
        {
            if(Type==HType.LIST_TYPE && len==2)
            {
                if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, &uint1EC)==1)
                {
                    iSECSMessageCanCloseByOperator=uint1EC;                     //Ifor 20171024 : add KYEC SECS GEM Can Close By Operator 0:一般流程 1:必須清除Alarm 2:啟動工站檢查流程
                    ActiveWire->GetDataItemLenAndType(len, Type);               //取得資料長度與格式
                    if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)==1)
                    {
                        HGemPtr->GetTimeInfo();
                        S="";
                        for(int i=0; i<len; i++)                                //Ifor 20150708  顯示多筆資料
                        {
                            ActiveWire->GetDataItemLenAndType(iDataLen, Type);   //取得資料長度與格式
                            ActiveWire->DataItemIn(iDataLen, Type, str);         //取值
                            S=S+str+"\r\n";
                        }
                        SecsAlarmMessage->Add(S);
                        iDisplay=0;
                    }
                }
            }
        }
    }
    else
    {
        iDisplay=2;
    }

    if(ActiveWire->Remote.W_Bit==1)
        ActiveWire->LocalAcknowledge(10, 6, iDisplay);
}
//---------------------------------------------------------------------------
// [S100,F4] Report All Alarm.
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2437-2450) -- "no defect" method (§2 category ①): strGrdAlarm (HGemPtr->,
// unchanged) + wire (-> ActiveWire->) is the WHOLE dependency chain.
void HTGem::S100F4_ReportAllAlarm()
{
    bool type;
    ActiveWire->InitLocalHead(100, 4, 0);
    ActiveWire->DataItemOut(HGemPtr->strGrdAlarm->RowCount-1, HType.LIST_TYPE, NULL);
    for(int i=1; i<HGemPtr->strGrdAlarm->RowCount; i++)
    {
        ActiveWire->DataItemOut(3, HType.LIST_TYPE ,NULL);
        ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdAlarm->Cells[1][i]);
        ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->strGrdAlarm->Cells[4][i]);
        type=atoi(HGemPtr->strGrdAlarm->Cells[2][i].c_str())!=0;
        ActiveWire->DataItemOut(1, HType.BOOLEAN_TYPE, &type);
    }
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S101,F2] Current EPPD Data (variant 1).
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro5) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2456-2476): sole recorded blockers were THGem members SV_70_UNT1_ReceipeStruct
// (already real)/UploadFileString/SV_71_ASCII_FilenameExtened (the latter two
// added this wave, uHGemEquipment.h). `HGemPtr->SV_70_UNT1_ReceipeStruct/
// UploadFileString/SV_71_ASCII_FilenameExtened` (real THGem state) stay
// HGemPtr->; `HGemPtr->InitLocalHead/DataItemOut/SendLocalData` (wire
// primitives) -> ActiveWire->, same mechanical-rename rule as every prior
// integrate wave.
//---------------------------------------------------------------------------
void HTGem::S101F2_CurrentEPPDData()
{
    int count;
    if(HGemPtr->SV_70_UNT1_ReceipeStruct!=1)
    {
        ActiveWire->InitLocalHead(101, 2, 0);
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        ActiveWire->SendLocalData();
        return;
    }
    count=HGemPtr->UploadFileString->Count;
    ActiveWire->InitLocalHead(101, 2, 0);

    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->SV_71_ASCII_FilenameExtened);
    ActiveWire->DataItemOut(count, HType.LIST_TYPE, NULL);
    for(int i=0; i<count; i++)
    {
        ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->UploadFileString->Strings[i]);
    }
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// [S101,F4] Current EPPD Data (variant 2).
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro5) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2482-2502): same blockers/resolution as S101F2_CurrentEPPDData immediately
// above (SV_70_UNT1_ReceipeStruct/UploadFileString/SV_71_ASCII_FilenameExtened
// stay HGemPtr->; wire -> ActiveWire->), just gated on
// SV_70_UNT1_ReceipeStruct!=2 (variant 2) instead of !=1.
// GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM (same species as the
// already-documented S6F16/S6F18 InitLocalHead(6,16,0) copy-paste bug, see
// this file's own file-head note): BOTH the early-return path (golden :2487)
// and the success path (golden :2493) call InitLocalHead(101, 2, 0) -- the
// *S101F2* header, never (101, 4, 0). This function never emits its own
// correct S,F header anywhere in golden; it is an evident copy-paste artifact
// from S101F2_CurrentEPPDData immediately above it in golden. NOT corrected
// here, per this project's faithful-translation mandate.
//---------------------------------------------------------------------------
void HTGem::S101F4_CurrentEPPDData()
{
    int count;
    if(HGemPtr->SV_70_UNT1_ReceipeStruct!=2)
    {
        ActiveWire->InitLocalHead(101, 2, 0);   // GOLDEN BUG preserved verbatim: S101F2's header, never (101,4,0) -- see comment above
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        ActiveWire->SendLocalData();
        return;
    }
    count=HGemPtr->UploadFileString->Count;
    ActiveWire->InitLocalHead(101, 2, 0);   // GOLDEN BUG preserved verbatim: same as above, not (101,4,0)

    ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->SV_71_ASCII_FilenameExtened);
    ActiveWire->DataItemOut(count, HType.LIST_TYPE,NULL);
    for(int i=0; i<count; i++)
    {
        ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->UploadFileString->Strings[i]);
    }
    ActiveWire->SendLocalData();
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2505-2513) -- "no defect" method (§2 category ①): `bDisableBinaryShow`/
// `LocalAcknowledge` are SecsWireCodec members/methods (-> ActiveWire->,
// D1/D2 precedent, NOT THGem's own -- corrects this stub's own prior gate
// comment, which assumed no engine home existed for them); MoveCheckCallBack/
// bReceiveS101F5 are THGem's own members (HGemPtr->, unchanged, added by the
// SysModWire wave).
// AI(W906-uHGemClass-Micro7) 20260721: S101F6_StoreHostUploadFile itself is
// NOW UN-GATED too (see its own definition immediately below) -- the
// UpLoadPath/GemRemoteReceipeList/SV_70_UNT1_ReceipeStruct/
// SV_71_ASCII_FilenameExtened/bFinishDownloadFile blockers this comment used
// to cite are all resolved this wave.
// GOLDEN BUG PRESERVED VERBATIM -- DOUBLE ACKNOWLEDGE (flag for review): on
// S101F6_StoreHostUploadFile's own success path (its outer
// `DataItemIn(5,LIST_TYPE,NULL)==1` branch), THAT Sub itself already calls
// `ActiveWire->LocalAcknowledge(101, 6, 0);` internally (golden
// uHGemClass.cpp:2579 -- see that method's own matching comment). THIS
// wrapper's `ActiveWire->LocalAcknowledge(101, 6, 0);` call below then fires
// UNCONDITIONALLY right after, regardless -- so on that success path the host
// receives the SAME (101,6,0) accept TWICE for one S101F6 primary message.
// Confirmed by direct read of golden (uHGemClass.cpp:2778-2787 this wrapper +
// :2519-2581 the Sub) -- a genuine golden defect, not a translation
// artifact. NOT corrected here, per this project's faithful-translation
// mandate: both calls are preserved, neither suppressed.
void HTGem::S101F6()
{
    ActiveWire->bDisableBinaryShow=true;
    S101F6_StoreHostUploadFile();
    ActiveWire->bDisableBinaryShow=false;
    if(HGemPtr->MoveCheckCallBack!=NULL)
        HGemPtr->MoveCheckCallBack();
    HGemPtr->bReceiveS101F5=true;
    ActiveWire->LocalAcknowledge(101, 6, 0);   // GOLDEN BUG preserved verbatim: 2nd (101,6,0) ack on the success path -- see this wrapper's own comment above
}
//---------------------------------------------------------------------------
// [S101,F6] Store Host Upload File.
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro7) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2519-2581). Blockers resolved this wave: THGem member `UpLoadPath`
// (already real, added Micro6) + `bFinishDownloadFile` (added this wave) +
// `THGemListBox::Checked` write support (added this wave, uHGemEquipment.h --
// see that struct's own comment) on the already-real `GemRemoteReceipeList`.
// MECHANICAL RENAME: golden `HGemPtr->DataItemIn/GetDataItemLenAndType/
// LocalAcknowledge` (wire-codec primitives) -> `ActiveWire->...`; `HGemPtr->
// bFinishDownloadFile/UpLoadPath/GemRemoteReceipeList/SV_70_UNT1_ReceipeStruct/
// SV_71_ASCII_FilenameExtened` (real THGem state) stay `HGemPtr->`, unchanged.
// `ActiveWire->DownLoadFilePtr` is the wire codec's own raw-binary-payload
// pointer (SecsWireCodec.h, already real since an earlier wave), NOT a THGem
// member -- called `ActiveWire->`, matching its real engine home.
// GOLDEN QUIRK, FLAGGED NOT FIXED (wire-supplied path, zero sanitization):
// `str` below is a 200-byte ASCII field read directly off the wire
// (`DataItemIn(200,...)`) and concatenated verbatim onto
// `HGemPtr->UpLoadPath+"HGem\\"` to build the fopen() target path -- no
// length/charset/".."/absolute-path check of any kind, in golden or here.
// Same documented risk category as this file's own S7F18/DeleteDirectory
// precedent (see this file's head-note "STILL GATED"/"GOLDEN QUIRKS" history
// for that method) -- a malicious or malformed host could steer this fopen()
// outside `UpLoadPath` entirely. NOT sanitized here, per this project's
// faithful-translation mandate.
// GOLDEN BUG PRESERVED VERBATIM -- DOUBLE ACKNOWLEDGE (see this method's own
// LocalAcknowledge call below, and HTGem::S101F6()'s own matching comment
// just above this method): this Sub's success-path `LocalAcknowledge(101,6,0)`
// plus its caller's own unconditional second call means the host is ACK'd
// twice per successful S101F6. NOT corrected here.
//---------------------------------------------------------------------------
void HTGem::S101F6_StoreHostUploadFile()
{
    char str[256];
    int len, i, j;
    unsigned char Type;
    AnsiString S;
    FILE *P;
    int iStoreCT, iTotalCount, iFileCount;

    if(ActiveWire->DataItemIn(5, HType.LIST_TYPE, NULL)==1)
    {
        ActiveWire->DataItemIn(200, HType.ASCII_TYPE, str);                     //  file name
        ActiveWire->DataItemIn(1, HType.INT_4_TYPE, &iStoreCT);
        ActiveWire->DataItemIn(1, HType.INT_4_TYPE, &iTotalCount);
        ActiveWire->DataItemIn(1, HType.INT_4_TYPE, &iFileCount);

        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            //pig 2014.04.01 ASEM SECS start
            HGemPtr->bFinishDownloadFile=false;
            // GOLDEN QUIRK preserved verbatim, NOT sanitized -- see this
            // method's own file-head comment above ("wire-supplied path,
            // zero sanitization").
            S = HGemPtr->UpLoadPath+"HGem\\";
            S+=str;
            //pig 2014.04.01 ASEM SECS end
            if(iStoreCT==1)
                P=fopen(S.c_str(), "wb");
            else
                P=fopen(S.c_str(), "ab+");

            if(P!=NULL)
            {
                fwrite(ActiveWire->DownLoadFilePtr, len, 1, P);
                fclose(P);
            }
        }

        if(iStoreCT==iTotalCount)
        {
            for(i=0; i<HGemPtr->GemRemoteReceipeList->Items->Count; i++)
            {
                if(HGemPtr->SV_70_UNT1_ReceipeStruct==0)
                {
                    S=HGemPtr->GemRemoteReceipeList->Items->Strings[i];
                }
                else if(HGemPtr->SV_70_UNT1_ReceipeStruct==1)
                {
                    S=HGemPtr->SV_71_ASCII_FilenameExtened;
                    j=S.LastDelimiter(".");
                    S=S.SubString(j, S.Length());
                    S=HGemPtr->GemRemoteReceipeList->Items->Strings[i]+S;
                }

                if(S==str)
                    HGemPtr->GemRemoteReceipeList->Checked[i]=false;
            }

            if(iFileCount<=1)
            {
                HGemPtr->bFinishDownloadFile=true;
            }
        }
        ActiveWire->LocalAcknowledge(101, 6, 0);   // GOLDEN BUG preserved verbatim: 1st (101,6,0) ack -- HTGem::S101F6() (this Sub's caller) unconditionally sends a 2nd one right after this Sub returns, see both methods' own file-head comments
    }
}
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2583-2595) -- same shape/reasoning as S101F6 above.
// AI(W906-uHGemClass-Micro7) 20260721: S101F8_StoreHostUploadFile itself is
// NOW UN-GATED too (see its own definition immediately below).
// NOTE ASYMMETRY vs S101F6_StoreHostUploadFile (flag for review, do NOT
// "fix" one to match the other): UNLIKE that Sub, S101F8_StoreHostUploadFile
// itself never calls LocalAcknowledge anywhere in golden (confirmed by direct
// read, uHGemClass.cpp:2597-2639) -- only THIS wrapper's own
// `ActiveWire->LocalAcknowledge(101, 8, 0);` call below fires, exactly once.
// So S101F8's host reply is single, never double like S101F6's -- a genuine
// golden difference between the two otherwise near-identical Subs, not a
// translation oversight on either side.
void HTGem::S101F8()
{
    ActiveWire->bDisableBinaryShow=true;
    S101F8_StoreHostUploadFile();
    ActiveWire->bDisableBinaryShow=false;
    if(HGemPtr->MoveCheckCallBack!=NULL)
        HGemPtr->MoveCheckCallBack();
    HGemPtr->bReceiveS101F7=true;
    ActiveWire->LocalAcknowledge(101, 8, 0);   // sole ack for S101F8 -- see this wrapper's own comment above (NOTE ASYMMETRY)
}
//---------------------------------------------------------------------------
// [S101,F8] Store Host Upload File (variant).
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro7) 20260721: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2597-2639). Blockers resolved this wave: THGem member `CurrentDirectory`
// (added this wave, READ-ONLY here -- golden's own `SetCurrentDirectory`
// setter/cascade is out of scope, see that member's own header comment) +
// `THGemListBox::Checked` write support (added this wave, shared with
// S101F6_StoreHostUploadFile above). `IncludeTrailingPathDelimiter` reuses
// the existing TU-local static helper already added for S6F24 (see this
// file's own INTEGRATE WAVE 7 note above / that helper's own definition).
// `MyForceDirectories` is the already-real `common.cpp` global (common.h
// already #include'd) -- called bare, exactly as golden does (a free
// function, not a THGem member). `__FUNC__` (BCB6 builtin) -> standard
// `__func__`, same substitution this project's own canary_support.h/
// aArmHeader.h shims already establish elsewhere (AnsiString's `const char*`
// ctor accepts it directly, no wrapper needed).
// MECHANICAL RENAME: golden `HGemPtr->DataItemIn/GetDataItemLenAndType`
// (wire-codec primitives) -> `ActiveWire->...`; `HGemPtr->CurrentDirectory/
// GemRemoteReceipeList` (real THGem state) stay `HGemPtr->`, unchanged;
// `ActiveWire->DownLoadFilePtr` (wire codec's own raw-binary pointer) ->
// `ActiveWire->`, same reasoning as S101F6_StoreHostUploadFile above.
// GOLDEN QUIRK, FLAGGED NOT FIXED (wire-supplied path, zero sanitization):
// same risk category as S101F6_StoreHostUploadFile's own comment above --
// `PathName`/`str` are BOTH 200-byte wire-supplied ASCII fields, fed straight
// into `MyForceDirectories` and the final `fopen` path with no sanitization
// of any kind, in golden or here. NOT fixed here.
// NOTE ASYMMETRY (see HTGem::S101F8()'s own comment just above this method):
// this Sub never calls LocalAcknowledge itself -- only its caller does, once.
//---------------------------------------------------------------------------
void HTGem::S101F8_StoreHostUploadFile()
{
    char str[2560], PathName[2560];
    int len;
    unsigned char Type;
    char *Ptr;
    AnsiString S;
    FILE *P;
    int iStoreCT, iTotalCount;

    if(ActiveWire->DataItemIn(5, HType.LIST_TYPE, NULL)==1)
    {
        ActiveWire->DataItemIn(200, HType.ASCII_TYPE, PathName);                //  path name
        ActiveWire->DataItemIn(200, HType.ASCII_TYPE, str);                     //  file name
        ActiveWire->DataItemIn(1, HType.INT_4_TYPE, &iStoreCT);
        ActiveWire->DataItemIn(1, HType.INT_4_TYPE, &iTotalCount);
        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            Ptr=ActiveWire->DownLoadFilePtr;
            // GOLDEN QUIRK preserved verbatim, NOT sanitized -- see this
            // method's own file-head comment above ("wire-supplied path,
            // zero sanitization").
            MyForceDirectories(IncludeTrailingPathDelimiter(HGemPtr->CurrentDirectory)+AnsiString(PathName), __func__);
            S=IncludeTrailingPathDelimiter(HGemPtr->CurrentDirectory)+AnsiString(PathName)+AnsiString("\\")+AnsiString(str);
            if(iStoreCT==1)
                P=fopen(S.c_str(), "wb");
            else
                P=fopen(S.c_str(), "ab+");
            if(P!=NULL)
            {
                fwrite(Ptr,len,1,P);
                fclose(P);
            }
        }

        if(iStoreCT==iTotalCount)
        {
            for(int i=0; i<HGemPtr->GemRemoteReceipeList->Items->Count; i++)
            {
                S=HGemPtr->GemRemoteReceipeList->Items->Strings[i];
                if(S==PathName)
                    HGemPtr->GemRemoteReceipeList->Checked[i]=false;
            }
        }
    }
}
//---------------------------------------------------------------------------
// [S125,F2] Enable/Disable EC Data Acknowledge.                  //wei 20150630
//---------------------------------------------------------------------------
// AI(W906-uHGemClass-Micro5) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 2643-2680): sole recorded blockers were GetDataItemLenAndTypeAndDelete
// (already real, ActiveWire->) + THGem::EnableDisableECDataAll/
// EnableDisableECData (added this wave, uHGemEquipment.h/.cpp). GOLDEN QUIRK
// preserved verbatim: golden itself mixes `HGemPtr->` and the bare global
// `HGem->` for wire-primitive calls within this ONE function (:2653/:2664 use
// `HGem->`, everything else uses `HGemPtr->`) -- in real golden deployment
// both point at the same live THGem instance, so this is cosmetic, not a
// behavioral fork. Per this project's established mechanical-rename rule,
// EVERY wire-primitive call (DataItemIn/GetDataItemLenAndType/
// GetDataItemLenAndTypeAndDelete/LocalAcknowledge) -> ActiveWire->,
// regardless of which of the two golden spellings it used;
// `HGemPtr->EnableDisableECDataAll/EnableDisableECData` (real THGem methods)
// stay HGemPtr->.
//---------------------------------------------------------------------------
void HTGem::S125F2_EnableDisableECDataAcknowledge()                             //wei 20150630
{
    unsigned char ALED, Type;
    int len, SVLen;
    AnsiString ID;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE ,NULL)==1)
    {
        if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, &ALED))
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVLen,Type)==1)
            {
                if(SVLen==0)
                {
                    HGemPtr->EnableDisableECDataAll(ALED);
                    ActiveWire->LocalAcknowledge(125, 2, 0);
                }
                else
                {
                    for(int i=0; i<SVLen; i++)
                    {
                        ActiveWire->GetDataItemLenAndType(len, Type);           // gloss: "peek this item's length and format"
                        ActiveWire->DataItemIn(len, Type, ID);
                        if(HGemPtr->EnableDisableECData(ID, ALED))
                            ActiveWire->LocalAcknowledge(125, 2, 0);
                        else
                            ActiveWire->LocalAcknowledge(125, 2, 1);
                    }
                }
                return;
            }
        }
    }
    S9F7_IllegalData("S125,F1 Data Format error !!!");
}
//---------------------------------------------------------------------------
// SetECValue -- writes an EC value out to *PtrSour by ECID's registered type.
// AI(W906-VCW1) 20260721 UN-GATED (golden SECSGEM/uHGemClass.cpp:2682-2882):
// vclcompat/Controls.h now supplies the 6 VCL-widget stand-ins
// (TPanel/TCustomEdit/TComboBox/TLabel/TCheckBox/TRadioGroup) golden's
// `IsVCL==1` branch dynamic_casts against; TStringList (the 7th candidate)
// already existed and gained the `: public TObject` base this same wave's R1
// step added (vclcompat/TStringList.h) so the dynamic_cast is well-formed.
// `HGem->EC_ID/EC_TYPE/EC_Ptr/EC_VCL_NAME` (golden's global THGem* access)
// -> `SvEcReg.EC_ID/EC_TYPE/EC_Ptr/EC_VCL_NAME` (HTGem's own embedded
// registration engine, same substitution CheckECValue/S2F15_Update already
// established in this file); `->Strings[i].c_str()` -> `->GetString(i)
// .c_str()` (vclcompat StringsProxy has no .c_str(), same accommodation);
// `EC_Ptr->Items[i]` -> `EC_Ptr->GetItem(i)` (vclcompat::TList function
// form, matching CheckECValue's own idiom in this same file). Nothing else
// changed vs. golden -- every dynamic_cast target, the first-match-wins
// if/else-if order, and the raw-pointer (IsVCL==0) scalar switch tail are
// byte-for-byte the same shape as golden.
//
// SCOPE BOUNDARY (see vclcompat/Controls.h's own file-head note for the full
// writeup): this does NOT make any real widget-backed EC functional. Zero
// real TPanel/TCustomEdit/TComboBox/TLabel/TCheckBox/TRadioGroup instances
// exist anywhere in the object graph today (uHGemHT9045_EC.cpp/_SV.cpp, the
// ~1740 real registration call sites, remain almost entirely untranslated) --
// every dynamic_cast below against those 6 correctly and safely returns
// nullptr for every EC registered today, exactly mirroring the precedent
// SecsSvEcRegistration.cpp's own GetECDataValue gate already established.
// Only the TStringList branch has any real reachable instance possible
// right now (SetECDataPointer's TObject* overload can register one).
//
// PRESERVED GOLDEN QUIRK -- IsVCL==2 bare `return;` (golden :2718-2721):
// unlike the IsVCL==1 branch above it, the IsVCL==2 (AnsiString*-backed) sub-
// branch NEVER writes back through the stored pointer at all -- it just
// returns immediately, dropping the incoming value on the floor. Meanwhile
// GetECDataValue/DataItemOutSV/DataItemOutEC (this same wave, below/
// SecsSvEcRegistration.cpp) all correctly READ IsVCL==2 ECs
// (`SS=(AnsiString*)EC_Ptr->GetItem(i); VCLStr=*SS;`). This is a real,
// previously-undocumented golden product-level asymmetry: an EC registered
// via the AnsiString* overload is host-READABLE but silently NON-WRITABLE
// via S2F15/SetECValue. Preserved verbatim -- NOT "fixed" into an actual
// write-through, per this project's faithful-translation mandate.
//---------------------------------------------------------------------------
void HTGem::SetECValue(unsigned ECID, void *PtrSour)
{
    unsigned char Type;
    AnsiString ECName;
    AnsiString ECUnit;
    void *P;
    TObject *VclP;
    AnsiString S, IsVCL;

    TPanel      *PanelPtr;
    TCustomEdit *EditPtr;
    TComboBox   *ComboBoxPtr;
    TLabel      *LabelPtr;
    TCheckBox   *CheckBoxPtr;
    TRadioGroup *RadioGroupPtr;
    TStringList *StringListPtr;

    int i=SvEcReg.EC_ID->IndexOf(ECID);
    if(i>=0)
    {
        Type    =(unsigned char)atoi(SvEcReg.EC_TYPE->GetString(i).c_str());
        P       =SvEcReg.EC_Ptr->GetItem(i);
        IsVCL   =SvEcReg.EC_VCL_NAME->GetString(i);
        if(IsVCL==1 || IsVCL==2)
        {
            if(IsVCL==1)
            {
                VclP=(TObject *)SvEcReg.EC_Ptr->GetItem(i);
                PanelPtr        =dynamic_cast<TPanel        *>(VclP);
                EditPtr         =dynamic_cast<TCustomEdit   *>(VclP);
                ComboBoxPtr     =dynamic_cast<TComboBox     *>(VclP);
                LabelPtr        =dynamic_cast<TLabel        *>(VclP);
                CheckBoxPtr     =dynamic_cast<TCheckBox     *>(VclP);
                RadioGroupPtr   =dynamic_cast<TRadioGroup   *>(VclP);
                StringListPtr   =dynamic_cast<TStringList   *>(VclP);
            }
            else
            {
                // AI(W906-VCW1) 20260721: preserved golden quirk -- see this
                // function's own file-head note above. Genuinely a bare
                // return, not a missing write-through.
                return;
            }

            char *Ptr2;
            Ptr2=(char *)PtrSour;
            if(Type==HType.ASCII_TYPE)
            {
                S=Ptr2;
            }
            else
            {
                S=*Ptr2;
            }

            if(StringListPtr!=NULL)
            {
                StringListPtr->CommaText=S;
            }
            else if(PanelPtr!=NULL)
            {
                PanelPtr->Caption=S;
            }
            else if(EditPtr!=NULL)
            {
                EditPtr->Text=S;
            }
            else if(ComboBoxPtr!=NULL)
            {
                if(Type==HType.ASCII_TYPE)
                    ComboBoxPtr->Text=S;
                else
                    ComboBoxPtr->ItemIndex=atoi(S.c_str());
            }
            else if(LabelPtr!=NULL)
            {
                LabelPtr->Caption=S;
            }
            else if(CheckBoxPtr!=NULL)
            {
                CheckBoxPtr->Checked=(S!="0");
            }
            else if(RadioGroupPtr!=NULL)
            {
                if(Type==HType.ASCII_TYPE)
                {
                    if(S!="")
                    {
                        for(int x=0; x<RadioGroupPtr->Items->Count; x++)
                        {
                            if(S==RadioGroupPtr->Items->Strings[x])
                                RadioGroupPtr->ItemIndex=x;
                        }
                    }
                    else
                    {
                        RadioGroupPtr->ItemIndex=-1;
                    }
                }
                else
                {
                    RadioGroupPtr->ItemIndex=atoi(S.c_str());
                }
            }
        }
        else
        {
            if(Type==HType.ASCII_TYPE)
            {
                char *Ptr, *Ptr2;
                Ptr=(char *)P;
                Ptr2=(char *)PtrSour;
                strcpy(Ptr, Ptr2);
            }
            else if(Type==HType.BINARY_TYPE)
            {
                char *Ptr, *Ptr2;
                Ptr=(char *)P;
                Ptr2=(char *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.BOOLEAN_TYPE)
            {
                unsigned char *Ptr, *Ptr2;
                Ptr=(unsigned char *)P;
                Ptr2=(unsigned char *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.INT_1_TYPE)
            {
                char *Ptr,*Ptr2;
                Ptr=(char *)P;
                Ptr2=(char *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.INT_2_TYPE)
            {
                short *Ptr,*Ptr2;
                Ptr=(short *)P;
                Ptr2=(short *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.INT_4_TYPE)
            {
                int *Ptr, *Ptr2;
                Ptr=(int *)P;
                Ptr2=(int *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.INT_8_TYPE)
            {
                long long *Ptr, *Ptr2;                 // golden __int64
                Ptr=(long long *)P;
                Ptr2=(long long *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.UINT_1_TYPE)
            {
                unsigned char *Ptr,*Ptr2;
                Ptr=(unsigned char *)P;
                Ptr2=(unsigned char *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.UINT_2_TYPE)
            {
                unsigned short *Ptr,*Ptr2;
                Ptr=(unsigned short *)P;
                Ptr2=(unsigned short *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.UINT_4_TYPE)
            {
                unsigned int  *Ptr,*Ptr2;
                Ptr=(unsigned int  *)P;
                Ptr2=(unsigned int  *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.UINT_8_TYPE)
            {
                unsigned long long  *Ptr,*Ptr2;         // golden unsigned __int64
                Ptr=(unsigned long long  *)P;
                Ptr2=(unsigned long long  *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.FT_4_TYPE)
            {
                float  *Ptr,*Ptr2;
                Ptr=(float  *)P;
                Ptr2=(float  *)PtrSour;
                *Ptr=*Ptr2;
            }
            else if(Type==HType.FT_8_TYPE)
            {
                double  *Ptr,*Ptr2;
                Ptr=(double  *)P;
                Ptr2=(double  *)PtrSour;
                *Ptr=*Ptr2;
            }
        }
        return;
    }
}
//---------------------------------------------------------------------------
// [S2,F15] Update New Equipment Constant.                //wei 20170417 (Steven)
// AI(W906-uHGemClass-Unlock2) 20260716 UN-GATED (golden SECSGEM/uHGemClass.cpp:2884-3024):
// both recorded blockers are now real WireCodec methods
// (GetDataItemLenAndTypeAndDelete / SendInvalidDataMessageToHost, SecsWireCodec
// "WAVE 3" addendum); every other call in this body was already
// WireCodec-only (GetDataItemLenAndType/DataItemIn) or a same-class virtual
// call (SetECValue -- still gated above at the time of THIS wave, see its
// own comment; ReloadParameter -- already an inline no-op in the header).
// `HGem->` -> `WireCodec.`, nothing else changed vs. golden.
// AI(W906-VCW1) 20260721: SetECValue (above) is UN-GATED as of this later
// wave -- this note's "still gated" is left as its own historical record
// (matching this file's established convention, see the file-head
// "INTEGRATE WAVE 3" note on why historical before/after narrative is not
// rewritten); S2F15_UpdateNewEquipmentConstant's calls into it now reach a
// real implementation instead of a no-op.
// PRESERVED GOLDEN QUIRK (confirmed by direct read, not a translation bug):
// the ASCII branch's `Str=new char[len+100]` is never `delete[]`d anywhere in
// golden (uHGemClass.cpp's ASCII branch, ~golden :2955-2960) -- a genuine
// golden per-call memory leak on every ASCII-typed EC update, preserved
// verbatim (not "fixed" into a delete[]).
//---------------------------------------------------------------------------
int HTGem::S2F15_UpdateNewEquipmentConstant()
{
    int EClen, i, len;
    unsigned char Type;

    __int64  int8EC;                                                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    int   int4EC;
    short int2EC;
    char  int1EC;
    unsigned __int64 uint8EC;                                                   //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    unsigned int   uint4EC, ECID;
    unsigned short uint2EC;
    unsigned char  uint1EC;
    float          floatEC;
    double         doubleEC;
    AnsiString S;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type)==1)
    {
        if(Type==HType.LIST_TYPE)
        {
            for(i=0; i<EClen; i++)
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)==1)
                {
                    if(Type!=HType.LIST_TYPE || len!=2)
                        break;
                    // get ECID

                    ActiveWire->GetDataItemLenAndType(len, Type);
                    ActiveWire->DataItemIn(len, Type, S);
                    ECID=(unsigned)atoi(S.c_str());

                    ActiveWire->GetDataItemLenAndType(len, Type);

                    if(Type==HType.UINT_1_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint1EC)==1)
                            SetECValue(ECID, &uint1EC);
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint2EC)==1)
                            SetECValue(ECID, &uint2EC);
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint4EC)==1)
                            SetECValue(ECID, &uint4EC);
                    }
                    else if(Type==HType.UINT_8_TYPE)                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint8EC)==1)
                            SetECValue(ECID, &uint8EC);
                    }
                    else if(Type==HType.INT_1_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int1EC)==1)
                            SetECValue(ECID, &int1EC);
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int2EC)==1)
                            SetECValue(ECID, &int2EC);
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int4EC)==1)
                            SetECValue(ECID, &int4EC);
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int8EC)==1)
                            SetECValue(ECID, &int8EC);
                    }
                    else if(Type==HType.ASCII_TYPE)
                    {
                        char *Str;
                        Str=new char [(size_t)len+100];
                        if(ActiveWire->DataItemIn(len, Type, Str)==1)
                            SetECValue(ECID, Str);
                        // NOTE: golden never delete[]s Str here -- see this
                        // function's own "PRESERVED GOLDEN QUIRK" comment above.
                    }
                    else if(Type==HType.BINARY_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                            SetECValue(ECID, &int1EC);
                    }
                    else if(Type==HType.BOOLEAN_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                        {
                            SetECValue(ECID, &int1EC);
                        }
                    }
                    else if(Type==HType.FT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &floatEC)==1)
                            SetECValue(ECID, &floatEC);
                    }
                    else if(Type==HType.FT_8_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &doubleEC)==1)
                            SetECValue(ECID, &doubleEC);
                    }
                    else                                                        // error format (SECS-II ASCII code ir correct ,but ITRI is failure,need confirm with ITRI
                    {
                        ActiveWire->SendInvalidDataMessageToHost("error format");
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
    ReloadParameter();
    return 0;                                                                   //sucess  //JerryYang 20250120 : modify
}
//---------------------------------------------------------------------------
// [S2,F15] Check New Equipment Constant.                 //wei 20170417 (Steven)
// AI(W906-uHGemClass-Unlock2) 20260716 UN-GATED (golden SECSGEM/uHGemClass.cpp:3026-3190):
// same rationale as S2F15_UpdateNewEquipmentConstant immediately above --
// both recorded blockers are now real WireCodec methods; the remaining calls
// are WireCodec-only or a same-class virtual call (CheckECValue -- already
// un-gated in the prior wave). `HGem->` -> `WireCodec.`, nothing else
// changed vs. golden.
//---------------------------------------------------------------------------
int HTGem::S2F15_CheckNewEquipmentConstant()                                    //wei 20170417 (Steven) add S2F15
{
    int EClen, i, len, ret;
    unsigned char Type;
    __int64  int8EC;                                                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    int   int4EC;
    short int2EC;
    char  int1EC;
    unsigned __int64  uint8EC;                                                  //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    unsigned int   uint4EC;
    AnsiString ECID;
    unsigned short uint2EC;
    unsigned char  uint1EC;
    float          floatEC;
    double         doubleEC;

    /*
        L,n
        1. L,2
        1. <ECID1>
        2. <ECV1>
        2. L,2
        .
        .
        n. L,2
        1. <ECIDn>
        2. <ECVn>
    */

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type)!=1)
        return -1;
    if(Type!=HType.LIST_TYPE)
        return -1;
    if(EClen<1)
        return -1;
    for(i=0; i<EClen; i++)
    {
        if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)!=1)
            return -1;
        if(Type!=HType.LIST_TYPE || len!=2)
            return -1;
        ret=1;
        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            if(ActiveWire->DataItemIn(len, Type, ECID)!=1)
                return -1;
        }
        else
        {
            return -1;
        }

        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            if(Type==HType.UINT_1_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint1EC)==1)
                    ret=CheckECValue(ECID, &uint1EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_2_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint2EC)==1)
                    ret=CheckECValue(ECID, &uint2EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint4EC)==1)
                    ret=CheckECValue(ECID, &uint4EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_8_TYPE)                                    //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                if(ActiveWire->DataItemIn(1, Type, &uint8EC)==1)
                    ret=CheckECValue(ECID, &uint8EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_1_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_2_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int2EC)==1)
                    ret=CheckECValue(ECID, &int2EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int4EC)==1)
                    ret=CheckECValue(ECID, &int4EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_8_TYPE)                                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                if(ActiveWire->DataItemIn(1, Type, &int8EC)==1)
                    ret=CheckECValue(ECID, &int8EC);
                else
                    return -1;
            }
            else if(Type==HType.ASCII_TYPE)
            {
                char *Str;
                Str=new char [(size_t)len+100];
                if(ActiveWire->DataItemIn(len, Type, Str)==1)
                    ret=CheckECValue(ECID, Str);
                else
                    return -1;
            }
            else if(Type==HType.BINARY_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.BOOLEAN_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.FT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &floatEC)==1)
                    ret=CheckECValue(ECID, &floatEC);
                else
                    return -1;
            }
            else if(Type==HType.FT_8_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &doubleEC)==1)
                    ret=CheckECValue(ECID, &doubleEC);
                else
                    return -1;
            }
            else                                                                // error format (SECS-II ASCII code ir correct ,but ITRI is failure,need confirm with ITRI
            {
                ActiveWire->SendInvalidDataMessageToHost("error format");
                return -1;
            }

            if(ret!=0)
                return ret;
        }
        else
        {
            ActiveWire->SendInvalidDataMessageToHost("GetDataItemLenAndType Error");
            return -1;
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
// CheckECValue -- validates an ECID/value pair against its registered
// Min/Max bounds, returning 0 (in range / no bound to check) or 3 (out of
// range); golden's own not-found path defaults to `return 1;` (golden
// :3558-3559), matched here.
// AI(W906-uHGemClass-Unlock) 20260713 UN-GATED (golden SECSGEM/uHGemClass.cpp:3192-3560):
// the ONLY one of the SetECValue/S2F15_Update/S2F15_Check/CheckECValue "EC
// value" cluster with zero VCL-widget dynamic_cast and zero
// GetDataItemLenAndTypeAndDelete call -- entirely EC_ID/EC_TYPE/EC_Ptr_Max/
// EC_Ptr_Min/EC_Ptr_Max_Value/EC_Ptr_Min_Value (now SvEcReg) + StringOut
// (now WireCodec.StringOut) on the not-found path. Golden itself accesses
// these through the GLOBAL `HGem` (not the member `HGemPtr`) -- both are
// THGem*, out of scope either way; this translation collapses both onto the
// one HTGem instance's own SvEcReg/WireCodec (see uHGemClass.h's own design
// note), a deliberate, harmless simplification since golden only ever has
// one live THGem singleton in practice too.
//---------------------------------------------------------------------------
int HTGem::CheckECValue(AnsiString ECID, void *PtrSour)
{
    int i;
    unsigned char Type;
    void *PMax, *PMin;
    AnsiString PMax_Value, PMin_Value;

    i=SvEcReg.EC_ID->IndexOf(ECID);
    if(i>=0)
    {
        Type=(unsigned char)atoi(SvEcReg.EC_TYPE->GetString(i).c_str());
        PMax=SvEcReg.EC_Ptr_Max->GetItem(i);
        PMin=SvEcReg.EC_Ptr_Min->GetItem(i);

        PMax_Value=SvEcReg.EC_Ptr_Max_Value->GetString(i);
        PMin_Value=SvEcReg.EC_Ptr_Min_Value->GetString(i);

        if(Type==HType.ASCII_TYPE)
        {
            return 0;
        }
        else if(Type==HType.BINARY_TYPE)
        {
            return 0;
        }
        else if(Type==HType.BOOLEAN_TYPE)
        {
            return 0;
        }
        else if(Type==HType.INT_1_TYPE)
        {
            char *Ptr2, *PtrMax, *PtrMin;
            char PtrMax_Value, PtrMin_Value;

            PtrMax=(char *)PMax;
            PtrMin=(char *)PMin;

            PtrMax_Value=(char)atoi(PMax_Value.c_str());
            PtrMin_Value=(char)atoi(PMin_Value.c_str());                        //2013/09/17 lee

            Ptr2=(char *)PtrSour;

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.INT_2_TYPE)
        {
            short *Ptr2, *PtrMax, *PtrMin;
            short PtrMax_Value, PtrMin_Value;

            PtrMax=(short *)PMax;
            PtrMin=(short *)PMin;
            PtrMax_Value=(short)atoi(PMax_Value.c_str());
            PtrMin_Value=(short)atoi(PMin_Value.c_str());                       //2013/09/17 lee
            Ptr2=(short *)PtrSour;

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return  3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.INT_4_TYPE)
        {
            int *Ptr2, *PtrMax, *PtrMin;
            int PtrMax_Value, PtrMin_Value;
            PtrMax=(int *)PMax;
            PtrMin=(int *)PMin;

            PtrMax_Value=atoi(PMax_Value.c_str());
            PtrMin_Value=atoi(PMin_Value.c_str());

            Ptr2=(int *)PtrSour;

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.INT_8_TYPE)                                         //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
        {
            __int64 *Ptr2, *PtrMax, *PtrMin;
            __int64 PtrMax_Value, PtrMin_Value;
            PtrMax=(__int64 *)PMax;
            PtrMin=(__int64 *)PMin;

            PtrMax_Value=strtoll(PMax_Value.c_str(), NULL, 10);
            PtrMin_Value=strtoll(PMin_Value.c_str(), NULL, 10);

            Ptr2=(__int64 *)PtrSour;

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.UINT_1_TYPE)
        {
            unsigned char *Ptr2,*PtrMax,*PtrMin;
            unsigned char PtrMax_Value,PtrMin_Value;
            PtrMax=(unsigned char *)PMax;
            PtrMin=(unsigned char *)PMin;
            Ptr2=(unsigned char *)PtrSour;
            PtrMax_Value=(unsigned char)atoi(PMax_Value.c_str());
            PtrMin_Value=(unsigned char)atoi(PMin_Value.c_str());               //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>=(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<=(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.UINT_2_TYPE)
        {
            unsigned short *Ptr2,*PtrMax,*PtrMin;
            unsigned short PtrMax_Value,PtrMin_Value;

            PtrMax=(unsigned short *)PMax;
            PtrMin=(unsigned short *)PMin;
            Ptr2=(unsigned short *)PtrSour;
            PtrMax_Value=(unsigned short)atoi(PMax_Value.c_str());
            PtrMin_Value=(unsigned short)atoi(PMin_Value.c_str());              //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>=(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<=(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.UINT_4_TYPE)
        {
            unsigned int  *Ptr2,*PtrMax,*PtrMin;
            unsigned int PtrMax_Value,PtrMin_Value;
            PtrMax=(unsigned int *)PMax;
            PtrMin=(unsigned int *)PMin;
            Ptr2=(unsigned int *)PtrSour;
            PtrMax_Value=atoi(PMax_Value.c_str());
            PtrMin_Value=atoi(PMin_Value.c_str());                              //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.UINT_8_TYPE)                                        //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
        {
            unsigned __int64 *Ptr2,*PtrMax,*PtrMin;
            unsigned __int64 PtrMax_Value,PtrMin_Value;
            PtrMax=(unsigned __int64 *)PMax;
            PtrMin=(unsigned __int64 *)PMin;
            Ptr2=(unsigned __int64 *)PtrSour;
            PtrMax_Value=(unsigned)strtoll(PMax_Value.c_str(), NULL, 10);
            PtrMin_Value=(unsigned)strtoll(PMin_Value.c_str(), NULL, 10);

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.FT_4_TYPE)
        {
            float *Ptr2, *PtrMax, *PtrMin;
            float PtrMax_Value,PtrMin_Value;

            PtrMax=(float *)PMax;
            PtrMin=(float *)PMin;
            Ptr2=(float *)PtrSour;

            PtrMax_Value=(float)atoi(PMax_Value.c_str());
            PtrMin_Value=(float)atoi(PMin_Value.c_str());                       //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        // AI(W906-uHGemClass-Unlock) 20260713 PRESERVED GOLDEN BUG (confirmed by direct
        // cp950-aware read of golden SECSGEM/uHGemClass.cpp -- NOT a
        // translation error): golden has TWO consecutive `else if(Type==
        // HType.FT_4_TYPE)` branches back to back -- this second one uses
        // `double` (clearly meant to be the FT_8_TYPE branch) but tests the
        // SAME `FT_4_TYPE` condition as the branch immediately above, so it
        // is provably unreachable dead code in an if/else-if chain (the
        // first FT_4_TYPE branch above already claims every FT_4_TYPE value).
        // The REAL FT_8_TYPE handling is the NEXT branch below, correctly
        // conditioned on `HType.FT_8_TYPE`. Preserved verbatim, not "fixed"
        // by renaming this branch's condition or deleting it.
        else if(Type==HType.FT_4_TYPE)
        {
            double  *Ptr2,*PtrMax,*PtrMin;
            double PtrMax_Value,PtrMin_Value;
            PtrMax=(double *)PMax;
            PtrMin=(double *)PMin;
            Ptr2=(double *)PtrSour;

            PtrMax_Value=atoi(PMax_Value.c_str());
            PtrMin_Value=atoi(PMin_Value.c_str());                              //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        else if(Type==HType.FT_8_TYPE)                                          //Steven 20130730 : FT_4 --> FT_8
        {
            double  *Ptr2,*PtrMax,*PtrMin;
            double PtrMax_Value,PtrMin_Value;
            PtrMax=(double *)PMax;
            PtrMin=(double *)PMin;
            Ptr2=(double *)PtrSour;

            PtrMax_Value=atoi(PMax_Value.c_str());
            PtrMin_Value=atoi(PMin_Value.c_str());                              //2013/09/17 lee

            if(PtrMax==NULL || PtrMin==NULL)
            {
                if((*Ptr2)>PtrMax_Value)
                    return 3;
                if((*Ptr2)<PtrMin_Value)
                    return 3;
                return 0;
            }
            else
            {
                if(PtrMax!=NULL)
                    if((*Ptr2)>(*PtrMax))
                        return 3;
                if(PtrMin!=NULL)
                    if((*Ptr2)<(*PtrMin))
                        return 3;
                return 0;
            }
        }
        return 4;
    }
    ActiveWire->StringOut("ECID:"+ECID+" not exist !!");
    return 1;
}
//---------------------------------------------------------------------------
// [S103,F12] Status Variable Namelist Reply (with value).        //Steven 20140911
//---------------------------------------------------------------------------
// AI(W906-SvEcDataItem) 20260720: UN-GATED (golden SECSGEM/uHGemClass.cpp:
// 3562-3625). SV_ID -> HGemPtr->SvEcReg.SV_ID; DataItemOutSVNameListWithValue
// -> HGemPtr-> (THGem's own method); wire -> ActiveWire->.
void HTGem::S103F12_StatusVariableNamelistReply()                               //Steven 20140911 : Add S103F11
{
    int SVlen, i, len;
    unsigned char Type;

    AnsiString S;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,Type)==1)
    {
        if(Type==HType.LIST_TYPE || SVlen==0)
        {
            ActiveWire->InitLocalHead(103,12,0);
            if(SVlen==0)
            {
                len=HGemPtr->SvEcReg.SV_ID->Count;
                ActiveWire->DataItemOut(len, HType.LIST_TYPE, NULL);
                for(i=0; i<len; i++)
                    HGemPtr->DataItemOutSVNameListWithValue(HGemPtr->SvEcReg.SV_ID->GetString(i));
            }
            else
            {
                ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
                for(i=0; i<SVlen; i++)
                {
                    if(ActiveWire->GetDataItemLenAndType(len,Type)==1)
                    {
                        if(ActiveWire->DataItemIn(len, Type, S)==1)
                        {
                            if(HGemPtr->DataItemOutSVNameListWithValue(S)==false)
                                return;
                        }
                        else
                        {
                            S9F7_IllegalData("S103,F11 data format error");
                            return;
                        }
                    }
                    else
                    {
                        S9F7_IllegalData("S103,F11 data format error");
                        return;
                    }
                }
            }
            ActiveWire->SendLocalData();
        }
        else
        {
            S9F7_IllegalData("S103,F11 data format error");
        }
    }
    else
    {
        S9F7_IllegalData("S103,F11 data format error");
    }
}
