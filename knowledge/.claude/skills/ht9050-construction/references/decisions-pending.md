# ★ 待 Steven 決定（未決斷）

> 從 `todo.md` 的 ★ 節拆出來（Steven 20260927：「把已決斷跟未決斷的分成兩個檔案，方便閱讀」）。已經決定的在 [decisions-decided.md](decisions-decided.md)。
> 20261003 分檔：2026-09-30 以前裁決的題目（Q1～Q61、R1～R146、W1～W62）封存在 [archive/decisions-decided-202609.md](archive/decisions-decided-202609.md)；本檔下面提到「搬到 decisions-decided.md」的舊題，現在要到封存檔找。
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

（Q102 已由 Steven 20261005 23:2x 裁決（原話「先不補」＝B），搬到 decisions-decided.md。）


（目前沒有。Q78、Q79 已由 Steven 20261003 05:3x 裁決，搬到 decisions-decided.md。）

（目前沒有。Q80 已由 Jimmy RULINGS_20261003.md 第 2 條回答，搬到 decisions-decided.md。）

（目前沒有。Q81 已由 Steven 20261003 14:3x 裁決，搬到 decisions-decided.md。）

（Q53～Q61、Q65～Q70 已由 Steven 裁決；Q77 由 Jimmy #23 回答；Q78、Q79 由 Steven 1003 裁決（照 912＋906 行號），搬到 decisions-decided.md。）

### 要 Steven 決定的 R（BCB 沒有答案、真的要 Steven 選的）

（目前沒有。R122、R144 已由 Steven 20260929 11:17 裁決，搬到 decisions-decided.md。）

### 要 EastSun／Frank01 回的 Q（Steven 不在時照 RULINGS_20261005 第 18 條：機台端問 EastSun、動作流程問 Frank01；20261005 23:2x 登記）

（目前沒有。Q125～Q133 已由 Steven 20261006 08:2x 裁決（Q125／Q126 原本要問 EastSun、Q127～Q133 原本要問 Frank01，Steven 自己一次回），搬到 decisions-decided.md（moved to decided 20261006；題目原文也整段搬過去）。）

### E-043 第 2 版的待答題（Q-R4'／Q-R6／Q-R7／Q-R8／Q-R10；1006 16:1x ST01-M 代 ST01-E 登記；程式已照預設出貨，答案不同再改）

> 來源：`D:\AI_TempFile\st01e-e043c2-plan-20261005.md`「## Revision 2」；FROM_STEVEN §3 1006 09:1x（EastSun 四題經筆電 W-99）、Q-R11 已由 Jimmy 1006 09:5x 回 A（RULINGS_20261006 #10）。程式在 `v906/st01-e043c2`（WIP，St01 出差回來 gate＋MR）。

#### Q-R7.（問 Steven）開機驗表新規則 T1'／T9' 一開始就當 ERROR，還是先 WARN

- **機台上會怎樣**：T1'＝`IO_CARD_TYPE` 不是 2／3／4 ⇒ IO 表沒讀、門／急停／馬達電源都沒接上；T9'＝有啟用的軸、但這支程式沒有那種卡的驅動（離線樁），那根軸永遠不會動也不會報警。ERROR 會擋 START／HOME／Motor Test 移動（模擬不擋）。
- **選項**：**A（預設，已出貨）**＝一開始就 ERROR。例：機台（IO_CARD_TYPE=4、Index 走 1203）不會觸發；Steven01 的電腦（IO_CARD_TYPE=1）開機就 ERROR，今天本來就是 ERROR，只是理由改成正確的。B＝照 skill 慣例新規則先 WARN、只記錄。
- **目前狀態**：待 Steven；ST01-M 1006 09:2x 已問，未回。

#### Q-R4'.（問 EastSun／Jimmy）Motor Test 硬停的「全部的門」（Q126）什麼時候可以真的靠它

- **機台上會怎樣**：門項已出貨，但機台用 TEMP-DOORS 全關，所以不作用；main 建置上 HT9050 的 9 扇 MotionNet 幻影門讀成「開」。要靠它之前要：W-86 #94 的規則 A（HT9050 只開 IO 表 Enable 1 的門）進 main，且 SnSafeDoor1 確認關＝1（RULINGS_20261005 #22）。
- **預設（已出貨）**：門項照 golden 門組出貨，TEMP-DOORS 下不作用。
- **目前狀態**：待 EastSun（W-99）。

