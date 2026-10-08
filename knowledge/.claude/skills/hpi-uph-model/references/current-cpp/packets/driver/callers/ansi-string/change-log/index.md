# Change Log 數值 caller、單位與 hook

識別 `STGPT-UPH-CHANGELOG-NUMERIC-CALLERS-20261008`；V906 pin `25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0`，推前核最新 main 的來源 byte。沿 UPH 字串依賴樹接續，既有入口、metadata 與歷史裁決原文保留。

| 查證問題 | function／變數與路由 |
| --- | --- |
| typed writer、hook 初值與安裝 | [接線](hooks.md)：WriteIniData、W906_InstallChangeLogHooks、W906_ChangeLogHook_* |
| printf 實參、double 比較、MM 換算 | [數值與單位](values.md)：四 numeric hook、ConvertToMMType、GetFloatFormatString、CutSpaceAtHead |
| 客戶 caption、DIO 檔名與 form 狀態 | [caption 與機型](captions.md)：CL_Pick、CL_FTestIF_*、CL_fCleaning_*、CL_fContact_* |
| 參數名稱與 ATC/site 索引 | [名稱轉換](names.md)：TempChangeLog、ATCTempOffset、iAutoClean_MotorSpeed |
| 尚未閉合的執行與版本問題 | [界線](limits.md)：locale、ABI、thread、儲存、913與各機型 runtime |

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

保存 22 個完整 cpp 定義、1 份完整 interface header、5 段所選歷史／接線區，共 28 原文、7 來源。五個 WriteIniData 舊 writer 只重核 body hash，已完成 W906_ChangeLog_Str 不再新增完成數。

回 [caller 樹](../../index.md)、[SysUtils caller](../sysutils-numeric/callers.md)、[printf backend](../numeric-format/printf-family.md)、[INI typed](../../ini-typed-enumeration/index.md)。

- [現行記錄backend](backend/index.md)：14cpp／2完整header／12region28原文；ChangeLog→Process欄位軸、兩參數adapter、SQLite gate、EventLogTxt／HANDLER／EventTracker／ProductionLog與啟閉；深層writer／callee／913與runtime另續。
