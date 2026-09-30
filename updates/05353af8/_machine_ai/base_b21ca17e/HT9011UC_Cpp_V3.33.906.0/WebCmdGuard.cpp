// =============================================================================
//  WebCmdGuard.cpp -- 伺服器端防連點（S107-3）。設計與判定式見 WebCmdGuard.h 檔頭。
//
//  AI(W906-CMDGUARD) 20260926: NEW-BUILD 基礎建設。只依賴 WebCommand／TagValue 與 cJSON，
//    沒有機台碼；-Wall -Wextra 乾淨。測試：tests/test_webcmdguard.cpp（ctest WebCmdGuard）。
//
//  預設：除了下面兩張白名單以外**全部擋**（含 dryRun 預覽 —— 擋下重複的預覽剛好阻止
//  A 路存檔跳第二個確認框）。之後新加的指令（例如 hw.access）自動受保護；按住型／
//  連續送的指令由提出的人加進白名單。
//
//  名稱級白名單（整條指令不受保護）
//    基礎設施      sys.ping、cfg.resync、log.event、ui.windows.put、stream.resync
//    回答          modal.answer、dialog.response、dialog.auth（qid 對不上本來就拒）
//    連續操作／輸出 motor.stop、io.btnPanelClick、pci1203.*、sim.di.set
//    停機方向      pause.run
//    純讀          editlist.get、contactct.get、counterclear.get、auth.mode（observer.get 20260927 起改到 op 級，見下表）
//    olp.*（Jimmy 20260926 18:4x 確定放行）
//    AI(W906-R0927-8) 20260927: motor.access 移到下面的 op 級表（RULINGS_20260927 第 2 條第 8 題 A：
//      jog／stop 放行，move／home／servo／power 400 ms 內重複的擋）。
//    AI(W906-R0927-9) 20260927: io.btnPanelClick 留在這張表，但**不是不擋**：它改由 W906_DispatchIoClick
//      第一行的 W906IoClickGuardScope 擋（同一顆鈕＋同一個 down 值才擋，down 跟上一次不同一律放行；第 9 題 A
//      ＋Jimmy 08:3x 條件，見 WebCmdGuard.h 最後一節）—— 主分派與輸出優先（W906_ServiceOutputs）兩條路都在
//      那裡執行。留在這裡是為了主分派那條不被擋兩次（主 guard 的 key 用 value 原位元組，1 與 1.0、true 會是
//      不同 key；IO 鈕的 key 用正規化後的 down）。
//
//  op 級白名單（解析 value JSON 的 op 或 act 欄位；只有列出的 op 放行，其他 op 照擋）
//    指令                   欄位  欄位缺時（照 C++ 本體的預設）  放行的 op／act
//    security.jam           op    ""                           open、stats、exportStatus、exportChunk、exportRelease
//    security.passwd        op    ""                           state、list、open
//    lotinfo.op             op    ""                           testerLog.get、selection.get
//    towerlight.op          op    ""                           get
//    recipe.change          op    ""                           list
//    act.main.peModel       op    "get"（FileRW/MainClick.cpp） get
//    act.sortCT.clearCount  op    ""（空 = Clear Count 本身）   lotId（golden Timer1Timer，將來輪詢用）
//    smartdiag.op           act   "open"（WebSmartDiag.cpp）     open、timer
//    builder.op             act   "state"（WebBuilder.cpp）      open、state、change、dir、drive、close
//    observer.get           act   "open"（wb_serve oact 預設）      open、""、timer、tab、rowNo、form、year、month、file、filter、query、ccKinds*、ccHistory*（讀）；yieldSite／yieldMax／yieldMin／yieldClear 擋（AI(W906-PROD-S116Y) 20260927）
//    motor.access           action ""（缺 action 本體拒絕，        jogP、jogN（按住型：按下送一次、放開走 motor.stop）、stop、
//                                   WebMotorAccess.cpp:4108）      setSpeed（速度捲軸拖動時連續送、值會來回；擋了會讓 C++ 停在中間值）、
//                                                                  setPos1、setPos2、refreshParameter、setTeachFromCurrent、setTeachFromOffset
//                                                                  （kActions 標 "ui"：C++ 無動作，:4147）；
//                                                                  ＋抬起方向：home／loopMove 帶 params.start=false（DoHome :1297 golden else 支停止歸零、
//                                                                  DoLoopMove :1498 停止來回；MotorTestStopDirection :3562 讓它們跟 STOP 一樣過運轉閘）。
//                                                                  其他全部擋：moveRelative、moveAbsolute、moveSoftLimitP／N、home／loopMove（start=true）、
//                                                                  servoToggle、motorPowerToggle、teachSet、teachGo、lightScale*、setRangeAndInit、
//                                                                  setRateAndInit、reloadMotorData、resetMNet、selectMotor、setParamCell、copyFrom、
//                                                                  set*Speed／setSoftLimit*、formShow、formClose（AI(W906-R0927-8) 20260927，37 個 action
//                                                                  全部對過 WebMotorAccess.cpp:50-96 kActions）
//      ⚠ motor.access 的 key 不是 value 原字串：先拿掉每按一下都會變的欄位（kMotorAccessStrip）。
//    value 不是 JSON、或欄位存在但不是字串 ⇒ 本體會拒絕 ⇒ 當成「不在白名單」（擋重複的
//    拒絕沒有壞處）。欄位名稱與預設值 20260926 逐一讀過各本體（WebSecurityJam.cpp:709、
//    WebLogin.cpp:841、WebLotInfo.cpp:181、WebTowerLight.cpp:216、WebRecipeChange.cpp:626、
//    FileRW/MainClick.cpp:185/193、WebSortCT.cpp:354、WebSmartDiag.cpp:239/248、WebBuilder.cpp:423/429；
//    motor.access：WebMotorAccess.cpp:4043-4110 MotorAccessParse，20260927）。
// =============================================================================
#include "WebCmdGuard.h"

