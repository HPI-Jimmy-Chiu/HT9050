# 原文 11：Conn original per-connection adapter context

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `Conn original per-connection adapter context` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `9884ed1e796910d2cf2170806fdc65a2af60904b589592fcb8f2b7d040282268`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
