# SXML 生成與使用指南 (SXML Generation Guide)

> 適用：HT9045 SECS/GEM 模組 — SECS Express 工具  
> 建立日期：2026-04-16

---

## 概述

SXML（SECS XML）是 SECS Express 使用的訊息定義格式（`.sxml`）。HT9045 提供一份預建的 SXML 範本，客戶可匯入 SECS Express 進行連線測試與訊息模擬。

### 檔案位置

| 檔案 | 路徑 | 說明 |
|------|------|------|
| 完整定義（原廠） | `D:\SECSGEM_TOOL\SECS Express\H9045_3.sxml` | 全部 SxFy 定義 |
| RCMD 範本 | [references/HT9045_RCMD_Template.sxml](HT9045_RCMD_Template.sxml) | 常用 RCMD + 測試訊息 |

---

## SXML 基本結構

```xml
<?xml version="1.0" encoding="UTF-8"?>
<secs>
  <message name="顯示名稱" stream="2" function="41" direction="host-to-equipment"
           description="說明文字">
    <item name="根節點" format="list">
      <item name="RCMD" format="ascii">START</item>
      <item name="PARAMS" format="list">
        <!-- 參數列表 -->
      </item>
    </item>
  </message>
</secs>
```

### 關鍵屬性

| 屬性 | 說明 | 範例 |
|------|------|------|
| `name` | SECS Express 左側樹狀目錄顯示名稱 | `S2F41_START` |
| `stream` | Stream 編號 | `2` |
| `function` | Function 編號 | `41` |
| `direction` | 訊息方向 | `host-to-equipment` 或 `equipment-to-host` |
| `description` | 說明文字 | `Host Command Send - START` |

### format 值對照

| SXML format | SECS-II 型別 | 說明 |
|-------------|:------------:|------|
| `list` | L (List) | 巢狀結構 |
| `ascii` | A (ASCII) | 文字字串 |
| `binary` | B (Binary) | 二進位資料 |
| `boolean` | BOOLEAN | 布林 |
| `int1` | I1 | 有號 1 byte |
| `int2` | I2 | 有號 2 byte |
| `int4` | I4 | 有號 4 byte |
| `uint1` | U1 | 無號 1 byte |
| `uint2` | U2 | 無號 2 byte |
| `uint4` | U4 | 無號 4 byte |
| `float4` | F4 | 單精度浮點 |
| `float8` | F8 | 雙精度浮點 |

---

## 新增 RCMD 步驟

### 步驟 1：確定 RCMD 名稱與參數

查閱 [RCMD-Reference.md](RCMD-Reference.md) 找到目標 RCMD 的：
- RCMD 名稱（如 `PP_SELECT`）
- 參數名稱與型別（如 `PPID` = ASCII）

### 步驟 2：撰寫 SXML 訊息

**無參數 RCMD**（如 `START`）：
```xml
<message name="S2F41_START" stream="2" function="41" direction="host-to-equipment"
         description="Host Command: START">
  <item name="S2F41" format="list">
    <item name="RCMD" format="ascii">START</item>
    <item name="PARAMS" format="list">
    </item>
  </item>
</message>
```

**帶參數 RCMD**（如 `PP_SELECT`）：
```xml
<message name="S2F41_PP_SELECT" stream="2" function="41" direction="host-to-equipment"
         description="Host Command: PP_SELECT (Recipe 切換)">
  <item name="S2F41" format="list">
    <item name="RCMD" format="ascii">PP_SELECT</item>
    <item name="PARAMS" format="list">
      <item name="CPNAME_1" format="list">
        <item name="CPNAME" format="ascii">PPID</item>
        <item name="CPVAL" format="ascii">MyRecipeName</item>
      </item>
    </item>
  </item>
</message>
```

**多參數 RCMD**（如 `SET_LOT_INFO`）：
```xml
<message name="S2F41_SET_LOT_INFO" stream="2" function="41" direction="host-to-equipment"
         description="Host Command: SET_LOT_INFO">
  <item name="S2F41" format="list">
    <item name="RCMD" format="ascii">SET_LOT_INFO</item>
    <item name="PARAMS" format="list">
      <item name="CPNAME_1" format="list">
        <item name="CPNAME" format="ascii">XML</item>
        <item name="CPVAL" format="ascii">LotID=TEST001;DeviceID=DUT01;Operator=OP01</item>
      </item>
      <item name="CPNAME_2" format="list">
        <item name="CPNAME" format="ascii">DISPLAY</item>
        <item name="CPVAL" format="ascii">1</item>
      </item>
    </item>
  </item>
</message>
```

