# §十三、S8～S11 留下的待辦（高級審查員第二輪，20260923 21:03） —— 全文

> 從 `SKILL.md` **§十三** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§十三 同名同號，待辦 #1～#12 編號不變；「處理進度」表、「#8 重寫」與原始待辦表三段原位保留。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：SKILL.md §〇 速查表（多列權威出處寫 §十三 #N）、§六 ⛔更正（「§十三 待辦 #10 結案」）、§4.7 ⛔更正（「§十三 待辦 #12 結案」）；`JsonBridge/ChanAction.cpp` 註解提到「§十三 待辦 #1 的結案」。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。
> ⚠ 表內「SKILL `:353, :430`」「SKILL §六 `:586`」這類 **SKILL.md 行號**是拆檔前量的，續拆後行號已變；請改用 § 編號找（原句不改）。
> ⚠ #11「見下方『ctest 基線』」與 #12「另見下方 sqlite 退場註」在拆出前的 SKILL.md 裡就**沒有**對應段落（原檔到待辦表就結束），不是拆檔弄丟的。

---

## 十三、S8～S11 留下的待辦（高級審查員第二輪，20260923 21:03）

> 來源：審查員第二輪報告（`handoff/02_review_report_r2.md`）。總評 **可以 commit**——四條擋關判準
> （會當掉／卡輪詢／寫壞機台檔或禁區檔／翻譯行為與 golden 不同／弄壞建置）無一命中。
> 使用者 20260923 裁示：**除非是大問題，否則不擋 commit，轉成待辦。**下面就是轉過來的。

### 處理進度（20260923 21:30～，本輪）

