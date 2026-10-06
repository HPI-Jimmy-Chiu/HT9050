---
name: ht9045-html-json
description: >
  HT9045 HTML Version（BCB6 GUI 網頁化模擬）開站時所需 JSON 資料規範。定義
  background.html 啟動時要載入哪些 JSON、各檔資料項為何。
  JSON 依來源分四大類：硬體設定檔（Gerneral.ini／IO_Table.csv／Mot_Table.csv／
  teach.ini 等，D:\HT9045\system\）、Config 檔（config.ini 功能開關）、
  Setup 檔／Recipe（D:\HT9045\IniData\Data\<Recipe>\ ＋ SetUp.inf ＋
  cbSetupFileName 清單）、生產記錄檔（Task/System/IO/Motor runtime、程式快照）。
  觸發關鍵字：JSON 啟動載入, 開站資料, background.html 預載, General-config.json,
  Config.json, View-rules.json, Setup-index.json, cbSetupFileName, edSetupFileName,
  labSetupFile, SetUp.inf, GetLastOpenFN, 硬體設定檔, config檔, Setup檔, Recipe,
  生產記錄檔, IO-config.json, Motor-config.json, Teach-config.json, Sim-scale.json,
  Task-runtime.json, System-runtime.json, 載入順序, JSON-only, file:// 墊片,
  HTSettings, HTJsonWriter, decideAll,
  wb_serve, /api/recipe, /api/system, /api/text, /api/system/levelset,
  system.levels.put, levelset.dat, Status.Security, 執行期 tag, tag 串流,
  HT9045Tags, snapshot patch, 接線分級, PAGE_WIRE_STATUS, TAG_WIRE_STATUS,
  C 路, golden 表單橋, editlist.get, editlist.save, CRouteOwner, kOwned, WebCmdGuard, busy:, W906_CMDGUARD_MS, HT9045Busy,
  wbserve-conventions, 同一行插入, 行號錨點, 檔尾附加, 函式指標安裝座, W906_XxxBody, W906_FRW_Install, 測試縫, getenv W906_, W906_LEVELSET_PATH, W906_LOGINDAT_PATH, W906_SOCKETIDLOG_ROOT, W906_PrintDataRedirects, 語法檢查, SIM／SHIP, 警告基準, W906IoClickGuardScope, motor.access action 級, editlist.save 運轉中, SystemStart||SoftStart, BeforeApply, Q14, Q15, CheckIniData, CRouteOwnerDio, 動態 DIO 檔名, Q3, form.event, Q40, gen_editlist --only, _expect, REPLACE, GATE, 冪等, form.event 已做, PageEventsRegistrar, g_evreg, k<P>_Events, RunPageEvent, ELOperable, 清單過期, TA_RadioIndex, TA_EvOnTab, activePageIndex, ELSetPageOrder, 開窗閘, kOpenGates, OpenGateRefused, no-gate, not-authorized, 關窗尾段, MainClickTail.h, CloseTailRunning, saveFlowAfter, S107-1, form.save 運轉中, IC_PasswordGuard, SU_DoPassword, ht9045_yieldmonitoring_c.js, InitialOK, 唯讀輪詢, 免權杖, contactct.get, observer.get, READ_ACTS, WebCmdGuard::Exempt, ui.windows.put, WebWindowRegistry, FShowConservative, 瀏覽器全關停產, W906_SocketIDLogBody, golden_root, W906_GOLDEN_ROOT, 產生器根目錄, golden 0618 切換, golden_methods, KEEP_V912, dfm_keep, E-031 第 1 批, Q81＝A 保留表, _hand_kept.py, keep-list, 手寫 bridge 留存表, E031_FormBridgeFullRun, formbridge_fullrun_check, 過期產生檔
applyTo: "**/*"
---

# ht9045-html-json 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/json/original-entry.md)

## ⛔ 先讀這一節：有三條路（20260916 裁決 A／B，20260924 新增 C，同日精簡）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#-先讀這一節有三條路20260916-裁決-ab20260924-新增-c同日精簡)

## 四大類 JSON

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#四大類-json)

### 1. 硬體設定檔 — 來源 `D:\HT9045\system\`

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#1-硬體設定檔--來源-dht9045system)

### 2. Config 檔 — 來源 `D:\HT9045\config\config.ini`

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#2-config-檔--來源-dht9045configconfigini)

### 3. Setup 檔（Recipe）— 來源 `D:\HT9045\IniData\Data\<RecipeName>\` ＋ `D:\HT9045\SetUp.inf`

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#3-setup-檔recipe-來源-dht9045inidatadata--dht9045setupinf)

### 4. 生產記錄檔 — 機台運轉期間產生／即時狀態

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#4-生產記錄檔--機台運轉期間產生即時狀態)

### 5. 其他讀寫檔（2026-09-09 補完轉換，人工稽核清單見 `page/ScreenShots.html` 表③）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#5-其他讀寫檔2026-09-09-補完轉換人工稽核清單見-pagescreenshotshtml-表③)

### 更新觸發與頻率分類

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#更新觸發與頻率分類)

### ⛳ B 路的正式契約另有專文（20260916 新增）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#-b-路的正式契約另有專文20260916-新增)

### 值域（min／max）與同五個分類在 B 路的對應——B 路細節

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#值域minmax與同五個分類在-b-路的對應b-路細節)

## ⛔ 新的 tag 串流／頁面輪詢：一律照筆電的規則（RULINGS_20260930 第 12 條，20260930 起）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#-新的-tag-串流頁面輪詢一律照筆電的規則rulings_20260930-第-12-條20260930-起)

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#apisystem-與-apitext--機台檔案的即時讀寫)

## ⛔ 「四大類」只存在於本文件，程式裡沒有（20261002 實測）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#-四大類只存在於本文件程式裡沒有20261002-實測)

## 待辦（本 skill 的落地缺口）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#待辦本-skill-的落地缺口)

## 與其他 Skill 的關係

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#與其他-skill-的關係)

## 產生器（沿用 `ht9045-html-version` 的工作副本）

[讀取此節](../hpi-web-hmi/references/json/original-entry.md#產生器沿用-ht9045-html-version-的工作副本)
