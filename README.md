# HT9050 機台更新用 repo

> **這不是主版本庫。** 主版本庫是公司 GitLab（`gitlab.honprec.com/.../ht9045`，分支 `main`）。
> 這個 repo 只為了讓連不到公司 git 的 HT9050 機台可以用 `git pull` 拿到更新；每一顆 commit ＝ GitLab main 某一版的**更新包快照**，不帶 GitLab 的歷史。

## 裡面有什麼

| 路徑 | 內容 |
|---|---|
| `HT9045/` | 從機台上次合進去的版本（`BASE`，見下表）到這一版（`REV`）之間**有變動的檔案**，路徑照 `D:\HT9045` 的結構（`HT9011UC_Cpp_V3.33.906.0/`、`web/`…） |
| `_machine_ai/` | 給機台端 Claude 的套用工具：`check_and_copy.ps1`（檢查／三方合併）、`package_manifest.tsv`、`known_versions.tsv`、`base_<BASE>/`（三方合併的底稿）、`README_MACHINE_AI.md`（這一版的內容與步驟） |

| 版本 | 值 |
|---|---|
| BASE（機台上次合進去的筆電版本） | 見 `_machine_ai/README_MACHINE_AI.md` |
| REV（這一包對應的 GitLab main） | 見 `_machine_ai/package_manifest.tsv` 第一行 |

## 更新包清單（照順序套，一包套完、commit 之後才套下一包）

| 順序 | 位置 | 對應 GitLab main | 相對（底稿） | 內容 |
|---|---|---|---|---|
| 1 | 根目錄 `HT9045/`＋`_machine_ai/` | `56bbf785` | 機台上次合進去的 `66cb14e0`（`_machine_ai/base_66cb14e0/`） | 合併包（取代 USB 第二、三包），277 檔 |
| 2 | `updates/410d27d9/` | `410d27d9` | `56bbf785`（`updates/410d27d9/_machine_ai/base_56bbf785/`） | 網頁權杖卡住（Motor Test 拿了不還，IO 頁按 Output 被擋 10 分鐘），4 檔 |

每一包各自有 `_machine_ai/README_MACHINE_AI.md`（內容與步驟）和自己的 `check_and_copy.ps1`。舊包不會被刪掉，`git pull` 不會讓正在套的那一包消失。

## 機台第一次下載

```
git clone https://github.com/HPI-Jimmy-Chiu/HT9050.git D:\HT9045\_from_github
```
這個 repo 是公開的（使用者 20260926 裁決：否則機台連不進來），clone／pull 不需要帳密。

## 之後每次更新

```
cd /d D:\HT9045\_from_github
git pull
```
接著**照 `_machine_ai/README_MACHINE_AI.md` 的步驟**用 `check_and_copy.ps1` 套到機台整合樹（`D:\HT9045\_integ_ioweb`）：
先 `-Mode Check`（唯讀）→ EastSun 同意 → 備份 → `-Mode Apply`。**不要直接複製覆蓋**，機台有自己的修改，要靠三方合併。

## 安全

- 權杖、7z 密碼都**不可以**寫進這個 repo、commit 訊息或任何檔案。
- 這個 repo 是**公開**的：任何人都能下載。所以只放「更新包內容」，不放 GitLab 的完整歷史；權杖、密碼、客戶資料一律不放。
- 機台端推回來的內容放 orphan 分支 `machine/integ-ioweb`（format-patch），同樣只放機台自己的 commit。
