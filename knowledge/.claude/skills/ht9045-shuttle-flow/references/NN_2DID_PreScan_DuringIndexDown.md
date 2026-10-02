# NN 模式 2DID 下壓期預掃（Pre-Scan During Index Down）

> 功能實作日期：2026-07-28（Steven）
> 基準版本：`HT9011UC_Code_V3.33.910.0_20260716`（試驗 build V3.33.910.1）
> 客戶來源：947_SCK（TeraTech）UPH 改善需求
> 詳細發版說明：`D:\docs\customers\TeraTech\947_SCK\release-notes\2026\20260728_HT-9xxx_Software_Release_Note_V3.33.910.1_廠內版.md`

---

## 1. 功能概述

NN 模式（TwoArm32Site）原本的 2DID 讀取時機在「測試完畢 → Index 放已測 IC 到 OutShuttle 之後」，InShuttle 才會移到 Index 區域掃碼——掃碼時間與電性測試時間是**序列關係**。

本功能把 2DID 讀取提前到「Index 雙臂下壓、等待測試」的時間窗（`iTestTwoArm32SiteTask==241`）內，讓 InShuttle 掃碼與電性測試**平行進行**，縮短 cycle time。

### AS-IS（開關關閉 / 舊行為）

```
Index 下壓(case230) → 等待測試(case240/241) → 測試完 → Up(case110) → destroy(case200)
                                                                        ↓
InShuttle 停在左邊等待 ──────────────────────────────→ OutShuttle 收到 IC 後才進掃碼(case200/201)
```

### TO-BE（開關開啟）

```
Index 下壓(case230) → case240 起動穩定延遲 timer → case241 等待測試
                                    ↓（timer 到期 + 8 重條件成立）
InShuttle case10 改道 → case200→201→2400 提前掃碼（與測試並行）
                                    ↓
掃完碼、測試未結束 → case210 hold → 主動回 case100（左邊停等）
                                    ↓
測試完 → Index case110 抬升（若 Shuttle 仍在掃碼則等待）→ 之後流程不變
```

---

## 2. Config 開關（`config.ini [Index]`，v1 為 ini-only 無 UI）

| Key | 型別 | 預設 | 說明 |
|-----|------|------|------|
| `bNNShuttlePreScan2DID` | bool | **false** | 功能總開關；關閉時所有插入點條件短路，100% no-op |
| `iNNPreScanStableDelayMS` | int | 500 | 下壓到位後的穩定延遲（ms），clamp 100~5000 |

讀取位置：`cConfiguration.cpp` `ReadLockByFile()` 尾端（開機自動種入預設值）。
結構定義：`Config.h` ~L590。

---

## 3. 核心 Gating 函式（兩層）

### 3.1 `IsIndexZ1Z2DownStableForPreScan()` — Index 端穩態判斷

位置：`atester_32Site.cpp` ~L698。八重條件**缺一不可**（依序短路）：

| # | 條件 | 目的 |
|---|------|------|
| 1 | `IniConfig.bNNShuttlePreScan2DID==true` | 功能開關 |
| 2 | `bUseTwoArm32Site==true` | 僅 NN 模式 |
| 3 | `USE_INDEX_ARM_AXES==IndexArm_4_Axis` | **3 軸機一律排除**（3 軸版互鎖視 Y1@Middle+Z down 為 Shuttle 禁動姿態） |
| 4 | `iTestTwoArm32SiteTask==241` | 僅測試等待窗；D41 retry（Task=50 鏈）/ Up（110）**結構性**自動失效 |
| 5 | `IndexStatus==Z1_Z2_Down` | 雙臂皆已下壓到位 |
| 6 | `iHome==0 && fAllMotorHome` | 無 Jam / 非 Home 中 |
| 7 | `!bResetMode` | Reset 流程維持原時序 |
| 8 | `bNNPreScanTimerArmed && NNPreScanStableDelay_32.Off()` | timer 曾被武裝且穩定延遲已到期（哨兵旗標防 `TQPF_Timer` 未初始化垃圾值） |

