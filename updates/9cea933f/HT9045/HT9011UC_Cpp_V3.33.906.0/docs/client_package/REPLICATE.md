# 怎麼把其他頁面也接上機台　—　依 Contact 那頁類推

> **Contact 那頁已經接好、而且在真機台上測過會動**（2026-09-14 驗證通過）。
> 這份講「照同一個形狀，把其他頁面也接起來」。

參考實作：`client\ht9045_contact_wire.js`（13,584 bytes，有完整註解）
對照表草稿：`docs\CONTACT_FIELD_MAP.md`

---

## 0. 先看懂 Contact 那頁做了什麼

只有三個東西：

```
Setup.Contact.html
  └─ <script src="ht9045_recipe_client.js">   ← 通訊層，已寫好，不用改
  └─ <script src="ht9045_contact_wire.js">    ← 這一頁專屬的接線
        ├─ FIELD_MAP   頁面 id → [配方區段, 鍵]
        ├─ load()      讀配方 → 填欄位
        └─ save()      收集 → preview → 人確認 → write → 重讀驗證
```

**每一頁要新寫的只有一支 `*_wire.js`，而且九成是那張 `FIELD_MAP`。**
`load()` / `save()` 的邏輯直接抄，不用重想。

---

## 1. 最重要的一步：對照表不要用猜的

這是整件事唯一會出錯的地方。**猜錯會把錯的值寫進機台配方。**

Contact 那頁的對照表**不是靠欄位名稱相似度配出來的**——我試過，268 個 id 只配到 42 個，
而且明顯有錯（`btOffset` 被配到 `fSocketInitialICCheckPositionOffset`，`lblDieForce`
這種標籤也被配上）。那條路不能走。

**正確作法：去 BCB6 原始碼抽。** 對應關係本來就寫在那裡。有兩種抽法，**先試一跳的**。

### ★ 方法 A（優先）　`HTEditList` 一跳法

九成的表單用 `HTEditList` 註冊控件，**一行就帶齊四樣東西**：

```
elTrayForm->Add(edBoatYDivision, &UserDefForm_File[0].iCassetteZItem, ECPosInt, "Type0", "Cassette Z Item", ...)
                ^頁面 id          ^C++ 成員                            ^型別     ^區段     ^鍵
```

抽法：`grep -a 'el[A-Za-z_]*->Add(' <golden .cpp>`

**這比方法 B 嚴格更好**，因為它多給了**型別**（`ECText` / `ECPosInt` / `ECDouble` /
`ECBool` / …）——那正是決定「這個欄位該用 `el.value` 還是要處理 select/radio」的資訊。
Contact 那頁留下 12 個 pending，就是因為兩跳法沒有型別。

實測 golden 全樹有 **1,974 個** `el*->Add(` 呼叫（單位：呼叫數），分佈在 10 個檔：

| golden .cpp | Add 數 | 對應頁面 |
|---|---|---|
| `cConfiguration.cpp` | 1,579 | `Setup.Configuration`（⚠ 目標是 `config.ini`，不在配方資料夾，見下） |
| `uteach.cpp` | 203 | `HW.teach`（⚠ **永不自動接線**，那是教導表） |
| `cTrayForm.cpp` | 56 | `Setup.TrayForm` |
| `fAOI.cpp` | 49 | 無對應頁面 |
| `cLd_ULd.cpp` | 29 | `Setup.Ld_ULd` |
| `ProductionInfo.cpp` | 24 | 無對應頁面 |
| `LaserSensor.cpp` | 15 | 無對應頁面 |
| `BarCode.cpp` | 12 | `Setup.BarCode`（⚠ 目標在配方資料夾外） |
| `VacuumUnit.cpp` | 6 | 無對應頁面 |
| `atester_ProcessCount.cpp` | 1 | — |

### 方法 B（備援）　`ReadIniData` 兩跳法

表單沒用 `HTEditList` 時（Contact 就是這種）才走這條：

```
第一跳   cContact.cpp:  <成員> = ReadIniData(szDir, "<區段>", "<鍵>", <預設值>)
第二跳   cContact.cpp:  <widget>->Text = ... <成員> ...
                              ↓
                    widget id → 區段 / 鍵
```

Contact 那頁這樣抽出 **47 筆**，比對頁面 id **47 筆全部命中、零遺漏**——
因為頁面是照 `cContact.dfm` 忠實建的，id 跟 Delphi 元件名一致。

