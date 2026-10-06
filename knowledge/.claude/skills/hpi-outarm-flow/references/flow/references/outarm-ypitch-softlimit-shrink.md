# OutArm Y-Pitch 縮 pitch 換行程 —— 大 IC 前後排互撞（JAM0203）

按需要選取以下章節，原文依順序保留。

- [OutArm Y-Pitch 縮 pitch 換行程 —— 大 IC 前後排互撞（JAM0203）](outarm-ypitch-softlimit-shrink/00.md)
- [0. ⚠ 讀碼前必看：磁碟上有 5 個 OutArm 模式檔不在 `HT9045.bpr` 裡](outarm-ypitch-softlimit-shrink/01.md)
- [1. 什麼時候載入這份文件](outarm-ypitch-softlimit-shrink/02.md)
- [2. 機制（為什麼會撞）](outarm-ypitch-softlimit-shrink/03.md)
- [3. 芯云 PPLD2924 實測數字（可拿來當判讀樣板）](outarm-ypitch-softlimit-shrink/04.md)
- [4. 程式鏈（實際會執行的那條）](outarm-ypitch-softlimit-shrink/05.md)
- [5. 修法（20260831，已進 908.16_AI）](outarm-ypitch-softlimit-shrink/06.md)
- [6. 硬體 / 設定面的解（優先於程式解）](outarm-ypitch-softlimit-shrink/07.md)
- [7. 驗證測項（風險轉負向測試）](outarm-ypitch-softlimit-shrink/08.md)
- [8. 這台機的三個配置查證（分析時容易踩空，先確認再推論）](outarm-ypitch-softlimit-shrink/09.md)
- [9. 待辦 / 待觀察](outarm-ypitch-softlimit-shrink/10.md)
- [10. 相關](outarm-ypitch-softlimit-shrink/11.md)

# OutArm Y-Pitch 縮 pitch 換行程 —— 大 IC 前後排互撞（JAM0203）

[讀取此節](outarm-ypitch-softlimit-shrink/00.md#outarm-y-pitch-縮-pitch-換行程--大-ic-前後排互撞jam0203)

## 0. ⚠ 讀碼前必看：磁碟上有 5 個 OutArm 模式檔不在 `HT9045.bpr` 裡

[讀取此節](outarm-ypitch-softlimit-shrink/01.md#0--讀碼前必看磁碟上有-5-個-outarm-模式檔不在-ht9045bpr-裡)

## 1. 什麼時候載入這份文件

[讀取此節](outarm-ypitch-softlimit-shrink/02.md#1-什麼時候載入這份文件)

## 2. 機制（為什麼會撞）

[讀取此節](outarm-ypitch-softlimit-shrink/03.md#2-機制為什麼會撞)

### 為什麼是偶發

[讀取此節](outarm-ypitch-softlimit-shrink/03.md#為什麼是偶發)

### 🔎 最有力的旁證：InArm 側早就修過同一個病，OutArm 側沒跟上

[讀取此節](outarm-ypitch-softlimit-shrink/03.md#-最有力的旁證inarm-側早就修過同一個病outarm-側沒跟上)

## 3. 芯云 PPLD2924 實測數字（可拿來當判讀樣板）

[讀取此節](outarm-ypitch-softlimit-shrink/04.md#3-芯云-ppld2924-實測數字可拿來當判讀樣板)

### ⚠ 「39×45」的證據強度（別當事實引用）

[讀取此節](outarm-ypitch-softlimit-shrink/04.md#-3945的證據強度別當事實引用)

### StateRecord 判讀捷徑

[讀取此節](outarm-ypitch-softlimit-shrink/04.md#staterecord-判讀捷徑)

## 4. 程式鏈（實際會執行的那條）

[讀取此節](outarm-ypitch-softlimit-shrink/05.md#4-程式鏈實際會執行的那條)

### 缺陷分佈（原始碼有 7 處，但只有 2 處會執行）

[讀取此節](outarm-ypitch-softlimit-shrink/05.md#缺陷分佈原始碼有-7-處但只有-2-處會執行)

## 5. 修法（20260831，已進 908.16_AI）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#5-修法20260831已進-90816_ai)

### 設計取向（兩層）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#設計取向兩層)

### 幾何模型：吸嘴在 IC 正中央

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#幾何模型吸嘴在-ic-正中央)

### `DeviceForm.X/YDimension` 就是 IC 外型尺寸（我一開始判錯，已更正）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#deviceformxydimension-就是-ic-外型尺寸我一開始判錯已更正)

### 必須配逃生出口，否則變靜默死迴圈

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#必須配逃生出口否則變靜默死迴圈)

### ⛔ 不可改 `AutoCalculateOutArmYClosePitch()`

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#-不可改-autocalculateoutarmyclosepitch)

### 「縮不夠就跳過該格」為什麼不是 regression

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#縮不夠就跳過該格為什麼不是-regression)

### 落點與帳為何不變（連動失效檢查結論）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#落點與帳為何不變連動失效檢查結論)

### 邊界（新碼比舊碼更安全的一點）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#邊界新碼比舊碼更安全的一點)

### 實際改動（只有 3 個檔案，全部在 `HT9045.bpr` 內）

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#實際改動只有-3-個檔案全部在-ht9045bpr-內)

### 動手順序提醒

[讀取此節](outarm-ypitch-softlimit-shrink/06.md#動手順序提醒)

## 6. 硬體 / 設定面的解（優先於程式解）

[讀取此節](outarm-ypitch-softlimit-shrink/07.md#6-硬體--設定面的解優先於程式解)

## 7. 驗證測項（風險轉負向測試）

[讀取此節](outarm-ypitch-softlimit-shrink/08.md#7-驗證測項風險轉負向測試)

## 8. 這台機的三個配置查證（分析時容易踩空，先確認再推論）

[讀取此節](outarm-ypitch-softlimit-shrink/09.md#8-這台機的三個配置查證分析時容易踩空先確認再推論)

## 9. 待辦 / 待觀察

[讀取此節](outarm-ypitch-softlimit-shrink/10.md#9-待辦--待觀察)

## 10. 相關

[讀取此節](outarm-ypitch-softlimit-shrink/11.md#10-相關)
