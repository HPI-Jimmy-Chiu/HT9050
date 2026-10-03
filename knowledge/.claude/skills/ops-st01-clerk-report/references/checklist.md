# 核對清單與常見錯誤

> A 段記錄員交件前逐項做；B 段 ST01-E 收件後抽查；C 段是真的發生過的錯（附出處），每輪開工前讀一次；D 段是指令。
> 新的錯誤型態由 ST01-E 補進 C 段（記錄員不改本 skill，寫在交件第 ⑤ 項）。

## A. 記錄員自查（交件前）

1. 範圍的起點、終點、顆數跟派工訊息一致；`§11.NN` 接上一節、摘要表那列的 # 也是 NN。
2. §11.NN 標題與摘要列裡出現的每個 hash，都在本輪 `git log` 的輸出裡（C-9）。
3. 每個 Q／R／W／S 題號都對過 decisions-pending.md／decisions-decided.md／RULINGS：題目對（C-1）、待決還是已決對（C-2）、最新選項對（C-3）、寫成「題號＝選項（S 編號）」。
4. R 題還在 decisions-pending.md 的，寫「已照建議先做（可推翻）」，不寫「定案」（C-5）。
5. §12.4 逐題列到的題數＝decisions-pending.md 的 `### Q／R／W` 標題數（C-4）；decisions-decided.md 的「⚠ 還要 Steven 回」也掃過。
6. §12.2 的工程師只寫派工訊息說「在跑」的（C-6）；§12 每一條都從來源重新確認過，不是抄上一版（C-7）。
7. 新寫的路徑全部跑過存在檢查（D-4）；沒有 `...`、沒有折行拆開的路徑（D-5）；機台檔寫 `D:\HT9045\system\…`（C-12）。
8. 沒有你／我／他／對方；沒有 github-xx（改名歷史「原 github-02」除外）（D-6）。
9. 時間都來自 commit（C-11）；跨日標「09-27 hh:mm」。
10. 狀態計數用 D-2 重算過；「最近變動」是 20 行（D-3）。
11. §1～§11 與日報沒有刪字（更正都是加 ⛔）；三份檔行尾跟改之前一樣、沒有控制字元（D-7）。
12. 交件五項齊全；來源互相矛盾的寫進第 ④ 項（C-16）。

## B. ST01-E 抽查（收件後）

每輪至少做 1、2、3、4、6；範圍大（>10 顆）或有大批裁決時全做。

1. **顆數**：`git -C /d/HT9045 rev-list --first-parent --count <上一輪終點>..<本輪終點>`＝記錄員寫的 N；§11.NN 的 a 段是上一輪終點的下一顆。
2. **題號**：抽 3 題（至少 1 題 R、1 題本輪狀態有變的），對 decisions 檔的標題與「目前狀態」。
3. **§12.4 條目數**：跟 D-1 的數字一樣。
4. **§12.2 工程師名單**：跟 ST01-E 自己的派工紀錄一致（誰在跑、誰只是預定）。
5. **§12.5 已轉達／待轉達**：跟 ST01-M 給的文字一致。
6. **diff**：派工前複製的三份檔 vs 現在（D-8）。刪除行只能出現在 ChangeLog §12.1～§12.4、rulings-index 的最後更新／狀態計數／改到的主表列（新列＝舊列＋後面補的字）／最近變動最舊那行；日報不應有刪除行；§1～§11 被改的行，新行要是舊行原字＋插入的 ⛔ 註。
7. **路徑存在**：對新段跑 D-4。
8. **計數**：D-2、D-3。
9. **行尾與控制字元**：D-7。
10. **要 ST01-E 決定的**：一大批裁決併成一列還是拆列、RULINGS_20260927 新條目要不要進 rulings-index、要不要開新一天的檔——記錄員寫在第 ④ 項，ST01-E 回覆後下一輪照做。
11. 錯的地方在原處加「⛔ 更正（ST01-E hh:mx 核對）：」或退回記錄員；錯誤型態是新的就補進 C 段。

## C. 常見錯誤（真的發生過的）

