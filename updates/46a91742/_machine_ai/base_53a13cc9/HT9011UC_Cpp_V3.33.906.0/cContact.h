// cContact.h
// Standard C++ translation of BCB6 cContact.h (V3.33.906.0_20260618) -- CALC CORE ONLY.
// Source of truth: HT9011UC_Code_V3.33.906.0_20260618/cContact.h (declarations, lines 607-609,
//                   612/dMinForce is actually at line 619; see note below), and
//                   HT9011UC_Code_V3.33.906.0_20260618/cContact.cpp (bodies, lines 18893-19022).
//
// AI(W64b-ForceCalc-Translate) 20260710: new file. Extract-calc-core translation of the two
// pure-math TfContact member functions used by CalculateTotalAirForce (golden :18675-18891,
// itself NOT translated here -- see scope note below).
//
// Translation scope: cContact.cpp is a 22761-line VCL form (TfContact) mixing dense switch-case
// state machines with VCL widgets (TRadioGroup, TEdit, TLabel, ...) and global config reads.  Per
// the project's established extract-calc-core convention (see ContactForce.h/.cpp -- the DIRECT
// PRECEDENT for this file, same SLK/contact-force domain), this wave translates ONLY:
//
//   1. TfContact::GetMaxIndexForceLimit()                    (golden cContact.cpp:18893-18955)
//   2. TfContact::GetMinForce(double dKitDiameter, int iTag) (golden cContact.cpp:18957-19022)
//
// as FREE FUNCTIONS that take every external read (class members, global config, and the one VCL
// widget selection GetMaxIndexForceLimit reads) as explicit parameters, and RETURN the computed
// value instead of mutating class state.  TfContact itself, the global fContactForce instance, and
// the rgKitDiameter VCL radio group do NOT exist in the translated tree; this file does not create
// them.  CalculateTotalAirForce (golden :18675-18891, VCL-widget-heavy caller) and everything else
// in cContact.cpp/.h/.dfm is OUT OF SCOPE for this wave -- see MIGRATION_ROADMAP.md's cContact
// DEFERRED entry (TfContact::DoTestContactFunction's CarlibrationTask + 6 sub-SMs, ~11347 lines).
//
// ---------------------------------------------------------------------------------------------
// DEFERRED note on dMinForce member-persistence semantics (read before wiring up a future
// full-TfContact translation):
//
//   Golden TfContact::GetMinForce mutates AND returns the class member `double dMinForce;`
//   (golden cContact.h:619) in every branch.  Grepping the whole golden 906 tree for
//   `GetMinForce\|GetMaxIndexForceLimit\|dMinForce` shows:
//     * The ONLY call site of GetMinForce anywhere in the golden tree is
//       cContact.cpp:18763 (inside CalculateTotalAirForce): `dMinForce=GetMinForce(dKitDiameter,
//       iTag);` -- i.e. it just re-assigns the SAME member from the function's return value.
//       Treating GetMinForce as a pure function (local variable in, local variable out, no member
//       side-effect) is therefore 100% behaviorally faithful for THAT call site.
//     * The member `dMinForce` is READ again later, independent of any GetMinForce call, at
//       cContact.cpp:5650 and :8414 (`ADAM_WriteVoltage(dMinForce/2.0);` inside the Z1/Z2
//       auto-get-height routines) -- both untranslated/out of scope this wave.
//     * This prompt's scoping guidance also cited a AutoClean/uCleaning.cpp:1741 dMinForce read;
//       that line/reference does NOT exist in this golden 906 snapshot (uCleaning.cpp has zero
//       occurrences of `dMinForce`) -- GOLDEN WINS, so that citation is dropped as a discrepancy
//       (documented in the wave's final report, not fixed here).
//   CONCLUSION: a future wave that translates the REAL TfContact class + CalculateTotalAirForce +
//   the Z1/Z2 auto-get-height routines will still need to wire up `dMinForce` as a persistent
//   member (assigned once per CalculateTotalAirForce call, read later by the unrelated Z1/Z2
//   routines) -- that persistence is NOT modeled by the free function below and is explicitly
//   OUT OF SCOPE / DEFERRED, not "solved" by returning a value instead of a member.
// ---------------------------------------------------------------------------------------------
//
// BCB6 source lines cited inline in cContact.cpp (this translated file).

#ifndef CCONTACT_H
#define CCONTACT_H

#include "MachineType.h"   // eIndexPressType, eTestMode constants (already translated, header-only)