| # | 結果 | 做了什麼 |
|---|---|---|
| 1 | ✅ **已修，但審查員的單本身也錯了** | 待辦說「連到 `cMyDB.cpp:1000` 的三參數**真本體**」。實測：`cMyDB.cpp:1000` **整段在 `#if 0` 內**（`:999` 開、`:1067` 收），`cMyDB.cpp.obj` 對該符號是 `U` 不是 `T`。真正的鏈是 **3 參數 → `uHGemEquipment.cpp:3483`（body 逐字 `::MyDBIProcess(S1,S2); (void)S3;`）→ 2 參數 `aHotPlateSubstrate.cpp:1264` 計數空槽**。⇒ 原註解的**結論**（sqlite 與文字 EventLog 都沒落地）**本來就是對的**，錯的只是中間那一層。`ChanAction.cpp:201-226` 已改成完整鏈；`sideEffectsSkipped` 字串同步改 |
| 2 | ⏸ 未做（需實跑寫入路徑） | 會寫硬編路徑 `D:\HT9045\system\lastdata.dat`（不在版控）。依 AGENTS 的「備份 → 驗證 → 刪備份」要有人在機台旁，留給 S11 驗收 |
| 3 | ✅ **已完成，兩邊同一顆 commit**（使用者 20260923：「改成新的方式, 兩邊同時修改!」） | C++：`WebBridgeTags.cpp` 刪掉 320 個 `pci1203.di<N>` ＋ 192 個 `pci1203.do<N>` **值** tag（`.ring/.addr/.flat/.station/.chan` **屬性** tag 刻意保留 —— 它們是接線圖不是量測值，開機後不再 patch，打包省不到東西卻會讓畫面失去逐 port 歸屬）。web：`web/js/pci1203.js` 新增 `unpackIoPlanes()`。<br>⚠ **展開放在資料入口，不是 `view.js` 的 5 個讀取點** —— `view.js` 的 `refresh()` 用 `data-k` 在每次 patch 之後重查 tags，只改建畫面的讀取點會讓畫面**第一次畫對、0.2 秒後全空**（正是 `ChanIo.h` 檔頭警告的「不報錯、只安靜空掉」）。改在入口 ⇒ `view.js` **一行都沒動**。<br>三態原樣保留（key 不在→`n/a`／null→`---`／number→值），valid=0 一律 null **不是 0**；位元序照 C++（每 byte **LSB 先**）。<br>**新增回歸 gate**：`tests/test_wb_tags.cpp` 同時釘「512 個值 tag 不在」**與**「屬性 tag 還在」—— 兩邊一起釘，因為曾經兩套並存（多 870 B），只釘 `io.*` 的話那段期間也會通過。實測 2 條新斷言 PASS |
| 4 | ✅ 已修（5 處） | `ChanAlarm.cpp:232` `:391`→`:405`；`ChanAlarm.cpp:288` `:391`→`:469/:478`（並補函式起點 `:405`）；`ChanAction.h:53` `:3369`→`:3656`；`ChanAction.cpp:86` `:3373-3382`→`:3657-3665`；`ChanConfig.h:59` `:393`→`:624` |
| 5 | ⛔ **待辦前提錯誤：`StopAllMotor()` 早就翻譯好了** | 查證（使用者 20260923「可以先使用 extern 的方式」時發現）：golden `Motor/myGALILmotor.cpp:4712` `void StopAllMotor(bool bIndexCanStop)` → 移植樹 **`Motor/myGALILmotor.cpp:5759` 已翻譯、標 ACTIVE**（同檔 :135）。但 `aHotPlateSubstrate.cpp:1255` 的 `void StopAllMotor() {}` 空殼**還在**，兩者是**多載不是重複宣告**（`Motor/myGALILmotor.h:18-33` 的 SIGNATURE COLLISION 檔頭），所以並存連結得起來 —— 而 8 個既有呼叫點**全部解析到空殼**。<br>⇒ `stopMotorPorted` **仍必須送 false**：真本體存在，但這條路徑到不了它，送 true 會讓瀏覽器以為馬達停了。<br>⇒ 真正的待辦是**統一兩個多載**（`myGALILmotor.h` 檔頭 INTEGRATION REQUESTS），**不是**「等翻譯」。<br>⚠⚠ 統一之後這條路徑會**真的讓機台停下來**（golden `note.cpp:806`：警報一起就 `StopAllMotor()`）。使用者 20260923 已指出「本來機台就是透過 C++ 的控制在動」，且 Jimmy 的 START／PAUSE 已可用（`feat/v912-port @ e997ba7`，已是本樹 HEAD 的祖先）⇒ 這條**不是禁區，是排程問題**：要在機台旁做。已把查證與警告寫進 `ChanAlarm.cpp`；**本輪沒有接線**。<br>⛔ **20260926 核對：本條已結案**——commit `c3c459f2`（20260924 08:32「告警照 golden 停機…StopAllMotor 統一」，使用者 20260924 裁決「兩者都停機」「StopAllMotor 統一」）之後，`JsonBridge/ChanAlarm.cpp:185` 送 `stopMotorPorted:true`、`:211` 每筆事件 `stopAllMotor:true`（事件只來自 `wb_serve.cpp` `ForwardShowErrorMessage` 的兩處 `EmitAlarm`，都在 `W906_AlarmStopLikeGolden` 之後）。`SKILL.md` §〇 那一列與 `wire-reduction.md` 的「恆 null／恆 false」同時更正 |
| 6 | ⏸ 未做 | (b) 需有 1203 卡的機器；(a)(c)(d) 排 S9b／S11fix |
| 7 | ⚠ **補了三條，但第十條是錯的、當日撤回**（見下方「本輪自己犯的錯」E1） | 十一＝`fSortCT` no-op facade（✅ 成立，但「第二個閘」的行號抄錯，見新待辦 #17）；十二＝`MyForceDirectories` 在守衛前（✅ 成立，**標明不是待辦**，是「不要順手改」的備忘）。~~十＝`iTo3Unload[]` 全 0~~ ⛔ **已撤回** |
| 8 | ⛔ **前提已死，待辦作廢重寫**（使用者 20260923：「`--dry` 不再使用」） | 待辦寫的是「`--dry` 模式下 probe 會弄髒禁區檔」。**`--dry` 已於 20260923 從三個啟動器移除**（commit `a332397`；`HT9045_Web.cmd:14-21` 原文「**Do not re-add it.**」），自 `f005a7c`（20260918 裁決 W906-ZEROARG）起 exe 預設 `dry=false`，行為由**建置期**的 `SOFT_SIMULTE` 決定，不由執行期旗標決定。`wb_serve.cpp:2556` 還認得這個引數，但**沒有人再傳它**。⇒ 「`--dry` 模式」這個情境不存在了，#8 原文作廢。**殘留的真問題見下方**，它與任何旗標無關 |
| 9 | ⏸ 未做（需與 web 側一起裁決） | `AckJson` 的 `ok=false` 把整包 JSON 轉義塞進 `error`；改 `wb_serve` 的 `ok` 判準或前端統一解析，兩條路要一起定 |
| 10 | ✅ 已修（§六 ⛔更正） | Alarm 那列改成「C++ 只送事實，分類在 HTML」，附 C++／HTML 分工表；原句用刪除線保留 |
| 11 | ✅ 已量（但 HEAD worktree 對照仍未做） | 20260923 實測：**158 支、26 失敗**。⚠ 其中 **4 支不是測試失敗，是建置產物壞掉**：`test_wb_tags.exe`／`test_wb_simpump.exe` 是 **0 bytes**（連結失敗留下的空檔，ctest 報 `BAD_COMMAND`）；`test_wb_crypto.exe`／`test_wb_wsproto.exe` **不存在**（重建 `test_wb_crypto` 仍連結失敗）⇒ 真正的測試失敗是 **22 支**。<br>⚠⚠ **兩份既有基線都過期**：`CLAUDE.md` 寫「常駐失敗是五個」、`build.bat:40-45` 寫 6 支（08-17），但實測 `IniFiles` 與 `GA1_ReadGeneralIni` **現在會過**。把會過的測試留在豁免清單上，等於預先授權忽略未來的真回歸。<br>⚠ 0-byte exe 本身是個坑：ctest 報成 `BAD_COMMAND`，看起來像環境問題，實際上是 `WB_Tags` 這條**整個沒有在跑** —— 本輪加 gate 時才發現它一跑起來就有 3 條既有失敗（`machine.id.type`／`machine.customerCode`／coverage，全是 IniConfig 在測試 harness 沒載入，與本次改動無關） |
| 12 | ✅ 已修，**但整條問題已經不存在了**（使用者 20260923：「SQLite 資料庫已經不再使用, 只使用單純的 csv 存檔方式」） | ⛔ **本條連同它的前提一起退場。**「`MyDBExecSQL` 對 null `dbReadWrite` 會不會 crash」是個**已經不需要回答的問題** —— 這棵樹不用 SQLite 了，留存一律走 CSV。<br>⇒ 連帶失效的還有 §4.7 規則 3 的整段推論鏈：「`wb_serve` 不開 sqlite ⇒ `log.event` 只能回 `{ok:false,"db not open"}` ⇒ P8 第一步是把 `MyDBOpenDB` 帶進開機序列」。**P8 那個第一步不用做了**，`log.event` 的落地目標要改成 CSV。<br>（原查證留檔：本樹自帶 sqlite **3.7.7.1**，`sqlite3_exec` 對 NULL db 回 `SQLITE_MISUSE`（`sqlite3.c:86901`→`:21572-21577`），`cMyDB.cpp:241-242` 的 `bUseMDB` 短路在前，`:250` rowid 已 NULL-guard ⇒ 當時的結論「沒有 crash 路徑」是對的，只是現在沒人問了。）<br>⚠ 後續：審查員開的新待辦 #18（限縮措辭、`sqlite3_get_table` 那 18 處）**一併作廢** |


