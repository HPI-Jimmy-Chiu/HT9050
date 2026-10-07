# 已知死鎖模式（案例庫）

按需要選取以下章節，原文依順序保留。

- [已知死鎖模式（案例庫）](deadlock-patterns/00.md)
- [快速比對指南](deadlock-patterns/01.md)
- [⚠️ 分析誤區與反模式（必讀！）](deadlock-patterns/02.md)
- [Pattern #1：PAUSE-during-test + JAM0302 + D42 四方死鎖](deadlock-patterns/03.md)
- [Pattern #2：OutArm 取料子任務完成但資料未轉移（Shuttle 右側空轉）](deadlock-patterns/04.md)
- [Pattern #3：GetNowShuttleMode 判斷錯誤導致 OutArm 取料位置錯誤（2x8 模式）](deadlock-patterns/05.md)
- [Pattern #N：{簡短描述}](deadlock-patterns/06.md)

# 已知死鎖模式（案例庫）

[讀取此節](deadlock-patterns/00.md#已知死鎖模式案例庫)

## 快速比對指南

[讀取此節](deadlock-patterns/01.md#快速比對指南)

## ⚠️ 分析誤區與反模式（必讀！）

[讀取此節](deadlock-patterns/02.md#-分析誤區與反模式必讀)

### 核心原則

[讀取此節](deadlock-patterns/02.md#核心原則)

### Shuttle case 10 誤判案例（20260417 教訓）

[讀取此節](deadlock-patterns/02.md#shuttle-case-10-誤判案例20260417-教訓)

### 死鎖分析檢查清單（避免誤判）

[讀取此節](deadlock-patterns/02.md#死鎖分析檢查清單避免誤判)

## Pattern #1：PAUSE-during-test + JAM0302 + D42 四方死鎖

[讀取此節](deadlock-patterns/03.md#pattern-1pause-during-test--jam0302--d42-四方死鎖)

### 識別特徵

[讀取此節](deadlock-patterns/03.md#識別特徵)

### 時間線模式

[讀取此節](deadlock-patterns/03.md#時間線模式)

### 根因

[讀取此節](deadlock-patterns/03.md#根因)

### 偶發條件

[讀取此節](deadlock-patterns/03.md#偶發條件)

### 修正方向

[讀取此節](deadlock-patterns/03.md#修正方向)

### 案例

[讀取此節](deadlock-patterns/03.md#案例)

### 相關程式碼

[讀取此節](deadlock-patterns/03.md#相關程式碼)

## Pattern #2：OutArm 取料子任務完成但資料未轉移（Shuttle 右側空轉）

[讀取此節](deadlock-patterns/04.md#pattern-2outarm-取料子任務完成但資料未轉移shuttle-右側空轉)

### 識別特徵

[讀取此節](deadlock-patterns/04.md#識別特徵)

### 時間線模式

[讀取此節](deadlock-patterns/04.md#時間線模式)

### 根因

[讀取此節](deadlock-patterns/04.md#根因)

### 偶發條件

[讀取此節](deadlock-patterns/04.md#偶發條件)

### 修正方向

[讀取此節](deadlock-patterns/04.md#修正方向)

### 案例

[讀取此節](deadlock-patterns/04.md#案例)

## Pattern #3：GetNowShuttleMode 判斷錯誤導致 OutArm 取料位置錯誤（2x8 模式）

[讀取此節](deadlock-patterns/05.md#pattern-3getnowshuttlemode-判斷錯誤導致-outarm-取料位置錯誤2x8-模式)

### 識別特徵

[讀取此節](deadlock-patterns/05.md#識別特徵)

### 時間線模式

[讀取此節](deadlock-patterns/05.md#時間線模式)

### 根因

[讀取此節](deadlock-patterns/05.md#根因)

### 偶發條件

[讀取此節](deadlock-patterns/05.md#偶發條件)

### 修正方向

[讀取此節](deadlock-patterns/05.md#修正方向)

### 案例

[讀取此節](deadlock-patterns/05.md#案例)

### 相關程式碼

[讀取此節](deadlock-patterns/05.md#相關程式碼)

### 關鍵學習：如何從 StateRecord 推論此類問題

[讀取此節](deadlock-patterns/05.md#關鍵學習如何從-staterecord-推論此類問題)

## Pattern #N：{簡短描述}

[讀取此節](deadlock-patterns/06.md#pattern-n簡短描述)

### 識別特徵

[讀取此節](deadlock-patterns/06.md#識別特徵)

### 時間線模式

[讀取此節](deadlock-patterns/06.md#時間線模式)

### 根因

[讀取此節](deadlock-patterns/06.md#根因)

### 偶發條件

[讀取此節](deadlock-patterns/06.md#偶發條件)

### 修正方向

[讀取此節](deadlock-patterns/06.md#修正方向)

### 案例

[讀取此節](deadlock-patterns/06.md#案例)

### 相關程式碼

[讀取此節](deadlock-patterns/06.md#相關程式碼)
