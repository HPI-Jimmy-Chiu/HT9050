// ===========================================================================
//  tests/test_wb_recipedoc.cpp -- WebBridgeRecipeDoc: the browser's read AND
//  write view of a recipe/settings document.
//
//  AI(W906-FW-C1R) 20260911 (groups 1-8, the reader).
//  AI(W906-FW-C1W) 20260911 (groups 9-13, the writer).
//
//  ---------------------------------------------------------------------------
//  WHAT THIS PINS, AND WHY EACH ONE IS HERE
//  ---------------------------------------------------------------------------
//  Every behaviour below was LEARNED on 20260911 by disagreeing with the store
//  and losing. They are pinned so the next person does not re-derive them:
//
//   * value NOT trimmed / key trimmed / FIRST '=' splits
//         vclcompat/IniFiles.cpp:162-164. A reader that trimmed values would
//         disagree with what BCB6 reads from the same bytes, which is the
//         comparison this whole feature exists to support.
//   * duplicate [section] -- FIRST WINS
//         Win32's profile API stops at the first matching section; the store
//         follows it. Measured on the recipe
//         "IniData/Data/SNH-FCBGA3757-68X68-H80C/ArmCondition.Data", a
//         genuinely corrupted production file with two [All] blocks: the store
//         keeps the first 3 keys and discards the later 11, and so does the
//         real handler. (Forward slashes on purpose -- a comment line ending in
//         a backslash splices the next line, and -Wcomment reports it even
//         though a pragma cannot suppress it. Same trap as golden
//         cLd_ULd.cpp:189.)
//   * duplicate key inside one section -- FIRST WINS
//         Same file: 'Vacuum Check Time' appears twice under [Iput  Arm], at
//         =0.10 and at =UsEnHP00. The store returns 0.10. Which value the
//         machine uses is decided by line order, so this is not cosmetic:
//         reverse the order and atof("UsEnHP00") is 0.
//   * NO-WRITE
//         The reader must not touch the file. TIniFile's destructor is a no-op,
//         but TMemIniFile's calls flush() unconditionally
//         (IniFiles.cpp:370-375) -- one wrong class and reading a recipe
//         rewrites it. system\Gerneral.ini was rewritten wholesale on 20260817
//         exactly that way, so this is a regression guard, not paranoia.
//
//  Hermetic: every file it touches is a RELATIVE temp path in the working
//  directory. It never names a path under D:\HT9045, so it cannot reach real
//  machine data even if the assertions are wrong.
//
//  The writer's groups (9-13) additionally pin that a DRY RUN writes nothing at
//  all, that an applied edit changes only the value text on the lines it was
//  given -- asserted by rebuilding the whole expected file, comments and uneven
//  indentation included -- and that a field the store cannot see is COUNTED,
//  never appended. Appending would put a key where the machine will not look:
//  a silent no-op that reports success.
//
//  Build: WebBridgeRecipeDoc.cpp + vclcompat + ht9045_webbridge. NO god-stack --
//  the TU defines no machine symbol and needs none, measured with
//  `nm --undefined-only`. If this link list ever has to grow, the reader or the
//  writer has started reaching into the machine and that is the thing to look at
//  first.
//
//  Non-zero exit on any failure.
// ===========================================================================
#include "WebBridgeRecipeDoc.h"

#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static int g_fail  = 0;
static int g_total = 0;

static void check(bool cond, const char* expr, const char* file, int line) {
    ++g_total;
    if (!cond) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}

static void check_eq_str(const std::string& got, const std::string& want,
                         const char* expr, const char* file, int line) {
    ++g_total;
    if (got != want) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n      got  %s\n      want %s\n",
                    file, line, expr, got.c_str(), want.c_str());
    }
}

#define CHECK(cond)          check((cond), #cond, __FILE__, __LINE__)
#define CHECK_STR(got, want) check_eq_str((got), (want), #got " == " #want, \
                                          __FILE__, __LINE__)

// ---------------------------------------------------------------------------
//  Temp files, relative to the working directory only.
// ---------------------------------------------------------------------------
static const char* kTmp  = "test_wb_recipedoc_tmp.Data";
static const char* kGone = "test_wb_recipedoc_absent.Data";

static bool WriteRaw(const char* path, const std::string& bytes) {
    std::FILE* fp = std::fopen(path, "wb");
    if (!fp) return false;
    const size_t n = bytes.empty() ? 0
                                   : std::fwrite(bytes.data(), 1, bytes.size(), fp);
    std::fclose(fp);
    return n == bytes.size();
}

static bool StatOf(const char* path, long* size, long long* mtime) {
    struct stat st;
    if (::stat(path, &st) != 0) return false;
    *size  = static_cast<long>(st.st_size);
    *mtime = static_cast<long long>(st.st_mtime);
    return true;
}

