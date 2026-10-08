//---------------------------------------------------------------------------
//  SECSGEM/uHGemEquipment.h -- THGem, first real slice
//
//  Translation wave: W906-uHGemEquipment (Front B of the "triple-front"
//  THGem wave; see this wave's hand-off prompt / synthesis note for the
//  full multi-front split)
//  Translator: AI(W906-uHGemEquipment) 20260716
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.h
//                 (730 lines) + uHGemEquipment.cpp (9353 lines)
//
//  WHAT THIS FILE IS (READ BEFORE EXTENDING)
//  ------------------------------------------
//  `THGem` (golden uHGemEquipment.h:90-724) is a ~6200-line VCL TForm and the
//  real SECS/GEM protocol engine: TClientSocket/TServerSocket connection
//  lifecycle, the SML wire codec (InitLocalHead/DataItemOut/SendLocalData/...
//  -- ALREADY translated as the standalone SecsWireCodec class,
//  SECSGEM/SecsWireCodec.h), SV/EC registration (ALREADY translated as
//  SecsSvEcRegistration, SECSGEM/SecsSvEcRegistration.h), and ~200 more
//  methods spanning spool files, terminal windows, upload/download, and the
//  ~90 S,F protocol handlers (those live on HTGem, SECSGEM/uHGemClass.h/.cpp,
//  a SEPARATE class that holds a `THGem *HGemPtr` -- see that file's own
//  notes).
//
//  This file creates `class THGem` FOR THE FIRST TIME. It is DELIBERATELY A
//  PARTIAL SLICE, not the full class: only the members/methods this wave's
//  scope actually needs are declared. THIS IS NOT THE FULL THGem --
//  a future wave (or several) will grow this SAME class definition
//  additively (add members/methods to the class body below), exactly the
//  way SECSGEM/uHGemClass.cpp has been unlocked incrementally, method group
//  by method group, across several past waves. Do NOT create a second
//  `class THGem` anywhere else -- this is the one and only definition.
//
//  THIS WAVE'S SLICE = the CEID/Report/Alarm "StringGrid database" family:
//    * Event/Report registration bookkeeping on strGrdCEID/stdGridReportID
//      (SetCEIDContent x2, GetCEIDContent, SetReportIDContent, DeleteReportID,
//      DeleteReportIDOfCeid, DeleteAllHostDefineReportID, GetReportIDContent,
//      IsEnableEvent, CheckCeidExist, GetReportIDType, CheckCeidAlreadyDefine,
//      CheckReportIDExist, AddCeidReportID, DeleteAllHostReportIDOfCeid,
//      DeleteHostReportIDOfAssignCeid, DeleteAllHostDefineCeid,
//      CheckReportAlreadyDefine, CheckCEIDExist(upper-case sibling),
//      EnableDisableEventReport, ReadEventReportData, SaveEventReportData).
//    * Generic StringGrid<->TStringList tab-format (de)serialization
//      (CopyStringGridAsTabFormat, PasteStringGridAsTabFormat, and the file-
//      scope helper SplitStrByTabOnly).
//    * The 3 Alarm-grid siblings (SetAlamData, ReadAlamData, WriteAlamData).
//  Plus, as a small standalone utility used throughout the real THGem (but
//  needed by none of the above -- included because it was named explicitly
//  in this wave's scope): `GemTimer` (golden uHGemEquipment.h:31-42).
//
//  NEW SUPPORTING SHIM: vclcompat/StringGrid.h (headless TStringGrid) was
//  created THIS wave alongside this file -- see that header for the full
//  design/scope note. strGrdCEID/stdGridReportID/strGrdAlarm/strGrdAlarmOld
//  below are allocated at the exact RowCount/ColCount their golden .dfm
//  entries specify (uHGemEquipment.dfm:375-436) -- see the constructor.
//
//  AI(W906-uHGemEquipment-ConnLifecycle) 20260717: SECOND WAVE landed on this
//  same class -- the TCP/IP connection-lifecycle slice: clientGem/srvGem
//  (TClientSocket*/TServerSocket*, allocated+wired in the ctor), their
//  connection-state bools/ints/GemTimers, Connect/DisConnect/IsConnect/OnLine
//  family/CloseCommuncation/CheckSocketActiveFalse, DoOpenCommuncation/
//  OnlineLocalOrRemote/DoOnLine, clientGemConnect/Disconnect/Error/Connecting,
//  srvGemClientConnect/GetSocketErrorMsg/Error/Disconnect, StringOut(1-arg)/
//  StringBinaryOut, SaveSECSGEMErrToLog, ClearDefaultEvenReport, and
//  GetTimeInfo's TimeString/GemClock half. See each method's own .cpp comment
//  for golden line citations.
//
//  AI(W906-uHGemEquipment-BucketB) 20260717: THIRD WAVE ("Bucket B" of the
//  original triple-front recon) landed on this same class -- the ~15 small
//  widget stand-ins (TRadioGroup/TEdit/TCheckBox/TComboBox/TPanel/
//  TSpeedButton/TMemo shapes, see the THGemXxx structs below) plus their
//  first real consumers: InitialHGem/SaveSystemDefault (ini-persisted widget
//  state), DoUpdateStatus (status-panel/button refresh + EventReport on
//  control-state transitions, INCLUDING the KYEC 30-second forced-disconnect
//  dance), ProcessShow, FormClose, ManualCreatergRoleClick, FormShow (partial
//  -- see its own .cpp comment), and the StringOut(AnsiString,TColor)
//  2-arg overload. A GATED NO-OP STUB for DoSeparate() is added too (golden's
//  real body is wire-codec/SML, out of scope -- DoUpdateStatus's KYEC branch
//  unconditionally calls it, same "stub so the caller's control flow stays
//  translatable" precedent as EnableDisableEventReportAcknowledgeError below).
//  DEPENDENCY WIDENING (flagged explicitly -- a real deviation from Bucket
//  A's own "does NOT include cmydef.h/common.h" claim above, which this wave
//  narrows rather than removes): DoUpdateStatus/InitialHGem/SaveSystemDefault
//  need CUSTOMER_CODE/CC_KYEC_LEE/CC_SIGURD_ChungXing/CC_MAXIM_THAILAND/
//  CosFunction/bSECSGEMbyPass/bSECSGEMConnectionFail (cmydef.h, which itself
//  pulls MachineType.h/cprod.h/cpublic.h) and ReadWriteIni/ReadIniData/
//  WriteIniData (common.h, confirmed lightweight -- only vclcompat +
//  MachineType.h, NOT the rest of golden's common.h). Both #includes are
//  confined to the .cpp (this header stays free of them); see root
//  CMakeLists.txt's ht9045_secsgem target for the matching new
//  ht9045_core/ht9045_globals link deps (same shape already established by
//  the KYECFTP/FTPClient_Transfer.cpp precedent for an identical need).
//  Still explicitly OUT OF SCOPE (see below): clientGemRead/
//  ProcessSocketReceiveData, DoConnect/DoSelect, Timer1Timer,
//  DoProcessSFNoResponse, DoLocalAllProcessLoop, FormCreate, and the
//  Left/Top/Width/Height/PageControl1 cosmetic half of FormShow (THGem is not
//  modeled as a real window in this port).
//
//  MEMBERS DELIBERATELY NOT PRESENT YET (out of scope; a future wave adds
//  them to this SAME class body, does not redefine it):
//    - The wire-codec-dependent connection functions: DoConnect/DoSelect
//      (need SecsWireCodec embedded as a member first), clientGemRead/
//      ProcessSocketReceiveData (need TMemoryStream/TFixedCriticalSection
//      shims, not yet designed), DoProcessSFNoResponse (needs a
//      TCriticalSection shim), Timer1Timer (its body calls into all of the
//      above) -- see each's golden citation at uHGemEquipment.cpp's
//      clientGemRead/DoConnect/Timer1Timer. (DoSeparate itself IS now
//      declared/defined -- as a GATED STUB, see above -- so it no longer
//      belongs on this specific "not present" list, but its REAL wire-codec
//      body is still deferred exactly like its DoConnect/DoSelect siblings.)
//    - DataItemInSub/DataItemIn/DataItemInNew/GetDataItemLenAndType(Sub)/
//      StringOut and SetSVDataPointer/SetECDataPointer/GetECDataValue -- per
//      the project's established design (see SecsWireCodec.h/
//      SecsSvEcRegistration.h's own "INTEGRATE-AGENT WIRING POINT" notes), a
//      future full-THGem wave should forward these to WireCodec/SvEcReg
//      (thin wrappers, matching HTGem's already-proven pattern in
//      SECSGEM/uHGemClass.h/.cpp) rather than re-implementing the logic
//      here. NOT done this wave. AI(W906-AlarmReportAck) 20260721:
//      InitLocalHead and ONE DataItemOut overload (the `(int len, unsigned
//      char Type, void *P)` pointer form) are the exception -- ADDED this
//      wave as one-line forwarders to THGem's own embedded WireCodec (see
//      the class body below), because this wave's new Report/Link
//      Acknowledge composer family (ReportAcknowledge/
//      ReportLinkAcknowledgeError/EnableDisableEventReportAcknowledgeError/
//      ProcessHostSendReportLinkID, all below) calls them bare (implicit
//      `this->`, per golden). The 2-arg `DataItemOut(unsigned char, AnsiString)`
//      overload golden's THGem.h also declares is still NOT added -- no
//      in-scope THGem-level caller uses it (verified by reading every new
//      method's body before writing this note); DataItemInSub/DataItemIn/.../
//      SetSVDataPointer/etc. remain fully out of scope as before.
//    - EnableDisableEventReportAcknowledgeError: AI(W906-AlarmReportAck)
//      20260721: NO LONGER a stub -- this wave gave it its real body (see the
//      .cpp), now that InitLocalHead/DataItemOut(pointer overload) exist on
//      THGem (immediately above). The paragraph below is preserved as
//      historical record of why it WAS stubbed:
//      it was THGem::EnableDisableEventReport's one dependency outside that
//      wave's StringGrid-only slice (its golden body is
//      InitLocalHead+DataItemOut+SendLocalData, i.e. the wire-codec family
//      above). Kept as a stub purely so EnableDisableEventReport's own
//      control flow (which unconditionally calls it) is translatable without
//      silently dropping the call.
//
//  AI(W906-uHGemEquipment-BucketC) 20260717: FOURTH wave ("Bucket C", the
//  final wave of the original triple-front recon) landed on this same class
//  -- the socket receive pump / T3 timeout machinery / HSMS control-message
//  handshake / Timer1Timer master state machine. THGem now embeds
//  `SecsWireCodec WireCodec;` BY VALUE (D1) -- codec state (LocalBuffer/
//  Local/Remote/bReceiveData/...) lives there, NOT duplicated on THGem; see
//  SendLocalDataFrom's own .cpp comment for the codec-state routing table.
//  Real methods added: clientGemRead, ProcessSocketReceiveData, Timer1Timer
//  (the ~350-line master SM), DoConnect, DoSelect, DoSeparate (REPLACES the
//  Bucket-B gated stub), DoProcessSFNoResponse, SendLocalData/
//  SendLocalDataFrom, DoLocalAllProcessLoop (shell + 4 gated callees),
//  CheckSFCodeResponse, SaveSECSGEMTextToLog, ShowLocalBufferBinaryData/
//  ShowLocalHeadInfo (each now 2 overloads), InitSTypeStruct, SelectRsp/
//  DeselectRsp/LinktestRsp, and ProcessReceiceData -- but ProcessReceiceData
//  is a SPLIT: the HSMS control-message head (Select/Deselect/Linktest/
//  Separate) is real; the ~180-line S,F DATA-MESSAGE dispatch tail stays ONE
//  gated `#if 0` block (needs live HSys.MyGem + ~15 flag members +
//  MoveCheckCallBack -- see that method's own .cpp comment for the full
//  rationale and the two-codec-instance boundary note it flags).
//  Still explicitly OUT OF SCOPE: FormCreate's SV/EC registration (a future
//  dedicated wave, brings SecsSvEcRegistration along), DoSpool/
//  DoUploadFileToHost/DoDownLoadRemoteFile (gated no-op stubs, same idiom as
//  EnableDisableEventReportAcknowledgeError).  AI(W906-trace) 20260804:
//  DoTraceDataResponse REMOVED from this list -- golden's REAL body now lives at
//  .cpp:4640-4693 and its caller was already live.  And the
//  ProcessReceiceData data-message tail above.
//
//  AI(W906-SysModWire) 20260720: FIFTH wave -- SystemModularInitial wiring.
//  The ProcessReceiceData tail's blocker list from the Bucket C note above
//  (live HSys.MyGem + ~15 flag members + MoveCheckCallBack) is now satisfied:
//  MoveCheckCallBack + the 6 bReceive* flags (bReceiveS7F6/S101F5/S101F6/
//  S101F7/S101F8/S110F2) are added members (see the class body below);
//  HSys.MyGem is wired by database.cpp's SystemModularInitial (new
//  SECSGEM/uHGemHT9045_Shim.h "thin shim" HT9045Gem class). The tail is now a
//  REAL block (single `if(HSys.MyGem != NULL)` guard around the golden body,
//  see ProcessReceiceData's own .cpp comment) -- no longer gated. Also added
//  this wave: GemMDLN/GemSOFTREV members + SetMachineTypeAndSoftwarseVer/
//  CheckSFFormatOnlyHead methods (both prerequisites for 8 of uHGemClass.cpp's
//  gated S,F handlers -- see that file's own un-gating note).
//
//  AI(W906-SvEcDataItem) 20260720: SIXTH wave -- SECSGEM closing-waves Wave 1
//  (design doc DESIGN_SECSGEM_closing_waves.md). THGem now embeds
//  `SecsSvEcRegistration SvEcReg;` BY VALUE (mirrors HTGem's own
//  already-proven precedent, uHGemClass.h:137) -- SV/EC registration
//  bookkeeping (SV_ID/SV_TYPE/.../EC_OldValue, SetSVDataPointer x4/
//  SetECDataPointer x4/GetECDataValue) now lives there, reused (not
//  reimplemented) via `#include "SECSGEM/SecsSvEcRegistration.h"`. Real
//  methods added: DataItemOutSV/DataItemOutSVNameList/
//  DataItemOutSVNameListWithValue/DataItemOutEC/DataItemOutECNameList (golden
//  VCL-widget-cast sub-branch GATED, `#if 0` -- see .cpp; the non-VCL raw-
//  ptr/AnsiString* path, which is ALL FormCreate ever registers, is real),
//  IsValidSVID, SendRepoerID/SendAnnotatedRepoerID/SendCeid/
//  SendAnnotatedCeid (Report/CEID data composers -- pure WireCodec+SV
//  composition, no VCL), and FormCreate itself (the "system SV" registration
//  block: GemClock/GemControlState/GemLinkState/SECSCommunicationMode/
//  GemControlPreState/CPU-freq-manufacturer-type (via new SECSGEM/
//  TasmInfo.{h,cpp})/disk-space/memory-status/GemMDLN/GemSOFTREV/
//  GemSpoolCountActual/GemSpoolStartTime/Time-Format EC/Receipe-Struct/
//  Receipe-Extend -- golden uHGemEquipment.cpp:6165-6207). The large bulk of
//  golden's SV/EC registration (uHGemHT9045_SV.cpp/_EC.cpp, ~877+~1740 calls)
//  stays OUT OF SCOPE (see SecsSvEcRegistration.h's own "INTEGRATE-AGENT
//  WIRING POINT" note -- unaffected by this wave).
//
//  AI(W906-AlarmReportAck) 20260721: SEVENTH wave -- SECSGEM closing-waves
//  Wave 2 (Alarm/Report-Ack family). Closes the exact InitLocalHead/
//  DataItemOut gap the SIXTH wave's own note (above) and the
//  EnableDisableEventReportAcknowledgeError stub both named as their
//  blocker. Added:
//    * 2 THGem-level wire-codec forwarders -- InitLocalHead(int,int,int) and
//      DataItemOut(int len, unsigned char Type, void *P), both one-line
//      forwards to THGem's own embedded WireCodec (same idiom as the
//      already-real SendLocalData() below). Only the ONE DataItemOut
//      overload this wave's new callers actually use is added (see that
//      declaration's own comment).
//    * 4 new members (golden uHGemEquipment.h:530-533): slTempReportID/
//      lTempReportIDContent/slTempCeID/lTempCeIDContent -- temp CEID/
//      ReportID staging lists populated by HTGem::
//      S2F34_DefineReportAcknowledgeSub/S2F36_LinkEventReportAcknowledgeSub
//      (uHGemClass.cpp, this same wave) while parsing an incoming S2F33/
//      S2F35 message, drained by ProcessHostSendReportID (STILL not present
//      -- see its own note below)/ProcessHostSendReportLinkID (below).
//    * 3 new Alarm methods (golden uHGemEquipment.cpp:3337-3372):
//      EnableDisableAlarm/EnableDisableAlarmAll/GetAlarmIndex -- pure
//      strGrdAlarm/WriteAlamData composition, zero new dependency.
//    * The Report/Link Acknowledge composer family (golden
//      uHGemEquipment.cpp:7838-8049): ProcessHostSendReportLinkID/
//      ReportAcknowledge(+5 named wrappers)/ReportLinkAcknowledgeError(+4
//      named wrappers) -- pure InitLocalHead/DataItemOut/SendLocalData
//      composition (now real, see above) plus already-real
//      CheckCeidExist/CheckCeidAlreadyDefine/CheckReportIDExist/
//      AddCeidReportID/DeleteHostReportIDOfAssignCeid/SaveEventReportData.
//    * EnableDisableEventReportAcknowledgeError given its REAL body
//      (replacing the no-op stub) + its 2 named wrappers
//      (EnableDisableEventReportAcknowledgeCeidNotExist/FormatError, golden
//      :8041-8049).
//  NOT added: golden's OWN `THGem::ProcessHostSendReportID()` (declared
//  golden uHGemEquipment.h:553) -- confirmed by grep to have ZERO definition
//  and ZERO call sites anywhere in golden's SECSGEM directory (a dead/
//  vestigial declaration golden itself never defines or calls; NOT the same
//  method as `HTGem::S2F34_ProcessHostSendReportID`, uHGemClass.cpp, which
//  IS translated this wave and needs no THGem-level sibling). Not declared
//  here either, matching golden's own effectively-abstract state for it.
//  See uHGemClass.cpp's own "INTEGRATE WAVE 5" note for the matching
//  8-method un-gating on the HTGem side.
//
//  AI(W906-uHGemClass-Micro5) 20260721: EIGHTH wave -- uHGemClass.cpp
//  "remaining 15 gated methods" cluster, micro-slice #5 (the last 4 of that
//  cluster resolvable without a whole new subsystem: recipe-upload-listing +
//  remote-recipe-checklist + EC-enable "StringGrid database"). Added:
//    * TStringList *UploadFileString -- see that member's own comment above
//      (mirrors HTGem's SecsAlarmMessage/FMessageList new/delete lifecycle,
//      NOT the default-NULL idiom below).
//    * THGemListBox *GemRemoteReceipeList -- see that member's own comment
//      above (mirrors TerminalMemoPtr's default-NULL/externally-assigned
//      idiom, NOT the new/delete idiom above -- the two members added this
//      wave deliberately use OPPOSITE lifecycles, each matching its own
//      golden precedent).
//    * TStringGrid *sgSECSECData + WriteECEnableData/EnableDisableECData/
//      EnableDisableECDataAll (golden uHGemEquipment.cpp:9257-9302) -- a
//      mechanical structural clone of the SEVENTH wave's own strGrdAlarm/
//      WriteAlamData/EnableDisableAlarm/EnableDisableAlarmAll family, just
//      for EC instead of Alarm. GOLDEN COMMENT preserved verbatim in the
//      .cpp (see EnableDisableECData/EnableDisableECDataAll's own citation):
//      "Steven 20150603 : T&0x10 --> T&0x80".
//  Consumed by HTGem::S101F2_CurrentEPPDData/S101F4_CurrentEPPDData/
//  Process_S7F20_CurrentEPPIDData/S125F2_EnableDisableECDataAcknowledge
//  (uHGemClass.cpp, this same wave) -- see that file's own "INTEGRATE WAVE 6"
//  note for the matching 4-method un-gating on the HTGem side.
//
// AI(W906-DoDownLoadRemoteFile) 20260721: NINTH wave -- un-gates
// THGem::DoDownLoadRemoteFile (golden uHGemEquipment.cpp:6746-6807), the
// remote-recipe-download (S7F5 PP-Request) retry state machine invoked from
// DoLocalAllProcessLoop. Added 4 members (golden uHGemEquipment.h:239/520-522):
// TStringList *RequestRemoteDownLoad (new/delete lifecycle, same idiom as
// UploadFileString above), int iRetryCTDownLoadRemoteFile (ctor 0),
// GemTimer DelayDownLoadRemoteFile (no ctor default, matches every other
// GemTimer member's own established precedent), int iDownLoadRemoteFileTask
// (ctor 1). Also added: the AnsiString-form `DataItemOut(unsigned char,
// AnsiString)` forwarder (golden .h:316) -- the EIGHTH wave's own note above
// explicitly deferred this exact overload for lack of an in-scope caller;
// DoDownLoadRemoteFile is now that caller (golden calls it bare, matching
// this wave's translated call site). No VCL widget dependency; bReceiveS7F6/
// bReceiveS101F5/bReceiveS101F7 already exist (SysModWire wave, FIFTH above).
//---------------------------------------------------------------------------
#ifndef uHGemEquipmentH
#define uHGemEquipmentH

