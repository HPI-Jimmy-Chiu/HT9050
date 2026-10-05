# ATC Multi Zone — EnablesChannel 通道未啟用問題

**確認日期：** 2026-04-21  
**案例來源：** ATK（971_AMKOR_Korea）PPLS557，HT-9132 / HT-9046LS + ATC 3.5 V1.2.1  
**問題編號：** P260421-ATK-H9-01

---

## 症狀

- Handler 連線 ATC，啟用 `cbMultiZoneFunction`，執行 Lot Start 後：
  - ATC Monitor 只顯示 **CH5** 加熱（Single Site socket 對映的唯一通道），CH6–CH8 = NA
  - Handler ATC Panel：CH5 = 84.56°C（紅），CH6–CH8 = NA
- ATC **Offline（單機執行）** 時 CH5–CH8 全部正常運作（ATC 自身 recipe `51_90dC` 正確）。

---

## 環境資訊

| Key | 來源檔案 | 值 |
|-----|----------|----|
| 客戶機台版本 | `Ver.txt` | **V3.21.895.2**（SVN r895，缺少修復） |
| Handler `Ver` | `Gerneral.ini [Version]` | V3.21 |
| `iATC_MODE_TYPE` | `system/ATC.ini [System]` | 35（ATC 3.5/3.6 共用，正常） |
| ATC SW 版本 | ATC 1031 指令回傳 | 1.2.1 |
| `ATC_SYSTEM` (`USE_ATC_MODE`) | `Gerneral.ini [ATC]` | **6 = `eNewATCSystem`** |
| `ATC_SYSTEM_USEHEAT` | `Gerneral.ini [ATC]` | **8** → `iATC_Use_Heat_Count = 8` |
| Test Mode | `system/current_SiteMap.txt` | SingleSite（r:1/c:1）|
| `Multi Zone Enable` | `Temperature.Data` | **1（true）** |
| `Zone Temp 1~4 Use` | `Temperature.Data` | **全部 1（true）** |
| `Zone Temp 1~4 Setting` | `Temperature.Data` | 25.0°C |

---

## 根本原因

### ATC 通訊 Log 實證（2026-04-20）

來自 State Record 的 ATC 通訊 log 直接證實 Handler→ATC 指令層面的問題：

**指令 1004（EnablesChannel / ATC_SITE_ENABLED）：**
```
[Send->] @1004,8,0,0,0,0,1,0,0,0,#     ← 整個 log 期間（09:07~13:39）全部一致
                     ↑
         CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8
          0   0   0   0   1   0   0   0      ← 只有 CH5 enabled
```
- Multi Zone 預期：`@1004,8,0,0,0,0,1,1,1,1,#`（CH5–CH8 全部 enabled）
- **全日 log 中不存在 CH6/7/8 = 1 的記錄。**

**指令 1002（SetAllTemp）：**
```
[Send->] @1002,8,1000,1000,1000,1000,1000,1000,1000,1000,#   ← 100°C 統一
[Send->] @1002,8,250,250,250,250,250,250,250,250,#           ← 25°C 統一
```
- Multi Zone 預期各區溫度不同（依 `dZoneTempSetting[0..3]`），實際 8 CH 均相同
- 代表 `SetMultiZoneTemp()` 未被呼叫，或其效果被 `SetAllTemp()` 覆蓋

**指令 1010（ReadTemp 回傳）：**
```
[<-Read] @1010,17,9999,9999,9999,9999,9999,9999,9999,9999,290.6,9999,...
```
- 只有 index 8（TC 溫度）有值（~29°C / ~100°C），其餘 9999 = 無效/disabled

**指令 1025（Read enabled status 回傳）：**
```
[<-Read] @1025,18,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,1,1#
```
- ATC 確認接收到的 enable 設定：只有位置 5 = 1（CH5），CH6–CH8 = 0

### 資料流

```
main.cpp — ATC channel enable 區段（L13235）
  ↓
方案 A: SingleSite + MultiZone 分支（wei 20240617）
  bUse[0..3] = bZoneTempEnable[0..3]          ← Index 1
  bUse[4..7] = bZoneTempEnable[0..3]          ← Index 2 (iATC_Use_Heat_Count/2 = 4)
  → 理論上 CH5–CH8 應該被 enable

方案 B: 其他分支（如 ATC_SYSTEM >= eATC60）
  bUse[iSiteToATC[arm][row][col]] = bTestSiteUse
  → SingleSite 只有 1 個 socket → bUse[4]=true → CH5 only

問題：ATC log 證實走入方案 B，未命中方案 A
  ↓
ATC_InterfaceForm->EnablesChannel(8, bUse)
  → ATC 收到：@1004,8,0,0,0,0,1,0,0,0,#
  ↓
uLotInfo.cpp case 7
  if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)
    → SetMultiZoneTemp() 應該被呼叫
    → 但 ATC log 顯示 @1002 為統一溫度值 → 被 SetAllTemp() 覆蓋或未呼叫
```

### 兩層問題彙整

| # | 指令 | ATC Log 實測 | Multi Zone 預期 | 結論 |
|---|------|-------------|----------------|------|
| 1 | `@1004`（EnablesChannel） | `0,0,0,0,1,0,0,0` | `0,0,0,0,1,1,1,1` | **bUse[] MultiZone 展開未命中** |
| 2 | `@1002`（SetAllTemp） | `1000,1000,...,1000`（統一） | 各區獨立值 | **SetMultiZoneTemp 未生效** |

### bUT150Install（顯示層）vs bUse[]（指令層）

