# HT9045 自動測高（Auto Height）／接觸測試（Contact Test）動作流程（golden 906 0618）

> 內容已分層整理，舊路徑保留供既有引用使用。

[讀取整理後文件](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045.md)

## 0. Steven 的 7 步（本文的骨架）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/01.md#0-steven-的-7-步本文的骨架)

## 1. 入口：按鈕 → START → DoTestContactFunction case 1

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/02.md#1-入口按鈕--start--dotestcontactfunction-case-1)

## 2. 步 1：In／Out Arm 讓開

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/03.md#2-步-1inout-arm-讓開)

## 3. 步 2：浮料（置偏）檢查 → 2D ID → 往右進 Index

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/04.md#3-步-2浮料置偏檢查--2d-id--往右進-index)

### 3.1 浮料檢查：DoTestContactFunction case 219／230

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/04.md#31-浮料檢查dotestcontactfunction-case-219230)

### 3.2 選臂：DoTestContactFunction case 290

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/04.md#32-選臂dotestcontactfunction-case-290)

### 3.3 2D ID 與往右：`DoZ1PickFromShuttle`（Z2 版 `DoZ2PickFromShuttle` 對稱）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/04.md#33-2d-id-與往右doz1pickfromshuttlez2-版-doz2pickfromshuttle-對稱)

## 4. 步 3：Index 從 In Shuttle 吸料

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/05.md#4-步-3index-從-in-shuttle-吸料)

## 5. 步 4：Index 1 到 socket 量高度——`Do_Z1_AutoGetHeight`（DoTestContactFunction case 400）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#5-步-4index-1-到-socket-量高度do_z1_autogetheightdotestcontactfunction-case-400)

### 5.1 Task 1 → 去 socket 上方（各模式共用）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#51-task-1--去-socket-上方各模式共用)

### 5.2 自動測高（模式 1）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#52-自動測高模式-1)

### 5.3 Contact Test（模式 3）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#53-contact-test模式-3)

### 5.4 手動測高（模式 2）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#54-手動測高模式-2)

### 5.5 Load Cell（模式 8）：`Do_LoadCellAutoHigh(iIndex)`（DoTestContactFunction case 2160 → 2170）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/06.md#55-load-cell模式-8do_loadcellautohighiindexdotestcontactfunction-case-2160--2170)

## 6. 步 5：Index 2 到 socket 量高度——`Do_Z2_AutoGetHeight`（DoTestContactFunction case 799 → 800）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/07.md#6-步-5index-2-到-socket-量高度do_z2_autogetheightdotestcontactfunction-case-799--800)

## 7. 步 6：Index 把 IC 放回 In Shuttle——`DoZPlaceToShuttle`（DoTestContactFunction case 900）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/08.md#7-步-6index-把-ic-放回-in-shuttledozplacetoshuttledotestcontactfunction-case-900)

## 8. 步 7：In Shuttle 回左邊——DoTestContactFunction case 1700 → 1800

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/09.md#8-步-7in-shuttle-回左邊dotestcontactfunction-case-1700--1800)

## 9. 存了什麼

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/10.md#9-存了什麼)

## 10. 扭力：怎麼設、怎麼讀（國際牌，COM2）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/11.md#10-扭力怎麼設怎麼讀國際牌com2)

## 11. 等待與逾時

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/12.md#11-等待與逾時)

## 12. V912 跟 golden 906 0618 的差別（只列跟本流程有關的）

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/13.md#12-v912-跟-golden-906-0618-的差別只列跟本流程有關的)

## 13. 三句話

[讀取此節](../../hpi-index-flow/references/machines/ht9045/autoheight-contact-test-ht9045/14.md#13-三句話)
