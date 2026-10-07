# VM／MV 欄位與全域符號

來源與原文見 [manifest](source-manifest.json)。此頁只整理選定的宣告，沒有證明二進位佈局、指標有效性或欄位 writer。

## 共用 VM 宣告

V906／V912 的 VM 宣告經各自編碼解碼後文字一致，九個欄位的型別、順序與陣列長度也一致。

| 欄位 | 宣告 |
| --- | --- |
| iCommand | `unsigned int` |
| Result | `unsigned int[32]` |
| bError、bEchoStop | `bool` |
| cReturn | `char[256]` |
| GpibStatus、GpibData | `char[32]`、`char[256]` |
| GPIBBin | `int` |
| bOneCycle | `bool` |

`Result[32]` 是宣告容量；不能直接當作這台機的啟用 site 數或計數口徑。GPIBBin 註解含歷史用途，當前讀寫者另查。

## 共用 MV 宣告與 V912 差異

V906 的十九個欄位按宣告順序如下；V912 保留相同前綴並加尾端欄位。

| 順序區段 | 宣告 |
| --- | --- |
| iSendCommand、Site | `unsigned int`、`unsigned int[32]` |
| bSimulate、bSupport32Bin、bCloseGpib、bTimeOutProcess | `bool` |
| GpibAddress | `int` |
| MachineISRun、IsTest、bGpibMode | `bool` |
| iLotStatus | `int` |
| HandlerHwnd、GpibHwnd | `HWND` |
| GPIBBin、iStatus | `int`、`int[17]` |
| Message | `char[2048]` |
| UseSiteMapData、asATC_TYPE、MultiMessage | `char[256]`、`char[32]`、`char[4096]` |
| V912 追加 sMulti2DIDStringSeparator | `char[10]`；V906 此宣告沒有此欄位 |

兩份 MV 中的 `// #ifdef AMD_Version`／`// #endif` 都是註解；不能把這段當作有效的客戶編譯閘門。`Message`／`MultiMessage` 的容量不同，不代表已查清實際使用哪個欄位、截斷或字串終止策略。

## 四個全域符號

兩版本 header 的 extern 與 cpp 的對應宣告一致：

| 符號 | 型態 |
| --- | --- |
| HGpib2Handler | `VM*` |
| HHandler2Gpib | `MV` 物件 |
| GGpib2Handler | `VM` 物件 |
| GHandler2Gpib | `MV*` |

來源將前兩者註作 Handler、後兩者註作 GPIB。指標的設置、解除、同步與真正傳送另查；本頁沒有由符號名推定生命週期或送達。

回 [封包索引](index.md)，續讀 [命令宣告](commands.md) 與 [界線](limits.md)。
