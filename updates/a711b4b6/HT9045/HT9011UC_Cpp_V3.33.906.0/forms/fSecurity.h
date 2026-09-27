// =============================================================================
//  forms/fSecurity.h  --  non-VCL facade for golden's TfSecurity (cSecurity.h)
//
//  AI(W906-FW-SecCC) 20260819: new file, FW-3 queue item "FW-SecCC".
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cSecurity.h (159 lines) +
//  cSecurity.cpp (1,772 lines, cp950/Big5 -- decoded with `python3 -c
//  "open(path,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfSecurity is golden's per-function access-level dialog AND the Alarm/Jam
//  Code editor (level, red-highlight, silent, unlock-password, per-customer
//  extras) that lives on the same form (PageControl1's tsJamCode tab). Two
//  independent jobs share one class:
//    (1) Insufficient(iType)/GetBit8(...) -- read-only permission/Jam-status
//        oracles other forms call constantly (5 + 1 known waiting call sites,
//        see CONSUMER REGISTER).
//    (2) The Jam-code-editor widgets + their Get*/Save* family, driven by
//        cbJamArea/cbJamCode/cbJamLang and persisted to a PER-FORM ini file
//        (FileNameJam000, under D:\HT9045\Error\, NOT the shared production
//        config tree -- see WRITE-PATH NOTES).
//  The ~180-entry `mySecurityPal` vector (job (1)'s DATA SOURCE, i.e. what
//  Insufficient() actually indexes) is populated in the ctor from ~100
//  `fMain->sbXXX->Glyph` TSpeedButton pointers that do not exist on this
//  tree's TfMain facade -- see GATE (SEC1). Until a future wave grows fMain
//  that far, `mySecurityPal` stays empty and `iMaxLevelItem` stays 0; see
//  CTOR SAFETY for what that does to Insufficient()'s answers TODAY.
//
//  FORM CLASS SHAPE: plain non-VCL class, NOT TForm-derived -- same posture
//  as forms/fContactCT.h / forms/fShowBinSelect.h / forms/fObserver.h
//  (`grep -n "^class Tf" forms/*.h` -- every existing translated form facade
//  in this tree follows this shape; none re-derive TForm).
//
//  TRANSLATION STATUS (every golden method, golden line span)
//  --------------------------------------------------------------------------
//    TfSecurity(TComponent*)   golden :23-239   MIXED -- GATE (SEC1) for the
//                                                mySecurityPal population
//                                                block (:26-213); the rest
//                                                (:215-238) FULL, wrapped in
//                                                an `if (INIFileGeneral!=0)`
//                                                SIOF guard (see CTOR SAFETY)
//    FormDestroy               golden :241-258  FULL (loop is a real 0-
//                                                iteration no-op today, see
//                                                CTOR SAFETY; DefaultImg
//                                                null-then-delete is golden
//                                                bug B11)
//    FormShow                  golden :260-436  FULL except GATE (SEC3-CALL,
//                                                folds into SecurityPalVisible
//                                                itself) and GATE (SEC-TAB)
//                                                for the ScrollBox/Controls[]
//                                                panel-arrange block (:382-414)
//    FormClose                 golden :438-468  FULL
//    SecurityPalVisible        golden :470-571  GATE (SEC3) -- entire body
//    Insufficient              golden :573-597  FULL
//    ChangePassword (free fn)  golden :599-811  GATE (SEC7) -- entire body
//    SecurityExitClick         golden :813-816  FULL, DEVIATION (Close() ->
//                                                FormClose(), see below)
//    TMySecurity::TMySecurity  golden :818-852  GATE (SEC10) -- entire body
//    TMySecurity::~TMySecurity golden :854-858  FULL (empty in golden too)
//    TMySecurity::SetParent    golden :860-902  GATE (SEC10) -- entire body
//    TMySecurity::SetPosition  golden :904-908  GATE (SEC10) -- entire body
//    TMySecurity::SetCaption   golden :910-920  GATE (SEC10) -- entire body
//    TMySecurity::GetCaption   golden :922-925  FULL
//    btnHonPrecClick           golden :927-933  FULL
//    sbSupervisorClick         golden :935-941  FULL
//    sbEngineerClick           golden :943-949  FULL
//    cbJamAreaChange           golden :951-962  FULL
//    cbJamCodeChange           golden :964-969  FULL
//    cbJamLangChange           golden :971-974  FULL
//    ChangeJamMessage          golden :976-1060 FULL except GATE (SEC5) for
//                                                the two `->Font->...` lines
//    SaveJamLevel              golden :1062-1202 FULL
//    GetJamLevel               golden :1204-1297 FULL
//    GetBit8                   golden :1299-1305 FULL
//    GetJemSilent              golden :1307-1316 FULL
//    GetJemUnlockPassWord      golden :1318-1327 FULL
//    GetJemIncludeMTBA         golden :1329-1358 FULL
//    GetJemContiAlarm          golden :1360-1369 FULL
//    GetO17ContiAlarm          golden :1371-1380 FULL
//    GetAddAlarmLog            golden :1382-1391 FULL
//    GetJemTCPAlarm            golden :1393-1401 FULL
//    GetAlarmAfterUnloaderFull golden :1403-1411 FULL
//    GetJamArea                golden :1413-1416 GATE (SEC4) -- entire body
//    GetJemRed                 golden :1418-1464 FULL
//    GetLevelSet               golden :1466-1501 FULL
//    SetLevelSet               golden :1503-1507 FULL
//    spbImportClick            golden :1509-1577 FULL (dead-but-compiled
//                                                inside `if(OpenDialog1->
//                                                Execute())`, see S13)
//    spbExportClick            golden :1579-1657 FULL (same S13 shape)
//    btnOperatorClick          golden :1659-1662 FULL
//    AddAlarmList              golden :1664-1676 FULL except GATE (SEC6) for
//                                                `->ColWidths[7]=50;`
//    ClearAllJamCount          golden :1678-1689 FULL
//    AddJamCount               golden :1691-1717 FULL
//    GetN27AlarmSel            golden :1719-1731 FULL
//    GetN27AddBoard            golden :1733-1742 FULL
//    GetContAlarmNotUpload     golden :1744-1752 FULL
//    GetPasswoard              golden :1754-1771 FULL except GATE (SEC9) for
//                                                the `bSIGURDFunction` block
//                                                (writes under system\)
//
//  DEVIATION -- FormClose/SecurityExitClick signature and Close()
//  --------------------------------------------------------------------------
//  Same posture as forms/fCounterClear.h: golden `void __fastcall FormClose
//  (TObject *Sender, TCloseAction &Action)` drops both params (TCloseAction
//  has no port anywhere in this tree). SecurityExitClick's golden body is
//  just `Close();` -> translated as `FormClose();` (ATCInterface.cpp:446/:513
//  precedent: "the ONLY observable effect golden's own FormClose has ... is
//  applied directly").
//
//  DEVIATION -- ctor drops the `TComponent* Owner` parameter
//  --------------------------------------------------------------------------
//  Not TComponent-derived (see FORM CLASS SHAPE above), and `Owner` is never
//  read inside golden's OWN ctor body for any purpose other than golden bug
//  B9's already-broken self-reference (see below) -- same trim precedent as
//  forms/fContactCT.h / forms/fShowBinSelect.h / forms/fCounterClear.h, all
//  of which drop the same `TComponent* Owner` parameter from their own
//  ctors for the identical reason.
//
//  CTOR SAFETY (the "SIOF homecoming" question)
//  --------------------------------------------------------------------------
//  Golden's ctor, walked statement group by statement group (line numbers
//  are golden's):
//    :26        `DefaultImg=fMain->sbSetting->Glyph;`             GATE (SEC1)
//    :28-206    178x `mySecurityPal.push_back(new TMySecurity(...,
//               fMain->sbXXX->Glyph, sbYYY));`                    GATE (SEC1)
//    :209-213   `iMaxLevelItem=mySecurityPal.size(); for(...)
//               mySecurityPal[i]->SetVisible(true);`              GATE (SEC1)
//    :215       `GetLevelSet();`                       -- reads
//               d:\HT9045\system\levelset.dat (this form's OWN file, not the shared
//               Gerneral.ini/INIFileGeneral) but ALSO reads
//               `CosFunction.bSecurityHave5Level` -- a plain (non-pointer)
//               global object defined in ANOTHER translation unit. If this
//               ctor runs at static-init time (which it now does -- see
//               global homecoming below) there is no language guarantee
//               CosFunction's own dynamic initializer has already run
//               (classic C++ SIOF: initialization order between globals in
//               different TUs is unspecified).                    SIOF RISK
//    :216-237   `FileNameJam000=...; if(!FileExists(...)) { for(...)
//               {cbJamArea->ItemIndex=i; ... GetJameCodeOfAxis(...);
//               ...ChangeJamMessage(...);} }` -- touches disk (FileExists,
//               and ChangeJamMessage's own CheckAndReadIniData/WriteIniData)
//               AND (through ChangeJamMessage) reads
//               CosFunction.bUseAlarmUnlockPassWord/bIncludeMTBA/
//               bUseAlarmLogXml/bOLPFunction/bConAlarmNeedKeyInPassword/
//               bConAlarmInTimeLevelUp/bEnableHandlerResultServer and
//               `CUSTOMER_CODE` (another TU's global, same SIOF class).
//                                                                  SIOF RISK
//    :238       `AddAlarmList();` -- reads `CosFunction.bStatisticsJamCount`,
//               same SIOF class, plus calls `GetAlarmCodeList(sgStatisticsJam)`
//               (cMyDB.cpp, itself untouched by this wave but a further
//               cross-TU call happening at static-init time).     SIOF RISK
//  Verdict: golden constructs this form inside WinMain's CreateForm chain,
//  strictly AFTER every global object (CosFunction, IniConfig, CUSTOMER_CODE,
//  LastSet, ...) already exists AND after config load. This ctor is NOT the
//  "cContactCT/cCounterClear shape" (zero cross-TU touch) -- it is exactly
//  the "cObserver/cShowBinSelect shape" this tree already has a proven fix
//  for: gate the SIOF-risky statement group behind `if (INIFileGeneral != 0)`
//  (common.h:51; INIFileGeneral starts nullptr, constant-initialized, and is
//  set ONLY by OpenGeneralIniFile(), called from the controlled startup
//  sequence strictly after every TU's globals have already run their own
//  dynamic initializers -- cObserver.cpp:428 / cShowBinSelect.cpp:181
//  precedent, using the SAME sentinel for the SAME reason). Applied here as
//  ONE guard wrapping the THREE SIOF-risk statement groups together (:215,
//  :216-237, :238) -- see cSecurity.cpp's ctor. The `#if 0`-gated
//  mySecurityPal block (:26-213) is a SEPARATE, unconditional gate (a
//  missing-symbol compile-time problem, not an ordering problem) and stays
//  outside the SIOF guard entirely -- it would not compile even after
//  config load, so guarding it on INIFileGeneral would be misleading.
//
//  EMERGENT BEHAVIOUR, MEASURED NOT GUESSED: because GATE (SEC1) leaves
//  `iMaxLevelItem` at its zero-init default, `Insufficient(iType)` for any
//  `iType>0` hits golden's own `if(iType>iMaxLevelItem) return false;` guard
//  (cSecurity.cpp:586, translated verbatim, unconditional -- no gate of ours
//  touches it) and returns false UNCONDITIONALLY today, regardless of
//  `AccessLevel`. This is not a new default invented for this wave -- it is
//  the SAME fail-closed answer every known consumer already assumed before
//  this file existed (cContactCT.cpp GATE (C3): "Safe default: FAIL CLOSED";
//  cShowBinSelect.cpp GATE (B5)/(B6): same). All 5 of today's known
//  Insufficient() calls use iType>0 (177/107/43/97/108, see CONSUMER
//  REGISTER) -- none currently observe a behaviour change from landing this
//  file. `Insufficient(-1)` (the one branch NOT gated by iMaxLevelItem,
//  golden :575-582, "hontech權限") DOES give a real, live answer today
//  (`AccessLevel<iDefHonPrecLevel`) -- no known caller uses -1 yet (grepped
//  this wave), but a future caller would get real behaviour immediately.
//  RE-VERIFY this paragraph's premise once GATE (SEC1) is ever lifted (fMain
//  grows the ~100 TSpeedButton members) -- iMaxLevelItem becomes 179 and
//  Insufficient() starts returning LevelSet.AccessLevel-driven answers.
//
//  GATE REGISTER (this file's own gates -- grep evidence, run 20260819
//  against the working tree, excluding build_*/ directories)
//  --------------------------------------------------------------------------
//  (SEC1) ctor's mySecurityPal population (golden :26-213) -- forms/fMain.h
//         declares ZERO `TSpeedButton *sbXXX` members (`grep -c "TSpeedButton
//         \*sb" forms/fMain.h` -- 0, 20260819; `class TfMain` has 1040 lines
//         and none of them are the ~100 golden names this loop reads --
//         sbSetting/sbConfig/sbOffset/sbSpeed/sbIO/sbMessage/
//         sbCloseProgram/... the full list is golden cSecurity.cpp:26-206).
//         `Graphics::TBitmap` (the `Img` parameter's type) also has 0 hits
//         anywhere in vclcompat. Gated as ONE block (DefaultImg assignment +
//         all 178 push_back calls + the iMaxLevelItem/SetVisible loop) --
//         see CTOR SAFETY for the self-consistent "iMaxLevelItem stays 0,
//         mySecurityPal stays empty" invariant this produces.
//  (SEC3) SecurityPalVisible's entire body (golden :470-571) -- every one of
//         its ~55 statements indexes `mySecurityPal[N]` for some fixed N up
//         to 178; with the vector empty (GATE SEC1), any such index is
//         undefined behaviour (out-of-bounds vector access), not merely
//         "would not compile" -- gated as a unit rather than left to crash
//         the first time FormShow's `#ifndef SOFT_SIMULTE SecurityPalVisible
//         ();#endif` call runs. RE-ACTIVATE VERBATIM once GATE (SEC1) lifts
//         -- every referenced global (CosFunction.*/IniConfig.*/CUSTOMER_CODE/
//         USE_*/ATC_SYSTEM/SHUTTLE_SENSOR_TYPE/...) is ALREADY real and
//         portable; only the mySecurityPal indexing needs the vector to be
//         populated first.
//  (SEC-TAB) FormShow's ScrollBox/Controls[] panel-arrangement block (golden
//         :382-414) -- needs BOTH `TScrollBox` (0 hits in vclcompat --
//         `sbMain`/`sbTools`/.../`sbOther` are declared TPanel stand-ins
//         here instead, see S12) AND a child-control enumeration API
//         (`->ControlCount`/`->Controls[i]`) that none of TPageControl/
//         TTabSheet/TPanel expose (`grep -n "ControlCount\|Controls\[" vclcompat/
//         Controls.h` -- 0 hits, 20260818). The block's ENTIRE purpose is to
//         lay out the (currently always-empty, GATE SEC1) mySecurityPal
//         panels inside their tab sheets -- with zero panels ever created,
//         gating this changes nothing observable today, same reasoning as
//         SEC3.
//  (SEC4) GetJamArea's `return fMain->AlarmUnitMap[sJamArea];` (golden
//         :1413-1416) -- `grep -n "AlarmUnitMap" forms/fMain.h` -- 0 hits,
//         20260819. DEFAULT: return `""` (empty AnsiString) -- GetJamArea has
//         exactly one golden caller tree-wide search-checked this wave
//         (nowhere in golden cSecurity.cpp/cCounterClear.cpp itself, and no
//         port `.cpp` calls it yet either -- it is an unreferenced public
//         method today), so there is no observable consumer of this default.
//  (SEC5) ChangeJamMessage's `RichEditJamCode->Font->Charset=ANSI_CHARSET;`
//         and the four `RichEditJamCode->Font->Name=...` assignments (golden
//         :987, :991/:996/:1001/:1007) -- vclcompat's TControl base (which
//         TMemo, the S11 stand-in for TRichEdit, inherits through
//         TCustomEdit) declares only Visible/Enabled/hCtl (vclcompat/
//         Controls.h:213-223, confirmed by reading the class this wave) --
//         no `Font` member anywhere in the TControl hierarchy. Purely
//         cosmetic (font family/charset for on-screen Chinese/Korean
//         rendering); dropping it changes no file path, no persisted value,
//         no control flow.
//  (SEC6) AddAlarmList's `sgStatisticsJam->ColWidths[7]=50;` (golden :1673)
//         -- vclcompat/StringGrid.h's own file-head comment lists ColWidths
//         among the properties NOT modelled (`grep -n "ColWidths"
//         vclcompat/StringGrid.h` -- 1 hit, that comment, 0 real members).
//         Purely cosmetic column width; `sgStatisticsJam->Cells[7][0]=
//         "Count";` right above it stays ACTIVE.
//  (SEC7) `ChangePassword(int)` (free function, golden :599-811) -- entire
//         body. `grep -rn "class TfLogin\|fLogin\s*;" --include=*.h .` --
//         0 hits tree-wide, 20260819 (neither `login.h` nor any `fLogin`
//         facade exists anywhere in the port; confirmed also: `find . -iname
//         "*login*"` under the source tree returns only tools/dfm2rc
//         generated-artifact paths, never a compiled header). Every branch
//         of ChangePassword dereferences `fLogin->...` (ShowModal/
//         edUserName/edLoginOldPassword/edLoginNewPassword/rgLoginOption/
//         cbLoginUserName/labUserName), so there is no partial-evaluation
//         split like SEC5/SEC6 -- the WHOLE function is unreachable-by-
//         construction. DEFAULT: no-op (void function, does nothing) --
//         called only from 4 UI click handlers (btnHonPrecClick/
//         sbSupervisorClick/sbEngineerClick/btnOperatorClick), so the
//         observable effect is "the change-password dialog does not run",
//         the same "no modal dialog surface yet" default already
//         established at GATE (C2)/S13 elsewhere in this tree.
//  (SEC9) GetPasswoard's `if(IniConfig.bSIGURDFunction){...}` block (golden
//         :1758-1768) -- `MyForceDirectories("D:\\HT9045\\system")` +
//         `WriteIniData("D:\\HT9045\\system\\SG_PW.ini", ...)` -- `system\`
//         is on the task brief's explicit write-gate list. Gated as a unit
//         (the read `CheckAndReadIniData(...)` right after it is the ONLY
//         thing that would give a non-default answer, and it depends on the
//         file the gated block would have created, so partially keeping the
//         read active would silently always return the pre-seeded default
//         anyway -- gating both together is the honest description of that).
//         DEFAULT: `asPassword="16943420"` (golden's own pre-set default,
//         unconditionally returned).
//  (SEC10) TMySecurity::TMySecurity/SetParent/SetPosition/SetCaption bodies
//         (golden :818-852, :860-902, :904-908, :910-920) -- TMySecurity is
//         UNREACHABLE this wave (GATE SEC1: nothing ever constructs one).
//         Even setting reachability aside, every one of these four bodies
//         sets `->Left`/`->Top`/`->Width`/`->Height` and/or `->Name` on
//         Panel/RadioGroup/SpeedButton or `->Caption` on RadioGroup -- NONE
//         of these exist on vclcompat's TPanel/TRadioGroup/TSpeedButton
//         (TControl's base has only Visible/Enabled/hCtl; TRadioGroup adds
//         only ItemIndex/Items; TSpeedButton adds only Caption/Down/
//         GroupIndex -- confirmed by reading vclcompat/Controls.h in full
//         this wave). Inventing a facade-only geometry/Name subclass (the
//         forms/fContactCT.h TfContactCTGrid/.../pattern) for a class with
//         ZERO live instances would be manufacturing surface no code can
//         exercise -- gated instead. The five members golden itself wrote
//         INLINE in the header (SetVisible/GetVisivle/SetLevel/GetLevel/
//         SetEnabled) and GetCaption (reads only ->Caption, which DOES
//         exist on TSpeedButton) stay FULL/active, unaffected -- see class
//         body below.
//
//  CONSUMER REGISTER (existing tree-wide #if 0 sites already waiting on this
//  file's symbols -- grep evidence, run 20260819)
//  --------------------------------------------------------------------------
//  Insufficient(int,bool) -- 5 call sites, ALL already fail-closed (see
//  EMERGENT BEHAVIOUR above for why landing this file changes none of them
//  today):
//    * atester.cpp:3823 `fSecurity->Insufficient(177)==false` -- inert: sits
//      inside the :3601-:3858 `#if 0 // GOLDEN VERBATIM ... GATE
//      G-PTk3-ProcessTesterTimeOut` block (confirmed by directive-nesting
//      count this wave; the ACTIVE ProcessTesterTimeOut() is the slim body
//      after that #endif).
//    * cContactCT.cpp:1190/1196/1244, GATE (C3) -- `Insufficient(107)`,
//      forced `false` (fail-closed), per forms/fContactCT.h's own banner.
//    * cShowBinSelect.cpp:928, GATE (B5) -- `Insufficient(43)`, forced false.
//    * cShowBinSelect.cpp:974, GATE (B5) sibling -- `Insufficient(97)`.
//    * cShowBinSelect.cpp:988, GATE (B6) -- `Insufficient(108)`, forced false.
//  GetBit8(AnsiString,AnsiString) -- 1 call site:
//    * Command.cpp:14959, GATE(FW3-WF) item 5 -- `fSecurity->GetBit8(fNote->
//      Edit3->Text+" "+fNote->edUnitName->Text, fNote->edErrorCode->Text)==1`
//      inside `#if 0`. NOTE: even after this file lands, that call site
//      needs a SECOND facade extension before it can un-gate --
//      `fNote->Edit3`/`fNote->edUnitName` do not exist on forms/fNote.h
//      either (`grep -n "Edit3\|edUnitName" forms/fNote.h` -- 0 hits,
//      re-confirmed 20260819) -- fSecurity landing is necessary but not
//      sufficient for this one gate.
//  fSecurity->fShow -- 2 sites, both inside large OR-chains already gated
//  for OTHER reasons (most of the ~26-33 operands per chain are absent
//  facades): Command.cpp:10207 (SettingsIsWindowOpened, `#if 0`) and
//  Command.cpp:14923 (sibling function, same shape).
//  ClearAllJamCount / AddJamCount / GetJamArea / GetPasswoard / the Get*
//  Alarm-code family -- grepped this wave, NO external caller anywhere in
//  the port tree yet (golden's own callers -- AddAlarmList calling
//  ClearAllJamCount, spbImportClick/ChangeJamMessage's own internal use --
//  are covered by THIS file's own translation, not a separate consumer).
//
//  STUB COLLISION (searched 20260819, excluding build_*/ and golden dirs)
//  --------------------------------------------------------------------------
//  `class TfSecurity` (real type) -- 0 hits anywhere in the port tree before
//  this file. `TfSecurity`/`fSecurity` (the bare names, any type) appear in
//  exactly 15 files, ALL either (a) comments/banners citing golden line
//  numbers (the majority), or (b) `#if 0`-gated call sites already itemised
//  in the CONSUMER REGISTER above, or (c) `tools/dfm2rc/.../cSecurity_
//  layout.gen.cpp` (a generated .dfm-derived layout table, data only, not a
//  C++ type). No prior stand-in class, no TU-local seam macro (unlike
//  fCounterClear's csystem.cpp:4851-4854 seam -- csystem.cpp has NO
//  equivalent `#define fSecurity` anywhere: `grep -n "define fSecurity"
//  csystem.cpp` -- 0 hits). Nothing to retire or reconcile on the fSecurity
//  side of the main-loop integration.
// =============================================================================
#ifndef FORMS_FSECURITY_H
#define FORMS_FSECURITY_H

