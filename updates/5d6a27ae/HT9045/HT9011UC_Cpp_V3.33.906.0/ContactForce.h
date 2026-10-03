// ContactForce.h
// Standard C++ translation of BCB6 ContactForce.h (V3.33.906.0_20260618).
// Source of truth: HT9011UC_Code_V3.33.906.0_20260618/ContactForce.h
//
// Translation scope: CALC CORE ONLY.
//   The original BCB6 file contains four VCL component classes whose constructors
//   both build UI widgets AND derive force values from the kit diameter.  This
//   translation extracts only the portable math (diameter parse + dMinForce ladder
//   + dMaxForce formula) and exposes it through two free functions and four plain
//   data structs that mirror the original names.
//
//   VCL form class TfContactForce, all widget pointers (TGroupBox, TTrackBar,
//   TEdit, TLabel, TPageControl, TScrollBox, TStringList), __fastcall event
//   handlers, ReadFile/WriteFile/ShowValue, and every global-config reference
//   are NOT translated here; they belong to a UI layer that depends on VCL.
//
// Key conversion notes (1-based AnsiString -> 0-based std::string):
//   BCB6 L183: Dia.Pos("Arm2_")==0  means NOT found (AnsiString.Pos is 1-based,
//              returns 0 when absent).  Correct std port: find()==npos.
//   BCB6 L185: Dia.SubString(6, Dia.Length())  starts at 1-based char 6, i.e.
//              skips the first 5 chars ("Arm2_" or similar prefix of len 5).
//              Correct std port: substr(5).
//
// BCB6 source lines cited inline in ContactForce.cpp.

#ifndef CONTACTFORCE_H
#define CONTACTFORCE_H

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// SlkForceData
//   Plain data struct mirroring the calc-relevant fields of THTSLKClass,
//   THTSLKIndClass, THTDieForceSLKClass, and THTDieForceOneByOneSLKClass.
//   Non-calc fields (dLoadRate, dHotOffset, dContactOffset, bShow, iTag,
//   sDiameter) are preserved so callers that need them can populate them
//   separately via ReadFile logic ported later.
// ---------------------------------------------------------------------------
struct SlkForceData
{
    std::string sDiameter;  // original diameter token string (BCB6: AnsiString sDiameter)
    int         iTag;       // slot index                     (BCB6: int iTag)
    bool        bShow;      // visible flag                   (BCB6: bool bShow)
    double      dDiameter;  // parsed mm value                (BCB6: double dDiameter)
    double      dMinForce;  // kg  -- derived in ctor         (BCB6: double dMinForce)
    double      dMaxForce;  // kg  -- derived in ctor         (BCB6: double dMaxForce)
    double      dLoadRate;        // ini-loaded at runtime  (BCB6: double dLoadRate)
    double      dLoadRate_NS;     // NS variant              (BCB6: double dLoadRate_NS, THTSLKClass only)
    double      dHotOffset;       // ini-loaded             (BCB6: double dHotOffset)
    double      dContactOffset;   // ini-loaded             (BCB6: double dContactOffset)
    double      dContactOffset_NS;// NS variant             (BCB6: double dContactOffset_NS, THTSLKClass only)

    SlkForceData()
        : iTag(0), bShow(false), dDiameter(0.0), dMinForce(0.0), dMaxForce(0.0),
          dLoadRate(1.0), dLoadRate_NS(1.0), dHotOffset(0.0),
          dContactOffset(0.0), dContactOffset_NS(0.0) {}
};

// ---------------------------------------------------------------------------
// Variant selector passed to ComputeSlkForce.
//
//   SLKV_STANDARD     -- THTSLKClass:             "80_Hi", "40x2" specials;
//                        EP_Install==5 "Arm2_" strip; 402 in dMinForce ladder.
//   SLKV_IND          -- THTSLKIndClass:           plain atof; no 402 case.
//   SLKV_DIEFORCE     -- THTDieForceSLKClass:      identical to IND (no 402).
//   SLKV_DIEFORCE_OBO -- THTDieForceOneByOneSLKClass: identical to IND (no 402).
// ---------------------------------------------------------------------------
enum SlkVariant
{
    SLKV_STANDARD     = 0,
    SLKV_IND          = 1,
    SLKV_DIEFORCE     = 2,
    SLKV_DIEFORCE_OBO = 3
};

