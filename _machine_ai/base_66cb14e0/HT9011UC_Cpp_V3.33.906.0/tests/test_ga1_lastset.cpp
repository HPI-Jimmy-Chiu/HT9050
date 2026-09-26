// ===========================================================================
//  tests/test_ga1_lastset.cpp
//  W906-GA1-B1 gate: proves the full-volume LastSet.h/.cpp translation
//  compiles STANDALONE and preserves the golden struct-family layout.
//
//  #includes ONLY "LastSet.h" -- deliberately NOT canary_support.h.
//  canary_support.h currently carries its own MINIMAL `struct LAST_GENERAL_SET`
//  shim (66 fields) used by 123 other TUs; that shim and this file's full
//  ~390-field struct share the same name, so #including both in one TU is an
//  ODR collision. This TU (and the LastSet.cpp it links) must stay isolated
//  from canary_support.h/.cpp until a future integration wave retires the
//  shim -- see _ga1_b1_report.md.
//
//  What this proves:
//    (1) sizeof(LAST_GENERAL_SET) is non-trivial (the struct is not silently
//        collapsed by a translation mistake).
//    (2) 15 representative LAST_GENERAL_SET fields -- first/middle/last field
//        plus at least one of every member type in the struct (char[], int,
//        long, float, double, bool, unsigned int, and 2D/3D array forms of
//        each) -- have STRICTLY INCREASING offsetof(), i.e. golden's
//        declaration order survived the translation untouched. Modeled on
//        the PROD_INFO_ST anchor pattern in tests/test_globals.cpp.
//    (3) The sibling structs in the same file (TECH / INFO / VPoint /
//        TeachInfo / ALIGNTEACH) also compile and have sane first/last-field
//        offsets.
//    (4) The four `extern` globals LastSet.h declares (LastSet / Tech /
//        CmdData / AlignTeach) link against the definitions in LastSet.cpp
//        and are read/write accessible at runtime.
//
//  LIMITATION: no Borland binary available -- byte-for-byte BCB6 struct
//  layout (padding/alignment) is NOT asserted, only golden's DECLARATION
//  ORDER (monotonically increasing offsetof), exactly as test_globals.cpp
//  documents for PROD_INFO_ST.
//
//  Toolchain: MinGW g++ 6.3+, C++17.  Non-zero exit on any failure.
// ===========================================================================
#include "LastSet.h"

#include <cstddef>   // offsetof
#include <cstdio>

// ---------------------------------------------------------------------------
//  (1) sizeof sanity -- LAST_GENERAL_SET is golden's ~390-field master
//      persisted-state struct; guard against an accidental truncation
//      collapsing it to a small size.  (46 unsigned int BinCT[4][256] alone
//      is 4*256*4 = 4096 bytes, so a faithful struct is comfortably >20000.)
// ---------------------------------------------------------------------------
static_assert(sizeof(LAST_GENERAL_SET) > 20000,
              "LAST_GENERAL_SET sanity: must remain the large master persisted-state struct");

// ---------------------------------------------------------------------------
//  (2) 15 representative field anchors, golden declaration order preserved.
//      Field index (0-based, in golden/ported declaration order) noted per
//      line; indices taken from a full field-order dump of the 387 live
//      LAST_GENERAL_SET members (see _ga1_b1_report.md field-count reconciliation).
// ---------------------------------------------------------------------------
//   idx 0   -- first field: char LastOpenFilename[512]  -> offset 0
static_assert(offsetof(LAST_GENERAL_SET, LastOpenFilename) == 0,
              "LastOpenFilename must be the first LAST_GENERAL_SET field (offset 0)");
//   idx 20  -- long SystemAccSecond[4][8]
static_assert(offsetof(LAST_GENERAL_SET, LastOpenFilename) <
              offsetof(LAST_GENERAL_SET, SystemAccSecond),
              "SystemAccSecond must follow LastOpenFilename");
//   idx 23  -- bool bMusicEnable
static_assert(offsetof(LAST_GENERAL_SET, SystemAccSecond) <
              offsetof(LAST_GENERAL_SET, bMusicEnable),
              "bMusicEnable must follow SystemAccSecond");
