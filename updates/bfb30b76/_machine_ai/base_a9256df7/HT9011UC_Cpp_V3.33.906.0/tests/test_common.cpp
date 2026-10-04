// ===========================================================================
//  tests/test_common.cpp
//
//  W906-CommonWaveFile -- smoke tests for the 6 functions un-gated that wave
//  in common.cpp/.h:
//    WriteDataToFile(char*,char*,bool) / WriteDataToFile(AnsiString,AnsiString,bool)
//    CheckFileIsEmpty(AnsiString)
//    ReadDataFromFile(AnsiString)
//    MyForceDirectories(AnsiString,AnsiString)
//    GetLastOpenFN() / WriteLastDataFN(AnsiString)
//
//  AI(W906-CommonCompletion) 20260721: extended with coverage for the items
//  un-gated THIS wave: AddSpace/OnlyNumberInPut family, InitCommonString/
//  EncodeStr/DecodeStr, GetRecipePath/GetRecipeFileName, ChangeSaveFileName,
//  MySecondsBetween, SGDToCSV, IsFileInUse, CopyAndCompressFile (compile/link
//  smoke only -- see its own test block for why), MyTickCount/MySleep/
//  MySleepEx. TempChangeLog is NOT covered here -- attempted, then re-gated
//  this same wave (see common.cpp's own banner at that item for why).
//
//  All file I/O happens under a SCRATCH directory (TEMP/TMP, falls back to
//  cwd) -- never a production path. `LastDataPath` (a mutable global common.cpp
//  otherwise seeds to "D:\\HT9045\\SetUp.inf") is repointed to a scratch file
//  before exercising GetLastOpenFN/WriteLastDataFN, mirroring the asGeneralPath
//  repoint idiom already established in test_ini_helpers.cpp.
//
//  Self-contained CHECK harness (matches the other vclcompat/common tests).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "common.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

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

static AnsiString tmpPath(const char* leaf) {
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(leaf);
}