#include "Public/cJSON.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ht9045 {

// 類別內有初值的 static const 成員：補上類別外定義，被 odr-use 時（例如綁到 const&）
// MinGW 6.3（沒有 C++17 inline variable）也連得起來。
const std::uint64_t WebCmdGuard::kDefaultWindowMs;
const std::uint64_t WebCmdGuard::kMaxWindowMs;
const std::size_t   WebCmdGuard::kPurgeAbove;
const std::uint64_t WebCmdGuard::kPurgeAgeUs;
const std::uint64_t WebCmdGuard::kPurgeEveryUs;
const std::size_t   WebCmdGuard::kHardCap;

namespace {

// ---- 名稱級白名單 -----------------------------------------------------------------
const char* const kExemptNames[] = {
    // 基礎設施
    "sys.ping", "cfg.resync", "log.event", "ui.windows.put", "stream.resync",
    // 回答
    "modal.answer", "dialog.response", "dialog.auth",
    // 連續操作與輸出（AI(W906-R0927-8) 20260927: motor.access 移到 op 級；io.btnPanelClick 由 W906IoClickGuardScope 擋）
    "motor.stop", "io.btnPanelClick", "sim.di.set",
    // 停機方向
    "pause.run",
    // 純讀
    "editlist.get", "contactct.get", "counterclear.get", "auth.mode",   // AI(W906-PROD-S116Y) 20260927: observer.get 改到下面的 op 級表（Yield 分頁的四個 act 會改記憶體狀態）
};
const char* const kExemptPrefixes[] = {
    "pci1203.",   // 連續操作與輸出（EastSun）
    "olp.",       // Jimmy 20260926 18:4x 確定放行（setter 冪等）
};

// ---- op 級白名單 ------------------------------------------------------------------
struct OpRule {
    const char*        cmd;
    const char*        field;   // "op" 或 "act"
    const char*        dflt;    // 欄位缺時本體用的值
    const char* const* ops;     // 放行的值，0 結尾
};
const char* const kJamOps[]       = { "open", "stats", "exportStatus", "exportChunk", "exportRelease", 0 };
const char* const kPasswdOps[]    = { "state", "list", "open", 0 };
const char* const kLotInfoOps[]   = { "testerLog.get", "selection.get", 0 };
const char* const kTowerOps[]     = { "get", 0 };
const char* const kRecipeChgOps[] = { "list", 0 };
const char* const kPeModelOps[]   = { "get", 0 };
const char* const kSortCTOps[]    = { "lotId", 0 };
const char* const kSmartDiagOps[] = { "open", "timer", 0 };
const char* const kBuilderOps[]   = { "open", "state", "change", "dir", "drive", "close", 0 };
const char* const kObserverActs[] = { "", "open", "timer", "tab", "rowNo", "form", "year", "month", "file", "filter", "query",
                                      "ccKinds", "ccKindsForm", "ccHistory", "ccHistoryForm", 0 };   // AI(W906-PROD-S116Y) 20260927: 讀取型 act（cObserver.cpp W906_ObserverJson；"" 本體也當 open）；yieldSite／yieldMax／yieldMin／yieldClear 不列 ⇒ 擋連點
//AI(W906-R0927-8) 20260927: 放行的 motor.access action（檔頭表的理由）；home／loopMove 的抬起方向另見 MotorAccessReleaseDir
const char* const kMotorAccessActs[] = { "jogP", "jogN", "stop", "setSpeed",
                                         "setPos1", "setPos2", "refreshParameter", "setTeachFromCurrent", "setTeachFromOffset", 0 };
const OpRule kOpRules[] = {
    { "security.jam",          "op",  "",      kJamOps       },
    { "security.passwd",       "op",  "",      kPasswdOps    },
    { "lotinfo.op",            "op",  "",      kLotInfoOps   },
    { "towerlight.op",         "op",  "",      kTowerOps     },
    { "recipe.change",         "op",  "",      kRecipeChgOps },
    { "act.main.peModel",      "op",  "get",   kPeModelOps   },
    { "act.sortCT.clearCount", "op",  "",      kSortCTOps    },
    { "smartdiag.op",          "act", "open",  kSmartDiagOps },
    { "builder.op",            "act", "state", kBuilderOps   },
    { "observer.get",          "act", "open",  kObserverActs },   // AI(W906-PROD-S116Y) 20260927: act 缺＝open（wb_serve oact 預設）
    { "motor.access",          "action", "",   kMotorAccessActs },   // AI(W906-R0927-8) 20260927: 缺 action＝本體拒絕（WebMotorAccess.cpp:4108）⇒ "" 不在名單
};

// ---- motor.access（AI(W906-R0927-8) 20260927）-------------------------------------
// value 是 motor-access.js 組好的整個 request（web/page/motor-access.js send()／stopReq()，
// ht9045_recipe_client.js motorAccess：value = JSON.stringify(req)）。

// 抬起方向：home／loopMove 帶 params.start=false（JSON 的 false；本體 MotorAccessParse 只收 bool）。
// 本體 DoHome :1297（golden else 支：停止歸零）、DoLoopMove :1498（停止來回）；
// MotorTestStopDirection :3562 讓它們跟 STOP 一樣過運轉閘。擋它只會讓「停」晚 400 ms，而且頁面收到
// 錯誤會把 HOME／LoopMove 鈕畫成抬起（HW.MotorTest.html onAck），跟 C++ 還在跑的工作分岔。
bool MotorAccessReleaseDir(const webbridge::WebCommand& wc)
{
    if (!wc.hasValue || !wc.value.isString()) return false;
    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
    if (root == 0) return false;
    bool rel = false;
    if (cJSON_IsObject(root)) {
        const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "action");
        const cJSON* jp = cJSON_GetObjectItemCaseSensitive(root, "params");
        if (cJSON_IsString(ja) && ja->valuestring &&
            (std::strcmp(ja->valuestring, "home") == 0 || std::strcmp(ja->valuestring, "loopMove") == 0) &&
            cJSON_IsObject(jp)) {
            const cJSON* js = cJSON_GetObjectItemCaseSensitive(jp, "start");
            rel = cJSON_IsFalse(js) != 0;
        }
    }
    cJSON_Delete(root);
    return rel;
}

