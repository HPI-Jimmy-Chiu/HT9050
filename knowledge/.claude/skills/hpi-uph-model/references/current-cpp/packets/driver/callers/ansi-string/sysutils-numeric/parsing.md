# 數值橋接、hex與浮點解析

[SysUtils.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.cpp)、[SysUtils.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.h)、[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)；選定完整原文與來源 hash 在 [manifest](source-manifest.json)。

| free function | 所選正文／預設行為 |
| --- | --- |
| IntToStr(int)、FloatToStr(double) | 分別建AnsiString(int)／AnsiString(double)，沿已完成assignInt／assignDouble；不另做UPH換算 |
| StrToInt／StrToIntDef | 原樣轉ToInt／ToIntDef；exception／default及NUL、range界線見 [原callee](../numeric-format/parsing.md) |
| StrToFloat | 原樣轉ToDouble；不包catch或補finite guard |
| StrToFloatDef | Trim；空字串或strtod無轉換／end非NUL才回def；其他結果原樣回double |
| HexStrToInt | Trim；去開頭0x／0X／$，其他直接以base16解析；空digits或無轉換／end非NUL回-1，否則long轉int |
| TryStrToFloat | null／空C string回false；strtod後只略空格／tab／CR／LF，end必須NUL；成功才寫value |

HexStrToInt("10")走base16，與StrToInt("10")的decimal路徑分開；此例是正文推導，沒有執行。HexStrToInt只辨認開頭prefix，不能把註解引用的AnsiPos任意位置搜尋當成本體；已有prefix的處理也不等於共用vc_parseIntBCB6。

StrToFloatDef與TryStrToFloat直接呼叫std::strtod，沒有locale物件／切locale、errno清除／讀取、finite或range檢查。註解稱locale-independent或force '.'，正文沒有實作這種強制；保留原說法並把差異記清楚，不宣稱目前locale一定是哪一種。

兩者的成功條件是NUL-terminated end，而非完整std::string長度；embedded NUL後的byte不在該條件內。TryStrToFloat與Trim後解析對尾端空白的處理不同；本輪不把兩支等同。NaN／Infinity、overflow／underflow是否產生值與errno依CRT／locale，沒有執行對照；caller不能把true當有限有效機台參數。

TryStrToFloat失敗保留value，成功才覆寫；呼叫端如何用value或重解析見 [Change Log](callers.md)。完整caller仍未閉合。
