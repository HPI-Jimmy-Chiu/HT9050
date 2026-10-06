# B 路（wb_serve）— 現行資料路線的詳細規格

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/json/references/route-b-wbserve.md)。

## 兩條路對照與 wb_serve 端點現況

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/01.md#兩條路對照與-wb_serve-端點現況)

### 四大類在 B 路（wb_serve）的實際狀態與端點

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/01.md#四大類在-b-路wb_serve的實際狀態與端點)

#### ⚠ 上表的端點都是「檔案鏡像」；C++ 結構層另算（Steven 20260924 實測）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/01.md#-上表的端點都是檔案鏡像c-結構層另算steven-20260924-實測)

### 執行期 tag 的實況（20260916 實測，不是推估）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/01.md#執行期-tag-的實況20260916-實測不是推估)

### 為什麼 96 個裡有 87 個是 null —— 不是「producer 缺」，是一個 early return

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/01.md#為什麼-96-個裡有-87-個是-null--不是producer-缺是一個-early-return)

## 同五個分類在 B 路（wb_serve）的對應（20260916 實測）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/02.md#同五個分類在-b-路wb_serve的對應20260916-實測)

## ⚠ 值域（min／max）：兩邊都要有（使用者裁決 20260916）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/03.md#-值域minmax兩邊都要有使用者裁決-20260916)

## 20260916 升級：衝突一律走 wb_serve

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/04.md#20260916-升級衝突一律走-wb_serve)

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫（20260915 實測）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/05.md#apisystem-與-apitext--機台檔案的即時讀寫20260915-實測)

### `/api/system/` 的 40 支（35 支本機實檔存在，5 支此環境未部署）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/05.md#apisystem-的-40-支35-支本機實檔存在5-支此環境未部署)

### `/api/text/` 的 6 個來源（唯讀）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/05.md#apitext-的-6-個來源唯讀)

### 五條設計約束，改這段程式前先讀

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/05.md#五條設計約束改這段程式前先讀)

## 待辦（本 skill 的落地缺口，20260916 重新盤點）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#待辦本-skill-的落地缺口20260916-重新盤點)

### B 路（wb_serve）——目前的接線分級（**20260919 實測 68 頁**）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#b-路wb_serve目前的接線分級20260919-實測-68-頁)

### ⛔ A 路（JSON Simulator）已於 20260918 全面退場

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#-a-路json-simulator已於-20260918-全面退場)

### tag → widget 對照：**61 / 4608**（20260919 實測）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#tag--widget-對照61--460820260919-實測)

### 引擎支援的 tag 綁定模式（`ht9045_wire_engine.js` 的 `tagApply`）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#引擎支援的-tag-綁定模式ht9045_wire_enginejs-的-tagapply)

### 還沒做的

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#還沒做的)

### 相關契約與規範（20260918 新增）

[讀取此節](../../hpi-web-hmi/references/json/references/route-b-wbserve/06.md#相關契約與規範20260918-新增)