// 算 key 之前拿掉的欄位：每按一下都會變、但不是操作員要的東西。不拿掉的話同一顆鈕連點兩下永遠是
// 不同 key（seq 每下 +1），這條規則等於沒有。其餘欄位照原規則：不同軸（motors／tag）、不同距離
// （interval）、不同教導點（params.btn）仍是不同 key。
struct StripRule {
    bool        inParams;     // false = request 最上層；true = request.params 裡
    const char* field;
    const char* onlyAction;   // 0 = 所有 action；否則只在這個 action 拿掉
};
const StripRule kMotorAccessStrip[] = {
    { false, "seq",        0 },               // motor-access.js ++seq
    { false, "id",         0 },               // 'cmd-' + seq
    { false, "issuedAt",   0 },               // nowIso()
    { false, "state",      0 },               // 'requested'（固定值，一併拿掉）
    { false, "reason",     0 },               // stopReq 的 'button'／'release'／'late-ack'
    { true,  "currentPos", 0 },               // HW.MotorTest getParams：edtCommandPos（軸在動就會變）
    { true,  "speedEvent", 0 },               // HW.teach getParams：改過速度後只有第一下帶
    { true,  "targetPos",  "moveRelative" },  // HW.MotorTest MoveP／MoveN：cur+iv（cur＝edtCommandPos）；
                                              //   btnGo／btnGoSoft*／教導頁 MoveTo 的 targetPos 是輸入值，不拿
    { true,  "servoOn",    "servoToggle" },   // HW.MotorTest：!runtime servoOn —— 第一下生效後第二下就反過來
};

