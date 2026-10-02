# 代碼格式化標準（Code Formatting Standards）

## 概述

本文檔定義了 HT9045 專案在代碼格式化方面的標準與規範，涵蓋空白行管理、控制結構樣式、字符編碼等方面。
這些標準應在每次 commit 或 release 前進行檢查。

---

## 格式化規則（F1–F12）

### F1 — Tab 符號轉為空白格

- **規則**：所有行內的 Tab 字符（`\t`）應統一轉換為空白格。
- **說明**：Tab 符號在不同編輯器中寬度不一，使用空白格可確保程式碼在所有環境下的視覺一致性。
- **標準設置**：
  - HT9045 專案推薦使用 **4 個空白格**代替一個 Tab。
  - 所有 `.cpp` 和 `.h` 檔案應統一採用此標準。

- **修正方式**：
  - **全文件轉換**：使用編輯器的"Replace All"功能，將 `\t` 替換為 4 個空白格。
  - **Python 腳本示例**：
    ```python
    with open(filename, encoding='cp950') as f:
        content = f.read()
    content = content.replace('\t', '    ')  # 4個空白
    with open(filename, 'w', encoding='cp950') as f:
        f.write(content)
    ```

---

### F2 — 行尾空白刪除

- **規則**：每行末尾不應包含空白字符（空白格或 Tab）。
- **說明**：行尾空白會造成不必要的版本控制差異，降低 git diff 可讀性，且可能在某些編輯器中造成視覺問題。
- **檢測方式**：
  - 使用正則運算式 `\s+$` 掃描並定位行尾空白。
  - VS Code 設定 `"files.trimTrailingWhitespace": true` 可自動移除。

- **修正方式**：
  - **全文件清理**：使用編輯器的"Replace All"功能，將正則 `\s+$` 替換為空字符。
  - **Python 腳本示例**：
    ```python
    import re
    with open(filename, encoding='cp950') as f:
        lines = f.readlines()
    lines = [re.sub(r'\s+$', '', line) + '\n' if line.strip() else '\n' 
             for line in lines]
    with open(filename, 'w', encoding='cp950') as f:
        f.writelines(lines)
    ```

---

### F3 — 連續空白行的合併

- **規則**：若相鄰兩行及以上均為空白行，應只保留一行空白行。
- **說明**：過多的連續空白行會浪費垂直空間，降低代碼視覺效率。
- **範例**：

  ❌ **不良**（3 行空白行）：
  ```cpp
  void FunctionA()
  {
      // ...
  }



  void FunctionB()
  {
      // ...
  }
  ```

  ✅ **正確**（1 行空白行）：
  ```cpp
  void FunctionA()
  {
      // ...
  }

  void FunctionB()
  {
      // ...
  }
  ```

- **修正方式**：
  - **正則替換**：使用 `\n\s*\n\s*\n` 替換為 `\n\n`（保留最多一個空白行）。
  - **Python 腳本示例**：
    ```python
    import re
    with open(filename, encoding='cp950') as f:
        content = f.read()
    # 將 2 個以上連續空白行合併為 1 個
    content = re.sub(r'\n\s*\n(\s*\n)+', '\n\n', content)
    with open(filename, 'w', encoding='cp950') as f:
        f.write(content)
    ```

---

### F4 — if/else if/else 括號一致性

- **規則**：在同一個 if-else 鏈中，如果其中某一個分支使用 `{}`，則所有分支都應使用 `{}`；反之亦然，都應為單行陳述式。
- **說明**：不一致的括號使用會導致代碼易讀性降低，且容易引入維護時的邏輯錯誤。
- **範例**：

  ❌ **不良**（不一致）：
  ```cpp
  if( iStatus == 0 )
      DoThis();
  else if( iStatus == 1 )
  {
      DoThat();
      DoMore();
  }
  else
      DoOther();
  ```

  ✅ **正確**（全部有括號）：
  ```cpp
  if( iStatus == 0 )
  {
      DoThis();
  }
  else if( iStatus == 1 )
  {
      DoThat();
      DoMore();
  }
  else
  {
      DoOther();
  }
  ```

  ✅ **也正確**（全部無括號）：
  ```cpp
  if( iStatus == 0 )
      DoThis();
  else if( iStatus == 1 )
      DoThat();
  else
      DoOther();
  ```

