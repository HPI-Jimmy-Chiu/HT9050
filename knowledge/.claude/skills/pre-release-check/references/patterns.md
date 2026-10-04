# Risk Pattern Reference — Detailed Examples

This file provides concrete code examples and regex search hints for each risk
pattern defined in SKILL.md. All examples are drawn from real HT9045 bugs.

---

## P1 — Bare Value in Chained Boolean Condition

### Bug Example (cOffSet.cpp, ReadFile)

```cpp
// WRONG — OutOfsAuto3 (enum value 4) is bare, always true
if(i==OutOfsAuto1 || i==OutOfsAuto2 || OutOfsAuto3 ||
   i==OutOfsFix1  || i==OutOfsFix2  || OutOfsFix3)

// CORRECT
if(i==OutOfsAuto1 || i==OutOfsAuto2 || i==OutOfsAuto3 ||
   i==OutOfsFix1  || i==OutOfsFix2  || i==OutOfsFix3)
```

### Detection Strategy

1. Find all lines containing `||` or `&&` with 3+ terms.
2. For each term, check whether it contains a comparison operator
   (`==`, `!=`, `<`, `>`, `<=`, `>=`).
3. Flag any term that is a bare identifier or literal without comparison.

### Regex Hint (grep)

```
\|\|\s*[A-Za-z_][A-Za-z0-9_]*\s*(\|\||&&|\)|$)
```

Matches `|| SomeIdentifier ||` or `|| SomeIdentifier)` — a bare identifier
between logical operators without a comparison.

---

## P2 — Enum Used as Boolean

### Bug Example (cOffSet.cpp, SaveSetupFile)

```cpp
// WRONG — InOfsBottom2DID is enum value 27, always true
if(InOfsBottom2DID)

// CORRECT
if(iSelPartData == OfsBottom2DID)
```

### Detection Strategy

1. Collect all enum member names from project headers.
2. Search for `if(EnumMember)` or `if(!EnumMember)` patterns.
3. Verify the enum member's value; non-zero values are always true.

### Regex Hint

```
if\s*\(\s*!?\s*[A-Z][A-Za-z0-9_]*\s*\)
```

Matches `if(SomeConstant)` — a single identifier in a condition without
comparison. Filter against known enum names for precision.

---

## P3 — Multi-Stage Loop Condition Change

### Bug Example (cOffSet.cpp, ReadFile)

The `for(int i=0; i<MaxOutOfs; i++)` loop processes all Output Arm stages.
A fix intended only for Auto1 inadvertently changed the `else if` branch
that controls PickUp/Place offset loading for Auto3 and Fix3, causing those
stages to lose their Z-offset values.

### Detection Strategy

1. Identify `for` / `while` loops iterating over stage/site indices
   (e.g., `i < MaxOutOfs`, `i < MaxInOfs`, `site < MaxSite`).
2. If any `if`/`else if` branch inside the loop was modified, list ALL
   branches and verify correctness for every iteration value.
3. Create a truth table: for each value of the loop variable, which branch
   is taken and what data is loaded/saved.

### Checklist

- [ ] Enumerate all possible loop index values
- [ ] For each value, trace which branch executes
- [ ] Confirm no branch is unreachable or always-true
- [ ] Confirm no branch accidentally captures unintended index values

---

## P4 — Save-Reload Path Integrity

### Bug Example (cOffSet.cpp)

Call chain: `spbSaveClick()` → `SaveFile()` → `SaveSetupFile()` → `ReadFile()`
→ `DoIniDataToForm()`.

The bug in `ReadFile()` was invisible during normal startup (data loaded
correctly from INI). It only manifested after Save, because Save triggered
a full reload that re-executed the buggy condition, overwriting valid data
with zeros.

### Detection Strategy

1. Find all `Save*()` functions.
2. Trace whether they call any `Read*()` / `Load*()` / `Reload*()` function.
3. If yes, audit the reload function with the same rigor as the save function.
4. Verify that every field written by Save is correctly restored by Read.

### Checklist

- [ ] Identify Save → Read call chain
- [ ] Verify Read handles all fields that Save writes
- [ ] Test the full Save-then-Read cycle, not just standalone Read

---

## P5 — Wrong Variable in Conditional (Copy-Paste Error)

### Bug Example (cOffSet.cpp, SaveSetupFile)

```cpp
// WRONG — uses bare enum name instead of comparison
if(InOfsBottom2DID)   // should be: if(iSelPartData == OfsBottom2DID)
```

This happened when code for Input Arm offset was copy-pasted to create the
Output Arm section, and the variable reference was not updated.

### Detection Strategy

1. In functions handling both Input and Output arms, search for `InOfs*`
   references in Output sections and `OutOfs*` in Input sections.
