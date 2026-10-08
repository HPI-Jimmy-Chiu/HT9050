# Hub 選擇、停止與重啟

定位 TesterCommHub.cpp 的六個選讀 body；factory／NI／Sim 沿用 [既有查證](../factory.md)。

| function／分支 | 選讀行為 |
| --- | --- |
| SelectTestType／同 type | 返回 engine!=0 且 thread.Running；不重啟，也不查 engine.IsUp |
| 不同 type | StopEngine；先存 requested type，再檢查範圍／factory，無效 false 仍保留 requested type |
| factory／thread Start | 建 engine，0 就 false；安裝 engine-side handler，thread.Start false 就 StopEngine／false，否則 true |
| StopEngine | 移除 mailbox handler→thread.Stop→mailbox.Reset→delete engine→engine=0 |
| Restart | type<0 false；保存 t、StopEngine、type=-1，再 SelectTestType(t)，該入口也會 StopEngine |
| IsUp | engine 非 0、thread.Running、engine.IsUp 三者成立 |
| Shutdown | StopEngine 後 type=-1 |
| EngineSideHandler | engine=0 返回 -1；OnHandlerMessage 的例外加 handlerErrors 並 -1 |

SelectTestType 的 true 是 thread.Start 的結果；engine.Start 在 [Loop](thread.md) 後續執行，尚不能稱外部 Tester 已連線。同 type 的 SelectTestType 與 IsUp 條件不同；[StartBridgeProgram](../init-selection/selection.md) 才在同 type 且 !IsUp 時選 Restart。

StopEngine 是來源次序，完整 mailbox Reset／SetHandler、WbThread join 與各 engine 析構 callee 尚未由此層閉合。factory 執行不在這個 body 的 catch 內；EngineSideHandler 局部 catch 不是所有 Start／Stop 的總保護。

回 [索引](index.md)、[thread](thread.md)、[界線](limits.md)。
