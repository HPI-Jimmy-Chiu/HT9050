// =============================================================================
//  WebBridge/WebBridgeServer.h -- the HTTP + WebSocket server that lets a
//  browser display real handler state.
//
//  Implements option A of D:\HT9045\web\docs\ARCHITECTURE.md section 3:
//  an embedded server inside the handler process, serving D:\HT9045\web as
//  static files and one WebSocket endpoint (default "/ht9045") speaking the
//  section-4 wire protocol.
//
//  THREADING CONTRACT -- ARCHITECTURE.md section 5. Read this before editing.
//  --------------------------------------------------------------------------
//  The handler's UI thread has hard obligations elsewhere (the SECS/GEM layer
//  has a 30-second reply budget; blocking it is a known failure mode in this
//  product). Therefore:
//
//    * This object owns ONE thread. It runs select() over the listener plus
//      every client socket. Not a thread per connection.
//    * That thread NEVER calls machine logic. Its entire view of the machine
//      is the TagSnapshot it reads and the CommandQueue it pushes into.
//    * State flows UI thread -> browser: the UI thread calls
//      TagSnapshot::Publish() on its existing timer tick; this server notices
//      the generation change and emits per-connection "patch" frames.
//    * Commands flow browser -> UI thread: the socket thread validates and
//      enqueues, then returns immediately. It does not wait.
//    * The ack for an accepted command is sent when the UI thread has actually
//      processed it and called CompleteCommand() -- section 5 again. Only
//      REJECTED commands (bad JSON, unknown shape, read-only, queue full) are
//      acked straight from the socket thread, because those never reach the UI
//      thread at all.
//
//  Which methods may be called from where:
//
//    UI thread (or any thread):  Start, Stop, SetReadOnly, CompleteCommand,
//                                PostAlarm, Wake, Stats, BoundPort, IsRunning
//    Socket thread (internal):   everything else
//
//  SAFETY POSTURE
//  --------------
//  bindAddress defaults to 127.0.0.1 (loopback ONLY) and readOnly defaults to
//  TRUE, because this endpoint can eventually command machine motion --
//  ARCHITECTURE.md section 6 questions 2 and 3. Widening either is an explicit,
//  deliberate act by the caller, never a default.
//
//  DEPENDENCIES ON SIBLING WebBridge COMPONENTS (written in parallel)
//  -----------------------------------------------------------------
//  Every call into a sibling component is confined to one clearly marked
//  "SIBLING ADAPTER" block at the top of WebBridgeServer.cpp. If a sibling's
//  real signature differs from the shape assumed there, that block is the only
//  place to fix -- nothing else in this file or the .cpp touches those APIs.
//  The assumed shapes are listed in that block.
//
//  This layer is independent of VCL: no <vcl.h>, no vclcompat, no AnsiString,
//  no machine headers.
// =============================================================================
#ifndef WEBBRIDGE_WEBBRIDGESERVER_H
#define WEBBRIDGE_WEBBRIDGESERVER_H

#include <memory>
#include <string>
#include <vector>                      // AI(W906-YESNO) 20260925: PostQueryOptions 的選項清單

#include "WebBridge/CommandQueue.h"
#include "WebBridge/HttpStatic.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

namespace webbridge {

// -----------------------------------------------------------------------------
struct WebBridgeConfig {
    // Loopback by default. "0.0.0.0" exposes this on the fab LAN, which
    // ARCHITECTURE.md section 6 question 2 says must never be the default.
    std::string    bindAddress;

    // 0 asks the OS for an ephemeral port; read it back with BoundPort().
    unsigned short port;

    // Directory served as "/" -- normally D:\HT9045\web. Empty = 404 for all
    // static requests (the WebSocket endpoint still works).
    std::string    documentRoot;

    // Simultaneous client sockets. Clamped internally to what select() can
    // carry (FD_SETSIZE) with room for the listener and the wakeup socket.
    int            maxConnections;

    // TRUE = every inbound "cmd" frame is refused with ack ok:false. This is
    // the read-only dashboard milestone of ARCHITECTURE.md section 6 q3, and
    // it is the DEFAULT.
    bool           readOnly;

