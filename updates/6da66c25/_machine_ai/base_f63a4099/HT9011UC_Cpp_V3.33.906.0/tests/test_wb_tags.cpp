// =============================================================================
//  test_wb_tags.cpp -- WebBridgeTags: does the browser get the truth?
//
//  AI(W906-WebBridge) 20260806.
//
//  The single property under test is the one the whole design exists for:
//
//      a tag whose source is loaded carries a REAL value;
//      a tag whose source is NOT loaded carries NULL, never 0.
//
//  The browser renders null as "---" and 0 as "0.00", so getting this backwards
//  puts a measurement on an operator's screen that was never taken. On a
//  machine that runs at 130 C that is not a cosmetic bug.
//
//  DO-NOT-MODIFY-REAL-CONFIG: LoadMachineConfig() seeds missing keys, i.e. it
//  WRITES to asGeneralPath. As in tests/test_wb_datalayer.cpp, this repoints
//  asGeneralPath at a scratch copy first and restores it afterwards.
// =============================================================================
#include "WebBridgeTags.h"

#include "database.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"

#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "WebBridge/TagJson.h"   // AI(W906-Q34-DOC) 20260924: EncodeTagObject —— 量快照的線上位元組（佔用原本的空行，不移動行號）
#include "wb_buildfact_tags.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)

static int g_total = 0;
static int g_fail = 0;