// ---------------------------------------------------------------------------
// AI(W906-cContactLeaf) 20260721: contact-mode / index-down-pos constants the
// leaves below need.  Golden home: cContact.h:20-28 (`extern const int ...`,
// values defined cContact.cpp:73-90) -- TfContact-adjacent free consts, NOT
// class members.
//
// AI(W906-FW-CONTACT-W28) 20260826: CONSOLIDATION PASS -- 8 of golden's 12
// contact-mode constants added here, so this header becomes the tree's single
// real home for them.  Read the two sections below before adding a 13th copy
// anywhere.
//
// ---------------------------------------------------------------------------
// DENOMINATOR -- RE-MEASURED THIS WAVE, NOT INHERITED
// ---------------------------------------------------------------------------
// Golden cContact.cpp:74-85 is a block of TWELVE `const int` contact-mode
// constants (:73 `iSuckDelay` and :87-88 PICK_UP_OFFSET/CONTACT_UP_OFFSET are
// adjacent but are NOT contact modes and are not in scope here).  Prior notes
// in this tree -- including forms/fContact.h's GATE (X-10) -- called it "the
// ten-constant block".  That count was wrong: it silently dropped
// CONTACT_IN_SHUTTLE_CHECK(6) and CONTACT_OUT_SHUTTLE_CHECK(7), which golden
// mirrors as `extern const int` at its own cContact.h:25-26 exactly like the
// rest.  Counted here by reading golden cContact.cpp:73-92 directly
// (20260826).  The real figures for this file are therefore:
//     golden contact-mode constants           12
//     present in this header before this wave  3   (CONTACT_NORMAL,
//                                                   CONTACT_MANUAL_GET_HEIGHT,
//                                                   CONTACT_DEVICE_MAP_CHECK)
//     added by this wave                       8
//     still absent                             1   (CONTACT_TEST -- see below)
//
// ---------------------------------------------------------------------------
// HAZARD -- FIVE OTHER PARTIAL COPIES EXIST, AND ONE OF THEM BLOCKS THE 12th
// ---------------------------------------------------------------------------
// A definition-shaped scan of all 1,578 .h/.hpp/.c/.cpp files in the port tree
// (comments AND string literals stripped by a character state machine, not a
// regex; 20260826 18:0x) found these copies of the family, all of them plain
// namespace-scope `const int`:
//
//   cContact.h              (this file)                CONTACT_NORMAL,
//                                                      CONTACT_MANUAL_GET_HEIGHT,
//                                                      CONTACT_DEVICE_MAP_CHECK
//   ATC/ATCInterface.cpp:206                           CONTACT_NORMAL
//   AutoClean/AutoClean.cpp:125  (`static const int`)  CONTACT_DEVICE_MAP_CHECK
//   Command.cpp:317                                    CONTACT_TEST
//   uTemp_Set.cpp:174                                  CONTACT_TEST
//   BarCode/BarCode_Shuttle2_CCDScan.h:187-189         CONTACT_NORMAL,
//                                                      CONTACT_AUTO_GET_HEIGHT,
//                                                      CONTACT_TEST
//
// So the fork count is SIX sites, not the "four" this tree's earlier notes
// recorded (they missed ATCInterface.cpp, AutoClean.cpp and uTemp_Set.cpp).
// Every one of them carries some variant of a "no ODR risk, `const int` at
// namespace scope has internal linkage" note.  That reasoning is correct about
// LINK time and irrelevant at COMPILE time: two definitions of the same name at
// namespace scope in ONE translation unit is a hard redefinition error whatever
// the linkage.
//
// ⚠ WHY CONTACT_TEST IS NOT IN THE LIST BELOW.
//   Which forks actually collide depends entirely on which TUs see this header.
//   Measured this wave with an include-closure walk that HONOURS `#if 0`
//   (a first pass that did not honour it reported 164 TUs and was wrong --
//   MachineDefine.h:137 does `#include "cContact.h"`, but MachineDefine.h:34-146
//   is one big `#if 0` block, so that edge does not exist):
//       TUs whose live include closure contains cContact.h -- SIX:
//         Command.cpp, ainarm9045.cpp, atester_shims.cpp, cContact.cpp,
//         forms/fContact.cpp, tests/test_cContact.cpp
//       TUs whose closure contains BarCode/BarCode_Shuttle2_CCDScan.h -- THREE:
//         BarCode/BarCode_Shuttle2_CCDScan.cpp, aHotPlateSubstrate.cpp,
//         tests/test_barcode_shuttle2_ccdscan.cpp
//       TUs containing BOTH -- ZERO.
//   Command.cpp is in the first list (it includes this header at its :239) AND
//   defines its own `const int CONTACT_TEST = 3;` at its :317.  Adding
//   CONTACT_TEST here therefore BREAKS Command.cpp, which is outside this
//   wave's write boundary.  Verified by compiler, not by reasoning: with
//   CONTACT_TEST present, `g++ -fsyntax-only` on Command.cpp fails with
//   `error: redefinition of 'const int CONTACT_TEST'`.  It was removed again
//   and the tree left compiling.  uTemp_Set.cpp:174 and
//   BarCode_Shuttle2_CCDScan.h:189 also define it but neither TU sees this
//   header, so neither is a blocker today.
//
//   RETIREMENT ORDER for whoever owns those files (one line each, and the
//   FIRST one is what un-gates forms/fContact.h's (X-10)/(X-11)/(X-32)):
//     1. delete Command.cpp:317      -- it already includes cContact.h at :239,
//                                       so it just inherits the value.
//        THEN add `const int CONTACT_TEST = 3;  // golden cContact.cpp:77`
//        to the block below, in golden's slot between
//        CONTACT_MANUAL_GET_HEIGHT and AUTO_CONTACT_TEST.
//     2. delete uTemp_Set.cpp:174    -- add `#include "cContact.h"` there.
//     3. delete BarCode/BarCode_Shuttle2_CCDScan.h:187-189 -- add
//        `#include "cContact.h"` there.  Do 3 only together with 1, or that
//        header's CONTACT_TEST re-collides through any TU that gains both.
//     4. delete ATC/ATCInterface.cpp:206 and AutoClean/AutoClean.cpp:125 --
//        add `#include "cContact.h"` to each.  Neither TU sees this header
//        today, so both are pure tidy-ups with no compile consequence.
//   Steps 1-4 are all outside THIS wave's write boundary (cContact.{h,cpp} and
//   forms/fContact.{h,cpp} only), which is why they are written down rather
//   than done.
// ---------------------------------------------------------------------------
const int CONTACT_NORMAL                   = 0;   // golden cContact.cpp:74
const int CONTACT_AUTO_GET_HEIGHT          = 1;   // golden cContact.cpp:75  -- added W28
const int CONTACT_MANUAL_GET_HEIGHT        = 2;   // golden cContact.cpp:76 -- added W906-cContactLeaf for ComputeAutoHeightMaxKGTorque
// golden cContact.cpp:77 `const int CONTACT_TEST = 3;` -- DELIBERATELY ABSENT.
// Blocked by Command.cpp:317; see the HAZARD section above for the measurement
// and the one-line retirement.  Do NOT add it without deleting that line in the
// same change.
const int AUTO_CONTACT_TEST                = 4;   // golden cContact.cpp:78  -- added W28  //Steven 20150224 : Auto Contact Test
const int STEP_CONTACT_TEST                = 5;   // golden cContact.cpp:79  -- added W28  //Steven 20150811 : Step by Step Contact Test
const int CONTACT_IN_SHUTTLE_CHECK         = 6;   // golden cContact.cpp:80  -- added W28
const int CONTACT_OUT_SHUTTLE_CHECK        = 7;   // golden cContact.cpp:81  -- added W28
const int CONTACT_LoadCell_AUTO_GET_HEIGHT = 8;   // golden cContact.cpp:82  -- added W28  //kevin 20190909 add Load cell AutoHigh
const int CONTACT_DEVICE_MAP_CHECK         = 9;   // golden cContact.cpp:83 -- added W906-cContactLeaf for ComputeIndexYSpeed/ComputeIndexZSpeed
const int CONTACT_DEVICE_LOOP_TEST         = 10;  // golden cContact.cpp:84  -- added W28  //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST
const int K_TEMP_INDEX_MOVE                = 11;  // golden cContact.cpp:85  -- added W28  //Ztex 2024.03.26 Add Contact Mode K Temperature

