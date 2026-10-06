> 保存來源：`.claude/skills/ht9045-html-version/references/qwerty-keyboard.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 彈出式小鍵盤（qwerty.js / HTQwerty）

`page/qwerty.js` — 模擬 BCB6 的 `TfQwertyKey`（`myQwertyKeyBoard.cpp`）觸控小鍵盤。
供各 HTML 畫面在點欄位時彈出，依呼叫旗標顯示不同版面（數字 / 全 QWERTY / 密碼）。

- 對應原始碼：`myQwertyKeyBoard.cpp` `ShowQwertyKey()`、旗標常數 `cmydef.cpp`
- 全域物件：`window.HTQwerty`

## N_ 旗標（`cmydef.cpp:346`）

以位元 OR 組合傳入。`HTQwerty.N.INTEGER` 等對應下表。

| 旗標 | 值 | 畫面 / 行為 |
|---|---|---|
| `N_INTEGER` | 0x0001 | 只有整數 → 數字鍵盤（± 步進鍵、`%`、`-`；步進鍵的字照 dp，見下） |
| `N_DOUBLE` | 0x0002 | 浮點 → 數字鍵盤含小數點（dp 只決定步進鍵的字，不四捨五入） |
| `N_NO_SYMBOL` | 0x0004 | 停用特殊符號鍵 |
| `N_PASSWORD` | 0x0008 | 遮罩 `*`、隱藏 Current Value |
| `N_NO_SPACE` | 0x0010 | 無空白鍵 |
| `N_UPPERCASE` | 0x0020 | 大寫優先 |
| `N_NO_NUM_PAD` | 0x0040 | 全鍵盤不含右側數字鍵盤（實機寬 745 vs 910） |
| `N_PORT` | 0x0080 | 通訊埠（自動 0~65535） |
| `N_IP_ADDR` | 0x0100 | IP（含小數點） |

## 模式邏輯（`ShowQwertyKey(Ptr, iFunction, iDP, bCheckRange, min, max)`）

`myQwertyKeyBoard.cpp:169` 的分支，HTML 版忠實對應：

- **數字類**（`INTEGER|DOUBLE|PORT|IP_ADDR`）：只顯示數字鍵盤（實機 `this->Width=290`）；`palQwertyKey` 隱藏。
- **文字/密碼**（其餘）：全 QWERTY；`palNumKey` 顯示（除非 `NO_NUM_PAD`）；`NO_SPACE` 隱藏空白鍵；`UPPERCASE` 大寫優先；`PASSWORD` 以 `*` 遮罩。
- **Current Value / Maximum / Minimun**（`palValueLimit`）：`PASSWORD` 不顯示 Current Value；`bCheckRange` 才顯示 Maximum/Minimun（大的那個是 Maximum，:259-271）；
  夾限在 OK 與 Abort 共用的尾段（:285-301），只有 `INTEGER|DOUBLE` 且 `bCheckRange` 才夾。`N_PORT`：min<0 或 max<=0 時強制 0~65535（:249-257；min／max 預設 0，.h:153）。
- 數字鍵盤上的 ± 步進鍵、`%`、`-` 僅在 `INTEGER|DOUBLE` 且非 PASSWORD 顯示；小數點 `.` 僅 `DOUBLE|IP_ADDR`；BS／Del／Abort／OK 只在數字類顯示；
  golden 藏起來的鍵（Visible=false，:190-202）是空格，不是灰掉的鍵。

## 按鍵行為（AI(W906-KB-GOLDEN) 20261004 起照 golden `myQwertyKeyBoard.cpp`）

- **開窗整段反白**（FormShow :137-143 SetFocus＋TEdit AutoSelect）：第一個字元鍵（數字／字母／符號／空白）**取代**舊值（spbKeyClick :320-339），之後接著打。
- **`-`** 是整段正負號切換（spbMinusClick :421-431），不是在尾巴加 `-`；**BS** 刪最後一個字（舊值反白時也是刪舊值的最後一字，:341-345）；**Del** 清空（:352-355）。
- **`%`** 只輪換 ± 步進鍵的刻度 dp 0→1→2→3→0（INTEGER 只有 0／1，spbPercentClick :368-377 → ChangeDecimalPoint :379-418），數值與反白都不動。
- **步進鍵的字**：每次開窗 `iDecimalPoint=dp`（:185）：dp 0＝`+10 +100 +1000 / -10 -100 -1000`、dp 1＝`+1 +10 +100`…、dp 2＝`+1.0 +0.1 +0.01`…、dp 3＝`+0.1 +0.01 +0.001`…；
  switch 沒有的 dp（golden 自己傳 4、6）保留這個鍵盤上一次的字。按下＝atof(畫面)+atof(鍵上的字)，INTEGER `int()` 截斷、DOUBLE `"%1.6f"`（:433-449）。
- **OK／Enter** 與 **Abort** 走同一段尾段：Abort 先放回開窗時的原值（spbCancelClick :362-366）再夾；寫回欄位（Abort 只在結果跟原值不同時寫），
  OK 一律寫回並呼叫 onCommit（引擎靠它補發 input／change）；Abort 一律呼叫 onAbort，夾完不同時再呼叫 onCommit。
- **沒有標題列 ✕**（dfm:4 `BorderIcons=[]`），**點遮罩不關**（:283 `ShowModal`）；離開只有 Abort／OK（Enter）。
- 開著又來一個 `show()` → 疊第二個（fQwertyKey2，:171-178）；兩個都開著再來 → 不理並回 onAbort（:180-181）；同一個欄位被要第二次（兩個綁定者）→ 後來的取代前一個。
- 頁面自己把最上層的 `.qkOv` 拿掉 → 那一個算關了（onAbort 一次、不寫欄位），下面那一個馬上可以用（20261004 KB-GOLDEN 2/2，`settle()`）。

## 版面

- **數字鍵盤（`.qkNp`，golden palNumKey dfm:102-520）**：`7 8 9 +a -a` / `4 5 6 +b -b` / `1 2 3 +c -c` / `0 . - %` / `BS Del Abort OK`；`%` 與 OK 佔兩格寬；步進鍵的字照上面的 dp。
- **全 QWERTY（`.qkQ`）**：數字符號列 + qwerty 三列 + `文A`(大小寫) `⌫` `Delete`，末列 `空白 / Abort / Enter`；`N_NO_SYMBOL` 停用符號鍵。
- **Value Limit（`.qkLim`）**：Current Value（唯讀，取目標現值）＋ Maximum / Minimun。

## 實體鍵（使用者 20260915 規則 B，`docs/web-client/REPLICATE.md` §2.5，不可違反）

`qwerty.js` 本身沒有實體鍵的程式；`ht9045_wire_engine.js` 的 `physicalKeys()`（對照表 `PHYS_MAP`）在 `.qkOv` 存在時把 keydown 轉成「字相同的那顆鍵」的 `click()`，
所以實體鍵＝按畫面上那顆鍵（上面的 golden 行為照樣成立）：可見字元（大小寫容錯）、Backspace→`⌫`／`BS`、Delete→`Delete`／`Del`、Enter→`Enter`／`OK`、
Escape→`Abort`（規則 B 要的；golden 不理 Escape）、Space→空白鍵（畫面上有才算）。對不到按鈕的鍵在小鍵盤開著時被吃掉（golden `ShowModal`：鍵碰不到後面的畫面），
只放 F1～F12、Ctrl／Alt／Meta 組合鍵、單獨修飾鍵；小鍵盤沒開時不處理（欄位 readonly）。

## API

```js
HTQwerty.show(target, flags, { dp, checkRange, min, max });
// target：寫回對象（input 寫 value；其他元素寫 textContent；null=僅顯示不寫回）
// flags ：N_ 旗標位元 OR
// dp    ：golden iDP —— 只決定 ± 步進鍵的刻度（dp 0＝±10/±100/±1000），OK 不照 dp 四捨五入
// checkRange/min/max：顯示 Max/Min，OK 與 Abort 的尾段夾限（INTEGER|DOUBLE 才夾）
// onCommit(v)／onAbort()：OK 一律 onCommit；Abort 一律 onAbort，放回原值夾完跟原值不同時再 onCommit
```
- OK / Enter：（`INTEGER|DOUBLE` 且 checkRange 時夾限後）寫回 target，關閉。
- Abort：放回原值、同一段尾段（有範圍就夾），跟原值不同才寫回；關閉。沒有 ✕，點遮罩不關（golden `ShowModal`）。
- overlay 追加到 `document.body`（疊第二個時插在第一個前面、z-index 100000，`document.querySelector('.qkOv')` 拿到的就是最上層），每頁各自載入 `qwerty.js`。

## 與既有 Input 表單的關係

`INPUT.dfm` 的 `TfInput *fInput`（`MyInputBox`、`MyFloatInputBox`、`MyDoubleInputBox`、`MySuperInputBox`）可由 `HTQwerty` 取代：以 `N_INTEGER` 或 `N_DOUBLE`、`dp` 與 `checkRange/min/max` 表達原本的數值輸入、步進與範圍限制。

`InputForm.dfm` 的 `TfInputForm *fInputForm` 只可**部分**以 `HTQwerty` 取代：`ShowMyInput()`、`ShowMyInputSkip()` 與 `ShowMyInputTrayID()` 的數字/Tray ID 欄位可共用鍵盤；`ShowMyInput1()` 則包含 OCR 圖片、條碼字串長度與 `CheckOCRWordType()` 驗證，仍需保留獨立 HTML 流程與相同驗證，不可僅以鍵盤視為完成替代。

## 接線範例

| 呼叫處 | 模式 | 依據 |
|---|---|---|
| 主畫面 `edSoakTime`（Soak Time 秒） | `N_INTEGER, dp0, range 5~10000` | main.cpp:26024 |
| 主畫面 `btLogin`（登入） | `N_NO_SYMBOL｜NO_SPACE｜PASSWORD` | main.cpp:34957 |
| 溫度設定 | `N_DOUBLE, dp2, checkRange` | main.cpp:26129 |
| cBinSel 存檔 `btnSave` | `N_NO_SYMBOL｜NO_SPACE｜PASSWORD`（取代 alert） | cBinSel.cpp:2988 |

### cBinSel 數值格 / Enable 格（`Sete*` 事件對應）

Setup.BinSel.html 依列名對應正確模式（`NUM_KB` / `ENABLE_KB`）：

| 列（row） | 事件 | 模式 |
|---|---|---|
| Double Contact | `SeteDoubleContact` | `INTEGER 0, 0~iD22DoubleContactCount+2` |
| Yield % Bin（Enable V） | `SetePersentEnable` | `DOUBLE 2, 0~100`（切 ON 跳鍵盤，值寫入 Yield % Number） |
| Yield Ignore Cnt | `SetePersentIgnore` | `INTEGER 0, 1~100000` |
| Yield % Number | `SetePersentNumber` | `DOUBLE 2, 0~100` |
| Count Bin（Enable V） | `SeteCountEnable` | `INTEGER 0, 1~100000`（→ Count Number） |
| Count Ignored / Count Number | `SeteCountIgnore/Number` | `INTEGER 0, 1~100000` |
| Spc. Bin By Arm/Socket（Enable V） | `SeteSpecialBinBy…` | `INTEGER 0, 1~100000`（→ Spc. Cnt …） |
| Spc. Cnt By Arm/Socket | `SeteSpecialBinCountBy…` | `INTEGER 0, 1~100000` |
| Site Gap % / By Arm Site Gap% | — | `DOUBLE 2, 0~100` |

- **num 列**：點可編輯輸入 → 依 `NUM_KB` 開鍵盤。
- **Enable(V) 列**：切 ON 後（`td.bincell.on`）依 `ENABLE_KB` 開鍵盤，值寫入 `writeTo` 指定的值欄同 bin 格。
- **Save**：capture 階段 `stopImmediatePropagation` 攔截原 alert，改開密碼鍵盤。

## CSS 類別（qwerty.js 動態注入 `#qkCss`）

`.qkOv`(遮罩) / `.qkWin`(視窗) / `.qkBar`(標題列) / `.qkDisp`(顯示欄) / `.qkLim`(Value Limit) /
`.qkKbs`(鍵盤區) / `.qkQ`(QWERTY) / `.qkNp`(數字鍵盤) / `.qk`(按鍵，`.k` 藍字功能鍵、`.ok`/`.ab`、`.dis` 停用)。

## 展示

IDE.WidgetTemplates.html 第 12 區「TfQwertyKey」：N_ 旗標表 + 用法 + 6 顆模式鈕（整數 / 浮點 / 全鍵盤 / 無符號 / 無數字鍵盤 / 密碼）+ 目標欄。

<!-- preserved-content:end -->
