// ===========================================================================
//  JsonBridge/EventLog.cpp
//
//  AI(W906-JSONBRIDGE-S1) 20260923.  NOT in golden.
// ===========================================================================
#include "JsonBridge/EventLog.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"
#include "WebBridge/TagValue.h"
#include "vclcompat/vcl_compat.h"   // AnsiString（cMyDB.h 的簽章要它）
#include "cMyDB.h"                  // RecordProcess / NewRecordProcess / RecordChangeLogProcess

namespace ht9045 {
namespace sjson {

namespace {

// ring 容量。200 是規格給的（SKILL.md 4.7）。挑這個數的理由：一次操作
// 通常產生 1～3 筆，200 足夠涵蓋操作員「剛才做了什麼」的範圍，而且
// 全部序列化出去約 20～30 KB，還在一個 HTTP 回應合理的大小內。
const std::size_t kCapacity = 200;

std::vector<LogEntry> g_ring;          // 最多 kCapacity 筆，最舊的在前
unsigned long long    g_seq = 0;       // 發出去的最後一個序號
unsigned long long    g_total = 0;     // 總共記過幾筆
unsigned long long    g_dropped = 0;   // 被環形蓋掉幾筆

std::string NowLocal() {
    // ⚠ 不要用 localtime_s：MinGW（這棵樹的主 oracle）沒有它，實測
    //   「'localtime_s' was not declared in this scope」。std::localtime 的
    //   執行緒安全問題在這裡不存在 —— 這個檔全程單執行緒（見標頭 THREADING）。
    const std::time_t t = std::time(0);
    const std::tm* p = std::localtime(&t);
    if (p == 0) return std::string("1970-01-01 00:00:00");
    const std::tm lt = *p;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
                  lt.tm_hour, lt.tm_min, lt.tm_sec);
    return std::string(buf);
}

const char* KindTable(LogKind k) {
    // golden 的表名。MyDBIProcess 的 asTable 同時是表名與欄名
    // （INSERT INTO Process(Process, Debug, OccurDateTime)），所以這個字串
    // 不是顯示用的標籤，改了會改到 SQL。
    return (k == kLogChange) ? "ChangeLog" : "Process";
}

const char* KindName(LogKind k) {
    if (k == kLogAlarm)  return "alarm";
    if (k == kLogChange) return "change";
    return "process";
}

// cJSON 取字串欄位；沒有就回預設。
std::string JStr(const cJSON* o, const char* key, const char* def) {
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, key);
    if (v != 0 && cJSON_IsString(v) && v->valuestring != 0)
        return std::string(v->valuestring);
    return std::string(def);
}

}  // namespace

unsigned long long LogAppend(LogKind kind,
                             const std::string& msg,
                             const std::string& debug,
                             const std::string& code,
                             const std::string& origin) {
    LogEntry e;
    e.seq    = ++g_seq;
    e.at     = NowLocal();
    e.kind   = kind;
    e.code   = code;
    e.msg    = msg;
    e.debug  = debug;
    e.origin = origin;
    e.sinks  = kSinkRing;

    // ---- 送進 golden 的入口 --------------------------------------------
    //
    // ⚠ 這是重點：ring 不取代 golden，它**同時**發生。哪一天 cMyDB 的
    //   homecoming gate 拆掉、DB 開起來，這幾行一個字都不用改，同一筆就
    //   會自動多一個去向。
    //
    // ⚠ AI(W906-CMYDB-P4) 20260927 (St02-E) 起三個入口都是 golden 本體（cMyDB.cpp），替身已刪。sinks 記的是
    //   「我們知道它去了哪」，不是「它一定到了」：golden 本體寫 EventLogTxt（slEventLog 有建時）／
    //   HANDLER LOG／EventTracker，不 printf，所以 kSinkStdout 不再設（以前只有
    //   canary_support.cpp:116 的 RecordProcess 替身會印；NewRecordProcess 的替身
    //   acatchtray_shims.cpp:152 是空的）。現在三條都是 ring + golden。這個差別要
    //   看得見，不能一律標成「已送出」。
    const AnsiString aMsg(msg.c_str());
    const AnsiString aDbg(debug.c_str());
    switch (kind) {
        case kLogAlarm:
            NewRecordProcess(AnsiString(code.c_str()), aMsg, aDbg);
            e.sinks |= kSinkGolden;                       // 空 body，不加 stdout
            break;
        case kLogChange:
            RecordChangeLogProcess(aMsg, aDbg);
            e.sinks |= kSinkGolden;                       // 真本體，但落到 MyDBIProcess
            break;
        case kLogProcess:
        default:
            RecordProcess(aMsg, aDbg);
            e.sinks |= kSinkGolden;                       // AI(W906-CMYDB-P4) 20260927 (St02-E): golden 本體不印 stdout（canary 替身已刪）
            break;
    }
    // kSinkDb 永遠不設：wb_serve 從不呼叫 MyDBOpenDB()（20260923 實測）。

    if (g_ring.size() >= kCapacity) {
        g_ring.erase(g_ring.begin());
        ++g_dropped;
    }
    g_ring.push_back(e);
    ++g_total;
    return e.seq;
}

