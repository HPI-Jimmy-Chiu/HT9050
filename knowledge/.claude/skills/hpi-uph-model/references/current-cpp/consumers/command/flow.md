# 格子字串到指令回覆

已讀兩版GetUPH／UPHStrings四完整文字，兩版GetAll只讀所選call區段；[manifest](source-manifest.json)保存source／body／片段hash。這裡記本地取值及呼叫，不把SendMSG_CMD當網路送達。

兩版 `TfMain::UPHStrings`讀 `fShowBinSelect->UPH_StringGrid->Cells[3][1]`：空字串回"0"，非空以 `asRet.sprintf("%s",...)`回格子字串。本地沒有檢查Cells[3][0]表頭、重新計算、格式／範圍或更新時間。

V906的字面 `#if 1`已打開讀grid支路，`#else`仍保留回"0"的歷史stand-in；V912沒有此stand-in。本批核對V906所選active文字與V912的token等價、兩GetUPH文字相同；這不是BCB／C++17平台、grid生命週期或實機結果等價驗證。舊GATE註解中的不存在聲明已明標STALE，不能當現在仍停用。

`TfMain::GetUPH`兩版都先 `asRet=UPHStrings()`，再 `SendMSG_CMD(MSG_CMD_UPH,asRet)`。SendMSG_CMD、訊息編碼、transport／接收端和作用中caller尚未查完；source註解Novatek只作來源線索，沒有以此宣稱只有該客戶或全部客戶可用。

兩版 `TfMain::GetAll`所選區段也呼叫 `UPHStrings()`，旁邊有DUTStausStrings／IndexCycleTimeStrings等。只查這一call實參，不據此推完整欄位位置、逗號／編碼格式、其他欄位或整個回覆已驗。

| Consumer | 空格與條件 | 本地值口徑 |
| --- | --- | --- |
| V906／V912 Command UPHStrings | Cells[3][1]空，沒有本地表頭gate | 字串"0"；不能單靠它分清尚無資料與真零UPH |
| V906 WebBridge StageUphStatusTags | 依live／customer與Cells[3][0]表頭；空白／尚無最新數字分開 | blank=-2、no-number=-1或atoi結果，見[WebBridge取值](../webbridge/values.md) |

這兩consumer不能互換sentinel或當實際產能校正證據；完整producer／counter仍回[計算](../../calculate.md)與[計數](../../count-time.md)逐鏈查。回[界線](limits.md)。
