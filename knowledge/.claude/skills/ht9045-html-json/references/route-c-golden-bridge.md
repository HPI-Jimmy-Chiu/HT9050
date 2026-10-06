# C 路（golden 表單橋）— 現行第三條資料路線

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/json/references/route-c-golden-bridge.md)。

## 1. 定義

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/01.md#1-定義)

## 2. 與 A／B 的對照表

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/02.md#2-與-ab-的對照表)

## 3. 端點

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#3-端點)

### 3.0 三種入口（`tools/wb_serve.cpp`，HEAD 8fad1522 行號）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30-三種入口toolswb_servecpphead-8fad1522-行號)

### 3.0a JSON → HTML：`WS editlist.get`（＝golden 開頁 `FormShow`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30a-json--htmlws-editlistgetgolden-開頁-formshow)

### 3.0b HTML → JSON：`WS editlist.save`（＝golden 存檔鈕）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30b-html--jsonws-editlistsavegolden-存檔鈕)

### 3.0c 診斷：`GET /api/editlist/<elConfig|cbLastSet|elConfig_byRecipe|elUdUld>`

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30c-診斷get-apieditlist)

### 3.0d 守衛：`editlist.save` 運轉中一律拒絕（RULINGS_20260927 #7，`f45b92f5`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30d-守衛editlistsave-運轉中一律拒絕rulings_20260927-7f45b92f5)

### 3.0e golden「按下即寫檔」的事件：BeforeApply 重播＋Q14＝B（`3ee547e5`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30e-golden按下即寫檔的事件beforeapply-重播q14b3ee547e5)

### 3.0f 開頁不寫檔的讀法（Q15，Steven 20260927 S137 定案；**計畫中，程式還沒動**）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30f-開頁不寫檔的讀法q15steven-20260927-s137-定案計畫中程式還沒動)

### 3.0g `form.event`（Q40＝A，RULINGS_20260926 S157）——C++ 已做（`76058840`、`4e74e8b4`、`c9d3c932`），頁面送出點還沒有

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-formeventq40arulings_20260926-s157c-已做760588404e74e8b4c9d3c932頁面送出點還沒有)

#### 3.0g-1 現況（20260927 晚，HEAD `db1b7638`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-1-現況20260927-晚head-db1b7638)

#### 3.0g-2 C 路 `RunPageEvent` 的檢查順序（`FileRW/_EditPage.cpp:367-512`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-2-c-路-runpageevent-的檢查順序filerw_editpagecpp367-512)

#### 3.0g-3 回覆格式（程式現況）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-3-回覆格式程式現況)

#### 3.0g-4 怎麼加一個事件（標準流程）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-4-怎麼加一個事件標準流程)

#### 3.0g-5 `4e74e8b4` 留下的三個寫法（Tray Assignment／Temp_Set，可當範本）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-5-4e74e8b4-留下的三個寫法tray-assignmenttemp_set可當範本)

#### 3.0g-6 分頁值 `activePageIndex`（`c9d3c932`，C 路共用層；R102～R106）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-6-分頁值-activepageindexc9d3c932c-路共用層r102r106)

#### 3.0g-7 Configuration 頁（`IniConfig`，不是 PageDesc 的結構怎麼接 form.event；20260927 晚，S158 Q41 CC-E2／CC-E7）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-7-configuration-頁iniconfig不是-pagedesc-的結構怎麼接-formevent20260927-晚s158-q41-cc-e2cc-e7)

#### 3.0g-8 Setup.Speed 滑桿／Setup.Temp_Set 基準點數（20260927 晚，S158 Q41 SP-1～SP-5、TS-7；commit 見 git log）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-8-setupspeed-滑桿setuptemp_set-基準點數20260927-晚s158-q41-sp-1sp-5ts-7commit-見-git-log)

#### 3.0g-9 控制項自己的新值 `position`／`activePageIndex`（B1＝X-2，`7e1785dc`，20260928）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-9-控制項自己的新值-positionactivepageindexb1x-27e1785dc20260928)

