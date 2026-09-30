// =============================================================================
//  test_wb_server.cpp -- WebBridgeServer against a REAL loopback socket.
//
//  AI(W906-WebBridge) 20260805: this is the one WB-0 component that shipped with
//  no test at all, and it is the riskiest one: 1,300+ lines of socket code that
//  had never been executed. The other three WB tests are pure-function suites;
//  this one starts an actual server, connects an actual client, and speaks the
//  actual wire protocol from D:\HT9045\web\docs\ARCHITECTURE.md section 4.
//
//  TWO RULES THIS FILE OBEYS, BOTH LEARNED THE HARD WAY IN THIS TREE
//
//  1. EVERY blocking read has a deadline. A hung test does not fail, it wedges
//     the whole ctest run -- which happened here on 20260728 and produced a log
//     with nothing in it. RecvUntil()/RecvExactly() below cannot block forever.
//
//  2. NEVER a fixed port. Bind port 0 and read the OS-assigned port back with
//     BoundPort(). A hard-coded port makes the suite fail on a machine that
//     happens to be using it, which looks like a code regression and is not.
//
//  WHAT IS DELIBERATELY *NOT* ASSERTED
//  Timing. The server polls on a 50 ms select() timeout, so "a patch arrives
//  after a publish" is asserted with a generous deadline and never with a tight
//  one. A test that pins scheduler latency is a test that fails on a busy
//  build machine for no useful reason.
// =============================================================================

#ifndef FD_SETSIZE
#define FD_SETSIZE 128
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include "WebBridge/WebBridgeServer.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/CommandQueue.h"
#include "WebBridge/TagValue.h"
#include "WebBridge/WsFrame.h"
#include "WebBridge/WsHandshake.h"
#include "WebBridge/Base64.h"
#include "WebBridge/Sync.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <direct.h>
#include <string>
#include <vector>

using namespace webbridge;

// ===========================================================================
//  Harness
// ===========================================================================
static int g_total = 0;
static int g_fail = 0;

static void check(bool ok, const char* expr, const char* file, int line)
{
    ++g_total;
    if (!ok) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}

static void checkInt(long long got, long long want, const char* expr,
                     const char* file, int line)
{
    ++g_total;
    if (got != want) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s : got %lld want %lld\n", file, line, expr, got, want);
    }
}

static void checkStr(const std::string& got, const std::string& want,
                     const char* expr, const char* file, int line)
{
    ++g_total;
    if (got != want) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n      got  \"%s\"\n      want \"%s\"\n",
                    file, line, expr, got.c_str(), want.c_str());
    }
}

#define CHECK(cond)          check((cond), #cond, __FILE__, __LINE__)
#define CHECK_INT(got, want) checkInt((long long)(got), (long long)(want), #got, __FILE__, __LINE__)
#define CHECK_STR(got, want) checkStr((got), (want), #got, __FILE__, __LINE__)

// ===========================================================================
//  A minimal blocking client with deadlines
// ===========================================================================
namespace {

const int kDeadlineMs = 5000;   // generous: only trips on a genuine wedge

unsigned long NowMs()
{
    return static_cast<unsigned long>(::GetTickCount());
}

class Client {
public:
    // dec_(false) is load-bearing: this decoder consumes SERVER->client frames,
    // which RFC 6455 5.1 says must NOT be masked. Constructing it with the
    // default (requireMaskedInput=true) makes every read fail, and it fails as
    // "no message arrived" rather than as "the server masked", which is a
    // confusing way to lose an afternoon.
    Client() : s_(INVALID_SOCKET), dec_(false) {}
    ~Client() { Close(); }

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool Connect(unsigned short port)
    {
        Close();
        s_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_ == INVALID_SOCKET) return false;

