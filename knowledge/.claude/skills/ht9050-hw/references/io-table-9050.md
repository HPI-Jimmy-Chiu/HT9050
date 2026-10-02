# HT9050 IO 表

兩個層次，不要混：

| 層次 | 檔案 | 是什麼 |
|------|------|--------|
| **設計面** | `HP-9050機構類元件代碼-*.xls` 分頁 `01_機構資訊`（166 列） | 硬體／電控填的「元件 ↔ 線號 ↔ I/O 點位名」 |
| **執行面** | `D:\HT9045\system\IO_Table_9050.csv`（1062 列，EastSun 20260923 填） | 機台程式真正載入的表；欄位是位址不是線號 |

本 skill `docs/IO_Table_9050.csv` 與 `D:\HT9045\system\` 下的那份**目前內容相同**。
（repo `core.autocrlf=true`，clone 出來的副本換行會是 CRLF，比對用內容不要用 hash。）

---

## 1. `IO_Table_9050.csv` 欄位

15 欄（表頭 14 欄 ＋ 選用的 `Note`）。程式端結構是 `database.h` 的 `TIODATA`，
欄位是**用表頭名字比對**的（`TIOTABLENO::SetIOTableNo`，`AnsiPos` 子字串、最後命中者勝），
所以欄位順序可以動，**名字不能打錯**。

| 欄 | 型別 | 空白時 | 意義 |
|----|------|--------|------|
| `IOType` | 字串 | — | `Sensor` / `Switch` / `Cylinder` / `Cylinder_On` / `Cylinder_Off` / `Sucker` / `Sucker_On` / `Sucker_Off` |
| `Alias` | 字串 | — | 點位名，要對得上 `cmydef.cpp` 的 `const int` 常數名 |
| `Lane` | int | −1 | 車道；`eMotionNet` 時空白會設 `bHasNullData` |
| `ModuleType` | int | −1 | 模組型別 |
| `IP` | int | −1 | 模組位址；`eMotionNet` 時空白會設 `bHasNullData` |
| `Port` | int | −1 | **`ISABase` ∈ {1,2,4} 時以 16 進位解析** |
| `Bit` | int | −1 | 位元 |
| `InType` | int | **0**（不是 −1） | 接點極性 |
| `ISABase` | int | `eMotionNet(0)` | IO 卡種類，見下表 |
| `Enable` | int | 0 | 0 = 這一列不生效 |
| `OnAlarmTime` / `OffAlarmTime` / `OnDelayTime` / `OffDelayTime` | int | −1 | 氣缸到位逾時與延時（ms 級距） |

`ISABase` → `MachineType.h` `enum eIOType`：

| 值 | 列舉 | HT9050 用量 |
|:--:|------|:-----------:|
| 0 | `eMotionNet` | 664 列 |
| 1 | `eISABase` | 0 |
| 2 | `ePCI1735U` | 0 |
| 3 | `ePCI1203` | **398 列** ← PCIe-1203-32A |
| 4 | `ePLCbase` | 0 |

## 2. 統計

| 項目 | 數字 |
|------|------|
| 資料列 | 1062 |
| 不重複 Alias | 905 |
| `Enable=1` | 236 |
| `Sensor` / `Switch` / `Cylinder` | 333 / 141 / 128 |
| `Cylinder_On` + `Cylinder_Off` | 79 + 79 |
| `Sucker` + `Sucker_On` + `Sucker_Off` | 49 + 49 + 49 |
| 比 HT9045 `IO_Table.csv` 多出來的 Alias | **263** |
| HT9045 有、HT9050 沒有的 Alias | 1 |

也就是 **HT9050 的表幾乎是 HT9045 的超集**。
第 895 列有一個標記列 `,#NEW_FROM_9050_DRAWING_20260923,,,,,,0,0,0,,,,`，
之後才是依 9050 圖面新增的段落（Tray arm 分離／抽屜／Multi bin／OTD／Clean panel…）。
這列自己 `Enable=0`，只是分隔用，**不要刪**——刪了就找不到新舊分界。

> 對照一下：HT9045 的 `IO_Table.csv` 是 446 列 `Enable=1`，HT9050 目前只有 236 列。
> 差額不是「9050 比較簡單」，是**還沒開完**。

## 3. 線號編碼（設計面的 `.xls`）

`01_機構資訊` 的兩欄線號：

