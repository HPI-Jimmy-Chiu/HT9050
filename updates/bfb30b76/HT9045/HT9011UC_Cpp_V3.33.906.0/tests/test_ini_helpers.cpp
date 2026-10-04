// ===========================================================================
//  tests/test_ini_helpers.cpp
//
//  W3-cont CONFIG-READERS GATEWAY -- end-to-end verification of the translated
//  common.cpp ini-helper family driving the vclcompat TIniFile / TMemIniFile
//  shim, oracled against the REAL machine config D:\HT9045\system\Gerneral.ini.
//
//  WHAT THIS VERIFIES
//  ------------------
//   1) The General-ini helper path:
//        OpenGeneralIniFile() -> CheckAndReadIniDataGeneral(int) -> ...
//      reads the known-truth [System]/[TempCtrl] values out of the real ini:
//        [System]  MOTION_CARD_TYPE = 1
//        [System]  IO_CARD_TYPE     = 2
//        [System]  TTL_CARD_TYPE    = 2
//        [System]  INDEX_MOTION_CARD= 0
//        [TempCtrl]HEATER_CTRL_TYPE = 4
//      and that a MISSING key returns the supplied default verbatim.
//   2) A direct TMemIniFile / TIniFile shim round-trip (write -> reopen -> read,
//      default fallback, %0.4f double round-trip, TMemIniFile flush-on-UpdateFile).
//
//  DO-NOT-MODIFY-REAL-INI DISCIPLINE
//  ---------------------------------
//  CheckAndReadIniDataGeneral SEEDS missing keys by writing them, and the shim's
//  TIniFile is WRITE-THROUGH (flushes to disk on every write).  The real
//  Gerneral.ini is READ-ONLY for verification.  So we COPY the real ini to a
//  scratch path, repoint the global `asGeneralPath` at the copy, and run the
//  helpers against the copy.  The copy is a byte-faithful oracle; any seed write
//  (e.g. the missing-key default path) lands in the throw-away copy, never the
//  real file.  The real file is only ever opened O_RDONLY by CopyFile.
//
//  RUNTIME-SKIP: if the real ini is absent, the real-config block exits 0
//  (skip, not fail) -- mirrors test_config_db / test_IniFiles policy.
//
//  Self-contained CHECK harness (matches the other vclcompat/common tests).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "common.h"      // pulls MachineType.h -> <windows.h> (DeleteFileA macro etc.)

#include <cstdio>
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)

// <windows.h> (dragged in transitively via MachineType.h) #defines DeleteFile->
// DeleteFileA / CopyFile->CopyFileA / FileExists is fine.  Those text macros
// shadow the vclcompat AnsiString overloads we want.  Undo just the colliding
// ones so the AnsiString-based vclcompat helpers resolve.  (test_IniFiles.cpp
// never hits this because it does not include common.h / windows.h.)
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