        sockaddr_in a;
        std::memset(&a, 0, sizeof(a));
        a.sin_family = AF_INET;
        a.sin_port = ::htons(port);
        a.sin_addr.s_addr = ::inet_addr("127.0.0.1");
        if (::connect(s_, reinterpret_cast<sockaddr*>(&a), sizeof(a)) != 0) {
            Close();
            return false;
        }
        // Non-blocking, so every read below is governed by our own deadline
        // rather than by the OS default of "forever".
        u_long nb = 1;
        ::ioctlsocket(s_, FIONBIO, &nb);
        return true;
    }

    void Close()
    {
        if (s_ != INVALID_SOCKET) { ::closesocket(s_); s_ = INVALID_SOCKET; }
        in_.clear();
    }

    bool Send(const std::string& bytes)
    {
        size_t sent = 0;
        const unsigned long deadline = NowMs() + kDeadlineMs;
        while (sent < bytes.size()) {
            const int n = ::send(s_, bytes.data() + sent,
                                 static_cast<int>(bytes.size() - sent), 0);
            if (n > 0) { sent += static_cast<size_t>(n); continue; }
            if (n < 0 && ::WSAGetLastError() == WSAEWOULDBLOCK) {
                if (NowMs() > deadline) return false;
                ::Sleep(2);
                continue;
            }
            return false;
        }
        return true;
    }

    // Pump until `in_` contains `needle`, or the deadline expires.
    bool RecvUntil(const std::string& needle)
    {
        const unsigned long deadline = NowMs() + kDeadlineMs;
        for (;;) {
            if (in_.find(needle) != std::string::npos) return true;
            if (!PumpOnce(deadline)) return false;
        }
    }

    // Pump until `in_` holds at least n bytes.
    bool RecvAtLeast(size_t n)
    {
        const unsigned long deadline = NowMs() + kDeadlineMs;
        for (;;) {
            if (in_.size() >= n) return true;
            if (!PumpOnce(deadline)) return false;
        }
    }

    // Pump until the decoder yields at least one complete message.
    bool RecvMessage(WsMessage* out)
    {
        const unsigned long deadline = NowMs() + kDeadlineMs;
        for (;;) {
            if (!msgs_.empty()) {
                *out = msgs_.front();
                msgs_.erase(msgs_.begin());
                return true;
            }
            const size_t had = in_.size();
            if (!PumpOnce(deadline)) return false;
            if (in_.size() > had) {
                const std::string chunk = in_.substr(had);
                if (!dec_.Feed(chunk, &msgs_)) return false;
            }
        }
    }

    const std::string& buffer() const { return in_; }
    void clearBuffer() { in_.clear(); }

    // The client is the masking side (RFC 6455 5.1). maskKey is fixed rather
    // than random because a test that varies its input for no reason is a test
    // that fails intermittently for no reason.
    std::string MaskedText(const std::string& payload) const
    {
        return EncodeMaskedFrame(kWsText, payload, 0xA1B2C3D4u);
    }

private:
    bool PumpOnce(unsigned long deadline)
    {
        char buf[4096];
        const int n = ::recv(s_, buf, sizeof(buf), 0);
        if (n > 0) { in_.append(buf, static_cast<size_t>(n)); return true; }
        if (n == 0) return false;                       // peer closed
        if (::WSAGetLastError() != WSAEWOULDBLOCK) return false;
        if (NowMs() > deadline) return false;
        ::Sleep(2);
        return true;
    }

    SOCKET s_;
    std::string in_;
    WsDecoder dec_;                     // server->client frames are UNmasked
    std::vector<WsMessage> msgs_;
};

// ---------------------------------------------------------------------------
//  A throwaway document root, so the static tests do not depend on the real
//  D:\HT9045\web being present or unmodified.
// ---------------------------------------------------------------------------
std::string MakeDocRoot()
{
    char tmp[MAX_PATH];
    const DWORD n = ::GetTempPathA(MAX_PATH, tmp);
    if (n == 0) return std::string();
    std::string root(tmp, n);
    root += "wb_srv_docroot";
    ::_mkdir(root.c_str());

    struct { const char* name; const char* body; } files[] = {
        { "index.html", "<!doctype html><title>root</title>" },
        { "app.js",     "export const x = 1;\n" },
        { "s.css",      "body{color:#000}\n" },
        { "secret.txt", "must-not-be-reachable-via-traversal\n" },
    };
    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
        const std::string p = root + "\\" + files[i].name;
        FILE* f = std::fopen(p.c_str(), "wb");
        if (f) { std::fputs(files[i].body, f); std::fclose(f); }
    }
    return root;
}