#include "vclcompat/vcl_compat.h"
#include "vclcompat/StringGrid.h"
// AI(W906-SpoolCluster) 20260721: headless TFileListBox shim (THGem's own
// FileListBox1 member, Spool cluster) -- see vclcompat/FileListBox.h's own
// file-head note for the full design.
#include "vclcompat/FileListBox.h"
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT.TotalEvent (array bound in SetCEIDContent)
#include <vector>   // AI(W906-uHGemClass-Micro7) 20260721: THGemCheckedArray's backing store (see below)

// AI(W906-uHGemEquipment-BucketC) 20260717: D1 -- embed `SecsWireCodec
// WireCodec;` by value (see the class body below), following the proven
// HTGem precedent (uHGemClass.h:127). This header supplies HTypeStruct/
// STypeStruct/HSMS_Head_Struct + `extern HType` per SecsWireCodec.h:99-107's
// own "INTEGRATE-AGENT WIRING POINT" note -- they are NOT redeclared here.
#include "SECSGEM/SecsWireCodec.h"
// AI(W906-SvEcDataItem) 20260720: SvEcReg embed (see file-head note) --
// reuses SecsSvEcRegistration's own SetSVDataPointer/SetECDataPointer/
// GetECDataValue + SV_*/EC_* bookkeeping lists rather than reimplementing
// them on THGem. #include, not redeclare (ODR-safe, same reuse pattern
// SecsSvEcRegistration.h itself already established for HType).
#include "SECSGEM/SecsSvEcRegistration.h"
// THGem::FormCreate's SV10-19 (CPU/disk/memory) source functions.
#include "SECSGEM/TasmInfo.h"
// TCriticalSection (TFixedCriticalSection's base below; csSFCodeResponse).
#include "vclcompat/SyncObjs.h"
// TMemoryStream (RecvMemoryBuffer/ProcBuffer/TempProcBuffer below); already
// brings TMemoryStream/soFromBeginning/soFromCurrent/soFromEnd into the
// global namespace itself (see that header's own tail note).
#include "vclcompat/MemoryStream.h"
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SIG-W18) 20260826

// This TU does not include aHotPlateSubstrate.h (the other place a global
// `class TList` lives -- see vclcompat/vcl_compat.h's own TList.h precedent
// note) and none of this wave's methods need vclcompat::TList, so bringing
// TStringGrid into the global namespace here is collision-free (grepped: no
// other `class TStringGrid` exists anywhere in this tree).
using vclcompat::TStringGrid;
// AI(W906-SpoolCluster) 20260721: same collision-check discipline as
// TStringGrid immediately above -- grepped the whole Cpp tree for
// `class TFileListBox`/`TFileListBox` before adding this: no other
// definition exists anywhere outside vclcompat/FileListBox.h itself, so
// bringing it into the global namespace here is collision-free.
using vclcompat::TFileListBox;
// TCriticalSection is NOT auto-brought into the global namespace by
// vclcompat/SyncObjs.h (matches TStringGrid.h's own posture) -- brought in
// explicitly here, same idiom as the TStringGrid line above (golden spells
// both types unqualified: `TCriticalSection *csSFCodeResponse;`,
// `class TFixedCriticalSection : public TCriticalSection`).
using vclcompat::TCriticalSection;

//---------------------------------------------------------------------------
//  GemTimer -- golden uHGemEquipment.h:31-42.
//  A simple elapsed-tick-count timer (BCB6 idiom: TimerOn()/TimerSet(...)
//  arm it, TimerOff() polls "has the interval elapsed yet").
//
//  golden ulStartTicks is filled from `MyTickCount()` (common.cpp:1726-1729,
//  a bare `return GetTickCount();` wrapper). common.h's own translation
//  currently has MyTickCount gated out (`#if 0 // TODO(wave-timing)`,
//  D:/HT9045/HT9011UC_Cpp_V3.33.906.0/common.h:304-305) and common.h/.cpp are
//  NOT in this front's allowed file list, so GemTimer calls
//  `::GetTickCount()` (windows.h) DIRECTLY here rather than depending on an
//  extern that doesn't exist as a linkable symbol yet -- semantically
//  IDENTICAL to what MyTickCount() would return (it is nothing but that one
//  call). When a future wave un-gates common.h's MyTickCount, this can be
//  simplified to call it instead; behavior does not change either way.
//---------------------------------------------------------------------------
class GemTimer
{
    private:
        DWORD   ulStartTicks;           // tick count when the timer was (re)started
        int     iTimeLen;               // timer duration, in milliseconds
    public:
        // AI(W906-fire-verify) 20260716: DEVIATION, flagged by fire-wave
        // fidelity review -- golden's GemTimer (uHGemEquipment.h:31-42) has NO
        // constructor, so a real BCB6 instance's fields are whatever memory
        // happened to hold until TimerSet*/TimerOn is first called. Zero-init
        // added here defensively (needed by this wave's own "never-armed
        // timer" test case, and safe/behavior-preserving for every real golden
        // call path since none reads ulStartTicks/iTimeLen before TimerSet*).
        GemTimer() : ulStartTicks(0), iTimeLen(0) {}

        void TimerSet(int iTime);              // golden: iTime is in units of 0.1s
        void TimerSetSecAndOn(double iTime);    // golden: iTime is in seconds
        void TimerSetMSAndOn(int iTime);        // golden: iTime is in milliseconds
        void TimerOn();
        bool TimerOff();
};

//---------------------------------------------------------------------------
//  AI(W906-uHGemEquipment-BucketB) 20260717: TColor -- golden Graphics.hpp
//  TColor (a plain BGR-packed int RGB code, 0x00BBGGRR). vclcompat/
//  vcl_compat.h has no such alias (confirmed by grep, matching this wave's
//  own brief's own note). A separate `typedef int TColor;` ALREADY exists
//  TU-locally in cmydef.h:16 and acatchtray_shims.h:41 (same underlying
//  type, so no ODR conflict if both ever appear in one TU) -- declared again
//  here, header-local, so uHGemEquipment.h stays independently compilable
//  without pulling in either of those files just for a type alias (the .cpp
//  separately #includes cmydef.h for the CUSTOMER_CODE/CosFunction family --
//  see this header's own file-head "DEPENDENCY WIDENING" note -- so TColor
//  would in practice resolve to the SAME identical typedef there too).
//  Only the 3 clXxx constants golden's DoUpdateStatus actually reads are
//  declared (verified by reading golden uHGemEquipment.cpp:4747-4985 -- NOT
//  clBlack; StringOutColor's own clBlack use, golden :502/1992, is outside
//  this wave's member set and not translated).
//---------------------------------------------------------------------------
typedef int TColor;
const TColor clRed    = 0x000000FF;
const TColor clLime   = 0x0000FF00;
const TColor clYellow = 0x0000FFFF;
// AI(W906-uHGemEquipment-BucketC) 20260717: clBlack -- golden StringOutColor's
// ctor default (golden :502, `StringOutColor=clBlack;`) and SendLocalDataFrom's
// own reset (golden :1992). Real VCL clBlack is 0x00000000 (BGR-packed).
const TColor clBlack  = 0x00000000;

//---------------------------------------------------------------------------
//  AI(W906-uHGemEquipment-BucketB) 20260717: THGemXxx widget stand-ins --
//  THGem was a TForm (golden uHGemEquipment.h:90); these tiny structs stand
//  in for its real VCL components/externally-assigned pointers, matching the
//  established per-owner-class idiom already used by vclcompat/StringGrid.h
//  (TStringGrid, above) and FormsFacade.h's own TfMainTrayPanel{int
//  Color;bool Visible;}-shaped stand-ins -- each struct here models ONLY the
//  fields this wave's in-scope methods (InitialHGem/SaveSystemDefault/
//  DoUpdateStatus/ProcessShow/ManualCreatergRoleClick/FormShow/StringOut)
//  actually read or write; see each member declaration below for its exact
//  golden field citation.
//---------------------------------------------------------------------------

// golden TRadioGroup* (EnableOrDisablePtr/OnLineOrOffLine/RemoteOrLocal/
// rgRole) -- ->ItemIndex (int, r/w), ->Enabled (bool, w -- only
// OnLineOrOffLine/RemoteOrLocal are ever ->Enabled-written, by FormShow's
// KYEC branch), ->Items->Count (int, r -- InitialHGem/SaveSystemDefault pass
// it as ReadWriteIni's Minimum bound; golden's own .dfm Items.Strings count
// per radio group, e.g. 2 for OnLineOrOffLine/RemoteOrLocal/rgRole -- a
// caller/test must set this explicitly, same "caller-set" idiom already
// established by THGem::GemSystemPath above).
struct THGemRadioGroup
{
    int ItemIndex;
    bool Enabled;
    struct { int Count; } Items;
    THGemRadioGroup() : ItemIndex(0), Enabled(true) { Items.Count = 0; }
};

// golden TEdit* (edtIP/edtPort/edDeviceID/edtT3TimeOut/edtT5TimeOut/
// edtT6TimeOut/edtT7TimeOut/edtT8TimeOut -- NOTE: golden has NO edtT4TimeOut,
// confirmed by reading golden uHGemEquipment.h:117-129/123-129; T3/T5/T6/T7/T8
// only) -- ->Text (AnsiString, r/w), ->Enabled (bool, w -- only edtIP is ever
// ->Enabled-written).
struct THGemEdit
{
    AnsiString Text;
    bool Enabled;
    THGemEdit() : Enabled(true) {}
};

// golden TCheckBox* (GemCheckBoxAcceptHostOnlineRequest/GemCheckBoxShowBinary/
// GemCheckBoxShowHeadInformation/GemCheckBoxUseExtendedAlarm/
// chkAnnotatedEventReport/chkMoreMessageAbortProcess/ckAddDefaultReport/
// cbECChaneEventReport) -- ->Checked (bool, r/w); ->Visible (bool, w -- only
// cbECChaneEventReport is ever ->Visible-written, by FormShow).
struct THGemCheckBox
{
    bool Checked;
    bool Visible;
    THGemCheckBox() : Checked(false), Visible(true) {}
};

// golden TComboBox* (ComboBox1) -- ->ItemIndex (int, read only in this
// wave's scope: DoUpdateStatus's `SECSCommunicationMode=ComboBox1->ItemIndex;`).
struct THGemComboBox
{
    int ItemIndex;
    THGemComboBox() : ItemIndex(0) {}
};