// golden cContact.cpp:90-92 (file-scope `const double`, TfContact-adjacent, NOT class members).
const double fIndexDownPos_for9045   = -135.0;                            // golden cContact.cpp:90
const double fIndexDownPos_forATC    = -146.0;   //wei 20160303 HT-9045HA -146     // golden cContact.cpp:91
const double fIndexDownPos_for9046LS = -148.0;   //ChungHung 20140604 LS AutoHeight // golden cContact.cpp:92

// ---------------------------------------------------------------------------
// ComputeMaxIndexForceLimit
//   Portable replacement for TfContact::GetMaxIndexForceLimit().
//   BCB6 source: cContact.cpp:18893-18955
//
//   Parameters (every external read the golden body performs, as an explicit param):
//     indexPressType         -- global INDEX_PRESS_TYPE            (cmydef.h/.cpp, eIndexPressType)
//     dDutCount               -- TfContact member double dDutCount  (cContact.h:555)
//     iTestMode               -- global TestIF.iTestMode            (int; compared against eTestMode
//                                 constants SingleSite/DualSite/QualSite2X2N, MachineType.h)
//     bD27UseSingleSite85kg   -- global IniConfig.bD27UseSingleSite85kg (Config.h, bool)
//     iKitDiameterItemIndex   -- VCL rgKitDiameter->ItemIndex (int; TRadioGroup selection index)
//
//   Faithfully preserves the fixed-kg if/else-if ladder over indexPressType, and the else-branch's
//   nested condition + its own inner if/else-if (ItemIndex==1 -> 55, ItemIndex==2 -> 85) with
//   NO trailing else -- this is a REAL golden gap: any other ItemIndex value inside that inner
//   branch leaves dMaxLimit at its prior value, which is 0.0 (the local's initializer), NOT a
//   sensible default.  Do not "fix" this gap; it is preserved on purpose (see inline comment in
//   the .cpp body).
// ---------------------------------------------------------------------------
double ComputeMaxIndexForceLimit(eIndexPressType indexPressType,
                                  double dDutCount,
                                  int iTestMode,
                                  bool bD27UseSingleSite85kg,
                                  int iKitDiameterItemIndex);

// ---------------------------------------------------------------------------
// ComputeMinForce
//   Portable replacement for TfContact::GetMinForce(double dKitDiameter, int iTag).
//   BCB6 source: cContact.cpp:18957-19022
//
//   Parameters:
//     dKitDiameter        -- unchanged golden param (double)
//     dTagContactOffset    -- fContactForce->SLKClass[iTag]->dContactOffset    (already-resolved
//                             scalar; see ContactForce.h's SlkForceData::dContactOffset)
//     dTagContactOffsetNS  -- fContactForce->SLKClass[iTag]->dContactOffset_NS (see
//                             SlkForceData::dContactOffset_NS)
//     bNSKitPress          -- global TestIF_File.bNSKitPress (cprod.h, bool)
//     bD04MinForceByFile   -- global IniConfig.bD04MinForceByFile (Config.h, bool)
//     dD04MinForceByFile{,_20mm,_30mm,_40mm,_60mm,_80mm}
//                          -- global IniConfig.dD04MinForceByFile* family (Config.h, double)
//
//   Faithfully preserves every diameter bucket (20 / 30-or-28 / 40 / 60-or-58 / 80 / else) and the
//   "only override if the file-configured min is LARGER" pattern (`if(dMinForce<X) dMinForce=X`)
//   in every bucket, and the else-branch's bNSKitPress selection (dContactOffset_NS vs
//   dContactOffset) before its own bD04MinForceByFile override.
//
//   NOTE: the golden function mutates+returns the TfContact member dMinForce (golden cContact.h:619,
//   NOT line ~612 as a rough estimate might suggest).  This free function returns a LOCAL value
//   instead of mutating a member -- see the DEFERRED note above this header's include guard for why
//   that is faithful for the one real call site, and what remains deferred for other call sites.
// ---------------------------------------------------------------------------
double ComputeMinForce(double dKitDiameter,
                        double dTagContactOffset,
                        double dTagContactOffsetNS,
                        bool bNSKitPress,
                        bool bD04MinForceByFile,
                        double dD04MinForceByFile,
                        double dD04MinForceByFile_20mm,
                        double dD04MinForceByFile_30mm,
                        double dD04MinForceByFile_40mm,
                        double dD04MinForceByFile_60mm,
                        double dD04MinForceByFile_80mm);

