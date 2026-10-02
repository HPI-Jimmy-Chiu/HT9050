# `ShowErrorUnit()` 79 分支 —— Pos → FlushPanel 權威對照

//Steven 20260924

本檔由腳本自 golden 機械抽取，**不要手改**；改了要連同產生腳本一起改。

| 項目 | 值 |
|---|---|
| golden | `HT9011UC_Code_V3.33.912.0_20260908_Jimmy` |
| 函式 | `void ShowErrorUnit(int Pos)`　`note.cpp:4551-4780` |
| 分支 | 79（含最後一個 `else`）|
| 預設 | `palSys` —— `note.cpp:4776-4779` 的 `else { FlushPanel=fNote->palSys; }` |
| 閃爍 | `note.cpp:3418-3425`　`Timer` 內 `FlushPanel->Color = FlushFlag ? clRed : cDark` |
| web 對應 | `dialog-page.js:75`　`$(d.flushPanel).classList.add("dbFlush")` |

> ⚠ **V906 移植樹沒有 `ShowErrorUnit`**（`forms/fNote.h:468-470` 明記全樹零命中），
> 所以 `Alarm-dialog-request.json` 的 `display.flushPanel` 恆為 `null`。
> 但 `arguments.position` 是活的 —— `tools/wb_serve.cpp` 的 `DialogMailboxPostAlarm()`
> 已經在寫 `"position":%d`。**panel 可以在前端用本表查出來，C++ 不必改。**

## 欄位說明

| 欄 | 意思 |
|---|---|
| `kind` | `mech` 機構／`tray` 料盤軌／`safety` 安全門／`facility` 廠務檢測／`system` 系統通訊 |
| `Panel5 y` | 該 panel 在 `Panel5`（947×531）內的相對 y；`外` = 不在 Panel5 裡 |
| `遮蔽` | 內嵌 Motion View 取 Panel5 y `0..355` 時，此 panel 會不會被蓋住 |
| `mv9045` / `mv9050` | Motion View 模組 id；`—` 表示該機種畫面上沒有對應物 |

## kind = `mech`（機構）

| # | Pos（unit 常數） | 值 | panel | Panel5 y | 遮蔽 | mv9045 | mv9050 |
|--:|---|--:|---|--:|---|---|---|
| 1 | `MInArmX` .. `MInArmZH`（11 軸） | 0–10 | `palInArm` | 208 | 蓋住 | `InArm` | `InPP` |
| 2 | `MInShuttle1` | 11 | `palInSh1` | 138 | 蓋住 | `InShuttle1` | `InShuttle` |
| 3 | `MInShuttle2` | 12 | `palInSh2` | 8 | 蓋住 | `InShuttle2` | — |
| 4 | `MTestY1` / `MTestZ1` | 13 / 14 | `palHead0` | 95 | 蓋住 | `Index1` | `Index` |
| 5 | `MTestY2` / `MTestZ2` | 16 / 15 | `palHead1` | 66 | 蓋住 | `Index2` | — |
| 6 | `MOutShuttle1` | 17 | `palOutSh1` | 138 | 蓋住 | `OutShuttle1` | `OutShuttle` |
| 7 | `MOutShuttle2` | 18 | `palOutSh2` | 8 | 蓋住 | `OutShuttle2` | — |
| 8 | `MOutArmX` .. `MOutArmZH`（11 軸） | 19–29 | `palOutArm` | 208 | 蓋住 | `OutArm` | `OutPP` |
| 9 | `MTrayX` | 30 | `palTrayArm` | 247 | 蓋住 | `TrayArm` | — |
| 10 | `MMPlate1` | 169 | `palPlate1` | 235 | 蓋住 | `HotPlate1` | `HotPlate` |
| 11 | `MMPlate2` | 170 | `palPlate2` | 85 | 蓋住 | `HotPlate2` | — |
| 12 | `MMInShuttle` | 501 | `palInSh` | 4 | 蓋住 | `InShuttle` | `InShuttle` |
| 13 | `MMOutShuttle` | 502 | `palOutSh` | 4 | 蓋住 | `OutShuttle` | `OutShuttle` |
| 14 | `MMIndex` | 503 | `palHead` | 60 | 蓋住 | `Index` | `Index` |

## kind = `tray`（料盤軌）