// golden TPanel* (SECSConnectionState/GEMCommunicatingState/
// GemPanelControlState) -- externally-assigned (public, not __published;
// confirmed by golden header: declared alongside GemTerminalSendEdit/
// BtnEnableComm etc in THGem's plain `public:` section, NOT inside the
// `__published:` IDE-component block) -- ->Caption (AnsiString, r/w,
// compared against literals like "1:OffLine"/"SECS GEM Connection"),
// ->Color (TColor, w).
struct THGemPanel
{
    AnsiString Caption;
    TColor Color;
    THGemPanel() : Color(0) {}
};

// golden TSpeedButton* (BtnEnableComm/GemBtnOfflineRequest/
// GemBtnOnlineRequest/GemBtnOnlineRemote/GemBtnOnlineLocal) -- externally
// assigned (same public-not-__published category as THGemPanel above);
// ->Enabled (bool, write only in this wave's scope -- DoUpdateStatus never
// reads it back). UNLIKE the 3 TPanel*s above, golden's own DoUpdateStatus
// body does NOT NULL-guard these 5 -- a genuine golden invariant ("must be
// wired externally before DoUpdateStatus ever runs"), not a translation gap;
// preserved as-is (a caller/test must assign a real instance to each before
// calling DoUpdateStatus, exactly matching golden's own real-VCL risk).
struct THGemSpeedButton
{
    bool Enabled;
    THGemSpeedButton() : Enabled(true) {}
};

// golden TMemo* (DB/TerminalMemoPtr) -- ->Lines (a TStrings*; modeled here
// directly as a real vclcompat::TStringList*, which already supports the
// exact ->Add/->Assign(TStringList*)/->Count/->Clear() calls golden's own
// ProcessShow/StringOut(2-arg) bodies make -- no extra "Lines" wrapper layer
// needed, unlike FormsFacade.h's own no-op TfMainMemoLines shape, because
// THIS wave's tests need to inspect REAL flushed content, not just a call
// count), ->SelStart (int, w). Both DB and TerminalMemoPtr are externally
// assigned in golden (DB via the out-of-scope SetDisplayPtr(TMemo*);
// TerminalMemoPtr likewise, always NULL-guarded at its own call site per
// this file's Bucket-A commentary) -- THGem does NOT allocate either; a
// caller/test must `new` one and assign it (see THGem's own ctor note).
struct THGemMemo
{
    TStringList *Lines;
    int SelStart;
    THGemMemo() : SelStart(0) { Lines = new TStringList(); }
    ~THGemMemo() { delete Lines; }
    THGemMemo(const THGemMemo&) = delete;
    THGemMemo& operator=(const THGemMemo&) = delete;
    void Clear() { Lines->Clear(); }
};

//---------------------------------------------------------------------------
//  AI(W906-uHGemClass-Micro7) 20260721: THGemCheckedArray -- stand-in for the
//  indexed bool property golden's real VCL TCheckListBox exposes as
//  `->Checked[i]` (golden uHGemEquipment.h:414 `TCheckListBox
//  *GemRemoteReceipeList;`). This wave's in-scope call sites
//  (S101F6_StoreHostUploadFile/S101F8_StoreHostUploadFile, uHGemClass.cpp,
//  golden :2571/:2634 `GemRemoteReceipeList->Checked[i]=false;`) only ever
//  WRITE to it -- golden elsewhere (out of this wave's scope) also READS
//  Checked[i], so this stand-in is deliberately shaped to add read support
//  later WITHOUT reshaping: `operator[]` returns a Proxy whose
//  `operator=(bool)` auto-grows the backing `std::vector<bool>` to fit
//  index `i` (default-false-filled) before storing -- a future wave just
//  adds `operator bool() const` to Proxy, touching neither this member's
//  declaration nor any existing call site. Deliberately NOT sized off
//  Items->Count automatically (golden's own TCheckListBox ties Checked's
//  size to the list's item count as a side effect of Items->Add(), which
//  this minimal stand-in does not model) -- auto-grow-on-write is the
//  simplest faithful substitute for this wave's write-only scope.
//---------------------------------------------------------------------------
struct THGemCheckedArray
{
    std::vector<bool> Values;
    struct Proxy
    {
        std::vector<bool> &Values;
        int Index;
        Proxy(std::vector<bool> &v, int i) : Values(v), Index(i) {}
        Proxy& operator=(bool val)
        {
            if ((int)Values.size() <= Index)
                Values.resize((size_t)Index + 1, false);
            Values[(size_t)Index] = val;
            return *this;
        }
        // AI(W906-FW-GEM-W12) 20260826: 這個 stand-in 原本是**唯寫**的
        //（見上方 :530-531 的 scope 說明）。golden 的檔案傳輸 UI 家族會**讀**它：
        //   if(GemRemoteReceipeList->Checked[i])   (uHGemEquipment.cpp:6588/6595)
        //   if(GemLocalFileLixtBox->Checked[i])    (:6624)
        // 沒有讀取路徑時那三行編不過。補一個 operator bool()：
        // 索引超出已寫入範圍時回 false，與 VCL TCheckListBox 未勾選的預設一致。
        operator bool() const
        {
            if (Index < 0 || (int)Values.size() <= Index)
                return false;
            return Values[(size_t)Index];
        }
    };
    Proxy operator[](int i) { return Proxy(Values, i); }
};

//---------------------------------------------------------------------------
//  AI(W906-uHGemEquipment-BucketC) 20260717: THGemListBox -- stand-in for
//  golden TListBox *SFCodeResponseList (uHGemEquipment.h:138, __published).
//  Fields actually used in this wave's scope: ->Items (Add :2062 /
//  Strings[i] :4644 / Delete :4654 / Count :4617 / IndexOf :7040 / Clear
//  :7037) and ->Clear() (:4624, real TListBox::Clear == Items.Clear).
//  Same minimal-stand-in idiom as the Bucket-B THGemXxx widgets above.
//  AI(W906-uHGemClass-Micro7) 20260721: ADDED `Checked` (THGemCheckedArray,
//  see its own comment immediately above) -- golden's own GemRemoteReceipeList
//  is a real TCheckListBox (golden uHGemEquipment.h:414), a TListBox
//  SUBCLASS that adds exactly this indexed Checked property; this reuses the
//  EXISTING THGemListBox stand-in (already standing in for GemRemoteReceipeList
//  since Micro5) rather than forking a second struct, so SFCodeResponseList
//  (the OTHER THGemListBox instance, which golden's real TListBox base class
//  has no Checked property on at all) simply carries an always-unused Checked
//  member -- harmless, and avoids a THGemCheckedArray-vs-plain-TListBox split
//  that no call site in this project actually needs yet.
//---------------------------------------------------------------------------
struct THGemListBox
{
    TStringList *Items;
    THGemCheckedArray Checked;
    THGemListBox() { Items = new TStringList(); }
    ~THGemListBox() { delete Items; }
    THGemListBox(const THGemListBox&) = delete;
    THGemListBox& operator=(const THGemListBox&) = delete;
    void Clear() { Items->Clear(); }
    // AI(W906-FW-GEM-W10) 20260826: golden GemRemoteReceipeListClick 讀
    // `GemRemoteReceipeList->ItemIndex`（uHGemEquipment.cpp:9212）。VCL 的語意是
    // -1 代表沒有選取項，這裡照用；沒有任何載入路徑會設它，所以本樹恆為 -1。
    int ItemIndex = -1;
};

// AI(W906-uHGemEquipment-BucketC) 20260717: TFixedCriticalSection --
// transcribed verbatim from golden uHGemEquipment.h:25-29 (golden `byte
// FDummy[95];` -> `unsigned char FDummy[95];`, noted deviation -- `byte` is
// a BCB6 Windows.pas alias for `unsigned char`, identical width/semantics).
// Needs vclcompat/SyncObjs.h's TCriticalSection (included above) as its base.
//---------------------------------------------------------------------------
class TFixedCriticalSection : public TCriticalSection
{
    private:
    unsigned char FDummy[95];
};

//---------------------------------------------------------------------------
//  THGem -- see this header's own file-head note above for scope.
//---------------------------------------------------------------------------
class THGem
{
public:
    THGem();
    ~THGem();

    THGem(const THGem&) = delete;
    THGem& operator=(const THGem&) = delete;

    // ==== StringGrid "databases" (golden uHGemEquipment.h:139-144) =========
    // Allocated (in the ctor) at the exact RowCount/ColCount their golden
    // .dfm entries specify -- see vclcompat/StringGrid.h's file-head note
    // for the full citation. Public so a test harness / future wave can
    // inspect them directly (matches golden __published visibility, which
    // in real VCL made these form-designer-visible/public-ish already).
    TStringGrid *strGrdAlarmOld;    // uHGemEquipment.dfm:375-383 (ColCount=<default 5>, RowCount=1)
                                     // NOTE: ReadAlamData (see .cpp) only ever
                                     // assigns RowCount, never ColCount -- a
                                     // real THGem needs ColCount widened
                                     // elsewhere (out of this wave's scope,
                                     // presumably to match strGrdAlarm's 12)
                                     // before ReadAlamData can hold a full
                                     // AlarmData.def row (11 used columns).
                                     // Confirmed empirically by this wave's
                                     // own test (tests/test_uHGemEquipment.cpp).
    TStringGrid *strGrdCEID;        // uHGemEquipment.dfm:396-405 (ColCount=258,  RowCount=1025)
    TStringGrid *stdGridReportID;   // uHGemEquipment.dfm:406-415 (ColCount=1026, RowCount=257)
    TStringGrid *strGrdAlarm;       // uHGemEquipment.dfm:428-437 (ColCount=12,   RowCount=<default 5>)

    // AI(W906-uHGemClass-Micro5) 20260721: sgSECSECData -- golden
    // uHGemEquipment.h:155 (__published), .dfm:452-462 (ColCount=4, RowCount=1,
    // FixedRows=0) -- the EC-side sibling of strGrdAlarm immediately above
    // (same "StringGrid database" shape: col0=No./col1=ECID/col2=Enable(0/1)/
    // col3=Function -- see golden uHGemEquipment.cpp:640-648, ReadECEnableData).
    // Allocated (ctor) at its exact .dfm dimensions, same idiom as
    // strGrdAlarm/strGrdCEID/stdGridReportID above.
    TStringGrid *sgSECSECData;      // uHGemEquipment.dfm:452-462 (ColCount=4, RowCount=1)

    // AI(W906-AlarmReportAck) 20260721: temp CEID/ReportID staging lists
    // (golden uHGemEquipment.h:530-533) -- populated by HTGem::
    // S2F34_DefineReportAcknowledgeSub/S2F36_LinkEventReportAcknowledgeSub
    // (uHGemClass.cpp, HGemPtr->...) while parsing an incoming S2F33/S2F35
    // message; drained by ProcessHostSendReportLinkID (below) /
    // HTGem::S2F34_ProcessHostSendReportID (uHGemClass.cpp). `vclcompat::TList`
    // spelled out fully-qualified here (NOT brought into the global
    // namespace by this header, matching SecsSvEcRegistration.h's own
    // established precedent -- see vcl_compat.h's own "TList deliberately
    // NOT brought into global namespace" note for why: a different,
    // unrelated global `class TList` shim lives in aHotPlateSubstrate.h).
    TStringList *slTempReportID;
    vclcompat::TList *lTempReportIDContent;
    TStringList *slTempCeID;
    vclcompat::TList *lTempCeIDContent;

    // ==== supporting state this wave's methods need ========================
    // golden AnsiString member (uHGemEquipment.h:428/429/430 area); populated
    // for real by THGem::SaveSystemDefault/InitialHGem (AI(W906-uHGemEquipment-
    // BucketB) 20260717: now in scope, see file-head note -- this comment's
    // original "OUT OF SCOPE" is stale). Defaults to "" here; a caller/test
    // must set it explicitly before calling ReadAlamData/WriteAlamData.
    AnsiString GemSystemPath;

    // ==== TCP/IP connection lifecycle (W906-uHGemEquipment-ConnLifecycle) ===
    // golden uHGemEquipment.h:93-94 (clientGem/srvGem), :185-227/309/451-479/
    // 591/693/700-701 (state bools/ints/AnsiStrings), .dfm:542-573 (clientGem/
    // srvGem design-time defaults, applied in the ctor -- see the .cpp).
    // TClientSocket/TServerSocket/TCustomWinSocket/TErrorEvent all come from
    // vclcompat/ClientSocket.h + vclcompat/ServerSocket.h, already pulled in
    // transitively by vclcompat/vcl_compat.h above (both headers' own
    // `using namespace Scktcomp;` already brings these into global scope).
    TClientSocket *clientGem;   // golden .dfm:542-554 (active/client role)
    TServerSocket *srvGem;      // golden .dfm:562-573 (passive/server role)

    bool bConnect;                // golden uHGemEquipment.h:309
    bool bOnLine;                 // golden :693
    bool bOnLineLocal;            // golden :224
    bool bAutoConnect;            // golden :186
    bool bStartConnect;           // golden :221
    bool bStartOnLine;            // golden :226
    bool bTCPIP_Error;            // golden :187
    bool bServoSocketConnect;     // golden :457
    bool bReceiveMultiConnect;    // golden :591
    bool bUseClientSocket;        // golden :464
    // AI(W906-uHGemEquipment-ConnLifecycle) 20260717: bOpenCommuncation/
    // bCloseCommuncation/bS1F2_OnLineData are golden members (:397,398,393)
    // not named in this wave's own header-additions list, but DoOpenCommuncation/
    // CloseCommuncation/OnlineLocalOrRemote/DoOnLine (all explicitly in this
    // wave's scope) directly read/write them -- added here as the minimal
    // extra surface those in-scope functions require to compile at all.
    bool bOpenCommuncation;       // golden :397
    bool bCloseCommuncation;      // golden :398
    bool bS1F2_OnLineData;        // golden :393

    int countConnect;                       // golden :477
    int iConnectTryCount;                   // golden :478
    int iEstablishCommunicationsTryCount;    // golden :222
    int iOpenCommuncationTask;               // golden :475
    int iStartConnectTask;                  // golden :220
    int iStartOnLineTask;                   // golden :225
    // AI(W906-uHGemEquipment-ConnLifecycle) 20260717: iTimeFormat (golden
    // :233) is likewise not in this wave's enumerated member list, but
    // GetTimeInfo (in scope) branches on it directly.
    int iTimeFormat;                        // golden :233

    GemTimer DelayOpenCommuncation;   // golden :476
    GemTimer ConnectDelay;            // golden :479

    // AI(W906-uHGemEquipment-BucketC) 20260717: D4 -- these two are ALIASED
    // (not separately allocated) as of this wave. WireCodec allocates its own
    // WaitShowString/LogDataString (SecsWireCodec.cpp ctor); THGem's ctor
    // assigns these two pointers to point at WireCodec's instances instead of
    // `new`ing its own pair, so codec-side StringOut() calls (SendLocalDataFrom,
    // ShowSML, ...) land in the SAME sink THGem's own ProcessShow/
    // SaveSECSGEMTextToLog drain -- golden had exactly ONE such pair; before
    // this wave the port had two (THGem's own + WireCodec's), so codec-side
    // trace lines never reached ProcessShow/the log file. ~THGem does NOT
    // delete these two (ownership = WireCodec; its dtor, which runs AFTER
    // ~THGem's body per member-destruction order, deletes them for real).
    TStringList *WaitShowString;   // golden :597 (ALIASED to WireCodec.WaitShowString, see ctor)
    TStringList *LogDataString;    // golden :447 (ALIASED to WireCodec.LogDataString, see ctor)

    AnsiString TimeString;   // golden :700
    AnsiString GemClock;     // golden :302
    // AI(W906-SysModWire) 20260720: GemMDLN/GemSOFTREV -- golden uHGemEquipment.h
    // :306-307. Set by SetMachineTypeAndSoftwarseVer(Mdln,SoftVer) (golden
    // :6510-6514, declared below near DoConnect); golden's own ctor never
    // touches either (both stay AnsiString-default "" until that setter runs) --
    // deliberately NOT added to this ctor's init-list, matching that golden
    // "ctor does not touch" semantics exactly (C++ default-constructs AnsiString
    // to "" either way, so omitting from the init-list is behavior-neutral).
    AnsiString GemMDLN;      // golden :306
    AnsiString GemSOFTREV;   // golden :307
    // NOTE: golden's real 3rd field name is "SystemDate" (uHGemEquipment.h:230),
    // not "SystemDay" -- kept as golden spells it (GetTimeInfo/DecodeDate both
    // reference it by this exact name).
    Word SystemYear, SystemMonth, SystemDate;           // golden :230
    Word SystemHour, SystemMin, SystemSec, SystemMSec;  // golden :231