// Scratch path under TEMP/TMP (falls back to cwd).
static AnsiString tmpPath(const char* leaf) {
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(W906_TestTmpName(leaf).c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
}

// The real machine config (READ-ONLY oracle).  This is the exact literal the
// translated common.cpp seeds asGeneralPath with (common.cpp:72), but we name
// it locally so the skip logic is explicit.
static const char* kRealIni = "D:\\HT9045\\system\\Gerneral.ini";

int main() {
    // =====================================================================
    //  A) REAL-CONFIG ORACLE via the translated *General* helper chain.
    // =====================================================================
    if (!FileExists(AnsiString(kRealIni))) {
        std::printf("SKIP real-config: %s not present\n", kRealIni);
    } else {
        // Copy the real ini to a scratch oracle so seeding writes never touch
        // the real file.  (CopyFile opens the source read-only.)
        AnsiString oracle = tmpPath("ht9045_general_oracle.ini");
        DeleteFile(oracle);                     // start clean (ignore result)
        bool copied = CopyFile(AnsiString(kRealIni), oracle, /*failIfExists=*/false);
        CHECK(copied);

        // Repoint the global the General helpers bind to, then open.
        // OpenGeneralIniFile() does: CloseGeneralIniFile(); new TIniFile(asGeneralPath).
        asGeneralPath = oracle;
        OpenGeneralIniFile();

        // ---- known-truth reads (key present -> NO seed write) -------------
        // Pass a WRONG default so a correct read is provably from the file,
        // not the default falling through.
        int motion = CheckAndReadIniDataGeneral(AnsiString("System"),
                                                AnsiString("MOTION_CARD_TYPE"), -1);
        int io     = CheckAndReadIniDataGeneral(AnsiString("System"),
                                                AnsiString("IO_CARD_TYPE"), -1);
        int ttl    = CheckAndReadIniDataGeneral(AnsiString("System"),
                                                AnsiString("TTL_CARD_TYPE"), -1);
        int idxmot = CheckAndReadIniDataGeneral(AnsiString("System"),
                                                AnsiString("INDEX_MOTION_CARD"), -1);
        int heater = CheckAndReadIniDataGeneral(AnsiString("TempCtrl"),
                                                AnsiString("HEATER_CTRL_TYPE"), -1);

        std::printf("real-ini read: MOTION_CARD_TYPE=%d IO_CARD_TYPE=%d "
                    "TTL_CARD_TYPE=%d INDEX_MOTION_CARD=%d HEATER_CTRL_TYPE=%d\n",
                    motion, io, ttl, idxmot, heater);

        CHECK(motion == 1);
        CHECK(io     == 2);
        CHECK(ttl    == 2);
        CHECK(idxmot == 0);
        CHECK(heater == 4);

        // ---- case-insensitive section + key (BCB6 profile semantics) ------
        CHECK(CheckAndReadIniDataGeneral(AnsiString("system"),
                                         AnsiString("motion_card_type"), -1) == 1);

        // ---- MISSING key returns the supplied default verbatim ------------
        // (the seed write lands in the throw-away oracle copy, never the real file)
        const int kDefault = 4242;
        int missing = CheckAndReadIniDataGeneral(
            AnsiString("System"),
            AnsiString("NO_SUCH_KEY_HT9045_VERIFY"), kDefault);
        CHECK(missing == kDefault);

        // Missing section, too.
        int missingSec = CheckAndReadIniDataGeneral(
            AnsiString("NO_SUCH_SECTION_HT9045"),
            AnsiString("Whatever"), 777);
        CHECK(missingSec == 777);

        // bool + AnsiString flavours over the real (copied) data: missing -> default.
        bool mb = CheckAndReadIniDataGeneral(AnsiString("System"),
                                             AnsiString("NO_SUCH_BOOL_KEY"), true);
        CHECK(mb == true);
        AnsiString ms = CheckAndReadIniDataGeneral(AnsiString("System"),
                                                   AnsiString("NO_SUCH_STR_KEY"),
                                                   AnsiString("FALLBACK"));
        CHECK(ms == AnsiString("FALLBACK"));

        // Close (flushes the oracle copy; since 4d3468a3 CloseGeneralIniFile also sets
        // INIFileGeneral to nullptr, so nothing dangles -- and nothing below reopens it).
        CloseGeneralIniFile();
        DeleteFile(oracle);
    }

    // =====================================================================
    //  B) DIRECT TMemIniFile / TIniFile shim round-trip (no real ini needed).
    // =====================================================================
    {
        AnsiString path = tmpPath("ht9045_ini_helpers_direct.ini");
        DeleteFile(path);

        // --- TIniFile write-through: each write flushes to disk -----------
        {
            TIniFile* ini = new TIniFile(path);
            CHECK(ini->FileName == path);
            CHECK(ini->SectionExists(AnsiString("System")) == false);

            ini->WriteInteger(AnsiString("System"), AnsiString("MOTION_CARD_TYPE"), 1);
            ini->WriteInteger(AnsiString("System"), AnsiString("IO_CARD_TYPE"),     2);
            ini->WriteBool   (AnsiString("System"), AnsiString("Flag"),          true);
            ini->WriteString (AnsiString("TempCtrl"), AnsiString("HEATER_CTRL_TYPE"),
                              AnsiString("4"));
            ini->UpdateFile();   // commit/no-op for write-through
            delete ini;
        }

        // Reopen via TMemIniFile (eager full load) and verify it sees what the
        // write-through TIniFile flushed.
        {
            TMemIniFile* mem = new TMemIniFile(path);
            CHECK(mem->SectionExists(AnsiString("System")) == true);
            CHECK(mem->ValueExists(AnsiString("System"), AnsiString("MOTION_CARD_TYPE")) == true);
            CHECK(mem->ReadInteger(AnsiString("System"), AnsiString("MOTION_CARD_TYPE"), -1) == 1);
            CHECK(mem->ReadInteger(AnsiString("System"), AnsiString("IO_CARD_TYPE"),     -1) == 2);
            CHECK(mem->ReadBool   (AnsiString("System"), AnsiString("Flag"),          false) == true);
            CHECK(mem->ReadInteger(AnsiString("TempCtrl"), AnsiString("HEATER_CTRL_TYPE"), -1) == 4);

            // default fallback: missing key/section -> default verbatim
            CHECK(mem->ReadInteger(AnsiString("System"), AnsiString("Nope"), 99) == 99);
            CHECK(mem->ReadInteger(AnsiString("NoSec"), AnsiString("Nope"), 55) == 55);

            // --- TMemIniFile flush policy: mutations are MEMORY-ONLY until
            //     UpdateFile().  Prove it by writing, then reading the file from
            //     a second handle BEFORE the flush (must NOT see it), then after.
            mem->WriteInteger(AnsiString("System"), AnsiString("LateKey"), 1357);
            {
                TIniFile peekBefore(path);   // re-reads disk
                CHECK(peekBefore.ValueExists(AnsiString("System"), AnsiString("LateKey")) == false);
            }
            mem->UpdateFile();               // the ONLY disk flush for Mem
            {
                TIniFile peekAfter(path);
                CHECK(peekAfter.ReadInteger(AnsiString("System"), AnsiString("LateKey"), -1) == 1357);
            }
            delete mem;
        }

        // --- %0.4f double round-trip (how CheckAndReadIniData(double) persists):
        //     WriteString("%0.4f") text, re-read via ReadFloat.
        {
            TIniFile* ini = new TIniFile(path);
            AnsiString s; s.sprintf("%0.4f", 12.5);
            ini->WriteString(AnsiString("F"), AnsiString("V"), s);
            double back = ini->ReadFloat(AnsiString("F"), AnsiString("V"), -1.0);
            CHECK(back > 12.4999 && back < 12.5001);
            // ReadFloat default fallback on miss
            CHECK(ini->ReadFloat(AnsiString("F"), AnsiString("Missing"), 3.14) == 3.14);
            delete ini;
        }

        DeleteFile(path);
    }

    // =====================================================================
    std::printf("ini_helpers: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) {
        std::printf("RESULT: FAIL (%d failures)\n", g_fail);
        return 1;
    }
    std::printf("RESULT: PASS\n");
    return 0;
}
