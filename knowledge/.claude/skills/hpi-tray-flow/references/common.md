# Tray 共同項與機型差異

基準：main `9d9dfa9c7`，20261006。四個原技能併在同一主題，表中的共同項是查證介面或工作目的，不代表每一支機型走相同機構。

| 項目 | 共同項／目的 | 通用 HT9045 與歷史資料 | HT9050 差異 |
|---|---|---|---|
| 取放盤 | 追 TrayArm、站點 fHasTray、Task 與實體交接 | DoCatchTray／DoCatchFromLoader／DoPlaceToBuffer 等原流程 | 相同主題下分流至 _9050 函式；逐層 Z 與 C_MobileTrayTableSelect 交接 |
| Tray Z | 分出／接收單張盤，須確認支撐與爪交棒 | CylinderUp/Middle/Lower 三階；LOAD_Z_USE_MOTOR 選軌的固定點馬達或氣缸路線 | _9050 原資料直接用 MLoaderZ／MEmptyZ／MAuto1～3Z，按層教導值；Auto 還有複合氣缸機構 |
| 陣列語意 | 先核對容量、索引起值與教導來源 | 原版 TrayZ_Up[] 是軌；LOAD_Y_USE_MOTOR 原說明為 V899 | HT9050 原910資料拿 TrayZ_Up[] 作層，原文件有越界及未賦值警示；待按當前 source 查證，不能宣稱已修 |
| Supply／Receive | 分別追資料寫入、感測確認與最後清帳 | DoAutoReceiveBinTray 是生產收料換盤；DoReceiveAutoTray 是收工批次退盤 | 部分通用 helper 與 _9050 流程並存，不能概括為 [TrayZ] 對 HT9050 完全無效 |
| 整盤 | 整理 IC 格位與盤移動需分開判讀 | P27 由 OutArm 單支基準吸嘴重排 IC，受客戶旗標及 Magazine 互斥控制 | 沒有本批直接核對的 HT9050 P27 啟用證據，不能當成共有有效功能 |

## 當前來源碼查證

- `HT9011UC_Cpp_V3.33.906.0/database.cpp`：目前 `9050GPIB` 預設解碼為 `Type_HT9050`；定義 `W906_HT9050_AS_LS` 才走舊替代路線。部分移植註解仍寫「never called today／Type_HT9046_LS」，該註解早於20261004機型裁決，不能壓過目前有效分派碼。
- `HT9011UC_Cpp_V3.33.906.0/asendic_Loader.cpp`：`DoLoad()` 開頭有 Type_HT9050 分流至 `DoLoad_9050()`，之後 return；不是把通用 case 表直接套用。
- `HT9011UC_Cpp_V3.33.906.0/acatchtray.cpp`：保留 `DoCatchFromLoader_9050()`、`DoPlaceTrayToEmpty_9050()`、`DoPlaceTrayToAuto_9050()` 與對應的呼叫／Task。存在分流程式不代表特定機台的機型設定與教導已就緒。
- [V906 移植狀態帳本](../../../../HT9011UC_Cpp_V3.33.906.0/docs/FLOW9050_PORT_LEDGER.md) 保留硬體、教導與翻譯缺口；本文未把910歷史警示推定為今日已解決或仍必然存在。

## 詳細分支

[機型 HT9045](machines/ht9045.md)／[機型 HT9050](machines/ht9050.md)／[流程](flow/index.md)／[機構](mechanisms/index.md)／[整盤](sorting/index.md)／[客戶](customers.md)。
