# GEM 客戶配置樣板 (Customer Config Templates)

> 適用：新客戶快速配置 HT9045 SECS/GEM  
> 建立日期：2026-04-16

---

## 1. GemSys.ini 基礎配置

設定檔位置：`system\GEM\GemSys.ini`

### 1.1 最小可用配置

```ini
[HSMS]
ConnectionMode=0          ; 0=Passive (Handler等Host連入), 1=Active
Port=5000                 ; TCP Port（兩端必須一致）
DeviceID=0                ; Session ID（兩端必須一致）
IPAddress=192.168.1.100   ; Active 模式時填 Host IP；Passive 模式此欄不用

[Timer]
T3TimeOut=120             ; Reply Timeout (秒)
T5TimeOut=10              ; Connect Separation (秒)
T6TimeOut=5               ; Control Transaction (秒)
T7TimeOut=10              ; Connection Select (秒)
T8TimeOut=5               ; Intercharacter (秒)

[GEM]
InitialOnline=1           ; 啟動後自動 Online (1=是, 0=否)
InitialRemote=1           ; 啟動後自動 Remote (1=是, 0=否)
```

### 1.2 常見客戶情境

#### 情境 A：標準工廠 (MES 控制)

```ini
ConnectionMode=0          ; Passive — MES 主動連入
Port=5000
DeviceID=0
T3TimeOut=120
InitialOnline=1
InitialRemote=1
```

> 大多數客戶使用此配置。MES 開機後自動連到 Handler。

#### 情境 B：SECS Express 測試（調試用）

```ini
ConnectionMode=0          ; Passive
Port=5000
DeviceID=0
T3TimeOut=300             ; 延長到 300 秒（手動操作時間充裕）
InitialOnline=1
InitialRemote=0           ; Local — 測試時不希望被遠端控制
```

#### 情境 C：Active 模式（Handler 主動連 Host）

```ini
ConnectionMode=1          ; Active — Handler 主動連出
Port=5000
DeviceID=0
IPAddress=192.168.1.50    ; Host/MES 的 IP 位址
T5TimeOut=30              ; 斷線後重連間隔（增加以避免頻繁重連）
T7TimeOut=30
```

#### 情境 D：GEM300 + ATK AMR

```ini
ConnectionMode=0
Port=5000
DeviceID=0
T3TimeOut=120

[GEM300]
EnableE84=1               ; 啟用 E84 PI/O Carrier Handoff
EnableE87=1               ; 啟用 E87 Carrier Management
EnableE23=1               ; 啟用 E23 Auto Run
```

> ATK AMR 場景詳見 → [ATK-AMR-Scenario.md](ATK-AMR-Scenario.md)

---

## 2. 連線測試步驟

### Step 1: 網路確認
```
ping [Handler IP]         → 必須通
telnet [Handler IP] 5000  → 必須能連（確認 Port 開放）
```

### Step 2: SECS Express 連線測試
1. 開啟 SECS Express (SecsExpress.exe)
2. Connection → Mode = Active → IP = Handler IP → Port = GemSys.ini 的 Port
3. Connect → 觀察狀態欄是否顯示 "Connected"
4. 發送 S1F13 → 等待 S1F14 回應

### Step 3: 基礎功能驗證

| 步驟 | 發送 | 預期回應 | 確認項目 |
|------|------|---------|---------|
| 1 | S1F13 | S1F14 (COMMACK=0) | 通訊建立 |
| 2 | S1F17 | S1F18 (ONLACK=0) | 上線成功 |
| 3 | S1F1 | S1F2 (MDLN+SOFTREV) | 設備識別 |
| 4 | S1F3 [SVID 1011] | S1F4 (MachineState) | SVID 查詢 |
| 5 | S2F41 "START" | S2F42 (HCACK=0) | RCMD 執行 |

---

## 3. 客戶需求對照表

| 客戶需求 | 需要配置 | 參考 |
|---------|---------|------|
| MES 自動控制 | Online/Remote + RCMD | 情境 A |
| Recipe 自動下載 | S7Fx 支援 + FTP | SKILL.md Recipe 章節 |
| Lot Info 帶入 | SET_LOT_INFO RCMD | [SECS-Real-Examples.md](SECS-Real-Examples.md) §4 |
| 警報通知 | S5F1 + S5F3 Enable | [SECS-Debug-Procedures.md](SECS-Debug-Procedures.md) §5 |
| Event Report 訂閱 | S2F33 + S2F35 + S2F37 | [SECS-Real-Examples.md](SECS-Real-Examples.md) §3 |
| Carrier/OHT 整合 | GEM300 E84/E87 | 情境 D + [SEMI-E84-Reference.md](SEMI-E84-Reference.md) |

---

## 4. 交付客戶的配置清單

交機時需確認的 GEM 設定：

| 項目 | 確認 |
|------|:----:|
| IP / Port | ☐ |
| ConnectionMode (Active/Passive) | ☐ |
| DeviceID | ☐ |
| Timer (T3/T5/T6/T7/T8) | ☐ |
| InitialOnline/Remote | ☐ |
| S1F13 連線測試通過 | ☐ |
| S2F41 RCMD 測試通過 | ☐ |
| S6F11 Event Report 收得到 | ☐ |
| S5F1 Alarm 通知正常 | ☐ |
| Recipe 上下載測試通過 | ☐ |
