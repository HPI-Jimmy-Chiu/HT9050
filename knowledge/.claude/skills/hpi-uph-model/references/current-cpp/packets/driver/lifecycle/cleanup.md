# 本地停止與driver解除順序

定位 `GpibEngine::DoClose`／`Stop`／`Teardown`／dtor，完整文字／hash見 [manifest](source-manifest.json)。

| 定位 | 選讀來源順序 |
| --- | --- |
| DoClose | closed_為true或SerialPoll為0就返回；否則closed_=true、up_=false，FormClose，發布關閉快照 |
| Stop | started_為false就返回；未closed時先DoClose，然後Teardown |
| dtor | started_為true才Teardown；原註解的thread已停止前提未在本單元驗證 |
| Teardown表單 | up_=false；delete fRS232Main／fDummyART、清各指標與bridge token，delete SerialPoll，再清SerialPoll及其他Handler狀態 |
| Teardown driver | SetGpibDriver(0)在delete ownedDriver_之前；再ownedDriver_=0、mailbox_=0、started_=false、g_live.store(0) |

Teardown選讀body沒有delete注入指標或清g_injected，也沒有把closed_寫回false；下一個Start有其自己的closed_初值路徑。這個語句順序不證明所有跨thread／重入／driver壽命安全。

DoClose的FormClose及Teardown的表單解構／Publish內部在本單元未查；Stop body沒有包catch，不能把「寫在下一行」視為前面丟例外時仍一定執行。原註解的正常路徑保留，完整例外與Hub停止時序沿 [engine查證](../../../consumers/command/transport/mailbox/engine/lifecycle.md) 續查。

回 [索引](index.md)、[選擇](selection.md)、[界線](limits.md)。
