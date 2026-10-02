// ===========================================================================
//  tests/test_jsonbridge_s7.cpp
//  JSON 橋接層 S2～S7 的**離線**不變量：結構欄位表 ＋ 溫控通道 producer。
//
//  AI(W906-JSONBRIDGE-S7-TESTS) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 4.1 / 4.8
//
//  ---------------------------------------------------------------------------
//  為什麼要有這一支（它補的是一個真的洞，不是湊覆蓋率）
//  ---------------------------------------------------------------------------
//  S7 的 213 個 temp.zone.* tag **全部必須是 null**，理由寫在
//  JsonBridge/StageThermo.h 的檔頭：三個資料源今天都不活，而 0 在這棵樹上是
//  一個合法的攝氏溫度。守住這條的原本只有
//  tools/webprobe/s7_thermo_probe.py —— 那支要人工先起一個 wb_serve，
//  所以它**不在 ctest 裡**。等於「把 null 換成 0 讓畫面好看」這種改動，
//  在 build.bat gate 全綠的情況下可以完全不被發現。
//
//  這一支把那些不變量搬成純離線斷言：不開 socket、不讀 ini、不碰任何
//  量產共用檔（所以它也不需要 --dry；它根本沒有可以踩到的路徑）。
//
//  ---------------------------------------------------------------------------
//  它守什麼，以及**為什麼那一條值得守**
//  ---------------------------------------------------------------------------
//  1. 六張型別表的欄位數（796/66/223/44/5/1，合計 1,135；AI(W906-TIF912-R10) 20260926：原 788／1,127，+5 是第 10 條、+3 是 TIF912 的補產）。
//     788 那個數字有歷史：反向大括號配對咬到 cprod.h:2156 註解裡的 `{`，
//     曾把 SYSTEM_TEST_IF 量成 351 —— 漏 436 欄，而且無聲。那個誤判寫進過
//     SKILL.md、commit message 與一封已寄出的信。這裡釘住它。
//     ⚠ 這些數字是**產生器輸出的欄位數**；它跟結構真正有幾個成員相不相等，
//       靠的是重跑 tools/gen_sjson.py。結構改了而沒重跑，這裡會紅 ——
//       那正是要的：紅在編譯機上，不要紅在畫面上。
//
//  2. 每一個欄位 offset + n*elemSize <= structSize。
//     這是整個 FieldDesc 設計裡唯一會造成「值看起來完全合理但其實是隔壁欄位
//     的位元組」的失效模式，而那種缺陷在畫面上分辨不出來。
//     ⚠ 三維陣列（kFieldUnsupported）的 dim 只記到前兩維，所以這裡算出來的
//       n*elemSize 是**下界**不是實際大小。故意的：寧可少算也不要假失敗，
//       而「連下界都越界」才是真的壞掉。
//
//  3. kThermoFields 每一欄的 live 今天都是 false，而且每一欄都給得出 why。
//     ⚠ live 變 true 的那一天，是有人把溫控讀取迴圈接進 wb_serve 了。
//       那時**要改的是這支測試**（連同 PublishThermoTags 的 null 斷言），
//       不是把斷言刪掉。這一條刻意會擋路。
//
//  4. kThermoChannels 的 idx 唯一、連續、落在 tcTotalCount 內。
//     enum 中間插一個通道而產生檔沒重產 —— 錯位的溫度通道在畫面上看起來
//     完全正常。gen 檔裡的 static_assert 只擋得住「總數變了」，擋不住
//     「總數一樣但順序換了」。這一條擋後者。
//
//  5. PublishThermoTags 離線跑一次：213 個 tag、每一個都是 null、
//     ready/state/sv/overAny **不在**串流裡。
//     這是 s7_thermo_probe.py 那條最重要的斷言的離線版本。
//
//  Build: 只 link vclcompat ＋ ht9045_webbridge。**刻意不 link god-stack** ——
//  這一支驗的是表本身（sizeof/offsetof，編譯期事實），不是任何實例的值，
//  所以它不需要 Bindings.cpp，也就不需要 DeviceForm_File 那批全域。
//  Non-zero exit on any failure.
// ===========================================================================
#include "JsonBridge/FieldDesc.h"
#include "JsonBridge/StageThermo.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "MachineType.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <string>