#include <vector>
#include <string>   // Steven 20260926: friend W906_SecurityJamOp signature

#include "vclcompat/vcl_compat.h"   // AnsiString, TObject
#include "vclcompat/Comm.h"          // TComponent
#include "vclcompat/Controls.h"       // TPanel, TSpeedButton, TRadioGroup, TLabel, TComboBox, TMemo, TCheckBox, TPageControl, TTabSheet
#include "vclcompat/StringGrid.h"      // TStringGrid

using vclcompat::TObject;
using vclcompat::TComponent;
using vclcompat::TPanel;
using vclcompat::TSpeedButton;
using vclcompat::TRadioGroup;
using vclcompat::TLabel;
using vclcompat::TComboBox;
using vclcompat::TMemo;             // S11: stand-in for golden TRichEdit (no port anywhere in vclcompat)
using vclcompat::TCheckBox;
using vclcompat::TPageControl;
using vclcompat::TTabSheet;
using vclcompat::TStringGrid;
using vclcompat::TControl;

// S13: local stand-ins for golden TOpenDialog/TSaveDialog -- neither type
// exists anywhere in vclcompat (0 hits, grepped this wave). Both golden call
// sites (spbImportClick/spbExportClick) only ever touch ->Execute() and
// ->FileName. Execute() hard-returns false ("operator cancelled"), matching
// the established "no modal dialog surface yet" fail-closed default (GATE
// (C2) precedent, forms/fContactCT.h) -- the verbatim golden body inside
// each `if (Dialog->Execute())` stays syntactically whole in cSecurity.cpp
// (dead but compiled), not deleted.
//  Steven 團隊 20260926 (Status.Security Jam)：W906Armed —— 網頁替代對話框（WebSecurityJam.cpp）先把瀏覽器上傳的 CSV
//  寫成暫存檔（匯入）或指定暫存輸出檔（匯出），設 FileName 並 W906Armed=true，再呼叫 golden spbImportClick／spbExportClick。
//  Execute() 只回一次 true（等於操作員按了「開啟／存檔」），之後回到 false（沒武裝＝操作員按取消，與原本相同）。
class TSecurityOpenDialog
{
public:
    AnsiString FileName;
    bool W906Armed = false;
    bool Execute() { const bool b = W906Armed; W906Armed = false; return b; }
};
class TSecuritySaveDialog
{
public:
    AnsiString FileName;
    bool W906Armed = false;
    bool Execute() { const bool b = W906Armed; W906Armed = false; return b; }
};