// ---------------------------------------------------------------------------
// ComputeSlkForce
//   Portable, self-contained replacement for the diameter + force derivation
//   that lives inside each BCB6 class constructor.
//
//   Parameters
//     dia        -- diameter token string (BCB6: AnsiString Dia ctor param)
//     tag        -- slot index            (BCB6: int Tag ctor param)
//     bDefault   -- visibility default    (BCB6: bool bDefault ctor param)
//     epInstall  -- EP_Install global     (BCB6: EP_Install macro/global, int)
//     variant    -- selects which class's rules to apply (see SlkVariant)
//
//   Returns a fully populated SlkForceData with dDiameter/dMinForce/dMaxForce
//   computed.  dLoadRate and offset fields are left at their default (0.0 / 1.0);
//   the caller populates them from ini data (ReadFile equivalent).
//
//   BCB6 source: ContactForce.cpp L171-205  (THTSLKClass)
//                ContactForce.cpp L313-327  (THTDieForceOneByOneSLKClass)
//                ContactForce.cpp L404-418  (THTDieForceSLKClass)
//                ContactForce.cpp L1545-1558 (THTSLKIndClass)
// ---------------------------------------------------------------------------
SlkForceData ComputeSlkForce(const std::string& dia,
                              int tag,
                              bool bDefault,
                              int epInstall,
                              SlkVariant variant);

// ---------------------------------------------------------------------------
// Button1 voltage interpolation helper
//   Self-contained numeric op from TfContactForce::Button1Click.
//   BCB6 source: ContactForce.cpp L1588-1598
//
//   Formula: dMaxVol = (((dMidVol - dMinVol) / 5) * 9) + dMinVol
//   All arithmetic is double; /5 is double division (no truncation).
// ---------------------------------------------------------------------------
double ComputeEpMaxVoltage(double dMinVol, double dMidVol);