timer 武裝點：case240（`fMain->SendMSG_CMD(MSG_CMD_Arm1Down)` 之後），僅開關開啟時執行。

### 3.2 `IsNNPreScan2DIDCanStart(int iSht)` — Shuttle 端掃碼前置條件

位置：`acarry.cpp` ~L3266。在 3.1 成立的前提下，再完整複製既有 case201 掃碼前的排除條件：

- `BAR_CODE_INSTALL` 必須為 in-shuttle 掃碼型（排除 `ebctUninstall`/`ebctOutShtAMD`）
- `TestIF_File.bEnableBarCode==true`（Recipe 啟用 2D）
- 排除：Bottom2D HTTP（`BOTTOM_2DID && bEnableBottom2D`）、Reset、AMD dummy run、rotate-shuttle 檢查、Shuttle 浮料檢查（`SHT_FLOATING_CHK`）、雷射距離檢查、Fix3 氣缸（`bShuttleMoveToLeftforFix3`）
- 每側（iSht=0→SHT1/FLCarryKit、iSht=1→SHT2/BLCarryKit）另檢查：
  - InShuttle kit 已滿載（`UseSiteHasIC() && IsF/BLCarrKitAllHasIC()`）
  - 尚未掃過（`fBarCode->IsSHT2DIDScanFinish(iSht)==false`）
  - 非 D43 retry（`bShuttleXMoveToLeft/Right==false`）

> **設計原則**：第一版對所有不相容機型/流程一律保守排除（維持原時序），待產線驗證後再逐項評估開放。

---

## 4. 狀態機修改點

### 4.1 Shuttle 端（`acarry.cpp`，SHT1/SHT2 對稱）

| 位置 | 修改 | 說明 |
|------|------|------|
| SHT1 case10（~L3615）/ SHT2 case10（~L5520） | `FRCarryKit.UseSiteNoIC()`（OutShuttle 尚無 IC）時，原本一律 `Task=100`；改為 `IsNNPreScan2DIDCanStart()` 成立時改道 `Task=200` | **根因修正**：測試期間 destroy 未發生、OutShuttle kit 必為空，舊邏輯永遠到不了掃碼入口 case200/201 |
| SHT1 case210（~L4645）/ SHT2 case210（~L6444） | 預掃開啟＋非 D43 retry＋`F/BTestSuck.UseSiteHasIC()`（該批 IC 還在測試中）→ 強制 `Task=100` | **hold 邏輯**：掃完碼主動停回左邊，防止測試中被導去右邊造成振盪或與 destroy 時序衝突；destroy 清空後條件自然解除 |

### 4.2 Index 端（`atester_32Site.cpp`）

| 位置 | 修改 | 說明 |
|------|------|------|
| case240（~L1856） | 開關開啟時 `NNPreScanStableDelay_32.SetMSAndOn(iNNPreScanStableDelayMS)` + `bNNPreScanTimerArmed=true` | 下壓完成起算穩定延遲；關閉時零副作用 |
| case110 busy guard（~L1308） | 任一 Shuttle 的 `AutoSHTxTask ∈ {200, 201, 2300, 2400}` 時，強制 `fCanMoveM=true` 並 `return false` 等待 | 見 §5「假成功地雷」 |

---

## 5. 安全設計要點（風險審查結論）

### 5.1 `MotorMove()` 假成功地雷（case110 busy guard 的存在理由）

既有 case110 在 Index 抬升前用 `CheckShuttlePos()` 確認兩 Shuttle 停在定位後，會鎖 `MOT[MInShuttleX].fCanMoveM=false`。但 `mymotor.cpp` 對 Shuttle 馬達在 `fCanMoveM==false` 時 `MotorMove()` 回傳 `1/-2/-3`——**皆為 truthy**，而 `BarCode_Sh1.cpp` 有 25 處裸 `if(MOT[...].MotorMove(...))`，會把「被鎖住沒動」誤判為「已到位」→ **用錯位置辨識 2DID 的靜默品質風險**。

