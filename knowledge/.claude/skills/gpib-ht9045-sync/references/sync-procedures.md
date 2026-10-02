# 跨專案同步：詳細操作程序

> 本文件為 `gpib-ht9045-sync` SKILL 的完整操作參考。
> 概述與啟動流程請見 [SKILL.md](../SKILL.md)。

---

## 路徑組合

取得版本後，組合出以下路徑（後續所有操作使用這些路徑）：

```python
# HT9045 基準檔
HT_DIR   = r"d:\HT9045\<使用者輸入的版本資料夾>"
HT_MTYPE = HT_DIR + r"\MachineType.h"       # CC_ 與 eTestMode 基準
HT_MDEF_H   = HT_DIR + r"\MessageDef.h"     # MSG_CMD extern 基準
HT_MDEF_CPP = HT_DIR + r"\MessageDef.cpp"   # MSG_CMD const 基準

# GPIB9045 目標檔
GPIB_DIR     = r"d:\GPIB9045\<使用者輸入的版本資料夾>"
GPIB_CMYDEF  = GPIB_DIR + r"\cmydef.h"      # CC_ 目標
GPIB_MDEF_H   = GPIB_DIR + r"\MessageDef.h" # MSG_CMD + eTestMode 目標
GPIB_MDEF_CPP = GPIB_DIR + r"\MessageDef.cpp"

# RS232Standard 目標檔（若使用者選擇跳過則設為 None）
RS232_DIR      = r"d:\RS232Standard\<使用者輸入的版本資料夾>"  # or None
RS232_CMYDEF   = RS232_DIR + r"\cmydef.h"      if RS232_DIR else None
RS232_MDEF_H   = RS232_DIR + r"\MessageDef.h"  if RS232_DIR else None
RS232_MDEF_CPP = RS232_DIR + r"\MessageDef.cpp" if RS232_DIR else None
```

> **注意**：若使用者只輸入資料夾名稱（不含完整路徑），自動加上對應的前置路徑：
> - HT9045 → `d:\HT9045\`
> - GPIB9045 → `d:\GPIB9045\`
> - RS232Standard → `d:\RS232Standard\`

---

## 執行前備份

在修改任何檔案前，建立 `.bak_YYYYMMDD` 備份：

```python
import shutil
from datetime import date

today = date.today().strftime('%Y%m%d')
targets = [GPIB_CMYDEF, GPIB_MDEF_H, GPIB_MDEF_CPP]
if RS232_DIR:
    targets += [RS232_CMYDEF, RS232_MDEF_H, RS232_MDEF_CPP]

for path in targets:
    bak = path + '.bak_' + today
    if not os.path.exists(bak):
        shutil.copy(path, bak)
```

---

## 共用工具函式

```python
import re

def read_lines_bytes(path):
    with open(path, 'rb') as f: raw = f.read()
    sep = b'\r\n' if b'\r\n' in raw else b'\n'
    return [l + sep for l in raw.split(sep)], sep

def write_bytes(path, lines):
    with open(path, 'wb') as f: f.write(b''.join(lines))
