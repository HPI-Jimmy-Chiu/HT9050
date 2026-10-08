# 版本、機台共用及未完成界線

[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)；[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)。完整選定函式與歷史註解保存在 [manifest](source-manifest.json)。

| 範圍 | 共用項／差異 | 證據 |
| --- | --- | --- |
| V906 C++17 shim | std::string byte儲存、選定搜尋／編輯／比較與case-copy | pin `6ce3c86225d661c67efa3da11d562722dc58efc9`；9 cpp／35inline靜態查證 |
| HT9050與其他V906機台 | 可使用同一library；輸入編碼、路徑、工單、機型caller仍各異 | 兩來源MachineTypeChoice／Type_HT9050 literal為0，只是窄範圍搜尋 |
| HT9045／HT9046A／HT9046LS客戶版 | 需依實際版本／客戶／input確認caller | 未新做機型或客戶矩陣 |
| V912／V899 BCB6 Big5 | RTL／MBCS／reference counting／ABI可不同 | 保留header歷史目的與golden註解；本輪未讀全RTL／跑oracle |
| runtime／實機UPH | 各台設定與輸入不同 | 沒有build、fixture、設定同步、C++執行或機台操作 |

完整header原文保留metadata、unsigned歷史裁決與所有宣告，僅35選定inline算本輪查證；Trim／assignInt與先前str()入口不重算。未宣稱全AnsiString、BCB相容性或UPH／S8發布語意完結。

剩餘numeric／printf backend、locale設定與library ABI／容量／配置失敗／並行、完整INI／GPIB／UI caller、其餘main來源及V912客戶矩陣。pointer生命期與標準library行為已標靜態推導，不等同實機結果。

批次每段ordinary push自己的codex/工作分支；約20:24一張Ready MR target main交Jimmy／筆電，不auto／self merge／直推main。合後自己pull核main包含，再由ST02-M寄RD5_SW；本輪不寄信。

回 [入口](index.md)、[caller樹](../../index.md)。

## 新 golden 裁決接續

ST-HandOver `68febfe9530b8b80f1e0eabbdb1a970c6fa9b585` 的 ST02-M 20261008 19:3x 轉達 Steven：新改動／Skill新單元引用golden，改認 [913 repo main](https://gitlab.honprec.com/honprec/rd/rd5/ht9045_913/-/tree/e9908638077a877cd4ea56355c6d98df97f48f08)。本輪唯讀ls-remote核main為 `e9908638077a877cd4ea56355c6d98df97f48f08`，tag V3.33.913.0的peeled commit為 `d46ac5d21eaec48cc8a49321eb25d28f692984f2`，兩者不同；依裁決只認main，不把tag或其他未合分支代替最新main。實際引用golden時按來源規則寫913檔名與定位。

本單元只查V906 shim，沒有新增BCB golden RTL或機台比較，故header中0618歷史golden註解原文保留，不改成913來假稱已驗。下一個需要golden的單元先讀913 main；既有已完成單元僅在查出差異時續補，不例行重做。NB3 clone路徑為peer回報，未假設本機也有同一clone；沒有操作其他電腦／session。
