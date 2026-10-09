# 原文 08：entire WebBridgeServer header original configuration/API/metadata

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.h`；以 `entire WebBridgeServer header original configuration/API/metadata` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `c62e52739421fabc59bf160dedc4654bab24e2f522186723600f7bf7507c95ba`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
