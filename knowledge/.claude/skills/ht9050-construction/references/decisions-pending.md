# ★ 待 Steven 決定（未決斷）

> 從 `todo.md` 的 ★ 節拆出來（Steven 20260927：「把已決斷跟未決斷的分成兩個檔案，方便閱讀」）。已經決定的在 [decisions-decided.md](decisions-decided.md)。
> **回法**：在題目的「目前狀態」後面直接寫答案（選 A／B／C，或直接打字），或在對話裡回；沒寫到的題目照建議做。Steven 回了之後，該題整段搬到 decisions-decided.md，並附 RULINGS 編號。
> 題號沿用原本的：**Q**＝St01（Steven01，資料讀寫）要 Steven 決定；**R**＝St01 已照建議先做、Steven 可推翻；**W**＝St02（Steven02，測試通訊），照 St02 `progress-st02.md`「待使用者裁決」的順序，選項代號沿用 St02 原本的大小寫。
> 維護：Q／R＝ST01-E（Steven01-Engineer，St01 工程線），W＝ST01-M（Steven01-Manager）代 St02 登記（Steven 20260927 定的角色名；session 名 github-xx 會變，不寫）。

## St01（Steven01，資料讀寫）

> **路徑寫法**（Steven 20260927 要求改成絕對路徑）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝golden V912（BCB6 原始碼，cp950／Big5 編碼，VS Code 要用「Reopen with Encoding」選 Big5 才不會亂碼）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝移植樹（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\` 開頭＝V899（BCB6，cp950）；`D:\HT9045\web\` 開頭＝網頁；`D:\HT9045\system\`、`D:\HT9045\config\`、`D:\HT9045\IniData\` 開頭＝機台資料（量產機共用的設定檔；配方在 `D:\HT9045\IniData\Data\<配方>\`）。行號是 20260927 當下的檔案。commit 後面括號裡是那顆 commit 改的主要檔案。
> **寫法規則**（Steven 20260927 21:xx）：每個檔案每一次出現都寫完整的絕對路徑加行號；內部代號（題號、盤點代號、指令名）先用白話說明是什麼，代號放在後面的括號裡；每一題先講機台上／畫面上會發生什麼、操作員看到什麼，再講程式細節；每個選項都附一個具體例子。
> 裁決紀錄：Steven 的 S 編號裁決記在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（20260927 的裁決也續記在這份；本節用到的 S41 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260925.md`）；RULINGS_20260927（第 N 條，筆電寫的）＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`；「todo X-NNN」是待辦表 `D:\HT9045\.claude\skills\ht9050-construction\references\todo.md` 的列號；「Q41 盤點」的代號（C-3、C-4、CC-L1～L5、SU-L1）出自 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md`；已決定的 Q／R 題全文在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`；FROM_STEVEN／TO_STEVEN／CHAT_ST02 的唯讀快照是 `D:\HT9045_handoff\FROM_STEVEN.md`、`D:\HT9045_handoff\TO_STEVEN.md`、`D:\HT9045_handoff\CHAT_ST02.md`。

> **20260929 07:4x 整理**：Steven 已經裁決的 Q44～Q52、Q4-2、R60～R62、R113、R119、R120 都已搬到 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`；這個檔只留還沒決定的。Steven 20260929：「你問題也太多了!」——照 BCB 就能決定的不再列題。
> **20260929 08:xx 整理（ST01-E）**：R63～R146 裡照 BCB 就有答案、照 Steven 先前規則、已被 Q45／Q51 取代、或是給 Jimmy 看的，共 78 題，全部搬到 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`（St01 段最後「20260929 照 BCB／已做」那一節，每題最後一行寫結果）；這裡只剩 R122、R144 兩題。
> **Steven 20260928 早上的通則**（在 ST01-M 對話裡）：「正確的說法是: 任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」（「我們」＝St01＋St02，分工照舊）；「我發現很多問題都跟bcb的事件沒有移植到c++有關」「你把這些一次歸類並移植做完, 然後再看有沒有新的問題」。ST01-E 在做歸類表與移植（已裁決的題 20260929 已全部搬走）。

### 要 Steven 決定或確認的 Q


#### Q77. Auto Clean 鈕「One Cycle 跑的時候不准再觸發」的保護，912 才有，要留還是照 #20 拿掉？（ST01-M 1002 21:0x 登記；St01 912 稽核 Q-A）

- **畫面上會發生什麼**：主畫面 Auto Clean 鈕（golden `btnAutoCleanClick`）。912 在開頭多一行「One Cycle 在跑就直接返回」（912 main.cpp :2254-2255，RogerYang 20260810）；906 :2101-2166 沒有這行。照 906 做的話，One Cycle 跑的時候按 Auto Clean，會再呼叫一次 One Cycle。Steven Q67＝B 讓運轉中也按得到 Auto Clean，所以這個情況真的會發生。
- **A（ST01-E 建議）**：留著 912 的保護，記成 #20 的例外。例：One Cycle 跑到一半按 Auto Clean → 沒反應，等 One Cycle 做完再按。
- **B**：照 #20 拿掉，跟 906 一樣。例：One Cycle 跑到一半按 Auto Clean → One Cycle 又被叫一次。
- 同一份稽核的 Q-B（START 鈕的 Teradyne-US 保護，912 才有、客戶專用）照 #20 拿掉，不另外問；Q-C（Contact 頁第 11 個模式 Visual Detection）已經在 main 上，併進 W68。稽核全文：handoff 分支 `docs/handoff/ST01_912_AUDIT_20261002.md`。
- **目前狀態**：等 Steven 選；選之前 St01 不動。

（Q53～Q61、Q65～Q70 已由 Steven 裁決；Q77 等 Steven，搬到 decisions-decided.md。）

### 要 Steven 決定的 R（BCB 沒有答案、真的要 Steven 選的）

（目前沒有。R122、R144 已由 Steven 20260929 11:17 裁決，搬到 decisions-decided.md。）

## St02（Steven02，測試通訊）

> **提醒（St02 在等 Steven）**：St02-E 要在 Steven02 這台跑完整 ctest，Steven 已經說可以，St02-E 在等 Steven **在 St02-E 自己的 session 裡**確認一次（St02 日報 09-29 11:5x）。

> W1～W62 已裁決或結案（在 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md）；目前沒有還沒裁的 W 題（20260929 08:1x 整理後全部結案）。St02 的新題目加在下面。
> 正本與背景細節見 St02 `D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`「待使用者裁決」節；這裡照抄內容，不改意思。選項代號沿用 St02 表原本的大小寫。

#### W67. #20「沒有例外」有沒有蓋掉 Steven 之前逐項裁決過的 912 項目？（St02-M 1002 20:30 請 ST01-M 統整；ST01-M 21:0x 登記）

- **哪三項**：0927 #35 P65 ARM-QA 重測回 iLotStatus=1（912）；W15＝B 事件記錄 CSV 拆欄（912）；R68-MYDB 警報代碼表（912，本來就在等 Jimmy）。
- **A**：蓋掉，三項都照 906 重做。例：ARM-QA 重測照 906 的流程，事件記錄 CSV 回到 906 的欄位。
- **B**：之前的逐項裁決照舊，算 #20 的例外。例：這三項維持 912 的做法，稽核表標「Steven 例外」。
- 背景：handoff 分支 `docs/handoff/ST02_912_AUDIT_20261002.md`（St02-M 1002 20:30，`ac48ae83`）。
- **目前狀態**：等 Steven 選；St02 回覆之前不動。

#### W68. main 上已經有的 912 內容，什麼時候清、由誰清？（St02-M 1002 20:30；ST01-M 21:0x 登記，併入 St01 稽核的 main 層發現）

- **有哪些**：St02 自查 main 上「912 才有、要照 906 重做」16 項，依影響排：!114 ADAM-6024（見 W69）、GPIB 遠端 START／STOP（含 H-012 !33）、H-013 IsSafePLCIOInstall（安全 PLC）、H-013 fSecsAlarm、Tester Pause 蜂鳴延遲、P65 ARM-QA、AMD 執行期分支……；St01 稽核另列 Contact 頁 Visual Detection 模式（rbVisualDetectionTest）、DF_SetContactMode 的 ASE-CL 灰掉、D-026 WAR04217 密碼規則、editlist 產生器讀 V912（.gen.inc 內容是 V912 的行）。筆電 NIGHT_REPORT #62＝A：動已經在 main 的東西要等 Steven。
- **A（ST01-M 建議）**：各組清自己的，照影響順序一張一張 MR（安全相關的 GPIB 遠端 START／STOP、IsSafePLCIOInstall 先），每張照 906 重做、兩組態 gate。例：St02 先送「GPIB 遠端 START／STOP 照 906」一張 MR，筆電照常合。
- **B**：全部交給筆電在 #62 一次清。例：筆電排一批「#20 清理」，St01／St02 只提供清單。
- **C**：先不清，只是之後不再加 912 的東西。
- **目前狀態**：等 Steven 選；St01、St02 回覆之前都不動 main 上的這些。

#### W69. !114 ADAM-6024 已經在 main，要留著補一張 906 修正，還是 revert？（St02-M 1002 20:30；ST01-M 21:0x 登記）

- **現況**：!114（`9e46491f`，26 支檔、+7442 行，912 adam6024.cpp 翻的）在 1002 18:51 用網頁合進 main，在 #20 說「抽出 batch 45」之後 41 分鐘。St02-E 已經照 906 重翻好（Steven02 本機 `c800f2fc`，還沒推）。同一時段網頁也合了 MR !120 `v912/nb2-app`（`fe17dfda`，18:51），跟 #20「912 不做」對不上，一併請 Steven 知道。
- **A（ST01-M 建議）**：留著，St02 送一張「ADAM-6024 照 906」的修正 MR（`c800f2fc`）蓋過去。例：main 上的 ADAM 頁短暫是 912 版，修正 MR 合了就變 906 版；別人的分支不用跟著處理 revert。
- **B**：先 revert !114，再合 906 版。例：main 先拿掉 26 支檔，之後再加回 906 版；revert 這麼大的 MR，其他開著的分支合 main 時會多一輪衝突。
- **目前狀態**：等 Steven 選。