#### 3.0g-10 `state` 不能改「有自己事件列」的控制項（`29a13bdb`，B2 頁面工程師查到的缺口）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-10-state-不能改有自己事件列的控制項29a13bdbb2-頁面工程師查到的缺口)

#### 3.0g-11 頁面送出點的寫法（B2 `1b213d11`、B3 `6ba451d5`／`b08ae6ad`，20260928）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-11-頁面送出點的寫法b2-1b213d11b3-6ba451d5b08ae6ad20260928)

#### 3.0g-12 Configuration 頁 B4（`875d3499`，20260928）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30g-12-configuration-頁-b4875d349920260928)

### 3.0h 守衛：開頁／存檔重查 golden 的開窗閘 `kOpenGates`（Q41 C-1／C-2，S158，`cb306f89`；R89～R93）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30h-守衛開頁存檔重查-golden-的開窗閘-kopengatesq41-c-1c-2s158cb306f89r89r93)

### 3.0i 關窗尾段：存檔後補跑 golden 主畫面 `sbXxxClick` 的尾段（Q41 第 3 項，S107-1 延伸；`c8028b21`＋`9268162b`；R84～R87、R107）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30i-關窗尾段存檔後補跑-golden-主畫面-sbxxxclick-的尾段q41-第-3-項s107-1-延伸c8028b219268162br84r87r107)

### 3.0j 頁面補件與 golden 密碼點（Q41 S158：YM-1、CC-L2～L4、SU-L1 延伸；R73～R75）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30j-頁面補件與-golden-密碼點q41-s158ym-1cc-l2l4su-l1-延伸r73r75)

### 3.1 `proxies` 的 `editable` 欄位與「先開後關」規則（20260924 第六輪審查 H1）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#31-proxies-的-editable-欄位與先開後關規則20260924-第六輪審查-h1)

### 3.2 `csDropDown` 清單外文字（20260924 第六輪審查 M2）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#32-csdropdown-清單外文字20260924-第六輪審查-m2)

### 3.3 同一頁有兩型時誰先

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#33-同一頁有兩型時誰先)

### 3.4 A 形狀：`GET /api/form/<Page>` ＋ `WS form.save`（只剩 `Setup.HotPlate.html`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#34-a-形狀get-apiform--ws-formsave只剩-setuphotplatehtml)

### 3.0m 程式設值也觸發 OnClick（`vcl_clicks`，B10b，commit `f14484e1`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/03.md#30m-程式設值也觸發-onclickvcl_clicksb10bcommit-f14484e1)

## 4. C++ 端怎麼來

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/04.md#4-c-端怎麼來)

### 4.1 要改 golden 行為時：改產生器設定、不改產生檔（C 形狀的標準流程）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/04.md#41-要改-golden-行為時改產生器設定不改產生檔c-形狀的標準流程)

## 5. 頁面端規則

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/05.md#5-頁面端規則)

## 6. 與 B 路的分工 ＋ C 路結構總表（單一出處）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/06.md#6-與-b-路的分工--c-路結構總表單一出處)

### C 路結構總表（20260926，HEAD 8fad1522 對程式核對）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/06.md#c-路結構總表20260926head-8fad1522-對程式核對)

## 7. 指標

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/07.md#7-指標)

## 8. 驗證探針

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/08.md#8-驗證探針)

### 3.0k 開頁記 golden 的 "Enter ..."（R101＝RULINGS_20260926 S165，commit `342779cc`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/08.md#30k-開頁記-golden-的-enter-r101rulings_20260926-s165commit-342779cc)

### 3.0l 主畫面事件 act.main.*（批次 B6＋B9，`cd496b52`；RULINGS_20260926 S167／S169、Q47／Q48）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/08.md#30l-主畫面事件-actmain批次-b6b9cd496b52rulings_20260926-s167s169q47q48)

### 3.0n Configuration 頁 Tray／Hot Plate 分頁：WS `cfgtrayplate.op`（S98＋S169，todo E-003 ①，20261001）

[讀取此節](../../hpi-web-hmi/references/json/references/route-c-golden-bridge/08.md#30n-configuration-頁-trayhot-plate-分頁ws-cfgtrayplateops98s169todo-e-003-①20261001)
