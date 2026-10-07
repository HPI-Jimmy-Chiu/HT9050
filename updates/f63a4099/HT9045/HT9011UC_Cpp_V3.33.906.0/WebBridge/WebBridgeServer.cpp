//Steven 20260916
// ----------------------------------------------------------------------
// AckJson()：成功時若第三個參數是 JSON 物件就併進 ack，不再丟掉。
// 原本那個參數只在失敗時輸出（它叫 error），於是 wb_serve 組好的
// {changed, identical, notFound} 從未送達瀏覽器 —— 前端的「notFound 就拒寫」
// 與變更確認對話框因此雙雙失效，按存檔顯示成功但什麼都沒寫。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

// 本地副本（HT9045 這棵樹）：D:\HT9045\backup\HT9045_V906_changes_20260916b\CHANGES_20260916_Steven.md

// =============================================================================
//  WebBridge/WebBridgeServer.cpp -- see WebBridgeServer.h for the threading
//  contract. If you are about to make this thread touch machine state, stop and
//  read ARCHITECTURE.md section 5 first.
// =============================================================================

// select() carries the listener + the wakeup socket + every client, so raise
// the fd_set capacity before <winsock2.h> is pulled in. maxConnections is
// clamped against this below.
#ifndef FD_SETSIZE
#define FD_SETSIZE 128
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include "WebBridge/WebBridgeServer.h"

#include "WebBridge/JsonWriter.h"
#include "WebBridge/TagJson.h"
#include "WebBridge/WsFrame.h"
#include "WebBridge/WsHandshake.h"
#include "WebBridge/Sync.h"

#include "Public/cJSON.h"
#include <memory>   //AI(W906-WSFANOUT) 20260926: std::shared_ptr for Conn::lastSent (PumpSnapshot); on the old blank line so no line below moves
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif

namespace webbridge {

// =============================================================================
//  SIBLING ADAPTER -- the ONLY code in this file that calls into the sibling
//  WebBridge components (WsHandshake / WsFrame / TagValue / TagSnapshot /
//  CommandQueue / JsonWriter), which are authored in parallel with this one.
//
//  This file was written against ASSUMED sibling signatures while the siblings
//  were being written in parallel. Every one of those guesses was wrong, and
//  they were reconciled here on 20260805. What follows is the REAL API this
//  adapter binds to -- kept accurate on purpose, because a comment describing
//  an API that does not exist costs more than no comment at all.
//
//    WsHandshake.h  std::string ComputeAcceptKey(const std::string& key);
//                       // base64(SHA1(key + RFC6455 GUID))
//    WsFrame.h      enum WsOpcode { kWsContinuation=0x0, kWsText=0x1,
//                       kWsBinary=0x2, kWsClose=0x8, kWsPing=0x9, kWsPong=0xA };
//                   std::string EncodeFrame(int opcode, const std::string& payload,
//                       bool fin=true, bool mask=false, uint32_t maskKey=0);
//                   class WsDecoder {                     // STATEFUL, per conn
//                       explicit WsDecoder(bool requireMaskedInput=true,
//                                          std::size_t maxMessageBytes=...);
//                       bool Feed(const std::string&, std::vector<WsMessage>*);
//                       bool Failed() const; uint16_t CloseCode() const;
//                       std::size_t PendingBytes() const, FragmentBytes() const; };
//                   // WsDecoder reassembles fragments and enforces masking,
//                   // UTF-8 and the size cap itself, so a delivered WsMessage
//                   // is always whole -- which is why sib::Frame::consumed is
//                   // dead and Conn::fragment/fragmenting are now unreachable.
//    TagValue.h     static TagValue makeNull()/makeBool(bool)/makeInt(int64)/
//                                   makeDouble(double)/makeString(const string&);
//                   TagType type() const;  asBool/asInt/asDouble/asString;
//                   bool operator==(const TagValue&) const;   // type-strict
//    TagSnapshot.h  TagSnapshotView read() const;   // { TagMap tags; uint64 generation; }
//                   TagPatch diffFrom(const TagSnapshotView&) const;
//                   // NOTE: diffFrom() would replace this file's hand-rolled
//                   // per-connection delta in PublishIfChanged(). Left alone for
//                   // now because that path has no test yet; see the DEVLOG.
//    CommandQueue.h struct WebCommand { uint64 id; string cmd; string tag;
//                                       bool hasTag; TagValue value;
//                                       bool hasValue; uint64 connId; };
//                   bool tryPush(const WebCommand&);   // false when full
//    JsonWriter.h   class JsonWriter { JsonWriter& BeginObject()/EndObject()/
//                       Key(const string&)/Null()/Bool(bool)/Number(wb_int64)/
//                       Number(double)/String(const string&)/RawValue(const string&);
//                       bool Ok() const; const std::string& Str() const; };
//                   std::string JsonQuote(const std::string& raw);
//                   // JsonWriter has its own JsonValue variant, unrelated to
//                   // TagValue; WebBridge/TagJson.h owns that mapping (both
//                   // directions) and sib::ObjectFrom forwards to it.
// =============================================================================
namespace sib {

// --- WsHandshake ------------------------------------------------------------
static std::string AcceptKey(const std::string& clientKey)
{
    return ComputeAcceptKey(clientKey);
}

// --- WsFrame ----------------------------------------------------------------
enum {
    kOpCont   = kWsContinuation,
    kOpText   = kWsText,
    kOpBinary = kWsBinary,
    kOpClose  = kWsClose,
    kOpPing   = kWsPing,
    kOpPong   = kWsPong
};

struct Frame {
    int         opcode;
    bool        fin;
    bool        masked;
    std::string payload;
    size_t      consumed;   // always 0: the decoder owns consumption now
    Frame() : opcode(0), fin(false), masked(false), consumed(0) {}
};

// Server-to-client frames are never masked (RFC 6455 section 5.1).
static std::string EncodeServerFrame(int opcode, const std::string& payload)
{
    return EncodeFrame(opcode, payload, /*fin=*/true, /*mask=*/false, /*maskKey=*/0);
}

// One per connection. WsDecoder is stateful -- it buffers partial frames and
// reassembles fragmented messages across TCP chunk boundaries -- so it cannot
// be a free function over the socket buffer the way this adapter first assumed.
struct Decoder {
    WsDecoder                dec;
    std::deque<WsMessage>    ready;

