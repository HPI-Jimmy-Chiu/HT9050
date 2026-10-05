# 長期規則（每輪都套用）

> 來源：記錄員狀態檔（暫存，20260927 13:0x～14:2x 的版本）的「常駐規則」「重要新規則」「重要教訓」「跨日規則」「ST01-E 13:3x 核對更正」；
> Claude memory 的 changelog-location-convention、docs-use-absolute-paths、reports-use-names-not-pronouns、usage-95-wind-down、
> steven-away-keep-working、ht9045-steven-machines；ChangeLog `D:\docs\ChangeLog\CHANGES_20260926_Steven.md` §11.30、§11.45、§11.46。
> 從本檔建立起，以本檔為準；暫存檔不再是規則來源。

## 1. 權限與範圍

**規則**：記錄員只改三份檔——`D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md`（20261005 17:0x 起；之前 `D:\docs\ChangeLog\CHANGES_20260926_Steven.md`）、`D:\docs\ops\daily\20260926.md`、
`D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md`。

- git 只用只讀指令：`log`、`show`、`diff`、`rev-list`、`status`、`ls-files`。**不** add／commit／push／fetch／pull／checkout／stash／merge／reset（fetch 由 ST01-E 在派工前做）。
  ⛔ 20261004 22:5x 澄清（記錄員 §11.99 ④-9）：記錄員**不 fetch**——遠端 ref 用 ST01-E 派工前 fetch 的那一份（派工訊息寫 fetch 時刻）；checklist C-55／C-64／C-86 說的「最後一步讀 `origin/main`／`origin/v906/steven-handoff`」＝讀本機已有的 `origin/*` ref，不是自己 fetch。要更新的遠端狀態寫進 ④ 請 ST01-E 核，不要自己動 ref（別的 session 同時 fetch 時也會讓 ref 變動，C-81 寫範圍一律用派工訊息寫死的終點 hash）。
- 不 build、不跑 ctest、不跑 wb_serve（會改機台真檔）、不開子代理、不寄信。
- 不改 decisions-pending.md／decisions-decided.md／todo.md／done.md／RULINGS_*.md／FROM_STEVEN.md／任何 skill（包括本 skill）；發現那些檔有錯，寫進交件第 ④ 項給 ST01-E。
- `D:\docs\ops\registers\HT9045_裁決進度表.md` 20260927 13:0x 起凍結（ST01-M `eada7571` 搬進 skill），不再改。
- rulings-index.md 由 ST01-E 維護；ST01-M 不改內容；記錄員寫完由 ST01-E commit。

例：ST01-E 的派工訊息附了一段 ST01-M 的文字「14:20 FROM_STEVEN §4 回報 St02 代編結果」——記錄員把它寫進 ChangeLog §11.NN 的 ST01-M 小節，不去動 FROM_STEVEN.md，也不去 `git log origin/v906/steven-handoff` 以外的地方找（只讀核對 hash 可以）。

## 2. 只增不改與整段覆蓋

**規則**：

| 區 | 做法 |
|---|---|
| ChangeLog §1～§11、日報全部 | 只增不改。舊字有錯，在原處加「⛔ 更正（誰 時間 核對）：…」；狀態後來變了，加「⛔ 更新（時間，見 §x）：…」。不刪原字、不重新編號 |
| ChangeLog §12.1～§12.4 | 每輪整段重寫（「只保留最新一份」）。12.1 只詳述最近一輪，更早的壓成一句話指回 §11 對應節 |
| ChangeLog §12.5 | 累加，只加不刪。新條目加在最上面、標 🆕；結案的在原處標「⛔ 已完成（hash，時間）」或「⛔ 已結案（誰 時間 核對）」，舊字用 `~~刪除線~~` 保留 |
| rulings-index 表頭「最後更新」「狀態計數」 | 每輪重寫 |
| rulings-index 主表 | 有狀態變化的列原地改「狀態」「最後更新」，「詳細內容」往後補（commit、時間、絕對路徑、ChangeLog 節）；新 S 編號加新列 |
| rulings-index「最近變動」 | 頂端加 1 行、砍最舊 1 行，維持 20 行 |

- §12 整段重寫時，上一版裡 ST01-E 加的「⛔ 更正」註記可以不留（§12 只保留最新一份），但**更正後的內容必須反映在新版**，而且不能把更正前的錯字抄回來。
- §12 要從來源（派工訊息、decisions 檔、git）重新組，不是拿上一版小修；上一版的每一條都要重新確認還成立（例：14:2x ST01-E 核對出上一版沿用「Q40 等 Jimmy 決定送出點」，其實 Jimmy 已同意）。

