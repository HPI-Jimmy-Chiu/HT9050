---
name: ht9050-motionview-layout
description: >
  HT9050（HP-9050）Motion View HTML 版面調整與對應。當要移動／新增／刪除機構模組方塊、
  改流程箭頭順序、換出料軌、綁定馬達軸、或把畫面對回 layout PDF 與實機教點時使用。
  涵蓋 page/Main.MotionView9050.html 的資料契約、JSON/MotionView9050-layout.json 欄位、
  版面編輯模式操作、機種切換鏈路（Machine-profile.json → settings.js → background.html）、
  以及 HTML-only 邊界（BCB6 無 HT9050，runtimeSupported:false）。
  觸發關鍵字：HT9050, HP-9050, MotionView9050, Main.MotionView9050.html,
  MotionView9050-layout.json, Machine-profile.json, 版面編輯, 匯出版面 JSON,
  機種切換, ?machine=, HT9050_Debug.cmd, debug9050.html, 模組方塊, 流程箭頭, 出料軌。
applyTo: "**/Main.MotionView9050.html, **/MotionView9050-layout.json, **/Machine-profile.json"
---

# HT9050 Motion View 版面調整（HTML-only）

> ⚠ **本頁與其資料完全不影響實機**。BCB6 `MachineType.h` 沒有 `Type_HT9050`，
> `Machine-profile.json` / `MotionView9050-layout.json` 一律 `runtimeSupported:false`。
> 幾何座標是依 layout PDF 等比推估的**示意值**，不得用於教點或工程判讀。

## 兩個版本，兩份 reference

| 版本 | 檔案 | 用途 | 詳見 |
|------|------|------|------|
| **純模擬動畫** | `page/IDE.MotionView9050-Concept.html` | 不載任何 JSON，雙擊即開，**載入後就自己跑動畫**；用來跟硬體人員確認版面、流程、互鎖；可由 hash 覆寫秒數 | [references/concept-animation.md](references/concept-animation.md) |
| **UPH 計算** | `page/IDE.MotionView9050-UPH.html` | 動作表填秒 → 平行路徑模型 → UPH 表／圖，內嵌概念動畫 | skill [ht9050-uph-model](../ht9050-uph-model/SKILL.md) |
| **串接 JSON（正式頁）** | `page/Main.MotionView9050.html` | **由 build 腳本自概念頁產生**：同一套立體圖／欄位，再加 JSON 覆寫層（`applyLayout / applyForms`）、機種閘門（非 HT9050 只顯示提示）、LIVE 投影（`applyLive`）。**載入後不自己動**，動不動由 `Production-update.json` 決定（比照 `Main.MotionView.html`）。需 `theme.css / theme.js / settings.js` | [references/json-runtime-wiring.md](references/json-runtime-wiring.md) |
| **版面編輯器** | `page/IDE.MotionView9050-LayoutEditor.html` | 平面圖，拖曳模組 rect 後匯出 `MotionView9050-layout.json`（原 Main 頁的編輯功能搬到這裡） | 本文 §1 |

## Scripts

| 腳本 | 用法 |
|------|------|
| [scripts/verify-concept.js](scripts/verify-concept.js) | 開概念頁 → DevTools console 貼上執行；檢查料件守恆、Hot plate 格點＝卡片＝chips、大 IC 不重複、Loader 盤格、吸嘴持料指示 |
| [scripts/regen-and-check.ps1](scripts/regen-and-check.ps1) | 改完 JSON/JS 必跑：重跑 shim、`node --check`、`runtimeSupported:false` 檢查、UTF-8 無 BOM |
| [scripts/build-main-motionview9050.py](scripts/build-main-motionview9050.py) | **概念頁改完必跑**：從 `IDE.MotionView9050-Concept.html` 產生 `Main.MotionView9050.html`（插入 JSON 覆寫層／閘門／LIVE）。工作副本 `D:\AI_TempFile\_build_main_motionview9050.py`。**不要手改 Main 頁**，改概念頁或 build 腳本再重跑 |