// true = *out 是正規化後的 JSON（cJSON_PrintUnformatted，欄位順序照原字串）；false = 不是 JSON 物件，
// 呼叫端退回原字串。
bool MotorAccessKeyText(const webbridge::WebCommand& wc, std::string* out)
{
    if (!wc.hasValue || !wc.value.isString()) return false;
    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
    if (root == 0) return false;
    if (!cJSON_IsObject(root)) { cJSON_Delete(root); return false; }
    const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "action");
    const std::string action = (cJSON_IsString(ja) && ja->valuestring) ? std::string(ja->valuestring) : std::string();
    cJSON* params = cJSON_GetObjectItemCaseSensitive(root, "params");
    for (std::size_t i = 0; i < sizeof(kMotorAccessStrip) / sizeof(kMotorAccessStrip[0]); ++i) {
        const StripRule& s = kMotorAccessStrip[i];
        if (s.onlyAction != 0 && action != s.onlyAction) continue;
        cJSON* obj = s.inParams ? (cJSON_IsObject(params) ? params : 0) : root;
        if (obj != 0) cJSON_DeleteItemFromObjectCaseSensitive(obj, s.field);
    }
    char* txt = cJSON_PrintUnformatted(root);
    const bool ok = txt != 0;
    if (ok) *out = txt;
    if (txt) cJSON_free(txt);
    cJSON_Delete(root);
    return ok;
}

// value（JSON 字串）裡的 op／act。true = 取得（欄位缺就是 dflt）；false = value 不是 JSON
// 或欄位不是字串。沒有 value／value 不是字串時比照本體，當成 "{}"。
bool ExtractOp(const webbridge::WebCommand& wc, const OpRule& r, std::string* op)
{
    const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
    cJSON* root = cJSON_Parse(payload.empty() ? "{}" : payload.c_str());
    if (root == 0) return false;
    bool got = true;
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, r.field);
    if (j == 0)                                     *op = r.dflt;
    else if (cJSON_IsString(j) && j->valuestring)   *op = j->valuestring;
    else                                            got = false;
    cJSON_Delete(root);
    return got;
}

// 十進位（不用 printf 的 %lld：MinGW.org 的 msvcrt 格式檢查會叫；也不用 std::to_string：
// MinGW.org GCC 6.3 的 libstdc++ 不一定有）。
std::string Dec(std::int64_t v)
{
    char buf[32];
    char* p = buf + sizeof(buf);
    *--p = '\0';
    const bool neg = v < 0;
    std::uint64_t u = neg ? (std::uint64_t)0 - (std::uint64_t)v : (std::uint64_t)v;
    do { *--p = (char)('0' + (int)(u % 10ULL)); u /= 10ULL; } while (u != 0);
    if (neg) *--p = '-';
    return std::string(p);
}

std::string DecU(std::uint64_t u) { return Dec((std::int64_t)u); }   // 只用在 ms／筆數，不會超過 int64

// a - b，單位 µs -> ms，帶正負號
std::int64_t DiffMs(std::uint64_t a, std::uint64_t b)
{
    if (a >= b) return (std::int64_t)((a - b) / 1000ULL);
    return -(std::int64_t)((b - a) / 1000ULL);
}