    // Server role: inbound frames MUST be masked, and the decoder enforces it.
    // The cap matches this file's own kMaxWsMessage so oversize is rejected by
    // the decoder (close 1009) rather than after reassembly.
    Decoder() : dec(/*requireMaskedInput=*/true, 64u * 1024u) {}
};

static size_t PendingBytes(const Decoder& d)
{
    return d.dec.PendingBytes() + d.dec.FragmentBytes();
}

// 1 = one message ready, 0 = need more bytes, -1 = protocol error.
//
// `in` is drained completely into the decoder; anything not yet a whole message
// stays inside the decoder, which is why Frame::consumed is always 0 and the
// caller must NOT erase from `in` itself.
static int TryDecode(Decoder& d, std::string& in, Frame& out)
{
    if (d.ready.empty()) {
        if (d.dec.Failed()) return -1;
        if (!in.empty()) {
            std::vector<WsMessage> got;
            const bool ok = d.dec.Feed(in, &got);
            in.clear();
            for (size_t i = 0; i < got.size(); ++i) d.ready.push_back(got[i]);
            if (!ok) {
                // Deliver whatever completed before the failure, then fail.
                if (d.ready.empty()) return -1;
            }
        }
        if (d.ready.empty()) return 0;
    }

    const WsMessage& m = d.ready.front();
    out.opcode  = m.opcode;
    // Data messages arrive fully reassembled, so a delivered message is always
    // a complete one; masking was enforced by the decoder before delivery.
    out.fin     = true;
    out.masked  = true;
    out.payload = m.payload;
    out.consumed = 0;
    d.ready.pop_front();
    return 1;
}

// --- TagValue ---------------------------------------------------------------
static TagValue MakeNull()                        { return TagValue::makeNull(); }
static TagValue MakeBool(bool v)                  { return TagValue::makeBool(v); }
static TagValue MakeNumber(double v)              { return TagValue::makeDouble(v); }
static TagValue MakeString(const std::string& v)  { return TagValue::makeString(v); }
static bool ValuesEqual(const TagValue& a, const TagValue& b) { return a == b; }

// --- TagSnapshot ------------------------------------------------------------
static unsigned long long SnapGeneration(const TagSnapshot* s)
{
    // generation() and NOT read().generation: read() copies the entire tag map
    // to build its view, and this is called on every poll iteration purely to
    // decide whether anything changed. Copying ~270 tags to learn "no" is the
    // kind of waste that only shows up under load.
    return s ? static_cast<unsigned long long>(s->generation()) : 0;
}

static void SnapRead(const TagSnapshot* s, std::map<std::string, TagValue>& out)
{
    out.clear();
    if (s) out = s->read().tags;
}

// --- CommandQueue -----------------------------------------------------------
static bool QueuePush(CommandQueue* q, unsigned long long ticket,
                      const std::string& cmd, const std::string& tag,
                      const TagValue& value, unsigned long long connId)   // AI(W906-CONNID) 20260926: 加 connId（見下面 c.connId）
{
    if (!q) return false;
    WebCommand c;
    c.id       = ticket;
    c.cmd      = cmd;
    c.tag      = tag;
    c.hasTag   = !tag.empty();
    c.value    = value;
    c.hasValue = !value.isNull();  c.connId = connId;   // AI(W906-CONNID) 20260926: 以前從沒設 ⇒ wb_serve 收到的每個指令 connId 都是 0，ui.windows.put 把每個分頁都登記成同一條連線、互相蓋掉（St01 18:35 報）
    return q->tryPush(c);
}

// --- JsonWriter -------------------------------------------------------------
// AI(W906-WebBridge-Tcp) 20260812: the TagValue -> JsonWriter mapping that used
// to live here as sib::WriteValue now lives in WebBridge/TagJson.cpp, which is
// also where the reverse direction lives. The Null vs "" distinction it
// preserves is unchanged and still load-bearing: the browser renders null as
// "---" and "" as blank, and collapsing them misreports an uninstalled device
// as a real zero.

// {"tag":value,"tag2":value2}
//
// AI(W906-WebBridge-Tcp) 20260812: delegated to WebBridge/TagJson.h. This used
// to be the only tag encoder in the tree; the TCP sidecar link now needs the
// identical bytes (and the decode direction, which has no counterpart here), so
// the definition moved to TagJson and this became a forwarder. Keeping a second
// hand-maintained copy is how the two wires would silently drift apart.
static std::string ObjectFrom(const std::map<std::string, TagValue>& m)
{
    return EncodeTagObject(m);
}

// A JSON string literal, quotes included.
static std::string QuoteString(const std::string& s)
{
    return JsonQuote(s);
}

}  // namespace sib
// =============================== END SIBLING ADAPTER =========================
void (*g_W906OpLogHook)(const char* kind, unsigned long long conn, double id, bool ok, const std::string& a, const std::string& b) = 0;   //AI(W906-OPLOG) 20260928: operation-log tap (wb_serve installs it: tools/wb_serve.cpp W906_OpLogInit). kind "RECV" (socket thread, a=cmd, b=raw frame) / "ACK" (socket thread SendAck, a=error) / "DONE" (tick thread CompleteCommand, a=result). 0 = off (tests, every other user). Occupies a blank line, no line moves
namespace {

const size_t kMaxHttpHead    = 32u * 1024u;   // request head before we give up
const size_t kMaxWsMessage   = 64u * 1024u;   // reassembled text message cap
const size_t kMaxPendingAcks = 4096u;         // unanswered CompleteCommand slots
const size_t kRecvChunk      = 8192u;

unsigned long long NowMs()
{
    using namespace std::chrono;
    return static_cast<unsigned long long>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

char LowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string Lower(const std::string& s)
{
    std::string o(s);
    for (size_t i = 0; i < o.size(); ++i) o[i] = LowerAscii(o[i]);
    return o;
}

std::string Trim(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' ||
                     s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

bool ContainsCI(const std::string& hay, const char* needle)
{
    return Lower(hay).find(needle) != std::string::npos;
}

// A command / tag name we are willing to hand to the UI thread. Deliberately
// narrow: these strings come off a socket and end up selecting machine actions.
bool IsSaneName(const std::string& s, size_t maxLen)
{
    if (s.size() > maxLen) return false;
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') ||
                        c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

std::string IsoLocalNow()
{
    std::time_t t = std::time(0);
    std::tm tmv;
#if defined(_MSC_VER)
    localtime_s(&tmv, &t);
#else
    std::tm* p = std::localtime(&t);
    if (p) tmv = *p; else std::memset(&tmv, 0, sizeof(tmv));
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
                  tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
                  tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return std::string(buf);
}

// --- Winsock reference counting ---------------------------------------------
// Two servers in one process, or a host app that already called WSAStartup,
// must both keep working. Winsock itself refcounts, but we still balance our
// own calls exactly so our WSACleanup never pulls the rug out from under the
// host application.
WbMutex g_wsaMx;
int        g_wsaRefs = 0;

bool WsaAcquire(std::string* err)
{
    WbGuard lk(g_wsaMx);
    if (g_wsaRefs > 0) { ++g_wsaRefs; return true; }
    WSADATA wsad;
    const int rc = WSAStartup(MAKEWORD(2, 2), &wsad);
    if (rc != 0) {
        if (err) {
            std::ostringstream os;
            os << "WSAStartup failed, rc=" << rc;
            *err = os.str();
        }
        return false;
    }
    g_wsaRefs = 1;
    return true;
}

void WsaRelease()
{
    WbGuard lk(g_wsaMx);
    if (g_wsaRefs <= 0) return;
    if (--g_wsaRefs == 0) WSACleanup();
}

void SetNonBlocking(SOCKET s)
{
    u_long nb = 1;
    ioctlsocket(s, FIONBIO, &nb);
}

std::string WsaErrText(const char* what, int code)
{
    std::ostringstream os;
    os << what << " failed, WSAGetLastError=" << code;
    return os.str();
}

}  // namespace

// =============================================================================
//  WebBridgeConfig / WebBridgeStats
// =============================================================================
WebBridgeConfig::WebBridgeConfig()
    : bindAddress("127.0.0.1"),   // loopback only -- see header
      port(8045),
      documentRoot(),
      maxConnections(16),
      readOnly(true),             // read-only by default -- see header
      wsPath("/ht9045"),
      pingIntervalMs(15000),
      idleTimeoutMs(45000),
      pollIntervalMs(50),
      controlIdleTimeoutMs(600000),   // AI(W906-FW-W3) 20260819: 10 min, design doc section 3
      checkOrigin(true),              // AI(W906-FW-C1WIRE) 20260911: see the header
      maxSendBacklog(256u * 1024u)
{
}

WebBridgeStats::WebBridgeStats()
    : httpRequests(0), wsAccepted(0), wsRejected(0), snapshotsSent(0),
      patchesSent(0), snapshotBytes(0), patchBytes(0), pumpRuns(0), pumpUs(0), pumpTags(0), alarmsSent(0), modalsSent(0), queriesSent(0), acksSent(0), cmdAccepted(0),   //AI(W906-STREAM-S1) 20260930: the five new counters, in declaration order
      cmdRejected(0), pingsSent(0), pongsReceived(0), slowClientDrops(0),
      connectionsAccepted(0), connectionsClosed(0), liveConnections(0)
{
}

// =============================================================================
//  Impl
// =============================================================================
class WebBridgeServer::Impl {
public:
    explicit Impl(const WebBridgeConfig& cfg);
    ~Impl();

    bool Start(std::string* errOut);
    void Stop();
    void Wake();

    void CompleteCommand(unsigned long long ticket, bool ok, const std::string& error);
    void PostAlarm(const std::string& code, const std::string& text, const std::string& at);
    void PostModal(const std::string& title, const std::string& text, const std::string& at);   // AI(W906-FW-W5a) 20260819
    void PostQuery(unsigned long long qid, const std::string& code, int kcodeMask, const std::string& at);   // AI(W906-FW-W5b) 20260819
    void PostQueryOptions(unsigned long long qid, const std::string& kind, const std::string& text,
                          const std::vector<std::string>& options, const std::string& at);   // AI(W906-YESNO) 20260925
    void ClearQuery(unsigned long long qid);   // AI(W906-Q30-REPLAY) 20260921

    WebBridgeConfig      cfg;
    HttpStatic           files;

    // AI(W906-FW-C1WIRE) 20260911: the dynamic route, checked BEFORE `files`.
    // Set once before Start(); read on the socket thread without a lock, which
    // is why the header makes install-before-Start part of the contract.
    std::string          routePrefix;
    HttpRouteFn          routeFn;
    void*                routeUser;
    TagSnapshot*         snapshot;
    CommandQueue*        queue;
    std::atomic<bool>    readOnly;
    // AI(W906-FW-W3) 20260819: single-operator control token. Owned by the
    // socket thread (all writes happen there); atomic so the UI thread's
    // ControlOwner() read is race-free. 0 = nobody holds it.
    std::atomic<unsigned long long> ctrlOwner_{0};  std::atomic<int> liveWs_{0};   // AI(W906-MODAL-WAKE) 20260926: live WebSocket connections (++ at the upgrade :1138, -- in CloseConn :892, 0 in CloseAllSockets :755) -- LiveWebSocketCount()
    unsigned long long              ctrlLastCmdMs_ = 0;   // socket thread only
    std::atomic<bool>    running;
    std::atomic<bool>    stopFlag;
    std::atomic<unsigned short> boundPort;

    mutable WbMutex   statsMx;
    WebBridgeStats       stats;

private:
    // --- one connection, owned solely by the socket thread ------------------
    struct Conn {
        SOCKET             s;
        unsigned long long id;
        bool               isWs;
        bool               closeAfterFlush;
        std::string        in;
        std::string        out;
        // The frame decoder is per-connection and stateful: it buffers partial
        // frames and reassembles fragmented messages across TCP chunk
        // boundaries. Because it reassembles, `fragment`/`fragmenting` below
        // are never exercised any more -- a delivered message is always whole.
        sib::Decoder       dec;
        std::string        fragment;      // text message being reassembled
        bool               fragmenting;
        bool               sentSnapshot;
        std::shared_ptr<const std::map<std::string, TagValue> > lastSent;   // this connection's view -- AI(W906-WSFANOUT) 20260926: SHARED, immutable (see PumpSnapshot); null = empty
        unsigned long long lastRecvMs;
        unsigned long long lastPingMs;
        bool               awaitingPong;

        Conn()
            : s(INVALID_SOCKET), id(0), isWs(false), closeAfterFlush(false),
              fragmenting(false), sentSnapshot(false), lastRecvMs(0),
              lastPingMs(0), awaitingPong(false) {}
    };

    struct Outgoing {
        unsigned long long connId;   // 0 = broadcast to every WS connection
        std::string        frame;
    };

    struct PendingAck {
        unsigned long long connId;
        double             browserId;
    };

    // WbThread takes a plain function pointer, so ThreadMain needs a static
    // trampoline. WebBridge/Sync.h explains why the standard thread type is
    // unusable on this tree's MinGW oracle.
    static void ThreadEntry(void* self);
    void ThreadMain();
    void CloseAllSockets();

    void AcceptNew();
    bool ReceiveInto(Conn& c);                 // false -> close this connection
    void Flush(Conn& c);
    void Enqueue(Conn& c, const std::string& bytes);
    void CloseConn(size_t index);

    bool ProcessHttpHead(Conn& c);             // false -> close
    bool DoWebSocketUpgrade(Conn& c, const std::string& target,
                            const std::map<std::string, std::string>& headers);
    bool ProcessWsBytes(Conn& c);              // false -> close
    void HandleTextMessage(Conn& c, const std::string& text);

    void PumpSnapshot(bool force);
    void PumpOutgoing();
    void PumpLiveness();

    void SendJson(Conn& c, const std::string& json);
    void SendAck(Conn& c, double id, bool ok, const std::string& error);
    static std::string AckJson(double id, bool ok, const std::string& error);

    std::vector<Conn>       conns_;
    WbThread                th_;
    WbMutex              lifeMx_;

    SOCKET                  listener_;
    SOCKET                  wake_;
    sockaddr_in             wakeAddr_;

    unsigned long long      lastGen_;
    unsigned long long      nextConnId_;
    std::atomic<unsigned long long> nextTicket_;

    WbMutex              outMx_;
    std::deque<Outgoing>    outQ_;

    // AI(W906-Q30-REPLAY) 20260921: 還沒被回答的那一個 query。
    //
    //   ⚠ 沒有這兩個欄位之前，`PostQuery` 只把 frame 以 `connId=0` 推進 `outQ_`
    //     一次就算了。警報觸發當下若沒有瀏覽器連著（或正好在重整），
    //     那個 frame 就廣播給空氣、**永遠不會再送**，而
    //     `tools/wb_serve.cpp` 的 `ForwardShowErrorMessage` 會 `for(;;)` 一直等
    //     ⇒ **只能重啟 wb_serve**。（Q30 第 3 題，20260921 量到。）
    //
    //   ⇒ 存一份，新連線握手完成、送完快照之後補發一次。
    //
    //   ⚠ 只存**一個**，不是佇列：`ForwardShowErrorMessage` 是同步阻塞的，
    //     同一時間不可能有第二個 query（它要等到這一個被回答才會返回）。
    //     存成佇列反而會讓「同時有兩個未答警報」看起來是可能的。
    //
    //   兩者都由 `outMx_` 保護 —— `PostQuery` 在 tick 執行緒、
    //   新連線補發在 socket 執行緒。
    std::string             pendingQueryFrame_;
    unsigned long long      pendingQueryQid_ = 0;      // 0 = 沒有待答的

    WbMutex              pendMx_;
    std::map<unsigned long long, PendingAck> pending_;
    std::deque<unsigned long long>           pendingOrder_;
};

// -----------------------------------------------------------------------------
WebBridgeServer::Impl::Impl(const WebBridgeConfig& c)
    : cfg(c),
      files(c.documentRoot),
      // AI(W906-FW-C1WIRE) 20260911: an uninitialised function pointer
      // would pass the `if (routeFn && ...)` guard on garbage and be
      // called. Placed here to match DECLARATION order (they are declared
      // beside `files`, not at the end) -- -Wreorder is an error budget
      // this library keeps at zero.
      routeFn(0),
      routeUser(0),
      snapshot(0),
      queue(0),
      readOnly(c.readOnly),
      running(false),
      stopFlag(false),
      boundPort(0),
      listener_(INVALID_SOCKET),
      wake_(INVALID_SOCKET),
      lastGen_(0),
      nextConnId_(1),
      nextTicket_(1)
{
    std::memset(&wakeAddr_, 0, sizeof(wakeAddr_));

    // select() must be able to hold listener + wakeup + every client.
    const int cap = static_cast<int>(FD_SETSIZE) - 8;
    if (cfg.maxConnections < 1)   cfg.maxConnections = 1;
    if (cfg.maxConnections > cap) cfg.maxConnections = cap;
    if (cfg.pollIntervalMs < 1)   cfg.pollIntervalMs = 1;
    if (cfg.wsPath.empty())       cfg.wsPath = "/ht9045";
    if (cfg.maxSendBacklog < 64u * 1024u) cfg.maxSendBacklog = 64u * 1024u;
}

WebBridgeServer::Impl::~Impl()
{
    Stop();
}

// -----------------------------------------------------------------------------
bool WebBridgeServer::Impl::Start(std::string* errOut)
{
    WbGuard lk(lifeMx_);
    if (running.load()) return true;   // idempotent

    if (!WsaAcquire(errOut)) return false;

    stopFlag.store(false);

    // --- listener, bound on the CALLING thread so failures are synchronous --
    listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_ == INVALID_SOCKET) {
        if (errOut) *errOut = WsaErrText("socket(listener)", WSAGetLastError());
        WsaRelease();
        return false;
    }

    // NOTE: SO_REUSEADDR is deliberately NOT set. On Windows it permits another
    // process to steal a bound port, and it would also mask a leaked listener
    // from our own lifecycle test.
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(cfg.port);
    {
        // getaddrinfo rather than inet_addr/inet_pton: present and
        // non-deprecated on both MinGW and MSVC.
        addrinfo hints;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags    = AI_NUMERICHOST;
        addrinfo* ai = 0;
        const std::string host = cfg.bindAddress.empty() ? std::string("127.0.0.1")
                                                         : cfg.bindAddress;
        int rc = getaddrinfo(host.c_str(), 0, &hints, &ai);
        if (rc != 0) {                       // allow "localhost" and friends
            hints.ai_flags = 0;
            rc = getaddrinfo(host.c_str(), 0, &hints, &ai);
        }
        if (rc != 0 || !ai) {
            if (ai) freeaddrinfo(ai);
            closesocket(listener_);
            listener_ = INVALID_SOCKET;
            if (errOut) *errOut = "cannot resolve bind address '" + host + "'";
            WsaRelease();
            return false;
        }
        const sockaddr_in* r = reinterpret_cast<const sockaddr_in*>(ai->ai_addr);
        addr.sin_addr = r->sin_addr;
        freeaddrinfo(ai);
    }

    if (bind(listener_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        const int e = WSAGetLastError();
        closesocket(listener_);
        listener_ = INVALID_SOCKET;
        if (errOut) *errOut = WsaErrText("bind", e);
        WsaRelease();
        return false;
    }
    if (listen(listener_, SOMAXCONN) == SOCKET_ERROR) {
        const int e = WSAGetLastError();
        closesocket(listener_);
        listener_ = INVALID_SOCKET;
        if (errOut) *errOut = WsaErrText("listen", e);
        WsaRelease();
        return false;
    }

    // Read back the real port so cfg.port == 0 (ephemeral) is usable.
    sockaddr_in got;
    std::memset(&got, 0, sizeof(got));
    int gotLen = sizeof(got);
    if (getsockname(listener_, reinterpret_cast<sockaddr*>(&got), &gotLen) == 0) {
        boundPort.store(ntohs(got.sin_port));
    } else {
        boundPort.store(cfg.port);
    }
    SetNonBlocking(listener_);

    // --- self-pipe: a loopback UDP socket we sendto() to break select() -----
    wake_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (wake_ == INVALID_SOCKET) {
        const int e = WSAGetLastError();
        closesocket(listener_);
        listener_ = INVALID_SOCKET;
        if (errOut) *errOut = WsaErrText("socket(wakeup)", e);
        WsaRelease();
        return false;
    }
    sockaddr_in wa;
    std::memset(&wa, 0, sizeof(wa));
    wa.sin_family = AF_INET;
    wa.sin_port   = 0;
    wa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(wake_, reinterpret_cast<sockaddr*>(&wa), sizeof(wa)) == SOCKET_ERROR) {
        const int e = WSAGetLastError();
        closesocket(wake_);   wake_ = INVALID_SOCKET;
        closesocket(listener_); listener_ = INVALID_SOCKET;
        if (errOut) *errOut = WsaErrText("bind(wakeup)", e);
        WsaRelease();
        return false;
    }
    int waLen = sizeof(wakeAddr_);
    if (getsockname(wake_, reinterpret_cast<sockaddr*>(&wakeAddr_), &waLen) != 0) {
        const int e = WSAGetLastError();
        closesocket(wake_);   wake_ = INVALID_SOCKET;
        closesocket(listener_); listener_ = INVALID_SOCKET;
        if (errOut) *errOut = WsaErrText("getsockname(wakeup)", e);
        WsaRelease();
        return false;
    }
    SetNonBlocking(wake_);

    lastGen_ = 0;
    running.store(true);
    th_.start(&WebBridgeServer::Impl::ThreadEntry, this);
    return true;
}

// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::Stop()
{
    WbGuard lk(lifeMx_);
    stopFlag.store(true);
    if (th_.joinable()) {
        Wake();                 // prompt: do not wait out pollIntervalMs
        th_.join();
    }
    const bool wasRunning = running.exchange(false);

    // The socket thread closes the listener and the client sockets on its way
    // out; the wakeup socket is ours to close once nobody can select() on it.
    if (wake_ != INVALID_SOCKET) { closesocket(wake_); wake_ = INVALID_SOCKET; }
    if (listener_ != INVALID_SOCKET) { closesocket(listener_); listener_ = INVALID_SOCKET; }

    {
        WbGuard ol(outMx_);
        outQ_.clear();
    }
    {
        WbGuard pl(pendMx_);
        pending_.clear();
        pendingOrder_.clear();
    }
    boundPort.store(0);
    if (wasRunning) WsaRelease();   // balances the Start() that succeeded
}

void WebBridgeServer::Impl::Wake()
{
    if (wake_ == INVALID_SOCKET) return;
    const char b = 'w';
    sendto(wake_, &b, 1, 0, reinterpret_cast<sockaddr*>(&wakeAddr_), sizeof(wakeAddr_));
}

// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::CloseAllSockets()
{
    for (size_t i = 0; i < conns_.size(); ++i) {
        if (conns_[i].s != INVALID_SOCKET) closesocket(conns_[i].s);
    }
    conns_.clear();  liveWs_.store(0);   // AI(W906-MODAL-WAKE) 20260926
    if (listener_ != INVALID_SOCKET) { closesocket(listener_); listener_ = INVALID_SOCKET; }
    WbGuard sl(statsMx);
    stats.liveConnections = 0;
}

// -----------------------------------------------------------------------------
//  The socket thread. Everything below runs here and nowhere else.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::ThreadEntry(void* self)
{
    static_cast<WebBridgeServer::Impl*>(self)->ThreadMain();
}

void WebBridgeServer::Impl::ThreadMain()
{
    while (!stopFlag.load()) {
        fd_set rd, wr;
        FD_ZERO(&rd);
        FD_ZERO(&wr);

        if (listener_ != INVALID_SOCKET &&
            static_cast<int>(conns_.size()) < cfg.maxConnections) {
            FD_SET(listener_, &rd);
        }
        if (wake_ != INVALID_SOCKET) FD_SET(wake_, &rd);

        for (size_t i = 0; i < conns_.size(); ++i) {
            FD_SET(conns_[i].s, &rd);
            if (!conns_[i].out.empty()) FD_SET(conns_[i].s, &wr);
        }

        timeval tv;
        tv.tv_sec  = cfg.pollIntervalMs / 1000;
        tv.tv_usec = (cfg.pollIntervalMs % 1000) * 1000;

        const int rc = select(0, &rd, &wr, 0, &tv);
        if (stopFlag.load()) break;

        if (rc == SOCKET_ERROR) {
            // A closed client between FD_SET and select() shows up here; drop
            // any dead socket and carry on rather than killing the thread.
            const int e = WSAGetLastError();
            if (e == WSAENOTSOCK || e == WSAEINVAL) {
                for (size_t i = conns_.size(); i-- > 0;) {
                    if (conns_[i].s == INVALID_SOCKET) CloseConn(i);
                }
                continue;
            }
            WbSleepMs(10);
            continue;
        }

        if (rc > 0) {
            if (wake_ != INVALID_SOCKET && FD_ISSET(wake_, &rd)) {
                char drain[64];
                sockaddr_in from;
                int fromLen = sizeof(from);
                while (recvfrom(wake_, drain, sizeof(drain), 0,
                                reinterpret_cast<sockaddr*>(&from), &fromLen) > 0) {
                    fromLen = sizeof(from);
                }
            }
            if (listener_ != INVALID_SOCKET && FD_ISSET(listener_, &rd)) AcceptNew();

            for (size_t i = conns_.size(); i-- > 0;) {
                Conn& c = conns_[i];
                bool keep = true;
                if (FD_ISSET(c.s, &rd)) keep = ReceiveInto(c);
                if (keep && FD_ISSET(c.s, &wr)) Flush(c);
                if (!keep) { CloseConn(i); continue; }
            }
        }

        PumpSnapshot(false);
        PumpOutgoing();
        PumpLiveness();

        // Flush whatever the pumps queued, and retire finished connections.
        for (size_t i = conns_.size(); i-- > 0;) {
            Conn& c = conns_[i];
            if (!c.out.empty()) Flush(c);
            if (c.out.size() > cfg.maxSendBacklog) {
                // One slow client must not stall the thread for everyone else.
                {
                    WbGuard sl(statsMx);
                    ++stats.slowClientDrops;
                }
                CloseConn(i);
                continue;
            }
            if (c.closeAfterFlush && c.out.empty()) { CloseConn(i); continue; }
            if (c.s == INVALID_SOCKET) { CloseConn(i); continue; }
        }
    }

    CloseAllSockets();
}

// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::AcceptNew()
{
    for (;;) {
        sockaddr_in from;
        int fromLen = sizeof(from);
        const SOCKET s = accept(listener_, reinterpret_cast<sockaddr*>(&from), &fromLen);
        if (s == INVALID_SOCKET) return;

        if (static_cast<int>(conns_.size()) >= cfg.maxConnections) {
            closesocket(s);   // at capacity: refuse now, do not queue work
            continue;
        }

        SetNonBlocking(s);
        // Small frames, latency matters more than packing.
        int one = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&one), sizeof(one));

        Conn c;
        c.s          = s;
        c.id         = nextConnId_++;
        c.lastRecvMs = NowMs();
        c.lastPingMs = c.lastRecvMs;
        conns_.push_back(c);

        WbGuard sl(statsMx);
        ++stats.connectionsAccepted;
        stats.liveConnections = static_cast<int>(conns_.size());
    }
}

void WebBridgeServer::Impl::CloseConn(size_t index)
{
    if (index >= conns_.size()) return;
    // AI(W906-FW-W3) 20260819: the control token dies with its connection
    // (design doc section 3, "持有權隨 ws 連線生命週期").
    if (conns_[index].id == ctrlOwner_.load()) ctrlOwner_.store(0);  if (conns_[index].isWs) liveWs_.fetch_sub(1);   // AI(W906-MODAL-WAKE) 20260926
    if (conns_[index].s != INVALID_SOCKET) closesocket(conns_[index].s);
    conns_.erase(conns_.begin() + static_cast<long>(index));
    WbGuard sl(statsMx);
    ++stats.connectionsClosed;
    stats.liveConnections = static_cast<int>(conns_.size());
}

bool WebBridgeServer::Impl::ReceiveInto(Conn& c)
{
    for (;;) {
        char buf[kRecvChunk];
        const int n = recv(c.s, buf, static_cast<int>(sizeof(buf)), 0);
        if (n > 0) {
            c.in.append(buf, static_cast<size_t>(n));
            c.lastRecvMs = NowMs();
            if (n < static_cast<int>(sizeof(buf))) break;
            continue;
        }
        if (n == 0) return false;                        // peer closed
        const int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) break;
        return false;
    }

