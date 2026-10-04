// =============================================================================
//  test_wb_datalayer.cpp -- can the machine data layer actually be LOADED?
//
//  AI(W906-WebBridge) 20260805.
//
//  WHY THIS EXISTS
//  Every machine-state global in this ported tree starts as a zero-initialised
//  object, and for a long time nothing ever filled any of them. A reader could
//  not tell "nobody loaded this" from "the machine really reads zero". For the
//  web HMI that distinction is the whole problem -- the browser renders null as
//  "---" and 0 as "0.00" on purpose, so publishing 0 for a value nobody loaded
//  is a screen that lies.
//
//  WHAT IT LOCKS DOWN
//    1. The data layer really does start empty (so a later "it works" claim
//       cannot be an artifact of something else having filled it).
//    2. ReadGeneralIni's ungated chain -- CustomerFunctionSelect /
//       ReadLastSetIni / ReadEventLogAutoSaveInfo -- DOES populate IniConfig
//       once the ini file has been opened. This is the regression guard on the
//       20260805 ungate in database.cpp.
//    3. LastSet stays zero even then. That gap is asserted, not glossed over,
//       so the day ReadLastDataFile() starts working this test fails and gets
//       updated instead of silently continuing to describe the old world.
//
//  THE TRAP THIS FILE EXISTS TO DOCUMENT
//  CheckAndReadIniDataGeneral() dereferences the global INIFileGeneral with NO
//  NULL check (faithful BCB6 behaviour, documented at that function in
//  common.cpp). INIFileGeneral is only ever set by OpenGeneralIniFile(), whose
//  only production caller is the SYSTEM_MODULAR constructor -- which is
//  entirely #if 0 in database.cpp. So ANY call into ReadGeneralIni without a
//  preceding OpenGeneralIniFile() segfaults, and that is true with the ungate
//  and without it (verified by control experiment).
//
//  DO-NOT-MODIFY-REAL-CONFIG
//  ReadGeneralIni and the chain below WRITE to whatever asGeneralPath points
//  at (CheckAndReadIniDataGeneral seeds missing keys). Exactly like
//  tests/test_ga1_readgeneralini.cpp, this copies the real Gerneral.ini to a
//  scratch path and repoints asGeneralPath at the copy; the real file is only
//  ever opened for reading by CopyFile, and the path is restored whatever
//  happens.
// =============================================================================
#include "database.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"

