> 保存來源：`.claude/skills/ht9045-load-y-use-motor/SKILL.md`，main `9d9dfa9c7`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->

# HT9045 LOAD_Y_USE_MOTOR Y 軸步進馬達知識庫

> 適用版本：`HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（V899）。

## 1. 變數定位

### 1.1 宣告

| 項目 | 檔案/行 | 內容 |
|------|---------|------|
| 全域宣告 | [cmydef.cpp](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/cmydef.cpp#L3038) | `bool LOAD_Y_USE_MOTOR[9];` |
| extern | [cmydef.h](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/cmydef.h#L2789) | `extern bool LOAD_Y_USE_MOTOR[9];` |
| 起源 | Jimmychiu 20240307 | Loader Tray 改用步進馬達 |
| 後續擴充 | 20260325 Jimmychiu | 改為「Y 軸步進馬達無特殊功能專用」分支 |

### 1.2 索引語意（與 `MAX_TRACK = 9` 軌道對齊）

陣列大小宣告為 9，但目前實際使用 0~5 共 6 條軌道：

| index | 軌道 | 對應步進馬達（`iStepMotor[6]`） | 狀態 |
|-------|------|-------------------------------|------|
| `[0]` | Loader   | `MLoaderY` | **已實作** |
| `[1]` | Empty    | `MEmptyY`  | 預設強制 false（未實作） |
| `[2]` | Color    | `MColorY`  | 預設強制 false（未實作） |
| `[3]` | Auto1    | `MAuto1Y`  | 預設強制 false（未實作） |
| `[4]` | Auto2    | `MAuto2Y`  | 預設強制 false（未實作） |
| `[5]` | Auto3    | `MAuto3Y`  | 預設強制 false（未實作） |
| `[6]~[8]` | Auto4/5/6 | — | 未定義馬達，無使用 |

對應步進馬達清單在 [asendic.cpp L1438](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1438)：
```cpp
int iStepMotor[6]={MLoaderY, MEmptyY, MColorY, MAuto1Y, MAuto2Y, MAuto3Y};
```

## 2. 設定來源（INI）

INI 檔案：`d:\HT9045\system\Gerneral.ini`，section = `[TrayY]`

| INI Key | 對應 | 讀取位置 | 寫入位置 |
|---------|------|----------|----------|
| `LOAD_Y_USE_MOTOR`  | `LOAD_Y_USE_MOTOR[0]` Loader | [database.cpp L759](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/database.cpp#L759) | [HandlerSys.cpp L479](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L479) |
| `EMPTY_Y_USE_MOTOR` | `[1]` Empty | **未讀**（強制 false） | [HandlerSys.cpp L480](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L480) 寫死 false |
| `COLOR_Y_USE_MOTOR` | `[2]` Color | **未讀** | L481 寫死 false |
| `AUTO1_Y_USE_MOTOR` | `[3]` Auto1 | **未讀** | L482 寫死 false |
| `AUTO2_Y_USE_MOTOR` | `[4]` Auto2 | **未讀** | L483 寫死 false |
| `AUTO3_Y_USE_MOTOR` | `[5]` Auto3 | **未讀** | L484 寫死 false |

### 2.1 [0] 載入規則
[database.cpp L759](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/database.cpp#L759)：
```cpp
LOAD_Y_USE_MOTOR[0] = (CheckAndReadIniDataGeneral("TrayY", "LOAD_Y_USE_MOTOR", false)
                       || (INSTALL_OCR > eocrUninstal));
