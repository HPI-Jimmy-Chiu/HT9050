// =============================================================================
//  test_tb_inspect.cpp -- Q32 (S69): AOI/TopBottomInspect.* -- golden TTopBottomInspect's AOI.Data parameters as data.
//
//  AI(W906-Q32-S69) 20260927 (St02).  Suite name (add_test): AOI_TopBottomInspectParams
//
//    1. key count 225 (golden 906_0625_Steven fAOI.cpp:182-263, loops expanded) and no 912 key (AI(W906-ST02-C912) 20261003 (St02-E helper));
//    2. every (section, key) is unique, and every key points at exactly one field;
//    3. golden defaults (spot values from :185-236, and every check box false, every list count 0);
//    4. the key names golden builds with sprintf ("Small_1_RB", "Large_3_PosX", "Consecutive Fail Count(Picture)_19")
//       and none of 912's FailStop_<code> / iAutoRetryCount / sRecipeName keys;
//    5. a key's pointer really is that field (write through the table, read the field);
//    6. SetGoldenDefaults restores after changes.
//  No file is read or written.
// =============================================================================
#include "AOI/TopBottomInspect.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static const aoi::TbKey* Find(const std::vector<aoi::TbKey>& v, const char* sec, const char* key)
{
    for (size_t i = 0; i < v.size(); ++i)
        if (std::strcmp(v[i].section, sec) == 0 && v[i].key == key) return &v[i];
    return 0;
}

int main()
{
    printf("AOI_TopBottomInspectParams\n");
    aoi::TopBottomInspectParams p;
    const std::vector<aoi::TbKey> k906 = aoi::TopBottomInspectKeys(p);   // AI(W906-ST02-C912) 20261003 (St02-E helper): 906 only, the with912 flag is gone

    // 1
    CHECK(k906.size() == 225, "1. 906: 225 keys (19 + 2x6 + 4x6 + 10 + 20x8)");
    CHECK(Find(k906, "TopBottomInspect", "iAutoRetryCount") == 0 && Find(k906, "TopBottomInspect", "sRecipeName") == 0,
          "1. no 912 key: iAutoRetryCount / sRecipeName (912 fAOI.cpp:237-238) gone, RULINGS_20261002 #20");

    // 2
    std::set<std::string> names;
    std::set<const void*> ptrs;
    bool onePtr = true;
    for (size_t i = 0; i < k906.size(); ++i) {
        names.insert(std::string(k906[i].section) + "|" + k906[i].key);
        const int set = (k906[i].pi ? 1 : 0) + (k906[i].pb ? 1 : 0) + (k906[i].ps ? 1 : 0);
        if (set != 1) onePtr = false;
        ptrs.insert(k906[i].pi ? (const void*)k906[i].pi : k906[i].pb ? (const void*)k906[i].pb : (const void*)k906[i].ps);
    }
    CHECK(names.size() == k906.size(), "2. every (section, key) is unique");
    CHECK(onePtr, "2. every key points at exactly one field");
    CHECK(ptrs.size() == k906.size(), "2. no two keys share a field");

    // 3
    CHECK(p.iEnable == 0 && p.iAction == 0, "3. iEnable / iAction 0 (:182-183)");
    CHECK(p.sSocketAddress == "172.16.8.200" && p.sSocketPort == "5109" && p.sCamaName == "CM1", "3. socket 172.16.8.200:5109, camera CM1 (:185-187)");
    CHECK(p.iStartDelayTime == 5 && p.iGetResultDelay == 200 && p.iTimeout == 10, "3. delays 5 / 200, timeout 10 (:188-190)");
    CHECK(p.iRotate0Pos == -20 && p.iRotate180Pos == 3230, "3. rotate 0 / 180 = -20 / 3230 (:191-192)");
    CHECK(p.tbCenter.ppPosition.X == 977 && p.tbCenter.ppPosition.Y == 80, "3. center 977 / 80 (:193-194)");
    CHECK(p.iAOIFailSetBin == 15, "3. AOI Fail Set Bin 15 (:236)");
    bool allFalse = true, allZeroCount = true;
    for (size_t i = 0; i < k906.size(); ++i) {
        if (k906[i].pb && *k906[i].pb) allFalse = false;
        if (k906[i].pi && std::strcmp(k906[i].section, "Function Setting") == 0 && k906[i].key != "AOI Fail Set Bin" && *k906[i].pi != 0)
            allZeroCount = false;
    }
    CHECK(allFalse, "3. every check box defaults to false (golden default \"\" = 0; checked only when exactly 1)");
    CHECK(allZeroCount, "3. every Function Setting count except AOI Fail Set Bin defaults to 0");

    // 4
    CHECK(Find(k906, "TopBottomInspect", "Small_1_RB") && Find(k906, "TopBottomInspect", "Large_3_PosX"), "4. Small_1_RB / Large_3_PosX");
    CHECK(Find(k906, "Function Setting", "Consecutive Fail Count(Picture)_19") &&
          Find(k906, "Function Setting", "Enable Interval Check_0"), "4. list keys _0 .. _19");
    CHECK(Find(k906, "ZPickOffset", "iZPickOffset") != 0, "4. iZPickOffset is in [ZPickOffset] (:203)");
    CHECK(Find(k906, "TopBottomInspect", "FailStop_MB") == 0 && Find(k906, "TopBottomInspect", "FailStop_CM") == 0,
          "4. no FailStop_ key (912 fAOI.cpp:316-319 only)");
    const aoi::TbKey* en = Find(k906, "TopBottomInspect", "iEnable");
    CHECK(en && en->kind == 'r', "4. iEnable is a radio group (ReadInteger)");

    // 5
    const aoi::TbKey* k = Find(k906, "TopBottomInspect", "Large_2_LB");
    if (k && k->pb) *k->pb = true;
    CHECK(p.tbpiMoreThan65mm[2].bClampLB, "5. Large_2_LB -> tbpiMoreThan65mm[2].bClampLB");
    k = Find(k906, "Function Setting", "Accumulated Fail Count_7");
    if (k && k->pi) *k->pi = 42;
    CHECK(p.iAccumulatedFailCount_List[7] == 42, "5. Accumulated Fail Count_7 -> iAccumulatedFailCount_List[7]");

    // 6
    p.sSocketPort = "1"; p.iRotate180Pos = 1;
    p.SetGoldenDefaults();
    CHECK(p.sSocketPort == "5109" && p.iRotate180Pos == 3230 && !p.tbpiMoreThan65mm[2].bClampLB &&
          p.iAccumulatedFailCount_List[7] == 0, "6. SetGoldenDefaults restores every field");

    printf("AOI_TopBottomInspectParams: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
