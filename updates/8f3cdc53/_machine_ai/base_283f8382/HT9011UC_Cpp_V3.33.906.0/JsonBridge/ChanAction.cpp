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
#include "JsonBridge/actions/MainTesterConnect.h"   // AI(W906-GB-P2e) 20260926: act.main.testerConnect（golden imgTesterClick）
#include "JsonBridge/actions/MainRecordClear.h"   // AI(W906-S119) 20260927: act.main.clearRecord／act.main.meShuttle2Dbl（golden spbClearRecordClick／meShuttle2DblClick）
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
#include "cSocket.h"             // Steven 20260925 (W906-CC-PAGE): ArmData／ArmHistory（counterclear.get 的 counters 摘要）
#include "cmydef.h"              // Steven 20260925 (W906-CC-PAGE): iTestBinCount、iTo3Unload

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

// ⚠ dryRun（AI(W906-CLARN-R12) 20260926，RULINGS_20260926 第 12 條）：**沒寫或明確 JSON bool false ⇒ 執行**（golden TfMain::Clarn_Data 沒有預覽，按下就做）；
//   **明確 JSON bool true ⇒ 只跑守衛與預覽**；型別不對（字串 "true"、1、null）⇒ 保守當成預覽，
//   寫檔的開關不靠型別轉換巧合。原本（20260923）是「只認 bool false，其他一律預覽」，與 wb_serve.cpp:3657-3665 的 struct.put 同源；
//   struct.put／recipe.doc.put 的 dryRun 預覽是另一件事（使用者 20260924 確認是真需求），不受這條影響。payload 壞掉（root==0）仍回 true。
bool ParseDryRun(const cJSON* root)
{
    if (root == 0) return true;
    const cJSON* j = cJSON_GetObjectItemCaseSensitive((cJSON*)root, "dryRun");
    if (j == 0 || (cJSON_IsBool(j) && cJSON_IsFalse(j))) return false;   //AI(W906-CLARN-R12) 20260926: 沒帶 dryRun＝直接執行（與 golden Clarn_Data 相同）；明確 true 才預覽；型別不對仍只預覽、不寫檔
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
    if (cmd == "act.main.testerConnect")   return DoTesterConnectAction(payloadJson);   // AI(W906-GB-P2e) 20260926: golden imgTesterClick main.cpp:29732-29794
    if (cmd == "act.main.clearRecord")     return DoClearRecordAction(payloadJson);     // AI(W906-S119) 20260927: golden spbClearRecordClick main.cpp:31109-31138
    if (cmd == "act.main.meShuttle2Dbl")   return DoMeShuttle2DblAction(payloadJson);   // AI(W906-S119) 20260927: golden meShuttle2DblClick main.cpp:29471-29475（AseRecordMemo OnDblClick）
    if (cmd == "act.main.stateRecord")     return DoStateRecordAction(payloadJson);   // AI(W906-STATEREC) 20260924: golden sbStateRecordClick（佔用原本的空行，不移動行號）
    webbridge::JsonWriter w;
    WriteRefusal(w, "unknown-action",
                 "目前只有 act.main.clarnData、act.main.stateRecord、act.main.testerConnect、act.main.clearRecord、act.main.meShuttle2Dbl 與 act.counterClear.exe；"   // AI(W906-STATEREC) 20260924: 補上新動作名；AI(W906-S119) 20260927: 補 S119 兩個
                 "清單見 GET /api/struct/act/schema");
    return w.Str();
}

std::string ActionSchemaJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("binding").String("act");
    w.Key("dryRunDefault").BeginObject();
    w.Key("act.main.clarnData").Bool(false);   //AI(W906-CLARN-R12) 20260926: RULINGS_20260926 第 12 條
    // 兩個動作現在都預設執行：clarnData 從 20260926 第 12 條起比照 golden（原本預設預覽，與 counter.clear 刻意不同）；
    //   counter.clear 是既有指令，本來就預設執行。差別只剩「型別不對」：clarnData 當預覽，counterClear 當執行（:265-273）。
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
        w.Key("dryRun").String("bool，預設 false（直接執行＝golden；要預覽帶 dryRun:true；型別不對視為預覽）");
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
        WriteTesterConnectActionSchema(w);   // AI(W906-GB-P2e) 20260926: act.main.testerConnect 的 schema 物件
        WriteClearRecordActionSchema(w);     // AI(W906-S119) 20260927: act.main.clearRecord 的 schema 物件
        WriteMeShuttle2DblActionSchema(w);   // AI(W906-S119) 20260927: act.main.meShuttle2Dbl 的 schema 物件
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

