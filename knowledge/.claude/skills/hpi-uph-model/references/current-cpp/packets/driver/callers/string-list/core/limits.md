# 版本、機台與剩餘查證

本節查的是V906 UTF-8 C++相容層共用實作。兩個來源中MachineTypeChoice／Type_HT9050 literal出現數均為0，結果保存在 [manifest](source-manifest.json)；文字缺席不能證明所有機型／客戶caller都相同。

| 對象 | 已保存與界線 |
| --- | --- |
| HT9050與其他V906 Handler | 共用TStrings核心／Text實作；機型選路、caller、configuration、customer操作及runtime未閉合 |
| V912量產／V899客戶參照 | 沿 [機型入口](../../../../../../machines/index.md)；BCB6原生VCL的行為／Big5／客戶機台版本不能由本移植層推定 |
| 舊INI／GPIB／UI文件 | 前段來源pin、原文、metadata、資源與相容入口保存；本節僅補兩共用來源，不宣稱完整call graph |
| 歷史裁決與fixture註解 | 全header與CRLF正文保存；原文日期／行號／520 new-sites／1030 Count等數字不作本輪活定位或測試結果 |

下一層續CommaText／DelimitedText parser與proxy、LoadFromFile／SaveToFile及AnsiString深層comparison／char*契約，再接完整INI／GPIB／UI caller、locale／ABI／容量／並行、其他main來源變更及V912／客戶機型矩陣。
UPH／S8實機驗證、HTTP／hook／PageTableTick／window registry／HomeCompose與W-159／週報的未結部分仍按既有分工；本節沒有執行程式、build、oracle、fixture、UI／API或機台量測。
引用／錨點／hash與保存檢查通過，只證明文件完整及選定靜態來源相符。回 [入口](index.md)。