    if (!c.isWs) {
        if (c.in.size() > kMaxHttpHead) return false;    // head never terminated
        return ProcessHttpHead(c);
    }
    return ProcessWsBytes(c);
}

void WebBridgeServer::Impl::Enqueue(Conn& c, const std::string& bytes)
{
    c.out += bytes;
}

void WebBridgeServer::Impl::Flush(Conn& c)
{
    while (!c.out.empty()) {
        const int n = send(c.s, c.out.data(), static_cast<int>(c.out.size()), 0);
        if (n > 0) {
            c.out.erase(0, static_cast<size_t>(n));
            continue;
        }
        const int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) return;    // stays buffered; write-set retries
        closesocket(c.s);
        c.s = INVALID_SOCKET;
        return;
    }
}

// -----------------------------------------------------------------------------
//  HTTP: either a WebSocket upgrade on cfg.wsPath, or a static file.
// -----------------------------------------------------------------------------
bool WebBridgeServer::Impl::ProcessHttpHead(Conn& c)
{
    const size_t end = c.in.find("\r\n\r\n");
    if (end == std::string::npos) return true;      // wait for the rest

    const std::string head = c.in.substr(0, end);
    c.in.erase(0, end + 4);

    // Request line.
    const size_t eol = head.find("\r\n");
    const std::string reqLine = (eol == std::string::npos) ? head : head.substr(0, eol);
    std::string method, target, version;
    {
        std::istringstream is(reqLine);
        is >> method >> target >> version;
    }

    // Headers.
    std::map<std::string, std::string> headers;
    if (eol != std::string::npos) {
        size_t p = eol + 2;
        while (p < head.size()) {
            size_t e2 = head.find("\r\n", p);
            if (e2 == std::string::npos) e2 = head.size();
            const std::string line = head.substr(p, e2 - p);
            const size_t colon = line.find(':');
            if (colon != std::string::npos) {
                const std::string k = Lower(Trim(line.substr(0, colon)));
                const std::string v = Trim(line.substr(colon + 1));
                if (headers.count(k)) headers[k] += ", " + v;
                else                  headers[k] = v;
            }
            p = e2 + 2;
        }
    }

    if (method.empty() || target.empty()) {
        HttpResponse bad;
        bad.status = 400; bad.reason = "Bad Request";
        bad.contentType = "text/plain; charset=utf-8";
        bad.body = "400 malformed request line\n";
        bad.contentLength = static_cast<long long>(bad.body.size());
        Enqueue(c, bad.ToWire());
        c.closeAfterFlush = true;
        return true;
    }

    std::string path = target;
    const size_t q = path.find_first_of("?#");
    if (q != std::string::npos) path.erase(q);

    const bool wantsUpgrade =
        ContainsCI(headers.count("upgrade") ? headers["upgrade"] : std::string(), "websocket") &&
        ContainsCI(headers.count("connection") ? headers["connection"] : std::string(), "upgrade");

    if (wantsUpgrade) {
        if (method != "GET" || path != cfg.wsPath) {
            {
                WbGuard sl(statsMx);
                ++stats.wsRejected;
            }
            HttpResponse bad;
            bad.status = 404; bad.reason = "Not Found";
            bad.contentType = "text/plain; charset=utf-8";
            bad.body = "404 no websocket endpoint here\n";
            bad.contentLength = static_cast<long long>(bad.body.size());
            Enqueue(c, bad.ToWire());
            c.closeAfterFlush = true;
            return true;
        }
        return DoWebSocketUpgrade(c, path, headers);
    }

    {
        WbGuard sl(statsMx);
        ++stats.httpRequests;
    }
    // AI(W906-FW-C1WIRE) 20260911: the dynamic route gets first refusal, and
    // only for an exact prefix match. It cannot shadow a file it does not claim:
    // a handler that returns false falls straight through to `files` below, so
    // installing a route can never make an existing URL stop working.
    if (routeFn && !routePrefix.empty() &&
        path.size() >= routePrefix.size() &&
        path.compare(0, routePrefix.size(), routePrefix) == 0) {
        std::string query;
        const size_t qmark = target.find('?');
        if (qmark != std::string::npos) {
            query = target.substr(qmark + 1);
            const size_t frag = query.find('#');
            if (frag != std::string::npos) query.erase(frag);
        }
        HttpResponse dyn;
        if (routeFn(routeUser, method, path, query, &dyn)) {
            // Content-Length is filled HERE, unconditionally, for anything that
            // is not a HEAD. HttpResponse defaults it to 0 (HttpStatic.cpp:308),
            // so a handler that fills `body` and forgets the length would
            // otherwise advertise 0 and the browser would render an empty
            // document -- a silent truncation that looks like a server bug.
            // A HEAD keeps whatever the handler set, since its body is
            // deliberately empty.
            if (!dyn.headOnly)
                dyn.contentLength = static_cast<long long>(dyn.body.size());
            Enqueue(c, dyn.ToWire());
            c.closeAfterFlush = true;
            return true;
        }
    }

    const HttpResponse res = files.Serve(method, target);
    Enqueue(c, res.ToWire());
    c.closeAfterFlush = true;      // one request per connection, then close
    return true;
}