// ===========================================================================
// AI(W906-FW-CONTACT-W28) 20260826: ComputeTotalAirForce -- the calc core of
// TfContact::CalculateTotalAirForce, golden cContact.cpp:18675-18891 (217
// lines).  This is the EP air-force setpoint calculation: golden's caller
// (ShowArmAndDeviceForce, :1908-1909) stores the result in `iTotalGf` and then
// in `DeviceForm.dPress`, which the gated edAirForceChange (:2268-2273) pushes
// to the regulator via ADAM_WriteVoltage.  Treat changes here as
// safety-relevant even though this function itself touches no hardware.
//
// ---------------------------------------------------------------------------
// ⚠ CORRECTION TO THE BRIEF THIS WAVE WAS GIVEN: NOT "217 LINES OF PURE
//   ARITHMETIC"
// ---------------------------------------------------------------------------
// The golden body is arithmetic PLUS three kinds of external effect, all read
// off the golden source line by line (20260826):
//   (a) SEVEN VCL widget writes -- edAirForce/edAirKPA/edSetKg/
//       edForcePerDeviceKG/edForcePerDeviceN ->Color (:18730-18734, :18746-
//       18749, :18874-18877, :18886), lblMaxForcePerIC ->Visible/->Caption
//       (:18738/:18742/:18756), lblMinForce->Caption (:18881), and
//       edForcePerDeviceKG/N ->Text (:18750-18751).
//   (b) TWO widget READS -- scrbSLK->Position (:18693) and rgKitDiameter
//       ->ItemIndex/->Items (:18707-18714).
//   (c) THREE non-local writes -- the members dDutCount (:18689) and dMinForce
//       (:18763, :18868), and the GLOBAL IniConfig.iEP_Min_KG (:18857).
// Per this file's established extract-calc-core convention every one of those
// is a parameter or a result field here; nothing is read or written directly.
//
// ---------------------------------------------------------------------------
// ⚠ THE PART THAT IS NOT TRANSLATED, AND WHY IT BLOCKS THE CALLERS
// ---------------------------------------------------------------------------
// Golden :18710-18727 walks `fContactForce->SLKClass` to turn the selected
// rgKitDiameter item into (iTag, dKitDiameter):
//     for(unsigned i=0; i<fContactForce->SLKClass.size(); i++)
//         { d1 = SLKClass[i]->dDiameter; ... if(d1==d2) { iTag=i; dKitDiameter=d2; } }
// The port has the ELEMENT type (ContactForce.h `struct SlkForceData`, with
// dDiameter/dContactOffset/dContactOffset_NS) and the element FACTORY
// (`ComputeSlkForce`), but it has NO RUNTIME CONTAINER and no `fContactForce`
// global -- re-verified this wave, see the ABSENCE RE-RUN in forms/fContact.h.
// So that loop cannot be translated yet, and this function takes its two
// results -- dKitDiameter and the chosen entry's two contact offsets -- as
// CALLER-SUPPLIED inputs.  That is the identical split ComputeMinForce already
// uses (it takes dTagContactOffset/dTagContactOffsetNS rather than indexing
// SLKClass[iTag]), so this file stays dependency-free.
//
// ⚠ DO NOT "SOLVE" THIS BY LETTING THE CALLER PASS THE DEFAULTS.  If a caller
// simply skips the loop, golden's locals keep their initialisers -- iTag=-1
// (:18683) which :18761 then forces to 0, and dKitDiameter=30.0 (:18680).
// A hardcoded 30.0 is NOT an inert default: it actively selects the 30mm arm of
// ComputeMinForce (dMinForce=1.5), it feeds the :18740 area formula
// (((30*30*3.14)/4)*coef*0.0101972) that CLAMPS the per-device force under
// IniConfig.bD28MaxForceLimitByDiameter, and it satisfies the ASE-Kaohsiung
// `if(dKitDiameter==30) dMinForce=1;` at :18866-18869.  The result would be a
// plausible-looking but wrong EP setpoint for every kit that is not 30mm.
// This is why forms/fContact.h keeps (X-01)..(X-06) gated even now that the
// arithmetic below exists: the five read-only callers cannot be delivered until
// something in the tree can answer "which SLK entry is selected?".
// ===========================================================================

// ---------------------------------------------------------------------------
// EForceFieldColor
//   Final colour of the four "value" edits golden recolours together --
//   edAirKPA, edSetKg, edForcePerDeviceKG, edForcePerDeviceN.
//   Modelled as ONE last-write-wins field rather than separate red/yellow
//   booleans because golden can genuinely write both in one call: :18746-18749
//   paints them red inside the bD28 clamp, and :18874-18877 can then repaint
//   the SAME four yellow inside the min-force branch.  Two booleans would lose
//   the ordering; the enum keeps it.
//   Note the ASE-Kaohsiung arm (:18863-18871) sets NO colour at all, so a red
//   from :18746 survives it -- which this model reproduces.
// ---------------------------------------------------------------------------
enum EForceFieldColor
{
    efcWhite  = 0,   // golden :18731-18734 (the unconditional reset)
    efcRed    = 1,   // golden :18746-18749
    efcYellow = 2    // golden :18874-18877
};