---

### ⛔ 本輪自己犯的錯（高級審查員第三輪抓到，20260923 深夜）

**兩條實質錯誤，都已當日修掉。記在這裡是因為錯誤的「形狀」比錯誤本身有價值。**

| | 錯了什麼 | 真相 | 怎麼發現的 |
|---|---|---|---|
| E1 | `porting-gaps.md` §十 宣稱 `iTo3Unload[]`「宣告了但沒人填」，QtyLog 六格全寫 `BinCT[0][0]`，並列了 6 個「受害使用點」 | **它 20260817 就填好了**：`cmydef.cpp:4281` 的 same-TU static initializer（`struct W906_TrayIndexMapInit`）、`:4386` 起 33 筆賦值、`:4431` 的實例。有進 link（`nm ht9045_globals.dir/cmydef.cpp.obj`）。⇒ 六格不會全寫同一格，6 個受害使用點**一個都不成立**，是**幽靈待辦** | **我自己寫在那一節裡的複驗指令**（`grep "iTo3Unload…= " … # 應為零`）照跑會得到 **33 筆** |
| E2 | 拆 §4.8 出去的理由是「全樹零引用」 | **實際有 9 處**：`StageThermo.h:5/12`、`StageThermo.cpp:62/72/99/113/124`、`gen_tempchan.py:6`、`s7_thermo_probe.py:69` | 我 grep 的是 `§4.8`，而那些引用寫的是 **`SKILL.md 4.8`，沒有 `§` 字元** |