// S17: facade-only widget extension for cbJamArea/cbJamCode/cbJamLang --
// golden calls `->Refresh()` on these (a GDI repaint trigger) at ~9 call
// sites (ctor, FormShow, spbExportClick); vclcompat's TComboBox/TControl has
// no `Refresh()` member anywhere (0 hits, grepped this wave). Same "GDI
// repaint, no-op this wave" posture already established for
// forms/fObserver.h's TfObserverGrid (StringGrid2/3->Refresh()) and
// forms/fContactCT.h's TfContactCTGrid.
//  Steven 團隊 20260926 (Status.Security Jam)：Refresh() 改成把 Text 同步成 Items[ItemIndex]。VCL 的 TComboBox 在設
//  ItemIndex 時 Text 就跟著變（csDropDown），vclcompat 的 Text/ItemIndex 是兩個獨立欄位；golden 每一處
//  `cbJamXxx->ItemIndex=n;` 後面都緊接 `->Refresh();`（cSecurity.cpp:222-231、:418-424、:955-958、:1613-1622），
//  接著就讀 `->Text`。不同步的話 JamArea／JamCode 永遠是空字串，SaveJamLevel :1066 直接 return。
class TSecurityComboBox : public vclcompat::TComboBox
{
public:
    void Refresh() { Text = (ItemIndex >= 0 && ItemIndex < Items->Count) ? Items->Strings[ItemIndex] : AnsiString(""); }
};