//   idx 33  -- int iRealDummy  (also the field the canary_support.h shim carries)
static_assert(offsetof(LAST_GENERAL_SET, bMusicEnable) <
              offsetof(LAST_GENERAL_SET, iRealDummy),
              "iRealDummy must follow bMusicEnable");
//   idx 46  -- unsigned int BinCT[4][256]
static_assert(offsetof(LAST_GENERAL_SET, iRealDummy) <
              offsetof(LAST_GENERAL_SET, BinCT),
              "BinCT must follow iRealDummy");
//   idx 88  -- int HeadTestCT[192]
static_assert(offsetof(LAST_GENERAL_SET, BinCT) <
              offsetof(LAST_GENERAL_SET, HeadTestCT),
              "HeadTestCT must follow BinCT");
//   idx 117 -- int iJamCount[3]
static_assert(offsetof(LAST_GENERAL_SET, HeadTestCT) <
              offsetof(LAST_GENERAL_SET, iJamCount),
              "iJamCount must follow HeadTestCT");
//   idx 158 -- int iInArmVacuumDummyOnTime[4][8]
static_assert(offsetof(LAST_GENERAL_SET, iJamCount) <
              offsetof(LAST_GENERAL_SET, iInArmVacuumDummyOnTime),
              "iInArmVacuumDummyOnTime must follow iJamCount");
//   idx 170 -- float fTempHistroy[20][60]  (golden's own "Histroy" typo, kept verbatim)
static_assert(offsetof(LAST_GENERAL_SET, iInArmVacuumDummyOnTime) <
              offsetof(LAST_GENERAL_SET, fTempHistroy),
              "fTempHistroy must follow iInArmVacuumDummyOnTime");
//   idx 192 -- float fEPrefdata[100]
static_assert(offsetof(LAST_GENERAL_SET, fTempHistroy) <
              offsetof(LAST_GENERAL_SET, fEPrefdata),
              "fEPrefdata must follow fTempHistroy");
//   idx 213 -- bool bUseTestSocket__[4][8]
static_assert(offsetof(LAST_GENERAL_SET, fEPrefdata) <
              offsetof(LAST_GENERAL_SET, bUseTestSocket__),
              "bUseTestSocket__ must follow fEPrefdata");
//   idx 270 -- double ____dTempHistroy[40][60]  (quirk: FOUR leading underscores, dead name)
static_assert(offsetof(LAST_GENERAL_SET, bUseTestSocket__) <
              offsetof(LAST_GENERAL_SET, ____dTempHistroy),
              "____dTempHistroy must follow bUseTestSocket__");
//   idx 298 -- double dTempHistroy[100][60]  (the live/current generation of the same field)
static_assert(offsetof(LAST_GENERAL_SET, ____dTempHistroy) <
              offsetof(LAST_GENERAL_SET, dTempHistroy),
              "dTempHistroy must follow ____dTempHistroy");
//   idx 333 -- unsigned int iSiteTotalCTForAlways[4][8]
static_assert(offsetof(LAST_GENERAL_SET, dTempHistroy) <
              offsetof(LAST_GENERAL_SET, iSiteTotalCTForAlways),
              "iSiteTotalCTForAlways must follow dTempHistroy");
//   idx 386 -- last field: bool bAutoHadIC_ATK_Backup[3]
static_assert(offsetof(LAST_GENERAL_SET, iSiteTotalCTForAlways) <
              offsetof(LAST_GENERAL_SET, bAutoHadIC_ATK_Backup),
              "bAutoHadIC_ATK_Backup must follow iSiteTotalCTForAlways");
// NOTE: deliberately NOT asserting offsetof(bAutoHadIC_ATK_Backup) == sizeof(...) - sizeof(member).
// The struct's tail may carry alignment padding after the last bool[3] member (e.g. to the
// alignment of its widest double member), so that equality is not guaranteed and is not part
// of the fidelity contract (only DECLARATION ORDER is asserted here, per test_globals.cpp's
// PROD_INFO_ST precedent). offsetof(bAutoHadIC_ATK_Backup) < sizeof(LAST_GENERAL_SET) always
// holds trivially and is not worth a separate assertion.

