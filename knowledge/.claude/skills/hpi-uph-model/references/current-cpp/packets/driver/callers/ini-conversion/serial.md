# TDateTime、epoch 與 encode/decode

定位 TDateTime.h 的 Word／value_／Val 與 constructor/operators；TDateTime.cpp 的 kOleEpochDaysFrom1970、daysFromCivil、civilFromDays、serialFromTm、splitSerial、DecodeDate、DecodeTime、EncodeTime。[manifest](source-manifest.json) 保存完整 header 及上述完整選定 cpp 定義／常數。

Word 是 unsigned short。TDateTime 是 double value_ 包裝：預設 0.0，double constructor／賦值／Val／轉型沒有合法值檢查，算術直接對 double 做加減。header 的 OLE／BCB6／golden usage 註解完整保留，屬來源描述，不是本輪實機或 BCB6 實測；Now／Date／Time／EncodeDate 等只有 header 宣告，未列為本單元已查完的 cpp。

| 符號 | 所選 body 的運算 | 尚未證明 |
|---|---|---|
| daysFromCivil／civilFromDays | Gregorian 整數換算、era／day／year 分解 | 非法日期、整數溢位、全部平台相容契約 |
| kOleEpochDaysFrom1970 | daysFromCivil(1899,12,30)，原註解 -25569 | 不以註解替代測試 |
| serialFromTm | mon/day 轉 unsigned，整數日減 epoch；加時分秒毫秒日分數 | 輸入範圍與跨平台全部數值行為 |
| splitSerial | days=floor(serial)，frac=serial-days；日轉 long long 加 epoch | finite／double→integer 範圍、BCB6 負 OLE date 語意 |
| DecodeDate | splitSerial→civilFromDays，再將年月日轉 Word | 原 serial 合法性、年超出 Word 範圍 |
| DecodeTime | frac*86400000+0.5 轉 long long；>=86400000 夾成 86399999，再分解毫秒秒分時 | 不將四捨五入跨日進位交回日期；非有限值／超範圍未測 |
| EncodeTime | Word 時分秒毫秒算日分數後包成 TDateTime | 本體沒有範圍檢查或自行以 24 小時取餘 |

splitSerial 使用 floor 與非負日分數的程式路徑，不把 header 的 OLE 名稱推為已完成負日期 differential。DecodeTime 的上限夾取與 EncodeTime 的無範圍檢查是不同控制流，不能互相當成合法性驗證。

解析如何到 serialFromTm／EncodeTime 見 [parse](parse.md)；DateTimeToStr 如何使用 DecodeDate／Time 見 [format](format.md)。沒有執行數值、BCB6、時區、clock、ABI 或機台測試。
