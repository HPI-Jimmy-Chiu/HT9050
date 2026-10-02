# DoLoad() 進料限制地圖（Loader 供料狀態機）

> 用途：查 `DoLoad()`（Loader 盤供給狀態機）**在什麼條件下才會/才不會進料**，以及要加「進料前置閘門」時的乾淨插入點。
> 實碼驗證版本：**V3.33.908.2**（行號以此為準）；906.5 邏輯一致，行號略有漂移。
> 關聯：本技能 §7（收料 vs 退盤）、§8（DoLoad 7 clean-out 觸發 / 收工後補料坑）、staterecord skill `deadlock-patterns.md` **Pattern #12**、記憶 `jscc-cleanout-deadchicken-judgeempty`。

---

## 0. 角色

`DoLoad()`（`asendic_Loader.cpp:2448`，`switch(LoadTask)`）是 **Loader 盤供給狀態機**：偵測 loader 盤在不在、供下一盤、盤空時決定「從 Loader Car 補盤（`SupplyNewIC_From_LoaderCar`）」或「觸發 clean-out（DoLoad 1~7）」。

- **真正「進新料」的動作**：case 600 → `InitSupplyNewIC_From_LoaderCarTask()` → `Task=1000`。
- InArm 從 loader stage 取 IC 是 `DoInArm`（另一支）；DoLoad 只負責「讓 loader stage 有一盤可取」。擋住 DoLoad 供盤/前進 ＝ 無新盤可取 ＝ 無料進機台。

---

## 1. 第①層：呼叫層閘門（`csystem.cpp` — DoLoad 被不被呼叫）

DoLoad 由主生產迴圈呼叫（非 `MainProc` 頂層），一連串前置 `return` 決定它會不會被呼叫：

| 條件 | 行號(908.2) | 效果 |
|---|---|---|
| ART GPIB mode 未收到 LotStart / contact 未就緒 | `10045-10082` | `return` |
| `CUSTOMER_CODE==CC_UTAC` 且 online 未 `bWaitStartLotAutoRetestGPIB` | `10084-10093` | `return` |
| `iClearSocketFunction==2`（清 socket 中）| `10095-10099` | `DoCleanSocket(); return` |
| `SoftStop \|\| SystemStart==false \|\| fAllMotorHome==false` | `10105-10108` | `return` |
| `flag`（=生產 pass，`9529 flag=true`）＋（AMR）`bLoaderLockActionFlag[0]==false` | `10110-10120` | 才真正呼叫 `DoLoad()` |

> **前提**：機台在正常生產、馬達已 home、非 soft-stop、非清 socket、（AMR）未鎖 loader，才會進 DoLoad。
> **注意**：`DoLoad` 的呼叫 gate 是 `flag`/`SystemStart`/`fAllMotorHome`，**與 `iTrayFeed` 無關**（見 §8 / Pattern #12 Q1：iTrayFeed=0 不是「開了進料的門」，是「根本沒有退盤完成度的門」）。

---

## 2. 第②層：DoLoad 入口守衛（switch 之前，`2457-2476`）

| 條件 | 行號 | 效果 |
|---|---|---|
| `bOneTimeHotPlateCheckAll`（HotPlate 檢查中）| `2457` | `return` |
| `bNewResetFunction && bResetLoadTray` | `2460-2476` | 重置 loader 盤狀態後 `return` |

---

## 3. 第③層：狀態機閘門（`LoadTask`，`2478` 起）