**這兩條的共同教訓（比修掉它們重要）：**

1. **E1 —— 寫了複驗指令卻沒跑，比不寫更糟。**
   一條沒跑過的「複驗指令」會讓後面每個讀者都以為這條被驗證過了。
   本檔以後的規則：**貼複驗指令＝已經跑過它，並且貼出實際輸出**。
2. **E1 的汙染源是過期註解。** 根因不是憑空捏造，是抄了 `WebBridgeTags.cpp:899-904`
   的「DECLARED but never initialized … reads all-zero today」—— 那段寫於 Fix1 **前一天**。
   ⇒ **汙染源已一起更正**（原句保留＋就地反駁），否則下一個人會再抄一次。
3. **E2 —— 搜尋字串本身就是一個假設。**
   我在 §〇 寫了「`§4.5` 等有 10+ 處被引用，改號會變懸空指標」的警告，
   然後**用同一個帶 `§` 的 pattern 去證明 §4.8 沒人引用** —— 一個只找得到自己預期格式的搜尋，
   回報「零命中」時，零命中的是**格式**不是**事實**。
   ⇒ 現已全部統一成 `§4.8` 拼法，並做過一次**不分拼法**的普查（10 種寫法全部有對應 stub）。

---

### 新待辦（高級審查員第三輪，20260923）

