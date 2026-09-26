// ===========================================================================
//  JsonBridge/ChanAction.cpp
//
//  AI(W906-SJSON-S11) 20260923.  NOT in golden.
// ===========================================================================
#include "JsonBridge/ChanAction.h"

#include <cstdio>
#include <string>
#include <vector>
#include "JsonBridge/actions/MainStateRecord.h"   // AI(W906-STATEREC) 20260924: act.main.stateRecord（佔用原本的空行，不移動行號）
#include "JsonBridge/EventLog.h"
#include "JsonBridge/actions/MainClarnData.h"
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cAuthority.h"          // GetCountClrAuth / authCounterClr
#include "cMyDB.h"               // MyDBIProcess
#include "cprod.h"               // Prod / RunInfo
#include "LastSet.h"             // LastSet（不在 cprod.h）
#include "Config.h"              // IniConfig
#include "forms/fMain.h"
#include "forms/fCounterClear.h"

namespace ht9045 {
namespace sjson {

namespace {

// -------------------------------------------------------------------------
//  counter.clear 的 7 個 family。
//
//  ⚠⚠ 這張表是**從 tools/wb_serve.cpp 的 counter.clear 分支原樣搬過來的**，
//    不是重寫。搬家的理由是派工單第 2 條（「所有實質邏輯一律在 JsonBridge/
//    新檔裡」）＋ SKILL §4.5 規則 1（「既有 counter.clear 保留為
//    act.counterClear.exe 的別名」）。原分支的長註解留在 wb_serve 沒動，
//    那裡是它的歷史；這裡是它的本體。
//
//  authIdx = funCounterClr[] 的順序（cAuthority.cpp:203）
//  ct 配對 = golden spbExeClick，含 loadingCount 的 ctIndexCount 附掛
//            （golden :433 kevin 20130125）
// -------------------------------------------------------------------------
struct FamilyRow { const char* key; int authIdx; int ct1; int ct2; };
const FamilyRow kFamilies[] = {
    { "alarmData",      0, ctAlarmData,       -1           },
    { "testerCategory", 1, ctTesterCategory,  -1           },
    { "loadingCount",   3, ctLoadingCounts,   ctIndexCount },
    { "contactCurr",    4, ctContactCounts,   -1           },
    { "contactHis",     5, ctContactCountsHis,-1           },
    { "sortingCount",   6, ctTraySortCount,   -1           },
    { "timeData",       7, ctTimeData,        -1           },
};
const std::size_t kFamilyCount = sizeof(kFamilies) / sizeof(kFamilies[0]);

// 13 個 Tag 的語意。抽自 golden V912 main.cpp:15491-15613 的分支與註解
// ＋ 46 個呼叫點自己帶的訊息字串。
struct TagDoc { int tag; const char* meaning; int liveCallers; };
const TagDoc kTagDocs[] = {
  {  0, "Initial / RT Initial Start：依 LastSet.bCTClear[iMode][] 選擇性清 7 類，"
        "另清 YieldMonitoring 的 site 顯示與 Ignore 計數", 6 },
  {  1, "Lot Start／ART 輸入量／遠端 ClearBinCountByDLL：清 Loading+Sort+Tester+Index", 13 },
  {  2, "ART Retest 清批：同 Tag1 但**不清** TraySortCount（golden 自己註解掉那行）", 6 },
  {  3, "SPIL RT：Loading+Index+Contact+ContactHis+Sort", 1 },
  {  4, "SPIL FT：Loading+Sort+Tester+Index+Contact+ContactHis", 1 },
  {  5, "DoAutoRetest ClearData1/3：AutoRetestCount+Loading+Index+Tester", 2 },
  {  6, "DoAutoRetest ClearData2：Loading+Index+Tester", 1 },
  {  7, "WaitStartLotAutoRetestGPIB：只清 Tester+Index", 1 },
  {  8, "SortCT btnClearCountClick／SECS S2F42／ASE：Loading（SCS/ASE_Korea/"
        "AMKOR_Korea 除外）+Sort+Tester（VTEST 除外）+Index；KYEC_LEE 另清 Yield", 7 },
  {  9, "LotInfo DownloadFile：fCounterClear->AutoClear()（依權限自動選）", 3 },
  { 10, "手動清除的前後標記／Software Start：**分支本體是空的**，"
        "只跑共用前置與尾段（QtyLog、TrayID/Total 歸零、WriteLastDataFile）", 3 },
  { 11, "LowYield 重新 Start：Sort + fContactCT->btClearCountClick", 1 },
  { 12, "ATK RT Continue Start：只清 Loading 與 FailBin", 1 },
};

std::string JStr(const cJSON* root, const char* key, const char* dflt)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive((cJSON*)root, key);
    if (v && cJSON_IsString(v) && v->valuestring) return std::string(v->valuestring);
    return std::string(dflt);
}

// ⚠ dryRun **只認真正的 JSON bool false**。字串 "false" 不算。
//   與 wb_serve.cpp:3657-3665 的判斷同源，理由也一樣：關掉一個會寫真實檔的
//   保護開關必須是明確的，不能靠型別轉換巧合。
//   沒寫 / payload 壞掉 / 型別不對 -> 維持 true。
bool ParseDryRun(const cJSON* root)
{
    if (root == 0) return true;
    const cJSON* j = cJSON_GetObjectItemCaseSensitive((cJSON*)root, "dryRun");
    if (j && cJSON_IsBool(j) && cJSON_IsFalse(j)) return false;
    return true;
}

void WriteRefusal(webbridge::JsonWriter& w, const char* guard, const char* detail)
{
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("guard").String(guard);
    if (detail && detail[0]) w.Key("detail").String(detail);
    w.EndObject();
}

// -------------------------------------------------------------------------
//  act.main.clarnData
// -------------------------------------------------------------------------
std::string DoClarnData(const std::string& payloadJson)
{
    webbridge::JsonWriter w;

    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (root == 0) { WriteRefusal(w, "bad-payload", "value 不是合法 JSON"); return w.Str(); }

    const cJSON* jtag = cJSON_GetObjectItemCaseSensitive(root, "tag");
    // ⚠ cJSON 的數字一律是 double。這裡要的是 0..12 的整數，所以先看
    //   IsNumber 再自己截斷並檢查範圍 —— 不接受 "8" 這種字串，理由同
    //   ParseDryRun：動作通道的參數要明確。
    if (jtag == 0 || !cJSON_IsNumber(jtag)) {
        cJSON_Delete(root);
        WriteRefusal(w, "bad-payload", "需要整數欄位 tag（0..12）");
        return w.Str();
    }
    const int tag = (int)jtag->valuedouble;
    if (tag < 0 || tag > 12 || (double)tag != jtag->valuedouble) {
        cJSON_Delete(root);
        WriteRefusal(w, "bad-tag",
                     "tag 必須是 0..12 的整數（golden 有 13 個分支 Tag 0..12）");
        return w.Str();
    }
    const std::string msg = JStr(root, "msg", "");
    const bool dry = ParseDryRun(root);
    cJSON_Delete(root);

    // --- 守衛 1：golden 自己的（main.cpp:15468） -------------------------
    //   golden 是 `return;`，網頁上會變成「按了沒反應」。這裡回報。
    if (IniConfig.bA61DisableCleanMUBA == true) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("A61DisableCleanMUBA");
        w.Key("goldenLine").String("V912 main.cpp:15468");
        w.Key("dryRun").Bool(dry);
        w.EndObject();
        return w.Str();
    }