    // ==== Bucket C: wire-codec-embedded engine + socket-receive-pump / ======
    // ==== T3-timeout / HSMS-handshake / Timer1Timer master-SM state    ======
    // (W906-uHGemEquipment-BucketC 20260717; golden line cites per-member.)
    // D1: embedded BY VALUE, following HTGem's own already-proven precedent
    // (uHGemClass.h:127). The WaitShowString/LogDataString raw pointers
    // declared earlier in this class are ALIASED to WireCodec's own lists in
    // the ctor BODY (which runs only after every member -- including this one
    // -- has finished constructing, so relative declaration order between
    // them is immaterial; the dtor likewise skips deleting the aliased pair,
    // see ~THGem). Does NOT embed SecsSvEcRegistration this wave -- no
    // in-scope method touches SV/EC registration state (FormCreate itself is
    // out of scope, see file-head note) -- deferred to a future "SV/EC
    // registration wave".
    SecsWireCodec WireCodec;

    // AI(W906-SvEcDataItem) 20260720: SvEcReg -- SV/EC registration
    // bookkeeping (see file-head note above). By-value embed, default-
    // constructed (SecsSvEcRegistration's own ctor allocates its 18 heap
    // TStringList*/TList*s; nothing extra needed in THGem's own ctor).
    SecsSvEcRegistration SvEcReg;

    STypeStruct SType;                              // golden :209 (InitSTypeStruct() populates it, ctor call below)
    TColor StringOutColor;                           // golden :237 (ctor = clBlack)

    bool bWaitSelectRsp;                             // golden :390 (ctor false)
    bool bWaitDeSelectRsp;                           // golden :391 (ctor false)
    bool bWaitEstablishCommunicationsResponse;       // golden :691 (ctor false)
    bool bWaitEstablishCommunicationsResponseError;  // golden :692 (ctor false)
    bool bReceiveEstablishCommunicationsRequest;     // golden :399 (ctor false)
    bool bSeprate;                                   // golden :394 (ctor false)
    // golden ctor-untouched (real BCB6 zero-inits for free; explicit here per
    // this file's own established "flagged deviation" precedent, see
    // DoUpdateStatus's own supporting-state note below).
    bool bDataFormatOK;                              // golden :528
    bool bFirstEntry;                                 // golden :454 (ctor :450 true)
    bool bFirstBlock;                                 // golden :472 (ctor :472 true)

    // AI(W906-SysModWire) 20260720: ProcessReceiceData's S,F data-message
    // dispatch tail (golden :8812-8988) needs these -- MoveCheckCallBack (a
    // GemCallBack installer, golden :319, ctor :503 NULL; the only real
    // installer in golden is UsecegemMainFrom.cpp:625, TFSECS -- untranslated,
    // so this stays NULL offline) and 6 flag members set (never guarded) by
    // the tail's own dispatch branches.
    int (*MoveCheckCallBack)();          // golden :319 (ctor :503 NULL)
    bool bReceiveS7F6;                   // golden :459 (ctor :462 false)
    bool bReceiveS101F5;                 // golden :460 (ctor :463 false)
    bool bReceiveS101F6;                 // golden :461 (ctor :464 false)
    bool bReceiveS101F7;                 // golden :462 (ctor :465 false)
    bool bReceiveS101F8;                 // golden :463 (ctor :466 false)
    // AI(W906-SysModWire) 20260720: bReceiveS110F2 (golden :625) -- GOLDEN BUG
    // preserved: golden's own ctor NEVER initializes this member (grep confirms
    // no ctor assignment anywhere in golden uHGemEquipment.cpp), and no golden
    // code ever READS it either (write-only at golden :8942). This port's ctor
    // explicitly zero-inits it below (matching this file's own established
    // "flagged deviation from golden's raw uninitialized state" precedent, see
    // bDataFormatOK/iOldSecProcessSFNoResponse/RemoteSystemByte above) -- the
    // ONLY behavioral non-bit-identity this wave introduces vs. golden.
    bool bReceiveS110F2;                 // golden :625 -- see note above

    int  iFileCount;                                  // golden :721 (ctor :672 = 0)
    int  Timer1Task, Timer1ct;                        // golden :519 (ctor :494-495 = 1, 0)
    Word iOldSecProcessSFNoResponse;                  // golden :508 (ctor-untouched; zero-init, flag)
    int  iAutoConnectSec, iAutoConnectDelay;          // golden :509 (ctor :485-486 = 0, 2)
    unsigned int RemoteSystemByte;                    // golden :212 (ctor-untouched; zero-init, flag)

    AnsiString Alias;                                 // golden :529
    // golden TForm's own `Caption` property (Timer1Timer :5251 `Caption=Alias;`)
    // -- THGem is not modeled as a real window in this port (same "not a real
    // window" posture as FormShow's own dropped Left/Top/Width/Height) -- a
    // plain AnsiString stand-in suffices since nothing reads it back except
    // that same assignment's own right-hand side.
    AnsiString Caption;

    GemTimer DelayForServoError;                      // golden :592

    // golden :473-474 (ctor :474-475 = 10240 + `new[]`; dtor `delete[]`).
    // NOTE: ProcessSocketReceiveData's own LOCAL `unsigned char *EthernetBuffer`
    // (golden :9035) SHADOWS this member inside that one function's body --
    // a genuine golden quirk (the member is never actually read/written by
    // that function; only the local is) -- preserved, see that method's own
    // .cpp comment.
    unsigned char *EthernetBuffer;
    int iEthernetBufferLen;

    THGemListBox *SFCodeResponseList;                 // golden :138 (__published; ctor-allocated)
    TStringList  *TimeLeft;                           // golden :229 (ctor :584 new)
    TFixedCriticalSection *pLockOnSocketRecvice;      // golden :717 (ctor :668 new)
    TCriticalSection      *csSFCodeResponse;          // golden :723 (ctor :673 new)
    TMemoryStream *RecvMemoryBuffer;                  // golden :718 (ctor :669 new)
    TMemoryStream *ProcBuffer;                        // golden :719 (ctor :670 new)
    TMemoryStream *TempProcBuffer;                    // golden :720 (ctor :671 new)

    // ==== Widget stand-ins (W906-uHGemEquipment-BucketB) ====================
    // golden uHGemEquipment.h's __published block (:92-165) for the first 20
    // (all real .dfm components -- allocated in THGem's own ctor, see the
    // .cpp; never NULL in a normally-constructed THGem, matching real VCL's
    // Owner/.dfm-streaming guarantee), and its plain `public:` section
    // (:400-423) for the trailing 11 (externally-assigned pointers, default
    // NULL -- a caller/test must wire one up before exercising the method
    // that needs it; see each THGemXxx struct's own comment above for the
    // exact golden field-category citation).
    THGemRadioGroup *EnableOrDisablePtr;     // golden :413 (externally assigned, default NULL)
    THGemRadioGroup *OnLineOrOffLine;        // golden :112 (__published)
    THGemRadioGroup *RemoteOrLocal;          // golden :113 (__published)
    THGemRadioGroup *rgRole;                 // golden :122 (__published)

    THGemEdit *edtIP;                        // golden :123 (__published)
    THGemEdit *edtPort;                      // golden :124 (__published)
    THGemEdit *edDeviceID;                   // golden :161 (__published)
    THGemEdit *edtT3TimeOut;                 // golden :125 (__published)
    THGemEdit *edtT5TimeOut;                 // golden :126 (__published)
    THGemEdit *edtT6TimeOut;                 // golden :127 (__published)
    THGemEdit *edtT7TimeOut;                 // golden :128 (__published)
    THGemEdit *edtT8TimeOut;                 // golden :129 (__published)

    THGemCheckBox *GemCheckBoxAcceptHostOnlineRequest;   // golden :131 (__published)
    THGemCheckBox *GemCheckBoxShowBinary;                // golden :132 (__published)
    THGemCheckBox *GemCheckBoxShowHeadInformation;       // golden :133 (__published)
    THGemCheckBox *GemCheckBoxUseExtendedAlarm;          // golden :134 (__published)
    THGemCheckBox *chkAnnotatedEventReport;              // golden :135 (__published)
    THGemCheckBox *chkMoreMessageAbortProcess;           // golden :136 (__published)
    THGemCheckBox *ckAddDefaultReport;                   // golden :153 (__published)
    THGemCheckBox *cbECChaneEventReport;                 // golden :162 (__published)

    THGemComboBox *ComboBox1;                // golden :111 (__published)

    THGemPanel *SECSConnectionState;         // golden :400 (externally assigned, default NULL)
    THGemPanel *GEMCommunicatingState;       // golden :401 (externally assigned, default NULL)
    THGemPanel *GemPanelControlState;        // golden :402 (externally assigned, default NULL)

    THGemSpeedButton *BtnEnableComm;         // golden :405 (externally assigned, default NULL)
    THGemSpeedButton *GemBtnOfflineRequest;  // golden :408 (externally assigned, default NULL)
    THGemSpeedButton *GemBtnOnlineRequest;   // golden :407 (externally assigned, default NULL)
    THGemSpeedButton *GemBtnOnlineRemote;    // golden :409 (externally assigned, default NULL)
    THGemSpeedButton *GemBtnOnlineLocal;     // golden :410 (externally assigned, default NULL)

    THGemMemo *DB;                // golden :300 (externally assigned via out-of-scope SetDisplayPtr, default NULL)
    THGemMemo *TerminalMemoPtr;   // golden :677 (externally assigned, default NULL, always null-guarded at its call site)

    // AI(W906-uHGemClass-Micro7) 20260721: TerminalListboxPtr/TerminalEditPtr/
    // TerminalPanelPtr/TerminalDisplayIndex + the "2"-suffixed mirror set --
    // un-gates HTGem::S10F4_TerminalDisplaySingleAcknowledge/
    // S10F6_TerminalDisplayMultiBlockAcknowledge (uHGemClass.cpp, this wave).
    // Golden mirrors ALL 5 base "Terminal*" concepts with a second, fully
    // parallel "2"-suffixed set (golden uHGemEquipment.h:677-686) -- this is
    // not a partial echo, both halves are read/written symmetrically by
    // S10F4's own body (see that method's own .cpp comment). Same
    // default-NULL/externally-assigned idiom as TerminalMemoPtr immediately
    // above (a caller/test must `new` a THGemListBox/THGemEdit/THGemPanel and
    // assign it; THIS class does NOT allocate any of them) -- golden's own
    // TListBox*/TCustomEdit*/TPanel* map onto the ALREADY-EXISTING
    // THGemListBox/THGemEdit/THGemPanel stand-ins (no new struct needed,
    // confirmed by reading golden's own S10F4/S10F6 bodies: only
    // ->Items/->Clear() (THGemListBox), ->Text (THGemEdit), ->Caption
    // (THGemPanel) are ever touched -- all already exposed). TerminalDisplayIndex/
    // TerminalDisplayIndex2 are the one int-typed pair in this set; UNLIKE the
    // rest of this member, golden's own ctor DOES explicitly zero them
    // (uHGemEquipment.cpp:599-600) -- see THGem's own ctor below for the
    // matching explicit 0 (not a "flagged deviation", a literal golden match).
    THGemListBox *TerminalListboxPtr;   // golden :678 (externally assigned, default NULL)
    THGemEdit *TerminalEditPtr;         // golden :679 (externally assigned, default NULL)
    THGemPanel *TerminalPanelPtr;       // golden :680 (externally assigned, default NULL)
    int TerminalDisplayIndex;           // golden :681 (ctor 0, golden ctor :599)
    THGemMemo *TerminalMemoPtr2;        // golden :682 (externally assigned, default NULL)
    THGemListBox *TerminalListboxPtr2;  // golden :683 (externally assigned, default NULL)
    THGemEdit *TerminalEditPtr2;        // golden :684 (externally assigned, default NULL)
    THGemPanel *TerminalPanelPtr2;      // golden :685 (externally assigned, default NULL)
    int TerminalDisplayIndex2;          // golden :686 (ctor 0, golden ctor :600)

    // AI(W906-uHGemClass-Micro5) 20260721: GemRemoteReceipeList -- golden
    // uHGemEquipment.h:414 (`TCheckListBox *GemRemoteReceipeList;`, plain
    // `public:`, NOT __published) -- externally assigned, matching golden's
    // own ctor :504 `GemRemoteReceipeList=NULL;` (right alongside
    // `MoveCheckCallBack=NULL;` immediately above it there). Reuses the
    // EXISTING THGemListBox stand-in (above, already used for
    // SFCodeResponseList) -- its ->Items/->Clear() cover every field
    // HTGem::Process_S7F20_CurrentEPPIDData (uHGemClass.cpp, this wave)
    // touches. UNLIKE SFCodeResponseList (__published, ctor-allocated --
    // golden .dfm-streamed before any real VCL form's ctor body runs), this
    // one is NOT allocated here -- same default-NULL/externally-assigned
    // idiom as TerminalMemoPtr immediately above (a caller/test must `new`
    // one and assign it; Process_S7F20's own golden NULL-guard, preserved
    // verbatim in uHGemClass.cpp, is what makes that safe).
    THGemListBox *GemRemoteReceipeList;

    // golden AnsiString members (:524-526); read by InitialHGem as the
    // ReadIniData "DefaultValue" fallback for Address/Port/DeviceID when no
    // ini entry exists yet. Ctor-inited to "" (golden ctor :451-453).
    AnsiString DefaultAddress;
    AnsiString DefaultPort;
    AnsiString DefaultDeviceID;

    // golden int members (:632-636, "pig 2014.07.28 KYEC_SECS"); ctor-inited
    // to 30 (golden ctor :454-458).
    int T3TimeOut;
    int T5TimeOut;
    int T6TimeOut;
    int T7TimeOut;
    int T8TimeOut;

    // golden AnsiString member (:429); the ini file InitialHGem/SaveSystemDefault
    // actually read/write (DISTINCT from GemSystemPath above, which
    // ReadAlamData/WriteAlamData use for AlarmData.def). Defaults to ""; a
    // caller/test must set it explicitly, same idiom as GemSystemPath.
    AnsiString GemSystemIniPath;

    // golden bool member (:... FormShow/ManualCreatergRoleClick's shared
    // "has the form actually been shown yet" latch -- gates
    // ManualCreatergRoleClick's whole body). Ctor-inited false (golden ctor
    // :617, "bShow=false;").
    bool bShow;

    // ==== DoUpdateStatus's own supporting state (golden uHGemEquipment.h
    // "GEM SV data" block :190-192, plus :... UpdateStatus locals) ==========
    // AI(W906-uHGemEquipment-BucketB) 20260717: golden's own ctor NEVER
    // explicitly assigns bClientSocketActive/bOldConnect/GemControlState/
    // GemControlPreState/OldSUpdateStatus/ctUpdateStatus (grepped golden's
    // whole ctor body, uHGemEquipment.cpp:444-674 -- confirmed absent) --
    // real BCB6/VCL zero-initializes EVERY instance field before a ctor body
    // even runs (TObject.NewInstance zeroes the block), so golden gets
    // false/0/"" for these "for free" without an explicit assignment. C++
    // gives no such guarantee, so (matching this file's own established
    // "flagged deviation: zero-init defensively" precedent, see GemTimer's
    // ctor note) all 6 are explicitly zero-initialized in THGem's own ctor
    // below.
    int ctUpdateStatus;                 // golden :... (the /10 update throttle counter)
    bool bClientSocketActive;
    int iServoConnectCT;                // golden ctor :492 (explicitly -1, NOT zero -- see ctor)
    bool bOldConnect;
    bool flag1UpdateStatus;             // golden ctor :489 (explicitly true, NOT zero -- see ctor)
    bool flag2UpdateStatus;             // golden ctor :490 (explicitly true)
    bool flag3UpdateStatus;             // golden ctor :491 (explicitly true)
    bool bConnectUpdateStatus;          // golden ctor :493 (explicitly true)
    AnsiString OldSUpdateStatus;
    unsigned char GemControlState;
    unsigned char GemControlPreState;
    char SECSCommunicationMode;         // golden ctor :500 (explicitly 0)

