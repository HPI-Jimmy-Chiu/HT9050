# 畫面 ↔ SECS/GEM ECID/SVID 對照手冊 製作指南

> 產出物：`docs/manual/SECS_Manual/HT9045_SECS_ScreenMap_ZH.html`（中文）／`_EN.html`（英文）
> 用途：把每個操作畫面（Form / TabSheet）上的欄位、按鈕，對應到 SECS/GEM 的 **SVID / ECID**，以及操作時發送的 **CEID 事件** 與可遠端觸發的 **RCMD**。單一自包含 HTML（截圖以 base64 內嵌）＋互動式像素標註框＋對照表。
> 參考範本：`docs/manual/HT9011UC_IOSetView_Alias_Map.html`（IO Alias 版）。

---

## 1. 資料來源（權威來源＝程式碼）

| 資料 | 來源檔 | 說明 |
|------|--------|------|
| SVID | `SECSGEM/uHGemHT9045_SV.cpp` | `AddSV()` 內 `SetSVDataPointer(id, type, name, unit, &ptr, desc)` |
| ECID | `SECSGEM/uHGemHT9045_EC.cpp` | `AddEC()` 內 `SetECDataPointer(id, type, name, unit, &ptr, max, min, def, desc)` |
| CEID 事件列舉 | `SECSGEM/uHGemHT9045.h` | `struct ETypeStruct` 列舉（`DoLotStart=6`、`DoLotEnd=8`、`SwitchTemperature=13`…）|
| CEID 觸發點 | `main.cpp` / `uLotInfo.cpp` / `note.cpp` / `AutoClean.cpp` … | `EventReport(SECS_EVENT.XXX)` 呼叫處 |
| RCMD | `SECSGEM/uHGemHT9045.cpp` | `S2F42_Host_Command_Acknowledge()` 內 `S.AnsiPos("LOTSTART")==1` 等字串比對 |
| 元件幾何 | `main.dfm` 等 `*.dfm` | `object Panel42 / imgTester …` 的 `Left/Top/Width/Height`（用於觸發元件畫框）|
| 截圖 | `IMG/ScreenShot/*.png` | **檔名沿用 Form / TabSheet 名稱**（如 `fContact.png`、`fTemp_Set.tsHot.png`）|

> C++ 原始檔為 **Big5 / cp950**，解析時務必用 `encoding="cp950"`。

---

## 2. 製作流程（4 步）

### Step 1 — 解析 SVID/ECID → JSON
`scripts/screen_map/extract_svid_ecid.py`：正則抽取所有 `SetSVDataPointer` / `SetECDataPointer`，
擷取 `(kind,id,type,name,unit,ptr,desc[,max,min,def])`，並由 `ptr` 判斷綁定的 **Form**（`f[A-Z]\w+->`）或全域結構（`RunInfo`/`LastSet`/`Temperature`/`DeviceForm_File`/`TestIF_File`…）。
輸出 `_secs_screen_map.json` + 依 form 分組的清單，方便查每個畫面欄位對應哪個 ID。

> 關鍵：**多數 ECID/SVID 的 ptr 直接指向 Form 控件**（如 `fContact->edForcePerDeviceKG`），這就是欄位→ID 的黃金對應；其餘指向全域變數者，用「名稱/說明關鍵字」語意比對。

### Step 2 — 讀取像素座標（網格法）
`scripts/screen_map/make_grid.py`：在截圖上疊一層每 50px 的座標網格（每 100px 標紅字），
存到暫存資料夾後**用眼睛讀出每個欄位的 `x,y,w,h`**（原生像素）。避免 DFM 巢狀座標換算誤差。

### Step 3 — 撰寫標註資料並產生 HTML
`scripts/screen_map/gen_screen_map.py`：每個畫面一個 `SCREENS.append(dict(key,title,file,w,h,desc,markers=[...]))`。
每個標註用 helper：