```
意義：勾選 `Loader Tray` 或安裝 OCR 時，Loader 軌道使用 Y 軸步進馬達。

## 3. UI 面板（HandlerSys）

| 項目 | 位置 |
|------|------|
| GroupBox | `grpTrayYUseMot`（Caption：`Tray Y Use Motor`），[HandlerSys.dfm L1267](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.dfm#L1267) |
| 既有勾選 | `chkLoaderY`（Caption：`Loader Track`），[HandlerSys.dfm L1274](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.dfm#L1274) |
| Load 設定 | [HandlerSys.cpp L100](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L100) `chkLoaderY->Checked = LOAD_Y_USE_MOTOR[0]` |
| Save 設定 | [HandlerSys.cpp L479](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L479)、L503 |

GroupBox 高度 49（單行）：補上 5 個 CheckBox 後須加大 Height（建議 ? 145，見計畫）。

## 4. 行為邏輯

### 4.1 進料：`TrayMoveIn(bool bMove, int iAxis, int iPos)`
[asendic.cpp L1265](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1265)。優先序（自上而下）：

1. `TRAY_ARM_MODE == eUnderCoveyor` → 先打開 floodgate 氣缸
2. **Loader OCR 條件**（僅 iAxis==0）：`INSTALL_OCR != eocrUninstal && CosFunction.bTrayOCR` 或 `INSTALL_OCR_YMot == eocrYMotInstal` → `MOT[MLoaderY].MotorMove(iPos)`（用 `iPos` 走精準位置）
3. **`LOAD_Y_USE_MOTOR[iAxis] == true`**（[L1288](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1288)）→ 走「無特殊功能專用」分支：
   ```cpp
   MOT[MLoaderY].SetSpeed(100);
   if (MOT[MLoaderY].ReadPos() >= 69900)
       MOT[MLoaderY].PCIL132_SetPos(0);
   MOT[MLoaderY].MotorMove(70000);
   ```
   **目前 hardcode `MLoaderY`**：擴充 [1]~[5] 時必須改用 `MOT[iStepMotor[iAxis]]`。
4. `LoaderUnload_StepMotor == 1`：用 IO Switch（`SW[iInSwitch[iAxis]] / SW[iOutSwitch[iAxis]]`）控制步進，反向需 `MySleepEx(100)` 緩衝
5. 否則：傳統氣缸 IO（iAxis<3 與 iAxis>=3 行為不同）

`bMove == false` 停止：iAxis==0 且符合 OCR / `LOAD_Y_USE_MOTOR[0]` → `MOT[MLoaderY].PCIL132_StopMotor()`；其他軌道僅關 IO。**[1]~[5] 的步進停止亦需擴充**。

### 4.2 出料：`TrayMoveOut(bool bMove, int iAxis, int iPos)`
[asendic.cpp L1358](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1358)。結構與 `TrayMoveIn` 鏡像，差別：

| 動作 | In | Out |
|------|----|-----|
| 步進目標 | `+70000`（前往 In） | `-70000`（前往 Out） |
| 邊界 reset | `ReadPos() >= 69900 → SetPos(0)` | `ReadPos() <= -69900 → SetPos(0)` |
| 氣缸 SW 組合 | iAxis<3：In On / Out Off；iAxis>=3：In On / Out On | iAxis<3：In On / Out On；iAxis>=3：In Off / Out On |

同樣 hardcode `MLoaderY`，擴充時要改成 `iStepMotor[iAxis]`。

### 4.3 狀態判讀：`TrayMoveStatus(int iAxis, AnsiString sFun)`
[asendic.cpp L1435](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1435)。

```cpp
int iStepMotor[6]={MLoaderY, MEmptyY, MColorY, MAuto1Y, MAuto2Y, MAuto3Y};
if (LOAD_Y_USE_MOTOR[iAxis] == true) {
    bool bRun = MOT[iStepMotor[iAxis]].Led[iInposLed];
    if (bRun) {
        int pos = MOT[iStepMotor[iAxis]].ReadPos();
        bInSwitch  = (pos > 0);
        bOutSwitch = (pos < 0);
    }
} else {
    bInSwitch  = SW[iInSwitch[iAxis]].Status();
    bOutSwitch = SW[iOutSwitch[iAxis]].Status();
}
```

回傳：`0=停止`, `1=Move In`, `2=Move Out`, 異常 → `ShowMyMessage` 並強制停。
此函式**已支援 [0]~[5] 全 6 軌**，不需動。

### 4.4 馬達測試列表：`uMotorTest.cpp`
[uMotorTest.cpp L200](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/uMotorTest.cpp#L200) `MLoaderY` 啟用條件：
```cpp
((INSTALL_OCR_YMot==eocrYMotInstal && CosFunction.bTrayOCR) ||
 USE_LdUldCassetteMode==1 ||
 LOAD_Y_USE_MOTOR[0]==true)