std::string HttpGet(Client& c, unsigned short port, const std::string& target,
                    const char* method = "GET")
{
    if (!c.Connect(port)) return std::string();
    std::string req = std::string(method) + " " + target + " HTTP/1.1\r\n"
                      "Host: 127.0.0.1\r\n"
                      "Connection: close\r\n\r\n";
    if (!c.Send(req)) return std::string();
    // Servers close after the body here, so "read to close" is the terminator;
    // RecvUntil on a needle we may never get would burn the whole deadline.
    const unsigned long deadline = NowMs() + kDeadlineMs;
    while (NowMs() < deadline) {
        if (!c.RecvAtLeast(c.buffer().size() + 1)) break;   // false on close
    }
    return c.buffer();
}

int StatusOf(const std::string& response)
{
    // "HTTP/1.1 NNN ..."
    if (response.size() < 12) return -1;
    if (response.compare(0, 5, "HTTP/") != 0) return -1;
    return std::atoi(response.c_str() + 9);
}

bool HeaderContains(const std::string& response, const std::string& needle)
{
    const size_t end = response.find("\r\n\r\n");
    const std::string head = (end == std::string::npos) ? response
                                                        : response.substr(0, end);
    return head.find(needle) != std::string::npos;
}

WebBridgeConfig BaseConfig(const std::string& docRoot)
{
    WebBridgeConfig cfg;
    cfg.port = 0;                       // ephemeral -- see the file head
    cfg.documentRoot = docRoot;
    return cfg;
}

}  // namespace

// ===========================================================================
//  1. Defaults are the SAFE ones -- this is a safety assertion, not a style one
// ===========================================================================
static void TestSafeDefaults()
{
    std::printf("-- 1. safe defaults\n");
    WebBridgeConfig cfg;
    // Loopback only, and read-only. Both are stated as mandatory defaults in
    // ARCHITECTURE.md section 6 questions 2 and 3, because this endpoint can
    // eventually command machine motion.
    CHECK_STR(cfg.bindAddress, std::string("127.0.0.1"));
    CHECK(cfg.readOnly == true);
    CHECK_STR(cfg.wsPath, std::string("/ht9045"));
}

// ===========================================================================
//  2. Lifecycle
// ===========================================================================
static void TestLifecycle(const std::string& docRoot)
{
    std::printf("-- 2. lifecycle\n");

    // Stop() on a server that was never started must be safe.
    {
        WebBridgeServer s(BaseConfig(docRoot));
        s.Stop();
        s.Stop();
        CHECK(!s.IsRunning());
        CHECK_INT(s.BoundPort(), 0);
    }

    // Start / Stop repeatedly. Each Start must report a real port, and Stop
    // must be prompt -- a slow Stop() is what makes a shutdown path hang.
    for (int i = 0; i < 3; ++i) {
        WebBridgeServer s(BaseConfig(docRoot));
        std::string err;
        const bool ok = s.Start(&err);
        CHECK(ok);
        if (!ok) { std::printf("      start error: %s\n", err.c_str()); continue; }
        CHECK(s.IsRunning());
        CHECK(s.BoundPort() != 0);

        const unsigned long t0 = NowMs();
        s.Stop();
        const unsigned long dt = NowMs() - t0;
        CHECK(!s.IsRunning());
        // The poll interval is 50 ms; 2000 ms means the wakeup path is broken.
        CHECK(dt < 2000);
        if (dt >= 2000) std::printf("      Stop() took %lu ms\n", dt);

        s.Stop();   // idempotent
    }

    // Two servers alive at once must both work: WSAStartup/WSACleanup is
    // reference counted, and getting that wrong breaks the second one only.
    {
        WebBridgeServer a(BaseConfig(docRoot));
        WebBridgeServer b(BaseConfig(docRoot));
        CHECK(a.Start(0));
        CHECK(b.Start(0));
        CHECK(a.BoundPort() != 0);
        CHECK(b.BoundPort() != 0);
        CHECK(a.BoundPort() != b.BoundPort());
        a.Stop();
        // b must still be usable after a shut down and (naively) WSACleanup'd.
        Client c;
        const std::string r = HttpGet(c, b.BoundPort(), "/index.html");
        CHECK_INT(StatusOf(r), 200);
        b.Stop();
    }
}

