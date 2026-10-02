# 三份檔的格式與例子

> 例子取自 `D:\docs\ChangeLog\CHANGES_20260926_Steven.md`、`D:\docs\ops\daily\20260926.md`、
> `D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md` 20260927 14:2x 的內容，有縮短。
> §11.44 以前的例子是舊寫法（相對路徑、github-xx），照格式學，路徑與角色名照 references/standing-rules.md §3、§4 的新規則寫。

## 1. 只讀指令（記錄員可以跑的）

| 要什麼 | 指令 |
|---|---|
| 本輪範圍 | `git -C /d/HT9045 log --first-parent --format="%h %ad %an %s" --date=format:%H:%M <上一輪終點>..<本輪終點>`（跨日改 `--date=format:"%m-%d %H:%M"`） |
| 顆數 | `git -C /d/HT9045 rev-list --first-parent --count <上一輪終點>..<本輪終點>` |
| 一顆 commit 改了哪些檔 | `git -C /d/HT9045 show --name-only --format= <hash>` |
| 合併 commit 帶進哪些 commit／哪些檔 | `git -C /d/HT9045 log --oneline <merge>^1..<merge>^2`、`git -C /d/HT9045 diff --name-only <merge>^1 <merge>` |
| 推送狀態 | `git -C /d/HT9045 rev-list --count origin/v906/steven-cbridge-review6..HEAD`、`git -C /d/HT9045 rev-list --count HEAD..origin/main` |
| 核對 ST01-M 給的交接 hash | `git -C /d/HT9045 log origin/v906/steven-handoff --format="%h %ad %s" --date=format:"%m-%d %H:%M" -20` |
| 新的 S 編號在 RULINGS 哪一行 | `grep -n "^| S160 " /d/HT9045/HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20260926.md` |

`--first-parent` 只列本分支自己的 commit 與合併 commit，main 帶進來的個別 commit 不會出現——§11 就是照這個粒度寫（「St01 合併 `origin/main` N 顆（`hash`）」再列帶進了什麼）。

## 2. ChangeLog §11 摘要表（每輪加一列）

**規則**：§11 開頭 `## 摘要` 那張表（欄：`# | 項目 | 結論 | 節`）最後加一列，# ＝新節號 NN。
「項目」＝`commit 首～尾（09-27 hh:mm～hh:mm，N 顆）：` 加粗體重點；「結論」＝有沒有衝突、有沒有工程師交件、有沒有新裁決、Steven 有沒有新回覆，再加一兩句最要緊的；「節」＝`§11.NN`。

例（第 44 列，縮短）：

```
| 44 | commit `65e1d66b`～`519b931a`（09-27 05:52～06:3x，7 顆）：St02 **S119** 併入 main → done `E-104`；**St01 NUMCMP**（Contact 頁 X／Y Dimension 判斷數值化）；**St01 S119 R1 裝好** | 無衝突；沒有工程師交件、沒有新裁決，Steven 沒有新回覆；NUMCMP 是 Jimmy／NB2 R89 發現的移植缺陷，St01 兩處已改（`8086d0f3`） | §11.44 |
```

## 3. ChangeLog §11.NN（每輪一節）

**位置**：最後一個 §11.NN 之後、`# §12. 目前狀態` 之前；節與節之間用一行 `---` 隔開。節號接上一節，不跳號、不重編。

**標題**：`## 11.NN commit `首`～`尾`（09-27 hh:mm～hh:mm，N 顆）：重點、重點、重點`。重點只放範圍內的 commit；範圍外的東西放進標題是錯的（§11.40 標題原本列了屬於 §11.39g 的 S94，事後 ⛔ 更正拿掉）。

**來源行**（標題下第一行，引用區塊）：

```
> 來源：`git -C /d/HT9045 log --first-parent --format="%h %ad %an %s" --date=format:%H:%M 45ae30fe..519b931a`（7 顆）。這段沒有工程師交件、沒有新裁決，Steven 沒有新回覆。
```

有 ST01-M 的項目時補「＋ ST01-M 給的交接項目（最後一節）」；有 ST01-E 交代的特殊事項（例：本節起改用絕對路徑）也寫在這裡。三句「工程師交件／新裁決／Steven 新回覆」有就寫有什麼，沒有就寫沒有，不要省略。

**小節**：`### a. 主題（`hash` hh:mm）`，照時間排；同一件事的幾顆 commit 可以併一節（`（`5d58c98d` 21:51）＋todo 登記（`d6daa8fc` 21:52）`）。內文寫：做了什麼、依哪個裁決（題號＝選項（S 編號））、主要檔案絕對路徑、驗證狀態、還剩什麼（指到 §12.x）。

例（§11.46l）：

