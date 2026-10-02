# 合併報告（Merge Report）— 格式規範

> 此文件由 [ht9045-code-merge/references/merge-build-verify.md Step 6](d:\HT9045\.github\skills\ht9045-code-merge\references\merge-build-verify.md) 移入統一管理。
> 上層技能見 [Make-Report-Skill](../../SKILL.md)。
> 報告類型：**MD only**，不輸出 HTML，無 Logo。

---

## 輸出路徑

```
<repo>\public\Docs\MergeReport\<年度>\MergeReport_<yyyyMMdd>_<Developer1>[_<Developer2>].md
```

> 20260929 起報告寫進入口網站 repo（Steven 20260929 12:0x：「未來報告跟手冊的寫入路徑都要放入 D:\RD5-Portal\public\Docs\ 裡面」），跟 SVN 的程式碼不在同一個 repo，無法同一個 commit：程式碼 commit 訊息寫上報告檔名，報告照入口網站流程開分支＋MR。20260929 之前的舊報告仍在 SVN（下面 `svn cat` 用的是舊路徑）。

---

## 報告頭部必填欄位

```markdown
# HT9011UC 合併報告

**日期**：<YYYY-MM-DD>  
**來源版本一**：HT9011UC_Code_V3.33.<REV>.0_<YYYYMMDD>_<來源作者>  
**來源版本二**：（若有）HT9011UC_Code_V3.33.<REV>.0_<YYYYMMDD>_<來源作者>  
**目標版本**：HT9011UC_Code_V3.33.<目標REV>.0_<YYYYMMDD>_<作者一>_<作者二>  
**SVN Base Revision**：Rev<號碼>（即 `svn export -r` 所用的 revision）  
**執行者**：AI Merge Assistant  
**審核者**：Steven  
```

> `SVN Base Revision` 填實際的 revision 數字（`svn info` 取得的 `Revision:` 欄位），不得填 N/A。

---

## 報告結構

```markdown
# HT9011UC <Developer> 版本合併報告
## 1. 修改概覽（統計指標）
## 2. 修改內容（直接複製 / 3-way Merge / 新增檔案）
## 3. 衝突分析與解決
## 4. 缺失宣告補充
## 5. 編譯驗證結果
## 6. 備份機制
## 7. 後續建議
## 8. 功能來源追蹤       ← 方案B：合併可追溯性
## 9. SVN Commit Message  ← 精簡版，直接用於 svn commit -m
```

---

## Section 8 功能來源追蹤（必填）

每份報告結尾必須包含此 Section，作為本次合併的永久追蹤紀錄。

```markdown
## 8. 功能來源追蹤

> 供未來 bug 追蹤、功能 revert、下次合併比對使用。

| 功能 / 變更說明 | 來源開發者 | 原始日期 | 涉及檔案 |
|---------------|----------|---------|--------|
| <功能名稱> | <開發者> | <YYYY-MM> | <檔案路徑> (~L<行號>) |
| ... | ... | ... | ... |

### SVN 查詢方式

\`\`\`powershell
# 查看本次 commit 的完整異動清單：
svn log -r <revision> --verbose "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20"

# 取出本報告（歷史版本）：
svn cat -r <revision> "D:\docs\release-notes\MergeReport\<年度>\MergeReport_<yyyyMMdd>_<Developer>.md"
\`\`\`
```

---

## Section 9 SVN Commit Message（必填）

從 Section 8 功能清單自動歸納，直接用於 `svn commit -m`。

```markdown
## 9. SVN Commit Message

\`\`\`
Merge: <來源一作者> + <來源二作者> → V3.33.<REV>.0_<YYYYMMDD>

[合入功能]
- <功能一簡述>
- <功能二簡述>
- 新增檔案：<檔名1>、<檔名2>

[衝突解決]
- <檔案>: <衝突說明與決策>

[驗證結果]
- Build: 0 errors, <N> warnings
- 審核者：<審核者姓名>
\`\`\`
```

> **重要**：報告本身也進 SVN commit（與程式碼同一筆 commit），確保追蹤紀錄與程式碼版本永遠對應。
