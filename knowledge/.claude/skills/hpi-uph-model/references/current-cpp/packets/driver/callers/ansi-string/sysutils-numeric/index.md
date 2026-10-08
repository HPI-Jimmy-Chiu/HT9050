# SysUtils 數值、格式與所選 caller

識別 `STGPT-UPH-SYSUTILS-NUMERIC-CALLERS-20261008`；來源 pin `39c6388c0e052532ea5933ab0c53ab7b3ecf2557`，推前核最新 main 的來源 byte。沿 UPH 同題字串依賴樹接續，保留既有入口與原 pin。

| 問題 | 路由／function |
| --- | --- |
| 整數、hex、浮點解析與預設 | [解析與數值橋接](parsing.md)：StrToInt／Def、HexStrToInt、StrToFloat／Def、TryStrToFloat |
| picture、precision、64-byte buffer | [浮點格式](pictures.md)：FormatFloat、FloatToStrF、IntToHex |
| Format、ARRAYOFCONST、UPH／Change Log | [所選呼叫端](callers.md)：Format、LotRecordUPH、W906_ChangeLog_Str |
| 版本、機型與未完成項 | [界線](limits.md)：locale、errno／range、ABI、913與runtime |

[SysUtils.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.cpp)、[SysUtils.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.h)、[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)；選定完整原文與來源 hash 在 [manifest](source-manifest.json)。

保存13完整cpp、2inline（含template）、3interface／歷史註解區，共18原文。7來源包含3個摘錄來源與4個caller／registration／test context來源；既有LotRecordUPH與WriteIniData只核body hash，不新增完成數。

回 [caller樹](../../index.md)、[AnsiString numeric](../numeric-format/index.md)、[UPH writer](../../../../../writers/index.md)、[INI typed](../../ini-typed-enumeration/index.md)。
