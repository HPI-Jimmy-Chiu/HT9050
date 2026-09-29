// tests/test_uHGemClass.cpp
// Verification harness for the SECSGEM/uHGemClass (HTGem) + SecsEventType
// (SECS_EVENT) translation.
//
// Golden reference:
//   HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemClass.h/.cpp
//   HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045.h:12-313 (ETypeStruct)
//
// SCOPE (see uHGemClass.cpp's file-head note for the full discrepancy
// writeup): 57 of HTGem's ~90 methods need `THGem` (uHGemEquipment.h, out of
// scope) and are GATED with a conservative-default ACTIVE stub.  This harness
// therefore verifies:
//   (1) the translation compiles + links clean under MinGW g++.  AI(W906-
//       SysModWire) 20260720: this test's link line grew a real THGem/
//       uHGemEquipment.cpp dependency this wave (8 more methods un-gated --
//       see uHGemClass.cpp's "INTEGRATE WAVE 3" note -- call real out-of-line
//       THGem methods) -- see tests/CMakeLists.txt's own updated comment on
//       this target and the new tests/test_uHGemClass_link_stubs.cpp
//       (supplies the two extra gated externals that dependency pulls in;
//       kept in a SEPARATE TU so this file itself stays untouched by that
//       plumbing), AND
//   (2) the genuinely ACTIVE surface behaves per golden: ctors/dtor/
//       UpdateDataPath, the free helper IsCorrectDateFormat, the header's
//       already-fully-inline virtuals (verbatim from golden, e.g.
//       S7F24_FormattedProcessProgramSendAcknowledge()==1), virtual dispatch
//       through a derived override (proves HTGem is a real usable abstract
//       base), the 57 gated stubs' documented conservative defaults, and
//       every SECS_EVENT enumerator's exact golden numeric value.
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure.

#include "SECSGEM/uHGemClass.h"
#include "SECSGEM/SecsEventType.h"
// AI(W906-VCW1) 20260721: SetECValue's UN-GATED IsVCL==1 dynamic_cast
// cascade (below) needs the 6 VCL-widget stand-ins this header supplies --
// see its own file-head scope-boundary note.
#include "vclcompat/Controls.h"
// AI(W906-uHGemClass-Unlock3) 20260723: S2F16_NewEquipmentConstantSendAcknowledge's
// own test section (below) needs a real `THGem` instance (so its
// unconditional `HGemPtr->MoveCheckCallBack` tail dereferences a valid
// pointer, matching golden's own assumption -- see that method's file-head
// comment in uHGemClass.cpp) and the real `HasICUnderMachine()`/
// `HasAnyICInMachine()` predicate declarations to document/assert their
// current conservative-false default.
#include "SECSGEM/uHGemEquipment.h"
#include "csystem.h"
#include <cstdio>
#include <cstring>   // strcpy (test-side buffer prep for SetECValue's void* sink)
#include <string>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
// AI(W5-SECSGEM-Translate) 20260710: IsCorrectDateFormat has external linkage
// in golden (uHGemClass.cpp) but no header declares it there either (it was,
// at the time of this original comment, only ever called from the then-GATED
// S2F32_DateAndTimeAcknowledge body) -- forward-declared here, test-local,
// exactly as golden leaves it undeclared-in-any-header.
// AI(W906-uHGemClass-TraceUnlock) 20260728: S2F32 is UN-GATED as of this
// wave (see the dedicated test section below) -- IsCorrectDateFormat is now
// ALSO called by the real ParseSECSDateTimeString (uHGemClass.h/.cpp, a
// SEPARATE free function with a real header declaration, unlike this one) --
// this forward declaration itself is unaffected and stays exactly as-is.
extern bool IsCorrectDateFormat(int y, int m, int d);

// AI(W906-FastcallFix) 20260720: this test's own local MyDBIProcess (3-arg,
// __fastcall) stub -- previously here, matching the REAL golden 3-arg
// signature (cMyDB.h:20) -- is REMOVED. SECSGEM/uHGemEquipment.cpp now
// supplies the real, externally-linkable, __fastcall-decorated definition of
// this overload (moved out of an anonymous namespace and given __fastcall
// this same wave), and this target's RESCAN link group (tests/CMakeLists.txt)
// already links ht9045_secsgem, the library that carries it, to satisfy
// this file's own uHGemClass.cpp:310 extern+__fastcall declaration. Keeping
// the local stub would now be a duplicate-definition link error, not a
// missing-symbol fix (2026-07-20 audit, AUDIT_fastcall_tree.md finding 1).
// The removed stub's call counter (g_dbiCalls) was incremented but never
// read/asserted anywhere in this file -- confirmed before deletion that no
// PASS/FAIL check depended on it, so no coverage is lost by this removal.
//
// NOTE: this edit knowingly breaks this file's own established "stays
// byte-for-byte unchanged" convention (see tests/test_uHGemClass_link_stubs.cpp's
// file-head note, citing design brief Sec.9 item 5) -- that convention was
// adopted to avoid touching this file for an unrelated (SysModWire) reason;
// it does not anticipate a defect fix that requires deleting code FROM this
// file itself, which cannot be done any other way (a stub left in place here
// collides at link time with the production definition this fix adds
// elsewhere). Flagged explicitly rather than silently overridden.

static int g_pass = 0;
static int g_fail = 0;

static void check_b(const char* name, bool got, bool expected)
{
    if (got == expected) { printf("PASS  %-64s got=%d\n", name, (int)got); ++g_pass; }
    else { printf("FAIL  %-64s got=%d exp=%d\n", name, (int)got, (int)expected); ++g_fail; }
}

static void check_i(const char* name, int got, int expected)
{
    if (got == expected) { printf("PASS  %-64s got=%d\n", name, got); ++g_pass; }
    else { printf("FAIL  %-64s got=%d exp=%d\n", name, got, expected); ++g_fail; }
}

static void check_s(const char* name, const std::string& got, const std::string& expected)
{
    if (got == expected) { printf("PASS  %-64s got=\"%s\"\n", name, got.c_str()); ++g_pass; }
    else { printf("FAIL  %-64s got=\"%s\" exp=\"%s\"\n", name, got.c_str(), expected.c_str()); ++g_fail; }
}