struct Fnv {
    std::uint64_t h;
    Fnv() : h(14695981039346656037ULL) {}
    void Mix(const char* p, std::size_t n)
    {
        for (std::size_t i = 0; i < n; ++i) {
            h ^= (std::uint64_t)(unsigned char)p[i];
            h *= 1099511628211ULL;
        }
    }
    void Mix(const std::string& s) { Mix(s.data(), s.size()); }
    void Byte(char c) { Mix(&c, 1); }
};

}  // namespace

// =============================================================================
WebCmdGuard::WebCmdGuard(std::uint64_t windowMs, ClockUs clock)
    : windowUs_(0), clock_(clock), done_(), blocked_(0), lastPurgeUs_(0), purgedOnce_(false)
{
    SetWindowMs(windowMs);
}

void WebCmdGuard::SetWindowMs(std::uint64_t ms)
{
    if (ms > kMaxWindowMs) ms = kMaxWindowMs;
    windowUs_ = ms * 1000ULL;
}

std::uint64_t WebCmdGuard::SteadyNowUs()
{
    // 與 CommandQueue.cpp tryPush 蓋 pushedUs 的式子逐字相同（同一個時鐘、同一個原點）。
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::uint64_t WebCmdGuard::Now() const
{
    return clock_ ? clock_() : SteadyNowUs();
}

bool WebCmdGuard::ParseWindowMs(const char* text, std::uint64_t* ms)
{
    std::uint64_t v = kDefaultWindowMs;
    bool good = true;
    if (text != 0 && *text != '\0') {
        const char* p = text;
        while (*p == ' ' || *p == '\t') ++p;                        // cmd 的 `set X=400 ` 會帶尾巴空白
        std::uint64_t acc = 0;
        bool any = false;
        for (; *p >= '0' && *p <= '9'; ++p) {
            any = true;
            if (acc < 1000000000ULL) acc = acc * 10ULL + (std::uint64_t)(*p - '0');
        }
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
        if (!any || *p != '\0') {
            good = false;                                           // 負號、非數字、尾巴有垃圾：用預設
        } else {
            v = acc;
            if (v > kMaxWindowMs) v = kMaxWindowMs;
        }
    }
    if (ms) *ms = v;
    return good;
}

bool WebCmdGuard::Exempt(const webbridge::WebCommand& wc)
{
    for (std::size_t i = 0; i < sizeof(kExemptNames) / sizeof(kExemptNames[0]); ++i)
        if (wc.cmd == kExemptNames[i]) return true;
    for (std::size_t i = 0; i < sizeof(kExemptPrefixes) / sizeof(kExemptPrefixes[0]); ++i) {
        const std::size_t n = std::strlen(kExemptPrefixes[i]);
        if (wc.cmd.size() >= n && wc.cmd.compare(0, n, kExemptPrefixes[i]) == 0) return true;
    }
    if (wc.cmd == "motor.access" && MotorAccessReleaseDir(wc)) return true;   // AI(W906-R0927-8) 20260927: HOME／LoopMove 抬起＝停止方向
    for (std::size_t i = 0; i < sizeof(kOpRules) / sizeof(kOpRules[0]); ++i) {
        const OpRule& r = kOpRules[i];
        if (wc.cmd != r.cmd) continue;
        std::string op;
        if (!ExtractOp(wc, r, &op)) return false;
        for (const char* const* o = r.ops; *o != 0; ++o)
            if (op == *o) return true;
        return false;
    }
    return false;
}

std::uint64_t WebCmdGuard::KeyOf(const webbridge::WebCommand& wc)
{
    Fnv f;
    f.Mix(wc.cmd);
    f.Byte('\x1f');
    if (wc.hasTag) f.Mix(wc.tag);
    f.Byte('\x1f');
    if (!wc.hasValue) {
        f.Byte('\0');                                               // 沒有 value
    } else {
        switch (wc.value.type()) {
        case webbridge::TagType::Null:
            f.Byte('n');
            break;
        case webbridge::TagType::Bool:
            f.Byte('b');
            f.Byte(wc.value.asBool() ? '1' : '0');
            break;
        case webbridge::TagType::Int:
            f.Byte('i');
            f.Mix(Dec(wc.value.asInt()));
            break;
        case webbridge::TagType::Double: {
            char buf[40];
            std::snprintf(buf, sizeof(buf), "%.17g", wc.value.asDouble());
            f.Byte('d');
            f.Mix(buf, std::strlen(buf));
            break;
        }
        case webbridge::TagType::String: {
            std::string norm;
            if (wc.cmd == "motor.access" && MotorAccessKeyText(wc, &norm)) {   // AI(W906-R0927-8) 20260927: 拿掉每下都變的欄位
                f.Byte('m');
                f.Mix(norm);
            } else {
                f.Byte('s');
                f.Mix(wc.value.asString());                         // 原字串，不做 JSON 正規化
            }
            break;
        }
        }
    }
    return f.h != 0 ? f.h : 1ULL;                                   // 0 保留給「不受保護」
}

int WebCmdGuard::IoClickDownOf(const webbridge::WebCommand& wc)
{
    // AI(W906-R0927-9) 20260927: 跟 tools/wb_serve.cpp W906_DispatchIoClick 的 ioDown 同一套，再照 Click_ 只收 0／1
    //   （JsonBridge/IoBtnPanelClick.cpp:221）把其他值併成 -1。
    if (!wc.hasValue) return -1;
    int d = -1;
    if (wc.value.isInt()) {
        d = (int)wc.value.asInt(-1);                                // 同 wb_serve 的 (int) 轉型（int64 → int）
    } else if (wc.value.isDouble()) {
        const double v = wc.value.asDouble(-1.0);
        // wb_serve 是 (int)v（往 0 截斷：0.7→0、-0.5→0、1.9→1）。(-1, 2) 以外 int 轉型的結果不是 0／1
        // （超出 int 範圍還是 UB），本體一律拒絕 —— 這裡不轉，直接 -1；NaN 兩個比較都是 false，也是 -1。
        if (v > -1.0 && v < 2.0) d = (int)v;
    } else if (wc.value.isBool()) {
        d = wc.value.asBool() ? 1 : 0;
    }
    return (d == 0 || d == 1) ? d : -1;
}

std::uint64_t WebCmdGuard::ButtonKeyOf(const webbridge::WebCommand& wc)
{
    return ButtonKeyOf(wc, IoClickDownOf(wc));
}

std::uint64_t WebCmdGuard::ButtonKeyOf(const webbridge::WebCommand& wc, int down)
{
    Fnv f;
    f.Mix(wc.cmd);
    f.Byte('\x1f');
    if (wc.hasTag) f.Mix(wc.tag);
    f.Byte('\x1f');
    f.Byte('B');                                                    // 「鈕」（與 KeyOf 的型別位元組都不同）
    f.Byte(down == 0 ? '0' : down == 1 ? '1' : '-');                // AI(W906-R0927-9) 20260927: ＋正規化後的 down（Jimmy 08:3x）
    return f.h != 0 ? f.h : 1ULL;
}

bool WebCmdGuard::Check(const webbridge::WebCommand& wc, std::uint64_t* key, std::string* why)
{
    if (key) *key = 0;
    if (why) why->clear();
    if (windowUs_ == 0) return false;                               // W906_CMDGUARD_MS=0：整個關掉
    if (Exempt(wc)) return false;

    const std::uint64_t k = KeyOf(wc);
    if (CheckKey(k, wc, "command", wc.cmd, why)) return true;       // AI(W906-R0927-9) 20260927: 後半段抽成 CheckKey，行為不變
    if (key) *key = k;
    return false;
}

bool WebCmdGuard::CheckKey(std::uint64_t k, const webbridge::WebCommand& wc, const std::string& what,
                           const std::string& label, std::string* why, const char* phrase)
{
    if (why) why->clear();
    if (windowUs_ == 0 || k == 0) return false;
    const std::unordered_map<std::uint64_t, std::uint64_t>::iterator it = done_.find(k);
    if (it != done_.end()) {
        const std::uint64_t now     = Now();
        const std::uint64_t arrived = wc.pushedUs != 0 ? wc.pushedUs : now;
        const std::uint64_t doneUs  = it->second;
        if (doneUs > now) {
            // 完成時間在「未來」＝時鐘往回跳了。⚠ MinGW.org GCC 6.3 的 steady_clock **不是**
            // 單調時鐘：c++config.h 沒有 _GLIBCXX_USE_CLOCK_MONOTONIC，libstdc++ 退回
            // system_clock（gettimeofday，牆上時間）——校時／手動改時間都會讓它跳。
            // 不處理的話，往回跳一小時就會把同一條指令擋一小時。丟掉這筆、照常執行。
            done_.erase(it);
        } else if (arrived < doneUs + windowUs_) {
            ++blocked_;                                             // 被擋下的不蓋時間（不延長窗口）
            if (why)
                *why = "busy: same " + what + " " + (phrase ? phrase : "in progress or just done")   // AI(W906-R0927-9) 20260927: phrase（IO 鈕）
                     + " (" + label + ", " + Dec(DiffMs(now, doneUs)) + " ms ago)";
            std::printf("[cmdguard] busy, not run: cmd=%s tag=%s id=%s arrived %s ms after the same "
                        "%s finished (negative = while it was still running); window %s ms; "
                        "blocked so far %s\n",
                        wc.cmd.c_str(), wc.hasTag ? wc.tag.c_str() : "-", Dec(wc.id).c_str(),
                        Dec(DiffMs(arrived, doneUs)).c_str(), what.c_str(), DecU(WindowMs()).c_str(),
                        DecU(blocked_).c_str());
            return true;
        }
    }
    return false;
}

void WebCmdGuard::Done(std::uint64_t key)
{
    if (key == 0) return;
    const std::uint64_t now = Now();
    done_[key] = now;
    if (done_.size() > kPurgeAbove) Purge(now);
}

// AI(W906-R0927-9) 20260927: IO 鈕（W906IoClickGuardScope）用 —— 見 WebCmdGuard.h
void WebCmdGuard::Forget(std::uint64_t key)
{
    if (key != 0) done_.erase(key);
}

bool WebCmdGuard::Touch(std::uint64_t key)
{
    if (key == 0) return false;
    const std::unordered_map<std::uint64_t, std::uint64_t>::iterator it = done_.find(key);
    if (it == done_.end()) return false;
    it->second = Now();
    return true;
}

void WebCmdGuard::Purge(std::uint64_t now)
{
    if (!purgedOnce_ || now < lastPurgeUs_ || now - lastPurgeUs_ >= kPurgeEveryUs) {
        purgedOnce_  = true;
        lastPurgeUs_ = now;
        for (std::unordered_map<std::uint64_t, std::uint64_t>::iterator it = done_.begin(); it != done_.end(); ) {
            // 10 秒前完成的，或完成時間在「未來」（時鐘往回跳，見 Check）
            if (it->second > now || now - it->second >= kPurgeAgeUs) it = done_.erase(it);
            else ++it;
        }
    }
    if (done_.size() > kHardCap) {
        // 10 秒內完成了四千多條不同的受保護指令 —— 不是人按得出來的。整個清空（fail-open：
        // 最壞放過一次重複，不會把該跑的擋掉）。
        std::printf("[cmdguard] %s distinct guarded commands finished within 10 s -- table cleared (fail-open)\n",
                    DecU((std::uint64_t)done_.size()).c_str());
        done_.clear();
    }
}

// =============================================================================
namespace {

std::uint64_t InitialWindowMs()
{
    const char* e = std::getenv("W906_CMDGUARD_MS");
    std::uint64_t ms = WebCmdGuard::kDefaultWindowMs;
    const bool good = WebCmdGuard::ParseWindowMs(e, &ms);
    if (e == 0 || *e == '\0')
        std::printf("[cmdguard] double-click guard: window %s ms (default; W906_CMDGUARD_MS unset, 0 = off)\n",
                    DecU(ms).c_str());
    else if (!good)
        std::printf("[cmdguard] W906_CMDGUARD_MS=\"%.32s\" is not a decimal number of ms -- using the default %s ms\n",
                    e, DecU(ms).c_str());
    else if (ms == 0)
        std::printf("[cmdguard] double-click guard OFF (W906_CMDGUARD_MS=0)\n");
    else
        std::printf("[cmdguard] double-click guard: window %s ms (W906_CMDGUARD_MS)\n", DecU(ms).c_str());
    return ms;
}

}  // namespace

WebCmdGuard& WebCmdGuardGlobal()
{
    static WebCmdGuard g(InitialWindowMs());
    return g;
}

}  // namespace ht9045

