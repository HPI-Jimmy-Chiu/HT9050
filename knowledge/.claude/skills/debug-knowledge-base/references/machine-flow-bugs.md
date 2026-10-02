# 機台流程 Bug 案例

---

### B01 Auto Clean InArm Z 軸碰撞 Shuttle Sensor
- **問題編號**：P260428-ATC-H9-01
- **影響版本**：V3.33.889
- **上次正常版本**：V3.28A
- **症狀**：Auto Clean 時 A 排吸嘴 Z 軸異常下降，撞擊 Shuttle Sensor
- **根因**：Auto Clean 內部函數在 site 關閉（`bUse8Picker==false`）時未正確跳過處理，導致記憶體越界寫入，A 排吸嘴被錯誤標記下降
- **修法**：補回遺漏的 skip 邏輯 + 加入防禦性檢查 `if(!bUse8Picker) continue;`
- **預防**：所有 site 迴圈必須檢查 site enable 狀態
- **案例**：AMKOR Shanghai (DLC915), V3.33.889 → 修復於 V3.33.904.0 (2026-05-04)
- **提案文件**：[docs/customers/鴻勁興業/972_AMKOR_China/proposals/2026/20260428_軟體異常修正紀錄表_AMKOR_AutoClean_InArm_Pickup_Collision.md](../../../../docs/customers/鴻勁興業/972_AMKOR_China/proposals/2026/20260428_軟體異常修正紀錄表_AMKOR_AutoClean_InArm_Pickup_Collision.md)

---

### B02 OutArm Picker 碰 Shuttle（Shaft 變形）
- **問題編號**：P260520-ATK-H9-01
- **影響版本**：V3.21.902.0（亦影響 V3.21.904.4）
- **症狀**：OutArm Picker 與 Output Shuttle 碰撞，4 支 Picker Shaft 變形
- **根因**：`DoOutARM_SHT_MoveSafe` 中 SHT1 Y 軸安全窗口僅 3mm，不足以涵蓋所有 Recipe 組合
- **修法**：擴大安全窗口至 ±70mm + 強化互鎖判斷
- **預防**：安全窗口值需依最大 Tray Pitch 計算，不可硬編碼小值
- **案例**：AMKOR Korea (PPLS841), 2026-05-12, 損壞 4 支 Picker
- **提案文件**：[docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026/20260520_軟體功能新增提案表_ATK_OutArm_Shuttle_Collision_Protection.md](../../../../docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026/20260520_軟體功能新增提案表_ATK_OutArm_Shuttle_Collision_Protection.md)

---

### B03 Index Position error Z1UpZ2Down1
- **問題編號**：P260504-SCK-H9L-01
- **影響版本**：V3.33.899.2（亦影響 V3.33.904 BETA）
- **上次正常版本**：V3.32.751
- **症狀**：`Index Position error, Index 4 Axis Need home` Alarm，整批生產被打斷
- **根因**：新版保護判斷在 Z2 已下降到 `Prod.All_TestZ_Test_Safe - iCheckZ` 以下時，與 Galil 向量插補時序產生競爭（race condition）
- **修法**：待修 — 需調整保護判斷時序，避免在插補過程中觸發
- **預防**：4 軸 Index 模式下的位置保護需考慮插補中間態
- **案例**：SCK (ILS057), TeraTech Korea
- **提案文件**：[docs/customers/TeraTech/947_SCK/JSCK_V3.33.904.0_20260504_IndexPositionError_Z1UpZ2Down1_Bug.md](../../../../docs/customers/TeraTech/947_SCK/JSCK_V3.33.904.0_20260504_IndexPositionError_Z1UpZ2Down1_Bug.md)

---

### B04 OutArm Z-Axis Offset Reset to 0
- **問題編號**：內部追蹤 (2026-02-26 回報)
- **影響版本**：V3.33.895.8
- **症狀**：OutArm Z-Axis Offset 在 Save 後被歸零（Auto2~Auto6, Fix1~Fix6）
- **根因**：`cOffSet.cpp` 的 `ReadFile()` 函數在 Save 後，當 `bE33InOutArmZOffsetSameOne == true` 時誤歸零每支吸嘴的 Z-axis 偏移值
- **修法**：修正 ReadFile 邏輯，在共享 Z offset 模式下保留已設定的值
- **預防**：Save/Load 函式必須成對驗證（寫什麼就讀什麼）
- **案例**：SCK (947_SCK), TeraTech Korea, 修復於 V3.33.898.0 (2026-03-16)
- **提案文件**：[docs/customers/TeraTech/947_SCK/20260316_軟體異常修正紀錄表_JSCK_Offset_Z_Reset.md](../../../../docs/customers/TeraTech/947_SCK/20260316_軟體異常修正紀錄表_JSCK_Offset_Z_Reset.md)

---