| # | 待辦 | 為什麼重要 | 證據（檔:行） | 建議排在哪一期 |
|---|---|---|---|---|
| 13 | ~~撤銷 `porting-gaps.md` §十~~ ✅ **本輪已做**（整節加撤回段＋原文收進 `<details>` 保留稽核） | 見上表 E1 | `cmydef.cpp:4281/4386/4431` | 已完成 |
| 14 | ~~更正 `WebBridgeTags.cpp:899-904` 的過期敘述~~ ✅ **本輪已做**（原句保留＋就地反駁） | E1 的汙染源，不修下一個人會再抄一次 | `WebBridgeTags.cpp:899-904` vs `cmydef.cpp:4281` | 已完成 |
| 15 | ~~§4.8 的 9 處指標改指新家~~ ✅ **本輪已做**（統一成 `§4.8` 並註明全文在 `references/api-shape.md`） | 見上表 E2 | `StageThermo.h/.cpp`、`gen_tempchan.py`、`s7_thermo_probe.py` | 已完成 |
| 16 | 修 `JsonBridge/` 另外 **3 條**過期 `wb_serve.cpp` 行號（上一輪沒查到的）：`StructApply.cpp:28` `:2341-2344`→`:2687-2691`；`StructApply.cpp:145` `:1475-1519`→`:1708-1749`；`StructApply.h:25` `:1360`→`:1591` | 與 #4 同一族。三條目前都指向**完全無關**的程式碼（HTTP 路由／字元切行迴圈），讀者按圖索驥會得到「看起來有憑有據的錯」 | 審查員已逐條 `sed -n` 複驗 | S11fix |
| 17 | 修 3 處**本輪新寫的**行號：`uHGemEquipment.cpp:3483`→**`:3480`**；`cMyDB.cpp:240-241`→**`:241-242`**（240 是空行；三處同錯：`ChanAction.cpp` 註解、§4.7 更正表、§〇 速查表）；`forms/fSortCT.h:64` 的 `csystem.cpp:2820-2821`→**`:6631-6632`**（定義）／**`:8296-8297`、`:8381-8382`**（呼叫） | 這一輪的主題就是行號正確性，自己新寫的註解不該再帶錯號 | `sed -n '3480p' SECSGEM/uHGemEquipment.cpp`；`sed -n '241,242p' cMyDB.cpp`；`grep -n W7C2_FSORTCT csystem.cpp` | S11fix |
| ~~18~~ | ~~限縮「一層 crash 路徑都沒有」的措辭~~ ⛔ **作廢**（使用者 20260923：「SQLite 資料庫已經不再使用, 只使用單純的 csv 存檔方式」） | 審查員原本要我限縮成「只對 `MyDBExecSQL` 成立，`sqlite3_get_table(dbReadOnly,…)` 那 18 處沒有第 1 層」。**SQLite 整個退場之後這條沒有意義了** —— 不會有人去開那個 DB | 同上（留作史料） | 作廢 |
| 19 | #8 的範圍是 **16 個** tracked 執行期檔，不是 3 個：`Alarm-`／`Message-`／`Dialog-close-` 的 request+response、`Dialog-auth-verify/result`，每個都有 `.json` 與 `js/*.js` 兩份 | 只挪一個會留下同樣的髒工作區 | `git ls-files web/JSON/` | 等 Jimmy 裁決（見下方） |
| 20 | **把行號引用做成 gate**：掃全樹註解裡的 `<檔>:<行>`，記下目標行內容指紋，進 ctest；漂了就紅燈 | 光 `JsonBridge/` 一個目錄這一輪就有 **6 條**行號壞掉（3 條舊的＋3 條新寫的），而 `wb_serve.cpp:3653-3655` 早就留下同一條教訓「註解裡的行號要在寫完註解後重量」。**人工複驗守不住** | `wb_serve.cpp:3653-3655` | 獨立工作 |
| 21 | ~~更正 `D:\HT9045\CLAUDE.md` 的 `--dry` 段落~~ ✅ **已裁決並完成**（使用者 20260923：「比照 #8，**行為由建置期的 `SOFT_SIMULTE` 決定，不由執行期旗標決定**」） | CLAUDE.md 原本仍把 `--dry` 當建議的安全旗標，會把下一個 agent 導向一個被明令 `Do not re-add it.` 的旗標；而它的失敗模式（存檔 ack 回 ok、重讀「正常」、伺服器一停就整批蒸發）正是 review A1 修掉的**假成功**。<br>已在 `CLAUDE.md` 20260918 那條之後新增「**20260923 裁決：`--dry` 已完全退場**」段：三個啟動器已移除（`a332397`）、exe 自 `f005a7c` 起預設 `dry=false`、`wb_serve.cpp:2556` 雖仍認得但沒人再傳、**模擬 vs 真機只看 `MachineType.h` 的 `SOFT_SIMULTE`**（出貨用 `-DW906_NO_SOFT_SIMULTE=ON`）。 | `HT9045_Web.cmd:13-21`；`wb_serve.cpp:2534`／`:2556`；`MachineType.h:63-64` | 已完成 |
| 22 | 統一 `skill-registry.md` 的數字：SKILL.md 現況（拆後 318 行）、`.github/skills` prose 寫 29 vs 表格 30（實測 30）、「漏約 24 個」應為**集合差 33 個**（`.github/` 另有 10 個 `.claude/` 沒有的 skill） | 表自己說「不要拿本表統計當答案」，就更不該留互相矛盾的數字 | `comm -13` 實測 | 併入 registry 後續動作 #6 |

---

### #8 的結論（使用者 20260923 裁示：通知 Jimmy 變更）

使用者判斷：「Jimmy 因為要做一些模擬所以沒拿掉，但實際上機台的時候，
要根據有沒有 `//#define SOFT_SIMULTE` 的定義來決定，這部分要通知他進行變更。」

⚠⚠ **方向對，但有一個前置條件，通知信裡一定要講：**

`web/page/dialog-bridge.js` 是**純輪詢**（`POLL_MS = 100`，`fetch()` 或 `file://` 的 `.js` 墊片），
**整支檔案 0 個 WebSocket**（實測 `grep -c -i "websocket|ws://" = 0`）。
也就是說**信箱檔就是告警對話框今天唯一的傳輸路徑**，真機台上也是。

⇒ 若照字面「沒有 `SOFT_SIMULTE` 就不要寫信箱」，**真機台會完全沒有告警對話框** ——
  操作員看不到框、答不了、機台卡住，而 log 裡什麼都沒有。

**建議改成：把信箱搬出版控，而不是依組態關掉它**：
`g_dialogMailboxDir` 預設改到 `web/JSON/runtime/`（新目錄＋`.gitignore`），HTTP 同時服務兩處。
- 不隨旗標分岔 —— 模擬與真機**同一條路徑**，不會有「模擬會動、真機不會」這種最難查的分岔
- 寫入端與服務端**同時**搬，不會出現「寫在 A、讀在 B」
- Jimmy 既有的 16 個契約樣本檔位置不動