// ===========================================================================
//  3. Static files
// ===========================================================================
static void TestStatic(const std::string& docRoot)
{
    std::printf("-- 3. static file serving\n");

    WebBridgeServer s(BaseConfig(docRoot));
    std::string err;
    if (!s.Start(&err)) {
        std::printf("      cannot start: %s -- skipping section\n", err.c_str());
        CHECK(false);
        return;
    }
    const unsigned short port = s.BoundPort();

    {   // "/" serves index.html
        Client c;
        const std::string r = HttpGet(c, port, "/");
        CHECK_INT(StatusOf(r), 200);
        CHECK(r.find("<title>root</title>") != std::string::npos);
    }

    {   // THE line that decides whether the UI loads at all: a browser refuses
        // to execute an ES module served with the wrong Content-Type.
        Client c;
        const std::string r = HttpGet(c, port, "/app.js");
        CHECK_INT(StatusOf(r), 200);
        CHECK(HeaderContains(r, "text/javascript"));
    }

    {   Client c;
        const std::string r = HttpGet(c, port, "/s.css");
        CHECK_INT(StatusOf(r), 200);
        CHECK(HeaderContains(r, "text/css"));
    }

    {   Client c;
        const std::string r = HttpGet(c, port, "/nope.html");
        CHECK_INT(StatusOf(r), 404);
    }

    {   // Methods other than GET/HEAD.
        Client c;
        const std::string r = HttpGet(c, port, "/index.html", "DELETE");
        CHECK_INT(StatusOf(r), 405);
    }

    // Traversal. Every one of these must be refused -- asserted by round trip,
    // not by reading a comment in HttpStatic.cpp that says they are.
    {
        const char* attacks[] = {
            "/../secret.txt",
            "/..%2fsecret.txt",
            "/%2e%2e%2fsecret.txt",
            "/%2e%2e/secret.txt",
            "/a/../../secret.txt",
            "/..\\secret.txt",
            "/....//secret.txt",
            "//secret.txt",
            "/C:/Windows/win.ini",
            "/%00index.html",
        };
        for (size_t i = 0; i < sizeof(attacks) / sizeof(attacks[0]); ++i) {
            Client c;
            const std::string r = HttpGet(c, port, attacks[i]);
            const int st = StatusOf(r);
            const bool refused = (st == 400 || st == 403 || st == 404);
            check(refused, attacks[i], __FILE__, __LINE__);
            // And whatever the status, the secret must never be in the body.
            check(r.find("must-not-be-reachable") == std::string::npos,
                  "traversal leaked the file body", __FILE__, __LINE__);
        }
    }

    s.Stop();
}