```
case 1   (2480) 驗 loader 盤: Sen[SnLoaderSureTray] vs MOT[MMTrayY].fHasTray
                 ├ DUMMY 且無盤            → 300
                 ├ 不一致(盤 miss)         → 200 (JAM0929 tray split fail)
                 ├ 不一致(多盤/sensor err) → 100 (MES0922 請移除 loader 盤)
                 └ 一致                    → 300
case 100 (2533) MES0922 alarm; RETRY→1 / 否則 Car 標 HAS_IC → 1
case 200 (2546) JAM0929 (bA04LoaderTraySplitFailCanSkip 才可 SKIP); RETRY→1 / SKIP→清盤→300
case 300 (2567) 驗 Loader Car: Sen[SnLoaderCarHasTray] vs MOT[MMTrayY_Car].fHasTray
                 ├ Enable 且 一致          → 600
                 ├ car 有盤但軟體無        → 400 (MES0922)
                 ├ 盤 lost                 → 500 (JAM0929)  (DUMMY→600)
                 └ sensor 未 Enable        → 600
case 400 (2593) MES0922(可 SKIP); RETRY→300 / SKIP→Car 標 HAS_IC→300
case 500 (2606) JAM0929; RETRY→300 / SKIP→Car 清盤→600
case 600 (2623) ★進料決策★
                 ├ (KYEC ART 且 bCleanOut_ART 且 loader/ART sensor on)
                 │        → ShowMyMessage「請移除 Loader Magazine Tray」, 停 600
                 ├ MOT[MMTrayY_Car].fHasTray==true
                 │        → InitSupplyNewIC_From_LoaderCarTask() + Task=1000  ← 【進料】
                 └ MOT[MMTrayY_Car].fHasTray==false
                          → Task=800  (loader + Car 皆空)
case 800 (2657) loader 無盤 → 依 ART / bA65_BundleIDList / bP57 / 一般分支：
                 → clean-out (DoLoad 1~7) 或 MES0920 問操作員(RETRY/CLEAN_OUT)
                 ※ DoLoad 7 觸發需外層 2697-2700「no any tray」(ALed1==false &&
                    MMTrayY_Car.fHasTray==false && MMTrayY.fHasTray==false)，見 §8.1
case 1000+      SupplyNewIC 取盤序列（把 Car 的盤供上 loader stage）
```

---

## 4. 客戶 / Config 相關額外限制（散在 case 600 / 800）

多半影響「盤空時走 clean-out 還是問人 / ART 保留一盤 / AMR 等盤」：
`USE_LdUldCassetteMode`、`bA04LoaderTraySplitFailCanSkip`、`bA10_AutoReTest`/`bAutoReTest_ART`、`bUseSCKART`、`bA65_BundleIDList`、`bP57LoaderAutoCleanOutByInputCT`、`bNoTrayAutoCleanOut`、`bA60EnableAMR`/`bA68_AutoLoadUnload`、`bP07NoTrayaAutoTrayFeed`(TSMC)、`CC_ASE_KaohSiung`、`CC_KYEC_LEE`。

---

## 5. 關鍵結論（給後續下 gate / 排查用）

**DoLoad 進料的限制全圍繞「loader 盤 / Loader Car 盤 的實體 vs 軟體一致性」＋「ART / AMR / 客戶模式」——沒有任何一條與出料側（退盤完成 / 出料盤就位）有關。**

→ 目前**不存在**「退盤未完成不進料」的閘門（正是 Pattern #12 要補的那道）。若要補：

| 插入點 | 行號 | 理由 |
|---|---|---|
| **入口（推薦）** | `2476` 之後、`switch` 前 | 最上游、一刀擋全部進料路徑；未完成→alarm（**非靜默 return**）|
| 或 case 600 進料前 | `2623` `if(MMTrayY_Car.fHasTray)` 之前 | 精準擋在「要 SupplyNewIC」那一刻 |

**gate 條件要用耐久訊號**（`ReceiveAutoTrayTask[i]` 未完成 / `MOT[iMMAuto_Car[i]].fHasTray` 與 `Sen[SnAutoTrayCar[i]]` 軟硬不一致 / 出料盤未就位），**不要用 `iTrayFeed`**（會被 ONE CYCLE 清 0，DoLoad 也不看它）。⚠ 解除退盤勿用慣例 `iTrayFeed=1; iTrayFeedTask=1` re-arm（會重跑再死鎖，見 Pattern #12 陷阱）。

> ⚠ 上述修法方向**未上機驗證**，暫為分析結論（RogerYang 20260710）。

---

## 6. 相關函式錨點（`asendic_Loader.cpp` 908.2）

| 函式 / Task | 行號 |
|---|---|
| `DoLoad()`（`int &Task=LoadTask`）| `2448` |
| `SupplyNewIC_From_LoaderCar`（`iSupplyNewIC_From_LoaderCar`）| `157` |
| `iCassetteLoadNewICTrayTask` 相關 | `73` |
| `iLoadNewICTrayTask` | `2025` |
| DoLoad 1~7 clean-out 呼叫 | `2738 / 2763 / 2829 / 2851 / 2863 / 2874 / 2911` |
| DoLoad 7 外層「no any tray」閘 | `2697-2700` |
