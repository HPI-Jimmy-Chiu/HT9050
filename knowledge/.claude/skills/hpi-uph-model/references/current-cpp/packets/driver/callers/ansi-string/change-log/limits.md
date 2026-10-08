# 保存與尚未驗證的界線

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

- 本輪是 V906 所選 Change Log caller／caption／名稱與 MM helper 的靜態文件；22cpp／1full-header／5region 共28新原文、7來源。五個既有 WriteIniData 只核body hash，W906_ChangeLog_Str既有原文不重算。37份歷史 manifest、原 metadata、正文、資源與相容入口保持。
- 可從 source 確認：bool/int gate、ULong的%d實參不相符、double先解析四位Str、MM入口int／共享static buffer、caption／DIO與form狀態、wb_serve所選boot接線。未執行C++、build、tests、INI／DB寫入、機台/runtime或machine_sync，不稱實機驗證。
- 不直接推導ULong實際輸出；不保證double轉int的非有限／超範圍值、GetFloatFormatString一般輸入／thread-safe、caption與UI/runtimelist等效。沒有改 ABI／format、加入runtimeguard或修正任何機台程式。
- RecordChangeLogProcess今天backend、lot logging、pointer生命週期／threads、全部啟動／HTTP/caller、locale／errno／range、機型／客戶／版本矩陣仍未閉合。靜態inventory包含comment／declaration，不當runtimecoverage。
- 新golden查證依Steven最新裁決用ht9045_913最新main；本輪只用V906 pin，未對照913 RTL或BCB6。banner／函式comment舊golden與當年測量裁決逐字保存，不把其結果冒稱本輪驗證。
- 舊!348第十四批29檔已核main／自己pull與ST02-M寄件回覆，依Batch ID標記不重寄。本輪沿新numeric批累積，普通push自己的codex/工作分支；約四小時一Ready MR給Jimmy／筆電，不auto／self merge／直推main；main包含後通知由ST02-M處理。

回 [入口](index.md)、[前numeric界線](../numeric-format/limits.md)、[前SysUtils界線](../sysutils-numeric/limits.md)。