```
其他 Y 軸目前：
- `MEmptyY`：固定 `false`
- `MColorY`：固定 `false`
- `MAuto1Y` / `MAuto2Y`：`USE_LdUldCassetteMode==1`（cassette mode 才開）
- `MAuto3Y`：固定 `false`

擴充時須補上 `LOAD_Y_USE_MOTOR[n]==true` 的 OR 條件。

## 5. 與其他相近旗標的關係

| 旗標 | 範圍 | 與 LOAD_Y_USE_MOTOR 關係 |
|------|------|-------------------------|
| `LOAD_Z_USE_MOTOR[0..8]` | 9 軌 Z 軸（升降） | 不同軸；UI 上分 GroupBox |
| `INSTALL_OCR_YMot`（`eocrYMotInstal`） | 僅 Loader Y | OCR 專用 Y 馬達；優先序高於 `LOAD_Y_USE_MOTOR[0]` |
| `INSTALL_OCR` | OCR 安裝模式 | `>eocrUninstal` 時自動啟用 `LOAD_Y_USE_MOTOR[0]` |
| `LoaderUnload_StepMotor` | 0/1 | 步進但用 IO Switch 觸發；優先序低於 `LOAD_Y_USE_MOTOR` |
| `USE_LdUldCassetteMode` | 0/1 | Boat Carrier；會獨立啟用 `MAuto1Y/MAuto2Y` |
| `LOADUNLOAD_USE_CASSETTE[0..8]` | 9 軌 cassette | 與本旗標互不衝突，可同時 |

## 6. 影響函式總覽

| 檔案 | 函式 | 用途 | 是否需擴充 [1]~[5] |
|------|------|------|------------------|
| [database.cpp L759-764](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/database.cpp#L759) | `ReadGeneralIni` | 從 INI 載入 | **是**（補 [1]~[5] 讀取） |
| [HandlerSys.cpp L100](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L100) | `LoaderSystemSet` | UI Load | **是**（顯示 5 個 CheckBox） |
| [HandlerSys.cpp L479-484](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L479) | `SaveSystemSet` | UI Save → INI | **是**（寫入勾選值） |
| [HandlerSys.cpp L503-508](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.cpp#L503) | `SaveSystemSet` | 更新全域 | **是**（用勾選值） |
| [HandlerSys.dfm L1267](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HandlerSys.dfm#L1267) | `grpTrayYUseMot` | 表單 | **是**（增加 5 個 TCheckBox） |
| [asendic.cpp L1265-1336](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1265) | `TrayMoveIn` | 進料 | **是**（hardcode `MLoaderY` → `iStepMotor[iAxis]`） |
| [asendic.cpp L1358-1432](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1358) | `TrayMoveOut` | 出料 | **是**（同上） |
| [asendic.cpp L1435-1490](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/asendic.cpp#L1435) | `TrayMoveStatus` | 狀態 | 否（已用 `iStepMotor[]`） |
| [uMotorTest.cpp L200-205](../../../../../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/uMotorTest.cpp#L200) | 馬達測試列表 | 開放手動測試 | **是**（其他 Y 馬達的啟用條件加 `LOAD_Y_USE_MOTOR[n]`） |

## 7. 風險與注意事項

| 風險 | 說明 | 緩解 |
|------|------|------|
| Big5 編碼 | 所有檔含中文註解，禁止整檔重存為 UTF-8 | 採最小行修改（multi_replace_string_in_file） |
| `.dfm` 格式 | Delphi 格式敏感，欄位順序、`object`/`end` 配對嚴格 | 修改前先讀大段 context，必要時 BCB6 IDE 預覽 |
| `MAuto1Y` 雙來源 | Cassette mode 已使用 `MAuto1Y/MAuto2Y` | 啟用 `LOAD_Y_USE_MOTOR[3]/[4]` 時須驗證不與 cassette 衝突；建議互斥檢查 |
| Hardcode 邊界 | `±70000`、`69900` 為 Loader Y 行程；其他軌道 Y 行程可能不同 | 若 Empty/Color/Auto 行程不同，需對應參數化（Tech / IniData） |
| `INSTALL_OCR_YMot` 優先序 | OCR 模式僅針對 iAxis==0；不影響 [1]~[5] | 擴充時保留 iAxis==0 OCR 分支不動 |
| 反向延遲 | 既有 `MySleepEx(100)` 在 IO 模式；步進馬達分支未做 | 若實際機構需反向延遲，新增 `if pos 反向 → 短停` |
| 安全互鎖 | Y 軸動作須確認 Z 在安全位置、抓盤手臂淨空 | 跟 `CatchTray`、`LOAD_Z_USE_MOTOR` 流程一併檢查 |

## 8. 驗證建議

| 層級 | 動作 |
|------|------|
| 編譯 | 跑 `BCB6: Build V899` task；確認 `.dfm` 無載入錯誤 |
| 設定 | 在 HandlerSys 勾選新 CheckBox → Save → 開啟 `system\Gerneral.ini` 確認新 key 寫入 |
| 馬達測試 | uMotorTest 介面確認對應 Y 馬達可手動正反轉、回 0 |
| 流程 | CatchTray 從 Loader/Empty/Color/Auto1~3 進出料；確認 `TrayMoveStatus` 回傳 1/2 正確 |
| 互斥 | `LOAD_Y_USE_MOTOR[3]=true` 且 `USE_LdUldCassetteMode=1` 時的競爭情境 |

<!-- preserved-content:end -->