bool WebBridgeServer::Impl::DoWebSocketUpgrade(
    Conn& c, const std::string& /*target*/,
    const std::map<std::string, std::string>& headers)
{
    std::map<std::string, std::string>::const_iterator itKey = headers.find("sec-websocket-key");
    std::map<std::string, std::string>::const_iterator itVer = headers.find("sec-websocket-version");

    const std::string key = (itKey == headers.end()) ? std::string() : itKey->second;
    const std::string ver = (itVer == headers.end()) ? std::string() : itVer->second;

    if (key.empty() || ver != "13") {
        {
            WbGuard sl(statsMx);
            ++stats.wsRejected;
        }
        HttpResponse bad;
        bad.status = 400; bad.reason = "Bad Request";
        bad.contentType = "text/plain; charset=utf-8";
        bad.body = key.empty() ? "400 missing Sec-WebSocket-Key\n"
                               : "400 unsupported websocket version\n";
        bad.contentLength = static_cast<long long>(bad.body.size());
        bad.extraHeaders["Sec-WebSocket-Version"] = "13";
        Enqueue(c, bad.ToWire());
        c.closeAfterFlush = true;
        return true;
    }

    // AI(W906-FW-C1WIRE) 20260911: the Origin gate. See WebBridgeConfig::
    // checkOrigin for the rule and for why an ABSENT Origin is allowed while
    // the literal "null" is not.
    if (cfg.checkOrigin) {
        std::map<std::string, std::string>::const_iterator itOrg = headers.find("origin");
        if (itOrg != headers.end()) {
            std::string org = itOrg->second;
            while (!org.empty() && (org[org.size() - 1] == '/' ||
                                    org[org.size() - 1] == ' '))
                org.erase(org.size() - 1);
            for (std::string::size_type i = 0; i < org.size(); ++i) {
                const unsigned char ch = static_cast<unsigned char>(org[i]);
                if (ch >= 'A' && ch <= 'Z') org[i] = static_cast<char>(ch - 'A' + 'a');
            }
            char want[6][64];
            const unsigned p = static_cast<unsigned>(boundPort);
            std::snprintf(want[0], sizeof(want[0]), "http://127.0.0.1:%u", p);
            std::snprintf(want[1], sizeof(want[1]), "http://localhost:%u", p);
            std::snprintf(want[2], sizeof(want[2]), "https://127.0.0.1:%u", p);
            std::snprintf(want[3], sizeof(want[3]), "https://localhost:%u", p);
            std::snprintf(want[4], sizeof(want[4]), "http://[::1]:%u", p);
            std::snprintf(want[5], sizeof(want[5]), "https://[::1]:%u", p);
            bool ok = false;
            for (int i = 0; i < 6 && !ok; ++i) ok = (org == want[i]);
            if (!ok) {
                {
                    WbGuard sl(statsMx);
                    ++stats.wsRejected;
                }
                HttpResponse bad;
                bad.status = 403; bad.reason = "Forbidden";
                bad.contentType = "text/plain; charset=utf-8";
                bad.body = "403 origin not permitted\n";
                bad.contentLength = static_cast<long long>(bad.body.size());
                Enqueue(c, bad.ToWire());
                c.closeAfterFlush = true;
                return true;
            }
        }
    }

    std::ostringstream os;
    os << "HTTP/1.1 101 Switching Protocols\r\n"
       << "Upgrade: websocket\r\n"
       << "Connection: Upgrade\r\n"
       << "Sec-WebSocket-Accept: " << sib::AcceptKey(key) << "\r\n"
       << "\r\n";
    Enqueue(c, os.str());

    c.isWs = true;  liveWs_.fetch_add(1);   // AI(W906-MODAL-WAKE) 20260926: counted only once the upgrade succeeded (HTTP requests never are)
    c.lastRecvMs = NowMs();
    c.lastPingMs = c.lastRecvMs;
    {
        WbGuard sl(statsMx);
        ++stats.wsAccepted;
    }

    // ARCHITECTURE.md section 4: full state on connect, deltas thereafter.
    std::shared_ptr<std::map<std::string, TagValue> > cur(new std::map<std::string, TagValue>());   //AI(W906-WSFANOUT) 20260926
    sib::SnapRead(snapshot, *cur);
    { const std::string sf = "{\"type\":\"snapshot\",\"data\":" + sib::ObjectFrom(*cur) + "}"; SendJson(c, sf); WbGuard sb(statsMx); stats.snapshotBytes += sf.size(); }   //AI(W906-STREAM-S1) 20260930: same frame as before, its size counted ([STREAM] ws snapshot bytes)
    c.lastSent     = cur;
    c.sentSnapshot = true;
    {
        WbGuard sl(statsMx);
        ++stats.snapshotsSent;
    }

    // AI(W906-Q30-REPLAY) 20260921: 若有**還沒被回答**的警報 query，補發給這條新連線。
    //
    //   為什麼必須做：`PostQuery` 是一次性廣播，只送給當下連著的人。
    //   而 `tools/wb_serve.cpp` 的 `ForwardShowErrorMessage` 會無限期等答案
    //  （那一點**是忠於 golden 的** —— golden `note.cpp:532` 的 modal 也永遠等）。
    //   ⇒ 警報觸發當下沒有瀏覽器、或瀏覽器正好在重整，
    //     沒有補發的話那台機器就再也解不開，只能重啟 wb_serve。
    //
    //   ⚠ 排在**快照之後**：瀏覽器要先有完整狀態才畫得出警報框的上下文
    //     （ARCHITECTURE.md §4 的「連上先給一份完整的」）。
    //
    //   ⚠ 先在鎖內複製再送，不在持鎖時呼叫 `SendJson` ——
    //     今天 `SendJson` 只寫 per-connection 緩衝不碰 `outMx_`，
    //     但那是實作細節，不該被這裡依賴。
    std::string replay;
    {
        WbGuard ol(outMx_);
        if (pendingQueryQid_ != 0) replay = pendingQueryFrame_;
    }
    if (!replay.empty()) {
        SendJson(c, replay);
        {
            WbGuard sl(statsMx);
            ++stats.queriesSent;      // 補發也算一次送出，才對得上帳
        }
    }

    // Any bytes the client pipelined behind the handshake are WS frames now.
    return c.in.empty() ? true : ProcessWsBytes(c);
}

