> 保存來源：`.claude/skills/ht9045-motionview-html-ui/references/verification.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 驗證清單與已知缺陷

這類頁面的 bug **看不出來** —— 動畫照樣會跑，只是料件悄悄消失或機構做出不可能的動作。
所以每次改完都必須跑機器驗證，不要只用眼睛看。

## 目錄

- [1. 怎麼跑驗證](verification.md#1-怎麼跑驗證)
- [2. 不變量清單](verification.md#2-不變量清單)
- [3. 已知缺陷（都真的發生過）](verification.md#3-已知缺陷都真的發生過)
- [4. 結構檢查](verification.md#4-結構檢查)

---

## 1. 怎麼跑驗證

```bat
node assets\smoke-test.js "<你的 html>"
python D:\HT9045\.claude\skills\make-report-skill\scripts\verify_animation_html.py "<你的 html>"
```

> 20261005 St01 查：`verify_animation_html.py` 在 `D:\HT9045\.claude\skills\make-report-skill\scripts\`、舊的 skill 位置都**不存在**，這一行目前跑不起來；先只跑 `smoke-test.js`。

`smoke-test.js` 用 DOM stub 在 node 裡把整條時間軸跑完（不需要瀏覽器）：
它會把每個機型組合（Y 變距 有/無 × Pitch Open-Close/Fixed）各跑 3000 格 `render()`，
`setAttribute` / `textContent` 一旦寫入 `NaN` 或 `undefined` 就立刻拋錯。

改參數後 op 數會變，**斷言若是為舊模型寫的就要一起改** ——
但要先確認是斷言過時，不是實作退步（實測踩過：把「16 列都要用到」當不變量，
低水位模型下自然用不到最後兩列，那是斷言錯不是程式錯）。

---

## 2. 不變量清單

### 2.1 料件守恆（最重要）

每一個快照都必須滿足：

```
Tray + InArm + HotPlate + In-Kit + Index + Socket + Out-Kit + OutArm + Bin + 已退盤 = 總數
```

實作方式：狀態只能透過 **move**（`{fk,fi,tk,ti,v}` 來源→目的）變更，
`doMove()` 是唯一的寫入點。這樣守恆是結構性保證，剩下的只是驗容量。

**再加一條更敏感的檢查**：replay 時若某個 move 的目的地已被佔用就報錯
（`assets/smoke-test.js` 的姊妹腳本做法，見 §3.1）。守恆數字有時會被兩個錯誤互相掩蓋，
「目的地已佔用」抓得更早。

### 2.2 容量

`arm ≤ NP`、`oa ≤ NP`、`hp ≤ NH`、`sk ≤ NP`、`bin ≤ NB`、每個 Kit ≤ NP。

### 2.3 動作者互斥

同一個 actor（IN / S1 / S2 / X1 / X2 / OUT / TRAY）的 op 在時間上不可重疊。

### 2.4 Socket 資源互斥

`IT`（Y→Socket）宣告 `res:"SOCK"`、`resDur = IT+IX+IB`，佔用區間不可重疊。
＝ 兩支 Index 不會同時在 Middle。

### 2.5 依賴滿足

每個 op 的 `a` ≥ 所有 `deps` 的 `b`。

### 2.6 取料規則：吸滿才放

`HPP` 動作生效前，若 Tray 還有料則吸嘴必須是滿的。
**一趟放料可以拆成多個 HPP**（上下排分拆），所以只檢查「該趟第一個 HPP」——
判斷方式：時間序上前一個 op 也是 HPP 就跳過。

### 2.7 Fixed（One by one）模式

`cfg.pmode===1` 時，每個 `TP` / `OPL` 的 `pk.length` 必須是 **1**，
且 Tray 下針次數 == 料件總數（見 §3.12）。有無 Y 變距不影響 Fixed 的結果。

### 2.8 列優先

Tray 取料的列號不可回頭（`min(該次的盤列)` 單調不減）。

### 2.9 幾何一致性

對每個 `TP` / `OPL`：

- 格數 == 吸嘴數
- 依吸嘴排分組後，**同一排必須落在同一個盤列**
- 吸嘴欄必須連續、盤欄以該次的 `kx` 遞增
- 配對模式：兩排支數相等、盤列差 == 該次的 `my`、盤欄與吸嘴欄一一對齊
- 每個 op 的 `pitch` 必須落在機構行程內

⚠ 這些檢查要用 **op 自帶的 `kx`/`my`**，不可假設固定步進（Open/Close 模式每針不同）。

### 2.10 HotPlate

- 有放過料的列一定要有被取過（不能有料卡在盤上）
- 靜止水位 == `峰值 − NP`（留一組 `HAS_NULL_IC`）；只有「放完還沒取」的瞬間才是峰值
  ⚠ 峰值**不是**盤面格數 —— 配不到對的列用不到（`16 列 / iYHalf=3` → 112 而非 128）
- **盤上還有 ≥ NP 顆時，不可以只取一部分**（見 §3.10）
- FIFO：每次取料的停留時間分布應該分散，不可集中在剛放的那幾組

### 2.11 兩條 Shuttle 的公平性

- 每批 Shuttle 移動次數應 ≈ **2**（右移一次、左移一次）。
  若是 4，表示每個動作各自要求停位、沒有把「同一停位能做的事」批次化
- `KIT` 分配到兩條 Shuttle 的次數應接近，中段應該乾淨交替
- 排空階段（Tray 空）尾端會偏向某一條是正常的 ——
  盤上剩什麼標記就只能餵那一條（標記在放料時就決定了）

---

## 3. 已知缺陷（都真的發生過）

### 3.1 時間順序 vs build 順序

**最容易漏的一類。** 狀態是依 `tau`（= `a + 0.55*dur`）排序後 replay 的，
不是依 build 順序。所以「build 時 A 在 B 前面」**不代表**時間上 A 先發生。

實例：`IPL`（Index 回填 Out-Kit）在 build 順序上排在 `OP`（OutArm 取走 Out-Kit）之後，
但 OutArm 是共用 actor、被別的批次卡住，時間上 `IPL` 反而先跑 → 回填到還沒取走的料上 → 漏料。
**修法**：把「清空該容器的 op id」記下來，加進後續回填 op 的 `deps`。

規則：**任何「先清空再填」的關係都必須寫成 deps，不能靠 build 順序。**

### 3.2 Kit 的 Site 槽位撞號

一個 Kit 每個 Site 只有一格。取料群組若沒檢查「上排 site == `SITE_MAP[0..3]`、
下排 == `SITE_MAP[4..7]`」，會取到「8 顆全是同一排 site」，放進 Kit 時兩顆搶同一格 → 漏料。

### 3.3 放料前吸嘴沒吸滿

「同一列尾端只吸到 1~2 顆就跑去 HotPlate」是物理上錯的（手上還有 6 支空吸嘴）。
修正後 HotPlate 放料趟數會直接掉到理論下限 `ceil(總數/吸嘴數)`。

### 3.4 HotPlate 取料用列號升序

會永遠在前幾排循環，後半段的料 soak 過頭卻永不取走。必須 FIFO。

### 3.5 配位游標沒有滾動

`SearchPlacePlateXItem*` 的 `iPlacePlateX/Y` 是持續游標。
若每次都從 (0,0) 重找，會導致最後 `iYHalf` 列永遠用不到
（因為那幾列只能靠「iy+iYHalf >= YDivision → 只放一排」的路徑用到）。

### 3.6 圖層順序把逐格點蓋掉

不透明盤面必須畫在逐格點**之前**。順序：`gS → gDot → gD → gT`。

### 3.7 手臂/機構停在干涉區

Tray Arm 若寫死停在 Auto1 正上方，就永遠壓在 OutArm 的作業區（Zone H）。
閒置件要停在干涉區外，並畫成虛線。

### 3.8 測試腳本自己的假設過時

改了模型後，為舊模型寫的斷言會誤報。判斷順序：
**先確認實作是否退步，再考慮改斷言**，並在改的時候把新的不變量寫清楚。

實例：把「所有列都要被取放」當不變量 → 低水位模型下最後兩列自然用不到；
把「兩排配對」的幾何檢查套到單排模式 → 誤報。

### 3.10 滿手走單排放料 → 生出取不滿的 team

**客戶直接看得到的病徵**：「HotPlate 尾盤明明還有 8 顆料，InArm 只取 4 顆就去放 Shuttle」。

根因在**放料端**：吸嘴滿手時若走了「只放一排」的路徑，上排落在第 R 列、
下排落在第 R+1 列，而吸嘴兩排固定差 `iYHalf` 列 → 這 8 顆永遠湊不回一組。
因為取料群組 == 放料群組（帳本 `PickFromHPList`），取料端無法補救。

**修法**：滿手只接受「兩排一起放」；游標到最後 `iYHalf` 列視同配位失敗、繼續前進。
單排放料只留給「Tray 已空、只剩一排有料」的尾批。

**驗證**：對每個 `HPK`，若 `pk.length < NP` 則該 op 開始前盤上料件數必須 `< NP`。
修正前 5 次違規（盤上 18/14/10/6 顆卻只取 4）→ 修正後 0 次。

### 3.11 用「剩幾格空」判斷放不放得下

最後 `iYHalf` 列的空格湊不成整組，用格數判斷會誤判有位 →
`searchPlace()` 回 null、手上還有料 → 主循環 `if(cnt(arm)>0){place;continue;}` 空轉。
必須實際探位（`hasFullSlot()`）。

### 3.12 把 Fixed（One by one）當成「一次多支、間距鎖定」

`rgInArmPitch` 的 Fixed 選項對應的 ini key 就叫 **One by one**，
`bVariModeFIX==true` 會直接讓 `bCanPick2ICAtOnceTime=false`，
`GetInArmToLoaderPosition_Single()` 再把其餘吸嘴（含另一排）全部關掉 ——
**一針只有一支吸嘴下去**。

誤讀成「只留 first-fit 那一個合法倍數、仍然一次 4 支」時，
Fixed 與 Open/Close 的統計會完全一樣（實測就是這樣才被抓到），
而正解是 Fixed 的下針次數 == 料件總數。

**驗證**：Fixed 模式下 `TP` 與 `OPL` 的 `pk.length` 必須恆為 1，
且下針次數 == 料件總數。

### 3.13 參數烤進頁面（架構錯誤）

「用腳本從機台檔案產出一段 JS、人工貼進 HTML」看起來能用，但換一台機就要改程式碼 ——
那不是模擬，是把當時那台機的數字畫成圖。

**正解**：頁面 runtime 自己讀機台檔案，`applyParams()` 是唯一參數入口，
連 `G`（站點座標）都由 `applyTeachGeometry()` 從 `teach.ini` 推。
判斷方法：**在頁面裡搜任何 recipe/teach 數字，只該出現在 `PARAM_DEFAULT` 一處。**

### 3.14 `var` 只提升宣告，不提升賦值

`applyParams()` 在檔頭就被呼叫，但 `var OFS={...}` 寫在後面的段落 →
呼叫時 `OFS` 是 `undefined` → `OFS.inX` 丟
`Cannot read properties of undefined (reading 'inX')`，
而錯誤訊息長得跟「`P.teach` 沒讀到」一模一樣，很容易查錯方向。

**規則**：`applyParams()` 用到的所有物件常數都要宣告在它的呼叫點之前。
排查時先確認是「被讀的那個」還是「拿去讀的那個」是 undefined。

### 3.15 機台 `.xls` 的值全是字串

BIFF2 記錄實測 100% 是 `LABEL(0x0004)`，連負數馬達位置也是字串。
取值只收 `typeof v === "number"` → 整張表讀成空的（踩過：`馬達筆數 = 0`）。
另外 INTEGER(`0x0002`) 的值在 payload `+7`，不是 `+5`。

### 3.16 LIVE 不可以用推算值補洞

`Loader / Auto / Fix 盤` 與 `Socket / Index` 的**逐格**在籍不在 StateRecord 裡。
畫成空盤會讓現場以為盤是空的 —— 比留白危險。頁面用斜線底紋標「不明」，
並在說明區列出缺口與 handler 端補 dump 的做法（見 [live-mode.md](live-mode.md) §3）。

### 3.9 對影片／畫面的顏色誤讀

**以程式碼為主，模擬畫面只是佐證。**
實例：把「已達 soak time」的顏色誤判成「已取走」——
因為 FIFO 下「先放的先加熱完」剛好也吻合放料順序，兩種解讀都對得上，
只能靠碼（或問人）分辨。誤讀後整個水位模型都會歪。

---

## 4. 結構檢查

`make-report-skill\scripts\verify_animation_html.py` 會檢查：

UTF-8 無 BOM、DOCTYPE/charset/lang/title、標籤配對、
所有 `getElementById()` 目標存在、所有 `var(--x)` 都在 `:root` 有定義、
無外部網域、`viewBox`、`overflow-x:auto`、`prefers-reduced-motion`、`@media print`、
`role="img"` / `aria-label`。

它有幾條檢查是比對函式名（`drawMech` / `occ` / `mechState`）的，
本模板函式名不同會出現 WARN —— **非阻斷**，因為那幾項的實質內容已由 `smoke-test.js` 直接驗過。

<!-- preserved-content:end -->