2. Verify that condition variables match the context (e.g., `iSelPartData`
   should be compared, not a raw enum constant).

### Regex Hint

```
// In Output Arm sections, flag any InOfs reference:
InOfs[A-Za-z0-9_]+

// In Input Arm sections, flag any OutOfs reference:
OutOfs[A-Za-z0-9_]+
```

---

## P6 — Division by Zero Risk

### Bug Example

```cpp
// WRONG — GearRatio could be 0, causing crash
LP = LP / GearRatio;

// CORRECT — safe template returns 0 when divisor is 0
LP = ChangeToFloatNonPcnt((double)(LP), (double)(GearRatio));
```

### Detection Strategy

1. Scan all `/` operators where the right operand is a variable **or a
   parenthesized expression** (not a non-zero literal constant like `2`,
   `100`, `1024`).
2. **Two forms must be scanned**：
   - **P6-a（裸變數除數）**：`expr / varName`，除數為單一變數
   - **P6-c（括號運算式除數）**：`expr / (sub-expr)`，除數為 `(A-B)` 或 `(A*B)` 等括號運算式。
     此形式中，括號內的子運算式結果可能為零（例如 `A==B` 時 `A-B==0`），
     但不會被 P6-a 的 regex 命中（`(` 不是 `[a-zA-Z_]`）。
3. For each candidate, check 20 lines of context for zero-guards:
   - `if(var != 0)` / `if(var > 0)` before the division
   - Ternary `(var==0) ? 0 : (a/var)`
   - Prior assignment to non-zero constant (`var = 1;`)
   - Already wrapped in `ChangeToFloat*` / `ChangeToPercentage`
   - For P6-c：`if(A==B) return` 或 `if(A-B==0)` 等等效 guard
4. Flag unprotected divisions. Note that `if(A>0 || B>0)` does NOT
   fully protect `/A` or `/B` (one may still be 0).
5. Compile-time constants (e.g., `MAX_SOCKET_ROW`) can be skipped.

### Regex Hint

**P6-a（裸變數除數）**：
```
(?<=[)\]\w])\s*/\s*(?![*/=])([a-zA-Z_]\w*)
```

Matches a `/` preceded by `)`, `]`, or a word character, followed by a
variable name (excluding `//` comments, `/*` block comments, and `/=`
compound assignment).

**P6-c（括號運算式除數）**：
```
(?<=[)\]\w])\s*/\s*\(
```

Matches a `/` followed by `(`（開括號）。命中後需以括號平衡掃描取得完整除數
`(sub-expr)`，再判斷 sub-expr 是否可能為零。常見高風險模式：
- `/(A-B)` — 當 `A==B` 時為零
- `/(A*B)` — 當任一為零時為零
- `/(fMax-fMin)` — 校正參數相等時為零

### Safe Division Template

```cpp
// Defined in MachineType.h
template <class T>
float ChangeToFloatNonPcnt(const T Numerator, const T Denominator)
{
    double str=0.00;
    if(Denominator!=0)
       str= ((double)Numerator/(double)Denominator);
    return str;
};
```

### Full Pipeline

For batch scanning and automated replacement, see
[division-safety.md](division-safety.md) which provides a complete 4-stage
PowerShell workflow: Search → Filter → Replace → Verify.

---

## P6b — Type Mismatch After ChangeToFloatNonPcnt (UB)

`ChangeToFloatNonPcnt` returns `float`. When replacing **integer division**
with this template, if the result is used in a `printf`/`sprintf` variadic
context with `%c`, `%d`, or character arithmetic (`'A' + result`), the
`float` argument is promoted to `double` (8 bytes) at the call site, but
the format specifier reads 4 bytes — causing undefined behaviour that
typically produces `\0` (empty string).

### Bug Example

```cpp
// Original integer division (OK: int → %c)
str.sprintf("%c", 'A' + (ARow-1)/TestSocket.iShtCol);

// After P6 replacement (BROKEN: float promoted to double → %c reads wrong bytes)
str.sprintf("%c", 'A' + ChangeToFloatNonPcnt((double)((ARow-1)), (double)(TestSocket.iShtCol)));

// Fixed (cast back to int)
str.sprintf("%c", 'A' + (int)(ChangeToFloatNonPcnt((double)((ARow-1)), (double)(TestSocket.iShtCol))));
```

### When to Apply

After any P6 replacement, check if the result is used in:
1. `sprintf` / `AnsiString::sprintf` with `%c`, `%d`, `%x`, `%o`, `%u`
2. Character arithmetic: `'A' + result`, `'a' + result`
3. Assignment to `int` / `char` variable

If yes, wrap in `(int)(...)`.

---

## P7 — LastSet Array Out-of-Bounds