#### Q-R6.（問 EastSun，要量）EtherCAT 站離開 OP（驅動器間拔線）時，軸的讀值會失敗還是維持舊值

- **預設（已出貨）**：R2'（任何啟用軸的 class 回報斷線就全部拒絕＋硬停）＋10 秒 link watch WAR16152。若讀值一直成功，修法在 class 後面的 `RouteRead`（EastSun 的 `EtherCAT/Pci1203MotorRoute.cpp`）把「站不在 OP」當無效。
- **目前狀態**：待 EastSun（W-99）。

#### Q-R8.（問 EastSun）Refresh（卡重開）或環恢復後，DS402 驅動器還保有正確位置嗎（決定 E-048 (c) 要不要清 HomeFlag）

- **預設（已出貨）**：不加，先記錄。
- **目前狀態**：待 EastSun（W-99）。

#### Q-R10.（問 EastSun）Q125 的「看看是不是可以解除 alarm」：引擎可不可以在 10 秒內自己下一次卡 Refresh

- **預設（已出貨）**：不自動 Refresh；解除只做重認領＋一次 ResetError，卡重開留給操作員。
- **目前狀態**：待 EastSun（W-99）。

## St02（Steven02，測試通訊）

> **提醒（St02 在等 Steven）**：St02-E 要在 Steven02 這台跑完整 ctest，Steven 已經說可以，St02-E 在等 Steven **在 St02-E 自己的 session 裡**確認一次（St02 日報 09-29 11:5x）。

> W1～W62 已裁決或結案（在 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md）；目前沒有還沒裁的 W 題（20260929 08:1x 整理後全部結案）。St02 的新題目加在下面。
> 正本與背景細節見 St02 `D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`「待使用者裁決」節；這裡照抄內容，不改意思。選項代號沿用 St02 表原本的大小寫。

（W67～W69 已由 Jimmy RULINGS_20261002 第 23 條回答，搬到 decisions-decided.md。）

（W70 已由 Steven 20261003 05:3x 裁決，搬到 decisions-decided.md。）

> 下面三題是 St02 照 Steven 1003 常設規則（912 比較好就照 912＋雙註記）挑出來的**客戶專屬**項目，規則說客戶專屬的要問 Steven（St02-M CHAT_ST02 1003 05:39／05:43；ST01-M 06:2x 登記）。St02 現在都先照 906，Steven 選 912 就補回。

（W72 已由 Steven 20261003 14:3x 裁決，搬到 decisions-decided.md。）

（W73 已由 Steven 20261003 14:3x 裁決，搬到 decisions-decided.md。）

> 下面兩題是筆電草稿 MR !70（計時器排程表，LI-6／INBOX 145 ②）`docs/TIMER_TABLE_PLAN.md` §8 原本等 Jimmy 的 Q1／Q2；Steven 1003 15:0x「如果有正在等待Jimmy判定的項目, 可以讓我 (Steven本人) 來協助裁決」⇒ ST01-M 1003 15:2x 在對話裡問 Steven、16:2x 登記於此。來源：MR !70 tip `6f94ba4a`（`refs/merge-requests/70/head`）。

（Q84 已由 Steven 20261003 16:3x 裁決，搬到 decisions-decided.md。）

（Q85 已由 Steven 20261003 16:2x 裁決，搬到 decisions-decided.md。）

> 下面這題是另一個驗證用 session 的 V-2 覆核（1003 18:3x，唯讀）提出的：B70／B71 當時照 Jimmy RULINGS_20261002 #23-5／6 拿掉了 912 才有的客戶功能，但 Steven 後來的 Q81（1003 14:3x）把「912 才有的客戶功能預設留 912」定成預設。

（Q86 已由 Steven 20261003 21:3x 裁決，搬到 decisions-decided.md。）

（1003 22:5x 記下的四題——筆電夜間報告 §0 #85 Frank 2×4 吸嘴、#73 Jerry Timetick、#49 Ifor RotateKit 客戶、#72 ADAM-6024／接地監測板——1006 09:2x ST01-M 查過都已不在 §0（#73 那件現在是 §0 #127「機台 tick 改回 500 ms，main 要不要分流」），不是 Steven 的待決題，移除。）

（Q90～Q97 已由 Steven 20261004 07:2x 裁決，搬到 decisions-decided.md。）

（Q98 已由 Steven 20261004 17:0x 裁決，搬到 decisions-decided.md。）

（Q99、Q100 已由 Steven 20261004 22:2x 裁決，搬到 decisions-decided.md。）