// =============================================================================
W906CmdGuardScope::W906CmdGuardScope(const webbridge::WebCommand& wc)
    : guard_(0), key_(0), busy_(false), why_()
{
    try { guard_ = &ht9045::WebCmdGuardGlobal(); } catch (...) { guard_ = 0; }
    Begin(wc);
}

W906CmdGuardScope::W906CmdGuardScope(ht9045::WebCmdGuard& guard, const webbridge::WebCommand& wc)
    : guard_(&guard), key_(0), busy_(false), why_()
{
    Begin(wc);
}

void W906CmdGuardScope::Begin(const webbridge::WebCommand& wc)
{
    if (guard_ == 0) return;
    try {
        busy_ = guard_->Check(wc, &key_, &why_);
    } catch (...) {                                                 // fail-open：照常執行、不蓋時間
        busy_ = false;
        key_  = 0;
        why_.clear();
    }
    if (busy_) key_ = 0;
}

W906CmdGuardScope::~W906CmdGuardScope()
{
    if (guard_ != 0 && key_ != 0) {
        try { guard_->Done(key_); } catch (...) {}
    }
}

// =============================================================================
//  AI(W906-R0927-9) 20260927: IO 面板輸出鈕（設計見 WebCmdGuard.h 最後一節）
W906IoClickGuardScope::W906IoClickGuardScope(const webbridge::WebCommand& wc)
    : guard_(0), key_(0), busy_(false), why_()
{
    try { guard_ = &ht9045::WebCmdGuardGlobal(); } catch (...) { guard_ = 0; }
    Begin(wc);
}

