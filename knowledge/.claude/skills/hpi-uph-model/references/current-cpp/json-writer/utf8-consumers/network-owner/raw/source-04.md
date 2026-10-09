# 原文 04：WebBridgeServer::Impl::ProcessHttpHead

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `WebBridgeServer::Impl::ProcessHttpHead` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `3d532cf416d7003b566b7881f158703412c2caae95f5782ef42a03da8d38e317`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
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


<!-- preserved-content:end -->
```
