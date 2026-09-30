// test_webstreamstats.cpp -- WebStreamStats (the [STREAM] measurement line).
// AI(W906-STREAM-S1) 20260930: RULINGS_20260930 #12 stage 1. Pure: no machine globals, no files, no sockets.
// Pins: the 10 s window (first call opens it, due after 10 s), publish / apiCache / http aggregation and reset per window,
// the /JSON path classifier (case-insensitive leaf), family counting on a real TagSnapshot, the server counters printed as
// the CHANGE since the previous window (the first window baselines them), and StreamEmit's sink.
#include "WebStreamStats.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "WebBridge/WebBridgeServer.h"

#include <cstdio>
#include <cstdlib>
#include <string>

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("FAIL line %d: %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)
static bool Has(const std::string& s, const char* sub)
{
    const bool h = s.find(sub) != std::string::npos;
    if (!h) std::printf("  missing \"%s\" in: %s\n", sub, s.c_str());
    return h;
}

static std::string g_sunk;
static int g_sinkCalls = 0;
static void Sink(const char* line) { g_sunk = line ? line : ""; ++g_sinkCalls; }

int main()
{
    using namespace ht9045;
    StreamResetForTest();

    // the classifier
    CHECK(StreamHttpKindOfJsonPath("/JSON/Production-update.json") == kStreamHttpProdUpdate);
    CHECK(StreamHttpKindOfJsonPath("/json/production-UPDATE.JSON") == kStreamHttpProdUpdate);
    CHECK(StreamHttpKindOfJsonPath("/JSON/runtime/Alarm-dialog-request.json") == kStreamHttpMailbox);
    CHECK(StreamHttpKindOfJsonPath("/JSON/Message-dialog-request.json") == kStreamHttpMailbox);
    CHECK(StreamHttpKindOfJsonPath("/JSON/Dialog-close-request.json") == kStreamHttpMailbox);
    CHECK(StreamHttpKindOfJsonPath("/JSON/Motor-runtime.json") == kStreamHttpJsonOther);

    // the window
    CHECK(!StreamDue(1000));          // opens at t=1000
    CHECK(!StreamDue(10999));
    CHECK(StreamDue(11000));

    StreamNotePublish(100, 2000);
    StreamNotePublish(300, 4000);
    StreamNoteApiCache(1500, 150000, 48000);
    StreamNoteHttp(kStreamHttpIoRuntime, 150000);
    StreamNoteHttp(kStreamHttpIoRuntime, 150000);
    StreamNoteHttp(kStreamHttpIoRuntime, 150000);
    StreamNoteHttp(kStreamHttpMailbox, 500);
    StreamNoteHttp(kStreamHttpMailbox, 700);
    StreamNoteHttp(99, 1);            // ignored
    StreamNoteHttp(-1, 1);            // ignored

    webbridge::TagSnapshot snap;
    snap.beginPublish();
    snap.stage("pci1203.axis.0.cmdPos", webbridge::TagValue::makeInt(1));
    snap.stage("pci1203.di.0", webbridge::TagValue::makeBool(true));
    snap.stage("secs.sv.1001", webbridge::TagValue::makeString("x"));
    snap.stage("motionView.trays.loader", webbridge::TagValue::makeString("y"));
    snap.stage("machine.state", webbridge::TagValue::makeString("idle"));
    snap.commitPublish();
    const StreamFamilies f = StreamCountFamilies(snap);
    CHECK(f.total == 5 && f.pci1203 == 2 && f.secsSv == 1 && f.motionView == 1 && f.other == 1);

    webbridge::WebBridgeStats ws;
    ws.snapshotsSent = 1; ws.snapshotBytes = 200; ws.patchesSent = 10; ws.patchBytes = 5000;
    ws.pumpRuns = 10; ws.pumpUs = 20000; ws.liveConnections = 1;

    const std::string l1 = StreamTick(11000, &snap, &ws);
    CHECK(Has(l1, "[STREAM] 10.0s publish n=2 staged avg=200 max=300 ms avg=3.00 max=4.00"));
    CHECK(Has(l1, "| tags=5 pci1203=2 secs.sv=1 motionView=1 other=1"));
    CHECK(Has(l1, "| apiCache n=1 ms avg=1.50 max=1.50 io=146.5KB motor=46.9KB"));
    CHECK(Has(l1, "| ws conns=1 snapshot=0/0B patch=0/0B diff n=0 ms avg=0.00 (first window: server counters baselined now)"));
    CHECK(Has(l1, "| http io=3/439.5KB motor=0/0B mailbox=2/1.2KB prod=0/0B json=0/0B"));

    // second window: only the change since the first one; the tick-side sums were reset
    CHECK(!StreamDue(15000));
    CHECK(StreamDue(21000));
    webbridge::WebBridgeStats ws2 = ws;
    ws2.patchesSent += 5; ws2.patchBytes += 2500; ws2.pumpRuns += 5; ws2.pumpUs += 10000; ws2.snapshotsSent += 1; ws2.snapshotBytes += 220000;
    const std::string l2 = StreamTick(21000, 0, &ws2);
    CHECK(Has(l2, "[STREAM] 10.0s publish n=0 staged avg=0 max=0 ms avg=0.00 max=0.00"));
    CHECK(Has(l2, "| tags ?"));
    CHECK(Has(l2, "| apiCache n=0"));
    CHECK(Has(l2, "| ws conns=1 snapshot=1/214.8KB patch=5/2.4KB diff n=5 ms avg=2.00"));
    CHECK(l2.find("first window") == std::string::npos);
    CHECK(Has(l2, "| http io=0/0B motor=0/0B mailbox=0/0B prod=0/0B json=0/0B"));

    // null ws
    const std::string l3 = StreamTick(31000, 0, 0);
    CHECK(Has(l3, "| ws ?"));

    // the sink (the console copy also prints here; _putenv is not declared by MinGW.org under -std=c++17, so the
    // W906_STREAM_STATS=0 switch is not exercised by this test)
    StreamSetSink(&Sink);
    StreamEmit("[STREAM] probe");
    CHECK(g_sinkCalls == 1 && g_sunk == "[STREAM] probe");
    StreamSetSink(0);
    StreamEmit("[STREAM] probe 2");
    CHECK(g_sinkCalls == 1);

    std::printf("test_webstreamstats: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