// ---------------------------------------------------------------------------
// TotalAirForceIn -- every external READ golden's body performs.
//
//   DEVIATION from this file's other five leaves: they take flat parameters.
//   CalculateTotalAirForce reads 24 distinct externals, and a 24-parameter
//   positional list is a call-site hazard (every one of the nine bools would be
//   silently swappable).  Grouped into a struct with designated field names
//   instead; the CONVENTION -- no global is read inside cContact.cpp -- is
//   unchanged.
// ---------------------------------------------------------------------------
struct TotalAirForceIn
{
    // -- widget reads -------------------------------------------------------
    int    iSlkPosition;            // scrbSLK->Position            golden :18693
    int    iKitDiameterItemIndex;   // rgKitDiameter->ItemIndex     golden :18707 and,
                                    //   via GetMaxIndexForceLimit, :18946-18951

    // -- results of golden's untranslated fContactForce->SLKClass walk ------
    //    (:18710-18727).  See the block comment above before supplying these.
    double dKitDiameter;            // golden local, init 30.0      golden :18680/:18723-18725
    double dTagContactOffset;       // SLKClass[iTag]->dContactOffset     (ContactForce.h)
    double dTagContactOffsetNS;     // SLKClass[iTag]->dContactOffset_NS  (ContactForce.h)

    // -- globals ------------------------------------------------------------
    int    iTestMode;               // TestIF.iTestMode             golden :18686/:18765
    bool   bQualSite2X2Shift;       // TestIF_File.bQualSite2X2Shift      golden :18687
    bool   bNS7000kit;              // TestIF_File.bNS7000kit             golden :18688
    bool   bOctal_12Kit;            // TestIF_File.bOctal_12Kit           golden :18819
    bool   bNSKitPress;             // TestIF_File.bNSKitPress   (via GetMinForce :19006)
    int    iCloseSiteModeFor2x8;    // iCloseSiteModeFor2x8               golden :18839
    int    iCustomerCode;           // CUSTOMER_CODE                      golden :18863
    double dEpMaxKpa;               // EP_MAXKPA (cmydef.h:2811, double)  golden :18739
    eIndexPressType indexPressType; // INDEX_PRESS_TYPE  (via GetMaxIndexForceLimit)

    bool   bD28MaxForceLimitByDiameter;  // IniConfig.  golden :18736
    bool   bD27UseSingleSite85kg;        // IniConfig.  golden :18775/:18789 + GetMaxIndexForceLimit
    bool   bD04MinForceByFile;           // IniConfig.  (via GetMinForce)
    double dD04MinForceByFile;           // IniConfig.  (via GetMinForce)
    double dD04MinForceByFile_20mm;
    double dD04MinForceByFile_30mm;
    double dD04MinForceByFile_40mm;
    double dD04MinForceByFile_60mm;
    double dD04MinForceByFile_80mm;
};

// ---------------------------------------------------------------------------
// TotalAirForceOut -- everything golden writes somewhere other than its return
// value, so the caller can apply it to the real widgets/globals.
// ---------------------------------------------------------------------------
struct TotalAirForceOut
{
    double dMinForce;             // golden member dMinForce  :18763, possibly :18868
    double dMinKgPerHead;         // golden local             :18769..:18853, init 1.0 (:18681)
    int    iEP_Min_KG;            // -> IniConfig.iEP_Min_KG  :18857.  Config.h:79 makes that
                                  //    field an `int` while dMinKgPerHead is a double, so
                                  //    golden TRUNCATES here.  Reproduced with an explicit
                                  //    cast; dMinKgPerHead above keeps the untruncated value.
    double dNowKgPerHead;         // golden local             :18768..:18852, init 0.0 (:18682)
    double fComplianceUnit;       // golden local             :18693-18705, init 1.0 (:18680)
    double dDeviceGf;             // golden local :18684, possibly clamped :18745
    double dHeadMaxForce;         // golden local :18740 (0.0 unless bMaxForcePerICVisible)
    float  fMaxForcePerCompliance;// ChangeToFloatNonPcnt(dHeadMaxForce, fComplianceUnit)
                                  //    :18741.  float because that template returns float
                                  //    (MachineType.h:1601) -- kept, not widened.
    double dMaxLimit;             // GetMaxIndexForceLimit()  :18883

    // -- what the caller must paint / show ----------------------------------
    bool bMaxForcePerICVisible;   // lblMaxForcePerIC->Visible  :18738 (true) / :18756 (false)
                                  //   when true, Caption := sprintf("Max force per compliance:
                                  //   %0.2fkg", fMaxForcePerCompliance)      :18741-18742
    bool bDeviceGfClamped;        // :18743 taken.  Caller then rewrites
                                  //   edForcePerDeviceKG->Text = FormatFloat("0.0000", dDeviceGf)
                                  //   edForcePerDeviceN ->Text = FormatFloat("0.0000", dDeviceGf*9.8)
                                  //   -- golden :18750-18751 really does use dDeviceGf*9.8 here,
                                  //   NOT the dDeviceN that ShowArmAndDeviceForce computes.
    EForceFieldColor eValueFieldColor;  // edAirKPA/edSetKg/edForcePerDeviceKG/edForcePerDeviceN
    bool bAirForceRed;            // edAirForce->Color = clRed  :18886 (else stays clWhite :18730)
    bool bMinForceCaptionPerIC;   // lblMinForce->Caption  :18881
                                  //   true  -> sprintf("Min force per ic: %0.2fkg",        dMinForce)  :18870
                                  //   false -> sprintf("Min force per compliance: %0.2fkg", dMinForce) :18858/:18878
};