// -----------------------------------------------------------------------------
//  WebSocket frames
// -----------------------------------------------------------------------------
bool WebBridgeServer::Impl::ProcessWsBytes(Conn& c)
{
    for (;;) {
        sib::Frame f;
        const int rc = sib::TryDecode(c.dec, c.in, f);
        if (rc == 0) {
            // Guard against a client that dribbles a giant frame header. The
            // decoder holds the partial bytes now, so ask it -- c.in has already
            // been drained into it and is always empty here.
            return sib::PendingBytes(c.dec) <= kMaxWsMessage + 1024u;
        }
        if (rc < 0) return false;                      // protocol error -> drop

        // RFC 6455 5.1: a client-to-server frame MUST be masked.
        if (!f.masked) return false;

        switch (f.opcode) {
            case sib::kOpPing:
                Enqueue(c, sib::EncodeServerFrame(sib::kOpPong, f.payload));
                break;

            case sib::kOpPong:
                c.awaitingPong = false;
                {
                    WbGuard sl(statsMx);
                    ++stats.pongsReceived;
                }
                break;

            case sib::kOpClose:
                Enqueue(c, sib::EncodeServerFrame(sib::kOpClose, std::string()));
                c.closeAfterFlush = true;
                return true;

            case sib::kOpBinary:
                return false;      // this protocol is JSON text only

            case sib::kOpText:
                if (f.fin) {
                    if (f.payload.size() > kMaxWsMessage) return false;
                    HandleTextMessage(c, f.payload);
                } else {
                    c.fragmenting = true;
                    c.fragment = f.payload;
                    if (c.fragment.size() > kMaxWsMessage) return false;
                }
                break;

            case sib::kOpCont:
                if (!c.fragmenting) return false;
                c.fragment += f.payload;
                if (c.fragment.size() > kMaxWsMessage) return false;
                if (f.fin) {
                    const std::string msg = c.fragment;
                    c.fragment.clear();
                    c.fragmenting = false;
                    HandleTextMessage(c, msg);
                }
                break;

            default:
                return false;      // reserved opcode
        }

        if (c.s == INVALID_SOCKET) return false;
    }
}

void WebBridgeServer::Impl::SendJson(Conn& c, const std::string& json)
{
    Enqueue(c, sib::EncodeServerFrame(sib::kOpText, json));
}

std::string WebBridgeServer::Impl::AckJson(double id, bool ok, const std::string& error)
{
    std::ostringstream os;
    os << "{\"type\":\"ack\",\"id\":";
    // Ids are integers on the wire (js/transport/ws.js counts 1,2,3...).
    const long long i = static_cast<long long>(id);
    if (static_cast<double>(i) == id) os << i; else os << id;
    os << ",\"ok\":" << (ok ? "true" : "false");
    if (!ok) {
        os << ",\"error\":" << sib::QuoteString(error);
    } else if (error.size() >= 2u && error[0] == '{' && error[error.size() - 1u] == '}') {
        //Steven 20260916
        // The third parameter is named `error`, but on SUCCESS a handler may pass
        // a JSON object instead -- the command's result. It used to be dropped
        // here, and that silently disabled the browser's whole write contract:
        //
        //   * wb_serve's system.file.put builds {"changed":N,"identical":N,
        //     "notFound":N} and hands it to CompleteCommand(). It never arrived,
        //     so every ack the browser saw was a bare {"type":"ack","ok":true}.
        //   * ht9045_wire_engine.js refuses to write when preview reports any
        //     notFound (rule 2, "the mapping is wrong, not the data"), and asks
        //     the operator to confirm the `changed` list. With both fields
        //     absent it read notFound as empty -- rule 2 could never fire -- and
        //     changed as empty, so save() always stopped at "nothing changed,
        //     not writing". No page could save anything, and nothing reported an
        //     error: the operator pressed Save and got a green message.
        //
        // Splice the object inline (drop its braces) rather than nesting it under
        // a "result" key, because that is the shape the client already reads
        // (p.changed / p.notFound straight off the ack).
        //
        // Every other call site passes an empty string on success and is
        // unaffected. A non-empty success string that is NOT a JSON object is
        // also left out, exactly as before -- this only ever adds fields that a
        // handler deliberately built.
        const std::string inner = error.substr(1u, error.size() - 2u);
        if (!inner.empty()) os << "," << inner;
    }
    os << "}";
    return os.str();
}

void WebBridgeServer::Impl::SendAck(Conn& c, double id, bool ok, const std::string& error)
{
    SendJson(c, AckJson(id, ok, error));  if (g_W906OpLogHook) g_W906OpLogHook("ACK", c.id, id, ok, error, std::string());   //AI(W906-OPLOG) 20260928
    WbGuard sl(statsMx);
    ++stats.acksSent;
}