### 步驟 3：加入 S2F42 回覆訊息

每個 RCMD 都需要一個對應的 S2F42 回覆定義：
```xml
<message name="S2F42_Ack" stream="2" function="42" direction="equipment-to-host"
         description="Host Command Acknowledge">
  <item name="S2F42" format="list">
    <item name="HCACK" format="binary">0</item>
    <item name="CPACK" format="list">
    </item>
  </item>
</message>
```

### 步驟 4：匯入 SECS Express

1. 開啟 SECS Express
2. File → Import Messages → 選擇 `.sxml` 檔案
3. 訊息會出現在左側樹狀列表
4. 雙擊訊息可修改參數值
5. 點 Send 發送

---

## 常見格式錯誤

| 錯誤 | 症狀 | 修正 |
|------|------|------|
| `format="string"` | 匯入失敗 | 改為 `format="ascii"` |
| 缺少 `<item>` 結尾標籤 | XML 解析錯誤 | 確認每個 `<item>` 都有 `</item>` |
| RCMD 名稱大小寫錯誤 | HCACK=3（未知命令） | RCMD 名稱需完全比對（大寫） |
| PARAMS list 為空但省略 | 部分版本解析失敗 | 保留空的 `<item name="PARAMS" format="list"></item>` |
| 編碼非 UTF-8 | 中文亂碼 | XML 第一行加 `encoding="UTF-8"` |
| `direction` 設錯 | SECS Express 警告 | S2F41 是 `host-to-equipment`，S2F42 是 `equipment-to-host` |

---

## HCACK 回傳值參照

| 值 | 意義 | 常見原因 |
|:--:|------|---------|
| 0 | 成功 | — |
| 1 | 拒絕（命令不存在） | RCMD 名稱打錯 |
| 2 | 拒絕（無法執行） | 狀態不對（如未 Online） |
| 3 | 拒絕（參數錯誤） | CPNAME 不存在或 CPVAL 格式錯 |
| 4 | 尚未完成 | 命令正在排隊 |
| 5 | 拒絕（權限不足） | 非 Remote 模式 |
| 7 | Recipe 不存在 | PP_SELECT 的 PPID 無效（HT9045 自定義） |
| 8 | 重複 MES 連線 | MES1/MES2 衝突（HT9045 自定義） |
| 9 | 設備忙碌 | 正在執行其他命令（HT9045 自定義） |
| 10 | Lot 進行中 | 命令需 Lot End 後才能執行（HT9045 自定義） |

> 完整 HCACK 定義 → [SECS-Debug-Procedures.md](SECS-Debug-Procedures.md)

---

## 完整測試流程範例

使用 SECS Express + SXML 範本進行功能驗證：

```
1. 匯入 HT9045_RCMD_Template.sxml
2. 建立連線：
   → 發送 S1F13 → 確認收到 S1F14 (COMMACK=0)
3. 查詢狀態：
   → 發送 S1F1 → 確認收到 S1F2
   → 發送 S1F3 (MachineState) → 確認收到 S1F4
4. 切換 Recipe：
   → 發送 S2F41_PP_SELECT (PPID=TestRecipe) → 確認 HCACK=0
5. 設定 Lot 資訊：
   → 發送 S2F41_SET_LOT_INFO → 確認 HCACK=0
6. 開始運轉：
   → 發送 S2F41_START → 確認 HCACK=0
   → 等待 S6F11 事件通知
7. 暫停/停止：
   → 發送 S2F41_PAUSE → 確認 HCACK=0
```

---

## 從程式碼生成 SXML 的方法

若需從 HT9045 原始碼自動生成 SXML，可參考以下思路：

1. **RCMD 來源**：解析 `uHGemHT9045.cpp` 中 `S2F42_Host_Command_Acknowledge()` 的 `else if` 分支
2. **SVID 來源**：解析 `uHGemHT9045_SV.cpp` 中 `SetSVDataPointer()` 呼叫
3. **ECID 來源**：解析 `uHGemHT9045_EC.cpp` 中 `SetECDataPointer()` 呼叫
4. **CEID 來源**：解析 `uHGemHT9045.h` 中 `ETypeStruct` enum

> 目前尚未建立自動化工具。如需大量更新，建議手動維護 SXML 範本。