    // WebSocket endpoint path. js/transport/ws.js connects to "/ht9045".
    std::string    wsPath;

    int            pingIntervalMs;   // WS ping cadence           (default 15000)
    int            idleTimeoutMs;    // silence before drop        (default 45000)
    int            pollIntervalMs;   // select() timeout           (default 50)

    // AI(W906-FW-W3) 20260819: single-operator control token (design doc
    // section 3, per the 20260819 ruling "同時只允許一個瀏覽器操作").
    // A cmd-capable connection must hold the token (control.acquire) before
    // any non-auth.*, non-control.* command is accepted; the token releases
    // on disconnect and after this idle window with no accepted command.
    // <= 0 disables the idle timeout (disconnect release still applies).
    int            controlIdleTimeoutMs;   // default 600000 (10 min)

    // AI(W906-FW-C1WIRE) 20260911: reject a WebSocket upgrade whose Origin is
    // not this server's own. Default true.
    //
    // Loopback binding keeps other MACHINES out; it does nothing about other
    // PAGES in the same browser. Any site the operator happens to open can run
    // `new WebSocket("ws://127.0.0.1:<port>/ht9045")` and the browser permits
    // it. Origin is the one header a page cannot forge, so it is the only thing
    // that distinguishes our page from someone else's.
    //
    // The rule, and why each branch is what it is:
    //   * Origin ABSENT   -> allowed. Not a browser. tools/webprobe and
    //                        tests/test_wb_server.cpp are raw TCP clients that
    //                        send none, and rejecting them would break the
    //                        harness without closing the hole -- a real browser
    //                        ALWAYS sends Origin on an upgrade.
    //   * Origin OURS     -> allowed. http(s)://127.0.0.1:<port> and
    //                        http(s)://localhost:<port>, compared
    //                        case-insensitively, trailing '/' tolerated.
    //   * anything else   -> 403, including the literal "null" that a file://
    //                        page sends. That IS a browser and it is not our
    //                        page.
    // Set false only if the HMI is ever fronted by something on another origin
    // (a reverse proxy, say). It is a field rather than a constant so that
    // deployment does not require editing this file.
    bool           checkOrigin;

    // A connection whose unsent backlog exceeds this is dropped rather than
    // allowed to stall the single socket thread for every other client.
    size_t         maxSendBacklog;   // default 256 KiB

    WebBridgeConfig();
};

// -----------------------------------------------------------------------------
//  Observable counters. Copy-returned under a lock; safe from any thread.
// -----------------------------------------------------------------------------
struct WebBridgeStats {
    unsigned long long httpRequests;
    unsigned long long wsAccepted;
    unsigned long long wsRejected;       // upgrade attempts we refused
    unsigned long long snapshotsSent;
    unsigned long long patchesSent;
    unsigned long long alarmsSent;
    unsigned long long modalsSent;       // AI(W906-FW-W5a) 20260819
    unsigned long long queriesSent;      // AI(W906-FW-W5b) 20260819
    unsigned long long acksSent;
    unsigned long long cmdAccepted;      // validated and enqueued
    unsigned long long cmdRejected;      // refused, ack ok:false sent
    unsigned long long pingsSent;
    unsigned long long pongsReceived;
    unsigned long long slowClientDrops;
    unsigned long long connectionsAccepted;
    unsigned long long connectionsClosed;
    int                liveConnections;

    WebBridgeStats();
};

// -----------------------------------------------------------------------------
class WebBridgeServer {
public:
    explicit WebBridgeServer(const WebBridgeConfig& cfg);
    ~WebBridgeServer();   // calls Stop()

    // Wire up the two -- and only two -- things that cross the machine
    // boundary. Both may be null (then the bridge serves an empty snapshot and
    // refuses all commands). Call before Start().
    void SetSnapshot(TagSnapshot* snapshot);
    void SetCommandQueue(CommandQueue* queue);

    // Live-switchable; takes effect on the next inbound frame.
    void SetReadOnly(bool readOnly);
    bool IsReadOnly() const;

