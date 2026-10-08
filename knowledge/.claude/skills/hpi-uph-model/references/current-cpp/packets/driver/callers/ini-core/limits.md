# 機型、版本、平台與未查範圍

同一 UPH Skill 仍整合 [HT9050／其他 Handler 機型樹](../../../../../machines/index.md)。本層 22 個 cpp body 沒有 MachineTypeChoice／CustomerCode 分支，只確認所選 V906 INI 共用符號；不同機型／客戶的路徑、缺值、recipe 和現場參數沒有因此變成相同。

- Aux 的 COM13／COM3、timeout 0／1／70 與客戶分流沿用 [consumer](../tick-consumer/consumer.md)／[form mapping](../form-settings/mapping.md)，不是本層或 HT9050 現場量測。
- Windows 的 CreateFileA／GetFileAttributesA 與非 Windows fopen 路徑分開；這不是 SIM／真機切換。本輪沒有更改 SOFT_SIMULTE 或 runtime 設定。
- V906 UTF-8 是本次選讀的移植版本；V912 Big5 的 BCB6 類別、系統 profile API、特定客戶對照並未重驗。header 的舊版本路徑／行號只保留在原文，活摘要以 function／變數定位。
- 檔名參數來自 caller；沒有讀取現場 INI 內容、工單或機台快照，也沒有測試磁碟寫入。純文件整理不適用機台測試的 snapshot apply／restore 操作。

仍待完整 Fast writer／memory store、typed read/write、enumeration、完整 caller／reload／shutdown、容量／ABI、V912 矩陣與實機 UPH；本輪不宣告 S8 全部語意收斂。來源 comment／裁決正文／metadata／原文／既有資源與相容入口完整保留。