// ===========================================================================
//  Steven 20260925 (W906-CC-PAGE)：Data.CounterClear.html 的 golden 表單橋。
//  NOT in golden —— golden 的 TfCounterClear 是 VCL 對話框，這裡是把同一個
//  fCounterClear 實例（cCounterClear.cpp，已逐行翻譯）的狀態交給網頁顯示。
//
//  三個 WS 指令（tools/wb_serve.cpp 主迴圈分派、持 FormLock）：
//    counterclear.get                  -> golden FormShow（V912 cCounterClear.cpp:92-114）後的畫面狀態
//    counterclear.click tag=<cb 名稱>  -> 網頁勾選框被按：設 Checked 後呼叫 golden 的 OnMouseUp
//                                         （cbSelectAll -> cbSelectAllMouseUp :50-77；
//                                           其他 8 個 -> cbAlarmDataMouseUp :79-88，dfm :203-308 八個共用）
//    counterclear.exe                  -> golden spbExeClick（:399-455）本體：勾幾個清幾類，
//                                         前後只有一對 Clarn_Data(10)（:426、:452）
//
//  為什麼不擴充 act.counterClear.exe：它是「一次一個 family」的既有動作，
//  有 e2e 跑過而且別名 counter.clear 仍有人送；它自己組 Clarn_Data＋ClearCount，
//  不是呼叫翻譯過的 spbExeClick。多選要照 golden 的順序與單一一對 Clarn_Data，
//  最忠實的做法是直接呼叫 fCounterClear->spbExeClick()（翻譯本體），
//  而那需要伺服器端的 checkbox 狀態 —— 也就是 FormShow／MouseUp 這一整組。
//  act.* 分派不持 FormLock，這組要持（spbExeClick 會寫 LastSet 與 lastdata.dat，
//  /api/form 的 HTTP 執行緒同時在讀），所以放在 wb_serve 自己的分派臂。
// ===========================================================================
namespace {

struct CcBox {
    const char* id;
    TCheckBox* TfCounterClear::* member;
    int authIdx;                  // authCounterClr[] 索引（golden FormShow :94-101）；-1 = cbSelectAll
    const char* dfmCaption;       // V912 cCounterClear.dfm 的 Caption
};

const CcBox kCcBoxes[] = {
    { "cbAlarmData",        &TfCounterClear::cbAlarmData,        0, "Alarm and Lot Data" },        // dfm :195
    { "cbTestCategory",     &TfCounterClear::cbTestCategory,     1, "Tester Category" },           // dfm :210
    { "cbScanner",          &TfCounterClear::cbScanner,          2, "Scanner Category" },          // dfm :225
    { "cbLoadingCount",     &TfCounterClear::cbLoadingCount,     3, "Loading Count" },             // dfm :240
    { "cbContactCountCurr", &TfCounterClear::cbContactCountCurr, 4, "Contact Count (Current)" },   // dfm :255
    { "cbContactCountHis",  &TfCounterClear::cbContactCountHis,  5, "Contact Count (History)" },   // dfm :270
    { "cbSortingCount",     &TfCounterClear::cbSortingCount,     6, "Sorting Count" },             // dfm :285
    { "cbTimeData",         &TfCounterClear::cbTimeData,         7, "Time Data" },                 // dfm :300
    { "cbSelectAll",        &TfCounterClear::cbSelectAll,       -1, "Select All" },                // dfm :315
};
const std::size_t kCcBoxCount = sizeof(kCcBoxes) / sizeof(kCcBoxes[0]);

const CcBox* CcFind(const std::string& id)
{
    for (std::size_t i = 0; i < kCcBoxCount; ++i)
        if (id == kCcBoxes[i].id) return &kCcBoxes[i];
    return 0;
}

TCheckBox* CcWidget(const CcBox& b) { return fCounterClear->*(b.member); }

// VCL 載入 dfm 時給的初值。移植的 TControl 預設 Enabled=false、Caption=""
// （vclcompat/Controls.h:256、:415），真 VCL 預設 Enabled=true，Caption 取 dfm。
// golden FormShow 只設 8 個清除框的 Enabled，cbSelectAll 永遠是 dfm 的預設 Enabled=true。
// 只做一次：之後 Caption 由 golden cbSelectAllMouseUp／cbAlarmDataMouseUp 改。
void CcSeedFromDfm()
{
    static bool seeded = false;
    if (seeded) return;
    seeded = true;
    for (std::size_t i = 0; i < kCcBoxCount; ++i) {
        TCheckBox* cb = CcWidget(kCcBoxes[i]);
        cb->Caption = kCcBoxes[i].dfmCaption;
        cb->Enabled = true;
        cb->Visible = true;
        // dfm 沒有任何一個 Checked = True（V912 cCounterClear.dfm 全檔），維持 false
    }
    fCounterClear->spbExe->Enabled = true;
    fCounterClear->spbExe->Visible = true;
}

long long CcSumArmBins(TArm* a)
{
    long long s = 0;
    for (int k = 0; k < iTestBinCount && k < TEST_MAX_BIN; ++k) s += (long long)a->GetSelBin(k);
    return s;
}

// 各清除類別「被清掉的值」的摘要，給網頁與探針核對「清的是這一類、別類不動」。
// 只讀記憶體；欄位清單就是 golden ClearCount 各 case 寫的那些（V912 cCounterClear.cpp:116-397）。
void CcWriteCounters(webbridge::JsonWriter& w)
{
    w.Key("counters").BeginObject();

    w.Key("testerCategory").BeginObject();                                    // case ctTesterCategory :124-155
    w.Key("armBinSum").Number((wb_int64)(CcSumArmBins(ArmData[0]) + CcSumArmBins(ArmData[1]) +
                                         CcSumArmBins(ArmData[2])));
    w.EndObject();

    w.Key("loadingCount").BeginObject();                                      // case ctLoadingCounts :156-176 ＋ ctIndexCount :268-273
    w.Key("SendCT").BeginArray();
    for (int i = 0; i < 4; ++i) w.Number((wb_int64)LastSet.SendCT[i]);
    w.EndArray();
    w.Key("iJamCount").BeginArray();
    for (int i = 0; i < 2; ++i) w.Number((wb_int64)LastSet.iJamCount[i]);
    w.EndArray();
    w.Key("iIndexCount").Number((wb_int64)LastSet.iIndexCount);
    w.Key("iIndexInputOutPut").BeginArray();
    for (int i = 0; i < 4; ++i) w.Number((wb_int64)LastSet.iIndexInputOutPut[i]);
    w.EndArray();
    w.EndObject();

    w.Key("contactCurr").BeginObject();                                       // case ctContactCounts :177-191
    w.Key("pass").Number((wb_int64)((long long)ArmData[0]->GetPassCT() + ArmData[1]->GetPassCT() + ArmData[2]->GetPassCT()));
    w.Key("fail").Number((wb_int64)((long long)ArmData[0]->GetFailCT() + ArmData[1]->GetFailCT() + ArmData[2]->GetFailCT()));
    w.EndObject();

    w.Key("contactHis").BeginObject();                                        // case ctContactCountsHis :192-206
    w.Key("pass").Number((wb_int64)((long long)ArmHistory[0]->GetPassCT() + ArmHistory[1]->GetPassCT() + ArmHistory[2]->GetPassCT()));
    w.Key("fail").Number((wb_int64)((long long)ArmHistory[0]->GetFailCT() + ArmHistory[1]->GetFailCT() + ArmHistory[2]->GetFailCT()));
    w.EndObject();

    w.Key("sortingCount").BeginObject();                                      // case ctTraySortCount :207-245
    long long bin0 = 0, bin23 = 0, data32 = 0;
    for (int i = 0; i < eTrayCount; ++i) {
        bin0  += LastSet.BinCT[0][iTo3Unload[i]];
        bin23 += (long long)LastSet.BinCT[2][iTo3Unload[i]] + LastSet.BinCT[3][iTo3Unload[i]];
    }
    for (int i = 0; i < TEST_MAX_BIN; ++i)
        data32 += (long long)LastSet.iBinData32[0][i] + LastSet.iBinData32[2][i] + LastSet.iBinData32[3][i];
    w.Key("BinCT0").Number((wb_int64)bin0);
    w.Key("BinCT23").Number((wb_int64)bin23);
    w.Key("iBinData32").Number((wb_int64)data32);
    w.EndObject();

    w.Key("timeData").BeginObject();                                          // case ctTimeData :246-253
    long long acc = 0;
    for (int k = 0; k < 2; ++k)
        for (int i = 0; i < 8; ++i) acc += LastSet.SystemAccSecond[k][i];
    w.Key("SystemAccSecond01").Number((wb_int64)acc);
    w.EndObject();

    w.EndObject();
}

void CcWriteState(webbridge::JsonWriter& w)
{
    w.Key("form").String("fCounterClear");
    w.Key("golden").String("V912 cCounterClear.cpp TfCounterClear");
    w.Key("auth").BeginArray();
    for (int i = 0; i < 8; ++i) w.Bool(authCounterClr[i]);
    w.EndArray();
    w.Key("widgets").BeginObject();
    for (std::size_t i = 0; i < kCcBoxCount; ++i) {
        const TCheckBox* cb = CcWidget(kCcBoxes[i]);
        w.Key(kCcBoxes[i].id).BeginObject();
        w.Key("enabled").Bool(cb->Enabled);
        w.Key("checked").Bool(cb->Checked);
        w.Key("caption").String(cb->Caption.c_str());
        w.EndObject();
    }
    w.Key("spbExe").BeginObject();
    w.Key("enabled").Bool(fCounterClear->spbExe->Enabled);
    w.EndObject();
    w.EndObject();
    CcWriteCounters(w);
    w.Key("customerSkipped").BeginArray();
    w.String("spbExeClick CC_KYEC_LEE 重新刷卡／權限分支（golden :401-423）—— 翻譯核心 GATE CC3 (#if 0)，客戶專屬，本次跳過");
    w.EndArray();
}

// 把網頁送來的勾選同步到 fCounterClear。disabled 的框在 golden 無法被勾（VCL 不送滑鼠事件），
// 所以只有「值要變」且框是 disabled 時才拒絕；值沒變就不算操作。
bool CcApplyChecked(const cJSON* checked, std::string* bad)
{
    if (checked == 0 || !cJSON_IsObject(checked)) return true;
    for (std::size_t i = 0; i < kCcBoxCount; ++i) {
        if (kCcBoxes[i].authIdx < 0) continue;                                 // cbSelectAll 不影響清除，交給 click
        const cJSON* v = cJSON_GetObjectItemCaseSensitive((cJSON*)checked, kCcBoxes[i].id);
        if (v == 0 || !cJSON_IsBool(v)) continue;
        TCheckBox* cb = CcWidget(kCcBoxes[i]);
        const bool want = cJSON_IsTrue(v) != 0;
        if (want == cb->Checked) continue;
        if (!cb->Enabled) { *bad = kCcBoxes[i].id; return false; }
        cb->Checked = want;
    }
    return true;
}

}  // namespace