```

---

## Step 1：Customer Code 同步（CC_xxx）

### 比對邏輯

```python
def get_cc_defs(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()
    pattern = r'#define\s+(CC_\w+)\s+(\d+)'
    return {name: int(val) for name, val in re.findall(pattern, content)}

ht_cc    = get_cc_defs(HT_MTYPE)
gpib_cc  = get_cc_defs(GPIB_CMYDEF)
rs232_cc = get_cc_defs(RS232_CMYDEF) if RS232_DIR else {}

missing_gpib_cc  = {k: v for k, v in ht_cc.items() if k not in gpib_cc}
missing_rs232_cc = {k: v for k, v in ht_cc.items() if RS232_DIR and k not in rs232_cc}
value_diff_gpib  = {k: (ht_cc[k], gpib_cc[k]) for k in ht_cc
                    if k in gpib_cc and ht_cc[k] != gpib_cc[k]}
value_diff_rs232 = {k: (ht_cc[k], rs232_cc[k]) for k in ht_cc
                    if RS232_DIR and k in rs232_cc and ht_cc[k] != rs232_cc[k]}
```

### 插入規則

- 從 `MachineType.h` 取出缺少的 CC_ 原始行（保留 Big5 中文註解）
- **插入位置**：在所有 `#define CC_` 中，找最後一個值 < 新代碼的行，插入其後
- 按數值升序逐一插入，確保整體順序正確
- **值不一致**的情形：顯示警告，由使用者確認是否覆蓋
- **同時對 GPIB9045 與 RS232Standard 兩個目標執行相同插入**

### 插入程式碼

```python
def insert_cc_to_target(target_path, missing_cc, ht_lines):
    """將缺少的 CC_ 定義插入目標 cmydef.h"""
    missing_raw = {}
    for bline in ht_lines:
        ls = bline.decode('cp950', errors='replace')
        m = re.match(r'#define\s+(CC_\w+)\s+(\d+)', ls)
        if m and m.group(1) in missing_cc:
            missing_raw[m.group(1)] = (int(m.group(2)), bline.rstrip(b'\r\n'))

    tgt_lines, tgt_nl = read_lines_bytes(target_path)
    for name, (val, new_bytes) in sorted(missing_raw.items(), key=lambda x: x[1][0]):
        insert_after = -1
        for i, bline in enumerate(tgt_lines):
            ls = bline.decode('cp950', errors='replace')
            m = re.search(r'#define\s+CC_\w+\s+(\d+)', ls)
            if m and int(m.group(1)) < val:
                insert_after = i
        tgt_lines.insert(insert_after + 1, new_bytes + tgt_nl)
    write_bytes(target_path, tgt_lines)

# 取出 MachineType.h 原始行
ht_lines, _ = read_lines_bytes(HT_MTYPE)

# 同步到 GPIB9045
insert_cc_to_target(GPIB_CMYDEF, missing_gpib_cc, ht_lines)

# 同步到 RS232Standard（若啟用）
if RS232_DIR:
    insert_cc_to_target(RS232_CMYDEF, missing_rs232_cc, ht_lines)
```

---

## Step 2：MSG_CMD 同步（MessageDef.h + .cpp）

### 比對邏輯

```python
def get_msg_extern(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()
    return set(re.findall(r'extern\s+const\s+unsigned\s+int\s+(MSG_CMD_\w+)\s*;', content))

def get_msg_const(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()
    matches = re.findall(r'const\s+unsigned\s+int\s+(MSG_CMD_\w+)\s*=\s*(\d+);', content)
    return {n: int(v) for n, v in matches}

ht_h_cmds    = get_msg_extern(HT_MDEF_H)
ht_cpp_cmds  = get_msg_const(HT_MDEF_CPP)

gpib_h_cmds   = get_msg_extern(GPIB_MDEF_H)
gpib_cpp_cmds = get_msg_const(GPIB_MDEF_CPP)
rs232_h_cmds   = get_msg_extern(RS232_MDEF_H)   if RS232_DIR else set()
rs232_cpp_cmds = get_msg_const(RS232_MDEF_CPP)  if RS232_DIR else {}

missing_gpib_h    = ht_h_cmds - gpib_h_cmds
missing_gpib_cpp  = set(ht_cpp_cmds) - set(gpib_cpp_cmds)
missing_rs232_h   = ht_h_cmds - rs232_h_cmds   if RS232_DIR else set()
missing_rs232_cpp = set(ht_cpp_cmds) - set(rs232_cpp_cmds) if RS232_DIR else set()
```

### 插入規則

- **`.h`**：從 HT9045 `MessageDef.h` 取出缺少 extern 行（原始 bytes，保留中文註解），插入在目標最後一個 `extern const unsigned int MSG_CMD_` 行之後（整批插入，保留 HT 側順序）
- **`.cpp`**：從 HT9045 `MessageDef.cpp` 取出缺少 const 行（原始 bytes），插入在目標最後一個 `const unsigned int MSG_CMD_` 行之後（整批插入，保留 HT 側順序）
- 數值以 HT9045 `MessageDef.cpp` 為唯一基準，不得自行分配

### 插入程式碼

```python
def insert_msg_to_target(target_h, target_cpp, missing_h, missing_cpp,
                         ht_h_lines, ht_cpp_lines):
    """將缺少的 MSG_CMD_ 定義插入目標 MessageDef.h 與 .cpp"""
    # .h 處理
    ext_to_add = [bline for bline in ht_h_lines
                  if any(cmd.encode() in bline and b'extern' in bline
                         for cmd in missing_h)]
    tgt_h_lines, _ = read_lines_bytes(target_h)
    last_extern = max(i for i, l in enumerate(tgt_h_lines)
                      if b'extern' in l and b'MSG_CMD' in l)
    for i, line in enumerate(ext_to_add):
        tgt_h_lines.insert(last_extern + 1 + i, line)
    write_bytes(target_h, tgt_h_lines)

    # .cpp 處理
    const_to_add = [bline for bline in ht_cpp_lines
                    if re.search(rb'const\s+unsigned\s+int\s+(MSG_CMD_\w+)\s*=', bline)
                    and re.search(rb'const\s+unsigned\s+int\s+(MSG_CMD_\w+)\s*=', bline)
                       .group(1).decode() in missing_cpp]
    tgt_cpp_lines, _ = read_lines_bytes(target_cpp)
    last_const = max(i for i, l in enumerate(tgt_cpp_lines)
                     if re.search(rb'const\s+unsigned\s+int\s+MSG_CMD_', l))
    for i, line in enumerate(const_to_add):
        tgt_cpp_lines.insert(last_const + 1 + i, line)
    write_bytes(target_cpp, tgt_cpp_lines)

ht_h_lines, _   = read_lines_bytes(HT_MDEF_H)
ht_cpp_lines, _ = read_lines_bytes(HT_MDEF_CPP)

# 同步到 GPIB9045
insert_msg_to_target(GPIB_MDEF_H, GPIB_MDEF_CPP,
                     missing_gpib_h, missing_gpib_cpp,
                     ht_h_lines, ht_cpp_lines)

# 同步到 RS232Standard（若啟用）
if RS232_DIR:
    insert_msg_to_target(RS232_MDEF_H, RS232_MDEF_CPP,
                         missing_rs232_h, missing_rs232_cpp,
                         ht_h_lines, ht_cpp_lines)
```

---

## Step 3：eTestMode 同步（enum eTestMode）

### 比對邏輯

```python
def get_test_mode_members(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()
    m = re.search(r'enum\s+eTestMode\s*\{([^}]+)\}', content, re.DOTALL)
    if not m: return {}
    block = m.group(1)
    members = {}
    cur = 0
    for item in re.findall(r'(\w+)\s*(?:=\s*(\d+))?', block):
        name, val = item
        if not name or name in ('TotalTestMode',): continue
        if val:
            cur = int(val)
        members[name] = cur
        cur += 1
    return members

ht_modes    = get_test_mode_members(HT_MTYPE)
gpib_modes  = get_test_mode_members(GPIB_MDEF_H)
rs232_modes = get_test_mode_members(RS232_MDEF_H) if RS232_DIR else {}

missing_gpib_modes  = {k: v for k, v in ht_modes.items() if k not in gpib_modes}
missing_rs232_modes = {k: v for k, v in ht_modes.items() if RS232_DIR and k not in rs232_modes}
value_diff_gpib_modes  = {k: (ht_modes[k], gpib_modes[k]) for k in ht_modes
                           if k in gpib_modes and ht_modes[k] != gpib_modes[k]}
value_diff_rs232_modes = {k: (ht_modes[k], rs232_modes[k]) for k in ht_modes
                          if RS232_DIR and k in rs232_modes and ht_modes[k] != rs232_modes[k]}
```

### 修改規則

- 若有 **新增成員** 或 **值偏移**：直接覆寫目標 `MessageDef.h` 中的 enum eTestMode 區塊，將其替換為從 HT9045 `MachineType.h` 取出的完整 enum 區塊
- **同時對 GPIB9045 與 RS232Standard 兩個目標執行相同覆寫**
- GPIB9045 修改後同步更新 `d:\GPIB9045\.github\skills\gpib-program-manual\references\GPIB_Program_Manual_V12.04.md` 的 eTestMode 章節

---

## Step 4：驗證

執行同步後，必須對所有三項進行數量驗證（GPIB9045 與 RS232Standard 分別報告）：

```python
# Customer Code
ht_cc_final    = get_cc_defs(HT_MTYPE)
gpib_cc_final  = get_cc_defs(GPIB_CMYDEF)
rs232_cc_final = get_cc_defs(RS232_CMYDEF) if RS232_DIR else {}
cc_missing_gpib  = set(ht_cc_final) - set(gpib_cc_final)
cc_missing_rs232 = set(ht_cc_final) - set(rs232_cc_final) if RS232_DIR else set()
cc_diff_gpib  = {k for k in ht_cc_final if k in gpib_cc_final and ht_cc_final[k] != gpib_cc_final[k]}
cc_diff_rs232 = {k for k in ht_cc_final if RS232_DIR and k in rs232_cc_final and ht_cc_final[k] != rs232_cc_final[k]}

# MSG_CMD
gpib_h_final    = get_msg_extern(GPIB_MDEF_H)
gpib_cpp_final  = get_msg_const(GPIB_MDEF_CPP)
rs232_h_final   = get_msg_extern(RS232_MDEF_H)  if RS232_DIR else set()
rs232_cpp_final = get_msg_const(RS232_MDEF_CPP)  if RS232_DIR else {}
cmd_missing_gpib_h    = ht_h_cmds - gpib_h_final
cmd_missing_gpib_cpp  = set(ht_cpp_cmds) - set(gpib_cpp_final)
cmd_missing_rs232_h   = ht_h_cmds - rs232_h_final   if RS232_DIR else set()
cmd_missing_rs232_cpp = set(ht_cpp_cmds) - set(rs232_cpp_final) if RS232_DIR else set()

# eTestMode
gpib_modes_final  = get_test_mode_members(GPIB_MDEF_H)
rs232_modes_final = get_test_mode_members(RS232_MDEF_H) if RS232_DIR else {}
mode_diff_gpib  = {k: (ht_modes[k], gpib_modes_final.get(k)) for k in ht_modes
                   if gpib_modes_final.get(k) != ht_modes[k]}
mode_diff_rs232 = {k: (ht_modes[k], rs232_modes_final.get(k)) for k in ht_modes
                   if RS232_DIR and rs232_modes_final.get(k) != ht_modes[k]}

# 輸出報告
print("=== GPIB9045 ===")
print(f"CC_       : HT={len(ht_cc_final)}, GPIB={len(gpib_cc_final)}, missing={len(cc_missing_gpib)}, diff={len(cc_diff_gpib)}")
print(f"MSG_CMD.h : missing={len(cmd_missing_gpib_h)}")
print(f"MSG_CMD.cpp: missing={len(cmd_missing_gpib_cpp)}")
print(f"eTestMode : diff={len(mode_diff_gpib)}")

if RS232_DIR:
    print("=== RS232Standard ===")
    print(f"CC_       : HT={len(ht_cc_final)}, RS232={len(rs232_cc_final)}, missing={len(cc_missing_rs232)}, diff={len(cc_diff_rs232)}")
    print(f"MSG_CMD.h : missing={len(cmd_missing_rs232_h)}")
    print(f"MSG_CMD.cpp: missing={len(cmd_missing_rs232_cpp)}")
    print(f"eTestMode : diff={len(mode_diff_rs232)}")
```

驗證全部為 0 差異時，輸出同步完成報告。

---

## Step 5：記錄同步歷史

同步完成後，自動更新相關 agent.md 中「已知同步歷史」表格，新增一行：

| 項目 | 更新目標 |
|------|---------|
| Customer Code 新增項目 | `GPIB9045.agent.md` → Customer Code 同步歷史 |
| MSG_CMD 新增項目 | `GPIB9045.agent.md` → MSG_CMD 同步歷史 |
| eTestMode 新增項目 | `GPIB9045.agent.md` → eTestMode 同步歷史 |
| Customer Code 新增項目（RS232） | `RS232Standard.agent.md` → Customer Code 同步歷史（若有） |
| MSG_CMD 新增項目（RS232） | `RS232Standard.agent.md` → MSG_CMD 同步歷史（若有） |
| eTestMode 新增項目（RS232） | `RS232Standard.agent.md` → eTestMode 同步歷史（若有） |
| 以上三項（HT 側觀點） | `HT9045.agent.md` → 對應歷史 |

記錄格式：
```
| YYYY-MM-DD | 新增 N 個 CC_（範圍 xxx~xxx，含 CC_XXX 等）| AI 同步 |
| YYYY-MM-DD | 新增 N 個 MSG_CMD_（值 xxx~xxx，含 MSG_CMD_XXX 等）| AI 同步 |
| YYYY-MM-DD | 新增 eTestMode 成員 XXX=N，後續值各 +1 | AI 同步 |
```

### 追加 ops daily log

同步完成後，在當日 daily log 追加一筆記錄：

| 專案 | 路徑格式 |
|------|---------|
| HT9045 / GPIB9045 | `D:\docs\ops\daily\YYYYMMDD.md` |

追加至檔案末尾（換行後插入）：

```
- [跨專案同步] CC_ +N, MSG_CMD +N（GPIB9045）; CC_ +N, MSG_CMD +N（RS232Standard，若有）— gpib-ht9045-sync Skill
```

**規則：**
- 若三項同步結果均為 0 差異（無實際變更），則跳過追加
- 若 daily log 檔案不存在，跳過此步驟（不自動建立 daily log）

---

## 常見錯誤排除

| 問題 | 原因 | 解法 |
|------|------|------|
| 插入後中文亂碼 | 以 UTF-8 讀取 Big5 檔 | 改用 `open(path, 'rb')` + bytes 操作 |
| `UnicodeEncodeError: cp950` | print 含 emoji 字元 | 改用 ASCII 文字輸出，或用 `sys.stdout.buffer.write` |
| 插入位置錯誤（CC_ 值順序亂） | `insert_after` 計算錯誤 | 確認每次插入後重新掃描，或一次性整批插入前先排序 |
| MSG_CMD 值不一致 | GPIB 自行分配了不同號碼 | 以 HT9045 `MessageDef.cpp` 的數值為準，強制覆寫 |
| enum eTestMode 範圍找不到 | regex 過於嚴格 | 改用多行模式 `re.DOTALL`，且匹配 `\{[^}]+\}` |