例（§1～§11 原地補註，§11.13 摘要列）：`Steven 11:5x（⛔更正：以 commit `a42f2850` 10:50 為準）：把 S52「讀寫檔移植」拆成一檔一頁一個編號`。

## 3. 路徑

**規則**：三份檔新寫的內容，檔案引用一律寫絕對路徑，並看得出是哪一棵樹（Steven 20260927：「你把路徑變成絕對路徑，不然我找不到文件」；「git 題號如果有文件的，也要給我絕對路徑」）。舊內容不用回頭改，ST01-E 另外交代才改。

| 樹／資料夾 | 絕對路徑開頭 |
|---|---|
| 移植樹（C++，UTF-8） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` |
| golden V912（BCB6，cp950） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` |
| golden V899（BCB6，少用） | `D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\` |
| 網頁 | `D:\HT9045\web\page\` |
| skill | `D:\HT9045\.claude\skills\<名字>\` |
| **機台資料夾（不屬於任何一棵樹）** | `D:\HT9045\system\`、`D:\HT9045\config\`、`D:\HT9045\IniData\Data\<配方>\`；log 在 `D:\HT9045_Log\` |
| 交接唯讀快照 | `D:\HT9045_handoff\` |
| 906_0625_Steven 那棵 | 只在 St02 那台，St01 這台引用時要寫明 |

- **git 路徑轉絕對路徑**：`git -C /d/HT9045 show --name-only --format= <hash>` 印出的是相對 repo 根目錄的路徑，前面加 `D:\HT9045\`、`/` 換 `\`。例：`HT9011UC_Cpp_V3.33.906.0/WebLevelSet.cpp` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp`；`web/page/ht9045_wire_statussecurity.js` → `D:\HT9045\web\page\ht9045_wire_statussecurity.js`。
- **程式碼裡的 `system\xxx`、`config\xxx` 是機台資料夾**，不是移植樹的子資料夾。例：R15 開機補鍵的檔是 `D:\HT9045\system\ContactInfo.ini`；寫成 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\system\ContactInfo.ini` 是錯的（那個資料夾不存在）。
- 不省略、不猜子資料夾：寫 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1620-1646`，不寫 `...\SCK_ART.cpp`、不寫 `HT9011UC_Code_V3.33.899.0_...`。不確定在哪就用 `find`／Glob 查，查不到寫「待確認」。
- commit hash 後面接主要檔案的絕對路徑（一顆 commit 超過 4 個檔，列最主要的 2～4 個再寫「等 N 個檔」）；「見 commit 本文」不夠，要把重點寫進來。
- 一條路徑放在同一行、同一對反引號裡，不要在 `\` 後面換行（ChangeLog 是手動折行的，路徑被折斷就點不開）。
- rulings-index 的 markdown 連結用相對路徑（VS Code 點得開）：`../../../../` ＝ `D:\HT9045\`、`../../../../../docs/` ＝ `D:\docs\`（不要再用舊檔的 `../../../HT9045/` 寫法）；連結以外的文字照樣寫絕對路徑。

- **§12.4 與任何轉給 Steven 的題目**（Steven 20260927 21:xx：「給我的決策文件裡面，相關的檔案都要使用絕對路徑，不要使用代號；功能也是要白話說明，淺顯易懂的方式」）：除了路徑寫全，也不要只寫代號（TS-8、CC-E2、D-6a、HTSET,354…）——先白話講是什麼、操作員看到什麼，代號放括號；題號 Q／R／W 保留。ChangeLog 其他段落的讀者是 Jimmy，可以照舊用代號，但第一次出現時仍附一句白話。

## 4. 人名與角色名

**規則**：用人名，不用「你／我／他／他們／對方／我們」；指角色時寫角色名。

| 寫 | 不寫 |
|---|---|
| Steven、Jimmy、Jimmy 的筆電（簡稱「筆電」）、EastSun（機台端） | 你、我、他、對方 |
| St01（Steven01，主機名 Steven-NB）、St02（Steven02，STEVEN-NB3） | 本機、那台 |
| ST01-E（Steven01-Engineer，工程線）、ST01-M（Steven01-Manager，協調登記）、ST02-E／ST02-M | github-02、github-70 等 session 名（重開就換） |

- 例外：引用原話保留原字（例：Steven「我明天會直接看 todo.md 的內容」）；說明改名歷史時可以寫「ST01-M（原 github-02）」。
- git 的 author 全都是 Steven，看不出是 ST01-E、ST01-M 還是 St02 的 commit：以 ST01-E 派工訊息的說明為準；commit 訊息看得出來的（例：`ht9050-construction: todo H-029 …（St02 014e9094 …）` 是 ST01-M 代登記）照寫；看不出來就寫 St01 並標「待確認」。

## 5. 時間

**規則**：時間一律以 commit 時間（`%ad`）為準，不用 session 的時鐘。

- 20260926 主 session 沒看時鐘，10:00 以後給記錄員的時段標籤偏晚 40～60 分鐘，事後全部用 ⛔ 更正對回 commit 時間（ChangeLog §11.30，`fee09aac`）。
- `13:0x` 表示 13:00～13:09；`13:xx` 表示那個小時內、分鐘不確定。
- Steven 下指示的時間找不到直接對應的 commit：寫「約 HH:MM 前」並註明依據哪兩顆 commit 推估。例：`12:0x（⛔更正：無直接 commit 可查，依前後 commit 0b1f4204 12:13／c913d5e5 12:26 推估約 12:1x 前）`。
- 跨日：20260926 21:35 起 commit 已經是 20260927，ChangeLog 與日報**檔名維持 20260926**，內文時間一律標「09-27 hh:mm」避免跟 26 日混淆；範圍指令跨日時改用 `--date=format:"%m-%d %H:%M"`。
- 什麼時候開新一天的檔（新的 `CHANGES_YYYYMMDD_Steven.md`、新的日報）：由 ST01-E 在派工訊息裡說；沒說就繼續寫同一份（新檔的檔頭格式照 `D:\.github\skills\make-report-skill\references\change-log\change-log.md` 與 `D:\.github\prompts\ops-daily-worklog.prompt.md`）。

## 6. 題目與裁決

**規則**：每個題號寫進三份檔之前，都要到來源檔對過「題目是什麼、現在是待決還是已決、最新選的是哪個」。

| 要查的 | 去哪查 |
|---|---|
| 還要 Steven 回的 | `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`（`### Q..`、`### R..`、`### W..` 標題） |
| 已裁決／已照建議做的 | `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`（「目前狀態」那行） |
| 原話與 S 編號 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`；筆電寫的第 N 條在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` |