int main() {
    // =====================================================================
    //  A) WriteDataToFile(char*,...) + ReadDataFromFile round-trip.
    // =====================================================================
    {
        AnsiString path = tmpPath("ht9045_common_wdtf.txt");
        DeleteFile(path);   // start clean (ignore result)

        // NOTE on assertions below: WriteDataToFile/ReadDataFromFile both use
        // golden's literal TEXT-mode fopen (no "b"), which has a real, VERIFIED
        // golden bug documented on ReadDataFromFile's own comment in
        // common.cpp -- CRLF translation means the tail bytes of the returned
        // buffer (right before the forced final NUL) can be uninitialized
        // malloc() garbage, not real content, whenever the source has embedded
        // newlines (which WriteDataToFile's files always do, since it appends
        // one unconditionally). So checks here use strncmp against a known-safe
        // PREFIX length, never a full strcmp/NUL-terminated comparison, to
        // avoid asserting on that undefined tail.
        WriteDataToFile(path.c_str(), "line one", true);   // overwrite/create
        char* buf1 = ReadDataFromFile(path);
        CHECK(buf1 != NULL);
        if (buf1 != NULL) {
            CHECK(std::strncmp(buf1, "line one", 8) == 0);
            free(buf1);   // OWNERSHIP CONTRACT: caller frees
        }

        // Default bOverWrite=false -> append.
        WriteDataToFile(path.c_str(), "line two");
        char* buf2 = ReadDataFromFile(path);
        CHECK(buf2 != NULL);
        if (buf2 != NULL) {
            CHECK(std::strncmp(buf2, "line one\nline two", 17) == 0);
            free(buf2);
        }

        // AnsiString overload forwards to the char* overload -- prove it does
        // NOT recurse (would stack-overflow/crash the whole test process if it
        // did; this is the exact bug this wave's own build+gdb pass caught and
        // fixed by switching the char* overload's params to `const char*`).
        WriteDataToFile(AnsiString(path), AnsiString("line three"), false);
        char* buf3 = ReadDataFromFile(path);
        CHECK(buf3 != NULL);
        if (buf3 != NULL) {
            CHECK(std::strncmp(buf3, "line one\nline two\nline three", 28) == 0);
            free(buf3);
        }

        // AnsiString overwrite=true truncates.
        WriteDataToFile(AnsiString(path), AnsiString("fresh"), true);
        char* buf4 = ReadDataFromFile(path);
        CHECK(buf4 != NULL);
        if (buf4 != NULL) {
            CHECK(std::strncmp(buf4, "fresh", 5) == 0);
            free(buf4);
        }

        DeleteFile(path);
    }

    // =====================================================================
    //  B) ReadDataFromFile on a missing file -> NULL.
    // =====================================================================
    {
        AnsiString path = tmpPath("ht9045_common_no_such_file.txt");
        DeleteFile(path);
        char* buf = ReadDataFromFile(path);
        CHECK(buf == NULL);
    }

    // =====================================================================
    //  C) CheckFileIsEmpty -- existing-empty / existing-non-empty / missing.
    //     GOLDEN BUG (verified empirically -- see CheckFileIsEmpty's own
    //     comment in common.cpp): the return value is effectively INVERTED
    //     from what the function's name suggests.
    //       - missing file            -> true
    //       - existing, has content   -> true
    //       - existing, genuinely empty -> false
    //     i.e. true means "missing OR non-empty"; false means "exists and is
    //     empty". Do not naively assume true=="is empty" here.
    // =====================================================================
    {
        AnsiString emptyPath = tmpPath("ht9045_common_empty.txt");
        DeleteFile(emptyPath);
        // NOTE: WriteDataToFile always appends a trailing "\n" after cData, so
        // even an empty cData would yield a 1-byte file, not a genuinely empty
        // one. Use fopen/fclose directly to produce a true 0-byte file.
        FILE* f = fopen(emptyPath.c_str(), "w");
        CHECK(f != NULL);
        if (f != NULL) fclose(f);
        CHECK(CheckFileIsEmpty(emptyPath) == false);   // golden bug: genuinely-empty -> false

        AnsiString nonEmptyPath = tmpPath("ht9045_common_nonempty.txt");
        DeleteFile(nonEmptyPath);
        WriteDataToFile(nonEmptyPath.c_str(), "not empty", true);
        CHECK(CheckFileIsEmpty(nonEmptyPath) == true);   // golden bug: has content -> true

        AnsiString missingPath = tmpPath("ht9045_common_missing.txt");
        DeleteFile(missingPath);
        CHECK(CheckFileIsEmpty(missingPath) == true);   // missing -> true (same as "has content")

        DeleteFile(emptyPath);
        DeleteFile(nonEmptyPath);
    }

    // =====================================================================
    //  D) MyForceDirectories -- creates a nested scratch directory.
    // =====================================================================
    {
        AnsiString base = tmpPath("ht9045_common_mfd_root");
        AnsiString nested = base + AnsiString("\\a\\b\\c\\");

        // Best-effort pre-clean (ignore result; directory may not exist yet).
        RemoveDir(base + AnsiString("\\a\\b\\c"));
        RemoveDir(base + AnsiString("\\a\\b"));
        RemoveDir(base + AnsiString("\\a"));
        RemoveDir(base);

        CHECK(DirectoryExists(nested) == false);
        int ret = MyForceDirectories(nested, AnsiString("test_common"));
        CHECK(ret == 1);
        CHECK(DirectoryExists(nested) == true);

        // Idempotent: calling again on an already-existing directory is a
        // no-op success (golden: DirectoryExists(...)==false guards ForceDirectories).
        int ret2 = MyForceDirectories(nested, AnsiString("test_common"));
        CHECK(ret2 == 1);

        // Empty path -> -1 (golden: RecordProcess + return -1).
        CHECK(MyForceDirectories(AnsiString(""), AnsiString("test_common")) == -1);

        // Best-effort cleanup (reverse order; ignore failures).
        RemoveDir(base + AnsiString("\\a\\b\\c"));
        RemoveDir(base + AnsiString("\\a\\b"));
        RemoveDir(base + AnsiString("\\a"));
        RemoveDir(base);
    }

    // =====================================================================
    //  E) GetLastOpenFN / WriteLastDataFN round-trip against a repointed
    //     LastDataPath (never touches the real D:\HT9045\SetUp.inf default).
    // =====================================================================
    {
        AnsiString savedLastDataPath = LastDataPath;   // restore at the end

        AnsiString scratchPath = tmpPath("ht9045_common_lastdata.inf");
        DeleteFile(scratchPath);
        LastDataPath = scratchPath;

        // E1) File does not exist yet -> GetLastOpenFN returns the "Fail Open"
        //     default (golden: ShowMyMessage("LastData does not exist") -- our
        //     stub is a no-op, so only the return value is observable).
        CHECK(FileExists(LastDataPath) == false);
        AnsiString r1 = GetLastOpenFN();
        CHECK(r1 == AnsiString("Fail Open"));

        // E2) WriteLastDataFN bootstrap branch (LastDataPath still absent at
        //     call time -> golden's FileCreate/FileWrite/FileClose path,
        //     substituted this wave with TStringList Add+SaveToFile -- see
        //     common.cpp's own deviation note on WriteLastDataFN).
        WriteLastDataFN(AnsiString("SETUP_A.INI"));
        CHECK(FileExists(LastDataPath) == true);
        AnsiString r2 = GetLastOpenFN();
        CHECK(r2 == AnsiString("SETUP_A.INI"));

        // E3) WriteLastDataFN if-branch (LastDataPath now exists -> TStringList
        //     LoadFromFile/Clear/Add/SaveToFile path).
        WriteLastDataFN(AnsiString("SETUP_B.INI"));
        AnsiString r3 = GetLastOpenFN();
        CHECK(r3 == AnsiString("SETUP_B.INI"));

        DeleteFile(LastDataPath);
        LastDataPath = savedLastDataPath;
    }

    // =====================================================================
    //  F) W906-CommonCompletion additions -- OnlyNumberInPut family / AddSpace.
    // =====================================================================
    {
        CHECK(OnlyNumberInPut('5') == true);
        CHECK(OnlyNumberInPut(8) == true);        // backspace
        CHECK(OnlyNumberInPut('-') == true);
        CHECK(OnlyNumberInPut('.') == false);
        CHECK(OnlyNumberInPut('a') == false);

        CHECK(OnlyNumberAndDotInPut('.') == true);
        CHECK(OnlyNumberAndDotInPut('7') == true);
        CHECK(OnlyNumberAndDotInPut(8) == true);
        CHECK(OnlyNumberAndDotInPut('a') == false);

        CHECK(OnlyMakeFileDataInPut('a') == true);
        CHECK(OnlyMakeFileDataInPut('\'') == false);
        CHECK(OnlyMakeFileDataInPut('/') == false);
        CHECK(OnlyMakeFileDataInPut(':') == false);
        CHECK(OnlyMakeFileDataInPut('*') == false);
        CHECK(OnlyMakeFileDataInPut('?') == false);
        CHECK(OnlyMakeFileDataInPut('"') == false);
        CHECK(OnlyMakeFileDataInPut('<') == false);
        CHECK(OnlyMakeFileDataInPut('|') == false);

        // AddSpace is a golden no-op -- just confirm it's callable (no crash).
        AddSpace(AnsiString("anything"));
        CHECK(true);
    }

    // =====================================================================
    //  G) EncodeStr / DecodeStr round-trip (pure XOR codec against the
    //     file-local asKeyStr "HontechPassword").
    // =====================================================================
    {
        AnsiString plain = AnsiString("Hello, HT9045!");
        AnsiString enc = EncodeStr(plain);
        AnsiString dec = DecodeStr(enc);
        CHECK(dec == plain);

        CHECK(DecodeStr(EncodeStr(AnsiString(""))) == AnsiString(""));
    }

    // =====================================================================
    //  H) InitCommonString -- repoints a representative sample of path
    //     globals; restored afterwards so later tests / other TUs are not
    //     affected by this process-wide global mutation.
    // =====================================================================
    {
        AnsiString savedSecsGemPath      = SecsGemPath;
        AnsiString savedAsBarCodeDownLot = asBarCodeDownLot;
        AnsiString savedAsSortingBy2DID  = asSortingBy2DID;

        InitCommonString();

        // VERIFIED DEVIATIONS from decl-site defaults (see InitCommonString's
        // own banner in common.cpp for the full explanation) -- these 3 do
        // NOT match their declaration-site defaults after InitCommonString runs.
        CHECK(SecsGemPath      == AnsiString("D:\\HT9045\\SECS\\SYSTEM\\Gerneral.ini"));
        CHECK(asBarCodeDownLot == AnsiString("D:\\HT9045_Log\\2DBarCode\\LotData1.txt"));
        CHECK(asSortingBy2DID  == AnsiString("D:\\HT9045_Log\\2DBarCode\\"));

        //AI(W906-BA-BU0g) 20260911: this used to be an unconditional
        //  CHECK(DataPath == "D:\\HT9045\\IniData\\Data\\"). It is now the
        //  REDIRECT ASSERTION the containment discipline requires, because
        //  common.cpp's W906IniDataRedirect() seam makes the correct value
        //  depend on the environment -- and tests/CMakeLists.txt:3316 sets
        //  W906_INIDATA_ROOT for the whole suite.
        //
        //  WHY AN ASSERTION AND NOT JUST A RELAXATION: a redirect that
        //  silently fails looks identical to a pass. Relaxing this check to
        //  "don't care" would have hidden exactly the incident that created
        //  the seam -- on 20260911 this tree's own ba0 baseline gate wrote
        //  into the machine's live recipe for the ACTIVE work order
        //  (iIndexArmAutoCleanCnt 4062 -> 0, [Mode] Temperature 125.0 -> 25.0),
        //  and every test that did it PASSED.
        //
        //  So both arms are checked, and each is the only thing that can catch
        //  its own failure mode:
        //    env UNSET -> must be golden's literal, byte for byte. Catches a
        //                 seam that leaks a scratch path into production.
        //    env SET   -> must equal the override AND differ from the literal.
        //                 Catches a seam that silently did NOT take, which is
        //                 the dangerous direction: the write lands on the real
        //                 machine and nothing fails.
        {
            const char* iniDataRoot = getenv("W906_INIDATA_ROOT");
            if (iniDataRoot == 0 || *iniDataRoot == '\0')
            {
                CHECK(DataPath == AnsiString("D:\\HT9045\\IniData\\Data\\"));
            }
            else
            {
                const AnsiString expect = AnsiString(iniDataRoot) + AnsiString("\\Data\\");
                CHECK(DataPath == expect);
                // Belt to that brace: prove the redirect moved it OFF the real
                // tree, not merely that it equals some computed string.
                CHECK(DataPath != AnsiString("D:\\HT9045\\IniData\\Data\\"));
            }
        }

        SecsGemPath      = savedSecsGemPath;
        asBarCodeDownLot = savedAsBarCodeDownLot;
        asSortingBy2DID  = savedAsSortingBy2DID;
    }

    // =====================================================================
    //  I) GetRecipePath / GetRecipeFileName -- scratch-repointed DataPath +
    //     LastDataPath so this never touches the real D:\HT9045\IniData\Data\.
    // =====================================================================
    {
        AnsiString savedDataPath     = DataPath;
        AnsiString savedLastDataPath = LastDataPath;

        AnsiString scratchLastData = tmpPath("ht9045_common_recipe_lastdata.inf");
        DeleteFile(scratchLastData);
        LastDataPath = scratchLastData;
        WriteLastDataFN(AnsiString("RECIPE_A"));   // GetLastOpenFN() -> "RECIPE_A"

        DataPath = AnsiString("C:\\scratch\\Data\\");

        AnsiString recipePath = GetRecipePath();
        CHECK(recipePath == AnsiString("C:\\scratch\\Data\\RECIPE_A\\"));

        AnsiString recipeFile = GetRecipeFileName(AnsiString("workfile.ini"));
        CHECK(recipeFile == AnsiString("C:\\scratch\\Data\\RECIPE_A\\workfile.ini"));

        DeleteFile(scratchLastData);
        DataPath     = savedDataPath;
        LastDataPath = savedLastDataPath;
    }

    // =====================================================================
    //  J) ChangeSaveFileName -- sanitizes several separator characters.
    //     GOLDEN BUG (see common.cpp's own banner): the `"` branch is dead
    //     code (a literal `""""` collapses to an empty Pos()/StringReplace()
    //     pattern under standard C++ literal concatenation), so a literal `"`
    //     survives unchanged -- verified below, not "fixed".
    // =====================================================================
    {
        AnsiString dirty = AnsiString("a\\b/c:d*e?f\"g<h>i|j");
        AnsiString clean = ChangeSaveFileName(dirty);
        CHECK(clean == AnsiString("a_b_c_d_e_f\"g_h_i_j"));
    }

    // =====================================================================
    //  K) MySecondsBetween -- two known TDateTime values with a known delta.
    // =====================================================================
    {
        TDateTime dt1 = EncodeDate(2026, 1, 1) + EncodeTime(0, 0, 0, 0);
        TDateTime dt2 = EncodeDate(2026, 1, 1) + EncodeTime(0, 5, 30, 0);   // +5min30sec
        // AI(W906-Reconcile) 20260721: golden bug, preserved verbatim, not "330".
        // TempDT=DT2-DT1 is a sub-1-day duration (330s = 0.003819...days).
        // DecodeDate() on a value in [0,1) decodes it as an ABSOLUTE date near
        // the OLE epoch (TDateTime 0.0 == 1899-12-30), so `day` comes back as
        // the calendar day-of-month 30 (Dec 30), not an elapsed-day count of 0.
        // day*86400 then dominates: 30*86400 + 0*3600 + 5*60 + 30 = 2592330.
        // Same bug shape as today's other preserved golden quirks (e.g.
        // CheckFileIsEmpty's inverted return) -- MySecondsBetween is only ever
        // meaningful in golden for whatever narrow multi-day usage its sole
        // (untranslated) caller Automation/uRENESAS_Server.cpp relies on; it is
        // NOT a general-purpose seconds-between-two-instants function despite
        // the name, and this test asserts golden's real computed value.
        CHECK(MySecondsBetween(dt1, dt2) == 2592330);
    }

    // =====================================================================
    //  L) SGDToCSV -- a small scratch TStringGrid written out via
    //     WriteDataToFile, one line per grid row.
    // =====================================================================
    {
        vclcompat::TStringGrid grid(3, 2);   // 3 cols x 2 rows
        grid.Cells[0][0] = AnsiString("A1"); grid.Cells[1][0] = AnsiString("B1"); grid.Cells[2][0] = AnsiString("C1");
        grid.Cells[0][1] = AnsiString("A2"); grid.Cells[1][1] = AnsiString("B,2"); grid.Cells[2][1] = AnsiString("C2");

        AnsiString path = tmpPath("ht9045_common_sgdtocsv.csv");
        DeleteFile(path);

        SGDToCSV(&grid, AnsiString(","), AnsiString(";"), path);

        // Prefix-only check (see ReadDataFromFile's own text-mode CRLF golden
        // bug note -- the last 2 bytes, one per embedded '\n', can be
        // uninitialized malloc() tail garbage; the length below stays well
        // short of that boundary).
        char* buf = ReadDataFromFile(path);
        CHECK(buf != NULL);
        if (buf != NULL) {
            CHECK(std::strncmp(buf, "A1,B1,C1,\nA2,B;2,C2,", 20) == 0);
            free(buf);
        }
        DeleteFile(path);
    }

    // =====================================================================
    //  M) IsFileInUse -- a plain scratch file is not in use; exclusively
    //     opening it (no FILE_SHARE_* flags) makes it appear "in use" (Windows
    //     enforces sharing per-handle, so this reproduces within one process).
    // =====================================================================
    {
        AnsiString path = tmpPath("ht9045_common_isfileinuse.txt");
        DeleteFile(path);
        WriteDataToFile(path.c_str(), "not locked", true);

        CHECK(IsFileInUse(path.c_str()) == false);

        HANDLE hExclusive = CreateFile(path.c_str(), GENERIC_READ, 0 /* no sharing */,
                                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        CHECK(hExclusive != INVALID_HANDLE_VALUE);
        if (hExclusive != INVALID_HANDLE_VALUE) {
            CHECK(IsFileInUse(path.c_str()) == true);
            CloseHandle(hExclusive);
        }

        DeleteFile(path);
    }

    // =====================================================================
    //  N) CopyAndCompressFile -- NOT exercised by a real call: it launches
    //     the real d:\HT9045\7z.exe via a detached system() shell command and
    //     depends on 7-Zip actually being installed/reachable, neither of
    //     which this test harness can assume. Faking a call here without a
    //     real 7z.exe present would either no-op past the
    //     `if(FileExists(sSourceFilePath))` guard (nothing exercised) or
    //     launch a `start /B` process this test has no way to verify --
    //     neither is a meaningful test, so it is deliberately left uncalled.
    //     Taking its address proves the real common.cpp definition compiles
    //     and links into this test binary.
    // =====================================================================
    {
        bool (*fnptr)(const AnsiString&, const AnsiString&, const AnsiString&,
                      const AnsiString&, const int&, AnsiString&) = &CopyAndCompressFile;
        CHECK(fnptr != NULL);
    }

    // =====================================================================
    //  O) MyTickCount / MySleep / MySleepEx -- basic sanity: MyTickCount()
    //     is non-decreasing across a MySleep call.
    // =====================================================================
    {
        DWORD t1 = MyTickCount();
        MySleep(20);
        DWORD t2 = MyTickCount();
        CHECK(t2 >= t1);   // GetTickCount() is monotonic (ms wraps at ~49.7 days, not hit here)

        DWORD ret = MySleepEx(20, false);
        (void)ret;         // SleepEx's return code -- not asserted, just confirm callable
        CHECK(true);
    }

    if (g_fail == 0)
        std::printf("test_common: ALL %d checks passed\n", g_total);
    else
        std::printf("test_common: %d/%d checks FAILED\n", g_fail, g_total);

    return g_fail == 0 ? 0 : 1;
}
