> 保存來源：`.claude/skills/ht9045-customer-code-manager/references/customer-code-workflow.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 客戶代碼新增 — 完整工作流程

> 從 SKILL.md 分離出的詳細步驟、Python 腳本、使用範例與特殊情況處理。

---

## 步驟 1：提取客戶資訊

從使用者提供的客戶代碼定義中提取以下資訊：

| 欄位 | 範例 | 說明 |
|------|------|------|
| **Code Symbol** | `CC_AMD_SG` | #define 常數名稱（大寫加底線） |
| **Code Number** | `807` | 客戶代碼（3~5位數字） |
| **Comment** | `AMD 新加坡` | 客戶名稱與地區（#define 後的註解） |
| **Function Name** | `FUNC_CC_AMD_SG` | 對應的功能函數名稱 |

### 地區 → 代理商自動推定表

從 Comment 提取地區關鍵字（不分大小寫），自動決定代理商：

| 地區關鍵字（任一符合即可） | 推定代理商 |
|---------------------------|------------|
| china / 中國 / suzhou / beijing / 上海 / 南京 / 西安 / 深圳 / 廣東 / 天津 / 武漢 / 長沙 / 寧波 / 成都 / 鄭州 / 杭州 / 重慶 / 合肥 | 鴻勁興業 |
| korea / 韓國 | TeraTech |
| singapore / 新加坡 / malaysia / 馬來西亞 | HTS |
| philippines / 菲律賓 / thailand / 泰國 / thai | JB-Elite |
| japan / 日本 | Spandnix |
| us / usa / 美國 / california | HPI-USA |
| europe / 歐洲 / germany / 德國 / france / 法國 / israel / 以色列 / malta | HPI-Euro |
| taiwan / 台灣 / tainan / 台南 / hsinchu / 新竹 | HPI-TW |
| 以上皆不符 | — |

## 步驟 2：定位最新版本檔案

- 掃描 `d:\HT9045\` 目錄，尋找最新版本的程式碼資料夾
- 版本模式：`HT9011UC_Code_V3.33.XXX.Z_YYYYMMDD*` 或 `HT9046LS_Code_V3.32.XXX_YYYYMMDD*`
- 提取版本號、日期，優先選擇日期最新的版本

## 步驟 3：在 MachineType.h 中新增定義

**自動位置解析**：掃描所有 `#define CC_\w+ (\d+)` 行，找到數值最大且 < 新代碼的行，插入其後。

```python
defines = [(i, int(m.group(1))) for i, l in enumerate(lines)
           if (m := re.search(r'^#define\s+CC_\w+\s+(\d+)', l))]
insert_after = max((i for i, n in defines if n < CC_NUM), default=None)
```

**插入格式**：`f"#define {CC_SYM:<26} {CC_NUM} //{COMMENT}"`  
**規則**：符號欄位左對齊至第 26 字元，必須保持 Big5（CP950）編碼。

## 步驟 4：在 CosFunction.cpp 中新增功能函數

**自動位置解析**：找到前一個較小代碼的 FUNC 函數體結束 `}\n//---` 位置後插入。

**插入格式**：
```cpp
void FUNC_CC_AMD_SG()
{
}
//------------------------------------------------------------------------------
```

## 步驟 5：在 DoCustomerFunction() 中新增 case 分支

**位置**：CosFunction.cpp 的 `DoCustomerFunction()` 函數中。

**自動位置解析**：switch 最後一個 `case` 行之後（`}` 前）：
```python
last_case_idx = max(i for i, l in enumerate(lines) if re.match(r'\s+case CC_', l))
```

**插入格式**：
```cpp
        case CC_AMD_SG:                     FUNC_CC_AMD_SG();                   break;  //AMD 新加坡
```
（對齊方式：case 符號欄到第 40 字元，FUNC 呼叫欄到第 60 字元）

**重要**：缺漏此步驟會導致新客戶在 switch 中無對應分支。

## 步驟 5.5：在 HandlerSys.dfm 中新增 UI 選項

**位置**：`rgCustomerList` TRadioGroup 的 `Items.Strings` 中。

**規則**：
- **按客戶代碼數字升序排列**（最重要）
- 格式：`'NNN CustomerSymbol        CustomerFullName'`
- 中文字元的 Unicode 轉義由 `dfm_escape()` 自動計算

```python
def dfm_escape(text):
    parts, buf = [], ""
    for ch in text:
        if ord(ch) < 128: buf += ch
        else:
            if buf: parts.append(f"'{buf}'"); buf = ""
            parts.append(f"#{ord(ch)}")
    if buf: parts.append(f"'{buf}'")
    return "".join(parts)
```