## 檔案地圖

| 檔案 | 角色 | 可否手改 |
|------|------|----------|
| `D:\HT9045\page\Main.MotionView9050.html` | 正式頁：概念頁立體圖＋動畫＋欄位 ＋ JSON 覆寫／機種閘門／LIVE | ❌ **由 build 腳本產生**，已列入 `_gen_dfm_abs.py` 的 `NO_OVERWRITE` |
| `D:\HT9045\page\IDE.MotionView9050-Concept.html` | **概念圖**（正式頁的來源）：斜投影立體圖＋12 步動畫＋下方欄位；**不載入任何 JSON**，雙擊即開 | ✅ 手工頁；改完重跑 build |
| `D:\HT9045\page\IDE.MotionView9050-LayoutEditor.html` | 版面編輯器（平面圖拖曳／匯出 layout JSON） | ✅ 手工頁 |
| `D:\HT9045\JSON\MotionView9050-layout.json` | **版面唯一資料來源**（v1.1.0：modules rect、`stations`、`strokes`、`defaults`盤形、`flow.steps[].sec` ＋ `label`、`axes.bindings` ＋ `axes.absent`） | ✅ 可手改，或用編輯器「⤴ 匯出版面 JSON」覆蓋 |
| `D:\HT9045\JSON\Machine-profile.json` | 機種能力集（caps / motionView.page / motors.hidden / io.hiddenGroups） | ✅ 可手改 |
| `D:\HT9045\JSON\js\*.js` | file:// 傳輸墊片 | ❌ 自動生成，**改完 JSON 必跑產生器** |
| `D:\HT9045\debug9050.html`／`HT9050_Debug.cmd` | HT9050 啟動入口 | ✅ |

**改完任何 JSON 一定要跑：**

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

沒跑就在 `file://` 下讀不到新值（Edge 封鎖 XHR，靠 `JSON\js\*.js` 墊片）。

---

## 1. 調版面的兩種方式

### A. 頁面內拖曳（快、適合對圖）

1. 開 `file:///D:/HT9045/page/IDE.MotionView9050-LayoutEditor.html?mode=debug&machine=HT9050`（平面圖編輯器；正式頁 `Main.MotionView9050.html` 是斜投影，不提供拖曳）。
2. 按 **✥ 版面編輯**。
3. 操作：

   | 操作 | 效果 |
   |------|------|
   | 滑鼠拖曳方塊 | 移動（即時更新 `rect[0]`、`rect[1]`） |
   | 方向鍵 | 選中方塊微調 10 mm |
   | Shift + 方向鍵 | 微調 50 mm |
   | `+` / `-` | 同時放大／縮小寬高 10（Shift 50） |
   | 勾「格線」 | 100 mm 細線／500 mm 粗線，用來對 PDF 比例 |
   | 滑鼠停在方塊上 | tooltip 顯示 `id（no.N） rect=[x, y, w, h] mm` |

4. 按 **⤓ 匯出版面 JSON** → 下載 `MotionView9050-layout.json`
   → 覆蓋 `D:\HT9045\JSON\` → 重跑 `_gen_json_shim.py`。

> 「版面編輯」與「匯出」按鈕帶 `class="debugOnly"`，release 模式自動隱藏。

### B. 直接改 JSON（精準、可 review）

改 `MotionView9050-layout.json` 的 `modules[].rect`，存成 **UTF-8 無 BOM**，再跑 shim 產生器。

---

## 2. `MotionView9050-layout.json` 欄位契約

```jsonc
{
  "footprintMm": { "w": 2650, "h": 1790 },   // SVG viewBox 尺寸；改機台外形才動
  "tracks": ["Loader","Auto1","Auto2","Auto3","Empty"],
  "modules": [
    {
      "id":     "InShuttle",      // 唯一鍵；流程 from/to 與軸綁定都用它
      "no":     2,                // layout PDF 的編號徽章；0 或省略＝不畫
      "name":   "In Shuttle",     // 方塊上顯示的字（會自動縮字／換行／直立）
      "kind":   "shuttle",        // 決定底色，見下表
      "option": false,            // true＝虛線框＋灰字，可由工具列勾選隱藏
      "rect":   [230, 150, 330, 430]   // [x, y, w, h]，單位 mm，原點在機台左上
    }
  ]
}
```

### `kind` → 底色對照

| kind | 用途 | 底色變數 |
|------|------|----------|
| `base` | Tray Table 底板 | `--mvbase` |
| `track` | Loader / Auto1~3 / Empty | `--mvtrack` |
| `heat` | Hot plate | `--mvheat` |
| `shuttle` | In / Out Shuttle | `--mvsht` |
| `index` | Index | `--mvidx` |
| `test` | Testing 區 | `--mvtest` |
| `rail` | In P&P / Out P&P 龍門軌 | `--mvrail` |
| `sensor` | Shuttle line sensor 等 | 固定 `#e4e9f0` |
| `opt` | Option 模組 | `--mvopt`（虛線框） |