| # | 錯誤型態 | 實例 | 出處 | 怎麼防 |
|---|---|---|---|---|
| 1 | 題號對錯題目 | Q34 寫成「levelset.dat 權限表新設計」；實際是 HandlerSys 逐通道 Heater 廠牌（S154，方案 D，Q15 併入）；levelset.dat 是 S64（Q25～Q30，已落地） | ST01-E 20260927 13:3x 核對 | 每個題號都去 decisions 檔找 `### Q34.` 標題對一次 |
| 2 | 已裁決寫成待決、漏寫結果 | R15 寫「待 Steven 重選」（實際 S162＝B）；Q31 寫「levelset.dat 新設計要重問」（實際 S152＝A′）；R39 寫「還在 decisions-pending」（實際 S163 已裁決，剩做法＝Q44） | ST01-E 13:3x | 看題目在 pending 還是 decided、「目前狀態」那行；一律寫「題號＝選項（S 編號）」 |
| 3 | 裁決重選沒跟上 | Q4 寫「定案 A」（S126）；Steven 10:4x 重選 B（S160 更正 S126：先把 Setup.Speed、Setup.TrayAssignment、Config.DIOInterFaceCFG 搬到第二型再退役第一型） | ST01-E 14:2x | decisions-decided.md 的「重選」字樣、RULINGS 的「更正 S…」 |
| 4 | §12.4 漏列 | 寫「只剩 Q34 與 Q44」，漏了 R60～R72（「已照建議做、但還要 Steven 點頭的 R」） | ST01-E 14:2x | 條目數＝D-1 |
| 5 | R 題寫成「定案」 | §11.46m 標題、§12.1、日報 13:58 段、rulings-index「S123～S163」列都寫「R71／R72 定案」；兩題在 decisions-pending.md，狀態是「已照建議先做，可推翻」（commit `3b519817` 訊息沒寫定案） | 本 skill 建立時（20260927 14:3x）查到，尚未更正 | R 題在 pending 就寫「已照建議先做（可推翻）」 |
| 6 | 預定派工寫成已派 | 「加派 S92／S91／S114 三位」，其實沒派（S92 本體早在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBackup.cpp`、S91 已接 `1d68d518`、S114 已做 `8ebb43f9`／`b40b140f`） | ST01-E 14:2x | 只寫派工訊息「在跑」的 |
| 7 | 沿用上一版 §12 的舊狀態 | Q40 寫「等 Jimmy 決定送出點」（Jimmy 已同意）；Q41 寫「沒有新進度」（已交件） | ST01-E 14:2x | §12 從來源重組，上一版每一條都重新確認 |
| 8 | 狀態計數照抄 | 日報 06:3x 段寫「不變 ✅67／📝18」，實際 S119 📝→✅，應為 ✅68／📝17 | 記錄員 13:0x 那輪自己發現，在該段後面補 ⛔更正 | 每輪用 D-2 重算 |
| 9 | 範圍外的 commit 寫進本節 | §11.40（`22ae48b1`～`5ac2570f`）標題列了「S94 批號清單」；S94 是 `26d0b3f8`（09-26 20:46），屬 §11.39g | 主 session 09-27 03:xx 核對，標題加 ⛔更正 | 標題與摘要列的每個 hash 都要在本輪 `git log` 輸出裡 |
| 10 | 編號後來改了 | W32（SIM 實跑）在 `ed3a0ed1` 改編成 W33，W32 讓給 St02 ESD G5 | §11.42a／e | 寫的當下以 decisions 檔為準；事後用「⛔ 更新」補註 |
| 11 | 時間用 session 時鐘 | 20260926 10:00 以後的時段標籤偏晚 40～60 分鐘 | §11.30（`fee09aac` 全部對回 commit 時間） | 只用 commit 的 `%ad` |
| 12 | 機台資料夾接到移植樹底下 | §12.5 寫 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\system\ContactInfo.ini`／`Gerneral.ini`／`LotSummary.csv`／`RunMode.txt`；§11.45q、日報 09-27 13:0x 段、rulings-index 一處寫 levelset.dat 備份「從 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\system\` 搬走」。那個資料夾不存在，機台檔在 `D:\HT9045\system\` | 本 skill 建立時查到，尚未更正 | D-4；程式碼裡的 `system\`、`config\` 是機台資料夾 |
| 13 | 漏子資料夾、用 `...` 省略 | §11.45j 寫 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\SCK_ART.cpp:1620-1646`（實際在 `…\Automation\SCK_ART.cpp`）；§11.45c 寫 `D:\HT9045\HT9011UC_Code_V3.33.899.0_...` | 本 skill 建立時查到，尚未更正 | D-4；不確定就 `find` |
| 14 | 路徑被折行拆開 | §12.1 兩處 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\` 換行才接 `AOISetup.cpp`、`StartCondition.cpp:425` | 本 skill 建立時查到 | D-5 |
| 15 | 原文寫「不重複列」 | §11.45c 列 RULINGS_20260927 時，第 3 條寫「本節暫不重複列」；第 3 條其實是「動作流程對照何時可以開始」 | 本 skill 建立時查到 | 寫一句重點＋原文絕對路徑 |
| 16 | 來源互相矛盾沒標出來 | S122 寫「關 Teach／Motor Test 分頁要重新 find home、重新整理先停馬達，兩者交 Jimmy」；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 2 條第 18 題寫 **B**：只做前半、重新整理不停產、St01 做；decisions-decided.md Q35 又寫第 18 題「仍待 Steven」。ChangeLog §12.2／§12.5 與 rulings-index S122 列照 S122 寫 | 本 skill 建立時查到，交 ST01-E 判斷 | 同一件事有兩個編號就兩邊都查；對不起來寫「待確認」、列進交件第 ④ 項 |
| 17 | 讀到超過本輪終點的檔 | 20260928 00:2x 這一輪終點是 `0bca1318`，但工作樹的 decisions-pending.md 已被 ST01-M 的 `c93baadf` 加了 W47／W48；直接數工作樹會把下一輪的題目算進 §12.4 | 本輪記錄員自己發現（clerk_state） | 開工先跑 `git -C /d/HT9045 rev-list --count <終點>..HEAD`，不是 0 就一律用 `git -C /d/HT9045 show <終點>:<路徑>` 讀 decisions-pending／decided 再數；超過終點的內容只在 §11 當「資訊」提一句，下一輪再算 |
| 18 | 派工後的新工作沒寫進 §12.2 | 同一輪 §12.2 寫「目前沒有工程師在跑」，其實 ST01-E 在記錄員出發後 00:3x 又派了 N34 | ST01-E 00:5x 核對 | 記錄員只照派工單寫；ST01-E 核對時一定比對「派工單之後又派了誰」，有就在 §12.2 補 ⛔ 更正 |
| 19 | 同一輪中途 repo 又往前 | 20260928 13:2x 這一輪開工時 `e67d1de3..HEAD`＝0，寫到一半 HEAD 已前進到 `eebb5c18`（W57～W62 已登記）；另外 §12.1 把已推送的 `e67d1de3` 寫成「還沒推」（讀到未 fetch 的遠端） | 記錄員自己抓到前者；ST01-E 14:5x 核對抓到後者 | 整輪讀 decisions-pending／decided 一律用 `git -C /d/HT9045 show <終點>:<路徑>`（不只開工那一次）；寫推送狀態前先 `git -C /d/HT9045 fetch` 再數 |
| 20 | 用 `sed -i` 修已寫好的路徑 | 20260928 15:5x 這一輪記錄員用 `sed -i` 批次改一個寫錯的 AlarmCodeList.txt 路徑，反斜線被吃掉（`D:\HT9045\Error\...` 變成 `D:HT9045rrorAlarmCodeList.txt`） | 記錄員自己用 D-4 路徑檢查抓到 | 含 Windows 反斜線路徑的修改一律用 Edit 工具（或 Write 工具寫 Python 腳本），連「只改一個已寫好的路徑」也不用 `sed -i`；改完再跑一次 D-4 |
| 21 | 把 patch 檔裡的 hash 當成本 repo 的 commit | St02 交接的 `D:\HT9045\docs\handoff\ST02_FSHOW_WRAP_20260928.patch` 第一行 `From 555193d9…` 是 St02 自己電腦上的 commit，本 repo 查不到（`git cat-file -e` 失敗）；真正送進來的是交接 commit `1f15753b` | 20260929 07:xx 這一輪記錄員自己查出來 | 寫成「patch（`<patch 裡的 hash>`，經交接 commit `<本 repo 的 hash>` 送達）」；查不到不要當成錯誤回報，也不要只寫 patch 裡的 hash |
| 22 | 共用工作樹裡未 commit 的修改寫成「待確認」放進 §12／日報 | 20260930 00:1x §11.59 看到 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp` 未 commit（S-10 3.7／3.8），§12.3 和日報風險表都寫「待確認」；其實 ST01-E 00:24 就 commit 成 `e6e537c2`，交件時已過時 | ST01-E 00:3x 收件 | 工作樹的未 commit 修改不是本輪的事實：不寫進 §12／日報，只放在交件第 ④ 項讓 ST01-E 回答 |
| 23 | 猜是哪個 session 登記的 | §11.59 把 `ccbe909a`（todo E-017）寫成「ST01-E3 登記」；其實是 ST01-M 登記、排給 ST01-E3。所有 session 的 commit 作者都是 Steven，`%an` 分不出來 | ST01-E 00:3x 收件 | 登記者只照 commit 訊息、派工單或交接列寫；都沒寫就寫「登記」不寫人 |
| 24 | 用 Git Bash 工具查 CRLF 得到假警報 | 20260930 §11.59 用 `sed`／`grep`／`xxd` 管線查 rulings-index.md 看起來像 LF，以為行尾壞了；用 Windows Python 數位元組其實 201／201 全是 CRLF | 記錄員 §11.59 交件第 ⑤ 項 | 行尾一律用 Python 讀 `D:\...` 路徑、比 `data.count(b'\r\n')` 和 `data.count(b'\n')` |
| 25 | ST01-M 的交接文字夾帶早就記過的項目 | 20260930 §11.60 的 ST01-M 原文列了 `21eeda6d`／`9f9d7be7`／`ce2c239f`／`c6b03165`（09-29 11:47～17:43，早記在 0929 §11.57）與 Q57 第二次探針（§11.59e 已記） | 記錄員 §11.60 交件第 ④、⑤ 項（這輪自己抓到，沒重複列） | ST01-M 原文裡每個 hash 先 `grep -n <hash> D:\docs\ChangeLog\CHANGES_*.md`（只查 ChangeLog 資料夾，不要掃整棵樹）；已記過的不重複，只在 §11.NN 最後一節寫一句「已見 §…」 |
| 26 | 轉述派工訊息裡的路徑時寫錯 | 20260930 §11.61 第一版草稿把派工訊息寫的 `D:\UnloaderInfo` 轉述成 `D:\HT9045\UnloaderInfo`（C-12 同一種：機台資料夾接到別的樹底下） | 記錄員 §11.61 交件第 ⑤ 項（自己用 D-4 抓到並改好） | 照抄派工訊息的路徑也要跑 D-4，而且逐字複製、不要憑記憶重打 |
| 27 | 把 golden 的函式名配上移植樹的路徑 | §11.61 rulings-index S169 寫「golden `sbtExitClick`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\BarCode\BarCode.cpp`」（那是移植樹；golden 在 V912 `…\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:2408-2417`） | ST01-E 03:4x 收件 | 寫「golden」兩個字時，後面的路徑一定是 V912（或 906_0625）那棵樹；移植樹的路徑前面寫「移植樹」 |
| 28 | 派工訊息列的「哪顆 commit 改了哪個檔」本身寫錯 | 20260930 §11.63 派工訊息把 `ops-ht9045-proxy-build\references\gotchas.md` 列在 `ac0c6ca2` 底下；`git show --stat ac0c6ca2` 沒有這個檔（是 `66bf56c4`／`c1e1ee8e`） | 記錄員 §11.63 交件第 ④ 項（自己查到、照實際 diff 寫） | 派工訊息的檔案清單也要用 `git show --stat <hash>` 逐顆核對；對不上就照 diff 寫，並列進交件第 ④ 項 |
| 29 | 抄表格既有列時漏掉 hash | §11.62 摘要表那一列「D-012 A3W 落地**（登出／關機」中間漏了 `42e6607e` | 記錄員 §11.63 交件第 ⑤ 項 | 新增或抄寫摘要表列時，粗體字與 hash 的位置一起比對 |
| 30 | 別的分支上的檔補了本機絕對路徑 | 20260930 §11.64 草稿把 St02 的 `TrayEditForm.cpp` 寫成 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TrayEditForm.cpp`，review6 沒有這個檔（在 St02 分支 `v906/steven-gpib-widget`），D-4 抓到 | 記錄員 §11.64 交件第 ⑤ 項 | 先 `git -C /d/HT9045 ls-tree -r <分支> --name-only | grep <檔名>` 查在哪個分支；不在簽出的分支就寫「分支 `v906/steven-xxx` 的 `路徑`」，不加 `D:\` |
| 31 | 「待修」照抄，沒查後來修好了沒 | §11.64 §12.2 還寫 St02 proxy gate 的 `FShow_Audit`／`SecsCatalogue`「待修」；其實筆電 `527ce723` 已修、隨 `9f1b2187` 合進 review6 | ST01-E 07:0x 收件 | §12 每一條「待修／待查」都重新查：`git log --oneline <上一輪終點>..origin/main` 與本輪合進來的 commit 訊息裡有沒有修掉 |
| 32 | 用 Edit 工具改 CRLF 檔整段文字，old_string 對不上 | 20260930 §11.65 改 rulings-index.md（CRLF）的「最後更新」段 | 記錄員 §11.65 交件第 ⑤ 項 | 改 CRLF 檔的整段：用 Python（Python314 絕對路徑）以 bytes 讀、找「開頭一行」與「結尾一行」兩個 marker 框出區塊整段換掉、寫回時保留 CRLF，再跑 D-7；不要整段重打 old_string |
| 33 | D-4 抓到 MISSING 就當成寫錯路徑改掉 | 20260930 §11.66 的 `D:\HT9045\config\AGV.ini` 在開發機上本來就不存在——那正是 B8 AG-1 的發現（golden 開頁就讀這個從沒被寫過的檔）；另外派工訊息說「MR !7 已由 Jimmy 合」，記錄員照 main 核對只看到「MR !7 README」，照實維持待 Steven 關 | 記錄員 §11.66 交件第 ④、⑤ 項 | MISSING 先看內文是不是在講「這個檔本來就沒有」；派工訊息的狀態（例如某 MR 已合）也要照來源核對，查不到就照實寫、放進交件第 ④ 項 |
| 34 | 用 commit 數當「列數」 | 20260930 §11.67 寫「B8 6／14」：把 AG-1 的 (A)／(B) 兩個 commit 算成兩列；其實 AG-1 是一列 ⇒ 5／14，而且剩下的清單漏了 BC-4 | ST01-E 18:0x 收件 | 進度一律照批次表／風險文件的「列」數；寫剩下幾列時照文件原清單逐項對，數目要跟「總列數－已做」相等 |
| 35 | 主控台印出來的中文是亂碼就以為檔壞了 | §11.67 用 Bash 呼叫 Python 印檔案內容，cp950 主控台把 UTF-8 顯示成亂碼、⛔ 觸發 UnicodeEncodeError | 記錄員 §11.67 交件第 ⑤ 項 | 要看內容：寫到暫存檔再用 Read 工具讀；要核對：只看位元組數或比對結果（True／False），不要看主控台列印 |
| 36 | 「見 §11.NN 某段」指錯段 | §11.67 草稿把 AGV_E84.cpp 兩個缺口寫成「見 §11.67d」，那段其實只講 Q60 登記 | 記錄員 §11.67 交件第 ⑤ 項 | 寫交叉引用前重讀那一段，確認真的提到；新寫的句子也要跑 D-6（代名詞） |
| 37 | 照上一輪的樣子套「Steven please look」 | §11.68 草稿把 AG-1／form.event 那種「Steven please look」標籤類推到 D-014、D-016，兩顆 commit 訊息都沒寫、decisions-pending.md 也沒登記 Q／R | 記錄員 §11.68 交件自己抓到 | 升級標籤只照這一顆 commit 訊息原文＋decisions-pending.md 有沒有登記；只收緊、沒新 Q 的不加 |
| 38 | 驗證結果不在 commit 訊息就寫「待確認」 | §11.68g 把合 main 的建置＋ctest 17／17 寫成「待確認」，其實 ST01-E 的交件簡報有講、log 在建置目錄 | ST01-E 19:0x 收件 | 簡報給的驗證結果照寫，並註明「commit 訊息沒寫」；有 log 路徑就附上；真的查不到來源才寫待確認 |
| 39 | 範圍起點寫成上一個 gate 過的頭，不是上一輪記錄員的終點 | §11.69 daily／rulings-index 寫「範圍 `5d6b4611..9e7c08a3`，3 顆」：`5d6b4611..` 其實有 4 顆（含記錄員自己的 `e19de373`），簡報給的是 `e19de373..9e7c08a3` | ST01-E 20:4x 收件 | 範圍一律照簡報的起點（＝上一輪記錄員 commit）；寫完用 `git rev-list --count --first-parent <起>..<迄>` 對「N 顆」 |
| 40 | 「風險與阻塞」照抄上一段，狀態已經變了 | §11.69 daily 照抄「MR !6（St02）仍未合」，其實 `dec3c019` 已合進 main（ST01-M 20:3x 告知） | ST01-E 20:4x 收件 | 每一輪把「仍未／還在等」的每一項重查一次（MR 用 `git merge-base --is-ancestor <head> origin/main`）；變了就寫 ⛔ 更正 |
| 41 | 5 小時檢查點：內容有寫但 hash 沒引；gate 只跑一半被當成還在跑 | §11.69 補記：`22e47927`／`bd59786e` 內容早有、hash 沒引；`a9f89386`／`62aa1430` 的 gate log 只到 SIM 半 | 記錄員 §11.69 交件建議 | 檢查點清單逐一搜 hash 本身；gate 看 log 最後一個階段標記（SIM／SHIP／真檔），不要照 commit 性質猜 |
| 42 | 照抄訊息裡「還沒發生的」hash | ST01-M 23:1x 訊息寫 human-review 在 `95f6d95c`（commit 還沒做就先猜），實際是 `770ef0ab` | 記錄員 §11.70 交件建議 | 訊息裡的 hash 用 `git cat-file -e` 查不到、而且描述的事看起來剛要做 ⇒ 當成猜測值，等更正或自己用 `git log` 找真的那顆，不要照抄 |
| 43 | 筆電那邊的「使用者」照抄成「使用者」 | §11.70 的 rulings-index R930 列寫「使用者當面裁決」：筆電的 RULINGS 檔裡「使用者」是 Jimmy，St01 的讀者看不出是誰 | ST01-E 23:3x 收件 | 別的 session 檔案裡的「使用者」「你」「我們」一律換成人名（對照 TO_STEVEN 同一條的寫法）；查不到是誰就寫「（原文：使用者）」 |
| 44 | gate 報「某支測試過了」，其實那支根本沒登記 | 20260930 ST01-M 的 gate 範本 PATH 沒有 nodejs ⇒ `W906_NODE_EXECUTABLE` 快取成 NOTFOUND，5 支 node 測試（WB_F5Contract／WB_TokenIdle／WB_WsLink／D015_A01MenuPage／Stream2E_PagePolls）一整天都沒登記（267 支、應為 271）；§11.70 照抄「`ca661a18` gate 裡 Stream2E_PagePolls 過了」 | 記錄員 §11.71 交件建議 | 寫「gate 裡某支過了」之前，對 gate log 的測試總數跟本機同一顆 commit 的 ctest 總數；少了就查那支測試當時有沒有登記（node／Python 路徑、`if(...)` 條件），沒登記寫「沒跑」 |
| 45 | 「已轉達、還沒 commit」幾分鐘後就落地，下一輪又當新的記一次 | 20261001 §11.72g：ST01-E 審 St02 MR !15 的 FormLock 回覆，本輪終點 `633222d5`（01:22）時還沒 commit，兩分鐘後落地成 handoff `ae2eb939`（01:24） | 記錄員 §11.72 交件建議 | 簡報寫「轉達中／還沒 commit」的事，交件前查 `git log origin/v906/steven-handoff` 終點之後幾分鐘有沒有對應的 commit；有就兩個事實都寫（當時還沒、實際 hash／時間），下一輪不要重記 |
| 46 | 簡報把不同時間的 commit 歸在同一個「X 之後」底下 | 20261001 §11.73 簡報把 `f5e4bc4f`（01:52）與 `568848d1`（02:53）都寫成「`8caaf1e4` 之後」，其實前者在 `8caaf1e4` 之前 | 記錄員 §11.73 交件建議 | 歸屬一律照 `git log --first-parent` 逐顆重排，不照簡報的分組；分開寫在各自的時間點 |
| 47 | 本機測試結果找不到 log 就寫「未找到」 | §11.73 的 SIM 16／16、SHIP 3／3 其實在 ST01-E 的建置目錄（`st01e-q44-build\m7_ctest.log`／`m8_ctest.log`／`m8_node.log`、`st01e-n34-build-ship\m7s_ctest.log`／`m8s_ctest.log`） | ST01-E 03:1x 收件 | ST01-E 自己的本機結果先找 `D:\AI_TempFile\st01e-q44-build\` 與 `st01e-n34-build-ship\` 下同時間的 `*ctest*.log`；還找不到才照 C-38 註明 |
| 48 | 長期「仍未見回覆」的項目：重查來源，但要分清楚「原本的指示」跟「後來的追問」 | 20261001 §11.74 把 Jimmy 09-30 20:54 的原指示（A01 掛 `:7627`）當成已回答 St01 22:2x 的追問（改掛 `:7621` 可不可以）——時間順序反了，追問其實還沒回 | ST01-E 10:3x 收件 | 每輪重查一次來源；比對時間：答覆要晚於問題；commit 歸屬照 `git log -S` 查（那個掛點在 `ea17dd3a`，不是 `0812b7da`） |
| 49 | 狀態表（例 EVENT_PORT_BATCH_20260928.md）沒人動就默默過期 | B1～B6 0928 就落地，表格到 1001 才補（41 格） | 記錄員 §11.74 交件建議 | 每輪看本輪 commit 有沒有對到某張狀態表的列（用列代號 grep commit 訊息）；有就在那一列尾端加「⛔ 更新（日期、commit）」，不改原文 |
| 50 | 用 Python 手刻 `\uXXXX` 轉義寫中文：打錯一個十六進位數字會變成另一個合法的中文字 | §11.75 記錄員把「綁」（`\u7d81`）打成 `\u7db1`（綱），UTF-8／控制字元／行尾檢查全部通過 | 記錄員 §11.75 交件建議 | 能直接寫中文就不要手刻轉義；一定要轉義時，寫完用 Read 工具讀渲染後的文字逐字看，或 grep 正確的字確認命中數 |
| 51 | 「用量上限」要照通知原文分清楚是哪一種：子代理的 session 上限（幾點重置）／帳號週用量 | §11.75 把 B8 工程師 11:48 的 session 上限（13:40 重置）寫成「帳號週用量上限」；同一輪又把 08:4x 那次（只打到一個子代理，10:1x 已更正帳號沒事）一起改錯 | ST01-E 14:2x 收件 | 抄通知裡的原句與重置時間；改舊句之前先看那一句講的是哪一次事件、那一次後來有沒有更正 |
| 52 | GitLab 網頁合併產生的 merge commit 時間是 UTC（比台灣晚 8 小時），main 與入口網站 repo 都一樣 | §11.76：Steven 的 15 個網頁合併 `78be5aff`～`caae69bb` 在 git 顯示 08:05～08:13，實際是 16:05～16:13 | 記錄員 §11.76 交件建議 | 用 merge commit 時間前先換算成台灣時間，再跟交接訊息（台灣時間）對一次；不要把 UTC 直接寫成本地時間 |
| 53 | 轉交的項目要寫原始來源，不要寫成轉交的人說的 | §11.76 把 ST01-M 19:1x 回報、經 ST01-E 轉交的項目寫成「ST01-E 口述」（5 處），還把有 log 的 22／22 寫成「尚無 log 路徑」 | ST01-E 19:3x 收件 | 派工訊息裡寫明「以下是 X 的回報」就照寫「X 回報（Y 轉交）」；寫「沒有 log」之前先問 ST01-E 要路徑 |
| 54 | 派工訊息裡寫「在跑」「排隊」的 gate／順序，交件時可能已經變了 | §11.77：派工說 review6 `6dd4a772` 的 SHIP 還在跑，log 其實 21:24 已結束；之後又有 `f17fbacd`／`b22a90a4` 兩顆 handoff 改了佇列 | 記錄員 §11.77 交件建議 | 寫 §12.2 前：每個「在跑」項目讀 log 最後的 `=== ... done` 標記；再跑 `git log origin/v906/steven-handoff --since=<派工時間>` 看有沒有新的交接改了順序 |
| 55 | 記錄員工作期間 repo 還在動（登記 commit、筆電合併） | §11.78：交件期間 ST01-M 多推了 `c489ad5d`／`1f0262a8`，筆電也在 23:08 把 d026 合進 main（`352e861c`），派工時都還不知道 | 記錄員 §11.78 交件建議 | 收集完事實、最後寫檔前各 `git fetch` 一次，範圍終點用最新那一顆；派工裡說「等審」的分支頭都跑一次 `git merge-base --is-ancestor <頭> origin/main`；decisions-pending 在最後終點重讀一次 |
| 56 | 派工說「待回覆／在跑」的項目，最後寫檔前要再讀一次來源；檔案可能只在 q59 或 main | 10-02 §11.79：第一次讀（01:27）到交件之間 main 多了 batch 29 與 NIGHT_REPORT #43；`LotInfo_SECSLotStart.cpp` 等只在 q59 worktree／main，用 D:\HT9045 查會顯示 MISSING | 記錄員 10-02 §11.79 交件建議 | 最後寫檔前重讀 gate log 與 main 的 docs/handoff/TO_STEVEN.md；路徑檢查 MISSING 時先查 q59 worktree（D:\AI_TempFile\st01e-q59）與 `git show origin/main:<path>`，文中寫明哪一棵樹，不要當成路徑寫錯（C-33 的變體）。另：§11.NN 跨檔連號（standing-rules），新的一天不從 §11.1 重來，派工說錯也照規則 |
| 57 | 長任務被用量上限打斷後接續，沿用中斷前的數字 | 10-02 §11.84：記錄員 21:1x 被 session 上限砍掉、23:5x 接續；中間 `HEAD..origin/main` 從 0 變 7、decisions-pending 從 4 題變 0 題 | 記錄員 §11.84／§11.85 交件建議 | 接續時重跑 `rev-list --left-right --count`、decisions-pending 的 grep、gate log，再寫 §12 |
| 58 | ST01-M 原文寫的 `D:\HT9045\docs\handoff\...` 在 review6 工作樹不存在（檔案只在交接分支） | §11.85：`ST01_912_AUDIT_20261002.md` 只在 `v906/steven-handoff` | 記錄員 §11.85 交件建議 | 照第 30 條查是哪個分支；非原文處寫「分支＋相對路徑」，原文引用處加一行記錄員註 |
| 59 | 用 bash heredoc 寫含 `\\` 的 Python，反斜線被砍成一半 | §11.84 記錄員的 `fix3.py` assert 失敗 | 記錄員 §11.84 交件建議 | 含反斜線的腳本用 Write 工具寫成檔，或直接用 Edit 工具改 |
| 60 | Jimmy 先答了原本列給 Steven 的題（decisions-decided 寫「Steven 要改再說」），§12.4 就直接刪掉 | §11.85：Jimmy RULINGS_20261002 第 23 條答了 Q77／W67～W69 | 記錄員 §11.85 交件建議 | §12.4 改列「Jimmy 先答、Steven 可推翻」並白話寫一次；Steven 下一輪沒改才拿掉 |
| 61 | 把題目「白話」時漏掉觸發條件，寫成「永遠」 | 10-03 §11.86 §12.4 的 Q78 寫「操作員按存檔…912 上這些頁的存檔鈕等於永遠不存」；實際只有 Config [A02] 打開＋OP 等級（`AccessLevel`＝0）才不存（ST01-E 01:5x 原地 ⛔ 更正；登記表 decisions-pending.md 同樣漏了，請 ST01-M 改） | ST01-E 10-03 01:5x 收件 | 白話之前先讀題目原文的 if 條件（工程師報告或 golden 行號）；條件寫進白話第一句；寫「永遠／一定／都不」之前再對一次原文 |
| 62 | 簡報寫「在跑」的 St02 代跑，交件時其實已跑完 | §11.86：簡報 01:2x 寫 !128 在跑；`st01-st02-128-queued.log` 01:28:52 已結束，交接分支 `21f266fd`（01:31）已寫 10／10 | 記錄員 §11.86 交件建議 | 和第 54、55 條同型：交件前讀 log 末行，並看 `git log origin/v906/steven-handoff`；兩邊時間都寫 |
| 63 | 開新一天的檔、舊檔 §12 只留一行指標時，搬不搬已結案的 §12.5 條目沒有規則 | §11.86：10-02 檔 §12 換成指標，已結案 7 條沒有搬到 10-03 檔 | 記錄員 §11.86 交件建議 | 已結案（⛔ 已完成／⛔ 已結案）的不搬，在新檔 §12.5 頂端寫一行「已結案條目見前一天檔 §11.NN～§11.MM」；未結案的全部搬 |

## D. 指令

**D-1　decisions-pending 的題數與清單**（⛔ 20261002 ST01-E 更正：題目標題可能是 `###` 或 `####`（Q70 用 `####`），舊的 `"^### [QRW][0-9]"` 會數到 0 被誤當「沒有待決題」；改用 `"^####* [QRW][0-9]"`，數完開檔看一眼）

