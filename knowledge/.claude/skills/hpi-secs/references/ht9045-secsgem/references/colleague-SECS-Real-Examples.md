# SECS/GEM 實際通訊範例

> 資料來源：`D:\SECSGEM_TOOL\SECS Express\H9045_3.sxml` + `D:\SECSGEM_TOOL\SECS GEM SF Code Log\`  
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`  
> 建立日期：2026-04-16

---

## 版本控制

| 版本 | 日期 | 更新者 | 說明 |
|------|------|--------|------|
| V1.00 | 2026-04-16 | Steven (AI) | 初版：7 個典型通訊流程範例 |

---

## 1. 連線建立 (S1F1 / S1F13)

### 1.1 Host → Handler: Are You There (S1F1)

```
[Receive] 2025-06-24 13:48:02.218
[S1F1] Are you There
.
```

### 1.2 Handler → Host: Online Data (S1F2)

```
[Send]    2025-06-24 13:48:02.232
[S1F2] On Line Data
<L[2]
  <A[9] "HT-9045HW">        ← 設備型號 (MDLN)
  <A[10] "V3.33_BETA">       ← 軟體版本 (SOFTREV)
>
```

### 1.3 Host → Handler: Connect Request (S1F13)

```
[Receive] 2025-06-24 13:48:11.279
[S1F13] Connect Request
<L[0]
>
```

### 1.4 Handler → Host: Connect Acknowledge (S1F14)

```
[Send]    2025-06-24 13:48:11.288
[S1F14] Connect Request Acknowledge
<L[2]
  <B[1] 0x00>                ← COMMACK: 0x00 = 成功
  <L[2]
    <A[9] "HT-9045HW">       ← MDLN
    <A[10] "V3.33_BETA">     ← SOFTREV
  >
>
```

**說明**：S1F13/S1F14 是 HSMS 層建立通訊連線的標準流程。Handler 回傳設備型號和軟體版本。

---

## 2. 查詢設備狀態 (S1F3 / S1F4)

### 2.1 查詢 SVID（Key Parameter 範例）

Host 發送 S1F3 指定要查的 SVID 清單：

```
[S1F3] Selected Equipment Status Request
<L>
  <U4 Name="Temperature Mode">1514</U4>    ← SVID 1514
  <U4 Name="Device Direction">2799</U4>     ← SVID 2799
  <U4 Name="Tray Direction">2800</U4>       ← SVID 2800
  <U4 Name="Setup File">1501</U4>           ← SVID 1501
</L>
```

**說明**：Host 查詢 4 個 SVID 的當前值。Handler 以 S1F4 回覆對應值。

---

## 3. 動態報告定義 (S2F33 + S2F35)

### 3.1 定義 Report (S2F33)

```
[Receive] 2025-06-24 13:49:04.399
[S2F33] Define Report
<L[2]
  <I4[1] 1>                                  ← DATAID
  <L[4]                                       ← 4 個 Report 定義
    <L[2]
      <U4[1] 100>                             ← RPTID = 100
      <L[3]
        <U4[1] 3>                             ← SVID 3 (ControlState)
        <U4[1] 4>                             ← SVID 4
        <U4[1] 5>                             ← SVID 5
      >
    >
    <L[2]
      <U4[1] 101>                             ← RPTID = 101
      <L[3]
        <U4[1] 1010>                          ← SVID 1010 (ProcessState)
        <U4[1] 1011>                          ← SVID 1011 (MachineState)
        <U4[1] 1501>                          ← SVID 1501 (SetupFile)
      >
    >
    <L[2]
      <U4[1] 102>                             ← RPTID = 102
      <L[7]
        <U4[1] 1502>                          ← SVID 1502
        <U4[1] 1513>                          ← SVID 1513
        <U4[1] 1514>                          ← SVID 1514 (TempMode)
        <U4[1] 1517>                          ← SVID 1517
        <U4[1] 1518>                          ← SVID 1518
        <U4[1] 2799>                          ← SVID 2799 (DeviceDir)
        <U4[1] 2800>                          ← SVID 2800 (TrayDir)
      >
    >
    <L[2]
      <U4[1] 26>                              ← RPTID = 26
      <L[2]
        <U4[1] 1420>                          ← SVID 1420
        <U4[1] 3540>                          ← SVID 3540
      >
    >
  >
>
```

```
[Send]    2025-06-24 13:49:04.563
[S2F34] Define Report Acknowledge
<B[1] 0x00>                                  ← DRACK: 0 = 成功
```

