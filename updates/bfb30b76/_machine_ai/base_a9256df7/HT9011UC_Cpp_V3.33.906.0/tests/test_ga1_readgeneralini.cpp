// ===========================================================================
//  tests/test_ga1_readgeneralini.cpp
//
//  GA-1-B6 VERIFY: drive the TRANSLATED SYSTEM_MODULAR::ReadGeneralIni()
//  (database.cpp) over a scratch COPY of the REAL D:\HT9045\system\Gerneral.ini
//  and lock a representative set of ground-truth key/value oracles.
//
//  DO-NOT-MODIFY-REAL-CONFIG DISCIPLINE
//  -------------------------------------
//  ReadGeneralIni's own missing-key seeding (e.g. the AUTO_EMPTY_COLOR /
//  OTDRecord CheckIniData-then-WriteIniDataGeneral idiom) means it WRITES to
//  whatever `asGeneralPath` points at.  Exactly like tests/test_ini_helpers.cpp,
//  we COPY the real Gerneral.ini to a scratch path and repoint the global
//  `asGeneralPath` at the copy before calling HSys.ReadGeneralIni() -- the real
//  file is only ever opened O_RDONLY by CopyFile.  asGeneralPath is restored
//  (and the scratch file deleted) before returning, whatever the outcome.
//
//  Note ReadGeneralIni ALSO does one read against a SECOND, hardcoded,
//  non-redirectable path: "D:\\GPIB9045\\system\\general.ini" (golden
//  database.cpp:5) -- this is the GPIB bridge's own config, a distinct real
//  file this dev box already has (Model=9045GPIB).  The corresponding golden
//  write-back line is disabled in the golden itself
//  (`//WriteIniData(str,"Version","Model","9046_32GPIB");` -- commented out),
//  so this path is read-only in practice; we still guard+SKIP if it is absent
//  or its Model value would make ReadGeneralIni bail out early (bHandlerModel
//  stays false and the function returns after ~5 lines, before touching any
//  of the ~280 fields this test verifies).
//
//  CustomerFunctionSelect()/ReadLastSetIni()/ReadEventLogAutoSaveInfo() (which
//  golden calls unconditionally near the top, and which would touch
//  AuthPath+"config.ini") are #if 0-gated in this port (see database.cpp's own
//  comment on SYSTEM_MODULAR::ReadGeneralIni) because their BODIES are inside
//  cprod.cpp's own file-wide TODO(W6) gate and are not linkable yet -- so
//  AuthPath is never touched by this call and needs no redirect here.
//
//  RUNTIME-SKIP: if either real config file is absent, or the GPIB Model value
//  is not one ReadGeneralIni recognizes, this test SKIPs (exit 0) rather than
//  failing the suite on a box without the machine files -- mirrors
//  test_config_db.cpp / test_ini_helpers.cpp policy.
//
//  LINK NOTES: same shape as tests/test_config_loaders.cpp -- database.cpp
//  forward-declares MyDBIProcess(3-arg __fastcall, satisfied by ht9045_secsgem
//  at link time)/ShowMyMessage, and SystemModularInitial's `new HT9045Gem(...)`
//  pulls the SECSGEM archive into this binary's link (RESCAN group required).
//  We supply the same local stubs test_config_loaders.cpp does.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"      // asGeneralPath
#include "cmydef.h"      // MOTION_CARD_TYPE / IO_CARD_TYPE / CUSTOMER_CODE / ... (the ~280
                          // free globals ReadGeneralIni assigns; Type_HT9045 via MachineType.h)

#include <cstdio>
#include <cstdlib>

// <windows.h> (dragged in transitively via MachineType.h/common.h) #defines
// DeleteFile->DeleteFileA / CopyFile->CopyFileA, shadowing the AnsiString
// overloads we want -- same guard test_ini_helpers.cpp uses.
#ifdef DeleteFile
#  undef DeleteFile
#endif
#ifdef CopyFile
#  undef CopyFile
#endif

static int g_fail = 0;
static int g_total = 0;

