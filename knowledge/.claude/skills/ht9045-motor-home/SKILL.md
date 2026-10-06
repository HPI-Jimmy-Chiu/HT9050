---
name: ht9045-motor-home
description: "HT9045 馬達回 Home / 單軸 Home 機制核心知識庫。Use when: 撰寫或除錯任何單軸 home 程式、客戶反應馬達抖動 / Home 30s timeout / Home 卡住、自動 home 子流程 (如 Check Device 進場前自動 home MLoaderY)、看到 HomeFlag 判斷不一致、混淆 Home() / MotorHome() / MotorInitial() 三個函式用途、Home 後速度殘留導致漂移。關鍵字：HomeFlag, MotorHome, MotorInitial, Home(), HomeReset, HomeObject, iMyHomeTask, ResetTime, RESET_TIMES, auto-home, Home All, Home timeout, 馬達抖動, MLoaderY home, Teach Home, fAllMotorHome, S122, WebTeachLeave, 關 Teach 清旗標, Home by Start, 瀏覽器全關停產, V906 移植樹, ProcessSingleMotorHome。"
---

# ht9045-motor-home 相容入口

同主題已整合到 [hpi-motor-home](../hpi-motor-home/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-motor-home/references/flow/generic/original-entry.md)

## 1. 三個函式用途分清楚

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#1-三個函式用途分清楚)

## 2. HomeFlag 三態語意

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#2-homeflag-三態語意)

## 3. 標準單軸自動 Home 子流程範本

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#3-標準單軸自動-home-子流程範本)

## 4. 反模式（會抖動 / 會 timeout）

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#4-反模式會抖動--會-timeout)

## 5. 與全機 Home 的差異

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#5-與全機-home-的差異)

## 6. 速度狀態還原陷阱（與 Home 直接相關）

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#6-速度狀態還原陷阱與-home-直接相關)

## 7. Teach 畫面為何「都正常」

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#7-teach-畫面為何都正常)

## 8. 診斷 log 必備欄位

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#8-診斷-log-必備欄位)

## 9. 已知案例索引

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#9-已知案例索引)

## 10. 實作位置參考

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#10-實作位置參考)

## 11. HomeClass：整機回 Home 的資料驅動註冊表

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#11-homeclass整機回-home-的資料驅動註冊表)

### 11.1 一筆 = 一顆馬達的 Home 設定

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#111-一筆--一顆馬達的-home-設定)

### 11.2 用 config flag 做機型自動適配

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#112-用-config-flag-做機型自動適配)

### 11.3 分階段排序 = 防撞（核心精神）

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#113-分階段排序--防撞核心精神)

### 11.4 平行陣列不變式（最易踩雷）

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#114-平行陣列不變式最易踩雷)

### 11.5 生命週期

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#115-生命週期)

### 11.6 與本 skill 前段的關係

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#116-與本-skill-前段的關係)

## 12. V906 移植樹（C++）現況

[讀取此節](../hpi-motor-home/references/flow/generic/original-entry.md#12-v906-移植樹c現況)