std::string EventLogTailJson(unsigned long long since, std::size_t maxEntries) {
    if (maxEntries == 0 || maxEntries > kCapacity) maxEntries = kCapacity;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("seq").Number((wb_int64)g_seq);
    w.Key("count").Number((wb_int64)g_total);
    // dropped 一定要送：不送的話瀏覽器分不出「這段時間沒事」和「事情多到被蓋掉」。
    w.Key("dropped").Number((wb_int64)g_dropped);
    w.Key("capacity").Number((wb_int64)kCapacity);
    w.Key("db").Bool(false);          // 見標頭的 ⚠

    // 先數出要回幾筆，再從那裡開始 —— 避免先塞再截斷。
    std::size_t first = 0;
    for (std::size_t i = 0; i < g_ring.size(); ++i) {
        if (g_ring[i].seq > since) { first = i; break; }
        first = g_ring.size();
    }
    if (g_ring.size() - first > maxEntries)
        first = g_ring.size() - maxEntries;

    w.Key("entries").BeginArray();
    for (std::size_t i = first; i < g_ring.size(); ++i) {
        const LogEntry& e = g_ring[i];
        w.BeginObject();
        w.Key("seq").Number((wb_int64)e.seq);
        w.Key("at").String(e.at);
        w.Key("kind").String(KindName(e.kind));
        w.Key("table").String(KindTable(e.kind));
        if (!e.code.empty()) w.Key("code").String(e.code);
        w.Key("msg").String(e.msg);
        if (!e.debug.empty()) w.Key("debug").String(e.debug);
        w.Key("origin").String(e.origin);
        w.Key("sinks").BeginArray();
        if (e.sinks & kSinkRing)   w.String("ring");
        if (e.sinks & kSinkGolden) w.String("golden");
        if (e.sinks & kSinkStdout) w.String("stdout");
        if (e.sinks & kSinkDb)     w.String("db");
        w.EndArray();
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::size_t PublishEventLogTags(webbridge::TagSnapshot& snap) {
    snap.stage("log.seq",     webbridge::TagValue::makeInt((std::int64_t)g_seq));
    snap.stage("log.count",   webbridge::TagValue::makeInt((std::int64_t)g_total));
    snap.stage("log.dropped", webbridge::TagValue::makeInt((std::int64_t)g_dropped));
    // ⚠ false 是**量出來的事實**不是佔位：wb_serve 從不呼叫 MyDBOpenDB()。
    //   哪天有人開了 DB，這裡要跟著變成真的查詢，不要留一個寫死的 false。
    snap.stage("log.db",      webbridge::TagValue::makeBool(false));
    return 4;
}

std::string HandleLogEvent(const std::string& payloadJson) {
    // ⚠ 回應裡不寫 ok：AckJson（WebBridgeServer.cpp:1262）成功時已經寫了一個，
    //   再把這個物件原地拼接進來。兩邊都寫會變成重複鍵。
    //   改用 accepted，而 wb_serve 也改成看這個鍵。
    webbridge::JsonWriter w;

    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (root == 0) {
        w.BeginObject();
        w.Key("accepted").Bool(false);
        w.Key("reason").String("log.event value is not JSON");
        w.EndObject();
        return w.Str();
    }

    const std::string kindStr = JStr(root, "kind", "process");
    const std::string msg     = JStr(root, "msg", "");
    std::string       debug   = JStr(root, "debug", "");
    const std::string code    = JStr(root, "alarmCode", "");
    const std::string page    = JStr(root, "page", "");
    const std::string ctrl    = JStr(root, "ctrl", "");

    LogKind kind = kLogProcess;
    if      (kindStr == "alarm")  kind = kLogAlarm;
    else if (kindStr == "change") kind = kLogChange;
    else if (kindStr != "process") {
        cJSON_Delete(root);
        w.BeginObject();
        w.Key("accepted").Bool(false);
        w.Key("reason").String("kind must be process | alarm | change");
        w.EndObject();
        return w.Str();
    }

    // 空訊息一律拒。golden 的去重是拿 S 跟上一筆比，空字串會讓那條規則
    // 變成「第一筆之後全部吞掉」—— 收下它比拒絕它糟。
    if (msg.empty()) {
        cJSON_Delete(root);
        w.BeginObject();
        w.Key("accepted").Bool(false);
        w.Key("reason").String("msg is required and must not be empty");
        w.EndObject();
        return w.Str();
    }
    if (kind == kLogAlarm && code.empty()) {
        cJSON_Delete(root);
        w.BeginObject();
        w.Key("accepted").Bool(false);
        w.Key("reason").String("kind=alarm requires alarmCode");
        w.EndObject();
        return w.Str();
    }

    // page/ctrl 接在 debug 尾端，不改 golden 的 schema。
    if (!page.empty() || !ctrl.empty()) {
        if (!debug.empty()) debug += " ";
        debug += "[";
        debug += page.empty() ? "?" : page;
        debug += "/";
        debug += ctrl.empty() ? "?" : ctrl;
        debug += "]";
    }
    cJSON_Delete(root);

    const unsigned long long seq = LogAppend(kind, msg, debug, code, "html");
    const unsigned sinks = g_ring.empty() ? 0u : g_ring.back().sinks;

    w.BeginObject();
    w.Key("accepted").Bool(true);
    w.Key("seq").Number((wb_int64)seq);
    w.Key("sinks").BeginArray();
    if (sinks & kSinkRing)   w.String("ring");
    if (sinks & kSinkGolden) w.String("golden");
    if (sinks & kSinkStdout) w.String("stdout");
    if (sinks & kSinkDb)     w.String("db");
    w.EndArray();
    w.Key("db").Bool(false);
    w.EndObject();
    return w.Str();
}

void EventLogResetForTest() {
    g_ring.clear();
    g_seq = 0;
    g_total = 0;
    g_dropped = 0;
}

}  // namespace sjson
}  // namespace ht9045
