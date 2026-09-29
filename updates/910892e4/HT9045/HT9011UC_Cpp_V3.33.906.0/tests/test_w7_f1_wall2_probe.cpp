// =============================================================================
//  tests/test_w7_f1_wall2_probe.cpp
//
//  AI(W906-W7-F1) 20260729: the W7-F1 GATE for
//  docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F1 ("Wall 2" unlock). The wave's own
//  gate is: "the 22 overrides must go from 'cannot reference fMain at all' to
//  'can be translated one by one' -- demonstrate that concretely". This file
//  is that demonstration.
//
//  WHAT WALL 2 WAS (plan SS4-V1, already fixed by W7-F0 -- re-verified here,
//  not re-fixed): FormsFacade.cpp used to compile into ht9045_sm, which
//  ht9045_secsgem did NOT link, while ht9045_sm DID link ht9045_secsgem -- a
//  genuine CMake target cycle that made it IMPOSSIBLE for
//  SECSGEM/uHGemHT9045.cpp to reach `fMain`, no matter how many facade members
//  existed. W7-F0 moved the facade into the new bottom-layer `ht9045_forms`
//  library and had `ht9045_secsgem` link it (CMakeLists.txt's
//  `target_link_libraries(ht9045_secsgem PUBLIC ... ht9045_forms)` line) --
//  acyclic, because `ht9045_forms` depends on nothing above itself.
//
//  WHAT THIS PROBE ACTUALLY CHECKS, IN LAYERS:
//    1. LINK LAYER: this section has now been corrected TWICE; the version
//       below is the one backed by an experiment. History, so nobody
//       re-litigates it: the wave originally claimed the executable "names
//       only ht9045_secsgem in its CODE" and resolves `fMain` "purely through
//       ht9045_secsgem's PUBLIC dependency on it" (FALSE -- the CMake call
//       names seven libraries explicitly); AI(W906-W7-F1fix) 20260729 then
//       replaced that with "the explicit list is REQUIRED ... a bare
//       target_link_libraries(... PRIVATE ht9045_secsgem) (relying purely on
//       ht9045_secsgem's PUBLIC transitive edge to ht9045_forms) hit the
//       IDENTICAL undefined-reference failure", i.e. it treated the transitive
//       edge as the thing that does not work. AI(W906-W7-F1fix2) 20260729
//       MEASURED it instead of arguing about it, and the true split is:
//         (a) The transitive PUBLIC edge DOES put ht9045_forms on the link
//             line by itself. Evidence, from the generated build system rather
//             than from reasoning: test_SecsWireCodec's CMake call is a bare
//             `PRIVATE ht9045_secsgem`, and the linkLibs response file CMake
//             generates for it is
//               libht9045_secsgem.a libht9045_core.a libht9045_public.a
//               libht9045_db.a libht9045_forms.a libht9045_globals.a
//               libvclcompat.a
//             -- ht9045_forms is there, unnamed by anyone, purely via the
//             PUBLIC closure. So a consumer naming only ht9045_secsgem CAN
//             see fMain.
//         (b) What the bare form lacks is the RESCAN (--start-group), not
//             ht9045_forms. Relinking THIS probe's own objects against exactly
//             that bare-form archive list fails with 5 errors, and every one
//             of them is `libht9045_globals.a(cpublic.cpp.obj): undefined
//             reference to SearchFile(...)/md5_Folder(...)` -- symbols defined
//             in libht9045_public.a (verified with nm --defined-only), which
//             single-pass GNU ld already scanned and passed. ZERO of the
//             errors involve fMain or anything in ht9045_forms.
//         (c) Re-linking that SAME bare archive list wrapped in
//             -Wl,--start-group/--end-group succeeds (exit 0). So the group,
//             not the explicit library list, is the load-bearing part.
//       WHAT THIS PROBE THEREFORE PROVES: Wall 2 is a CONFIGURE-time target-
//       cycle question and it is answered (ht9045_secsgem and ht9045_forms
//       coexist on one link line without CMake diagnosing a cycle), AND fMain
//       genuinely resolves for a consumer of ht9045_secsgem -- (b) shows the
//       facade symbols resolve even in the un-grouped link. WHAT IT DOES NOT
//       prove: that a future consumer can drop the RESCAN group. It cannot --
//       (b) is a cpublic/ht9045_public archive-ordering problem that has
//       nothing to do with Wall 2, and the explicit list exists only because
//       CMake's $<LINK_GROUP:RESCAN,...> wraps only the libraries named
//       INSIDE it. Same reason test_config_loaders needs the same set.
//    2. INCLUDE LAYER: this ONE translation unit #includes BOTH
//       "SECSGEM/uHGemHT9045.h" (the header the real 22 overrides will live
//       in) AND "FormsFacade.h" (the facade umbrella, at repo root -- mirrors
//       golden's own main.h location) -- exactly the situation
//       golden's uHGemHT9045.cpp is in (it #includes "main.h" directly,
//       uHGemHT9045.cpp:8). If these two headers could not coexist, no
//       override could ever be translated.
//    3. EXPRESSION LAYER (the part that actually matters for "can be
//       translated one by one"): PART A below reproduces, EXPRESSION FOR
//       EXPRESSION, the exact `fMain->member` shapes golden's
//       SECSGEM/uHGemHT9045.cpp dereferences (golden file:line cited at each
//       site), inside free functions shaped like a future override body. If
//       every one of golden's 26 LIVE (non-commented-out) `fMain->` spellings
//       compiles here, a translator has nothing left blocking a first
//       override. ONE EXCEPTION, flagged where it occurs below
//       (mimic_site_on_off_decode): the tSiteOnOff site needs an explicit
//       `AnsiString(...)` wrapper around `->Strings[z]` that golden's own
//       `.Strings[z].c_str()` does not need, because vclcompat's
//       StringsProxy return type has no `c_str()` of its own. That one site
//       is therefore "compiles against the real facade" but not literally
//       "expression for expression" with golden.
//
//  INVENTORY NOTE (re-derived this wave by grepping golden directly, not
//  trusting the plan's own count): golden SECSGEM/uHGemHT9045.cpp touches 28
//  DISTINCT `fMain->NAME` spellings by a naive grep. 2 of those --
//  `fMain->PPID` and `fMain->bNeedClearFile` (golden :5311-5312) -- are
//  inside `//`-commented-out code with NO live call site, so they are
//  deliberately NOT reproduced below and NOT added to the facade (adding a
//  member nothing dereferences would be inventing surface, which this
//  project's conventions forbid). The 26 live members split as:
//    * 10 already existed before this wave: Start, ShowTestHeadComp,
//      CleanOut, LoadTestModePicture, SetStartModeData, BtnOneCycleClick,
//      Reset, cbSetupFileName, edWorkTemperBase, Home. (The plan's own "do
//      not duplicate" list named only the first 9 of these -- Home was ALSO
//      already present and reusable but not mentioned there.)
//    * 16 added by this wave: cbSetupFileNameChange, Clarn_Data,
//      BtnPauseClick, LoadRunModePicture, CanChangeSite, BtnTrayEndClick,
//      UpdateMainOperateMode, LoadStartModePicture, LookForFile,
//      ChangeTesterConnect, SetTemp, tSiteOnOff, edSoakTime (all 13 named by
//      the plan) PLUS 3 the plan's own inventory missed: ChangePassword,
//      FTClick, RTClick (golden uHGemHT9045.cpp:2513/2241/2266 -- all live,
//      non-commented calls).
//
//  AI(W906-W7-F1fix) 20260729 / AI(W906-W7-F1fix3) 20260731 -- HOW FAR THIS
//  ACTUALLY UNLOCKS TRANSLATION (corrects an over-broad "the 22 overrides can
//  now be translated one by one" reading of the gate above).
//
//  FIRST, THE FAILURE MODE THAT PRODUCED THE FIGURES THIS BLOCK REPLACES, named
//  so it stops recurring: fix's scan recognised only `//` comments and NEVER
//  `/* */` BLOCK comments. It reported "9 of the 22 overrides touch fMain, 5 of
//  them touch ONLY fMain and are therefore FULLY unblocked". Two of that 9 --
//  ProcessS7F23FromatReceipe and ProcessS7F25FromatReceipe -- have NO live
//  fMain dereference whatsoever: their only fMain sites sit inside block
//  comments that OPEN at golden uHGemHT9045.cpp:5844 (S7F23 --
//  fMain->cbSetupFileName at :5850 and :5852) and :5961 (S7F25 -- the same
//  member at :5966 and :5968; that same block comment also hides 13 dead fSetup
//  sites, :5993-6017). Those are not small comments: `/*` at :5844 closes at
//  :5925, i.e. 82 of ProcessS7F23FromatReceipe's 101 body lines (:5827-5927) are
//  commented out, and `/*` at :5961 closes at :6021, 61 of
//  ProcessS7F25FromatReceipe's 80 body lines (:5944-6023). Both overrides are
//  mostly-empty shells. Note the blind spot bit the PER-OVERRIDE count only, not
//  the INVENTORY NOTE above: cbSetupFileName is live elsewhere, so the "28
//  distinct / 26 live" spelling figures survive re-derivation unchanged.
//
//  RE-DERIVED THIS WAVE with a character-level comment-AND-string-aware scan of
//  the cp950-decoded golden, over the brace-matched bodies (code-level braces
//  only) of all 22 virtuals declared at golden uHGemHT9045.h:346-365 + :367-368,
//  the real split of the 22 is 7 / 2 / 13:
//   (A) SEVEN overrides LIVE-DEREFERENCE fMain: ReloadParameter (golden
//       uHGemHT9045.cpp:371), LookForFile (:466),
//       S2F15_CheckNewEquipmentConstant (:483),
//       S2F15_UpdateNewEquipmentConstant (:692),
//       S2F42_Host_Command_Acknowledge (:1146, body :1146-4189 -- corrected from
//       :1146-4191 by AI(W906-W7-F1fix2), re-confirmed here by brace-matching:
//       :4189 is its closing brace, :4190 a separator comment, :4191 already
//       S5F6_ListAlarmData's signature), AddSV (uHGemHT9045_SV.cpp:55) and
//       AddEC (uHGemHT9045_EC.cpp:52).
//   (B) THREE of those seven touch ONLY fMain, so this facade surface is their
//       whole form-side blocker -- and every member each one needs is present in
//       forms/fMain.h today (checked member by member):
//         ReloadParameter  -> LoadTestModePicture, UpdateMainOperateMode,
//                             LoadRunModePicture, LoadStartModePicture
//         LookForFile      -> cbSetupFileName, LookForFile
//         S2F15_CheckNewEquipmentConstant -> CanChangeSite
//       "Unblocked" means ON THE FORM-FACADE AXIS ONLY. ReloadParameter also
//       calls SaveAllFile(GetLastOpenFN()) under a CUSTOMER_CODE gate (:373-374)
//       and S2F15_CheckNewEquipmentConstant walks HGemPtr/LastSet; whether those
//       are ready is a separate question this file does not answer.
//   (C) The other FOUR of the seven also dereference forms with no facade member
//       for them, and the earlier "+fLotInfo/+fSetup"-style lists UNDERCOUNTED
//       that badly. Live NON-fMain form pointers, re-derived:
//         S2F15_UpdateNewEquipmentConstant -- 8: fBarCode, fBinSel, fBuilder,
//             fLotInfo, fSetup, fSpeed, fTemp_Set, fTesterTCP
//         S2F42_Host_Command_Acknowledge -- 14: fAGV, fConfiguration,
//             fContactCT, fFTPClient, fLotInfo, fNote, fObserver, fPassword,
//             fProductionInfo, fSCKART, fShowBinSelect, fSortCT, fTemp_Set,
//             fYieldMonitoring
//         AddSV -- 8: fCleaning, fContact, fGroundMan, fLotInfo, fObserver,
//             fShowBinSelect, fSmartDiagnostic, fTrayAssignment (PLUS 3 missing
//             fMain members of its own -- see the fix2 block below)
//         AddEC -- 7: fBinSel, fCleaning, fContact, fLotInfo, fSCKART, fSetup,
//             fStartCondition (effectively all ~1912 lines of
//             uHGemHT9045_EC.cpp)
//   (D) TWO overrides are form-blocked WITHOUT touching fMain at all -- omitted
//       entirely from every earlier list:
//         S7F4_ProcessProgramAcknowledge (:4478) -- fLotInfo, fOffSet, fSetup
//             live. (Its fMain->PPID / fMain->bNeedClearFile at :5311-5312 are
//             the `//`-dead pair the INVENTORY NOTE above already excludes.)
//         S125F4_LevelSettingChangeAcknowledge (:6176) -- fSecurity live.
//   (E) THIRTEEN overrides have ZERO live form dereference of any kind, so no
//       facade work gates them at all: AddAlarmList (:382), AddCEID (:437),
//       AddReprot (:450), S5F6_ListAlarmData (:4191),
//       S7F2_ProcessProgramLoadGrant (:4397), BOTH S7F6_ProcessProgramData
//       overloads (:5326, :5605), ProcessS7F23FromatReceipe (:5826),
//       S7F24_FormattedProcessProgramSendAcknowledge (:5929),
//       ProcessS7F25FromatReceipe (:5943),
//       S7F26_FormattedProcessProgramData (:6025), S14F4_Get2DID_BinCode
//       (:6042), S110F5_RequestCustomerNameList (:6155).   7 + 2 + 13 = 22.
//
//  THE FORM-FACADE GAP AT LARGE (what every earlier list left out): those 22
//  bodies live-dereference 29 DISTINCT form pointers. PORTED/forms holds 9 form
//  headers (fAGV, fCleaning, fLotInfo, fMain, fNote, fOffSet, fSCKART,
//  fShowMessage, fSortCT). Only 8 of the 29 have a header at all -- those 9
//  minus fShowMessage, which no override touches -- so 21 form pointers have NO
//  facade header whatsoever: fBarCode, fBinSel, fBuilder, fConfiguration,
//  fContact, fContactCT, fFTPClient, fGroundMan, fObserver, fPassword,
//  fProductionInfo, fSecurity, fSetup, fShowBinSelect, fSmartDiagnostic,
//  fSpeed, fStartCondition, fTemp_Set, fTesterTCP, fTrayAssignment,
//  fYieldMonitoring. And a header existing is not coverage: against the members
//  these 22 bodies need, the 8 existing headers are missing fAGV 2 of 2,
//  fCleaning 16 of 17, fLotInfo 31 of 39, fNote 4 of 4, fOffSet 1 of 1,
//  fSCKART 20 of 25, fSortCT 2 of 2 -- fMain is the only nearly-complete one
//  (3 of 32 missing). Those missing counts come from a NAME-PRESENCE scan of
//  each header's code (a name absent from the header is certainly missing; a
//  name present might still not be a usable member), so they are LOWER bounds.
//  Do NOT plan a later bucket assuming all 22, or even all 7 fMain-touching,
//  overrides are ready.
//  UNKNOWN, deliberately not guessed: whether anything OUTSIDE the form layer
//  (HGemPtr / SaveAllFile / LastSet / the SECS wire helpers) additionally blocks
//  the 15 overrides in (D)+(E). This scan measured form dereferences only.
//  See docs/W7_UI_ARCHITECTURE_PLAN.md SS10 for the per-form member inventory.
//
//  AI(W906-W7-F1fix2) 20260729 -- ONE MORE GAP BOTH BLOCKS ABOVE MISS, and it
//  changes the AddSV entry: the INVENTORY NOTE is scoped to uHGemHT9045.cpp
//  alone, but AddSV lives in uHGemHT9045_SV.cpp, and that file dereferences SIX
//  fMain members of its own (grepped and comment-filtered the same way):
//  SVID1190_OSSetup (:231), palMainStatus (:73) and tTestResult (:337) -- all
//  three already in the facade from earlier waves -- PLUS edTorue0 (:74),
//  edTorue1 (:75) and lbEPenconder (:100), which do NOT exist in forms/fMain.h
//  and were NOT added by this wave (verified by grepping the whole ported tree,
//  not just fMain.h). So AddSV is blocked on the fMain side TOO, not only by
//  fLotInfo; reading its entry above as "+fLotInfo" only would send the next
//  agent hunting 18 fLotInfo members and let it hit the 3 missing fMain widgets
//  the hard way. uHGemHT9045_EC.cpp's fMain surface, by contrast, IS complete
//  (cbSetupFileName :64 + tSiteOnOff :72-73, both present). Counting golden's
//  three SECSGEM TUs together: 34 distinct fMain spellings, 32 live, 29 of
//  which now exist in the facade.
// =============================================================================
#include "SECSGEM/uHGemHT9045.h"   // the header the 22 overrides will live in
#include "FormsFacade.h"           // fMain -- exactly what those overrides will dereference
#include "MachineType.h"           // MAX_SOCKET_ROW / MAX_SOCKET_COL (tSiteOnOff prefill check)
#include "LastSet.h"               // LastSet.iTester -- the P2d check at :471 (AI(W906-GB-P2d) 20260926: added on the laptop merge; the file never included it)
#include <cstdio>
#include <cstdlib>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
static int g_pass = 0;
static int g_fail = 0;