- 題號意思：**Q**＝St01 要 Steven 決定；**R**＝St01 已照建議先做、Steven 可推翻；**W**＝St02 的題（ST01-M 代登記）；**S**＝RULINGS_20260926 的裁決編號；**#N／第 N 題**＝RULINGS_20260927 第 2 條表格或 NIGHT_REPORT §0 的題號。
- 寫法：`題號＝選項（S 編號）＋一句意思`。例：`**R15＝B**（S162，ContactForce 建構子開機就跑）`、`**Q4＝B**（S160 更正 S126：先把三頁搬到第二型再退役第一型）`。
- **R 題在 decisions-pending.md「已照建議做、但還要 Steven 點頭的 R」裡的，一律寫「已照建議先做（可推翻）」**，不寫「定案」「已裁決」。
- **重選**：同一題後來又裁一次，以最新的為準，並寫出更正關係（S160 更正 S126）。
- **改號**：ST01-M／St02 常先給一個編號、之後又改（例：W32 原本是 SIM 實跑，`ed3a0ed1` 改編成 W33，W32 讓給 St02 ESD G5）。以寫的當下 decisions 檔為準；事後改號用「⛔ 更新」補註。
- **來源互相矛盾**：寫「待確認」，把各處位置列進交件第 ④ 項，不要自己挑一邊。
- 不要寫「見原文，本節不重複列」：寫一句重點＋原文的絕對路徑。
- todo.md 的「★ 待 Steven 決定」節已在 20260927 10:34 拆成 decisions-pending.md／decisions-decided.md（`62ce440a`），不要再去 todo.md 找。

## 7. 狀態與派工

- §12.2 的工程師名單只寫 ST01-E 派工訊息說「在跑」的；「預定要派」寫「預定」，不要寫成已派（14:2x 核對：上一版寫「加派 S92／S91／S114 三位」，其實沒派）。
- 驗證狀態照實寫：「只做語法檢查」「未 build」「ctest 13/13 通過（模擬組態）」「出貨組態 ctest 在跑」。RULINGS_20260927 第 4 條：0927～0928 兩組態全量 gate＋該項模擬驗證通過就算完成。
- 推送狀態寫寫的當下的值：`origin/v906/steven-cbridge-review6..HEAD`＝N、`HEAD..origin/main`＝M（ST01-E fetch 過的 ref；記錄員不 fetch）。
- rulings-index 主表有實質 S 編號列的，狀態變了要改那一列並重算計數，不是只改 §12 的文字。
- 「狀態計數不變」只能在重算之後寫（06:3x 那輪照抄上一輪數字寫錯，見 references/checklist.md C 段第 8 條）。

