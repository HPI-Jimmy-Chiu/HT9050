# 彈出式小鍵盤（qwerty.js / HTQwerty）

`page/qwerty.js` — 模擬 BCB6 的 `TfQwertyKey`（`myQwertyKeyBoard.cpp`）觸控小鍵盤。
供各 HTML 畫面在點欄位時彈出，依呼叫旗標顯示不同版面（數字 / 全 QWERTY / 密碼）。

- 對應原始碼：`myQwertyKeyBoard.cpp` `ShowQwertyKey()`、旗標常數 `cmydef.cpp`
- 全域物件：`window.HTQwerty`

## N_ 旗標（`cmydef.cpp:346`）

以位元 OR 組合傳入。`HTQwerty.N.INTEGER` 等對應下表。

| 旗標 | 值 | 畫面 / 行為 |
|---|---|---|
| `N_INTEGER` | 0x0001 | 只有整數 → 數字鍵盤（+1/-1…+100/-100、%） |
| `N_DOUBLE` | 0x0002 | 浮點 → 數字鍵盤含小數點（iDP 位） |
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
- **Current Value / Maximum / Minimun**（`palValueLimit`）：`PASSWORD` 不顯示 Current Value；`bCheckRange` 才顯示 Maximum/Minimun，並於 OK 時 `CheckRange()` 夾限。
- 數字鍵盤上的 `+1/-1/+10/-10/+100/-100`、`%`、`-` 僅在數字類且非 PASSWORD 顯示；小數點 `.` 僅 `DOUBLE|IP_ADDR`。

## 版面

- **數字鍵盤（`.qkNp`）**：`7 8 9 +1 -1` / `4 5 6 +10 -10` / `1 2 3 +100 -100` / `0 . - %` / `BS Del Abort OK`。
- **全 QWERTY（`.qkQ`）**：數字符號列 + qwerty 三列 + `文A`(大小寫) `⌫` `Delete`，末列 `空白 / Abort / Enter`；`N_NO_SYMBOL` 停用符號鍵。
- **Value Limit（`.qkLim`）**：Current Value（唯讀，取目標現值）＋ Maximum / Minimun。

## API

```js
HTQwerty.show(target, flags, { dp, checkRange, min, max });
// target：寫回對象（input 寫 value；其他元素寫 textContent；null=僅顯示不寫回）
// flags ：N_ 旗標位元 OR
// dp    ：小數位（DOUBLE）
// checkRange/min/max：顯示 Max/Min 並於 OK 夾限
```
- OK / Enter：（數字且 checkRange 時夾限後）寫回 target，關閉。
- Abort / ✕ / 點遮罩：取消關閉。
- overlay 追加到 `document.body`，每頁各自載入 `qwerty.js`。

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