// ---------------------------------------------------------------------------
// ComputeTotalAirForce
//   Portable replacement for TfContact::CalculateTotalAirForce(double,double).
//   BCB6 source: cContact.cpp:18675-18891  (JimmyChiu 20220119 獨立計算氣壓壓力)
//
//   dBallCount / dSingleGf -- unchanged golden parameters.
//   bUseTestSocket         -- LastSet.bUseTestSocket[2][4][8] (LastSet.h:431).  Golden reads
//                             six elements of it, at :18773-18774 and :18787-18788.  First
//                             dimension decayed per this tree's 2D/3D-array-param convention
//                             (see ComputeAutoHeightMaxKGTorque's iSiteMap above).
//   dDutCount        [in/out] -- golden's TfContact member (cContact.h:552 area).  IN because
//                             :18759 multiplies by it and GetMaxIndexForceLimit reads it; OUT
//                             because :18689 FORCES it to 4 under
//                             (iTestMode==DualSite && bQualSite2X2Shift && bNS7000kit), and that
//                             write outlives the call in golden.  Do not pass a temporary.
//   out              [out]   -- see TotalAirForceOut.
//   returns dTotalForce, golden's own return value (:18890).
//
//   Faithfully preserves both DEFAULTLESS switches, which are real golden gaps:
//     * switch(scrbSLK->Position) :18693-18705 covers 2..6 only, so any other Position leaves
//       fComplianceUnit at 1.0.
//     * switch(TestIF.iTestMode) :18765-18855 covers 17 modes and has NO default, so an
//       unlisted mode leaves dNowKgPerHead at 0.0 and dMinKgPerHead at 1.0 -- and 0.0 then
//       satisfies :18860 `dNowKgPerHead<=dMinForce`, i.e. an unknown test mode silently takes
//       the minimum-force branch.  Preserved; do not add a default.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ComputeDutCount -- AI(W906-P2b-CF) 20260919.
//   golden `TfContact::DutCount()` 的資料半邊（golden cContact.cpp:1962-2059）。
//   參數就是 golden 讀的那四個全域（照本檔 ComputeMaxIndexForceLimit 的慣例，
//   這個 TU 不碰全域）：
//       iTestMode              <- TestIF_File.iTestMode
//       bD27UseSingleSite85kg  <- IniConfig.bD27UseSingleSite85kg
//       bUseTestSocket         <- LastSet.bUseTestSocket
//       iCloseSiteModeFor2x8   <- 同名全域
//       bOctal_12Kit           <- TestIF_File.bOctal_12Kit
//       iSiteMap               <- TestIF_File.iSiteMap
//
//   ⚠ golden 讀的是快取成員 `TfContact::dDutCount`，呼叫端這裡是**當場重算**。
//   ⚠ golden 的 widget 尾段（含 `DeviceForm_File.iHeadDeviceCT` 的寫入）
//     **不在這裡** —— 來源是 UI 捲軸，這棵樹沒有實例。
//   ⚠ 用它的地方（adam6024.cpp 的 TransformFuntion）是**除法的分母**，
//     所以那一行不能加閘 —— 閘掉力量會偏大。詳見 cContact.cpp 的橫幅。
// ---------------------------------------------------------------------------
double ComputeDutCount(int iTestMode,
                       bool bD27UseSingleSite85kg,
                       const bool bUseTestSocket[][MAX_SOCKET_ROW][MAX_SOCKET_COL],
                       int iCloseSiteModeFor2x8,
                       bool bOctal_12Kit,
                       const int iSiteMap[][MAX_SOCKET_COL]);

double ComputeTotalAirForce(double dBallCount,
                            double dSingleGf,
                            const TotalAirForceIn& in,
                            const bool bUseTestSocket[][MAX_SOCKET_ROW][MAX_SOCKET_COL],
                            double& dDutCount,
                            TotalAirForceOut& out);

// ===========================================================================
// AI(W906-cContactLeaf) 20260721: W906 cContact leaf-function wave (Step 1 +
// Step 2 of the wave plan).  All 6 functions below follow the SAME
// extract-calc-core / explicit-parameter convention as ComputeMaxIndexForceLimit
// / ComputeMinForce above: every global/member the golden body reads is an
// explicit parameter, no globals are read directly inside cContact.cpp.
// ===========================================================================