// ===========================================================================
//  4. WebSocket: handshake, snapshot, patch
// ===========================================================================
static void TestWebSocket(const std::string& docRoot)
{
    std::printf("-- 4. websocket snapshot + patch\n");

    TagSnapshot snap;
    snap.beginPublish();
    snap.stage("machine.state", TagValue::makeString("HALT"));
    snap.stage("temp.sv", TagValue::makeDouble(130.0));
    snap.stage("zone.heatgun.1", TagValue::makeNull());
    snap.commitPublish();

    WebBridgeConfig cfg = BaseConfig(docRoot);
    WebBridgeServer s(cfg);
    s.SetSnapshot(&snap);
    std::string err;
    if (!s.Start(&err)) {
        std::printf("      cannot start: %s -- skipping section\n", err.c_str());
        CHECK(false);
        return;
    }
    const unsigned short port = s.BoundPort();

    Client c;
    CHECK(c.Connect(port));

    const std::string key = "dGhlIHNhbXBsZSBub25jZQ==";
    std::string req =
        "GET /ht9045 HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + key + "\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    CHECK(c.Send(req));
    CHECK(c.RecvUntil("\r\n\r\n"));

    const std::string head = c.buffer();
    CHECK_INT(StatusOf(head), 101);
    // RFC 6455 section 1.3: this key has exactly one correct accept value.
    CHECK(HeaderContains(head, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));

    // Everything after the handshake is frames. Hand the residue to the decoder
    // by clearing the buffer boundary the client tracked.
    const size_t hdrEnd = head.find("\r\n\r\n") + 4;
    const std::string residue = head.substr(hdrEnd);
    c.clearBuffer();

    // A snapshot frame must arrive on connect (section 4).
    WsMessage m;
    bool got = false;
    if (!residue.empty()) {
        // The server may have coalesced the snapshot with the 101; feed it.
        WsDecoder d(false);
        std::vector<WsMessage> ms;
        if (d.Feed(residue, &ms) && !ms.empty()) { m = ms[0]; got = true; }
    }
    if (!got) got = c.RecvMessage(&m);

    CHECK(got);
    if (got) {
        CHECK_INT(m.opcode, (int)kWsText);
        CHECK(m.payload.find("\"type\":\"snapshot\"") != std::string::npos);
        CHECK(m.payload.find("\"machine.state\":\"HALT\"") != std::string::npos);
        // null must survive as null, NOT as 0 -- the browser renders them
        // differently and publishing 0 for an uninstalled zone is a misreport.
        CHECK(m.payload.find("\"zone.heatgun.1\":null") != std::string::npos);
    }

    s.Stop();
}

// ===========================================================================
//  5. Commands are refused while read-only (the section 6 q3 milestone)
// ===========================================================================
static void TestReadOnlyRefusal(const std::string& docRoot)
{
    std::printf("-- 5. read-only refuses commands\n");

    TagSnapshot snap;
    CommandQueue queue;
    WebBridgeConfig cfg = BaseConfig(docRoot);
    cfg.readOnly = true;

    WebBridgeServer s(cfg);
    s.SetSnapshot(&snap);
    s.SetCommandQueue(&queue);
    std::string err;
    if (!s.Start(&err)) {
        std::printf("      cannot start: %s -- skipping section\n", err.c_str());
        CHECK(false);
        return;
    }
    CHECK(s.IsReadOnly());

    Client c;
    CHECK(c.Connect(s.BoundPort()));
    std::string req =
        "GET /ht9045 HTTP/1.1\r\nHost: 127.0.0.1\r\n"
        "Upgrade: websocket\r\nConnection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    CHECK(c.Send(req));
    CHECK(c.RecvUntil("\r\n\r\n"));
    c.clearBuffer();

    CHECK(c.Send(c.MaskedText("{\"type\":\"cmd\",\"id\":7,\"cmd\":\"temp.setSV\","
                              "\"tag\":\"temp.sv\",\"value\":\"135\"}")));

    // Look for the ack among the frames the server sends (a snapshot may come
    // first). Bounded loop -- never "read until we see what we want".
    bool sawRefusal = false;
    for (int i = 0; i < 8 && !sawRefusal; ++i) {
        WsMessage m;
        if (!c.RecvMessage(&m)) break;
        if (m.payload.find("\"type\":\"ack\"") == std::string::npos) continue;
        sawRefusal = m.payload.find("\"ok\":false") != std::string::npos;
    }
    CHECK(sawRefusal);

    // And nothing may have reached the UI thread's queue.
    std::vector<WebCommand> drained;
    queue.drain(drained);
    CHECK_INT(drained.size(), 0);

    s.Stop();
}