See [lastset-array-audit.md](lastset-array-audit.md) for the complete
reference: full array boundary table, common index sources, historical
defect cases, and grep scripts.

**Quick summary**: `LAST_GENERAL_SET` (LastSet) is persisted as raw binary.
Variable-indexed access to its arrays without bounds checking causes silent
memory corruption. Key audit points: `||`/`&&` direction in guard
conditions, off-by-one (`j+1`/`X-1`), and negative indices from tester or
UI (e.g. `-1`).

**Fix**: Add `if(idx >= 0 && idx < SIZE)` guard; do NOT reorder struct
members or change existing field sizes (binary layout compatibility).

---

## P8 — Index-Map Bypass (Row Swap)

See [index-map-bypass.md](index-map-bypass.md) for the complete reference.

**Quick summary**: 索引映射陣列（如 `iOCRMap`）若在某些路徑上被 bypass
（直接用硬編碼 row 而未經 mapping），會造成 row swap、重複碼殘留、
2DID 誤判。

**Fix**: 凡需要 site→row 的存取，全部統一經由 mapping helper；禁止
直接以硬編碼 site index 存取 `cDeviceInf`、`iBarCode1_*` 等資料結構。

---

## P9 — Folder Existence Before File Write

See [folder-ensure-before-write.md](folder-ensure-before-write.md) for the
complete reference: full risk API list, helper usage, ctor / FormCreate
audit, and fix examples.

**Quick summary**: 在呼叫 `fopen(...,"w/a/wb/ab/w+/a+")`、`SaveToFile`、
`CopyFile`（target side）、`CreateFile(CREATE_*)`、`TFileStream(...,fmCreate)`、
`WritePrivateProfileString` **之前**，必須先確認目標資料夾存在。
否則路徑深層第一次使用時會擲出例外或寫檔失敗。

**Fix**：寫檔前呼叫下列任一 helper（任選其一即可，不需重複）：

```cpp
// HT9045：一般情況推薦（有例外彈窗 + LOG 記錄）
MyForceDirectories(asPath, "TMyClass::FunctionName");

// HT9045：已在 catch 內或不需要彈窗通知時：
FileInfo().EnsureDirectoriesExist(asPath);

// 其他專案（GPIB9045 / RS232Standard 等無上述 helper）：
if(!DirectoryExists(sFolder))
    ForceDirectories(sFolder);

// 若明確是目錄路徑，加結尾 '\\' 可跨過內部 GetFileAttributes 查詢
MyForceDirectories(asDirPath + "\\", "TMyClass::FunctionName");
```

**特別注意**：建構子（ctor）與 FormCreate 中的寫檔，會在 main.cpp 的
`FormShow` 全域 `MyForceDirectories` 區塊**之前**執行 → 必為高風險。

**Skip 條件**：若呼叫點上游已被 helper 包覆（同函式內、同分支可達），
則不需重複防護。

**Detection regex**：
- `fopen\s*\([^,]+,\s*"[wa]` （新建/附加模式）
- `SaveToFile\s*\(`
- `CopyFile\s*\([^,]+,\s*` （第二個參數是 destination）
- `TFileStream\s*\([^,]+,\s*fmCreate` 
- `WritePrivateProfileString` （ini 寫入新路徑）

---

## P10 — vector<T*> clear() Without delete (Memory Leak on Reload)

### Description

`vector<T*>` 存放 `new` 出來的指標，呼叫 `.clear()` 只移除 vector 內的指標元素，
**不會呼叫 destructor / delete 所指向的物件**。
若該函式可被 UI 按鈕重複觸發（Reload 類按鈕），每次執行均洩漏上一批物件。

### Bug Example (database.cpp, LoadMotData / LoadIoData)

```cpp
// WRONG — TMOTDATA* / TIODATA* objects are leaked on every Reload click
mapMotTable.clear();
MotTable.clear();                           // only removes pointers, not objects
...
MotTable.push_back(new TMOTDATA(...));      // allocates new objects each time

// CORRECT
mapMotTable.clear();
for(int i=0; i<(int)MotTable.size(); i++)  // Steven 20260612 : Fix memory leak - delete TMOTDATA* before clear()
    delete MotTable[i];
MotTable.clear();
```

### Detection Strategy

1. 找出所有 `vector<T*>` 宣告（`vector\s*<\s*[A-Z][A-Za-z]+\s*\*`）。
2. 追蹤該 vector 是否有 `.push_back(new ...)` 呼叫。
3. 找出所有 `.clear()` 呼叫點，確認**緊接在 `.clear()` 之前是否有 delete 迴圈**。
4. 若無 delete 迴圈，且 `.clear()` 所在函式可被 Reload 類按鈕重複呼叫 → **P10 命中**。

### Regex Hints