// S18: facade-only widget extension for rgJamLevel -- golden reads/writes
// `->Columns` (golden :303-309) as an insert-once guard ("has the extra
// Open/Operator level already been added to Items") -- NOT purely cosmetic
// (the guarded `Items->Insert(0,...)` line has real, non-idempotent effect
// on an Items list other code indexes by position). vclcompat's TRadioGroup
// has no `Columns` member (0 hits, grepped this wave) -- same
// facade-extension pattern as forms/fContactCT.h's TfContactCTRadioGroup
// (which also adds a Columns field for the identical reason).
class TSecurityRadioGroup : public vclcompat::TRadioGroup
{
public:
    int Columns = 4;   // golden TRadioGroup .dfm design-time default for a 4-level palette
};

// =============================================================================
//  TMySecurity -- golden cSecurity.h:17-38, one row of the (unreachable this
//  wave, see GATE SEC1) per-function access-level palette.
// =============================================================================
class TMySecurity : public TComponent
{
private:
    int index;
    AnsiString Caption;
    TPanel *Panel;
    TSpeedButton *SpeedButton;
    bool Visible;
public:
    // DEVIATION (S14): golden `TMySecurity(AnsiString Caption, Graphics::
    // TBitmap *Img, TWinControl *Sender)` -- neither `Graphics::TBitmap` nor
    // `TWinControl` exists anywhere in vclcompat (0 hits, grepped this
    // wave). `Img` dropped, `Sender` retyped to the closest existing analog
    // (`TControl*`, vclcompat's own generic widget base). This class is
    // unreachable this wave (GATE SEC1) so no call site is affected.
    TMySecurity(AnsiString Caption, TControl *Sender);
    ~TMySecurity();
    // GATE (SEC10): see forms/fSecurity.h banner -- Name/geometry properties
    // this body needs do not exist on vclcompat's TPanel/TRadioGroup/
    // TSpeedButton.
    void SetCaption(AnsiString Caption, int index);
    AnsiString GetCaption();
    // GATE (SEC10).
    void SetParent(TControl *Sender);
    void SetVisible(bool bShow) { Panel->Visible = bShow; Visible = bShow; };
    bool GetVisivle() { return Visible; };
    // GATE (SEC10).
    void SetPosition(int Top, int Left = 7);
    void SetLevel(int Level) { RadioGroup->ItemIndex = Level; };
    int  GetLevel() { return RadioGroup->ItemIndex; };
    TRadioGroup *RadioGroup;
    void SetEnabled(bool Enabled) { RadioGroup->Enabled = Enabled; };      //wei 20150803
};