static std::string Read(const char* path) {
    return ht9045::RecipeDocToJson(vclcompat::AnsiString(path));
}

static std::string Slurp(const char* path) {
    std::FILE* fp = std::fopen(path, "rb");
    if (!fp) return std::string();
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0) s.append(buf, n);
    std::fclose(fp);
    return s;
}

static bool FileThere(const char* path) {
    std::FILE* fp = std::fopen(path, "rb");
    if (!fp) return false;
    std::fclose(fp);
    return true;
}

static ht9045::RecipeFieldEdit MakeEdit(const char* section, const char* key,
                                       const char* raw) {
    ht9045::RecipeFieldEdit e;
    e.section  = section;
    e.key      = key;
    e.rawValue = raw;
    return e;
}

static bool Contains(const std::string& hay, const char* needle) {
    return hay.find(needle) != std::string::npos;
}

// ---------------------------------------------------------------------------
int main() {
    // -----------------------------------------------------------------------
    //  1. The classifier. "int" vs "float" comes from the RAW's SHAPE, not
    //     from the numeric value: "2.00" is a float whose value is 2.
    // -----------------------------------------------------------------------
    using namespace ht9045;
    CHECK(ClassifyRecipeField("70")          == kRecipeFieldInt);
    CHECK(ClassifyRecipeField("-5")          == kRecipeFieldInt);
    CHECK(ClassifyRecipeField("+5")          == kRecipeFieldInt);
    CHECK(ClassifyRecipeField("  70  ")      == kRecipeFieldInt);   // trimmed
    CHECK(ClassifyRecipeField("0.20")        == kRecipeFieldFloat);
    CHECK(ClassifyRecipeField("2.00")        == kRecipeFieldFloat);
    CHECK(ClassifyRecipeField("-111.710")    == kRecipeFieldFloat);
    CHECK(ClassifyRecipeField(".5")          == kRecipeFieldFloat);
    CHECK(ClassifyRecipeField("5.")          == kRecipeFieldFloat);
    CHECK(ClassifyRecipeField("")            == kRecipeFieldString); // never null
    CHECK(ClassifyRecipeField("   ")         == kRecipeFieldString);
    CHECK(ClassifyRecipeField("Auto3")       == kRecipeFieldString);
    CHECK(ClassifyRecipeField("0,0,0,0")     == kRecipeFieldString); // bin tables
    CHECK(ClassifyRecipeField("172.16.8.150")== kRecipeFieldString); // two dots
    CHECK(ClassifyRecipeField("1e5")         == kRecipeFieldString); // no exponent
    CHECK(ClassifyRecipeField("$FF")         == kRecipeFieldString); // documented
    CHECK(ClassifyRecipeField("0x10")        == kRecipeFieldString); // documented
    CHECK(ClassifyRecipeField("1.2.3")       == kRecipeFieldString);
    CHECK(ClassifyRecipeField("-")           == kRecipeFieldString);

    // -----------------------------------------------------------------------
    //  2. Exact whole-document output for a small crafted input.
    //     Asserted in full rather than by substring: JsonWriter emits keys in
    //     insertion order with no whitespace, so the whole document is a
    //     stable, readable expectation.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp,
        "[Test Arm1]\r\n"
        "   Pick Up=16.99\r\n"
        "   Up Speed=1\r\n"
        "   EPControl=\r\n"));
    {
        const std::string j = Read(kTmp);
        std::string want = "{\"path\":\"";
        want += kTmp;
        want += "\",\"available\":true,\"sections\":{\"Test Arm1\":{"
                "\"Pick Up\":{\"value\":16.99,\"type\":\"float\",\"raw\":\"16.99\"},"
                "\"Up Speed\":{\"value\":1,\"type\":\"int\",\"raw\":\"1\"},"
                "\"EPControl\":{\"value\":\"\",\"type\":\"string\",\"raw\":\"\"}}}}";
        CHECK_STR(j, want);
    }

    // -----------------------------------------------------------------------
    //  3. `raw` keeps the value VERBATIM: trailing zeros, trailing spaces and
    //     internal spaces all survive, because the write half writes raw back
    //     and must not reformat a field nobody edited.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp,
        "[S]\r\n"
        "  Trailing Zeros=16.9900\r\n"
        "  Padded  =  3.5  \r\n"
        "  Text=a b  c\r\n"));
    {
        const std::string j = Read(kTmp);
        // trailing zeros preserved in raw, normalised in value
        CHECK(Contains(j, "\"Trailing Zeros\":{\"value\":16.99,\"type\":\"float\",\"raw\":\"16.9900\"}"));
        // key trimmed ("Padded"), value NOT trimmed ("  3.5  "), still a float
        CHECK(Contains(j, "\"Padded\":{\"value\":3.5,\"type\":\"float\",\"raw\":\"  3.5  \"}"));
        CHECK(Contains(j, "\"Text\":{\"value\":\"a b  c\",\"type\":\"string\",\"raw\":\"a b  c\"}"));
    }

    // -----------------------------------------------------------------------
    //  4. Grammar edges the store documents: FIRST '=' splits, keys before any
    //     [section] are dropped, ';' and '#' are comments, a section header may
    //     omit its ']'.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp,
        "Orphan=1\r\n"                  // before any section -> dropped
        "[S]\r\n"
        "; comment=1\r\n"
        "# comment=2\r\n"
        "  Eq=a=b=c\r\n"                // first '=' splits
        "[NoBracket\r\n"
        "  K=9\r\n"));
    {
        const std::string j = Read(kTmp);
        CHECK(!Contains(j, "Orphan"));
        CHECK(!Contains(j, "comment"));
        CHECK(Contains(j, "\"Eq\":{\"value\":\"a=b=c\",\"type\":\"string\",\"raw\":\"a=b=c\"}"));
        CHECK(Contains(j, "\"NoBracket\":{"));
    }

    // -----------------------------------------------------------------------
    //  5. FIRST WINS, twice over. Both were learned by being wrong about a
    //     corrupted production recipe; see the header.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp,
        "[All]\r\n"
        "  Speed=60\r\n"
        "  Dup=first\r\n"
        "  Dup=second\r\n"              // duplicate KEY   -> first wins
        "[All]\r\n"
        "  Later=999\r\n"));            // duplicate SECTION -> discarded
    {
        const std::string j = Read(kTmp);
        CHECK(Contains(j, "\"Dup\":{\"value\":\"first\""));
        CHECK(!Contains(j, "second"));
        CHECK(!Contains(j, "Later"));
        CHECK(Contains(j, "\"Speed\":{\"value\":60,\"type\":\"int\",\"raw\":\"60\"}"));
    }

    // -----------------------------------------------------------------------
    //  6. Bare CR terminates a line. The corrupted recipe has 24 of them where
    //     every other file on the machine is pure CRLF, so this is the shape a
    //     damaged file actually takes.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp, "[S]\r  A=1\r  B=2\r"));
    {
        const std::string j = Read(kTmp);
        CHECK(Contains(j, "\"A\":{\"value\":1"));
        CHECK(Contains(j, "\"B\":{\"value\":2"));
    }

    // -----------------------------------------------------------------------
    //  7. A missing file is a state, not an error.
    // -----------------------------------------------------------------------
    std::remove(kGone);
    {
        const std::string j = Read(kGone);
        std::string want = "{\"path\":\"";
        want += kGone;
        want += "\",\"available\":false,\"sections\":{}}";
        CHECK_STR(j, want);
    }

    // -----------------------------------------------------------------------
    //  8. NO-WRITE. The guard that matters: reading must not change the file.
    //     Checked after several reads, including of a file with duplicate
    //     sections -- the case where a write-through store would have most
    //     reason to "tidy up".
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp,
        "[All]\r\n  Speed=60\r\n[All]\r\n  Later=1\r\n"
        "[S]\r  Dup=first\r  Dup=second\r"));
    {
        long  size0 = -1, size1 = -1;
        long long mt0 = -1, mt1 = -1;
        CHECK(StatOf(kTmp, &size0, &mt0));
        for (int i = 0; i < 5; ++i) (void)Read(kTmp);
        CHECK(StatOf(kTmp, &size1, &mt1));
        CHECK(size0 == size1);
        CHECK(mt0 == mt1);
    }

    // =======================================================================
    //  C1-W -- the writer.
    // =======================================================================
    //  A document with everything a re-serialising writer would destroy:
    //  comments, blank lines, uneven indentation, a duplicate section and a
    //  duplicate key. The point of every assertion below is that the bytes
    //  around the edit do not move.
    static const char* kDoc =
        "; a comment that must survive\r\n"
        "\r\n"
        "[Test Arm1]\r\n"
        "   Pick Up=16.99\r\n"
        "      Contact=-111.710\r\n"
        "   Padded=  3.5  \r\n"
        "   Dup=first\r\n"
        "   Dup=second\r\n"            // duplicate key -> only the first is addressable
        "\r\n"
        "# another comment\r\n"
        "[Test Arm1]\r\n"              // duplicate section -> discarded by the store
        "   Orphaned=99\r\n";

    // -----------------------------------------------------------------------
    //  9. Dry run is the default and it writes NOTHING -- not the target, not a
    //     backup, not a temp file. wb_publish/wb_serve are default-dry for the
    //     same reason.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp, kDoc));
    {
        long s0 = -1, s1 = -1; long long m0 = -1, m1 = -1;
        CHECK(StatOf(kTmp, &s0, &m0));

        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("Test Arm1", "Pick Up", "20.00"));
        e.push_back(MakeEdit("Test Arm1", "Contact", "-111.710"));   // identical
        e.push_back(MakeEdit("Test Arm1", "Nope", "1"));             // absent
        e.push_back(MakeEdit("Test Arm1", "Orphaned", "1"));         // discarded block

        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kTmp), e, kRecipeWriteDryRun);
        CHECK(r.ok);
        CHECK(r.changed   == 1);
        CHECK(r.identical == 1);
        CHECK(r.notFound  == 2);      // "Nope" AND "Orphaned"
        CHECK(r.backupPath.empty());
        CHECK(StatOf(kTmp, &s1, &m1));
        CHECK(s0 == s1);
        CHECK(m0 == m1);
        CHECK(Read(kTmp).find("16.99") != std::string::npos);  // untouched
    }

    // -----------------------------------------------------------------------
    // 10. Apply changes exactly one value and NOTHING else. Asserted by
    //     rebuilding the expected file byte-for-byte: comments, blank lines and
    //     the two-space vs six-space indentation all have to come back.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp, kDoc));
    {
        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("Test Arm1", "Pick Up", "20.00"));
        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kTmp), e, kRecipeWriteApply);
        CHECK(r.ok);
        CHECK(r.error.empty());
        CHECK(r.changed == 1);
        CHECK(!r.backupPath.empty());

        std::string want(kDoc);
        const std::string::size_type at = want.find("Pick Up=16.99");
        CHECK(at != std::string::npos);
        want.replace(at, std::strlen("Pick Up=16.99"), "Pick Up=20.00");
        CHECK_STR(Slurp(kTmp), want);

        // the backup is the ORIGINAL, byte for byte
        CHECK_STR(Slurp(r.backupPath.c_str()), std::string(kDoc));
        std::remove(r.backupPath.c_str());
        // no temp file left behind
        CHECK(!FileThere((std::string(kTmp) + ".tmp_webwrite").c_str()));
    }

    // -----------------------------------------------------------------------
    // 11. The FIRST-WINS rules apply to the writer too, and a missing field is
    //     never appended.
    //
    //     Both matter on real data: the corrupted recipe has a discarded [All]
    //     block, and a writer that appended a key would put it where the
    //     machine cannot see it -- a silent no-op that looks like success.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp, kDoc));
    {
        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("Test Arm1", "Dup", "EDITED"));
        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kTmp), e, kRecipeWriteApply);
        CHECK(r.ok);
        CHECK(r.changed == 1);
        const std::string got = Slurp(kTmp);
        CHECK(got.find("Dup=EDITED") != std::string::npos);
        CHECK(got.find("Dup=second") != std::string::npos);   // the second is untouched
        CHECK(got.find("Dup=first")  == std::string::npos);
        if (!r.backupPath.empty()) std::remove(r.backupPath.c_str());
    }
    CHECK(WriteRaw(kTmp, kDoc));
    {
        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("Test Arm1", "Orphaned", "1"));  // in the discarded block
        e.push_back(MakeEdit("No Such Section", "K", "1"));
        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kTmp), e, kRecipeWriteApply);
        CHECK(r.ok);
        CHECK(r.changed  == 0);
        CHECK(r.notFound == 2);
        CHECK(r.backupPath.empty());          // nothing to change -> nothing written
        CHECK_STR(Slurp(kTmp), std::string(kDoc));   // file untouched
    }

    // -----------------------------------------------------------------------
    // 12. Case-insensitive lookup (the store folds ASCII), and the value is
    //     written VERBATIM -- padding included, because the caller owns
    //     formatting and a field's original spacing is part of `raw`.
    // -----------------------------------------------------------------------
    CHECK(WriteRaw(kTmp, kDoc));
    {
        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("test arm1", "padded", "  9.5  "));   // both folded
        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kTmp), e, kRecipeWriteApply);
        CHECK(r.ok);
        CHECK(r.changed == 1);
        CHECK(Slurp(kTmp).find("Padded=  9.5  \r\n") != std::string::npos);
        if (!r.backupPath.empty()) std::remove(r.backupPath.c_str());
    }

    // -----------------------------------------------------------------------
    // 13. An unreadable path is an error, not a crash and not a silent success.
    // -----------------------------------------------------------------------
    {
        std::vector<RecipeFieldEdit> e;
        e.push_back(MakeEdit("S", "K", "1"));
        const RecipeWriteResult r =
            RecipeDocApplyEdits(vclcompat::AnsiString(kGone), e, kRecipeWriteApply);
        CHECK(!r.ok);
        CHECK(!r.error.empty());
        CHECK(r.backupPath.empty());
    }

    std::remove(kTmp);

    std::printf("%s: %d/%d checks passed\n",
                g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