static int g_fail = 0;
static int g_pass = 0;

static void check(bool cond, const char* what, const char* file, int line) {
    if (cond) {
        ++g_pass;
    } else {
        std::printf("FAIL %s:%d  %s\n", file, line, what);
        ++g_fail;
    }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

namespace ht9045 {
namespace sjson {
// 這六張表定義在 gen/sjson_*.gen.cpp，宣告原本只在 Bindings.cpp 裡（那個 TU
// 引用 DeviceForm_File 等全域，會把 god-stack 拖進來）。這裡自己宣告，
// 讓這支測試維持在「只看表」的範圍。
extern const TypeDesc kType_SYSTEM_DEVICE_FORM;
extern const TypeDesc kType_LAST_LEVEL_SET;
extern const TypeDesc kType_SYSTEM_TEST_IF;
extern const TypeDesc kType_SYSTEM_TEMPERATURE;
extern const TypeDesc kType_SYSTEM_TRAY_FORM;
extern const TypeDesc kType_SYSTEM_TEST_MODE;
}  // namespace sjson
}  // namespace ht9045

using ht9045::sjson::FieldDesc;
using ht9045::sjson::TypeDesc;
using ht9045::sjson::ThermoChannel;
using ht9045::sjson::ThermoFieldLiveness;

namespace {

struct TypeExpect {
    const TypeDesc* type;
    const char*     name;
    std::size_t     count;
};

// 這張表就是「期望」本身。改這裡的數字之前先問：是結構真的多了一個欄位
// （那就順手確認 tools/gen_sjson.py 重跑過），還是產生器又漏數了。
const TypeExpect kExpect[] = {
    { &ht9045::sjson::kType_SYSTEM_TEST_IF,     "SYSTEM_TEST_IF",     796 },   // AI(W906-TIF912-R10) 20260926: 788→796＝第 10 條補的 5 欄＋TIF912（b547f27a）加了卻沒重產的 3 欄
    { &ht9045::sjson::kType_SYSTEM_DEVICE_FORM, "SYSTEM_DEVICE_FORM",  66 },
    { &ht9045::sjson::kType_SYSTEM_TEMPERATURE, "SYSTEM_TEMPERATURE", 223 },
    { &ht9045::sjson::kType_SYSTEM_TRAY_FORM,   "SYSTEM_TRAY_FORM",    44 },
    { &ht9045::sjson::kType_SYSTEM_TEST_MODE,   "SYSTEM_TEST_MODE",     5 },
    { &ht9045::sjson::kType_LAST_LEVEL_SET,     "LAST_LEVEL_SET",       1 },
};
const std::size_t kExpectCount = sizeof(kExpect) / sizeof(kExpect[0]);

// 期望的總欄位數。上一輪（20260923 第三輪審查）用臨時 probe 逐欄驗過
// 1,127 欄的邊界、0 筆越界；這個數字就是那次量測的常駐化。
const std::size_t kExpectTotalFields = 1135;   // AI(W906-TIF912-R10) 20260926: 1127→1135（SYSTEM_TEST_IF +8）

// 一個欄位佔的位元組數的**下界**（三維只記到前兩維，見檔頭第 2 條）。
std::size_t FieldSpanLowerBound(const FieldDesc& f) {
    std::size_t n = 1;
    if (f.dim0) n *= f.dim0;
    if (f.dim1) n *= f.dim1;
    return n * f.elemSize;
}

bool NonEmpty(const char* s) { return s != 0 && s[0] != '\0'; }

// why 要指得到檔名，否則「還沒好」是一句沒有資訊的話 —— 下一個人無法複驗。
bool CitesAFile(const char* s) {
    if (!s) return false;
    const std::string w(s);
    return w.find(".cpp") != std::string::npos ||
           w.find(".h")   != std::string::npos;
}

bool EndsWith(const std::string& s, const char* suffix) {
    const std::size_t n = std::strlen(suffix);
    return s.size() > n && s.compare(s.size() - n, n, suffix) == 0;
}

}  // namespace

int main() {
    // =====================================================================
    //  1. 六張型別表：名字、欄位數、structSize。
    // =====================================================================
    std::size_t total = 0;
    for (std::size_t t = 0; t < kExpectCount; ++t) {
        const TypeExpect& e = kExpect[t];
        const TypeDesc& td = *e.type;

        CHECK(NonEmpty(td.name));
        CHECK(td.name != 0 && std::strcmp(td.name, e.name) == 0);
        if (td.count != e.count) {
            std::printf("FAIL %s: count=%u, want %u"
                        "  (結構改了就要重跑 tools/gen_sjson.py)\n",
                        e.name, (unsigned)td.count, (unsigned)e.count);
            ++g_fail;
        } else {
            ++g_pass;
        }
        CHECK(td.fields != 0);
        CHECK(td.structSize > 0);
        // ini 鍵表可以是空的（二進位 blob 沒有 ini 鍵），但指標不可以是 null。
        CHECK(td.iniKeys != 0);
        total += td.count;
    }
    CHECK(total == kExpectTotalFields);
    if (total != kExpectTotalFields)
        std::printf("       總欄位數 %u，期望 %u\n",
                    (unsigned)total, (unsigned)kExpectTotalFields);

    // =====================================================================
    //  2. 每一欄的邊界。offset + n*elemSize <= structSize。
    //
    //  這是把上一輪的臨時 probe 變成常駐的那一段。它用的全是編譯器算出來的
    //  數（offsetof / sizeof），所以它證明的是**這次編譯**的事實。
    // =====================================================================
    std::size_t checkedFields = 0;
    for (std::size_t t = 0; t < kExpectCount; ++t) {
        const TypeDesc& td = *kExpect[t].type;
        std::set<std::size_t> seenOffsets;
        std::set<std::string> seenNames;

        for (std::size_t i = 0; i < td.count; ++i) {
            const FieldDesc& f = td.fields[i];
            ++checkedFields;

            if (!NonEmpty(f.name)) {
                std::printf("FAIL %s[%u]: 欄位沒有名字\n",
                            td.name, (unsigned)i);
                ++g_fail;
                continue;
            }
            ++g_pass;

            // elemSize 由 sizeof 來，0 表示產生器輸出壞了。
            if (f.elemSize == 0) {
                std::printf("FAIL %s.%s: elemSize == 0\n", td.name, f.name);
                ++g_fail;
                continue;
            }
            ++g_pass;

            // dim1 有值而 dim0 沒有 = 二維被記成「沒有第一維」，不可能。
            if (f.dim1 != 0 && f.dim0 == 0) {
                std::printf("FAIL %s.%s: dim1=%u 而 dim0=0\n",
                            td.name, f.name, (unsigned)f.dim1);
                ++g_fail;
            } else {
                ++g_pass;
            }

            const std::size_t span = FieldSpanLowerBound(f);
            if (f.offset >= td.structSize ||
                span > td.structSize ||
                f.offset + span > td.structSize) {
                std::printf("FAIL %s.%s: off=%u + span=%u > sizeof=%u"
                            "  (讀出來會是隔壁欄位的位元組)\n",
                            td.name, f.name,
                            (unsigned)f.offset, (unsigned)span,
                            (unsigned)td.structSize);
                ++g_fail;
            } else {
                ++g_pass;
            }

            // 同一個 offset 出現兩次 = 產生器把某個欄位重複輸出了，或結構裡
            // 有 union。這六張表實測沒有 union，所以重複一定是產生器壞了。
            if (!seenOffsets.insert(f.offset).second) {
                std::printf("FAIL %s.%s: offset %u 與另一欄重複\n",
                            td.name, f.name, (unsigned)f.offset);
                ++g_fail;
            } else {
                ++g_pass;
            }

            if (!seenNames.insert(std::string(f.name)).second) {
                std::printf("FAIL %s.%s: 欄位名重複\n", td.name, f.name);
                ++g_fail;
            } else {
                ++g_pass;
            }
        }
    }
    CHECK(checkedFields == kExpectTotalFields);

    // ini 鍵表裡的 field 名一定要在欄位表裡找得到（寫方向照它找欄位）。
    //
    // ⚠⚠ 空表的表示法是一筆**全 null 的哨兵**，不是 iniKeyCount==0：
    //     sjson_LAST_LEVEL_SET.gen.cpp:32  { 0, 0, 0, 0, 0, false }
    //     所以 LAST_LEVEL_SET 的 iniKeyCount 是 1 而不是 0。
    //     消費端（StructJson.cpp:173/176/188）每一處都先擋 `field == 0`，
    //     所以這個約定今天是成立的 —— 但它是**約定**，沒有任何東西在守。
    //     這裡把它變成被檢查的：field 是 null 的那一筆，其餘指標也必須全是
    //     null（純哨兵），而且那張表只能有那一筆。
    //     半 null 的一筆會直接炸：StructJson.cpp:190-193 對 pathVar /
    //     section / key / deflt 是無條件 String(...)，只靠 field 非 null
    //     當作「其餘欄位也非 null」的代理。
    for (std::size_t t = 0; t < kExpectCount; ++t) {
        const TypeDesc& td = *kExpect[t].type;
        for (std::size_t k = 0; k < td.iniKeyCount; ++k) {
            const ht9045::sjson::IniKey& ik = td.iniKeys[k];

            if (ik.field == 0) {
                // 純哨兵：其餘四個指標也必須是 null，而且整張表只有它。
                if (ik.pathVar || ik.section || ik.key || ik.deflt) {
                    std::printf("FAIL %s: ini 鍵第 %u 筆是半 null 的哨兵"
                                "（消費端只擋 field，其餘欄位會被無條件取值）\n",
                                td.name, (unsigned)k);
                    ++g_fail;
                } else if (td.iniKeyCount != 1) {
                    std::printf("FAIL %s: 空表哨兵出現在有 %u 筆的表裡\n",
                                td.name, (unsigned)td.iniKeyCount);
                    ++g_fail;
                } else {
                    ++g_pass;
                }
                continue;
            }

            // 真的一筆：四個字串欄位都不可以是 null（消費端不擋它們）。
            if (!ik.pathVar || !ik.section || !ik.key || !ik.deflt) {
                std::printf("FAIL %s: ini 鍵 '%s' 有 null 欄位\n",
                            td.name, ik.field);
                ++g_fail;
            } else {
                ++g_pass;
            }

            bool found = false;
            for (std::size_t i = 0; i < td.count && !found; ++i)
                if (td.fields[i].name &&
                    std::strcmp(td.fields[i].name, ik.field) == 0)
                    found = true;
            if (!found) {
                std::printf("FAIL %s: ini 鍵指向不存在的欄位 '%s'\n",
                            td.name, ik.field);
                ++g_fail;
            } else {
                ++g_pass;
            }
        }
    }

    // =====================================================================
    //  3. 溫控通道表：idx 唯一、連續、落在 tcTotalCount 內。
    // =====================================================================
    const std::size_t nCh = ht9045::sjson::kThermoChannelCount;
    CHECK(nCh == 71);
    CHECK(nCh == (std::size_t)tcTotalCount);
    if (nCh != 71)
        std::printf("       kThermoChannelCount=%u，期望 71"
                    "（MachineType.h 改了就要重跑 tools/gen_tempchan.py）\n",
                    (unsigned)nCh);
    {
        std::set<int>         seenIdx;
        std::set<std::string> seenName;
        for (std::size_t i = 0; i < nCh; ++i) {
            const ThermoChannel& c = ht9045::sjson::kThermoChannels[i];
            CHECK(NonEmpty(c.name));
            CHECK(NonEmpty(c.group));

            // 連續：第 i 筆的 idx 必須是 i。這一條擋的是「總數沒變但順序換了」
            // —— gen 檔裡的 static_assert 擋不住那種改動。
            if (c.idx != (int)i) {
                std::printf("FAIL 通道 %s: idx=%d，位置 %u（idx 必須連續）\n",
                            c.name ? c.name : "?", c.idx, (unsigned)i);
                ++g_fail;
            } else {
                ++g_pass;
            }
            if (c.idx < 0 || c.idx >= (int)tcTotalCount) {
                std::printf("FAIL 通道 %s: idx=%d 落在 [0,%d) 之外\n",
                            c.name ? c.name : "?", c.idx, (int)tcTotalCount);
                ++g_fail;
            } else {
                ++g_pass;
            }
            if (!seenIdx.insert(c.idx).second) {
                std::printf("FAIL 通道 %s: idx=%d 重複\n",
                            c.name ? c.name : "?", c.idx);
                ++g_fail;
            } else {
                ++g_pass;
            }
            if (c.name && !seenName.insert(std::string(c.name)).second) {
                std::printf("FAIL 通道名 %s 重複\n", c.name);
                ++g_fail;
            } else {
                ++g_pass;
            }
        }
    }

    // =====================================================================
    //  4. 欄位活性宣告。
    //
    //  ⚠ 「live 今天全是 false」是一個**會過期**的斷言，而且它過期的那一天
    //    是好事（有人把溫控讀取迴圈接進來了）。那時要做的是同時改這裡與
    //    kThermoFields，不是把這一條刪掉。
    // =====================================================================
    const std::size_t nF = ht9045::sjson::kThermoFieldCount;
    CHECK(nF > 0);
    std::size_t nStaged = 0;
    {
        std::set<std::string> seenField;
        for (std::size_t i = 0; i < nF; ++i) {
            const ThermoFieldLiveness& f = ht9045::sjson::kThermoFields[i];
            CHECK(NonEmpty(f.field));
            CHECK(NonEmpty(f.from));

            // why 一律非空 —— staged 的也要，因為 staged 但 live=false 的欄位
            // 正是最需要解釋「為什麼值是 null」的那一批。
            if (!NonEmpty(f.why)) {
                std::printf("FAIL 欄位 %s: why 是空的\n",
                            f.field ? f.field : "?");
                ++g_fail;
            } else {
                ++g_pass;
            }
            if (!CitesAFile(f.why)) {
                std::printf("FAIL 欄位 %s: why 沒有指到任何檔名（無法複驗）\n",
                            f.field ? f.field : "?");
                ++g_fail;
            } else {
                ++g_pass;
            }

            if (f.live) {
                std::printf("FAIL 欄位 %s: live=true。若溫控來源真的活了，"
                            "請一起更新這支測試與 s7_thermo_probe.py，"
                            "不要只放寬斷言\n", f.field ? f.field : "?");
                ++g_fail;
            } else {
                ++g_pass;
            }

            if (f.field && !seenField.insert(std::string(f.field)).second) {
                std::printf("FAIL 欄位名 %s 重複\n", f.field);
                ++g_fail;
            } else {
                ++g_pass;
            }
            if (f.staged) ++nStaged;
        }
        // 今天 staged 的是 pv/comm/inst；不在這裡寫死名字清單（那是 probe 的
        // 事），但 213 = 71 x 3 這個乘積要對得起來。
        CHECK(nStaged == 3);
        CHECK(nCh * nStaged == 213);
    }

    // =====================================================================
    //  5. 離線跑一次 producer：213 個 tag，每一個都是 null。
    //
    //  這是 s7_thermo_probe.py 那條「送 0 會是合法溫度」的離線版本。
    //  不需要伺服器：TagSnapshot 是純資料結構。
    // =====================================================================
    {
        webbridge::TagSnapshot snap;
        snap.beginPublish();
        const std::size_t n = ht9045::sjson::PublishThermoTags(snap);
        snap.commitPublish();

        CHECK(n == 213);
        if (n != 213)
            std::printf("       PublishThermoTags 回 %u，期望 213\n",
                        (unsigned)n);

        const webbridge::TagSnapshotView v = snap.read();

        std::size_t nZone = 0, nNonNull = 0;
        std::size_t nPv = 0, nComm = 0, nInst = 0, nForbidden = 0;
        for (webbridge::TagMap::const_iterator it = v.tags.begin();
             it != v.tags.end(); ++it) {
            const std::string& k = it->first;
            if (k.compare(0, 10, "temp.zone.") != 0) continue;
            ++nZone;
            if (!it->second.isNull()) {
                if (nNonNull < 5)
                    std::printf("FAIL %s 不是 null"
                                "（0 在這棵樹上是合法的攝氏溫度）\n",
                                k.c_str());
                ++nNonNull;
            }
            if (EndsWith(k, ".pv"))        ++nPv;
            else if (EndsWith(k, ".comm")) ++nComm;
            else if (EndsWith(k, ".inst")) ++nInst;
            // ready / state / sv / overAny 不是「值未知」，是「還沒有東西
            // 算得出來」。送 null 會把後者說成前者，所以它們根本不該進串流。
            if (EndsWith(k, ".ready") || EndsWith(k, ".state") ||
                EndsWith(k, ".sv")    || EndsWith(k, ".overAny")) {
                std::printf("FAIL %s 不該進串流\n", k.c_str());
                ++nForbidden;
            }
        }
        if (nNonNull) g_fail += (int)nNonNull; else ++g_pass;
        if (nForbidden) g_fail += (int)nForbidden; else ++g_pass;
        CHECK(nZone == 213);
        CHECK(nPv == nCh);
        CHECK(nComm == nCh);
        CHECK(nInst == nCh);

        // 通道表說有哪些通道，串流就要有哪些通道。
        std::size_t missing = 0;
        for (std::size_t i = 0; i < nCh; ++i) {
            std::string tag = "temp.zone.";
            tag += ht9045::sjson::kThermoChannels[i].name;
            tag += ".pv";
            if (v.tags.count(tag) != 1) {
                if (missing < 5)
                    std::printf("FAIL 少了 tag %s\n", tag.c_str());
                ++missing;
            }
        }
        if (missing) g_fail += (int)missing; else ++g_pass;
    }

    // =====================================================================
    //  6. schema JSON：形狀與完整性（離線，不經 HTTP）。
    // =====================================================================
    {
        const std::string s = ht9045::sjson::ThermoSchemaJson();
        CHECK(s.find("\"error\"") == std::string::npos);
        CHECK(s.find("\"binding\":\"temp.zone\"") != std::string::npos);
        CHECK(s.find("\"channels\":71") != std::string::npos);
        CHECK(s.find("\"fmt\":\"%5.1f\"") != std::string::npos);
        // anyLive 是瀏覽器該先看的那一欄。它今天必須是 false。
        CHECK(s.find("\"anyLive\":false") != std::string::npos);

        // 每一個欄位、每一個通道都要出現在 schema 裡 —— 沉默缺席正是這個
        // schema 最不該做的事（overAny 已經被漏過一次）。
        std::size_t miss = 0;
        for (std::size_t i = 0; i < nF; ++i) {
            std::string k = "\"";
            k += ht9045::sjson::kThermoFields[i].field;
            k += "\":{";
            if (s.find(k) == std::string::npos) {
                std::printf("FAIL schema 少了欄位 %s\n",
                            ht9045::sjson::kThermoFields[i].field);
                ++miss;
            }
        }
        for (std::size_t i = 0; i < nCh; ++i) {
            std::string k = "\"name\":\"";
            k += ht9045::sjson::kThermoChannels[i].name;
            k += "\"";
            if (s.find(k) == std::string::npos) {
                if (miss < 5)
                    std::printf("FAIL schema 少了通道 %s\n",
                                ht9045::sjson::kThermoChannels[i].name);
                ++miss;
            }
        }
        if (miss) g_fail += (int)miss; else ++g_pass;
    }

    if (g_fail == 0) {
        std::printf("test_jsonbridge_s7: OK "
                    "(%u checks; %u fields across %u types; "
                    "%u thermo channels x %u staged = %u null tags)\n",
                    (unsigned)g_pass, (unsigned)checkedFields,
                    (unsigned)kExpectCount, (unsigned)nCh,
                    (unsigned)nStaged, (unsigned)(nCh * nStaged));
    } else {
        std::printf("test_jsonbridge_s7: %d FAILURE(S)\n", g_fail);
    }
    return g_fail == 0 ? 0 : 1;
}