Jimmy 自己的 20260923 信（`[HT9045 V906] 20260923 START／PAUSE 模擬已可用`）規則 8 寫的是
「**不要 commit 這三個檔的執行期變更**」—— 那是 workaround 不是修法，而且**少算了**：實際是 16 個。


---

### ⛔ SQLite 退場（使用者 20260923 裁示）

> 原話：「**SQLite 資料庫已經不再使用, 只使用單純的 csv 存檔方式**」。

這條把好幾段推論一次作廢，列在這裡免得下一個人照著已死的前提做事：

| 受影響的地方 | 原本寫什麼 | 現在 |
|---|---|---|
| §4.7 規則 3 | 「`wb_serve` 目前不開 sqlite … P8 第一步是把 `MyDBOpenDB` 帶進 `wb_serve` 開機序列並解 gate」 | ⛔ **那一步不用做了。**`log.event` 的落地目標改成 CSV |
| §4.7 規則 3 的 ⛔更正段 | 花了一整段論證「`MyDBExecSQL` 沒有 crash 路徑」 | 結論仍正確，但**已是史料** —— 沒人會去開那個 DB |
| §4.7 規則 4 | 「兩個空 shim 要退場…連結時以 `cMyDB.cpp:1831`／`:1803` 為準」 | ⚠ **要重新裁決**：留痕改走 CSV 之後，這兩個 shim 的替代目標是什麼？ |
| 待辦 #12、新待辦 #18 | 都在問 sqlite 的 crash 語意 | ⛔ 兩條一起作廢 |
| `act.main.clarnData` 的 `sideEffectsSkipped` | 目前寫「sqlite 與文字 EventLog 都沒有落地」 | ⚠ 前半段變成**恆真而且不再重要**；真正要講的是**文字／CSV 那半**有沒有落地 |

### 怎麼做（使用者 20260923 補充）：**照 `if(CosFunction.bUseMDB==false)` 那個現成的形狀**

> 原話：「修改方式就是指參照 `if( CosFunction.bUseMDB==false )` 的方式」。

⇒ **不是把 `cMyDB.cpp` 或 `third_party/sqlite3/` 從建置移除**（我原本問錯方向了）。
  退場走的是 golden 自己就有的那個旗標 —— `//Steven 20210526 : 部分客戶取消使用MDB`。

**量到的現況（20260923 實測，兩棵樹對照）：**

| | 移植樹 | golden V912 |
|---|---:|---:|
| `cMyDB.cpp` 裡的 `bUseMDB` 守衛 | **13** | 6 |
| `sqlite3_get_table` | 19 | 18 |

⇒ 移植樹的守衛**比 golden 還多**，翻譯時沒有掉守衛（審查員新待辦 #18 說的
  「18+ 個 `sqlite3_get_table` 沒有 `bUseMDB` 短路」是**數了原始出現次數、沒看外層守衛與 `#if 0`**
  —— 該檔有 33 個 `^#if 0`。這條因此**雙重作廢**：一是 sqlite 退場，二是前提本身不成立）。

**golden 的兩種寫法，照抄就好：**
```cpp
if(CosFunction.bUseMDB==false)      // 早退型（cMyDB.cpp:241-242 / :175）
    return 0;
...
if(CosFunction.bUseMDB)             // 包起來型（golden cMyDB.cpp:614）
{
    sqlite3_get_table(dbReadOnly, ...);
}
```

⚠⚠ **還沒解決的一件事**：`CosFunction.cpp:2040` 目前是 `CosFunction.bUseMDB = true;`（**預設開**），
  而且它不是 `config.ini` 讀進來的，是 `CosFunction` 依客戶碼設定的。
  ⇒ 「SQLite 已不再使用」要真的成立，**得先決定 `bUseMDB` 在哪裡變成 false**
    （全域改預設？還是照客戶碼？）。在那之前，守衛寫對了也還是會走進 sqlite。
    **這一條要問使用者或 Jimmy，不要自己假設。**

⚠ 另一半同樣待確認：CSV 留痕的**實際落點與格式**。不要自己假設成 `QtyLog` 那套 `TMyStringList`。