```regex
// Step 1: find vector<T*> declarations
vector\s*<\s*[A-Z][A-Za-z0-9_]+\s*\*

// Step 2: find push_back(new ...) in same file
push_back\s*\(\s*new\s+

// Step 3: find .clear() without preceding delete loop
\.clear\(\)
```

### High-Risk Call Sites

| 函式 | 觸發來源 | 風險 |
|------|----------|------|
| `LoadMotData()` | `btnReloadMotorDataClick` (uMotorTest.cpp) | 每次 Reload Motor 按鈕洩漏全部 `TMOTDATA*` |
| `LoadIoData()` | `btnReloadClick` (iosetview.cpp) | 每次 Reload IO 按鈕洩漏全部 `TIODATA*` |

### Fix Template

```cpp
// Before calling .clear() on a vector<T*>
for(int i=0; i<(int)VecName.size(); i++)
    delete VecName[i];
VecName.clear();
```

### Safe Patterns (no fix needed)

- `vector<T*>` 只在 **constructor / FormCreate** 填入一次，沒有 `.clear()` + 重填路徑 → 無累積洩漏。
- `.clear()` 只在 **FormDestroy / 程式結束** 呼叫 → OS 回收，影響極低（可選修）。
- `vector<int>` / `vector<AnsiString>` 等非指標型別 → `.clear()` 完全安全。

### Case History

- **HT9045 V3.33.905.8 (2026-06-12)**：`database.cpp` `LoadMotData()` 與 `LoadIoData()` 均發現此問題，修正後加上 `Steven 20260612` 註解。

---

## P10b — Repeated `new` Without Null Guard in Re-entrant `Initial*`/`Load*` Functions

### Description

可重入的初始化函式（`Initial*`、`Load*`、`Reload*`）在**每次呼叫時對成員指標無條件 `new`**，
舊指標被覆寫但未 `delete` → 每次 Reload / 換 Lot 觸發均洩漏整批物件。
與 P10 不同：此型不透過 `vector`，而是對**單一指標或結構陣列內嵌指標**直接 `= new`。

### Subtype A — 成員指標無條件重建（PordRec 案例）

```cpp
// WRONG — old TMyProductionRecord* is leaked every time InitialMotorParameter() is called
MOT[MMTrayY].Tray.PordRec[x][y] = new TMyProductionRecord();   // no null check, no delete

// CORRECT — allocate only once; keep existing object on re-entry
if(MOT[MMTrayY].Tray.PordRec[x][y] == NULL)
    MOT[MMTrayY].Tray.PordRec[x][y] = new TMyProductionRecord();
```

> ⚠️ 本例特別選擇「NULL guard」而非「先 delete 再 new」：`PordRec` 可在 Lot 進行中被 Reload，
> MainProc thread 隨時可能 dereference 指標；UI thread 進行 delete 會造成跨 thread 懸空。
> 若確認無跨 thread 存取風險，亦可先 delete 再重建。

### Subtype B — 建構子當語句（Constructor-as-Statement）

呼叫建構子但未接收回傳值，只會產生一個**匿名暫時物件**立刻銷毀——`this` 的成員完全未被重新初始化，且暫時物件內 `new` 出的資源立刻洩漏。

```cpp
// WRONG — creates a temporary object, destroys it immediately; this->asBuffer is still NULL
DeleteProductionRecord();
TMyProductionRecord();          // ← 暫時物件，對 this 無任何效果

// CORRECT — directly rebuild the member
DeleteProductionRecord();
asBuffer = new TStringList();
for(int i = eScheduleName; i < eDataTotal; i++)
    asBuffer->Add("");
```

### Detection Strategy

1. 掃描可重入函式（名稱含 `Initial`、`Load`、`Reload`、`Reset`）內出現的 `= new`。
2. 確認該 `new` 前**沒有對同一指標的 `delete` 或 `if(ptr==NULL)` guard**。
3. 掃描「獨立成行的建構子呼叫」（`^\s+[A-Z][A-Za-z0-9_]+\(\);` 且行首非 `new`）→ P10b Subtype B 命中。

### Regex Hints

```regex
// Subtype A: unconditional new in Initial*/Load*/Reload* functions
=\s*new\s+[A-Z][A-Za-z0-9_]+\s*\(

// Subtype B: constructor-as-statement (no `new`, no assignment)
^\s+[A-Z][A-Za-z0-9_]+\s*\(\s*\)\s*;
```

### Case History