- **修正方式**：
  - 檢查整個 if-else 鏈，確認是否所有分支都使用 `{}`。
  - 若不一致，統一改為全部添加 `{}` 或全部移除 `{}`（推薦添加以提高安全性）。

---

### F5 — if 判斷與開括號之間的空白行

- **規則**：if 判斷式與下方開括號 `{` 之間不應包含空白行。
- **說明**：if 條件與其對應的程式碼區塊應保持視覺上的緊鄰。
- **範例**：

  ❌ **不良**：
  ```cpp
  if( iStatus == 0 )

  {
      DoSomething();
  }
  ```

  ✅ **正確**：
  ```cpp
  if( iStatus == 0 )
  {
      DoSomething();
  }
  ```

- **修正方式**：移除 if 最後一行與 `{` 之間的所有空白行。

---

### F6 — 開括號之後的空白行

- **規則**：`{` 符號之後不應包含空白行。
- **說明**：開括號後立即加入空白行造成浪費，降低代碼密度。
- **範例**：

  ❌ **不良**：
  ```cpp
  {

      int i = 0;
      // ...
  }
  ```

  ✅ **正確**：
  ```cpp
  {
      int i = 0;
      // ...
  }
  ```

- **修正方式**：移除 `{` 之後的所有空白行。

---

### F7 — 閉括號之前的空白行

- **規則**：`}` 符號之前不應包含空白行。
- **說明**：閉括號前的空白行會增加不必要的行數，影響整體代碼視覺效果。
- **範例**：

  ❌ **不良**：
  ```cpp
  {
      int i = 0;

  }
  ```

  ✅ **正確**：
  ```cpp
  {
      int i = 0;
  }
  ```

- **修正方式**：移除 `}` 之前的所有空白行。

---

### F8 — 閉括號與 else 之間的空白行

- **規則**：`}` 符號後若接 `else` 關鍵字，兩者之間不應包含空白行。
- **說明**：else 是 if 區塊的延續，應與 `}` 視覺上相鄰。
- **範例**：

  ❌ **不良**：
  ```cpp
  if( bCondition )
  {
      DoThis();
  }

  else
  {
      DoThat();
  }
  ```

  ✅ **正確**：
  ```cpp
  if( bCondition )
  {
      DoThis();
  }
  else
  {
      DoThat();
  }
  ```

- **修正方式**：確保 `}` 與 `else` 在相鄰行上（中間無空白行）。

---

### F9 — if 判斷式括號內的空白行

> ⚠️ **執行依賴**：**本規則必須在 F5–F8 之後執行。** F5 修正 `) {` 模式後，會暴露原本被遡蓋的多行 if 條件空白行，此時再執行 F9 才能確保全數清除。

- **規則**：if 判斷式括號內（`if( ... )`）不應包含空白行，包括多行條件的情況。
- **說明**：空白行會影響代碼緊湊度，降低可讀性。
- **範例**：

  ❌ **不良**：
  ```cpp
  if( iStageCfg == 0
  
      && iPartType == ePartDIP )
  {
      // ...
  }
  ```

  ✅ **正確**：
  ```cpp
  if( iStageCfg == 0
      && iPartType == ePartDIP )
  {
      // ...
  }
  ```

- **修正方式**：移除 `if(` 和 `)` 之間的所有空白行。

---

### F10 — 閉括號後接 if 應有空白行

- **規則**：若 `}` 符號下一行緊接 `if`（非 `else if`），兩者之間應插入一個空白行。
- **說明**：`else` 是 if 區塊的延續，故 `}` 後接 `else`/`else if` 不需空白行（F8）；但若是獨立的新 `if` 陳述式，應有空白行以區隔邏輯段落，提高可讀性。
- **範例**：

  ❌ **不良**：
  ```cpp
  if( bConditionA )
  {
      DoThis();
  }
  if( bConditionB )
  {
      DoThat();
  }
  ```

  ✅ **正確**：
  ```cpp
  if( bConditionA )
  {
      DoThis();
  }

  if( bConditionB )
  {
      DoThat();
  }
  ```

- **注意**：`} else if(` 和 `} else {` 不在此規則範圍內（由 F8 管理）。
- **修正方式**：在 `}` 與緊接的 `if` 之間插入一個空白行。
  - **Python 腳本示例**：
    ```python
    import re
    with open(filename, encoding='cp950') as f:
        content = f.read()
    # 在 } 後直接接 if（非 else if）時插入空白行
    content = re.sub(r'(\})(\n)([ \t]*if\b)', r'\1\2\n\3', content)
    with open(filename, 'w', encoding='cp950') as f:
        f.write(content)
    ```

