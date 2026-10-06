# 寫檔盤點 —— 以結構為主軸（S12 第二型的工作清單）

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/bridge/references/write-inventory.md)。

### 〇、頁面讀寫總表（20260925 `008db55e` 時點；每次整合或測完都要更新這張）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/00.md#〇頁面讀寫總表20260925-008db55e-時點每次整合或測完都要更新這張)

### 〇之二、Data.*.html 顯示頁（20260925 `008db55e` 時點）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/00.md#〇之二datahtml-顯示頁20260925-008db55e-時點)

## 一、寫檔的四種形狀（決定 bridge 怎麼做）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/01.md#一寫檔的四種形狀決定-bridge-怎麼做)

### 一之二、C 類 HTEditList 的寫檔設計（Steven 20260924，含高級審查員第二輪意見）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/01.md#一之二c-類-hteditlist-的寫檔設計steven-20260924含高級審查員第二輪意見)

### 一之三、D 類的做法（使用者 20260924：「可以用一個暫存的結構去接 JSON 的值」）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/01.md#一之三d-類的做法使用者-20260924可以用一個暫存的結構去接-json-的值)

### 一之四、公式衍生欄位的存檔：移植公式＋存檔前重算，不要因為缺公式就整組拒存（20260925）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/01.md#一之四公式衍生欄位的存檔移植公式存檔前重算不要因為缺公式就整組拒存20260925)

### 一之五、golden `TfMain` 建構子裡「移植樹沒做的初始化」——轉表單前要查（20260925）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/01.md#一之五golden-tfmain-建構子裡移植樹沒做的初始化轉表單前要查20260925)

## 二、逐結構清單

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#二逐結構清單)

### `TestIF_File`（SYSTEM_TEST_IF，788 欄）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#testif_filesystem_test_if788-欄)

### `DeviceForm_File`（SYSTEM_DEVICE_FORM）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#deviceform_filesystem_device_form)

### `Temperature`（SYSTEM_TEMPERATURE）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#temperaturesystem_temperature)

### `TestMode`（SYSTEM_TEST_MODE）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#testmodesystem_test_mode)

### `IniConfig`（config.ini）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#iniconfigconfigini)

### `LastSet`（LAST_GENERAL_SET，lastdata.dat）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#lastsetlast_general_setlastdatadat)

### `LevelSet`（LAST_LEVEL_SET，levelset.dat）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#levelsetlast_level_setlevelsetdat)

### 其餘結構

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#其餘結構)

### Gerneral.ini（形狀 E，`WriteIniDataGeneral`；20260924 補）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#gerneralini形狀-ewriteinidatageneral20260924-補)

### `TfMain` 開機／關程式／計時器的手寫 FileRW（20260927 補，St01；全部只編進 `wb_serve`，`CMakeLists.txt:3404`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/02.md#tfmain-開機關程式計時器的手寫-filerw20260927-補st01全部只編進-wb_servecmakeliststxt3404)

## 三、順序（Steven 20260924 暫定，依「有頁面＋讀檔端已在」優先）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/03.md#三順序steven-20260924-暫定依有頁面讀檔端已在優先)

## 四、golden `cprod.h` 全域變數的讀寫盤點（20260924；使用者：「根據定義的變數與結構，如果有被存檔與寫檔的，都要根據他的讀寫檔方式，生成到 FileRW；如果有對應的元件，那就是要給 html 用的，需要跟著 JSON 發送」）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/04.md#四golden-cprodh-全域變數的讀寫盤點20260924使用者根據定義的變數與結構如果有被存檔與寫檔的都要根據他的讀寫檔方式生成到-filerw如果有對應的元件那就是要給-html-用的需要跟著-json-發送)

### 四之二、FileRW 與 JsonBridge／HTML 要對齊的事（工程師檢查 20260924）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/04.md#四之二filerw-與-jsonbridgehtml-要對齊的事工程師檢查-20260924)

## 🆕 進行中（20260926 15:30，更正舊敘述）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/05.md#-進行中20260926-1530更正舊敘述)

## 🆕 待派佇列（20260926）

[讀取此節](../../hpi-web-hmi/references/bridge/references/write-inventory/06.md#-待派佇列20260926)