// ===========================================================================
// AI(W906-FW-SLK-W31) 20260826: THE SLK TABLE -- runtime container + query +
// parse-only loaders.  This is the piece forms/fContact.h's (X-01) entry asked
// for by name: "give the tree a real SLK table ... a container of SlkForceData
// plus its loader".
//
// ---------------------------------------------------------------------------
// WHAT GOLDEN HAS, AND WHERE
// ---------------------------------------------------------------------------
// golden ContactForce.h:341-356 gives TfContactForce FOUR parallel vectors:
//       vector<THTSLKClass *>              SLKClass;                  :341
//       vector<THTSLKIndClass *>           SLKIndClass;               :342
//       vector<THTDieForceSLKClass *>      DieForceSLKClass;          :349
//       vector<THTDieForceOneByOneSLKClass*>DieForceOneByOneSLKClass; :354
// All four hold the SAME calc-relevant fields, which is why one SlkForceData
// (above) already mirrors all four and why one container type serves all four.
//
// They are FILLED in golden's form constructor, NOT in ReadFile:
//       SLKClass                 ContactForce.cpp:463-491
//       SLKIndClass              ContactForce.cpp:501-539
//       DieForceSLKClass         ContactForce.cpp:611-615
//       DieForceOneByOneSLKClass ContactForce.cpp:626-636
// ReadFile (:963-1178) then OVERWRITES the per-entry ini fields (dLoadRate,
// dHotOffset, dContactOffset, dContactOffset_NS) on the already-built vectors.
// Reading only ReadFile and porting "the ReadFile path" would therefore produce
// an EMPTY table; the ctor loop is the load-bearing half.  Verified by reading
// both bodies in full on 20260826.
//
// ---------------------------------------------------------------------------
// WHY THIS UNBLOCKS (X-01)..(X-06)
// ---------------------------------------------------------------------------
// golden cContact.cpp:18710-18727 (re-read line by line 20260826, it is exactly
// as forms/fContact.h describes it):
//       for(unsigned int i=0; i<fContactForce->SLKClass.size(); i++)
//       {
//           double d1=fContactForce->SLKClass[i]->dDiameter;
//           double d2=atof(rgKitDiameter->Items->Strings[ItemIndex].c_str());
//           sBuffer =rgKitDiameter->Items->Strings[ItemIndex].c_str();
//           if(sBuffer=="40x2") d2=402;
//           if(d1==d2) { iTag=i; dKitDiameter=d2; if(dKitDiameter==402) dKitDiameter=40; }
//       }
// -- i.e. the ONLY thing the walk needs from the container is `size()` and each
// entry's `dDiameter`, keyed by the SELECTED ITEM'S TEXT (not by index: the
// rgKitDiameter list is a bShow-FILTERED, Arm-prefix-STRIPPED projection of
// SLKClass, built at golden cContact.cpp:1108-1128, so ItemIndex does NOT index
// SLKClass).  That is `SlkForceTable::ResolveKitDiameter` below, and its two
// extra outputs -- SLKClass[iTag]->dContactOffset / ->dContactOffset_NS, read by
// GetMinForce at golden cContact.cpp:19008 / :19012 (both re-read 20260826) --
// are then just field reads on the resolved entry.
//
// WARNING: `ResolveKitDiameter` DELIBERATELY REPORTS `bMatched`.  golden's
// dKitDiameter keeps its :18680 initialiser 30.0 when nothing matches, and that
// IS golden behaviour in a real configuration (CosFunction.bUseDynamicKitDiameter
// ==false leaves SLKClass empty -- golden ContactForce.cpp:430 gates the entire
// ctor fill).  What is NOT golden behaviour is a port that never loaded the
// table at all and silently returns the same 30.0.  `SlkForceTable::bLoaded`
// separates those two: a caller that finds `bLoaded==false` must refuse to
// compute, not fall back.  30.0 actively selects ComputeMinForce's 30mm arm, the
// :18740 area clamp, and the ASE-Kaohsiung `if(dKitDiameter==30) dMinForce=1;`
// at :18866 -- and the output reaches the EP regulator via DeviceForm.dPress.
//
// ---------------------------------------------------------------------------
// PARSE / IO SPLIT (this wave is PARSE ONLY -- deliberate, see below)
// ---------------------------------------------------------------------------
// golden's fill reads its two input strings from disk:
//       edtCurrentType->Text = CheckAndReadIniData(FileName,"SLK Type","Type",   default)   :439/:448
//       edtVisible->Text     = CheckAndReadIniData(FileName,"SLK Type","Visible",default)   :440/:449
// and ReadFile reads each entry's four values the same way (:1061-1065).
// NOTHING here opens a file.  Every loader takes ALREADY-READ TEXT; every
// per-entry ini value is an ALREADY-READ double handed to an Apply* function.
// The disk half is deferred ON PURPOSE and is NOT merely "not done yet":
//   * `CheckAndReadIniData` is a WRITE path -- the port's own overload family
//     (common.cpp:540 onward) SEEDS the default back into the file when the key
//     is absent; the int overload does it at common.cpp:603
//     (`INIFile->WriteInteger(Group, Name, Value); // seed`).  The pure-read
//     sibling is the separate `ReadIniData` family at common.cpp:685-757 (first
//     overload :692).  Line numbers re-checked against the port source
//     20260826 -- the ":678 / :684-686" figures this wave was briefed with are
//     wrong.  Porting the disk half here would make merely constructing this
//     table mutate a machine's ContactInfo.ini.
//   * It keeps this TU dependency-free (only <string>/<vector>/<cstdlib>/
//     <cstdio>), so it stays linkable from tests/test_ContactForce.cpp with no
//     new archive edge and no MachineType.h -- which would cost -Wpedantic noise
//     this target has no carve-out for (CMakeLists.txt:312/:317 grant one to
//     cContact.cpp and MainCalcCore.cpp only, and CMakeLists.txt is outside
//     this wave's write boundary).
// The IO wave has to supply: FileName, FileExists(), the CheckAndReadIniData
// calls, and golden's WriteFile()/WriteIniData() back-half.  The named
// Slk*Default*Csv() helpers below exist so it does not have to re-derive the
// defaults from golden.
//
// ---------------------------------------------------------------------------
// CONVENTION DEVIATION vs cContact.h: PREDICATES, NOT `int iCustomerCode`
// ---------------------------------------------------------------------------
// cContact.h's leaves take `int iCustomerCode` because cContact.cpp #includes
// MachineType.h and can compare against CC_*.  This TU cannot (see the
// -Wpedantic note above), so the customer/config reads golden performs are
// passed as already-evaluated BOOLEANS (`bCustomerAseSg` == `CUSTOMER_CODE ==
// CC_ASE_SG`, etc.).  Every one names its golden line so the caller can check
// it is evaluating the right predicate.
//
// ---------------------------------------------------------------------------
// FIELDS COVERED / NOT COVERED
// ---------------------------------------------------------------------------
// COVERED (SlkForceData already carries them): sDiameter, iTag, bShow,
//   dDiameter, dMinForce, dMaxForce, dLoadRate, dLoadRate_NS, dHotOffset,
//   dContactOffset, dContactOffset_NS.  That is every non-widget member of all
//   four golden classes (golden ContactForce.h:38-50, :66-75, :92-101,
//   :118-126) -- checked field by field 20260826.  Two of the eleven exist only
//   on THTSLKClass (dLoadRate_NS, dContactOffset_NS, golden :42/:46); the other
//   three classes leave them at their SlkForceData defaults, which is why the
//   Ind/DieForce Apply helper takes only the two fields those classes have.
// NOT COVERED, on purpose: every TGroupBox/TLabel/TEdit/TTrackBar pointer, and
//   the derived widget state golden writes from these numbers
//   (trckbr*->Position at :1068/:1070/:1086/:1101/:1157/:1173, edt*->Text at
//   :1072-1074/:1087/:1102/:1158, gb*->Visible at :205/:327/:418/:1559).  The
//   ONE piece of that state later arithmetic re-reads is the trackbar position,
//   so it is exposed as the pure function SlkTrackBarPosition().
// KNOWN DIVERGENCE, PRE-EXISTING, NOT CHANGED BY THIS WAVE: BCB6 zero-fills
//   TObject-derived instances on `new`, so golden's THTSLKClass leaves
//   dLoadRate/dLoadRate_NS at 0.0 when there is no ini file (its ctor never
//   assigns them -- golden :21-206 contains no such statement).  SlkForceData's
//   ctor above defaults them to 1.0 instead.  The SAFETY-relevant fields
//   (dContactOffset, dContactOffset_NS -- the two GetMinForce actually reads)
//   are 0.0 on BOTH sides, and dLoadRate is not read by (X-01)..(X-06) at all,
//   so this wave leaves the existing default alone rather than change a value
//   it cannot verify against a BCB6 binary.
// ===========================================================================

