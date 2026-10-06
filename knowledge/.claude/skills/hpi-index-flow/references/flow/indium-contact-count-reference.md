# 銦片計數（Indium LifeTime Count）完整 Reference

> 原始文件拆成 11 個順序片段；依下面導覽讀取需要的段落。機型與版本來源見 [共同與差異](../common.md)。

- [銦片計數（Indium LifeTime Count）完整 Reference](indium-contact-count-reference/00.md)
- [1. 旗標啟用條件](indium-contact-count-reference/01.md)
- [2. 相關變數](indium-contact-count-reference/02.md)
- [2.5 實體架構與索引語意（權威定義）](indium-contact-count-reference/03.md)
- [3. SPEC 上限的兩條存取路徑](indium-contact-count-reference/04.md)
- [4. editContactCountAlarm UI 權限](indium-contact-count-reference/05.md)
- [5. 跑料中計數累加與告警](indium-contact-count-reference/06.md)
- [6. Lot Start 互動（VTEST）](indium-contact-count-reference/07.md)
- [7. 客戶別差異總表](indium-contact-count-reference/08.md)
- [8. 修改歷史（2026/05 起）](indium-contact-count-reference/09.md)
- [9. 已知風險與防護](indium-contact-count-reference/10.md)

# 銦片計數（Indium LifeTime Count）完整 Reference

[讀取此節](indium-contact-count-reference/00.md#銦片計數indium-lifetime-count完整-reference)

## 1. 旗標啟用條件

[讀取此節](indium-contact-count-reference/01.md#1-旗標啟用條件)

## 2. 相關變數

[讀取此節](indium-contact-count-reference/02.md#2-相關變數)

## 2.5 實體架構與索引語意（權威定義）

[讀取此節](indium-contact-count-reference/03.md#25-實體架構與索引語意權威定義)

### 2.5.1 機構配置

[讀取此節](indium-contact-count-reference/03.md#251-機構配置)

### 2.5.2 索引語意 —— dim2 是 Arm，不是 row

[讀取此節](indium-contact-count-reference/03.md#252-索引語意--dim2-是-arm不是-row)

### 2.5.3 三端一致性檢查（2026/07/28 稽核結果）

[讀取此節](indium-contact-count-reference/03.md#253-三端一致性檢查20260728-稽核結果)

### 2.5.4 UI 容量上限

[讀取此節](indium-contact-count-reference/03.md#254-ui-容量上限)

### 2.5.5 全客戶對齊到 by-arm 模型的評估

[讀取此節](indium-contact-count-reference/03.md#255-全客戶對齊到-by-arm-模型的評估)

## 3. SPEC 上限的兩條存取路徑

[讀取此節](indium-contact-count-reference/04.md#3-spec-上限的兩條存取路徑)

### 3.1 路徑 A — ProcessLastSetIni_Count（多值，每 Head 獨立）

[讀取此節](indium-contact-count-reference/04.md#31-路徑-a--processlastsetini_count多值每-head-獨立)

### 3.2 路徑 B — ReadWriteStartCondition（單值，UI 設定用）

[讀取此節](indium-contact-count-reference/04.md#32-路徑-b--readwritestartcondition單值ui-設定用)

#### 銦片 SPEC（用 szDirIndium）

[讀取此節](indium-contact-count-reference/04.md#銦片-spec用-szdirindium)

#### 非銦片資料（用 szDir，跟 D05）

[讀取此節](indium-contact-count-reference/04.md#非銦片資料用-szdir跟-d05)

### 3.3 D05 與銦片路徑的關係（重要）

[讀取此節](indium-contact-count-reference/04.md#33-d05-與銦片路徑的關係重要)

## 4. editContactCountAlarm UI 權限

[讀取此節](indium-contact-count-reference/05.md#4-editcontactcountalarm-ui-權限)

## 5. 跑料中計數累加與告警

[讀取此節](indium-contact-count-reference/06.md#5-跑料中計數累加與告警)

### 告警代碼

[讀取此節](indium-contact-count-reference/06.md#告警代碼)

## 6. Lot Start 互動（VTEST）

[讀取此節](indium-contact-count-reference/07.md#6-lot-start-互動vtest)

## 7. 客戶別差異總表

[讀取此節](indium-contact-count-reference/08.md#7-客戶別差異總表)

## 8. 修改歷史（2026/05 起）

[讀取此節](indium-contact-count-reference/09.md#8-修改歷史202605-起)

## 9. 已知風險與防護

[讀取此節](indium-contact-count-reference/10.md#9-已知風險與防護)

### 9.1 D05 與 VTEST 銦片路徑分裂（已修復 05/17）

[讀取此節](indium-contact-count-reference/10.md#91-d05-與-vtest-銦片路徑分裂已修復-0517)

### 9.2 兩套索引模型並存（**未修復**，2026/07/28 確認）

[讀取此節](indium-contact-count-reference/10.md#92-兩套索引模型並存未修復20260728-確認)

### 9.3 `bUseHeadContactCount` 會翻轉 Contact alarm 的設定語意（陷阱）

[讀取此節](indium-contact-count-reference/10.md#93-buseheadcontactcount-會翻轉-contact-alarm-的設定語意陷阱)

### 9.4 grid 空白列會被回讀成 0（保留槽遭清零）

[讀取此節](indium-contact-count-reference/10.md#94-grid-空白列會被回讀成-0保留槽遭清零)
