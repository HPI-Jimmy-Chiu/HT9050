# BCB6 編譯錯誤案例

---

### A01 E2015 GetTickCount Ambiguity
- **問題編號**：內部追蹤
- **影響版本**：V3.33.904.2（合併後觸發）
- **症狀**：`E2015 Ambiguity between '_fastcall Idglobal::GetTickCount()' and '__stdcall GetTickCount()'`
- **根因**：HT9045 引入 Indy 元件後，某些 header 鏈把 `Idglobal` namespace 拉進來，導致 `GetTickCount()` 與 Windows API 歧義
- **修法**：呼叫點加 `::` 強制使用 global Windows API → `::GetTickCount()`
- **預防**：所有新增的 `GetTickCount()` 呼叫統一用 `::GetTickCount()`
- **案例**：HT9045 V3.33.904.2 (2026-05-07), `acarry.cpp` line 4009/4010/5750/5751, RogerYang 904.1 引入 SHT1/SHT2 watchdog

---

### A02 E2141 Declaration syntax error（3-way merge 破壞）
- **問題編號**：內部追蹤
- **影響版本**：V3.33.904.0 合併過程
- **症狀**：`E2141 Declaration syntax error`，函式體被插入到 `{` 之前
- **根因**：`scripts/three_way_merge.py` 用 difflib 做 3-way merge，source≈base 而 target 有大量新增時靜默遺失內容
- **修法**：改用「複製 target + 套用 source delta」策略（patch 或 replace_string_in_file）
- **預防**：
  1. 合併後檢查檔案大小變化（-7000 bytes 以上為異常）
  2. 全重建驗證 0 errors
- **案例**：HT9045 V3.33.904.0 (2026-04-28), JerryYang1 r902 + Steven r903 → r904, 5 個檔案被破壞

---

### A03 Unresolved external / Undefined symbol
- **問題編號**：通用類型
- **影響版本**：多版本
- **症狀**：`[Linker Error] Unresolved external 'xxx'` 或 `Undefined symbol 'xxx'`
- **根因**：
  1. 合併遺漏 .cpp 檔案
  2. .bpr/.mak 未同步更新
  3. cmydef.h/cmydef.cpp 新增宣告但未包含實作
- **修法**：
  1. 確認 `bpr2mak` 重新產生 .mak
  2. 比對來源版本的 .bpr 檔差異
  3. 搜尋 symbol 在哪個 .cpp 定義
- **預防**：合併時先比對 .bpr 檔案是否有新增 .cpp

---

### A04 SOFT_SIMULTE 未關閉（release build 失誤）
- **影響版本**：通用
- **症狀**：Release 版本包含模擬模式程式碼，機台行為異常
- **根因**：`MachineType.h` 中 `#define SOFT_SIMULTE 1` 未改回 0
- **修法**：發版前強制檢查 `SOFT_SIMULTE` = 0
- **預防**：已建立 Instruction `ht9045-release-guards` 自動攔截