W906IoClickGuardScope::W906IoClickGuardScope(ht9045::WebCmdGuard& guard, const webbridge::WebCommand& wc)
    : guard_(&guard), key_(0), busy_(false), why_()
{
    Begin(wc);
}

void W906IoClickGuardScope::Begin(const webbridge::WebCommand& wc)
{
    if (guard_ == 0 || !wc.hasTag || wc.tag.empty()) return;       // 沒有 Alias：不是一顆鈕（本體會拒絕），不擋也不記
    try {
        // AI(W906-R0927-9) 20260927: key 含正規化後的 down（Jimmy 08:3x）；down 跟上一次不同 ⇒ 那個 key 不在表裡 ⇒ 放行
        const int down = ht9045::WebCmdGuard::IoClickDownOf(wc);
        const std::uint64_t k = ht9045::WebCmdGuard::ButtonKeyOf(wc, down);
        const std::string label  = wc.cmd + " " + wc.tag + " down=" + (down == 0 ? "0" : down == 1 ? "1" : "invalid");
        const std::string phrase = "within " + ht9045::DecU(guard_->WindowMs()) + " ms";   // Jimmy 給的字：busy: same button and state within 400 ms
        busy_ = guard_->CheckKey(k, wc, "button and state", label, &why_, phrase.c_str());
        key_  = (busy_ || guard_->WindowMs() == 0) ? 0 : k;         // W=0：不記（同主 guard）
        if (key_ != 0) {
            // 放行：同一顆鈕另外兩個 down 值的紀錄刪掉（下一下換狀態一律放行），自己的先蓋一次（執行中又到同一個狀態的也擋）
            for (int d = -1; d <= 1; ++d)
                if (d != down) guard_->Forget(ht9045::WebCmdGuard::ButtonKeyOf(wc, d));
            guard_->Done(key_);
        }
    } catch (...) {                                                 // fail-open：照常執行、不蓋時間
        busy_ = false;
        key_  = 0;
        why_.clear();
    }
}

W906IoClickGuardScope::~W906IoClickGuardScope()
{
    if (guard_ != 0 && key_ != 0) {
        // AI(W906-R0927-9) 20260927: 蓋完成時間，但只在自己的紀錄還在時 —— 執行中插進來跑完的「另一個 down」已經把它刪了（WebCmdGuard.h）
        try { guard_->Touch(key_); } catch (...) {}
    }
}