    // ==== FormCreate's "system SV" targets (W906-SvEcDataItem 20260720) =====
    // golden uHGemEquipment.h:191/196/198/235/303-304/602-604/608-617/701/707
    // (member declarations) + uHGemEquipment.cpp:6165-6207 (FormCreate, the
    // sole registration site -- see .cpp). None of these is ever explicitly
    // assigned by golden's own ctor (uHGemEquipment.cpp:444-674, grepped) --
    // real BCB6/VCL zero-inits every instance field for free; all are
    // explicitly zero/empty-initialized in THIS port's ctor below, matching
    // this file's own established "flagged deviation: zero-init defensively"
    // precedent (see GemTimer/DoUpdateStatus's own ctor notes above).
    unsigned char GemLinkState;                    // golden :191 (SV5)
    long lCPUFreq;                                  // golden :602 (SV10, via TasmInfo::GetCPUFreq)
    char szManID[256];                              // golden :603 (SV11, via TasmInfo::GetManID)
    char szGetCPUType[256];                         // golden :604 (SV12, via TasmInfo::GetCPUType)
    int Disk_C_TotalSpaceMB;                        // golden :608 (SV13)
    int Disk_D_TotalSpaceMB;                        // golden :609 (SV14)
    int Disk_C_TotalFreeSpaceMB;                    // golden :611 (SV15)
    int Disk_D_TotalFreeSpaceMB;                    // golden :612 (SV16)
    unsigned long ulMemoryLoad;                     // golden :615 (SV17)
    unsigned long ulTotalPhys;                      // golden :616 (SV18)
    unsigned long ulAvailPhys;                      // golden :617 (SV19)
    // golden :303-304/235 (EC68 "Time Format" min/max/default bounds --
    // iTimeFormat itself already exists above, golden :233).
    int iMinTimeFormat;                             // golden ctor :650 (explicitly 0)
    int iMaxTimeFormat;                             // golden ctor :651 (explicitly 3)
    int iTimeFormatDefault;                         // golden NEVER inits this either (see note above) -- flagged deviation
    int SV_70_UNT1_ReceipeStruct;                   // golden :707 (SV70)
    AnsiString SV_71_ASCII_FilenameExtened;         // golden :701 (SV71)

    // AI(W906-uHGemClass-Micro5) 20260721: UploadFileString -- golden
    // uHGemEquipment.h:675 (part of golden's SV/EC-cluster private members,
    // ctor-allocated alongside SV_ID/EC_ID -- golden uHGemEquipment.cpp:567
    // `UploadFileString=new TStringList;`, ~THGem :713/744 Clear()+delete).
    // Added standalone here rather than folded into SvEcReg -- that embed
    // (above) models the DISTINCT SV_ID/SV_TYPE/EC_ID/... registration-table
    // cluster only (confirmed by reading SecsSvEcRegistration.h: no
    // UploadFileString member there); this is a simple recipe/upload-filename
    // staging list, populated by SetReceipeDirectoryAndGlobalName (still out
    // of scope) and read by S101F2/S101F4/Process_S7F20's sibling S7F20 (all
    // uHGemClass.cpp -- S101F2/S101F4 un-gated this wave). Lifecycle idiom
    // deliberately mirrors HTGem's OWN SecsAlarmMessage/FMessageList pair
    // (uHGemClass.h:104/242; uHGemClass.cpp ctor/dtor) -- `new`'d in THGem's
    // ctor, `delete`d (NULL-guarded, ->Clear()'d first) in THGem's dtor -- see
    // both .cpp definitions for the exact shape this mirrors.
    TStringList *UploadFileString;

    // AI(W906-DoDownLoadRemoteFile) 20260721: RequestRemoteDownLoad -- golden
    // uHGemEquipment.h:239 (part of the SAME SV/EC-cluster private-member
    // block as UploadFileName/RequestRemoteDownLoad/ALID_ALED there; NOT the
    // SV/EC registration-table cluster SvEcReg models -- confirmed by reading
    // SecsSvEcRegistration.h, no such member). Golden ctor-allocates it
    // alongside UploadFileName/TimeLeft (.cpp:582 `RequestRemoteDownLoad=new
    // TStringList;`) and separately Clears it right after (.cpp:586) -- see
    // this wave's own ctor comment for that redundant-but-harmless-on-a-
    // freshly-allocated-list Clear() call, reproduced verbatim. Golden's own
    // ~THGem (uHGemEquipment.cpp:724/755) Clears then deletes it for real --
    // this port mirrors UploadFileString's own established NULL-guarded
    // Clear-then-delete dtor idiom (see that member's own header comment),
    // NOT golden's un-guarded version, same already-accepted deviation class.
    // Populated by DoDownLoadRemoteFile's own case 1/100/200 (this wave);
    // golden's other writer, SetReceipeDirectoryAndGlobalName, stays out of
    // scope (same blocker as UploadFileName/UpLoadPath above).
    //
    // NAMING TRAP (flagged in this wave's own brief): do not confuse with
    // UploadFileName (a DIFFERENT TStringList*, golden .h:228, upload-family
    // scope, NOT added here) or UploadFileString (a THIRD, already-existing
    // member, S101F2/S101F4 scope, unrelated lifecycle purpose despite the
    // similar new/delete shape).
    TStringList *RequestRemoteDownLoad;

    // AI(W906-DoDownLoadRemoteFile) 20260721: DoDownLoadRemoteFile's own
    // retry-loop state (golden uHGemEquipment.h:520-522, ctor :496-497 for
    // the 2 ints -- see the .cpp ctor init list). DelayDownLoadRemoteFile
    // (GemTimer) gets NO explicit ctor entry, same established precedent as
    // DelayOpenCommuncation/ConnectDelay/DelayForServoError above -- its own
    // default ctor already zero-inits defensively (see GemTimer's own header
    // comment).
    int iRetryCTDownLoadRemoteFile;                 // golden ctor :496 (explicitly 0)
    GemTimer DelayDownLoadRemoteFile;
    int iDownLoadRemoteFileTask;                    // golden ctor :497 (explicitly 1)

    int GemSpoolCountActual;                        // golden :196 (SV54)
    char GemSpoolStartTime[256];                    // golden :198 (SV57)

    // ==== S6F24/S7F18 supporting state (W906-uHGemClass-Micro6) =============
    // golden uHGemEquipment.h:430/694-695/698. bSpoolActive/bBeginTransferSpool
    // are S6F24_RequestSpooledDataAcknowledgementSend's own gate + latch
    // (ctor false, golden ctor :604/607 -- see THGem's ctor below); with the
    // safe false default, S6F24's only side effect (the `system("del ...")`
    // spool-wipe) stays a no-op until some future "DoSpool subsystem" wave
    // actually drives bSpoolActive true -- NOT built this wave, per plan.
    // GemSpoolPath is the directory that wipe targets (golden ctor :814 builds
    // it from CurrentDirectory+"SPOOL", out of scope; defaults to "" here,
    // same "caller/test must set it explicitly" idiom as GemSystemPath above).
    // AI(W906-uHGemClass-Micro6) 20260721: added 3 THGem members to un-gate
    // S6F24_RequestSpooledDataAcknowledgementSend (golden uHGemClass.cpp:2053-2079).
    bool bSpoolActive;                              // golden :694 (ctor false)
    bool bBeginTransferSpool;                       // golden :695 (ctor false)
    AnsiString GemSpoolPath;                        // golden :430

    // UpLoadPath -- golden :698. The recipe-upload base directory S7F18
    // (delete process program) resolves each PPID against (`UpLoadPath+"\\"+
    // PPID`) and SetReceipeDirectoryAndGlobalName (still out of scope, see
    // S7F20's own gate note above UploadFileString) is its only real writer
    // in golden. Plain scalar, no ctor default in golden's own ctor either
    // (grepped uHGemEquipment.cpp:444-674 -- absent), so it defaults to ""
    // here too, same as UpLoadPath's own AnsiString() default ctor would give
    // BCB6 for free.
    // AI(W906-uHGemClass-Micro6) 20260721: added to un-gate
    // S7F18_DeleteProcessProgramAcknowledge (golden uHGemClass.cpp:2115-2166).
    AnsiString UpLoadPath;                          // golden :698

    // ==== Trace cluster (W906-uHGemClass-TraceUnlock 20260728) ==============
    // golden uHGemEquipment.h:674 (TraceData[10]), :696 (bTraceData[10]),
    // :699 (iTRID[10]), :702-706 (DSPER/iTOTSMP/iREPGSZ/iTOTSMP_Count/
    // TraceDataResponseTask[10]), :485 (TraceDataResponseDelay[10]). Un-gates
    // HTGem::S2F24_TraceInitializeAcknowledgeSub (uHGemClass.cpp, this wave --
    // golden :810-990). DESIGNED to also satisfy THGem's own
    // DoTraceDataResponse (uHGemEquipment.cpp:4190-4242, the S6F1 Trace Data
    // Send retry state machine that shares this SAME member cluster) -- that
    // AI(W906-trace) 20260804: SUPERSEDED for DoTraceDataResponse -- no longer a
    // gated stub, golden's real body landed at .cpp:4640-4693.  The rest of this note
    // still applies to the OTHER methods named.  (Formerly: "that
    // method stays its own separate gated stub".)  Un-gating IT was explicitly
    // NOT part of THIS wave's assigned scope (S2F24Sub, uHGemClass.cpp); this
    // member set is simply shaped so a future wave can un-gate it
    // mechanically, citing this same golden line range, rather than inventing
    // a second, incompatible set of Trace members later.
    //
    // TraceData[10] (TStringList*): per-trace-slot SVID list, `new`'d in the
    // ctor / NULL-guarded Clear-then-delete in the dtor, same lifecycle idiom
    // already established by UploadFileString/RequestRemoteDownLoad/
    // UploadFileName above (golden ctor :588-591 loop + ~THGem :766-773 loop,
    // both reproduced verbatim as ctor-body/dtor-body `for` loops over index
    // 0..9 -- see .cpp).
    TStringList *TraceData[10];                     // golden :674

    // bTraceData[10]: per-slot "is this trace slot active" latch. golden ctor
    // :596-597 explicitly sets every slot false -- reproduced verbatim.
    bool bTraceData[10];                            // golden :696 (ctor false)

    // iTRID[10]/DSPER[10]/iTOTSMP[10]/iREPGSZ[10]/iTOTSMP_Count[10]/
    // TraceDataResponseTask[10]: golden's own ctor (uHGemEquipment.cpp:
    // 380-680, grepped) NEVER explicitly initializes any of these 6 -- same
    // "flagged deviation: zero-init defensively" posture already established
    // by iTimeFormatDefault/SpoolPtr/OldSpoolSystemMin elsewhere in this file
    // (real BCB6 zero-inits every instance field for free; this port must do
    // so explicitly -- see .cpp ctor).
    AnsiString iTRID[10];                           // golden :699
    unsigned DSPER[10];                             // golden :702
    unsigned iTOTSMP[10];                           // golden :703
    unsigned iREPGSZ[10];                           // golden :704
    unsigned iTOTSMP_Count[10];                     // golden :705
    int TraceDataResponseTask[10];                  // golden :706

    // TraceDataResponseDelay[10] (GemTimer): DoTraceDataResponse's OWN retry-
    // delay timer array (golden :485) -- NOT read by S2F24Sub itself (added
    // here only for the "coherent set both consumers can use" reason stated
    // above). Gets NO explicit ctor entry, same established precedent as
    // every other GemTimer member in this file (its own default ctor already
    // zero-inits defensively -- see GemTimer's own header comment).
    GemTimer TraceDataResponseDelay[10];            // golden :485

    // AI(W906-uHGemClass-Micro7) 20260721: bFinishDownloadFile/CurrentDirectory
    // -- un-gates S101F6_StoreHostUploadFile/S101F8_StoreHostUploadFile
    // (uHGemClass.cpp, this wave). bFinishDownloadFile (golden :626) is
    // S101F6's own "whole multi-part upload finished" latch; golden never
    // explicitly initializes it (grepped ctor body -- absent), so zero-init
    // defensively here, same established precedent as this cluster's own
    // bSpoolActive/bBeginTransferSpool above. CurrentDirectory (golden :427)
    // was READ-ONLY as of that wave (S101F8 only reads it, via
    // IncludeTrailingPathDelimiter) -- golden's own `SetCurrentDirectory(Path)`
    // setter (golden :426/.cpp:810-819, which ALSO cascades into
    // GemSystemPath/GemSpoolPath/GemSystemIniPath) was deliberately NOT
    // ported at that time, out of that wave's scope.
    // AI(W906-SpoolCluster) 20260721: UN-GATED for real this wave (see .cpp)
    // -- the above "deliberately NOT ported" note is now STALE, kept only for
    // the historical record. SetCurrentDirectory has zero remaining blockers:
    // IncludeTrailingPathDelimiter already exists as this TU's own local
    // static helper (see top of .cpp), and MyForceDirectories is already real
    // (common.cpp, already #include'd via common.h above). A caller/test may
    // still set CurrentDirectory/GemSystemPath/GemSpoolPath/GemSystemIniPath
    // directly if it wants to bypass the cascade (same "caller/test must set
    // explicitly" idiom used everywhere else in this cluster) -- but now has
    // the real setter available too.
    bool bFinishDownloadFile;                       // golden :626
    AnsiString CurrentDirectory;                    // golden :427
    void SetCurrentDirectory(AnsiString Path);      // golden :426/.cpp:810-819

