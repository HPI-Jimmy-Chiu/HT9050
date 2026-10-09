# 目前 C++ 的 UPH 局部查證

來源 pin8f213da4f；五份source blob、六個指定body、四個RUN_INFO欄位宣告與Profiler符號盤點見 [manifest](source-manifest.json)。本層是20261007靜態證據，與 [HTML／歷史來源](../resources.md) 分開保存。

| 問題 | 路由 | 已查／仍待查 |
| --- | --- | --- |
| 公式、重置與平均 | [計算本體](calculate.md) | 兩版CalculateUPH；全部caller／pause writer未閉合 |
| 計數單位、site、時間 | [計數與時間](count-time.md) | 兩版AddLoadingCount、V906兩個date helper與RUN_INFO欄位；所有writer／caller待查 |
| 版本／客戶輸出差異 | [輸出分流](outputs.md) | 指定body的guard／呼叫；DB／檔案／SECS／UI結果未查 |
| CSV writer／共用寫檔／日期路徑 | [writer 子樹](writers/index.md) | 五writer／四common overload／兩V912 FileInfo body；實際保存仍未驗 |
| Profiler舊設計是否已實作 | [限定符號盤點](profiler.md) | 六個符號在指定追蹤檔零命中；不是全功能不存在或安全證明 |
| 機型／版本與剩餘 | [機型與界線](versions.md) | 同題分HT9050／其他Handler；容量／site／校正待查 |
| Kernel暫停開始與刷新／排程 | [kernel子樹](state/kernel/index.md) | 兩排程body與四ShowRunLabel區段；全UI／caller／實際tick仍待查 |
| 配置InArm計數call／字面閘門 | [caller子樹](callers/index.md) | 七call區段／一短stub；完整caller／機型分派／每顆口徑仍待查 |
| 全域初值、暫停累加與部分重置 | [狀態子樹](state/index.md) | 24定義／extern、22已讀區段；六完整body語意與所有writer未閉合 |
| Loader記錄旗標／Task條件 | [Loader子樹](state/loader/index.md) | 兩版短入口／case 1300；全部派工、helper與取樣鏈仍待查 |
| WebBridge表格到UPH tag／sentinel | [consumer子樹](consumers/index.md) | 三body文字／兩常數／一caller區段；完整transport／producer仍待查 |

沒有執行C++、Profiler、Home、檔案寫入、API、build、runtime或機台；實際UPH誤差仍待查。

[封包宣告子樹](packets/index.md)補四份 MessageDef 的 VM／MV、MSG_CMD_UPH 與 V906／V912 版本對照；ABI、driver、機型／客戶部署仍待查。

## JSON編碼與數值底層

[八函式primitive子樹](json-writer/primitives/index.md)以function／變數定位UTF-8／CP950、JsonNumber及locale界線。
來源 `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；歷史註解與assert不代表本輪實機測試。
