// =============================================================================
//  AOI/TopBottomInspect.h -- Q32 (S69): the AOI.Data parameters of golden TTopBottomInspect ("Top & Bottom Inspect",
//  Jimmychiu 20240322) as a plain data structure with golden's defaults.  No socket, no motion, no VCL form.
//
//  AI(W906-Q32-S69) 20260927 (St02).  ctest: tests/test_tb_inspect.cpp (AOI_TopBottomInspectParams).
//
//  Golden, cited by tree name:
//    906_0625_Steven fAOI.h:538-780     class TTopBottomInspect (the fields below keep golden's names and types)
//    906_0625_Steven fAOI.cpp:166-267   TFrmAOI::IntialParameter -- the HTEditList registrations: every key, section and
//                                       default below comes from :182-263; registered only when
//                                       USE_Scanner_AOI_Inspection == eBtnAOI_TopBottomInstall (:180)
//    906_0625_Steven fAOI.cpp:4582-4615 the TTopBottomInspect ctor (values before the file is read)
//    906_0625_Steven Public/HTEditList.cpp:1009-1137  ReadEditTextFromFile -- how a key is read (see "Read rules")
//  ⚠ Golden version: the team's golden 906 is HT9011UC_Code_V3.33.906.0_20260618 (the laptop / NB2 numbers).  STEVEN-NB3
//    can only read the unpacked 906_0625_Steven copy (Steven #34, 20260927: "先用這個").  0625 matches 0618 up to about
//    main.cpp:18106 and drifts +78 lines later in main.cpp; fAOI.* has not been compared with 0618 line by line, so the
//    fAOI numbers above are 0625 numbers -- NB2 can confirm them against 0618.
//  912 additions (the 0926 14:3x ruling "906 base + 912 additions"), kept in their own block and their own key list:
//    912 fAOI.cpp:237 iAutoRetryCount (Ifor 20260812), :238 sRecipeName (Ifor 20260827), :314-319 FailStop_<code> x15
//    (Ifor 20260820; codes and names :203-210, count 912 fAOI.h:24).
//
//  File: <recipe>\AOI.Data (golden fAOI.cpp:2966 / :3202 build the path, :3374 ReadEditTextFromFile, :3161
//  SaveEditTextToFile), sections [TopBottomInspect], [ZPickOffset] and [Function Setting].
//  Read rules (golden HTEditList.cpp ReadEditTextFromFile):
//    * a missing key takes the default string below;
//    * edit fields read as text (:1094; numbers are then atoi'd by the form), radio groups (iEnable, iAction) with
//      ReadInteger (:1120-1124);
//    * check boxes with ReadInteger and are true ONLY when the value is exactly 1 (:1125-1130).  Their default string
//      is "", which is 0, which is false.
//  Not here (runtime state and behaviour, not AOI.Data): socketAOI, the task counters, t_ti / t_bi, the yield counters,
//  bSimuAOICommand / bSimuMot / bCycleRun, asErrorMsg, iCCD_Up_Z, SuckActiveCount and every method.
//  This header does not read or write any file; FileRW/AOISetup.* is not touched.
// =============================================================================
#pragma once

#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"

