# Payload 版本與機型界線

GPIB／RS232差異在同一 UPH Skill 中依 engine 分流。這兩個 V906 body 沒有 MachineTypeChoice／CUSTOMER_CODE gate；實際機型／客戶選哪個 engine、初始化與閉合時序尚未查完。V912 先前所選[WM_COPYDATA](../../../flow.md)不可套用本地 padded buffer 或 depth 狀態。

MV／Command結構與編碼、完整 GPIB／RS232 OnMyCopyMsg、g_handlerMsgs全部 reader、factory／IsUp／closeRequested caller、buffer生命週期、並行與外部傳送結果仍待查。舊 golden讀者／安全說明只保存在歷史證據，未重驗。

同題機型參考[HT9050](../../../../../../../machines/ht9050/index.md)與[其他Handler](../../../../../../../machines/ht9045.md)；UPH容量／site／校正與保存鏈仍未閉合，沒有執行程式、API／LIVE或機台。回[上層界線](../limits.md)。
