> 保存來源：`.claude/skills/ht9045-qamode/references/qamode-ui-fields.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# QA Mode UI 與 Tester.Data 欄位詳細對照

> 本文件為 `ht9045-qamode` SKILL 的補充參考，僅在需要查詢 UI 欄位細節時載入。

## UI 表單

`TfQAMode`（QAMode.cpp / QAMode.dfm），主畫面亦有 `fLotInfo->edQAMode`。

存檔路徑：`{DataPath}{LastOpenFN}\Tester.Data`，分兩個 INI section。

## [QA Mode]

| Key | TestIF_File 變數 | 預設 | 範圍 | 說明 |
|-----|------------------|------|------|------|
| `Count` | `iQAModeCount` | 200 | 5~10000 | QA 抽測顆數 N |
| `Tray Direct` | `iQATrayDirect` | TrayForm.Loader.Direction | 0~7 | Tray 方向圖示 |
| `Run Type` | `iQAModeRunType` | 0 | 0~3 | 做完後動作（見 SKILL §4） |
| `Untest Bin` | `iQAModeBin` | iTestBinCount-1（ATK=1） | 0~iTestBinCount-1 | QA 結束後剩餘 IC 的 Bin（0-based）|
| `bQAModeAfterTrayEnd` | `bQAModeAfterTrayEnd` | false | bool | TrayEnd 後再重做一次 QA |
| `bQAD22DoubleContact` | `bQAD22DoubleContact` | false | bool | 整合到 QA 的 D22 多重下壓 |
| `iQAD22DoubleContactCount` | `iQAD22DoubleContactCount` | 1 | int | D22 下壓次數 |
| `bQATrayEndCloseYield100Site` | `bQATrayEndCloseYield100Site` | false | bool | TrayEnd 把 Yield 100% 的 Site 關閉 |

## [QA Sampling]（僅 `CC_AMKOR_Korea && bSCKART_EnableART` 才顯示）

| Key | TestIF_File 變數 | 預設 | 說明 |
|-----|------------------|------|------|
| `Enable` | `bEnableQASampling` | false | 啟用抽樣 |
| `Bin` | `iQASamplingBin` | 1 | 抽樣放置 Bin |
| `Tray Name` | `sQASamplingTray` | "Fix1" | 抽樣放置的 Auto Tray 名稱 |
| `Count` | `iQASamplingCnt` | 100 | 抽樣顆數 |

## 變數層級對應

存檔 / 載入 / 執行三層變數命名一致，請勿混用：

```
UI (TEdit/TComboBox) ─DoFormToData→ TestIF_File.iQAModeCount        // 表單暫存
                                       │
                                  ReadFile()  WriteIniData()
                                       │
                              Tester.Data INI 檔
                                       │
                                  ReadIniData()
                                       │
                                  TestIF.iQAModeCount               // 執行緒讀檔暫存
                                       │
                              cinitial.cpp Initial Start
                                       │
                                  Prod.iQAModeCount                 // 量產執行用
```

**所有 QA 流程判斷一律使用 `Prod.iQAModeCount` / `Prod.iQAModeRunType` /
`Prod.iQAModeBin` / `Prod.bQAModeAfterTrayEnd`**（cinitial.cpp:7074~ 賦值）。

## 客戶鎖死規則

- `CC_SCS` → `iQAModeRunType=1` 鎖死
- `CC_SIGURD_PeiXing` → `iQAModeRunType=3` 鎖死、`edQAMode/rgQARunMode/cbQAModeBin/cbTrayEndDoQAModeAgain` Enabled=false
- `CC_AnalogDevice_Phil` → `bQAModeAfterTrayEnd=false`、隱藏 `cbQAModeBin/cbTrayEndDoQAModeAgain`
- `CosFunction.bQAModeUseUnloadCnt`（Maxim）→ `rgQARunMode` Enabled=false、隱藏 `cbQAModeBin/cbTrayEndDoQAModeAgain`

<!-- preserved-content:end -->