```
### l. **R15＝B、Q31＝A′ 落地**（`725038a6` 13:53）

`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`：開機跑 golden `TfContactForce` 建構子（**R15**＝B，S162）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp`／`AOISetup.gen.inc`：修 `AOI.Data` Top 延遲／`Timeout` 鍵的寫入端（**Q31**＝A′，S152，906 單邊修 golden 的兩個既有錯誤）。只做語法檢查，未 build。
```

幾種常見小節：

| 類型 | 寫法 |
|---|---|
| 合併 main | `### e. St01 合併 `origin/main` 2 顆（`af169169` 06:2x）`＋帶進誰的什麼（Jimmy NUMCMP 其餘同行修改 `3f1882fc`…）、有沒有衝突、衝突怎麼解、語法檢查結果 |
| ST01-M 在共用工作樹的代登記 | `### d. ST01-M 代登記：E-011 R1 說明＋新增 H-027（`151a84b0` 06:1x）` |
| 記錄員上一輪的 rulings-index commit | 一行：`### x. 記錄員上一輪的 rulings-index（`23efc733` 14:25，ST01-E 核對後 commit）` |
| ST01-M 的交接動作（放最後） | `### q. **ST01-M 這段的交接動作**（同一時段，hh:mm～hh:mm，分支 `v906/steven-handoff`，非本分支 commit）`，條列：FROM_STEVEN 轉達（時間、對象、內容、hash）、代編結果、代登記 |

## 4. ⛔ 的六種寫法

| 用途 | 寫法 | 真實例子 |
|---|---|---|
| 事實寫錯（§1～§11 行內） | `（⛔更正：…為準）` 緊接在錯字後面 | `Steven 10:5x（⛔更正：以 commit `a5a95fb6` 10:25 為準）兩題新裁決` |
| 事實寫錯（ST01-E 核對出來的） | 另起一行 `⛔ 更正（ST01-E hh:mx 核對）：上一版寫「…」，錯；正確是…` | `⛔ 更正（ST01-E 14:2x 核對）：上一版寫「只剩 Q34 與 Q44」，漏了 R60～R72。` |
| 當時對、後來變了 | `⛔ 更新（時間或 hash，見 §x）：…` | `⛔ 更新（見 e 段，`ed3a0ed1`）：這題隨後改編成 **W33**，**W32** 讓給 St02 ESD G5。` |
| §12.5 條目做完了 | `⛔已完成（hash，時間）：~~舊字~~ 現況` | `⛔已完成（`217e7e5e`，09-26 23:34）：~~**S108～S110** 三支表單雖歸 St01…~~ 要交 Jimmy 的通訊部分已經轉達。` |
| §12.5 條目不用做了 | `⛔已結案（誰 時間 核對）：~~舊字~~ 理由` | `⛔已結案（主 session 09-27 核對）：~~S112／S114／S116 交給 Jimmy：…~~ clock.text 由 St01 照 golden 格式改（`b40b140f`）…` |
| 日報同一輪範圍延伸 | 另起一段 `⛔ 補充（同一輪，範圍延伸到 `hash`，hh:mm～hh:mm 多 N 顆）：…` | `⛔ 補充（同一輪，範圍延伸到 `9f6c178c`，14:06～14:07 多兩顆）：**R68** 落地…` |

日報寫錯的更正放在那一段後面另起一行：`⛔更正：本行寫「不變」有誤，S119 這輪其實從 📝 改 ✅，當時進度表已同步改成 ✅68／⏳7／📝17／❓9／➡10／—11（合計 122），這裡漏改，特此補記。`

## 5. ChangeLog §12

範本與每塊的組法見 references/section12-template.md。`# §12. 目前狀態（每小時／每段落更新；只保留最新一份）` 標題和下面的「查法」引用區塊不動。

## 6. 日報（`D:\docs\ops\daily\20260926.md`）

**規則**：檔尾加一段，前面空一行；只加不改。格式：

```
**09-27 HH:MM 更新**（範圍 `上一輪終點..本輪終點`，N 顆，09-27 hh:mm～hh:mm）：<粗體重點>……。<有沒有工程師交件／新裁決／Steven 新回覆>。已推送，`origin/v906/steven-cbridge-review6..HEAD`＝N，`HEAD..origin/main`＝M。詳見 ChangeLog §11.NN、§12（本輪改了哪幾塊）；裁決進度表狀態計數 ✅a／⏳b／📝c／❓d／➡e／—f（合計 T）。
```

- `HH:MM` 用本輪最後一顆 commit 的時間（或 `hh:mx`）。
- 狀態計數從 rulings-index.md 表頭**重新抄**，不要沿用上一段的數字（13:58 那段寫「狀態計數詳見該檔」也可以，但寫數字時一定要重算過）。
- 檔頭「一句話」「今天真正重要的 4 件事」「待辦」等前段不動；那是日報開檔時寫的（格式權威 `D:\.github\prompts\ops-daily-worklog.prompt.md`；那份寫的範本 `D:\docs\ops\templates\daily-worklog.tmpl.md` 20260927 查的時候不存在，開新日報時照 `D:\docs\ops\daily\20260926.md` 的前段排）。

例（13:58 那段開頭，縮短）：