static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);

    using webbridge::TagSnapshot;
    using webbridge::TagSnapshotView;
    using webbridge::TagValue;

    // --- 1. before loading, EVERYTHING must be null --------------------------
    // This is the control. If a tag carried a value here, the "live" checks
    // below would prove nothing -- they could be reading a leftover.
    {
        TagSnapshot snap;
        ht9045::PublishHandlerTags(snap);
        const TagSnapshotView v = snap.read();

        //AI(W906-BU-C3) 20260916: this was `nonNull == 0` until BU-C3 merged the
        // colleague's build.* family and the 1203 module-level tags.  Those 14
        // are non-null pre-load ON PURPOSE -- they are compile-time and
        // module-level facts, and WebBridgeTags.cpp:620 / :1079 say so in the
        // source.  The property under test is unchanged; it is now stated
        // precisely instead of approximately, and the exemption is pinned in
        // BOTH directions.  See tests/wb_buildfact_tags.h.
        std::size_t nonNull = 0;
        std::size_t factsSeen = 0;
        std::string leaked;
        for (webbridge::TagMap::const_iterator it = v.tags.begin();
             it != v.tags.end(); ++it) {
            if (it->second.isNull()) continue;
            ++nonNull;
            if (wbtest::IsBuildFactTag(it->first.c_str())) { ++factsSeen; continue; }
            if (!leaked.empty()) leaked += ", ";
            leaked += it->first;
        }
        std::printf("-- 1. before LoadMachineConfig: %u tags, %u non-null "
                    "(%u exempt build/module facts, %u leaked)\n",
                    (unsigned)v.tags.size(), (unsigned)nonNull,
                    (unsigned)factsSeen, (unsigned)(nonNull - factsSeen));
        if (!leaked.empty())
            std::printf("   LEAKED: %s\n", leaked.c_str());

        //AI(W906-BU-C3) 20260916: a family census, printed not asserted.  BU-C3
        // was described as "63 pci1203 tags", but that counted STRING LITERALS
        // in the source -- the family is emitted by four nested loops bounded at
        // kPci1203TagAxes=32, kPci1203TagDiPorts=128, kPci1203TagDoPorts=96 and
        // kPci1203TagSlaves=48, so the RUNTIME count is thousands.  Every one of
        // them is null in a binary built without INSTALL_1203_MONITOR, which is
        // every target here.  FrameSnapshot() encodes nulls too
        // (WebBridge/TagJson.cpp EncodeTagObject), so this number is the wire
        // cost of a snapshot.  Printed rather than pinned: a number asserted here
        // would just be another hard-coded count going stale, which is the exact
        // failure WebBridgeTags.cpp:1922 and WebBridgeTags.h:342 are guilty of.
        {
            std::size_t nPci = 0, nBuild = 0;
            for (webbridge::TagMap::const_iterator it = v.tags.begin();
                 it != v.tags.end(); ++it) {
                if (it->first.compare(0, 8, "pci1203.") == 0) ++nPci;
                else if (it->first.compare(0, 6, "build.") == 0) ++nBuild;
            }
            std::printf("   family census: %u total = %u pci1203.* + %u build.* "
                        "+ %u rest; snapshot wire bytes = %u\n",   // AI(W906-Q34-DOC) 20260924: EncodeTagObject（FrameSnapshot 用的同一支）的長度，含 null
                        (unsigned)v.tags.size(), (unsigned)nPci, (unsigned)nBuild,
                        (unsigned)(v.tags.size() - nPci - nBuild), (unsigned)webbridge::EncodeTagObject(v.tags).size());
        }

        //AI(W906-SJSON-S9b) 20260923: THE 512 PER-PORT VALUE TAGS ARE GONE,
        //  pinned here so a future edit cannot quietly bring them back.
        //
        //  They were replaced by the packed pair io.di/io.di.valid +
        //  io.do/io.do.valid (JsonBridge/ChanIo.cpp), with the browser
        //  unpacking them back into these same key names at its data entry
        //  point (web/js/pci1203.js unpackIoPlanes).
        //
        //  ⚠ WHY PIN THE ABSENCE and not just the presence of io.*: for a
        //    while BOTH families were on the wire at once, which cost 870
        //    bytes MORE than the old scheme and looked like a working
        //    reduction from either side alone. An assertion on io.* only
        //    would have passed throughout that period.
        //
        //  ⚠ THE ATTRIBUTE TAGS MUST SURVIVE. .ring/.addr/.flat/.station/.chan
        //    are the wiring diagram, not the measurement; they settle at
        //    startup and never patch again. Deleting them would strip the page
        //    of its per-port attribution, so one of them is pinned PRESENT
        //    right next to the value tag pinned ABSENT -- the pair states the
        //    boundary that this change deliberately did not cross.
        {
            const bool diValGone = (v.tags.find("pci1203.di0")  == v.tags.end());
            const bool doValGone = (v.tags.find("pci1203.do0")  == v.tags.end());
            const bool diAttrKept =
                (v.tags.find("pci1203.di0.station") != v.tags.end());
            const bool doAttrKept =
                (v.tags.find("pci1203.do0.station") != v.tags.end());
            check(diValGone && doValGone,
                  "the 512 pci1203.di<N>/do<N> VALUE tags are no longer staged "
                  "(they are io.di/io.do now)");
            check(diAttrKept && doAttrKept,
                  "the per-port ATTRIBUTE tags (.station et al) are still staged "
                  "-- the packing deliberately did not touch the wiring diagram");
        }

        //AI(W906-MT-E3a) 20260925: the batch-DI mode and the actual-torque tags
        //  are STAGED (present) and NULL without a monitor -- a page binds them on
        //  the first snapshot, and "no monitor" must never read as 0 % torque or
        //  as a DI mode.
        {
            const char* mt[] = { "pci1203.di.mode", "pci1203.di.batchWhy", "pci1203.di.batchMismatches",
                                 "pci1203.torque.focusAxis", "pci1203.torque.sdoMs",
                                 "pci1203.ax0.torque", "pci1203.ax0.torquePct", "pci1203.ax0.torqueNm",
                                 "pci1203.ax0.torqueSrc", "pci1203.ax0.torqueUnitVerified",
                                 "pci1203.ax31.torquePct", "pci1203.ax31.torqueErr" };
            bool allNull = true;
            for (std::size_t q = 0; q < sizeof(mt) / sizeof(mt[0]); ++q) {
                webbridge::TagMap::const_iterator it = v.tags.find(mt[q]);
                if (it == v.tags.end() || !it->second.isNull()) { allNull = false; std::printf("   not present-and-null: %s\n", mt[q]); }
            }
            check(allNull, "MT-E3a: pci1203.di.mode / di.batch* / torque.* / axN.torque* are staged and null with no monitor");
        }

        //AI(W906-ONSITE-1) 20260926: the drive's torque-limit read-back (60E0h / 60E1h) -- the
        //  same pin as MT-E3a above: STAGED under these exact names on the first and last slot,
        //  and NULL without a monitor (a limit of 0 means "no torque", so "no monitor" must
        //  never read as 0; the Read word and the Ret code must not read as "ok" / SUCCESS).
        {
            const char* tl[] = { "pci1203.ax0.trqLim.pos", "pci1203.ax0.trqLim.posRead", "pci1203.ax0.trqLim.posRet",
                                 "pci1203.ax0.trqLim.neg", "pci1203.ax0.trqLim.negRead", "pci1203.ax0.trqLim.negRet",
                                 "pci1203.ax31.trqLim.pos", "pci1203.ax31.trqLim.negRet" };
            bool allNull = true;
            for (std::size_t q = 0; q < sizeof(tl) / sizeof(tl[0]); ++q) {
                webbridge::TagMap::const_iterator it = v.tags.find(tl[q]);
                if (it == v.tags.end() || !it->second.isNull()) { allNull = false; std::printf("   not present-and-null: %s\n", tl[q]); }
            }
            check(allNull, "ONSITE-1: pci1203.axN.trqLim.pos/neg + Read + Ret are staged and null with no monitor");
        }

        check(v.tags.size() > 0, "PublishHandlerTags stages a non-empty snapshot");
        check(leaked.empty(),
              "every tag whose source is the data layer is null before it loads "
              "(nothing is inventing values)");
        check(factsSeen == wbtest::BuildFactTagCount(),
              "every exempt build/module fact really IS published pre-load "
              "(the list in wb_buildfact_tags.h has not gone stale)");

        const ht9045::TagCoverage c = ht9045::HandlerTagCoverage();
        std::printf("   coverage: %u live / %u total\n",
                    (unsigned)c.live, (unsigned)c.total);
        check(c.live == 0, "coverage reports 0 live before loading");
    }

    // --- 2. load the data layer, on a scratch copy ---------------------------
    const AnsiString savedGeneralPath = asGeneralPath;

    char tmp[MAX_PATH];
    if (::GetTempPathA(MAX_PATH, tmp) == 0) {
        std::printf("SKIP: no temp path\n");
        std::printf("\ntest_wb_tags: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }
    const AnsiString scratch = AnsiString(tmp) + AnsiString(W906_TestTmpName("wb_tags_general.ini").c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)

    if (!::CopyFileA(savedGeneralPath.c_str(), scratch.c_str(), FALSE)) {
        std::printf("SKIP: no Gerneral.ini on this box (err %lu)\n",
                    (unsigned long)::GetLastError());
        std::printf("\ntest_wb_tags: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }

    asGeneralPath = scratch;
    const bool loaded = LoadMachineConfig();
    check(loaded, "LoadMachineConfig() succeeded");

    //AI(W906-SJSON-S9b) 20260923: ⚠ LoadMachineConfig() RETURNING TRUE DOES NOT
    //  MEAN THE IDENTITY FIELDS LOADED, and this test used to fail on that gap
    //  with three assertions that looked like product bugs.
    //
    //  SYSTEM_MODULAR::ReadGeneralIni() (database.cpp:313) opens a DIFFERENT,
    //  SECOND ini before it reads anything from asGeneralPath:
    //      D:\GPIB9045\system\general.ini  ->  [Version] Model
    //  and if that model is not one of the seven handler models it recognises
    //  (9045GPIB / 9046GPIB / 9046_32GPIB / 9045GPIB_12Site / 502GPIB /
    //  1032GPIB / 7080GPIB) it sets bHandlerModel=false and **returns early**
    //  -- before CUSTOMER_CODE is read and before ReadLastSetIni() runs.
    //  LoadMachineConfig() still returns true, because all it checks is that
    //  the TIniFile was constructed (database.cpp:3075).
    //
    //  The list is a translation of golden V912 database.cpp:308-316, which
    //  carries the identical seven entries and the identical `return`.
    //
    //  ⚠⚠ WHAT THIS SKIP DOES **NOT** CLAIM.
    //    It does NOT claim the machine is misconfigured, and it does NOT claim
    //    the model list is correct. `Model` is a HARDWARE setting, and this
    //    program supports several machine types -- a model this list does not
    //    name may be a perfectly real, supported machine that the list has not
    //    caught up with. Deciding that is a product question for the people who
    //    own the model list, not something a unit test may assume either way.
    //    (An earlier version of this comment asserted "this is the environment,
    //    not a defect". That was a guess about the setup and is withdrawn.)
    //
    //  ⇒ All this test can honestly say is NARROWER: the data layer did not
    //    load, so the assertions below have no subject. Report that and stop,
    //    rather than reporting a product defect this test cannot diagnose.
    //  ⚠ SKIP ONLY ON THAT EXACT CONDITION. If bHandlerModel is true and the
    //    identity tags are still null, that IS a real defect and must still
    //    fail -- hence the check is bHandlerModel, not "are the tags null".
    if (!bHandlerModel) {
        std::printf("\n");
        std::printf("SKIP: [Version] Model in D:\\GPIB9045\\system\\general.ini is not\n");
        std::printf("      one of the seven models ReadGeneralIni() matches on, so it\n");
        std::printf("      returned early (database.cpp:328-334): CUSTOMER_CODE was\n");
        std::printf("      never read and ReadLastSetIni() never ran. The identity and\n");
        std::printf("      coverage checks below need that path, so they have no\n");
        std::printf("      subject here and are skipped.\n");
        std::printf("      NOTE: this says nothing about whether the model list is\n");
        std::printf("      complete -- that is a product question, not a test result.\n");
        std::printf("\ntest_wb_tags: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }

    // --- 3. after loading ----------------------------------------------------
    {
        TagSnapshot snap;
        const std::size_t staged = ht9045::PublishHandlerTags(snap);
        const TagSnapshotView v = snap.read();

        std::printf("\n-- 3. after LoadMachineConfig: %u tags staged, snapshot wire bytes = %u\n",
                    (unsigned)staged, (unsigned)webbridge::EncodeTagObject(v.tags).size());   // AI(W906-Q34-DOC) 20260924: 載入後的快照位元組

        const TagValue& type = v.tags.find("machine.id.type")->second;
        const TagValue& cust = v.tags.find("machine.customerCode")->second;
        std::printf("   machine.id.type      = %s\n", type.debugString().c_str());
        std::printf("   machine.customerCode = %s\n", cust.debugString().c_str());

        check(type.isString() && type.asString().size() > 0,
              "machine.id.type carries a REAL value once IniConfig strings load");
        check(cust.isInt() && cust.asInt() != 0,
              "machine.customerCode carries a REAL value");
        // AI(W906-HT9050-ID) 20260924: Steven c56af0b 定的機種身分三個 tag（JSON/Machine-type-index.json）。
        {
            auto itC = v.tags.find("machine.typeChoice");
            auto itN = v.tags.find("machine.typeName");
            auto itG = v.tags.find("machine.gpibModel");
            const bool have = itC != v.tags.end() && itN != v.tags.end() && itG != v.tags.end();
            check(have, "machine.typeChoice / typeName / gpibModel are all staged");
            if (have) {
                std::printf("   machine.typeChoice   = %s\n   machine.typeName     = %s\n   machine.gpibModel    = %s\n",
                            itC->second.debugString().c_str(), itN->second.debugString().c_str(), itG->second.debugString().c_str());
                check(itC->second.isInt() && itC->second.asInt() == MachineTypeChoice,
                      "machine.typeChoice == MachineTypeChoice");
                check(itN->second.isString() && !itN->second.asString().empty() &&
                      itN->second.asString() == W906_MachineTypeName(MachineTypeChoice),
                      "machine.typeName is the eMachineType name of MachineTypeChoice");
                check(itG->second.isString() && !itG->second.asString().empty() &&
                      itG->second.asString() == std::string(W906_GpibModel.c_str()),
                      "machine.gpibModel carries the Model read at database.cpp:318");
            }
        }


        // The load-bearing negative. UN150Read[] stays measurably unreachable
        // offline (WebBridgeTags.h AI(W906-FW-TEMP1)/(FW-TEMP2) blocks), so
        // temp.pv and all 8 zone.* tags MUST be null here -- LoadMachineConfig()
        // alone does not touch them (that needs g_webTempLoaded/SetWebTempLoaded,
        // exercised separately in section 6 below). If this ever reports 0
        // instead, the browser will draw "0.00" for a heater zone nobody read.
        //AI(W906-FW-TEMP2) 20260820: temp.sv/temp.soak/temp.mode REMOVED from
        // this array -- they are no longer a dead-source claim, they are a
        // gated-but-not-yet-gated-open one in THIS test flow (g_webTempLoaded
        // defaults false here since nothing in this section calls
        // SetWebTempLoaded). Testing them here would conflate "genuinely
        // unreachable source" with "reachable source, not loaded in this
        // flow" -- section 6 below is the correct, separate test for them.
        const char* mustBeNull[] = {
            "temp.pv",
            "zone.hotplate.1", "zone.hotplate.2",
            "zone.shuttle.1",  "zone.shuttle.2",
            "zone.index.1",    "zone.index.2",
            "zone.heatgun.1",  "zone.heatgun.2",
            "tower.red", "tester.name", "status.uph"
        };
        bool allNull = true;
        for (std::size_t i = 0; i < sizeof(mustBeNull) / sizeof(mustBeNull[0]); ++i) {
            webbridge::TagMap::const_iterator it = v.tags.find(mustBeNull[i]);
            if (it == v.tags.end() || !it->second.isNull()) {
                std::printf("   NOT NULL: %s = %s\n", mustBeNull[i],
                            it == v.tags.end() ? "(absent)"
                                               : it->second.debugString().c_str());
                allNull = false;
            }
        }
        check(allNull,
              "every tag with an unloaded source is NULL, not 0 "
              "(null renders \"---\", 0 renders \"0.00\")");

        const ht9045::TagCoverage c = ht9045::HandlerTagCoverage();
        std::printf("   coverage: %u live / %u total\n",
                    (unsigned)c.live, (unsigned)c.total);
        check(c.live > 0 && c.live < c.total,
              "coverage is partial and honest about it");
    }

    // --- 4. FW-1a: the LastSet-blob tags decode and map correctly ------------
    //AI(W906-FW1) 20260817: direct-write oracle. The test seeds exact fields
    // and asserts the published tag, pinning three properties: (a) the decode
    // goes through golden's own StartModeName array (cmydef.cpp:62), (b) the
    // [arm][row][col] -> site.arm{a}.s{n} mapping measured from ReadTestMode
    // ([0]=Arm1 "Dut <name>" / [1]=Arm2 "Dut <name>2", cprod.cpp:3689/:3773;
    // sites 9..16 = row 1), and (c) out-of-range codes publish null. Tests may
    // write machine globals; production code may not -- that asymmetry is the
    // same one the idle-pump change established.
    {
        LastSet.iRunStartMode = rsmContinuStart_ART;             // code 9
        LastSet.bUseTestSocket[1][1][2] = true;                  // arm2 row1 col2 -> s11
        LastSet.bUseTestSocket[0][0][0] = false;                 // arm1 s1 -> real 0

        TagSnapshot snap;
        ht9045::PublishHandlerTags(snap);
        const TagSnapshotView v = snap.read();

        const TagValue& sm  = v.tags.find("startmode.value")->second;
        const TagValue& s11 = v.tags.find("site.arm2.s11")->second;
        const TagValue& s1  = v.tags.find("site.arm1.s1")->second;
        std::printf("\n-- 4. FW-1a oracle\n");
        std::printf("   startmode.value = %s\n", sm.debugString().c_str());
        check(sm.isString() && sm.asString() == "ContinuStart_ART",
              "startmode.value decodes code 9 through StartModeName (cmydef.cpp:66)");
        check(s11.isInt() && s11.asInt() == 1,
              "site.arm2.s11 reads bUseTestSocket[1][1][2] -- arm dim per cprod.cpp:3773");
        check(s1.isInt() && s1.asInt() == 0,
              "site.arm1.s1 is a REAL 0 (site off) while the blob is live, not null");

        LastSet.iRunStartMode = -1;                              // rsmNull
        TagSnapshot snap2;
        ht9045::PublishHandlerTags(snap2);
        check(snap2.read().tags.find("startmode.value")->second.isNull(),
              "an out-of-range start-mode code publishes null, never a guessed word");
    }

    // --- 5. FW-1b: sort counters pin BOTH index spaces ------------------------
    //AI(W906-FW1b) 20260817: the gate reads Prod.iTrayType[e6TrayName]
    // (eFix2 == 7) while the value reads LastSet.BinCT[0][e3TrayName]
    // (e3Fix2 == 4) -- golden maps between them via iTo3Unload (main.cpp:
    // 1954-1966), whose PORT global is uninitialized all-zero, which is why
    // the wiring uses constants. This oracle fails if anyone "simplifies"
    // the two spaces into one.
    {
        Prod.iTrayType[eFix2]  = tTrayFix;                       // gate idx 7 -> configured
        Prod.iTrayType[eAuto2] = tNotUse;                        // gate idx 1 -> not configured
        LastSet.BinCT[0][e3Fix2] = 77;                           // column idx 4

        TagSnapshot snap;
        ht9045::PublishHandlerTags(snap);
        const TagSnapshotView v = snap.read();
        const TagValue& f2 = v.tags.find("sort.fix2.count")->second;
        const TagValue& a2 = v.tags.find("sort.auto2.count")->second;
        std::printf("\n-- 5. FW-1b oracle: sort.fix2.count = %s\n",
                    f2.debugString().c_str());
        check(f2.isInt() && f2.asInt() == 77,
              "sort.fix2.count reads BinCT[0][e3Fix2==4] gated by iTrayType[eFix2==7] (cSortCT.cpp:396/:399)");
        check(a2.isNull(),
              "an unconfigured station (iTrayType==tNotUse) publishes null even with a live blob");
    }

    // --- 6. FW-TEMP2: temp.sv/soak/mode gate on g_webTempLoaded --------------
    //AI(W906-FW-TEMP2) 20260820: direct-write oracle, same shape as sections 4
    // and 5 -- tests may write machine globals directly; production code may
    // not. Pins three things: (a) temp.sv/temp.soak carry the REAL double
    // values, not truncated ints; (b) temp.mode carries the RAW ini code
    // (not a decoded label -- this file never had one to decode, see
    // WebBridgeTags.h); (c) the gate is a real gate -- flipping
    // g_webTempLoaded back to false must return all three to null even
    // though the underlying globals still hold the values, proving liveness
    // is keyed on the flag wb_serve sets, not on the fields being nonzero.
    {
        Temperature.fWorkTemperBase = 82.5;
        Temperature.fSoakTime       = 12.0;
        Temperature.iMachineTempMode = 1;             // Ambient, ReadTempFile :2248-2264
        ht9045::SetWebTempLoaded(true);

        TagSnapshot snap;
        ht9045::PublishHandlerTags(snap);
        const TagSnapshotView v = snap.read();

        const TagValue& sv   = v.tags.find("temp.sv")->second;
        const TagValue& soak = v.tags.find("temp.soak")->second;
        const TagValue& mode = v.tags.find("temp.mode")->second;
        std::printf("\n-- 6. FW-TEMP2 oracle: temp.sv=%s temp.soak=%s temp.mode=%s\n",
                    sv.debugString().c_str(), soak.debugString().c_str(),
                    mode.debugString().c_str());
        check(sv.isDouble() && sv.asDouble() == 82.5,
              "temp.sv carries the REAL double (Temperature.fWorkTemperBase), not truncated to int");
        check(soak.isDouble() && soak.asDouble() == 12.0,
              "temp.soak carries the REAL double (Temperature.fSoakTime)");
        check(mode.isInt() && mode.asInt() == 1,
              "temp.mode carries the RAW ini code (iMachineTempMode), not a guessed label");

        ht9045::SetWebTempLoaded(false);
        TagSnapshot snap2;
        ht9045::PublishHandlerTags(snap2);
        const TagSnapshotView v2 = snap2.read();
        check(v2.tags.find("temp.sv")->second.isNull() &&
              v2.tags.find("temp.soak")->second.isNull() &&
              v2.tags.find("temp.mode")->second.isNull(),
              "temp.sv/soak/mode go back to null when g_webTempLoaded is false, "
              "even though the underlying globals still hold real values -- "
              "the GATE is what publishes, not the field's own nonzero-ness");

        // hygiene: don't leave a nonzero Temperature struct behind for any
        // later coverage/canary check in this same process to trip over.
        Temperature.fWorkTemperBase = 0.0;
        Temperature.fSoakTime       = 0.0;
        Temperature.iMachineTempMode = 0;
    }

    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;
    ::DeleteFileA(scratch.c_str());

    std::printf("\ntest_wb_tags: %d checks, %d failure(s)\n", g_total, g_fail);
    std::printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
