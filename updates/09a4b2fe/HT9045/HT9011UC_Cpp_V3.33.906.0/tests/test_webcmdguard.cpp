// =============================================================================
//  test_webcmdguard.cpp -- ht9045::WebCmdGuard（WebCmdGuard.h），S107-3 伺服器端防連點
//
//  AI(W906-CMDGUARD) 20260926: 時鐘全部手給（µs），模擬 wb_serve 分派迴圈的單執行緒依序執行：
//    Run() = 迴圈頭建 W906CmdGuardScope -> busy 就不執行 -> 否則「執行」execMs（推進假時鐘）
//    -> 這一圈結束，Scope 解構蓋完成時間。
//    [1] 第一條執行中，同 key 到達（pushed 早於 done）：busy；慢指令排在佇列裡的第二下也 busy
//    [2] 完成後 100 ms：busy；完成後 W+1 ms（與邊界 W-1µs／W）：放行
//    [3] motor.access jogP 每 20 ms 一次共 10 次、motor.stop、modal.answer、observer.get 每秒：全部放行、不進表
//        AI(W906-FRW-S115) 20260927: contactct.get 同一個 value 每秒一次（Data.ContactCT 即時更新），以及 tick 剛做完
//        5 ms 後同 value 的一下（頁面排隊的點選）：全部放行、不進表
//    [4] editlist.save 的 tag=IniConfig 與 tag=Teach 相隔 10 ms：都放行
//    [5] sortCT {confirmed:false} 接著 {confirmed:true} 相隔 5 ms：都放行；再一次 {confirmed:true}：busy
//    [6] towerlight {op:get} 兩次：放行；{op:click,led:RGB12} 兩次：第二次 busy；換一格 LED：放行
//    [7] 三連點：第三下在 W+1 放行（被擋的不延長窗口）
//    [8] W=0：全部放行、不進表
//    [9] 一萬個不同 key 之後表的大小有上限；10 秒前的會被清掉
//    [10] 名稱級／op 級白名單（含欄位缺時照本體預設、value 不是 JSON）
//    [11] key：value 的型別、有無 value、tag 都分得開；同樣輸入同樣 key、不是 0
//    [12] W906_CMDGUARD_MS 的解析（預設、0、夾上限、不合法）
//    [13] pushedUs 為 0 時用當下時間
//    [14] 時鐘往回跳（MinGW 6.3 的 steady_clock 是牆上時間）：「未來」的完成時間丟掉、照常執行，之後照常防連點
//    [15] W906CmdGuardScope 走 WebCmdGuardGlobal（真的 steady_clock）
//    [16] AI(W906-R0927-8) 20260927: motor.access 依 action 細分 —— 37 個 action 逐一分類；用頁面真的會送的
//         request（seq／id／issuedAt 每下不同、params 有畫面即時值）連點：相對移動、伺服、HOME、Loop 的第二下
//         busy；jog／stop／速度捲軸放行；不同距離／不同軸放行；HOME／Loop 抬起（start=false）放行
//    [17] AI(W906-R0927-8) 20260927: motor.access 的 key 正規化（拿掉哪些欄位、哪些留著；不是 JSON 時退回原字串）
//    [18] AI(W906-R0927-9) 20260927: W906IoClickGuardScope（Jimmy 08:3x 條件）—— 同一顆鈕＋同一個 down 400 ms 內第二下
//         busy（訊息 "busy: same button and state within 400 ms (...)"）；同一顆鈕 down 不同一律放行（開→關→開 三下都跑）；
//         第一下還在跑時到的：同 down busy、不同 down 放行；巢狀時表裡留較晚到的狀態；不同鈕不擋；超過 W 放行；
//         W=0 全放行；沒有 tag 不擋；主 guard 仍把它當白名單；down 的正規化（Int／Double／Bool／字串／沒有 value）
//  NOT COVERED：wb_serve 真的在分派迴圈頭建 Scope、W906_DispatchIoClick 第一行建 W906IoClickGuardScope
//    （AI(W906-R0927-9) 20260927：已同行插入 tools/wb_serve.cpp:6215，但本測試不經過 wb_serve —— 由整合者 build）；
//    瀏覽器端收到 busy: 的處理（web/page 另一位工程師）。
// =============================================================================
#include "WebCmdGuard.h"
#include "Public/cJSON.h"   // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #8 -- [16] reads web/JSON/motor-access.json (cJSON via ht9045_webbridge -> ht9045_public)

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

using ht9045::WebCmdGuard;
using webbridge::TagValue;
using webbridge::WebCommand;

static std::uint64_t g_now = 0;                      // 假時鐘（µs）
static std::uint64_t FakeNow() { return g_now; }

static const std::uint64_t MS = 1000ULL;
static const std::uint64_t T0 = 5000000000ULL;       // 任意起點（steady_clock 不是 0 起算）

static WebCommand Cmd(const char* cmd, const char* tag, const char* strValue, std::uint64_t pushedUs)
{
    WebCommand c;
    static long long nextId = 1;
    c.id       = nextId++;
    c.cmd      = cmd;
    c.tag      = tag ? tag : "";
    c.hasTag   = !c.tag.empty();
    if (strValue) { c.value = TagValue::makeString(strValue); c.hasValue = true; }
    c.pushedUs = pushedUs;
    return c;
}

// 分派迴圈的一圈：到點（g_now 至少到 pushedUs）-> Scope -> busy 不執行 / 執行 execMs -> 解構蓋時間。
// 回 true = 執行了；busy 時 *why 帶回訊息。
static bool Run(WebCmdGuard& g, const WebCommand& wc, std::uint64_t execMs, std::string* why = 0)
{
    if (g_now < wc.pushedUs) g_now = wc.pushedUs;
    W906CmdGuardScope s(g, wc);
    if (why) *why = s.why();
    if (s.busy()) return false;
    g_now += execMs * MS;
    return true;
}

static bool StartsWith(const std::string& s, const char* p) { return s.compare(0, std::string(p).size(), p) == 0; }

// AI(W906-R0927-8) 20260927: 頁面真的會送的 motor.access value —— web/page/motor-access.js send() 的 req：
//   {seq, id:'cmd-'+seq, source, button, action, kind, motors, params, issuedAt, state:'requested'}（JSON.stringify 的欄位順序）。
//   seq／id／issuedAt 每呼叫一次就變，跟頁面一樣。
static int g_maSeq = 0;
static std::string MaReq(const char* source, const char* button, const char* action, const char* kind,
                         const char* motor, const char* paramsJson)
{
    ++g_maSeq;
    char head[64];
    std::snprintf(head, sizeof(head), "{\"seq\":%d,\"id\":\"cmd-%d\",", g_maSeq, g_maSeq);
    std::string s = head;
    s += "\"source\":\"";  s += source;
    s += "\",\"button\":\""; s += button;
    s += "\",\"action\":\""; s += action;
    s += "\",\"kind\":\"";  s += kind;
    s += "\",\"motors\":[";
    if (motor && *motor) { s += "\""; s += motor; s += "\""; }
    s += "],\"params\":";
    s += paramsJson ? paramsJson : "{}";
    char tail[96];
    std::snprintf(tail, sizeof(tail), ",\"issuedAt\":\"2026-09-27T08:00:%02d.%03dZ\",\"state\":\"requested\"}",
                  g_maSeq % 60, (g_maSeq * 37) % 1000);
    s += tail;
    return s;
}
// tag = motors[0]（ht9045_recipe_client.js motorAccess）
static WebCommand Ma(const char* button, const char* action, const char* kind, const char* motor,
                     const char* paramsJson, std::uint64_t pushedUs, const char* source = "uMotorTest")
{
    const std::string v = MaReq(source, button, action, kind, motor, paramsJson);
    return Cmd("motor.access", motor, v.c_str(), pushedUs);
}

