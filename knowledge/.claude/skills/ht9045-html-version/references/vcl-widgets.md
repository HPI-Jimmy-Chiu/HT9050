# VCL 自訂元件轉 HTML（hwidgets.js / HTWidgets）

`page/hwidgets.js` — 把 HT9xxx 的**自訂 VCL 元件**（`elec/myvcl` 等）轉成可重用的 HTML maker 函式。
載入時自動注入共用 CSS；全域物件 `window.HTWidgets`。展示頁：`IDE.WidgetTemplates.html`（Debug ▾「🧩 元件模板」）。

> 與產生器 `_gen_dfm_abs.py` 的關係：**基礎元件**（TALed/TBtnPanel/TTMyTray/TSpeedButton…）產生器解析 dfm 時**直接輸出 HTML**（不呼叫 hwidgets）；
> **執行期動態 class**（Omron/MotorTest/Home/Security/Yield…）產生器以 `RUNTIME_INJECT` 注入。hwidgets 主要供 **WidgetTemplates 展示**與**手工頁**（Main.html 的 SitePanel/PanelMain6）使用。
> ⚠️ 產生器的 Python builder 與 hwidgets maker 為**平行實作**，改版須兩處同步。
> **20260919 起新增的執行期面板改走宣告式**：產生器 `widget_host()` 只放
> `<div class="htWidgetHost" data-maker data-spec>`，由新檔 `page/page-widgets.js`（**載入順序必須在
> `hwidgets.js` 之後**，由 `PAGE_EXTRA` 掛上）呼叫 maker 展開——座標只留在 hwidgets.js 一份，不再兩邊各寫一份。
> 目前 `makeContactForceGroup`（Setup.ContactForce）與 `makeVacuumPanel`（HW.VacuumUnit）走這條；
> 舊的 `omron/motortest/home` 三支仍是 `RUNTIME_INJECT` 直接吐 HTML 的平行實作。

## 共通約定

- 每個 maker 回傳一個 DOM 元素，`opt.name` → `id`。
- `title` 一律含 dfm 屬性（`Port/Bit/Type/Alias/Ring/IP/IsISA/XItem…`），滑鼠移上可見（release 模式 theme.js 會移到 `data-htitle`）。
- 對應 VCL 方法的操作函式：LED `setValue(v)`（ChangeValue）、BtnPanel `setDown(v)`（SetPanelStatus）、SpeedButton `setGlyph(url)`、Labeled `setCaption(s)`。
- `opt.left/top` 有給則絕對定位（複合面板用）。

## 基礎 VCL 元件

| maker | VCL 原始碼 | 主要 opt | 說明 |
|---|---|---|---|
| `makeALed` | `elec/Component/aled.pas` | `value, blink, interval, ledStyle, trueColor, falseColor, alias` | LED 本體；6 種 `ledStyle` 對應 `TLEDStyle`；`blink` 用 CSS animation（Interval=半週期）；`trueColor`預設 clLime、`falseColor` clSilver |
| `makeMyLed` | `myvcl/MyLed.h` | ＋`port, bit, type` | TALed ＋ IO 屬性（`.ledbox` 包裝，可帶 alias/caption 文字） |
| `makeMyLedLane` | `myvcl/MyLedLane.h` | ＋`ring, ip, isISA` | TMyLed ＋ Lane 網路屬性 |
| `makeLabeledALed` / `makeMyLabeledLed` / `makeMyLabeledLedLane` | 虛擬元件（尚未實作於 BCB6） | ＋`caption, captionPos(lpLeft/lpRight/lpTop/lpBottom), captionColor, captionBold` | LED＋Caption 四方位排列（`.lledf`） |
| `makeBtnPanel` | `myvcl/butPa1.h` | `caption, down, trueColor/falseColor, trueFontColor/falseFontColor, port, bit, type, alias, flat, onChange` | 可按壓面板按鈕（`Style=tsFlatButtons`→`flat`）；點擊切 Down 對應 `SetPanelStatus()` |
| `makeBtnPanelLane` | `myvcl/BtnPanelLane.h` | ＋`lane, ip, isISA` | TBtnPanel ＋ Lane 網路屬性 |
| `makeSpeedButton` | VCL `TSpeedButton` | `caption, img(glyph URL), w, h, fs, layout(left/top), flat, onClick` | Glyph＋Caption 置中（`blGlyphLeft/blGlyphTop`） |
| `makeMyTray` | `myvcl/HTray.h` | 見下方 TTMyTray | Tray 格盤 |