⚠ 兩跳法拿不到型別，所以 select / radio group 會變成 pending。能用方法 A 就不要用它。

---

## 2. 三條規則，不要破壞

這三條寫在 `ht9045_contact_wire.js` 裡，抄的時候一起抄：

### 規則 1　寫入前一定 `preview`，把 `changed` 給操作員看

```js
const p = await HT9045Recipe.preview(DOC, edits);
// 把 p.changed 攤開顯示，由人按確認才繼續
```

不要做「按下去就寫」的按鈕。操作員要看到「這次會改哪幾個值」。

### 規則 2　**配方感知**：只送這個工單真的有的鍵

> ⚠ **20260914 重寫。** 原本這條寫的是「`notFound` 非空就拒絕寫入」。
> **那個設計是錯的**，而且是實測發現的。

**為什麼錯**：同一個頁面在不同工單上，配方文件裡的鍵集合**本來就不一樣**。
實測 Contact 那頁的 35 個鍵，掃全機 63 份 `Contact.Data`：

```
24 個   全部 63 份都有
 9 個   只有部分工單有 —— Drop_DropContactMode 只在 6/63、AutoKSHTOfs 7/63、
        三個 Air Purge 12/63
 2 個   一份都沒有 —— golden 讀它們時帶預設值 0.0，是選配參數
```

**那是正常的，不是缺陷。** 但舊設計把每個鍵都送出去，只要工單缺一個，
`notFound` 就非空，**整頁的儲存被拒**。實測：**63 個工單裡有 57 個會存不了檔。**

而且那條把關本身沒有鑑別力——**一個幾乎必然觸發的檢查，跟一個永遠不會觸發的
檢查一樣沒用**。

**正確做法**：

```js
// 載入時記錄「這個工單真的有哪些鍵」
liveKeys[sec + ' ' + key] = true;      // 只在 doc.sections[sec][key] 存在時

// 沒有的欄位：停用 + 標 N/A（不要只留白）
//   一個空白但可編輯的欄位看起來像「值是空的」，操作員會往裡面打字

// 儲存時只送 liveKeys 裡的
if (!liveKeys[sec + ' ' + key]) return;   // 本工單沒有，不送
```

這樣 **`notFound` 由構造保證為空**。它一旦非空，就真的是 `FIELD_MAP` 錯了——
**那才是一個有鑑別力的檢查**。

參考實作見 `client\ht9045_contact_wire.js` 的 `load()` / `collect()`。

### 規則 3　寫完一定重讀回填

```js
await HT9045Recipe.write(DOC, edits);
await load();          // ← 這一步不能省
```

伺服器回報成功**不等於**你手上的物件是權威狀態。重讀一次才知道機台真正存了什麼。

### ⚠ 還有一條：互鎖不要寫在 JS 裡

該擋的由 C++ 端擋。**兩層互鎖會互相遮蔽**——當 JS 這層先擋下來，C++ 那道就永遠不會被
測試到；等到 JS 這層被繞過或改壞，沒有人知道下面那層還在不在。

一層、在正確的位置，比兩層可靠。

---

<!-- AI(W906-BA-DOCS) 20260915：本節來自網頁同事 20260915 交付的 REPLICATE.md，原文收錄。
     它記的是使用者 20260915 對輸入途徑的裁決，不是建議。 -->

## 2.5 ⚠ 輸入途徑規格（使用者 20260915 定案，不可違反）

**機台上沒有實體鍵盤。** 由此衍生兩條硬規則：

### 規則 A　HTML 畫面本身不可以使用實體鍵盤

所有文字輸入框一律設 `readonly`，唯一的輸入途徑是點一下欄位叫出 QWERTY 小鍵盤。
這與 golden 一致——`.dfm` 上是 `OnMouseDown = <handler>`，單擊就開。

⚠ 不要「為了開發方便」把 `readonly` 拿掉。20260914 曾經拿掉過一次，前提是
「開發機有實體鍵盤所以讓它能直接打字」，那個前提在機台上不成立。

### 規則 B　只有小鍵盤出現時，才可以用對應按鍵的實體鍵

小鍵盤開著的時候，實體鍵盤上「小鍵盤畫面上有的那些鍵」要能用：

