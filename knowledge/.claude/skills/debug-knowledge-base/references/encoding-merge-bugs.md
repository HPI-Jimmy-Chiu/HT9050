# 編碼與合併問題案例

---

### E01 Big5 檔案中文變亂碼
- **問題編號**：內部追蹤
- **影響版本**：合併操作通用
- **症狀**：Big5(CP950) 編碼的 .cpp/.h 檔案中，中文註解顯示為 `??閬?`、`銋?憿舐內` 等亂碼
- **根因**：`replace_string_in_file` 或 `create_file` 以 UTF-8 bytes 寫入，但檔案實際為 Big5 編碼
- **偵測**：VS Code 以 Big5 開啟看到亂碼 → 以 UTF-8 開啟反而正常 → 確認為編碼錯置
- **修法**：
  1. **首選**：直接用 binary 模式從原始檔複製整段 bytes
  2. **次選**：用 ASCII/英文註解，避免中文
  3. **禁止**：使用 `replace_string_in_file` 寫入含中文的內容到 Big5 檔案
- **預防**：
  - Big5 檔案的修改若涉及中文，一律用 Python binary 操作
  - 修改後用 binary scan 驗證無 UTF-8 三字節序列
- **案例**：HT9045 V3.33.904.0 合併 (2026-04-28), main.cpp 3 行 + ckernel.cpp 1 行亂碼

**偵測腳本**：
```python
with open(fname, 'rb') as f:
    data = f.read()
for i, line in enumerate(data.split(b'\n'), 1):
    j = 0
    while j < len(line):
        b = line[j]
        if 0xE0 <= b <= 0xEF and j+2 < len(line) and 0x80 <= line[j+1] <= 0xBF and 0x80 <= line[j+2] <= 0xBF:
            print(f'{fname}:{i}: possible UTF-8 garbled')
            break
        j += 1
```

---

### E02 3-way merge difflib 靜默遺失內容
- **問題編號**：內部追蹤
- **影響版本**：合併操作通用
- **症狀**：
  - 合併後檔案大小 -7000 ~ -17000 bytes
  - BCB6 編譯出現 `E2141 Declaration syntax error`
  - Report 顯示「(無衝突)」但實際內容被砍
- **根因**：`scripts/three_way_merge.py` 用 difflib 做 3-way merge，當 source ≈ base 而 target 有大量新增時，difflib 的 hunk 對齊失敗，靜默丟棄 target 內容
- **修法**：當 source ≈ base 時，改用「複製 target + 套用 source delta」：
  1. 從 target 完整複製
  2. 用 `patch -p0 < source.diff` 或 `replace_string_in_file` 套用 source hunks
  3. patch 偏移失敗時改手動 replace_string_in_file
- **預防**：
  1. 合併前比較 source vs base 差異量
  2. 合併後比對檔案大小
  3. 全重建驗證 0 errors
- **案例**：HT9045 V3.33.904.0 (2026-04-28), 第一次 difflib 合併破壞 5 個檔案，改策略後 0 errors
