# OnHandlerMessage 的本地適配

兩個完整函式與舊註解保存在[manifest](source-manifest.json)，本頁只核對所選語句。

| 對照 | GPIB | RS232 |
| --- | --- | --- |
| 早退 gate | SerialPoll 為空、closed_、SerialPoll->closeRequested | fRS232Main 為空、closed_、fRS232Main->closeRequested |
| 接收呼叫 | SerialPoll->OnMyCopyMsg(msg) | fRS232Main->OnMyCopyMsg(msg) |
| 本地正常結果 | 早退及正常呼叫後均回 0 | 早退及正常呼叫後均回 0 |

通過早退 gate 才增加 `g_handlerMsgs`。此計數位於接收方法呼叫之前，不是測試機確認數。兩版取 `n=max(payload.size(),sizeof(MV))`，配置零填充 `vector<unsigned long long>`並複製 payload；配置數為 `n/sizeof(unsigned long long)+1`。`COPYDATASTRUCT`的 cbData 仍是原 payload.size()，lpData 指向該 buffer，再把結構地址放入 `TMessage::LParam`交給本地 OnMyCopyMsg。

這裡是函式直接呼叫；不能由結構名字推成一次已驗證的跨程序 WM_COPYDATA。MV 欄位布局／截斷、DWORD 尺寸轉換、所有接收端前置驗證及持有契約仍待查。

呼叫前保存 `GHandler2Gpib`到 outer 並增加 depth；正常／catch 兩路都減 depth，depth>0 時還原 outer，最外層設空；catch 再 throw 到上層。這只描述所選函式的恢復語句，沒有重驗舊註解「所有外部 readers 已查／NULL 安全」或完整重入／並行契約。

兩個方法的 0 不能區分早退或接收方法正常返回，更不表示 external ack；所選[上游](../lifecycle.md)catch-all 會將丟出的例外計為 handlerErrors_並回 -1。接收方法後續分派以[GPIB 局部](gpib/index.md)續查，RS232完整分派仍待補。

回[界線](limits.md)與[入口](index.md)。
