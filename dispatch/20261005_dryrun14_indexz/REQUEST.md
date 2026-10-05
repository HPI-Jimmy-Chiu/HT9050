# 派工 2026-10-05：HT9050 空跑改成 14 步＋Index Z 分 In／Out Shuttle 兩個高度

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-05 下午
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`，HEAD = `7d58cf2`；web HEAD = `8bd78b0`。
> golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。行號會漂，用之前先 grep 名稱。

## 0. EastSun 原話

1. 「因為9050 inshuttle 和outshuttle 高度不一樣 所以index arm Z 高度需要有兩個 一個inshuttle indexZ 一個outshuttle indexZ」
2. 「請你再派一個agent 改昨天額外加入的流程 要多加幾個動作」（下面 14 步，逐字）
3. 「將以上的步驟使用程式模擬是否能正確執行，若能模擬正確執行再進行真正的機台動作」
4. 「請你先暫停 把相關資料 上傳給jimmy做 我們先用之前的dry run」

→ **機台端現在照舊用 10-05 的空跑**（`Ht9050DryRun.cpp`，Step1~10，Index Z 不動，IC 只當資料傳）。下面兩件事請 Jimmy 做，做好以筆電包的方式交回，機台端照「先寫計畫、昨天的流程要先問 EastSun」的規矩整合。

### 14 步（EastSun 逐字）

```
Step1:在Loader上吸取IC
Step2:吸取IC後到In Shuttle1放置IC
Step3:判斷Index Z 是否在Home點以及Out Shuttle1是否在右邊
Step4:將In Shuttle1移動至Index區(Index Z動作By Pass)
Step5:indexZ 下降到inshuttle indexZ 吸取IC
Step6:indexZ 吸完IC 後並且有吸到IC 上升到home點位
Step7:將In Shuttle1移動至左邊
Step8:將Out Shuttle1移動至左邊
Step9:indexZ 下降到outshuttle indexZ位置後 釋放IC 並且破真空0.3秒
Step10:indexZ 上升到index home位置
Step11:將Out Shuttle1移動至右邊
Step12:將Out Arm先移動 Out Arm X軸到達取Out Shuttle1的位置後，在移動Out Arm Y軸到取放點位
Step13:Out Arm 吸取Out Shuttle1的IC
Step14:Out Arm 將IC放置於Auto 1的地方(動作先將Out Arm Y軸移動到Auto 1的位置後再移動Out Arm X，因為有干涉區的關係)
```

**驗收**：先用程式模擬（`tests/test_ht9050_dryrun.cpp` 那種模擬軸／感測器的方式）跑過至少兩輪完整 14 步，下面 §3 的條件都斷言通過，才交給 EastSun 上機。

---

## 1. 附檔

| 檔名 | 內容 | 大小 | MD5 |
|---|---|---|---|
| `Ht9050DryRun.cpp` / `.h` / `test_ht9050_dryrun.cpp` | **機台現用版**（`7d58cf2`）空跑與它的測試，參考用 | 20,978 / 2,366 / 19,635 | `1D8C2B5C…` / `E2883837…` / `66E5280C…` |
| `indexz_outsht_cpp.patch` | **Index Z 兩個高度，C++ 半，機台端已寫好、已驗證可套**（`git apply` 在 `7d58cf2` 上 check 通過）。產生器也改了，已重跑、`gen_teach_editlist.py --check` 通過 | 9,369 | `F598D7C2CA573E27ACE74099708F5ADF` |
| `indexz_outsht_web.patch` | 同上，web 半（在 web `8bd78b0` 上 check 通過；HW.teach.html 是單行大檔所以 patch 大） | 424,929 | `E8989A5F0A5D12BC8F20726CB152B9C3` |
| `PLAN_INDEXZ_OUTSHT.md` | Index Z 兩個高度的計畫（EastSun 已同意「照計畫做」、初始值「0」） | 2,558 | `6BED610C…` |
| `dryrun14_DRAFT_unverified.patch` | **草稿，未編譯、未驗證**：機台端小幫手被暫停前寫到一半的 14 步（`Ht9050DryRun.*`＋測試，+433／-98 行）。只當參考，**不要直接套** | 63,647 | `08DD6BE206E7BEB5D814D6D0DB001DFE` |

⚠ Index Z 的 patch 機台端**沒有 commit、沒有上機**（EastSun 喊暫停時還在跑 o2 測試）。請在筆電端套上、跑 `WebMotorAccess`、`TeachButtonsGen`、`GaliRouteEngine` 等再隨包交回。

---

## 2. Index Z 兩個高度（已查證的事實）

- 原版只有一個 Index Z1 Shuttle 高度：teach.ini `[MTestZ1] setEditIndex1ToSht1Z`（機台現值 **-4357**）＝`Tech.iTestZ1ShutlePick`（`forms/fTeachRegistry.cpp:114`，golden `uteach.cpp:391`）。網頁 Index 頁的「Arm1 Z」＝它的 Set、下面 GO＝Go，下方表格 Shuttle Z 欄。
- 吸／放都用它（`cinitial.cpp:5816 / :5818`）：
  - `Prod.TestZ1_Pick  = iTestZ1ShutlePick + DeviceForm.IndexArmPick[0]（Contact「Pick Up1」）+ Offset.iIndexArmPickUp[0]` → `aTester_Front.cpp:1358/1408`（DoFrontTestSuckIC，從 Shuttle 吸）
  - `Prod.TestZ1_Place = iTestZ1ShutlePick + DeviceForm.IndexPlace[0]（「Place1」）+ Offset.iIndexArmPlace[0]` → `aTester_Front.cpp:455-468`（DoFrontTestDestroyIC）；AutoClean、清 Socket 也用 TestZ1_Place
  - 超過 `MOT[MTestZ1].IndexPickLimit` 退回原始教導值（`:5849 / :5855`）
- 放進 Socket 的高度不是 Teach 點：`Prod.TestZ1_Test = DeviceForm.IndexContact[0] + Offset.iIndexArmContact[0]`（Contact 頁「Test Arm1」）。
- patch 的做法（只 `IO_CARD_TYPE==PCI1203_IO`，＝HT9050；其他機種照原版）：
  - 新教導點 **Out Shuttle Z**：teach.ini `[MTestZ1] setEditIndex1ToOutSht1Z`，存在 golden 宣告但全樹（含 golden）從沒用過的 `Tech.iHT9040TestZ1_PlaceSH2`（`LastSet.h:708`，TECH 大小不變）；照 10-01 TEACH-3AXES 的路：`tools/gen_teach_registry.py` EXT_ROWS 第 7 列 → `fTeachRegistry.cpp`／`WebTeachButtons.gen.inc`／`FileRW/Teach.gen.inc`；按鈕 `btnSetIndex1OutShtZ`／`btnGoIndex1OutShtZ`（SetButton140Click／GoButton140Click）。
  - `Prod.TestZ1_Place` 在 HT9050 改用 Out Shuttle Z（PickLimit 退回也用它）；吸 IC 仍用 In Shuttle Z。
  - 初始值 0（EastSun 1005 選的；Z 在最上面，未教點前放 IC 會在高處放）。
  - 網頁：Index 頁「Arm1 Z」改字「In Sht Z」、表格「Shuttle Z」改「InSht Z」；Arm1 方框 Server ON 右邊加「Out Sht Z」Set（545,24）／GO（545,56）／欄位（620,57）；`ht9045_teach_3axes_c.js` 只在 MTestZ1 Enable=1 才顯示；`teach-access.json`／`.js` 加一列（w906TechPara 6→7）。⚠ 同頁另一顆 `SetButton077` 也叫「Arm1 Z」，**沒有改**。
- 網頁沒有輸入框的 Socket Y／Shuttle Y／Arm 2 整列：MTestY1／MTestZ2／MTestY2 在機台 Mot_Table 是 Enable=0（HT9050 只有一支 Index），頁面照 10-01 規則隱藏。

## 3. 14 步要注意的地方（機台端查到的，請在模擬裡斷言）

- **Index Z1 = MTestZ1**，PCI-1203 軸走 Galil 路線：移動用 `MOT[MTestZ1].Gali_MotMove(pos, speed, name)`（HT9050 HOME 也這樣，`uhome.cpp` W906_HomeTwoZ）；在原點判斷用 `W906_Ht9050IndexZ1AtHome()`（acarry.cpp，homed＋編碼器 ±20）。
- 高度：Step5 用 `Prod.TestZ1_Pick`（In Shuttle Z）、Step9 用 `Prod.TestZ1_Place`（套了 §2 的 patch 後＝Out Shuttle Z）。
- Index 吸嘴真空／破真空／真空 OK 感測器：用 golden Index 吸放的同一組 IO（`aTester_Front.cpp` DoFrontTestSuckIC ~:1118、DoFrontTestDestroyIC ~:330）。Enable=false 的 IO 不能卡死（照空跑檔現有慣例）。Step6「有吸到 IC 才上升」：等真空 OK 有逾時，逾時用空跑檔現有的等待訊息方式（**不加確認視窗、不蓋畫面**，EastSun 規矩）。Step9 破真空 0.3 秒。
- 飛梭防撞（acarry.cpp `AI(W906-HT9050-SHTSAFE)`）全部要守：Index Z1 不在原點不准動飛梭；Out Shuttle 1 沒讓開（在右邊）不准 In Shuttle 1 進 Index 區。In Shuttle 1 哪一邊是 Index 區、Out Shuttle 1 左＝Index 下方（EastSun Step8），請照現有空跑與 acarry 確認。點位：`Prod.InSHT[0].iLeft/iRight`、`Prod.OutSHT[0].iLeft/iRight`（Out Shuttle 1/2 左右點是 10-01 TEACH-3AXES 加的教導點）。
- Out Arm：XY 移動前 Out Arm Z 要在安全位；Step12 **先 X 後 Y**、Step14 **先 Y 後 X**（干涉區）。取 Out Shuttle 1 與放 Auto 1 的點位、Out Arm 真空 IO 照現有空跑／golden OutArm 流程。
- Step1～2 沿用現有空跑 In Arm 吸 Loader、放 In Shuttle 1 的寫法。
- 保留空跑現有機制：`W906_HT9050_DRYRUN`、`W906_HT9050_DRYRUN_SPEED`、暫停／START、cycle 計數、等待訊息；機台端 10-05 起**回原點時會重設空跑**（uhome 第 300 步呼叫 `W906_Ht9050DryRunReset()`，HOME-AUDIT1005），重設後要從新 Step1 開始。
- 規矩：只動 HT9050 分支、golden 不變；enable=0 的軸／氣缸一律跳過；不自己動機台（上機由 EastSun）。

## 4. 機台端狀態

- 機台 C++ `7d58cf2`（含 HOME-AUDIT1005 664d186、JOG-VLTIME-2 7d58cf2），web `8bd78b0`；Index Z patch 與 14 步**都不在機台程式裡**。
- GitHub `machine/integ-ioweb` 已推到 ff1b55f（本派工單另推）。