| 實體鍵 | 對應小鍵盤按鈕（全 QWERTY ／ 數字鍵盤） |
|---|---|
| 可見字元（0-9 / a-z / 符號） | 文字相同的那顆（大小寫互相容錯）；數字鍵盤上的 `-`、`%`、`.` 也是 |
| Backspace | `⌫` ／ `BS` |
| Delete | `Delete` ／ `Del` |
| Enter | `Enter` ／ `OK` |
| Escape | `Abort` |
| Space | 空白鍵（畫面上有空白鍵才算：數字鍵盤與 `N_NO_SPACE` 沒有） |

**小鍵盤上沒有的鍵一律不接受**，小鍵盤沒開時實體鍵完全無效。

實作在 `client/ht9045_wire_engine.js` 的 `physicalKeys()`：偵測 `.qkOv` 覆蓋層存在時，
把 keydown 轉成對應按鈕的 `click()`，用**按鈕上的文字**比對，不依賴 `qwerty.js`
的內部結構。

> **20261004 KB-GOLDEN（`AI(W906-KB-GOLDEN)`）**：畫面上的鍵改成照 golden `TfQwertyKey`
> （`myQwertyKeyBoard.cpp`，906 樹與 V912 位元組相同）——開窗時整段反白、第一個字元鍵**取代**舊值、
> `-` 是整段正負號切換、`%` 只輪換 ± 步進鍵的刻度（不再除以 100）、`dp` 決定步進鍵的字（dp 0＝±10／±100／±1000）、
> OK 與 Abort 走同一段尾段（`N_INTEGER`／`N_DOUBLE` 且有範圍才夾；Abort 先放回原值）、沒有標題列 ✕、點遮罩不關。
> 實體鍵＝按畫面上那顆鍵，所以自動照 golden。同日 2/2 補上表中數字鍵盤的 `BS`／`Del`／`OK`
> （以前數字鍵盤上 Backspace／Delete／Enter 沒作用；golden 自己的 Enter 也是關窗＝OK，`KeyDown :463-466`），
> 並且小鍵盤開著時對不到按鈕的鍵**吃掉**（不給後面的畫面）——golden 小鍵盤是 `ShowModal`，鍵碰不到後面的表單；
> 放過去的話，數字鍵盤上按 Space 會觸發後面還有焦點的按鈕（例如剛按過的 Go）。
> 只有 F1～F12、Ctrl／Alt／Meta 組合鍵、單獨的修飾鍵照舊給瀏覽器。Escape＝Abort 是規則 B 要的（golden 沒有）。

⚠ **為什麼不做在 `qwerty.js`**：那是網頁作者的檔案，存在於鏡像來源，
`sync_web.py --apply` 會整檔覆蓋，改在那裡會被洗掉。
**長久解法是網頁作者把這段收進 `qwerty.js`**，屆時引擎這份可以移除。

### 小鍵盤的旗標與夾限要從 golden 抽，不要自己設

```
golden  ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)   （.cpp）
        OnMouseDown = <handler>                                     （.dfm）
web     HTQwerty.show(target, flags, {dp, checkRange, min, max})
```

參數一一對應。⚠ 兩個已知陷阱：

1. **min/max 是 C++ 執行期變數時**（`InputLimit.dContactHigh` 之類）**一律關掉夾限**，
   不要填猜的數字。猜錯的夾限會擋掉合法輸入或放過非法輸入，比沒有夾限更危險。
2. **golden `myQwertyKeyBoard.cpp:252` 有一條 `if (min<0 || max<=0) { min=0; max=65535; }`**。
   用在負值欄位上會把輸入夾成 0——`HW.teach` 的行程值是 `-500 ~ -7000`，
   照抄那條會讓教導值根本打不進去。**這類一律關掉夾限**（欄位仍是數字鍵盤）。

覆蓋率紀錄在 `docs/KEYBOARD_AUDIT.md`，由 `scratchpad/kb_audit.py` 產生，可重跑。

---

## 3. 型別：不是每個欄位都能直接接

Contact 那頁 47 筆裡，**只接了 35 筆純文字輸入框**。剩下 12 筆刻意留白，原因值得你知道，
因為你會遇到一樣的：

| 型別 | Contact 頁的數量 | 難在哪 |
|---|---|---|
| `<input type="text">` | 35 | 沒難處，直接 `el.value` |
| `<select>` | 3 | 配方存**整數**，頁面是選項——哪個數字對哪個選項要逐個確認 |
| radio group（`<fieldset>`） | 3 | 同上。而且 **`rgKitDiameter` 的選項清單會依機種改變**（`fContact.cpp:191-203` 與 `:344-360` 各 `Add` 一組不同字串），所以 ItemIndex 對應的實際口徑**不是固定的** |
| radio 配對 | 2 | `rbNormal` / `rbDummyMode` 是同一個鍵的兩極 |
| 抽取誤配 | 4 | 機械抽取把四個 id 指到同一個鍵，明顯錯誤，要回原始碼逐個確認 |

