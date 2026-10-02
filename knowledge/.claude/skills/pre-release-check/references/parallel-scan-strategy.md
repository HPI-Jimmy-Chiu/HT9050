# 平行掃描策略（Parallel Scan Strategy）

P1–P7 各模式的搜尋 regex 與判斷邏輯互相獨立，可同時啟動多個 sub-agent 進行平行掃描，大幅縮短等待時間。

## 平行分組

依搜尋方式將所有檢查分為五個平行 batch，同時發出：

| Batch | 模式 | 說明 | 主要工具 |
|-------|------|------|----------|
| **Batch A** | P1、P2 | regex 搜尋，逐行比對 | `grep_search` / `Select-String` |
| **Batch B** | P3、P4 | 函式呼叫追蹤 + 迴圈邏輯分析 | `read_file` + 人工推理 |
| **Batch C** | P5、P6、P7 | 變數語意比對 + 除法掃描 + 陣列索引稽核 | `grep_search` + `read_file` |
| **Batch D** | DFM 解析度 | `.dfm` 控件座標與解析度邊界稽核（無 `.dfm` 修改時跳過） | `read_file` |

> **依賴說明**：四個 batch 均不依賴彼此的輸出，可真正平行啟動。P6 過濾階段（判定 false positive）需在 Batch C 回傳後才能執行，但不阻擋其他 batch。
> **Batch E（代碼格式化 F1–F11）**：改為步驟 3 獨立執行（Batch A/B/C/E 完成後），格式化稽核屬不同維度，不與風險掃描混用同一批次。

## 平行執行範本（主 agent 指令）

在步驟 2 時，**同時**發出四個 `runSubagent` 或平行工具呼叫；步驟 3 再獨立發出 Batch D：

```
[平行啟動（步驟 2）]
  → Sub-agent A：掃描 P1 (bare value in || chain) + P2 (enum as bool)
      - 對每個 .cpp/.h 執行 grep_search regex
      - 回傳：命中行號、代碼片段、是否為 bool/enum 的初步判斷

  → Sub-agent B：掃描 P3 (multi-stage loop) + P4 (save-reload path)
      - 讀取含 for/while stage 迴圈的函式
      - 追蹤 Save*/Read*/Load* 呼叫鏈
      - 回傳：迴圈分支表、Save/Load 欄位對稱性

  → Sub-agent C：掃描 P5 (copy-paste var) + P6 (division) + P7 (LastSet bounds)
      - 檢查 Input/Output 區段變數語意
      - grep 所有 /（除法），過濾已知安全模式
      - 稽核 LastSet 陣列存取索引
      - 回傳：P5 候選、P6 候選（含是否有 guard）、P7 候選

  → Sub-agent D：DFM 畫面解析度稽核（無 .dfm 修改時跳過）
      - 讀取 .dfm 中所有控件的 Left/Top/Width/Height
      - 驗證在 1280×1024 與 1920×1080 下均不超出框架
      - 回傳：超出邊界的控件清單（Form | Control | Property | Value | Limit）
[等待全部完成，合併結果]

[獨立執行（步驟 3）]
  → Sub-agent E：代碼格式化檢查 F1–F11
      - 依據 references/formatting-standards.md 逐規則驗證
      - ⚠️ F9 須在 F5–F8 之後執行（內部依序處理）
      - 回傳：格式化違規清單（File | Line | Rule | Description）
```

## Sub-agent prompt 範本

每個 sub-agent 的 prompt 應包含：

1. **目標目錄與檔案清單**（明確列出）
2. **負責的模式編號與說明**（僅含自己的 batch）
3. **輸出格式要求**（固定欄位：File \| Line \| Code \| Description）
4. **禁止跨 batch 分析**（避免重複工作）

```
你是 C/C++ 風險分析專家。
目標目錄：<PROJECT_DIR>
檔案：<FILE_LIST>

請僅執行以下模式掃描：<P1+P2 / P3+P4 / P5+P6+P7>
[模式說明與 regex...]

輸出格式：
| File | Line | Code | Pattern | Description |
```

## 合併與後處理（主 agent）

步驟 2 的四個 batch（A/B/C/D）全部回傳後，主 agent 執行：

1. **去重**：相同 File + Line 若跨 batch 均回報，合併為單一條目。
2. **P6 False Positive 過濾**：對照 [division-safety.md](division-safety.md) 已知安全模式（`QueryPerformanceFrequency`、`ChangeToFloatNonPcnt`、常數除數等）剔除。
3. **Severity 排序**：Critical → High → Low。
4. **合併 Batch E 結果**：DFM 解析度問題以 `DFM` 標示 Pattern 欄位併入 findings 表。
5. **進入步驟 3**（Batch E 格式化檢查）；Batch E 回傳後以 `F1–F11` 標示 Pattern 欄位補入 findings 表，再進入步驟 4。

## 適用時機

| 情境 | 建議策略 |
|------|----------|
| 檔案數 ≤ 5，修改量小 | 單一 sub-agent 或主 agent 直接逐模式掃描；Batch D/E 仍可平行 |
| 檔案數 6–20 | **本文件策略**（四 batch 平行 + Batch E 獨立） |
| 檔案數 > 20 或大型專案 | 在每個 batch 內再依模式拆分，最多 5 個平行 sub-agent |

> ⚠️ **注意**：平行 sub-agent 之間不共享狀態。每個 sub-agent 必須自行讀取所需檔案，不可假設其他 agent 已讀取。