- **HT9045 V3.33.905.8+ (2026-05-26)**：`cinitial.cpp` `InitialMotorParameter()` 對 18 個 tray 馬達 × `_MAX_X_ITEM` × `_MAX_Y_ITEM` 個 `PordRec` 無條件 `new`，每次 Reload Motor 洩漏 ~130 MB（Subtype A）。
- **HT9045 V3.33.905.8+ (2026-05-26)**：`Public/MyProductionRecord.cpp` `InitialRecord()` 呼叫 `TMyProductionRecord();` 產生暫時物件，`this->asBuffer` 仍是 NULL，後續 dereference 當機（Subtype B）。
- 完整 MEM Sample 追查流程與 EventLog 對照，見 [memory-leak-runtime-debug.md](memory-leak-runtime-debug.md)。

---

## P11 — `==` Used Instead of `=` (Comparison with No Side Effect)

### Description

在陳述式位置（statement context）寫了 `var == value;` 而非 `var = value;`，
編譯器只會產生 **Warning: expression has no effect**，不會報錯，變數實際未被賦值。
這是 typo / copy-paste 引入的 silent bug，在 bool / int / enum 型別的 flag 重置最常見。

### Bug Example (csystem.cpp, OneCycle)

```cpp
// WRONG — bBackupCleanOut is never cleared; ret=K_CLEAN_OUT fires every cycle
if(bBackupCleanOut==true)
{
    bBackupCleanOut==false;   // ← comparison, no assignment
    ret=K_CLEAN_OUT;
}

// CORRECT
if(bBackupCleanOut==true)
{
    bBackupCleanOut=false;    //Steven 20260612 : Fix == to = (was comparison, not assignment)
    ret=K_CLEAN_OUT;
}
```

### Detection Strategy

用 regex 掃描「非 `if` / `while` / `for` / `return` 開頭的陳述式，包含 `==` 且以 `;` 結尾」：

```regex
^\s+\w[\w.->]*\s*==\s*\w[\w.->]*\s*;
```

符合後須人工確認不是 `if(...) expr==val;` 的格式（理論上 regex 已過濾，但仍需驗證行首）。

### 強制 False Positive 過濾（B）— 原始命中誤報率約 82%

> 全專案掃描時，純 regex 命中極易誤報（多行 `if()` 條件續行、三元運算）。
> V3.33.906.0 全量掃描原始命中 **94 筆**，套用下列 4 道過濾後降為 **17 筆**（全部為真實 bug）。
> 自動化掃描器**必須**內建這 4 道過濾，否則不可信賴：

1. **括號平衡**：若該行 `)` 數量 > `(` 數量 → 是多行條件的續行（如 `if(... ==` 換行後的下半段），跳過。
2. **三元排除**：若該行同時含 `?` 與 `:` → 為三元運算式，非賦值語句，跳過。
3. **lvalue 純淨**：若 `==` 左側 token 含 `(` → 是函式呼叫片段，跳過。
4. **前一行續行**：往上找第一個非空程式碼行，若其結尾為 `| & ( , ? : + - * / < > =` 任一 → 本行屬多行運算式中段，跳過。

對應實作見 `scripts/scan_and_report_pre_release.py` 的 `find_p11()`。

### 嚴重度啟發式（G）— 不要一律標 Critical

> P11 的嚴重度取決於 lvalue 用途，**不是一律 Critical**：

| lvalue 特徵 | 嚴重度 | 理由 |
|-------------|--------|------|
| 控制流變數（`ret`/`result`）、`->Active`、`->Port` | **Critical** | 控制流斷裂、通訊行為異常 |
| 一般 flag / 狀態 / 資料持久化欄位 | **High** | 功能靜默失效（如自動關站、校正未存） |
| 純 UI 顯示（`->Checked`/`->Visible`/`->Caption`/`->Text`/`->ActivePageIndex`/`->Color`/`->ItemIndex`） | **Low** | 僅畫面顯示，不影響流程 |

對應實作見 `find_p11()` 的 `_p11_severity()`。

### False Positive 排除

- `if(a==b)` — 在 `if` 條件中，正常比較，不是 bug。
- `assert(a==b);` — 斷言語句，正常。
- `while(a==b)` — 迴圈條件，正常。
- `#ifdef` / `#if` 中的比較 — 正常。

### Fix Template

```cpp
// 直接將 == 改為 =，並加上修正備注
var=value;  //Steven YYYYMMDD : Fix == to = (was comparison, not assignment)
```

### Case History (HT9045 V3.33.905.8, 2026-06-12)