// =============================================================================
//  TfSecurity -- non-VCL facade (golden cSecurity.h)
// =============================================================================
class TfSecurity
{
public:
    // -- widgets (golden __published, cSecurity.h:42-102) --------------------
    TPanel       *Panel13                = new TPanel();
    TPanel       *Panel1                 = new TPanel();
    TSpeedButton *sbSupervisor           = new TSpeedButton();
    TSpeedButton *sbEngineer             = new TSpeedButton();
    TPanel       *Panel17                = new TPanel();
    TPanel       *SecurityExit           = new TPanel();
    TSpeedButton *btnHonPrec             = new TSpeedButton();
    TPageControl *PageControl1           = new TPageControl();
    TTabSheet    *tsMain                 = new TTabSheet();
    TTabSheet    *tsTools                = new TTabSheet();
    TTabSheet    *tsConfig               = new TTabSheet();
    TTabSheet    *tsContact              = new TTabSheet();
    TTabSheet    *tsSetup                = new TTabSheet();
    TTabSheet    *tsTemp                 = new TTabSheet();
    TPanel       *sbMain                 = new TPanel();   // S12: TScrollBox stand-in (no port anywhere in vclcompat)
    TTabSheet    *tsIo                   = new TTabSheet();
    TTabSheet    *tsConfiguration        = new TTabSheet();
    TTabSheet    *tsYield                = new TTabSheet();
    TTabSheet    *tsOther                = new TTabSheet();
    TPanel       *sbTools                = new TPanel();   // S12
    TPanel       *sbConfig               = new TPanel();   // S12
    TPanel       *sbContact              = new TPanel();   // S12
    TPanel       *sbSetup                = new TPanel();   // S12
    TPanel       *sbTemp                 = new TPanel();   // S12
    TPanel       *sbIO                   = new TPanel();   // S12
    TPanel       *sbConfiguration        = new TPanel();   // S12
    TPanel       *sbYield                = new TPanel();   // S12
    TPanel       *sbOther                = new TPanel();   // S12
    TTabSheet    *tsJamCode              = new TTabSheet();
    TSecurityRadioGroup *rgJamLevel      = new TSecurityRadioGroup();   // S18: ->Columns extension
    TLabel       *labJamArea             = new TLabel();
    TSecurityComboBox *cbJamArea         = new TSecurityComboBox();   // S17: ->Refresh() extension
    TLabel       *labJamCode             = new TLabel();
    TSecurityComboBox *cbJamCode         = new TSecurityComboBox();   // S17
    TLabel       *Label1                 = new TLabel();
    TLabel       *labLang                = new TLabel();
    TSecurityComboBox *cbJamLang         = new TSecurityComboBox();   // S17
    TMemo        *RichEditJamCode        = new TMemo();    // S11: TRichEdit stand-in
    TCheckBox    *cbJamNeedRed           = new TCheckBox();
    TSpeedButton *spbExport              = new TSpeedButton();
    TSpeedButton *spbImport              = new TSpeedButton();
    TSecurityOpenDialog *OpenDialog1     = new TSecurityOpenDialog();   // S13
    TSecuritySaveDialog *SaveDialog1     = new TSecuritySaveDialog();   // S13
    TLabel       *labMustCheck_35        = new TLabel();
    TSpeedButton *btnOperator            = new TSpeedButton();
    TCheckBox    *cbSilentMode           = new TCheckBox();
    TCheckBox    *cbUnlockPassWord       = new TCheckBox();
    TRadioGroup  *rgMachineStatusBit8    = new TRadioGroup();
    TTabSheet    *tsStatisticsJam        = new TTabSheet();
    // AI(W906-FW-SecCC-integrate) 20260819: hydrated to the .dfm design-time
    // ColCount=8 (cSecurity.dfm; RowCount unset there -> ctor default 5) --
    // the bare default-5 first landed here made golden's own Cells[7][...]
    // accesses (AddJamCount/ClearAllJamCount/AddAlarmList's "Count" column)
    // throw out_of_range; caught by the fresh-dir gate running
    // test_security_core (Obs2fix-class defect, same fix shape).
    TStringGrid  *sgStatisticsJam        = new TStringGrid(8, 5);
    TCheckBox    *cbIncludeMTBA          = new TCheckBox();
    TCheckBox    *chkCheckContAlarm      = new TCheckBox();
    TCheckBox    *chkO17                 = new TCheckBox();
    TCheckBox    *cbAddAlarmLog          = new TCheckBox();
    TCheckBox    *cbN27AddBoard          = new TCheckBox();
    TCheckBox    *cbN27AlarmSel          = new TCheckBox();
    TCheckBox    *cbN27AlarmSelByArea    = new TCheckBox();
    TCheckBox    *chkTCPAlarm            = new TCheckBox();
    TCheckBox    *cbContAlarmNotUpload   = new TCheckBox();
    TCheckBox    *chkAlarmAfterFullTray  = new TCheckBox();