`Index16Heater()`（main.cpp L18107）中 **有** Multi Zone 展開：
```cpp
if(Temperature.bMultiZoneEnable) {
    bUT150Install[tcAa1] = bZoneTempEnable[0];   // Zone A
    bUT150Install[tcAb1] = bZoneTempEnable[1];   // Zone B
    bUT150Install[tcAc1] = bZoneTempEnable[2];   // Zone C
    bUT150Install[tcAd1] = bZoneTempEnable[3];   // Zone D
}
```
此路徑影響 **Handler 端溫度顯示面板**，不影響 ATC 指令。

`bUse[]` 建構（main.cpp L13235）中也 **有** Multi Zone 展開（wei 20240617），但此段落是否被 ATK 機台執行到，取決於進入哪個 if-else 分支。

**ATC log 直接證明 `bUse[]` 展開沒有被正確執行。**

### 關鍵程式碼位置

| 檔案 | 行為 |
|------|------|
| `main.cpp`（`DoSetChannelEnable` 附近） | 建構 `bUse[]`，SingleSite 只啟用 1 個通道 |
| `ATC/ATC_Handler_Side.cpp:1896` | `SetMultiZoneTemp()` — 送溫度，但先決條件是 channel 已 enabled |
| `uLotInfo.cpp:8679` | `SetMultiZoneTemp()` 呼叫點，在 case 7 溫度校核流程中 |
| `cTemperFrom.cpp:922` | 溫度顯示邏輯中亦有 Multi Zone 分支，但此處只影響 UI，不影響 ATC 指令 |

---

## 版本確認（關鍵發現）

### SVN 版本對照

| 版本 | SVN Revision | 日期 | `bUse[]` MultiZone 展開碼 |
|------|-------------|------|--------------------------|
| **V3.21.895.2（客戶）** | **r895** | 2026-01-16 | **❌ 不存在** |
| V3.33.896.0 | r896 | **2026-02-04** | **✅ 加入（Jerryyang）** |
| V3.33.903.0 | r903 | 2026-04-17 | ✅ 存在 |

### 驗證方式

```powershell
# r895 — 查無結果（空輸出）
svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/main.cpp" -r 895 |
  grep "iTestMode==SingleSite && Temperature.bMultiZoneEnable"
# → 無輸出

# r896 — 有結果
svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/main.cpp" -r 896 |
  grep "iTestMode==SingleSite && Temperature.bMultiZoneEnable"
# → else if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)  //wei 20240617 Multi Zone
```

### r896 加入的程式碼（svn diff r895:896 main.cpp）

```cpp
// 新增：bUse[] MultiZone 展開（EnablesChannel 路徑）
+else if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)  //wei 20240617 Multi Zone
+    for(int k=0; k<4; k++)
+        if(Temperature.bZoneTempEnable[k])
+            bUse[k]=true;         // Index 1: bUse[0..3]
+        else bUse[k]=false;
+    for(int k=0; k<4; k++)
+        if(Temperature.bZoneTempEnable[k])
+            bUse[k+4]=true;       // Index 2: bUse[4..7]
+        else bUse[k+4]=false;

// 新增：bUT150Install MultiZone 展開（顯示層）
+if(Temperature.bMultiZoneEnable)  //wei 20240617 Multi Zone
+    bUT150Install[tcAa1]=Temperature.bZoneTempEnable[0]; ...
```

### ATK 機台在 r896+ 的預期行為

| 條件 | 值 |
|------|-----|
| `ATC_SYSTEM` | `eNewATCSystem`(6) → 進入 `else if(ATC_SYSTEM==eNewATCSystem)` ✓ |
| `iATC_Use_Heat_Count/2` | 8/2 = 4 → Index 2 基底 offset = 4 |
| `bTestSiteUse[1][0][0]`（後臂） | true（SingleSite 後臂測試） |
| `bZoneTempEnable[0..3]` | 全 true（Temperature.Data Zone 1~4 Use=1）|
| 結果 `bUse[4..7]` | **全 true → EnablesChannel 送出 CH5~CH8 全開** |

→ `@1004,8,0,0,0,0,1,1,1,1,#` ← 升版後預期指令

---

## 結論：版本問題，非新 Bug

**客戶的 V3.21.895.2（r895）缺少 r896 加入的 MultiZone EnablesChannel 修復。**  
ATC 通訊 log 中的 `@1004,8,0,0,0,0,1,0,0,0,#` 完全符合 r895 程式行為。  
**修復方式：提供客戶包含 r896+ 更新的新版 release。**

---

## 修改方向（已在 r896 解決，無需額外修改）

---

## 測試驗收項目

| # | 情境 | 預期 |
|---|------|------|
| 1 | SingleSite + MultiZone Enable | CH5、CH6、CH7、CH8 均 Enable；各自收到對應 Zone 溫度 |
| 2 | SingleSite + MultiZone Disable | 行為不變（只 CH5 Enable） |
| 3 | 其他 Test Mode（DualSite / QualSite 等） | 不受影響 |

---

## ATC 3.5 / 3.6 型號說明（補充）

- ATC 3.6 硬體安裝 ATC 3.5 V1.2.1 軟體，`iATC_MODE_TYPE` 回傳 **35**，屬正常。
- `Config\ATC.ini [System] iATC_MODE_TYPE=35` 對 ATC 3.6 機台**不需要修改**。
- ATC 3.5 軟體同時支援 3.5 與 3.6 硬體，Multi Zone 功能在兩個硬體版本上邏輯相同。