```
grep -c "^####* [QRW][0-9]" /d/HT9045/.claude/skills/ht9050-construction/references/decisions-pending.md
grep "^####* [QRW][0-9]" /d/HT9045/.claude/skills/ht9050-construction/references/decisions-pending.md | cut -d. -f1
grep -n "⚠ 還要 Steven 回" /d/HT9045/.claude/skills/ht9050-construction/references/decisions-decided.md
```

**D-2　rulings-index 狀態計數重算**（依「狀態」欄第一個符號；總數＝合計）

```
awk '/^## 主表/{f=1;next} /^## 最近變動/{f=0} f && /^\| / && !/^\| # \|/ && !/^\|---/' /d/HT9045/.claude/skills/ht9050-construction/references/rulings-index.md | awk -F'|' '{s=$5; split(s,a," "); print a[1]}' | sort | uniq -c
```

20260927 14:2x 的結果：✅70、⏳7、📝17、❓9、➡10、—11（合計 124），跟表頭一致。

**D-3　最近變動行數（要是 20）**

```
awk '/^## 最近變動/{f=1;next} f && /^\* /' /d/HT9045/.claude/skills/ht9050-construction/references/rulings-index.md | wc -l
```

**D-4　路徑存在檢查**：下面的腳本**用 Write 工具**存到 session 暫存資料夾（不要用 Bash heredoc，反斜線會被吃掉），再用 Python314 跑：`/c/Users/steven/AppData/Local/Programs/Python/Python314/python.exe <腳本> <檔案絕對路徑> <起始行> <結束行>`。印出 `MISSING` 的逐一看：本輪新建、已退場（例 `D:\HT9045_ref`）的可以不管，其餘要改。