    // ==== Spool cluster (W906-SpoolCluster) =================================
    // golden uHGemEquipment.h:98 (FileListBox1), :185 (bSpooling), :470
    // (ctSpoolFile), :481-484 (iSpoolTask/SpoolPtr,SpoolRunPtr/
    // OldSpoolSystemMin/SpoolDelay), :197 (GemSpoolCountTotal). Un-gates
    // THGem::DoSpool/WriteToSpoolFile/SetSpoolActive/GetSpoolActive/
    // DoSpoolSendLocalData (golden .cpp :1887-1977/:4025-4189/:6133-6161,
    // this wave). bSpoolActive/bBeginTransferSpool/GemSpoolPath (golden :430/
    // 694-695) and GemSpoolCountActual/GemSpoolStartTime (golden :196/198,
    // SV53/SV57) already exist above (S6F24/SvEcDataItem waves) -- NOT
    // re-declared here.
    //
    // FileListBox1 is THGem's own private FileListBox1-equivalent widget
    // (golden .dfm:20-28, TFileListBox, design-time Mask='spool\\*.dat', no
    // Directory set). See vclcompat/FileListBox.h's own file-head note for
    // why this port's stand-in resolves its Mask deterministically from
    // GemSpoolPath instead of replicating golden's CWD-dependent default --
    // a disclosed deviation, NOT a silent fix. Allocated in the ctor body /
    // deleted in ~THGem(), same established "no VCL form-ownership mechanism
    // here" convention already used for strGrdCEID/strGrdAlarm/... above
    // (golden itself never `new`s/`delete`s FileListBox1 -- it is form-owned
    // by the .dfm streaming system, which this port does not have).
    //
    // bSpooling (golden :185): WRITE-ONLY in every golden call site inside
    // this wave's own scope (DoSpool sets it, nothing golden-side ever reads
    // it back) -- confirmed dead state, preserved faithfully (not removed
    // just because it looks unused).
    //
    // ctSpoolFile/iSpoolTask: golden explicitly initializes both in its ctor
    // BODY (ctSpoolFile=0 :471, iSpoolTask=1 :481 -- assignment statements
    // there, not an init-list in golden; this port uses its own established
    // init-list idiom for the same end state, see .cpp). NOTE golden quirk
    // preserved verbatim: WriteToSpoolFile's own ctSpoolFile-increment logic
    // is DEAD CODE (commented out in golden itself, .cpp :1898-1924) -- so
    // ctSpoolFile stays 0 forever in practice, and every spool filename
    // WriteToSpoolFile generates for a given exact-same-second timestamp is
    // identical (suffix always "000"); see that method's own .cpp comment.
    //
    // OldSpoolSystemMin/SpoolPtr/SpoolRunPtr/GemSpoolCountTotal: golden NEVER
    // explicitly initializes any of these in its own ctor (grepped ctor body
    // -- absent) -- zero/NULL-initialized here defensively, same "flagged
    // deviation from golden's raw uninitialized state" precedent already
    // established by GemTimer's own header comment / this file's
    // iEstablishCommunicationsTryCount note. SpoolDelay (GemTimer) gets NO
    // explicit ctor entry, same established precedent as
    // DelayDownLoadRemoteFile/DelayOpenCommuncation/ConnectDelay above -- its
    // own default ctor already zero-inits defensively.
    TFileListBox *FileListBox1;                     // golden :98
    // AI(W906-UploadFamily) 20260723: FileListBox2 -- golden uHGemEquipment.h:99
    // (`TFileListBox *FileListBox2;`, __published, the SECOND TFileListBox
    // instance immediately after FileListBox1 there). Reuses the SAME
    // vclcompat::TFileListBox shim as FileListBox1 -- that class's own
    // file-head note (vclcompat/FileListBox.h) already anticipated this
    // exact reuse ("golden's OTHER two FileListBox instances (FileListBox2
    // -- DoUploadFileToHost family; ...) are explicitly OUT OF SCOPE for
    // this wave; nothing below is specialized to FileListBox1 in a way that
    // would block reusing this same class for them later"). FileListBox2's
    // only readers/writers are THGem::DoUploadFileToHost_ForSingleFile/
    // _ForMultiFile/_ForDirectoryFile (this wave, see .cpp) -- every call
    // site there bakes a directory into ->Mask before ->Update()/->Refresh(),
    // the same "directory folded into Mask" shape FileListBox1's own shim
    // comment documents. Allocated in the ctor body / deleted in ~THGem(),
    // same "no VCL form-ownership mechanism here" convention as FileListBox1
    // immediately above (golden itself never `new`s/`delete`s FileListBox2
    // either -- also form-owned by the .dfm streaming system this port does
    // not have).
    TFileListBox *FileListBox2;                     // golden :99
    bool bSpooling;                                 // golden :185 (ctor false, write-only -- dead state, preserved)
    int ctSpoolFile;                                // golden :470 (ctor 0; increment logic is dead in golden, see above)
    int iSpoolTask;                                 // golden :481 (ctor 1)
    unsigned char *SpoolPtr;                        // golden :482 (golden never inits -- NULL here defensively)
    unsigned char *SpoolRunPtr;                     // golden :482 (golden never inits -- NULL here defensively)
    WORD OldSpoolSystemMin;                         // golden :483 (golden never inits -- 0 here defensively)
    GemTimer SpoolDelay;                            // golden :484
    int GemSpoolCountTotal;                         // golden :197 (golden never inits -- 0 here defensively)

    // ==== DoUploadFileToHost family (W906-UploadFamily 20260723) ===================
    // golden uHGemEquipment.h:228 (UploadFileName), :234
    // (EC69_UNT1_MaxTranslateLen), :419 (GemLocalFileLixtBox), :486-507
    // (Task-state scalars/FILE*/AnsiString members), :523
    // (PtrUploadFileToHost_ForSingleFile), :622 (ForMultiFileFileName).
    // Un-gates THGem::DoUploadFileToHost_ForSingleFile/_ForMultiFile/
    // _ForDirectoryFile + the DoUploadFileToHost() 3-way dispatcher itself
    // (golden .cpp :4249-4599, this wave) -- see .cpp for each function's
    // own body/citation. FileListBox2 (declared immediately above) and
    // UpLoadPath/SV_70_UNT1_ReceipeStruct (already declared further above,
    // golden :698/:707) are this family's other dependencies -- NOT
    // re-declared here.
    //
    // UploadFileName (golden .h:228) -- queue of local filenames staged for
    // upload. golden ctor-allocates it alongside RequestRemoteDownLoad/
    // TimeLeft (.cpp:583 `UploadFileName=new TStringList;`) and separately
    // Clear()s it a few lines later (.cpp:594) -- a redundant-but-harmless
    // Clear() on a freshly-allocated empty list, reproduced verbatim in this
    // port's ctor (see .cpp). golden's own ~THGem (.cpp:725/756) Clear()s
    // then deletes it for real -- this port mirrors UploadFileString/
    // RequestRemoteDownLoad's own established NULL-guarded Clear-then-delete
    // dtor idiom (see those members' own comments above), NOT golden's
    // un-guarded version, same already-accepted deviation class.
    //
    // NAMING TRAP (same one flagged at RequestRemoteDownLoad's own comment
    // above): UploadFileName is a DIFFERENT TStringList* from
    // RequestRemoteDownLoad (golden .h:239, DoDownLoadRemoteFile's own
    // queue) and from UploadFileString (golden .h:675, S101F2/S101F4 scope)
    // despite the similar new/delete shape -- all 3 coexist.
    TStringList *UploadFileName;                    // golden :228 (ctor :583, new TStringList)

    // EC69_UNT1_MaxTranslateLen (golden .h:234) -- plain int, read by
    // DoUploadFileToHost_ForSingleFile's own case 100 (see .cpp) to pick the
    // per-chunk send size. Grepped golden's own ctor body
    // (uHGemEquipment.cpp:444-674) -- ABSENT, golden itself never
    // initializes this either (only its "...Default" sibling at .h:236,
    // EC69_UNT1_MaxTranslateLenDefault, is a future SV/EC-registration
    // concern, out of THIS family's scope, deliberately NOT added here) --
    // same "flagged deviation: zero-init defensively" posture already
    // established by iTimeFormatDefault's own header comment above. Do NOT
    // invent a nonzero default.
    int EC69_UNT1_MaxTranslateLen;                  // golden never inits this either -- flagged deviation, see note above

    // GemLocalFileLixtBox (golden .h:419) -- IMPORTANT: golden's real type
    // is `TCheckListBox *GemLocalFileLixtBox;` (plain `public:`, NOT
    // __published -- re-confirmed by re-reading the golden header for this
    // wave), the SAME real VCL base class GemRemoteReceipeList (above)
    // stands in for -- so this reuses the EXISTING THGemListBox stand-in
    // (->Items->IndexOf(), ->Checked[j]=... via THGemCheckedArray), NOT
    // TFileListBox despite the "FileLixtBox" name (a golden typo for
    // "FileListBox", carried through faithfully, not corrected). Golden's
    // own ctor sets it to NULL (.cpp:510 `GemLocalFileLixtBox=NULL;`, right
    // alongside GemRemoteReceipeList's own :504) -- externally assigned,
    // same default-NULL idiom as GemRemoteReceipeList; NOT allocated here (a
    // caller/test must `new` a THGemListBox and assign it). None of this
    // family's 3 sub-functions NULL-guards it before calling
    // ->Items->IndexOf() in golden (unlike GemSelectAllRemoteFileClick's own
    // NULL-guard elsewhere in this file) -- a golden gap preserved verbatim,
    // NOT fixed here; a caller/test must supply a non-null instance.
    THGemListBox *GemLocalFileLixtBox;              // golden :419 (TCheckListBox in golden; ctor NULL)

    // ---- DoUploadFileToHost_ForSingleFile's own Task-state (golden .h:486-490) ----
    int iUploadFileToHost_ForSingleFile;                   // golden :486 (Task; ctor :482, explicitly 1)
    long UploadFileToHost_ForSingleFileTotalFileSize;      // golden :487 (golden never inits -- 0 here defensively)
    FILE *UploadFileToHost_ForSingleFileFilePtr;           // golden :488 (golden never inits -- NULL here defensively)
    int iUploadFileToHost_ForSingleFileStoreCT;            // golden :489 (golden never inits -- 0 here defensively)
    int iUploadFileToHost_ForSingleFileTotalCount;         // golden :489 (golden never inits -- 0 here defensively)
    // AI(W906-UploadFamily) 20260723: PRESERVED GOLDEN BUG (cited again, verbatim,
    // at its own use site in .cpp case 100) -- golden .cpp:4259
    // `int iMaxSend=iUploadFileToHost_ForSingleFileMaxSend;` is a PLAIN
    // VALUE COPY (not a reference, unlike the sibling `int &Task=...`/
    // `long &TotalFileSize=...` aliases on the same lines), and this member
    // is NEVER assigned anywhere else in golden -- grepped the ENTIRE golden
    // tree for this identifier (`grep -rn iUploadFileToHost_ForSingleFileMaxSend`
    // across HT9011UC_Code_V3.33.906.0_20260618): its ONLY 3 appearances
    // anywhere are the .h:490 declaration and .cpp:259/269 (both inside
    // DoUploadFileToHost_ForSingleFile itself). So the chunk size golden
    // computes into it at case 100 (from EC69_UNT1_MaxTranslateLen) is lost
    // the moment the function returns -- see
    // DoUploadFileToHost_ForSingleFile's own .cpp comment for the resulting
    // apparent 0-byte-send stall this produces once Task reaches 200/300.
    // Translated as a plain copy here too, NOT fixed.
    int iUploadFileToHost_ForSingleFileMaxSend;            // golden :490 (golden never assigns this anywhere else -- see PRESERVED GOLDEN BUG note above)

    // ---- DoUploadFileToHost_ForMultiFile's own Task-state (golden .h:491-498) ----
    // UploadFileToHost_ForMultiFileDelay (GemTimer) gets NO explicit ctor
    // entry, same established precedent as DelayDownLoadRemoteFile/
    // SpoolDelay/ConnectDelay above -- its own default ctor already
    // zero-inits defensively.
    GemTimer UploadFileToHost_ForMultiFileDelay;           // golden :491
    int iUploadFileToHost_ForMultiFileTask;                // golden :492 (Task; ctor :483, explicitly 1)
    int iUploadFileToHost_ForMultiFileCT;                  // golden :493 (golden never inits -- 0 here defensively)
    long UploadFileToHost_ForMultiFileTotalFileSize;       // golden :494 (golden never inits -- 0 here defensively)
    FILE *PFileUploadFileToHost_ForMultiFile;              // golden :495 (golden never inits -- NULL here defensively)
    AnsiString UploadFileToHost_ForMultiFileMultiFile;     // golden :496 (golden never inits -- "" here defensively)
    int iStoreCTUploadFileToHost_ForMultiFile;             // golden :497 (golden never inits -- 0 here defensively)
    int iTotalCountUploadFileToHost_ForMultiFile;          // golden :498 (golden never inits -- 0 here defensively)

    // ---- DoUploadFileToHost_ForDirectoryFile's own Task-state (golden .h:499-507) ----
    int iUploadFileToHost_ForDirectoryFileTask;            // golden :499 (Task; ctor :484, explicitly 1)
    int iCTUploadFileToHost_ForDirectoryFile;              // golden :500 (golden never inits -- 0 here defensively)
    long TotalFileSizeUploadFileToHost_ForDirectoryFile;   // golden :501 (golden never inits -- 0 here defensively)
    FILE *FilePUploadFileToHost_ForDirectoryFile;          // golden :502 (golden never inits -- NULL here defensively)
    AnsiString PathNameUploadFileToHost_ForDirectoryFile;  // golden :503 (golden never inits -- "" here defensively)
    AnsiString MultiFileUploadFileToHost_ForDirectoryFile; // golden :504 (golden never inits -- "" here defensively)
    AnsiString FileNameUploadFileToHost_ForDirectoryFile;  // golden :505 (golden never inits -- "" here defensively)
    int iStoreCTUploadFileToHost_ForDirectoryFile;         // golden :506 (golden never inits -- 0 here defensively)
    int iTotalCountUploadFileToHost_ForDirectoryFile;      // golden :507 (golden never inits -- 0 here defensively)

    // PtrUploadFileToHost_ForSingleFile (golden .h:523) -- IMPORTANT: despite
    // the "Ptr" name this is a genuinely EMBEDDED fixed-size char array in
    // golden (`char PtrUploadFileToHost_ForSingleFile[256*256*256];`, a real
    // 16MB member of THGem, not a heap pointer). FLAGGED DEVIATION (found +
    // fixed 20260727, post-hoc): this wave originally kept it as a literal
    // embedded array, reasoning "a THGem is long-lived/singleton in both
    // golden and this port" -- that assumption is FALSE for this port's own
    // test suite: tests/test_uHGemEquipment.cpp alone stack-allocates
    // `THGem g;` (or gem/gemWriter/gemReader) as a plain automatic-storage
    // local in 110+ places (an established convention predating this wave),
    // and tests/test_uHGemClass.cpp does the same once (`THGem realThgem;`).
    // An embedded 16MB member makes EVERY one of those stack frames need
    // 16MB+, which blows the default thread stack (MinGW default ~1-8MB)
    // instantly -- this is what caused ctest's "uHGemClass"/"uHGemEquipment"
    // SegFault (really STATUS_STACK_OVERFLOW, confirmed via direct exe
    // invocation) after this wave landed. Converting the WHOLE test suite's
    // 110+ stack-locals to heap allocation instead would be a far larger,
    // riskier diff against already-reviewed, already-passing test code for
    // no behavioral gain. So this member itself is ported as an
    // equivalent-capacity HEAP buffer (allocated in the ctor init-list,
    // `delete[]`d in the dtor) instead of an embedded array -- same capacity,
    // same raw-byte-buffer usage, same read/write call sites (both existing
    // call sites already just pass the bare pointer, see .cpp), only the
    // storage duration changes. This buffer is only ever used as a raw byte
    // buffer: every read of it (DataItemOut's BINARY_TYPE copy of exactly
    // iReadSize bytes) is preceded, in the very same case, by an fread() of
    // that SAME iReadSize byte count (case 200 in every one of the 3
    // sub-functions) -- so leaving its contents uninitialized introduces no
    // out-of-bounds/UB read in normal use, matching golden's own real member
    // layout (BCB6 does not zero-init raw member arrays for heap-allocated
    // objects either).
    char *PtrUploadFileToHost_ForSingleFile;   // golden :523 (golden: embedded char[256*256*256]; ported as heap buffer, see comment above)

    // ForMultiFileFileName (golden .h:622) -- the "current file being sent"
    // scratch AnsiString shared by DoUploadFileToHost_ForMultiFile's own
    // case 120/140 (golden .cpp:4377/4398, this wave).
    AnsiString ForMultiFileFileName;                       // golden :622 (golden never inits -- "" here defensively)