    // --- 守衛 2：本體有沒有裝上（移植樹自己的事實，不是 golden） ---------
    //   沒裝的話 TfMain::Clarn_Data 仍然是純計數 no-op。回「執行了」會是謊話。
    if (!ClarnDataBodyInstalled()) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("body-not-installed");
        w.Key("detail").String("InstallClarnDataBody() 沒被呼叫 -- "
                               "TfMain::Clarn_Data 仍是 no-op（forms/fMain.cpp）");
        w.Key("dryRun").Bool(dry);
        w.EndObject();
        return w.Str();
    }

    const std::vector<std::string> would = ClarnDataPreview(tag);

    w.BeginObject();
    w.Key("dryRun").Bool(dry);
    w.Key("tag").Number((wb_int64)tag);
    w.Key("would").BeginArray();
    for (std::size_t i = 0; i < would.size(); ++i) w.String(would[i]);
    w.EndArray();

    if (dry) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("");           // 沒有守衛擋，是 dryRun 自己停的
        w.Key("note").String("dryRun：只跑守衛與預覽，沒有呼叫 Clarn_Data");
        w.EndObject();
        return w.Str();
    }

    // --- 留痕：golden 的按鈕處理器自己會記，原語沒有，所以這裡補一筆 -----
    //   SKILL §4.7 規則 1：每個 act.* 在本體執行前叫一次 RecordProcess。
    {
        char buf[160];
        std::snprintf(buf, sizeof(buf), "act.main.clarnData(%d) pressed", tag);
        LogAppend(kLogProcess, buf, msg, "", "act");
    }

    const int before = fMain ? fMain->W906_Clarn_DataCallCount : -1;
    fMain->Clarn_Data(tag, AnsiString(msg.c_str()));
    const int after  = fMain ? fMain->W906_Clarn_DataCallCount : -1;

    w.Key("executed").Bool(true);
    // ⚠ 這個差值是「真的進了 TfMain::Clarn_Data」的證據。回 true 而不附證據
    //   的話，呼叫端分不出「執行了」與「我們相信它執行了」——
    //   而這棵樹已經有過 counter.clear 回 SUCCESS 但什麼都沒動的先例。
    w.Key("callCountDelta").Number((wb_int64)(after - before));
    w.Key("sideEffectsSkipped").BeginArray();
    // golden 這一支自己不叫 SECS/OEE，但它呼叫的三個東西在移植樹少做了事。
    // ⚠ 回 executed:true 而不列出這些，等於宣稱「golden 做的事都做了」。
    w.String("fSortCT.ShowLoadingIC()/ShowSortIC() 在移植樹是 no-op facade"
             "（forms/fSortCT.h:51-63）-- LastSet.SendCT[2] 與 "
             "RunInfo.iUnloadCount 的匯總沒有發生");
    // AI(W906-SJSON-S11) 20260923: 第一版漏了這一條，審查前自查補上。
    //   golden :15632 的 MyDBIProcess("Process", Msg) 會寫 sqlite 的 Process
    //   資料表＋文字 EventLog（golden cMyDB.cpp:789）。
    //
    // AI(W906-SJSON-S11fix) 20260923: **上面那版說「連到 2 參數空槽」是錯的**
    //   （審查員第二輪 §十三 待辦 #1 開的單）。這裡不是直接連到空槽，中間
    //   還隔一層轉發。實測的完整鏈（`nm`，不是讀原始碼推論）：
    //
    //     本檔的呼叫點           MainClarnData.cpp
    //       │  U @_Z12MyDBIProcessN9vclcompat10AnsiStringES0_S0_@72  ← **3 參數**
    //       │    （cMyDB.h:81 宣告 `AnsiString S2=""`，所以 2 個引數
    //       │      的寫法**仍然**選到 3 參數這支，不是 2 參數那支）
    //       ▼
    //     3 參數本體            SECSGEM/uHGemEquipment.cpp:3483
    //       │    body 逐字是 `::MyDBIProcess(S1, S2); (void)S3;`  ← 純轉發
    //       ▼
    //     2 參數本體            aHotPlateSubstrate.cpp:1264        ← **計數式空槽**
    //            只累加 W906_MyDBIProcess_Count 並記下最後一組字串，
    //            一個 row 都不寫。
    //
    //   ⚠ cMyDB.cpp:1000 那個「3 參數真本體」**整段包在 `#if 0` 裡**
    //     （`:999` 的 `#if 0 // TODO(GA1-B4-integrate)`，`:1067` 收尾），
    //     所以它的 ProductionLog／slEventLog／SaveEventLogInfo **不會**執行。
    //     `nm` 也看得到：cMyDB.cpp.obj 對這個符號是 `U`（用它），不是 `T`（定義它）。
    //   ⇒ **結論不變：sqlite 與文字 EventLog 都沒有落地。**變的只是「為什麼」。
    //
    //   ⓘ 至於「會不會當掉」：**不會，而且本來就沒有要防的東西。**
    //     舊註解寫的「兩層保險」其實一層都不需要 ——
    //     MyDBExecSQL 在這棵樹根本沒有 crash 路徑（SKILL §4.7 規則 3 的更正段）：
    //       * cMyDB.cpp:240-241 的 `if(bUseMDB==false) return 0;` 擋在
    //         sqlite3_exec 之前；
    //       * 就算 bUseMDB 開著而 dbReadWrite 是 NULL，本樹自帶的
    //         sqlite 3.7.7.1 也是 `if(!sqlite3SafetyCheckOk(db)) return
    //         SQLITE_MISUSE_BKPT;`（sqlite3.c:86901、:21572-21577），回錯誤碼不是當機；
    //       * cMyDB.cpp:250 連 last_insert_rowid 都已經 NULL-guard 過。
    //     ⇒ 不要再用「會 crash」當作不把 MyDBOpenDB 帶進開機序列的理由。
    //       真正的理由是「不開 DB 就什麼都沒寫進去」。
    //   20260923 實測：nm ＋ 四處原始碼。
    w.String("MyDBIProcess(\"Process\", msg) -- 呼叫解析到 3 參數版"
             "（cMyDB.h:81 的 S2 預設值），本體在 uHGemEquipment.cpp:3483 "
             "且只是 `::MyDBIProcess(S1,S2)` 轉發，最終落在 "
             "aHotPlateSubstrate.cpp:1264 的計數式空槽；cMyDB.cpp:1000 的真本體"
             "整段在 #if 0 內。sqlite 與文字 EventLog 都沒有落地"
             "（golden cMyDB.cpp:789 會寫兩者）");
    w.EndArray();
    w.EndObject();
    return w.Str();
}