```
**09-27 13:58 更新**（範圍 `46e2d7f9..de534b23`，14 顆，09-27 13:08～13:58）：**角色改名**——從這輪起文件一律寫 **ST01-M**（Steven01-Manager，協調登記，原 github-02）／**ST01-E**（Steven01-Engineer，工程線）……**R15＝B**（開機跑 golden `TfContactForce` 建構子，開機補 `ContactInfo.ini` 64 段 128 鍵）……沒有新裁決（都是先前決議落地）。已推送，`origin/v906/steven-cbridge-review6..HEAD`＝0，`HEAD..origin/main`＝0。詳見 ChangeLog §11.46、§12（四塊全部更新）……
```

（這段原文的 `ContactInfo.ini` 沒寫路徑；照新規則要寫 `D:\HT9045\system\ContactInfo.ini`。）

## 7. rulings-index.md（`D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md`）

表頭（說明、連結深度、編號規則）不動，只改下面四區。

### 7.1 最後更新（每輪整段重寫）

```
**最後更新**：20260927 14:07（以 commit `9f6c178c` 為準；`46e2d7f9..9f6c178c` 這批（16 顆，09-27 13:08～14:07）是**重點**、**重點**……，見 ChangeLog §11.46；`origin/v906/steven-cbridge-review6..HEAD` = 0；`HEAD..origin/main` = 0（追上了）。
```

### 7.2 狀態計數（每輪重算）

```
**狀態計數**：✅ 已完成 70　⏳ 進行中 7　📝 待辦 17　❓ 待查 9　➡ 交給 Jimmy 10　— 不用做 11　（合計 124；本輪為既有列補記進度，未新增／刪除列）
```

- 依主表「狀態」欄的**第一個符號**算：✅ 包含「已完成」「生效中」「已完成（大部分已落地）」；⏳ 包含「進行中」「第一步完成」；📝 包含「待辦（…）」「待派」；➡ 包含「交給 Jimmy（…）」「Steven 另一 session（…）」；— ＝不用做。合計＝主表列數。
- 括號裡寫本輪新增幾列（哪幾列）或「未新增／刪除列」。
- 重算指令見 references/checklist.md D 段。

### 7.3 主表

欄：`# | 題目 | 裁決（短，原話） | 狀態 | 最後更新 | 原文 | 詳細內容`。

- 狀態變了：改「狀態」與「最後更新」（YYYYMMDD），「詳細內容」後面補 `commit `hash`（時間，未 build）：做了什麼（絕對路徑）；見 [0926 §11.NN](<../../../../../docs/ChangeLog/CHANGES_20260926_Steven.md>)`。
- 新的 S 編號：加一列，「原文」連到 RULINGS 那一行：`[RULINGS_20260926:L399](<../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20260926.md#L399>)`（行號用 §1 的 grep 查）。
- 一大批一起裁的（例：S123～S163 共 41 題）曾經併成一列、細節指到 decisions-decided.md；要不要拆列照 ST01-E 交代（待確認的做法，見 references/checklist.md B 段第 10 條）。
- 筆電寫的 RULINGS_20260927 目前只有 R927 一列（涵蓋第 1～4 條）；之後新增的條目要不要進本表，照 ST01-E 交代。

例（S119 列，縮短）：

```
| S119 | Main.Record 分頁（三個顯示區、CLEAR、Save Log） | 靜態 | ✅ 已完成 | 20260927 | [RULINGS_20260926:L312](<../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20260926.md#L312>) | 改歸 **St02** 完成……**R1 已由 St01 裝上**（`519b931a`，`wb_serve.cpp:4111` 接上 S113 的 `W906_TfMain_UpdateRecordScreen`，語法 0／101），見 [0926 §11.44](<../../../../../docs/ChangeLog/CHANGES_20260926_Steven.md>) |
```

### 7.4 最近變動（最多 20 行，最新在最上面）

**規則**：`## 最近變動` 下面「（最新的放最上面，只保留最近 20 行）」之後，最上面加 1 行、刪掉最下面 1 行，加完數一次是 20。每行格式：

```
* YYYYMMDD HH:MM（以 commit `終點` 為準，範圍 `起點..終點` 共 N 顆，09-27 hh:mm～hh:mm）— <本輪摘要：做了什麼、哪些列狀態從什麼改成什麼>。<工程師交件／新裁決／Steven 新回覆>。已推送；`origin/v906/steven-cbridge-review6..HEAD`＝N，`HEAD..origin/main`＝M。詳見 ChangeLog §11.NN；狀態計數 ✅a／⏳b／📝c／❓d／➡e／—f（合計 T）。
```

例（06:3x 那行，縮短）：

```
* 20260927 06:3x（以 commit `519b931a` 為準，範圍 `45ae30fe..519b931a` 共 7 顆，09-27 05:52～06:3x）— St02 **S119** 併入 main（`0faa75be`）→ done `E-104`……S119 由 📝 往後排 改 ✅ 已完成。沒有工程師交件，沒有新裁決，Steven 沒有新回覆。已推送、與遠端一致。詳見 ChangeLog §11.44；狀態計數 ✅68／⏳7／📝17／❓9／➡10／—11（合計 122）。
```

被刪掉的最舊一行不用搬到別處（ChangeLog §11 有完整紀錄）；交件時寫出刪掉的是哪一行（開頭的時間）。
