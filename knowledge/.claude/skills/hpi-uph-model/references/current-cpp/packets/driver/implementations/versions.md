# 共用項、版本與機型差異

| 分流 | 已有來源依據 | 還不能推定的項目 |
| --- | --- | --- |
| V906共用wrapper | 經IGpibDriver呼叫，再Refresh狀態；[wrapper](../index.md)已有獨立查證 | 相同wrapper不代表NI／Sim具相同送達、等待或錯誤語意 |
| V906 NI | LoadLibrary／十二export與九呼叫、三狀態getter | 實際DLL版本、ABI、硬體及所有caller |
| V906 Sim | 三佇列／talk_／FindFails，本地狀態與讀寫 | 真實NI GPIB時間、匯流排或實機UPH |
| V912 BCB6 | [封包版本對照](../../versions.md)已保留其宣告差異 | 本單元未讀V912 driver對應實作，不由V906 class名套用 |
| HT9050 | 兩份選讀class定義沒有Type_HT9050分支 | 實際部署、NI／Sim選擇、SOFT_SIMULTE與機型／客戶啟用矩陣 |
| HT9045／其他Handler／客戶 | 同題保留共用wrapper與版本界線 | 不把來源註解的golden行為當作所有機型／客戶已驗證 |

constructor在本單元指兩個driver自己的定義；誰建立與SetGpibDriver如何連接仍待下一層caller。機型容量／site與現場校正沿既有 [UPH機型樹](../../../../machines/index.md) 查。回 [兩實作索引](index.md)、[界線](limits.md)。
