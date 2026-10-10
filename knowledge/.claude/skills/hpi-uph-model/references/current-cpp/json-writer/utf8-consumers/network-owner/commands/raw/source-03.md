# 原文：WebBridgeServer::Impl::PostQuery（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `220ca2dad41178fc9ad21129b3ac965bdcde1b34d1c5e8e8f3f0ef58af662771`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// AI(W906-FW-W5b) 20260819: the answer-carrying sibling. One-way broadcast;
// the answer travels back as a normal queued command (see header).
void WebBridgeServer::Impl::PostQuery(unsigned long long qid, const std::string& code,
                                      int kcodeMask, const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"query\",\"qid\":" << qid
       << ",\"code\":" << sib::QuoteString(code)
       << ",\"kcode\":" << kcodeMask
       << ",\"options\":[";
    // AI(W906-Q30-KMAP) 20260921: golden 的 K_* 按鈕遮罩，**補滿 9 個**。
    //
    //   ⚠ 這裡原本只認 3 個（0x1 RETRY / 0x2 SKIP / 0x4 CLEAN_OUT）。
    //     後果不是「少幾顆鈕」而是**死結**：一個 `KCode` 只提供未對映按鈕的警報
    //    （例如 `K_RESET|K_HOME`）會讓 `options` 變成**空陣列**，
    //     瀏覽器就算做好了 dialog 也沒有按鈕可按，而
    //     `tools/wb_serve.cpp` 的 `ForwardShowErrorMessage` 會驗 `k & kcode`，
    //     所以硬送一個沒提供的也會被拒 —— `for(;;)` 永遠出不來。
    //
    //   清單與**順序**照 golden `note.cpp:1234-1235` 的那一對陣列：
    //     TBtnPanel *Ptr[]   = {BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd,
    //                           BtnCleanOut, BtnReset, BtnHome, BtnTrain, BtnOneCycle};
    //     int KeyComp[]      = {K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END,
    //                           K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN, K_ONECYCLE};
    //   （note.cpp 的 :1235 / :3529 / :3836 三處完全一致）
    //
    //   ⚠ **順序是照 golden 的顯示順序，不是照位元值**。理由不是美觀：
    //     操作員在 BCB6 機台上按的是固定位置的那一顆，換了順序就會按錯。
    //     ⇒ 這也是為什麼 RETRY 與 SKIP 的先後**跟以前不一樣** —— 以前是照位元
    //        值排的，那個順序在 golden 上不存在。
    //
    //   ⚠ **`K_FIX`（0x100）刻意不在列** —— golden 自己的註解寫
    //     「kevin 20130722 cancel K_FIX」。`K_PAUSE`（0x400）與 `K_START`（0x800）
    //     也不在 `KeyComp[]` 裡。⇒ 不要「順手補齊 12 個」，那會加出 golden 沒有的鈕。
    //
    //   位元值出處：`cmydef.cpp:337-347`（本樹）／golden `cmydef.h:263-274`。
    //   這裡照既有慣例硬寫數值而不 include cmydef.h —— 這一層不該相依機台標頭。
    static const struct { int bit; const char* name; } kButtons[] = {
        { 0x0002, "SKIP"      },
        { 0x0001, "RETRY"     },
        { 0x0008, "TRAY_FEED" },
        { 0x0010, "TRAY_END"  },
        { 0x0004, "CLEAN_OUT" },
        { 0x0020, "RESET"     },
        { 0x0040, "HOME"      },
        { 0x0080, "TRAIN"     },
        { 0x0200, "ONECYCLE"  },
    };
    bool first = true;
    for (std::size_t bi = 0; bi < sizeof(kButtons) / sizeof(kButtons[0]); ++bi) {
        if ((kcodeMask & kButtons[bi].bit) == 0) continue;
        os << (first ? "" : ",") << '"' << kButtons[bi].name << '"';
        first = false;
    }
    os << "],\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
        // AI(W906-Q30-REPLAY) 20260921: 同一份留著，給**之後才連上**的瀏覽器補發。
        //   廣播只送給當下連著的人；警報時剛好沒人連著的話，
        //   沒有這一行就再也沒有第二次機會（見上面欄位宣告處的說明）。
        pendingQueryFrame_ = o.frame;
        pendingQueryQid_   = qid;
    }
    {
        WbGuard sl(statsMx);
        ++stats.queriesSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "query", os.str());   //AI(W906-OPLOG) 20260929
}


<!-- preserved-content:end -->
```