---

### F11 — 函式分隔線（.cpp 檔案專用）

- **規則**：**僅適用於 `.cpp` 檔案**。函式之間應使用分隔線進行視覺分隔，標準分隔線為註解行 `//==============================================================================`。若已存在分隔線（`//----` 或 `//===` 系列），則保留；若無分隔線，應添加標準分隔線。
- **說明**：分隔線提高代碼的視覺結構清晰度，便於快速定位函式邊界。此規則不適用於標頭檔（`.h` 檔案）。
- **適用檔案**：`*.cpp` 檔案
- **不適用檔案**：`*.h` 或其他標頭檔
- **範例**：

  ❌ **不良**（無分隔線）：
  ```cpp
  void FunctionA()
  {
      // ...
  }
  void FunctionB()
  {
      // ...
  }
  ```

  ✅ **正確**（有分隔線）：
  ```cpp
  void FunctionA()
  {
      // ...
  }

  //==============================================================================

  void FunctionB()
  {
      // ...
  }
  ```

- **修正方式**：在相鄰函式之間添加分隔線（**僅在 `.cpp` 檔案中**）。若已存在 `//----` 或 `//===` 形式的分隔線，無需修改；若不存在，添加標準分隔線 `//==============================================================================`。

---

### F12 — 行內註解階梯對齊（選用，需單獨要求）

> ⚠️ **此規則預設不執行**。僅在使用者明確要求「對齊行內註解」時才套用。

- **規則**：程式碼行內的 `//` 註解，依程式碼長度選擇最近的對齊欄位（三階段）：

  | 程式碼長度（字元數） | 對齊目標 |
  |---|---|
  | ≤ 79 | **第 81 欄**（最常見）|
  | 80–119 | **第 121 欄** |
  | 120–159 | **第 161 欄** |
  | ≥ 160 | 保留至少 **2 個空格**間距 |

- **不適用對象**：整行皆為 `//` 的純註解行，位置不變。
- **說明**：三階段設計可避免長程式碼行因對齊而大量留白，同時維持同一縮排層級內的視覺整齊度。
- **範例**：

  ```cpp
  // 短程式碼 → col 81
  int iUseArm = 0;                                                                //0: inarm  1: Outarm

  // 中等長度程式碼 → col 121
  if(CUSTOMER_CODE != CC_ASE_KaohSiung && iTeachMode == 2)                        //kevin 20210331 add Teach mode

  // 長程式碼 → col 161
  if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)          //Ztex 2024.02.24
  ```

- **執行腳本**（`align_comments.py`）：

  ```python
  # Tiered alignment columns (0-based indices, i.e. col 81/121/161)
  TIER_COLS = [80, 120, 160]
  MIN_GAP   = 2    # minimum spaces when all tiers exceeded

  def find_inline_comment_pos(line):
      """偵測行內 // 的 byte index（跳過字串內的 //）。"""
      in_str, str_char = False, ''
      i = 0
      while i < len(line) - 1:
          c = line[i]
          if in_str:
              if c == '\\': i += 2; continue
              if c == str_char: in_str = False
          else:
              if c in ('"', "'"): in_str, str_char = True, c
              elif c == '/' and line[i+1] == '/': return i
          i += 1
      return -1

  def align_line(line):
      """將行內 // 依三階段規則對齊。純 // 開頭行不動。"""
      if line.lstrip().startswith('//') or line.strip() == '':
          return line
      pos = find_inline_comment_pos(line)
      if pos < 0:
          return line
      code_part    = line[:pos].rstrip()
      comment_part = line[pos:]
      code_len     = len(code_part)
      target = next((t for t in TIER_COLS if code_len + MIN_GAP <= t), None)
      spaces = (target - code_len) if target else MIN_GAP
      return code_part + ' ' * spaces + comment_part

  # 套用至檔案（CP950）
  with open(fp, 'rb') as f:
      raw = f.read()
  has_crlf = b'\r\n' in raw
  try:   text = raw.decode('cp950'); enc = 'cp950'
  except: text = raw.decode('latin-1'); enc = 'latin-1'
  text = text.replace('\r\n', '\n').replace('\r', '\n')
  lines = [align_line(l) for l in text.split('\n')]
  result = '\n'.join(lines)
  if has_crlf: result = result.replace('\n', '\r\n')
  with open(fp, 'wb') as f:
      f.write(result.encode(enc))
  ```

