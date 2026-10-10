// =============================================================================
//  test_l07_datamodel.cpp -- AI(W906-L07) 20261010 (Ifor01)
//
//  L07 (W-190 5, TO_IFOR 1009 20:4x: the data model first, then St02-E regenerates, then the logic) -- golden 913 JerryYang
//  20260917 ATC second pre-compensation, data only:
//    [1] SYSTEM_TEMPERATURE::fTempOffSet has 23 rows (golden 913 cprod.h:1388; was 19) x tcTotalCount
//    [2] the second group d2ndATCPreOffset[32] / i2ndATCPreOfsTime[32] / d2ndATCAfterOfs[32] (cprod.h:1626-1628), right after the
//        first group dATCPreOffset / iATCPreOfsTime / dATCAfterOfs and before bPowerFollower_Enable, as in golden
//    [3] the row constants PreOffset=17 .. AfterOffset2nd=22 (golden 913 uTemp_Set.cpp:65-70) fit the 23 rows, after SHigBase=16
//    [4] iATCRemoteChangeTempCnt (golden 913 cmydef.cpp:5291) starts at 0; the new arrays start zeroed (Temperature is a global)
// =============================================================================
#include "MachineDefine.h"
#include "cprod.h"
#include "cmydef.h"
#include "forms/fTemp_Set.h"
#include <cstddef>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }

int main()
{
    std::printf("L07_AtcPreOffsetDataModel\n");
    const size_t rows = sizeof(Temperature.fTempOffSet) / sizeof(Temperature.fTempOffSet[0]);
    const size_t cols = sizeof(Temperature.fTempOffSet[0]) / sizeof(Temperature.fTempOffSet[0][0]);
    Check(rows == 23 && cols == (size_t)tcTotalCount, "1. fTempOffSet is [23][tcTotalCount] (golden 913 cprod.h:1388)");

    Check(sizeof(Temperature.d2ndATCPreOffset) == 32 * sizeof(double) && sizeof(Temperature.i2ndATCPreOfsTime) == 32 * sizeof(int)
          && sizeof(Temperature.d2ndATCAfterOfs) == 32 * sizeof(double), "2a. second group: double[32] / int[32] / double[32] (golden 913 cprod.h:1626-1628)");
    Check(offsetof(SYSTEM_TEMPERATURE, dATCAfterOfs) < offsetof(SYSTEM_TEMPERATURE, d2ndATCPreOffset)
          && offsetof(SYSTEM_TEMPERATURE, d2ndATCPreOffset) < offsetof(SYSTEM_TEMPERATURE, i2ndATCPreOfsTime)
          && offsetof(SYSTEM_TEMPERATURE, i2ndATCPreOfsTime) < offsetof(SYSTEM_TEMPERATURE, d2ndATCAfterOfs)
          && offsetof(SYSTEM_TEMPERATURE, d2ndATCAfterOfs) < offsetof(SYSTEM_TEMPERATURE, bPowerFollower_Enable),
          "2b. golden order: first group, then d2ndATCPreOffset / i2ndATCPreOfsTime / d2ndATCAfterOfs, then bPowerFollower_Enable");

    Check(PreOffset == 17 && PreOffsetTime == 18 && AfterOffset == 19 && PreOffset2nd == 20 && PreOffsetTime2nd == 21 && AfterOffset2nd == 22,
          "3a. row constants 17..22 (golden 913 uTemp_Set.cpp:65-70)");
    Check(SHigBase == 16 && SHigBase < PreOffset && (size_t)AfterOffset2nd < rows, "3b. they follow SHigBase=16 and fit the 23 rows");

    bool zero = iATCRemoteChangeTempCnt == 0;
    for (int i = 0; i < 32; i++)
        zero = zero && Temperature.d2ndATCPreOffset[i] == 0.0 && Temperature.i2ndATCPreOfsTime[i] == 0 && Temperature.d2ndATCAfterOfs[i] == 0.0;
    for (int r = 19; r < 23; r++)
        for (int c = 0; c < tcTotalCount; c++)
            zero = zero && Temperature.fTempOffSet[r][c] == 0.0;
    Check(zero, "4. iATCRemoteChangeTempCnt starts 0; the second group and the new rows 19..22 start zeroed");

    std::printf("L07_AtcPreOffsetDataModel: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
