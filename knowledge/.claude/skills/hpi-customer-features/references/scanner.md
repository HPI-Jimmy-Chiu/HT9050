# 唯讀客戶候選scanner

[customer_scan.py](../scripts/customer_scan.py)從指定Git commit讀兩棵來源樹的blob，不依賴稀疏checkout。排除任何.svn路徑；V912用cp950、V906 Cpp用UTF-8。JSON寫到明確指定的repo外新檔，既有檔不覆寫，人工客戶表不改。

```powershell
# 使用已有Python；下列路徑是本機工作樹與暫存範例。
$env:PYTHONUTF8='1'
& 'D:\HT9045\.venv\Scripts\python.exe' '.claude\skills\hpi-customer-features\scripts\customer_scan.py' --self-test
& 'D:\HT9045\.venv\Scripts\python.exe' '.claude\skills\hpi-customer-features\scripts\customer_scan.py' --repo 'D:\AI_TempFile\codex-ht9045-entry-20261006' --ref '<實際commit>' --output 'C:\AI_TempFile\st-gpt-ops\customer-scan-<批次>.json'
```

輸出釘住source commit、每列來源blob、所有掃描檔的manifest SHA-256與scanner SHA-256；沒有目前機台值或source行號欄。MachineType.h定義各版分開，數值別名只在同版對照。

| 候選類型 | 如何使用 |
|---|---|
| direct-comparison | 直接相鄰的CUSTOMER_CODE與CC符號／數值比較，含反向運算元；完整條件及caller仍需讀源碼 |
| symbol-reference | CC符號引用，可含宣告／switch case／其他用途；不是已證明分支 |
| function-flag-reference | FUNC_CC_識別符引用；不自動認定對應客戶碼或已啟用 |
| literal_inactive_candidate | 詞法#if 0／#if 1的提示；未知條件與#if鏈不作真正建置選擇 |
| source_class | test／harness、generated僅依路徑標候選；實際project／CMake收錄另查 |

註解、字串、raw strings排除；一般多行比較有fixture驗證。括號包住運算元、間接CUSTOMER_CODE別名、完整switch關係、複雜macro／template／operator與constructor initializer不保證完整；lambda位置沿用外層function。已列出不能可靠歸屬的定位，不能以它們生成「功能已啟用」或912-only結論。

重新掃描後先比manifest、decoder notes、unresolved與人工抽查，再更新本Skill的候選樹及[待補清單](pending.md)。[render_index.py](../scripts/render_index.py)由JSON重生本Skill的候選references，檢查scanner SHA-256相符；不同來源commit先保留舊摘要並審查，不直接蓋掉既有基準。人工行為說明、裁決與主題表不覆寫。這是源碼詞法／文件驗證，沒有執行機台程式、API、程式碼產生器或runtime測試。

```powershell
& 'D:\HT9045\.venv\Scripts\python.exe' '.claude\skills\hpi-customer-features\scripts\render_index.py' 'C:\AI_TempFile\st-gpt-ops\customer-scan-<批次>.json'
```
