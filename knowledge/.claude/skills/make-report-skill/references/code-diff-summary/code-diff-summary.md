# Code Diff Summary（程式碼差異摘要）

## 適用時機
- 每日或每週對 HT9045 / GPIB9045 程式碼進行異動彙整
- 輸出結構化的修改摘要，嵌入 ops daily/weekly worklog

## 輸入模式
**mtime 模式**（唯一支援模式）：
- 使用 PowerShell `Get-Item` 比對指定時間範圍內 `.cpp`、`.h` 檔案的修改時間（LastWriteTime）
- 與上次快照 `.last_snapshot_{project}.json` 比對，找出異動檔案

## 腳本
`../../scripts/generate_code_diff_summary.py`

CLI 用法：
```
python generate_code_diff_summary.py --project HT9045|GPIB9045 --range daily|weekly
python generate_code_diff_summary.py --project HT9045 --date 2026-04-10
```

## 快照機制
- 每次執行後，將檔案清單（路徑 + mtime）儲存至 `../../scripts/.last_snapshot_{project}.json`
- 下次執行時比對差異，輸出異動清單

## 輸出格式
純 **Markdown**，嵌入 ops worklog 使用（不獨立輸出 HTML）

## 輸出欄位

```markdown
## Code Diff Summary — {YYYYMMDD} — {專案}

| 模組 | 檔案 | 風險等級 | 影響函式 |
|------|------|----------|----------|
| {模組} | {filename.cpp} | ⚠ High / ℹ Normal | {函式名1, 函式名2} |
```

### 模組分類（HT9045）

| 模組名 | 對應目錄/前綴 |
|---|---|
| inarm | `aInArm*.cpp/h` |
| outarm | `aOutArm*.cpp/h` |
| shuttle | `aShuttle*.cpp/h` |
| index | `aIndex*.cpp/h`, `aTester*.cpp/h` |
| catchtray | `aCatchTray*.cpp/h` |
| motor | `Motor*.cpp/h`, `cMotor*.cpp/h` |
| io | `cIO*.cpp/h`, `myio*.cpp/h` |
| secs | `HT9045Gem*.cpp/h`, `uHGem*.cpp/h` |
| general | `CosFunction.cpp/h`, `HandlerSys*.cpp/h` |

### 模組分類（GPIB9045）

| 模組名 | 對應前綴 |
|---|---|
| main | `Main.cpp` |
| rs232 | `RS232*.cpp/h` |
| msgdef | `MessageDef*.cpp/h`, `cmydef.h` |
| art | `*ART*.cpp/h`, `DummyArt*.cpp/h` |
| dut | `MyDutPanel*.cpp/h` |

## 風險標記規則

| 等級 | 標記 | 觸發關鍵字 |
|---|---|---|
| High | ⚠ High | Motor, Sucker, Destroy, Alarm, JAM, Interlock, Safety |
| Normal | ℹ Normal | 上述以外 |

## 函式解析方式
在異動的 `.cpp/.h` 檔中，用 regex 搜尋：
```regex
^(void|bool|int|AnsiString|double|float)\s+[A-Za-z_]\w*\s*\(
```
列出所有符合定義的函式名稱作為「影響函式」欄位。

## 模板
`../../templates/code-diff-summary-template/template.md`
