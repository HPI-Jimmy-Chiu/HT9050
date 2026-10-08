# Picture與數值格式的實際支援界線

[SysUtils.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.cpp)、[SysUtils.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.h)、[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)；選定完整原文與來源 hash 在 [manifest](source-manifest.json)。

`FormatFloat` 的fmtA轉std::string；空format借道AnsiString(double)。非空format只掃0／#／.／,／%：第一個dot之後所有0／#都算小數位，任一comma開grouping，任一percent讓value乘100並在輸出尾端加一個%。沒有解析quote、semicolon正負零子格式或科學記號。

| 步驟／變數 | 正文行為 |
| --- | --- |
| totalDec | decZeros＋decHashes，交snprintf的%.*f；不是有效位數 |
| neg | 僅v<0才另加負號，與CRT對負零／特殊值的文字輸出分開 |
| numbuf[64] | fixed buffer，忽略snprintf回傳；沒有長度重試或錯誤回報 |
| intPart／fracPart | 以文字'.'切割，先補decimal之前mandatory 0，再修剪末端optional 0 |
| decZeros | 作為修剪的最少長度，不追蹤各個0／#的原位置 |
| grouping | 從右向左每3字元插comma；實際comma偵測不限制在decimal左邊 |
| out | intPart＋必要的'.'／fracPart＋一個%；format其他literal不輸出 |

註解說other chars emitted literally，正文未輸出這些literal；例如USD0.00中的USD不被拼入out（靜態推導，未跑）。註解說fracPart必有totalDec字元也不能當普遍保證：64-byte截斷、CRT錯誤或decimal不是'.'時，要另查。intPlaceholders有計數但後續未使用；'#'不是完整Delphi picture引擎。

`FloatToStrF` 分支不同：ffFixed用digits作%.*f；ffExponent用max(precision-1,0)作%.*e；ffGeneral／default用precision>0?precision:15作%.*g。ffNumber遞迴ffFixed後原樣return，未加千分位；enum的ffCurrency沒有case，走general default。每支同樣64-byte buffer、回傳值未檢查；不能把enum存在或faithful enough註解當BCB6／913一致性驗證。

`IntToHex(long long,digits)` 將value轉unsigned long long逐nibble轉uppercase，負digits歸0，至少輸出一個0；digits是最小寬度，不截掉超出的位數，也沒設width上限。int多載先轉unsigned int再擴long long，與負long long的unsigned64表示範圍不同。原註解的MinGW %llX歷史理由保存在manifest，本輪沒有重跑CRT。

回 [數值解析](parsing.md)、[printf backend](../numeric-format/printf-family.md)、[界線](limits.md)。
