# 方案：六個硬體／馬達畫面改用 C++ 內建表單（原生視窗），其餘維持 HTML（20260928）

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/native/references/native-forms-plan.md)。

## 0. 一句話結論

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#0-一句話結論)

## 1. 直接回答 Steven 的兩個理由

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#1-直接回答-steven-的兩個理由)

### 1.1 「大量的 JSON 傳輸」——今天每一頁到底傳多少（從程式算，估計值標明）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#11-大量的-json-傳輸今天每一頁到底傳多少從程式算估計值標明)

### 1.2 「與安全性高度相關」——今天怎麼擋、原生表單好在哪、新風險是什麼

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#12-與安全性高度相關今天怎麼擋原生表單好在哪新風險是什麼)

## 2. 移植樹今天有什麼（原生表單相關）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#2-移植樹今天有什麼原生表單相關)

## 3. 候選做法

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#3-候選做法)

### A 案：純 Win32 對話框（dfm2rc 的 .rc）＋復原 GA-4 引擎＋golden 處理器直接綁定 — **建議**

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#a-案純-win32-對話框dfm2rc-的-rc復原-ga-4-引擎golden-處理器直接綁定--建議)

### B 案：把 vclcompat 的 TForm／控制項做成真視窗，讓翻譯好的 golden .cpp 幾乎不改就跑

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#b-案把-vclcompat-的-tform控制項做成真視窗讓翻譯好的-golden-cpp-幾乎不改就跑)

### C 案：第三方 GUI 工具箱（一句話各評）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#c-案第三方-gui-工具箱一句話各評)

### D 案：獨立 exe（原生「硬體操作台」程序）經 loopback 與 wb_serve 對話

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#d-案獨立-exe原生硬體操作台程序經-loopback-與-wb_serve-對話)

## 4. 關鍵技術限制與對應

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#4-關鍵技術限制與對應)

## 5. 與 20260812 架構定案的衝突，以及最窄的例外

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#5-與-20260812-架構定案的衝突以及最窄的例外)

## 6. 對這六頁，原生相對於網頁的優勢

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#6-對這六頁原生相對於網頁的優勢)

## 7. 方案（建議採 A 案＋E2）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#7-方案建議採-a-案e2)

### 7.1 架構（六張原生表單）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#71-架構六張原生表單)

### 7.2 分期與工作量（估計；一位熟這棵樹的工程師；不含 Jimmy 審查與真機驗證）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#72-分期與工作量估計一位熟這棵樹的工程師不含-jimmy-審查與真機驗證)

### 7.3 第一頁（HW.IoSetView）的驗收清單

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#73-第一頁hwiosetview的驗收清單)

### 7.4 風險與對策

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#74-風險與對策)

### 7.5 維持 web 的部分（不動）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#75-維持-web-的部分不動)

### 7.6 開工前 Steven 必須決定的題

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#76-開工前-steven-必須決定的題)

### 7.7 「用 define 隔開 C++ form／HTML form」——可以嗎？可以；設計如下（名稱皆為**提案**）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#77-用-define-隔開-c-formhtml-form可以嗎可以設計如下名稱皆為提案)

#### 7.7.1 兩個選項

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#771-兩個選項)

#### 7.7.2 建議：(ii)

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#772-建議ii)

#### 7.7.3 開站／開窗流程（(ii) 模式，以 IoSetView 為例）

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#773-開站開窗流程ii-模式以-iosetview-為例)

## 8. 沒查證／推論的地方

[讀取此節](../../hpi-web-hmi/references/native/references/native-forms-plan.md#8-沒查證推論的地方)
