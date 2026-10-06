> 保存來源：`.claude/skills/ht9045-motionview-html-ui/references/live-mode.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# LIVE 模式：反映實機當下狀態

Motion View 的本分是「現在機台在幹什麼」。所以這個頁面有兩條路：

| 模式 | 來源 | 行為 |
|---|---|---|
| **SIM** | recipe ＋ `teach.ini` | 依參數與程式碼規則**推算**動作序列，可播放 |
| **LIVE** | StateRecord | **不推算**，直接畫讀到的當下狀態；播放與時間軸停用 |

LIVE 的原則只有一條：**讀到什麼畫什麼，讀不到就標「不明」，絕不用推算值填補。**
把推算值畫成實機狀態，比留白危險得多。

## 目錄

- [1. 讀哪些檔](live-mode.md#1-讀哪些檔)
- [2. 機構位置怎麼換算](live-mode.md#2-機構位置怎麼換算)
- [3. IC 在籍：已對接與缺口](live-mode.md#3-ic-在籍已對接與缺口)
- [4. 檔案格式陷阱](live-mode.md#4-檔案格式陷阱)
- [5. 要即時（非快照）需要什麼](live-mode.md#5-要即時非快照需要什麼)

---

## 1. 讀哪些檔

| 檔案 | 供什麼 |
|---|---|
| `MainFormSnapshot.txt` | 快照時間、**各 actor 的 Task**、InArm／OutArm 吸嘴在籍、**四個 Carry Kit 逐格在籍**、Loader/TrayArm 交接旗標 |
| `Motor.xls` | **每一軸的實際脈波**（29 軸） |
| `HP1_*/HP2_*.xls` | HotPlate **逐格** `Site` / `WhichShuttle` / `WhichKit` / `HotTime` / `Count` / `Row` |
| `Task.xls` | 全部 task 變數（比 snapshot 更完整，可選） |
| `DecisionVariables.csv` | 決策旗標快照（可選） |

⚠ **參數（recipe / teach）不從 StateRecord 拿**，那是機台本機
`system\` 與 `IniData\Data\<Device>\` 的事。StateRecord 只是剛好包了一份副本，
所以拖 StateRecord 進去時參數也會一起對接 —— 但不要把 StateRecord 當成參數的來源。

---

## 2. 機構位置怎麼換算

規則：**脈波 ÷ 100 = mm**，再加該軸的座標框偏移（`OFS`）。
偏移只影響整張圖平移，相對幾何完全由 `teach.ini` 決定。

```js
InArm  X = MInArmX/100 + OFS.inX        Y = MInArmY/100
OutArm X = MOutArmX/100 + OFS.outX      Y = MOutArmY/100
Shuttle n 位移 = (MInShutte<n> - InSht<n>Left) / (InSht<n>Right - InSht<n>Left) × 行程
Index n 行程比 = (MTestY<n> - Index<n>ToSht<n>Y) / (Index<n>ToSocketY - Index<n>ToSht<n>Y)
```

`OFS.outX` 不是猜的，是反推：**OutArm 停在 SHT1 的位置就是站C**，
而站C = 站A + 2×行程（Shuttle 三站雙 Kit）→ `OFS.outX = G.C - MOutArmX(SHT1教點)/100`。

### 實測交叉驗證（HHT-139，2026-09-05 18:21:48 快照）

| LIVE 讀值 | 對應教點 | 判讀 |
|---|---|---|
| `MTestY1 = 15955` | `setEditIndex1ToSocketY = 15955` | Index1 **在 Socket** ✓ |
| `MTestY2 = 153` | `setEditIndex2ToSht2Y = 153` | Index2 **在 SHT2** ✓ |
| `MOutArmY = -37642` | `setEditOutSht1Y = -37642` | OutArm **在 SHT1**，與 `OutArmTask=200`（取料中）一致 ✓ |
| `MInShutte1 = 37` | `setEditInSht1Left = -1` | SHT1 **在 iLeft** ✓ |

⇒ 換算對得上教點，就代表 LIVE 的定位是真的，不是畫給人看的示意。
**做完一定要做這個交叉核對**，否則畫面看起來很像卻是錯的。

---

## 3. IC 在籍：已對接與缺口

### 已對接（逐格）

| 位置 | 來源 | 說明 |
|---|---|---|
| InArm 吸嘴 | `MainFormSnapshot` `---- InArmSuck ---- Item r0/r1` | 每支吸嘴的 `Item[]` 值 |
| OutArm 吸嘴 | `---- OutArmSuck ---- Item r0/r1` | 同上 |
| 四個 Carry Kit | `---- Shuttle Item (FL/FR/BL/BR) ----` | FL/FR＝前 Shuttle 的 In-Kit/Out-Kit；BL/BR＝後 Shuttle |
| HotPlate | `HP*_Site.xls`（值＝Site）、`HP*_WhichShuttle.xls`（送哪條）、`HP*_HotTime.xls`（soak 進度） | 逐格完整 |

### ⚠ 缺口（目前拿不到逐格）

| 位置 | StateRecord 只有 | 後果 |
|---|---|---|
| Loader 盤 | `MMTrayY.fHasTray` / `.HasIC` 布林 | LIVE 下只能標**「不明」**（斜線底紋），不可畫成空盤 |
| Auto / Fix 盤 | `BinCount.txt` 總數 | 同上 |
| Socket / Index | 無 | 同上 |

**這是資料缺口，不是顯示問題。** 頁面刻意用斜線底紋標「不明」，
因為把沒有資料的盤畫成空盤，會讓現場以為「盤是空的」——比留白更危險。

### 要補齊的話（handler 端提案，需使用者裁決）

比照 HotPlate 現成的做法，在 StateRecord 匯出時多寫幾個逐格 dump：

```
Tray_Loader.xls    ← MOT[MMTrayY].Tray.Data[col][row]
Tray_Auto1..3.xls  ← 各 Auto 盤
Tray_Fix1..6.xls   ← 各 Fix 盤
Socket.xls         ← TestSocket / FTestSuck / BTestSuck 的 Item[][]
```

HotPlate 的匯出程式碼已經在了，複製同一段換資料來源即可，格式（BIFF2 全字串）也不用改，
頁面的 `biffToGrid()` 直接就能吃。**這是新增匯出、不動生產流程**，風險低。

---

## 4. 檔案格式陷阱

### 4.1 `.xls` 是 BIFF2，而且**所有值都寫成字串**

機台用最簡 BIFF 寫檔，只出現三種記錄：

| id | 記錄 | 版面 |
|---|---|---|
| `0x0002` | INTEGER | rw(2) col(2) attr(3) w(2) |
| `0x0003` | NUMBER | rw(2) col(2) attr(3) f64(8) |
| `0x0004` | LABEL | rw(2) col(2) attr(3) cch(1) rgch |

**實測 `Motor.xls` 與 `HP*_*.xls` 的記錄 100% 是 `0x0004`（LABEL）** ——
連數字（含負數）也是字串。所以取值時**不可以只收 `typeof v === "number"`**，
否則整張表讀出來是空的（踩過：`馬達筆數 = 0`）。

### 4.2 `TestMode.Data` 沒有 `Mode=2x4` 這種鍵

實際內容只有 `Tester Connection` / `Temperature Mode` / `Running Mode`，
吸嘴排欄數要從 `[DutOnOff]` 推：

```
Dut  Aa=1  Dut  Ab=1  Dut  Ac=1  Dut  Ad=1   → 排 A、欄 a~d
Dut  Ba=1  Dut  Bb=1  Dut  Bc=1  Dut  Bd=1   → 排 B
Dut  Aa2=1 …                                  → 字尾 2 是 Arm2，推 Arm1 時要排除
⇒ 2 排 × 4 欄
```

### 4.3 `teach.ini` 的鍵名是 UI 元件名

不是 `Pitch40` 而是 `setEditInXPitch40`；不是 `InLeft` 而是 `setEditInSht1Left`。
主要幾何鍵：

```
[MInArmX]    setEditLoaderX / setEditHP1X / setEditHP2X / setEditInSht1X / setEditInSht2X
             setEditPreciserX / setEditAutoCleanX