| # | 檔案 | 行號 | 程式碼 | 嚴重度 |
|---|------|------|--------|--------|
| 1 | aoutarm9045.cpp | 2290 | `OutArmSuck.bAlreadyRotate==true;` | Critical |
| 2 | aTester_Front.cpp | 2380 | `bAlreadyTested==false;` | Critical |
| 3 | aTester_Rear.cpp | 2409 | `bAlreadyTested==false;` | Critical |
| 4 | cinitial.cpp | 8132 | `iLoadData==2;` | Critical |
| 5 | cinitial.cpp | 11633 | `TestIF.iAutoClean_Function==false;` | High |
| 6 | cShowBinSelect.cpp | 1388 | `bUpdateBinDigital==false;` | High |
| 7 | csystem.cpp | 13136 | `ret==K_RETRY;` | Critical |
| 8 | csystem.cpp | 13777 | `bBackupCleanOut==false;` | Critical |
| 9 | csystem.cpp | 14188 | `LastSet.bFirstTestAutoRetestGPIB==false;` | Critical |
| 10 | main.cpp | 16882 | `bTesterLowYieldOneCycle==false;` | High |
| 11 | uhome.cpp | 4003 | `flag1==true;` (in `#ifdef DEBUG_HOME`) | Low |

### Case History (HT9045 V3.33.905.8 r906, 2026-06-18)

| # | 檔案 | 行號 | 程式碼 | 嚴重度 |
|---|------|------|--------|--------|
| 1 | csystem.cpp | — | OCR LotID error handler 3 處 `==` → `=` | Critical |
| 2 | AutoClean/uCleaning.cpp | — | `iAutoClean_Function==false;` → `=false;` | High |
| 3–7 | KYECFTP/FTPClient.cpp | — | `bCanExit==true;` / `bCanExit==false;` 共 5 處 → `=true/false` | High |

> ⚠️ **FTPClient.cpp `bCanExit` 歷史殘留提示**（避免下次重複掃描）：
> `KYECFTP/FTPClient.cpp` 中的 `bCanExit==true/false` 共 5 處已於 r906（2026-06-18）修正。
> 此檔為 Big5 CP950 編碼，修正時必須使用 **Python binary 模式**，
> 禁止使用 `replace_string_in_file`（會以 UTF-8 bytes 寫入造成亂碼，見 user memory `big5-replace-string-utf8-bug.md`）。

### Case History（HT9045 V3.33.906.0 全專案全量掃描，2026-06-18）

> 全量掃描（339 src + 368 hdr）原始命中 94 筆 → 套用 B 的 4 道 FP 過濾 → 17 筆，逐筆讀上下文確認 17/17 皆真實 bug。

| # | 檔案 | 行 | 程式碼 | 嚴重度 |
|---|------|----|--------|--------|
| 1 | csystem.cpp | 18646 | `ret==ShowErrorMessage("WAR0955", ...);` | Critical |
| 2 | Command.cpp | 12626 | `TCPCommandServer->Active==false;` | Critical |
| 3–4 | AutoAlignment/cAutoAlignment.cpp | 612–613 | `dOutBottomPixelSizeX/Y==atof(...);` | High |
| 5–7 | uYieldMonitoring.cpp | 5249/5287/5288 | `bLowYieldCloseSite[...]==true;` | High |
| 8–10 | acatchtray.cpp | 8403/8444/8512 | `asTrayIDDataCorverLoader=="NOREAD";` | High |
| 11 | HS_Function.cpp | 446 | `RTMServerSocket->Port==IniConfig.iN24_RTMPort;` | High |
| 12 | cTrayAssignment.cpp | 178 | `bTrayUpDownSet[eFix3]==false;` | Medium |
| 13 | OCRInsp.cpp | 408 | `bOcr_ReceiveOK[...]==false;` | Medium |
| 14 | cTemperFrom.cpp | 1600 | `strShowYield[i].bFlag==true;` | Low |
| 15 | cObserver.cpp | 2897 | `sTestIndexZTime=="";` | Low |
| 16 | cSetUp.cpp | 2118 | `cbEnableRealTimeCCD->Checked==...;` | Low (UI) |
| 17 | cShowBinSelect.cpp | 272 | `PageControl1->ActivePageIndex==3;` | Low (UI) |

> ⚠️ #1 `csystem.cpp` WAR0955 與 r906 已修的「OCR LotID 3 處」**不同分支，當時被遺漏**。
> 全部 17 處於 906 以 Python binary 模式修正（Big5 安全），**禁止用 `replace_string_in_file` 寫入中文**。
> ⛔ 20261003 V906 C++ 移植樹（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`）的對應狀態（AI(W906-E034) 20261003，todo E-034＝筆電卡 S-21，St01；Steven 1003 14:5x「Q82. A」＝#20 例外，Steven 1003 常設規則）：上表是 0625 修的那一批；移植樹抄的是 0618，所以這些 `==` 原本都還在。E-034 在筆電的移植檔改成 `=`（同一行改、行數不變）：#1 `csystem.cpp:32417`（在 `#if 0` OCRSTART 閘裡，閘打開前沒有執行期差異）、#5–7 `uYieldMonitoring.cpp:2242`／`:2286`／`:2287`、#8–10 `acatchtray.cpp:8568`／`:8609`／`:8677`、#12 `forms\fTrayAssignment.cpp:235`、#13 `OCRInsp.cpp:819`、#14 `cTemperFrom.cpp:1302`、#15 `cObserver.cpp:2540`；#17 `cShowBinSelect.cpp:2315` 是 St02 C14 已改（`8db5c2c9`）；#16 cSetUp 在 St01 的 `FileRW\TestIF_File_SetUp.gen.inc`（E-032 保留 `=`）；#2 Command.cpp 是 St02 W10；#3–4 cAutoAlignment DoUIToData、#11 HS_Function TimerAutoBackupTimer 沒移植。測試 ctest `E034_NoopEq`、`YieldMonCore`、`ObserverCore`、`TemperFromCore`。

