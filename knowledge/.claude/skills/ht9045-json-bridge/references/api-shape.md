# §四、API 形狀 —— 全文

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/bridge/references/api-shape.md)。

## 四、API 形狀

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#四api-形狀)

### 4.1 核心切分：型別表產生、實例綁定手寫、頁面投影沿用 pagewire

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#41-核心切分型別表產生實例綁定手寫頁面投影沿用-pagewire)

### 4.2 端點

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#42-端點)

### 4.3 C++ function 簽章（Steven 提供）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#43-c-function-簽章steven-提供)

### 4.5 動作通道（command）：`act.<單元>.<動作>`

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#45-動作通道commandact)

### 4.6 開機配置廣播與 `cfg.resync`（使用者 20260923：「`#define SOFT_SIMULTE`／`HW.HandlerSys.html` 之類的定義也要有 JSON 傳輸，而且一開機就該傳；C++ 先開、HTML 後開時，要有 HTML→C++ 的訊號叫它重送機台配置檔」）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#46-開機配置廣播與-cfgresync使用者-20260923define-soft_simultehwhandlersyshtml-之類的定義也要有-json-傳輸而且一開機就該傳c-先開html-後開時要有-htmlc-的訊號叫它重送機台配置檔)

### 4.7 event log 通道：`log.event`（使用者 20260923 指定看 `RecordProcess`／`NewRecordProcess`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#47-event-log-通道logevent使用者-20260923-指定看-recordprocessnewrecordprocess)

### 4.8 執行期顯示 producer（`Show*`／`Update*` 族）—— 生產通道的 C++ 端

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#48-執行期顯示-producershowupdate-族-生產通道的-c-端)

### 4.9 檔案佈局（使用者 20260923：可以分成多個不同的 cpp）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#49-檔案佈局使用者-20260923可以分成多個不同的-cpp)

### 4.8 執行期顯示 producer（`Show*`／`Update*` 族）—— 生產通道的 C++ 端（使用者 20260923：「`TfTemperFrom::ShowThermo` 的 `sprintf("%5.1f", UN150Read[Addr])` 也要有 JSON，屬於跟 `.live` 一樣的動作」）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#48-執行期顯示-producershowupdate-族-生產通道的-c-端使用者-20260923tftemperfromshowthermo-的-sprintf51f-un150readaddr-也要有-json屬於跟-live-一樣的動作)

#### 4.8 補記 —— S7 第一刀實際做成什麼（20260923，與上面規格的差異逐條說明）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#48-補記--s7-第一刀實際做成什麼20260923與上面規格的差異逐條說明)

### 4.9 檔案佈局（使用者 20260923：可以分成多個不同的 cpp）

[讀取此節](../../hpi-web-hmi/references/bridge/references/api-shape/01.md#49-檔案佈局使用者-20260923可以分成多個不同的-cpp)