- **注意事項**：
  - 此操作會改動大量行，建議在 SVN commit 前單獨執行，不與其他格式化修正混用同一 commit。
  - **不加入** F1–F11 的常規自動化流程（`format_code_f1_f11.py`）。

---

## 自動化檢查與修復流程

### 步驟 1：檢查（Scanning）

建議按以下順序掃描，避免因修復順序導致的重複檢查：

1. **標識所有格式化問題**（F1–F8）
2. **收集問題清單**（檔案 + 行號 + 問題類型）
3. **產生報告**（Markdown 或 HTML）

### 步驟 2：修復（Fixing）

> ⚠️ **關鍵依賴**：**F9 必須在 F5–F8 之後執行**。F5 修正 `) {` 模式後會暴露原本被遮蓋的 `if(...)\n\n  statement` 模式，此時再執行 F9 才能確保不遺漏。（實測：先跑 F9 得 0，F5–F8 後殘留 35 檔，F9 最後執行後歸零。）

建議執行順序（由於依賴性）：

1. **F1**（Tab 轉為空白格）- 獨立，無依賴
2. **F2**（行尾空白）- 獨立，無依賴
3. **F3**（連續空白行）- 獨立，無依賴
4. **F4**（if/else if/else 括號一致性）- 應在空白行清理後進行，避免重複檢查
5. **F5–F8**（`{`/`}` 與 else 相關的空白行）- 需在 F1–F4 後進行
6. **F9**（if 判斷式括號內的空白行）- **必須在 F5–F8 之後執行**（見上方警告）
7. **F10**（`}` 後接 if 的空白行補充）- 在 F9 之後執行，避免 F9 去除的空白行與 F10 補充的空白行相互覆蓋
8. **F11**（函式分隔線）- 最後執行，獨立操作


### 步驟 3：驗證（Verification）

修復完成後，重新掃描全文件，確認所有問題已解決：

```powershell
# 偽代碼示例
$issues = ScanFormattingIssues($filePath)
if ($issues.Count -eq 0) {
    Write-Host "✓ 檔案已通過所有格式化檢查"
}
else {
    Write-Host "✗ 仍有 $($issues.Count) 個未解決的問題"
}
```

---

## Big5 編碼注意事項

由於 HT9045 專案使用 Big5（CP950）編碼：

- 所有格式化修復操作應使用 `encoding='cp950'` 或 `encoding='big5'`
- 避免在 UTF-8 編輯器中進行批量替換，以免中文註解亂碼
- 使用 Python 或 PowerShell 時，應顯式指定編碼

---

## 常見問題（FAQ）

**Q：F5 與 F6 是否有執行順序相依性？**

A：有。**F5 必須在 F6–F9 之後執行**。F6 處理 `)

{` 模式時，會將部分同時符合 F5 條件的空白行一併清除；F6 執行後，剩餘的 `if(condition)

 statement`（無大括號）模式才會完整露出，此時再執行 F5 才能確保不遺漏。實際驗證：初始掃描 F5=0，F6–F9 執行後 F5 殘留 35 個檔案，F5 最後執行後歸零。

**Q：連續空白行的定義是「2 行以上」還是「3 行以上」？**

A：根據 F3 規則，**2 行以上**的空白行應合併成 1 行。即若相鄰出現 2 行空白行，應刪除其中 1 行。

**Q：行尾空白是否包含 CRLF（`\r\n`）中的 `\r`？**

A：F2（行尾空白）只移除 `\r\n` 之前的空白字符（如 space 或 tab），不移除換行符本身。

**Q：F4（括號一致性）只適用於 if-else 鏈嗎？**

A：F4 主要針對 if-else-if-else 鏈結構。單獨的 if 或 while 可根據實際需求決定是否添加 `{}`。

**Q：F10（函式分隔線）可以使用其他格式嗎？**

A：推薦使用標準格式 `//==============================================================================`。如已存在 `//----` 或 `//===` 系列的分隔線，可保留既有風格以保持一致性。

---

## 參考資源

- [SKILL.md](../SKILL.md) - 上線前程式碼風險檢查主文檔
- [patterns.md](patterns.md) - P1–P6 風險模式詳細定義
- VS Code Settings - `files.trimTrailingWhitespace`, `editor.insertSpaces` 等相關設定