**建議的節奏**：一次接一個型別，接完在機台上「讀一次 → 改一個值 → 存 → 再讀一次」，
確認往返都對，再接下一個。**不要一次接十二個然後一起測**——出錯時你會分不出是哪一個。

---

## 4. ⚠⚠ 同步會洗掉接線，這件事一定要處理

`tools\sync_web.py --apply` 用 `shutil.copy2` 從你的來源目錄覆蓋到 `D:\HT9045\web`。

| 檔案 | 同步後會怎樣 | 為什麼 |
|---|---|---|
| `page\ht9045_recipe_client.js` | ✅ 活著 | 已列入 `OURS` |
| `page\ht9045_contact_wire.js` | ✅ 活著 | 已列入 `OURS`（20260914 加的） |
| `page\Setup.Contact.html` 裡那兩行 `<script>` | ❌ **會被洗掉** | 這個檔在你的來源目錄裡有，所以會被整檔覆蓋 |

**而且洗掉之後不會報錯**——頁面照常打開，只是欄位不會填值、Save 沒反應。是靜默失效。

### 唯一的長久解法

**把那兩行加進你自己的來源檔**（`D:\HT9050\docs\機台介面 HTML\HT9045\page\Setup.Contact.html`），
放在 `</body>` 之前：

```html
<script src="ht9045_recipe_client.js"></script>
<script src="ht9045_contact_wire.js"></script>
```

順序很重要：**client 一定要先載入**。`ht9045_contact_wire.js` 開頭會檢查，
沒載入就在畫面底部印紅字，不會靜默失敗。

每接一頁就在那一頁做一樣的事。

---

## 4.5 ★ 排序過的工作清單（2026-09-14 測繪）

> **數字來源與可信度**：以下每頁的欄位數是一支唯讀 agent 抽的，它拿 Contact 那頁的
> 人工驗證結果當校準（recall 42/47 = 89%，已解析鍵的一致率 28/28，純文字欄位高估約 11%）。
> **把每個數字當成 ±15%**。「哪個檔對哪個頁面」與「一跳法可用」是我親自驗過的。

### 🟢 第一梯：可以直接接（不影響機台運動）

| # | 頁面 | golden 來源 | 配方文件 | 純文字欄位 | 備註 |
|---|---|---|---|---|---|
| 1 | `Setup.Temp_Set` | `uTemp_Set.cpp` | `Temperature.Data` | ~117 | **單一最大收穫**。⚠ 見下方前置條件 |
| 2 | `Setup.TesterIF` | `cTesterIF.cpp` | `Tester.Data` | ~60 | 逾時／GPIB 位址。排除 `RPDefault.ini` 與 `D:\RS232Standard\...` 那兩處讀取，它們不在配方裡 |
| 3 | `Setup.YieldMonitoring` | `uYieldMonitoring.cpp` | `Tester.Data` | ~77 | 門檻值會停批／關站，但不動機構。鍵覆蓋率最弱：只有 16/77 在全部 63 個配方裡都有 |
| 4 | `Setup.SCK_ART` | `Automation\SCK_ART.cpp` | `Tester.Data` | ~12 | 小而乾淨 |
| 5 | `Data.StartCondition` | `cStartCondition.cpp` | `HandlerCondition.Data` | ~4 | 很小 |
| 6 | `Setup.QAMode` | `QAMode.cpp` | `Tester.Data` | ~2 + 4 select | 很小 |