```python
m(x, y, w, h, field, entries=None, ptr="", note="", event="")
#   entries: [(kind, id, desc), ...]  kind in ("SVID","ECID")   → 表格列
#   ptr    : 綁定變數 / 控件（如 fContact->edSetKg）
#   note   : 備註（無對應時填說明；預設「目前無獨立 SVID/ECID」）
#   event  : CEID/RCMD 事件字串，用「；」分段，會併入「SECS 說明」自動拆列
```

產生器輸出：導覽列、每畫面（截圖 base64 + 紅框標註 + 對照表）、使用說明、後續清單、互動 JS。

### Step 4 — 瀏覽器驗證對齊
用瀏覽器開啟，逐畫面截圖確認**紅框對準欄位**、表格列與框互相反白。dense 畫面尤其要核對。

**批次稽核法（建議）**：`scripts/screen_map/audit_screen_map.py` 直接讀取產生器的 `SCREENS`
（截斷原始碼至 `def esc(` 前再 `exec`，或 `runpy` 執行），把所有標註框＋欄位名畫在截圖上，
每 2 個畫面拼成一張稽核圖（`_audit_secs/secs_NN.png`），逐張目視即可覆蓋全部畫面；
比逐一開瀏覽器截圖快得多。2026-08 已用此法完整複驗 50 畫面全數對齊（30 原始 + 20 擴充：fSCKART / fGroundMan / fObserver.tsCounter / FrmRotate / fStartCondition×8 / fSpeed×8）。

---

## 3. 事件 / RCMD 標註規則

- **CEID**（設備→主機事件報告 S6F11）：操作元件的 `Click` 觸發 `EventReport(SECS_EVENT.X)`。
  於 `event=` 寫成 `"TfMain::Panel42Click→CEID 13 SwitchTemperature"`；產生器以 `CEID` 徽章單獨列出，
  ID 取字串內 `CEID (\d+)`。
- **RCMD**（主機→設備遠端指令 S2F41/S2F42）：於 `event=` 以 `；` 分段加一句 `"RCMD LOTSTART …"`；
  產生器以 `RCMD` 徽章單獨列出。**同一動作同時有 ECID 與 RCMD 時，自動分成兩列**。
- **觸發元件要畫在「實際元件」上**：例如溫度模式是點擊 `Panel42`（溫度計圖示，非旁邊文字），
  由 `main.dfm` 找出 `palTester`/`palRunMode`/`Panel42` 為 48×48 圖示，再對照網格圖定位。
  觸發元件的框以**橘色**（`.mkE`）標示。

常用 CEID（`ETypeStruct`）：`DoStart=1 / DoPause=2 / DoOneCycle=3 / DoCleanOut=4 / DoClearCount=5 / DoLotStart=6 / DoLotEnd=8 / SwitchRunMode=9 / SwitchTesterMode=10 / SwitchTemperature=13 / SwitchStartMode=14 / SwitchSetupFile=15 / SwitchUser=16 / EnterTool=17…EnterMessage=22 / DoExit=24 / DoHome=25 / RunStatus=27 / DoAlarmReset=30 / DoTrayEnd=31 / DoReset=33 / DoAutoClean=34 / SiteOnOff=44 / ArmOnOff=45 / SwitchTempData=46 / AutoCleanFinish=50 / SiteMappingStart=51 / SiteMappingEnd=52 / AutoCleanClearCount=68 / DoStartHasIC=76`。
常用 RCMD（S2F42）：`LOTSTART / INITIAL_START(_ART/_MRT) / CLEAN_OUT / REMOTE_START / RESET / RESUME / PAUSE / HOME / HALT / SWITCH_TO_FT / SWITCH_TO_RT / PP_SELECT`。

---

## 4. 雙語（ZH / EN）