    // AI(W906-FW-C1WIRE) 20260911: a dynamic HTTP route, for a response the
    // host must COMPUTE rather than read off disk.
    //
    // WHY A CALLBACK AND NOT A METHOD ON THIS CLASS.
    // This library is everything wb_gateway links, and CMakeLists.txt:3031
    // makes its "structurally incapable of touching the machine" property
    // load-bearing -- the comment there says so in as many words. Recipe
    // documents live on the machine side, so the HANDLER cannot be in here.
    // Only the hook can. wb_serve installs one; wb_gateway installs none and
    // keeps the property intact.
    //
    // Contract:
    //   * Install BEFORE Start(). The socket thread reads these three members
    //     without a lock, so changing them on a running server is a data race.
    //   * `path` already has ?query and #fragment stripped. `query` carries
    //     whatever followed '?' verbatim -- the browser's cache-busters ride
    //     there (`?_=<timestamp>`, see the web author's dialog-bridge.js).
    //   * Return true ONLY if *out was filled. Returning false falls through
    //     to the static file handler, so an unmatched prefix behaves exactly
    //     as it did before this hook existed.
    //   * It runs ON THE SOCKET THREAD. Nothing in it may block on machine
    //     I/O -- the same rule CommandQueue.h states for tryPush, and for the
    //     same reason: one stalled handler stalls every other connection.
    //     Reading one small file is the intended cost. Anything that can wait
    //     on the machine belongs on the command queue instead.
    typedef bool (*HttpRouteFn)(void* user, const std::string& method,
                                const std::string& path, const std::string& query,
                                HttpResponse* out);
    //AI(W906-WEB-W2a) 20260917: ⚠ THERE IS ONLY ONE ROUTE. This does not add to
    //   a table -- WebBridgeServer.cpp:1630-1633 ASSIGNS impl_->routePrefix and
    //   impl_->routeFn, so a second call silently REPLACES the first and every
    //   request that matched the old prefix falls through to the static file
    //   handler as a 404, with no error anywhere.
    //   Measured by the web colleague on his own tree (20260916): registering
    //   "/api/system" after "/api/recipe" killed every /api/recipe/* request.
    //   If you need two, register ONE prefix both share and dispatch inside the
    //   handler -- that is what his ApiRoute() does, and it is a precondition for
    //   WEB-W2b. Do not "just add another SetHttpRoute call".
    void SetHttpRoute(const std::string& pathPrefix, HttpRouteFn fn, void* user);

    // Creates and binds the listener on the CALLING thread (so a bind failure
    // is reported synchronously and BoundPort() is valid on return), then
    // spawns the socket thread and returns immediately.
    // Returns false and fills errOut on failure. Calling Start() twice is a
    // no-op returning true.
    bool Start(std::string* errOut = 0);

    // Wakes the socket thread, joins it, and releases every socket and handle.
    // Safe to call twice, safe when Start() was never called, safe from the
    // destructor. Prompt: bounded by one wakeup round trip, not by
    // pollIntervalMs and not by any client's behaviour.
    void Stop();

    bool           IsRunning() const;
    unsigned short BoundPort() const;   // 0 until Start() succeeds

    // --- called by the UI thread -------------------------------------------
    // Report the outcome of a command previously drained from the CommandQueue.
    // `ticket` is the id this server put on the queued command. The browser
    // receives {"type":"ack","id":<its own id>,"ok":...,"error":"..."} on the
    // connection that sent it; if that connection has since gone, the result is
    // dropped silently. Non-blocking.
    void CompleteCommand(unsigned long long ticket, bool ok, const std::string& error);

    // AI(W906-FW-W3) 20260819: the current control-token holder's connection
    // id (0 = nobody). Safe from the UI thread (atomic read); the UI tick
    // stages it into the snapshot as the `control.owner` tag.
    unsigned long long ControlOwner() const;  int LiveWebSocketCount() const;   // AI(W906-MODAL-WAKE) 20260926: browsers connected over WebSocket right now (HTTP does not count; a dead one lingers up to idleTimeoutMs). Atomic read, any thread

    // Broadcast {"type":"alarm","code":...,"text":...,"at":...} to every
    // connected browser. Non-blocking. `at` should be ISO-8601; when empty the
    // server fills in the current local time.
    void PostAlarm(const std::string& code, const std::string& text,
                   const std::string& at = std::string());