**對策**：掃碼流程進行中（Task 200/201/2300/2400，含轉換窗）一律不鎖 `fCanMoveM`，Index 在 case110 原地等待。最壞情況是 Up 延後幾個 tick，不會撞機。

> ⚠️ 首輪實作只涵蓋 2300/2400，漏掉 200→201 過渡窗，經風險複審補上。**未來若擴充掃碼鏈 case（如 2500/3000/3100/2600 納入預掃路徑），busy guard 清單必須同步擴充。**

### 5.2 3 軸機強制排除

3 軸版 `IsTestZ2NotSafeShuttle2CanNotMove()`（`acarry.cpp` ~L5217 變體）明文將「Y1@`TestY1_Middle` + Z 下壓」視為 Shuttle2 禁動姿態——這正是 NN 下壓穩態位置。3 軸機可以跑 NN 模式（`bUseTwoArm32Site` 由 TestMode 決定，與軸數無關），故必須以 `USE_INDEX_ARM_AXES==IndexArm_4_Axis` 硬性排除，**此為必要防線非可選項**。

### 5.3 結構性 gating 優於平行旗標

用 `iTestTwoArm32SiteTask==241` 判斷測試窗，天然涵蓋所有離開路徑（D41 retry 的 5 處 `Task=50`、Up 的 `Task=110`、STOP/RESET 的 `InitAllProcessTask()`），無需手動 set/clear 同步的平行 bool 旗標。

### 5.4 既有互鎖不動

`IsTestZ1NotSafeShuttle1CanNotMove()` / `IsTestZ2NotSafeShuttle2CanNotMove()`（encoder 級硬停）與 `DoInOutARM_SHT_MoveSafe()` 皆**未修改**，保留為最後防線。下壓穩態時 Y1/Y2 已在 Middle（離開 Front/Rear 範圍），4 軸版互鎖判斷式為 false，Shuttle 可移動——這是本功能可行的幾何前提（詳見 spec `motor-spatial-layout.md` §3.4/§4.4）。

---

## 6. 已知事項

1. **掃碼時間 > 測試時間的極端情況**：case110 busy guard 會延後 Up 直到掃完＋回左；最差 cycle time 與原流程相同（不劣化、無增益）。
2. **掃碼失敗 alarm**：Index 會安全停等於 case110，關閉對話框後恢復（可見、不危險、與既有 pattern 相同）。
3. **case110 鎖存 vs Shuttle Task==1/10 的 1~2 tick 窄窗 race**：原始碼既有行為，本功能不放大。
4. **UI 未實作**：v1 僅 `config.ini` 直接設定。

## 7. 實機驗證清單（上線試驗前）

1. flag off 回歸（預設狀態，行為應與 910.0 完全相同）
2. flag on + 4 軸 NN 正常流程（預掃發生於 case241、2DID 對碼無錯位）
3. flag on + Index drop alarm（case110→50 retry 期間預掃立即失效）
4. flag on + 掃碼失敗 alarm（Index 停等 case110、恢復正常）
5. flag on + 掃碼中 STOP/RESET（Home 後正常收斂）
6. 3 軸機 flag on（功能完全不啟動）
7. D43 retry / Fix3 / drop mode（`TestZ2_Drop_Offset≠0`）時序不變
8. 量測 UPH 增益、評估 `iNNPreScanStableDelayMS` 現場調整

---

## 8. 修改檔案速查

| 檔案 | 內容 |
|------|------|
| `Config.h` | `bNNShuttlePreScan2DID` / `iNNPreScanStableDelayMS` 欄位 |
| `cConfiguration.cpp` | `ReadLockByFile()` 讀取 / clamp / 種預設值 |
| `atester.h` | `extern bool IsIndexZ1Z2DownStableForPreScan();` |
| `atester_32Site.cpp` | 穩態判斷式、`NNPreScanStableDelay_32` timer、case110 busy guard、case240 武裝 |
| `acarry.cpp` | `IsNNPreScan2DIDCanStart()`、SHT1/SHT2 case10 改道、case210 hold |

修改前備份：`D:\HT9045\backup\20260728_NN_2DID_PreScan\`
