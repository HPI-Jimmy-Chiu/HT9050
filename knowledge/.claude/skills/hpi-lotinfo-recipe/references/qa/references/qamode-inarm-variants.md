> 保存來源：`.claude/skills/ht9045-qamode/references/qamode-inarm-variants.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# QA Mode InArm 變體 / SiteMap / ART 互動 / 程式碼座標

> 本文件為 `ht9045-qamode` SKILL 的補充參考。

## 各 InArm 變體散布的「剩餘 IC → Untest Bin」邏輯

進入 `bQAModeFinishCleanOut==true` 階段後，所有 InArm 變體
（ainarm9045_*.cpp）皆有相同 pattern：

```cpp
if(IniConfig.bQAMode && LastSet.iRunStartMode==rsmQAMode)
{
    if(iQAModeLoaderCT > Prod.iQAModeCount)
    {
        // 把 IC 的 Bin 直接設為 Prod.iQAModeBin（Untest Bin）
    }
}
```

涵蓋的變體（全部 22+ 個 InArm 模式）：
`1x1_1, 1x2_2, 1x2_2_14, 1x2_4, 1x2_4_Hot, 1x3_2_14, 1x3_4, 1x4_2,`
`1x4_4, 1x4_4_Back, 1x4_8_Hot, 2x1_2, 2x2_4, 2x2_4_12, 2x2_4_14,`
`2x2_8_Hot, 2x3_6, 2x3_6_14, 2x4_4, 2x4_8, 2x4_16, 2x5_8, 2x6_8,`
`2x8_8, 2x8_32, S_1x4_4, S_2x4_4_13, All_1Pick`

## SiteMap 備份 / 還原（QABackupStatus）

`QAMode.cpp:QABackupStatus(bool bBackup)`：與
`bQATrayEndCloseYield100Site` / `bQAD22DoubleContact` 配合使用。
- `bBackup=true`：把當下 `bTestSiteUse[2][Y][X]` 與
  `LastSet.bUseTestSocket[2][Y][X]` 序列化為 32-bit BitMask，存入
  `{AuthPath}config.ini` 的 `[Index]` section
- `bBackup=false`：還原 SiteMap，並重置 D22 / TrayEndCloseYield100Site

## 與 Auto Retest / SCK_ART 互動

`tsQASampling->TabVisible = (CUSTOMER_CODE==CC_AMKOR_Korea && bSCKART_EnableART)`
— QA Sampling 分頁僅 ATK 客戶 + ART 啟用才顯示。

## 主要程式碼座標（HT9011UC_Code_V3.33.904.3）

| 檔案 | 行 | 內容 |
|------|----|------|
| `QAMode.cpp` | 全檔 | UI 表單 / 存取檔 / QABackupStatus |
| `ainarm2.cpp` | 155 | `Check_QA_ModeCount()` 三階段判斷 |
| `ainarm2.cpp` | 1622~1645 | InArm 入口呼叫 + Suck NULL_IC 設定 |
| `ainarm9045.cpp` | 2579 | `AddLoadingCount()`（含 `iQAModeLoaderCT++`）|
| `ainarm9045.cpp` | 7078 | `bNeedOneCycle()` QA 加成 |
| `ainarm9045_*.cpp` | 各檔 ~2200/2400 | 進入階段 B 後改 Bin 為 Untest Bin |
| `cinitial.cpp` | 7074 | `Prod.iQAModeCount = TestIF.iQAModeCount` |
| `csystem.cpp` | 10943 | Run Type 3 TrayEnd 重設 |
| `csystem.cpp` | 12004 | Lot Start 歸零 |
| `cmydef.cpp` | 3350 | 全域變數宣告 |
| `main.cpp` | 889, 1267 | OffT 切換（904.3 加 QA Guard）|
| `main.cpp` | 16677 | iBin 覆寫（904.3 擴展 RunType 0）|

## State Record 快速判讀

當客戶回報 QA 異常時，從 state record 抓：
- `DecisionVariables.csv`：`iQAModeLoaderCT`、`bQAModeQuickCleanOut`、
  `bQAModeFinishCleanOut`、`Prod.iQAModeCount`
- `{Recipe}\Tester.Data` [QA Mode] section
- `system\LastSet.ini` 找 `iRunStartMode`（應為 1=rsmQAMode）
- Task_ListWithTime：搜尋 `"QA Mode"` 任務啟動點
- 若 `iQAModeLoaderCT > Prod.iQAModeCount` 但 `bQAModeQuickCleanOut=false`
  → 撞到 `==` 跳過 bug（已在 904.2 修正）

## 測試模式 → InArm Type 對照

```
"32-Site N Mode"  → iInArmType = e9045_2x8_32  → ainarm9045_2x8_8.cpp（共用流程，_2x8_32.cpp 為死碼）
"16-Site N Mode"  → iInArmType = e9045_2x4_16  → ainarm9045_2x4_16.cpp
"8-Site 2x4"      → iInArmType = e9045_2x4_8   → ainarm9045_2x4_8.cpp
```
（完整對應請參考 `ht9045-inarm-flow` SKILL）

<!-- preserved-content:end -->
