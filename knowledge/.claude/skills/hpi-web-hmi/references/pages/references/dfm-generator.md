> 保存來源：`.claude/skills/ht9045-html-version/references/dfm-generator.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# dfm → HTML 產生器與定位法則

> 隸屬 skill：ht9045-html-version。產生器腳本存於本 skill `scripts/`（保存版），工作副本在 `D:\AI_TempFile\`。

## 產生器 _gen_dfm_abs.py

`scripts/_gen_dfm_abs.py`（絕對座標版，取代舊 `_gen_dfm_pages.py` 流式版）：

- 解析 dfm 物件樹（object/inherited…end、字串 `#NNN` 解碼、`''`跳脫、`&&`→`&`）。
- 依 Left/Top/Width/Height 絕對定位；TGroupBox 內容偏移 (2,15)；Align=alClient/alBottom 處理。
- 型別對應：TPageControl→巢狀分頁（:scope 切換）、TMyLed/TMyLedLane/TALed→`.aled`、
  TBtnPanel/TBtnPanelLane→`.btnpanel`（點擊切換 Down）、TTMyTray→`data-tray` + 執行期
  `HTWidgets.makeMyTray`、TImage→IMG_MAP 對照 BMP 轉 PNG。
- 自動重寫 IDE.ComponentMap.html 的 offset/speed/io/config＋新 5 頁區段。
- **dfm 更新後重跑一次即可全部同步**：`& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_dfm_abs.py`
- JOBS 現有 30 頁（2026-08-27 +5+7+8+1+5）：cOffSet／cSpeed／IoSetView／cConfiguration＋
  cCounterSel(397×559)／cCounterClear(270×559)／cBuilder(817×632)／DIOInterFaceCFG(598×469，fDIOFrom)／LtcSensor(1198×480)＋
  cTowerLight(760×530)／OmronEJ1N(1326×764，EJ1N\，fOmron)／QAMode(494×469)／BarCode(1086×846，BarCode\)／
  MyCCLinkSensor(1069×676，CCLink\，fCCLink)／uCleaning(996×847，AutoClean\)／cContact(1019×999)＋
  cTesterIF(907×800，FTestIF)／GroundMan\GroundMan(884×537)／cLd_ULd(735×717)／cSecurity(874×871)／
  cTrayForm(897×726)／Automation\SCK_ART(798×1002，fSCKART)／uYieldMonitoring(1053×956)／cHotPlate(606×531)＋
  cObserver(992×775，fObserver，視窗 id observer，Message 鈕 {open:'observer'} 開啟）＋
  cSetUp(1082×956，fSetup)／SmartDiagnostic(804×658，fSmartDiagnostic)／cStartCondition(1097×727)／
  uTemp_Set(1272×995，fTemp_Set)／cBinSel(1087×852，fBinSel)。
  background 視窗尺寸公式 `w=cw+2, h=ch+26`；視窗 id：countersel/counterclear/builder/dioform/ltcsensor/
  towerlight/omron/qamode/barcode/cclink/cleaning/contact/testerif/groundman/ldud/security/trayform/
  sckart/yieldmon/hotplate/observer/setup/smartdiag/startcond/tempset/binsel（hidden+fixed）。
  Main.html `DFM_MAP` 23 項（sbSelete/sbClear/sbBuilder/sbDioSet/sbSensorLatch/sbTowerLight/sbOmron/
  sbQAMode/sbBarCode/sbShuttleSensor/sbAutoClean/sbContact/sbTester/sbLdUld/sbPassword/sbTrayForm/
  spbAutoRetest/sbYield/sbPlateForm/sbSetup/sbStartMode/sbTempOffset/sbBin）改開 dfm 頁，
  click 先查 DFM_MAP 再 SHOT_MAP（剩 2 項：sbTrayAssign/sbRotate），
  標色 IIFE 對 DFM_MAP 命中者不標 shot/todo；fGroundMan/fSmartDiagnostic 無主選單入口
  （fSmartDiagnostic 由 fStartCondition sb_Maintenance_SmartDiagnosticFunctionClick 開）→只加視窗不加 DFM_MAP；
  同步把已轉表單加入 `_gen_screenshot_pages.py` SKIP 並重生（現 3 頁 7 張：FrmRotate/fTrayAssignment/MainTabs；舊 shot 頁手動刪，
  注意實際檔名**無 shot_ 前綴**，如 page/shot/FTestIF.html）。
- **JOBS 續增**（上面那段「30 頁」是 2026-08-27 的數字，**現為 42 筆**）：Alert 三頁（mymessbox／note／Password）、
  20260918 `ShuttleMove.dfm`→`HW.ShuttleMove.html`，20260919 三頁 ——
  `ContactForce.dfm`→`Setup.ContactForce.html`(862×716)、
  `VacuumUnit\VacuumUnit.dfm`→`HW.VacuumUnit.html`(682×989)、
  `Automation\AGV.dfm`→`Setup.AGV.html`(895×683)。
  20260919 三頁的 golden 入口（行號為 **V910** 樹）：`cContact.cpp:15099 TfContact::btContactForceClick`
  → `fContactForce->Show()`（**入口在 Setup.Contact 頁內的按鈕，不是主畫面選單**）、
  `main.cpp:34660 TfMain::sbVacuumUnitClick`、`main.cpp:34879 TfMain::spbAGVClick`。
  `background.html` 的 `WINDOWS` 依既有公式 `w=cw+2, h=ch+26` 登錄 `contactforce`／`vacuumunit`／`agv`
  （`hidden:true, fixed:true`，並帶 `form:` 欄）。
  ⚠ **三頁都沒有加進 `MODAL_POLICY`** —— 那張表就是 `.github\specs\page-access-policy.md` §3 的六列，
  是「開窗前要先過哪些機台守衛」的政策，不是頁面清單，**不因為新增頁面就自行擴大**。
- client 尺寸 fallback（2026-08-27 改自動扣邊框）：dfm 缺 ClientWidth/ClientHeight 時
  `cw=Width−8、ch=Height−31`（XP 邊框＋標題列），不再殘留右/下空白；
  `CLIENT_OVERRIDE` 只留特例：OmronEJ1N (1326,764)（依子面板幾何）、
  cObserver (992,775)（FormShow 設 Width=1000，dfm ClientWidth 932 會讓 labDeviceName 944 溢出出捲軸
  ——「dfm 有 ClientWidth 但執行期改寬」型態，需查表單 FormShow）。
- JOBS title 字串禁反斜線路徑（`\u`/`\O` 跳脫會炸 SyntaxError）→ title 用 `/`、dfm 路徑欄用 raw string。
- 新型別 render：TCheckListBox（checkbox 列表）、TDirectoryListBox（📂 目錄意象）、TDriveComboBox、TScrollBar（Kind 水平/垂直灰條）；SKIP_TYPES 含 TComm/TClientSocket/TServerSocket。
- 按鈕文字（Set 35）：`.btn3d{overflow:hidden;white-space:nowrap}`（VCL 按鈕單行不折行）＋產生器估寬自動縮字——
  `est=Σ(全形1.0／半形0.55)×fs`、`avail=Width−10`，超出時 `fs=max(8, fs×avail/est)`
  （cBinSel「Set all to not use」W97 案例：HTML 字寬比實機 MS Sans Serif 寬，不縮字會左右裁切）。
- 分頁內只有空 Panel＝執行期動態建欄位陣列（uTemp_Set thNormal 的 Plate/SH Edit 陣列），留空正常；
  巢狀 TPageControl（cStartCondition tsLifeTime→pgLifeTime 9 子頁籤）產生器原生支援，無需特例。

### JOB_BASE：單一 job 指定別棵 golden 樹（Steven 20260919）

`JOB_BASE = {dfm: 來源樹根}`；未列的 job 一律走 `BASE`（V910），取檔時
`os.path.join(JOB_BASE.get(dfm, BASE), dfm)`。目前只有一筆：

| dfm | 來源樹 |
|---|---|
| `Automation\AGV.dfm` | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（V912） |

**為什麼不能用 V910**：V910 那份 `AGV.dfm` 的 E84 燈號還停在 IDE 自動命名（`Label33`、`Label128`…），
V912 才改成 `lblE84_1_In0` / `lblE84_1_Out4` 這種帶語意的名字。核心原則 1 要求「元件名稱必須與 .dfm
完全一致」，用 V910 轉出來會得到一批 `LabelNN`——之後接線對不上，也沒辦法從 id 看出那是哪一支 E84 訊號。
`ContactForce.dfm` 與 `VacuumUnit\VacuumUnit.dfm` 在兩棵樹是 byte-identical，不需覆寫。

⚠ `JOB_BASE` 只換「這支 dfm 讀哪一棵樹」。同名 `.cpp` 的行號引用（`RUNTIME_PROPS` 註解、`IMG_MAP` 依據、
本文件的 golden 入口）**必須標明是哪一棵樹的行號**，否則下一個人拿 V910 去對會對不上。

### HT9045_DFM_OUT：輸出目錄覆寫（Steven 20260919）

`OUT = os.environ.get('HT9045_DFM_OUT') or OUT`。用途是**只想取幾頁、不想動整棵 `page\`**：
把 `OUT` 指到 scratch 目錄跑一次，再把要的檔挑出來。

⚠ **仍要跑完整 `JOBS`，不可以只留想要的那幾筆 job**：`IDE.ComponentMap.html` 是「一次重寫全部區段」的，
只跑部分 `JOBS` 會把它砍到只剩那幾段。這也是這個環境變數存在的理由——用它換輸出位置，而不是刪 job。

### widget_host()：宣告式執行期面板（Steven 20260919）

適用型態：**dfm 上只有一個空 ScrollBox，真正的面板是 cpp 執行期 `new` 出來的**。

舊做法是 `home_grid_html()` / `motortest_grid_html()` / `omron_scrollbox_html()`——Python 直接吐 HTML。
結果同一組 dfm 座標同時寫在 `_gen_dfm_abs.py` 與 `hwidgets.js` 兩個地方，**改一邊另一邊不會跟著動**。
新做法讓產生器只宣告「要幾個、叫什麼」，座標與長相在 `hwidgets.js` 留一份。

- 產生器端：`widget_host(maker, items, pitch_x=None, pitch_y=None, note='', pad_top=0)` 產出
  `<div class="htWidgetHost" data-maker="…" data-spec='{"items":[…],"pitchX":…,"pitchY":…,"padTop":…}'>`，
  一樣經 `RUNTIME_INJECT[(dfm, 容器名)]` 塞進該 ScrollBox。
- 網頁端：**新檔 `page/page-widgets.js`** 掃 `.htWidgetHost[data-maker]`，把每個 item 當 opt 餵給
  `HTWidgets[maker]`；未指定 `left/top` 者依 `col/row × pitchX/pitchY` 自動排位，最後把 host 撐到
  內容大小（ScrollBox 才捲得到最後一個面板）。`pad_top` 是留給 GroupBox legend（`top:-10` 畫上去）的
  頂邊，否則第一列會被裁掉。
- **載入順序是承重的**：`page-widgets.js` 必須排在 `hwidgets.js` 之後，由 `PAGE_EXTRA` 掛上
  （目前 `Setup.ContactForce.html`、`HW.VacuumUnit.html` 兩頁）。順序反了 `window.HTWidgets` 還不存在，
  host 會停在「hwidgets.js 未載入」字樣。
- 舊的三支 `*_grid_html()` **沒有回頭改**（已驗收，改動風險大於收益）；新增這類面板一律走 `widget_host()`。

目前用在：

| 頁 | ScrollBox | maker |
|---|---|---|
| `Setup.ContactForce.html` | `scrlbxDynamicKit` / `scrlbxDynamicKitInd` / `scrlbxDieForceDynamicKit` / `scrlbxDieForceOneByOneDynamicKit` | `makeContactForceGroup`（THTSLKClass 家族四變體） |
| `HW.VacuumUnit.html` | `scrlbxIndexArm1` / `scrlbxIndexArm2` / `scrlbxInArm` / `scrlbxOutArm` | `makeVacuumPanel`（TMyVacuumPanel，4 欄×2 列） |

### RUNTIME_PROPS 實例：VacuumUnit 的執行期尺寸還原（Steven 20260919）

`TfVacuumUnit` 建構式（V910 `VacuumUnit\VacuumUnit.cpp:52-99`）**無條件**依吸嘴數重算四個 GroupBox，
`FormShow` 再把表單設成 690×1020。dfm 的設計期尺寸（四個 GroupBox 都 300×300、表單 655×711）
**不是任何一台機台看得到的樣子**——照 dfm 直轉會得到一張沒人看過的畫面。

本機 `system\Gerneral.ini` 的 `USE_46_SUCKER_DB=0` → HT9045 分支，
`iIndexColMax = iInOutColMax = TOTAL_VACUUM_UNIT/2 = 4`，故
`Width=(VACUUM_UNIT_WIDTH+2)×4=332`、`Height=(VACUUM_UNIT_HEIGHT+12)×2=378`；
`CLIENT_OVERRIDE[r'VacuumUnit\VacuumUnit.dfm'] = (682, 989)`（690/1020 扣 XP 邊框標題列）。

⚠ **既有限制：`pos()` 只處理 `alClient` 的伸縮，`alBottom`／`alTop`／`alLeft`／`alRight` 仍留在設計期座標。**
平常看不出來（client 高＝dfm 高），但**只要用了 `CLIENT_OVERRIDE` 改高度，`alBottom` 的按鈕列就會卡在
畫面中間**。本頁以 `RUNTIME_PROPS` 的 `Panel1: {Top:'925', Left:'0', Width:'682'}` 補算解決（925 = 989−64）。
→ **任何新增或修改 `CLIENT_OVERRIDE` 的頁，都要回頭檢查該表單有沒有 `alBottom`／`alRight` 兄弟。**

### TAB_EXTRACT（頁籤抽出樹改造）

`TAB_EXTRACT = {dfm: [(來源TabSheet名, 元件名, 新TabSheet名, 新Caption, anchor頁籤名)]}`：
parse 後、LED pre-pass 前執行，把容器內指定元件抽成同 PageControl 的新 TTabSheet
（插在 anchor 之後、元件重定位 (0,0)、移除 Align）。
現有：iosetview `grpManual`（EMPTY & COLOR TRAY，Above Conveyor 底部 alBottom 413px 使頁面過高）
→ `tsStack1_Manual`「Manual Track」插在 tsStack1_Cassette 之後（2026-08-27 使用者要求）。

### TImage 底圖對照（IMG_MAP）

key = `(JOBS dfm 字串, 元件名)`；dfm 含子資料夾者用 raw string（如 `r'AutoClean\uCleaning.dfm'`）。
IMG_MAP 命中 → BMP 轉 `img/dfm_<檔名>.png`（空格轉底線）。

| dfm | TImage | 來源 BMP（依 cpp 執行期邏輯） |
|---|---|---|
| cOffSet.dfm | Image1/Image3 | IMG\BMP\InOutArmOffset_6.bmp（AUTO_EMPTY_COLOR>=3）/ InOutArmOffset.BMP / InOutArmOffset_AU.bmp |
| cOffSet.dfm | Image2 | IMG\BMP\SuckBaseD.bmp（InArm）/ SuckBaseF.bmp / SuckUnit1.BMP |
| iosetview.dfm | imgOther | IMG\BMP\IP Setting.bmp（另有 COMSetting/KVM_2PC/KVM_3PC 切換） |
| AutoClean\uCleaning.dfm | Image1/Image3 | HoatPlate1.bmp（Kit/Tray 分頁同圖） |
| cContact.dfm | imgIndex / imgSLK | Contact.bmp / Contact2.bmp |
| cLd_ULd.dfm | Image1 / Image2 | LoaderCondition.bmp / UnloaderCondition.bmp |
| cTrayForm.dfm | Image1~3 / Image4~6 | TrayForm2.bmp / TrayForm3.bmp（Type1/2/3 三分頁各一組） |
| cTrayForm.dfm | imgDevice0/90/180/270 | Device0/90/180/270.bmp |
| cTrayForm.dfm | imgTray0/180/0_Flip/180_Flip | Tray0/Tray180/Tray0_Flip/Tray180_Flip.bmp |
| cHotPlate.dfm | Image1/Image2 / Image3 | HoatPlate2.bmp ×2 / HoatPlate1.bmp |
| cSetUp.dfm | Image1 | 8siteCenterX.bmp（機種相依：另有 2site.bmp 等） |
| uTemp_Set.dfm | Image1 | tmode1.bmp（tmode{iPoint}，取預設單點） |
| QAMode.dfm | Image1 | type0.bmp（type{0..7} 點擊輪替方向，取預設 0） |
| cConfiguration.dfm | imgI37_3 | type0.bmp（同上，I37 Lock Loader Direction） |

### dfm 內嵌圖（Picture.Data）與透明熱區

- `parse_dfm` 收集 `Picture.Data = {` 的 hex → `embed_png()`：格式 = 1 byte 類別名長度 + 類別名（TBitmap/TJPEGImage）+ 4 byte 大小(LE) + 圖檔內容 → PIL 轉 `img/dfm_embed_<dfm stem>_<元件名>.png`。命中者：cTrayForm Image7/8/9、uCleaning ImgCleanUnit、uTemp_Set Image2。
- 無 IMG_MAP 也無內嵌、且 `Transparent=True`（如 Config Image1 雙擊熱區）→ 輸出透明 div（不畫 .imgph 虛線框，同實機不可見）；其餘無來源者維持 .imgph placeholder。
- 動態方向圖（type{n}/tmode{n}）取程式預設值那張，不做互動輪替。

### 按鈕 Glyph（Glyph.Data，Set 38）

- `parse_dfm` 同步收集 `Glyph.Data = {` 的 hex → `glyph_png()`：格式 = **4 byte 大小(LE) + BMP**（與 Picture.Data 不同，無類別名前綴）→ PIL 開圖 → `NumGlyphs` 橫排切第一格（全 30 dfm 掃描 194 顆均為 NumGlyphs=1）→ **左下角像素色轉透明**（VCL TSpeedButton 透明色規則）→ 存 `img/dfm_glyph_<md5[:10]>.png`，`GLYPH_CACHE` 以內容 hash 去重（Save/Exit 等共用圖示只存一份）。
- 渲染：TSpeedButton/TBitBtn 有 glyph 時輸出 `<img src style="flex:none;">` 於 caption 前；`.btn3d` 為 inline-flex + gap:3px；`Layout=blGlyphTop` → `flex-direction:column`；縮字估寬 avail 需扣圖寬+3。
- 分佈：24 個 dfm 共 194 顆（cOffSet 77 最多、cObserver 19、cConfiguration 15、cSetUp 8…）。

### TImage 圖片去背（Set 39）

- `bmp_debg(im)`：RGBA 化後把「左下角像素色」精確匹配的像素轉 (0,0,0,0)——沿用 VCL 透明色規則。
- 套用點兩處：外掛 `IMG_MAP` 轉檔迴圈（`src.lower().endswith('.bmp')` 才去背）與 `embed_png`（`payload[:2]==b'BM'` 才去背）；**JPEG 照片一律跳過**（實物照去背會斑駁）。

### Exit/Save 按鈕全域統一（Set 39）→ 詳見 [exit-save-unified.md](exit-save-unified.md)

- 標準源：cOffSet 的 `sbtExit` / `spbSave`（主流 18/18 顆同款）——主迴圈前預載 `STD_BTN = {'exit':(glyph,fs14),'save':(glyph,fs14)}`（`_find_node()` 遞迴取節點）。
- 條件：caption **純** `Exit/EXIT/Close/CLOSE`（EXIT_CAPS）或 `Save/SAVE`（SAVE_CAPS）的 TSpeedButton/TButton/TBitBtn → 覆蓋 glyph＋字級，title 標「TSpeedButton（統一 exit/save 樣式）」；複合字樣（Save Image/Save Data/Save Config. 等功能鈕）不動。
- **按鈕型 TPanel 也統一**：無子元件＋有 OnClick＋caption 命中（全寬底部 Exit 條共 8 顆，如 cObserver btExit）→ 直接輸出 `<button class="btn3d exitbtn">`，title 加註「，原 TPanel」，關窗綁定保留。
- glyph `<img>` 加 `max-height:88%` 防小按鈕爆框。
- 全清單（57 顆＝30 exit＋27 save）與重掃工具 `_scan_exitsave_table.py` 見 exit-save-unified.md。


## hwidgets.js 模板 API（window.HTWidgets）

| VCL 元件 | 原始碼 | 工廠函式 | 重點屬性 |
|---|---|---|---|
| TALed | elec\Component\aled.pas | makeALed | ledStyle（6 種 LEDStyle）、value、trueColor(clLime)/falseColor(clSilver)、blink/interval；`setValue()` |
| TMyLed | elec\myvcl\MyLed.h | makeMyLed | + port/bit/type/alias（title 顯示） |
| TMyLedLane | elec\myvcl\MyLedLane.h | makeMyLedLane | + ring/ip/isISA |
| TBtnPanel | elec\myvcl\butPa1.h | makeBtnPanel | caption/down/trueColor/falseColor/字色、flat（tsFlatButtons）；點擊=SetPanelStatus；`setDown()` |
| TBtnPanelLane | elec\myvcl\BtnPanelLane.h | makeBtnPanelLane | + lane/ip/isISA |
| TSpeedButton/TBitBtn | VCL 標準 | makeSpeedButton | caption/img(glyph URL)/w/h/fs/layout('top'=blGlyphTop)/flat/onClick；`setGlyph(url)`；.btn3d 樣式自帶 inline |
| TTMyTray | elec\myvcl\HTray.h | makeMyTray | xitem/yitem、x/yblockItem+Width、trayDirect 角標、colorMap、cells、`setCell()/clearCell()/onCellClick` |

## 驗證比對資料

| 用途 | 位置 |
|---|---|
| IO 畫面元件相對位置（座標標框） | <入口網站 repo>\public\Docs\manual\HT9011UC_IOSetView_Alias_Map.html |
| 其餘畫面截圖對照 | <入口網站 repo>\public\Docs\manual\SECS_Manual\HT9045_SECS_ScreenMap_ZH.html、HT9045_Config_ScreenMap_ZH.html |
| 底圖 | D:\HT9045\IMG\BMP、D:\HT9045\IMG\Graphic |

## VCL→HTML 定位法則（2026-08 以 cSpeed 驗證）

- **所有 Align 一律直接用 dfm 的 Left/Top/Width/Height**（設計期真實幾何都有存）；僅 alClient 用 right/bottom 貼齊（預留 alRight/alBottom 兄弟空間，見下）。alClient 絕不能渲染成 inset:0（會蓋住兄弟容器）。
- **TGroupBox 子元件原點 = 控制項左上角 (0,0)**：`AdjustClientRect` 只影響 Align 子件，非對齊子件不加 caption 偏移。`.cli` 用 `inset:0` + `overflow:hidden` 模擬 Windows 對子視窗的裁切。
- **HTML fieldset legend 會佔頂部空間** → legend 必須 `position:absolute; top:-2px` 並帶父背景色（render() 以 bg 參數下傳），否則子元件整體下移 ~12px。
- **空 Caption 絕不能 fallback 成元件名稱**（RadioGroup/GroupBox/CheckBox/Label 皆同）。
- **dfm Items.Strings 解析**：讀到 `Items.Strings = (` 該行必須 i+=1 跳過，否則首項會多出幽靈空選項。
- **多行 Caption 解析**：`Caption =`（等號後空值、字串從下一行開始）→ 先讀下一行再進 `+` 續行迴圈；續行合併時若前段以 `'` 結尾、後段以 `'` 開頭，須去掉邊界引號，否則畫面殘留多餘 `'`（案例：IoSetView pnlStack1_Top 的 Front...Rear）。

## Tab 層級與 Panel 標題樣式

- **頁籤統一比照 main 畫面各視窗**（ht9xxx.css `.tabs/.tab`，使用者 2026-08-26 決定）：所有層級同一樣式——`--tab-bg` 底、圓角上緣、act 頁籤**底色 = 內容面板色（--form-bg）且蓋掉交接處框線**（pcBody top = tabsH−1 重疊 1px、act 的 border-bottom-color 同底色）呈無縫融合；未作用頁籤保留 border-bottom 形成面板上框線。`flex-wrap` + `align-items:end` 處理多列換行。曾嘗試「第一層經典 strip＋巢狀 pill」的層級化設計，已捨棄。
- **TPanel 標題 `.pnlCap`**：完整輸出 DFM `Font.Name/Height/Color/Style`，並設 `white-space:pre` 保留長空白排版。每個 TPanel 都必須保留直接子層 `.pnlCap`；Caption==元件名（如 pnlFT_Left/pnlKitSetting）時只清空文字，不刪節點。Observer、Teach、MotorTest 等 runtime 更新只能寫 `panel.querySelector(':scope > .pnlCap').textContent`；禁止寫 `panel.textContent`，否則會刪子元件並退回頁面預設字形。Observer 已以 Playwright 驗證 `Arial 9px / 400 / center / white`。
- **`.ckb` checkbox 文字用 `white-space:pre` + flex 置中**：caption 常以連續空白替 TEdit 留位（如 cbA01「idle over　　sec.」中間放 edA01），HTML 摹疊空白會讓 edit 蓋字；另 `display:flex;align-items:center;line-height:13px` + input 12px，否則 dfm Height=15 的 checkbox（D40/D44/D45）文字下緣被 overflow:hidden 裁掉，看起來像被下方元件擋住。
- **caption 長空白（≥6 個連續空白）→ 尾段右錨定**：VCL 以長空白替疊放 TEdit 留位，且 TLabel AutoSize 的 dfm Width＝VCL 實測文字寬（保證尾段 `(Unit : 0.1 Sec)` 起點在 Edit 右側）；瀏覽器字寬較窄會讓尾段左滑被 Edit 蓋住（cLd_ULd Knocker 頁 labP13/cbF23 等）。修正：TLabel/TStaticText（非 WordWrap）與 TCheckBox 偵測 `re.search(r' {6,}', c)` 時拆頭/尾兩段——頭段照常靠左、尾段 `<span style="position:absolute;right:0">` 右緣對齊 dfm 右緣（Left+Width），天然閃開疊放 Edit；TLabel 拆分時保留 dfm width（不用 width:auto）。
- **alClient 定位**：`left:{L};top:{T};right:{R}px;bottom:{B}px`——保留設計期原點（已含 alLeft/alTop 兄弟佔位，如 MemoD Left=437），右/下貼齊父容器伸縮，才不會因 .pcPane 內縮溢出出捲軸。R/B 由 `mark_alclient()` 預算＝同層 alRight 兄弟 Width 總和/alBottom 兄弟 Height 總和（無兄弟則 0，行為同舊）。**不可寫死 right:0;bottom:0**：dfm 中 alClient 若排在 alRight/alBottom 兄弟之後（z-order 較上）會整片蓋住它們——DIO 案例：Panel1(alClient) 排最後，Category Signal(alRight 309)、Name(alBottom 71)、Exit(alBottom 41) 全部消失。禁止 `inset:0`（MemoD 會蓋滿整頁）也禁止寫死 W/H（溢出）。
- **.pcPane 必須 `left:0;right:0;top:6px;bottom:0;overflow:hidden`**：left 內縮 2px 會讓 alClient 子面板水平溢出 2px → 整條難看捲軸（IO Vacuum/In Arm 案例 2026-08-26）。overflow 原為 auto，2026-08-27 改 hidden：實機 VCL TabSheet 不捲動直接裁切（cTesterIF rgTime1 頁 edtAfterTestedDelay 超出 2px 就出捲軸的案例）。溢出檢查須逐一切換分頁再量：`display:none` 的 pane `scrollWidth=0` 量不到，只掃當前頁會漏。
- **PANEL_BG_MAP 底色主題化（Set 36，使用者要求統一佈景）**：TPanel/TScrollBox 底色改走 `panel_bg()`——先查 PANEL_BG_MAP 值映射再 fallback `color()`。映射：強調標題 panel（#517B91 主流 9 頁／#A3675C cObserver 磚紅／#9E9ECF SCK_ART 紫／clTeal／#494D6D）→`var(--dfm-hdr,#517b91)`；大面積淺藍底（clSkyBlue／#89BBD6／#78B7DC／#8DCDCD／#88A0AE／#B7C4CE cSetUp／#CCD9DF uCleaning）→`var(--form-bg)`；淡黃提示欄（#FCFBD1／clInfoBk／clMoneyGreen）→`var(--note-bg,#fcfbd1)`；綠狀態塊（#00B058／clLime）→`var(--green)`；clWhite→`var(--input-bg)`。theme.css 四主題各定義 `--dfm-hdr`／`--note-bg`（classic #517b91/#fcfbd1、dark #3d5a6b/#4a4632、steel #4a7191/#f4f1d4、contrast #003050/#ffffc0）。**不映射**：IoSetView 專屬色（#3E3A39/#DCDDDD/#FFF33B/#9FA0A0——已驗收的刻意設計）、clGray（值顯示格中性色，Omron ##.##／cOffSet Loader 標籤）、Font.Color 字色／LED TrueColor／TShape Brush.Color（僅容器底色走 map，字色誤映射會在 dark 主題撞底色）。cOffSet 機構示意圖的 tray 紅／Arm 通道藍為功能語意色保留。
- **GroupBox 間距（--gbi 自適應內縮，2026-08-26 v3）**：`.gbx{border:none}` + `.gbx::before{inset:var(--gbi,2px 3px 3px 2px);border:1px solid var(--gbx-border);border-radius:3px}` → 外框往內縮畫、dfm 座標不動。**固定內縮會在子元件貼邊的盒子穿越元件**（VCL 實機邊框畫在元件邊界上），改由產生器 `gbx_inset(n)` 依每盒子元件淨空逐盒計算寫入 inline `--gbi`：top/left=max(1,min(2,淨空−1))、right/bottom=max(1,min(3,…))；alClient 子件視為淨空 0；無子元件維持預設。v3 使用者反饋「盒子高度再高一點」→ 上限由 3/5/7/3 調小為 2/3/3/2（外框更貼元件邊界=更接近實機）。穿越稽核時 **`.cli`（inset:0 互動覆蓋層）必須排除**，否則每盒都誤報 ovR/ovB=內縮量。
- **layoutPC() 巢狀頁籤定位**：隱藏 pane 內的 `.pcTabs` `offsetHeight=0`，直接算 top 會得 −1px 造成頁籤層疊——必須 guard（offsetHeight>0 才寫入），並在每次 tab click 後對全部 `.pcWrap` 重算。
- **CSS 不可依賴 JS 量測才可見**：`.pcBody` 必須保留静態 `top:26px` 保底；若改成 `top:auto` 等 JS 填入，在 background.html 隱藏視窗（display:none iframe）內載入時 layoutPC 量不到高度不寫 top，`top:auto+bottom+height:auto` 的 absolute 盒高度由內容（全 absolute 子件）決定 = 0 → 整頁元件消失（Edge 實測案例 2026-08-26）。顯示後的精確重算由 ResizeObserver(tabs) 補。

## cConfiguration 頁（Visible=False 顯示策略）

- Config 頁有 137 個 `Visible = False` 元件，實機由執行期程式碼依機型/客戶開啟。手冊模擬比照實機截圖**全部顯示**：產生器 `SHOW_HIDDEN_JOBS = {'cConfiguration.dfm'}`。
- 例外 `HIDE_ALWAYS`（互斥疊放/開發測試框，維持隱藏）：`cbA69`（疊 A66）、`cbOverSetTempMustOpenFan_UltraTempKitSupportAmbient`（99% 疊 LA20-4）、`edtTemp`（Text='edtTemp' 測試框蓋 palTrayDef）。條件必須是 `name in HIDE_ALWAYS or (Visible=False and not SHOW_HIDDEN)`——不可要求 Visible=False 才生效（cbA15_1 在 dfm 是可見的）。
- `POS_OVERRIDE`：執行期位置與設計期不同時改座標。`cbA15_1` 改 (9,268)：原 Top=140 與 chA16(142)/lblA16(168) 疊放，移到 grpA17(結束261)～cbA19(321) 之間空位。`pgcVacuum` 改 (4,4)：IO Vacuum 頁的 lbSuckEnabled（Visible=False 的 40px 警示列，alTop）佔位使設計期 Top=44，實機執行期該列隱藏、alClient 重新對齊貼頂。`pnlRearEMG` (4,4)/`pnlSystemClient` (4,34)：System 頁同型案例（labIonFanClean Visible=False 的 40px alTop 列佔位）——**Visible=False 的 alTop 占位元件 + alTop/alClient 兄弟 = 設計座標與執行期不符的典型模式**。
- 重疊掃描工具：`scripts/_scan_cfg_overlap.py`（同容器 Visible=False vs 其他元件幾何交集）；label/edit 成對重疊與「caption 留空位」模式屬正常 VCL 佈局，不豁免。
- 參考截圖：Q:/Docs_Manual/SECS_Manual_Web_HT9xxx/pages/cat_config.html 內 42 張 base64 內嵌 PNG，抽出腳本 D:\AI_TempFile\_extract_cfg_imgs.py（存 D:\AI_TempFile\_ref_cfg_NN.png）。
- 桌面整合：background.html `WINDOWS` 加 `{id:'config', src:'page/Config.Configuration.html', w:939, h:901, hidden:true, fixed:true}`；Main.html palConfig 的 `sbConfiguration` 項 postMessage `{open:'config'}` 並排除舊「模擬：開啟」alert。
- 固定尺寸視窗：dfm 表單為固定尺寸 → WINDOWS 項加 `fixed:true` 不產生 .rsz 縮放把手；最佳尺寸 = 內容寬+2（邊框）× 內容高+26（標題列+邊框）：speed 750×843、io 924×925、config 939×901，iframe 與頁面尺寸精準吻合即無捲軸。
- **尺寸上限（使用者規範 2026-08-26）**：設備螢幕解析度 **1920×1080**，固定不可縮放的畫面不得超出；高度須扣除 Windows 工作列（約 48px）→ 實際可用高度 ≤ 約 1032px。新增 fixed 視窗或設計 dfm 固定表單時都要檢查。
- **DEFAULT_ACTIVE 預設分頁**（產生器）：`{dfm名: {PageControl名: 頁籤名}}`，未列者取第 1 頁。dfm 的 `ActivePage` 是設計器最後存檔狀態（如 pcConfig=tsL00、pcA00=tsA06）**不可直接採用**。cConfiguration：PageControl1→tsConfig（非第一頁 tsSoftSimu）、pcConfig→tsA00、pcA00→tsA_00 → 開啟即見 A[01]。
- **Exit 按鈕關視窗**：產生器對 Caption 在 {Exit, EXIT, Close, CLOSE} 的按鈕型元件（TSpeedButton/TButton/TBitBtn/TPanel）加 `exitbtn` class（`exit_cls()`），PAGE_TMPL JS 對 `.exitbtn` 綁 click → `parent.postMessage({closeMe:1})`；background.html message handler 以 `e.source===iframe.contentWindow` 找到所屬視窗隱藏（同標題列 ✕）。不可用 id 白名單：各表單命名不一（spbExit/sbtExit/sbExit/btnClose/btnOk(QAMode Caption=Exit)/sbCleanExit）。Main.html 的 sbCloseProgram 不在此列。
- **TRadioGroup 多欄排列為 column-major**（先直後橫，同 VCL）：`grid-template-rows:repeat(ceil(n/Columns),1fr);grid-auto-flow:column`——不可用 grid-template-columns（row-major 項目順序錯位，DIO Channel & Bit Length 案例）。
- **TComboBox 要展開 Items.Strings**：選取 Text（在清單內）或 ItemIndex 或第 1 項；無 Items 才 fallback Text/元件名（否則顯示成 cbOneSTChannel 這種元件名）。

## LED＋Label 整合（虛擬 TLabeledALed / TMyLabeledLed / TMyLabeledLedLane）

- 使用者構想（2026-08-26）：LED 元件加 Caption 屬性可顯示於上下左右；**目前只實作 HTML 模擬層**（BCB6 實體元件待日後：原始碼在 elec\myvcl\MyLed.h/MyLedLane.h、elec\Component\aled.pas（Pascal），由 dclusr50\dclusr60.bpk 編譯註冊）。
- hwidgets.js：`makeLabeledALed/makeMyLabeledLed/makeMyLabeledLedLane`，opt 加 `caption`、`captionPos:'lpLeft|lpRight(預設)|lpTop|lpBottom'`；flex 版 class `.lledf`（lpLeft=row-reverse、lpTop=column-reverse、lpBottom=column）。
- 產生器自動配對：`LABELED_LED_JOBS={'iosetview.dfm', r'EJ1N\OmronEJ1N.dfm'}`，`pair_led_labels()` 在同容器內找緊鄰 TLabel（左右：垂直重疊且 gap −2～maxgap；上下：水平重疊±10px 且 gap −4～10），距離最近優先一對一；配對後 Label 進 `LABEL_SKIP` 不再單獨輸出，LED 改輸出 `.lled` 包裝（union bbox 定位、內部保留原座標 → 視覺零差異），title 標記虛擬型別與 Caption/CaptionPos；hover 整組虛線框。
- **`LED_SIDE_RULES` 容器別方向規則**（2026-08-27，使用者校正）：先匹配先贏，先特定容器後頁籤。`gbSocketSensor`→lpTop、`grpWinWayGroup`/`tsSystem`/`tsIndex`/`tsAOI`→lpRight、`tsAGV`→lpLeft(maxgap 65，E84 GO 間隙 60)、`tsTTL`→空集合=不整合（分拆）、`grpStatus`（Omron Module Status）→lpRight（直排 LED 間距 25px 會誤抱下一列 Label：lpBottom 3.5 贏 lpRight 6）；未命中容器用預設演算法（maxgap 14）。背景：Ion Fan 直行密排時 lpBottom(gap4→dist4.5) 會贏過 lpRight(gap5→dist5.0) 造成錯位交叉配對，需用語意規則鎖方向。逐頁驗證：tsSystem 40 lpRight、tsIndex 26 lpRight+gbSocketSensor 32 lpTop、tsATC 26 lpRight（含 WinWay ledSnATC*Ready＋lblWinwayATC*）、tsAOI 13 lpRight、tsAGV 18 lpLeft、tsTTL 0（40 顆 LED 全分拆）。
- **`LED_RESIZE` 容器內 LED 縮小置中**（2026-08-27）：`{'pnlSystemPower': 18}`——dfm 該欄 12 顆 LED 行距 26px、LED 24px 高只剩 2px 縫視覺過緊，且 grpTowerLight 緊接在下無法加大行距；`resize_leds()` pre-pass（在 `pair_led_labels` 前呼叫）把命中容器直屬 LED 縮至 N px 並置中（L/T += (W-N)//2）。縮後 lpRight gap=10 仍 ≤ maxgap 14，配對不受影響。
- **`ROW_REPITCH` 容器內列距展開**（2026-08-27 Set 23）：`{'gbSocketSensor': (15, 29, 4, 16)}`＝(基準Top, 原列距, 每列加距, 群組增高)——該盒 4 列 label(H19)+LED(H14) 內容高 31px > 原列距 29px 造成跨列重疊 2px；`repitch_rows()` 對命中容器子件算 `row=(Top-基準)//列距`、`Top += row*加距`，群組 Height 144→160。lpTop 配對 gt=-2 仍在 −4..10 範圍內，配對不受影響。
- **`RESPACE` 頁籤群組盒垂直加縫**（2026-08-27 Set 23）：`{'tsIndex': 6}`——tsIndex 中欄群組盒 Top 連續相貼（前盒 bottom＝後盒 top）視覺過緊；`respace_stacks()` 只處理「**頁籤直接子 TPanel**」（巢狀 Panel 如 pnlIndexSwitch 不動），其內 TGroupBox/TPanel 依 Top 排序、第 2 個起 `newTop = prevBottom + 6`，Panel Height 取 max(原值, 最後盒 bottom+6)。頁面本身有捲軸，Panel 增高可接受。
- JOB 內 pre-pass 順序：`resize_leds → repitch_rows → respace_stacks → pair_led_labels`（僅 LABELED_LED_JOBS 命中檔）。
- 配對每個 JOB 前須 `LED_PAIR.clear(); LABEL_SKIP.clear()`。

<!-- preserved-content:end -->
