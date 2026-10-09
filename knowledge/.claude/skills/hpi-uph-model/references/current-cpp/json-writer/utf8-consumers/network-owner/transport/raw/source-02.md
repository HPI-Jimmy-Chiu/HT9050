# 原文：WebBridgeServer::Impl::ThreadMain

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `b22cdff0213f62821c6eb3673989886e091c71ee9f5cbdffeb9e10accbef554a`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
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


<!-- preserved-content:end -->
```
