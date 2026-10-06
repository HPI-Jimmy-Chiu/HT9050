# WAR0335「RTC arm 1 error!」— Auto Clean 後 RTC 不回應 @ARM2+

> 內容已分層整理，舊路徑保留供既有引用使用。

[讀取整理後文件](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md)

## 目錄

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#目錄)

## 1. 症狀簽章（30 秒辨識）

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#1-症狀簽章30-秒辨識)

## 2. RTC Index Arm 握手機制

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#2-rtc-index-arm-握手機制)

## 3. 根因：r780 把 Full View 預設全域打開

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#3-根因r780-把-full-view-預設全域打開)

## 4. 發作需要的三個條件

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#4-發作需要的三個條件)

## 5. 為何機台會自行復原（且每 3 次才報警一次）

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#5-為何機台會自行復原且每-3-次才報警一次)

## 6. 修法

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#6-修法)

### 方案 B（已採用，20260815 進 908.12）— 補上 Full View 收尾

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#方案-b已採用20260815-進-90812-補上-full-view-收尾)

### 方案 A（備援）— 客戶碼關掉 Full View

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#方案-a備援-客戶碼關掉-full-view)

## 7. 驗證判準

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#7-驗證判準)

## 8. 判讀陷阱與教訓

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#8-判讀陷阱與教訓)

## 9. 程式碼錨點表

[讀取此節](../../hpi-index-flow/references/flow/rtc-autoclean-war0335.md#9-程式碼錨點表)