// AI(W906-R0927-9) 20260927: io.btnPanelClick —— tag＝Alias、value＝按下後的 Down（0／1，數字）
static WebCommand Io(const char* alias, int down, std::uint64_t pushedUs)
{
    WebCommand c = Cmd("io.btnPanelClick", alias, 0, pushedUs);
    c.value = TagValue::makeInt(down);
    c.hasValue = true;
    return c;
}
// W906_DispatchIoClick 的一次：第一行建 W906IoClickGuardScope -> busy 就 return -> 否則「執行」execMs -> return 時解構蓋時間
static bool RunIo(WebCmdGuard& g, const WebCommand& wc, std::uint64_t execMs, std::string* why = 0)
{
    if (g_now < wc.pushedUs) g_now = wc.pushedUs;
    W906IoClickGuardScope s(g, wc);
    if (why) *why = s.why();
    if (s.busy()) return false;
    g_now += execMs * MS;
    return true;
}

int main()
{
    const std::uint64_t W = WebCmdGuard::kDefaultWindowMs;
    CHECK(W == 400);

    std::printf("[1] same key arrives while the first is still running: busy\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        const char* v = "{\"op\":\"click\",\"led\":\"RGB12\"}";
        WebCommand a  = Cmd("towerlight.op", 0, v, T0);
        WebCommand a2 = Cmd("towerlight.op", 0, v, T0 + 10 * MS);   // 第一條還在跑時到（排在佇列裡）
        CHECK(Run(g, a, 50));                                        // T0 .. T0+50ms
        std::string why;
        CHECK(!Run(g, a2, 50, &why));                                // 輪到它時第一條已完成，但它是跑的時候到的
        CHECK(StartsWith(why, "busy:"));
        CHECK(why.find("towerlight.op") != std::string::npos);
        CHECK(g.Blocked() == 1);

        // 慢指令（editlist.save 1-2 秒）：第二下 150 ms 後到、排在佇列裡等 2 秒 —— 還是 busy
        const char* sv = "{\"widgets\":{\"edA01\":\"1\"}}";
        g_now = T0 + 10000 * MS;
        WebCommand s1 = Cmd("editlist.save", "IniConfig", sv, g_now);
        WebCommand s2 = Cmd("editlist.save", "IniConfig", sv, g_now + 150 * MS);
        const std::uint64_t start = g_now;
        CHECK(Run(g, s1, 2000));
        CHECK(!Run(g, s2, 2000));
        CHECK(g_now == start + 2000 * MS);                           // 被擋的沒有執行（時鐘沒推進）
        WebCommand s3 = Cmd("editlist.save", "IniConfig", sv, start + 2000 * MS + (W + 1) * MS);
        CHECK(Run(g, s3, 2000));                                     // 窗口過了的新一下照常跑
    }

    std::printf("[2] 100 ms after done: busy; W+1 ms after done: runs (boundary: W-1us busy, W runs)\n");
    {
        WebCmdGuard g(W, FakeNow);
        const char* v = "{\"family\":\"all\"}";
        g_now = T0;
        CHECK(Run(g, Cmd("counterclear.exe", 0, v, T0), 20));
        const std::uint64_t D = g_now;                               // 完成時間
        CHECK(!Run(g, Cmd("counterclear.exe", 0, v, D + 100 * MS), 20));
        CHECK(!Run(g, Cmd("counterclear.exe", 0, v, D + W * MS - 1), 20));
        g_now = D + W * MS;
        std::uint64_t key = 0; std::string why;
        CHECK(g.Check(Cmd("counterclear.exe", 0, v, D + W * MS), &key, &why) == false);   // 剛好 W：放行
        CHECK(key != 0);
        CHECK(why.empty());
        CHECK(Run(g, Cmd("counterclear.exe", 0, v, D + (W + 1) * MS), 20));
    }

    std::printf("[3] jog every 20 ms x10, motor.stop, modal.answer, observer.get / contactct.get every 1 s: all run, none recorded\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        bool all = true;
        for (int i = 0; i < 10; ++i)
            all = Run(g, Cmd("motor.access", 0, "{\"action\":\"jogP\",\"axis\":3}", T0 + (std::uint64_t)i * 20 * MS), 5) && all;
        CHECK(all);
        CHECK(Run(g, Cmd("motor.stop", 0, "{\"action\":\"stop\",\"axis\":3}", g_now), 5));
        CHECK(Run(g, Cmd("motor.stop", 0, "{\"action\":\"stop\",\"axis\":3}", g_now + 1 * MS), 5));
        CHECK(Run(g, Cmd("modal.answer", "17", "BtnStart", g_now), 5));
        CHECK(Run(g, Cmd("modal.answer", "17", "BtnStart", g_now + 1 * MS), 5));
        bool obs = true;
        const std::uint64_t t = g_now;
        for (int i = 0; i < 5; ++i)
            obs = Run(g, Cmd("observer.get", 0, "{}", t + (std::uint64_t)i * 1000 * MS), 30) && obs;
        CHECK(obs);
        // AI(W906-FRW-S115) 20260927: Data.ContactCT 每 1000 ms 送 contactct.get {"yieldType":<頁面正在看的那一項>}
        //   （ht9045_contactct_wire.js tick）；做完 5 ms 後頁面排隊的點選剛好是同一個 value 也要跑（名稱級白名單）
        bool cct = true;
        const std::uint64_t tc = g_now;
        for (int i = 0; i < 5; ++i)
            cct = Run(g, Cmd("contactct.get", 0, "{\"yieldType\":3}", tc + (std::uint64_t)i * 1000 * MS), 20) && cct;
        cct = Run(g, Cmd("contactct.get", 0, "{\"yieldType\":3}", g_now + 5 * MS), 20) && cct;
        CHECK(cct);
        CHECK(Run(g, Cmd("sys.ping", 0, 0, g_now), 0));
        CHECK(Run(g, Cmd("sys.ping", 0, 0, g_now), 0));
        CHECK(g.Size() == 0);
        CHECK(g.Blocked() == 0);
    }

    std::printf("[4] editlist.save tag=IniConfig then tag=Teach 10 ms apart: both run\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        const char* v = "{\"widgets\":{},\"answers\":{}}";
        CHECK(Run(g, Cmd("editlist.save", "IniConfig", v, T0), 5));
        CHECK(Run(g, Cmd("editlist.save", "Teach", v, T0 + 10 * MS), 5));
        CHECK(g.Size() == 2);
    }

    std::printf("[5] sortCT {confirmed:false} then {confirmed:true} 5 ms apart: both run; a second {confirmed:true}: busy\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        CHECK(Run(g, Cmd("act.sortCT.clearCount", 0, "{\"confirmed\":false}", T0), 3));
        CHECK(Run(g, Cmd("act.sortCT.clearCount", 0, "{\"confirmed\":true}", T0 + 5 * MS), 30));
        CHECK(!Run(g, Cmd("act.sortCT.clearCount", 0, "{\"confirmed\":true}", g_now + 80 * MS), 30));
    }

    std::printf("[6] towerlight get x2: run; click RGB12 x2: second busy; click RGB13: runs\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        CHECK(Run(g, Cmd("towerlight.op", 0, "{\"op\":\"get\"}", T0), 5));
        CHECK(Run(g, Cmd("towerlight.op", 0, "{\"op\":\"get\"}", T0 + 1 * MS), 5));
        const char* c12 = "{\"op\":\"click\",\"led\":\"RGB12\"}";
        CHECK(Run(g, Cmd("towerlight.op", 0, c12, g_now), 20));
        std::string why;
        CHECK(!Run(g, Cmd("towerlight.op", 0, c12, g_now + 120 * MS), 20, &why));
        CHECK(StartsWith(why, "busy: same command in progress or just done (towerlight.op, "));
        CHECK(why.size() > 8 && why.compare(why.size() - 8, 8, " ms ago)") == 0);
        CHECK(Run(g, Cmd("towerlight.op", 0, "{\"op\":\"click\",\"led\":\"RGB13\"}", g_now), 20));
    }

    std::printf("[7] triple click: the blocked second one does not extend the window; the third at W+1 runs\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        const char* v = "{\"op\":\"click\",\"led\":\"RGB00\"}";
        CHECK(Run(g, Cmd("towerlight.op", 0, v, T0), 20));
        const std::uint64_t D = g_now;
        CHECK(!Run(g, Cmd("towerlight.op", 0, v, D + 300 * MS), 20));      // 擋下，不蓋時間
        CHECK(Run(g, Cmd("towerlight.op", 0, v, D + (W + 1) * MS), 20));    // 若被擋的有延長窗口，這裡會是 busy
    }

    std::printf("[8] W=0: everything runs, nothing recorded\n");
    {
        WebCmdGuard g(0, FakeNow);
        g_now = T0;
        const char* v = "{\"op\":\"click\",\"led\":\"RGB12\"}";
        CHECK(Run(g, Cmd("towerlight.op", 0, v, T0), 20));
        CHECK(Run(g, Cmd("towerlight.op", 0, v, T0 + 1 * MS), 20));
        CHECK(Run(g, Cmd("start.run", 0, 0, g_now), 20));
        CHECK(Run(g, Cmd("start.run", 0, 0, g_now), 20));
        CHECK(g.Size() == 0);
        CHECK(g.WindowMs() == 0);
    }

    std::printf("[9] 10000 distinct keys: the table stays bounded; entries 10 s old are purged\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;                                                   // 時鐘不動：年齡清理清不掉，靠硬上限
        std::size_t peak = 0;
        char v[64];
        for (int i = 0; i < 10000; ++i) {
            std::snprintf(v, sizeof(v), "{\"op\":\"click\",\"led\":\"X%d\"}", i);
            Run(g, Cmd("towerlight.op", 0, v, g_now), 0);
            if (g.Size() > peak) peak = g.Size();
        }
        CHECK(peak <= WebCmdGuard::kHardCap);
        CHECK(g.Size() <= WebCmdGuard::kHardCap);

        WebCmdGuard h(W, FakeNow);
        g_now = T0;
        for (int i = 0; i < 600; ++i) {
            std::snprintf(v, sizeof(v), "{\"op\":\"click\",\"led\":\"Y%d\"}", i);
            Run(h, Cmd("towerlight.op", 0, v, g_now), 0);
        }
        CHECK(h.Size() == 600);                                       // 都在 10 秒內，清不掉
        g_now = T0 + 11000 * MS;                                      // 11 秒後再一條：600 條舊的清掉
        Run(h, Cmd("towerlight.op", 0, "{\"op\":\"click\",\"led\":\"Z\"}", g_now), 0);
        CHECK(h.Size() == 1);
    }

    std::printf("[10] name-level and op-level whitelist\n");
    {
        struct Case { const char* cmd; const char* value; bool exempt; };
        const Case cases[] = {
            // 名稱級
            { "sys.ping", 0, true }, { "cfg.resync", 0, true }, { "log.event", "{}", true },
            { "ui.windows.put", "{}", true }, { "stream.resync", 0, true },
            { "modal.answer", "BtnStart", true }, { "dialog.response", "{}", true }, { "dialog.auth", "{}", true }, { "dialog.notifyAck", 0, true },   // AI(W906-J5-ACK) 20260930
            { "motor.access", "{\"action\":\"moveRelative\"}", false }, { "motor.stop", "{}", true },   // AI(W906-R0927-8) 20260927: motor.access 改到 op 級（[16]）
            { "io.btnPanelClick", "1", true }, { "sim.di.set", "1", true }, { "pause.run", 0, true }, { "act.home.abort", "{}", true },   //AI(W906-HOMEMON) 20261001
            { "editlist.get", 0, true }, { "contactct.get", 0, true }, { "observer.get", 0, true },  { "act.observerSG.state", "{}", true }, { "act.observerSG.queryNow", "{}", false }, { "act.observerSG.queryYesterday", "{}", false },   // AI(W906-ST02-OB7F) 20261004 (St02-E, NB2 R180 low (b)): REVERSE -- drop "act.observerSG.state" from WebCmdGuard.cpp:90 -> red
            { "counterclear.get", 0, true }, { "auth.mode", 0, true },
            { "pci1203.do.setBit", "{}", true }, { "olp.setMode", "1", true },
            // 不在名單：擋
            { "start.run", 0, false }, { "lot.start", 0, false }, { "editlist.save", "{}", false },
            { "counter.clear", 0, false }, { "counterclear.click", "1", false }, { "counterclear.exe", 0, false },
            { "hw.access", "{}", false }, { "pci1203", 0, false }, { "olp", 0, false },
            { "auth.login", "{}", false }, { "system.file.put", "{\"dryRun\":true}", false },
            { "act.main.testerConnect", "{}", false }, { "act.main.stateRecord", "{}", false },
            // op 級
            { "security.jam", "{\"op\":\"open\"}", true }, { "security.jam", "{\"op\":\"exportChunk\",\"offset\":0}", true },
            { "security.jam", "{\"op\":\"stats\"}", true }, { "security.jam", "{\"op\":\"exportStatus\"}", true },
            { "security.jam", "{\"op\":\"exportRelease\"}", true },
            { "security.jam", "{\"op\":\"save\"}", false }, { "security.jam", "{\"op\":\"export\"}", false },
            { "security.jam", 0, false },
            { "security.passwd", "{\"op\":\"state\"}", true }, { "security.passwd", "{\"op\":\"list\"}", true },
            { "security.passwd", "{\"op\":\"open\"}", true }, { "security.passwd", "{\"op\":\"apply\"}", false },
            { "lotinfo.op", "{\"op\":\"testerLog.get\"}", true }, { "lotinfo.op", "{\"op\":\"selection.get\"}", true },
            { "lotinfo.op", "{\"op\":\"selection.save\"}", false }, { "lotinfo.op", "{\"op\":\"barcode.clearCount\",\"confirmed\":true}", false },
            { "towerlight.op", "{\"op\":\"get\"}", true }, { "towerlight.op", "{\"op\":\"music\",\"combo\":\"cbJam\",\"index\":2}", false },
            { "towerlight.op", "not json", false }, { "towerlight.op", "{\"op\":5}", false }, { "towerlight.op", 0, false },
            { "recipe.change", "{\"op\":\"list\"}", true }, { "recipe.change", "{\"op\":\"change\",\"name\":\"A\"}", false },
            { "recipe.change", 0, false },
            { "act.main.peModel", "{\"op\":\"get\"}", true }, { "act.main.peModel", "{}", true },      // 欄位缺 = get（MainClick.cpp）
            { "act.main.peModel", 0, true }, { "act.main.peModel", "{\"op\":\"click\"}", false },
            { "observer.get", "{\"act\":\"timer\"}", true }, { "observer.get", "{\"act\":\"\"}", true }, { "observer.get", "{}", true },   // AI(W906-PROD-S116Y) 20260927
            { "observer.get", "{\"act\":\"yieldSite\",\"arg\":0,\"text\":\"3,2\"}", false }, { "observer.get", "{\"act\":\"yieldClear\"}", false }, { "observer.get", "{\"act\":\"yieldMax\",\"text\":\"x\"}", false }, { "observer.get", "{\"act\":\"yieldMin\"}", false },   //AI(W906-Q2-OBS) 20260927 (St02-E): all four Yield acts need the token
            { "act.sortCT.clearCount", "{\"op\":\"lotId\",\"text\":\"L1\"}", true },
            { "act.sortCT.clearCount", "{\"confirmed\":true}", false },
            { "act.sortCT.clearCount", "{\"op\":\"dblClick\",\"panel\":\"pnlAuto1\"}", false },
            { "smartdiag.op", "{\"act\":\"open\"}", true }, { "smartdiag.op", "{\"act\":\"timer\"}", true },
            { "smartdiag.op", "{}", true },                                                            // 欄位缺 = open（WebSmartDiag.cpp）
            { "smartdiag.op", "{\"act\":\"cell\",\"col\":1,\"row\":2}", false }, { "smartdiag.op", "{\"act\":\"save\"}", false },
            { "smartdiag.op", "{\"op\":\"timer\"}", true },                                            // 看的是 act，不是 op（缺 act = open）
            { "builder.op", "{\"act\":\"state\"}", true }, { "builder.op", "{\"act\":\"dir\",\"path\":\"C:\\\\\"}", true },
            { "builder.op", "{\"act\":\"change\",\"widget\":\"lbFiles\"}", true }, { "builder.op", "{\"act\":\"drive\"}", true },
            { "builder.op", "{\"act\":\"open\"}", true }, { "builder.op", "{\"act\":\"close\"}", true },
            { "builder.op", 0, true },                                                                  // 欄位缺 = state（WebBuilder.cpp）
            { "builder.op", "{\"act\":\"create\",\"name\":\"R1\"}", false }, { "builder.op", "{\"act\":\"delete\"}", false },
            { "builder.op", "{\"act\":\"import\"}", false }, { "builder.op", "{\"act\":\"export\"}", false },
            { "builder.op", "{\"act\":true}", false },                                                  // 欄位不是字串：本體拒 -> 擋
        };
        int bad = 0;
        for (std::size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
            const bool e = WebCmdGuard::Exempt(Cmd(cases[i].cmd, 0, cases[i].value, T0));
            if (e != cases[i].exempt) {
                ++bad;
                std::printf("  FAIL Exempt(%s, %s) = %d, want %d\n", cases[i].cmd,
                            cases[i].value ? cases[i].value : "(no value)", (int)e, (int)cases[i].exempt);
            }
        }
        CHECK(bad == 0);
    }

    std::printf("[11] key: value type, absent value, tag all distinguish; stable; never 0\n");
    {
        WebCommand s1 = Cmd("counterclear.click", "chkA", "1", T0);
        WebCommand n1 = Cmd("counterclear.click", "chkA", 0, T0);   n1.value = TagValue::makeDouble(1.0); n1.hasValue = true;
        WebCommand b1 = Cmd("counterclear.click", "chkA", 0, T0);   b1.value = TagValue::makeBool(true);  b1.hasValue = true;
        WebCommand i1 = Cmd("counterclear.click", "chkA", 0, T0);   i1.value = TagValue::makeInt(1);      i1.hasValue = true;
        WebCommand none = Cmd("counterclear.click", "chkA", 0, T0);
        WebCommand empty = Cmd("counterclear.click", "chkA", "", T0);
        WebCommand otherTag = Cmd("counterclear.click", "chkB", "1", T0);
        WebCommand b0 = Cmd("counterclear.click", "chkA", 0, T0);   b0.value = TagValue::makeBool(false); b0.hasValue = true;
        const std::uint64_t k = WebCmdGuard::KeyOf(s1);
        CHECK(k != 0);
        CHECK(k == WebCmdGuard::KeyOf(Cmd("counterclear.click", "chkA", "1", T0 + 999 * MS)));   // 不含時間／id
        CHECK(k != WebCmdGuard::KeyOf(n1));
        CHECK(k != WebCmdGuard::KeyOf(b1));
        CHECK(WebCmdGuard::KeyOf(n1) != WebCmdGuard::KeyOf(i1));
        CHECK(WebCmdGuard::KeyOf(b1) != WebCmdGuard::KeyOf(b0));
        CHECK(WebCmdGuard::KeyOf(none) != WebCmdGuard::KeyOf(empty));
        CHECK(k != WebCmdGuard::KeyOf(otherTag));
        // cmd／tag／value 的邊界不會混：("a", "bc") 與 ("ab", "c")
        CHECK(WebCmdGuard::KeyOf(Cmd("a", "bc", 0, T0)) != WebCmdGuard::KeyOf(Cmd("ab", "c", 0, T0)));
        CHECK(WebCmdGuard::KeyOf(Cmd("x", "t", "1", T0)) != WebCmdGuard::KeyOf(Cmd("x", "t1", 0, T0)));
    }

    std::printf("[12] W906_CMDGUARD_MS parsing\n");
    {
        std::uint64_t ms = 1;
        CHECK(WebCmdGuard::ParseWindowMs(0, &ms) && ms == 400);
        CHECK(WebCmdGuard::ParseWindowMs("", &ms) && ms == 400);
        CHECK(WebCmdGuard::ParseWindowMs("0", &ms) && ms == 0);
        CHECK(WebCmdGuard::ParseWindowMs("250", &ms) && ms == 250);
        CHECK(WebCmdGuard::ParseWindowMs(" 250 ", &ms) && ms == 250);   // cmd 的 set X=250 會帶尾巴空白
        CHECK(WebCmdGuard::ParseWindowMs("99999", &ms) && ms == 10000);
        CHECK(WebCmdGuard::ParseWindowMs("99999999999999999999999", &ms) && ms == 10000);
        CHECK(!WebCmdGuard::ParseWindowMs("-5", &ms) && ms == 400);
        CHECK(!WebCmdGuard::ParseWindowMs("abc", &ms) && ms == 400);
        CHECK(!WebCmdGuard::ParseWindowMs("12x", &ms) && ms == 400);
        WebCmdGuard g(20000, FakeNow);
        CHECK(g.WindowMs() == 10000);
        g.SetWindowMs(250);
        CHECK(g.WindowMs() == 250);
    }

    std::printf("[13] pushedUs == 0: the check uses the current time\n");
    {
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        const char* v = "{\"op\":\"click\",\"led\":\"RGB21\"}";
        CHECK(Run(g, Cmd("towerlight.op", 0, v, 0), 20));
        const std::uint64_t D = g_now;
        g_now = D + 100 * MS;
        CHECK(!Run(g, Cmd("towerlight.op", 0, v, 0), 20));
        g_now = D + 500 * MS;
        CHECK(Run(g, Cmd("towerlight.op", 0, v, 0), 20));
    }

    std::printf("[14] the clock jumps backwards (MinGW 6.3 steady_clock is wall time): the stale entry is dropped, the command runs\n");
    {
        WebCmdGuard g(W, FakeNow);
        const char* v = "{\"op\":\"click\",\"led\":\"RGB31\"}";
        g_now = T0 + 3600000 * MS;                                   // 一小時後完成……
        CHECK(Run(g, Cmd("towerlight.op", 0, v, g_now), 20));
        CHECK(g.Size() == 1);
        g_now = T0;                                                   // ……時鐘往回校一小時
        CHECK(Run(g, Cmd("towerlight.op", 0, v, g_now + 100 * MS), 20));   // 不可以被擋一小時
        CHECK(g.Size() == 1);                                         // 舊的那筆丟掉、換成這次的
        CHECK(!Run(g, Cmd("towerlight.op", 0, v, g_now + 100 * MS), 20));  // 之後照常防連點
    }

    std::printf("[15] W906CmdGuardScope on WebCmdGuardGlobal (real steady_clock)\n");
    {
        std::uint64_t expect = 400;
        WebCmdGuard::ParseWindowMs(std::getenv("W906_CMDGUARD_MS"), &expect);
        CHECK(ht9045::WebCmdGuardGlobal().WindowMs() == expect);
        WebCommand a = Cmd("towerlight.op", 0, "{\"op\":\"click\",\"led\":\"RGB70\"}", WebCmdGuard::SteadyNowUs());
        bool firstBusy = true;
        { W906CmdGuardScope s(a); firstBusy = s.busy(); }
        CHECK(!firstBusy);
        WebCommand b = Cmd("towerlight.op", 0, "{\"op\":\"click\",\"led\":\"RGB70\"}", WebCmdGuard::SteadyNowUs());
        W906CmdGuardScope s2(b);
        if (expect >= 50) {
            CHECK(s2.busy());                                          // 幾微秒後的同一下
            CHECK(StartsWith(s2.why(), "busy:"));
        } else {
            std::printf("  (W906_CMDGUARD_MS=%lu in this environment: second-click assertion skipped)\n",
                        (unsigned long)expect);
            CHECK(expect != 0 || !s2.busy());
        }
    }

    std::printf("[16] motor.access by action: jog/stop/scroll run; move/servo/home/loop/power second click busy\n");
    {
        // (a) 37 個 action（WebMotorAccess.cpp:50-96 kActions）逐一分類
        struct Act { const char* action; bool exempt; };
        const Act acts[] = {
            { "jogP", true }, { "jogN", true }, { "stop", true }, { "setSpeed", true },
            { "setPos1", true }, { "setPos2", true }, { "refreshParameter", true },
            { "setTeachFromCurrent", true }, { "setTeachFromOffset", true },
            { "moveRelative", false }, { "moveAbsolute", false }, { "moveSoftLimitP", false }, { "moveSoftLimitN", false },
            { "home", false }, { "loopMove", false }, { "servoToggle", false }, { "motorPowerToggle", false },
            { "setJogHighSpeed", false }, { "setJogLowSpeed", false }, { "setHomeHighSpeed", false }, { "setHomeLowSpeed", false },
            { "setSoftLimitP", false }, { "setSoftLimitN", false }, { "setRangeAndInit", false }, { "setRateAndInit", false },
            { "reloadMotorData", false }, { "resetMNet", false }, { "selectMotor", false }, { "setParamCell", false },
            { "copyFrom", false }, { "formShow", false }, { "formClose", false }, { "lightScale", false },
            { "lightScaleSave", false }, { "lightScaleDataSave", false }, { "teachSet", false }, { "teachGo", false },
            { "resetAlarm", false },                                            //AI(W906-MT-ALMRST) 20260929: the 38th (a second click inside the window is busy)
            { "moveToTrayCell", false },                                        //AI(W906-ARMCELL) 20261002: the 39th -- the Teach Arm Cell Go (a second click inside the window is busy; C++ also refuses a second job)
            { "gearCalMove", false }, { "gearRatioPreview", false }, { "gearRatioSave", false },   //AI(W906-GEARRATIO) 20261002: the 40th-42nd -- the Motor Test Gear Ratio tab (NB2 spec §5.5: none exempt, a second click inside 400 ms is busy)
            // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #8 -- the 8 catalog actions that had no row. Rows = the policy as it
            //   stands (none is in WebCmdGuard.cpp kMotorAccessActs, so a second click inside the window is busy); St01 to confirm each.
            { "saveMotTable", false }, { "gearHandBegin", false },
            { "teachZAllUp", false }, { "teachIndexServo", false }, { "teachSt02", false },
            { "teachSetAllArmZ", false }, { "teachOutZAllDown", false }, { "teachHomeAll", false },
        };
        // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #8 -- no pinned row count (was == 42: + moveToTrayCell AI(W906-ARMCELL),
        //   + the three Gear Ratio actions AI(W906-GEARRATIO); it was 8 behind the catalog): every action of web/JSON/motor-access.json
        //   has exactly one policy row in acts, and every acts row is a catalog action. Totals only printed.
        {
            std::string self(__FILE__);                                        // <tree>/tests/test_webcmdguard.cpp -> <tree>/../web/JSON
            for (std::size_t i = 0; i < self.size(); ++i) if (self[i] == '\\') self[i] = '/';
            const std::size_t t = self.rfind("/tests/");
            const std::string path = (t == std::string::npos ? std::string() : self.substr(0, t)) + "/../web/JSON/motor-access.json";
            std::ifstream f(path.c_str(), std::ios::binary);
            std::ostringstream ss;
            ss << f.rdbuf();
            cJSON* root = cJSON_Parse(ss.str().c_str());
            const cJSON* cmds = root ? cJSON_GetObjectItemCaseSensitive(root, "commands") : 0;
            const bool read = cJSON_IsArray(cmds) != 0;
            const std::size_t nActs = sizeof(acts) / sizeof(acts[0]);
            std::vector<int> used(nActs, 0);
            std::set<std::string> seen;
            int missing = 0, stale = 0;
            const cJSON* c = 0;
            cJSON_ArrayForEach(c, cmds) {
                const cJSON* a = cJSON_GetObjectItemCaseSensitive(c, "action");
                const std::string an = (cJSON_IsString(a) && a->valuestring) ? a->valuestring : "";
                if (!seen.insert(an).second) continue;                         // one check per distinct action
                int rows = 0;
                for (std::size_t i = 0; i < nActs; ++i) if (an == acts[i].action) { ++rows; ++used[i]; }
                if (rows != 1) { ++missing; std::printf("  FAIL catalog action \"%s\" has %d policy rows in acts (want 1)\n", an.c_str(), rows); }
            }
            for (std::size_t i = 0; i < nActs; ++i)
                if (used[i] == 0) { ++stale; std::printf("  FAIL acts row \"%s\" is not a motor-access.json action\n", acts[i].action); }
            std::printf("  (%s: %u distinct actions, %u policy rows)\n", path.c_str(), (unsigned)seen.size(), (unsigned)nActs);
            cJSON_Delete(root);
            CHECK(read && !seen.empty() && missing == 0 && stale == 0);
        }
        int bad = 0;
        for (std::size_t i = 0; i < sizeof(acts) / sizeof(acts[0]); ++i) {
            const WebCommand c = Ma("x", acts[i].action, "edit", "MInArmX", "{}", T0);
            if (WebCmdGuard::Exempt(c) != acts[i].exempt) {
                ++bad;
                std::printf("  FAIL Exempt(motor.access %s) = %d, want %d\n", acts[i].action, (int)!acts[i].exempt, (int)acts[i].exempt);
            }
        }
        CHECK(bad == 0);
        // 抬起方向（params.start 是 JSON false）放行；start=true、沒有 start、字串 "false" 照擋
        CHECK(WebCmdGuard::Exempt(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":false}", T0)));
        CHECK(WebCmdGuard::Exempt(Ma("btnLoopMove", "loopMove", "motion", "MInArmX", "{\"start\":false}", T0)));
        CHECK(WebCmdGuard::Exempt(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":false}", T0, "uteach")));
        CHECK(!WebCmdGuard::Exempt(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":true}", T0)));
        CHECK(!WebCmdGuard::Exempt(Ma("btnHome", "home", "motion", "MInArmX", "{}", T0)));
        CHECK(!WebCmdGuard::Exempt(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":\"false\"}", T0)));
        CHECK(!WebCmdGuard::Exempt(Ma("SetButton*", "teachSet", "edit", "MInArmX", "{\"btn\":\"SetButton140\",\"start\":false,\"accept\":true}", T0, "uteach")));
        // value 壞掉／缺 action（本體拒絕）：照擋
        CHECK(!WebCmdGuard::Exempt(Cmd("motor.access", 0, "not json", T0)));
        CHECK(!WebCmdGuard::Exempt(Cmd("motor.access", 0, 0, T0)));
        CHECK(!WebCmdGuard::Exempt(Cmd("motor.access", 0, "{\"source\":\"uMotorTest\"}", T0)));
        CHECK(!WebCmdGuard::Exempt(Cmd("motor.access", 0, "{\"action\":5}", T0)));
        CHECK(!WebCmdGuard::Exempt(Cmd("motor.access", 0, "{\"action\":\"stop \"}", T0)));   // 完全相符

        // (b) 相對移動 10 mm 連點：第二下的 value 跟第一下不一樣（seq／id／issuedAt，畫面位置也更新了）—— 照樣 busy
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        const char* p10a = "{\"speed\":5,\"currentPos\":100,\"softLimitP\":500,\"softLimitN\":-5,\"interval\":10,\"targetPos\":110}";
        const char* p10b = "{\"speed\":5,\"currentPos\":103.5,\"softLimitP\":500,\"softLimitN\":-5,\"interval\":10,\"targetPos\":113.5}";
        CHECK(Run(g, Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", p10a, T0), 20));
        const std::uint64_t D = g_now;
        std::string why;
        CHECK(!Run(g, Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", p10b, D + 150 * MS), 20, &why));
        CHECK(StartsWith(why, "busy: same command in progress or just done (motor.access, "));
        // 不同距離、不同軸、另一顆鈕（MoveN）：不擋
        CHECK(Run(g, Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX",
                        "{\"speed\":5,\"currentPos\":103.5,\"softLimitP\":500,\"softLimitN\":-5,\"interval\":1,\"targetPos\":104.5}", D + 160 * MS), 20));
        CHECK(Run(g, Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmY", p10a, D + 170 * MS), 20));
        CHECK(Run(g, Ma("sbMotorTest_MoveN", "moveRelative", "motion", "MInArmX",
                        "{\"speed\":5,\"currentPos\":100,\"softLimitP\":500,\"softLimitN\":-5,\"interval\":-10,\"targetPos\":90}", D + 180 * MS), 20));
        // 第一下完成後 W+1：同一顆鈕照常
        CHECK(Run(g, Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", p10b, D + (W + 1) * MS), 20));
        // 絕對移動：連點擋
        CHECK(Run(g, Ma("btnGo", "moveAbsolute", "motion", "MInArmZ", "{\"speed\":5,\"currentPos\":0,\"targetPos\":50}", g_now), 20));
        CHECK(!Run(g, Ma("btnGo", "moveAbsolute", "motion", "MInArmZ", "{\"speed\":5,\"currentPos\":7,\"targetPos\":50}", g_now + 100 * MS), 20));

        // (c) 伺服：頁面送 !runtime servoOn —— 第一下生效後第二下送的是反的；照樣 busy（不會「開了又關」）
        g_now += 1000 * MS;
        CHECK(Run(g, Ma("btnServoOff", "servoToggle", "control", "MInArmX", "{\"speed\":5,\"currentPos\":0,\"servoOn\":true}", g_now), 10));
        const std::uint64_t Ds = g_now;
        CHECK(!Run(g, Ma("btnServoOff", "servoToggle", "control", "MInArmX", "{\"speed\":5,\"currentPos\":0,\"servoOn\":false}", Ds + 200 * MS), 10));
        CHECK(Run(g, Ma("btnServoOff", "servoToggle", "control", "MInArmX", "{\"speed\":5,\"currentPos\":0,\"servoOn\":false}", Ds + (W + 1) * MS), 10));
        CHECK(Run(g, Ma("btnServo", "servoToggle", "control", "MTeach1", "{\"speed\":1}", g_now, "uteach"), 10));      // 教導頁送 {}（+speed）
        CHECK(!Run(g, Ma("btnServo", "servoToggle", "control", "MTeach1", "{\"speed\":1}", g_now + 50 * MS, "uteach"), 10));

        // (d) HOME：start=true 連兩下 -> 第二下 busy；start=true 後的 start=false（抬起＝停止）放行、不進表
        g_now += 1000 * MS;
        CHECK(Run(g, Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":true,\"speed\":5,\"currentPos\":0}", g_now), 15));
        const std::uint64_t Dh = g_now;
        CHECK(!Run(g, Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":true,\"speed\":5,\"currentPos\":0}", Dh + 100 * MS), 15));
        const std::size_t before = g.Size();
        CHECK(Run(g, Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":false,\"speed\":5,\"currentPos\":0}", Dh + 150 * MS), 15));
        CHECK(Run(g, Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":false,\"speed\":5,\"currentPos\":0}", g_now + 1 * MS), 15));
        CHECK(g.Size() == before);
        CHECK(!Run(g, Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":true,\"speed\":5,\"currentPos\":0}", Dh + 300 * MS), 15));   // 三連點的第三下（又是開始）
        // Loop Move：同
        const char* lpOn  = "{\"start\":true,\"mode\":0,\"confirmNotHomed\":false,\"pos1\":0,\"pos2\":50,\"waitTime\":0}";
        const char* lpOff = "{\"start\":false,\"mode\":0,\"confirmNotHomed\":false,\"pos1\":0,\"pos2\":50,\"waitTime\":0}";
        CHECK(Run(g, Ma("btnLoopMove", "loopMove", "motion", "MInArmY", lpOn, g_now), 10));
        CHECK(!Run(g, Ma("btnLoopMove", "loopMove", "motion", "MInArmY", lpOn, g_now + 80 * MS), 10));
        CHECK(Run(g, Ma("btnLoopMove", "loopMove", "motion", "MInArmY", lpOff, g_now + 90 * MS), 10));

        // (e) Motor Power、選軸：照一般規則
        g_now += 1000 * MS;
        CHECK(Run(g, Ma("btnMotorPower", "motorPowerToggle", "control", "", "{\"confirmOff\":false,\"expectRelayOn\":false}", g_now), 10));
        CHECK(!Run(g, Ma("btnMotorPower", "motorPowerToggle", "control", "", "{\"confirmOff\":false,\"expectRelayOn\":false}", g_now + 150 * MS), 10));
        CHECK(Run(g, Ma("labName", "selectMotor", "edit", "MInArmX", "{}", g_now), 5));
        CHECK(!Run(g, Ma("labName", "selectMotor", "edit", "MInArmX", "{}", g_now + 60 * MS), 5));
        CHECK(Run(g, Ma("labName", "selectMotor", "edit", "MInArmY", "{}", g_now + 70 * MS), 5));

        // (f) jog 每 20 ms、motor.access 的 stop、速度捲軸 5 -> 6 -> 5（來回）：全部放行、不進表
        g_now += 1000 * MS;
        const std::size_t n0 = g.Size();
        bool all = true;
        for (int i = 0; i < 5; ++i)
            all = Run(g, Ma("sbMotorTest_JogP", "jogP", "motion", "MInArmX", "{\"speed\":5,\"currentPos\":0}", g_now + 20 * MS), 2) && all;
        all = Run(g, Ma("btnStop", "stop", "control", "MInArmX", "{}", g_now), 2) && all;
        all = Run(g, Ma("btnStop", "stop", "control", "MInArmX", "{}", g_now), 2) && all;
        all = Run(g, Ma("scrlbrMotorSpeed", "setSpeed", "edit", "MInArmX", "{\"pct\":5,\"jog\":true}", g_now), 3) && all;
        all = Run(g, Ma("scrlbrMotorSpeed", "setSpeed", "edit", "MInArmX", "{\"pct\":6,\"jog\":true}", g_now + 30 * MS), 3) && all;
        all = Run(g, Ma("scrlbrMotorSpeed", "setSpeed", "edit", "MInArmX", "{\"pct\":5,\"jog\":true}", g_now + 60 * MS), 3) && all;
        CHECK(all);
        CHECK(g.Size() == n0);

        // (g) 教導頁：改過速度後第一下帶 speedEvent、第二下沒有 —— 照樣 busy；另一個教導點（params.btn）不擋
        g_now += 1000 * MS;
        CHECK(Run(g, Ma("btnMoveP", "moveRelative", "motion", "MTeach1", "{\"speed\":20,\"speedEvent\":true,\"interval\":5}", g_now, "uteach"), 10));
        CHECK(!Run(g, Ma("btnMoveP", "moveRelative", "motion", "MTeach1", "{\"speed\":20,\"interval\":5}", g_now + 120 * MS, "uteach"), 10));
        CHECK(Run(g, Ma("GoButton*", "teachGo", "motion", "MTeach1", "{\"speed\":20,\"btn\":\"GoButton140\",\"fields\":{},\"backlashIn\":0,\"backlashOut\":0}", g_now, "uteach"), 10));
        CHECK(Run(g, Ma("GoButton*", "teachGo", "motion", "MTeach1", "{\"speed\":20,\"btn\":\"GoButton020\",\"fields\":{},\"backlashIn\":0,\"backlashOut\":0}", g_now + 50 * MS, "uteach"), 10));
        CHECK(!Run(g, Ma("GoButton*", "teachGo", "motion", "MTeach1", "{\"speed\":20,\"btn\":\"GoButton020\",\"fields\":{},\"backlashIn\":0,\"backlashOut\":0}", g_now + 100 * MS, "uteach"), 10));
    }

    std::printf("[17] motor.access key: per-press fields dropped, the operator's choices kept; not JSON -> raw string\n");
    {
        const char* pa = "{\"speed\":5,\"currentPos\":100,\"interval\":10,\"targetPos\":110}";
        const char* pb = "{\"speed\":5,\"currentPos\":250,\"interval\":10,\"targetPos\":260}";
        const std::uint64_t k1 = WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", pa, T0));
        const std::uint64_t k2 = WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", pa, T0));
        CHECK(k1 == k2);                                                                                           // seq／id／issuedAt 不同
        CHECK(k1 == WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", pb, T0)));     // currentPos／targetPos（相對移動）
        CHECK(k1 != WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX",
                                          "{\"speed\":5,\"currentPos\":100,\"interval\":1,\"targetPos\":101}", T0)));   // 距離
        CHECK(k1 != WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX",
                                          "{\"speed\":6,\"currentPos\":100,\"interval\":10,\"targetPos\":110}", T0)));  // 速度
        CHECK(k1 != WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmY", pa, T0)));      // 軸
        CHECK(k1 != WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveN", "moveRelative", "motion", "MInArmX", pa, T0)));      // 鈕
        CHECK(k1 != WebCmdGuard::KeyOf(Ma("sbMotorTest_MoveP", "moveRelative", "motion", "MInArmX", pa, T0, "uteach")));   // 頁
        // targetPos 只在相對移動拿掉：絕對移動的目標是輸入值
        CHECK(WebCmdGuard::KeyOf(Ma("btnGo", "moveAbsolute", "motion", "MInArmX", "{\"targetPos\":50}", T0)) !=
              WebCmdGuard::KeyOf(Ma("btnGo", "moveAbsolute", "motion", "MInArmX", "{\"targetPos\":60}", T0)));
        // servoOn 只在 servoToggle 拿掉
        CHECK(WebCmdGuard::KeyOf(Ma("btnServoOff", "servoToggle", "control", "MInArmX", "{\"servoOn\":true}", T0)) ==
              WebCmdGuard::KeyOf(Ma("btnServoOff", "servoToggle", "control", "MInArmX", "{\"servoOn\":false}", T0)));
        CHECK(WebCmdGuard::KeyOf(Ma("btnHome", "home", "motion", "MInArmX", "{\"servoOn\":true}", T0)) !=
              WebCmdGuard::KeyOf(Ma("btnHome", "home", "motion", "MInArmX", "{\"servoOn\":false}", T0)));
        // start 留著（抬起方向本來就放行；開始 vs 停止是兩件事）
        CHECK(WebCmdGuard::KeyOf(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":true}", T0)) !=
              WebCmdGuard::KeyOf(Ma("btnHome", "home", "motion", "MInArmX", "{\"start\":false}", T0)));
        // speedEvent 拿掉
        CHECK(WebCmdGuard::KeyOf(Ma("btnMoveP", "moveRelative", "motion", "MTeach1", "{\"speed\":20,\"speedEvent\":true,\"interval\":5}", T0, "uteach")) ==
              WebCmdGuard::KeyOf(Ma("btnMoveP", "moveRelative", "motion", "MTeach1", "{\"speed\":20,\"interval\":5}", T0, "uteach")));
        // 不是 JSON 物件：退回原字串（不同字串不同 key、同字串同 key）
        CHECK(WebCmdGuard::KeyOf(Cmd("motor.access", 0, "abc", T0)) == WebCmdGuard::KeyOf(Cmd("motor.access", 0, "abc", T0)));
        CHECK(WebCmdGuard::KeyOf(Cmd("motor.access", 0, "abc", T0)) != WebCmdGuard::KeyOf(Cmd("motor.access", 0, "abd", T0)));
        CHECK(WebCmdGuard::KeyOf(Cmd("motor.access", 0, "[1]", T0)) != WebCmdGuard::KeyOf(Cmd("motor.access", 0, "[2]", T0)));
        // 其他指令不受影響：value 原字串（seq 不同就是不同 key）
        CHECK(WebCmdGuard::KeyOf(Cmd("towerlight.op", 0, "{\"op\":\"click\",\"seq\":1}", T0)) !=
              WebCmdGuard::KeyOf(Cmd("towerlight.op", 0, "{\"op\":\"click\",\"seq\":2}", T0)));
    }

    std::printf("[18] W906IoClickGuardScope: same button + same down within W is busy; a different down always runs (Jimmy 08:3x); other buttons run\n");
    {
        CHECK(WebCmdGuard::Exempt(Io("C_Load_Up", 1, T0)));                 // 名稱級白名單還在：主分派迴圈頭不擋、不記
        {
            WebCmdGuard g(W, FakeNow);
            g_now = T0;
            { W906CmdGuardScope s(g, Io("C_Load_Up", 1, T0)); CHECK(!s.busy()); }
            CHECK(g.Size() == 0);
        }
        WebCmdGuard g(W, FakeNow);
        g_now = T0;
        CHECK(RunIo(g, Io("C_Load_Up", 1, T0), 5));                        // 開
        const std::uint64_t D = g_now;
        std::string why;
        CHECK(!RunIo(g, Io("C_Load_Up", 1, D + 120 * MS), 5, &why));       // 同一顆鈕、同一個 down（雙擊）：busy
        CHECK(why == "busy: same button and state within 400 ms (io.btnPanelClick C_Load_Up down=1, 120 ms ago)");
        CHECK(StartsWith(why, "busy:"));
        // Jimmy 08:3x：400 ms 內送「開」再送「關」，「關」一定要執行（不然輸出留在開）
        CHECK(RunIo(g, Io("C_Load_Up", 0, D + 150 * MS), 5));              // 關：down 不同 → 放行
        const std::uint64_t D2 = g_now;                                     // = D + 155 ms
        CHECK(RunIo(g, Io("C_Load_Up", 1, D2 + 20 * MS), 5));              // 又開：跟上一下（關）不同 → 放行（開→關→開 三下都跑）
        const std::uint64_t D3 = g_now;                                     // = D + 180 ms
        CHECK(!RunIo(g, Io("C_Load_Up", 1, D3 + 30 * MS), 5));             // 再開一次（跟上一下相同）：busy
        CHECK(RunIo(g, Io("C_Load_Up", 0, D3 + 40 * MS), 5));              // 關：放行（被擋的那下不算「上一次」）
        const std::uint64_t D4 = g_now;                                     // = D + 225 ms
        CHECK(!RunIo(g, Io("C_Load_Up", 0, D4 + 50 * MS), 5));             // 再關一次：busy
        CHECK(RunIo(g, Io("C_Load_UpOff", 0, D4 + 60 * MS), 5));           // 另一顆鈕（Loader 的另一半）：不擋
        CHECK(RunIo(g, Io("C_Load_UpOff", 1, D4 + 70 * MS), 5));           //   它自己換狀態也放行
        CHECK(!RunIo(g, Io("C_Load_Up", 0, D4 + (W - 1) * MS), 5));        // 上一下（關）完成後 W-1：還是 busy
        CHECK(RunIo(g, Io("C_Load_Up", 0, D4 + (W + 1) * MS), 5));         // 上一下完成後 W+1：同一個 down 也放行
        CHECK(g.Blocked() == 4);
        // 第一下還在跑（輸出優先服務裡的 Poll 讓出時間）時到的：同一個 down busy、不同 down 放行
        g_now += 1000 * MS;
        const std::uint64_t t = g_now;
        CHECK(RunIo(g, Io("C_Auto1_Up", 1, t), 50));
        CHECK(!RunIo(g, Io("C_Auto1_Up", 1, t + 10 * MS), 5));             // pushed 早於第一下完成：佇列去重
        CHECK(RunIo(g, Io("C_Auto1_Up", 0, t + 20 * MS), 5));              // 關：放行
        // 巢狀（執行中又跑了一下）：外層開、裡面插進來一下關 → 表裡留較晚到的「關」
        g_now += 1000 * MS;
        {
            W906IoClickGuardScope a(g, Io("C_Nest", 1, g_now));
            CHECK(!a.busy());
            g_now += 2 * MS;
            { W906IoClickGuardScope c(g, Io("C_Nest", 1, g_now)); CHECK(c.busy()); }    // 同一個 down 插進來：busy
            { W906IoClickGuardScope b(g, Io("C_Nest", 0, g_now)); CHECK(!b.busy()); g_now += 2 * MS; }
            g_now += 2 * MS;
        }                                                                   // a 解構：自己的紀錄已被 b 刪掉 → 不蓋
        const std::uint64_t tn = g_now;
        CHECK(!RunIo(g, Io("C_Nest", 0, tn + 10 * MS), 1));                // 重複關：busy
        CHECK(RunIo(g, Io("C_Nest", 1, tn + 20 * MS), 1));                 // 開：放行
        // 沒有 tag：不是一顆鈕（本體拒絕），不擋也不記
        const std::size_t n = g.Size();
        CHECK(RunIo(g, Io("", 1, g_now), 1));
        CHECK(RunIo(g, Io("", 1, g_now), 1));
        CHECK(g.Size() == n);
        // key：主 guard 的 key 與鈕的 key 不相撞；down 算進去；不同鈕不同 key
        WebCommand nv = Cmd("io.btnPanelClick", "C_Load_Up", 0, T0);
        CHECK(WebCmdGuard::ButtonKeyOf(nv) != WebCmdGuard::KeyOf(nv));
        CHECK(WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0)) != WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 0, T0 + 5 * MS)));
        CHECK(WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0)) == WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0 + 5 * MS)));
        CHECK(WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0)) != WebCmdGuard::ButtonKeyOf(Io("C_Load_UpOff", 1, T0)));
        CHECK(WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0)) == WebCmdGuard::ButtonKeyOf(Io("C_Load_Up", 1, T0), 1));
        // down 的正規化（照 wb_serve W906_DispatchIoClick 的 ioDown ＋ Click_ 只收 0／1）
        {
            WebCommand c = Io("C_Load_Up", 1, T0);
            CHECK(WebCmdGuard::IoClickDownOf(c) == 1);
            c.value = TagValue::makeInt(0);           CHECK(WebCmdGuard::IoClickDownOf(c) == 0);
            c.value = TagValue::makeInt(2);           CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            c.value = TagValue::makeInt(-1);          CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            c.value = TagValue::makeDouble(1.0);      CHECK(WebCmdGuard::IoClickDownOf(c) == 1);
            c.value = TagValue::makeDouble(0.0);      CHECK(WebCmdGuard::IoClickDownOf(c) == 0);
            c.value = TagValue::makeDouble(0.7);      CHECK(WebCmdGuard::IoClickDownOf(c) == 0);    // (int)0.7＝0（wb_serve 同）
            c.value = TagValue::makeDouble(-0.5);     CHECK(WebCmdGuard::IoClickDownOf(c) == 0);    // (int)-0.5＝0
            c.value = TagValue::makeDouble(1.9);      CHECK(WebCmdGuard::IoClickDownOf(c) == 1);
            c.value = TagValue::makeDouble(2.0);      CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            c.value = TagValue::makeDouble(1e30);     CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            c.value = TagValue::makeBool(true);       CHECK(WebCmdGuard::IoClickDownOf(c) == 1);
            c.value = TagValue::makeBool(false);      CHECK(WebCmdGuard::IoClickDownOf(c) == 0);
            c.value = TagValue::makeString("1");      CHECK(WebCmdGuard::IoClickDownOf(c) == -1);   // 字串：本體拒絕
            c.value = TagValue::makeNull();           CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            c.hasValue = false;                       CHECK(WebCmdGuard::IoClickDownOf(c) == -1);
            // 正規化後相同的 down ＝ 同一個 key（1、1.0、true 是同一個狀態）
            WebCommand i1 = Io("C_Load_Up", 1, T0), d1 = i1, b1 = i1;
            d1.value = TagValue::makeDouble(1.0);
            b1.value = TagValue::makeBool(true);
            CHECK(WebCmdGuard::ButtonKeyOf(i1) == WebCmdGuard::ButtonKeyOf(d1));
            CHECK(WebCmdGuard::ButtonKeyOf(i1) == WebCmdGuard::ButtonKeyOf(b1));
            WebCmdGuard h(W, FakeNow);
            g_now = T0;
            CHECK(RunIo(h, i1, 5));
            CHECK(!RunIo(h, b1, 5));                                        // true 跟 1 同一個狀態：busy
            WebCommand bad = Io("C_Load_Up", 7, g_now + 10 * MS);
            CHECK(RunIo(h, bad, 1));                                        // 不合法的值（本體拒絕）：跟 1 不同 → 放行，交給本體回錯
            CHECK(RunIo(h, Io("C_Load_Up", 1, g_now + 10 * MS), 5));       // 然後再開：跟上一下（不合法）不同 → 放行
        }
        // W=0：全放行、不記
        WebCmdGuard z(0, FakeNow);
        g_now = T0;
        CHECK(RunIo(z, Io("C_Load_Up", 1, T0), 5));
        CHECK(RunIo(z, Io("C_Load_Up", 1, T0 + 1 * MS), 5));
        CHECK(RunIo(z, Io("C_Load_Up", 0, T0 + 2 * MS), 5));
        CHECK(z.Size() == 0);
        // 走 WebCmdGuardGlobal（真的 steady_clock；[15] 同一個實例）
        std::uint64_t expect = 400;
        WebCmdGuard::ParseWindowMs(std::getenv("W906_CMDGUARD_MS"), &expect);
        bool firstBusy = true;
        { W906IoClickGuardScope s(Io("C_Global_Test", 1, WebCmdGuard::SteadyNowUs())); firstBusy = s.busy(); }
        CHECK(!firstBusy);
        {
            W906IoClickGuardScope s2(Io("C_Global_Test", 1, WebCmdGuard::SteadyNowUs()));   // 同一個 down
            if (expect >= 50) CHECK(s2.busy() && StartsWith(s2.why(), "busy: same button and state within "));
            else              CHECK(expect != 0 || !s2.busy());
        }
        W906IoClickGuardScope s3(Io("C_Global_Test", 0, WebCmdGuard::SteadyNowUs()));        // 不同 down：一律放行
        CHECK(!s3.busy());
    }

    std::printf("%s (%d passed, %d failed)\n", g_fail ? "FAILED" : "ALL PASSED", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
