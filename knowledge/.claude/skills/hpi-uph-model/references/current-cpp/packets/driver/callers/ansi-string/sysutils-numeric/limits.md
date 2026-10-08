# 版本、機型與保存界線

[SysUtils.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.cpp)、[SysUtils.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.h)、[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)；選定完整原文與來源 hash 在 [manifest](source-manifest.json)。

- 本單元是V906 C++17／UTF-8 shim與所選caller局部靜態查證；13cpp／2inline／3interface，共18新原文，7來源。前單元numeric／byte、既有UPH與INI正文與metadata／原pin保留，不把context重算為新完成。
- SysUtils共用函式沒有MachineTypeChoice／Type_HT9050分支，不據此宣稱HT9050、HT9045／9046或各客戶runtime相同；沿 [機型樹](../../../../../../machines/index.md) 分Hot／Ambient模型、量測與CSV。LotRecordUPH的KYEC／FOREHOPE／VTEST版本分流仍依舊writer來源與caller，未新驗V912／V899或913機台。
- 新golden引用依Steven最新裁決用ht9045_913最新main。本輪只引用V906固定pin；來源comment的BCB／golden／舊行號原文完整保留在manifest，沒有查913 RTL、重編BCB6或更改歷史裁決。
- FormatFloat literal／semicolon／locale、FloatToStrF ffNumber／ffCurrency、TryStrToFloat的locale註解與正文差異已按實際分支說明。推論不冒稱執行結果或機台異常。
- 未閉合：完整Format／sprintf多載和所有實參型別、CRT locale／errno／range與ABI、hook安裝／thread／保存結果、所有客戶機型caller、913實際差異及UPH／S8實機。18原文完成不代表整個SysUtils或UPH Skill完成。
- 本單元與AnsiString numeric屬同一新批；!348舊29檔保持固定且不計新批，未進main前不重送依賴。普通checkpoint持續；整批依約四小時cadence交一張Ready MR給Jimmy，不auto／self merge、直推main或自行寄信。

回 [入口](index.md)、[caller](callers.md)、[numeric界線](../numeric-format/limits.md)。