**位置自動解析**：
```python
dfm_entries = [(i, int(m2.group(1))) for i, l in enumerate(lines)
               if (m2 := re.match(r"\s+'(\d+) ", l))]
insert_after = max((i for i, n in dfm_entries if n < CC_NUM), default=None)
```

## 步驟 6：更新客戶代碼分布器 Skill

在 `u:\共用區\客戶需求單\.github\skills\customer-code-distributor\SKILL.md` 的索引表中新增一行。  
**代理商**由步驟 1 的推定表自動決定。插入位置用數值排序確定。

---

## 標準一鍵執行腳本

> AI 接到需求後直接以此模板生成並執行，**只需修改前 3 行**。

```python
import re, os

# ═══ 只需修改以下三行 ═══
CC_DEFINE  = "#define CC_ChenYuanXiang_CHINA   828 //西安 晨元翔"
BASE       = r"d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331"
DIST_SKILL = r"u:\共用區\客戶需求單\.github\skills\customer-code-distributor\SKILL.md"
# ════════════════════════

# --- 解析 ---
m = re.match(r'#define\s+(CC_\w+)\s+(\d+)\s*//(.+)', CC_DEFINE.strip())
CC_SYM, CC_NUM, COMMENT = m.group(1), int(m.group(2)), m.group(3).strip()
FUNC_SYM = "FUNC_" + CC_SYM

# --- 地區→代理商 ---
REGION_MAP = {
    **{k: "鴻勁興業" for k in ["china","suzhou","beijing","上海","shanghai","南京","nanjing",
        "西安","xian","shenzhen","深圳","guangdong","tianjin","wuhan","changsha",
        "ningbo","hefei","chengdu","zhengzhou","hangzhou","chongqing","中國","中国"]},
    **{k: "TeraTech"  for k in ["korea","韓國"]},
    **{k: "HTS"       for k in ["singapore","新加坡","malaysia","馬來西亞"]},
    **{k: "JB-Elite"  for k in ["philippines","菲律賓","thailand","泰國","thai"]},
    **{k: "Spandnix"  for k in ["japan","日本"]},
    **{k: "HPI-USA"   for k in ["us","usa","美國","california"]},
    **{k: "HPI-Euro"  for k in ["europe","歐洲","germany","德國","france","法國",
                                  "israel","以色列","malta","馬爾他"]},
    **{k: "HPI-TW"    for k in ["taiwan","台灣","tainan","台南","hsinchu","新竹"]},
}
comment_lower = COMMENT.lower().replace(" ", "")
DISTRIBUTOR = next((v for k, v in REGION_MAP.items() if k in comment_lower), "—")
REGION_EN   = COMMENT.split()[-1] if COMMENT else "—"

# --- dfm Unicode 轉義 ---
def dfm_escape(text):
    parts, buf = [], ""
    for ch in text:
        if ord(ch) < 128: buf += ch
        else:
            if buf: parts.append(f"'{buf}'"); buf = ""
            parts.append(f"#{ord(ch)}")
    if buf: parts.append(f"'{buf}'")
    return "".join(parts)

SYMBOL_SHORT = CC_SYM[3:]
_cp = COMMENT.split(None, 1)
_dfm_comment = f"{_cp[1]} {_cp[0]}" if len(_cp) == 2 else COMMENT
DFM_ENTRY = dfm_escape(f"{CC_NUM} {SYMBOL_SHORT:<20} {_dfm_comment}")

def edit_big5(fname, transform):
    with open(fname, "rb") as f: data = f.read()
    lines = data.decode("big5", errors="replace").splitlines(keepends=True)
    lines = transform(lines)
    with open(fname, "wb") as f: f.write("".join(lines).encode("big5", errors="xmlcharrefreplace"))

# ════ 1. MachineType.h ════
def patch_machinetype(lines):
    defines = [(i, int(m2.group(1))) for i, l in enumerate(lines)
               if (m2 := re.search(r'^#define\s+CC_\w+\s+(\d+)', l))]
    ins = max((i for i, n in defines if n < CC_NUM), default=0)
    lines.insert(ins + 1, f"#define {CC_SYM:<26} {CC_NUM} //{COMMENT}\r\n")
    return lines
edit_big5(os.path.join(BASE, "MachineType.h"), patch_machinetype)
print("MachineType.h OK")

# ════ 2. CosFunction.cpp - FUNC 定義 ════
def patch_cosfunc_func(lines):
    from pathlib import Path
    mth = Path(BASE, "MachineType.h").read_bytes().decode("big5", errors="replace")
    all_cc = [(int(m2.group(2)), m2.group(1)) for m2 in
              re.finditer(r'#define\s+(CC_\w+)\s+(\d+)', mth)]
    prev_sym = max(((n, s) for n, s in all_cc if n < CC_NUM), key=lambda x: x[0])[1]
    prev_func = "FUNC_" + prev_sym
    for i, l in enumerate(lines):
        if prev_func + "()" in l:
            for j in range(i, min(i+100, len(lines))):
                if lines[j].startswith("//---"):
                    new_block = (f"void {FUNC_SYM}()\r\n{{\r\n}}\r\n"
                                 f"//{'─'*78}\r\n")
                    lines.insert(j, new_block)
                    return lines
    return lines
edit_big5(os.path.join(BASE, "CosFunction.cpp"), patch_cosfunc_func)
print("CosFunction.cpp FUNC OK")

# ════ 3. CosFunction.cpp - case 分支 ════
def patch_cosfunc_case(lines):
    last_case = max(i for i, l in enumerate(lines) if re.match(r'\s+case CC_', l))
    case_line = f"        case {CC_SYM:<35} {FUNC_SYM}();{' '*19}break;  //{COMMENT}\r\n"
    lines.insert(last_case + 1, case_line)
    return lines
edit_big5(os.path.join(BASE, "CosFunction.cpp"), patch_cosfunc_case)
print("CosFunction.cpp case OK")

# ════ 4. HandlerSys.dfm ════
def patch_dfm(lines):
    entries = [(i, int(m2.group(1))) for i, l in enumerate(lines)
               if (m2 := re.match(r"\s+'(\d+) ", l))]
    ins = max((i for i, n in entries if n < CC_NUM), default=0)
    lines.insert(ins + 1, f"            {DFM_ENTRY}\r\n")
    return lines
edit_big5(os.path.join(BASE, "HandlerSys.dfm"), patch_dfm)
print("HandlerSys.dfm OK")

# ════ 5. customer-code-distributor SKILL ════
with open(DIST_SKILL, encoding="utf-8") as f: text = f.read()
lines_d = text.splitlines(keepends=True)
entries_d = [(i, int(m2.group(1))) for i, l in enumerate(lines_d)
             if (m2 := re.match(r'\| (\d+) \|', l))]
ins_d = max((i for i, n in entries_d if n < CC_NUM), default=0)
new_row = f"| {CC_NUM} | {SYMBOL_SHORT} | {REGION_EN} | {DISTRIBUTOR} |\n"
lines_d.insert(ins_d + 1, new_row)
with open(DIST_SKILL, "w", encoding="utf-8") as f: f.writelines(lines_d)
print("customer-code-distributor SKILL OK")

print(f"\n完成：{CC_SYM} = {CC_NUM}，代理商 = {DISTRIBUTOR}")
```