// ---------------------------------------------------------------------------
// SlkSplitCommaText
//   Delphi/BCB6 `TStringList::CommaText` SETTER semantics, used by golden at
//   :457/:461/:495/:499/:605/:609/:619/:623 to turn "30,40,60,56" into a list.
//
//   AI(W906-COMMATEXT-FU) 20261003: it IS vclcompat TStringList::CommaText now
//   (it was a copy of the old vclcompat parseDelimited) -- so the tree keeps ONE
//   CommaText behaviour rather than two.  Since INBOX 152 (71be4468) that is
//   BCB6's own parse (classes.pas SetDelimitedText): an unquoted field also ends
//   at a blank, like real Delphi -- the old gap is gone.  Every value this file
//   is fed is a bare digit CSV, so the lists themselves do not change.
// ---------------------------------------------------------------------------
std::vector<std::string> SlkSplitCommaText(const std::string& sCsv);

// ---------------------------------------------------------------------------
// SlkKitResolve -- the two locals golden's SLK walk produces, plus a flag.
//   Defaults reproduce golden's UN-WALKED state exactly:
//     iTag        = -1    golden cContact.cpp:18683
//     dKitDiameter= 30.0  golden cContact.cpp:18680
//   (golden then forces iTag to 0 at cContact.cpp:18761 -- that fix-up belongs
//   to the CALLER and is deliberately NOT applied here.)
// ---------------------------------------------------------------------------
struct SlkKitResolve
{
    bool   bMatched;        // a `d1==d2` hit happened (golden cContact.cpp:18720)
    int    iTag;            // golden local iTag
    double dKitDiameter;    // golden local dKitDiameter

    SlkKitResolve() : bMatched(false), iTag(-1), dKitDiameter(30.0) {}
};

// ---------------------------------------------------------------------------
// SlkForceTable
//   Runtime container standing in for ONE of golden's four vectors.
//   Value semantics (std::vector<SlkForceData>) rather than golden's
//   vector<T*>: golden's pointers exist because each element is a TComponent
//   that owns widgets and golden must delete them by hand (FormDestroy
//   :662-666).  With the widgets gone there is nothing to own.
//
//   WARNING: THE CTOR STUFFS FIELDS ONLY -- no loading, no allocation beyond
//   the empty vector.  See docs/KNOWLEDGE.md (88 of 134 ctests SEGFAULTed once
//   a ctor started reaching cross-TU objects at static-initialisation time).
// ---------------------------------------------------------------------------
class SlkForceTable
{
public:
    SlkForceTable();