新增 kind 要同時改 `Main.MotionView9050.html` 的 `fillOf()`。

### 標籤自動排版規則（`putLabel()`）

- 先試單行；字級 = `min(34, (w-14) / (字數 × 0.58))`。
- 塞不下（< 12px）就依空白斷成兩行，取字級最大的切點。
- **窄高方塊**（`w < h × 0.45`，如 In P&P / Out P&P 軌）自動旋轉 −90° 直立排。
- 字級一定寫成 **inline `style="font-size:..."`**——CSS class 的 `font-size` 會贏過
  SVG presentation attribute，用 attribute 設會完全無效（此坑已踩過）。

### 編號徽章

半徑 `r = clamp(11, min(22, h×0.20, w×0.20))`，畫在方塊左上內側 `(x+r+5, y+r+5)`。
方塊太小會自動縮徽章，不會壓到字。

---

## 3. 流程（`flow`）

```jsonc
"flow": {
  "steps": [
    { "id":"load", "label":"① Loading", "from":"Loader", "to":"HotPlate", "actor":"InPP" }
  ],
  "unloadTargets": ["Auto1","Auto2","Auto3"]
}
```

- 箭頭畫在 `from` / `to` 兩個模組的**中心點**之間，`from`/`to` 必須是存在的 `modules[].id`。
- 已走過的段落實線加粗（`--mvic`），未走到的虛線淡色。
- 工具列「出料軌」選單由 `unloadTargets` 產生；選了之後
  `outsht` 的 `to` 與 `unload` 的 `from`/`to` 會即時改寫（見 `flowSteps()`）。
- `actor` 目前只是註記（哪個機構在動），尚未驅動繪圖；要用它做機構動畫時再擴充。

HP-9050 標準流程（layout PDF 第 3 頁）：
`Loader 吸料 → Hot plate 加熱 → In Shuttle 送至 Index → Index test → Out Shuttle 出料 → Unload 分類放料`

---

## 4. 軸綁定（`axes.bindings`）

```jsonc
// 在籍軸
{ "module":"InShuttle", "motorNo":"M11", "motorId":"MInShuttle1", "axis":"x",
  "label":"In Shuttle X", "note":"進出測區" }
// HT9045 有、HT9050 沒有的軸（軸讀數表以淡出列顯示）
{ "motorNo":"M12 / M18", "motorId":"MInShuttle2 / MOutShuttle2", "note":"HT9050 無第二組 Shuttle" }
```

`Main.MotionView9050.html` 的**「軸讀數」表整份由這裡產生**：`applyLayout()` 用
`axes.bindings`＋`axes.absent` 覆寫 JS 內的 `AXES` 預設，`renderAxes()` 依序畫在籍列
（`type:"cylinder"` 第二欄顯示「氣缸」）與 `axis-disabled` 淡出列。**要增刪軸改 JSON，不要改 JS**。
`motorNo` 只是 HT9045 `Mot_Table.csv` 編號，供對照用，非 HT9050 定案編號。