**⚠ 第 1 項動手前必查**：`uTemp_Set.cpp:4216-4220` 在
`bSaveTemperatureByMachine && bA57_2SaveTemperatureByMachine` 成立時會改存到
`D:\HT9045\IniData\SaveByMachine\`（`common.cpp:122`）。**這台如果開著那個旗標，
配方 API 改的就是錯的檔案。** 先確認再接。

### 🟡 第二梯：抽取乾淨但會影響機構——**可以建對照表，但不要自己上線**

| # | 頁面 | golden 來源 | 為什麼要人 |
|---|---|---|---|
| 7 | `Setup.TrayForm` | `cTrayForm.cpp` | **全樹抽取最乾淨**（48/48 一跳、全是 input:text、鍵全在 `Tray.Data`），但每個鍵都是放置幾何（X/Y 起點、間距、Pick Up、分割數） |
| 8 | `Setup.Speed` | `cSpeed.cpp` | `ArmCondition.Data` 是各機構的 XY 速度／加速度／Z 升降速度 |
| 9 | `Setup.Cleaning` | `AutoClean\uCleaning.cpp` | 含 `iAutoClean_MotorSpeed[0..2]`、`iAutoClean_ContactShiftHeight` |
| 10 | `Setup.SetUp` | `cSetUp.cpp` | 13 個純文字裡有 9 個是運動相關（站距 X/Y Pitch） |
| 11 | `Setup.HotPlate` | `cHotPlate.cpp` | 7 個純文字裡 6 個是幾何；且 `HotPlate.Data` 每個配方差異最大（9/10 鍵不同） |
| 12 | `Setup.Ld_ULd` | `cLd_ULd.cpp` | 氣缸等待時間——夾持延時太短會在鎖定前就移動料盤 |
| 13 | `Setup.TrayAssignment` | `cTrayAssignment.cpp` | 5 純文字 + 11 select，select 要逐個確認整數對應 |

### ⛔ 需要先改伺服器，不是改頁面

| 頁面 | 為什麼 |
|---|---|
| `Setup.Configuration` | **全機最大機會**（1,579 個 Add、988 個 widget 全在頁面上、而且有型別），但目標是 `D:\HT9045\config\config.ini`，**不在配方資料夾內**，現在的 API 到不了 |
| `Setup.BarCode` | 12 個 widget 指向 `D:\HT9045\config\UnloaderClipSetting.ini`，同樣在配方資料夾外 |

### 🔴 永遠不要自動接線

**`HW.teach`** ← `uteach.cpp` → `D:\HT9045\system\teach.ini`。140 個 widget、203 個 Add、
全是 input:text——**全樹第二乾淨，也是運動風險最高的一頁**。它就是教導表本身
（原始軸座標、LoadCellY/Z、AutoCleanPick）。

### 🚫 兩跳／一跳都不適用

- **`Setup.BinSel`**：`cBinSel` 用變數當區段名（`"Bin Func FT"`／`"Bin Func RT"`…），
  524 個呼叫的 GroupName 是變數不是字面值。頁面只有 17 個 id。
- **`Data.LotInfo`**：3,966 bytes 的**假頁**，7 個泛用 id（`lotNo`／`deviceName`／`runMode`），
  不是從 `uLotInfo.dfm` 建的——「頁面照 .dfm 建」這個假設在這裡不成立。

### 沒有對應頁面的 golden 表單

`cTrayMapping` / `AutoAlignment` / `fAOI` / `OCR` / `FixAICCD` / `RFID` / `Magazine` /
`ShuttleMove` / `ContactForce` / `ArmOffsetData` / `fRotate` / `uTrayEditForm` /
`TesterTCP` / `LaserSensor` / `VacuumUnit` / `ProductionInfo` / `ASE_K Socket` / `AGV`

---

## 5. 一頁的完整檢查清單

- [ ] 找到這一頁對應的 golden `.cpp`，抽出 (區段, 鍵) → widget 對照
- [ ] 比對頁面 id，記下**配不到的**（可能是版面元件，也可能是漏的欄位）
- [ ] 複製 `ht9045_contact_wire.js`，改檔名、改 `DOC`、換掉 `FIELD_MAP`
- [ ] 純文字欄位先接，其他型別列進 `PENDING` 並寫明疑點
- [ ] 兩行 `<script>` 加進**你的來源檔**（不是只加在 `D:\HT9045\web`）
- [ ] 新的 `*_wire.js` 加進 `sync_web.py` 的 `OURS`
- [ ] 在機台上測：讀 → 改一個值 → 存（看確認框列的是不是你改的那個）→ 重讀
- [ ] 確認 `notFound` 是空的。不是空的代表對照表有錯，先修對照表

---

## 6. 測試時要知道的一件事

目前 F5 啟動的參數帶了 `--production-recipe`，代表**配方 API 讀寫的是真實量產配方**，
不是暫存副本。程式碼有確認框擋著（一定先 preview、列出會改什麼、按確定才寫），
但存下去就是真的。

要用暫存副本測，把 `--production-recipe` 拿掉即可（`--dry` 仍在，會 scratch-copy）。