    SlkVariant                variant;         // which golden class this mirrors
    bool                      bLoaded;         // a Load*() has run (see banner)
    bool                      bNeedWriteFile;  // golden global bNeedWriteFile,
                                               //   ContactForce.cpp:18/:426/:443
    std::vector<SlkForceData> items;           // golden vector<THT*SLKClass *>

    void   Clear();
    size_t size()  const { return items.size(); }
    bool   empty() const { return items.empty(); }

    // Golden cContact.cpp:18710-18727.  `sItemText` is
    // rgKitDiameter->Items->Strings[rgKitDiameter->ItemIndex] -- the CALLER
    // must first satisfy golden's :18707-18708 range guard
    // (ItemIndex>=0 && ItemIndex<Items->Count) and skip this call otherwise,
    // exactly as golden skips the loop.
    // Golden has NO `break`, so the LAST matching entry wins; preserved.
    SlkKitResolve ResolveKitDiameter(const std::string& sItemText) const;
};

// ---------------------------------------------------------------------------
// SlkForceTables
//   The four tables golden hangs off the `fContactForce` global
//   (golden ContactForce.h:341-356, :361 `extern PACKAGE TfContactForce
//   *fContactForce`).  There is no TfContactForce in this tree, so this struct
//   is the port's home for them.
//
//   WARNING: `ContactForceTables()` is a FUNCTION-LOCAL static (lazy,
//   first-use): nothing is constructed at static-initialisation time, so it
//   cannot join the SIOF lottery documented in docs/KNOWLEDGE.md.
//   WARNING: it starts EMPTY with bLoaded==false on all four.  It is NOT
//   self-loading and must never become self-loading -- loading needs disk IO,
//   which is the deferred half.  A caller that reads it without checking
//   bLoaded is reproducing exactly the silent-30.0 defect this wave exists to
//   prevent.
// ---------------------------------------------------------------------------
struct SlkForceTables
{
    SlkForceTable SLKClass;                  // golden ContactForce.h:341
    SlkForceTable SLKIndClass;               // golden ContactForce.h:342
    SlkForceTable DieForceSLKClass;          // golden ContactForce.h:349
    SlkForceTable DieForceOneByOneSLKClass;  // golden ContactForce.h:354

    // -----------------------------------------------------------------------
    // AI(W906-P2b-CF) 20260919: CSV 的**原始 token 數**。
    //
    // golden 的 `TransformFuntion` 拿 TStringList 的 Count 當迴圈上界：
    //     :350  for(i<fContactForce->slDieForceOneByOneSLKType->Count)
    //     :623  for(i<fContactForce->slSLKTypeInd->Count)
    //
    // ⚠ 那**不是**四張表的 size()。填充迴圈會丟掉空 token 與 atof<=15.0 的
    //   （golden :465 等），所以 `size() <= tokens`。兩者混用會讓迴圈走過頭
    //   或走不夠 —— 名字裡寫 `Tokens` 就是為了讓下一個人不會拿錯。
    //
    // 由 `LoadContactForceTables()` 填；`Clear()` 不會動它們（它清的是表），
    // 所以驅動自己在重載時一併重設。
    // -----------------------------------------------------------------------
    int iSlkTypeIndTokens;      // golden slSLKTypeInd->Count
    int iDieForceTypeTokens;    // golden slDieForceOneByOneSLKType->Count

    SlkForceTables();
};

SlkForceTables& ContactForceTables();

// ---------------------------------------------------------------------------
// Ini DEFAULTS golden passes to CheckAndReadIniData.  Exposed as named
// constants so the deferred IO wave does not have to re-derive them.
//   SlkDefaultTypeCsv             golden :439 (KYEC) / :448
//   SlkDefaultVisibleCsv          golden :440 (KYEC) / :449
//   SlkDefaultTypeIndCsv          golden :452
//   SlkDefaultVisibleIndCsv       golden :453
//   SlkDefaultDieForceTypeCsv     golden :579 / :591
//   SlkDefaultDieForceVisibleCsv  golden :582 (SPIL) / :586 / :592
// ---------------------------------------------------------------------------
const char* SlkDefaultTypeCsv(bool bCustomerKyecLee);
const char* SlkDefaultVisibleCsv(bool bCustomerKyecLee);
const char* SlkDefaultTypeIndCsv();
const char* SlkDefaultVisibleIndCsv();
const char* SlkDefaultDieForceTypeCsv();
const char* SlkDefaultDieForceVisibleCsv(bool bSpilFunction);