位置數值（pulse / mm）仍是 `–`，尚未接上 `Motor-runtime.json`。要接動態位置時：

1. 產生 `JSON/Sim-scale.9050.json`（`source.toolchain` 必須寫 `"HTML-only"`，
   **不可**寫 `"BCB6"`——`_gen_simscale.py` 的來源 `cinitial.cpp SetSimuScreenPara()`
   沒有 HT9050 分支）。
2. 在 `Main.MotionView9050.html` 加 `motionview-sim.js` 或自寫線性換算。
3. `Machine-profile.json.profiles.HT9050.motionView.simScale` 已預留檔名欄位。

HT9050 的軸事實（提案 §1.1）：

| 項目 | HT9050 |
|------|--------|
| Index | **只有 1 個 Z 軸**（無 Y1 / Y2 / Z2） |
| Shuttle | In / Out **各自獨立馬達** |
| Out Shuttle | **X ＋ Y 兩軸**（Y 為新增軸 `MOutShuttleY`） |
| Out P&P | X 長軌伺服 ＋ **Y 懸臂氣壓缸（兩段，非伺服）** ＋ Z ＋ Rotator |
| Picker | **1 Picker**，吸嘴**內建 rotator** |
| Track | 5 軌（Loader / Auto1 / Auto2 / Auto3 / Empty） |

---

## 4b. 行程與互鎖範圍（畫動畫／改流程前必讀）

> 完整表格見提案 §1.3。以下是做畫面時最容易畫錯的四條。

### Out Shuttle

| 軸 | 停位 | 互鎖 |
|----|------|------|
| X `MOutShuttle1` | 測區位（與 In Shuttle **共用同一個 DUT 停位**）↔ 出測區位 | Index Z 未回安全高度 → 不可進出測區；In Shuttle 在測區 → 不可進入 |
| Y `MOutShuttleY` | 基準位 ／ **取料位（往前微降，供 Out P&P 接料）** ／ TOP AOI 拍攝位（往後，Option） | **只在出測區後**才可離開基準位；X 在測區時 Y 鎖基準位 |

⚠ 常見錯誤：把 Y 畫成「每個 cycle 都動」。標配機的 Y **只在出測區後往前微降到取料位**；
往後的掃描行程**只有 TOP AOI（Option）** 才會發生。概念頁用 `OPT_TOP_AOI` 旗標分開這兩者。

### Out P&P

| 軸 | 停位 | 互鎖 |
|----|------|------|
| X `MOutArmX` | Out Shuttle 取料位 ↔ Auto1/2/3（＋Bottom AOI Option 站） | Z 未回安全高度 → 不可移動；**不得越過 Index／In 側** |
| **Y 懸臂（氣壓缸）** | **二值**：縮回＝Out Shuttle 側／伸出＝Tray 側，**無中間位** | Z 需在安全高度才可切換；Out Shuttle Y 未到取料位 → 不可縮回接料 |
| Z | 安全 ／ 取料（Shuttle KIT）／ 放料（Tray） | Y 懸臂動作中不可下降 |
| Rotator | 依 Recipe；4S AOI 時配合防掉落夾爪 | 夾爪未張開 → 不可旋轉（p.11） |

⚠ 常見錯誤：把 Y 懸臂當伺服軸做線性內插。它是**氣壓缸快動**，畫面上要用兩段切換
（概念頁以橘色粗虛線段 `.beam.cyl` 表示，並標「伸出→Tray／縮回→Out Shuttle」）。

### 共用 DUT 停位互斥（p.24 In PUT／Out PUT）

```
In Shuttle 進測區 → Index 取料 → In Shuttle 出測區
                          ↓
                  Index 下壓 DUT 測試
                          ↓
Out Shuttle 進測區 → Index 放料 → Out Shuttle 出測區
```

- 任一時刻**只有一台 Shuttle 可在測區**。
- Index Z 下降期間兩台 Shuttle 的 X 全鎖。
- Out Shuttle 進測區前先由 **Out Shuttle Z Sensor** 確認 KIT 無殘料。