- **輸入線號**：5 碼，如 `15000`、`11220`、`10231`；小數形式 `15010.0` 表示該點的 bit。
  可能寫成 `15020、15021`（一顆氣缸兩顆磁簧）或 `15000推` / `11220開 11221閉`（動作方向註記）。
- **輸出線號**：5 碼，如 `01104收 01105推`；`~` 表示沿用上一列同一組輸出。

⚠ **軟體版與電控版只差輸出線號的前綴**：29 列的輸出從 `01xxx` 變成 `0Bxxx`
（`01104`→`0B004`、`01302`→`0B202`…）。**輸入線號與所有其他欄位完全相同。**
接線以電控版（20260923）為準，點位命名以軟體版為準。詳見
[source-workbooks.md](source-workbooks.md)。

## 4. 命名來源欄：哪些是新名字

`.xls` 有一欄「命名來源」，把每個 I/O 點位標成 `軟體既有`（48 列）或 `建議新增`（23 列），
其餘 93 列留空。**這一欄經查核是準的**——下列 11 個「建議新增」的名字在
`HT9011UC_Cpp_V3.33.906.0/cmydef.cpp` 裡確實一個都沒有；而 `SnHead5TempOverDetect`、
`SwHeaterRelay`、`SwSafeDoorLock`、`SwBigFan`、`SwIonBarPower`、`SnSafeLock`、
`SnMotorPower` 這些標「軟體既有」的則確實存在。

| 建議新增的點位 | 用途 | 表上給的參考 |
|----------------|------|--------------|
| `C_CleanPanel` | Clean panel 伸縮汽缸 | 比照 `C_` 命名新增 |
| `SnHead7TempOverDetect` / `SnHead8TempOverDetect` | DUT7/8 溫度保險絲 | 軟體僅有 Head1/2/5/6 |
| `SnChamberTempOverDetect` | Chamber 溫度保險絲 | 相似既有 `SnChamberHeatDetect` |
| `SnHotGun1TempOverDetect` / `SnHotGun2TempOverDetect` | 熱風槍溫度保險絲 | 相似既有 `SnHotGun1/2` |
| `SnSLK1TempOverDetect` … `SnSLK8TempOverDetect` | SLK-1~8 溫度保險絲（8 個） | 相似既有 `SnArm1SLK` / `C_SLK1_Clamp` |
| `SwCupLight` | 杯燈開關 | 比照 `Sw` 命名新增 |
| `SnIndexZBreakerButton` / `SwIndexZBreakerLed` | Index Z 煞車按鈕燈（**不是煞車本體**） | 煞車本體是 `SwInArmZBreaker`／`SwOutArmZBreaker` |
| `C_OTD_LeftRight` / `C_OTD_FrontBack` / `C_OTD_Valve` | OTD 左右／前後／電磁閥 | 比照 `C_` 命名新增 |
| `SwCassetteEmptyMotBreaker` | M36 = `MEmptyZ` 煞車 | 軟體 CassetteBreaker 只有 LD/Auto1/Auto2 |
| `SwCassetteAuto3MotBreaker` | M40 = `MAuto3Z` 煞車 | 同上 |

這 23 個名字**已經全部寫進 `IO_Table_9050.csv`**（抽查 7 個，7 個都在）。

### 實際缺口是 93 個，不是 23 個

`.xls` 只涵蓋 166 列，CSV 涵蓋 1062 列。跑 `scripts/check_io_table.py` 實測：

| 指標 | 數字 |
|------|------|
| `cmydef.cpp`（V906 樹）的 `const int` 常數 | 2065 |
| HT9050 表對不到常數的 base name | 106 |
| HT9045 表對不到常數的（既有雜訊，扣掉） | 13 |
| **HT9050 專屬、程式端還沒有常數的點位** | **93** |

（計算時已排除兩類假陽性：`Sucker*` 的名字是 `cinitial.cpp:326` 用
`sprintf("FTestSuck%c%c")` 組出來的，本來就沒有常數；`_On`／`_Off` 是
`cinitial.cpp:4871-4872` 從 `CylinderName` 衍生的，只看 base。）