// ---------------------------------------------------------------------------
// SlkComputeNeedWriteFile -- golden :437-445.
//   KYEC-only: if the type CSV STARTS WITH "30" or "60" the machine is still on
//   the pre-conversion diameters and golden latches bNeedWriteFile.
//   1-based Pos() conversion: golden's `Text.Pos("30")==1` means "starts with"
//   (AnsiString::Pos is 1-based and returns 0 when absent), so the std port is
//   `find("30")==0` -- NOT `find(...)==npos`, and NOT "contains".
//   Non-KYEC callers always get false (golden's :446 else-arm never latches).
// ---------------------------------------------------------------------------
bool SlkComputeNeedWriteFile(const std::string& sTypeCsv, bool bCustomerKyecLee);

// ---------------------------------------------------------------------------
// SlkLoadOptions -- the config/customer reads golden's SLKClass fill performs.
// ---------------------------------------------------------------------------
struct SlkLoadOptions
{
    int  iEpInstall;        // EP_Install                  golden :471 (and :181)
    bool bCustomerAseSg;    // CUSTOMER_CODE==CC_ASE_SG    golden :483
    bool bCustomerKyecLee;  // CUSTOMER_CODE==CC_KYEC_LEE  golden :437 (defaults only)

    SlkLoadOptions() : iEpInstall(0), bCustomerAseSg(false), bCustomerKyecLee(false) {}
};

// ---------------------------------------------------------------------------
// LoadSlkTable -- golden ContactForce.cpp:455-491 (the SLKClass fill).
//   `sTypeCsv`    == golden edtCurrentType->Text  (already read from ini)
//   `sVisibleCsv` == golden edtVisible->Text      (already read from ini)
//
//   Faithful to every branch of golden's loop:
//     :465  skip empty tokens and tokens whose atof() is <= 15.0
//     :467  the token "402" is constructed as "40x2" (the 40mm dual-socket
//           sentinel; ComputeSlkForce then maps it to dDiameter 402.0)
//     :471  EP_Install==5 pushes TWO entries per token, "Arm1_<t>" and
//           "Arm2_<t>", BOTH with tag i.  Note the consequence, which is
//           golden's and is preserved: ParseDiameter_Standard strips the prefix
//           only when "Arm2_" is ABSENT, so the Arm2_ entry parses to 0.0.
//     :483  CC_ASE_SG additionally pushes an "80_Hi" entry, tag i+1, when the
//           token is exactly "80".
//   Also sets table.bNeedWriteFile from SlkComputeNeedWriteFile (golden :441).
//
//   DEVIATION, cited: golden indexes slSLKTypeVisible->Strings[i] with i
//   running over slSLKType, so a Visible CSV SHORTER than the Type CSV raises
//   EStringListError in BCB6.  This port cannot raise a VCL exception; a
//   missing token is treated as "" (=> atoi 0 => bShow false), i.e. the entry
//   is still created but hidden.  Chosen over silently defaulting to VISIBLE.
// ---------------------------------------------------------------------------
void LoadSlkTable(SlkForceTable& table,
                  const std::string& sTypeCsv,
                  const std::string& sVisibleCsv,
                  const SlkLoadOptions& opt);

// ---------------------------------------------------------------------------
// SlkIndLoadOptions -- golden's reads inside the SLKIndClass fill.
// ---------------------------------------------------------------------------
struct SlkIndLoadOptions
{
    bool bDoubleEpMulti;     // INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI  golden :501
    int  iIndEPCnt;          // iIndEPCnt                           golden :522/:528
    bool bTestModeDualSite;  // TestIF_File.iTestMode==DualSite     golden :523

    SlkIndLoadOptions() : bDoubleEpMulti(false), iIndEPCnt(0), bTestModeDualSite(false) {}
};

