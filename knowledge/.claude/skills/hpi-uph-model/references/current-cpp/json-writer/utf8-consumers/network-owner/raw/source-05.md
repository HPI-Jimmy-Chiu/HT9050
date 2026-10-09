# 原文 05：WebBridgeServer::Impl::DoWebSocketUpgrade

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `WebBridgeServer::Impl::DoWebSocketUpgrade` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `78bdc7ec73594fa021c873806369858b70ef2b3c0164b7aee55b9f41d93f3229`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
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


<!-- preserved-content:end -->
```