[MInArmY]    同名 …Y
[MInShuttle1] setEditInSht1Left / setEditInSht1Right      ← 行程
[MOutArmX]   setEditOutSht1X / setEditAuto1~3X / setEditFix1~3X
[MOutArmY]   setEditOutSht1Y / setEditAuto1Y / setEditFix1Y
[MTrayX]     setEditTrayLoaderX / …EmptyX / …ColorX / …Auto1~3X
[MTestY1]    setEditIndex1ToSht1Y / setEditIndex1ToSocketY
[MInArmPitch] setEditInXPitch40 / setEditInXPitch120
[MInArmZE]   SetEditPickLoader / SetEditHP / SetEditPlaceInShuttle
```

⚠ 各版本／機型鍵名略有差異，**讀不到就退回內建預設並在畫面上列出警告**，
不要靜默沿用（頁面的 `P.warn[]` 就是幹這件事）。

### 4.4 檔案是 Big5

瀏覽器端 `reader.readAsText(f, "big5")`。用 UTF-8 讀會讓中文欄位變亂碼
（數值不受影響，但 `Name=` 之類會壞）。

---

## 5. 要即時（非快照）需要什麼

現在的 LIVE 是**快照回放**：StateRecord 是「按下擷取的那一刻」。
要做到真正即時（畫面跟著機台跑），有兩條路：

| 做法 | 需要 | 代價 |
|---|---|---|
| **A. handler 週期寫一個小 JSON** | handler 端加一個 timer，把 task／馬達／各處在籍寫成 `system\MotionView.json`；頁面每 200~500 ms fetch | 要改 handler；但格式自由、資料最完整 |
| **B. 頁面輪詢現有檔案** | 不改 handler，靠已經在寫的 log／dat | 覆蓋率與延遲受限於既有寫檔時機，Loader/Auto 逐格仍然沒有 |

⚠ 本機檔案的 `fetch()` 受瀏覽器 `file://` 限制，實機上要嘛用 `http://localhost` 起一個極簡靜態服務，
要嘛把頁面掛在 handler 自己的 web 端點下。**這一段還沒實作，屬提案。**

<!-- preserved-content:end -->
