# HT9011UC 合併腳本參考手冊

> 本文件為 `scripts/` 目錄下所有腳本的完整說明。
> 當任何腳本的 CLI 介面有更新時，此文件必須同步維護。

---

## 腳本目錄

| 腳本 | 觸發時機 | 說明 |
|------|---------|------|
| `merge_main.py` | 每輪合併主入口 | 串接 SVN 檢查 → 重疊偵測 → 合併 → 編譯 |
| `svn_helper.py` | Phase 0b（每輪合併前） | SVN 工作複本診斷與修復 |
| `overlap_analysis.py` | Phase 2 | SVN diff 分析，輸出合併分類清單 |
| `three_way_merge.py` | Phase 3（3-way 候選檔）| 三方合併核心，支援 diff3 後端 |
| `bpr_updater.py` | Phase 4（新增檔案時）| BCB6 .bpr 專案檔增刪管理 |
| `restore_changeToFloat.py` | Phase 4（驗收失敗時）| 還原被誤刪的 ChangeToFloatNonPcnt 呼叫 |
| `whitespace_normalize.py` | Phase 5（空白標準化）| Tab/空白/行尾清理，保護 CP950 編碼 |
| `build_verify_safe.ps1` | Step 5（BCB6 編譯）| 安全執行 bpr2mak + make，含孤兒視窗清理 |
| `md2html.py` | Step 6（報告產生）| 合併報告 Markdown → HTML 轉換 |

---

## 詳細說明

### merge_main.py

| 欄位 | 內容 |
|------|------|
| **用途** | 合併主入口：串接 SVN 診斷、重疊偵測、直接複製/3-way merge、空白標準化、BCB6 編譯、報告產生 |
| **CLI** | `merge_main.py --source <來源路徑> --target <目標路徑> --developer <開發者> [--skip-build]` |
| **主要參數** | `--source`：來源版本路徑；`--target`：目標版本路徑；`--developer`：來源開發者名稱；`--skip-build`：跳過 BCB6 編譯 |
| **輸出** | stdout 進度；build.log；合併報告草稿 |
| **備註** | 多來源合併時對每位來源各呼叫一次；目標版本由 Phase 0a svn export 預先建立 |

---

### svn_helper.py

| 欄位 | 內容 |
|------|------|
| **用途** | 自動檢查 .svn 工作複本健康狀態；修復遺失/損毀的 .svn 目錄；從資料夾名稱解析 Revision |
| **CLI** | `svn_helper.py check --path <路徑>` / `repair --path <路徑> [--revision <rev>]` / `clone-svn --source <s> --target <t>` / `parse-folder --name <名稱>` |
| **主要參數** | `--path`：工作複本根路徑；`--revision`：手動指定 SVN revision（repair 用）|
| **輸出** | 健康狀態報告；修復後的 .svn 目錄 |
| **觸發時機** | Phase 0b：每輪合併前確認來源工作複本可用 |
| **備註** | `parse-folder` 從 `V3.33.901.0_20260408_Steven` 格式字串解析版本號；SVN repo URL 硬編碼為 `file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20` |

---

### overlap_analysis.py

| 欄位 | 內容 |
|------|------|
| **用途** | 掃描來源/目標的 SVN 修改清單，偵測重疊檔案，分類為直接複製/3-way/跨版本 3-way/新增檔案/跳過 |
| **CLI** | `overlap_analysis.py --source <路徑> --target <路徑> [--check-cross-rev] [--temp-dir <dir>] [--skip-ext ...]` |
| **主要參數** | `--source`、`--target`：兩端版本路徑；`--check-cross-rev`：當 source rev < target rev 時啟用跨版本比對；`--temp-dir`：跨版本 base 的暫存目錄 |
| **輸出** | JSON / stdout 分類清單；`ChangeToFloatNonPcnt` 基線計數（供 Phase 4 驗收） |
| **觸發時機** | Phase 2：差異比對與風險分類 |
| **備註** | `--check-cross-rev` 同時將跨版本 base 存入 temp-dir，供後續 three_way_merge.py 使用 |

---

### three_way_merge.py

| 欄位 | 內容 |
|------|------|
| **用途** | 單檔三方合併核心；優先使用 diff3 後端，回退 difflib；含 post-merge validation hook |
| **CLI** | `three_way_merge.py --base <base> --source <src> --target <tgt> --output <out> [--risk-check] [--source-label <標籤>] [--target-label <標籤>]` |
| **主要參數** | `--base`：共同祖先；`--source`：來源修改端；`--target`：目標修改端；`--output`：輸出路徑；`--risk-check`：僅輸出風險等級，不執行合併 |
| **輸出** | 合併後檔案；exit code `0`=乾淨、`1`=有衝突、`2`=post-merge hook 發現問題 |
| **觸發時機** | Phase 3：3-way merge 候選檔 |
| **備註** | exit code 2 表示括號深度失衡或重複 `#define`，**不可繼續**；diff3 路徑：`C:\Program Files\Git\usr\bin\diff3.exe` |