    TfSecurity();

    void FormShow(TObject *Sender);
    // DEVIATION: golden `void __fastcall FormClose(TObject *Sender,
    // TCloseAction &Action)` -- TCloseAction has no port, see banner.
    void FormClose();
    void FormDestroy(TObject *Sender);
    void SecurityExitClick(TObject *Sender);
    void btnHonPrecClick(TObject *Sender);
    void sbSupervisorClick(TObject *Sender);
    void sbEngineerClick(TObject *Sender);
    void cbJamAreaChange(TObject *Sender);
    void cbJamCodeChange(TObject *Sender);
    void cbJamLangChange(TObject *Sender);
    void spbImportClick(TObject *Sender);
    void spbExportClick(TObject *Sender);
    void btnOperatorClick(TObject *Sender);

private:
    // Steven 團隊 20260926 (Status.Security Jam)：網頁（WS security.jam，WebSecurityJam.cpp）與開機（cSecurity.cpp 檔尾）要呼叫
    // golden 的 private ChangeJamMessage／SaveJamLevel、讀 FileNameJam000 —— 不改存取層級，只開這兩個 friend。
    friend std::string W906_SecurityJamOp(const std::string& payloadJson, bool* ok);
    friend void W906_SecurityJamBoot(); friend std::string W906_LevelSetPut(const std::string& tag, const std::string& payloadJson, bool allowSystemWrite, bool* ok);   //AI(W906-FRW-S64) 20260926: 網頁權限表存檔（WebLevelSet.cpp）照 golden FormClose :463 呼叫 SaveJamLevel；同一行，不移動行號
    void ChangeJamMessage(bool bSave = true);
    void SaveJamLevel();