---

## P12 — `malloc`/`delete` Mismatch (C/C++ Allocator Mismatch, Undefined Behavior)

### Description

C 標準庫的 `malloc()` / `calloc()` / `realloc()` 分配的記憶體，**必須用 `free()` 釋放**，
不得使用 C++ 的 `delete` 或 `delete[]`。混用是 **Undefined Behavior**，在 BCB6/MSVC 的 debug heap 下通常不立即 crash，但行為未定義，不可依賴。

同理：
- `new T` → 必須 `delete` 釋放（不得 `free()`）
- `new T[n]` → 必須 `delete[]` 釋放（不得 `delete` 或 `free()`）

### Bug Example (uLotInfo.cpp, WhenTestRecordTemperatureLog_3Sigma)

```cpp
// WRONG — malloc paired with delete → undefined behavior
dListTemperatureData = (double*)malloc(sizeof(double) * iListTemperatureData);
// ... use dListTemperatureData ...
if(dListTemperatureData != NULL)
{
    delete dListTemperatureData;    // ← UB! malloc must be freed with free()
    dListTemperatureData = NULL;
}

// CORRECT
if(dListTemperatureData != NULL)
{
    free(dListTemperatureData);     //Steven 20260612 : Fix malloc/delete mismatch - use free() to match malloc()
    dListTemperatureData = NULL;
}
```

### Detection Strategy

靜態掃描無法 100% 精準（跨函式、跨作用域誤判多），建議：

1. 在同一函式內找 `varname = (cast*)malloc(...)` 與 `delete varname;` 同時出現 → 確認命中。
2. 掃描 `new TYPE[` 配合 `delete varname;`（無 `[]`）在同一函式內 → 確認命中。
3. **注意跨函式誤判**：掃描器可能將不同函式中同名局部變數（如 `P`）誤配對，需人工確認同作用域。

### Detection Script (Python snippet)

```python
import re

def find_malloc_delete_mismatch(filepath):
    with open(filepath, 'rb') as f:
        lines = f.read().decode('cp950', errors='replace').split('\n')
    # Track malloc vars within a function (naive, same-function heuristic)
    depth = 0
    func_vars = {}
    for i, line in enumerate(lines):
        depth += line.count('{') - line.count('}')
        if depth == 1:
            func_vars = {}  # new function scope
        m = re.search(r'(\w+)\s*=\s*\([^)]+\)\s*malloc\s*\(', line)
        if m:
            func_vars[m.group(1)] = i+1
        for var, src_line in func_vars.items():
            if re.search(r'\bdelete\s*\[?\]?\s*' + re.escape(var) + r'\s*;', line.strip()):
                print(f'{filepath}:{i+1}: delete {var} (malloc at line {src_line})')
```

### Regex Hints

```regex
// Find malloc assignment
(\w+)\s*=\s*\([^)]+\)\s*malloc\s*\(

// Find delete (not free) in same scope
\bdelete\s*(\[\])?\s*<varname>\s*;

// Find new[] + delete (without [])
(\w+)\s*=\s*new\s+\w[\w:]*\s*\[
\bdelete\s+<varname>\s*;
```

### False Positives

掃描器常見的**跨作用域誤判**情形（不是真正問題）：
- 不同函式中恰好同名的局部變數（如 `P`, `buff`），掃描器跨函式配對 → 手動確認同函式作用域
- 注解掉的 `//delete buff;` 已有正確的 `delete[] buff;` 緊接其後 → 實際已修正

### Fix Template

```cpp
// Option A: malloc 系列 → 用 free()
free(ptr);          //Steven YYYYMMDD : Fix malloc/delete mismatch - use free() to match malloc()
ptr = NULL;

// Option B: 改用 new[]/delete[] 一致
ptr = new double[n];
// ...
delete[] ptr;       //Steven YYYYMMDD : Fix delete→delete[] for dynamic array
ptr = NULL;
```

### Case History