- 產生器有 `LANG` 全域與 `t(s)`：`LANG=="en"` 時查 `screen_map_trans.py` 的 `ZH2EN` 字典（查不到則原樣輸出）。
- Chrome 字串（標題、欄位表頭、使用說明、meta）放 `L = {"zh":{...}, "en":{...}}`。
- 每個 marker 的 `field` / `note` / `event` 段、section `title` / `desc` 以 `t()` 包裹。
- 輸出 `docs/manual/SECS_Manual/HT9045_SECS_ScreenMap_ZH.html` 與 `_EN.html`；兩檔頂端互相有語言切換連結。

### 新增 / 更新翻譯
1. 產生器支援 `DUMPSTR=1` 環境變數：跑一次會把所有含中日韓字元的 `field/note/title/desc/event` 匯出到 `_cjk_strings.txt`。
2. 把新出現的字串補進 `screen_map_trans.py` 的 `ZH2EN`。
3. 重跑產生器；用 `re.findall(r'[\u4e00-\u9fff]+', EN_html)` 檢查殘留中文（只應剩語言切換連結 `中文 ▶`）。

---

## 5. 檔案位置

| 檔案 | 說明 |
|------|------|
| `scripts/screen_map/extract_svid_ecid.py` | 解析 SV/EC cpp → JSON + 分組清單 |
| `scripts/screen_map/make_grid.py` | 截圖疊座標網格（讀像素用）|
| `scripts/screen_map/gen_screen_map.py` | 主產生器（含 `SCREENS` 標註資料、雙語）|
| `scripts/screen_map/audit_screen_map.py` | 稽核圖產生器（標註框畫回截圖，批次目視驗證）|
| `scripts/screen_map/screen_map_trans.py` | `ZH2EN` 翻譯字典 |
| 產出 | `docs/manual/SECS_Manual/HT9045_SECS_ScreenMap_{ZH,EN}.html` |

> 相依：Python 3 + `Pillow`（網格）。C++ 讀取用 cp950；輸出 HTML 用 UTF-8。
> 版本基準：`HT9011UC_Code_V3.33.910.0`（SVID 882 / ECID 1743 / CEID 289）。

---

## 6. 常見陷阱

- **cp950**：SV/EC cpp 用 Big5，錯誤編碼會讓中文說明變亂碼。
- **CSS `%` 與 `{}`**：DOC 模板含 `top:100%` 與 CSS `{}`，**用 `str.replace()` 佔位符**組裝，不要用 `%` / `.format()`。
- **座標為原生像素**：截圖若被縮放顯示，仍以原圖 `width×height` 為準（用網格圖讀值）。
- **觸發元件 vs 顯示欄位**：`Panel42`（可點擊圖示）與旁邊「Hot Mode」文字是不同元件；事件要標在可點擊元件上。
- **同 ID 多群組**：`fBinSel` 各列的 ECID 依 Normal/RT/OffLine/ART/MRT 群組分段（如 Double Contact = 3636/3718/3723/3802/3902/4002/4102），備註要註明整段。

---

## 7. Configuration 畫面版（fConfiguration，獨立手冊）

> 產出：`docs/manual/SECS_Manual/HT9045_Config_ScreenMap_{ZH,EN}.html`（與 SECS 畫面手冊互相有導覽連結）。

Configuration（機台設定）畫面每個 `[X##]` 設定項目對應到 **ECID**：

- **ECID 來源**：`uHGemHT9045_EC.cpp` 內名稱含 `[X##]` 前綴者（如 `[A05] Use Auto Docking` → ECID 35005）；
  用 `scripts/screen_map/prep_config_grids.py` 以正則 `^\s*\[([A-P]\d[0-9A-Za-z\-]*)\]` 建立 `section→ECID`。
- **項目標籤 / 分頁歸屬**：截圖位於 `ht9045-config/references/output/screenshots/<群組>/tab*.png`
  （`tabA01`=群組 A 第 1 分頁 [A01]-[A10]…）；每個 `[X##]` 標籤直接顯示在畫面上。
  另可參 `ht9045-config` 的 `config-fields-*.md`（section / Caption / 元件 / ECID 表）。
