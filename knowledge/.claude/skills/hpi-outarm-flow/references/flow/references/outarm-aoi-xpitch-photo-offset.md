# Scanner AOI 拍照位置偏移 — X-Pitch 出界 + 吸嘴欄位映射

按需要選取以下章節，原文依順序保留。

- [Scanner AOI 拍照位置偏移 — X-Pitch 出界 + 吸嘴欄位映射](outarm-aoi-xpitch-photo-offset/00.md)
- [1. 症狀分層（兩個不同的問題，別混在一起）](outarm-aoi-xpitch-photo-offset/01.md)
- [2. P1：X-Pitch 出界撞止檔失步（已修，背景）](outarm-aoi-xpitch-photo-offset/02.md)
- [3. P2：拍照位置算錯（本文重點）](outarm-aoi-xpitch-photo-offset/03.md)
- [4. 修法](outarm-aoi-xpitch-photo-offset/04.md)
- [5. ⚠ teach / offset：改哪一層](outarm-aoi-xpitch-photo-offset/05.md)
- [6. 診斷用探針（驗證完可移除）](outarm-aoi-xpitch-photo-offset/06.md)
- [7. 教訓（LL）](outarm-aoi-xpitch-photo-offset/07.md)
- [7b. ⏸ 已知未修：Y 方向的同型缺陷（RogerYang 20260902 裁決暫緩）](outarm-aoi-xpitch-photo-offset/08.md)
- [8. 相關](outarm-aoi-xpitch-photo-offset/09.md)

# Scanner AOI 拍照位置偏移 — X-Pitch 出界 + 吸嘴欄位映射

[讀取此節](outarm-aoi-xpitch-photo-offset/00.md#scanner-aoi-拍照位置偏移--x-pitch-出界--吸嘴欄位映射)

## 1. 症狀分層（兩個不同的問題，別混在一起）

[讀取此節](outarm-aoi-xpitch-photo-offset/01.md#1-症狀分層兩個不同的問題別混在一起)

## 2. P1：X-Pitch 出界撞止檔失步（已修，背景）

[讀取此節](outarm-aoi-xpitch-photo-offset/02.md#2-p1x-pitch-出界撞止檔失步已修背景)

### clamp 未涵蓋的孿生函式（已知缺口）

[讀取此節](outarm-aoi-xpitch-photo-offset/02.md#clamp-未涵蓋的孿生函式已知缺口)

## 3. P2：拍照位置算錯（本文重點）

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#3-p2拍照位置算錯本文重點)

### 3.1 出問題的算式

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#31-出問題的算式)

### 3.2 吸嘴欄位映射（B 的機制，容易看漏）

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#32-吸嘴欄位映射b-的機制容易看漏)

### 3.3 數字（HHT-532）

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#33-數字hht-532)

### 3.4 交叉驗證（取料端是對的）

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#34-交叉驗證取料端是對的)

### 3.5 ⚠ 兩個錯必須一起修（只修一個會更糟）

[讀取此節](outarm-aoi-xpitch-photo-offset/03.md#35--兩個錯必須一起修只修一個會更糟)

## 4. 修法

[讀取此節](outarm-aoi-xpitch-photo-offset/04.md#4-修法)

### 4.1 🚨 升版影響面：**單顆拍照也會位移，全部要重 teach**

[讀取此節](outarm-aoi-xpitch-photo-offset/04.md#41--升版影響面單顆拍照也會位移全部要重-teach)

### 4.2 重 teach 換算式（免現場試誤）

[讀取此節](outarm-aoi-xpitch-photo-offset/04.md#42-重-teach-換算式免現場試誤)

### 4.3 單顆（SingleSite）機台的 dT 全表 — **不是單一數字，共 5 種**

[讀取此節](outarm-aoi-xpitch-photo-offset/04.md#43-單顆singlesite機台的-dt-全表--不是單一數字共-5-種)

### 4.4 多顆 / 其他

[讀取此節](outarm-aoi-xpitch-photo-offset/04.md#44-多顆--其他)

## 5. ⚠ teach / offset：改哪一層

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#5--teach--offset改哪一層)

### 5.1 兩層補償的層級與**容量**

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#51-兩層補償的層級與容量)

### 5.2 為什麼沒有「不動 teach」的公式解

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#52-為什麼沒有不動-teach的公式解)

### 5.3 升版程序：teach 是**機台級一次性**動作

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#53-升版程序teach-是機台級一次性動作)

### 5.3.1 ★關鍵：修正後 teach 是「一台一個常數」，切模式**不需要**再動

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#531-關鍵修正後-teach-是一台一個常數切模式不需要再動)

### 5.3.2 那為什麼「不同模式的 dT 不一樣」？

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#532-那為什麼不同模式的-dt-不一樣)

### 5.3.3 由此得到一個可驗證的預測

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#533-由此得到一個可驗證的預測)

### 5.4 ⚠ AOI teach 點的語意＝**基準軸位置**，不是「第一顆的位置」

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#54--aoi-teach-點的語意基準軸位置不是第一顆的位置)

#### 客戶證言（偉測，20260902）— 完全對上

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#客戶證言偉測20260902-完全對上)

#### ★ 因此升版後的現場程序極簡化（不需要算 dT）

[讀取此節](outarm-aoi-xpitch-photo-offset/05.md#-因此升版後的現場程序極簡化不需要算-dt)

## 6. 診斷用探針（驗證完可移除）

[讀取此節](outarm-aoi-xpitch-photo-offset/06.md#6-診斷用探針驗證完可移除)

### 6.1 log 節流的坑

[讀取此節](outarm-aoi-xpitch-photo-offset/06.md#61-log-節流的坑)

## 7. 教訓（LL）

[讀取此節](outarm-aoi-xpitch-photo-offset/07.md#7-教訓ll)

## 7b. ⏸ 已知未修：Y 方向的同型缺陷（RogerYang 20260902 裁決暫緩）

[讀取此節](outarm-aoi-xpitch-photo-offset/08.md#7b--已知未修y-方向的同型缺陷rogeryang-20260902-裁決暫緩)

## 8. 相關

[讀取此節](outarm-aoi-xpitch-photo-offset/09.md#8-相關)
