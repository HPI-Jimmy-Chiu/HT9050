# thread 非同步啟動與停止

定位 TesterCommThread.cpp 的 Start／Stop／Entry／Loop，及 header 快照。Running／StartFailed／Heartbeat／EngineErrors 各讀 atomic；thread 不擁有 engine，Hub 負責刪除。

| 階段 | 選讀行為 |
| --- | --- |
| Start guard | thread 已 joinable，或 engine／mailbox 為 0，就 false |
| Start 通過 | 保存指標，stop=false、startFailed=false、running=true；WbThread.start 失敗清 running／false，成功 true |
| Entry | 轉 self 為 TesterCommThread* 後 Loop |
| Loop 啟動 | try engine.Start；例外加 errors；!started 時 startFailed=true、running=false 並直接 return |
| Loop 輪詢 | 每輪 heartbeat 加 1、waitMs 先 10；try Poll 再 RunOnce 取 wait，例外加 errors 並改 10；檢查 stop 後等待 wake |
| 正常停止尾段 | 同一 try 先 Poll 再 engine.Stop；例外加 errors；最後 running=false |
| Stop | 非 joinable 直接返回；其餘 stop=true，有 mailbox 就 SetEvent(engine side)，join 後 running=false |

Start 不等待 engine.Start 成功；Loop 的 !started 出口不經尾段 engine.Stop。尾段若 Poll 丟例外，同一 try 後面的 engine.Stop 也不執行。各 engine 析構及 WbThread 失敗還原需另查，不能寫成所有出口一定 Stop。

join 沒有 timeout 或 caller-thread guard；10 ms 預設 wait 不是停止延遲上限，callback／RunOnce／Stop 仍可能耗時。WaitForSingleObject 回傳值未在此 body 分流；WbThread／事件 API 與完整併發契約未閉合。

heartbeat 是迭代次數，errors 是捕獲例外數，均不是顆數、Tray 產出或 UPH。header「不影響機台執行緒」是設計註解，本層沒有排程／實機效能驗證。

回 [索引](index.md)、[Hub](hub.md)、[界線](limits.md)。