// ===========================================================================
//  6. every queued command carries ITS connection's id
//     AI(W906-CONNID) 20260926: QueuePush never set WebCommand.connId, so every
//     command reached wb_serve with connId 0 and ui.windows.put registered every
//     browser tab as the same connection (the window registry keeps one frame per
//     connId -- WebWindowRegistry.cpp:179-186 -- so tabs overwrote each other).
//     ui.windows.put is exempt from the control token, so no auth is needed here.
// ===========================================================================
static void TestConnIdOnQueuedCommands(const std::string& docRoot)
{
    std::printf("-- 6. queued commands carry their connection id\n");

    TagSnapshot snap;
    CommandQueue queue;
    WebBridgeConfig cfg = BaseConfig(docRoot);
    cfg.readOnly = false;

    WebBridgeServer s(cfg);
    s.SetSnapshot(&snap);
    s.SetCommandQueue(&queue);
    s.SetReadOnly(false);
    std::string err;
    if (!s.Start(&err)) {
        std::printf("      cannot start: %s -- skipping section\n", err.c_str());
        CHECK(false);
        return;
    }

    const std::string req =
        "GET /ht9045 HTTP/1.1\r\nHost: 127.0.0.1\r\n"
        "Upgrade: websocket\r\nConnection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    Client a, b;
    CHECK(a.Connect(s.BoundPort()));
    CHECK(a.Send(req));
    CHECK(a.RecvUntil("\r\n\r\n"));
    a.clearBuffer();
    CHECK(b.Connect(s.BoundPort()));
    CHECK(b.Send(req));
    CHECK(b.RecvUntil("\r\n\r\n"));
    b.clearBuffer();

    CHECK(a.Send(a.MaskedText("{\"type\":\"cmd\",\"id\":11,\"cmd\":\"ui.windows.put\",\"value\":\"{}\"}")));
    CHECK(b.Send(b.MaskedText("{\"type\":\"cmd\",\"id\":12,\"cmd\":\"ui.windows.put\",\"value\":\"{}\"}")));

    // Bounded wait for both to reach the queue (the server thread pushes them).
    std::vector<WebCommand> got;
    for (int i = 0; i < 100 && got.size() < 2; ++i) {
        std::vector<WebCommand> d;
        queue.drain(d);
        got.insert(got.end(), d.begin(), d.end());
        if (got.size() < 2) Sleep(20);
    }
    CHECK_INT(got.size(), 2);
    if (got.size() == 2) {
        CHECK(got[0].cmd == "ui.windows.put" && got[1].cmd == "ui.windows.put");
        CHECK(got[0].connId != 0);                  // was always 0 before the fix
        CHECK(got[1].connId != 0);
        CHECK(got[0].connId != got[1].connId);      // two tabs = two connections
    }

    s.Stop();
}

