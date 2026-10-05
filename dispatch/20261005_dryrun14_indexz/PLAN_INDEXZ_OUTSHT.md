# HT9050 Index Z 分成 In Shuttle／Out Shuttle 兩個高度（2026-10-05 計畫）

EastSun 1005：「因為9050 inshuttle 和outshuttle 高度不一樣 所以index arm Z 高度需要有兩個 一個inshuttle indexZ 一個outshuttle indexZ」

## 現況（已查證）
- 原版只有一個 Index Z1 的 Shuttle 高度：teach.ini `[MTestZ1] setEditIndex1ToSht1Z`（目前 -4357）＝`Tech.iTestZ1ShutlePick`（forms/fTeachRegistry.cpp:114）。
- 吸 IC 與放 IC 都用它（cinitial.cpp:5816 / :5818）：
  - `Prod.TestZ1_Pick  = iTestZ1ShutlePick + Contact「Pick Up1」 + Offset.iIndexArmPickUp[0]` → aTester_Front.cpp:1358/1408（從 In Shuttle 吸）
  - `Prod.TestZ1_Place = iTestZ1ShutlePick + Contact「Place1」 + Offset.iIndexArmPlace[0]` → aTester_Front.cpp:455-468（測完放回 Shuttle）；AutoClean、清 Socket 也用 TestZ1_Place
  - 超過 PickLimit 時退回原始教導值（:5849 / :5855）
- 空跑（Ht9050DryRun）不動 Index Z1，不受影響。
- 原版 LastSet.h:708 有一個宣告但全樹（含 golden）從沒用過的欄位 `Tech.iHT9040TestZ1_PlaceSH2`（HT9040「Index Z1 放到 Shuttle 2」）。

## 改法（只 HT9050／PCI1203_IO 機台，其他機種照原版）
1. **新教導點「Out Shuttle Z」**：teach.ini `[MTestZ1] setEditIndex1ToOutSht1Z`，存在空著的 `Tech.iHT9040TestZ1_PlaceSH2`（不改 TECH 結構大小）。
   照 10-01 TEACH-3AXES 的做法：tools/gen_teach_registry.py EXT_ROWS 加一列 → 重新產生 fTeachRegistry.cpp / WebTeachButtons.gen.inc / FileRW/Teach.gen.inc；按鈕 btnSetIndex1OutSht / btnGoIndex1OutSht（SetButton140Click / GoButton140Click 同一套）。
2. **流程**：HT9050 上 `Prod.TestZ1_Place = Out Shuttle Z + Contact「Place1」 + Offset.iIndexArmPlace[0]`（PickLimit 退回也用 Out Shuttle Z）。吸 IC（TestZ1_Pick）仍用原本的 Shuttle Z＝In Shuttle。
3. **網頁 Teach Index 頁**：
   - 原本「Arm1 Z」按鈕改名「In Sht Z」（＝吸 IC，Shuttle 1 圖旁）；新增「Out Sht Z」Set＋GO（同一個 Arm1 方框內，不蓋畫面）。
   - 下方表格：原「Shuttle Z」欄改標「In Shuttle Z」，新增「Out Shuttle Z」欄（Arm 1 列）。
   - teach-access.json / .js 加一列；MTestZ1 Enable=0 時照現有規則隱藏。
4. 測試：test_web_motor_access（登錄列數 +1、新按鈕解析）、TeachButtonsGen、gen_teach_editlist --check。

## 要 EastSun 決定
- 新點第一次開機的初始值：0（Z 在最上面，未教點前放 IC 會從高處放）或 複製目前 In Shuttle Z（-4357）。