// -------------------------------------------------------------------------
//  act.counterClear.exe（別名：counter.clear）
//
//  行為與 tools/wb_serve.cpp 原本的 counter.clear 分支**逐行相同**。
//  刻意不「順手改良」：它已經有 e2e 驗證跑過，改動等於把驗證作廢。
// -------------------------------------------------------------------------
std::string DoCounterClearExe(const std::string& payloadJson, const std::string& wsTag)
{
    webbridge::JsonWriter w;

    // family 可以從 WS 的 tag 來（舊的 counter.clear 慣例），也可以從
    // payload 的 "family" 來（act.* 的慣例）。兩條都收，不逼瀏覽器改。
    std::string family = wsTag;
    bool dry = false;    // ⚠ 舊的 counter.clear **沒有** dryRun，預設就是執行。
                         //   改成預設 dry 會讓既有的 e2e 與畫面全部靜默失效。
    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (root != 0) {
        const std::string f = JStr(root, "family", "");
        if (!f.empty()) family = f;
        // 只有 payload 明確寫了 dryRun 才理它（明確 true 才 dry）。
        const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
        if (j && cJSON_IsBool(j) && cJSON_IsTrue(j)) dry = true;
        cJSON_Delete(root);
    }

    const FamilyRow* row = 0;
    for (std::size_t f = 0; f < kFamilyCount; ++f)
        if (family == kFamilies[f].key) { row = &kFamilies[f]; break; }

    if (row == 0) { WriteRefusal(w, "unknown-family", family.c_str()); return w.Str(); }

    GetCountClrAuth();                                   // golden FormShow gate
    if (!authCounterClr[row->authIdx]) {
        WriteRefusal(w, "not-authorized", family.c_str());
        return w.Str();
    }

    if (dry) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("dryRun").Bool(true);
        w.Key("family").String(family);
        w.Key("would").BeginArray();
        w.String("Clarn_Data(10, \"Manual clear count\")");
        w.String(std::string("ClearCount(") + family + ")");
        if (row->ct2 >= 0) w.String("ClearCount(ctIndexCount)");
        w.String("Clarn_Data(10, \"Manual clear count done\")");
        w.EndArray();
        w.EndObject();
        return w.Str();
    }

    fMain->Clarn_Data(10, "Manual clear count");          // golden :421
    fCounterClear->ClearCount(row->ct1);
    if (row->ct2 >= 0) fCounterClear->ClearCount(row->ct2);
    fMain->Clarn_Data(10, "Manual clear count done");     // golden :448
    MyDBIProcess("Process", "Counter Clear has been executed!!");   // golden :449

    std::printf("counter.clear[%s] done (SendCT[0]=%d iIndexCount=%d)\n",
                row->key, (int)LastSet.SendCT[0], LastSet.iIndexCount);

    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("family").String(family);
    w.Key("sideEffectsSkipped").BeginArray();
    // 原分支註解記的兩個刻意偏離，原樣帶進回應（SKILL §4.5 規則 4）。
    w.String("MyDBIProductionData -- wb_serve 沒開 sqlite；"
             "MyDBExecSQL 對 null dbReadWrite 是 crash 不是空操作");
    w.String("CC_KYEC_LEE re-auth 分支 -- 翻譯核心自己就是 GATE CC3 (#if 0)");
    w.EndArray();
    w.EndObject();
    return w.Str();
}

}  // namespace