## 8. 寫檔技術

- 編碼一律 UTF-8 無 BOM；行尾保持原樣：ChangeLog、日報是 LF；rulings-index.md 在工作樹是 CRLF（git 裡是 LF，`core.autocrlf=true`）。寫前先看原檔行尾，寫回同一種。
- 優先用 Edit 工具。要用腳本改，腳本檔用 Write 工具寫，**不要用 Bash heredoc**：heredoc 會把 `\\` 收成 `\`（20260927 弄斷過一列表格；建本 skill 時測試腳本也被收掉一個反斜線而語法錯誤）。腳本裡的反斜線用 `chr(92)` 組。
- 腳本先在記憶體 encode 成 bytes，成功才用 `'wb'` 開檔寫（`open(path,'w')` 一開就截斷）。
- Python 用 `/c/Users/steven/AppData/Local/Programs/Python/Python314/python.exe`；PATH 上的 `python` 是 Store 殼，exit 49、沒有任何輸出。
- 寫完查控制字元（指令見 references/checklist.md D 段）；20260927 03:2x Jimmy CTRLFIX 還原過 6 個檔 23 處被吃掉的反斜線。
- ST01-E 在同一輪中途補充更正時，三份檔都要回頭同步，不是只改 ChangeLog。

## 9. 不確定就寫「待確認」

查不到、兩邊說法不一、ST01-E 沒交代的，寫「待確認」並在交件第 ④ 項列出來，不要憑印象補。

## 10. St02 的日報與 ChangeLog 也由這一份合併（Steven 20260927 19:3x）

- Steven 對 St02 說「日報可以推給 st01-m 幫你寫」「Change log 也推過去」⇒ St02 只把內容寫在交接分支 `v906/steven-handoff` 的 `docs/handoff/ST02_DAILY_<YYYYMMDD>.md` 與 `docs/handoff/ST02_CHANGELOG_<YYYYMMDD>.md`，每次更新在 CHAT_ST02 留一行；ST01-M 巡檢時看到變動就通知 ST01-E。
- 記錄員**每一輪**都要讀這兩份（`git -C /d/HT9045 show origin/v906/steven-handoff:docs/handoff/ST02_DAILY_<YYYYMMDD>.md`，唯讀快照也可看 `D:\HT9045_handoff\`），把新內容併進 ChangeLog 與日報：
  - ChangeLog：§11 新一節裡另開小節「St02（ST02-E）」，照 St02 給的內容寫（附 St02 的 commit hash 與分支名 `v906/steven-gpib-widget`／`v906/steven-st02-on-cbridge`），不改寫 St02 的結論；§12.5 有給 St01 或 Jimmy 的事才加。
  - 日報：同一個「09-27 HH:MM 更新」段裡加一小段「St02」。
- 只併「上一輪之後新增的部分」：記下這一輪讀到的最後一段標題或時間（寫進記錄員狀態檔），下一輪從那裡接。
- St02 的內容是 St02 的說法：記錄員不自己查 St02 分支的程式；只核對 commit hash 存不存在（`git -C /d/HT9045 cat-file -t <hash>`）。

## 11. ChangeLog 與日報一天一份（Steven 20260928：選「開 0928 新檔」＋「每天一份呀！」）

- ChangeLog：`D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md`（20261005 17:0x 起，§14；之前 `D:\docs\ChangeLog\CHANGES_YYYYMMDD_Steven.md`）；日報：`D:\docs\ops\daily\YYYYMMDD.md`。**每天開一份新檔**，當天的 §11 小節、§11 摘要表的列、日報「HH:MM 更新」段都寫在當天那份。
- §12「目前狀態」只放在最新一天的檔；前一天的檔在 §12 的位置留一行「目前狀態見 <下一天的檔> §12」。
- 小節編號（§11.NN）跨檔連號、不重來，舊的引用才對得到。
- 跨午夜的輪次看範圍的**結束時間**歸哪一天。
- 開新檔時標題寫日期與讀者（Jimmy），並加一行「前一天：…、下一天：…」；舊檔尾加一行指向新檔。
- 20260928 20:xx 起把原本 0926 檔裡 0927、0928 的內容拆到 `CHANGES_20260927_Steven.md`／`CHANGES_20260928_Steven.md`、`20260927.md`／`20260928.md`（rulings-index 的連結一起改）。

## 12. Steven 的日報 `D:\docs\ops\daily\YYYYMMDD.md` 改由 ST01-M 編輯（Steven 20260929 11:1x，經 ST01-E2：「日報一律通報 ST01-M做內容編輯」）

- 20260929 起，**記錄員不再寫 `D:\docs\ops\daily\YYYYMMDD.md`**（不新增「HH:MM 更新」段、不改舊段）。ST01-M 是唯一編輯者，每 2 小時（在 :17）依各方回報寫一次、一天一份，當天結束經 ST01-E2 同步到 portal。
- 記錄員照舊寫：ChangeLog `D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md`（20261005 17:0x 起，§14；§11 小節、摘要表、§12）、repo 日報 `D:\HT9045\docs\ops\daily\YYYY-MM-DD.md`、`D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md`。
- 記錄員交件後，ST01-E 把摘要轉給 ST01-M（ST01-M 拿去寫 Steven 日報）；Steven 20260929「記得通知每個小弟要寫日報回饋給你」。
- 上面 §11 寫的「日報一天一份」仍適用於 ST01-M 寫的那份；本 skill 其他地方提到寫 `D:\docs\ops\daily\` 的步驟，一律照這一節略過。
- 時間：Steven 訊息的時刻以 git commit 時間與 ST01-M 更正過的紀錄為準（20260929 ST01-M 11:0x～11:3x 的標籤快了 30～60 分鐘，已更正）。
- ⛔ 20260929 15:2x 再更新（Steven「今天的日報 … 要改寫到 D:\RD5-Portal\public\Docs\Daily\Steven 裡面」）：Steven 的日報改放 portal repo `D:\RD5-Portal\public\Docs\Daily\Steven\YYYYMMDD.md`（ST01-M 寫、ST01-E3 管 portal 並 push）；`D:\docs\ops\daily\YYYYMMDD.md` 只剩指向那份的短檔，寄信存檔仍放 `D:\docs\ops\daily\`。記錄員**兩邊都不寫**。

## 13. 子代理數量上限：每個 session 同時最多 3 個（Steven 20261004 11:0x「一次最多開3個agent」）

- 記錄員本身就是一個子代理，算進 ST01-E 的 3 個裡。ST01-E 已經有 3 個工程師在跑時，記錄員**等一個工程師交件、名額空出來再派**；不要為了趕整點先停掉工程師。
- 整台電腦 7 個的上限（Steven 20260928）照舊，兩條同時遵守。
- 記錄員自己不開子代理（§1 原本就不准），這條不變。
- 等名額時，ST01-E 先把本輪輸入寫成 `D:\AI_TempFile\st01e-clerk-st01m-HHMM.txt`（ST01-M 的文字原文＋ST01-E 的項目），名額空出來就照 dispatch-and-handin.md §1 派，範圍終點寫派工當下 fetch 到的 hash。

## 14. ChangeLog 改放入口網站 repo（Steven 20261005 17:0x，在 St02-E 那邊裁決，St02-M 經 CHAT_ST02 `d68810ad1` 轉、ST01-M 17:5x 轉 ST01-E）

**規則**：
- ChangeLog 不再寫 `D:\docs\ChangeLog\`，改寫入口網站 repo：St01＝`D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md`；St02 用 `Steven02\`（`build_portal.py` 以資料夾＋日期當鍵，兩邊不能同名）。
- 格式、§11／§12 規則都不變；舊檔 `D:\docs\ChangeLog\CHANGES_*_Steven.md` 保留不刪、不再改。
- 記錄員照樣只改檔、不做 git；**ST01-E 每天推一次入口網站 MR**（一天一個分支，例 `st01/changelog-YYYYMMDD`）：`node tools/md2html.js <md>` 產生同名 html → `py tools/build_portal.py --ci` 通過 → commit md＋html → push 開 MR（不加 auto-merge；ST01-M 經 Steven 同意後用 GitLab API 合）。推之前掃一次密碼／權杖／金鑰（`public/` 合進 main 後全公司看得到）。
- 入口網站 MR 還沒合、`D:\RD5-Portal` 還沒有當天的檔時：記錄員先寫 `D:\docs\ChangeLog\` 那份，ST01-E 推 MR 時一起搬過去。
- 第一批：0929～1005 搬上去是入口網站 MR !181（`st01/changelog-20261005` `8506650`，0926～0928 早就在 main 而且內容相同）。
- 不在這條範圍：repo 日報 `D:\HT9045\docs\ops\daily\YYYY-MM-DD.md` 與 rulings-index 照舊（Steven 的裁決只講 ChangeLog）。

**為什麼**：讀者（Jimmy、研五軟體組）都看入口網站；St02 的 ChangeLog 已經放在那裡（入口網站 MR !179）。