---

## 使用範例

### 範例 1：新增新加坡 AMD 客戶

**輸入**：`新增客戶代碼 #define CC_AMD_SG 807 //AMD 新加坡`

**MachineType.h**：
```cpp
#define CC_STM                  806 //意法半導體 馬爾他
#define CC_AMD_SG               807 //AMD 新加坡
#define CC_IMEC_TAIWAN          809 //台灣愛美科
```

**CosFunction.cpp**：
```cpp
void FUNC_CC_AMD_SG()
{
}
//------------------------------------------------------------------------------
```

**CosFunction.cpp case**：
```cpp
        case CC_AMD_SG:                     FUNC_CC_AMD_SG();                   break;  //AMD 新加坡
```

**HandlerSys.dfm**：
```
    '806 STM                  意法半導體 馬爾他'
    '807 AMD_SG                AMD 新加坡'
    '809 IMEC_TAIWAN          台灣愛美科'
```

### 範例 2：批量新增多個客戶

```
新增以下客戶代碼：
1. CC_AI_SG 1000 //AI 新加坡
2. CC_NPU_JP 1001 //NPU 日本
```
分別按步驟 2~6 執行。

---

## 特殊情況處理

| 情況 | 檢測 | 處理 |
|------|------|------|
| 客戶代碼已存在 | 掃描 MachineType.h 發現相同 `#define CC_` | 中止修改，提示已存在 |
| 名稱含特殊字元 | `/`、`\` 等 | 自動轉義或提示修正 |
| 版本資料夾不存在 | 找不到匹配的版本目錄 | 提示指定版本號 |
| 檔案編碼非 Big5 | 編碼偵測 | 自動轉換為 Big5（CP950） |

## 回滾與撤銷

使用 `@ht9045-customer-code-manager 撤銷 CC_AMD_SG` 可回滾：
1. 從 MachineType.h 刪除該客戶定義
2. 從 CosFunction.cpp 刪除函數 + case
3. 從 HandlerSys.dfm 移除 UI 選項
4. 從 Customer-Code-Distributor SKILL 刪除索引項

<!-- preserved-content:end -->