namespace aoi {

const int kAOIFailCheckMax  = 20;   // golden 906_0625_Steven fAOI.h:23 iAOIFailCheckMax
const int kTBAOIFailStopMax = 15;   // 912 fAOI.h:24 iTBAOIFailStopMax

// golden uPoint2D {int X; int Y;} (Public/HTEditList.h); only the two coordinates, so this header needs no cmydef.h.
struct TbPoint2D {
    int X;
    int Y;
};

// golden t_TopBtnPhotoInfo (906_0625_Steven fAOI.h:554-569)
struct TbPhotoInfo {
    TbPoint2D ppPosition;
    bool bClampLT;
    bool bClampLB;
    bool bClampRT;
    bool bClampRB;
};

struct TopBottomInspectParams {
    // ---- [TopBottomInspect] (906_0625_Steven fAOI.cpp:182-201, :207-221) ----
    int iEnable;                                  // "iEnable"                 0      (radio rgTopBtmScannEnable)
    int iAction;                                  // "iAction"                 0      (radio rgActionMode; eTopBottom=0, eBottom=1, fAOI.h:615-618)
    AnsiString sSocketAddress;                    // "SocketAddress"           "172.16.8.200"
    AnsiString sSocketPort;                       // "SocketPort"              "5109"
    AnsiString sCamaName;                         // "sCamaName"               "CM1"
    int iStartDelayTime;                          // "iStartDelayTime"         5
    int iGetResultDelay;                          // "iGetResultDelay"         200
    int iTimeout;                                 // "iTimeout"                10
    int iRotate0Pos;                              // "iRotate0Pos"             -20
    int iRotate180Pos;                            // "iRotate180Pos"           3230
    TbPhotoInfo tbCenter;                         // "iTopBtnCenterX" 977 / "iTopBtnCenterY" 80 (its clamps are not in the file)
    int iLight_Up_Z_Top;                          // "iLight_Up_Z_Top"         0
    int iLight_Up_Z_Btm;                          // "iLight_Up_Z_Btm"         0
    int iCCD_Up_Z_Top;                            // "iCCD_Up_Z_Top"           0
    int iCCD_Up_Z_Btm;                            // "iCCD_Up_Z_Btm"           0
    int iRotateKitAngOffset_In;                   // "iRotateKitAngOffset_In"  0      (Eastsun 20260305)
    int iRotateKitAngOffset_Out;                  // "iRotateKitAngOffset_Out" 0      (Eastsun 20260305)
    TbPhotoInfo tbpiLessThanOrEqual65mm[2];       // "Small_<i>_PosX|PosY" 0, "Small_<i>_LT|LB|RT|RB" false
    TbPhotoInfo tbpiMoreThan65mm[4];              // "Large_<i>_PosX|PosY" 0, "Large_<i>_LT|LB|RT|RB" false
    // ---- [ZPickOffset] (:203) ----
    int iZPickOffset;                             // "iZPickOffset"            0
    // ---- [Function Setting] (:224-237) ----
    bool bConsecutiveFailCheck;                   // "Enable Consecutive Fail Check"          false
    int  iConsecutiveFailCount;                   // "Consecutive Fail Count"                 0
    bool bConsecutiveFailPictureCheck;            // "Enable Consecutive Fail Check(Picture)" false
    int  iConsecutiveFailPictureCount;            // "Consecutive Fail Count(Picture)"        0
    bool bAccumulatedFailCheck;                   // "Enable Accumulated Fail Check"          false
    int  iAccumulatedFailCount;                   // "Accumulated Fail Count"                 0
    bool bIntervalCheck;                          // "Enable Interval Check"                  false  (hidden on the form)
    int  iIntervalCount;                          // "Interval Count"                         0      (hidden)
    int  iAOIFailSetBin;                          // "AOI Fail Set Bin"                       15     (hidden)
    bool bAOIFailBin;                             // "Enable AOI Fail Bin"                    false  (Eastsun 20260319)
    // ---- [Function Setting], one set per list row, <i> = 0..19 (:240-264, Eastsun 20260402 / 20260515) ----
    bool bConsecutiveFailCheck_List[kAOIFailCheckMax];         // "Enable Consecutive Fail Check_<i>"          false
    int  iConsecutiveFailCount_List[kAOIFailCheckMax];         // "Consecutive Fail Count_<i>"                 0
    bool bConsecutiveFailPictureCheck_List[kAOIFailCheckMax];  // "Enable Consecutive Fail Check(Picture)_<i>" false
    int  iConsecutiveFailPictureCount_List[kAOIFailCheckMax];  // "Consecutive Fail Count(Picture)_<i>"        0
    bool bAccumulatedFailCheck_List[kAOIFailCheckMax];         // "Enable Accumulated Fail Check_<i>"          false
    int  iAccumulatedFailCount_List[kAOIFailCheckMax];         // "Accumulated Fail Count_<i>"                 0
    bool bIntervalCheck_List[kAOIFailCheckMax];                // "Enable Interval Check_<i>"                  false  (hidden)
    int  iIntervalCount_List[kAOIFailCheckMax];                // "Interval Count_<i>"                         0      (hidden)

    // ---- 912 additions ([TopBottomInspect]) ----
    int  iAutoRetryCount;                         // "iAutoRetryCount"   0   912 fAOI.cpp:237
    AnsiString sRecipeName;                       // "sRecipeName"       ""  912 fAOI.cpp:238 (hidden; "" = use the setup file name)
    bool bFailStopCheck[kTBAOIFailStopMax];       // "FailStop_<code>"   false  912 fAOI.cpp:318, codes :204

    TopBottomInspectParams() { SetGoldenDefaults(); }
    // Every field above to golden's default (the value a missing key reads as).  The fields not in AOI.Data
    // (tbCenter's clamps) take golden's ctor value: tbCenter.Clear() (906_0625_Steven fAOI.cpp:4611) = 0 / false.
    void SetGoldenDefaults();
};

// One AOI.Data key, pointing into a TopBottomInspectParams.  Exactly one of pi / pb / ps is set.
struct TbKey {
    const char* section;
    std::string key;
    char kind;              // 'i' edit read as a number, 'r' radio group (ReadInteger), 'b' check box, 's' text
    int*  pi;
    bool* pb;
    AnsiString* ps;
    const char* def;        // golden's default string, as registered
    bool from912;
};

// Every key in golden's registration order (906_0625_Steven fAOI.cpp:182-263), then the 912 additions when with912.
// 906: 225 keys (49 registrations, loops expanded: 19 + 2x6 + 4x6 + 10 + 20x8); with912: 242 (+2 +15).
std::vector<TbKey> TopBottomInspectKeys(TopBottomInspectParams& p, bool with912);

// 912 fAOI.cpp:204 -- the FailStop key suffixes, index = bFailStopCheck index.
extern const char* const kTBAOIFailStopCode[kTBAOIFailStopMax];

}  // namespace aoi