93 個的組成大致是：Loader/Auto1-3/Empty 的抽屜與分盤機構
（`C_*DrawerLock`／`C_*EdgeClip`／`C_*EdgePush`／`Sn*Drawer*` 共約 40 個）、
防掉落 `C_InPnPDrop1-4`／`C_OutPnPDrop1-4`、OTD 三個、Multi bin／Mobile tray、
以及溫度保險絲 `Sn*TempOverDetect` 一族（SLK 8 個 ＋ Head7/8 ＋ Chamber ＋ HotGun1/2）。
完整清單跑腳本就有，不在這裡列死——**表會改，清單會過期**。

也就是說 **CSV 走在程式碼前面**：表上有名字，`cmydef.cpp` 還沒有對應常數，
`LoadIoData()` 讀進來後也對不到任何 `Sen[]`／`CY[]`／`SW[]` 索引。
要用這些點位就得先在 `cmydef.cpp` 補常數（並注意既有常數區的編號別撞號，
見 [ht9045-array-audit](../../ht9045-array-audit/SKILL.md)）。

## 5. 煞車與電源主開關

`01_機構資訊` 末段（無站別欄）的系統級 IO：

| 輸出 | 點位 | 對應 | 命名來源 |
|------|------|------|----------|
| `01004` | `SwFMotorBreaker` | M14 = `MTestZ1` 測試頭 Z 煞車 | 軟體既有 |
| `01005` | `SwInArmZBreaker` | M3 = `MInArmZA` | 軟體既有 |
| `01006` | `SwOutArmZBreaker` | M22 = `MOutArmZA` | 軟體既有 |
| `01030` | `SwCassetteLDMotBreaker` | M35 = `MLoaderZ` | 軟體既有 |
| `01031` | `SwCassetteEmptyMotBreaker` | M36 = `MEmptyZ` | **建議新增** |
| `01032` | `SwCassetteAuto1MotBreaker` | M38 = `MAuto1Z` | 軟體既有 |
| `01033` | `SwCassetteAuto2MotBreaker` | M39 = `MAuto2Z` | 軟體既有 |
| `01034` | `SwCassetteAuto3MotBreaker` | M40 = `MAuto3Z` | **建議新增** |
| `01035` | `SwCCDZBreaker` | M153 = `MTopAOICCDZ` | 軟體既有 |
| `01021/01022/01023` | `SwTowerRed` / `SwTowerYellow` / `SwTowerGreen` | 三色燈（特規） | 軟體既有 |
| `01024`–`01027` | `SwMusic1`–`SwMusic4` | 音樂盒 | 軟體既有 |
| `00122` | `SwSafeDoorLock` | 所有安全門門栓 | 軟體既有 |
| `00131` | `SwIonBarPower` | 離子 BAR | 軟體既有 |
| `00132` | `SwBigFan` | 大風扇（220V） | 軟體既有 |
| `00136` / 輸入 `10237` | `SwHeaterRelay` | Hotplate 全加熱主開關 | 軟體既有 |
| `00135` / 輸入 `10236` | `SwMotorRelay` / `SnMotorPower` | 伺服馬達電源 | 軟體既有 |
| 輸入 `10235` | `SnSafeLock` | SAFE LOCK 按鈕 | 軟體既有 |
| `00137` | `SwCupLight` | 杯燈 | **建議新增** |

`SwHeaterRelay` 在溫控迴圈裡是硬閘：`bthermo.cpp` 的
`if(SW[SwHeaterRelay].Status()==false) Temp[Addr]=0.0;`（`#ifndef SOFT_SIMULTE` 內）。
主開關沒開，**所有 SV 都被歸零**，不是只有警示。

## 6. ⚠ 執行期路徑寫死

`IoTablePath` 在兩處硬寫成 `D:\HT9045\System\IO_Table.csv`：

- `common.cpp:232`（初始值）
- `database.cpp:1698`（`LoadIoData()` 之前又設一次）

**沒有任何依機種切換的邏輯**，所以放在 `system\IO_Table_9050.csv` 的表現在不會被讀到。
要讓 HT9050 真的吃到這張表，得先改路徑選擇（依 `MachineType` 或 `Gerneral.ini`），
別用「複製蓋掉 `IO_Table.csv`」了事——那會讓 HT9045 的表消失，而且沒有任何警告。

相關：[ht9045-io-control](../../ht9045-io-control/SKILL.md)（`TMyCylinder`／`TMySensor`／
`TMySucker` 怎麼吃這張表）、[ht9045-array-audit](../../ht9045-array-audit/SKILL.md)（加常數時的越界稽核）。