// ---------------------------------------------------------------------------
// LoadSlkIndTable -- golden ContactForce.cpp:493-539 (the SLKIndClass fill).
//   `sTypeIndCsv`    == golden edtCurrentTypeInd->Text
//   `sVisibleIndCsv` == golden edtVisibleInd->Text
//   `sVisibleCsv`    == golden edtVisible->Text -- the *SLKClass* visible list.
//
//   THAT THIRD ARGUMENT IS NOT A TYPO AND MUST NOT BE "FIXED".  golden :509
//   (the DOUBLE_EP_MULTI arm) reads slSLKTypeIndVisible, but golden :534 (the
//   normal arm) reads slSLKTypeVisible -- the list built from edtVisible at
//   :459-461, not the Ind one built at :497-499.  Read on the golden source
//   20260826.  Both are passed so the quirk is reproduced exactly and stays
//   visible to the next reader.
//
//   Tags: 8 slots per type under MULTI (:509, i*8+j), 16 otherwise (:526/:530/
//   :534, i*16+j).  The `bShow=false` arms at :522-531 are the closed-EP slots.
// ---------------------------------------------------------------------------
void LoadSlkIndTable(SlkForceTable& table,
                     const std::string& sTypeIndCsv,
                     const std::string& sVisibleIndCsv,
                     const std::string& sVisibleCsv,
                     const SlkIndLoadOptions& opt);

// ---------------------------------------------------------------------------
// SlkDieForceVisibleCsvForSpil -- golden ContactForce.cpp:595-601.
//   SPIL-only post-adjustment applied to the ALREADY-READ visible CSV: when the
//   DieForce type CSV is exactly "20,30,40,50", force "1,1,1,1".
//   (golden :580-587's SPIL branch only picks which DEFAULT to hand
//   CheckAndReadIniData; that is the IO half -- see SlkDefaultDieForceVisibleCsv.)
// ---------------------------------------------------------------------------
std::string SlkDieForceVisibleCsvForSpil(const std::string& sDieForceTypeCsv,
                                         const std::string& sDieForceVisibleCsv,
                                         bool bSpilFunction);

// ---------------------------------------------------------------------------
// LoadDieForceSlkTable -- golden ContactForce.cpp:603-615.
//   One entry per type token that is non-empty and > 15.0, tag i.
//   Caller gates on golden's :575 `INSTALL_DOUBLE_EP>0`.
// ---------------------------------------------------------------------------
void LoadDieForceSlkTable(SlkForceTable& table,
                          const std::string& sDieForceTypeCsv,
                          const std::string& sDieForceVisibleCsv);

// ---------------------------------------------------------------------------
// LoadDieForceOneByOneSlkTable -- golden ContactForce.cpp:617-636.
//   EIGHT entries per type token, tag i*8+j.  Caller gates on golden's :575
//   AND :624 (INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI).
//
//   golden iterates slDieForceOneByOneSLKType (:626) but tests and reads
//   slDieForceSLKType (:630/:632).  Harmless -- :605 and :619 assign BOTH from
//   the same edtDieForceCurrentType->Text -- so the port takes ONE type CSV and
//   says so here rather than inventing a second parameter that can never
//   differ.  The VISIBLE list, however, genuinely is the OneByOne one (:632).
// ---------------------------------------------------------------------------
void LoadDieForceOneByOneSlkTable(SlkForceTable& table,
                                  const std::string& sDieForceTypeCsv,
                                  const std::string& sDieForceOneByOneVisibleCsv);

// ---------------------------------------------------------------------------
// Ini GROUP-NAME derivation (golden ReadFile).  Pure string building: these
// tell the deferred IO wave which [Section] to read for each entry.
//
//   SlkIniGroupName  golden :1040-1060.  Three arms:
//     :1040  CC_ASE_SG && dDiameter==80 -> "Diameter_%0.3fmm" for the FIRST
//            such entry and "Diameter_%0.3f_Himm" for every later one.
//            `bHasDiameter` is golden's function-scope latch (:967) and MUST be
//            carried across the whole walk by the caller -- pass the same bool
//            in for every i, initialised false.
//            That first arm ALSO writes tsContactHighOffset->TabVisible =
//            entry.bShow (:1046); returned through
//            `pbContactHighOffsetTabVisible` (may be null) rather than touching
//            a widget.
//     :1053  EP_Install==5 -> "Diameter_%smm" using the RAW sDiameter token
//            ("Arm1_30" / "Arm2_30"), not the parsed number.
//     :1057  otherwise      -> "Diameter_%0.3fmm" using dDiameter.
//
//   DieForceSlkIniGroupName            golden :1081
//   DieForceOneByOneSlkIniGroupName    golden :1093-1096 (j<4 -> Arm1_%d with
//                                      j+1, else Arm2_%d with j-3)
//   SlkIndIniGroupName                 golden :1149-1152 (MULTI, same Arm1/Arm2
//                                      split) and :1168 (normal, "_%d" with j)
// ---------------------------------------------------------------------------
std::string SlkIniGroupName(const SlkForceData& entry,
                            bool  bCustomerAseSg,
                            int   iEpInstall,
                            bool& bHasDiameter,
                            bool* pbContactHighOffsetTabVisible);