// -----------------------------------------------------------------------------
//  One inbound JSON text message. Parsed with the vendored cJSON.
//
//  NOTHING here calls machine logic. A validated command is pushed onto the
//  CommandQueue and this function returns; the UI thread drains it later and
//  calls CompleteCommand(), which is what finally produces the ack.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::HandleTextMessage(Conn& c, const std::string& text)
{
    cJSON* root = cJSON_Parse(text.c_str());
    if (!root) {
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        SendAck(c, 0, false, "malformed json");
        return;
    }

    const cJSON* jType = cJSON_GetObjectItemCaseSensitive(root, "type");
    const std::string type = (jType && cJSON_IsString(jType) && jType->valuestring)
                             ? std::string(jType->valuestring) : std::string();

    const cJSON* jId = cJSON_GetObjectItemCaseSensitive(root, "id");
    const bool haveId = (jId && cJSON_IsNumber(jId));
    const double id = haveId ? jId->valuedouble : 0.0;

    if (type == "ping") {
        // Liveness. Answered from this thread: it touches nothing but the socket.
        SendAck(c, id, true, std::string());
        cJSON_Delete(root);
        return;
    }

    if (type != "cmd") {
        // Forward compatibility: unknown frame types are ignored, not fatal.
        cJSON_Delete(root);
        return;
    }

    const cJSON* jCmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    const std::string cmdName = (jCmd && cJSON_IsString(jCmd) && jCmd->valuestring)
                                ? std::string(jCmd->valuestring) : std::string();
    if (g_W906OpLogHook) g_W906OpLogHook("RECV", c.id, id, true, cmdName, text);   //AI(W906-OPLOG) 20260928: every cmd frame as received (occupies a blank line, no line moves)
    const cJSON* jTag = cJSON_GetObjectItemCaseSensitive(root, "tag");
    const std::string tagName = (jTag && cJSON_IsString(jTag) && jTag->valuestring)
                                ? std::string(jTag->valuestring) : std::string();

    std::string reject;
    if (!haveId)                            reject = "missing numeric id";
    else if (cmdName.empty())               reject = "missing cmd";
    else if (!IsSaneName(cmdName, 64))      reject = "illegal cmd name";
    else if (!tagName.empty() && !IsSaneName(tagName, 128)) reject = "illegal tag name";

    TagValue value = sib::MakeNull();
    if (reject.empty()) {
        const cJSON* jVal = cJSON_GetObjectItemCaseSensitive(root, "value");
        if (!jVal || cJSON_IsNull(jVal))    value = sib::MakeNull();
        else if (cJSON_IsBool(jVal))        value = sib::MakeBool(cJSON_IsTrue(jVal) != 0);
        else if (cJSON_IsNumber(jVal))      value = sib::MakeNumber(jVal->valuedouble);
        else if (cJSON_IsString(jVal) && jVal->valuestring)
                                            value = sib::MakeString(std::string(jVal->valuestring));
        else                                reject = "unsupported value type";
    }

    // The read-only gate. Default configuration lands here for every command.
    if (reject.empty() && readOnly.load()) reject = "bridge is read-only";
    if (reject.empty() && !queue)          reject = "no command queue attached";

    // AI(W906-FW-W3) 20260819: single-operator control token (design doc
    // section 3). control.acquire/release are answered HERE, from the socket
    // thread -- they touch nothing but server state, same in-thread rule as
    // "ping". Every other command except the auth.* family requires the
    // caller to BE the holder. Sits after the read-only gate on purpose: a
    // read-only bridge stays uniformly fail-closed for every cmd.
    if (reject.empty() && (cmdName == "control.acquire" || cmdName == "control.takeover")) {   //AI(W906-TAKEOVER) 20260926: EastSun「我進入IO頁面就應該把控制權拿回來」
        const unsigned long long owner = ctrlOwner_.load();
        if (owner == 0 || owner == c.id || cmdName == "control.takeover") {   //AI(W906-TAKEOVER) 20260926: takeover moves the token to this connection whoever holds it (loopback-only bridge, one operator at the HMI); the old holder learns it on its next command (not-operator). Same lines, no line moves.
            ctrlOwner_.store(c.id);
            ctrlLastCmdMs_ = NowMs();
            SendAck(c, id, true, std::string());
        } else {
            SendAck(c, id, false, "control-held");
        }
        cJSON_Delete(root);
        return;
    }
    if (reject.empty() && cmdName == "control.release") {
        if (ctrlOwner_.load() == c.id) {
            ctrlOwner_.store(0);
            SendAck(c, id, true, std::string());
        } else {
            SendAck(c, id, false, "not-operator");
        }
        cJSON_Delete(root);
        return;
    }
    // AI(W906-P6-WINREG) 20260920: `ui.windows.put` 與 `auth.*` 同列豁免。
    //
    //   它是**回報**，不是寫入 —— 瀏覽器只是告訴 C++「哪些視窗開著」
    //   （Steven `WINDOW_REGISTRY_CONTRACT.md` §4/§9）。
    //
    //   ⚠ 為什麼一定要豁免，而不是叫 background.html 去 acquire：
    //   `background.html` 是常駐的背景頁，它一旦拿到權杖就會**一路持有不放**，
    //   於是真正要存檔的那一頁（Contact / Teach / Speed）就再也拿不到 ——
    //   整個 HMI 的存檔會失效。把「回報」放進「單一操作員」的閘門裡，
    //   本來就是把兩種不同的東西混在一起。
    //
    //   ⚠ 刻意用**完全比對**而不是 `ui.` 前綴：前綴等於預先替所有未來的
    //   `ui.*` 指令開好洞，而其中很可能會有真的會寫東西的。豁免要一條一條給。
    //
    //   ⚠ 豁免也順帶代表它**不會**去更新 `ctrlLastCmdMs_`（權杖的閒置計時）。
    //   那是對的：背景頁推總表不該把別人的權杖續命。
    // AI(W906-JSONBRIDGE-S1) 20260923: `cfg.resync` 與 `log.event` 比照
    //   `ui.windows.put` 豁免，理由與上面那三個 ⚠ 完全同源。
    //
    //   `cfg.resync` —— **純讀**，只把「這顆 exe 是什麼組態建出來的」再說一次，
    //   不碰機台任何狀態。而它非豁免不可的理由跟 `ui.windows.put` 一樣尖銳：
    //   `background.html` 在渲染任何依機型而異的元件之前就需要這份組態，
    //   但它一旦 acquire 就會一路持有不放，真正要存檔的那一頁永遠拿不到。
    //   把「開機要問的事」放進「單一操作員」的閘門裡，是把兩種東西混在一起。
    //
    //   `log.event` —— 它是**回報**不是機台寫入：瀏覽器告訴 C++「操作員在畫面上
    //   做了什麼」，落點是留痕，不是機台。擋住它會讓稽核軌跡剛好在非操作員的
    //   那些頁面上破一個洞 —— 而那正是最需要知道「誰在什麼時候看了什麼」的地方。
    //   ⚠ 代價是任何連上的瀏覽器都能寫 ring。可接受，因為 ring 有界（200 筆）、
    //     每筆都標 origin、而且 `log.dropped` 會讓灌爆這件事看得見。
    //
    //   ⚠ 一樣用**完全比對**而不是前綴。`cfg.` 與 `log.` 底下都很可能長出真的
    //     會寫東西的指令（例如未來的 `cfg.put`），豁免要一條一條給。
    //   ⚠ 一樣不更新 `ctrlLastCmdMs_`：查組態與留痕都不該把別人的權杖續命。
    if (reject.empty()
        && cmdName.compare(0, 5, "auth.") != 0
        && cmdName != "ui.windows.put"
        && cmdName != "cfg.resync"
        && cmdName != "log.event" && cmdName != "modal.answer" && cmdName != "dialog.response" && cmdName != "dialog.notifyAck" && cmdName != "dialog.auth" /*AI(W906-D026) 20261001 St01: the alarm note's password answer, exempt like dialog.response (an alarm must be answerable without the token; the wait loop only takes it for the current qid)*/ && cmdName != "motor.stop" && cmdName != "act.home.abort" && cmdName != "contactct.get" && cmdName != "counterclear.get" && cmdName != "observer.get"   //AI(W906-HOMEMON) 20261001: act.home.abort exempt with motor.stop -- the Home Monitor's Abort Home (golden sbAbortHomeClick, stop direction) must not be held by a token in another tab; wb_serve runs it only while the home sequence shows the form (JsonBridge/ChanHome.cpp)
        && cmdName != "vacuum.get" && cmdName != "vacuum.open" && cmdName != "vacuum.close" && cmdName != "pad.get" && cmdName != "pad.close" /*AI(W906-W155) 20261007 (St02-E): the Pad window's poll (= its heartbeat) and its close (golden FormClose, bShow=false) are a READ and the safe direction -- exempt like vacuum.get / .close; pad.open / button / send / bling / exit write the pad COM and need the token*/ && cmdName != "act.observerSG.state" && cmdName != "panel.key" /*AI(W906-SOFTKEY-NOTOKEN) 20261003 (machine cpp 0157 f23150a6; ST02-P2 port 20261006 St02-E, without the machine's panel.estop -- RULINGS_20261005 #17): a screen panel key is a press sent without the token step, like the physical key; golden's own gates (ScanPannelKey / TfMain::ScanKey) still decide. Human review*/) {   //AI(W906-VACUNIT-1203) 20260930: HW.VacuumUnit's live view (golden tmr1Timer, polled every 1 s) is a READ -- exempt like observer.get, so the poll never takes the operator token (Motor Test's jog/HOME watchdog follows the holder). vacuum.setSV / do / setAll / reset are presses and stay behind the token.  // AI(W906-ALARM-ANSWER-TOKEN) 20260924: 告警回答豁免權杖 —— 使用者 F5 實測：MES0920 框上按 PAUSE 回 not-operator。ht9045_recipe_client.js:532-537 的 modalAnswer 刻意不 acquire（「警報一定要答得掉」，權杖可能在別的分頁），這裡卻沒豁免 ⇒ 告警框 iframe 的連線永遠答不掉。安全性不靠權杖：wb_serve 等待迴圈只收當前 qid＋有提供的選項（tools/wb_serve.cpp 回答處理），沒有告警時回 no query pending。⚠ c3c459f 起回答 START 會重走 StartFromWeb ⇒ 任何連上的瀏覽器答 START 都能讓機台繼續（golden 也是誰在框前誰按）  AI(W906-W4-MOTOR) 20260925: `motor.stop` 同列豁免 —— 停機不能被「權杖在別的分頁」擋住（golden 任何畫面的 STOP 都停得了；EastSun 的停止也不看控制權）。安全性不靠權杖：wb_serve 的 W906_MotorAccessWire 把 motor.stop 鎖死在 action=="stop"，其他動作走 motor.access 照樣要權杖。  AI(W906-Q2-S124) 20260927 (St02): Steven S124 = B -- the three read-only queries by exact name, no prefix (the laptop 20260927 10:2x: a `*.get` prefix would also pass a future `xxx.get` that writes). contactct.get / counterclear.get read, write no file and move nothing; observer.get is exempt HERE by name, but its four Yield acts (yieldSite / yieldMax / yieldMin / yieldClear) change memory, so wb_serve checks the token for them per act (WebCmdGuard::Exempt, AI(W906-Q2-OBS) 20260927, St02-E) -- the earlier "only read" wording was wrong. editlist.get stays token-gated (it is not read-only). ctest: tests/test_wb_server.cpp section 7.   AI(W906-J5-ACK) 20260930: `dialog.notifyAck` (INBOX 119) exempt by exact name with the two alarm answers -- the operator acknowledges a kCode==0 notice from the alarm iframe, which never acquires the token (same reason as AI(W906-ALARM-ANSWER-TOKEN)); it only retires a notice whose requestId matches (tools/wb_dialog_mailbox.h NotifyAckHandle), never starts anything.  ctest: tests/test_wb_server.cpp section 7   //AI(W906-ST02-OB7F) 20261004 (St02-E, NB2 R180 low (b)): + act.observerSG.state = the Observer SG_JamCount grid read back (ObserverSGJam.cpp op "state": no log line, no golden code), read only -- a viewer without the operator token sees the table (queryNow / queryYesterday stay operator-only)
        if (ctrlOwner_.load() != c.id) reject = "not-operator";
        else                           ctrlLastCmdMs_ = NowMs();
    }

    if (!reject.empty()) {
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        SendAck(c, id, false, reject);
        cJSON_Delete(root);
        return;
    }

    const unsigned long long ticket = nextTicket_++;  { WbGuard pl(pendMx_); PendingAck pa; pa.connId = c.id; pa.browserId = id; pending_[ticket] = pa; pendingOrder_.push_back(ticket); while (pendingOrder_.size() > kMaxPendingAcks) { pending_.erase(pendingOrder_.front()); pendingOrder_.pop_front(); } }   //AI(W906-IOWEB-P25c) 20260925: the pending ack is registered BEFORE the push. The tick thread now wakes on the push (CommandQueue::waitForPush) and a DO write takes microseconds, so it could CompleteCommand before the old registration below ran -> CompleteCommand found no ticket and dropped the ack silently; the browser then reported 'no ack' for a coil that DID switch (laptop review of P25, Q3-1)
    if (!sib::QueuePush(queue, ticket, cmdName, tagName, value, c.id)) {   // AI(W906-CONNID) 20260926: 帶上這條連線的 id（同上一行 PendingAck 的 connId）
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        { WbGuard pl(pendMx_); pending_.erase(ticket); }  SendAck(c, id, false, "command queue full");
        cJSON_Delete(root);
        return;
    }

    if (false) {   //AI(W906-IOWEB-P25c) 20260925: the registration moved to :1461 (kept here, dead, so no line below moves)   [AI(W906-MERGE-56bbf785) 20260926: now :1463 -- the laptop's PostQueryOptions declaration (W906-YESNO) added 2 lines at :420-421]
        WbGuard pl(pendMx_);
        PendingAck pa;
        pa.connId    = c.id;
        pa.browserId = id;
        pending_[ticket] = pa;
        pendingOrder_.push_back(ticket);
        while (pendingOrder_.size() > kMaxPendingAcks) {
            pending_.erase(pendingOrder_.front());
            pendingOrder_.pop_front();
        }
    }
    {
        WbGuard sl(statsMx);
        ++stats.cmdAccepted;
    }
    // No ack yet -- ARCHITECTURE.md section 5: the ack is sent when the UI
    // thread has actually processed the command (CompleteCommand).
    cJSON_Delete(root);
}

