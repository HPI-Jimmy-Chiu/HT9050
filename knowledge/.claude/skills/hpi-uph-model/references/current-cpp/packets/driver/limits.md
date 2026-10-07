# 共用／版本差異與仍待查

| 範圍 | 本單元已查 | 尚未查 |
| --- | --- | --- |
| V906 C++ | IGpibDriver 狀態介面、g_driver、五個 wrapper／helper cpp body | 所有 caller、NI／Sim 實作、constructor 選擇／解除、錯誤／送達與並行 |
| V912 BCB6 | 本層沒有讀 V912 的 driver 對應實作；其 VM／MV／UPH 宣告已有 [同題對照](../versions.md) | 不由 V906 swappable driver 名稱推定 V912 使用相同路徑 |
| HT9050 | 選讀 body 沒有 Type_HT9050 分支 | 實際部署、driver 選擇、機型／客戶啟用、site／容量與實機 UPH |
| HT9045／其他 Handler／客戶 | 共用符號與版本界線保留同一 Skill | 不把 source 註解的 golden 行為或無卡路徑套成完整機型／客戶矩陣 |

本次確認的是 wrapper 的來源文字、返回與狀態欄位來源；不是 NI DLL 的實際行為、Sim 測試結果、指標壽命、thread safety 或傳送成功證明。封包 ABI、完整 producer／consumer、計數與現場校正沿 [封包界線](../limits.md) 續查。

沒有執行 driver、build、C++、API、機台或 runtime。未改來源、執行期設定、快照及他人 Skill；上一批 !321 的30檔已確認進 main `77887765d`，pull後重驗；本層的五函式查證界線不因整合而擴大。回 [driver 索引](index.md)。