### 3.2 連結 CEID 與 Report (S2F35)

```
[Receive] 2025-06-24 13:49:12.213
[S2F35] Link Event Report
<L[2]
  <U4[1] 18>                                 ← DATAID
  <L[54]                                      ← 54 個 CEID-Report 連結
    <L[2]
      <U4[1] 1>                               ← CEID = 1 (Equipment_Offline)
      <L[3]
        <U4[1] 100>                           ← RPTID 100
        <U4[1] 101>                           ← RPTID 101
        <U4[1] 102>                           ← RPTID 102
      >
    >
    ...（共 54 組 CEID-Report 連結）
  >
>
```

**說明**：Host 先透過 S2F33 定義 Report 內容（SVID 組合），再透過 S2F35 將 Report 連結到特定 CEID。之後 S6F11 發報時就會攜帶這些 SVID 值。

---

## 4. Remote Command (S2F41 → S2F42 + S6F11)

### 4.1 SET_LOT_INFO（最典型的 RCMD 範例）

```
[Receive] 2025-07-30 09:11:10.539
[S2F41] Remote Command with Parameters
<L[2]
  <A[12] "SET_LOT_INFO">                     ← RCMD 名稱
  <L[2]                                       ← 2 組 CPNAME-CPVAL 參數
    <L[2]
      <A[8] "LOT_INFO">                      ← CPNAME = LOT_INFO
      <A[529] "<LOT_INFO>                    ← CPVAL = XML 格式
        <OPERATOR_ID>153004</OPERATOR_ID>
        <CUSTOMER>Huawei</CUSTOMER>
        <CUSTOMER_DEVICE_GROUP>Hi6H12</CUSTOMER_DEVICE_GROUP>
        <DEVICE_NAME>i6H12GFCV100</DEVICE_NAME>
        <STAGE>QC</STAGE>
        <STEP>RC</STEP>
        <REPORTCOUNT>1</REPORTCOUNT>
        <TEMPERATURE>25</TEMPERATURE>
        <TESTER_ID>TA608</TESTER_ID>
        <PROGRAM_NAME>Hi6H12GFCV100_FTAx8_V93K_R11.prog</PROGRAM_NAME>
        <INNER_LOT_ID>63K2D530.1</INNER_LOT_ID>
        <CUST_LOT_ID>HP9WK2006</CUST_LOT_ID>
        <TEST_BIN_NO>ALL</TEST_BIN_NO>
        <HANDLER_ID>H9SA5</HANDLER_ID>
        <CURR_QTY>362</CURR_QTY>
        <RC_QTY>1</RC_QTY>
      </LOT_INFO>">
    >
    <L[2]
      <A[7] "DISPLAY">                       ← CPNAME = DISPLAY
      <A[31] "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1"> ← 16-site bitmap
    >
  >
>
```

### 4.2 Handler 回應流程（S6F11 先於 S2F42）

```
[Send]    2025-07-30 09:11:10.704
[S6F11] Event Report Send                    ← ① 先發 Event Report
<L[3]
  <U4[1] 1>                                  ← DATAID
  <U4[1] 120>                                ← CEID = 120 (LOT_INFO_SET)
  <L[1]
    <L[2]
      <U4[1] 120>                            ← RPTID = 120
      <L[0]>                                  ← 報告本體（此處無額外 SVID）
    >
  >
>
```

```
[Send]    2025-07-30 09:11:10.711
[S2F42] Remote Command Acknowledge           ← ② 再回覆 RCMD Ack
<L[2]
  <B[1] 0x00>                                ← HCACK: 0 = 成功
  <L[0]>                                      ← 無額外 CPACK
>
```

```
[Receive] 2025-07-30 09:11:10.858
[S6F12] Event Report Acknowledge             ← ③ Host 確認收到 Event
<B[1] 0x00>                                  ← ACKC6: 0 = OK
```

**關鍵時序**：S2F41 收到 → **S6F11 先發** → **S2F42 後回** → S6F12 收到。這是 HT9045 的標準行為。

---

## 5. EC 常數修改 (S2F15 / S2F16)

```
[Receive] 2025-06-24 13:49:30.651
[S2F15] New Equipment Constant Send
<L[1]
  <L[2]
    <U4[1] 1530>                              ← ECID = 1530
    <A[63] "1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0">
  >                                           ← 32-site enable bitmap
>
```

```
[Send]    2025-06-24 13:49:30.731
[S2F16] New Equipment Constant Send Acknowledge
<B[1] 0x00>                                  ← EAC: 0 = 成功
```

