# 原文 28：entire header original contract, declarations and metadata

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.h`；以 `entire header original contract, declarations and metadata` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `d746272ddcefc07b3e66d8cf6604cba6f3fd18eedbc7a2c961b4b4785f29e930`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  WebBridge/WsHandshake.h
//
//  HTTP/1.1 request parsing + the RFC 6455 opening handshake (server side).
//
//  SCOPE / LAYERING
//  ----------------
//  This file is PURE BYTES IN -> BYTES OUT. It opens no sockets, touches no
//  machine state, and includes neither <winsock2.h> nor <vcl.h>. That is
//  deliberate: it keeps the protocol layer unit-testable off-machine, and it
//  keeps the ARCHITECTURE.md section 5 threading contract easy to honour --
//  nothing in here can block on machine I/O because nothing in here can reach
//  the machine.
//
//  The same server socket serves BOTH static files out of D:\HT9045\web\ AND
//  the /ht9045 WebSocket endpoint (ARCHITECTURE.md section 7 steps 1 and 4),
//  so the parse result exposes the request path and query string separately:
//  the path selects file-vs-endpoint, and the query selects the data source.
//
//  C++ DIALECT
//  -----------
//  C++14-compatible on purpose. The tree sets CMAKE_CXX_STANDARD 17, but the
//  configured MinGW compiler is GCC 6.3.0, which accepts -std=c++17 while
//  implementing only part of C++17 (verified: structured bindings are a hard
//  error). So: no structured bindings, no std::optional, no std::string_view,
//  no if-init statements. Builds under MSVC cl as well.
// ===========================================================================
#ifndef WEBBRIDGE_WSHANDSHAKE_H
#define WEBBRIDGE_WSHANDSHAKE_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace webbridge {

// ---------------------------------------------------------------------------
//  Adapter over the two crypto primitives this file needs.
//
//  These resolve to webbridge::Sha1Raw (WebBridge/Sha1.h) and
//  webbridge::Base64Encode (WebBridge/Base64.h). The indirection exists only so
//  that a rename on that side can be absorbed at the compile line instead of by
//  editing this file:
//      -DWB_SHA1_RAW=mynamespace::sha1_raw
//      -DWB_BASE64_ENCODE=mynamespace::b64_encode
//  WB_SHA1_RAW must return the 20 RAW digest bytes (not hex) in a std::string.
// ---------------------------------------------------------------------------
#ifndef WB_SHA1_RAW
#  define WB_SHA1_RAW webbridge::Sha1Raw
#endif
#ifndef WB_BASE64_ENCODE
#  define WB_BASE64_ENCODE webbridge::Base64Encode
#endif

// ---------------------------------------------------------------------------
//  Default cap on the size of the request header block.
//
//  WHY A CAP: a client that opens a connection and streams header bytes
//  forever without ever sending CRLFCRLF would otherwise make the server
//  accumulate without bound -- one socket, unlimited memory. 16 KiB is well
//  above any real browser request (typical is under 1 KiB) and matches the
//  default limits of common servers.
// ---------------------------------------------------------------------------
const std::size_t kDefaultMaxHeaderBytes = 16u * 1024u;

// The RFC 6455 section 1.3 handshake GUID.
extern const char* const kWebSocketGuid;   // "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// ---------------------------------------------------------------------------
//  A parsed HTTP request-header block.
// ---------------------------------------------------------------------------
struct HttpRequest {
    std::string method;      // upper-case as received, e.g. "GET"
    std::string target;      // raw request-target as received, e.g. "/ht9045?src=live"
    std::string version;     // e.g. "HTTP/1.1"
    std::string rawPath;     // target up to '?', NOT percent-decoded
    std::string path;        // rawPath, percent-decoded (see PathIsSuspicious below)
    std::string query;       // after the first '?', raw (no leading '?'); "" if absent

    // Header names are lower-cased; values have leading/trailing OWS trimmed
    // and obs-fold continuation lines unfolded to a single space. Order of
    // first appearance is preserved.
    std::vector<std::pair<std::string, std::string> > headers;

    // Case-insensitive lookup. Repeated headers are joined with ", " per
    // RFC 7230 section 3.2.2 -- this matters for real clients that split
    // "Connection: keep-alive, Upgrade" across two header lines.
    std::string Header(const std::string& name) const;
    bool HasHeader(const std::string& name) const;

    // Value of `name` from the query string, percent-decoded, with '+' read as
    // space (application/x-www-form-urlencoded convention). Returns `def` if
    // the key is absent. Present-but-empty ("?src=") yields "".
    std::string QueryParam(const std::string& name,
                           const std::string& def = std::string()) const;
    bool HasQueryParam(const std::string& name) const;

    void Clear();
};

// ---------------------------------------------------------------------------
//  Result of trying to parse a header block out of an accumulated buffer.
// ---------------------------------------------------------------------------
enum HttpParseResult {
    kHttpOk = 0,        // a complete header block was parsed
    kHttpNeedMore,      // no CRLFCRLF yet and still under the cap -- read more
    kHttpBad,           // malformed request line / header line
    kHttpTooLarge       // cap exceeded before CRLFCRLF -- respond 431 and close
};

// Parse a request header block from the front of [data, data+len).
//
// INCREMENTAL USE: the caller appends received bytes to its own buffer and
// calls this after each read. kHttpNeedMore means "no terminator yet, keep
// reading". On kHttpOk, *consumed (if non-NULL) receives the number of bytes
// the header block occupied INCLUDING the terminating CRLFCRLF, so the caller
// can erase them and treat whatever follows as body / first WebSocket bytes.
//
// Only CRLF line endings are accepted (RFC 7230 section 3.5 permits a server
// to be lenient about bare LF; this parser is not, because every client that
// matters sends CRLF and strictness removes a class of smuggling ambiguity).
HttpParseResult ParseHttpRequest(const char* data,
                                 std::size_t len,
                                 HttpRequest& out,
                                 std::size_t* consumed = 0,
                                 std::size_t maxHeaderBytes = kDefaultMaxHeaderBytes);

// ---------------------------------------------------------------------------
//  Handshake validation
// ---------------------------------------------------------------------------
enum HandshakeResult {
    kHsOk = 0,             // valid RFC 6455 upgrade -- reply BuildHandshakeResponse
    kHsNotWebSocket,       // a well-formed HTTP GET that is NOT an upgrade;
                           //   the caller should serve it as a static file
    kHsBadRequest,         // an upgrade attempt that is malformed -> 400
    kHsUpgradeRequired,    // Sec-WebSocket-Version present but not 13 -> 426
    kHsMethodNotAllowed    // Upgrade: websocket on a non-GET method -> 405
};

// Classify a parsed request. Checks, per RFC 6455 section 4.1/4.2.1:
//   * method is GET and version is HTTP/1.1
//   * Upgrade header contains the token "websocket" (case-insensitive)
//   * Connection header contains the token "upgrade" (case-insensitive), which
//     may arrive inside a comma list such as "keep-alive, Upgrade"
//   * Sec-WebSocket-Version is exactly 13
//   * Sec-WebSocket-Key is present and is a syntactically valid base64
//     encoding of 16 bytes (24 chars ending "==")
// `whyNot`, if non-NULL, receives a short human-readable reason for logging on
// any non-kHsOk result.
HandshakeResult CheckWebSocketUpgrade(const HttpRequest& req, std::string* whyNot = 0);

// True if the request carries "Upgrade: websocket" at all. Lets the caller
// distinguish "a browser asking for /style.css" (serve the file) from "a
// broken WebSocket client" (return the HTTP error) before it decides how to
// respond to a non-kHsOk classification.
bool LooksLikeWebSocketUpgrade(const HttpRequest& req);

// True if the Sec-WebSocket-Key is syntactically what RFC 6455 section 4.1
// requires: the base64 encoding of a 16-byte nonce.
bool IsValidWebSocketKey(const std::string& key);

// ---------------------------------------------------------------------------
//  Response construction
// ---------------------------------------------------------------------------
// base64(SHA1(key + kWebSocketGuid)) -- RFC 6455 section 4.2.2 step 5.
// The section 1.3 vector this MUST satisfy (asserted in tests/test_wb_wsproto.cpp):
//   "dGhlIHNhbXBsZSBub25jZQ==" -> "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
std::string ComputeAcceptKey(const std::string& secWebSocketKey);

// The full 101 Switching Protocols response, terminated with CRLFCRLF.
// `subprotocol`, if non-empty, is echoed in Sec-WebSocket-Protocol.
// NOTE: no extensions are negotiated, so the frame layer can insist that the
// RSV bits are zero.
std::string BuildHandshakeResponse(const std::string& secWebSocketKey,
                                   const std::string& subprotocol = std::string());

// A minimal HTTP error response (with Connection: close and Content-Length: 0)
// matching a non-kHsOk classification, or the 431 for kHttpTooLarge via
// BuildTooLargeResponse.
std::string BuildHandshakeErrorResponse(HandshakeResult why);
std::string BuildTooLargeResponse();

// ---------------------------------------------------------------------------
//  Helpers the static-file half of the server needs
// ---------------------------------------------------------------------------
std::string UrlDecode(const std::string& s, bool plusAsSpace = false);
std::string ToLowerAscii(const std::string& s);

// True if a decoded path must NOT be joined onto a filesystem root: it is
// empty, does not start with '/', contains a NUL or a backslash, or contains a
// ".." segment. This is a necessary check, not a sufficient one -- the file
// server still owns canonicalising and confining the result.
bool PathIsSuspicious(const std::string& decodedPath);

}  // namespace webbridge

#endif  // WEBBRIDGE_WSHANDSHAKE_H

<!-- preserved-content:end -->
```