    // ==== CEID / Report StringGrid-backed "database" family =================
    void SetCEIDContent(unsigned iCeid, AnsiString CeidAlias, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    void SetCEIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    unsigned GetCEIDContent(unsigned iCeid, unsigned *iReportIDData, int Mode);
    bool SetReportIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    bool DeleteReportID(unsigned iCeid, int Type);
    bool DeleteReportIDOfCeid(unsigned iCeid);
    bool DeleteAllHostDefineReportID();
    unsigned GetReportIDContent(unsigned iCeid, unsigned *iReportIDData, int Mode);
    bool IsEnableEvent(unsigned iDataID, unsigned iCeid);
    bool CheckCeidExist(AnsiString Ceid);
    int GetReportIDType(AnsiString ReportID);
    bool CheckCeidAlreadyDefine(AnsiString Ceid);
    bool CheckReportIDExist(AnsiString ReportID);
    void AddCeidReportID(AnsiString Ceid, AnsiString ReportID);
    void DeleteAllHostReportIDOfCeid(unsigned ReportID);
    void DeleteHostReportIDOfAssignCeid(AnsiString Ceid);
    void DeleteAllHostDefineCeid();
    bool CheckReportAlreadyDefine(AnsiString ReportID);
    bool CheckCEIDExist(AnsiString sID);   // upper-case-CEID sibling of CheckCeidExist (golden :8051)

    // ==== Report/Link Acknowledge composers (AI(W906-AlarmReportAck) 20260721)
    // golden uHGemEquipment.cpp:7838-8026 -- pure InitLocalHead/DataItemOut/
    // SendLocalData composition (see those forwarders below) + already-real
    // CheckCeidExist/CheckCeidAlreadyDefine/CheckReportIDExist/
    // AddCeidReportID/DeleteHostReportIDOfAssignCeid/SaveEventReportData
    // (above); no VCL dependency. ProcessHostSendReportLinkID drains
    // slTempCeID/lTempCeIDContent (above); consumed by
    // HTGem::S2F36_LinkEventReportAcknowledge (uHGemClass.cpp). The
    // ReportAcknowledge/ReportLinkAcknowledgeError families are consumed by
    // HTGem::S2F34_ProcessHostSendReportID/S2F34_DefineReportAcknowledge/
    // S2F36_LinkEventReportAcknowledgeSub (ditto).
    void ProcessHostSendReportLinkID();                                    // golden :7838-7898
    void ReportAcknowledge(unsigned char Code);                            // golden :7969-7974 (S2F34 DRACK composer)
    void DefineReportAcknowledgeAccept();                                  // golden :7976-7979
    void DefineReportAcknowledgeInsufficientSpace();                       // golden :7981-7984
    void DefineReportAcknowledgeFormatError();                             // golden :7986-7989
    void DefineReportAcknowledgeAlreadyDefined();                          // golden :7991-7994
    void DefineReportAcknowledgeInvalidSVID();                             // golden :7996-7999
    void ReportLinkAcknowledgeError(unsigned char ErrCode);                // golden :8001-8006 (S2F36 LRACK composer)
    void LinkReportAcknowledgeFormatError();                               // golden :8008-8011
    void LinkReportAcknowledgeAlreadyDefined();                            // golden :8013-8016
    void LinkReportAcknowledgeInvalidCeID();                               // golden :8018-8021
    void LinkReportAcknowledgeInvalidReportID();                           // golden :8023-8026

    // golden's tail call inside EnableDisableEventReport (golden
    // uHGemEquipment.cpp:8034-8039). AI(W906-AlarmReportAck) 20260721: no
    // longer a stub -- now a REAL forwarder (InitLocalHead/DataItemOut/
    // SendLocalData, see .cpp), now that THGem has its own InitLocalHead/
    // DataItemOut (above). Replaces the earlier no-op stub this wave closed
    // the gap for.
    void EnableDisableEventReportAcknowledgeError(unsigned char ErrCode);
    void EnableDisableEventReportAcknowledgeCeidNotExist();                 // golden :8041-8044
    void EnableDisableEventReportAcknowledgeFormatError();                  // golden :8046-8049
    void EnableDisableEventReport(bool CEED, int slen, unsigned *CEID);

    void ReadEventReportData();
    void SaveEventReportData();

    // ==== generic StringGrid <-> TStringList tab-format (de)serialization ===
    void CopyStringGridAsTabFormat(TStringGrid *strGrd, TStringList *memoPtr);
    void PasteStringGridAsTabFormat(TStringGrid *strGrd, TStringList *memoPtr);

    // ==== Alarm-grid siblings ===============================================
    void SetAlamData(int iRowCount, AnsiString ALID, AnsiString Class, AnsiString ALTX, AnsiString Position);
    void ReadAlamData();
    void WriteAlamData();

    // AI(W906-AlarmReportAck) 20260721: golden uHGemEquipment.cpp:3337-3372 --
    // pure strGrdAlarm StringGrid composition (+ WriteAlamData above); zero
    // new dependency. Consumed by HTGem::S5F4_EnableDisableAlarmAcknowledge/
    // S5F6_ListAlarmData (uHGemClass.cpp, this same wave).
    bool EnableDisableAlarm(AnsiString S, unsigned char T);       // golden :3337-3352
    void EnableDisableAlarmAll(unsigned char T);                  // golden :3354-3364
    int GetAlarmIndex(AnsiString S);                              // golden :3366-3372

    // AI(W906-uHGemClass-Micro5) 20260721: EC-side siblings of the Alarm
    // family above -- golden uHGemEquipment.cpp:9257-9302, a mechanical
    // structural clone of WriteAlamData/EnableDisableAlarm/
    // EnableDisableAlarmAll, just against sgSECSECData instead of
    // strGrdAlarm (see that member's own comment above). golden's remaining
    // 3 EC-file siblings -- ReadECEnableData/GetECEnableData/SetECEnableData,
    // uHGemEquipment.cpp:9226-9253/9306-9334 -- are NOT added: no in-scope
    // caller needs them (only HTGem::S125F2_EnableDisableECDataAcknowledge,
    // this same wave, calls EnableDisableECData/EnableDisableECDataAll).
    void WriteECEnableData();                                      // golden :9257-9269
    bool EnableDisableECData(AnsiString ID, unsigned char T);      // golden :9273-9288
    void EnableDisableECDataAll(unsigned char T);                  // golden :9292-9302

    // ==== SV/EC DataItem family (W906-SvEcDataItem 20260720) ================
    // golden uHGemEquipment.cpp:2472-3336 (SvEcReg-backed) + :7623-7688
    // (Report/CEID composers, pure WireCodec+SV composition). See .cpp for
    // the golden-VCL-branch gate note (design doc D2) -- AI(W906-VCW1)
    // 20260721: that note's dynamic_cast branch is UN-GATED as of this wave
    // (vclcompat/Controls.h); see the .cpp note's own updated text.
    bool DataItemOutSV(AnsiString SVID);                          // golden :2472-2761
    bool DataItemOutSVNameList(AnsiString SVID);                  // golden :2765-2801
    bool DataItemOutSVNameListWithValue(AnsiString SVID);         // golden :2803-2840 (S103F11/F12)
    void DataItemOutEC(AnsiString ECID);                          // golden :2845-3112
    bool IsValidSVID(AnsiString SVID);                            // golden :3114-3122
    void DataItemOutECNameList(AnsiString ECID);                  // golden :3127-3332
    void FormCreate(TObject *Sender);                             // golden :6165-6207 (__fastcall dropped, project convention)

    void SendRepoerID(unsigned iReportID);                        // golden :7623-7632 (__fastcall dropped)
    void SendAnnotatedRepoerID(unsigned iReportID);                // golden :7648-7662 (__fastcall dropped)
    void SendCeid(unsigned iCeid);                                 // golden :7664-7675 (__fastcall dropped)
    void SendAnnotatedCeid(unsigned iCeid);                        // golden :7677-7688 (__fastcall dropped)

    // ==== TCP/IP connection lifecycle methods (W906-uHGemEquipment-ConnLifecycle) ====
    void __fastcall clientGemConnect(TObject *Sender, TCustomWinSocket *Socket);        // golden :2100-2104
    void __fastcall clientGemDisconnect(TObject *Sender, TCustomWinSocket *Socket);      // golden :2108-2116
    void __fastcall clientGemError(TObject *Sender, TCustomWinSocket *Socket,
                                    TErrorEvent ErrorEvent, int &ErrorCode);              // golden :2120-2134
    void __fastcall clientGemConnecting(TObject *Sender, TCustomWinSocket *Socket);      // golden :2138-2142

    bool DoOpenCommuncation();       // golden :3382-3490
    void OnlineLocalOrRemote();      // golden :3604-3621 (mostly inert -- see .cpp)
    bool DoOnLine();                 // golden :3626-3645

    bool CheckSocketActiveFalse();   // golden :5146-5159

    void CloseCommuncation();                              // golden :5548-5552
    void Connect();                                        // golden :5557-5563
    void DisConnect();                                      // golden :5567-5572
    bool IsConnect();                                       // golden :5576-5579
    void SetEstablishCommunicationsTryCount(int ct);        // golden :5583-5586
    void OnLine(bool Mode);                                 // golden :5592-5598
    void OnLineLocal();                                     // golden :5602-5606
    void OnLineRemote();                                    // golden :5610-5614
    void OffLine();                                         // golden :5619-5624
    bool IsOnLine();                                        // golden :5628-5631
    bool GetOnLineMode();                                   // golden :5635-5638
    void SetCanAcceptHostOnLineRequest(bool flag);          // golden :5642-5644 (truly empty body)

    void __fastcall srvGemClientConnect(TObject *Sender, TCustomWinSocket *Socket);      // golden :6812-6837
    AnsiString __fastcall GetSocketErrorMsg(TObject *Sender, int iErrCode);               // golden :6842-6845
    void __fastcall srvGemClientError(TObject *Sender, TCustomWinSocket *Socket,
                                       TErrorEvent ErrorEvent, int &ErrorCode);           // golden :6850-6874
    void __fastcall srvGemClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);    // golden :6897-6910

    void __fastcall StringOut(AnsiString S);          // golden :392-396
    void __fastcall StringBinaryOut(AnsiString S);    // golden :401-404 (truly commented-out body)

    void __fastcall SaveSECSGEMErrToLog(AnsiString asSaveStr);   // golden :422-439

    void __fastcall ClearDefaultEvenReport();   // golden :4988-5008

    void GetTimeInfo();   // golden :315-348 (TimeString/GemClock half only -- see .cpp)

    // ==== Widget-persisted state + status refresh (W906-uHGemEquipment-BucketB) ====
    void __fastcall InitialHGem();        // golden :5011-5099
    void __fastcall SaveSystemDefault();  // golden :5101-5140
    void DoUpdateStatus();                // golden :4747-4985
    void __fastcall ProcessShow();        // golden :5165-5173
    void FormClose();                     // golden :6887-6892 (Sender/TCloseAction& dropped -- see .cpp)
    void __fastcall ManualCreatergRoleClick(TObject *Sender);   // golden :6915-6931
    void __fastcall FormShow(TObject *Sender);                  // golden :6936-6950 (partial -- see .cpp)
    void __fastcall StringOut(AnsiString S, TColor C);          // golden :409-417

    // AI(W906-uHGemEquipment-BucketC) 20260717: golden uHGemEquipment.h:249
    // `void DoSeparate();` -- REAL as of this wave (REPLACES the Bucket-B
    // gated no-op stub that used to sit here -- golden's real body is
    // wire-codec/SML, which needed WireCodec embedded first; see
    // uHGemEquipment.cpp's own DoSeparate definition, golden :3518-3531).
    // DoSelect is golden's sibling (golden :248, :3496-3509) -- was never
    // declared at all before this wave (no stub existed for it).
    void DoSelect();      // golden :3496-3509
    void DoSeparate();    // golden :3518-3531 (now real -- see above)

    // ==== Bucket C: socket receive pump / T3 timeout / HSMS control-message
    // ==== handshake / Timer1Timer master state machine (W906-uHGemEquipment-
    // ==== BucketC 20260717) ==================================================
    void __fastcall clientGemRead(TObject *Sender, TCustomWinSocket *Socket);   // golden :9008-9028
    void __fastcall ProcessSocketReceiveData();                                // golden :9030-9207
    void __fastcall Timer1Timer(TObject *Sender);                              // golden :5176-5527

    int  DoConnect();                          // golden :3536-3599
    void __fastcall DoProcessSFNoResponse();   // golden :4604-4694
    void DoLocalAllProcessLoop();              // golden :4699-4741 (shell; AI(W906-trace) 20260804: was "4 callees gated" -- DoTraceDataResponse is now REAL, so at most 2 remain)

    // AI(W906-SysModWire) 20260720: SetMachineTypeAndSoftwarseVer/
    // CheckSFFormatOnlyHead -- both prerequisites for un-gating uHGemClass.cpp's
    // S1F1/S1F2/S1F13/S1F14/Process_S1F14/S1F16/S1F18/S2F18 (see that file's own
    // un-gating notes); also consumed directly by ProcessReceiceData's tail.
    void __fastcall SetMachineTypeAndSoftwarseVer(AnsiString Mdln, AnsiString SoftVer); // golden :6510-6514 ("Softwarse" misspelling preserved verbatim)
    bool CheckSFFormatOnlyHead(AnsiString ErrStr);   // golden :8679-8693 (no __fastcall, matches golden)

    // golden's private "S,F 主要處理" block (uHGemEquipment.h:292-295) --
    // SelectRsp/DeselectRsp/LinktestRsp are pure WireCodec composition (REAL);
    // ProcessReceiceData is a SPLIT -- the HSMS control-message head is real,
    // the S,F data-message dispatch tail stays gated -- see the .cpp's own
    // comment on ProcessReceiceData for the full rationale.
    void LinktestRsp();          // golden :8751-8765
    void SelectRsp();            // golden :8707-8723
    void DeselectRsp();          // golden :8729-8745
    void ProcessReceiceData();   // golden :8772-8989 (control head REAL; S,F tail GATED, see .cpp)

    // AI(W906-AlarmReportAck) 20260721: THGem-level InitLocalHead/DataItemOut
    // forwarders (golden .h:314-315) -- same one-line-forwarder idiom as
    // SendLocalData() below, targeting THGem's own embedded WireCodec
    // directly (`this->WireCodec` / bare `WireCodec`, NEVER `ActiveWire` --
    // that pointer belongs to HTGem, a DIFFERENT class, see this file's own
    // D1/D2 notes above). This wave's new wire composers below
    // (ReportAcknowledge/ReportLinkAcknowledgeError/
    // EnableDisableEventReportAcknowledgeError/ProcessHostSendReportLinkID)
    // call these bare (implicit `this->`, matching golden). Only the ONE
    // DataItemOut overload those composers actually call (the pointer form)
    // is added -- golden's sibling `DataItemOut(unsigned char, AnsiString)`
    // overload has no in-scope THGem-level caller today (verified by reading
    // every new method's body), so it is not added speculatively.
    //
    // AI(W906-DoDownLoadRemoteFile) 20260721: the deferral above no longer
    // holds -- DoDownLoadRemoteFile (below) calls the AnsiString-form
    // overload bare, matching golden (uHGemEquipment.cpp:6762). Added now,
    // same one-line-forwarder idiom as its pointer-form sibling.
    void InitLocalHead(int SCode, int FCode, int WBit);        // golden .h:314
    void DataItemOut(int len, unsigned char Type, void *P);    // golden .h:315
    void DataItemOut(unsigned char Type, AnsiString S);        // golden .h:316

    void SendLocalData();   // golden .h:318, uHGemEquipment.cpp:1985-2107 (now real -- forwards to SendLocalDataFrom(WireCodec))
    // AI(W906-uHGemEquipment-BucketC) 20260717: ADDITIVE, not a golden method
    // (D2) -- the REAL golden SendLocalData(1985-2107) body, parameterized
    // over WHICH SecsWireCodec instance to send (THGem's own WireCodec, or --
    // once a future wiring wave installs the identical hook on
    // HSys.MyGem->WireCodec -- HTGem's). See the .cpp definition for the full
    // golden-to-parameterized substitution table.
    void SendLocalDataFrom(SecsWireCodec &wc);

    void __fastcall CheckSFCodeResponse();       // golden :7020-7058
    void __fastcall SaveSECSGEMTextToLog();      // golden :7068-7091
    void InitSTypeStruct();                      // golden :376-387

    // golden's zero-arg signatures (uHGemEquipment.h:263-264) forward to
    // `(WireCodec)` below; the `(SecsWireCodec&)` overloads are ADDITIVE (not
    // golden) so SendLocalDataFrom can trace the INVOKING codec's own buffer
    // (needed once HTGem-originated sends are wired through the same hook).
    void ShowLocalHeadInfo();                    // golden :1378-1405 (forwards to ShowLocalHeadInfo(WireCodec))
    void ShowLocalHeadInfo(SecsWireCodec &wc);        // ADDITIVE (real body)
    void ShowLocalBufferBinaryData();            // golden :1350-1373 (forwards to ShowLocalBufferBinaryData(WireCodec))
    void ShowLocalBufferBinaryData(SecsWireCodec &wc); // ADDITIVE (real body)