### 6 種 LED 樣式（`TLEDStyle`）
`LEDSmall`（圓小）/`LEDLarge`（圓大）/`LEDSqSmall`（方小）/`LEDSqLarge`（方大）/`LEDVertical`（直條）/`LEDHorizontal`（橫條）。

### TTMyTray（`makeMyTray`）
```js
{ name, xitem, yitem, xblockItem, yblockItem, xblockWidth, yblockWidth,
  cellW, cellH, lineWidth, edgeWidth, trayColor, frameColor,
  trayDirect:'csNull|csLeftTop|csLeftBottom|csRightTop|csRightBottom',
  showFont, colorMap:[...], cells:[{x,y,colorIndex,text}], onCellClick(x,y) }
```
- `xitem/yitem`＝欄列數；`xblockItem/yblockItem`＝每 n 格加分隔（`xblockWidth/yblockWidth` 間距）。
- `cells[].colorIndex` 查 `colorMap`（對應 `SetCellColorIndex`）；`cells[].text`（`SetCellNumber`，`showFont:false` 不顯示）。
- `trayDirect` 於角落畫黃色方向標（`.dirmark`）；`onCellClick(x,y)` 點格回呼。
- **注意**：實機 Tray 為整體元件，拖曳/選取時 theme.js 的 `pick()` 回傳整個 `.traypos` 容器（勿個別選格）。

## 執行期動態 class（`.cpp` 內 `new`，dfm 無）

| maker | VCL 原始碼 | 主要 opt | 對應 |
|---|---|---|---|
| `makeOmronPanel` | `EJ1N/MyOmronPanel.cpp` | `ch, pv, degree, runStop, at, inputErr, event, sv, left, top` | Omron 溫控單通道面板（fOmron 每通道 `new`） |
| `makeMotorTestRow` | `uMotorTest.cpp` | `label, v1, v2, left, top` | Motor Test 單列（cbUsing＋labName＋edPos1/2） |
| `makeHomeRow` | `uhome.cpp` | `label, pos, on, left, top` | Home Monitor 單列（labName＋ledHome＋edPos） |
| `makeATPanel` | `AutoTemperature.cpp` | `index, label, temp, channel, enabled, left, top` | ATC 量測點面板（GroupBox＋青底溫度＋channel＋Use） |
| `makeSecurityRow` | `cSecurity.cpp` | `caption, level, levels[], img, width, left, top` | 權限項目列（Glyph＋RadioGroup 等級 Open/Operator/Engineer/Supervisor/HonPrec） |
| `makeContactForceGroup` | `ContactForce.cpp` | `name, dia, variant, pos, left, top, width` | 接觸力 Load rate 群組，**四變體**（20260919 依 dfm 原型重寫）；見下「THTSLKClass 家族」 |
| `makeVacuumPanel` | `VacuumUnit/MyVacuumPanel.cpp` | `name, caption, cur, event, threshold, sv, left, top` | 單一吸嘴真空面板 81×177（20260919 新增）；**執行期畫法≠設計期**，見下「TMyVacuumPanel」 |
| `makeYieldPanel` | `uYieldMonitoring.cpp` | `bins, left, top` | Bin 良率面板（mtTrayItem/mtTrayName 雙欄＋BinSetting ScrollBox 勾選矩陣＋ART limit combo） |

## THTSLKClass 家族（`makeContactForceGroup`，20260919 重寫）

`ContactForce.cpp` 執行期依 Recipe 的 Kit 直徑清單，往四個 ScrollBox 各堆一疊 `Align=alTop` 的群組盒。
`variant` 參數選四個變體，各自對應 dfm 裡一個**設計期原型 GroupBox**（座標全部抄原型，不再估）：

| `variant` | class | dfm 原型 | 尺寸 | 列數 |
|---|---|---|---|---|
| `std` | `THTSLKClass` | `gbLoadRate` | 842×100 | 兩列（第二列是 `contact offset__NS`） |
| `ind` | `THTSLKIndClass` | `gbLoadRateInd` | 842×61 | 一列 |
| `dieforce` | `THTDieForceSLKClass` | `gbDieForceLoadRate` | 850×65 | 一列 |
| `dieforce1` | `THTDieForceOneByOneSLKClass` | `gbDieForceOneByOneLoadRate` | 842×61 | 一列 |

