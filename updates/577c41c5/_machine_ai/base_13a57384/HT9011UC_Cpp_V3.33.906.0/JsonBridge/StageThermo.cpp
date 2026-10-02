// ===========================================================================
//  JsonBridge/StageThermo.cpp -- 溫控通道 producer。
//
//  AI(W906-JSONBRIDGE-S7) 20260923.  NOT in golden.
//  設計與三個資料源的實測狀態寫在 StageThermo.h 的檔頭，不重複。
// ===========================================================================
#include "JsonBridge/StageThermo.h"

#include <cstdint>

#include "WebBridge/JsonWriter.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

namespace ht9045 {
namespace sjson {

// -------------------------------------------------------------------------
//  每個欄位的活性宣告。
//
//  ⚠⚠ 這張表是**手寫的，而且必須手動維護**。它記的是「移植樹今天的翻譯
//    進度」，那件事沒有辦法從程式碼自動推導 —— 一個全域有沒有人寫，
//    grep 得出來；但驅動那些寫入點的執行緒在 wb_serve 裡會不會啟動，
//    grep 不出來。所以這裡每一條 why 都寫到檔名行號，讓下一個人能自己複驗。
//
//    翻譯進度一往前走（例如有人把 bthermo.cpp 的讀取迴圈接進 wb_serve），
//    **這張表就要跟著改**，否則 schema 會繼續說「不活」而值已經是真的了
//    —— 那個方向的錯更糟：瀏覽器會把真溫度當成佔位符丟掉。
// -------------------------------------------------------------------------
const ThermoFieldLiveness kThermoFields[] = {
    { "pv", "UN150Read[]", true, false,
      "The whole write chain is translated -- THeaterThread::Execute -> "
      "DoThermo (uHeaterThread.cpp:392) -> DoThermoReal (bthermo.cpp:1284) -> "
      "UN150Read[Addr]=DOUN150ReadTemp(Addr) (bthermo.cpp:3442) -- but nothing "
      "constructs or starts that thread: every HeaterThread hit outside "
      "uHeaterThread.cpp is a comment, and tools/wb_serve.cpp mentions neither "
      "HeaterThread nor DoThermo. The SOFT_SIMULTE fill branches "
      "(bthermo.cpp:4581/4598/4615) are compiled IN under the default build "
      "(port MachineType.h:64 defines SOFT_SIMULTE) but sit on that same "
      "never-started chain. NOTE: WebBridgeTags.h AI(W906-FW-TEMP2) says the "
      "fill branch is 'compiled out'; that was true on 20260820 and stale since "
      "20260918 -- the death cause is reachability, not preprocessing." },

    { "comm", "UN150CommError[]", true, false,
      "written by the same bthermo.cpp loops as UN150Read[] (:2796/2828/2877), "
      "so it lives and dies with pv. Staged anyway because 'comms lost' and "
      "'temperature is 0' must be distinguishable, and the value alone cannot "
      "distinguish them." },

    { "inst", "bUT150Install[]", true, false,
      "All-false at runtime, but NOT because there are no writers. Golden V912 "
      "has 1096 live assignments; 1085 of them live in main.cpp "
      "(Index16Heater 931, IndexHeatMode 71, Tri_Temp_Set_Site 36, "
      "HotplateHeatMode 28, Timer2Timer 18, ctor 1) and main.cpp does not "
      "exist in the port tree at all. (Counts exclude 6 commented-out "
      "assignments -- a raw grep gives 1091 for main.cpp.) "
      "The remaining 11 are OpenDUTHeat(bool) (csystem.cpp:24629-24686), fully "
      "translated and marked ACTIVE (csystem.cpp:23955) but with zero callers. "
      "So making this field live means TRANSLATING main.cpp's heater-mode "
      "logic (Index16Heater alone is ~1650 golden lines, V912 main.cpp:19229-20881 "
      "-- see MainCalcCore.h:94), not opening OpenDUTHeat's call sites, which "
      "are only 1% of the writers. SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） said to skip uninstalled "
      "channels to save bandwidth -- doing that today would publish ZERO "
      "channels. Staged as null, NOT false: false would read as 'checked, "
      "not installed'." },

    { "ready", "ShowThermo bTemperatureReady[]", false, false,
      "NOT STAGED. It is a function-local static inside "
      "TfTemperFrom::ShowThermo (cTemperFrom.cpp:238) with no extern -- "
      "unreachable from here. The port-tree comment records that golden only "
      "ever WRITES it, never reads it. Making it a tag requires refactoring "
      "ShowThermo to write into a module-level struct (SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） rule 1); "
      "that means touching ~920 lines of already-translated golden logic "
      "(port cTemperFrom.cpp:234-1157; golden V908 885 lines, V912 894)." },

    { "state", "iTempOverShowAlarmT[] (cmydef.cpp:3528)", false, false,
      "NOT STAGED -- and NOT because nothing readable holds it. The "
      "classification IS in a global: iTempOverShowAlarmT[] (extern "
      "cmydef.h:5018, def cmydef.cpp:3528), and ShowThermo writes over=1 / "
      "below=2 / ok=0 into it at cTemperFrom.cpp:796,800,810,819,823,833. "
      "The real reason is worse: ShowThermo has ZERO production callers in "
      "this tree (all 4 hits are tests/test_temperfrom_core.cpp; golden's "
      "driver TfTemperFrom::Timer1Timer is not ported), so that global sits "
      "at its zero initialiser forever -- AND 0 IS \"ok\". Staging it would "
      "publish 71 confident 'ok' verdicts for channels nothing has ever "
      "measured. That is the null-vs-0 rule wearing a different coat: the "
      "readable thing exists, but its 0 and 'never computed' are the same "
      "bit pattern." },

    { "sv", "ShowThermo per-group SetTemp", false, false,
      "NOT STAGED. ShowThermo (cTemperFrom.cpp:234) picks a different SetTemp "
      "per channel group "
      "(ATC / Heater / HotPlate / Shuttle / DUT / Base / Door) through a chain "
      "of range comparisons. The `group` field in the channel table is a "
      "name-prefix approximation from gen_tempchan.py and MUST NOT be used to "
      "pick SetTemp -- it would be wrong for the tri-temp and 1-to-2-site "
      "cases. Needs the same ShowThermo refactor." },

    // AI(W906-JSONBRIDGE-S7) 20260923：SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） 列了第 7 個 tag
    // `temp.overAny`，而這張表原本完全沒提它 —— 既不 staged 也沒宣告
    // not-staged。這個 schema 的賣點就是「每個欄位都誠實描述」，
    // 沉默缺席剛好是它最不該做的事。補上。
    // （注意它與其他六個不同：它不是 per-channel 的，是 71 個通道的彙整。
    //   真的上線時要 stage 成單一 tag `temp.overAny`，不是
    //   temp.zone.<ch>.overAny。）
    { "overAny", "ShowThermo bOverTemp aggregate", false, false,
      "NOT STAGED, and NOT per-channel: golden aggregates it in "
      "TfTemperFrom::Timer1Timer (golden V912 cTemperFrom.cpp:1627, loop at "
      ":1648-1652) as `for(i..) bOverTemp |= ShowThermo(i)` and uses it to "
      "decide bCooling. That driver is not ported (port tree cTemperFrom.cpp "
      "has no Timer1Timer). Blocked by the same root cause as state/sv "
      "-- ShowThermo has zero production callers here, so there is nothing to "
      "OR together. Listed explicitly rather than omitted: SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） "
      "specifies this tag, and a schema that claims to describe every field "
      "must not let one go silently missing." },
};

const std::size_t kThermoFieldCount =
    sizeof(kThermoFields) / sizeof(kThermoFields[0]);

namespace {

// golden 的顯示格式（cTemperFrom.cpp 的 sprintf("%5.1f", …)）。
// 進 schema 不進值 —— SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） 規則 2。
const char* const kThermoFmt = "%5.1f";

}  // namespace

std::string ThermoSchemaJson() {
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("binding").String("temp.zone");
    w.Key("channels").Number((wb_int64)kThermoChannelCount);
    w.Key("fmt").String(kThermoFmt);

    // ⚠ anyLive 放在最前面附近是刻意的：瀏覽器應該先看這一欄再看任何值。
    //   今天是 false —— 底下 71 個通道的每一個值都是 null，而那是正確的
    //   答案，不是錯誤。
    bool anyLive = false;
    for (std::size_t i = 0; i < kThermoFieldCount; ++i)
        if (kThermoFields[i].live) anyLive = true;
    w.Key("anyLive").Bool(anyLive);

    w.Key("fields").BeginObject();
    for (std::size_t i = 0; i < kThermoFieldCount; ++i) {
        const ThermoFieldLiveness& f = kThermoFields[i];
        w.Key(f.field).BeginObject();
        w.Key("from").String(f.from);
        w.Key("staged").Bool(f.staged);
        w.Key("live").Bool(f.live);
        w.Key("why").String(f.why ? f.why : "");
        w.EndObject();
    }
    w.EndObject();

    w.Key("items").BeginArray();
    for (std::size_t i = 0; i < kThermoChannelCount; ++i) {
        const ThermoChannel& c = kThermoChannels[i];
        w.BeginObject();
        w.Key("idx").Number((wb_int64)c.idx);
        w.Key("name").String(c.name);
        w.Key("group").String(c.group);
        w.EndObject();
    }
    w.EndArray();

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::size_t PublishThermoTags(webbridge::TagSnapshot& snap) {
    std::size_t n = 0;
    std::string tag;
    tag.reserve(48);

    for (std::size_t i = 0; i < kThermoChannelCount; ++i) {
        const ThermoChannel& c = kThermoChannels[i];
        for (std::size_t f = 0; f < kThermoFieldCount; ++f) {
            const ThermoFieldLiveness& fl = kThermoFields[f];
            if (!fl.staged) continue;

            tag.assign("temp.zone.");
            tag.append(c.name);
            tag.append(1, '.');
            tag.append(fl.field);

            // ⚠⚠ 今天每一個 staged 欄位的 live 都是 false，所以這裡一律
            //   makeNull()。**不要**因為「反正都是 null」就把讀取那一段
            //   先寫起來放 —— 那會變成一段沒有任何資料驗證過的路徑，
            //   而它讀的是溫度。等哪一個欄位真的活了，在這裡加它自己的
            //   讀取分支，並且同時改 kThermoFields 的 live 旗標。
            //
            //   送 null 不是佔位符：它精確表示「這個通道存在、值還沒有
            //   來源」。送 0 會是一個合法的攝氏溫度。
            snap.stage(tag, webbridge::TagValue::makeNull());
            ++n;
        }
    }
    return n;
}

}  // namespace sjson
}  // namespace ht9045
