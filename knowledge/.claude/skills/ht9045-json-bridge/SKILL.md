---
name: ht9045-json-bridge
description: >
  HT9045 V906 移植樹的 JSON ↔ C++ 結構橋接層（設定／生產／mot-io／alarm／動作／event log 六條通道＋開機配置廣播）
  的分工、API 形狀、通訊縮減規則與驗收 gate。使用者 20260923 定案：C++ 讀檔由
  JerryYang 翻譯；讀完後 struct→JSON、HTML Save 後 JSON→struct→存檔→再讀檔、
  生產數值回傳、mot/io 串流、alarm 雙向，全部由 Steven 提供 C++ function。
  型別表由 golden 的 ReadIniData/WriteIniData 產生（offsetof 由 C++ 出，Python
  不猜），實例綁定手寫，頁面投影沿用 pagewire 三元組。Use when：設計或實作
  /api/struct、struct.put、FieldDesc、Binding、TestIF_File／DeviceForm_File／
  LevelSet 的 JSON 化、IO 位元打包、alarm 事件、判斷「畫面顯示的值是不是機台在用的值」。
  關鍵字：JSON 中介層, struct→JSON, JSON→struct, /api/struct, struct.put, FieldDesc,
  Binding, offsetof, TestIF_File, DeviceForm_File, LevelSet, SYSTEM_TEST_IF,
  SYSTEM_DEVICE_FORM, LAST_LEVEL_SET, LAST_GENERAL_SET, ReadIniData, WriteIniData,
  HTEditList, SaveAllFile, DoIniDataToForm, SaveSetupFile, vclcompat, dryRun,
  snapshot patch, io.di, io.do, 位元打包, Dialog-bridge-contract, modal.answer,
  dialog.response, act.*, 動作通道, btnClearCountClick, Clarn_Data, counter.clear,
  ReadWriteIni, ReadLastSetIni, SaveLastSetIni, IniConfig, config.ini, elConfig,
  Config.Configuration.html, Config.DIOInterFaceCFG.html, cUnitConvert, DoStructUnitConvert,
  RecordProcess, NewRecordProcess, RecordChangeLogProcess, MyDBIProcess, log.event, log.tail, event log,
  machine.defines, cfg.resync, cfg.ver, 開機配置, SOFT_SIMULTE, MachineType.h,
  通訊縮減, 表⑧, FILEIO_BRIDGE_STATUS, DoIniDataToForm, /api/form, S12, 頁面改讀 C++,
  存檔後重讀, Chiller Temp, ATC.ini, iATC_MODE_TYPE, golden 表單橋, C 路, A 形狀, C 形狀,
  gen_formbridge.py, gen_editlist.py, tools/formbridge, FileRW, --only, _hand_kept.py, E031_FormBridgeFullRun, editlist.get,
  editlist.save, PageDesc, PageRegistrar, _EditPage.h, EL<T>, 具名替身, Ld_UldDelayTime,
  W906_SecurityBoot, iMaxLevelItem, GATE (SEC1), iDecimalPoint, G1 驗收, s12c_page_probe.py,
  s12_form_probe.py, _integrated.txt, GOLDEN_BRIDGE, CRouteOwner, kOwned, W906_DoReadLastData,
  KeepNewerOverlap, 多寫者, MainTempOffsetTail, 關窗尾段, blocks not hit, _find, ATKRecipeInfo,
  HT9045_TESTERCOMM, HT9045_ref 退場, ACTForm, Winway, Monitor, CfgTrayPlate, MainBoot, MainBackup, MainClose,
  MainRecord, W906_BackupSetupFileBody, W906_SaveRunModeBody, 函式指標安裝座, WebCmdGuard, busy:, W906_CMDGUARD_MS,
  同一行插入, [W906] 偏離
---

# ht9045-json-bridge 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/bridge/original-entry.md)

## 〇、現況速查（20260923 21:30 skill 維護盤點加。**先讀這張表，再讀下面任何一節**）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/01.md#〇現況速查20260923-2130-skill-維護盤點加先讀這張表再讀下面任何一節)

## 〇之二、新增一個表單的讀寫，先看這裡（20260924 深夜；20260926 Steven 團隊依現況改寫，舊版在 `references/archive/SKILL_superseded.md`）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/02.md#〇之二新增一個表單的讀寫先看這裡20260924-深夜20260926-steven-團隊依現況改寫舊版在-referencesarchiveskill_supersededmd)

## 一、分工（使用者 20260923 定案）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#一分工使用者-20260923-定案)

### 設定相關（配方／系統檔）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#設定相關配方系統檔)

### 生產相關

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#生產相關)

### mot／io 相關

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#motio-相關)

### 動作相關（第五條通道，使用者 20260923 補：「透過畫面去清除數據的操作有加入嗎？」—— 原本沒有）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#動作相關第五條通道使用者-20260923-補透過畫面去清除數據的操作有加入嗎-原本沒有)

### event log 相關（第六條通道，使用者 20260923 補：「html 的操作會需要有 event log，也需要設計一個 JSON」）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#event-log-相關第六條通道使用者-20260923-補html-的操作會需要有-event-log也需要設計一個-json)

### alarm 相關

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/03.md#alarm-相關)

## 二、裁決紀錄（20260923，使用者）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/04.md#二裁決紀錄20260923使用者)

## 三、量到的事實（改設計前先讀）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/05.md#三量到的事實改設計前先讀)

## 四、API 形狀

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/06.md#四api-形狀)

## 五、寫方向（#3 → #6）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/07.md#五寫方向3--6)

## 六、通訊縮減（我們有 JSON 結構設計權）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/08.md#六通訊縮減我們有-json-結構設計權)

## 七、驗收 gate（每個結構、每條通道一致）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/09.md#七驗收-gate每個結構每條通道一致)

## 八、分期

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/10.md#八分期)

## 九、陷阱（今天踩過或量到的）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/11.md#九陷阱今天踩過或量到的)

## 十、待裁決

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/12.md#十待裁決)

## 十二、20260923 審查更正（審查員獨立查證 12 條宣稱的結果）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/13.md#十二20260923-審查更正審查員獨立查證-12-條宣稱的結果)

## 十一、相關檔案與 skill

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/14.md#十一相關檔案與-skill)

### 本 skill 的 references（共 ~~**6**~~ → ~~13~~ → **16** 份（20260926 實數：加 `pending-pages.md` 等；另有 `archive/` 放移出的舊內容）；20260923 skill 維護盤點更正——原本寫「先讀這兩份」，當時就已經有 5 份；同日 21:50 續拆再加 8 份、併掉 `producers-and-layout.md` 1 份）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/14.md#本-skill-的-references共-6--13--16-份20260926-實數加-pending-pagesmd-等另有-archive-放移出的舊內容20260923-skill-維護盤點更正原本寫先讀這兩份當時就已經有-5-份同日-2150-續拆再加-8-份併掉-producers-and-layoutmd-1-份)

## 十三、S8～S11 留下的待辦（高級審查員第二輪，20260923 21:03）

[讀取此節](../hpi-web-hmi/references/bridge/original-entry/15.md#十三s8s11-留下的待辦高級審查員第二輪20260923-2103)