// -----------------------------------------------------------------------
// Derived subclass proving HTGem is a real, instantiable, polymorphic base
// (mirrors the role uHGemHT9045.h's HT9045Gem plays in golden, minimally).
// -----------------------------------------------------------------------
class TestGem : public HTGem
{
public:
    int s1f1Calls = 0;
    void S1F1_AreYouThereRequest() override
    {
        ++s1f1Calls;   // override does NOT chain to HTGem's gated stub -- fine,
                       // that is exactly what a real site-specific override does.
    }
};

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("uHGemClass", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    printf("=== SECSGEM/uHGemClass (HTGem) + SecsEventType (SECS_EVENT) translation verification ===\n");
    printf("(as of W906-SysModWire: real THGem/uHGemEquipment link dependency -- see this file's own updated file-head note)\n\n");

    // -----------------------------------------------------------------------
    // Construction / destruction (ACTIVE, faithful -- golden :34-79)
    // -----------------------------------------------------------------------
    printf("-- ctors / dtor / UpdateDataPath (golden :34-84) --\n");
    {
        HTGem g;
        check_b("default ctor: HGemPtr==NULL", g.HGemPtr == NULL, true);
        check_s("default ctor: HandlerPath==\"\"", g.HandlerPath.c_str(), "");
        check_b("default ctor: SecsAlarmMessage != NULL", g.SecsAlarmMessage != NULL, true);
        check_b("default ctor: FMessageList != NULL", g.FMessageList != NULL, true);

        g.UpdateDataPath("D:\\HT9045\\system");
        check_s("UpdateDataPath sets DataPath", g.DataPath.c_str(), "D:\\HT9045\\system");
    }   // dtor runs here -- must not crash / must not leak (ASan not available
        // in this environment; scope-exit-without-crash is the practical proof).

    {
        HTGem g2("HT9045");
        check_s("AnsiString-ctor: HandlerPath==Path", g2.HandlerPath.c_str(), "HT9045");
        check_b("AnsiString-ctor: SecsAlarmMessage != NULL", g2.SecsAlarmMessage != NULL, true);
    }

    {
        HTGem g3((THGem*)NULL);   // THGem is only forward-declared; NULL never derefs it.
        check_b("THGem*-ctor(NULL): HGemPtr==NULL", g3.HGemPtr == NULL, true);
    }

    // -----------------------------------------------------------------------
    // IsCorrectDateFormat (ACTIVE, faithful leap-year ladder -- golden :17-32)
    // -----------------------------------------------------------------------
    printf("\n-- IsCorrectDateFormat (golden :17-32) --\n");
    check_b("2024-02-29 leap year -> true",  IsCorrectDateFormat(2024, 2, 29), true);
    check_b("2023-02-29 non-leap -> false",  IsCorrectDateFormat(2023, 2, 29), false);
    check_b("1900-02-29 %100==0 not %400 -> false", IsCorrectDateFormat(1900, 2, 29), false);
    check_b("2000-02-29 %400==0 -> is a leap year, but y<=2000 rejected -> false",
            IsCorrectDateFormat(2000, 2, 29), false);
    check_b("2400-02-29 %400==0 -> true", IsCorrectDateFormat(2400, 2, 29), true);
    check_b("2025-13-01 bad month -> false", IsCorrectDateFormat(2025, 13, 1), false);
    check_b("2025-00-01 bad month -> false", IsCorrectDateFormat(2025, 0, 1), false);
    check_b("2025-04-31 April has 30 days -> false", IsCorrectDateFormat(2025, 4, 31), false);
    check_b("2025-04-30 -> true", IsCorrectDateFormat(2025, 4, 30), true);
    check_b("10000-01-01 y>9999 -> false", IsCorrectDateFormat(10000, 1, 1), false);

    // -----------------------------------------------------------------------
    // Already-inline virtuals -- carried over VERBATIM from golden's header,
    // zero gating (golden itself gives them a trivial body).
    // -----------------------------------------------------------------------
    printf("\n-- already-inline virtuals (verbatim from golden .h, zero gating) --\n");
    {
        HTGem g;
        g.AddSV(); g.AddEC(); g.AddAlarmList(); g.AddCEID(); g.AddReprot();     // must not crash
        g.ReloadParameter(); g.LookForFile();
        g.S7F4_ProcessProgramAcknowledge();
        g.S7F6_ProcessProgramData();
        g.S7F6_ProcessProgramData("dummy.stp");
        check_i("S7F24_FormattedProcessProgramSendAcknowledge()==1 (golden literal)",
                g.S7F24_FormattedProcessProgramSendAcknowledge(), 1);
        check_i("ProcessS7F23FromatReceipe()==2 (golden literal)",
                g.ProcessS7F23FromatReceipe(), 2);
        check_i("S7F26_FormattedProcessProgramData()==1 (golden literal)",
                g.S7F26_FormattedProcessProgramData(), 1);
        check_i("ProcessS7F25FromatReceipe()==2 (golden literal)",
                g.ProcessS7F25FromatReceipe(), 2);
        g.S14F4_Get2DID_BinCode();
        g.S110F5_RequestCustomerNameList(); g.S110F6_ListCustomerName();
        g.S110F7_RequestReceipeInformation(); g.S110F8_ListReceipeInformation();
        g.S120F1_RequestReceipeSetupFile(); g.S120F2_ListReceipeSetupFile();
        g.S125F4_LevelSettingChangeAcknowledge();
        printf("PASS  all already-inline virtuals callable without crash\n");
        ++g_pass;
    }

    // -----------------------------------------------------------------------
    // Virtual dispatch through a derived override (proves HTGem is a real,
    // usable, instantiable abstract base -- the role uHGemHT9045's HT9045Gem
    // plays in golden).
    // -----------------------------------------------------------------------
    printf("\n-- virtual dispatch through derived override --\n");
    {
        TestGem tg;
        HTGem* base = &tg;
        base->S1F1_AreYouThereRequest();
        base->S1F1_AreYouThereRequest();
        check_i("derived override invoked via base pointer (2 calls)", tg.s1f1Calls, 2);
    }

    // -----------------------------------------------------------------------
    // Gated stubs (47 of the original 57 remain gated after the 20260713
    // integrate wave -- see uHGemClass.cpp's file-head "INTEGRATE WAVE" note)
    // -- spot-check a representative sample across void / int-ack /
    // AnsiString-param shapes; every one must be callable without crashing
    // and return its documented conservative default.
    // -----------------------------------------------------------------------
    printf("\n-- gated stubs: conservative defaults, no crash (sample) --\n");
    {
        HTGem g;
        // void, no-arg (representative of the still-gated void/no-arg stubs)
        // AI(W906-SysModWire) 20260720: S1F1_AreYouThereRequest/S1F2_OnLineData
        // MOVED OUT of this sample (same "moved out" precedent as the
        // S2F42/CheckECValue/S7F2/S2F15_Update/S2F15_Check comment above) --
        // this wave un-gated both (see uHGemClass.cpp's "INTEGRATE WAVE 3"
        // note). They are NO LONGER blanket-safe on a default-constructed `g`:
        // both now dereference `HGemPtr` for real THGem state
        // (bS1F2_OnLineData / CheckSFFormatOnlyHead+GemMDLN+GemSOFTREV), and
        // `g`'s HGemPtr is NULL here (golden pre-AddSV UB window, deliberately
        // un-guarded -- see design brief risk R8). This TU only
        // forward-declares THGem (cannot construct one to wire HGemPtr for
        // real), so their real behavior is exercised over in
        // tests/test_uHGemEquipment.cpp instead, where a real THGem instance
        // exists (design brief's W2/W3/W4/W5 tests). Replaced here with two
        // more still-gated void/no-arg stubs to keep the sample's width.
        g.S1F4_SelectedStatusReply();
        g.S1F12_StatusVariableNamelistReply();
        g.S1F24_CollectionEventNamelist();
        g.S6F16_EventReportData();
        g.S103F12_StatusVariableNamelistReply();
        printf("PASS  void/no-arg gated stubs callable without crash\n");
        ++g_pass;

        // int-returning ack stubs -- conservative default documented in each
        // uHGemClass.cpp stub comment. (S2F42_Host_Command_Acknowledge,
        // CheckECValue, S7F2_ProcessProgramLoadGrant,
        // S2F15_UpdateNewEquipmentConstant, S2F15_CheckNewEquipmentConstant
        // moved out of this sample -- see the UN-GATED section below, they
        // are no longer blanket stubs.)
        // AI(W906-uHGemClass-TraceUnlock) 20260728: S2F24_TraceInitializeAcknowledgeSub
        // MOVED OUT of this sample -- same "moved out" precedent as S1F1/S1F2/
        // S2F34/S2F36 above (this wave un-gated it, see uHGemClass.cpp's own
        // "INTEGRATE WAVE" note). It is NO LONGER a blanket conservative-
        // default-1 stub: on THIS default-constructed `g` (empty
        // WireCodec.SReceiveData), its own very first read
        // (`ActiveWire->DataItemIn(5,LIST_TYPE,NULL)`) now fails for real and
        // returns -1 (not 1) -- still safe to call (the format-error early
        // exit happens BEFORE any `HGemPtr` dereference, so `g`'s NULL
        // HGemPtr is never touched here), but asserting a hardcoded "1" would
        // now be WRONG, not just stale. Real behavior (all 2/-1/1/4/0 return
        // paths) is exercised in its own dedicated section below, with a real
        // THGem instance wired (this TU only forward-declares THGem, cannot
        // construct one here). Every one of the now-1 still-gated methods
        // left in uHGemClass.cpp (S7F20_CurrentEPPDData) is void-returning
        // (grepped) -- no like-for-like int-ack replacement exists to keep
        // this sample's width; not replaced.
        // AI(W906-AlarmReportAck) 20260721: S2F34_DefineReportAcknowledgeSub/
        // S2F36_LinkEventReportAcknowledgeSub MOVED OUT of this sample -- same
        // "moved out" precedent as S1F1/S1F2 above (this wave un-gated both,
        // see uHGemClass.cpp's "INTEGRATE WAVE 5" note). They are NO LONGER
        // blanket-safe on a default-constructed `g`: both now dereference
        // `HGemPtr->slTempReportID`/`HGemPtr->slTempCeID` for real THGem
        // state, and `g`'s HGemPtr is NULL here (golden pre-AddSV UB window,
        // deliberately un-guarded -- see design brief risk R8). This TU only
        // forward-declares THGem (cannot construct one to wire HGemPtr for
        // real), so their real behavior is exercised over in
        // tests/test_uHGemEquipment.cpp instead, where a real THGem instance
        // exists.

        // SetECValue -- void, two args, must not crash even with a NULL sink.
        // UN-GATED as of AI(W906-VCW1) 20260721 (see the dedicated section
        // below for real dispatch coverage); still exercised once here with
        // an unregistered ECID (70) to confirm the not-found path (i<0) is
        // a harmless no-op, matching golden's own falls-off-the-end-of-the-
        // if body for that case.
        int dummy = 0;
        g.SetECValue(70, &dummy);
        printf("PASS  SetECValue(unregistered ECID) callable without crash\n");
        ++g_pass;
    }

    // -----------------------------------------------------------------------
    // AI(W906-VCW1) 20260721: SetECValue UN-GATED -- vclcompat/Controls.h
    // dynamic_cast dispatch coverage (golden uHGemClass.cpp:2682-2882).
    // SCOPE REMINDER: this proves the CAST-DISPATCH CODE compiles and
    // dispatches correctly against real instances of the 6 new stand-in
    // types constructed BY THIS TEST -- it does NOT mean any real widget-
    // backed EC from uHGemHT9045_EC.cpp is functional (none exist yet; see
    // vclcompat/Controls.h's own file-head note).
    // -----------------------------------------------------------------------
    printf("\n-- SetECValue: UN-GATED VCL-widget dynamic_cast dispatch --\n");
    {
        // (c) each of the 6 new stand-in types' dynamic_cast against an
        // UNRELATED sibling type correctly resolves to nullptr -- the "safe
        // no-op for unbacked types" property this whole wave rests on,
        // confirmed explicitly here rather than just relying on it
        // compiling. Also confirms TStringList (pre-existing, gained its
        // TObject base via this wave's own R1 step) is a correct match/
        // non-match partner too.
        TPanel panelInstance;
        TObject *asPanelObj = &panelInstance;
        check_b("dynamic_cast<TPanel*> matches a real TPanel instance",
                dynamic_cast<TPanel*>(asPanelObj) != NULL, true);
        check_b("dynamic_cast<TCustomEdit*> vs TPanel instance -> nullptr",
                dynamic_cast<TCustomEdit*>(asPanelObj) == NULL, true);
        check_b("dynamic_cast<TComboBox*> vs TPanel instance -> nullptr",
                dynamic_cast<TComboBox*>(asPanelObj) == NULL, true);
        check_b("dynamic_cast<TLabel*> vs TPanel instance -> nullptr",
                dynamic_cast<TLabel*>(asPanelObj) == NULL, true);
        check_b("dynamic_cast<TCheckBox*> vs TPanel instance -> nullptr",
                dynamic_cast<TCheckBox*>(asPanelObj) == NULL, true);
        check_b("dynamic_cast<TRadioGroup*> vs TPanel instance -> nullptr",
                dynamic_cast<TRadioGroup*>(asPanelObj) == NULL, true);
        check_b("dynamic_cast<TStringList*> vs TPanel instance -> nullptr",
                dynamic_cast<TStringList*>(asPanelObj) == NULL, true);

        TStringList realList;
        TObject *asListObj = &realList;
        check_b("dynamic_cast<TStringList*> matches a real TStringList instance (R1)",
                dynamic_cast<TStringList*>(asListObj) != NULL, true);
        check_b("dynamic_cast<TPanel*> vs TStringList instance -> nullptr",
                dynamic_cast<TPanel*>(asListObj) == NULL, true);

        // (a) SetECValue writing to a REAL TStringList*-backed EC (VCL_NAME
        // =="1") -- the one widget type with a real instance possible today
        // via SetECDataPointer's TObject* overload (R1 gave TStringList the
        // `: public TObject` base this dynamic_cast needs). Registered by
        // directly populating HTGem's own SvEcReg parallel lists (matching
        // exactly what SetECValue itself reads) rather than through
        // SetECDataPointer, to sidestep that overload's own documented
        // registration-time ASCII-hazard (see SecsSvEcRegistration.cpp's
        // file-head note) -- irrelevant to what THIS test is verifying.
        HTGem g;
        TStringList *slEc = new TStringList();
        g.SvEcReg.EC_ID->Add(AnsiString(200));
        g.SvEcReg.EC_TYPE->Add(AnsiString((int)HType.ASCII_TYPE));
        g.SvEcReg.EC_NAME->Add("TestStrEC");
        g.SvEcReg.EC_UNIT->Add("unit");
        g.SvEcReg.EC_Ptr->Add((void*)slEc);
        g.SvEcReg.EC_VCL_NAME->Add("1");

        char strBuf[64];
        strcpy(strBuf, "hello,world");
        g.SetECValue(200, strBuf);
        check_s("SetECValue: TStringList-backed EC (StringListPtr branch) -> CommaText written",
                slEc->GetCommaText().str(), "hello,world");

        // Second widget type, end to end through the SAME real SetECValue
        // dispatch -- proves the dynamic_cast cascade selects the CORRECT
        // branch out of all 7 candidates (not just "compiles"), and that
        // TCheckBox's ->Checked write-through (S!="0") is byte-for-byte
        // golden's own logic.
        TCheckBox cbEc;
        g.SvEcReg.EC_ID->Add(AnsiString(202));
        g.SvEcReg.EC_TYPE->Add(AnsiString((int)HType.BOOLEAN_TYPE));
        g.SvEcReg.EC_NAME->Add("TestCheckEC");
        g.SvEcReg.EC_UNIT->Add("unit");
        g.SvEcReg.EC_Ptr->Add((void*)&cbEc);
        g.SvEcReg.EC_VCL_NAME->Add("1");
        unsigned char boolByte = 1;   // non-"0" byte -> S!="0" -> Checked=true
        g.SetECValue(202, &boolByte);
        check_b("SetECValue: TCheckBox-backed EC (CheckBoxPtr branch) -> Checked==true",
                cbEc.Checked, true);

        // (b) SetECValue's IsVCL==2 bare-return asymmetry (golden :2718-2721,
        // preserved verbatim): an EC registered via the AnsiString* overload
        // is host-READABLE (GetECDataValue) but silently NON-WRITABLE via
        // SetECValue. Assert the stored value is genuinely UNCHANGED after a
        // SetECValue call, proving the preserved golden bug (this
        // translation did not silently "fix" it into an actual write-through).
        AnsiString ecBacking = "original";
        g.SvEcReg.EC_ID->Add(AnsiString(201));
        g.SvEcReg.EC_TYPE->Add(AnsiString((int)HType.ASCII_TYPE));
        g.SvEcReg.EC_NAME->Add("TestAnsiEC");
        g.SvEcReg.EC_UNIT->Add("unit");
        g.SvEcReg.EC_Ptr->Add((void*)&ecBacking);
        g.SvEcReg.EC_VCL_NAME->Add("2");
        char strBuf2[64];
        strcpy(strBuf2, "attempted-overwrite");
        g.SetECValue(201, strBuf2);
        check_s("SetECValue: IsVCL==2 (AnsiString*-backed) EC -> bare return, value UNCHANGED (golden asymmetry)",
                ecBacking.str(), "original");

        // Non-VCL (raw-pointer, VCL_NAME=="0") scalar write-through still
        // works exactly as before this wave (unaffected by the IsVCL==1/2
        // additions above) -- spot-check one scalar type end to end.
        int rawEcValue = 0;
        g.SvEcReg.EC_ID->Add(AnsiString(203));
        g.SvEcReg.EC_TYPE->Add(AnsiString((int)HType.INT_4_TYPE));
        g.SvEcReg.EC_NAME->Add("TestRawEC");
        g.SvEcReg.EC_UNIT->Add("unit");
        g.SvEcReg.EC_Ptr->Add((void*)&rawEcValue);
        g.SvEcReg.EC_VCL_NAME->Add("0");
        int newRawValue = 777;
        g.SetECValue(203, &newRawValue);
        check_i("SetECValue: raw-pointer (VCL_NAME==\"0\") EC -> *Ptr=*Ptr2 scalar write-through",
                rawEcValue, 777);
    }

    // -----------------------------------------------------------------------
    // UN-GATED (10 methods, integrate wave 20260713): S9F1/S9F3/S9F5/S9F7/
    // S9F9_*, S2F24_TraceInitializeAcknowledge, S2F26_DiagnosticLoopbackData,
    // S2F42_Host_Command_Acknowledge, S2F44_ResetSpoolingAcknowledge,
    // CheckECValue. These now call HTGem's own `WireCodec`/`SvEcReg` engine
    // members for real instead of returning a blanket conservative default --
    // verified here via WireCodec/SvEcReg OBSERVABLE STATE, not just
    // "does not crash". See uHGemClass.cpp's file-head "INTEGRATE WAVE" note
    // for exactly why these 10 (of 57) and not more.
    //
    // UN-GATED WAVE 2 (3 MORE, integrate wave 20260716, 13/57 total):
    // S7F2_ProcessProgramLoadGrant, S2F15_UpdateNewEquipmentConstant,
    // S2F15_CheckNewEquipmentConstant -- unblocked once SecsWireCodec gained
    // GetDataItemLenAndTypeAndDelete + SendInvalidDataMessageToHost (see
    // uHGemClass.cpp's "INTEGRATE WAVE 2" note). IMPORTANT: unlike the 47
    // still-gated stubs, these 3 (like the first 10) no longer return a
    // blanket conservative default on ANY input -- their return value now
    // genuinely depends on what is seeded into WireCodec.SReceiveData, so
    // each gets its own format-error-path AND success-path case below
    // (see uHGemClass.cpp's "INTEGRATE WAVE 2" note for exactly why these 3
    // and not more).
    // -----------------------------------------------------------------------
    printf("\n-- UN-GATED methods: real WireCodec/SvEcReg behavior --\n");
    {
        // S9F1/S9F3/S9F5/S9F7/S9F9 -- identical 4-call shape (StringOut +
        // InitLocalHead(9,<F>,0) + DataItemOut(ASCII,S) + SendLocalData).
        // Verify the real StringOut side effect (LogDataString gains the
        // exact message) and the real InitLocalHead side effect (Local.
        // MessageID_S/F set from the S,F code) -- a blanket stub could do
        // neither.
        HTGem g;
        g.S9F7_IllegalData("format error test");
        check_i("S9F7_IllegalData: WireCodec.LogDataString gained a line",
                g.WireCodec.LogDataString->Count, 1);
        check_s("S9F7_IllegalData: LogDataString[0] == the message",
                g.WireCodec.LogDataString->GetString(0).c_str(), "format error test");
        check_i("S9F7_IllegalData: Local.MessageID_S == 9 (InitLocalHead(9,7,0))",
                g.WireCodec.Local.MessageID_S, 9);
        check_i("S9F7_IllegalData: Local.MessageID_F == 7",
                g.WireCodec.Local.MessageID_F, 7);

        g.S9F1_UnrecognizedDeviceID("dev id test");
        check_i("S9F1_UnrecognizedDeviceID: Local.MessageID_F == 1", g.WireCodec.Local.MessageID_F, 1);
        g.S9F3_Unrecognized_Stream_Function_Type("s9f3 test");
        check_i("S9F3_Unrecognized_Stream_Function_Type: Local.MessageID_F == 3", g.WireCodec.Local.MessageID_F, 3);
        g.S9F5_UnrecognizedFunctionType("s9f5 test");
        check_i("S9F5_UnrecognizedFunctionType: Local.MessageID_F == 5", g.WireCodec.Local.MessageID_F, 5);
        g.S9F9_TransactionTimerTimeout("s9f9 test");
        check_i("S9F9_TransactionTimerTimeout: Local.MessageID_F == 9", g.WireCodec.Local.MessageID_F, 9);

        // S2F44_ResetSpoolingAcknowledge -- pure InitLocalHead+DataItemOut
        // burst; verify the wire cursor actually advanced past the bare
        // 14-byte header (proves DataItemOut really ran, not just InitLocalHead).
        g.S2F44_ResetSpoolingAcknowledge();
        check_i("S2F44_ResetSpoolingAcknowledge: Local.MessageID_S == 2", g.WireCodec.Local.MessageID_S, 2);
        check_i("S2F44_ResetSpoolingAcknowledge: Local.MessageID_F == 44", g.WireCodec.Local.MessageID_F, 44);
        check_b("S2F44_ResetSpoolingAcknowledge: LocalLength_4 advanced past bare header",
                g.WireCodec.LocalLength_4 > 14, true);

        // S2F26_DiagnosticLoopbackData -- format-error path: seed a
        // NON-BINARY item (ASCII) so GetDataItemLenAndType's peek is fully
        // deterministic (Type gets a real value, no uninitialized-read
        // risk) and the `Type==HType.BINARY_TYPE` check is reliably false ->
        // falls to S9F7_IllegalData.
        {
            HTGem g2;
            g2.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g2.WireCodec.SReceiveData->Add(AnsiString(3));
            g2.S2F26_DiagnosticLoopbackData();
            check_i("S2F26 with non-BINARY peeked item -> falls to S9F7 (MessageID_F==7)",
                    g2.WireCodec.Local.MessageID_F, 7);
        }
        // S2F26_DiagnosticLoopbackData -- success path: seed a real
        // BINARY_TYPE item (Type token, len token, len data-value tokens) so
        // GetDataItemLenAndType + DataItemIn both succeed -> real
        // InitLocalHead(2,26,0)+DataItemOut(BINARY)+SendLocalData echo.
        {
            HTGem g3;
            g3.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
            g3.WireCodec.SReceiveData->Add(AnsiString(3));
            g3.WireCodec.SReceiveData->Add(AnsiString(0xAA));
            g3.WireCodec.SReceiveData->Add(AnsiString(0xBB));
            g3.WireCodec.SReceiveData->Add(AnsiString(0xCC));
            g3.S2F26_DiagnosticLoopbackData();
            check_i("S2F26 with seeded BINARY item -> real echo (MessageID_S==2)",
                    g3.WireCodec.Local.MessageID_S, 2);
            check_i("S2F26 with seeded BINARY item -> real echo (MessageID_F==26)",
                    g3.WireCodec.Local.MessageID_F, 26);
        }

        // S2F42_Host_Command_Acknowledge -- empty SReceiveData -> DataItemIn
        // returns -1 immediately (SReceiveData->Count==0 is the very FIRST
        // check, before any local variable is touched -- deterministic, no
        // uninitialized-read risk) -> HCACK=3 (golden's own quirk: this path
        // never calls InitLocalHead first, preserved verbatim).
        {
            HTGem g4;
            check_i("S2F42 with empty SReceiveData -> HCACK=3 (format error, golden quirk)",
                    g4.S2F42_Host_Command_Acknowledge(), 3);
        }
        // S2F42_Host_Command_Acknowledge -- seeded success path: <L,2 <A,3
        // "ABC">> so DataItemIn(2,LIST,NULL) then the ASCII decode both
        // succeed -> HCACK=1 (golden's own dead if(HCACK==0)/else split,
        // where HCACK is unconditionally 1 either way, preserved verbatim).
        {
            HTGem g5;
            g5.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g5.WireCodec.SReceiveData->Add(AnsiString(2));
            g5.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g5.WireCodec.SReceiveData->Add(AnsiString(3));
            g5.WireCodec.SReceiveData->Add(AnsiString("ABC"));
            check_i("S2F42 with seeded LIST+ASCII command -> HCACK=1 (accept)",
                    g5.S2F42_Host_Command_Acknowledge(), 1);
        }

        // S2F24_TraceInitializeAcknowledge -- thin wrapper.
        // AI(W906-uHGemClass-TraceUnlock) 20260728: UPDATED -- its own Sub()
        // sibling is UN-GATED as of this wave (was previously a blanket
        // conservative-default-1 stub, which made this wrapper always take
        // the `else` branch -> LocalAcknowledge(2,24,1)). On THIS
        // default-constructed `g6` (empty WireCodec.SReceiveData), Sub()'s
        // own very first read (`ActiveWire->DataItemIn(5,LIST_TYPE,NULL)`)
        // now fails for real and returns -1 (format error, BEFORE touching
        // HGemPtr -- see the "gated stubs" sample's own updated comment
        // above) -> the wrapper's `if(ret==-1)` branch fires instead:
        // `S9F7_IllegalData("S2,F23 Format error !!!")`, NOT
        // LocalAcknowledge(2,24,...). Sub()'s own full success/2/4/1/-1
        // behavior (needing a real THGem for HGemPtr) is covered in its own
        // dedicated section below.
        {
            HTGem g6;
            g6.S2F24_TraceInitializeAcknowledge();
            check_i("S2F24_TraceInitializeAcknowledge with empty SReceiveData: Sub()=-1 -> falls to S9F7 (MessageID_S==9)",
                    g6.WireCodec.Local.MessageID_S, 9);
            check_i("S2F24_TraceInitializeAcknowledge with empty SReceiveData: Sub()=-1 -> falls to S9F7 (MessageID_F==7)",
                    g6.WireCodec.Local.MessageID_F, 7);
        }

        // CheckECValue -- not-found path matches golden's own default (1),
        // now reached via a REAL SvEcReg.EC_ID->IndexOf lookup instead of a
        // blanket stub -- proved by ALSO exercising the found/in-range/
        // out-of-range paths, which a blanket stub could never do.
        check_i("CheckECValue(\"999\", NULL): not registered -> golden not-found default",
                g.CheckECValue("999", NULL), 1);

        int ecRaw = 50;
        g.SvEcReg.SetECDataPointer(AnsiString("100"), HType.INT_4_TYPE, "TestEC", "unit",
                                    (void*)&ecRaw, 0, 100, 50, "remark");
        int inRange = 30, outHigh = 999, outLow = -5;
        check_i("CheckECValue: 30 in [0,100] -> 0 (ok)", g.CheckECValue("100", &inRange), 0);
        check_i("CheckECValue: 999 > max 100 -> 3 (out of range)", g.CheckECValue("100", &outHigh), 3);
        check_i("CheckECValue: -5 < min 0 -> 3 (out of range)", g.CheckECValue("100", &outLow), 3);

        // -------------------------------------------------------------------
        // S7F2_ProcessProgramLoadGrant (golden uHGemClass.cpp:2081-2113,
        // UN-GATED 20260716) -- format-error path (empty SReceiveData):
        // WireCodec.DataItemIn(2,LIST_TYPE,NULL) hits SReceiveData->Count==0
        // -> DataItemInSub's very first check -> -1 -> falls straight to
        // S9F7_IllegalData("S7,F1 Data Format error !!!") -> return 0.
        // -------------------------------------------------------------------
        {
            HTGem g7;
            int ret = g7.S7F2_ProcessProgramLoadGrant();
            check_i("S7F2 with empty SReceiveData -> 0 (format error)", ret, 0);
            check_i("S7F2 format error: Local.MessageID_S == 9 (S9F7_IllegalData)",
                    g7.WireCodec.Local.MessageID_S, 9);
            check_i("S7F2 format error: Local.MessageID_F == 7",
                    g7.WireCodec.Local.MessageID_F, 7);
        }
        // Success path: <L,2 <A,3 "ABC"> <U1,1 42>> -- a well-formed PPGNT
        // body (PPID="ABC", a 1-byte numeric LENGTH item of any of the 8
        // accepted numeric types).
        {
            HTGem g8;
            g8.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g8.WireCodec.SReceiveData->Add(AnsiString(2));
            g8.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g8.WireCodec.SReceiveData->Add(AnsiString(3));
            g8.WireCodec.SReceiveData->Add(AnsiString("ABC"));
            g8.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_1_TYPE));
            g8.WireCodec.SReceiveData->Add(AnsiString(1));
            int ret = g8.S7F2_ProcessProgramLoadGrant();
            check_i("S7F2 with well-formed PPGNT body -> 1 (grant)", ret, 1);
            check_i("S7F2 grant: Local.MessageID_S == 7 (LocalAcknowledge(7,2,0))",
                    g8.WireCodec.Local.MessageID_S, 7);
            check_i("S7F2 grant: Local.MessageID_F == 2",
                    g8.WireCodec.Local.MessageID_F, 2);
            check_i("S7F2 grant: fully consumes its seeded burst",
                    g8.WireCodec.SReceiveData->Count, 0);
        }

        // -------------------------------------------------------------------
        // S2F15_UpdateNewEquipmentConstant (golden uHGemClass.cpp:2884-3024,
        // UN-GATED 20260716).
        // -------------------------------------------------------------------
        {
            // Format-error path: empty SReceiveData -> GetDataItemLenAndTypeAndDelete
            // returns -2 (Count<2) -> outer if fails -> return -1.
            HTGem g9;
            check_i("S2F15_Update with empty SReceiveData -> -1", g9.S2F15_UpdateNewEquipmentConstant(), -1);
        }
        {
            // EClen==0 (an empty <L,0>) -- golden's for-loop simply never
            // runs; falls straight through to ReloadParameter()+return 0.
            HTGem g10;
            g10.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g10.WireCodec.SReceiveData->Add(AnsiString(0));
            check_i("S2F15_Update with <L,0> (no ECs) -> 0 (success, no-op)",
                    g10.S2F15_UpdateNewEquipmentConstant(), 0);
        }
        {
            // Full single-EC round trip: <L,1 <L,2 <A,3 "100"> <U1,1 42>>> --
            // one EC (ECID="100") set to a UINT_1 value of 42. SetECValue
            // itself is STILL gated (no observable side effect from the set
            // itself), so this test's oracle is that WireCodec correctly
            // walks/consumes the ENTIRE nested burst without misparsing --
            // proved by SReceiveData->Count==0 afterward -- and the function
            // still returns golden's success code (0).
            HTGem g11;
            g11.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));   // outer <L,1
            g11.WireCodec.SReceiveData->Add(AnsiString(1));
            g11.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));   // per-EC <L,2
            g11.WireCodec.SReceiveData->Add(AnsiString(2));
            g11.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));  // ECID
            g11.WireCodec.SReceiveData->Add(AnsiString(3));
            g11.WireCodec.SReceiveData->Add(AnsiString("100"));
            g11.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_1_TYPE)); // EC value
            g11.WireCodec.SReceiveData->Add(AnsiString(1));
            g11.WireCodec.SReceiveData->Add(AnsiString(42));
            int ret = g11.S2F15_UpdateNewEquipmentConstant();
            check_i("S2F15_Update with 1 well-formed UINT_1 EC -> 0 (success)", ret, 0);
            check_i("S2F15_Update fully consumes its seeded burst",
                    g11.WireCodec.SReceiveData->Count, 0);
        }
        {
            // Trailing "error format" else-branch: EC value Type byte
            // matches none of golden's known HType constants -> falls to
            // `WireCodec.SendInvalidDataMessageToHost("error format"); return -1;`
            // (0x01 is not any HType.*_TYPE literal -- see SecsWireCodec.cpp's
            // g_HTypeInit literal table).
            HTGem g12;
            g12.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g12.WireCodec.SReceiveData->Add(AnsiString(1));
            g12.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g12.WireCodec.SReceiveData->Add(AnsiString(2));
            g12.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g12.WireCodec.SReceiveData->Add(AnsiString(3));
            g12.WireCodec.SReceiveData->Add(AnsiString("100"));
            g12.WireCodec.SReceiveData->Add(AnsiString(1));    // unrecognized Type byte
            g12.WireCodec.SReceiveData->Add(AnsiString(1));
            int ret = g12.S2F15_UpdateNewEquipmentConstant();
            check_i("S2F15_Update with unrecognized EC value Type -> -1 (error format)", ret, -1);
            check_i("S2F15_Update error format: Local.MessageID_S == 9 (SendInvalidDataMessageToHost)",
                    g12.WireCodec.Local.MessageID_S, 9);
            check_i("S2F15_Update error format: Local.MessageID_F == 7",
                    g12.WireCodec.Local.MessageID_F, 7);
        }

        // -------------------------------------------------------------------
        // S2F15_CheckNewEquipmentConstant (golden uHGemClass.cpp:3026-3190,
        // UN-GATED 20260716).
        // -------------------------------------------------------------------
        {
            // Format-error path: empty SReceiveData -> -1 (same guard as Update).
            HTGem g13;
            check_i("S2F15_Check with empty SReceiveData -> -1", g13.S2F15_CheckNewEquipmentConstant(), -1);
        }
        {
            // EClen==0: UNLIKE Update, Check has its own explicit
            // `if(EClen<1) return -1;` guard (golden's own asymmetry between
            // the two siblings, preserved verbatim).
            HTGem g14;
            g14.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g14.WireCodec.SReceiveData->Add(AnsiString(0));
            check_i("S2F15_Check with <L,0> (EClen<1) -> -1 (golden's own guard)",
                    g14.S2F15_CheckNewEquipmentConstant(), -1);
        }
        {
            // Full single-EC round trip, in-range value -> CheckECValue's
            // real registered-bounds check (via SvEcReg), not a stub --
            // proves this un-gated method actually reaches CheckECValue.
            HTGem g15;
            int ecRaw2 = 50;
            g15.SvEcReg.SetECDataPointer(AnsiString("200"), HType.INT_4_TYPE, "TestEC2", "unit",
                                          (void*)&ecRaw2, 0, 100, 50, "remark2");
            g15.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g15.WireCodec.SReceiveData->Add(AnsiString(1));
            g15.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g15.WireCodec.SReceiveData->Add(AnsiString(2));
            g15.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g15.WireCodec.SReceiveData->Add(AnsiString(3));
            g15.WireCodec.SReceiveData->Add(AnsiString("200"));
            g15.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
            g15.WireCodec.SReceiveData->Add(AnsiString(1));
            g15.WireCodec.SReceiveData->Add(AnsiString(30));   // in [0,100]
            int ret = g15.S2F15_CheckNewEquipmentConstant();
            check_i("S2F15_Check with in-range EC value -> 0 (via real CheckECValue)", ret, 0);
        }
        {
            // Same EC, out-of-range value -> CheckECValue's real bounds check
            // returns 3, and S2F15_CheckNewEquipmentConstant propagates it
            // immediately (`if(ret!=0) return ret;`).
            HTGem g16;
            int ecRaw3 = 50;
            g16.SvEcReg.SetECDataPointer(AnsiString("200"), HType.INT_4_TYPE, "TestEC2", "unit",
                                          (void*)&ecRaw3, 0, 100, 50, "remark2");
            g16.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g16.WireCodec.SReceiveData->Add(AnsiString(1));
            g16.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g16.WireCodec.SReceiveData->Add(AnsiString(2));
            g16.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g16.WireCodec.SReceiveData->Add(AnsiString(3));
            g16.WireCodec.SReceiveData->Add(AnsiString("200"));
            g16.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
            g16.WireCodec.SReceiveData->Add(AnsiString(1));
            g16.WireCodec.SReceiveData->Add(AnsiString(999));  // > max 100
            int ret = g16.S2F15_CheckNewEquipmentConstant();
            check_i("S2F15_Check with out-of-range EC value -> 3 (via real CheckECValue)", ret, 3);
        }

        // ---------------------------------------------------------------
        // S2F16_NewEquipmentConstantSendAcknowledge (golden uHGemClass.cpp:
        // 731-781, UN-GATED 20260723).
        // ---------------------------------------------------------------
        printf("\n-- S2F16_NewEquipmentConstantSendAcknowledge (golden :731-781) --\n");
        {
            // Precondition documented, not assumed: this test target links
            // ht9045_sm (the RESCAN group expansion, see tests/CMakeLists.txt),
            // so HasICUnderMachine()/HasAnyICInMachine() here are the REAL
            // csystem_predicates.cpp bodies (HT9045_KITSUCK_GRID_AVAILABLE=1
            // since the W7 substrate wave, 20260629) evaluating the real,
            // wired TMyKitSuck grid objects -- NOT a stub. They return false
            // here simply because this test never seeds an IC into any of
            // those grid objects (empty-grid default), not because the grid
            // is architecturally unwired. S2F16's "IC still under machine"
            // early-return (ack code 2) is NOT reachable from this test as a
            // result. Asserted here so a future test that DOES seed the grid
            // gets an immediate, loud signal if this default ever flips.
            check_b("HasICUnderMachine() == false (empty KitSuck grid, real csystem.h predicate)",
                    HasICUnderMachine(), false);
            check_b("HasAnyICInMachine() == false (empty KitSuck grid, real csystem.h predicate)",
                    HasAnyICInMachine(), false);
        }
        {
            // Format-error path: empty SReceiveData -> S2F15_CheckNewEquipmentConstant
            // returns -1 (GetDataItemLenAndTypeAndDelete's own Count<2 guard)
            // -> `S9F7_IllegalData("S2,F15 data format error")` -- reachable
            // with HGemPtr==NULL (returns before the HGemPtr->MoveCheckCallBack
            // tail). Also proves the CUSTOMER_CODE!=CC_SJ_Semiconductor_OS
            // `else` branch's `HasICUnderMachine() || HasAnyICInMachine()`
            // check (both false per the precondition above) correctly falls
            // through into S2F15_Check instead of short-circuiting.
            HTGem g17;
            g17.S2F16_NewEquipmentConstantSendAcknowledge();
            check_i("S2F16 with empty SReceiveData -> falls to S9F7 (MessageID_F==7)",
                    g17.WireCodec.Local.MessageID_F, 7);
            check_i("S2F16 with empty SReceiveData -> falls to S9F7 (MessageID_S==9)",
                    g17.WireCodec.Local.MessageID_S, 9);
        }
        {
            // Same format-error path, but with CUSTOMER_CODE temporarily set
            // to CC_SJ_Semiconductor_OS -- proves the OTHER half of the
            // if/else branch selection (golden :736) also falls through to
            // S2F15_Check when HasICUnderMachine() is false (this build's
            // only reachable case -- see the precondition test above; the
            // SJSemi branch's own distinct behavior, ack code 2 when IC IS
            // present, is not exercisable until the KitSuck grid wave).
            int savedCustomerCode = CUSTOMER_CODE;
            CUSTOMER_CODE = CC_SJ_Semiconductor_OS;
            HTGem g18;
            g18.S2F16_NewEquipmentConstantSendAcknowledge();
            check_i("S2F16 (CC_SJ_Semiconductor_OS branch) with empty SReceiveData -> S9F7 (MessageID_F==7)",
                    g18.WireCodec.Local.MessageID_F, 7);
            CUSTOMER_CODE = savedCustomerCode;
        }
        {
            // S2F15_Check returns a nonzero, NON-(-1) code (1 == "ECID not
            // registered", CheckECValue's own not-exist default, golden
            // :3558-3559 / this file's own CheckECValue comment) -> S2F16's
            // `else` branch fires: `ActiveWire->LocalAcknowledge(2, 16, ret)`
            // and an early return -- BEFORE SReceiveData is ever restored
            // from the backup. Also proves SReceiveDataBackup really did
            // snapshot the burst BEFORE S2F15_Check destructively consumed
            // the original SReceiveData (Backup->Count still holds every
            // token pushed, even though SReceiveData itself is now empty).
            HTGem g19;
            g19.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g19.WireCodec.SReceiveData->Add(AnsiString(1));
            g19.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g19.WireCodec.SReceiveData->Add(AnsiString(2));
            g19.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g19.WireCodec.SReceiveData->Add(AnsiString(3));
            g19.WireCodec.SReceiveData->Add(AnsiString("999"));   // unregistered ECID
            g19.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_1_TYPE));
            g19.WireCodec.SReceiveData->Add(AnsiString(1));
            g19.WireCodec.SReceiveData->Add(AnsiString(5));
            int backupCountBefore = 10;   // exactly the 10 tokens just pushed above
            g19.S2F16_NewEquipmentConstantSendAcknowledge();
            check_i("S2F16 with unregistered ECID -> S2F15_Check returns 1 -> LocalAcknowledge(2,16,ret): MessageID_S==2",
                    g19.WireCodec.Local.MessageID_S, 2);
            check_i("S2F16 with unregistered ECID -> LocalAcknowledge(2,16,ret): MessageID_F==16",
                    g19.WireCodec.Local.MessageID_F, 16);
            check_i("S2F16 with unregistered ECID: SReceiveDataBackup snapshotted the full burst before Check consumed it",
                    g19.WireCodec.SReceiveDataBackup->Count, backupCountBefore);
            check_i("S2F16 with unregistered ECID: SReceiveData itself was fully consumed by Check, NOT restored (ret!=0 short-circuits before the restore Assign)",
                    g19.WireCodec.SReceiveData->Count, 0);
        }
        {
            // Full success round trip, proving the un-gated backup/restore
            // dance actually works end to end: S2F15_Check destructively
            // consumes SReceiveData and succeeds (ret==0) -> S2F16 restores
            // SReceiveData from SReceiveDataBackup -> S2F15_Update re-parses
            // the SAME burst and really calls SetECValue on the registered
            // EC pointer -> both LocalAcknowledge(2,16,0) AND the
            // HGemPtr->MoveCheckCallBack tail run without crashing. A real
            // THGem is used (not the default-NULL HGemPtr the other cases
            // above use) so that unconditional golden tail
            // (`HGemPtr->MoveCheckCallBack!=NULL`) dereferences a valid
            // pointer, matching golden's own assumption here (preserved
            // verbatim, NOT NULL-guarded -- see this file's own
            // S101F6()/S101F8() precedent) -- THGem's own ctor
            // (uHGemEquipment.cpp) sets MoveCheckCallBack=NULL, so the call
            // itself is safely skipped either way.
            THGem realThgem;
            HTGem g20(&realThgem);
            int ecRaw = 50;
            g20.SvEcReg.SetECDataPointer(AnsiString("200"), HType.INT_4_TYPE, "TestEC2", "unit",
                                          (void*)&ecRaw, 0, 100, 50, "remark2");
            g20.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g20.WireCodec.SReceiveData->Add(AnsiString(1));
            g20.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
            g20.WireCodec.SReceiveData->Add(AnsiString(2));
            g20.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
            g20.WireCodec.SReceiveData->Add(AnsiString(3));
            g20.WireCodec.SReceiveData->Add(AnsiString("200"));
            g20.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
            g20.WireCodec.SReceiveData->Add(AnsiString(1));
            g20.WireCodec.SReceiveData->Add(AnsiString(30));   // in [0,100]
            g20.S2F16_NewEquipmentConstantSendAcknowledge();
            check_i("S2F16 full success round trip -> LocalAcknowledge(2,16,0): MessageID_S==2",
                    g20.WireCodec.Local.MessageID_S, 2);
            check_i("S2F16 full success round trip -> LocalAcknowledge(2,16,0): MessageID_F==16",
                    g20.WireCodec.Local.MessageID_F, 16);
            check_i("S2F16 full success round trip: S2F15_Update really ran SetECValue on the restored burst (ecRaw==30)",
                    ecRaw, 30);
            check_i("S2F16 full success round trip: SReceiveData fully consumed after the restore+re-parse",
                    g20.WireCodec.SReceiveData->Count, 0);
        }
    }

    // -----------------------------------------------------------------------
    // S2F24_TraceInitializeAcknowledgeSub (golden uHGemClass.cpp:810-990,
    // UN-GATED W906-uHGemClass-TraceUnlock 20260728) -- needs a real THGem
    // (dereferences HGemPtr->bTraceData/iTRID/DSPER/iTOTSMP/iREPGSZ/
    // iTOTSMP_Count/TraceData/TraceDataResponseTask directly), same
    // "construct a real THGem" pattern as the S2F16 section above.
    // -----------------------------------------------------------------------
    printf("\n-- S2F24_TraceInitializeAcknowledgeSub (golden :810-990) --\n");
    {
        // Format-error path: empty SReceiveData -> the outer
        // DataItemIn(5,LIST_TYPE,NULL) fails immediately (BEFORE touching
        // HGemPtr) -> -1.
        THGem realThgem;
        HTGem g(&realThgem);
        check_i("S2F24Sub with empty SReceiveData -> -1 (format error, HGemPtr untouched)",
                g.S2F24_TraceInitializeAcknowledgeSub(), -1);
    }
    {
        // No free trace slot: every one of the 10 slots is already active
        // (bTraceData[i]=true) with a TRID that will NOT match the incoming
        // one -> golden's own new-vs-reuse search both fail -> iIndex stays
        // -1 -> return 2. Minimal seed: just the outer <L,5> header + a TRID
        // that matches none of the pre-filled slots (Sub() returns 2 before
        // ever reading DSPER/TOTSMP/REPGSZ/SVID-list).
        THGem realThgem;
        HTGem g(&realThgem);
        for (int i = 0; i < 10; i++)
        {
            realThgem.bTraceData[i] = true;
            realThgem.iTRID[i] = AnsiString(900 + i);
        }
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("1"));   // TRID="1", matches none of "900".."909"
        check_i("S2F24Sub with all 10 trace slots active (no match) -> 2 (no free slot)",
                g.S2F24_TraceInitializeAcknowledgeSub(), 2);
    }
    {
        // TOTSMP peek len!=1 -> return 1 (GOLDEN QUIRK -- NOT a format
        // error code; distinct from REPGSZ's own len!=1 case below).
        // Seed: <L,5> + TRID + DSPER(6 bytes, valid) + a TOTSMP header whose
        // OWN peeked len is 2 (not 1) -- Sub() returns 1 right after the
        // peek, WITHOUT ever consuming that item (peek-only).
        THGem realThgem;
        HTGem g(&realThgem);
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("1"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("010203"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));   // TOTSMP header: len=2, not 1
        check_i("S2F24Sub with TOTSMP peek len!=1 -> 1 (golden quirk, NOT a format error)",
                g.S2F24_TraceInitializeAcknowledgeSub(), 1);
    }
    {
        // REPGSZ peek len!=1 -> return -1 (structurally identical check to
        // TOTSMP's own above, but golden's OWN asymmetric return code --
        // preserved verbatim, see this method's own uHGemClass.cpp comment).
        THGem realThgem;
        HTGem g(&realThgem);
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("1"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("010203"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // TOTSMP: len=1, value=20 (consumed for real)
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(20));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));   // REPGSZ header: len=2, not 1
        check_i("S2F24Sub with REPGSZ peek len!=1 -> -1 (golden's OWN asymmetry vs TOTSMP's ==1 above)",
                g.S2F24_TraceInitializeAcknowledgeSub(), -1);
    }
    {
        // Full success round trip via the LIST_TYPE SVID-list branch: 2
        // registered SVIDs ("10","11") -> return 0. Verifies DSPER/iTOTSMP
        // (post-division)/iREPGSZ/bTraceData/iTOTSMP_Count/
        // TraceDataResponseTask/TraceData contents -- exact wire-frame-
        // derived state, not just the return code.
        THGem realThgem;
        HTGem g(&realThgem);
        realThgem.SvEcReg.SV_ID->Add(AnsiString(10));
        realThgem.SvEcReg.SV_ID->Add(AnsiString(11));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));     // outer <L,5>
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));    // TRID="1"
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("1"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));    // DSPER="010203" (hh=1,mm=2,ss=3)
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("010203"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // TOTSMP=20
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(20));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // REPGSZ=99 (overwritten below by the SVID-list len)
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(99));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));     // SVID sub-list <L,2>
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // SVID#1=10
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(10));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // SVID#2=11
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(11));
        int ret = g.S2F24_TraceInitializeAcknowledgeSub();
        check_i("S2F24Sub full success (LIST_TYPE SVID branch) -> 0", ret, 0);
        check_i("S2F24Sub success: fully consumes its seeded burst",
                g.WireCodec.SReceiveData->Count, 0);
        check_i("S2F24Sub success: DSPER[0] == hh*3600000+mm*60000+ss*1000 (1*3600000+2*60000+3*1000)",
                (int)realThgem.DSPER[0], 3723000);
        check_i("S2F24Sub success: iREPGSZ[0] == 2 (overwritten by the SVID sub-list's own <L,2>)",
                (int)realThgem.iREPGSZ[0], 2);
        check_i("S2F24Sub success: iTOTSMP[0] == 20/2 == 10 (post-division)",
                (int)realThgem.iTOTSMP[0], 10);
        check_b("S2F24Sub success: bTraceData[0] == true", realThgem.bTraceData[0], true);
        check_i("S2F24Sub success: iTOTSMP_Count[0] == 1", (int)realThgem.iTOTSMP_Count[0], 1);
        check_i("S2F24Sub success: TraceDataResponseTask[0] == 1", realThgem.TraceDataResponseTask[0], 1);
        check_i("S2F24Sub success: TraceData[0]->Count == 2 (both SVIDs valid)",
                realThgem.TraceData[0]->Count, 2);
        check_s("S2F24Sub success: TraceData[0]->Strings[0] == \"10\"",
                realThgem.TraceData[0]->GetString(0).str(), "10");
        check_s("S2F24Sub success: TraceData[0]->Strings[1] == \"11\"",
                realThgem.TraceData[0]->GetString(1).str(), "11");
    }
    {
        // Invalid SVID path: SAME shape as the success case above, but SV_ID
        // is left EMPTY (neither "10" nor "11" registered) -> IsValidSVID
        // false for both -> bSVIDError -> return 4 BEFORE the trailing
        // bTraceData/iTOTSMP_Count/TraceDataResponseTask block ever runs
        // (golden's own early `if(bSVIDError==true) return 4;`, ahead of
        // that block) -- so bTraceData[idx] stays FALSE (ctor default,
        // never flipped true) and iTOTSMP[idx] stays UN-divided (still 20).
        THGem realThgem;
        HTGem g(&realThgem);
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("1"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("010203"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(20));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(99));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(10));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(11));
        int ret = g.S2F24_TraceInitializeAcknowledgeSub();
        check_i("S2F24Sub with 2 unregistered SVIDs -> 4 (bSVIDError)", ret, 4);
        check_i("S2F24Sub invalid-SVID: TraceData[0]->Count == 0 (neither SVID added)",
                realThgem.TraceData[0]->Count, 0);
        check_b("S2F24Sub invalid-SVID: bTraceData[0] stays false (return 4 short-circuits BEFORE this assignment)",
                realThgem.bTraceData[0], false);
        check_i("S2F24Sub invalid-SVID: iTOTSMP[0] stays 20, UN-divided (same short-circuit)",
                (int)realThgem.iTOTSMP[0], 20);
    }
    {
        // UINT_4_TYPE SVID-array branch (golden's OTHER SVID-list encoding,
        // :937-964) -- same success shape as the LIST_TYPE case above, just
        // a different wire Type for the SVID sub-list itself. Also the
        // GOLDEN BUG's own exercise path (P is `new`'d and, on this SUCCESS
        // path, never `delete[]`d -- see this method's own uHGemClass.cpp
        // comment; not independently observable under this harness, no ASan
        // available, but the path is exercised end to end without crashing).
        THGem realThgem;
        HTGem g(&realThgem);
        realThgem.SvEcReg.SV_ID->Add(AnsiString(20));
        realThgem.SvEcReg.SV_ID->Add(AnsiString(21));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString("2"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("040506"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(30));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(99));
        // SVID sub-list encoded as ONE UINT_4_TYPE item of len=2 (2 values),
        // NOT a LIST_TYPE header -- golden's `else if(Type==HType.UINT_4_TYPE)`
        // branch reads BOTH SVIDs directly out of this single item's own
        // `len` (item count), no nested per-SVID Type/len tokens.
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString(20));
        g.WireCodec.SReceiveData->Add(AnsiString(21));
        int ret = g.S2F24_TraceInitializeAcknowledgeSub();
        check_i("S2F24Sub full success (UINT_4_TYPE SVID branch) -> 0", ret, 0);
        // Fresh THGem (all bTraceData[] false) -> the "find first free slot"
        // loop picks index 0 (same as every other fresh-instance case above).
        check_i("S2F24Sub UINT_4_TYPE branch: iREPGSZ[0] == 2", (int)realThgem.iREPGSZ[0], 2);
        check_i("S2F24Sub UINT_4_TYPE branch: TraceData[0]->Count == 2 (both SVIDs valid)",
                realThgem.TraceData[0]->Count, 2);
    }

    // -----------------------------------------------------------------------
    // ParseSECSDateTimeString (uHGemClass.h/.cpp, W906-uHGemClass-TraceUnlock
    // 20260728) -- pure golden SECSGEM/uHGemClass.cpp:1095-1216 decode logic,
    // zero OS side effects. Every length branch uses golden's OWN inline
    // comment example string verbatim.
    // -----------------------------------------------------------------------
    printf("\n-- ParseSECSDateTimeString (golden :1095-1216, pure extraction) --\n");
    {
        SECSDateTimeFields f;
        bool bRangeError, bApplyClock;

        // len==12: "030601134700" (golden :1111)
        check_b("len=12 \"030601134700\" -> structurally valid", ParseSECSDateTimeString("030601134700", 12, f, bRangeError, bApplyClock), true);
        check_i("len=12: year==2003 (2-digit + 2000)", f.year, 2003);
        check_i("len=12: month==6", f.month, 6);
        check_i("len=12: day==1", f.day, 1);
        check_i("len=12: hour==13", f.hour, 13);
        check_i("len=12: min==47", f.min, 47);
        check_i("len=12: sec==0", f.sec, 0);
        check_i("len=12: hundredths==0 (format has none)", f.hundredths, 0);
        check_b("len=12: bRangeError==false", bRangeError, false);
        check_b("len=12: bApplyClock==true (valid calendar date + in-range fields)", bApplyClock, true);

        // len==14: "20030602134700" (golden :1122)
        check_b("len=14 \"20030602134700\" -> structurally valid", ParseSECSDateTimeString("20030602134700", 14, f, bRangeError, bApplyClock), true);
        check_i("len=14: year==2003 (4-digit)", f.year, 2003);
        check_i("len=14: day==2", f.day, 2);
        check_b("len=14: bApplyClock==true", bApplyClock, true);

        // len==16: "2003060313401000" (golden :1133)
        check_b("len=16 \"2003060313401000\" -> structurally valid", ParseSECSDateTimeString("2003060313401000", 16, f, bRangeError, bApplyClock), true);
        check_i("len=16: min==40", f.min, 40);
        check_i("len=16: sec==10", f.sec, 10);
        check_i("len=16: hundredths==0", f.hundredths, 0);
        check_b("len=16: bApplyClock==true", bApplyClock, true);

        // len==19: "2003-06-04T13:01:01" (golden :1144)
        check_b("len=19 \"2003-06-04T13:01:01\" -> structurally valid", ParseSECSDateTimeString("2003-06-04T13:01:01", 19, f, bRangeError, bApplyClock), true);
        check_i("len=19: day==4", f.day, 4);
        check_i("len=19: min==1", f.min, 1);
        check_i("len=19: sec==1", f.sec, 1);
        check_i("len=19: hundredths==0 (explicit t.ti_hund=0 in golden)", f.hundredths, 0);
        check_b("len=19: bApplyClock==true", bApplyClock, true);

        // len==21: "2003-06-05T13:01:01.2" (golden :1155) -- 1-digit tenths*10
        check_b("len=21 \"2003-06-05T13:01:01.2\" -> structurally valid", ParseSECSDateTimeString("2003-06-05T13:01:01.2", 21, f, bRangeError, bApplyClock), true);
        check_i("len=21: day==5", f.day, 5);
        check_i("len=21: hundredths==20 (1-digit tenths '2'*10)", f.hundredths, 20);
        check_b("len=21: bApplyClock==true", bApplyClock, true);

        // len==22: "2003-06-06T13:01:01.25" (golden :1166) -- 2-digit hundredths
        check_b("len=22 \"2003-06-06T13:01:01.25\" -> structurally valid", ParseSECSDateTimeString("2003-06-06T13:01:01.25", 22, f, bRangeError, bApplyClock), true);
        check_i("len=22: day==6", f.day, 6);
        check_i("len=22: hundredths==25 (2-digit '25')", f.hundredths, 25);
        check_b("len=22: bApplyClock==true", bApplyClock, true);

        // Structural failure: len matches none of the 6 -> false, out untouched.
        check_b("len=10 (unsupported length) -> false (golden's own Error=true else-branch)",
                ParseSECSDateTimeString("0306011347", 10, f, bRangeError, bApplyClock), false);

        // Range-check failure: hour=24 (>23) -- calendar date itself IS valid
        // (2003-06-01), so IsCorrectDateFormat passes, but the hour range
        // check flips bRangeError true -> bApplyClock stays false.
        check_b("len=12 hour=24 (out of range) -> still structurally valid",
                ParseSECSDateTimeString("030601244700", 12, f, bRangeError, bApplyClock), true);
        check_b("len=12 hour=24: bRangeError==true", bRangeError, true);
        check_b("len=12 hour=24: bApplyClock==false (no clock write)", bApplyClock, false);

        // GOLDEN QUIRK: calendar-invalid date (month=13) -- IsCorrectDateFormat
        // fails, so golden's own Error stays at its PRIOR value (false) --
        // bRangeError==false (NOT true) even though the date is nonsense, and
        // bApplyClock==false (no clock write happens either). See
        // ParseSECSDateTimeString's own header comment (uHGemClass.h) for the
        // full writeup of why this is NOT "fixed" into bRangeError==true here.
        check_b("len=12 month=13 (calendar-invalid) -> still structurally valid",
                ParseSECSDateTimeString("031301010000", 12, f, bRangeError, bApplyClock), true);
        check_b("len=12 month=13: bRangeError==false (GOLDEN QUIRK -- IsCorrectDateFormat failure is NOT surfaced as Error)",
                bRangeError, false);
        check_b("len=12 month=13: bApplyClock==false (no clock write, despite bRangeError==false)",
                bApplyClock, false);
    }

    // -----------------------------------------------------------------------
    // S2F32_DateAndTimeAcknowledge (golden uHGemClass.cpp:1095-1216, UN-GATED
    // W906-uHGemClass-TraceUnlock 20260728) -- wire-level integration on top
    // of ParseSECSDateTimeString + SetSystemDateTimeHook. Every case resets
    // SetSystemDateTimeHook back to an empty std::function afterward (same
    // save/restore-global discipline this file already uses for
    // CUSTOMER_CODE) -- CRITICAL here: this hook must NEVER leak a wired
    // lambda into a LATER, unrelated test in this same process.
    // -----------------------------------------------------------------------
    printf("\n-- S2F32_DateAndTimeAcknowledge (golden :1095-1216) --\n");
    {
        // Format-error path: peeked Type is NOT ASCII_TYPE -> Error=true ->
        // LocalAcknowledge(2,32,1). Hook must NOT fire.
        HTGem g;
        bool hookFired = false;
        SetSystemDateTimeHook = [&hookFired](int,int,int,int,int,int,int){ hookFired = true; };
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_2_TYPE));   // NOT ASCII_TYPE
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.S2F32_DateAndTimeAcknowledge();
        check_i("S2F32 with non-ASCII peeked item -> LocalAcknowledge(2,32,1): MessageID_S==2",
                g.WireCodec.Local.MessageID_S, 2);
        check_i("S2F32 with non-ASCII peeked item -> LocalAcknowledge(2,32,1): MessageID_F==32",
                g.WireCodec.Local.MessageID_F, 32);
        check_b("S2F32 format-error path: hook NOT fired", hookFired, false);
        SetSystemDateTimeHook = std::function<void(int,int,int,int,int,int,int)>();   // reset -- MUST NOT leak into later tests
    }
    {
        // Success path: well-formed ASCII date/time string (len=12,
        // "030601134700") -> Error=false -> LocalAcknowledge(2,32,0), AND the
        // hook fires with the EXACT decoded fields (proves the wire-to-parse-
        // to-hook plumbing end to end, not just the ack code).
        HTGem g;
        int gotYear=0, gotMonth=0, gotDay=0, gotHour=0, gotMin=0, gotSec=0, gotHundredths=-1;
        bool hookFired = false;
        SetSystemDateTimeHook = [&](int y,int mo,int d,int h,int mi,int s,int hu){
            hookFired=true; gotYear=y; gotMonth=mo; gotDay=d; gotHour=h; gotMin=mi; gotSec=s; gotHundredths=hu;
        };
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(12));
        g.WireCodec.SReceiveData->Add(AnsiString("030601134700"));
        g.S2F32_DateAndTimeAcknowledge();
        check_i("S2F32 success path -> LocalAcknowledge(2,32,0): MessageID_S==2",
                g.WireCodec.Local.MessageID_S, 2);
        check_i("S2F32 success path -> LocalAcknowledge(2,32,0): MessageID_F==32",
                g.WireCodec.Local.MessageID_F, 32);
        check_b("S2F32 success path: hook DID fire", hookFired, true);
        check_i("S2F32 success path: hook received year==2003", gotYear, 2003);
        check_i("S2F32 success path: hook received month==6", gotMonth, 6);
        check_i("S2F32 success path: hook received day==1", gotDay, 1);
        check_i("S2F32 success path: hook received hour==13", gotHour, 13);
        check_i("S2F32 success path: hook received min==47", gotMin, 47);
        check_i("S2F32 success path: hook received sec==0", gotSec, 0);
        check_i("S2F32 success path: hook received hundredths==0", gotHundredths, 0);
        SetSystemDateTimeHook = std::function<void(int,int,int,int,int,int,int)>();   // reset -- MUST NOT leak into later tests
    }
    {
        // GOLDEN QUIRK end to end: calendar-invalid date (month=13) still
        // ACKs 0 (accepted) -- but the hook must NOT fire (no real clock
        // write happens for this input, per ParseSECSDateTimeString's own
        // documented quirk).
        HTGem g;
        bool hookFired = false;
        SetSystemDateTimeHook = [&hookFired](int,int,int,int,int,int,int){ hookFired = true; };
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(12));
        g.WireCodec.SReceiveData->Add(AnsiString("031301010000"));   // month=13
        g.S2F32_DateAndTimeAcknowledge();
        check_i("S2F32 GOLDEN QUIRK (month=13): still LocalAcknowledge(2,32,0) (MessageID_F==32)",
                g.WireCodec.Local.MessageID_F, 32);
        // The ack byte itself is the LAST byte DataItemOut(1,BINARY_TYPE,&C)
        // wrote verbatim into LocalBuffer (see SecsWireCodec.cpp's own
        // BINARY_TYPE branch -- raw bytes, no further encoding for len=1).
        check_i("S2F32 GOLDEN QUIRK (month=13): still ACK code 0 (last LocalBuffer byte)",
                (int)g.WireCodec.LocalBuffer[g.WireCodec.LocalLength_4 - 1], 0);
        check_b("S2F32 GOLDEN QUIRK (month=13): hook NOT fired despite ACK==0 (no real clock write)",
                hookFired, false);
        SetSystemDateTimeHook = std::function<void(int,int,int,int,int,int,int)>();   // reset -- MUST NOT leak into later tests
    }
    {
        // Range-check failure end to end: hour=24 -> Error=true ->
        // LocalAcknowledge(2,32,1), hook NOT fired.
        HTGem g;
        bool hookFired = false;
        SetSystemDateTimeHook = [&hookFired](int,int,int,int,int,int,int){ hookFired = true; };
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(12));
        g.WireCodec.SReceiveData->Add(AnsiString("030601244700"));   // hour=24
        g.S2F32_DateAndTimeAcknowledge();
        check_i("S2F32 range-check failure (hour=24): LocalAcknowledge(2,32,1) -> last LocalBuffer byte==1",
                (int)g.WireCodec.LocalBuffer[g.WireCodec.LocalLength_4 - 1], 1);
        check_b("S2F32 range-check failure (hour=24): hook NOT fired", hookFired, false);
        SetSystemDateTimeHook = std::function<void(int,int,int,int,int,int,int)>();   // reset -- MUST NOT leak into later tests
    }

    // -----------------------------------------------------------------------
    // SECS_EVENT (ETypeStruct) -- every enumerator's exact golden numeric
    // value (golden HT9011UC_Code_.../SECSGEM/uHGemHT9045.h:12-313).
    // -----------------------------------------------------------------------
    printf("\n-- SECS_EVENT (ETypeStruct) golden numeric values --\n");
    check_i("SECS_EVENT.DoStart == 1",              SECS_EVENT.DoStart, 1);
    check_i("SECS_EVENT.DoPause == 2",               SECS_EVENT.DoPause, 2);
    check_i("SECS_EVENT.DoOneCycle == 3",            SECS_EVENT.DoOneCycle, 3);
    check_i("SECS_EVENT.DoCleanOut == 4",            SECS_EVENT.DoCleanOut, 4);
    check_i("SECS_EVENT.DoExit == 24",               SECS_EVENT.DoExit, 24);
    check_i("SECS_EVENT.OneCycleFinish == 41",       SECS_EVENT.OneCycleFinish, 41);
    check_i("SECS_EVENT.CleanOutFinish == 42",       SECS_EVENT.CleanOutFinish, 42);
    check_i("SECS_EVENT.ArtRTFinish == 61",          SECS_EVENT.ArtRTFinish, 61);
    check_i("SECS_EVENT.ArtFTFinish == 63",          SECS_EVENT.ArtFTFinish, 63);
    check_i("SECS_EVENT.SECSOffline == 91",          SECS_EVENT.SECSOffline, 91);
    check_i("SECS_EVENT.SECSOnline == 92",           SECS_EVENT.SECSOnline, 92);
    check_i("SECS_EVENT.SECSOnlineRemote == 93",     SECS_EVENT.SECSOnlineRemote, 93);
    check_i("SECS_EVENT.LoaderTrayState == 271",     SECS_EVENT.LoaderTrayState, 271);
    check_i("SECS_EVENT.AGVSupplement == 272",       SECS_EVENT.AGVSupplement, 272);
    check_i("SECS_EVENT.AGVLdID == 291",             SECS_EVENT.AGVLdID, 291);
    check_i("SECS_EVENT.DoSecsGemIndexFail == 292 (explicit re-anchor)",
            SECS_EVENT.DoSecsGemIndexFail, 292);
    check_i("SECS_EVENT.TotalEvent == 293 (one past the last -- 'must stay at the bottom')",
            SECS_EVENT.TotalEvent, 293);

    // -----------------------------------------------------------------------
    // Summary
    // -----------------------------------------------------------------------
    printf("\n=== SUMMARY: %d passed, %d failed (total %d) ===\n",
           g_pass, g_fail, g_pass + g_fail);
    if (g_fail != 0)
    {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: ALL PASS\n");
    return 0;
}
