# SECS/GEM 開發操作規程

> 程式碼版本：`HT9011UC_Code_V3.33.900.0_20260331`
> 適用：HT9045 / HT9046 SECS/GEM 模組新增與維護作業

---

## 1. 新增 CEID（收集事件）

1. 在 `uHGemHT9045.h` 的 `ETypeStruct::enum` 中加入新事件（`TotalEvent` 之前）
2. 在 `uHGemHT9045.cpp` 建構式中加入 `EventDescription[SECS_EVENT.XXX] = "描述"`
3. 在 `AddReprot()` / `AddCEID()` 中註冊 RPTID 與 CEID 連結
4. 在觸發位置呼叫 `HGem->LocalEventReport(SECS_EVENT.XXX)`

---

## 2. 新增 SVID（狀態變量）

在 `uHGemHT9045_SV.cpp` 的 `AddSV()` 內新增：

```cpp
HGemPtr->SetSVDataPointer(SVID編號, HType.型別, "名稱", "單位", &變量指標, "描述");
```

> ⚠️ **新增前必須確認 ID 未被 ECID 使用**（見第 4 節稽核程序）

---

## 3. 新增 ECID（設備常數）

在 `uHGemHT9045_EC.cpp` 的 `AddEC()` 內新增：

```cpp
HGemPtr->SetECDataPointer(ECID編號, HType.型別, "名稱", "單位", &變量指標, "Max", "Min", "Default", "描述");
```

> ⚠️ **新增前必須確認 ID 未被 SVID 使用**（見第 4 節稽核程序）

---

## 4. SVID / ECID 號碼重複稽核

**規則**：SVID 與 ECID 的 ID 號碼**不可相同**。新增任何一方時，需先確認另一方未使用同一 ID。

### 快速稽核腳本（Python）

```python
import re

sv_path = r"D:\HT9045\HT9011UC_Code_xxxx\SECSGEM\uHGemHT9045_SV.cpp"
ec_path = r"D:\HT9045\HT9011UC_Code_xxxx\SECSGEM\uHGemHT9045_EC.cpp"

with open(sv_path, "r", encoding="cp950", errors="replace") as f:
    sv_lines = f.readlines()
with open(ec_path, "r", encoding="cp950", errors="replace") as f:
    ec_lines = f.readlines()

sv_ids = {}
for i, line in enumerate(sv_lines, 1):
    m = re.search(r'^\s+HGemPtr->SetSVDataPointer\s*\(\s*(\d+)', line)
    if m: sv_ids[int(m.group(1))] = i

ec_ids = {}
for i, line in enumerate(ec_lines, 1):
    m = re.search(r'^\s+HGemPtr->SetECDataPointer\s*\(\s*(\d+)', line)
    if m: ec_ids[int(m.group(1))] = i

overlap = set(sv_ids) & set(ec_ids)
if overlap:
    for n in sorted(overlap):
        print(f"  [CONFLICT] ID {n} -- SV line {sv_ids[n]}, EC line {ec_ids[n]}")
else:
    print("No SVID/ECID conflicts found.")
```

### 尋找可用空閒 ID

```python
# 以 start_num 為起點向後尋找最近的空閒號碼
used = set(sv_ids) | set(ec_ids)
for n in range(start_num, start_num + 50):
    if n not in used:
        print(f"Next free ID: {n}")
        break
```

### 已知衝突修正紀錄

| 日期 | 衝突 ID | 說明 | 修正方式 |
|------|---------|------|----------|
| 2026-04-01 | 1191 | SVID 1191 `Error Bin Count` (JerryYang 20250120) 與 ECID 1191 `USE_RFID_READER` 重複 | SVID 改為 **1192**；Excel `SECS_20250717_Ifor.xlsx` 同步更新：插入 ECID 1191 行，原 SVID 1191 行改為 1192 |

---

## 5. 同步更新 Excel 規格表

修改 SVID/ECID 後，必須同步更新：
`d:\HT9045\.github\skills\ht9045-secs-sem\docs\SECS_20260401_Steven.xlsx`（工作表 `SV & EC`）

| 欄位（`SV & EC` 工作表） | 說明 |
|--------------------------|------|
| 欄 B (ID) | SVID 或 ECID 號碼 |
| 欄 C (SV) | 若為狀態變量填 `V` |
| 欄 D (EC) | 若為設備常數填 `V` |
| 欄 F (Function) | 名稱 |
| 欄 G (Type) | 資料型別（ASCII / INT_4 / BOOLEAN 等） |
| 欄 I/J/K (Max/Min/Default) | 僅 ECID 需填 |
| 欄 L (Description) | 描述 |
| 欄 O (HT9045/HT9046) | 適用機型填 `V` |

---

## 6. 新增 Remote Command (RCMD)

在 `uHGemHT9045.cpp` 的 `S2F42_Host_Command_Acknowledge()` 中加入新的 `else if(S.AnsiPos("CMD_NAME")==1)` 區塊。

同步更新 `SECS_20260401_Steven.xlsx` 工作表 `Remote Command`，HT9045/HT9046 欄填入 `V`。