| # | Pos（unit 常數） | 值 | panel | Panel5 y | 遮蔽 | mv9045 | mv9050 |
|--:|---|--:|---|--:|---|---|---|
| 15 | `MMTrayZ` / `MMTrayY_Car` | 245 / 168 | `palLoad_Car` | 405 | 不蓋 | `Loader` | `Loader` |
| 16 | `MMEmptyZ` / `MMEmpty_Car` | 246 / 179 | `palEmpty_Car` | 405 | 不蓋 | `Empty` | `Empty` |
| 17 | `MMEmpty1_Car` | 182 | `palColor_Car` | 405 | 不蓋 | `Color` | — |
| 18 | `MMColorZ` / `MMColor_Car` | 247 / 180 | `palColor_Car` | 405 | 不蓋 | `Color` | — |
| 19 | `MMAuto1Z` / `MMAuto1_Car` | 248 / 174 | `palAuto1_Car` | 405 | 不蓋 | `Auto1` | `Auto1` |
| 20 | `MMAuto2Z` / `MMAuto2_Car` | 249 / 175 | `palAuto2_Car` | 405 | 不蓋 | `Auto2` | `Auto2` |
| 21 | `MMAuto3Z` / `MMAuto3_Car` | 250 / 176 | `palAuto3_Car` | 405 | 不蓋 | `Auto3` | `Auto3` |
| 22 | `MMAuto4Z` / `MMAuto4_Car` | 251 / 260 | `palAuto4_Car` | 405 | 不蓋 | `Auto4` | — |
| 23 | `MMAuto5Z` / `MMAuto5_Car` | 252 / 261 | `palAuto5_Car` | 405 | 不蓋 | `Auto5` | — |
| 24 | `MMAuto6Z` / `MMAuto6_Car` | 253 / 262 | `palAuto6_Car` | 405 | 不蓋 | `Auto6` | — |
| 25 | `MManualTray1` | 164 | `palFix1` | 83 | 蓋住 | `Fix1` | — |
| 26 | `MManualTray2` | 165 | `palFix2` | 83 | 蓋住 | `Fix2` | — |
| 27 | `MManualTray3` | 166 | `palFix3` | 83 | 蓋住 | `Fix3` | — |
| 28 | `MManualTray4` | 254 | `palFix4` | 83 | 蓋住 | `Fix4` | — |
| 29 | `MManualTray5` | 255 | `palFix5` | 83 | 蓋住 | `Fix5` | — |
| 30 | `MManualTray6` | 256 | `palFix6` | 83 | 蓋住 | `Fix6` | — |
| 31 | `MManualTrayAll` | 569 | `palManualAll` | 78 | 蓋住 | `FixAll` | — |
| 32 | `MMTrayY` | 167 | `palLoad` | 235 | 蓋住 | `Loader` | `Loader` |
| 33 | `MMAuto1` | 171 | `palAuto1` | 235 | 蓋住 | `Auto1` | `Auto1` |
| 34 | `MMAuto2` | 172 | `palAuto2` | 235 | 蓋住 | `Auto2` | `Auto2` |
| 35 | `MMAuto3` | 173 | `palAuto3` | 235 | 蓋住 | `Auto3` | `Auto3` |
| 36 | `MMAuto4` | 257 | `palAuto4` | 235 | 蓋住 | `Auto4` | — |
| 37 | `MMAuto5` | 258 | `palAuto5` | 235 | 蓋住 | `Auto5` | — |
| 38 | `MMAuto6` | 259 | `palAuto6` | 235 | 蓋住 | `Auto6` | — |
| 39 | `MMEmpty` | 177 | `palEmpty` | 235 | 蓋住 | `Empty` | `Empty` |
| 40 | `MMColor` | 178 | `palColor` | 235 | 蓋住 | `Color` | — |
| 41 | `MMEmpty1` | 181 | `palColor` | 235 | 蓋住 | `Color` | — |

## kind = `safety`（安全門）

| # | Pos（unit 常數） | 值 | panel | Panel5 y | 遮蔽 | mv9045 | mv9050 |
|--:|---|--:|---|--:|---|---|---|
| 42 | `MMSafeDoor1` | 510 | `palSafeDoor1` | 外 | 外（不在 Panel5） | `SafeDoor1` | `SafeDoor1` |
| 43 | `MMSafeDoor2` | 511 | `palSafeDoor2` | 外 | 外（不在 Panel5） | `SafeDoor2` | `SafeDoor2` |
| 44 | `MMSafeDoor3` | 512 | `palSafeDoor3` | 外 | 外（不在 Panel5） | `SafeDoor3` | `SafeDoor3` |
| 45 | `MMSafeDoor4` | 513 | `palSafeDoor4` | 外 | 外（不在 Panel5） | `SafeDoor4` | `SafeDoor4` |
| 46 | `MMSafeDoor5` | 514 | `palSafeDoor5` | 外 | 外（不在 Panel5） | `SafeDoor5` | `SafeDoor5` |
| 47 | `MMSafeDoor6` | 515 | `palSafeDoor6` | 外 | 外（不在 Panel5） | `SafeDoor6` | `SafeDoor6` |
| 48 | `MMSafeDoor7` | 516 | `palSafeDoor7` | 外 | 外（不在 Panel5） | `SafeDoor7` | `SafeDoor7` |
| 49 | `MMSafeDoor8` | 517 | `palSafeDoor8` | 外 | 外（不在 Panel5） | `SafeDoor8` | `SafeDoor8` |
| 50 | `MMSafeDoor9` | 518 | `palSafeDoor9` | 381 | 不蓋 | `SafeDoor9` | — |
| 51 | `MMSafeDoor10` | 519 | `palSafeDoor10` | 外 | 外（不在 Panel5） | `SafeDoor10` | — |

