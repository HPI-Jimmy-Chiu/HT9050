# 原文：WebBridgeServer::Impl::Start

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `48f7f8a8ad867b737743e1d8197b26fa570ae8d9c4a0ddb6bf6230252d409226`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
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


<!-- preserved-content:end -->
```
