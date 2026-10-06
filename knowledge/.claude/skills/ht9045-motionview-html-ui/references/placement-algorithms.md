# 取放料演算法（對照程式碼）

舊引用路徑保留；[讀取整理後文件](../../hpi-motionview/references/template/references/placement-algorithms.md)。

## 目錄

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#目錄)

## 1. Tray 取料

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#1-tray-取料)

### 1.1 鐵則：吸滿才走

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#11-鐵則吸滿才走)

### 1.2 位置計算

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#12-位置計算)

### 1.3 間距倍數的選法

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#13-間距倍數的選法)

### 1.4 ⚠ Fixed 模式 ＝ One by one，一次只吸一顆

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#14--fixed-模式--one-by-one一次只吸一顆)

### 1.4 掃描順序：列優先

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#14-掃描順序列優先)

## 2. HotPlate 放料（滾動游標）

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#2-hotplate-放料滾動游標)

### 2.1 ⚠ 滿手時**不要**走 `Row=1`（實測缺陷）

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#21--滿手時不要走-row1實測缺陷)

### 2.2 水位：用「探位」不要用「格數」

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#22-水位用探位不要用格數)

## 3. HotPlate 取料

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#3-hotplate-取料)

### 3.1 FIFO

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#31-fifo)

### 3.2 Site ↔ 吸嘴 必須對得上

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#32-site--吸嘴-必須對得上)

### 3.3 整組同一條 Shuttle

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#33-整組同一條-shuttle)

### 3.4 湊不成整組時

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#34-湊不成整組時)

## 4. Kit / Socket / 出料

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#4-kit--socket--出料)

## 5. Shuttle 三站雙 Kit

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#5-shuttle-三站雙-kit)

## 6. 幾何上的硬限制

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#6-幾何上的硬限制)

### 6.1 列對配對數上限

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#61-列對配對數上限)

### 6.2 Tray 欄數與吸嘴數不合時的尾欄成本

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#62-tray-欄數與吸嘴數不合時的尾欄成本)

### 6.3 Y 變距才是上料節拍的槓桿

[讀取此節](../../hpi-motionview/references/template/references/placement-algorithms.md#63-y-變距才是上料節拍的槓桿)
