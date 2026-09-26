// =============================================================================
//  test_contactforce_load.cpp -- P2a VERIFY: ContactForce 的驅動層真的把
//                                 `ContactForceTables()` 的四張表填起來了。
//
//  Wave: AI(W906-P2a-CF) 20260919.  Suite name (add_test): ContactForceLoad
//
//  這支測試存在的理由
//  ------------------
//  20260919 量到：`ContactForceTables()` 與四支 `Load*SlkTable()` 全樹呼叫點
//  都是 **0**。計算核心（ContactForce.cpp）翻得很完整，但沒有任何人去讀
//  `ContactInfo.ini` 並呼叫它們 —— 四張表永遠空著、`bLoaded` 永遠 false。
//  `ContactForceLoad.cpp` 是補上的那一層；這支測試證明它接上了。
//
//  ⚠⚠ 刻意**不釘死任何機台組態值**
//  ---------------------------------------------------------------------------
//  `D:\HT9045\system\ContactInfo.ini` 是**這一台機器的**檔案。
//  週末計畫 P8 記的缺陷就是「IniFiles 測試釘死 5 個機台組態值 -> 換台機器必紅」。
//  所以這裡斷言的全部是**結構與不變式**，不是值：
//    * 筆數 == Type CSV 裡通過 golden `>15.0` 濾網的 token 數
//      （CSV 由測試自己重讀，不是寫死的數字）
//    * 每一筆的值落在 golden 的 CheckRange 夾限內
//    * 冪等：連呼叫兩次得到一樣的筆數（證明 Clear() 有效）
//    * `bUseDynamicKitDiameter==false` -> 回 0 且四張表全空（golden :430）
//  這樣換一台機器、換一份 ContactInfo.ini，這支測試照樣成立。
//
//  等價性說明：沒有 Borland 二進位可比，所以「等價」== 乾淨編譯連結 ＋
//  上面那組不變式與 golden 原始碼（ContactForce.cpp:421-652 / :963-1178）
//  手推的規則相符。
// =============================================================================
#include "ContactForceLoad.h"
#include "ContactForce.h"
#include "common.h"                  // CheckAndReadIniData
#include "CosFunction.h"             // CosFunction
#include "cmydef.h"                  // EP_Install
#include "MachineType.h"             // CUSTOMER_CODE / CC_ASE_SG
#include "vclcompat/SysUtils.h"      // FileExists

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static const char* kIni = "D:\\HT9045\\system\\ContactInfo.ini";

//  golden ContactForce.cpp:465 的濾網，測試自己重算一次 ——
//  不是去問被測物「你載了幾筆」，而是獨立算出「應該幾筆」。
static int ExpectedSlkCount(const std::string& csv, int iEpInstall, bool bAseSg)
{
    const std::vector<std::string> toks = SlkSplitCommaText(csv);
    int n = 0;
    for (size_t i = 0; i < toks.size(); ++i) {
        if (toks[i].empty())                        continue;
        if (std::atof(toks[i].c_str()) <= 15.0)     continue;
        if (toks[i] == "402")        { n += 1; }     // golden :467
        else if (iEpInstall == 5)    { n += 2; }     // golden :471  Arm1_ + Arm2_
        else                         { n += 1; }
        if (bAseSg && toks[i] == "80") n += 1;       // golden :483  80_Hi
    }
    return n;
}