static void check(bool cond, const char* expr, const char* file, int line) {
    ++g_total;
    if (!cond) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

// ---------------------------------------------------------------------------
//  ShowMyMessage / MyDBIProcess(2-arg) stubs -- see test_config_loaders.cpp's
//  own header note for why exactly these two (and not the 3-arg __fastcall
//  MyDBIProcess, which ht9045_secsgem now supplies for real).
// ---------------------------------------------------------------------------
// AI(W906-GA1-B2-integrate) 20260804: local ShowMyMessage stub RETIRED -- the full
// RESCAN group now supplies the real definition (was a duplicate).

// AI(W906-GA1-B2-integrate) 20260804: local MyDBIProcess stub RETIRED -- the full
// RESCAN group now supplies the real definition (was a duplicate).

// ---------------------------------------------------------------------------
//  HasICUnderMachine / HasAnyICInMachine link-satisfying stubs -- SAME
//  rationale as tests/test_config_loaders.cpp's own identical stubs: pulling
//  in ht9045_secsgem (for HT9045Gem/HGem, needed by SystemModularInitial in
//  this same database.cpp TU) unconditionally drags in uHGemClass.cpp.o,
//  whose S2F16 handler calls these two real csystem.h predicates
//  (csystem_predicates.cpp, part of ht9045_sm). This test never reaches S2F16
//  through ReadGeneralIni, so `false` is a correct link-time stand-in, not a
//  behavioral fork.
// ---------------------------------------------------------------------------
// AI(W906-GA1-B2-integrate) 20260804: local HasICUnderMachine stub RETIRED (real body in group)
// AI(W906-GA1-B2-integrate) 20260804: local HasAnyICInMachine stub RETIRED (real body in group)

// Scratch path under TEMP/TMP (falls back to cwd) -- same helper shape as
// tests/test_ini_helpers.cpp.
static AnsiString tmpPath(const char* leaf) {
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(leaf);
}

int main()
{
    const AnsiString kRealIni  = AnsiString("D:\\HT9045\\system\\Gerneral.ini");   // sic: "Gerneral" (real filename)
    const AnsiString kGpibIni  = AnsiString("D:\\GPIB9045\\system\\general.ini");

    if (!FileExists(kRealIni)) {
        std::printf("SKIP test_ga1_readgeneralini: real config not found (\"%s\"). Exiting 0.\n",
                    kRealIni.c_str());
        return 0;
    }
    if (!FileExists(kGpibIni)) {
        std::printf("SKIP test_ga1_readgeneralini: GPIB bridge config not found (\"%s\"); "
                    "ReadGeneralIni's own MachName gate would bail out before touching "
                    "any field. Exiting 0.\n", kGpibIni.c_str());
        return 0;
    }
    // Pre-check the Model value ReadGeneralIni itself will read from kGpibIni,
    // read-only, via the SAME helper (CheckAndReadIniData bound to an explicit
    // filename) -- if it is not one of golden's seven recognized names,
    // bHandlerModel stays false and ReadGeneralIni returns after ~5 lines.
    AnsiString gpibModel = CheckAndReadIniData(kGpibIni, AnsiString("Version"),
                                               AnsiString("Model"), AnsiString("ModelNG"));
    bool modelOk = (gpibModel == "9045GPIB"  || gpibModel == "9046GPIB" ||
                    gpibModel == "9046_32GPIB" || gpibModel == "9045GPIB_12Site" ||
                    gpibModel == "502GPIB"    || gpibModel == "1032GPIB" ||
                    gpibModel == "7080GPIB");
    if (!modelOk) {
        std::printf("SKIP test_ga1_readgeneralini: GPIB Model=\"%s\" not recognized -- "
                    "ReadGeneralIni would set bHandlerModel=false and return early. "
                    "Exiting 0.\n", gpibModel.c_str());
        return 0;
    }
    std::printf("GPIB bridge Model=\"%s\" (bHandlerModel gate open)\n", gpibModel.c_str());

    // =========================================================================
    //  Copy the real Gerneral.ini to a scratch oracle; repoint asGeneralPath.
    //  Every CheckAndReadIniDataGeneral/WriteIniDataGeneral/CheckIniData call
    //  inside ReadGeneralIni binds to this global -- the real file is only
    //  ever opened O_RDONLY by CopyFile.
    // =========================================================================
    AnsiString oracle = tmpPath("ht9045_ga1_general_oracle.ini");
    DeleteFile(oracle);                      // start clean (ignore result)
    bool copied = CopyFile(kRealIni, oracle, /*failIfExists=*/false);
    CHECK(copied);

    AnsiString savedGeneralPath = asGeneralPath;
    asGeneralPath = oracle;
    // CheckAndReadIniDataGeneral/WriteIniDataGeneral/CheckIniData all bind to
    // the module-level INIFileGeneral TIniFile*, which is NULL until opened
    // (normally done once by the app's startup sequence, well before
    // ReadGeneralIni runs -- outside this standalone test's reach). Mirrors
    // tests/test_ini_helpers.cpp's identical OpenGeneralIniFile() call.
    OpenGeneralIniFile();

    // ---- run the REAL, unmodified, translated production function ---------
    HSys.ReadGeneralIni();

    // =========================================================================
    //  Representative key/value oracle -- 16 fields spanning multiple ini
    //  sections ([System]/[TempCtrl]/[ATC]), read straight off the SAME
    //  scratch copy just consumed, cross-checked against the values
    //  hand-verified against the real file's current content.  A WRONG
    //  default is passed to each independent CheckAndReadIniDataGeneral below
    //  so a pass is provably "ReadGeneralIni actually set this from the file",
    //  not "the default fell through" (same discipline as test_ini_helpers.cpp).
    // =========================================================================
    std::printf("MOTION_CARD_TYPE=%d IO_CARD_TYPE=%d TTL_CARD_TYPE=%d "
                "TC401HeaterControl=%d CUSTOMER_CODE=%d NUMBER_PANEL_TYPE=%d\n",
                MOTION_CARD_TYPE, IO_CARD_TYPE, TTL_CARD_TYPE,
                TC401HeaterControl, CUSTOMER_CODE, NUMBER_PANEL_TYPE);
    std::printf("SAFE_DOOR_AMOUNT=%d USE_PICKER_COUNT=%d AUTO_EMPTY_COLOR=%d "
                "RTC_TemperNumber=%d BASE_X_TO_HP=%d iIndEPCnt=%d\n",
                SAFE_DOOR_AMOUNT, USE_PICKER_COUNT, AUTO_EMPTY_COLOR,
                RTC_TemperNumber, BASE_X_TO_HP, iIndEPCnt);
    std::printf("MachineTypeChoice=%d bTEMPCTRL_Shuttle_TOGTHER=%d "
                "asATC1ComPort=\"%s\" HOT_PLATE_POSITION=%d bHandlerModel=%d\n",
                MachineTypeChoice, (int)bTEMPCTRL_Shuttle_TOGTHER,
                HSys.asATC1ComPort.c_str(), HOT_PLATE_POSITION, (int)bHandlerModel);

    // [System] section -- direct card-type / model anchors.
    CHECK(MOTION_CARD_TYPE == 1);
    CHECK(IO_CARD_TYPE     == 1);
    CHECK(TTL_CARD_TYPE    == 0);
    CHECK(CUSTOMER_CODE    == 970);           // real file: CUSTOMER_CODE=970 (!=CC_ASE_KaohSiung(936), stays as-is)
    CHECK(NUMBER_PANEL_TYPE== 3);
    CHECK(SAFE_DOOR_AMOUNT == 2);
    CHECK(USE_PICKER_COUNT == 1);
    CHECK(AUTO_EMPTY_COLOR == 0);             // key present -> CheckAndReadIniDataGeneral else-branch read
    CHECK(BASE_X_TO_HP     == 6800);
    CHECK(HOT_PLATE_POSITION == 1);
    CHECK(iIndEPCnt == 16);                   // derived: Individual_EP_COUNT="16" -> iIndEPCnt=16 (not the raw string)
    CHECK(bHandlerModel == true);             // gate confirmed open (else this whole block never ran)

    // [TempCtrl] section.
    CHECK(TC401HeaterControl == 2);           // HEATER_CTRL_TYPE=2
    CHECK(RTC_TemperNumber   == 1);

    // MachName-driven branch (MachName=="9045GPIB" from the GPIB bridge config):
    // deterministic regardless of Gerneral.ini content.
    CHECK(MachineTypeChoice == Type_HT9045);
    CHECK(bTEMPCTRL_Shuttle_TOGTHER == true);  // unconditionally written true for MachName=="9045GPIB"

    // [ATC] section -- SYSTEM_MODULAR member (HSys.*), not a free global.
    CHECK(HSys.asATC1ComPort == AnsiString("COM7"));

    // ---- restore + cleanup (never touch the real file) ---------------------
    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;
    DeleteFile(oracle);

    std::printf("test_ga1_readgeneralini: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) {
        std::printf("RESULT: FAIL (%d failures)\n", g_fail);
        return 1;
    }
    std::printf("RESULT: PASS\n");
    return 0;
}
