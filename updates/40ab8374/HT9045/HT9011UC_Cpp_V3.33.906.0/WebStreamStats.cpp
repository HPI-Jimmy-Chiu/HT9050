// WebStreamStats.cpp -- see WebStreamStats.h. AI(W906-STREAM-S1) 20260930: RULINGS_20260930 #12 stage 1, measurement only.
#include "WebStreamStats.h"

#include "WebBridge/TagSnapshot.h"
#include "WebBridge/WebBridgeServer.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>

namespace ht9045 {
namespace {

struct Window {
    bool          open = false;
    std::uint64_t t0Ms = 0;
    // publish (tick thread)
    unsigned long long pubN = 0, pubStagedSum = 0, pubStagedMax = 0, pubUsSum = 0, pubUsMax = 0;
    // api cache (tick thread)
    unsigned long long apiN = 0, apiUsSum = 0, apiUsMax = 0, apiIoBytes = 0, apiMotorBytes = 0;
    // server counters at the start of the window (cumulative in WebBridgeStats)
    bool               wsBase = false;
    unsigned long long snapN0 = 0, snapB0 = 0, patchN0 = 0, patchB0 = 0, pumpN0 = 0, pumpUs0 = 0;
};
Window g_w;

std::atomic<unsigned long long> g_httpN[kStreamHttpKinds];
std::atomic<unsigned long long> g_httpB[kStreamHttpKinds];

void Kb(char* out, std::size_t n, unsigned long long bytes)
{
    if (bytes >= 10ull * 1024 * 1024) std::snprintf(out, n, "%.1fMB", bytes / (1024.0 * 1024.0));
    else if (bytes >= 1024)           std::snprintf(out, n, "%.1fKB", bytes / 1024.0);
    else                              std::snprintf(out, n, "%sB", std::to_string(bytes).c_str());   // no %llu: msvcrt dialect
}

std::string U(unsigned long long v) { return std::to_string(v); }
std::string Ms(double ms) { char b[32]; std::snprintf(b, sizeof(b), "%.2f", ms); return b; }

bool StartsWith(const std::string& s, const char* p)
{
    std::size_t i = 0;
    for (; p[i]; ++i) if (i >= s.size() || s[i] != p[i]) return false;
    return true;
}

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

}  // namespace

std::uint64_t StreamNowUs()
{
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

void StreamNotePublish(std::size_t staged, std::uint64_t us)
{
    ++g_w.pubN;
    g_w.pubStagedSum += staged;
    if (staged > g_w.pubStagedMax) g_w.pubStagedMax = staged;
    g_w.pubUsSum += us;
    if (us > g_w.pubUsMax) g_w.pubUsMax = us;
}

void StreamNoteApiCache(std::uint64_t us, std::size_t ioRuntimeBytes, std::size_t motorRuntimeBytes)
{
    ++g_w.apiN;
    g_w.apiUsSum += us;
    if (us > g_w.apiUsMax) g_w.apiUsMax = us;
    g_w.apiIoBytes = ioRuntimeBytes;          // the size of the body a page gets (last build of the window)
    g_w.apiMotorBytes = motorRuntimeBytes;
}

void StreamNoteHttp(int kind, std::size_t bytes)
{
    if (kind < 0 || kind >= kStreamHttpKinds) return;
    g_httpN[kind].fetch_add(1, std::memory_order_relaxed);
    g_httpB[kind].fetch_add((unsigned long long)bytes, std::memory_order_relaxed);
}

int StreamHttpKindOfJsonPath(const std::string& path)
{
    const std::string p = Lower(path);
    const std::string::size_type slash = p.find_last_of('/');
    const std::string leaf = (slash == std::string::npos) ? p : p.substr(slash + 1);
    if (leaf == "production-update.json") return kStreamHttpProdUpdate;
    if (leaf == "alarm-dialog-request.json" || leaf == "message-dialog-request.json" || leaf == "dialog-close-request.json")
        return kStreamHttpMailbox;
    return kStreamHttpJsonOther;
}

StreamFamilies StreamCountFamilies(const webbridge::TagSnapshot& snap)
{
    StreamFamilies f = { 0, 0, 0, 0, 0 };
    const webbridge::TagSnapshotView v = snap.read();
    for (webbridge::TagMap::const_iterator it = v.tags.begin(); it != v.tags.end(); ++it) {
        ++f.total;
        const std::string& n = it->first;
        if (StartsWith(n, "pci1203."))         ++f.pci1203;
        else if (StartsWith(n, "secs.sv."))    ++f.secsSv;
        else if (StartsWith(n, "motionView.")) ++f.motionView;
        else                                   ++f.other;
    }
    return f;
}

bool StreamDue(std::uint64_t nowMs, std::uint64_t windowMs)
{
    if (!g_w.open) { g_w = Window(); g_w.open = true; g_w.t0Ms = nowMs; return false; }
    return nowMs - g_w.t0Ms >= windowMs;
}

std::string StreamTick(std::uint64_t nowMs, const webbridge::TagSnapshot* snap, const webbridge::WebBridgeStats* ws)
{
    char buf[1400];
    std::string line;
    const double secs = g_w.open ? (nowMs - g_w.t0Ms) / 1000.0 : 0.0;
    std::snprintf(buf, sizeof(buf), "[STREAM] %.1fs", secs);
    line += buf;
    line += " publish n=" + U(g_w.pubN) + " staged avg=" + U(g_w.pubN ? g_w.pubStagedSum / g_w.pubN : 0ull) + " max=" + U(g_w.pubStagedMax)
          + " ms avg=" + Ms(g_w.pubN ? (g_w.pubUsSum / (double)g_w.pubN) / 1000.0 : 0.0) + " max=" + Ms(g_w.pubUsMax / 1000.0);
    if (snap) {
        const StreamFamilies f = StreamCountFamilies(*snap);
        std::snprintf(buf, sizeof(buf), " | tags=%u pci1203=%u secs.sv=%u motionView=%u other=%u", (unsigned)f.total,
                      (unsigned)f.pci1203, (unsigned)f.secsSv, (unsigned)f.motionView, (unsigned)f.other);
    } else std::snprintf(buf, sizeof(buf), " | tags ?");
    line += buf;
    char io[32], mo[32];
    Kb(io, sizeof(io), g_w.apiIoBytes); Kb(mo, sizeof(mo), g_w.apiMotorBytes);
    line += " | apiCache n=" + U(g_w.apiN) + " ms avg=" + Ms(g_w.apiN ? (g_w.apiUsSum / (double)g_w.apiN) / 1000.0 : 0.0)
          + " max=" + Ms(g_w.apiUsMax / 1000.0) + " io=" + io + " motor=" + mo;
    if (ws) {
        const unsigned long long sn = ws->snapshotsSent - (g_w.wsBase ? g_w.snapN0 : ws->snapshotsSent);
        const unsigned long long sb = ws->snapshotBytes - (g_w.wsBase ? g_w.snapB0 : ws->snapshotBytes);
        const unsigned long long pn = ws->patchesSent - (g_w.wsBase ? g_w.patchN0 : ws->patchesSent);
        const unsigned long long pb = ws->patchBytes - (g_w.wsBase ? g_w.patchB0 : ws->patchBytes);
        const unsigned long long dn = ws->pumpRuns - (g_w.wsBase ? g_w.pumpN0 : ws->pumpRuns);
        const unsigned long long du = ws->pumpUs - (g_w.wsBase ? g_w.pumpUs0 : ws->pumpUs);
        char sbs[32], pbs[32];
        Kb(sbs, sizeof(sbs), sb); Kb(pbs, sizeof(pbs), pb);
        line += " | ws conns=" + std::to_string(ws->liveConnections) + " snapshot=" + U(sn) + "/" + sbs + " patch=" + U(pn) + "/" + pbs
              + " diff n=" + U(dn) + " ms avg=" + Ms(dn ? (du / (double)dn) / 1000.0 : 0.0)
              + (g_w.wsBase ? "" : " (first window: server counters baselined now)");
    } else line += " | ws ?";
    static const char* const kName[kStreamHttpKinds] = { "io", "motor", "mailbox", "prod", "json" };
    line += " | http";
    for (int k = 0; k < kStreamHttpKinds; ++k) {
        const unsigned long long n = g_httpN[k].exchange(0, std::memory_order_relaxed);
        const unsigned long long b = g_httpB[k].exchange(0, std::memory_order_relaxed);
        char bs[32];
        Kb(bs, sizeof(bs), b);
        line += std::string(" ") + kName[k] + "=" + U(n) + "/" + bs;
    }
    // open the next window, baselining the server's cumulative counters
    g_w = Window();
    g_w.open = true;
    g_w.t0Ms = nowMs;
    if (ws) {
        g_w.wsBase = true;
        g_w.snapN0 = ws->snapshotsSent; g_w.snapB0 = ws->snapshotBytes;
        g_w.patchN0 = ws->patchesSent;  g_w.patchB0 = ws->patchBytes;
        g_w.pumpN0 = ws->pumpRuns;      g_w.pumpUs0 = ws->pumpUs;
    }
    return line;
}

namespace { void (*g_sink)(const char*) = 0; }

void StreamSetSink(void (*sink)(const char* line)) { g_sink = sink; }

void StreamEmit(const std::string& line)
{
    static int s_console = -1;
    if (s_console < 0) { const char* e = std::getenv("W906_STREAM_STATS"); s_console = (e && e[0] == '0' && e[1] == 0) ? 0 : 1; }
    if (s_console) { std::printf("%s\n", line.c_str()); std::fflush(stdout); }
    if (g_sink) g_sink(line.c_str());
}

void StreamResetForTest()
{
    g_w = Window();
    for (int k = 0; k < kStreamHttpKinds; ++k) { g_httpN[k].store(0); g_httpB[k].store(0); }
}

}  // namespace ht9045