### In P&P 補料節奏（HP 不可長時間空置）

單吸嘴；**In Shuttle 一回到入料側（SHT_BACK 結束）就立刻把 Hot plate 已達 soak time 的最舊一顆放上 In Shuttle KIT（IN_KIT），
接著回 Loader 取 1 顆補回 Hot plate（IN_PICK→IN_HP）**；三步與測區鏈（TEST→OSHT_IN→…）**平行**，不在 Hot plate 上方空等。
因此每個正式 cycle 起點（SHT_IN）In Shuttle KIT 上已經有料。
開機：`HP_CAP` 個暖機 cycle（只有 IN_PICK→IN_HP）→ 1 個 kit0 cycle（第一次 IN_KIT＋補料，測區仍待命）→ 正式 cycle。
概念頁 12 步（主鏈順序）：
`SHT_IN → IDX_PICK → SHT_BACK → TEST → OSHT_IN → IDX_PLACE → OSHT_OUT → OUT_PICK → OUT_PLACE`，另 `IN_KIT → IN_PICK → IN_HP` 從 SHT_BACK 結束起平行。

各步秒數可由 URL hash 覆寫（UPH 頁就是這樣餵）：`#IN_KIT=3&TEST=10&hpc=2&hpr=2&trc=2&trr=1`。
取放時序、A/B/C 三顆料的格位對應、盤點守恆公式（`LOT = HP_CAP + 1 + AUTO_CAP`）→ [references/concept-animation.md](references/concept-animation.md) §4–§5。

---

## 5. 機種切換鏈路（要動這條才需要看）

```
?machine=HT9050  ─┐
launchOptions ────┼→ settings.js resolveMachine()
Version.Model 前綴 ┤     ↓
localStorage ─────┘  cache.machine = HTSettings.machine()
default                   ↓
             background.html：WINDOWS 的 motionview.src 換成 profile.motionView.page
                          ↓
             withMode() 為每個 iframe 附加 &machine=
```

| 檔案 | 關鍵位置 |
|------|----------|
| `page/settings.js` | `resolveMachine()`、`load()`／`loadSync()` 都要載 `Machine-profile.json`、`HTSettings.machine()` |
| `background.html` | `MACHINE_ID`／`MACHINE`、`withMode()` 附加 `&machine=`、`WINDOWS.forEach` 前一次性覆寫 `motionview.src`、預載 `<script src="JSON/js/Machine-profile.js">` |
| `debug9050.html` | 寫 `sessionStorage.ht9xxx-launch-options = {mode,simulator,machine}` 後 redirect |

**解析順序刻意把 `localStorage` 排在 `Version.Model` 之後**，避免開過一次 HT9050
就永遠黏在 HT9050。若要強制指定，用 `?machine=`。

---

## 6. 驗證清單（改完必跑）

1. `powershell -ExecutionPolicy Bypass -File .github\skills\ht9050-motionview-layout\scripts\regen-and-check.ps1`
   （重跑 shim、`node --check`、`runtimeSupported:false`、UTF-8 無 BOM）
2. 開 `file:///D:/HT9045/page/IDE.MotionView9050-Concept.html`（**不帶參數**），console 貼上
   `scripts/verify-concept.js` → 必須 `✅ verifyConcept PASS`
3. 概念頁或 build 腳本有改 → `python D:\AI_TempFile\_build_main_motionview9050.py` 重產 Main 頁，再對內嵌 script 跑 `node --check`。
   開 `file:///D:/HT9045/page/Main.MotionView9050.html?mode=debug&machine=HT9050`：
   - 頂部綠邊 `srcLead` 須列出 `MotionView9050-layout.json v1.1.0（N 模組）`，`modeTag`=`SIM`（或 `LIVE`），無 pageerror
   - console 貼 `scripts/verify-concept.js` → `PASS`（Main 頁用同一套引擎，驗證腳本共用）
   - `?machine=HT9045` → `body.gated`，只顯示提示與 `?machine=HT9050` 強制連結，`BOOTED===false`
   - 不帶 `?machine=` → 依 `Gerneral.ini Model` 前綴判定（目前 HT-9050 → 顯示）