| 版本 | 檔案 | 行號 | 說明 |
|------|------|------|------|
| HT9045 V3.33.905.8 (2026-06-12) | `uLotInfo.cpp` | L12238 (malloc) / L12283 (delete) | `WhenTestRecordTemperatureLog_3Sigma` 中 `dListTemperatureData` 用 `malloc` 分配、`delete` 釋放；修正為 `free()` |

---

## P13 — block-memory size 比對錯誤（strcpy / strncpy / memcpy size vs 目標大小）

### Description

`strcpy`、`strncpy`、`memcpy`、`memmove`、`memset`、`ZeroMemory`、`sprintf` 等 block-memory 操作，
若 **size 引數 > 目標緩衝宣告大小**，或 `strncpy` 填滿後未補結尾 `\0`，將造成寫越界或 read-past-end。

常見三種陷阱：

| 陷阱 | 說明 | 風險 |
|------|------|------|
| (a) 以來源長度為 N | `strncpy(dst, src, src.Length())` 而非 `sizeof(dst)-1` | 來源超出目標時寫越界 |
| (b) strncpy 填滿無結尾 `\0` | `strncpy(dst, src, N)` 且 `src.Length()>=N` → dst 無 null terminator | 後續以字串使用時 read-past-end |
| (c) 跨結構 memcpy 大小不一致 | `memcpy(Prod.X, Tech.Y, sizeof(Tech.Y))` 但 Prod.X 比 Tech.Y 小 | 寫越界破壞相鄰成員 |

**注意**：VCL `AnsiString::sprintf(...)` 為成員方法（自管緩衝），**不列入本 Pattern 範圍**。

### Bug Examples

```cpp
// WRONG (a) — 以來源長度為 N，而非目標大小
strncpy(HHandler2Gpib.Message, Str.c_str(), Str.Length());   // Message[2048]，Str 可能 > 2048

// CORRECT (a)
strncpy(HHandler2Gpib.Message, Str.c_str(), sizeof(HHandler2Gpib.Message)-1);
HHandler2Gpib.Message[sizeof(HHandler2Gpib.Message)-1] = 0;

// WRONG (b) — strncpy 填滿時無結尾 null
strncpy(buf, src, sizeof(buf));   // src.length() >= sizeof(buf) → no null

// CORRECT (b)
strncpy(buf, src, sizeof(buf));
buf[sizeof(buf)-1] = 0;           //Steven YYYYMMDD : ensure null-termination

// WRONG (c) — 目標 [2][4]，迴圈走 j<8（Tech [2][8]）→ 等同 memcpy 超界
for(j=0; j<8; j++) Prod.X[i][j] = Tech.X[i][j];  // Prod.X[2][4] 只有 4 欄

// CORRECT (c)
for(j=0; j<MAX_ARM_Col; j++) Prod.X[i][j] = Tech.X[i][j];  // j<4，與 Prod 對齊
```

### Detection Regex Hints

```regex
// (a) strncpy 以來源 Length() 為上限
strncpy\s*\([^,]+,[^,]+,\s*\w+\.Length\s*\(\)

// (b) strncpy 填滿（size == sizeof(dst)，需補結尾 null）
strncpy\s*\([^,]+,[^,]+,\s*sizeof\s*\([^)]+\)\s*\)

// (c) 裸 strcpy（完全不設限）
\bstrcpy\s*\(

// memcpy 跨結構（size 來自不同型別 sizeof）
memcpy\s*\([^,]+,[^,]+,\s*sizeof\s*\([^)]+\)\s*\)
```

### Fix Template

```cpp
// strncpy 標準修法：限制目標大小，補結尾 null
strncpy(dst, src.c_str(), sizeof(dst)-1);
dst[sizeof(dst)-1] = 0;               //Steven YYYYMMDD : P13 ensure null-termination

// strcpy 改為 strncpy
strncpy(dst, src, sizeof(dst)-1);     //Steven YYYYMMDD : P13 strcpy → strncpy
dst[sizeof(dst)-1] = 0;
```

### Case History

| 版本 | 嚴重度 | 檔案 | 行號 | 說明 |
|------|--------|------|------|------|
| HT9045 V3.33.905.8 (2026-06-16) | Medium | `Command.cpp` | L10491 | `strncpy(Message, Str, Str.Length())`：Message[2048]，以來源長度為上限，Str 可能超界 |
| HT9045 V3.33.905.8 (2026-06-16) | Medium | `cShowBinSelect.cpp` | L2336 | `strcpy(LastSet.szJamClearData[0], ...)` 無上限，目標 char[64]，LastSet 持久化成員 |
| HT9045 V3.33.905.8 (2026-06-16) | Low | `BarCode/BarCode.cpp` | L8590 | `strncpy(..., sizeof(24))` 正確限制大小，但 2D ID ≥ 24 字元時缺結尾 `\0` → read-past-end |

