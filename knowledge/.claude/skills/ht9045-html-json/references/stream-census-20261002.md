# tag 串流普查與「少送」的空間（20261002 實測，Jerry）

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/json/references/stream-census-20261002.md)。

## 0. 先講三個會讓人白做工的坑

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#0-先講三個會讓人白做工的坑)

### 0.1 ⛔ 不要自己寫 WebSocket client —— 樹裡已經有兩支

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#01--不要自己寫-websocket-client--樹裡已經有兩支)

### 0.2 ⛔ `W906_OPLOG_DIR` 開著的時候，`[STREAM]` 的 apiCache 數字是灌水的

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#02--w906_oplog_dir-開著的時候stream-的-apicache-數字是灌水的)

### 0.2b ⛔⛔ `W906_OPLOG_DIR` 是「每個啟動設定各自決定」的，不是全域開關

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#02b--w906_oplog_dir-是每個啟動設定各自決定的不是全域開關)

### 0.2c 拿不到 `[STREAM]` 的時候怎麼辦（20261002 實際遇到）

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#02c-拿不到-stream-的時候怎麼辦20261002-實際遇到)

### 0.2a 量測前的「無磁碟」檢查單（20261002 Jerry 實測）

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#02a-量測前的無磁碟檢查單20261002-jerry-實測)

### 0.3 這台機器 `python` 不能用，要用 `py`

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/01.md#03-這台機器-python-不能用要用-py)

## 1. 「四大類」在程式裡不存在

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/02.md#1-四大類在程式裡不存在)

### 1.1 線上格式是完全扁平的

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/02.md#11-線上格式是完全扁平的)

### 1.2 程式裡實際存在的是三套互不相通的局部分類

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/02.md#12-程式裡實際存在的是三套互不相通的局部分類)

## 2. 20261002 的普查數字

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/03.md#2-20261002-的普查數字)

### 2.1 「有發無人收」的家族分佈

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/03.md#21-有發無人收的家族分佈)

### 2.2 `secs.*` 772 個 —— 整個網頁樹一次都沒出現

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/03.md#22-secs-772-個--整個網頁樹一次都沒出現)

### 2.3 20 秒內真的變過的只有 6 個

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/03.md#23-20-秒內真的變過的只有-6-個)

## 3. 開頁閘：機制已經在跑，但只接了 3 個家族

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/04.md#3-開頁閘機制已經在跑但只接了-3-個家族)

## 4. ⚠ 靜態分析定不出「哪個 tag 屬於哪一頁」

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/05.md#4--靜態分析定不出哪個-tag-屬於哪一頁)

## 5. 可以省多少（20261002 閒置基準，1790 tags / 56,189 B）

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/06.md#5-可以省多少20261002-閒置基準1790-tags--56189-b)

### 5.1 ⚠ 但要先搞清楚 A／B 省的是**CPU**，不是頻寬

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/06.md#51--但要先搞清楚-ab-省的是cpu不是頻寬)

## 6. 本檔的量測限制（引用前務必看）

[讀取此節](../../hpi-web-hmi/references/json/references/stream-census-20261002/07.md#6-本檔的量測限制引用前務必看)
