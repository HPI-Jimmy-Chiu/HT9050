# Engine 接收與啟停局部流程

pin `6069004dc2ca0906b5089316ac569f993ae18aab`；[來源清冊](source-manifest.json)保存三個 V906 source、七完整函式文字／hash及 `Running` 宣告。接續[mailbox](../index.md)，同一 UPH Skill 整合 HT9050 與其他 Handler 的共用路徑和版本差異。

- [Hub 與 thread](lifecycle.md)：`EngineSideHandler`、`SelectTestType`、`IsUp`、`StopEngine`、`Start`、`Stop`、`Loop`。
- [GPIB／RS232 payload 適配](payload/index.md)：兩個 `OnHandlerMessage` 本地返回與巢狀指標保存。
- [機型與未查界線](limits.md)：完整生命週期、作用中介面、計數／保存與送達仍待查。

活正文以 function／變數定位；manifest 內舊設計註解與 golden 行號保存為歷史，未重新驗證其安全或相容聲明。沒有建置或執行程式、機台與 runtime。
