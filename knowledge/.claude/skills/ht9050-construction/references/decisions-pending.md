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


#### Q78. 19 個存檔鈕的「A02 開著、OP 等級時直接關頁、不存」保護，912 才有，要留還是照 #20 改回 906？（ST01-M 1003 01:1x 登記；ST01-E 的 E-030a 找到；同一題已在 FROM_STEVEN §3 問 Jimmy）

- **畫面上會發生什麼**：V912 在 19 個設定頁的存檔處理加了一段 A02 `Close(); return;`（RogerYang 的保護）：AOISetup、BinSelect、GroundMan、IniConfig_OCR、Ld_UldDelayTime、Rotate、StartCondition、Temperature、BarCode、Cleaning、QAMode、SetUp、TesterIF、VacuumUnit、YieldMonitoring、TrayForm、TTLCfg、UserDefForm_File、Contact（另有 TrayAssignment）。906 都沒有。**只有 Config [A02]（切到 OP 等級不准存參數）打開、而且目前是 OP 等級（AccessLevel＝0）時**，912 這 19 頁按存檔會直接關頁、不寫檔；906 在同樣情況會繼續存（一般對話框頁存操作員改的值；Contact 是非對話框，Close() 先跑 FormClose，改的值被丟掉、檔案用原值重寫）。**A02 沒開或工程師等級以上，兩版都照常存。**main／review6 自動產生的 .gen.inc 裡是 V912 的寫法。（⛔ ST01-M 1003 02:0x 更正：01:1x 的版本寫成「912 永遠不存」，漏了 A02＋OP 等級這個條件；ST01-E 指出。）
- **A（ST01-E 建議）**：留著 912 的保護，記成第 20 條的例外（跟 Q77 Auto Clean 一樣是保護）。例：A02 開著、OP 等級在 Cleaning 頁改了值按存檔 → 頁面關掉、不寫檔；工程師等級照常存。
- **B**：照 #20 改回 906。例：A02 開著、OP 等級在 TrayAssignment 改了值按存檔 → 寫進檔案；在 Contact 改了按存檔 → 改的值被丟掉、檔案用原值重寫。
- **目前狀態**：Jimmy 或 Steven 回答之前，E-030a 19 個都不動。

#### Q79. ⚠ 送給測試機的接觸力字串（GPIB）現在照 912 的算法，要留還是改回 906？（ST01-M 1003 02:0x 登記；E-030a 報告 §8 (b)；同一題已在 FROM_STEVEN §3 問 Jimmy）

- **畫面上會發生什麼**：Contact 頁讀檔時算出每支手臂的接觸力字串 asArmForce1／2，這個字串會經 GPIB 送給測試機。V912 ReadFile :609-616 用 `CalcDeviceForce(...,true)`（RogerYang 20260624，註解「這裡決定送 Kgf 還是 N」）；906 :607-608 用 `iPinCT*ForcePerPinN/9.8`。畫面上的力量顯示（ShowArmAndDeviceForce）也一樣是 912 的算法。CalcDeviceForce 在 906 _0625 裡沒有。main／review6 現在送的是 912 算出來的值。
- **A**：留著 912 的算法，記成第 20 條的例外（像 #20c ADAM 那樣，當成真的 bug 修正）。例：測試機收到的力量照 912 的 Kgf／N 換算。
- **B**：改回 906 的算法。例：測試機收到 `腳數×每腳牛頓數÷9.8` 算出來的值，跟今天 906 量產機一樣。
- ST01-E 沒有建議，請 Jimmy 判斷（跟安全有關：力量單位錯會讓測試機設定錯的壓力）。詳見 `D:\AI_TempFile\st01e-e030a-report-20261003.md` §8。
- **目前狀態**：回答之前不動。

（Q53～Q61、Q65～Q70 已由 Steven 裁決；Q77 由 Jimmy #23 回答；Q78、Q79 等 Jimmy／Steven，搬到 decisions-decided.md。）

### 要 Steven 決定的 R（BCB 沒有答案、真的要 Steven 選的）

（目前沒有。R122、R144 已由 Steven 20260929 11:17 裁決，搬到 decisions-decided.md。）

## St02（Steven02，測試通訊）

> **提醒（St02 在等 Steven）**：St02-E 要在 Steven02 這台跑完整 ctest，Steven 已經說可以，St02-E 在等 Steven **在 St02-E 自己的 session 裡**確認一次（St02 日報 09-29 11:5x）。

> W1～W62 已裁決或結案（在 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md）；目前沒有還沒裁的 W 題（20260929 08:1x 整理後全部結案）。St02 的新題目加在下面。
> 正本與背景細節見 St02 `D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`「待使用者裁決」節；這裡照抄內容，不改意思。選項代號沿用 St02 表原本的大小寫。

（W67～W69 已由 Jimmy RULINGS_20261002 第 23 條回答，搬到 decisions-decided.md。）
