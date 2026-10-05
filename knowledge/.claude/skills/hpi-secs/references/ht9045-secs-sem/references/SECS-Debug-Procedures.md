# SECS/GEM 除錯程序 (Debug Procedures)

> 適用：HT9045 SECS/GEM 通訊問題排查  
> 建立日期：2026-04-16

---

## 1. HSMS 連線失敗

### 症狀
- Handler 顯示 Offline / Disconnect
- Host 無法收到 S1F14 回應

### 排查流程

```
HSMS 連線失敗
├─ Step 1: 確認網路連通
│  └─ ping Handler IP → 不通 → 檢查網路線 / IP 設定
│
├─ Step 2: 確認 Port 設定
│  ├─ Handler 端：system\GEM\GemSys.ini → [HSMS] Port
│  └─ Host 端：SECS Express → Connection → Port (預設 5000)
│
├─ Step 3: 確認 Active/Passive 模式
│  ├─ GemSys.ini → ConnectionMode
│  │  ├─ 0 = Passive (Handler 等待 Host 連入) ← 大多數
│  │  └─ 1 = Active (Handler 主動連 Host)
│  └─ 兩端必須一端 Active、一端 Passive
│
├─ Step 4: 確認 DeviceID 一致
│  ├─ GemSys.ini → DeviceID
│  └─ Host 端 Session ID 必須一致
│
└─ Step 5: 檢查 Timer（T5/T6/T7）
   ├─ T5 = 10s (Connect Separation) — 太短可能重複斷線
   ├─ T7 = 10s (Connection Select) — 太短可能 Timeout
   └─ 在 GemSys.ini 中調整
```

### 常用修復

| 問題 | 修復 |
|------|------|
| 連不上 | 確認 IP:Port + Active/Passive 模式 |
| 連上又斷 | 增加 T5 到 30s |
| Select 超時 | 增加 T7 到 30s |
| 防火牆阻擋 | 開放 TCP Port 5000（或自訂 Port） |

---

## 2. 訊息超時 (Reply Timeout)

### 症狀
- S2F42 沒有回來
- S6F12 沒有回來
- Handler Log 顯示 "T3 Timeout"

### 排查流程

```
Reply Timeout (T3)
├─ Step 1: 確認 T3 設定
│  ├─ 預設 120 秒
│  └─ GemSys.ini → T3TimeOut
│
├─ Step 2: Host 是否有回應
│  ├─ 檢查 Host 端 Log — 是否收到訊息但未回覆
│  └─ 如果 Host 處理慢，增加 T3 到 300s
│
├─ Step 3: 訊息格式問題
│  ├─ S2F41 的 RCMD 名稱是否正確
│  ├─ 封包結構是否符合 SECS-II 格式
│  └─ 用 SECS Express 抓包對比
│
└─ Step 4: 網路品質
   └─ 封包丟失 → 檢查網路交換機 / 線路品質
```

### Timer 參數參考

| Timer | 預設 | 用途 | 建議調整場景 |
|:-----:|:----:|------|------------|
| T3 | 120s | Reply Timeout | Host 回覆慢時增加 |
| T5 | 10s | Connect Separation | 連線不穩時增加 |
| T6 | 5s | Control Transaction | 一般不需調整 |
| T7 | 10s | Connection Select | HSMS 握手慢時增加 |
| T8 | 5s | Intercharacter | 一般不需調整 |

---

## 3. RCMD 執行失敗

### 症狀
- S2F42 回傳 HCACK ≠ 0

### HCACK 錯誤碼排查

| HCACK | 含義 | 常見原因 | 修復 |
|:-----:|------|---------|------|
| 0x01 | Invalid command | RCMD 名稱錯誤 | 檢查拼寫，參照 [RCMD-Reference.md](RCMD-Reference.md) |
| 0x02 | Cannot do now | 機台狀態不允許 | 確認 GEM 狀態為 Online/Remote + Auto 模式 |
| 0x03 | Parameter error | CPNAME/CPVAL 格式錯 | 檢查參數名、XML 格式 |
| 0x04 | Will finish later | 正常（異步執行） | 等待 S6F11 事件通知完成 |
| 0x05 | Already active | 重複下達 | 等待前一次執行完成 |
| 0x07 | HT9045: Offline | 需要上線 | 發送 S1F17 → Online |
| 0x08 | HT9045: Not Remote | 需要遠端 | Handler 切換為 Remote 模式 |
| 0x09 | HT9045: Not Idle | 需要閒置 | 等待機台閒置 |
| 0x0A | HT9045: Recipe error | 工作檔問題 | 檢查 PPID 是否存在 |