// ---------------------------------------------------------------------------
//  (3) Sibling structs in the same file: compile + sane first/last offsets.
// ---------------------------------------------------------------------------
static_assert(sizeof(TECH) > 1000, "TECH sanity: ~290-field teach-position struct");
static_assert(offsetof(TECH, iZLoad) == 0, "TECH.iZLoad must be first (offset 0)");
static_assert(offsetof(TECH, iZLoad) < offsetof(TECH, iOutArmToSortShtPlaceZ),
              "TECH.iOutArmToSortShtPlaceZ (last field) must follow iZLoad (first field)");

static_assert(sizeof(INFO) > 0, "INFO sanity");
static_assert(offsetof(INFO, bCommandList) == 0, "INFO.bCommandList must be first (offset 0)");
static_assert(offsetof(INFO, bCommandList) < offsetof(INFO, cVersion),
              "INFO.cVersion (last field) must follow bCommandList (first field)");

static_assert(sizeof(VPoint) == 6 * sizeof(int), "VPoint = 6 named ints, no padding surprises expected");
static_assert(sizeof(ALIGNTEACH) > sizeof(TeachInfo), "ALIGNTEACH holds TeachInfo IO[2] + iAccuracy");

// ---------------------------------------------------------------------------
//  Runtime checks
// ---------------------------------------------------------------------------
static int g_fail  = 0;
static int g_total = 0;

#define CHECK(cond, msg)                                                     \
    do {                                                                     \
        ++g_total;                                                           \
        if (!(cond)) {                                                       \
            ++g_fail;                                                        \
            std::printf("FAIL [%s:%d] %s\n", __FILE__, __LINE__, msg);       \
        }                                                                    \
    } while (0)

int main()
{
    // ---- (4) extern global linkage: LastSet / Tech / CmdData / AlignTeach ----
    // LastSet / Tech / AlignTeach are defined (not just declared) in LastSet.cpp;
    // this TU only declares them via the header's `extern`.  Successfully
    // reading/writing them here without a linker error IS the link-time proof.
    LastSet.iRealDummy = 42;
    CHECK(LastSet.iRealDummy == 42, "LastSet.iRealDummy read/write roundtrip");

    LastSet.BinCT[0][0] = 7u;
    LastSet.BinCT[3][255] = 9u;
    CHECK(LastSet.BinCT[0][0] == 7u && LastSet.BinCT[3][255] == 9u,
          "LastSet.BinCT[4][256] read/write at both ends");

    LastSet.dTempHistroy[0][0] = 1.5;
    LastSet.dTempHistroy[99][59] = 2.5;
    CHECK(LastSet.dTempHistroy[0][0] == 1.5 && LastSet.dTempHistroy[99][59] == 2.5,
          "LastSet.dTempHistroy[100][60] read/write at both ends");

    LastSet.bAutoHadIC_ATK_Backup[2] = true;
    CHECK(LastSet.bAutoHadIC_ATK_Backup[2] == true,
          "LastSet.bAutoHadIC_ATK_Backup (last field) read/write");

    Tech.iZLoad = 100;
    Tech.iOutArmToSortShtPlaceZ = 200;
    CHECK(Tech.iZLoad == 100 && Tech.iOutArmToSortShtPlaceZ == 200,
          "Tech first/last field read/write");

    CHECK(CmdData == 0, "CmdData starts as a null pointer (golden never allocates it in LastSet.cpp)");

    AlignTeach.iAccuracy = 5;
    AlignTeach.IO[0].dtPixelSizeX = 3.14;
    AlignTeach.IO[1].vtMark[19].iOriX = 123;   // TeachInfo::vtMark[20] last element, IO[2] last element
    CHECK(AlignTeach.iAccuracy == 5 &&
          AlignTeach.IO[0].dtPixelSizeX == 3.14 &&
          AlignTeach.IO[1].vtMark[19].iOriX == 123,
          "AlignTeach nested TeachInfo/VPoint read/write");

    // ---- eCommandList enum sanity (defined between INFO and its own struct) ----
    CHECK(eSetSiteMap == 0 && eCommandTotal == 18,
          "eCommandList enum: eSetSiteMap==0 first, eCommandTotal==18 sentinel");

    if (g_fail == 0) {
        std::printf("PASS  %d/%d tests passed (test_ga1_lastset)\n", g_total, g_total);
        return 0;
    }
    std::printf("FAIL  %d/%d tests failed (test_ga1_lastset)\n", g_fail, g_total);
    return 1;
}