std::string W906_CounterClearGet()
{
    CcSeedFromDfm();
    fCounterClear->FormShow(0);                                                // golden :92-114（內含 GetCountClrAuth()）
    webbridge::JsonWriter w;
    w.BeginObject();
    CcWriteState(w);
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::string W906_CounterClearClick(const std::string& tag, const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    *ok = false;
    CcSeedFromDfm();
    const CcBox* b = CcFind(tag);
    if (b == 0) { ht9045::sjson::WriteRefusal(w, "unknown-widget", tag.c_str()); return w.Str(); }
    cJSON* root = cJSON_Parse(payloadJson.c_str());
    const cJSON* v = root ? cJSON_GetObjectItemCaseSensitive(root, "checked") : 0;
    if (v == 0 || !cJSON_IsBool(v)) {
        if (root) cJSON_Delete(root);
        ht9045::sjson::WriteRefusal(w, "bad-payload", "value 要是 {\"checked\":true|false}");
        return w.Str();
    }
    const bool want = cJSON_IsTrue(v) != 0;
    cJSON_Delete(root);
    TCheckBox* cb = CcWidget(*b);
    if (!cb->Enabled) { ht9045::sjson::WriteRefusal(w, "not-enabled", b->id); return w.Str(); }
    cb->Checked = want;                                                        // VCL：OnMouseUp 之前 Checked 已切換
    if (b->authIdx < 0) fCounterClear->cbSelectAllMouseUp(cb);                 // golden :50-77
    else                fCounterClear->cbAlarmDataMouseUp(cb);                 // golden :79-88
    *ok = true;
    w.BeginObject();
    CcWriteState(w);
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::string W906_CounterClearExe(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    *ok = false;
    CcSeedFromDfm();
    bool dry = false;
    std::string bad;
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (root == 0) { ht9045::sjson::WriteRefusal(w, "bad-payload", "value 不是合法 JSON"); return w.Str(); }
    const cJSON* jd = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
    if (jd && cJSON_IsBool(jd) && cJSON_IsTrue(jd)) dry = true;               // 與 act.counterClear.exe 同：明確 true 才 dry
    const bool applied = CcApplyChecked(cJSON_GetObjectItemCaseSensitive(root, "checked"), &bad);
    cJSON_Delete(root);
    if (!applied) { ht9045::sjson::WriteRefusal(w, "not-enabled", bad.c_str()); return w.Str(); }

    // 不信任前端：執行前在 C++ 端重讀一次權限（golden spbExeClick 本身不重讀，
    // 它靠 FormShow 把未授權的框清掉並停用 —— 這裡是同一條規則的伺服器端複查）。
    GetCountClrAuth();
    for (std::size_t i = 0; i < kCcBoxCount; ++i) {
        const CcBox& b = kCcBoxes[i];
        if (b.authIdx >= 0 && CcWidget(b)->Checked && !authCounterClr[b.authIdx]) {
            ht9045::sjson::WriteRefusal(w, "not-authorized", b.id);
            return w.Str();
        }
    }

    TfCounterClear* f = fCounterClear;
    if (dry) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("dryRun").Bool(true);
        w.Key("would").BeginArray();                                           // golden spbExeClick :425-453 的順序
        w.String("MyDBIProductionData(\"Clear Count executed\")");
        w.String("Clarn_Data(10, \"Manual clear count\")");
        if (f->cbAlarmData->Checked)        w.String("ClearCount(ctAlarmData)");
        if (f->cbTestCategory->Checked)     w.String("ClearCount(ctTesterCategory)");
        if (f->cbLoadingCount->Checked)   { w.String("ClearCount(ctLoadingCounts)"); w.String("ClearCount(ctIndexCount)"); }
        if (f->cbContactCountCurr->Checked) w.String("ClearCount(ctContactCounts)");
        if (f->cbContactCountHis->Checked)  w.String("ClearCount(ctContactCountsHis)");
        if (f->cbSortingCount->Checked)     w.String("ClearCount(ctTraySortCount)");
        if (f->cbTimeData->Checked)         w.String("ClearCount(ctTimeData)");
        w.String("Clarn_Data(10, \"Manual clear count done\")");
        w.String("MyDBIProcess(\"Process\", \"Counter Clear has been executed!!\")");
        w.EndArray();
        CcWriteState(w);
        w.EndObject();
        *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
    }

    f->spbExeClick(f->spbExe);                                                 // golden :399-455（翻譯本體，cCounterClear.cpp）
    std::printf("counterclear.exe done (SendCT[0]=%d iIndexCount=%d)\n",
                (int)LastSet.SendCT[0], LastSet.iIndexCount);
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("writes").BeginArray();
    w.String("D:\\HT9045\\system\\lastdata.dat ＋ lastdata_backup.dat（Clarn_Data -> WriteLastDataFile）");
    w.String("D:\\HT9045_Log\\QtyData\\YYYYMM\\<PC>_<Lot>_<date>.csv（Clarn_Data 的 QtyLog，第二次呼叫起寫）");  if (f->cbTestCategory->Checked) w.String("D:\\HT9045\\system\\BinCount.txt 刪除（ClearCount(ctTesterCategory)，golden cCounterClear.cpp:146-152；GATE CC1 已開）");  if (bRefreshCount) w.String("下一拍：D:\\HT9045\\system\\Arm*.dat／ArmHis*.dat／ArmByLot*.dat（＋_backup.dat）整份覆寫（bRefreshCount → golden Timer10Timer main.cpp:35335 WriteCTInfo）");   // AI(W906-PROD-S111) 20260926：同一行，不移動行號
    w.EndArray();
    CcWriteState(w);
    w.EndObject();
    *ok = true;
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}
