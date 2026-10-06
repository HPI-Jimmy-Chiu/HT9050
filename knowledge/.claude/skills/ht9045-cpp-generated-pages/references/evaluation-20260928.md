# 【已否決的解讀，只留對照】評估：六個硬體／馬達頁面改由 C++ 產生 HTML（20260928 上午）

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/native/references/evaluation-20260928.md)。

## 0. 摘要（五行）

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#0-摘要五行)

## 1. 六頁今天的樣子

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#1-六頁今天的樣子)

### 1.1 `D:\HT9045\web\page\Main.MotorView.html`（276 行）— 全部馬達的位置與極限感測器一覽

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#11-dht9045webpagemainmotorviewhtml276-行-全部馬達的位置與極限感測器一覽)

### 1.2 `D:\HT9045\web\page\HW.home.html`（107 行）— Home Monitor（回原點進度）

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#12-dht9045webpagehwhomehtml107-行-home-monitor回原點進度)

### 1.3 `D:\HT9045\web\page\HW.IoSetView.html`（830 行）— IO 檢查與輸出測試

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#13-dht9045webpagehwiosetviewhtml830-行-io-檢查與輸出測試)

### 1.4 `D:\HT9045\web\page\HW.MotorTest.html`（1,935 行）— 單軸馬達測試

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#14-dht9045webpagehwmotortesthtml1935-行-單軸馬達測試)

### 1.5 `D:\HT9045\web\page\HW.ShuttleMove.html`（116 行）— Shuttle 維護

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#15-dht9045webpagehwshuttlemovehtml116-行-shuttle-維護)

### 1.6 `D:\HT9045\web\page\HW.teach.html`（760 行）— 教導（Teaching）

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#16-dht9045webpagehwteachhtml760-行-教導teaching)

## 2. 「C++ 直接生成」的三種解釋

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#2-c-直接生成的三種解釋)

### (a) C++ 執行期產 HTML

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#a-c-執行期產-html)

### (b) C++ 送 JSON 畫面描述，一支通用 JS 渲染器畫所有頁

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#b-c-送-json-畫面描述一支通用-js-渲染器畫所有頁)

### (c) 建置期產生器（既有 Python，或改寫成 C++ 工具）

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#c-建置期產生器既有-python或改寫成-c-工具)

## 3. 每個選項的難度與工作量（估計；一位熟這棵樹的工程師；不含真機驗證與 Jimmy 審查）

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#3-每個選項的難度與工作量估計一位熟這棵樹的工程師不含真機驗證與-jimmy-審查)

## 4. 優勢

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#4-優勢)

## 5. 缺點與風險

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#5-缺點與風險)

## 6. 建議與試點

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#6-建議與試點)

## 7. 要 Steven 決定的題

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#7-要-steven-決定的題)

## 8. 沒查證／推論的地方

[讀取此節](../../hpi-web-hmi/references/native/references/evaluation-20260928.md#8-沒查證推論的地方)