    AnsiString JamArea, JamCode;

    AnsiString FileNameJam000;
    int JamLang = 0, MachineStatusBit8 = 0;

    void AddAlarmList();                                             //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    // GATE (SEC3): see banner -- entire body.
    void SecurityPalVisible();

public:
    bool Insufficient(int iType, bool bAlarm = true);                  //jou 981207 權限判斷
    std::vector<TMySecurity *> mySecurityPal;
    int  GetJamLevel(AnsiString sJamArea, AnsiString sJamCode);
    bool GetJemRed(AnsiString sJamArea, AnsiString sJamCode);
    bool GetJemSilent(AnsiString sJamArea, AnsiString sJamCode);                        //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
    void GetLevelSet();
    void SetLevelSet();
    bool GetJemUnlockPassWord(AnsiString sJamArea, AnsiString sJamCode);        //Ifor 20170214 add 可以自訂Alarm是否要有解除密碼
    bool GetJemIncludeMTBA(AnsiString sJamArea, AnsiString sJamCode);           //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
    bool GetJemContiAlarm(AnsiString sJamArea, AnsiString sJamCode);            //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量
    bool GetO17ContiAlarm(AnsiString sJamArea, AnsiString sJamCode);            //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    bool GetAddAlarmLog(AnsiString sJamArea, AnsiString sJamCode);              //Sam 20210611 : Alarm Log 可以自訂該不需要記 Log 名稱
    // GATE (SEC4): see banner -- entire body.
    AnsiString GetJamArea(AnsiString sJamArea);