## kind = `facility`（廠務與檢測）

| # | Pos（unit 常數） | 值 | panel | Panel5 y | 遮蔽 | mv9045 | mv9050 |
|--:|---|--:|---|--:|---|---|---|
| 52 | `MMIonFan01` | 540 | `palIonFan01` | 272 | 蓋住 | — | — |
| 53 | `MMIonFan02` | 541 | `palIonFan02` | 148 | 蓋住 | — | — |
| 54 | `MMIonFan03` | 542 | `palIonFan03` | 44 | 蓋住 | — | — |
| 55 | `MMIonFan04` | 543 | `palIonFan04` | 272 | 蓋住 | — | — |
| 56 | `MMIonFan05` | 544 | `palIonFan05` | 148 | 蓋住 | — | — |
| 57 | `MMIonFan06` | 545 | `palIonFan06` | 272 | 蓋住 | — | — |
| 58 | `MMIonFan07` | 546 | `palIonFan07` | 148 | 蓋住 | — | — |
| 59 | `MMIonFan08` | 547 | `palIonFan08` | 44 | 蓋住 | — | — |
| 60 | `MMIonFan09` | 548 | `palIonFan09` | 432 | 不蓋 | — | — |
| 61 | `MMIonFan10` | 549 | `palIonFan10` | 432 | 不蓋 | — | — |
| 62 | `MMIonFan11` | 550 | `palIonFan11` | 124 | 蓋住 | — | — |
| 63 | `MMIonFan12` | 551 | `palIonFan12` | 432 | 不蓋 | — | — |
| 64 | `MMATC_Handler` | 560 | `pnlATC_Handler` | 外 | 外（不在 Panel5） | — | — |
| 65 | `MMATC_TCPIP` | 561 | `pnlATC_TCPIP` | 外 | 外（不在 Panel5） | — | — |
| 66 | `MMATC_NI` | 562 | `pnlATC_NI` | 外 | 外（不在 Panel5） | — | — |
| 67 | `MMATC_ATC` | 563 | `pnlATC_ATC` | 外 | 外（不在 Panel5） | — | — |
| 68 | `MMATC_Chiller` | 564 | `pnlATC_Chiller` | 外 | 外（不在 Panel5） | — | — |
| 69 | `MMATC_RS232` | 565 | `pnlATC_RS232` | 外 | 外（不在 Panel5） | — | — |
| 70 | `MMATC_Head` | 566 | `pnlATC_Head` | 外 | 外（不在 Panel5） | — | — |
| 71 | `MMATC_PowerSupply` | 567 | `pnlATC_PowerSupply` | 外 | 外（不在 Panel5） | — | — |
| 72 | `MMATC_WaterValve` | 568 | `pnlATC_WaterValve` | 外 | 外（不在 Panel5） | — | — |
| 73 | `MMTemperature` | 504 | `palTemp` | 28 | 蓋住 | — | — |
| 74 | `MMScanner` | 506 | `palScan` | 32 | 蓋住 | `Scanner` | `Reader2DID` |
| 75 | `MMCCD` | 507 | `palCCD` | 28 | 蓋住 | `CCD` | — |
| 76 | `MMOCR` | 186 | `palOCR` | 外 | 外（不在 Panel5） | `OCR` | — |

## kind = `system`（系統通訊）

| # | Pos（unit 常數） | 值 | panel | Panel5 y | 遮蔽 | mv9045 | mv9050 |
|--:|---|--:|---|--:|---|---|---|
| 77 | `MMSystem` | 500 | `palSys` | 28 | 蓋住 | — | — |
| 78 | `MMInterface` | 505 | `palIF` | 28 | 蓋住 | — | — |
| 79 | *（以上皆非）* | — | `palSys` | 28 | 蓋住 | — | — |

## 需要退回 tsHandler 原圖的只有 14 個 panel

規則：`hideMotionView = (panel 在 Panel5 內) && (Panel5 y < 355) && (mv 無對應)`

`palIonFan01` `02` `03` `04` `05` `06` `07` `08` `11`、`palTemp`、`palScan`、`palCCD`、`palSys`、`palIF`

其餘 65 個分支都不必退回 —— 要嘛整機圖有模組可亮，
要嘛 golden 的 pal 落在切線以外（安全門十道、ATC 九格、`palOCR`、九個 `*_Car`、`palIonFan09/10/12`）自己會閃。