4. 開 `file:///D:/HT9045/background.html?mode=debug&machine=HT9050`，在 console 確認：
   ```js
   MACHINE.id === 'HT9050'
   document.querySelector('#win-motionview iframe').getAttribute('src')
     === 'page/Main.MotionView9050.html?mode=debug&machine=HT9050'
   ```
5. **回歸**：把 `D:\HT9045\system\Gerneral.ini` 的 `[Version] Model` 改回 `HT-9046AT`
   （備份在 `Gerneral.ini.bak_20260909`），重跑 `_gen_ini_json.py` + `_gen_json_shim.py`，
   確認不帶 `?machine=` 時 `MACHINE.id === 'HT9045'` 且 `motionview` 回到 `Main.MotionView.html`。
6. **release 模式**：`?mode=release` 下編輯器的「版面編輯／匯出」按鈕必須消失；
   所有受模擬元件必須有 `id`（release 會把 `title` 移到 `data-htitle`）。
7. 1920×1080 檢視，不得出現方塊重疊或文字溢出框外。

---

## 7. 已知坑

| 坑 | 說明 |
|----|------|
| CSS `font-size` 蓋掉 SVG attribute | 字級一定要用 inline `style`，不可用 `setAttribute('font-size')` |
| 改 JSON 沒跑 shim | `file://` 下讀不到，狀態列會顯示紅色「載入失敗」 |
| `<g title="">` 不是 tooltip | SVG 要用子元素 `<title>`，本頁已改用 `<title>` |
| localStorage 機種黏著 | 已把 `localStorage` 排到 `Version.Model` 之後；若仍異常，手動 `localStorage.removeItem('ht9xxx-machine')` |
| 直接改 `Main.MotionView.html` | 那是 `_gen_dfm_abs.py` 的 `SUBTREE_JOBS` 自動產物（main.dfm `tsActionView`），會被覆寫；HT9050 一律只改 `Main.MotionView9050.html` |
| 把幾何當工程座標 | `rect` 是依 PDF 等比推估，**不是** teach.ini 位置；不可用來判讀教點 |
| Out P&P Y 當伺服軸插值 | 它是**氣壓缸兩段**，只能快動切換，沒有中間位 |
| Out Shuttle Y 每圈都動 | 標配只在**出測區後往前微降到取料位**；往後的掃描行程只有 TOP AOI（Option）才有 |
| Hot plate 長時間空盤 | In P&P 送料到 In Shuttle KIT 後必須**立即回 Loader 補料**，否則下一輪沒有達 soak time 的料 |
| Tray Status Unit 的名稱／LED 壓到格子 | 錨點要用盤面**前緣**`iso(x, y-depth/2, 0)`，放到盤外側 |
| Tray 格點用軸對齊 `rect` | 要用 `polygon` ＋ `localPlate()`，跟盤面同一斜投影的**平行四邊形** |
| Hot plate 上格點亮了又多一顆大 IC | 料在 Hot plate 上**只由格點表示**；`icBAt()` 放上後、`icCAt()` IN_KIT 前必須回 `null`；`cCell()=kittedBefore()%HP_CAP` 要跟 `applyPanels` 的 `first-1` 一致 |
| IN_KIT 提前到 SHT_BACK 之後後 Loader 數字跳動 | `taken` 不可用 `inPh>=2`（IN_KIT 之前的待命也是 3），要用 `inT(T)>=K.IN_PICK.b` |
| 從 Loader 取起的料直接算進 Hot plate | 吸起後到放下前都在**吸嘴**（chips `In P&P`＋inset 持料方塊），`IN_HP` 結束才併入 Hot plate |
| In P&P 還沒到 Loader，IC 就消失 | Loader 盤格在 **IN_PICK 完成**（idx>=1）才扣；大 IC 在 Z 到底（82%）才出現 |
| 手改 `Main.MotionView9050.html` 後被蓋掉 | 它是 build 腳本自概念頁產生的；改概念頁（畫面／動畫）或 build 腳本（JSON 層／閘門／LIVE）再重跑 |
| Python 三引號內寫 JS 的 `\"` | 會被 Python 吃掉變成裸 `"` → JS SyntaxError；JS 字串内的引號改用 `'`，且 build 後一律 `node --check` |
| `<p class="lead concept">` 被切錯 | build 腳本的 `cut()` 找第一個匹配；插入閘門（也用 `lead concept`）前要先切掉原概念頁的 lead |
| Production-update 的 motionView 是 HT9045 形狀 | `trays/ledGroups/trayLocations` 是 HT9045 runtime 契約；Main 頁只在 `motionView.machine==='HT9050'` 且 `available!==false` 才進 LIVE，否則維持 SIM 並註明已忽略 |
| `--panel` 與 theme.css 衝突 | Main 頁引入 theme.css，概念頁的 `--panel` 在 build 時改名 `--cpanel` |
| Main 頁一載入就自己跑動畫 | 那是**概念頁**的行為。正式頁 build 時把 `PLAY` 改成 `false`，`applyRuntimeState()` 每次收到 JSON 先 `stopSim()`；SIM 只剩 debug 手動播放（`.simctl.debugOnly`），`LIVE` 時三顆按鈕直接 `return` |
| 嵌在 background.html 時收不到 Runtime 更新 | `settings.js` 只有在**頂層視窗**跑 `refreshProduction()` 才發 `HT_SETTINGS_REFRESHED`；iframe 端收到的是 background 每 250ms（`event.seq` 有變才送）postMessage 的 `HT_SETTINGS`。**兩個都要聽**（`motionview-full.js` 就是這樣寫的） |
| 換佈景主題（dark／steel／contrast）畫面不動 | 概念頁自帶寫死色票；build 時把 `--bg/--cpanel/--ink/--dim/--line/--line2` 改成 `var(--form-bg)/var(--panel)/var(--text)/var(--text-dim)/var(--border)/var(--pane-border)`，並補一段 `:root[data-theme="dark"]` 壓暗機構色（`--plate*`／`--cell*`／`--mech` 這些 theme.css 沒有）。**不要在概念頁寫死回去** |
| 軸讀數表／Option 模組清單改了 JSON 卻沒變 | 這兩塊在 2026-09-09 之前是寫死在 JS 的；現在分別讀 `layout.axes` 與 `Machine-profile.layoutModules`，改完記得重跑 `_gen_json_shim.py` |
| Setup-current 是 HT9045 Recipe | `layout.defaults.useRecipeForms` 預設 false，盤形用 `layout.defaults`；有 HT9050 Recipe 後才改 true |

---

## 8. 相關資源

| 項目 | 路徑 |
|------|------|
| 提案（HTML-only 範圍、風險、分階段、§1.3 行程與互鎖） | `D:\docs\proposals\2026\20260909_HT9050_新增機種提案_廠內版.md` |
| 機台 layout | `D:\HT9045\layout\HT9050 layout.pdf` |
| 概念頁（純模擬）內部結構 | [references/concept-animation.md](references/concept-animation.md) |
| 正式頁（JSON 串接）資料契約 | [references/json-runtime-wiring.md](references/json-runtime-wiring.md) |
| Tray Status Unit 元件規格 | `D:\HT9045\page\IDE.WidgetTemplates.html` §4-1 |
| HTML Version 總則 | `d:\HT9045\.github\skills\ht9045-html-version\SKILL.md` |
| 開站 JSON 規範 | `d:\HT9045\.github\skills\ht9045-html-json\SKILL.md` |
| HT9045 版 Motion View（SIM/LIVE 完整模擬） | `d:\HT9045\.github\skills\ht9045-motionview-html-ui\SKILL.md` |