    int  GetBit8(AnsiString sJamArea, AnsiString sJamCode);
    int  iBit8 = 0;
    bool fShow = false;
    void ClearAllJamCount();                                         //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    bool AddJamCount(AnsiString asJamCode);                          //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    bool GetN27AlarmSel(AnsiString sJamArea, AnsiString sJamCode = "");
    bool GetN27AddBoard(AnsiString sJamArea, AnsiString sJamCode);              //Sam 20210911 :  南特中興系統要求顯示增加開啟
    AnsiString GetPasswoard();                                                  //Sam 20220106 : 取出密碼
    bool GetJemTCPAlarm(AnsiString sJamArea, AnsiString sJamCode);              //Sam 20230426 : 通知客戶系統 Handler 已經密碼鎖定
    bool GetContAlarmNotUpload(AnsiString sJamArea, AnsiString sJamCode);       //Sam 20231116 : 新增 Alarm 不需上傳伺服器
    bool GetAlarmAfterUnloaderFull(AnsiString sJamArea, AnsiString sJamCode);   //Jimmychiu 20240902 : Need Alarm After Unloader Full
};

// AI(W906-FW-SecCC) 20260819: global homecomed UNCONDITIONALLY at static
// init -- this is the same choice cObserver.cpp:3128 / cShowBinSelect.cpp:92
// made (both ALSO have SIOF-risky ctors): the guard belongs INSIDE the ctor
// body around the risky statements (see cSecurity.cpp), not around the
// `new` itself. See forms/fSecurity.h's CTOR SAFETY section above for the
// full argument.
extern TfSecurity *fSecurity;   // golden cSecurity.h:157 (extern PACKAGE TfSecurity *fSecurity;)

#endif // FORMS_FSECURITY_H