#include <windows.h>
#include <cstdio>

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

    // --- 1. the layer starts empty ------------------------------------------
    std::printf("-- 1. data layer before anything loads it\n");
    std::printf("   IniConfig.sMachineType = \"%s\"\n", IniConfig.sMachineType.c_str());
    std::printf("   LastSet.iRunStartMode  = %d\n", LastSet.iRunStartMode);

    check(IniConfig.sMachineType.Length() == 0,
          "IniConfig starts empty (nothing has populated it)");
    check(LastSet.iRunStartMode == 0 && LastSet.iTemperature == 0,
          "LastSet starts zero-initialised");

    // AI(W906-PT-W3-ungate) 20260808: the customer-profile layer, same
    // did-the-mechanism-run shape as the LastSet blob check below rather than an
    // assertion about one machine's flags.  This is the regression guard on the
    // 20260808 ungate of the SEVEN `#if 0` gates in cprod.cpp's
    // CustomerFunctionSelect() -- InitialCosFunction() (CosFunction.cpp:4063,
    // 517 flag assignments) plus the six per-customer profile functions.  Before
    // that ungate this struct stayed all-zero no matter what was loaded, which is
    // precisely the "hundreds of feature flags all 0" this file's own banner and
    // database.cpp:316 describe.
    size_t cosNonZeroBefore = 0;
    {
        const unsigned char* raw = reinterpret_cast<const unsigned char*>(&CosFunction);
        for (size_t i = 0; i < sizeof(HT9045_COUSTOMER_FUNCTION); ++i)
            if (raw[i] != 0) ++cosNonZeroBefore;
        std::printf("   CosFunction non-zero bytes = %u / %u\n",
                    (unsigned)cosNonZeroBefore, (unsigned)sizeof(HT9045_COUSTOMER_FUNCTION));
    }
    check(cosNonZeroBefore == 0,
          "CosFunction starts all-zero (the customer-profile layer has not run)");

    // --- 2. load it, on a scratch copy ---------------------------------------
    const AnsiString savedGeneralPath = asGeneralPath;

    char tmp[MAX_PATH];
    if (::GetTempPathA(MAX_PATH, tmp) == 0) {
        std::printf("SKIP: no temp path available\n");
        std::printf("\ntest_wb_datalayer: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }
    const AnsiString scratch = AnsiString(tmp) + "wb_datalayer_general.ini";

    if (!::CopyFileA(savedGeneralPath.c_str(), scratch.c_str(), FALSE)) {
        // The real Gerneral.ini is a machine artifact; on a box without one
        // there is nothing to load and nothing to assert. Skipping is correct,
        // and is reported rather than silently passing.
        std::printf("SKIP: cannot copy \"%s\" (err %lu) -- no config on this box\n",
                    savedGeneralPath.c_str(), (unsigned long)::GetLastError());
        std::printf("\ntest_wb_datalayer: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }

    std::printf("\n-- 2. loading via a scratch copy of Gerneral.ini\n");
    asGeneralPath = scratch;

    // Exercise the PRODUCTION entry point, not a hand-rolled copy of the same
    // sequence. LoadMachineConfig() (database.h) is what the application init
    // path is meant to call, so testing anything else would leave the real
    // thing untested while looking covered.
    const bool loaded = LoadMachineConfig();
    check(loaded, "LoadMachineConfig() reports success");

    //AI(W906-SJSON-S9b) 20260923: LoadMachineConfig() returning true does NOT
    //  mean the data layer loaded -- all it checks is that the TIniFile was
    //  constructed (database.cpp:3075).
    //
    //  SYSTEM_MODULAR::ReadGeneralIni() first opens a SECOND, different ini --
    //      D:\GPIB9045\system\general.ini  ->  [Version] Model
    //  -- and if that model is not one of the seven handler models it knows
    //  (9045GPIB / 9046GPIB / 9046_32GPIB / 9045GPIB_12Site / 502GPIB /
    //  1032GPIB / 7080GPIB) it sets bHandlerModel=false and RETURNS EARLY
    //  (database.cpp:328-334), before CUSTOMER_CODE is read and before
    //  CustomerFunctionSelect() / ReadLastSetIni() / ReadLastDataFile() run.
    //
    //  Every check below is about those exact calls having run, so on such a
    //  box they were asserting a precondition that was never met -- four
    //  failures that read like product defects and are not.
    //
    //  The early return is FAITHFUL: golden V912 database.cpp:308-316 has the
    //  identical seven-model list. A box whose GPIB bridge is configured as
    //  9050GPIB is simply not a handler model in EITHER tree.
    //  ⚠ Skip ONLY on that condition -- if the box IS a handler model and these
    //    are still empty, that is a real defect and must still fail.
    if (!bHandlerModel) {
        std::printf("\n");
        std::printf("SKIP: D:\\GPIB9045\\system\\general.ini [Version] Model is not one of\n");
        std::printf("      the seven handler models, so ReadGeneralIni() returned early\n");
        std::printf("      (database.cpp:328-334) and the data layer never loaded.\n");
        std::printf("      ENVIRONMENT, not a defect -- golden V912 database.cpp:308-316\n");
        std::printf("      carries the same list.\n");
        std::printf("\ntest_wb_datalayer: %d checks, %d failure(s)\n", g_total, g_fail);
        return g_fail == 0 ? 0 : 1;
    }

    std::printf("   IniConfig.sMachineType   = \"%s\"\n", IniConfig.sMachineType.c_str());
    std::printf("   IniConfig.sGPIBMachineID = \"%s\"\n", IniConfig.sGPIBMachineID.c_str());
    std::printf("   IniConfig.RMSTesterID    = \"%s\"\n", IniConfig.RMSTesterID.c_str());
    std::printf("   LastSet.iRunStartMode    = %d\n", LastSet.iRunStartMode);

    // --- 3. what worked, and what still does not -----------------------------
    check(IniConfig.sMachineType.Length() > 0,
          "IniConfig.sMachineType populated -> ReadLastSetIni ran and the "
          "20260805 ungate in database.cpp is live");
    check(IniConfig.sGPIBMachineID.Length() > 0,
          "IniConfig.sGPIBMachineID populated");

    // AI(W906-PT-W3-ungate) 20260808: the other half of the pair above.  Deliberately
    // NOT an assertion on any particular flag: which profile applies depends on this
    // box's CUSTOMER_CODE, so naming a flag would be asserting one machine's identity
    // (the same mistake this file's LastSet note calls out).  "It stopped being all
    // zero" is the property that belongs to the CODE.
    {
        const unsigned char* raw = reinterpret_cast<const unsigned char*>(&CosFunction);
        size_t cosNonZeroAfter = 0;
        for (size_t i = 0; i < sizeof(HT9045_COUSTOMER_FUNCTION); ++i)
            if (raw[i] != 0) ++cosNonZeroAfter;
        std::printf("   CosFunction non-zero bytes = %u / %u (was %u)\n",
                    (unsigned)cosNonZeroAfter, (unsigned)sizeof(HT9045_COUSTOMER_FUNCTION),
                    (unsigned)cosNonZeroBefore);
        check(cosNonZeroAfter > cosNonZeroBefore,
              "CustomerFunctionSelect() populated CosFunction -> the 20260808 ungate "
              "of the 7 customer-function gates is live");
    }

    // LastSet is loaded by ReadLastDataFile() as a RAW BINARY BLOB: it reads
    // sizeof(LAST_GENERAL_SET) bytes straight over the struct from
    // system\lastdata.dat (cprod.cpp:1682). Two things follow, and both were
    // nearly misdiagnosed as "LastSet does not load":
    //
    //  1. Individual fields can legitimately be 0. On this box iRunStartMode
    //     and iTemperature really are 0 IN THE FILE -- only 6.9% of its bytes
    //     are non-zero at all. Asserting a specific field would be asserting a
    //     property of one machine's saved state, not of the code.
    //  2. The ported struct is 178,896 bytes and the file is 178,096 -- 800
    //     bytes shorter. That is BY DESIGN: LastSet.h's own header says new
    //     fields may only be appended at the bottom, so an older file is short
    //     and the tail reads back as zero. It is NOT a packing mismatch; the
    //     values at the measured offsets line up.
    //
    // So the honest check is "did the blob land at all", by byte count.
    {
        const unsigned char* raw = reinterpret_cast<const unsigned char*>(&LastSet);
        size_t nonZero = 0;
        for (size_t i = 0; i < sizeof(LAST_GENERAL_SET); ++i)
            if (raw[i] != 0) ++nonZero;
        std::printf("   LastSet non-zero bytes   = %u / %u\n",
                    (unsigned)nonZero, (unsigned)sizeof(LAST_GENERAL_SET));
        check(nonZero > 0,
              "LastSet received the lastdata.dat blob -> ReadLastDataFile() works");
    }

    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;
    ::DeleteFileA(scratch.c_str());

    std::printf("\ntest_wb_datalayer: %d checks, %d failure(s)\n", g_total, g_fail);
    std::printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
