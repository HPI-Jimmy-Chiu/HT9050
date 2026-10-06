# Case：跨臂（Front↔Rear）Z 安全位互鎖用嚴格 `==` 卡死（「下壓後 hangup」，單臂模式必現）

> 內容已分層整理，舊路徑保留供既有引用使用。

[讀取整理後文件](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md)

## 一句話定義

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#一句話定義)

## 1. 涉及的函式與位置

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#1-涉及的函式與位置)

## 2. 第 1 層：`DoInterFaceErrorStep` case 1 的互鎖邏輯

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#2-第-1-層dointerfaceerrorstep-case-1-的互鎖邏輯)

## 3. 第 2 層（唯讀驅動層）：`IndexZCanMove[0]/[1]` 互斥旗標——**已證實健康，不是本案的阻擋者**

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#3-第-2-層唯讀驅動層indexzcanmove01-互斥旗標已證實健康不是本案的阻擋者)

## 4. 第 3 層（唯讀驅動層）：`GalilTwoY_Move` 的 Z 軸互鎖 + 正負號轉換

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#4-第-3-層唯讀驅動層galiltwoy_move-的-z-軸互鎖--正負號轉換)

## 5. 目前是否足以修改程式碼？——**「能不能改」可以；「改成什麼形狀」還沒答案（2026-09-22 再修正）**

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#5-目前是否足以修改程式碼能不能改可以改成什麼形狀還沒答案2026-09-22-再修正)

### 判斷框架（可套用到未來同類「卡在互鎖判斷式」的死結）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#判斷框架可套用到未來同類卡在互鎖判斷式的死結)

### 本案套用結果

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#本案套用結果)

### 建議修法（尚未落地，待授權與版本資料夾）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#建議修法尚未落地待授權與版本資料夾)

#### 參照實作：Ifor 20260903 AMKOR PH 的同型修法（行號為 912.2）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#參照實作ifor-20260903-amkor-ph-的同型修法行號為-9122)

#### 本案要做什麼（取代原建議）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#本案要做什麼取代原建議)

## 6. 與其他「Index 卡住」案例的區別

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#6-與其他index-卡住案例的區別)

## 7. 安全位教點（供查詢，不建議當修法）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#7-安全位教點供查詢不建議當修法)

## 8. 案例

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#8-案例)

### 案例一：2026-09-12 15:31:28（首次擷取，尚無診斷 log）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#案例一2026-09-12-153128首次擷取尚無診斷-log)

### 案例二：2026-09-12 19:02:45（第二次擷取，**已含診斷 log，關鍵修正證據**）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/case-crossarm-z-safe-interlock-deadlock.md#案例二2026-09-12-190245第二次擷取已含診斷-log關鍵修正證據)