// -----------------------------------------------------------------------------
//  Snapshot -> per-connection patch frames.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpSnapshot(bool force)
{
    const unsigned long long gen = sib::SnapGeneration(snapshot);
    if (!force && gen == lastGen_) return;
    lastGen_ = gen;

    if (conns_.empty()) return;

    //AI(W906-WSFANOUT) 20260926: EastSun "遮 CW 極限 3 秒才顯示" -- measured: the socket thread at 100% of a core, every HTTP request
    //  waiting ~0.7 s for its first byte. This loop diffed ~6,400 tags and COPIED the whole map once PER WS CONNECTION (14 HMI
    //  windows) on every generation (~5/s: the 1203 Poll republishes). Same frames as before, computed once per BASELINE instead:
    //  lastSent is a shared immutable map, so connections in step share one pointer, one diff, one encoded frame, and `= cur` is O(1).
    std::shared_ptr<std::map<std::string, TagValue> > cur(new std::map<std::string, TagValue>());
    const std::chrono::steady_clock::time_point pumpT0 = std::chrono::steady_clock::now();  sib::SnapRead(snapshot, *cur);   //AI(W906-STREAM-S1) 20260930: time copy + diff + encode of one generation (added after the loop)
    std::vector<std::pair<std::shared_ptr<const std::map<std::string, TagValue> >, std::string> > frames;   // baseline -> frame ("" = no change)
    const std::map<std::string, TagValue> kEmpty;
    for (size_t i = 0; i < conns_.size(); ++i) {
        Conn& c = conns_[i];
        if (!c.isWs || !c.sentSnapshot || c.closeAfterFlush) continue;
        size_t f = 0;  while (f < frames.size() && frames[f].first != c.lastSent) ++f;   // held in `frames`, so a baseline cannot be freed and its address reused
        if (f == frames.size()) {
            // Deltas per BASELINE: a client that joined mid-run has a different baseline from one that has been watching for an hour.
            const std::map<std::string, TagValue>& base = c.lastSent ? *c.lastSent : kEmpty;
            std::map<std::string, TagValue> delta;
            for (std::map<std::string, TagValue>::const_iterator it = cur->begin(); it != cur->end(); ++it) {
                std::map<std::string, TagValue>::const_iterator prev = base.find(it->first);
                if (prev == base.end() || !sib::ValuesEqual(prev->second, it->second)) delta[it->first] = it->second;
            }
            // A tag that disappeared becomes null == "unknown / not installed" (ARCHITECTURE.md section 4 rule 3).
            for (std::map<std::string, TagValue>::const_iterator it = base.begin(); it != base.end(); ++it)
                if (cur->find(it->first) == cur->end()) delta[it->first] = sib::MakeNull();
            frames.push_back(std::make_pair(c.lastSent, delta.empty() ? std::string() : "{\"type\":\"patch\",\"data\":" + sib::ObjectFrom(delta) + "}"));
        }
        if (frames[f].second.empty()) continue;
        SendJson(c, frames[f].second);
        c.lastSent = cur;
        WbGuard sl(statsMx);
        ++stats.patchesSent;  stats.patchBytes += frames[f].second.size();   //AI(W906-STREAM-S1) 20260930
    }  { WbGuard sp(statsMx); ++stats.pumpRuns; stats.pumpUs += (unsigned long long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pumpT0).count(); stats.pumpTags = cur->size(); }   //AI(W906-STREAM-S1) 20260930: one generation diffed (also when no connection had a change)
}

// -----------------------------------------------------------------------------
//  Frames handed over by the UI thread (acks and alarms).
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpOutgoing()
{
    std::deque<Outgoing> batch;
    {
        WbGuard ol(outMx_);
        if (outQ_.empty()) return;
        batch.swap(outQ_);
    }

    for (std::deque<Outgoing>::const_iterator it = batch.begin(); it != batch.end(); ++it) {
        for (size_t i = 0; i < conns_.size(); ++i) {
            Conn& c = conns_[i];
            if (!c.isWs || c.closeAfterFlush) continue;
            if (it->connId != 0 && c.id != it->connId) continue;
            SendJson(c, it->frame);
        }
    }
}

// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpLiveness()
{
    const unsigned long long now = NowMs();

    // AI(W906-FW-W3) 20260819: idle operator forfeits the token (design doc
    // section 3, default 10 min without an accepted command).
    if (ctrlOwner_.load() != 0 && cfg.controlIdleTimeoutMs > 0 &&
        now - ctrlLastCmdMs_ > static_cast<unsigned long long>(cfg.controlIdleTimeoutMs)) {
        ctrlOwner_.store(0);
    }
    for (size_t i = conns_.size(); i-- > 0;) {
        Conn& c = conns_[i];
        if (!c.isWs) continue;

        if (cfg.idleTimeoutMs > 0 &&
            now - c.lastRecvMs > static_cast<unsigned long long>(cfg.idleTimeoutMs)) {
            CloseConn(i);          // dead peer, or one that stopped ponging
            continue;
        }
        if (cfg.pingIntervalMs > 0 &&
            now - c.lastPingMs >= static_cast<unsigned long long>(cfg.pingIntervalMs)) {
            Enqueue(c, sib::EncodeServerFrame(sib::kOpPing, std::string()));
            c.lastPingMs   = now;
            c.awaitingPong = true;
            WbGuard sl(statsMx);
            ++stats.pingsSent;
        }
    }
}

// -----------------------------------------------------------------------------
//  UI-thread entry points. Both only touch a mutex-guarded queue, then wake the
//  socket thread. Neither blocks.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::CompleteCommand(unsigned long long ticket, bool ok,
                                           const std::string& error)
{
    unsigned long long connId = 0;
    double browserId = 0.0;
    {
        WbGuard pl(pendMx_);
        std::map<unsigned long long, PendingAck>::iterator it = pending_.find(ticket);
        if (it == pending_.end()) return;      // connection already gone
        connId    = it->second.connId;
        browserId = it->second.browserId;
        pending_.erase(it);
    }
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = connId;
        o.frame  = AckJson(browserId, ok, error);
        outQ_.push_back(o);
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("DONE", connId, browserId, ok, error, std::string());   //AI(W906-OPLOG) 20260928: tick thread
}

void WebBridgeServer::Impl::PostAlarm(const std::string& code, const std::string& text,
                                     const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"alarm\",\"code\":" << sib::QuoteString(code)
       << ",\"text\":" << sib::QuoteString(text)
       << ",\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
    }
    {
        WbGuard sl(statsMx);
        ++stats.alarmsSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, 0, true, "alarm", os.str());   //AI(W906-OPLOG) 20260929: what the operator's screen was shown
}

