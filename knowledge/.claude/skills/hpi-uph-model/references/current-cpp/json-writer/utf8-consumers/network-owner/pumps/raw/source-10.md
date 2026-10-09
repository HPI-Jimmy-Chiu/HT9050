# 原文：generation, outgoing and pending fields, including original Q30 replay rationale（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `a6703bcf07647c9e272bfc399831ea212e73d834aa71c44a9fb269a358d857b3`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