std::string DieForceSlkIniGroupName(const SlkForceData& entry);
std::string DieForceOneByOneSlkIniGroupName(const SlkForceData& entry, int j);
std::string SlkIndIniGroupName(const SlkForceData& entry, int j, bool bDoubleEpMulti);

// ---------------------------------------------------------------------------
// Applying ALREADY-READ ini values.  The clamps are golden's, via the
// MachineType.h:1519-1540 `CheckRange(Value, Maximum, Minimum)` template --
// which tolerates SWAPPED bounds, and golden DOES swap them (it writes
// CheckRange(v, 0.8, 1.5), i.e. Maximum=0.8 < Minimum=1.5).  Re-derived from
// the template body 20260826, not assumed.  CheckRange itself is NOT called
// here: pulling in MachineType.h would cost this TU its dependency-free
// property (see banner).
//
//   ApplySlkIniValues       golden :1061-1065
//     LoadRate/LoadRate_NS  clamp [0.8, 1.5]    (ini default 1.0)
//     HotOffset             clamp [-0.5, 0.5]   (ini default 0.0)
//     ContactOffset(_NS)    clamp [-10.0, 10.0] (ini default 0.0)
//   ApplySlkPairIniValues   golden :1082-1083 (DieForce, ContactOffset ini
//                           default 1.0), :1097-1098 (DieForce one-by-one,
//                           default 0.0), :1153-1154 and :1169-1170 (Ind,
//                           default 0.0).  All four use the SAME two clamps, so
//                           they share one function; only the ini DEFAULTS
//                           differ and those belong to the deferred IO call.
//   SlkTrackBarPosition     golden :1067-1068 --
//                           `dTemp=dLoadRate*(double)100.0; Position=int(dTemp);`
//                           The redundant-looking cast is golden's, with
//                           golden's comment about forcing double first; kept,
//                           and the TRUNCATION (int(), not round()) with it.
// ---------------------------------------------------------------------------
struct SlkIniValues
{
    double dLoadRate;
    double dLoadRate_NS;
    double dHotOffset;
    double dContactOffset;
    double dContactOffset_NS;

    SlkIniValues()
        : dLoadRate(1.0), dLoadRate_NS(1.0), dHotOffset(0.0),
          dContactOffset(0.0), dContactOffset_NS(0.0) {}
};

void ApplySlkIniValues(SlkForceData& entry, const SlkIniValues& raw);
void ApplySlkPairIniValues(SlkForceData& entry, double dLoadRate, double dContactOffset);
int  SlkTrackBarPosition(double dLoadRate);

// ---------------------------------------------------------------------------
// ApplyKyecDiameterRemap -- golden ContactForce.cpp:1112-1125, MEMORY HALF ONLY.
//   30 -> 28 and 60 -> 58, sDiameter rewritten to match, and EVERY entry forced
//   bShow=true (:1124).  Clears table.bNeedWriteFile, mirroring golden :1111.
//
//   WHAT IS NOT HERE, and why it matters: golden's same block then PERSISTS the
//   remap -- edtCurrentType/edtVisible rewritten and WriteIniData()'d
//   (:1126-1129), rgKitDiameter->Items rebuilt (:1131-1138), and WriteFile()
//   (:1139).  All of that is disk IO / widget state and is deferred with the
//   rest of the IO half.  Consequence a caller must understand: calling this
//   alone remaps the IN-MEMORY table but leaves ContactInfo.ini saying
//   "30,40,60,56", so the remap would repeat on every load instead of being a
//   one-shot migration.  Golden is one-shot.
// ---------------------------------------------------------------------------
void ApplyKyecDiameterRemap(SlkForceTable& table);

#endif // CONTACTFORCE_H