---

### bpr_updater.py

| 欄位 | 內容 |
|------|------|
| **用途** | 新增、移除或列出 BCB6 `.bpr` 專案檔中的 `.cpp`/`.h` 來源檔 |
| **CLI** | `bpr_updater.py add --bpr <.bpr路徑> --file <檔案路徑>` / `remove` / `list` |
| **主要參數** | `--bpr`：目標 .bpr 路徑；`--file`：要新增/移除的檔案路徑 |
| **輸出** | 修改後的 .bpr 檔案；自動備份舊版（加時間戳 .bak_YYYYMMDD_HHMMSS） |
| **觸發時機** | Phase 4：新增檔案時，若為 `.cpp` 必須呼叫此腳本加入 .bpr |
| **備註** | 以 CP950 讀寫 .bpr；自動備份機制確保可還原；修改後需執行 L1-5 驗證 |

---

### restore_changeToFloat.py

| 欄位 | 內容 |
|------|------|
| **用途** | 掃描 SVN diff 輸出，偵測並還原 merge 過程中被誤替換為裸除法的 `ChangeToFloatNonPcnt` 保護呼叫 |
| **CLI** | `restore_changeToFloat.py --project <目標版本根路徑>` |
| **主要參數** | `--project`：目標版本根路徑（必填，例：`D:\HT9045\HT9011UC_Code_V3.33.901.0_20260408_Steven_RogerYang`） |
| **輸出** | 還原後的 .cpp 檔案；`<project>/restore_changeToFloat.log`（含 RESTORED/NOT_FOUND/deleted block 記錄） |
| **觸發時機** | Phase 4：`ChangeToFloatNonPcnt` 驗收計數小於基線時 |
| **備註** | 搜尋算法：以 token 相似度（>=2 個共用識別碼）匹配 removed/added 行；修復 ±20 行範圍內搜尋；deleted block 需人工確認 |

---

### whitespace_normalize.py

| 欄位 | 內容 |
|------|------|
| **用途** | 統一空白格式：Tab → 4 空格、移除行尾空白、壓縮多餘空白行；保護 CP950 編碼 |
| **CLI** | `whitespace_normalize.py <target_dir> [--extensions .cpp .h .c .dfm]` |
| **主要參數** | `target_dir`：處理根目錄；`--extensions`：指定副檔名（預設 .cpp .h .c .asm .dfm .rc） |
| **輸出** | 原地修改各目標檔案 |
| **觸發時機** | Phase 5：所有檔案合併完成後、BCB6 編譯前 |
| **備註** | 必須以 CP950 讀寫；使用 `--extensions` 避免處理二進位檔 |

---

### build_verify_safe.ps1

| 欄位 | 內容 |
|------|------|
| **用途** | 安全執行 BCB6 編譯（`bpr2mak` + `make`），自動偵測並關閉編譯過程產生的孤兒 PowerShell 子視窗 |
| **CLI** | `.\build_verify_safe.ps1 -TargetPath <路徑> [-BprFile HT9045.bpr] [-LogPath <log路徑>]` |
| **主要參數** | `-TargetPath`（必填）：目標版本根路徑；`-BprFile`：.bpr 檔名（預設 HT9045.bpr）；`-LogPath`：build log 輸出路徑（預設 &lt;TargetPath&gt;\build.log） |
| **輸出** | build.log；exit code = `make` 的 exit code（0 = 成功）|
| **觸發時機** | Step 5：BCB6 編譯驗證 |
| **備註** | BCB6 路徑硬編碼為 `D:\ProgramFiles\Borland\CBuilder6`；相比 inline 指令，此腳本多了孤兒視窗清理邏輯，**建議優先使用** |

---

### md2html.py

| 欄位 | 內容 |
|------|------|
| **用途** | 將合併報告 Markdown 轉換為帶有內嵌 CSS 樣式的 HTML，支援表格、粗體、code block |
| **CLI** | `md2html.py <input.md> [output.html]` |
| **主要參數** | 第一個位置參數：.md 輸入路徑；第二個（選填）：.html 輸出路徑（預設同目錄同名） |
| **輸出** | .html 檔案 |
| **觸發時機** | Step 6（選用）：合併報告 .md 完成後，視需要產生 HTML 版本 |
| **備註** | 純 Python 實作，無外部依賴；輸出為 UTF-8 HTML（含 `<meta charset="UTF-8">`）；表格支援 `\|` 轉義 |
