# 已知問題與解決方案 (Known Issues & Workarounds)

> 適用：HT9045 SECS/GEM 模組  
> 建立日期：2026-04-16

---

## 問題索引

| # | 問題 | 影響版本 | 狀態 | 嚴重度 |
|:-:|------|---------|:----:|:-----:|
| 1 | SVID 1191 衝突 | ≤V3.33.900.0 | ✅ 已修 | 🔴 高 |
| 2 | S6F11 先於 S2F42 發送 | 全版本 | ℹ️ 設計行為 | 🟡 中 |
| 3 | Recipe 刪除時使用中判斷 | 全版本 | ℹ️ 設計行為 | 🟢 低 |
| 4 | S2F34 DRACK=0x02 （EAP VID mapping 不符） | 全版本 | ℹ️ 客戶端設定 | 🔴 高 |

---

## Issue #1: SVID 1191 / ECID 1191 衝突

### 描述
SVID 1191 和 ECID 1191 使用了相同的 ID 值。SEMI E30 要求 SVID 和 ECID 的 ID 空間不重複。

### 影響
- Host 查詢 ID 1191 時收到錯誤的值
- 部分 MES 系統拒絕連線

### 修復
- **版本**：V3.33.900.0_20260331
- **修改**：SVID 1191 改為 **SVID 1192**
- **ECID 1191** `USE_RFID_READER` 保留不動
- **同時更新** Excel `SECS_20260401_Steven.xlsx`

### 驗證方式
```python
# 使用 SECS-Dev-Procedures.md §4 的 Python 稽核腳本
python check_id_conflict.py --sv-file uHGemHT9045_SV.cpp --ec-file uHGemHT9045_EC.cpp
```

---

## Issue #2: S6F11 先於 S2F42 發送

### 描述
當 Handler 收到 S2F41 RCMD 時，會**先發 S6F11 Event Report**，**再回 S2F42 Ack**。

### 原因
這是 HT9045 的設計行為，不是 BUG。原因：
- S6F11 通知 Host "事件已發生"（如 LOT_INFO_SET）
- S2F42 回覆 "命令已處理"
- 按 SEMI E30 規範，兩者為獨立訊息流

### 影響
部分 Host/MES 可能預期先收到 S2F42 再收到 S6F11，會出現時序衝突。

### 解決方案
- 告知客戶 Host 端需支援 **非同步處理**（S6F11 和 S2F42 可能交錯到達）
- 參考實際時序 → [SECS-Real-Examples.md](SECS-Real-Examples.md) §4

---

## Issue #4: S2F34 DRACK=0x02 — EAP 請求的 VID 不存在

### 描述
Host EAP 發 S2F33 Define Report 後，設備回 S2F34 **DRACK=0x02**（拒絕），
隨後所有 S6F11 事件報告被壓制（log：`CEID be disabled, abort send`），
本次 lot 完全無生產資料上傳。

### 原因
DRACK=0x02（SEMI E30）= S2F33 中至少一個 VID（SVID/ECID）在該機台不存在，
且為 **atomic 拒絕** — 只要一個 VID 缺失，整個 S2F33（含所有 RPTID）全部被拒。

**根本原因並非機台問題，而是 EAP mapping 不符：**
- HT9045/HT9046 的 SVID/ECID 是**全客戶共用的同一套固定集合**（號碼範圍約 1000～65095）。
- 機台**不會**為個別客戶重新編號 SVID/ECID。
- 客戶必須**依據不同機台軟體版本，去調整 EAP 對機台的 SVID/ECID mapping**。

### 識別方法
將 S2F33 log 中 Host 請求的 VID 清單，與程式碼實際註冊的 VID 比對：
```powershell
$sv="SECSGEM\uHGemHT9045_SV.cpp"; $ec="SECSGEM\uHGemHT9045_EC.cpp"
$svids=Select-String -Path $sv -Pattern 'SetSVDataPointer\(\s*(\d+)' | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
$ecids=Select-String -Path $ec -Pattern 'SetECDataPointer\(\s*(\d+)' | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
$all=$svids+$ecids
# 將 $req 改成 log 中 S2F33 請求的 VID 清單
$req=@(250,300,301,100001,300001,400001)
foreach($r in $req){ "VID $r : $(if($all -contains $r){'EXISTS'}else{'MISSING'})" }
```

### 解決方案（客戶 / EAP 端）
1. 向鴻勁取得該軟體版本的**官方 SVID/ECID 清單**（全客戶共用、同版本一致）。
2. EAP 刪除清單中不存在的 VID，並重新 mapping 到機台實際的 SVID/ECID 號碼。
3. 重連 HSMS，確認 S2F34 回 `0x00`（Accepted）、S6F11 恢復。

### 案例
Microchip Philippines（CC 862, JB-Elite）HT9046LS V3.33，2026-06-02：
EAP 請求 6 位數 VID（`100001`、`300001–300056`、`400001–400048`）及 <1000 的 VID，
這些在本機（最大 VID = 65095）全部不存在，146 個 VID 中僅 2 個（`10720`/`10730`）巧合命中。
為典型的 EAP 套錯規格案例。

---

## Issue #3: S7F17 Recipe 刪除 — 使用中判斷

### 描述
S7F17 刪除 Recipe 時，如果該 Recipe 正在使用中，回傳 ACKC7=1（拒絕）。

### 注意事項
- Host 端需處理 ACKC7=1 的情況（不能直接刪除正在跑的 Recipe）
- 需先切換到其他 Recipe，再刪除

---

## 問題回報格式

遇到新問題時，請按以下格式填寫：

```markdown
## Issue #N: [問題簡述]

### 描述
[問題詳細描述]

### 重現步驟
1. ...
2. ...
3. ...

### 影響版本
[程式碼版本 + Excel 版本]

### 修復
[修復方式、修改的檔案、相關 commit]

### 驗證方式
[如何確認已修復]
```
