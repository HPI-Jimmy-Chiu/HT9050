# HT9045 Web Bridge — HTML 端的 JSON 契約

按需要選取以下章節，原文依順序保留。

- [HT9045 Web Bridge — HTML 端的 JSON 契約](web-bridge-json-contract/00.md)
- [這份文件是什麼](web-bridge-json-contract/01.md)
- [這份文件不是什麼](web-bridge-json-contract/02.md)
- [一個貫穿全文的原則](web-bridge-json-contract/03.md)
- [1.1 傳輸](web-bridge-json-contract/04.md)
- [1.2 伺服器 → 瀏覽器的訊框](web-bridge-json-contract/05.md)
- [1.3 瀏覽器 → 伺服器的指令](web-bridge-json-contract/06.md)
- [1.4 HTTP 讀取端點](web-bridge-json-contract/07.md)
- [2.1 訊框層的 metadata，不要掛在每個 tag 上](web-bridge-json-contract/08.md)
- [2.2 `seq` — 串流序號](web-bridge-json-contract/09.md)
- [2.3 `snapshot` 要能主動重送](web-bridge-json-contract/10.md)
- [2.4 `at` 要帶時區位移](web-bridge-json-contract/11.md)
- [2.5 `trigger` — 這一幀為什麼發](web-bridge-json-contract/12.md)
- [2.6 `available`：只在要分辨兩種「沒有值」時才需要](web-bridge-json-contract/13.md)
- [2.7 tag 命名規則（給新增 tag 的人）](web-bridge-json-contract/14.md)
- [2.9 值域（min／max）—— 兩邊都要有，不是二選一](web-bridge-json-contract/15.md)
- [2.8 值的型別](web-bridge-json-contract/16.md)
- [3.1 `updateClass` 對照表](web-bridge-json-contract/17.md)
- [3.2 tag → 畫面元素的對照](web-bridge-json-contract/18.md)
- [3.3 stale 與斷線的顯示](web-bridge-json-contract/19.md)
- [4.1 `stream.resync` 的回應方式 → **建議：ack 之後另送一幀 snapshot，只送給提出要求的那條連線**](web-bridge-json-contract/20.md)
- [4.2 `production` 事件要不要重建 delta ＋ checkpoint → **建議：不要**](web-bridge-json-contract/21.md)
- [4.3 `absent` 清單要不要做 → **建議：不要放進串流，HTML 自己從 `/api/system/gerneral` 算**](web-bridge-json-contract/22.md)
- [4.4 值域 → 見 §2.9（使用者已裁決：兩邊都要有）](web-bridge-json-contract/23.md)

# HT9045 Web Bridge — HTML 端的 JSON 契約