// ---------------------------------------------------------------------------
// ComputeIsRun2DCheck
//   Portable replacement for TfContact::IsRun2DCheck().
//   BCB6 source: cContact.cpp:21203-21219 (JerryYang 20250220 : 2DID安裝順序檢查功能)
//
//   Parameters (every external read the golden body performs, as an explicit param):
//     bCheck2DHardware  -- global IniConfig.bF33_Check2DHardware      (Config.h, bool)
//     iBarCodeInstall   -- global BAR_CODE_INSTALL                    (cmydef.h, int; compared
//                          against ebctUseCCDMode/ebctInShtIntel/ebctEtherNetCCD, MachineType.h)
//     bEnableBarCode    -- global TestIF_File.bEnableBarCode          (cprod.h, bool)
//     iInArmShtRow      -- global InArmSuck.iShtRow                   (aHotPlateSubstrate.h, int)
//     iContactMode      -- global iContactMode                        (cmydef.h, int; NOT a
//                          TfContact member -- verified golden cmydef.h:3089 `extern int iContactMode;`)
//     bRun2DCheck       -- global bRun2DCheck                         (cmydef.h, bool; also NOT a
//                          TfContact member -- golden cmydef.h:5817)
//
//   Faithfully preserves the flat 6-term AND (the 3-way BAR_CODE_INSTALL OR-group counts as one
//   term) with no else branch (ret stays false unless every term is true).  Golden also carries a
//   commented-out 7th term `//TestIF_File.bEnableBottom2D==false` (JerryYang 20250428 note "fix
//   contact mode有alarm") -- dead code in golden itself, not translated (nothing to translate).
//
//   CALLER NOTE (behavior-change risk, checked this wave): as of this translation, NOTHING in the
//   translated tree ever assigns bRun2DCheck=true (grep confirms cmydef.cpp's own zero-initializing
//   definition -- un-gated this same wave from the InitialMemory tail block, see the
//   AI(W906-cContactLeaf) comment at its new location -- is the ONLY assignment; the golden setter
//   logic lives in the still-deferred ainarm InArm-suck workflow).  So this function currently
//   evaluates to `false` for every one of its 78 golden call sites across 34 files (26
//   ainarm*.cpp variants incl. ainarm2.cpp/ainarm9045.cpp, csystem.cpp:1812/9577/9583/13012/24260,
//   3 BarCode/BarCode_*.cpp files, AutoClean.cpp, ckernel.cpp, cOffSet.cpp, uhome.cpp -- recounted
//   by AI(W906-cContactLeaf-review) 20260722, correcting this wave's original "40+ ... 18
//   ainarm9045_* ... csystem.cpp:838/2649 ... 8 BarCode" citation, which did not match any real
//   IsRun2DCheck() call site) under every reachable global state in the CURRENT tree --
//   identical to the hardcoded-false stub it replaces.  This is a real behavior change only once a
//   future wave translates whatever sets bRun2DCheck=true.
// ---------------------------------------------------------------------------
bool ComputeIsRun2DCheck(bool bCheck2DHardware,
                          int  iBarCodeInstall,
                          bool bEnableBarCode,
                          int  iInArmShtRow,
                          int  iContactMode,
                          bool bRun2DCheck);

// ---------------------------------------------------------------------------
// kgTranToMitsubishikg
//   Translated VERBATIM: this is already a free function in golden (NOT a TfContact member).
//   BCB6 source: cContact.cpp:5261-5271.  Pure int clamp (kg/3, floor 15, ceil 30), zero deps.
// ---------------------------------------------------------------------------
int kgTranToMitsubishikg(int kg);

// ---------------------------------------------------------------------------
// ComputeAutoHeightMaxKGTorque
//   Portable replacement for TfContact::GetAutoHeightMaxKGTorque().
//   BCB6 source: cContact.cpp:5273-5382 (Steven 20170720 (wei) : for low contact force)
//
//   Parameters:
//     iContactMode                     -- global iContactMode (cmydef.h, int)
//     bD10ManualHeightComptibleWithNS  -- global IniConfig.bD10ManualHeightComptibleWithNS (bool)
//     bD14_AutoHeightUseSetTorque      -- global IniConfig.bD14_AutoHeightUseSetTorque (bool)
//     iD14_AutoHeightUseSetTorque      -- global IniConfig.iD14_AutoHeightUseSetTorque (int)
//     dKitDiameter                     -- global DeviceForm.dKitDiameter (cprod.h, double)
//     indexPressType                   -- global INDEX_PRESS_TYPE (cmydef.h, eIndexPressType)
//     iTestMode                        -- global TestIF.iTestMode (cprod.h, int)
//     iTestSocketShtRow/Col            -- global TestSocket.iShtRow/iShtCol (aHotPlateSubstrate.h, int)
//     iSiteMap                         -- global TestIF_File.iSiteMap[MAX_SOCKET_ROW][MAX_SOCKET_COL]
//                                         (cprod.h, int; first dim decayed per this tree's existing
//                                         2D-array-param convention, see aoutarm9045_2x4_8.cpp:164)
//     iNNMode                          -- return value of global IsNNMode() (atester_shims.h/.cpp,
//                                         int; passed in rather than called here to keep this file
//                                         dependency-free, matching the ComputeMinForce precedent)
//
//   Faithfully preserves every INDEX_PRESS_TYPE branch (e240KG/e260KG open-site-count loop over
//   TestIF_File.iSiteMap, e400KG/e360KG, e500KG, e640KG, e800KG, else) and the low-contact-force
//   dKitDiameter<2.5 branch's e85KG special case.  Uses CONTACT_MANUAL_GET_HEIGHT (golden
//   cContact.cpp:76, value 2 -- added to this header this wave; the header previously only had
//   CONTACT_NORMAL, which Step 1's ComputeIsRun2DCheck needed).
// ---------------------------------------------------------------------------
int ComputeAutoHeightMaxKGTorque(int iContactMode,
                                  bool bD10ManualHeightComptibleWithNS,
                                  bool bD14_AutoHeightUseSetTorque,
                                  int iD14_AutoHeightUseSetTorque,
                                  double dKitDiameter,
                                  eIndexPressType indexPressType,
                                  int iTestMode,
                                  int iTestSocketShtRow,
                                  int iTestSocketShtCol,
                                  const int iSiteMap[][MAX_SOCKET_COL],
                                  int iNNMode);