- TrackBar 抄 dfm：`Left=108 Width=275 Height=35`、`Min=80 Max=150 Position=80`
  → 對應 Load rate **0.80 ~ 1.50**；`edtLoadRate` 是 `Position/100` 的**唯讀**顯示（dfm `Enabled=False`），
  拖 TrackBar 會同步。
- **舊版是錯的**（20260919 前）：trackbar 寫死 `left:130 / width:300`、只畫兩個編輯框、offset 標籤擺錯邊，
  全部是估出來的，與 golden 對不上。現已全部改抄 dfm——**改這支時請對照 dfm 原型，不要回頭用估的**。
- 直徑清單來自 Recipe（`CheckAndReadIniData "SLK Type"`）；產生器用 golden 內建預設值
  （`ContactForce.cpp:447/579`），`Visible=0` 那一個 golden 仍會 `new` 但 `GroupBox->Visible=false`，HTML 也不畫。

## TMyVacuumPanel（`makeVacuumPanel`，20260919 新增）

`VacuumUnit/MyVacuumPanel.cpp`，單一吸嘴的真空面板，`VACUUM_UNIT_WIDTH=81` / `VACUUM_UNIT_HEIGHT=177`
（`VacuumUnit.h`）。座標抄 dfm 的設計期原型 `GroupBox1`（`Caption='myPalSamle'`，`Visible=False`）。

⚠ **關鍵：執行期與設計期的畫法不一樣，模板照執行期畫。**
原型 GroupBox 裡的三個 TPanel（`pnlCurectVal` / `pnlEvent` / `pnlThreshold`）**只是樣式來源**，
執行期 `TMyVacuumPanel` **不會複製它們**——只 `new` 一個 `TImage`（`ImgVacuumPanel`，`Left=4 Top=20`，73×152），
那三行字是 `Canvas->TextOutA()` 畫上去的。所以 HTML 用一個 `.vacImg` 區塊放三行置中文字，
而**不是**三個 panel。照原型畫會多出三個實際不存在的元件，且日後接 tag 時會找不到對應物件。

- 三行文字：目前值（clMaroon `#800000`，15px）／Event（clBlack，11px）／閥值（clBlue `#0000ff`，15px），
  對應 `ShowCurectVal()` / `SetEvent()` / `ShowThreshold()`。
- 其餘元件照 dfm：`edSV`（`Left=5 Top=72` 71×20，閥值輸入，範圍 −116~148）、
  `btnSV`（`Left=6 Top=92` 71×23，Caption `Set`）、`bplOn`/`bplOff`（TBtnPanelLane，`Top=120` 22×24，`^` / `v`）。
- 顏色取自 dfm：底 `14540252=#DCDCDC`、外框 clGray；`bplOn/bplOff` `Color=10307329=#014A9D`、
  `TrueColor=14464261=#45B0DC`。
- 排列由 `page-widgets.js` 依 `pitchX=81 / pitchY=177` 自動排成 4 欄×2 列
  （`USE_46_SUCKER_DB=0` → `iIndexColMax=iInOutColMax=4`）。

> 兩者都已加進 `IDE.WidgetTemplates.html`：THTSLKClass 是 **§10 改寫**、TMyVacuumPanel 是**新的 §13**。

## CSS 類別（載入時注入）

`.aled`（＋`.on/.blink/.LEDxxx`）、`.ledbox`（LED＋文字）、`.lledf`（Labeled＋`.lpLeft/lpTop/lpBottom`）、
`.btnpanel`（＋`.down/.flat`）、`.mytray`（＋`.trow/.cell/.dirmark.csXxx`）、
`.slkgroup`（Load rate 群組盒）、`.vacpanel`＋`.vacImg`（真空面板與其 Canvas 文字區）。CSS 變數：`--led-on/--led-off/--led-interval`、`--bp-true/false(-font)`、`--tray-color/line/edge`。

## 動態 class 記錄狀態

`ScreenShots.html` 的 `window.DYNAMIC_CLASSES`（`_scan_dfm_shot.py`）記 9 個動態 class 的 `inVCL`（是否已入本頁）；
`TQwertyKeyClass` 標 `na:true`（僅封裝畫面便於操作、免實作，鍵盤實體為 `qwerty.js`／HTQwerty）。

## 陷阱

- **平行實作同步**：新增/改 maker 時，產生器的 Python inject 版（omron/motortest/home/security/yield…）與 hwidgets maker 需一起改。
- **file:// 快取**：Chromium 對 `hwidgets.js` 有快取，改後驗證需完整 `reload({waitUntil:'load'})`。
