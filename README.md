# HT9050 機台更新用 repo（私有）

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

## 機台第一次下載

```
git clone https://github.com/HPI-Jimmy-Chiu/HT9050.git D:\HT9045\_from_github
```
登入視窗選 **Token**，貼上機台專用的唯讀權杖（Contents: Read-only、只限這個 repo）。

## 之後每次更新

```
cd /d D:\HT9045\_from_github
git pull
```
接著**照 `_machine_ai/README_MACHINE_AI.md` 的步驟**用 `check_and_copy.ps1` 套到機台整合樹（`D:\HT9045\_integ_ioweb`）：
先 `-Mode Check`（唯讀）→ EastSun 同意 → 備份 → `-Mode Apply`。**不要直接複製覆蓋**，機台有自己的修改，要靠三方合併。

## 安全

- 權杖、7z 密碼都**不可以**寫進這個 repo、commit 訊息或任何檔案。
- 這個 repo 必須保持 **Private**。