int main()
{
    printf("==== P2a ContactForceLoad: 驅動層把四張表填起來了嗎 ====\n");

    const bool bHasFile = FileExists(AnsiString(kIni));
    printf("   %s : %s\n", kIni, bHasFile ? "存在" : "不存在");

    // ---- (a) golden :430 的前置條件 ----------------------------------------
    const bool savedDyn = CosFunction.bUseDynamicKitDiameter;
    CosFunction.bUseDynamicKitDiameter = false;
    {
        const int n = LoadContactForceTables();
        SlkForceTables& t = ContactForceTables();
        CHECK(n == 0, "a1 bUseDynamicKitDiameter==false -> 回 0 (golden :430)");
        CHECK(t.SLKClass.empty() && t.SLKIndClass.empty() &&
              t.DieForceSLKClass.empty() && t.DieForceOneByOneSLKClass.empty(),
              "a2 bUseDynamicKitDiameter==false -> 四張表全空");
        CHECK(t.SLKClass.bLoaded == false,
              "a3 bLoaded 沒有被亂設成 true（呼叫端要分得出「沒載」）");
    }

    // ---- (b) 真的載一次 ----------------------------------------------------
    CosFunction.bUseDynamicKitDiameter = true;
    const int n1 = LoadContactForceTables();
    {
        SlkForceTables& t = ContactForceTables();
        printf("   載入筆數: SLK=%u  Ind=%u  DieForce=%u  DieForce1by1=%u  (回傳 %d)\n",
               (unsigned)t.SLKClass.size(), (unsigned)t.SLKIndClass.size(),
               (unsigned)t.DieForceSLKClass.size(),
               (unsigned)t.DieForceOneByOneSLKClass.size(), n1);

        if (!bHasFile) {
            printf("   （ContactInfo.ini 不存在，b/c/d 段跳過；a 段已足以證明前置條件）\n");
        } else {
            //  測試自己重讀 CSV，獨立算出應該幾筆。
            const AnsiString sType =
                CheckAndReadIniData(AnsiString(kIni), AnsiString("SLK Type"),
                                    AnsiString("Type"),
                                    AnsiString(SlkDefaultTypeCsv(false)));
            const int expect = ExpectedSlkCount(std::string(sType.c_str()),
                                                EP_Install,
                                                CUSTOMER_CODE == CC_ASE_SG);
            printf("   [SLK Type] Type=\"%s\" -> 依 golden :465/:467/:471/:483 應為 %d 筆\n",
                   sType.c_str(), expect);
            CHECK((int)ContactForceTables().SLKClass.size() == expect,
                  "b1 SLKClass 筆數 == Type CSV 通過 >15.0 濾網的 token 數");
            CHECK(n1 > 0, "b2 回傳的總筆數 > 0（表真的被填了）");
            CHECK(t.SLKClass.bLoaded, "b3 SLKClass.bLoaded 變成 true");
            CHECK(t.SLKIndClass.bLoaded, "b4 SLKIndClass.bLoaded 變成 true");

            // ---- (c) 值落在 golden 的 CheckRange 夾限內（不是釘死值）-------
            bool bRangeOk = true;
            for (size_t i = 0; i < t.SLKClass.items.size(); ++i) {
                const SlkForceData& e = t.SLKClass.items[i];
                if (e.dLoadRate    < 0.8  || e.dLoadRate    > 1.5)  bRangeOk = false;
                if (e.dLoadRate_NS < 0.8  || e.dLoadRate_NS > 1.5)  bRangeOk = false;
                if (e.dHotOffset   < -0.5 || e.dHotOffset   > 0.5)  bRangeOk = false;
                if (e.dContactOffset    < -10.0 || e.dContactOffset    > 10.0) bRangeOk = false;
                if (e.dContactOffset_NS < -10.0 || e.dContactOffset_NS > 10.0) bRangeOk = false;
            }
            CHECK(bRangeOk,
                  "c1 每一筆都落在 golden CheckRange 的夾限內 "
                  "(LoadRate 0.8~1.5 / HotOffset ±0.5 / ContactOffset ±10)");

            //  ⓘ 至少有一筆的 LoadRate 不是建構預設 1.0 —— 證明 ini 真的被讀了，
            //    而不是「四張表建好了但每一筆都還是預設值」。
            //    這是「綠燈不等於接上了」的解藥：只看筆數的話，一個什麼都沒讀
            //    的實作也會過。
            bool bAnyNonDefault = false;
            for (size_t i = 0; i < t.SLKClass.items.size(); ++i) {
                const SlkForceData& e = t.SLKClass.items[i];
                if (e.dLoadRate != 1.0 || e.dLoadRate_NS != 1.0 ||
                    e.dContactOffset != 0.0) { bAnyNonDefault = true; break; }
            }
            CHECK(bAnyNonDefault,
                  "c2 至少一筆的值不是建構預設 -> ini 真的被讀進來了（不是空殼）");
        }
    }

    // ---- (d) 冪等 -----------------------------------------------------------
    {
        const size_t a = ContactForceTables().SLKClass.size();
        const int n2 = LoadContactForceTables();
        const size_t b = ContactForceTables().SLKClass.size();
        CHECK(a == b && n1 == n2,
              "d1 連呼叫兩次筆數不變（Clear() 有效，不會累加）");
    }

    CosFunction.bUseDynamicKitDiameter = savedDyn;

    printf("\n==== P2a ContactForceLoad summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
