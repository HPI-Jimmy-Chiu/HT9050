# ScreenShots.html 截圖索引 — 資料來源與更新流程

`page/ScreenShots.html` 是**手工維護的索引頁**（Debug ▾「實機截圖預覽」開啟），本身只有版面與繪製邏輯；
實際內容全部來自兩支**產生器**輸出的資料檔。**任何 dfm 完成 HTML 轉換後，必須同步更新本索引**，
否則會與實際狀態脫節（表單仍顯示未轉換、或截圖卡殘留）。

## 資料流

```
D:\HT9045\IMG\ScreenShot\*.png/jpg
        │
        ├─(_gen_screenshot_pages.py)→ page/shot/<Form>.html          （各未轉換表單截圖頁）
        │                              page/shot_windows.js  → window.SHOT_WINDOWS（截圖卡/桌面視窗）
        │
原始碼所有 *.dfm ─(_scan_dfm_shot.py)→ page/screenshot_meta.js → window.ALL_DFM（表單狀態表）
                                                                  window.DYNAMIC_CLASSES（動態生成 class）
        │
        └────────────→ page/ScreenShots.html（讀上述 3 個 window.* 繪製卡片＋兩張表）
```

## 兩支產生器（工作副本 `D:\AI_TempFile\`，保存版 skill `scripts/`）

### 1. `_gen_screenshot_pages.py` → `shot_windows.js`＋`shot/*.html`
- 掃 `D:\HT9045\IMG\ScreenShot`，檔名 `Form.tab.png` 依 `Form` 分組。
- **`SKIP` 集合**：已有 HTML 模擬頁的表單 → **不**產生截圖卡（`ScreenShots.html` 的 grid 卡片）。
- **`MAINTABS` 集合**：主畫面下方零散截圖（Tab_UPH/tsCategoryInfo/tsIndex/tsLotID/tsTestBin）收成一組「Main 下方分頁」。
- 輸出：每組一頁 `shot/<slug>.html`＋ `SHOT_WINDOWS`（`background.html` 併入為 `noTask` 視窗）。

### 2. `_scan_dfm_shot.py` → `screenshot_meta.js`
- 掃全部 `*.dfm`（首行 `object <Form>: <Class>`）產生 `ALL_DFM`：`{form,cls,dfm,shot,html,i18n}`。
  - `shot` = `IMG\ScreenShot` 內有同名截圖。
  - **`html` = dfm base name ∈ `converted_dfm` 集合**（此集合是「已轉換」的唯一判定來源）。
  - `i18n` = 同名 `.cpp` 含 `iLanguageCountry`。
- `DYNAMIC_CLASSES`（**手寫清單**）：cpp 執行期 `new` 建 UI 的 class 及是否已入 VCL 元件頁（`inVCL`/`na`）。

## ✅ dfm 轉換完成後的更新檢查清單

1. **`_scan_dfm_shot.py` → `converted_dfm`**：加入該 dfm 的 **base name**（例：`cOffSet`、`uMotorTest`）。
   - fMain 的 pgMotionView 各 TabSheet 皆歸屬 `main.dfm`（`main` 已在集合內、html=✔），**不需**逐頁加；
     但若該頁用到執行期動態生成 UI，請更新對應 `DYNAMIC_CLASSES` 項（如 Motor View → `TMotorTestClass`）。
2. **`_gen_screenshot_pages.py` → `SKIP`**：若該表單在 `IMG\ScreenShot` 有截圖，將 **Form 名**加入 `SKIP`，
   避免仍以「未轉換截圖卡」出現。
3. **兩支都重跑**（venv）：
   ```powershell
   & d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_scan_dfm_shot.py
   & d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_screenshot_pages.py
   ```
4. **同步保存版**：把改動同步回 skill `scripts/_scan_dfm_shot.py`、`scripts/_gen_screenshot_pages.py`。
5. **驗證**：debug 開 `ScreenShots.html`，確認 `statHtml` 增加、該表單 `html` 欄為 ✔、無殘留截圖卡。

## ScreenShots.html 繪製欄位（供修版對照）
- 卡片 grid：`SHOT_WINDOWS`（`form`/`count`/`thumb`/`id`；點卡 `postMessage({open:id})`）。
- 表①「全部 dfm 表單狀態」：`ALL_DFM`，可點欄位排序；統計 `statShot`/`statHtml`/`statI18n`。
- 表②「動態生成畫面的 class」：`DYNAMIC_CLASSES`（`inVCL` ✔已記錄／`na` 免實作／否則待補）。

## ⚠ 截圖只能認「是哪一頁」，不能當「執行期長相」的權威（Steven 20260919）

`D:\HT9045\web\page\shot\Tf*.png` 這批交付的實機截圖，**是 BCB6 設計器（Form Designer）畫面，
不是執行期畫面**。判斷依據（任一成立即可認定）：

- **ScrollBox 是空的** —— 面板是 cpp 執行期才 `new` 的（THTSLKClass 家族、TMyVacuumPanel…），設計器上當然沒有。
- **元件上有選取控制點**（例：`tsIndex` 那張）。
- **`Visible = False` 的原型群組照樣畫出來** —— 設計器不吃 `Visible`（例：`myPalSamle`）。
- **分頁標題顯示原始元件名**而非 Caption（例：`tsDieForceOneByOneKit`）。

**結論：截圖可以用來認「這是哪一張表單、大致有哪些區塊」，但不可以拿來當版面驗收基準。**
執行期長相一律要看 cpp 的 `FormShow` / 建構式（表單尺寸、`Visible`、`Align`、動態 `new` 的面板），
必要時以 `CLIENT_OVERRIDE` / `RUNTIME_PROPS` 還原——案例見
[dfm-generator.md](dfm-generator.md) 的 VacuumUnit 段（dfm 設計期 655×711 vs 執行期 690×1020）。

這件事值得單獨記一筆，是因為「照著截圖做」的頁**看起來很像截圖**、又有截圖當佐證，
在 review 時幾乎抓不到；要到接 tag 找不到對應物件時才會爆出來。

## 已知重點
- `converted_dfm` 是 **html 狀態的唯一開關**——漏加會讓已轉頁在索引顯示為未轉換。
- 產生器輸出會**整檔覆寫** `screenshot_meta.js`／`shot_windows.js`；請勿手改輸出檔，一律改產生器再重跑。
- `ScreenShots.html` 本體（版面/繪製）才手改；資料檔不手改。