### B05 QA Mode 計數不準、停測失效
- **問題編號**：待編號
- **影響版本**：V3.21.904.1
- **症狀**：設定 QA Mode Count=125 顆，實際測完整盤 365 顆才停
- **根因**：計數邏輯在多盤 Tray 情境下，未正確累計跨盤已測數量
- **修法**：修正 `Check_QA_ModeCount` 累計邏輯
- **預防**：QA Mode 修改後必須以多盤情境驗證
- **案例**：AMKOR Korea (ATK), 2026-05-11, Recipe: MESABI_4.82X6.42
- **提案文件**：[docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026-05-11_QA-Mode-Stop-Failed.md](../../../../docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026-05-11_QA-Mode-Stop-Failed.md)

---

### B06 Auto Clean Count 不增加
- **問題編號**：P260427-ATK-H9-01
- **影響版本**：V3.21.895.5
- **症狀**：Auto clean count 不增加；Clean out alarm 發生時計數被重置
- **根因**：Clean out alarm handler 重置了 auto clean 計數器
- **修法**：分離 alarm 重置路徑與 auto clean 計數路徑
- **預防**：計數器重置操作需列出所有呼叫點，確認副作用
- **案例**：AMKOR Korea (PPLS841), V3.21.895.5
- **提案文件**：[docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026/20260427_軟體異常修正紀錄表_ATK_AutoCleanCount.md](../../../../docs/customers/TeraTech/971_AMKOR_Korea/proposals/2026/20260427_軟體異常修正紀錄表_ATK_AutoCleanCount.md)

---

### B07 Contact Test 不放 IC → Z_Height_Task 快速空轉
- **問題編號**：P260527-XINYUN-H9L-01（子問題 2）
- **影響版本**：全版本（by design）
- **症狀**：Contact Test 按 T.Start 後看似在跑但什麼都沒測，GPIB 端未收到 SOT
- **根因**：`FTestSuck` 全為 NULL_IC 時，`ProcessTestResult` 的 bin 迴圈不執行，`flag` 維持 `true`，`DoSetupTest` 立即 return 1 → `Z_Height_Task` 3000→3010→3100→3000 每 ~12ms 循環
- **流程詳解**：
  - case 3010：`WaitManualStartKey()` 返回 true → 送 `MSG_CMD_ContactTestArm1`（GPIB 端僅記 log）→ 設 `iSetupTask=1`, `IsTest=true` → Task=3100
  - case 3100：`DoSetupTest(0)` → case 1 copies FTestSuck（全 NULL）→ case 2200 `InitTestTask` → case 2400 `ProcessTestResult` → `GetTesterResult` 因 `TestSocket.All_HAS_NULL_IC()` → ret=1 → DoSetupTest return true → Task=3000
  - 若持續按住 T.Start 硬體按鈕 → `Sen[SnRKManualTStart].IsOn()` 每次都 true → 循環不停
- **State Record 特徵**：Task_ListWithTime2.csv 中 Z_Height_Task 值在 3000/3010/3100 間 ~12ms 振盪
- **預防**：Contact Test 前確認吸嘴上有 IC；此行為屬設計如此（非 bug），但可加 UI 警告
- **案例**：XINYUN (PLLW1688), 2026-05-27, 93K EXA

---

