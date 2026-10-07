# 同題整合：版本、客戶與機型

HT9050 與其他 Handler 的共用宣告、差異與待查項留在同一份 hpi-uph-model，不另複製機型 Skill。

| 範圍 | 本單元證據 | 不可直接推定 |
| --- | --- | --- |
| V906 C++ | MessageDef.h/.cpp 的 VM／MV、四全域符號、UPH 常數；UTF-8 | 不套用 V912 追加欄位或其命令表機制 |
| V912 BCB6 | 相同 VM；MV 前綴相同但多 sMulti2DIDStringSeparator[10]；命令表產生 UPH=132；cp950 | 宣告相似不代表兩個編譯器／部署程式的 ABI 一致 |
| HT9050 | 這些選定 VM／MV 宣告中沒有有效 Type_HT9050 閘門 | 未核對現場使用的版本、連結與 transport；宣告容量不是啟用 site 或真實 UPH |
| HT9045 與其他 Handler | 同題讀共用欄位，再查各機型實際 dispatcher／caller | 未查全部機型啟用條件、site／批量／節拍或參數 |
| 客戶 | UPH 原註解含 Novatek；AMD_Version 閘門為註解 | 不能視為完整客戶矩陣、客戶專屬支援或 runtime 裁決 |

HT9050 排程／runtime 與歷史機型來源依 [既有版本與機型索引](../versions.md)；版本差異與機型／客戶差異分開判定。此頁沒有修改原有機型資料、裁決或 runtime。

封包欄位 writer、初始化／清除、driver 實作與容量／校正仍待查，見 [界線](limits.md)。回 [封包索引](index.md)。