// AI(W906-FW-W5a) 20260819: PostAlarm's sibling for display-only dialogs --
// same broadcast queue, same thread contract (see header).
void WebBridgeServer::Impl::PostModal(const std::string& title, const std::string& text,
                                      const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"modal\",\"title\":" << sib::QuoteString(title)
       << ",\"text\":" << sib::QuoteString(text)
       << ",\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
    }
    {
        WbGuard sl(statsMx);
        ++stats.modalsSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, 0, true, "modal", os.str());   //AI(W906-OPLOG) 20260929
}

// AI(W906-FW-W5b) 20260819: the answer-carrying sibling. One-way broadcast;
// the answer travels back as a normal queued command (see header).
void WebBridgeServer::Impl::PostQuery(unsigned long long qid, const std::string& code,
                                      int kcodeMask, const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"query\",\"qid\":" << qid
       << ",\"code\":" << sib::QuoteString(code)
       << ",\"kcode\":" << kcodeMask
       << ",\"options\":[";
    // AI(W906-Q30-KMAP) 20260921: golden 的 K_* 按鈕遮罩，**補滿 9 個**。
    //
    //   ⚠ 這裡原本只認 3 個（0x1 RETRY / 0x2 SKIP / 0x4 CLEAN_OUT）。
    //     後果不是「少幾顆鈕」而是**死結**：一個 `KCode` 只提供未對映按鈕的警報
    //    （例如 `K_RESET|K_HOME`）會讓 `options` 變成**空陣列**，
    //     瀏覽器就算做好了 dialog 也沒有按鈕可按，而
    //     `tools/wb_serve.cpp` 的 `ForwardShowErrorMessage` 會驗 `k & kcode`，
    //     所以硬送一個沒提供的也會被拒 —— `for(;;)` 永遠出不來。
    //
    //   清單與**順序**照 golden `note.cpp:1234-1235` 的那一對陣列：
    //     TBtnPanel *Ptr[]   = {BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd,
    //                           BtnCleanOut, BtnReset, BtnHome, BtnTrain, BtnOneCycle};
    //     int KeyComp[]      = {K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END,
    //                           K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN, K_ONECYCLE};
    //   （note.cpp 的 :1235 / :3529 / :3836 三處完全一致）
    //
    //   ⚠ **順序是照 golden 的顯示順序，不是照位元值**。理由不是美觀：
    //     操作員在 BCB6 機台上按的是固定位置的那一顆，換了順序就會按錯。
    //     ⇒ 這也是為什麼 RETRY 與 SKIP 的先後**跟以前不一樣** —— 以前是照位元
    //        值排的，那個順序在 golden 上不存在。
    //
    //   ⚠ **`K_FIX`（0x100）刻意不在列** —— golden 自己的註解寫
    //     「kevin 20130722 cancel K_FIX」。`K_PAUSE`（0x400）與 `K_START`（0x800）
    //     也不在 `KeyComp[]` 裡。⇒ 不要「順手補齊 12 個」，那會加出 golden 沒有的鈕。
    //
    //   位元值出處：`cmydef.cpp:337-347`（本樹）／golden `cmydef.h:263-274`。
    //   這裡照既有慣例硬寫數值而不 include cmydef.h —— 這一層不該相依機台標頭。
    static const struct { int bit; const char* name; } kButtons[] = {
        { 0x0002, "SKIP"      },
        { 0x0001, "RETRY"     },
        { 0x0008, "TRAY_FEED" },
        { 0x0010, "TRAY_END"  },
        { 0x0004, "CLEAN_OUT" },
        { 0x0020, "RESET"     },
        { 0x0040, "HOME"      },
        { 0x0080, "TRAIN"     },
        { 0x0200, "ONECYCLE"  },
    };
    bool first = true;
    for (std::size_t bi = 0; bi < sizeof(kButtons) / sizeof(kButtons[0]); ++bi) {
        if ((kcodeMask & kButtons[bi].bit) == 0) continue;
        os << (first ? "" : ",") << '"' << kButtons[bi].name << '"';
        first = false;
    }
    os << "],\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
        // AI(W906-Q30-REPLAY) 20260921: 同一份留著，給**之後才連上**的瀏覽器補發。
        //   廣播只送給當下連著的人；警報時剛好沒人連著的話，
        //   沒有這一行就再也沒有第二次機會（見上面欄位宣告處的說明）。
        pendingQueryFrame_ = o.frame;
        pendingQueryQid_   = qid;
    }
    {
        WbGuard sl(statsMx);
        ++stats.queriesSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "query", os.str());   //AI(W906-OPLOG) 20260929
}

// AI(W906-YESNO) 20260925: PostQuery 的兄弟 —— 選項名字由呼叫端直接給（見標頭）。
//   與 PostQuery 共用同一個廣播佇列、同一份補發槽（pendingQueryFrame_／pendingQueryQid_），
//   所以同一時間仍只有一個待答的 query —— 單執行緒的 tick 本來就不可能同時問兩題。
//   ⚠ `kcode` 固定寫 0：這一題沒有 K_* 遮罩。ht9045_dialog_host.js 只拿它印錯誤訊息，
//     答案對不對由 wb_serve 的等待迴圈照 options 判。
void WebBridgeServer::Impl::PostQueryOptions(unsigned long long qid, const std::string& kind,
                                             const std::string& text,
                                             const std::vector<std::string>& options,
                                             const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"query\",\"qid\":" << qid
       << ",\"code\":\"\",\"kcode\":0"
       << ",\"kind\":" << sib::QuoteString(kind)
       << ",\"text\":" << sib::QuoteString(text)
       << ",\"options\":[";
    for (std::size_t i = 0; i < options.size(); ++i)
        os << (i ? "," : "") << sib::QuoteString(options[i]);
    os << "],\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
        pendingQueryFrame_ = o.frame;   // 補發給之後才連上的瀏覽器（同 PostQuery）
        pendingQueryQid_   = qid;
    }
    {
        WbGuard sl(statsMx);
        ++stats.queriesSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "query", os.str());   //AI(W906-OPLOG) 20260929
}

// -----------------------------------------------------------------------------
//  AI(W906-Q30-REPLAY) 20260921: 宿主回答完（或放棄）之後清掉待答的 query。
//
//  ⚠ 一定要呼叫。不清的話，**下一個**連上來的瀏覽器會收到一個早就被回答過的
//    警報框，而且它送回來的 `modal.answer` 會被判成 `no query pending` ——
//    操作員會看到一個關不掉的框。
//
//  ⚠ 用 `qid` 比對而不是無條件清：若在極短時間內第一個被答完、第二個已經
//    `PostQuery` 上來，無條件清會把**新的**那一個抹掉。
//    （今天的同步模型不會發生，但這個保護是免費的，而且它讓這支函式可以
//     在不確定狀態下安全地被呼叫。）
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::ClearQuery(unsigned long long qid)
{
    if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "clear", std::string());  WbGuard ol(outMx_);   //AI(W906-OPLOG) 20260929: the dialog was answered or given up (logged before taking outMx_)
    if (pendingQueryQid_ == qid) {
        pendingQueryQid_ = 0;
        pendingQueryFrame_.clear();
    }
}

// =============================================================================
//  WebBridgeServer -- thin forwarding shell
// =============================================================================
WebBridgeServer::WebBridgeServer(const WebBridgeConfig& cfg)
    : impl_(new Impl(cfg))
{
}

WebBridgeServer::~WebBridgeServer()
{
    impl_->Stop();
}

void WebBridgeServer::SetSnapshot(TagSnapshot* snapshot)   { impl_->snapshot = snapshot; }
void WebBridgeServer::SetCommandQueue(CommandQueue* queue) { impl_->queue = queue; }
void WebBridgeServer::SetReadOnly(bool ro)                 { impl_->readOnly.store(ro); }
bool WebBridgeServer::IsReadOnly() const                   { return impl_->readOnly.load(); }
unsigned long long WebBridgeServer::ControlOwner() const   { return impl_->ctrlOwner_.load(); }  int WebBridgeServer::LiveWebSocketCount() const { return impl_->liveWs_.load(); }   // AI(W906-FW-W3) 20260819  // AI(W906-MODAL-WAKE) 20260926: LiveWebSocketCount

// AI(W906-FW-C1WIRE) 20260911: see the header for why this is a hook and not a
// method, and for the install-before-Start contract it relies on.
void WebBridgeServer::SetHttpRoute(const std::string& pathPrefix,
                                   HttpRouteFn fn, void* user) {
    impl_->routePrefix = pathPrefix;
    impl_->routeFn     = fn;
    impl_->routeUser   = user;
}
bool WebBridgeServer::Start(std::string* errOut)           { return impl_->Start(errOut); }
void WebBridgeServer::Stop()                               { impl_->Stop(); }
bool WebBridgeServer::IsRunning() const                    { return impl_->running.load(); }
unsigned short WebBridgeServer::BoundPort() const          { return impl_->boundPort.load(); }
void WebBridgeServer::Wake()                               { impl_->Wake(); }

void WebBridgeServer::CompleteCommand(unsigned long long ticket, bool ok,
                                      const std::string& error)
{
    impl_->CompleteCommand(ticket, ok, error);
}

void WebBridgeServer::PostAlarm(const std::string& code, const std::string& text,
                                const std::string& at)
{
    impl_->PostAlarm(code, text, at);
}

void WebBridgeServer::PostModal(const std::string& title, const std::string& text,
                                const std::string& at)   // AI(W906-FW-W5a) 20260819
{
    impl_->PostModal(title, text, at);
}

void WebBridgeServer::PostQuery(unsigned long long qid, const std::string& code,
                                int kcodeMask, const std::string& at)   // AI(W906-FW-W5b) 20260819
{
    impl_->PostQuery(qid, code, kcodeMask, at);
}

void WebBridgeServer::PostQueryOptions(unsigned long long qid, const std::string& kind,
                                       const std::string& text,
                                       const std::vector<std::string>& options,
                                       const std::string& at)   // AI(W906-YESNO) 20260925
{
    impl_->PostQueryOptions(qid, kind, text, options, at);
}

// AI(W906-Q30-REPLAY) 20260921: 見 Impl::ClearQuery 的說明。
void WebBridgeServer::ClearQuery(unsigned long long qid)
{
    impl_->ClearQuery(qid);
}

WebBridgeStats WebBridgeServer::Stats() const
{
    WbGuard lk(impl_->statsMx);
    return impl_->stats;
}

}  // namespace webbridge