    // AI(W906-FW-W5a) 20260819: broadcast {"type":"modal","title":...,
    // "text":...,"at":...} -- the browser face of golden's display-only
    // dialogs (ShowMyMessage returns void, so nothing flows back; an
    // answer-carrying modal is a separate, future surface). Same threading
    // contract as PostAlarm: UI thread, non-blocking.
    void PostModal(const std::string& title, const std::string& text,
                   const std::string& at = std::string());

    // AI(W906-FW-W5b) 20260819: broadcast {"type":"query","qid":...,
    // "code":...,"kcode":...,"options":[...],"at":...} -- the ANSWER-carrying
    // dialog (golden ShowErrorMessage). `kcodeMask` is golden's K_* button
    // mask (K_RETRY=1, K_SKIP=2, K_CLEAN_OUT=4); the frame carries both the
    // raw mask and the decoded option names. The answer comes back as a
    // normal `modal.answer` command through the CommandQueue (token holder
    // only, like every non-auth command); this method itself is one-way and
    // non-blocking -- the CALLER owns the waiting (wb_serve pumps the queue).
    void PostQuery(unsigned long long qid, const std::string& code,
                   int kcodeMask, const std::string& at = std::string());

    // AI(W906-YESNO) 20260925: PostQuery 的兄弟 —— 選項**直接給名字**，不從 K_* 遮罩解。
    //   給 golden ShowMyMessageBox_YES_NO（mymessbox.cpp:1009）用：它的兩顆鍵是
    //   pnlYes/pnlNo（Tag 1/2），不在 note.cpp KeyComp[] 裡，K_* 遮罩表達不出來。
    //   訊框與 PostQuery **同一種**（type:"query"，qid／options／at 同義），只多兩個
    //   說明欄位，所以 ht9045_dialog_host.js 不用改就能記下 qid 並答覆：
    //     {"type":"query","qid":N,"code":"","kcode":0,"kind":"<kind>",
    //      "text":"<S1>","options":["YES","NO"],"at":"..."}
    //   同樣留一份給之後才連上的瀏覽器補發，同樣要在拿到答案後 ClearQuery(qid)。
    //   執行緒契約與 PostQuery 相同：UI（tick）執行緒呼叫、不阻塞。
    void PostQueryOptions(unsigned long long qid, const std::string& kind,
                          const std::string& text,
                          const std::vector<std::string>& options,
                          const std::string& at = std::string());

    // AI(W906-Q30-REPLAY) 20260921: 清掉還沒被回答的那一個 query。
    //
    //   `PostQuery` 送出去的是一次性廣播，只到得了**當下連著的**瀏覽器。
    //   伺服器因此留了一份，**新連線握手完成、送完快照之後會補發**，
    //   否則「警報跳出來時剛好沒開瀏覽器」＝ 永久凍結、只能重啟行程
    //   （Q30 第 3 題，20260921 量到）。
    //
    //   ⚠⚠ **呼叫端有義務在拿到答案（或放棄等待）之後呼叫這一支。**
    //   不呼叫的話，下一個連上來的瀏覽器會收到一個早就被回答過的警報框，
    //   而它送回來的 `modal.answer` 會被判成 `no query pending` ——
    //   操作員看到的是一個**關不掉**的框。
    //
    //   以 `qid` 比對，所以清錯一個（例如清到已經換新的那一個）不會發生。
    //   可以安全地重複呼叫。
    void ClearQuery(unsigned long long qid);

    // Nudge the socket thread to re-check the snapshot now instead of at the
    // next poll tick. Cheap; safe to call from the UI timer after Publish().
    void Wake();

    WebBridgeStats Stats() const;

private:
    WebBridgeServer(const WebBridgeServer&);
    WebBridgeServer& operator=(const WebBridgeServer&);

    class Impl;                      // all winsock lives in the .cpp
    std::unique_ptr<Impl> impl_;
};

}  // namespace webbridge

#endif  // WEBBRIDGE_WEBBRIDGESERVER_H
