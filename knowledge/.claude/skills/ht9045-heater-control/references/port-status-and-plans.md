# 移植樹溫控現況、底層待改清單、兩份混廠牌方案對照

舊引用路徑保留；[讀取整理後文件](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md)。

## 1. 移植樹現況（事實）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#1-移植樹現況事實)

### 1.1 HandlerSys 頁（St01 的檔）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#11-handlersys-頁st01-的檔)

### 1.2 溫控底層（Jimmy 的檔）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#12-溫控底層jimmy-的檔)

## 2. 底層待改清單（方案 D 第 ⑥ 點，歸 Jimmy，**沒有人做**）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#2-底層待改清單方案-d-第-⑥-點歸-jimmy沒有人做)

## 3. 兩份混廠牌方案對照（都還沒實作）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#3-兩份混廠牌方案對照都還沒實作)

## 4. 例子

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#4-例子)

## 5. 方案 D 的實作（St01，20260927，commit `bc970c38`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#5-方案-d-的實作st0120260927commit-bc970c38)

### 5.1 改了哪些檔

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#51-改了哪些檔)

### 5.2 檔案格式（`D:\HT9045\system\Gerneral.ini` 的 `[TempCtrl]`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#52-檔案格式dht9045systemgerneralini-的-tempctrl)

### 5.3 讀檔（開頁、以及底層之後第一次跑溫控，都不寫檔）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#53-讀檔開頁以及底層之後第一次跑溫控都不寫檔)

### 5.4 存檔（`SaveSystemSet` 答「是」之後）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#54-存檔savesystemset-答是之後)

### 5.5 頁面（`D:\HT9045\web\page\ht9045_hsys_heater_c.js`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#55-頁面dht9045webpageht9045_hsys_heater_cjs)

### 5.6 St01 的解讀（裁決沒寫到的細節；已列給 Steven 確認）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#56-st01-的解讀裁決沒寫到的細節已列給-steven-確認)

### 5.7 給 Jimmy（底層 ⑥）的介面

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#57-給-jimmy底層-⑥的介面)

### 5.8 驗證（20260927）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#58-驗證20260927)

## 6. E-029 的實作（St01，20261002）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/port-status-and-plans.md#6-e-029-的實作st0120261002)