// ===========================================================================
//  7. read-only queries need no control token, writes still do
//     AI(W906-Q2-S124) 20260927 (St02): Steven S124 = B -- contactct.get, counterclear.get and observer.get are exempt
//     by exact name (WebBridgeServer.cpp :1448; the laptop 20260927 10:2x "照你寫的做", no prefix).  This connection
//     never takes the token: the three reach the queue, and every write -- plus editlist.get (not read-only) and
//     look-alike names that only a prefix rule would pass -- is refused "not-operator" and never queued.
// ===========================================================================
static void TestReadQueriesWithoutToken(const std::string& docRoot)
{
    std::printf("-- 7. read-only queries need no token, writes still do\n");

    TagSnapshot snap;
    CommandQueue queue;
    WebBridgeConfig cfg = BaseConfig(docRoot);
    cfg.readOnly = false;

    WebBridgeServer s(cfg);
    s.SetSnapshot(&snap);
    s.SetCommandQueue(&queue);
    s.SetReadOnly(false);
    std::string err;
    if (!s.Start(&err)) {
        std::printf("      cannot start: %s -- skipping section\n", err.c_str());
        CHECK(false);
        return;
    }

    const std::string req =
        "GET /ht9045 HTTP/1.1\r\nHost: 127.0.0.1\r\n"
        "Upgrade: websocket\r\nConnection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    Client c;
    CHECK(c.Connect(s.BoundPort()));
    CHECK(c.Send(req));
    CHECK(c.RecvUntil("\r\n\r\n"));
    c.clearBuffer();

    static const char* const kRead[]  = { "contactct.get", "counterclear.get", "observer.get", "dialog.notifyAck" };   // AI(W906-J5-ACK) 20260930: + the notice acknowledgement (token-exempt like dialog.response; it is not a query, but the check is the same -- queued without the token)
    static const char* const kWrite[] = { "temp.setSV", "editlist.get", "editlist.save", "counterclear.do",
                                          "contactct.set", "observer.get2", "x.get" };
    const int nRead = 4, nWrite = 7;   // AI(W906-J5-ACK) 20260930: 3 -> 4
    char buf[160];
    for (int i = 0; i < nRead; ++i) {
        std::snprintf(buf, sizeof(buf), "{\"type\":\"cmd\",\"id\":%d,\"cmd\":\"%s\",\"value\":\"\"}", 100 + i, kRead[i]);
        CHECK(c.Send(c.MaskedText(buf)));
    }
    for (int i = 0; i < nWrite; ++i) {
        std::snprintf(buf, sizeof(buf), "{\"type\":\"cmd\",\"id\":%d,\"cmd\":\"%s\",\"value\":\"1\"}", 200 + i, kWrite[i]);
        CHECK(c.Send(c.MaskedText(buf)));
    }

    // every write gets an immediate not-operator ack; the three queries get none (they are queued for wb_serve)
    bool refused[7] = { false, false, false, false, false, false, false };
    int nRefused = 0, readAcked = 0;
    for (int i = 0; i < 40 && nRefused < nWrite; ++i) {
        WsMessage m;
        if (!c.RecvMessage(&m)) break;
        if (m.payload.find("\"type\":\"ack\"") == std::string::npos) continue;
        const size_t p = m.payload.find("\"id\":");
        if (p == std::string::npos) continue;
        const int id = std::atoi(m.payload.c_str() + p + 5);
        if (id >= 100 && id < 100 + nRead) { ++readAcked; continue; }
        if (id >= 200 && id < 200 + nWrite && !refused[id - 200] &&
            m.payload.find("\"ok\":false") != std::string::npos &&
            m.payload.find("not-operator") != std::string::npos) {
            refused[id - 200] = true;
            ++nRefused;
        }
    }
    CHECK_INT(nRefused, nWrite);
    CHECK_INT(readAcked, 0);

    std::vector<WebCommand> got;
    for (int i = 0; i < 100 && got.size() < (size_t)nRead; ++i) {
        std::vector<WebCommand> d;
        queue.drain(d);
        got.insert(got.end(), d.begin(), d.end());
        if (got.size() < (size_t)nRead) Sleep(20);
    }
    CHECK_INT(got.size(), nRead);
    bool seen[4] = { false, false, false, false };
    for (size_t i = 0; i < got.size(); ++i)
        for (int k = 0; k < nRead; ++k)
            if (got[i].cmd == kRead[k]) seen[k] = true;
    CHECK(seen[0] && seen[1] && seen[2] && seen[3]);

    s.Stop();
}

// ===========================================================================
//  main
// ===========================================================================
int main()
{
    const std::string docRoot = MakeDocRoot();
    if (docRoot.empty()) {
        std::printf("test_wb_server: cannot create a temp document root\n");
        return 1;
    }
    std::printf("doc root: %s\n", docRoot.c_str());

    TestSafeDefaults();
    TestLifecycle(docRoot);
    TestStatic(docRoot);
    TestWebSocket(docRoot);
    TestReadOnlyRefusal(docRoot);
    TestConnIdOnQueuedCommands(docRoot);
    TestReadQueriesWithoutToken(docRoot);                      // AI(W906-Q2-S124) 20260927

    std::printf("\ntest_wb_server: %d checks, %d failure(s)\n", g_total, g_fail);
    std::printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