---

## 4. Event Report 未收到 (S6F11)

### 排查流程

```
S6F11 未觸發
├─ Step 1: 確認 CEID 已啟用
│  └─ S2F37 是否發了 Enable Event？
│     └─ 用 SECS Express 發送 S2F37 啟用所有 CEID
│
├─ Step 2: 確認 Report 已定義
│  └─ S2F33 是否定義了 RPTID + SVID？
│     └─ S2F35 是否連結 CEID 和 RPTID？
│
├─ Step 3: Handler 有觸發但 Host 沒收到
│  ├─ 檢查 Handler 端 GEM Log
│  └─ 確認 S6F12 有回覆（Host 必須回 S6F12 Ack）
│
└─ Step 4: S6F12 沒回覆
   └─ Handler 會在 T3 後超時，後續 S6F11 可能被暫停
```

### 標準連線後初始化順序

```
1. S1F13  — 建立通訊
2. S2F33  — 定義 Report（RPTID → SVID 組合）
3. S2F35  — 連結 CEID → RPTID
4. S2F37  — 啟用 CEID（Enable Event）
5. S1F3   — 查詢初始狀態（可選）
6. S2F15  — 設定 EC 常數（可選）
7. 等待 S6F11 — Handler 開始發送 Event
```

---

## 5. Alarm 相關問題

### 5.1 Alarm Code 查詢

Alarm ID (ALID) 格式：`CCCPPPSSS`
- `CCC` = Alarm Class（位置群組，如 317 = Fix Tray）
- `PPP` = Position Code
- `SSS` = Alarm Code

> 完整 1551 筆 Alarm 清單 → [Alarm-Reference.md](Alarm-Reference.md)

### 5.2 S5F1 未通知

| 原因 | 修復 |
|------|------|
| Alarm 未啟用 | S5F3 發送 Enable Alarm |
| 通訊未建立 | 先完成 S1F13 連線 |
| 狀態為 Offline | 切換為 Online/Remote |

---

## 6. Recipe 下載失敗 (S7Fx)

### 排查流程

| 步驟 | 檢查 | 修復 |
|------|------|------|
| S7F1→S7F2 | 是否收到 PPGNT=0（授權）| PPGNT=1 = 已存在，PPGNT=2 = 空間不足 |
| S7F3→S7F4 | 資料傳送是否成功 | 檢查 Recipe 檔案格式 |
| S7F5→S7F6 | 查詢是否可用 | 確認 PPID 在 IniData/Data/ 下存在 |

---

## 7. 通訊 Log 位置

| 項目 | 路徑 |
|------|------|
| HT9045 GEM Log | Handler 端 `system\GEM\Log\` 或 GEM UI 中查看 |
| SECS Express Log | `D:\SECSGEM_TOOL\SECS Express\Log\` |
| SF Code 範例日誌 | `D:\SECSGEM_TOOL\SECS GEM SF Code Log\` |
| GemSys.ini 設定檔 | `system\GEM\GemSys.ini` |

---

## 參照

| 文件 | 用途 |
|------|------|
| [SECS-Real-Examples.md](SECS-Real-Examples.md) | 實際通訊範例（連線、RCMD、Event） |
| [RCMD-Reference.md](RCMD-Reference.md) | RCMD 完整列表與 HCACK 回傳值 |
| [Alarm-Reference.md](Alarm-Reference.md) | 1551 筆 Alarm Code |
| [SECS-Programming-Guide.md](SECS-Programming-Guide.md) | SEMI 標準、GEM 狀態模型 |
| [GEM-Customer-Config-Templates.md](GEM-Customer-Config-Templates.md) | GemSys.ini 配置樣板 |
