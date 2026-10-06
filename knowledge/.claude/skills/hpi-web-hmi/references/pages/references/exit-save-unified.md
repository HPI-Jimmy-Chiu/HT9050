> 保存來源：`.claude/skills/ht9045-html-version/references/exit-save-unified.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Exit/Save 按鈕全域統一規則（Set 39）

HTML 模擬手冊中所有「離開／儲存」按鈕統一為同一種 TSpeedButton 樣式（同 glyph 圖示、同字型），
消除原 dfm 中 TSpeedButton／TButton／TBitBtn／按鈕型 TPanel 多款混雜的外觀。

## 統一標準（樣式來源）

| 項目 | exit | save |
|---|---|---|
| 標準元件 | cOffSet.dfm `sbtExit` | cOffSet.dfm `spbSave` |
| Glyph 圖示 | `img/dfm_glyph_0bb3440e36.png`（紅 ✕，22×22） | `img/dfm_glyph_0d1673da46.png`（磁片 💾，36×36） |
| 字型 | Arial，fontpx=14（Font.Height=-16） | Arial，fontpx=14（Font.Height=-16） |
| 選定原因 | 主流款：30 頁掃描 Exit 18/20 顆已是此款 | 主流款：Save 18/23 顆已是此款 |

## 規則（產生器 `_gen_dfm_abs.py` 實作）

1. **預載標準**：主迴圈前 parse cOffSet.dfm，`_find_node()` 取 `sbtExit`/`spbSave` 的
   glyph 與字級存入 `STD_BTN = {'exit':(glyph,fs), 'save':(glyph,fs)}`。
2. **命中條件**（caption 取 `.strip()` 後**完全等於**）：
   - `EXIT_CAPS = {'Exit','EXIT','Close','CLOSE'}` → 統一為 exit 款
   - `SAVE_CAPS = {'Save','SAVE'}` → 統一為 save 款
3. **適用型別**：
   - `TSpeedButton` / `TButton` / `TBitBtn`（按鈕分支）
   - `TPanel` **且** 無子元件 **且** 有 OnClick（實機以全寬 TPanel 當按鈕，如 cObserver btExit）
     → 直接輸出 `<button class="btn3d exitbtn">`（保留關窗綁定）
4. **渲染覆蓋**：glyph 換標準圖、font-size 換 14px、title 標
   `「TSpeedButton（統一 exit/save 樣式）」`（TPanel 改造者加註「，原 TPanel」）。
5. **豁免**：複合字樣（`Save Image` / `Save Data` / `Save Config.` /
   `[A27] Save Standard Config` 等）為功能鈕，**不**統一。
6. glyph `<img>` 加 `max-height:88%` 防小按鈕（如 MyCCLinkSensor btSave 75×25）爆框。

## 命中元件全清單（57 顆＝exit 30＋save 27；重生時可用 `_scan_exitsave_table.py` 重掃）

「原 glyph／原 fs」＝dfm 原始值（統一前）；「—」＝原本無圖示（TButton/TPanel）。

| 頁面 | 元件 | 原型別 | Caption | 統一為 | 位置 L,T (W×H) | 原 glyph | 原 fs |
|---|---|---|---|---|---|---|---|
| Setup.OffSet.html | spbSave | TSpeedButton | Save | save | 110,9 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.OffSet.html | sbtExit | TSpeedButton | Exit | exit | 624,8 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.Speed.html | spbSave | TSpeedButton | Save | save | 70,9 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.Speed.html | sbtExit | TSpeedButton | Exit | exit | 420,8 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| HW.IoSetView.html | btnClose | TButton | Close | exit | 19,735 (233×41) | — | 11 |
| HW.IoSetView.html | sbUpdate | TSpeedButton | Save | save | 640,49 (250×36) | dfm_glyph_0d1673da46.png | 17 |
| Config.Configuration.html | sbUpdateTray | TSpeedButton | Save | save | 736,9 (170×36) | dfm_glyph_0d1673da46.png | 17 |
| Config.Configuration.html | sbUpdateHP | TSpeedButton | Save | save | 736,9 (170×36) | dfm_glyph_0d1673da46.png | 17 |
| Config.Configuration.html | sbExit | TPanel | Exit | exit | 0,833 (937×42) | — | 17 |
| Status.CounterSel.html | spbExit | TSpeedButton | Exit | exit | 43,504 (314×48) | dfm_glyph_0bb3440e36.png | 14 |
| Data.CounterClear.html | spbExit | TSpeedButton | Exit | exit | 29,504 (218×48) | dfm_glyph_0bb3440e36.png | 14 |
| Data.Builder.html | spbExit | TPanel | Exit | exit | 0,591 (817×41) | — | 17 |
| Config.DIOInterFaceCFG.html | spbSave | TSpeedButton | Save | save | 283,19 (100×40) | dfm_glyph_0d1673da46.png | 14 |
| Config.DIOInterFaceCFG.html | spbExit | TPanel | Exit | exit | 0,428 (598×41) | — | 17 |
| Status.LtcSensor.html | btnClose | TPanel | Exit | exit | 0,441 (1198×39) | — | 17 |
| Status.TowerLight.html | spbExit | TPanel | Exit | exit | 0,489 (760×41) | — | 17 |
| Setup.QAMode.html | btnApply | TButton | Save | save | 12,415 (200×40) | — | 17 |
| Setup.QAMode.html | btnOk | TButton | Exit | exit | 257,415 (200×40) | — | 17 |
| Setup.BarCode.html | spbSave | TSpeedButton | Save | save | 206,4 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.BarCode.html | sbtExit | TSpeedButton | Exit | exit | 556,3 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| HW.MyCCLinkSensor.html | btSave | TButton | Save | save | 728,168 (75×25) | — | 11 |
| HW.MyCCLinkSensor.html | spbSave | TSpeedButton | Save | save | 10,240 (175×49) | dfm_glyph_5bd76345e9.png | 11 |
| HW.MyCCLinkSensor.html | sbExit | TSpeedButton | Exit | exit | 894,12 (160×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.Cleaning.html | sbCleanSave | TSpeedButton | Save | save | 14,5 (150×36) | dfm_glyph_0d1673da46.png | 14 |
| Setup.Cleaning.html | sbCleanExit | TSpeedButton | Exit | exit | 177,5 (150×36) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.Contact.html | sbtExit | TSpeedButton | Exit | exit | 736,8 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.Contact.html | spbSave | TSpeedButton | Save | save | 502,9 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.TesterIF.html | spbSave | TSpeedButton | Save | save | 62,9 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.TesterIF.html | sbtExit | TSpeedButton | Exit | exit | 384,8 (237×41) | dfm_glyph_0bb3440e36.png | 14 |
| Status.GroundMan.html | spbSave | TSpeedButton | Save | save | 46,5 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Status.GroundMan.html | sbtExit | TSpeedButton | Exit | exit | 316,5 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.Ld_ULd.html | spbSave | TSpeedButton | Save | save | 22,17 (241×41) | dfm_glyph_0d1673da46.png | 14 |
| Setup.Ld_ULd.html | sbtExit | TSpeedButton | Exit | exit | 316,17 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Status.Security.html | SecurityExit | TPanel | Exit | exit | 0,830 (874×41) | — | 25 |
| Setup.TrayForm.html | spbSave | TSpeedButton | Save | save | 330,656 (150×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.TrayForm.html | sbtExit | TSpeedButton | Exit | exit | 511,656 (150×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.SCK_ART.html | btnExit1 | TSpeedButton | Exit | exit | 180,108 (149×37) | dfm_glyph_0bb3440e36.png | 19 |
| Setup.SCK_ART.html | btnExit | TSpeedButton | Exit | exit | 4,882 (380×37) | dfm_glyph_0bb3440e36.png | 19 |
| Setup.YieldMonitoring.html | btnApply | TButton | Save | save | 144,15 (200×40) | — | 17 |
| Setup.YieldMonitoring.html | btnOk | TButton | Exit | exit | 424,15 (200×40) | — | 17 |
| Setup.HotPlate.html | spbSave | TSpeedButton | Save | save | 54,8 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.HotPlate.html | sbtExit | TSpeedButton | Exit | exit | 332,7 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Data.Observer.html | Button7 | TButton | Save | save | 848,608 (60×45) | — | 11 |
| Data.Observer.html | sbPrecautionSave | TSpeedButton | Save | save | 814,445 (131×35) | dfm_glyph_cd26d583dd.png | 11 |
| Data.Observer.html | sbMajorMaintenanceSave | TSpeedButton | Save | save | 838,453 (100×30) | dfm_glyph_2473a2707a.png | 11 |
| Data.Observer.html | btExit | TPanel | Exit | exit | 0,738 (932×37) | — | 17 |
| Setup.SetUp.html | sbUpdate | TSpeedButton | Save | save | 61,8 (323×43) | dfm_glyph_0d1673da46.png | 27 |
| Setup.SetUp.html | sbtExit | TSpeedButton | Exit | exit | 427,7 (323×44) | dfm_glyph_0bb3440e36.png | 27 |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | TSpeedButton | Exit | exit | 675,5 (120×40) | dfm_glyph_cc74cc3d63.png | 14 |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Save | TSpeedButton | Save | save | 550,4 (120×40) | dfm_glyph_0d1673da46.png | 14 |
| Data.StartCondition.html | spbExit | TPanel | Exit | exit | 0,686 (1097×41) | — | 17 |
| Data.StartCondition.html | sbSave | TSpeedButton | Save | save | 456,148 (169×38) | dfm_glyph_15ad2305ef.png | 11 |
| Data.StartCondition.html | sbHeadCondition1Save | TSpeedButton | Save | save | 665,14 (182×41) | dfm_glyph_15ad2305ef.png | 11 |
| Setup.Temp_Set.html | spbSave | TSpeedButton | Save | save | 170,9 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.Temp_Set.html | sbtExit | TSpeedButton | Exit | exit | 488,8 (241×41) | dfm_glyph_0bb3440e36.png | 14 |
| Setup.BinSel.html | spbSave | TSpeedButton | Save | save | 30,37 (227×40) | dfm_glyph_0d1673da46.png | 14 |
| Setup.BinSel.html | sbtExit | TSpeedButton | Exit | exit | 472,36 (241×41) | dfm_glyph_0bb3440e36.png | 14 |

## 統計

- 命中：57 顆（exit 30／save 27）；原型別 TSpeedButton 42、TPanel 8、TButton 7
- 原 glyph 分布：exit 主流 0bb3440e36×18、save 主流 0d1673da46×18，
  其餘雜款（5bd76345e9／cd26d583dd／2473a2707a／cc74cc3d63／15ad2305ef）與無圖示者全數換為標準款
- 按鈕型 TPanel 8 顆（全部是全寬底部 Exit 條）：cConfiguration sbExit、cBuilder spbExit、
  DIOInterFaceCFG spbExit、LtcSensor btnClose、cTowerLight spbExit、cSecurity SecurityExit、
  cObserver btExit、cStartCondition spbExit

## 維護注意

- 新增 dfm 頁後重生即自動套用；若要重掃清單更新本表，執行
  `D:\AI_TempFile\_scan_exitsave_table.py`（輸出 `_exitsave_table.md`）。
- caption 判斷是**完全等於**，複合字樣（Save Image 等）永遠不受影響。
- 若客戶頁面出現新的按鈕型 TPanel，條件需同時滿足：無子元件＋有 OnClick＋caption 命中。

<!-- preserved-content:end -->
