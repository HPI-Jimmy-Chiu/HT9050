---
name: ht9045-page-table-fshow
description: >
  HT9045 V906 移植樹的「頁面表」（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp／.h）：取代 golden BCB6 每個表單自己的
  fShow／bShow「畫面開著沒」旗標。網頁版的畫面在瀏覽器裡，C++ 表單成員沒人設成 true，直接讀永遠是 false；
  Steven 20260928 Q51（第一優先）定案：C++ 用一張 90 列的表記所有畫面開／關、跟 background.html 互通，所有讀取一律問
  單一函式 W906_FormShowing("fXxx", fXxx->fShow)。本 skill 講：表的欄位與各種類列數（kPgWeb／kPgBoth／kPgProgram／kPgNoWeb）、
  回答規則 1～7、沒有畫面不准 START、瀏覽器全關寬限 10 秒後正常 STOP、jog／LoopMove 死人開關、告警 MES16441、過期門檻 15 秒、
  background.html 送 page registry（pushRegistry、5 秒心跳、pagehide 全部關）、ui.pages 回傳、FShow_Audit 棘輪
  （fshow_audit.py＋fshow_audit_baseline.json）、頁面表加一列／把表單從「沒有網頁」段搬到網頁段的 checklist（test_pagetable 計數、
  WINDOWS 表同一顆 commit）、誰擁有哪些檔、Q51／Q-P1／Q-P2／Q-P3／R122／R143／R144 原話。
  關鍵字：fShow, bShow, W906_FormShowing, W906_FormProgramShow, WebPageTable, 頁面表, 頁面狀態表, 畫面開著沒, FShow_Audit,
  fshow_audit_baseline, p6b_fshow_audit, kPgWeb, kPgProgram, kPgBoth, kPgNoWeb, kWebRowCount, 寬限 10 秒, kPageNoScreenGraceMs,
  MES16441, page registry, pushRegistry, 心跳, pagehide, bye, ui.pages, ui.windows.put, PageStartAllowed, Q51, Q-P1, Q-P2,
  頁面表加一列, test_pagetable, WINDOWS 表, kNeverReportedForms。
---

# ht9045-page-table-fshow 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/windows/original-entry.md)

## 1. 白話

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#1-白話)

## 2. 檔案在哪

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#2-檔案在哪)

## 3. 表的結構

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#3-表的結構)

## 4. 規則與行為

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#4-規則與行為)

## 5. 要改表時怎麼做（checklist）

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#5-要改表時怎麼做checklist)

### 5.1 加一列／把表單從「沒有網頁」段搬到網頁段

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#51-加一列把表單從沒有網頁段搬到網頁段)

### 5.2 新翻 golden 碼（或解開 `#if 0` 閘）讀到 fShow

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#52-新翻-golden-碼或解開-if-0-閘讀到-fshow)

### 5.3 誰的檔

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#53-誰的檔)

## 6. 相關裁決（原話＋出處）

[讀取此節](../hpi-web-hmi/references/windows/original-entry.md#6-相關裁決原話出處)
