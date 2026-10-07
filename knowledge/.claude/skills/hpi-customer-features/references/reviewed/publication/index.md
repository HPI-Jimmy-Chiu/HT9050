# 密碼簿說明的公開發布界線

依 Jimmy 20261007 19:5x 裁決，密碼簿加解密算法說明固定排除公開 GitHub 同步。這是公司私有 Skill 的發布界線；同題供 HT9050 與其他 Handler 查閱，不能由機型、版本或客戶碼放寬。

## 固定排除的已列範圍

| 私有 references 子樹 | 內容路由 |
| --- | --- |
| `customer-book-storage` | [結構、slot、binary 編輯與存檔](../customer-book-storage/index.md) |
| `customer-book-text` | [EncodeStr 與文字密碼簿](../customer-book-text/index.md) |
| `customer-login-readers` | [讀取、解碼與字串](../customer-login-readers/index.md) |

三組都位於 `hpi-customer-features/references/reviewed/`。之後新寫的同類算法說明也應加入筆電 `gh_knowledge.py` 的 HOLD 清單；不能因不在這三個目錄就推定可以公開。

## 裁決與實作分開記錄

- 公司 GitLab 的原稿、metadata、source manifest、歷史證據與相容入口繼續保留；固定排除公開發布不等於刪除私有知識。
- 裁決維持 HT9050 公開機台更新包的既有交付方式，不改寫公開 GitHub 歷史、不更換金鑰；本 Skill 文件整理不執行機台包交付。
- HOLD 是筆電同步流程的責任。此次只確認裁決與私有 references 路由，沒有修改或執行同步工具，沒有公開發布；未查證筆電實際 HOLD 清單或發布結果，不能宣稱排除已在工具生效。
- 沒有讀實際密碼簿、帳密、key 值或 runtime；既有 body 靜態結論與未查界線維持原查證日期，不因這份發布規則升級為完整 S8 或實機驗證。

裁決定位：版本 `HT9011UC_Cpp_V3.33.906.0`、檔案 `docs/RULINGS_20261007.md`、第 11 條／#141＝C；釘點與範圍見 [manifest](scope-manifest.json)。裁決原文見[私有 repo 正文](../../../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261007.md)。

回到[人工查證樹](../index.md)；完整 caller／reader／writer、機型 dispatch 與語意未決項仍依[待補清單](../../pending.md)逐批核對。

## 本批交付與待續

20261007：本批只交付固定排除的發布界線。S8 的 846 項 file-scope／unresolved 語意候選、完整 caller／reader／writer 與機型分派明標「待續」；保留 13 個既有人工客戶列與原始候選，不新增完成列、不宣稱密碼簿算法或筆電 HOLD 實作已全數查證。