**說明**：Host 修改 ECID 1530 的值（32-site 啟用位元圖）。

---

## 6. 警報通知 (S5F1 / S5F2)

```
[Send]    2023-08-04 18:01:08.875
[S5F1] Alarm Report Send
<L[3]
  <B[1] 0x80>                                ← ALCD: 0x80 = Alarm Set
  <U4[1] 317001721>                           ← ALID: 317_001_721
  <A[21] "No tray on fix tray 1">            ← ALTX: 警報描述文字
>
```

```
[Receive] 2023-08-04 18:01:09.171
[S5F2] Alarm Report Acknowledge
<B[1] 0x00>                                  ← ACKC5: 0 = OK
```

**ALID 格式解析**：`317001721`
- `317` = Alarm Class（位置群組）
- `001` = Position Code
- `721` = Alarm Code

> 詳細 ALID 格式與完整 1551 筆清單 → [Alarm-Reference.md](Alarm-Reference.md)

---

## 7. Event Report (S6F11 / S6F12)

### 7.1 典型 Event Report

```
[Send]    2025-06-09 08:00:04.518
[S6F11] Event Report Send
<L[3]
  <U4[1] 1>                                  ← DATAID（遞增）
  <U4[1] 27>                                 ← CEID = 27 (Lot_Start)
  <L[1]                                       ← Report 列表
    <L[2]
      <U4[1] 27>                             ← RPTID = 27（與 CEID 同編號）
      <L[0]>                                  ← 預設只含 SVID 1027（系統時間）
    >
  >
>
```

```
[Receive] 2025-06-09 08:00:04.596
[S6F12] Event Report Acknowledge
<B[1] 0x00>                                  ← ACKC6: 0 = OK
```

**說明**：每個 CEID 預設自帶同編號 Report ID，內含 SVID 1027（系統時間）。Host 可透過 S2F33 + S2F35 動態重新定義。

---

## HCACK 回傳值對照

| 值 | 含義 | 說明 |
|:--:|------|------|
| 0x00 | OK | 指令執行成功 |
| 0x01 | Invalid command | RCMD 名稱不認識 |
| 0x02 | Cannot do now | 目前狀態不允許 |
| 0x03 | Parameter error | CPNAME/CPVAL 格式錯誤 |
| 0x04 | Acknowledge, will finish later | 已接受，稍後完成 |
| 0x05 | Rejected, already active | 重複下達（已在執行中） |
| 0x06 | No such object exists | 指定物件不存在 |
| 0x07~0x0A | HT9045 自定義 | 見 RCMD-Reference.md |

---

## 通訊時序總覽

```
Host                          Handler (HT9045)
  |                               |
  |── S1F13 Connect ──────────>   |
  |   <────── S1F14 Ack ─────────|
  |                               |
  |── S2F33 Define Report ────>   |
  |   <────── S2F34 Ack ─────────|
  |                               |
  |── S2F35 Link Event ───────>   |
  |   <────── S2F36 Ack ─────────|
  |                               |
  |── S2F37 Enable Event ─────>   |
  |   <────── S2F38 Ack ─────────|
  |                               |
  |── S2F41 RCMD ─────────────>   |
  |   <────── S6F11 Event ────────|  ← 先發 Event
  |   <────── S2F42 RCMD Ack ─────|  ← 後回 Ack
  |── S6F12 Event Ack ────────>   |
  |                               |
  |   <────── S5F1 Alarm ─────────|  ← Handler 主動
  |── S5F2 Alarm Ack ─────────>   |
  |                               |
  |   <────── S6F11 Event ────────|  ← Handler 主動
  |── S6F12 Event Ack ────────>   |
```

---

## 參照

| 需要更多資訊 | 參照檔 |
|-------------|--------|
| 完整 142 SxFy 清單 | [SF-List-Reference.md](SF-List-Reference.md) |
| SVID/ECID 編號查詢 | [SVID-ECID-Reference.md](SVID-ECID-Reference.md) |
| CEID 事件定義 | [CEID-Reference.md](CEID-Reference.md) |
| RCMD 完整列表 | [RCMD-Reference.md](RCMD-Reference.md) |
| Alarm Code 清單 | [Alarm-Reference.md](Alarm-Reference.md) |
| SEMI 標準格式 | [SECS-Programming-Guide.md](SECS-Programming-Guide.md) |
| SXML 模板（客戶模擬用） | [HT9045_RCMD_Template.sxml](HT9045_RCMD_Template.sxml) |