### B08 1x2/2x2 大 Pitch + 4×8 雙加熱盤只放半盤 → WAR0150 掛機
- **問題編號**：P260602-TFME-H9-01
- **影響版本**：V3.33.905.0（RogerYang 20250820 / Steven 20251217 分支引入後）
- **上次正常版本**：未受影響版本為 AxxG(`_14`) 吸嘴路徑；AxEx(`_13`) 路徑於 2025 大 Pitch 修正後出現此回歸
- **症狀**：通富微 1x2(DualSite) 模式、FCBGA1343、X-Pitch=45mm、加熱盤 4×8 雙盤（`iPlateSelect==0x03`）；InArm 放料到加熱盤時 `InArmPlaceToHotPlateTask` 卡在 step 100、`InArmTask` 停在 step 1100，EventLog 連續報 `WAR0150 In arm search hot plate data error / SearchPlacePlateXItem_1x2Suck`
- **根因**：`SearchPlacePlateXItem_1x2Suck()` 的 `bPitchOver12000 && XDivision==4 && bUseAxExPicker()`（RogerYang 20250820）失敗遞增分支中，**換盤邏輯 `iPlateSelect==0x03` 被綁在 `iy` 繞回層、與 `ix+=2` 同層**。導致每掃完一欄就換盤，`ix` 與盤號被鎖死同相位：盤0 永遠只到 col 0、盤1 永遠只到 col 2，每盤只用一半欄位；兩欄填滿後搜尋永遠 `bSuccess=false` → 跑滿 `iHangUpCount(10000)` → `DoHotPlateHangUp` → WAR0150。`SearchPlacePlateXItem_2x2Suck()` 的 `Steven 20251217`（XDivision==4 AxEx）分支有完全相同的耦合錯誤
- **修法**：將換盤 `if(iPlateSelect==0x03){swap iPlate}` 從 `iy` 繞回層搬進 `if(ix>spacX){ix=0; ...}` 內，使「`ix` 掃完該盤所有有效欄位（0、2）後才換盤」，與已驗證正確的 XDivision==8（Steven 20250930）/ b4x11HP（KevinC 20250912）分支結構一致。`1x2Suck` 與 `2x2Suck` 兩處同步修正
- **pitch 門檻差異**：AxEx(`_13`) 的 pitchOver 分界 = `iXpitchMaxX2`(≈40mm)；AxxG(`_14`) = `iXpitchMaxX3`(≈60mm)。故 45mm 在 AxEx 跨界進缺陷分支，在 AxxG 仍走安全通用分支（不掛）
- **預防**：搜尋型函式的「換盤/翻頁」動作必須巢狀在「該維度索引繞回」之內，禁止與內層索引遞增同層；新增 XDivision 特例分支時務必驗證雙盤填滿與單盤兩種情境
- **State Record 特徵**：Task_ListWithTime2.csv 中 `InArmPlaceToHotPlateTask` 凍結於 step 100；EventLog WAR0150 FuncName=`SearchPlacePlateXItem_1x2Suck`
- **案例**：通富微電子 (PPLR2694), 2026-06-02, HT9045HA, CUSTOMER_CODE=916；修復於 V3.33.905.0
- **相關 skill**：[ht9045-inarm-flow](../../ht9045-inarm-flow/SKILL.md)（加熱盤搜尋路徑）、[ht9045-state-record-analysis](../../ht9045-state-record-analysis/SKILL.md)（WAR0150 卡點定位）


---

### B09 單臂模式 + D63 馬達尋相衝突 → Index Home 卡機
- **問題編號**：P260603-TFME-H9-02
- **影響版本**：V3.33.905.1（啟用 D63 的 4-axis Index Arm 版本）
- **上次正常版本**：未啟用 D63 時正常（走 iHomeStep=1300 路徑）
- **症狀**：單臂模式（Arm1-only 或 Arm2-only）同時啟用 D63 馬達尋相時，Index Arm Home 流程卡住（Hang）。Log 顯示 Y1 已順利到達教導 Middle(15864)、iHomeStep 已推進到 3060，卡點在「被關 Arm1 的 Z1 尋相」步驟（case 3060/3100），非 Y 定位。
- **根因**：單臂 disable 時 cinitial.cpp 歸零 `Prod.TestY1_Middle=0`、`Prod.TestZ1_Test=0`（被關軸 production 參數），但 `Prod.TestY1_Middle_Home=15864`（教導值）保留。D63 Home 流程（uhome.cpp case 3050→3060→3100）未排除被關軸，仍對被關 Arm1 的 Z1 執行尋相（`Gali_FindZPhase()`）；被關軸測試參數已歸零，尋相無法完成 → Home 停滯。本質：被關軸不應參與 D63 尋相，但 V905.1 未做排除。
- **修法（V905.2，最低風險）**：被關軸略過 D63 尋相，但 Y 定位維持原始教導值（不驅向 0，避免違反「Middle 需用 Teach 位置避免關 ARM 為 0」而撞機）。判斷用既有變數 `iShuttleMode==1 && TestY*_Middle==0`。uhome.cpp：case 3050 routing `(被關)?3250:3060` 略過 Z1 尋相、case 3250 routing `(被關)?3400:3260` 略過 Z2 尋相、case 3610 對被關軸以預設旗標跳過 Y 尋相。Y 目標全部維持原 Middle_Home。不新增變數、不新增移動。
- **避坑**：(1) 不可把被關軸 Y 驅到絕對 0（原碼註解明載 Middle 需用 Teach 位置避免關 ARM 為 0，強驅 0 有撞機風險）。(2) 卡點是「被關軸尋相」非「Y 位置 vs Middle 保護誤判」，須以 log（Y 已到位、iHomeStep 已進尋相步驟）確認真正卡點。(3) Middle 與 Middle_Home 為刻意分離設計，勿改 cinitial.cpp。
- **案例**：通富微電 (PPLR2694), 2026-06-03, HT9045HA, CUSTOMER_CODE=916；修復於 V3.33.905.2 (2026-06-04)
- **關聯 skill**：[ht9045-index-flow](../../ht9045-index-flow/SKILL.md)（D63 Home 尋相路徑 case 3000-3400）；[ht9045-state-record-analysis](../../ht9045-state-record-analysis/SKILL.md)
- **Release Note**：[20260604_HT-9xxx_Software_Release_Note_V3.33.905.2_廠內版.md](../../../../docs/customers/鴻勁興業/916_TFME_CHINA/release-notes/2026/20260604_HT-9xxx_Software_Release_Note_V3.33.905.2_廠內版.md)