// ---------------------------------------------------------------------------
// ComputeTestZCompensationHight
//   Portable replacement for TfContact::TestZ_CompensationHight().
//   BCB6 source: cContact.cpp:18453-18540 (Ifor 20200318 : add 針對80mm套件因為手臂Hold的變化需補下壓高度)
//   NOTE: golden's own method name misspells "Height" as "Hight" -- preserved verbatim in this
//   free-function name for grep-back-to-golden fidelity (not silently corrected).
//
//   Parameters:
//     dKitDiameter    -- global DeviceForm_File.dKitDiameter (cprod.h, double)
//     dPress          -- global DeviceForm_File.dPress       (cprod.h, double)
//     dIndexZOffset   -- global dIndexZOffset[3][15]         (cmydef.h, double; first dim decayed
//                        per this tree's 2D-array-param convention)
//     iTotalOffset_1/2 [out] -- golden file-scope globals `double iTotalOffset_1/2` (cContact.cpp:
//                        98-99 -- NOTE: declared `double` despite the `i`-prefix; this is a golden
//                        naming quirk, preserved verbatim, NOT a translation typo).  Passed by
//                        reference instead of being real persistent globals: as of this wave every
//                        real call site that READS iTotalOffset_1/2 (cContact.cpp:6591/7162/7198/
//                        7431/9767/9801/9976) is inside the still-fully-deferred CarlibrationTask
//                        cluster, so there is no live caller needing global persistence yet -- same
//                        "faithful for the one real call site, rest deferred" reasoning as
//                        ComputeMinForce's dMinForce note above.
//
//   GOLDEN QUIRK preserved verbatim: when dKitDiameter is NOT 8 or 40.2, the golden function body
//   does NOTHING -- it does not touch iTotalOffset_1/iTotalOffset_2 at all (they retain whatever
//   value they already had).  This free function mirrors that exactly: the out-params are only
//   ever written inside the `dKitDiameter==8 || dKitDiameter==40.2` branch, never zeroed/touched
//   otherwise.  Do NOT "fix" this into an unconditional reset; it would diverge from golden.
// ---------------------------------------------------------------------------
void ComputeTestZCompensationHight(double dKitDiameter,
                                    double dPress,
                                    const double dIndexZOffset[][15],
                                    double& iTotalOffset_1,
                                    double& iTotalOffset_2);

// ---------------------------------------------------------------------------
// ComputeIndexDownPos
//   Portable replacement for TfContact::SetIndexDownPos().
//   BCB6 source: cContact.cpp:18628-18661 (Steven 20211109 : 改成公用變數)
//
//   Parameters:
//     bUserDefMaxContactHeight  -- global bUserDefMaxContactHeight (cmydef.h, bool)
//     dUserDefMaxContactHeight  -- global dUserDefMaxContactHeight (cmydef.h, double)
//     iMachineTypeChoice        -- global MachineTypeChoice (cmydef.h, int; compared against
//                                  Type_HT9046_LS, MachineType.h)
//     iCustomerCode             -- global CUSTOMER_CODE (cmydef.h, int; compared against
//                                  CC_QUALCOMM/CC_KYEC_LEE/CC_KYEC_CHEN/CC_KYEC_JCTHIU/CC_DL_TEK/
//                                  CC_KYEC_XILINX, MachineType.h)
//     iATCMixMode               -- global ATC_MixMode (cmydef.h, int; compared against
//                                  eMixATC60_ATC33, MachineType.h)
//     iATCSystem                -- global ATC_SYSTEM (cmydef.h, int; compared against
//                                  eATCUninstall, MachineType.h)
//
//   Returns the value golden assigns to the class member/global `float fIndexDownPos` (cmydef.h:
//   5404, `extern double fIndexDownPos;` in this tree) instead of mutating it -- fIndexDownPos IS
//   already a real global in this tree, but this leaf has no live caller yet (see file banner in
//   cContact.cpp), so mutating it here would be an unused side effect; the caller assigns the
//   return value once this function gets wired up.  Faithfully preserves the branch order
//   (bUserDefMaxContactHeight > Type_HT9046_LS > ATC_SYSTEM!=eATCUninstall > else) and the
//   Type_HT9046_LS branch's nested CC_QUALCOMM+eMixATC60_ATC33 -20 adjustment and the else branch's
//   5-customer-code -5 adjustment list.
// ---------------------------------------------------------------------------
double ComputeIndexDownPos(bool bUserDefMaxContactHeight,
                            double dUserDefMaxContactHeight,
                            int iMachineTypeChoice,
                            int iCustomerCode,
                            int iATCMixMode,
                            int iATCSystem);

// ---------------------------------------------------------------------------
// ComputeIndexYSpeed / ComputeIndexZSpeed
//   Portable replacements for TfContact::GotIndexYSpeed(int)/GotIndexZSpeed(int).
//   BCB6 source: cContact.cpp:18663-18667 / 18669-18673 (JimmyChiu 20211028 : All speed can set by
//   speed setting. iContactMode==CONTACT_DEVICE_MAP_CHECK)
//
//   Parameters:
//     iFixValue      -- unchanged golden param (int, the `_fixValue` default speed)
//     iContactMode   -- global iContactMode (cmydef.h, int)
//     iMotGailSpeed  -- golden MOT[MTestY1].GailSpeed / MOT[MTestZ1].GailSpeed (Motor/mymotor.h,
//                       int; MOT[] is real -- Motor/mymotor.h:385 `extern class TTrayMotor
//                       MOT[MAX_TRAY_MOTOR];`).  Passed in rather than indexed here to keep this
//                       file free of a Motor/mymotor.h dependency, matching this file's existing
//                       explicit-parameter convention; caller passes MOT[MTestY1].GailSpeed /
//                       MOT[MTestZ1].GailSpeed respectively.
//
//   NOTE: golden mutates+returns the TfContact member `int itemp` (cContact.h:534, "工具判斷使用" --
//   a scratch/shared temp, NOT a per-call-site-isolated local).  This free function uses a plain
//   local instead and returns it -- itemp has no other reader in golden between the assignment and
//   the return (single-statement mutate-then-return), so this is behaviorally identical; same
//   "local stand-in for the golden member" pattern as ComputeMinForce's dMinForce.
// ---------------------------------------------------------------------------
int ComputeIndexYSpeed(int iFixValue, int iContactMode, int iMotGailSpeed);
int ComputeIndexZSpeed(int iFixValue, int iContactMode, int iMotGailSpeed);

#endif // CCONTACT_H