    // AI(W906-SpoolCluster) 20260721: DoSpool UN-GATED for real this wave --
    // moved OUT of the "Bucket C: gated no-op stubs" group below (it now has
    // a real spool-file surface: vclcompat::TFileListBox, see this file's own
    // Spool-cluster member-comment above). Real bodies for all 5 live in the
    // .cpp -- see each one's own comment there for the exact golden citation.
    void DoSpool();                                // golden :4079-4189
    void WriteToSpoolFile();                       // golden :1887-1977
    int DoSpoolSendLocalData(unsigned char *Ptr);  // golden :4025-4074
    void SetSpoolActive(bool Active);              // golden :6133-6154
    bool GetSpoolActive();                         // golden :6158-6161

    // ==== Bucket C: gated no-op stubs (golden citation each; no in-scope
    // ==== consumer needs a real body -- same idiom as
    // ==== EnableDisableEventReportAcknowledgeError above) ====================
    // AI(W906-uHGemClass-TraceUnlock) 20260728: refreshed status -- the
    // TraceData[]/TraceDataResponseTask[]/bTraceData[]/TraceDataResponseDelay[]/
    // iTRID[]/DSPER[]/iTOTSMP[]/iREPGSZ[]/iTOTSMP_Count[] member cluster this
    // TODO used to cite as missing now EXISTS (added this wave, see the
    // "Trace cluster" member comment above, for HTGem::
    // S2F24_TraceInitializeAcknowledgeSub's sake -- NOT for this method,
    // deliberately out of THIS wave's scope). GetTimeInfo()/iTimeFormat/
    // SystemYear.../DataItemOutSV (golden's other reads, uHGemEquipment.cpp:
    // 4217-4230) are ALSO already real (earlier waves). A future wave un-
    // gating this method specifically should therefore need NO new member
    // additions -- just the real body (golden :4190-4242, a Task==1/100
    // one-shot-per-poll retry state machine) translated in .cpp, following
    // the same InitLocalHead/DataItemOut/SendLocalData -> ActiveWire-
    // equivalent mechanical-rename rule this file's siblings already use
    // (NOTE: THGem's own methods call these directly as bare members, not
    // through an ActiveWire-style indirection -- confirm the exact call
    // shape against THIS file's OWN established convention, not uHGemClass.cpp's,
    // before assuming a 1:1 copy).
    // AI(W906-trace) 20260804: NO LONGER A BUCKET-C STUB -- golden's real body is
    // translated at .cpp:4640-4693, byte-exact bar one `// fallthrough` comment.
    // Left in place rather than physically moved so declaration order stays stable;
    // the Bucket C banner above no longer applies to this one.  The .cpp names the
    // param `iIndex` per golden; golden's own .h:275 leaves it unnamed.
    void DoTraceDataResponse(int TR);  // golden :4190-4242 -- REAL BODY, see .cpp:4640

    // AI(W906-DoDownLoadRemoteFile) 20260721: UN-GATED (was a stub in this
    // same cluster) -- real body added, see .cpp. golden :6746-6807. Pure
    // RequestRemoteDownLoad/InitLocalHead/DataItemOut/SendLocalData retry
    // state machine, no VCL widget dependency (unlike its sibling
    // DoUploadFileToHost family immediately below, UN-GATED separately this
    // wave -- see its own comment).
    void DoDownLoadRemoteFile();       // golden :6746-6807

    // AI(W906-UploadFamily) 20260723: DoUploadFileToHost family -- UN-GATED for
    // real this wave (was the "#if 0 TODO(W906-SECSGEM-upload)" gated no-op
    // stub immediately above DoDownLoadRemoteFile, until now). golden
    // .cpp:4249-4344 (_ForSingleFile), :4350-4471 (_ForMultiFile), :4477-4586
    // (_ForDirectoryFile), :4591-4599 (DoUploadFileToHost() itself, the
    // 3-way dispatcher on SV_70_UNT1_ReceipeStruct). Real bodies for all 4
    // live in the .cpp, each with its own golden citation and, for
    // _ForSingleFile, a PRESERVED GOLDEN BUG note (see that member's own
    // header comment above, iUploadFileToHost_ForSingleFileMaxSend).
    void DoUploadFileToHost_ForSingleFile();     // golden :4249-4344
    void DoUploadFileToHost_ForMultiFile();      // golden :4350-4471
    void DoUploadFileToHost_ForDirectoryFile();  // golden :4477-4586
    void DoUploadFileToHost();                   // golden :4591-4599 (dispatcher)


    // AI(W906-FW-GEM-W8) 20260826: 本波方法用到的 golden 成員。
    bool bALEDflag = false;                       // golden uHGemEquipment.h:469
    Word iReportECDataChangeCheckOldSecond = 0;   // golden :480
    // EC_ID / EC_OldValue 是 EC 變動偵測的兩條平行清單：DoReportECChange 用
    // iIndex 同時索引兩者，比對 GetECDataValue(EC_ID[i]) 與 EC_OldValue[i]。
    // golden 由 out-of-scope 的 FormCreate SV/EC 註冊填充（見檔頭 :161），
    // 本樹保持空清單 -> DoReportECChange 的迴圈不會有可索引的項目。
    TStringList *EC_ID      = new TStringList();   // golden :656
    TStringList *EC_OldValue = new TStringList();  // golden :670
    // ========================================================================
    // AI(W906-FW-GEM-W8) 20260826: EC-change / EC-enable / Terminal / Alarm
    // 小家族 12 支。簽章由定義生成（golden 的 __fastcall 已剝除）。
    // SetTerminalWindows/SetTerminalWindows2 是 gated stub，理由見 .cpp。
    // ========================================================================

    // golden uHGemEquipment.h:627-629（pig 2014.04.23 KYEC SECS）。
    // DoReportECChange 每次偵測到 EC 值變動就往這三串各 Add 一筆，
    // ReplyECDataChange 讀完之後清空。三者長度必須同步。
    TStringList *SECSReportIDChange = new TStringList();   // golden :627
    TStringList *SECSOriginalValue  = new TStringList();   // golden :628
    TStringList *SECSNewECValue     = new TStringList();   // golden :629

    void InitHType();   // golden :353-370
    AnsiString ReadALED(AnsiString ALID);   // golden :824-835
    void ReplyECDataChange();   // golden :3927-3942
    void DoReportECDataChangeCheck();   // golden :3947-3985
    int DoReportECChange(int iIndex);   // golden :3989-4018
    void SetTerminalWindows(TObject *Ptr);   // golden :6040-6067
    void SetTerminalWindows2(TObject *Ptr);   // golden :6069-6096
    void TerminalRequest(AnsiString S);   // golden :6100-6129
    bool CheckNeedReportAlarm(AnsiString S);   // golden :6371-6384
    bool ReadECEnableData();   // golden :9226-9253
    bool GetECEnableData(AnsiString ID);   // golden :9306-9319
    void SetECEnableData(AnsiString ECID,AnsiString Function);

    // AI(W906-FW-GEM-W9) 20260826: 退役 GATE (W8-ECEvent) 用。
    void EventReport(unsigned iDataID, unsigned iCeid);   // golden :7703-7761   // golden :9323-9334

    // ======================================================================
    // AI(W906-FW-GEM-W10) 20260826: Alarm 回報 / 連線與終端按鈕 / SV/EC 查詢，19 支（原排 20，GemTerminalSendEditKeyDown 因 TShiftState 退出，理由見 .cpp）。
    // 簽章由定義生成（golden 的 __fastcall 已剝除；跨行簽章壓成一行）。
    // ======================================================================

    // AI(W906-FW-GEM-W10) 20260826: 本波方法用到的 golden 成員。
    bool bReportSECS_GEM_Message = false;         // golden uHGemEquipment.h:455
    int  iReturnCode = 0;                         // golden :453
    THGemEdit *GemTerminalSendEdit = new THGemEdit();   // golden :403（golden 是 TEdit*）
    TStringList *SReceiveData = new TStringList();      // golden :672

    // golden uHGemEquipment.h:654。GetAllSVInformation 逐筆讀它填 SV 說明欄。
    // SV_ID/SV_TYPE/SV_NAME/SV_UNIT/SV_Remark 是五條平行清單，GetAllSVInformation
    // 用同一個 Index 逐筆取出來組成一行 SV 說明。golden 由 out-of-scope 的
    // FormCreate SV/EC 註冊填充（見檔頭 :161），本樹保持空清單 -> 迴圈不會有項目。
    TStringList *SV_ID     = new TStringList();   // golden :647
    TStringList *SV_TYPE   = new TStringList();   // golden :648
    TStringList *SV_NAME   = new TStringList();   // golden :649
    TStringList *SV_UNIT   = new TStringList();   // golden :650
    TStringList *SV_Remark = new TStringList();   // golden :654

    void ReportAlarm(AnsiString AlarmCode, bool bIsJam, int iDuplicateError, AnsiString SubMessage, bool bReleaseAlm, AnsiString sRecovery);   // golden :6276-6369
    void ReportAlarmWithMessage(AnsiString ALID, AnsiString ALTX, AnsiString sHappenedTime, AnsiString sProcessKey, bool SameCase);   // golden :6386-6420
    void LocalAcknowledge(unsigned char SCode, unsigned char FCode , unsigned char Command);   // golden :2207-2214
    void SetTimeFormat(int Format);   // golden :6033-6036
    int SetReceipeDirectoryAndGlobalName(AnsiString Path, AnsiString FileMask, int Type);   // golden :6427-6489
    void GemBtnSendTerminalMessageClick(TObject *Sender);
    //AI(W906-FW-SIG-W18) 20260826: W10/W12 排除它的理由（TShiftState 無 port）已消失。
    void GemTerminalSendEditKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);   // golden :6501-6506   // golden :6493-6497
    void BtnEnableCommClick(TObject *Sender);   // golden :6518-6521
    void BtnDisableCommClick(TObject *Sender);   // golden :6525-6528
    void GemBtnOnlineRequestClick(TObject *Sender);   // golden :6532-6535
    void GemBtnOfflineRequestClick(TObject *Sender);   // golden :6539-6542
    void GemBtnOnlineRemoteClick(TObject *Sender);   // golden :6546-6549
    void GemBtnOnlineLocalClick(TObject *Sender);   // golden :6553-6556
    int GetDataItemLenAndTypeAndDelete(int &len, unsigned char &Type);   // golden :7099-7106
    void SetDefaultAddressAndPort(AnsiString Address, AnsiString Port, AnsiString DeviceID);   // golden :7304-7310
    bool CheckSFFormatDataRequest(AnsiString ErrString);   // golden :7312-7351
    void SendInvalidDataMessageToHost(AnsiString S);   // golden :7353-7358
    void GetAllSVInformation(THGemMemo *Ptr);   // golden 是 TMemo*，本樹的對應 stand-in 是 THGemMemo   // golden :8225-8268
    void GetALLECInformation(TObject * Ptr);

    // AI(W906-FW-GEM-W11) 20260826: 退役 GATE (W10-ECInfo) 用。
    void GetECInformation(TObject * Ptr, int Index);   // golden :8278-8597
    // golden uHGemEquipment.h（EC_* 平行清單的說明欄）。
    // EC_* 平行清單：GetECInformation 用同一個 Index 逐筆取出來組一行 EC 說明。
    // golden uHGemEquipment.h:657-668。三條 TList 存的是「指向真實 EC 變數的指標」，
    // 由 out-of-scope 的 FormCreate SV/EC 註冊填入（見檔頭 :161），本樹保持空，
    // 所以 GetECInformation 讀到的 Items[i] 會是 NULL，走 golden 自己的 NULL 分支。
    TStringList *EC_TYPE   = new TStringList();          // golden :657
    TStringList *EC_NAME   = new TStringList();          // golden :658
    TStringList *EC_UNIT   = new TStringList();          // golden :659
    // 三條 TList 一律全限定成 vclcompat::TList，照本檔 :644-649 已訂的慣例：
    // aHotPlateSubstrate.h 有另一個**無關的**全域 class TList，這個 header
    // 刻意不把 vclcompat::TList 帶進全域命名空間，避免選錯型別。
    vclcompat::TList *EC_Ptr_Min     = new vclcompat::TList();   // golden :661
    vclcompat::TList *EC_Ptr_Max     = new vclcompat::TList();   // golden :662
    vclcompat::TList *EC_Ptr_Default = new vclcompat::TList();   // golden :663
    TStringList *EC_Ptr_Min_Value     = new TStringList();   // golden :666
    TStringList *EC_Ptr_Max_Value     = new TStringList();   // golden :667
    TStringList *EC_Ptr_Default_Value = new TStringList();   // golden :668
    TStringList *EC_Remark = new TStringList();   // golden :8270-8276
    void GemRemoteReceipeListClick(TObject *Sender);   // golden :9210-9220

    // ======================================================================
    // AI(W906-FW-GEM-W12) 20260826: 檔案傳輸 UI 家族 + 三支雜項，12 支。
    // 本檔可翻譯表面的收尾波；未翻的 3 支理由見 .cpp 的 banner。
    // ======================================================================

    void SetDisplayPtr(THGemMemo *DispBox);   // golden 是 TMemo*；DB 成員是 THGemMemo*   // golden :2093-2096
    void GemListRemoteFileNameClick(TObject *Sender);   // golden :6560-6577
    void GemDownLoadRemoteFileNameClick(TObject *Sender);   // golden :6583-6606
    void GemDeleteRemoteFileNameClick(TObject *Sender);   // golden :6612-6614
    void GemUpLoadLocalFileNameClick(TObject *Sender);   // golden :6620-6627
    void GemRefreshLocalFileClick(TObject *Sender);   // golden :6633-6691
    void GemSelectAllRemoteFileClick(TObject *Sender);   // golden :6697-6703
    void GemDisSelectAllRemoteFileClick(TObject *Sender);   // golden :6709-6715
    void GemSelectAllLocalFileClick(TObject *Sender);   // golden :6721-6727
    void GemDisSelectAllLocalFileClick(TObject *Sender);   // golden :6733-6739
    void GemSBSetupClick(TObject *Sender);   // golden :6879-6882
    void edtT3TimeOutClick(TObject *Sender);   // golden :9341-9344
};

//---------------------------------------------------------------------------
//  SplitStrByTabOnly -- file-scope helper (golden is a bare global function,
//  NOT a THGem method, and is NOT declared in golden's own uHGemEquipment.h
//  either -- golden's .cpp defines it (line 8086) strictly before its one
//  call site inside PasteStringGridAsTabFormat (line 8182), so no forward
//  declaration is needed there. Declared here (a header-level addition
//  beyond golden's own header) purely so this wave's test harness can
//  exercise it directly -- no behavior change, source-compatible.
//---------------------------------------------------------------------------
bool SplitStrByTabOnly(char *str, char *dest, int Max);

//---------------------------------------------------------------------------
//  AI(W906-uHGemEquipment-BucketB) 20260717: HGem -- golden's own global
//  singleton pointer (`extern PACKAGE THGem *HGem;`, golden uHGemEquipment.h's
//  own tail declaration, right before `extern struct HTypeStruct HType;`).
//  ProcessShow's own golden body (uHGemEquipment.cpp:5165-5173) dereferences
//  `HGem->WaitShowString`/`HGem->DB` rather than an implicit `this->` --
//  preserved verbatim (see ProcessShow's own .cpp comment) rather than
//  "simplified" to `this->`, so a caller/test MUST set `HGem = &instance;`
//  before calling ProcessShow (and DoUpdateStatus's own KYEC branch, which
//  ALSO dereferences `HGem->srvGem`/`HGem->clientGem`) for either to behave
//  sensibly -- exactly the same "must be wired externally" category already
//  established by the THGemPanel*/THGemSpeedButton* members above. Defaults
//  NULL here (golden itself never explicitly initializes this file-scope
//  global; C++ static-storage globals zero-init by default regardless, so
//  no deviation-flag needed, unlike THGem's own instance-field members).
//---------------------------------------------------------------------------
extern THGem *HGem;

#endif // uHGemEquipmentH
