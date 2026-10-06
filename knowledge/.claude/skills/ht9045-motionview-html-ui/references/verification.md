# 驗證清單與已知缺陷

舊引用路徑保留；[讀取整理後文件](../../hpi-motionview/references/template/references/verification.md)。

## 目錄

[讀取此節](../../hpi-motionview/references/template/references/verification.md#目錄)

## 1. 怎麼跑驗證

[讀取此節](../../hpi-motionview/references/template/references/verification.md#1-怎麼跑驗證)

## 2. 不變量清單

[讀取此節](../../hpi-motionview/references/template/references/verification.md#2-不變量清單)

### 2.1 料件守恆（最重要）

[讀取此節](../../hpi-motionview/references/template/references/verification.md#21-料件守恆最重要)

### 2.2 容量

[讀取此節](../../hpi-motionview/references/template/references/verification.md#22-容量)

### 2.3 動作者互斥

[讀取此節](../../hpi-motionview/references/template/references/verification.md#23-動作者互斥)

### 2.4 Socket 資源互斥

[讀取此節](../../hpi-motionview/references/template/references/verification.md#24-socket-資源互斥)

### 2.5 依賴滿足

[讀取此節](../../hpi-motionview/references/template/references/verification.md#25-依賴滿足)

### 2.6 取料規則：吸滿才放

[讀取此節](../../hpi-motionview/references/template/references/verification.md#26-取料規則吸滿才放)

### 2.7 Fixed（One by one）模式

[讀取此節](../../hpi-motionview/references/template/references/verification.md#27-fixedone-by-one模式)

### 2.8 列優先

[讀取此節](../../hpi-motionview/references/template/references/verification.md#28-列優先)

### 2.9 幾何一致性

[讀取此節](../../hpi-motionview/references/template/references/verification.md#29-幾何一致性)

### 2.10 HotPlate

[讀取此節](../../hpi-motionview/references/template/references/verification.md#210-hotplate)

### 2.11 兩條 Shuttle 的公平性

[讀取此節](../../hpi-motionview/references/template/references/verification.md#211-兩條-shuttle-的公平性)

## 3. 已知缺陷（都真的發生過）

[讀取此節](../../hpi-motionview/references/template/references/verification.md#3-已知缺陷都真的發生過)

### 3.1 時間順序 vs build 順序

[讀取此節](../../hpi-motionview/references/template/references/verification.md#31-時間順序-vs-build-順序)

### 3.2 Kit 的 Site 槽位撞號

[讀取此節](../../hpi-motionview/references/template/references/verification.md#32-kit-的-site-槽位撞號)

### 3.3 放料前吸嘴沒吸滿

[讀取此節](../../hpi-motionview/references/template/references/verification.md#33-放料前吸嘴沒吸滿)

### 3.4 HotPlate 取料用列號升序

[讀取此節](../../hpi-motionview/references/template/references/verification.md#34-hotplate-取料用列號升序)

### 3.5 配位游標沒有滾動

[讀取此節](../../hpi-motionview/references/template/references/verification.md#35-配位游標沒有滾動)

### 3.6 圖層順序把逐格點蓋掉

[讀取此節](../../hpi-motionview/references/template/references/verification.md#36-圖層順序把逐格點蓋掉)

### 3.7 手臂/機構停在干涉區

[讀取此節](../../hpi-motionview/references/template/references/verification.md#37-手臂機構停在干涉區)

### 3.8 測試腳本自己的假設過時

[讀取此節](../../hpi-motionview/references/template/references/verification.md#38-測試腳本自己的假設過時)

### 3.10 滿手走單排放料 → 生出取不滿的 team

[讀取此節](../../hpi-motionview/references/template/references/verification.md#310-滿手走單排放料--生出取不滿的-team)

### 3.11 用「剩幾格空」判斷放不放得下

[讀取此節](../../hpi-motionview/references/template/references/verification.md#311-用剩幾格空判斷放不放得下)

### 3.12 把 Fixed（One by one）當成「一次多支、間距鎖定」

[讀取此節](../../hpi-motionview/references/template/references/verification.md#312-把-fixedone-by-one當成一次多支間距鎖定)

### 3.13 參數烤進頁面（架構錯誤）

[讀取此節](../../hpi-motionview/references/template/references/verification.md#313-參數烤進頁面架構錯誤)

### 3.14 `var` 只提升宣告，不提升賦值

[讀取此節](../../hpi-motionview/references/template/references/verification.md#314-var-只提升宣告不提升賦值)

### 3.15 機台 `.xls` 的值全是字串

[讀取此節](../../hpi-motionview/references/template/references/verification.md#315-機台-xls-的值全是字串)

### 3.16 LIVE 不可以用推算值補洞

[讀取此節](../../hpi-motionview/references/template/references/verification.md#316-live-不可以用推算值補洞)

### 3.9 對影片／畫面的顏色誤讀

[讀取此節](../../hpi-motionview/references/template/references/verification.md#39-對影片畫面的顏色誤讀)

## 4. 結構檢查

[讀取此節](../../hpi-motionview/references/template/references/verification.md#4-結構檢查)