static void check(const char *name, bool ok)
{
    if (ok) { printf("PASS  %s\n", name); ++g_pass; }
    else    { printf("FAIL  %s\n", name); ++g_fail; }
}

// =============================================================================
//  PART A -- expression-shape mirror of every LIVE fMain-> site golden's
//  uHGemHT9045.cpp dereferences. Each function below is shaped like a small
//  slice of a future override body: it is never called from a real SM, it
//  exists purely so the compiler proves the expression is well-formed against
//  the REAL fMain type (not a mock) -- the same role test_w7_f0_controls_guard
//  plays for the widget-overload hazard, one layer up.
// =============================================================================
namespace w7f1_wall2_probe {

// golden uHGemHT9045.cpp:1135 `fMain->cbSetupFileNameChange(fMain);`
void mimic_S2F49_or_similar_recipe_dl(int HCACK)
{
    if (HCACK != 0)
        fMain->cbSetupFileNameChange(fMain);
}

// golden uHGemHT9045.cpp:1771 `fMain->Clarn_Data(8, "Clean by TRAY CHECK");`
// golden uHGemHT9045.cpp:3473 `fMain->Clarn_Data(1, "ART_LOTCLEARED");`
void mimic_S2F42_clear_count()
{
    fMain->Clarn_Data(8, "Clean by TRAY CHECK");
    fMain->Clarn_Data(1, "ART_LOTCLEARED");
}

// golden uHGemHT9045.cpp:4116/:4121 `fMain->BtnPauseClick(fMain);`
void mimic_S2F41_pause_rcmd()
{
    fMain->BtnPauseClick(fMain);
}

// golden uHGemHT9045.cpp:378 `fMain->LoadRunModePicture();`
// golden uHGemHT9045.cpp:377 `fMain->UpdateMainOperateMode();`
// golden uHGemHT9045.cpp:379 `fMain->LoadStartModePicture();`
void mimic_ctor_or_reload_picture_refresh()
{
    fMain->UpdateMainOperateMode();
    fMain->LoadRunModePicture();
    fMain->LoadStartModePicture();
}

// golden uHGemHT9045.cpp:543-544
//   `if((LastSet.iTemperature==Tempture_Hot && fMain->CanChangeSite(true)==false) ||
//       (LastSet.iTemperature!=Tempture_Hot && fMain->CanChangeSite(false)==false))`
bool mimic_temperature_site_gate(bool bIsHot)
{
    if ((bIsHot  && fMain->CanChangeSite(true) == false) ||
        (!bIsHot && fMain->CanChangeSite(false) == false))
        return false;
    return true;
}

// golden uHGemHT9045.cpp:2092/:2108 `fMain->BtnTrayEndClick(fMain);`
void mimic_tray_end_rcmd()
{
    fMain->BtnTrayEndClick(fMain);
}

// golden uHGemHT9045.cpp:859 `fMain->ChangeTesterConnect(LastSet.iTester, false, true);`
void mimic_online_mode_change(int iTester)
{
    fMain->ChangeTesterConnect(iTester, false, true);
}

// golden uHGemHT9045.cpp:1077
//   `fMain->SetTemp(true, atof(fMain->edWorkTemperBase->Text.c_str()), atof(fMain->edSoakTime->Text.c_str()));`
void mimic_ec_temperature_apply()
{
    fMain->SetTemp(true,
                   atof(fMain->edWorkTemperBase->Text.c_str()),
                   atof(fMain->edSoakTime->Text.c_str()));
}

// golden uHGemHT9045.cpp:2513 `fMain->ChangePassword();`
void mimic_s2f49_password_change()
{
    fMain->ChangePassword();
}

// golden uHGemHT9045.cpp:2241 `HCACK=fMain->FTClick();`
// golden uHGemHT9045.cpp:2266 `HCACK=fMain->RTClick();`
int mimic_switch_to_ft_rt(bool bToFt)
{
    return bToFt ? fMain->FTClick() : fMain->RTClick();
}

// golden uHGemHT9045.cpp:893-914 (site-on-off decode loop, shape preserved)
void mimic_site_on_off_decode(int x, int y, int z, bool bUseTestSocket[2][4][8])
{
    if (z < fMain->tSiteOnOff[0]->Count)   // StringsProxy has no c_str() -- explicit AnsiString cast (established idiom,
                                            // e.g. database.cpp's TIODATA::TIODATA constructor). NOT expression-for-
                                            // expression with golden here (golden writes `.Strings[z].c_str()`
                                            // directly, uHGemHT9045.cpp:894/914) -- see the header's EXPRESSION LAYER
                                            // note for this one documented exception. AI(W906-W7-F1fix2) 20260729:
                                            // the same site has a SECOND, smaller deviation the header did not
                                            // mention -- golden assigns the atoi() int straight into the bool array
                                            // element (`bUseTestSocket[0][x][y]=atoi(...)`), this probe writes
                                            // `(atoi(...) != 0)` to stay clean under -Wall. Neither deviation
                                            // changes what is being proved (that the fMain-> subexpressions are
                                            // well-formed against the real facade), but "expression for expression"
                                            // is not literally true for this line.
        bUseTestSocket[0][x][y] = (atoi(AnsiString(fMain->tSiteOnOff[0]->Strings[z]).c_str()) != 0);
    if (z < fMain->tSiteOnOff[1]->Count)
        bUseTestSocket[1][x][y] = (atoi(AnsiString(fMain->tSiteOnOff[1]->Strings[z]).c_str()) != 0);
}

// -- the 10 members that ALREADY existed before this wave (plan's "do not
//    duplicate" list + Home, which the plan's list omitted but which was also
//    already present) -- reproduced here too so PART A's claim ("all 26 live
//    members compile against the real facade in this one TU") is total, not
//    partial.
void mimic_preexisting_members()
{
    fMain->Start("probe");
    fMain->ShowTestHeadComp(true);
    fMain->CleanOut("probe");
    fMain->LoadTestModePicture();
    fMain->SetStartModeData();
    fMain->BtnOneCycleClick(fMain);
    fMain->Reset("probe");
    AnsiString s = fMain->cbSetupFileName->Text;
    (void)s;
    fMain->edWorkTemperBase->Text = AnsiString("25.0");
    (void)fMain->Home("probe");
}

} // namespace w7f1_wall2_probe

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("w7_f1_wall2_probe", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    printf("=== W7-F1 Wall-2 unlock probe ===\n");
    // AI(W906-W7-F1fix2) 20260729: this banner used to end with "fMain ...
    // resolves purely via ht9045_secsgem's own PUBLIC dependency on it" while
    // the target's own link line names ht9045_forms explicitly -- the header's
    // LINK LAYER section (corrected twice, now measured) is the authority.
    printf("NOTE  PART A (namespace w7f1_wall2_probe) is a COMPILE-TIME gate:\n");
    printf("NOTE  this executable existing and linking at all means every\n");
    printf("NOTE  fMain-> expression in it -- copied shape-for-shape from\n");
    printf("NOTE  golden SECSGEM/uHGemHT9045.cpp -- compiled clean against the\n");
    printf("NOTE  REAL forms/fMain.h facade, in the SAME translation unit as\n");
    printf("NOTE  SECSGEM/uHGemHT9045.h. This target's link line names\n");
    printf("NOTE  ht9045_forms explicitly (inside a LINK_GROUP:RESCAN), so the\n");
    printf("NOTE  bare-transitive-edge property is NOT what this run shows; see\n");
    printf("NOTE  the LINK LAYER section in this file's header for the measured\n");
    printf("NOTE  split between 'transitive edge delivers ht9045_forms' (true)\n");
    printf("NOTE  and 'the RESCAN group is droppable' (false).\n");

    // -------------------------------------------------------------------
    //  RUNTIME LAYER -- exercise every new member's documented default /
    //  observable behaviour (the "not a bare no-op" gate).
    // -------------------------------------------------------------------
    // AI(W906-W7-F1fix2) 20260729: label corrected -- this target does NOT link
    // "via ht9045_secsgem only" (see the LINK LAYER header section).
    // AI(W906-W7-F1fix3) 20260731 -- TWO further corrections to this one line:
    //  (a) "reached from a ht9045_secsgem consumer TU" was OVERSTATED. Measured
    //      with nm on this TU's own object file: it has 18 undefined symbols, and
    //      the intersection of `nm --undefined-only` on that object with
    //      `nm --defined-only` on libht9045_secsgem.a is EMPTY -- zero Gem/SECS
    //      symbols of any kind. Its only non-libc/libstdc++ externals are `_fMain`
    //      (defined in libht9045_forms.a, `nm`: `_fMain B`) and vclcompat::
    //      StringsProxy's AnsiString conversion operator. So this TU consumes
    //      SECSGEM/uHGemHT9045.h as a HEADER and references not one
    //      ht9045_secsgem SYMBOL. That is exactly the INCLUDE LAYER claim (the
    //      two headers coexist in one TU), not a symbol-level "a SECSGEM consumer
    //      reaches fMain" claim.
    //  (b) The RUNTIME half is a TAUTOLOGY, kept deliberately. fMain is set by a
    //      namespace-scope dynamic initialiser (`TfMain *fMain = new TfMain();`,
    //      forms/fMain.cpp:270), so it can only be null if operator new returned
    //      null. The value of this check is COMPILE-TIME + LINK-TIME (the symbol
    //      exists and resolves); do NOT read its PASS line as behavioural
    //      evidence about anything in forms/.
    check("fMain symbol resolves: non-NULL global defined in ht9045_forms (LINK-TIME check; the runtime non-null is a tautology, see note above)",
          fMain != 0);

    check("W906_cbSetupFileNameChangeCallCount starts at 0", fMain->W906_cbSetupFileNameChangeCallCount == 0);
    fMain->cbSetupFileNameChange(fMain);
    fMain->cbSetupFileNameChange(fMain);
    check("cbSetupFileNameChange() increments its call-count seam (gap is test-observable)",
          fMain->W906_cbSetupFileNameChangeCallCount == 2);

    check("W906_Clarn_DataCallCount starts at 0", fMain->W906_Clarn_DataCallCount == 0);
    fMain->Clarn_Data(8, "test");
    check("Clarn_Data() increments its call-count seam", fMain->W906_Clarn_DataCallCount == 1);

    check("W906_LoadRunModePictureCallCount starts at 0", fMain->W906_LoadRunModePictureCallCount == 0);
    fMain->LoadRunModePicture();
    check("LoadRunModePicture() increments its call-count seam", fMain->W906_LoadRunModePictureCallCount == 1);

    // AI(W906-W7-F1fix3) 20260731: the two CanChangeSite calls below used to vary
    // the ARGUMENT (true, then false) at the SAME TIME as the Sim seam, so the
    // pair pinned only "the return is not a constant" -- NOT "the body reads the
    // seam", which is what its label claimed. MEASURED in both directions with
    // the isolated technique (a deliberately-broken forms/fMain.cpp compiled from
    // an off-tree copy, ar-replaced into a COPY of libht9045_forms.a, linked into
    // a separate exe -- the shared tree was never written):
    //   * facade body replaced by `return bNoIncludeHotplate;` (ignores the seam
    //     entirely and just echoes its argument) -> 42 passed, 0 failed. Every
    //     assertion green against a facade that never reads the seam.
    //   * facade body replaced by `return true;` -> 41 passed, 1 failed.
    // The ARGUMENT is now HELD FIXED at `true` across both calls, so the only
    // thing that varies is the seam -- the same shape the four other Sim seams in
    // this file already use (ChangeTesterConnect(1,false,true) twice,
    // SetTemp(true,25.0,0.0) twice, FTClick() twice, RTClick() twice). Both
    // mutations above now go red. Golden's own call site passes BOTH argument
    // values (uHGemHT9045.cpp:543-544); that both spellings COMPILE is covered by
    // mimic_temperature_site_gate in PART A, which is where an argument-shape
    // check belongs -- it is not something this runtime pair can pin.
    check("W906_CanChangeSite_Sim defaults true (golden's own 'no IC anywhere' fall-through)",
          fMain->W906_CanChangeSite_Sim == true);
    check("CanChangeSite(true) returns the Sim seam's value (true)", fMain->CanChangeSite(true) == true);
    fMain->W906_CanChangeSite_Sim = false;
    check("CanChangeSite READS the Sim seam: argument held fixed at true, seam flipped to false -> return flips",
          fMain->CanChangeSite(true) == false);
    fMain->W906_CanChangeSite_Sim = true;   // restore

    check("W906_BtnTrayEndClickCallCount starts at 0", fMain->W906_BtnTrayEndClickCallCount == 0);
    fMain->BtnTrayEndClick(fMain);
    check("BtnTrayEndClick() increments its call-count seam", fMain->W906_BtnTrayEndClickCallCount == 1);

    check("W906_UpdateMainOperateModeCallCount starts at 0", fMain->W906_UpdateMainOperateModeCallCount == 0);
    fMain->UpdateMainOperateMode();
    check("UpdateMainOperateMode() increments its call-count seam", fMain->W906_UpdateMainOperateModeCallCount == 1);

    check("W906_LoadStartModePictureCallCount starts at 0", fMain->W906_LoadStartModePictureCallCount == 0);
    fMain->LoadStartModePicture();
    check("LoadStartModePicture() increments its call-count seam", fMain->W906_LoadStartModePictureCallCount == 1);

    check("W906_LookForFileCallCount starts at 0", fMain->W906_LookForFileCallCount == 0);
    fMain->LookForFile();
    check("LookForFile() increments its call-count seam", fMain->W906_LookForFileCallCount == 1);

    check("W906_ChangeTesterConnect_Sim defaults 0 (golden success code)", fMain->W906_ChangeTesterConnect_Sim == 0);
    check("ChangeTesterConnect(1,false,true) with the seam at 0 runs the golden body: returns 0 and LastSet.iTester reads back ON_LINE (AI(W906-GB-P2d) 20260926, user ruling)", [](){ const int oldTester = LastSet.iTester; const bool ok = fMain->ChangeTesterConnect(1, false, true) == 0 && LastSet.iTester == 1; fMain->ModifyTester(oldTester); return ok; }());
    fMain->W906_ChangeTesterConnect_Sim = 1;
    check("ChangeTesterConnect is test-DRIVABLE: Sim seam=1 flips the return value",
          fMain->ChangeTesterConnect(1, false, true) == 1);
    fMain->W906_ChangeTesterConnect_Sim = 0;   // restore

    check("W906_SetTemp_Sim defaults 0", fMain->W906_SetTemp_Sim == 0);
    check("SetTemp(...) returns the Sim seam's value", fMain->SetTemp(true, 25.0, 0.0) == 0);
    // AI(W906-W7-F1fix2) 20260729: SetTemp/FTClick/RTClick previously only ever
    // asserted "== 0" against a seam that was never moved off its 0 default --
    // which a hard-coded `return 0;` would satisfy just as well. Drive each seam
    // to a DISTINCT golden return code (golden main.cpp:23897 SetTemp=1;
    // :29700 FTClick=8; :29797 RTClick=7 -- the SPIL gate FTClick has no
    // counterpart for) so the assertion pins "returns the seam", not "returns 0".
    fMain->W906_SetTemp_Sim = 1;
    check("SetTemp is test-DRIVABLE: Sim seam=1 (golden SystemStart block) flips the return value",
          fMain->SetTemp(true, 25.0, 0.0) == 1);
    fMain->W906_SetTemp_Sim = 0;   // restore

    check("W906_ChangePasswordCallCount starts at 0", fMain->W906_ChangePasswordCallCount == 0);
    fMain->ChangePassword();
    check("ChangePassword() increments its call-count seam", fMain->W906_ChangePasswordCallCount == 1);

    check("W906_FTClick_Sim defaults 0 (golden success code)", fMain->W906_FTClick_Sim == 0);
    check("FTClick() returns the Sim seam's value", fMain->FTClick() == 0);
    fMain->W906_FTClick_Sim = 8;
    check("FTClick is test-DRIVABLE: Sim seam=8 (a real golden FTClick code) flips the return value",
          fMain->FTClick() == 8);
    fMain->W906_FTClick_Sim = 0;   // restore
    check("W906_RTClick_Sim defaults 0", fMain->W906_RTClick_Sim == 0);
    check("RTClick() returns the Sim seam's value", fMain->RTClick() == 0);
    fMain->W906_RTClick_Sim = 7;
    check("RTClick is test-DRIVABLE: Sim seam=7 (RTClick's SPIL-gate code, which FTClick never returns) flips it",
          fMain->RTClick() == 7);
    fMain->W906_RTClick_Sim = 0;   // restore

    // tSiteOnOff: REAL concrete storage, not a Sim seam -- pins the golden
    // prefill count (main.cpp:2242-2248) that uHGemHT9045.cpp's
    // `if(z<fMain->tSiteOnOff[0]->Count)` guard depends on.
    check("tSiteOnOff[0] prefilled to MAX_SOCKET_ROW*MAX_SOCKET_COL entries",
          fMain->tSiteOnOff[0]->Count == MAX_SOCKET_ROW * MAX_SOCKET_COL);
    check("tSiteOnOff[1] prefilled to MAX_SOCKET_ROW*MAX_SOCKET_COL entries",
          fMain->tSiteOnOff[1]->Count == MAX_SOCKET_ROW * MAX_SOCKET_COL);
    check("tSiteOnOff[0] entries default \"0\" (golden main.cpp:2246)",
          fMain->tSiteOnOff[0]->Strings[0] == AnsiString("0"));

    // edSoakTime: reused TfLotInfoEdit widget, ->Text read/written exactly
    // like the sibling edWorkTemperBase already in the facade.
    // AI(W906-W7-F1fix3) 20260731: this round-trip is a RUNTIME TAUTOLOGY and is
    // kept for its COMPILE-TIME value only. TfLotInfoEdit is a typedef for
    // vclcompat::TEdit (forms/FormWidgets.h:188), whose ->Text is a PLAIN
    // AnsiString data member inherited from TCustomEdit (vclcompat/Controls.h:264)
    // -- no accessor, no forms/ logic, nothing between the write and the read but
    // AnsiString's own operator= / operator==. What it does buy is real: fMain HAS
    // an edSoakTime, it is a widget carrying a ->Text, and the read/write
    // spellings golden uses compile against it. Do NOT read its PASS line as
    // behavioural coverage. (Contrast the tSiteOnOff checks just above, which DO
    // observe real ctor behaviour: a prefilled Count and a default string value.)
    fMain->edSoakTime->Text = AnsiString("5.0");
    check("edSoakTime->Text round-trips through a plain data member (TfLotInfoEdit stand-in; compile-time existence check, not behavioural coverage)",
          fMain->edSoakTime->Text == AnsiString("5.0"));

    // BtnPauseClick: TRANSLATED (forwards to the pre-existing Pause() virtual),
    // but Pause() is a bare `return false;` -- so the forward is invisible
    // unless Pause() itself is instrumented. Three generations of this block:
    //   1. the wave asserted the literal constant `true` (could never fail);
    //   2. AI(W906-W7-F1fix) 20260729 replaced it with the
    //      W906_BtnPauseClickCallCount checks below -- failable, but only for
    //      "BtnPauseClick ran", NOT for the forward;
    //   3. AI(W906-W7-F1fix2) 20260729 added W906_PauseCallCount /
    //      W906_PauseLastFunc on Pause() itself and the assertions below,
    //      which DO see the forward (and pin the "BtnPauseClick" argument
    //      golden passes, main.cpp:6967).
    // MEASURED, by relinking this probe against a fMain.cpp with exactly one
    // line changed (ar-replacing fMain.cpp.obj inside libht9045_forms.a, so the
    // shared tree was never left broken):
    //   * increment removed  -> 41 passed, 1 failed: "increments its own
    //     call-count seam" FAILS, both forward checks still PASS (correct -- the
    //     forward really did still happen).
    //   * forward removed    -> 40 passed, 2 failed: the two forward checks
    //     FAIL, and "increments its own call-count seam" still PASSES. That
    //     pass is the whole point: it is direct proof that the fix1 seam alone
    //     could not see the forward, and that the gap MEDIUM-1 was raised about
    //     survived the first fix.
    check("W906_BtnPauseClickCallCount starts at 0", fMain->W906_BtnPauseClickCallCount == 0);
    check("W906_PauseCallCount starts at 0 (Pause not yet called by anything in this TU)",
          fMain->W906_PauseCallCount == 0);
    check("W906_PauseLastFunc starts empty", fMain->W906_PauseLastFunc == AnsiString(""));
    fMain->BtnPauseClick(fMain);
    check("BtnPauseClick() increments its own call-count seam (it ran)",
          fMain->W906_BtnPauseClickCallCount == 1);
    check("BtnPauseClick() FORWARDED into Pause() exactly once (the forward itself, not just the call)",
          fMain->W906_PauseCallCount == 1);
    check("BtnPauseClick()'s forward carried golden's own \"BtnPauseClick\" Func argument (golden main.cpp:6967)",
          fMain->W906_PauseLastFunc == AnsiString("BtnPauseClick"));

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