bool IsActionCommand(const std::string& cmd)
{
    return cmd.compare(0, 4, "act.") == 0;
}

std::string HandleAction(const std::string& cmd, const std::string& payloadJson)
{
    return HandleActionWithTag(cmd, payloadJson, std::string());
}

std::string HandleActionWithTag(const std::string& cmd,
                                const std::string& payloadJson,
                                const std::string& wsTag)
{
    if (cmd == "act.main.clarnData")       return DoClarnData(payloadJson);
    if (cmd == "act.counterClear.exe" ||
        cmd == "counter.clear")            return DoCounterClearExe(payloadJson, wsTag);
    if (cmd == "act.main.stateRecord")     return DoStateRecordAction(payloadJson);   // AI(W906-STATEREC) 20260924: golden sbStateRecordClick（佔用原本的空行，不移動行號）
    webbridge::JsonWriter w;
    WriteRefusal(w, "unknown-action",
                 "目前只有 act.main.clarnData、act.main.stateRecord 與 act.counterClear.exe；"   // AI(W906-STATEREC) 20260924: 補上新動作名
                 "清單見 GET /api/struct/act/schema");
    return w.Str();
}

std::string ActionSchemaJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("binding").String("act");
    w.Key("dryRunDefault").BeginObject();
    w.Key("act.main.clarnData").Bool(true);
    // ⚠ 兩個動作的預設**不一樣**，而且是故意的。寫進 schema 免得有人以為
    //   漏了：counter.clear 是既有指令，改預設會讓既有頁面靜默失效。
    w.Key("act.counterClear.exe").Bool(false);
    w.EndObject();

    w.Key("actions").BeginArray();
    {
        w.BeginObject();
        w.Key("cmd").String("act.main.clarnData");
        w.Key("golden").String("V912 main.cpp:15458-15648 (191 行)");
        w.Key("portBody").String("JsonBridge/actions/MainClarnData.cpp");
        w.Key("installed").Bool(ClarnDataBodyInstalled());
        w.Key("args").BeginObject();
        w.Key("tag").String("int 0..12（必填）");
        w.Key("msg").String("string，進 QtyLog 與 MyDBIProcess（選填）");
        w.Key("dryRun").String("bool，預設 true");
        w.EndObject();
        w.Key("guards").BeginArray();
        w.String("A61DisableCleanMUBA -- IniConfig.bA61DisableCleanMUBA==true");
        w.String("body-not-installed -- 移植樹自己的狀態，不是 golden 的守衛");
        w.EndArray();
        w.Key("writes").BeginArray();
        w.String("D:\\HT9045\\system\\lastdata.dat（硬編路徑，--dry 蓋不到）");
        w.String("D:\\HT9045\\system\\lastdata_backup.dat");
        w.String("D:\\HT9045_Log\\QtyData\\YYYYMM\\<PC>_<Lot>_<date>.csv");
        w.EndArray();
        w.Key("tags").BeginArray();
        for (std::size_t i = 0; i < sizeof(kTagDocs)/sizeof(kTagDocs[0]); ++i) {
            w.BeginObject();
            w.Key("tag").Number((wb_int64)kTagDocs[i].tag);
            w.Key("meaning").String(kTagDocs[i].meaning);
            w.Key("goldenCallers").Number((wb_int64)kTagDocs[i].liveCallers);
            w.EndObject();
        }
        w.EndArray();
        // 量到的，不是抄的。方法寫在旁邊讓人複驗。
        w.Key("goldenCallerTotal").Number((wb_int64)46);
        w.Key("goldenCallerMethod")
            .String("grep -rn \"Clarn_Data *(\" --include=*.cpp（V912）= 48 行，"
                    "扣掉 SECSGEM/uHGemHT9045.cpp:1892 整行註解掉的一筆與 "
                    "csystem.cpp:6592 只出現在行尾註解裡的一筆 -> 46。"
                    "⚠ SKILL.md §4.5 的標題數字寫 47，但它自己的分佈表加起來是 46");
        w.EndObject();
        WriteStateRecordActionSchema(w);   // AI(W906-STATEREC) 20260924: act.main.stateRecord 的 schema 物件（佔用原本的空行，不移動行號）
        w.BeginObject();
        w.Key("cmd").String("act.counterClear.exe");
        w.Key("aliasOf").String("counter.clear（舊名，仍然接受）");
        w.Key("golden").String("cCounterClear.cpp spbExeClick");
        w.Key("args").BeginObject();
        w.Key("family").String("alarmData | testerCategory | loadingCount | "
                               "contactCurr | contactHis | sortingCount | timeData"
                               "（也可放在 WS 的 tag 欄，舊慣例）");
        w.Key("dryRun").String("bool，預設 false（見 dryRunDefault 的說明）");
        w.EndObject();
        w.Key("guards").BeginArray();
        w.String("unknown-family");
        w.String("not-authorized -- authCounterClr[]，讀自 config\\Security_new.def");
        w.EndArray();
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

}  // namespace sjson
}  // namespace ht9045