- **重點**：Configuration 多數項目**未綁定 ECID**（純內部/本機設定），標「無」；有綁定者以 `ECID` 徽章標示。
  因此建議每個分頁全列，讓使用者看出「哪些設定可被 SECS 讀寫」。
- **產生器**：`scripts/screen_map/gen_config_map.py`（**資料驅動**：由 `cConfiguration.dfm` 元件座標自動算出每個 `[X##]` 的位置，ECID-only 標註）。
- **位置自動化**：`scripts/screen_map/parse_config_positions.py`（**樹狀解析** `cConfiguration.dfm`）：
  建整棵物件樹後，找出每個群組 PageControl 的 **leaf TabSheet**（不含巢狀 PageControl 者＝實際截圖）；
  逐 leaf 累加巢狀 `Top` 得項目相對 y，再以 **snap-fit** 對齊截圖。
  **Snap-fit（關鍵）**：公式估算 tab strip 高度（列數×21）不可靠（實際渲染高度不一）。
  改為：`text_bands()` 掃描截圖左側欄（x=2..460）找「深色像素 ≥4 的列」聚成 4–22px 文字帶；
  `fit_offset()` 對每個分頁嘗試常數位移 k∈[-10,175)，以「各標註 y 到最近文字帶的距離總和」最小者為準。
  DFM 相對間距可靠，故一個 k 即可整頁精準對齊（含 tabN22 這種截圖無 tab strip 的情況）。
  **例外覆蓋**：`IMG_OVERRIDE`（leaf 名 → 正確 PNG，如 `tsN10_11_20 → tabN10_2.png`）；
  `SEC_Y_OVERRIDE`（單一標註分頁 snap-fit 有歧義時強制 y，如 N05/N06/N16 固定在 GroupBox 標題列，x=6/w=440）。
  **影像對應**：一般群組用 leaf 的 DFS 順序 → `tab<G>0<N>.png`；**N 群組為 section-named** → `tabN<section>[_1].png`。
  已正確處理巢狀分頁：C（單層）、**E（X/Y Scale 巢狀 → tabE01=E30、tabE06=E33-49…）**、I、**N（section-named tabN05/06/07/11/16/22/35）**。
  **驗證**：`scripts/screen_map/audit_config_markers.py`（3 分頁/張的稽核圖）逐張目視複驗全部 38 分頁。
- **ECID 對應**：以 **UI section 精確比對**（config-fields `_config_ecid.json` + 程式碼 bracket `[X##]`）；
  **不做 base fallback**（避免 `A10-2`~`A10-7` 誤繼承 `A10` 的 ECID）。
- **進度**：自動產生 **38 個含 ECID 的分頁**（A/C/D/E/F/I/L/N/O/P，含巢狀 C/E/I/N 已校正對齊）；B/G/M 無 ECID 項目故不列。

## 8. 覆蓋率缺口報告（要補哪些截圖）

> 產出：`docs/manual/SECS_Manual/HT9045_SECS_Coverage_Gaps.html`
> 產生器：`scripts/screen_map/gen_gap_report.py`

列出**有 SVID/ECID、但尚未出現在任何手冊畫面**的項目，依綁定 Form / 全域家族分組，方便判斷「還需要補哪些畫面截圖」：

- **placed** ＝ 掃描已產生的手冊 HTML（`class="cid">nnn`）取得已上畫面的 ID。
- **gap** ＝ `uHGemHT9045_SV/EC.cpp` 全部 ID − placed。
- 摘要表把 `f` 開頭的 Form（如 `fGroundMan`/`fSmartDiagnostic`/`fObserver`/`fSCKART`/`fStartCondition`）排前面 → CP 值最高的補圖對象；全域家族（IniConfig/TestIF_File/…）多為設定值，可對照既有畫面確認。