```python
import re, os, sys
BS = chr(92)
f, a, b = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
lines = open(f, encoding='utf-8').read().splitlines()[a-1:b]
pat = re.compile('[A-Z]:' + BS + BS + '[^`\\s：（）()，、；「」。*<]+')
seen = set()
for t in lines:
    for p in pat.findall(t):
        p = re.sub(':[0-9][0-9,\\-]*$', '', p).rstrip(BS)
        if p in seen:
            continue
        seen.add(p)
        if not os.path.exists(p):
            print('MISSING', p)
print('checked', len(seen))
```

20260927 14:3x 對 ChangeLog 第 2264 行起跑的結果，抓到 C-12、C-13 那幾處。

**D-5　被折行拆開的路徑**（行尾是反斜線）

```
grep -n '[\]$' /d/docs/ChangeLog/CHANGES_20260926_Steven.md /d/docs/ops/daily/20260926.md
```

**D-6　代名詞與 session 名**（對本輪新段；「」裡的原話命中可以保留）

```
grep -nE '你|我|她|對方' <新段檔>
grep -n '他' <新段檔> | grep -v '其他'
grep -nE 'github-[0-9]+' <新段檔>
```

新段檔＝用 `sed -n '<起>,<迄>p' <檔> > <暫存資料夾>\<名>.txt` 切出來的本輪新內容。

**D-7　行尾、BOM、控制字元**

```
file /d/docs/ChangeLog/CHANGES_20260926_Steven.md /d/docs/ops/daily/20260926.md /d/HT9045/.claude/skills/ht9050-construction/references/rulings-index.md
LC_ALL=C grep -nP '[\x00-\x08\x0b\x0c\x0e-\x1f]' /d/docs/ChangeLog/CHANGES_20260926_Steven.md /d/docs/ops/daily/20260926.md /d/HT9045/.claude/skills/ht9050-construction/references/rulings-index.md
```

預期：ChangeLog、日報是「UTF-8 text」（LF，無 BOM），rulings-index 是「UTF-8 text, with CRLF line terminators」；控制字元指令沒有輸出（ChangeLog §10.2 有一個歷史上的 Tab，不在檢查範圍內）。

**D-8　跟派工前的複本比（ST01-E）**

```
diff <暫存資料夾>/CHANGES_20260926_Steven.before.md /d/docs/ChangeLog/CHANGES_20260926_Steven.md | grep '^<'
diff <暫存資料夾>/20260926.before.md /d/docs/ops/daily/20260926.md | grep '^<'
diff <暫存資料夾>/rulings-index.before.md /d/HT9045/.claude/skills/ht9050-construction/references/rulings-index.md | grep '^<'
```

`<` 開頭的是被刪掉或被改掉的舊行，只能落在 B 段第 6 條允許的地方。
