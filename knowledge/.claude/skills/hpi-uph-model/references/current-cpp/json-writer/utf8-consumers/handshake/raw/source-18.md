# 原文 18：ParseHttpRequest

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `ParseHttpRequest` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `747ec18fbe24d7f7ae48bc2e8b7ca4e9def6eb411b80ea75e43c54db2778a11a`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  ParseHttpRequest
// ---------------------------------------------------------------------------
HttpParseResult ParseHttpRequest(const char* data, std::size_t len,
                                 HttpRequest& out, std::size_t* consumed,
                                 std::size_t maxHeaderBytes) {
    out.Clear();
    if (consumed) *consumed = 0;
    if (data == 0) return kHttpNeedMore;

    // Locate the end of the header block. std::string::find on a std::string
    // built from data would copy; scan the raw bytes instead so an over-long
    // hostile stream costs no allocation at all.
    std::size_t end = std::string::npos;   // index of the first byte of CRLFCRLF
    if (len >= 4) {
        for (std::size_t i = 0; i + 3 < len; ++i) {
            if (data[i] == '\r' && data[i + 1] == '\n' &&
                data[i + 2] == '\r' && data[i + 3] == '\n') {
                end = i;
                break;
            }
        }
    }
    if (end == std::string::npos) {
        // No terminator yet. THE CAP IS CHECKED HERE, before any parsing or
        // copying, which is the whole point of doing the scan first.
        if (len >= maxHeaderBytes) return kHttpTooLarge;
        return kHttpNeedMore;
    }
    std::size_t blockLen = end + 4;
    if (blockLen > maxHeaderBytes) return kHttpTooLarge;

    std::string block(data, end);          // header lines, no trailing CRLFCRLF
    if (consumed) *consumed = blockLen;

    // --- split into CRLF-delimited lines, unfolding obs-fold as we go -------
    std::vector<std::string> lines;
    std::size_t pos = 0;
    while (pos <= block.size()) {
        std::size_t crlf = block.find("\r\n", pos);
        std::string line = (crlf == std::string::npos) ? block.substr(pos)
                                                      : block.substr(pos, crlf - pos);
        // A bare CR or LF inside a line is malformed (and a smuggling vector).
        if (line.find('\r') != std::string::npos ||
            line.find('\n') != std::string::npos) {
            return kHttpBad;
        }
        if (!line.empty() && IsOws(line[0]) && !lines.empty()) {
            // obs-fold: continuation of the previous line.
            lines.back() += " ";
            lines.back() += TrimOws(line);
        } else {
            lines.push_back(line);
        }
        if (crlf == std::string::npos) break;
        pos = crlf + 2;
    }
    if (lines.empty()) return kHttpBad;
    // A folded FIRST line would mean the request line itself began with
    // whitespace, which is malformed.
    if (!lines[0].empty() && IsOws(lines[0][0])) return kHttpBad;

    // --- request line: METHOD SP TARGET SP VERSION -------------------------
    const std::string& rl = lines[0];
    std::size_t sp1 = rl.find(' ');
    if (sp1 == std::string::npos) return kHttpBad;
    std::size_t sp2 = rl.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return kHttpBad;
    out.method = rl.substr(0, sp1);
    out.target = rl.substr(sp1 + 1, sp2 - sp1 - 1);
    out.version = rl.substr(sp2 + 1);
    if (out.method.empty() || out.target.empty()) return kHttpBad;
    if (out.version.find(' ') != std::string::npos) return kHttpBad;
    if (out.version.compare(0, 5, "HTTP/") != 0) return kHttpBad;

    // --- split target into path + query ------------------------------------
    std::size_t q = out.target.find('?');
    if (q == std::string::npos) {
        out.rawPath = out.target;
        out.query.clear();
    } else {
        out.rawPath = out.target.substr(0, q);
        out.query = out.target.substr(q + 1);
    }
    // Strip a "#fragment" if some client sent one (it should not).
    std::size_t hash = out.query.find('#');
    if (hash != std::string::npos) out.query.erase(hash);
    hash = out.rawPath.find('#');
    if (hash != std::string::npos) out.rawPath.erase(hash);
    out.path = UrlDecode(out.rawPath, false);   // '+' is literal in a path

    // --- header lines ------------------------------------------------------
    for (std::size_t i = 1; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        if (line.empty()) continue;              // tolerate a stray blank line
        std::size_t colon = line.find(':');
        if (colon == std::string::npos || colon == 0) return kHttpBad;
        std::string name = line.substr(0, colon);
        // RFC 7230 section 3.2: field-name = token, and SP/HTAB are not tchar.
        // Checking only the character before the colon (which is all this did
        // before) accepts "Bad Name: v", because its last character is 'e'.
        // The whole name has to be a token -- section 3.2.4 makes rejecting
        // malformed field-names mandatory, and this endpoint is one that can
        // eventually command machine motion, so it parses strictly.
        if (!IsToken(name)) return kHttpBad;
        out.headers.push_back(
            std::make_pair(ToLowerAscii(name), TrimOws(line.substr(colon + 1))));
    }
    return kHttpOk;
}


<!-- preserved-content:end -->
```