[讀取此節](web-bridge-json-contract/00.md#ht9045-web-bridge--html-端的-json-契約)

## 這份文件是什麼

[讀取此節](web-bridge-json-contract/01.md#這份文件是什麼)

## 這份文件不是什麼

[讀取此節](web-bridge-json-contract/02.md#這份文件不是什麼)

## 一個貫穿全文的原則

[讀取此節](web-bridge-json-contract/03.md#一個貫穿全文的原則)

# 第一部分：現況（已在服務中，不要重做）

[讀取此節](web-bridge-json-contract/03.md#第一部分現況已在服務中不要重做)

## 1.1 傳輸

[讀取此節](web-bridge-json-contract/04.md#11-傳輸)

## 1.2 伺服器 → 瀏覽器的訊框

[讀取此節](web-bridge-json-contract/05.md#12-伺服器--瀏覽器的訊框)

## 1.3 瀏覽器 → 伺服器的指令

[讀取此節](web-bridge-json-contract/06.md#13-瀏覽器--伺服器的指令)

## 1.4 HTTP 讀取端點

[讀取此節](web-bridge-json-contract/07.md#14-http-讀取端點)

# 第二部分：HTML 端的要求（目前沒有）

[讀取此節](web-bridge-json-contract/07.md#第二部分html-端的要求目前沒有)

## 2.1 訊框層的 metadata，不要掛在每個 tag 上

[讀取此節](web-bridge-json-contract/08.md#21-訊框層的-metadata不要掛在每個-tag-上)

## 2.2 `seq` — 串流序號

[讀取此節](web-bridge-json-contract/09.md#22-seq--串流序號)

## 2.3 `snapshot` 要能主動重送

[讀取此節](web-bridge-json-contract/10.md#23-snapshot-要能主動重送)

## 2.4 `at` 要帶時區位移

[讀取此節](web-bridge-json-contract/11.md#24-at-要帶時區位移)

## 2.5 `trigger` — 這一幀為什麼發

[讀取此節](web-bridge-json-contract/12.md#25-trigger--這一幀為什麼發)

## 2.6 `available`：只在要分辨兩種「沒有值」時才需要

[讀取此節](web-bridge-json-contract/13.md#26-available只在要分辨兩種沒有值時才需要)

## 2.7 tag 命名規則（給新增 tag 的人）

[讀取此節](web-bridge-json-contract/14.md#27-tag-命名規則給新增-tag-的人)

## 2.9 值域（min／max）—— 兩邊都要有，不是二選一

[讀取此節](web-bridge-json-contract/15.md#29-值域minmax-兩邊都要有不是二選一)

### 現況：一半已經有了，另一半完全沒有

[讀取此節](web-bridge-json-contract/15.md#現況一半已經有了另一半完全沒有)

### 為什麼瀏覽器端擋了還要伺服器端再擋

[讀取此節](web-bridge-json-contract/15.md#為什麼瀏覽器端擋了還要伺服器端再擋)

### 要求

[讀取此節](web-bridge-json-contract/15.md#要求)

### 變動門檻（deadband）—— 暫時不做

[讀取此節](web-bridge-json-contract/15.md#變動門檻deadband-暫時不做)

## 2.8 值的型別

[讀取此節](web-bridge-json-contract/16.md#28-值的型別)

# 第三部分：HTML 端自己負責的（producer 不用管）

[讀取此節](web-bridge-json-contract/16.md#第三部分html-端自己負責的producer-不用管)

## 3.1 `updateClass` 對照表

[讀取此節](web-bridge-json-contract/17.md#31-updateclass-對照表)

## 3.2 tag → 畫面元素的對照

[讀取此節](web-bridge-json-contract/18.md#32-tag--畫面元素的對照)

## 3.3 stale 與斷線的顯示

[讀取此節](web-bridge-json-contract/19.md#33-stale-與斷線的顯示)

# 第四部分：三個設計問題的建議（待使用者確認）

[讀取此節](web-bridge-json-contract/19.md#第四部分三個設計問題的建議待使用者確認)

## 4.1 `stream.resync` 的回應方式 → **建議：ack 之後另送一幀 snapshot，只送給提出要求的那條連線**

[讀取此節](web-bridge-json-contract/20.md#41-streamresync-的回應方式--建議ack-之後另送一幀-snapshot只送給提出要求的那條連線)

## 4.2 `production` 事件要不要重建 delta ＋ checkpoint → **建議：不要**

[讀取此節](web-bridge-json-contract/21.md#42-production-事件要不要重建-delta--checkpoint--建議不要)

### ⚠ 但有一個真的缺口，請一起評估

[讀取此節](web-bridge-json-contract/21.md#-但有一個真的缺口請一起評估)

## 4.3 `absent` 清單要不要做 → **建議：不要放進串流，HTML 自己從 `/api/system/gerneral` 算**

[讀取此節](web-bridge-json-contract/22.md#43-absent-清單要不要做--建議不要放進串流html-自己從-apisystemgerneral-算)

## 4.4 值域 → 見 §2.9（使用者已裁決：兩邊都要有）

[讀取此節](web-bridge-json-contract/23.md#44-值域--見-29使用者已裁決兩邊都要有)

# 附錄：現況數字（2026-09-16 實測）

[讀取此節](web-bridge-json-contract/23.md#附錄現況數字2026-09-16-實測)
