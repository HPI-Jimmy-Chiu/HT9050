# 20261006 main更新：LotInfo三處解除舊閘

本機初版基準f57d93f15之後，已pull／整合main `b8ea3a511`。以下按該次差異及函式本文靜態核對，來源HT9011UC_Cpp_V3.33.906.0/forms/fLotInfo.cpp及.h；沒有開批、啟機或動態測試。

| 原閘／函式 | 此次main狀態 | 仍須分清 |
|---|---|---|
| WA-5／InitialRefrigerantSystem | ATC_OFFLINE_FormComInit的呼叫已解除#if 0 | 呼叫恢復不代表每種ATC／壓縮機已上機驗證 |
| WA-9／btTesterTCPShowClick | fTesterTCP->Show()呼叫已開，include forms/fTesterTCP.h | 目前移植的Show是no-op；不得寫成已打開原生Tester TCP視窗 |
| WD-3／LotKeyInTimeTimer | CC_KYEC_LEE內依cbRunStartMode->Text判讀Operator ID的原鏈已開 | 客戶／AMR／RunMode條件仍存在，不推成所有機型同一路 |

原文的關閘紀錄仍保留其日期，回答現在狀態先看本表。wb_serve此次變更多為pass profiler／WdMark及1203巡檢節流；Lot的lot.start直接SetLotStart、Config的gAllowSystemWrite／Cowner與QA計數／還原函式未在這次diff改動。

[Lot導讀](../lot/index.md)／[目前Recipe寫者](recipe.md)／[目前QA](qa.md)。本次只補Skill文件，不重寫他人的實作或宣稱實機已驗證。
