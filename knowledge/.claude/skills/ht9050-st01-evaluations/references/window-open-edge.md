# 評估：網頁的「開設定頁」其實發生在開站時（R108；R89～R93、R84、R110 同一個前提）

> 讀者：Steven。撰寫：ST01-E（Steven01 工程線）派的工程師，20260927 21:40。**只讀研究，沒有改任何程式、沒有 build。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6` 的 commit `c20bdec2`（那顆 commit 改的主要檔是 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）。程式一律用 `git -C D:\HT9045 show c20bdec2:<路徑>` 讀；網頁兩個檔另外跟工作樹比過（忽略換行符號後完全相同）。
> 三棵樹的寫法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6 原始碼，Big5／cp950 編碼，VS Code 要「Reopen with Encoding」選 Big5）；`D:\HT9045\web\` 開頭＝**網頁**。行號是 20260927 當下的檔案。
> 名詞：「C 路頁」＝由 C++ 跑 golden 的開頁程式與存檔程式的設定頁（網頁只負責顯示與送出），目前 24 頁。

---

## 0. 一句話結論

網頁把 24 個 C 路設定頁在**開站（HMI 載入）時**就先在背景載好、藏起來；C++ 照 golden 在「開頁」要做的三件事——**查能不能開、記一筆「Enter …」、依登入等級決定哪些欄位能改**——因此都發生在開站，不是操作員按開窗的那一刻。

建議 **B＋D**：St01 先把「Enter …」改成看「哪些視窗開著」清單的「打開」那一下才記（B，St01 自己做得完）；同時請 Jimmy 把網頁引擎改成「視窗打開才向 C++ 要資料」（D，根本解：權限、欄位、Contact 開頁換算、Teach 開頁清回原點旗標會一起對到 golden 的時間點）。**C（C++ 推通知叫頁面重讀）不需要**：golden 自己也不會在表單開著時因為換人登入而重查，而網頁每次存檔都重查，已經比 golden 嚴。

---

## 1. 背景（白話）

### 1.1 BCB 版（golden）開一個設定畫面時發生什麼

操作員在主畫面按一顆鈕（例：主畫面工具列的 Speed），**當下**依序：

1. **查能不能開**：運轉中不開、登入等級不夠不開（例：工具選單要權限表第 0 項的等級、Yield Monitoring 另外要第 39 項）。
2. **記一筆事件**：例「Enter Speed」（事件代碼 MES2187，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28694`）。
3. **打開表單**：表單的開頁程式（FormShow）重讀檔案，並依「這一刻」的登入等級決定哪些欄位能改。

每按一次開窗鈕，這三步就重做一次。

### 1.2 網頁版現在發生什麼

1. **開站**：主畫面外框 `D:\HT9045\web\background.html` 一開始就把所有視窗建好，預設隱藏的設定頁也在背景載入（`D:\HT9045\web\background.html:906-907`）。例外只有標了「開了才載入」（lazy）的三個視窗：PCIE-1203、Event Log、Tester Comm（`D:\HT9045\web\background.html:482`、`D:\HT9045\web\background.html:511`、`D:\HT9045\web\background.html:512`）。
2. **每個設定頁一載好**，共用的網頁引擎就向 C++ 要資料（開頁指令 editlist.get；`D:\HT9045\web\page\ht9045_wire_engine.js:2108` 的 `load();`）。C++ 收到這個指令，才做 1.1 的三步。
3. **之後操作員按開窗**，外框只是把藏著的頁顯示出來（`D:\HT9045\web\background.html:746-771` 開窗函式 openWin），再把「哪些視窗開著」的清單送給 C++（下稱**視窗總表**，指令 ui.windows.put；`D:\HT9045\web\background.html:845-872`）——**不會再向 C++ 要資料**。

⇒ 三件事都在開站時做完了。

### 1.3 操作員會遇到的情況（由程式推得）

- **記事件的時間不對（R108）**：wb_serve 啟動後第一次開站時，開窗閘有過的頁各記一筆「Enter …」（最多 21 筆一起出現）；之後操作員真的開窗不記；HMI 重新整理也不再記（C++ 記得「這頁開過了」，只有 golden 的關窗 Close() 才忘掉）。
  今天這些事件其實**還沒寫進任何檔**（R109：移植樹寫事件紀錄的入口還是空殼 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\acatchtray_shims.cpp:152`，另一個只印主控台 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\canary_support.cpp:117`），所以現在看不到壞處；等事件紀錄接上以後，客戶查紀錄會看到一串開站時的假「Enter」。
- **換人登入後，頁面還是開站時那個等級的樣子（R89～R93 的前提）**：
  - 例 1（等級升高）：出貨組態開機是 Operator（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:128-145`，照 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:11067-11081`）。假設這台的權限表把 Yield Monitoring 設成 Engineer 以上：開站時 Yield Monitoring 頁要資料 → 等級不足被拒 → 頁面顯示紅字「C 路讀取失敗」（`D:\HT9045\web\page\ht9045_wire_engine.js:1220`），沒有值。10:00 工程師登入、打開 Yield Monitoring → **還是那行紅字**。BCB 版這時會正常打開、可以改。
  - 例 2（等級降低）：模擬組態開機是 HonPrec（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:118-127`，照 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:11064-11066`）。開站時各頁以 HonPrec 載好、欄位全可改。有人登出成 Operator 再開 Speed → 欄位**看起來**仍可改；按存檔時 C++ 會擋（存檔前重查開窗閘 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303`；等級和開頁時不同也擋 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:88-93`）→ **不會寫錯**，畫面叫操作員「請先按重讀（權限或登入狀態變了）」（`D:\HT9045\web\page\ht9045_wire_engine.js:1293`）。
  - 但 24 個 C 路頁只有 Handler System 一頁有自己的「重讀」鈕（`D:\HT9045\web\page\ht9045_wire_hwhandlersys.js:257`）；**其他 23 頁唯一能重讀的辦法是整個 HMI 重新整理（F5）**。
- **同一前提的另外兩個副作用**：
  - Contact 頁開頁時的「單位換算＋重載參數」（R84，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:213`）也在開站時跑，不是開窗時。
  - Teach 頁的開頁程式照 golden 會清「要重新回原點」旗標（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:1610`；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:259`，停機時才清）。Teach 頁在開站就要資料 ⇒ **模擬組態下每次打開或重新整理 HMI（機台停著），下一次 START 都會先全部回原點**；BCB 版只有真的打開 Teach 才會。（由程式碼推得，**未實測**；出貨組態開機是 Operator，Teach 的開窗閘不過就不會清。）
- **Configuration 存完再記一筆（R110）**：golden 的 Configuration 存檔就是關窗；網頁存完引擎自動重讀，C++ 把它當成重新開窗，再記一筆「Enter Configuration」。

---

## 2. 現況：相關程式在哪裡、誰的

| 內容（白話） | 位置（絕對路徑） | commit／主人 |
|---|---|---|
| 開站就在背景載入隱藏的設定頁 | `D:\HT9045\web\background.html:906-907`（非 lazy 的 iframe 直接給 src）；四態起點 `D:\HT9045\web\background.html:913`（預設隱藏＝never） | 外框是共用檔；開窗政策表（MODAL_POLICY）歸 Jimmy |
| 引擎一接上就向 C++ 要資料 | `D:\HT9045\web\page\ht9045_wire_engine.js:2108`；C 路讀取 `D:\HT9045\web\page\ht9045_wire_engine.js:1188-1223`（被拒只顯示紅字，不會自己再要） | Jimmy 登記的檔（`D:\HT9045_handoff\TO_STEVEN.md` §1「第 13 條」那一列） |
| 開窗只顯示、送視窗總表 | 開窗 `D:\HT9045\web\background.html:746-771`；狀態改變 `D:\HT9045\web\background.html:665-670` → 60 毫秒後送總表 `D:\HT9045\web\background.html:845-848`；連著時每 5 秒重送一次（心跳）`D:\HT9045\web\background.html:1088-1092` | 同上 |
| 外框另外通知頁面「你的視窗開了／關了」（HT_WIN 訊息） | `D:\HT9045\web\background.html:671-683`；頁面載好補送一次 `D:\HT9045\web\background.html:1200-1201` | 機台端 MT-E3（commit `00ec3fb6`，主要改動檔 `D:\HT9045\web\page\HW.MotorTest.html`） |
| C++ 開頁指令那一臂（先查開窗閘，再跑 golden 開頁） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5199-5223`（查閘在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5207`） | St01 |
| C++ 存檔指令那一臂（運轉中擋、存檔前再查一次開窗閘） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5292-5303` | St01 |
| 開窗閘表（C++ 照 golden 重查能不能開，25 列） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:759-789`；查詢函式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:792-809` | commit `cb306f89`（主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`），St01 |
| 開頁記「Enter …」的表（21 列）與記錄函式 | 表 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:866-892`；函式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:907-919` | commit `342779cc`（主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`），St01 |
| 呼叫「記 Enter」的四個地方 | 共用開頁函式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:57`；Teach `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:256`；Offset `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp:353`；Configuration `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:493` | 同上（都是 St01 加在同一行；Jimmy 的 J1 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:259`，沒碰） |
| C++「這頁開過了」旗標 | 宣告 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:27-31`；開頁後設為開過 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:59-60`；只有 golden 的 Close() 才設回沒開 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:211` | St01 |
| 視窗總表（C++ 收網頁的「哪些視窗開著」） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h`；收指令 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5820-5835` | Jimmy（commit `f21860e6`、`22b485bc`，主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`）＋機台端 EastSun（commit `7414cc6f`，同一檔）。本評估**只讀不改**它 |
| S122：Teach／Motor Test 開關時標成「要重新回原點」 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h`；主迴圈每 0.5 秒一拍呼叫它 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953`（節拍常數 500 毫秒 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:2931`） | commit `0b166feb`（主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`），St01 |
| 今晚另一位 St01 工程師把 S122 通用化成「任何表單開／關時呼叫」 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 尾段 W906_WindowEdgeRegister（**工作樹、還沒 commit**，21:3x 看到的樣子，會變） | St01 |

---

## 3. 查證

### 3.1 網頁端

- 開站就載入：`D:\HT9045\web\background.html:906-907` 非 lazy 視窗的 iframe 直接給 `src`；只有 `lazy:true` 的三個視窗是空殼（`D:\HT9045\web\background.html:753-759` 開窗時才載入、`D:\HT9045\web\background.html:785-791` 關窗時卸載）。24 個 C 路頁全都**不是** lazy（對照表 3.3）。
  ⇒ **2026-10-03 起（S-16，AI(W906-S16)）**：hidden 視窗改成第一次開才載入（`iframe data-defer-src`，`D:\HT9045\web\background.html` 建 iframe 那一行與 `openWin`），所以 C 路頁的「接上就讀資料」（`load()`）現在發生在第一次開窗，不是開站；開過之後再開仍不重讀（關閉只藏起來）。開站看得到的 9 個視窗照舊開站就載。
- 引擎接上就要資料：`D:\HT9045\web\page\ht9045_wire_engine.js:2046` 接上函式 attach，最後一行 `D:\HT9045\web\page\ht9045_wire_engine.js:2108` `load();`；C 路頁的 `load()` 就是 C 路讀取（`D:\HT9045\web\page\ht9045_wire_engine.js:1298-1299` → `D:\HT9045\web\page\ht9045_wire_engine.js:1188`）。
- 引擎**沒有**聽外框的「視窗開了」訊息（在 `D:\HT9045\web\page\ht9045_wire_engine.js` 找 `HT_WIN`：0 筆）；會聽的頁只有 `D:\HT9045\web\page\HW.MotorTest.html`（`D:\HT9045\web\page\HW.MotorTest.html:995-1019`：視窗開著才輪詢、打開時補跑開頁）與 `D:\HT9045\web\page\ht9045_cleaning_c.js:197-205`（Exit 解鎖）。
- 開窗不要資料：`D:\HT9045\web\background.html:746-771` 只做「設定檔判定」「開窗政策」「lazy 載入」「顯示」「設狀態 open」「全螢幕／聚焦」。
- 外框自己的開窗政策（`D:\HT9045\web\background.html:631-661` decideOpen）只管 6 個全螢幕頁（Teach、Motor Test、IO、Contact、Home、Shuttle Move）的「運轉中／機台內有料」，**不看登入等級**；等級只由 C++ 查。
- 重讀鈕：接線檔宣告 `reloadBtn` 的只有 `D:\HT9045\web\page\ht9045_wire_hwhandlersys.js:257`；另兩個表格頁（`D:\HT9045\web\page\ht9045_wire_hwiosetview.js:43`、`D:\HT9045\web\page\ht9045_wire_hwmotortest.js:51`）不是 C 路頁。

### 3.2 C++ 端

- 開頁指令：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5207` 先查開窗閘，拒絕時 golden 開頁一行都不跑、回原因給頁面；過了才跑 golden 開頁。
- 存檔指令：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303` 運轉中擋、再查一次開窗閘（**每次存檔都用當下等級重查**）；C 路頁共用的存檔函式再比對「開頁時的等級」和「現在的等級」，不同就要求重讀（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:88-93`；Configuration、Bin、Offset、DIO 各自的入口也有同樣的檢查：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:240`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp:778`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp:398`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp:419`；Teach 只查「開過了沒」`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:316`）。頁面事件（form.event）也要求同一個等級（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:387-390`），但不重查開窗閘。
  ⇒ 等級變了**不會寫錯**，只會被要求重讀。
- 記 Enter 的規則：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:907-919`，「這頁已經開過」就不記；開過旗標只有 golden Close() 才清（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:211`）。所以**每個 wb_serve 行程只在第一次開站記一次**（HMI 重新整理不記），另外 Configuration 每次存檔都算關窗（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:347` 附近，R110），存完的自動重讀再記一筆。S165 的檔頭已經自己寫了這個限制（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:853-857`「偏離 4」）。
- 視窗總表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`）：
  - 每條連線記最後一份清單，**只增不減**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:50-57`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:157-161`）；以 golden 表單名為準，清單裡表單名是 null 的視窗直接略過（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:137-141`）。
  - 15 秒沒收到新清單算過期（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:16`），時鐘是秒級（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:59-62`）。
  - 查一個表單：只要有新鮮的回報就只看新鮮的，新鮮的之間取**聯集**（任何一個分頁說開著就是開著）；全部過期才照「保守」處理（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:171-223`）。
  - 保守答案：從沒人報過這個表單、或全部過期 ⇒ 回答「開著」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:226-237`）；政策版再加兩條：從沒收過任何清單 ⇒ 沒開；瀏覽器不會回報的 5 個表單 ⇒ 沒開（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:337-339`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:365-377`）。這個「不知道＝開著」是給 START 閘用的安全方向；**拿來判斷「要不要記 Enter」方向就反了**（見選項 B 誤判 1）。
- S122 的邊緣判斷（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`）：每 0.5 秒用政策版取樣 fTeach／fMotorTest，全部過期時沿用上一拍、不算邊緣。
- 今晚的通用化（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 工作樹，未 commit）：`W906_WindowEdgeRegister(表單名, 打開時呼叫, 關掉時呼叫, 運轉中跳過)`；取樣照 S122（全部過期沿用上一拍，否則政策版）；登記表 **16 格**；呼叫的函式**不帶表單名**。第一個使用者是 St02 的 Setup.TesterIF（"FTestIF"）。

### 3.3 對照表：24 個 C 路頁 ↔ 視窗 ↔ golden 表單名 ↔ golden 怎麼開、記不記 Enter

（頁面與結構名取自 `D:\HT9045\web\page\ht9045_wire_engine.js:1038-1061`；視窗取自 `D:\HT9045\web\background.html` 的 WINDOWS 表；golden 行號都是 golden V912。24 頁全部是「預設隱藏、不是 lazy」。）

| C 路頁 | C++ 結構 | 視窗與 golden 表單名 | golden 開法 | golden 記的 Enter |
|---|---|---|---|---|
| `D:\HT9045\web\page\Config.Configuration.html` | IniConfig | config／fConfiguration（`D:\HT9045\web\background.html:434`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28624` | MES2185 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28616` |
| `D:\HT9045\web\page\Setup.Ld_ULd.html` | Ld_UldDelayTime | ldud／fLd_ULd（`D:\HT9045\web\background.html:449`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28453` | MES2176 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28452` |
| `D:\HT9045\web\page\Setup.TrayForm.html` | UserDefForm_File | trayform／fTrayForm（`D:\HT9045\web\background.html:451`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28409` | MES2173 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28408` |
| `D:\HT9045\web\page\Setup.Speed.html` | ArmSpeed_File | speed／fSpeed（`D:\HT9045\web\background.html:432`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28695` | MES2187 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28694` |
| `D:\HT9045\web\page\Setup.YieldMonitoring.html` | TestIF_File_YieldMonitoring | yieldmon／fYieldMonitoring（`D:\HT9045\web\background.html:453`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28509` | MES2178 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28508` |
| `D:\HT9045\web\page\Setup.TrayAssignment.html` | TrayForm | trayassign／fTrayAssignment（`D:\HT9045\web\background.html:471`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28433` | MES2175 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28432` |
| `D:\HT9045\web\page\Setup.Contact.html` | DeviceForm_File | contact／fContact（`D:\HT9045\web\background.html:446`） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28314` | MES2170 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28306` |
| `D:\HT9045\web\page\HW.teach.html` | Teach | teach／fTeach（`D:\HT9045\web\background.html:460`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28846` | MES2189 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28845` |
| `D:\HT9045\web\page\Setup.BinSel.html` | BinSelect | binsel／fBinSel（`D:\HT9045\web\background.html:459`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28324` | MES2171 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28323` |
| `D:\HT9045\web\page\Config.DIOInterFaceCFG.html` | TTLCfg | dioform／fDIOFrom（`D:\HT9045\web\background.html:438`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28669` | MES2186 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28668` |
| `D:\HT9045\web\page\HW.HandlerSys.html` | HSys | handlersys／HandlerSystem（`D:\HT9045\web\background.html:499`，**只在 debug 模式建立**；總表列為瀏覽器不回報 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:337-339`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTemperFrom.cpp:1778` | 不記 |
| `D:\HT9045\web\page\Setup.Temp_Set.html` | Temperature | tempset／fTemp_Set（`D:\HT9045\web\background.html:458`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28352` | MES21109 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28351` |
| `D:\HT9045\web\page\Setup.OffSet.html` | Offset_File | offset／fOffSet（`D:\HT9045\web\background.html:429`） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28718` | MES2188 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28717` |
| `D:\HT9045\web\page\Data.StartCondition.html` | StartCondition | startcond／fStartCondition（`D:\HT9045\web\background.html:457`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28553` | MES2180 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28552` |
| `D:\HT9045\web\page\Setup.QAMode.html` | TestIF_File_QAMode | qamode／fQAMode（`D:\HT9045\web\background.html:442`） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29925` | MES21102 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29924` |
| `D:\HT9045\web\page\HW.VacuumUnit.html` | TestIF_File_VacuumUnit | vacuumunit／fVacuumUnit（`D:\HT9045\web\background.html:489`） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:35560` | 不記 |
| `D:\HT9045\web\page\HW.ShuttleMove.html` | ShuttleMove | shuttlemove／fShuttleMove（`D:\HT9045\web\background.html:470`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:33693` | MES21107 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:33692` |
| `D:\HT9045\web\page\Status.GroundMan.html` | GroundMan | groundman／fGroundMan（`D:\HT9045\web\background.html:448`；**沒開 USE_GROUND_MAN 的機台不建立**） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:34644` | 無代碼的 RecordProcess `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:34643` |
| `D:\HT9045\web\page\Main.AOAInfo.html` | AOAOffset | aoainfo／**沒有 golden 表單名**（`D:\HT9045\web\background.html:510`，form:null，總表不收） | Motion View 的頁籤 | 不記 |
| `D:\HT9045\web\page\Status.CounterSel.html` | IniConfig_CounterSel | countersel／fCounterSel（`D:\HT9045\web\background.html:435`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28565` | MES2181 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28564` |
| `D:\HT9045\web\page\Setup.Cleaning.html` | TestIF_File_Cleaning | cleaning／fCleaning（`D:\HT9045\web\background.html:445`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29674` | MES2194 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29667` |
| `D:\HT9045\web\page\Setup.TesterIF.html` | TestIF_File_TesterIF | testerif／FTestIF（大寫 F，`D:\HT9045\web\background.html:447`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28336` | MES2172 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28335` |
| `D:\HT9045\web\page\Setup.BarCode.html` | TestIF_File_BarCode | barcode／fBarCode（`D:\HT9045\web\background.html:443`；**沒開 BAR_CODE_INSTALL 的機台不建立**） | **非模態** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29916` | MES21101 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29915` |
| `D:\HT9045\web\page\Setup.SetUp.html` | TestIF_File_SetUp | setup／fSetup（`D:\HT9045\web\background.html:455`） | 模態 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28471` | MES2177 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28469` |

另外開窗閘表多一列 Contact Force：視窗 contactforce／fContactForce（`D:\HT9045\web\background.html:486`），但它的頁面 `D:\HT9045\web\page\Setup.ContactForce.html` 還沒有接進引擎的 C 路表，golden 從 Contact 頁開（非模態，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15237-15240`）也不記。

小結：
- 記 Enter 的 21 頁**全部**有 golden 表單名、都在視窗總表裡回報得到；其中 Bar Code、Ground Man 在沒裝該功能的機台上連視窗都不建立（`D:\HT9045\web\JSON\View-rules.json` 的 barcode、groundman 是 `onFalse:"hide"`；`D:\HT9045\web\background.html:890` 不建立）。
- 看不到開關邊緣的只有 AOA（沒有表單名）與 Handler System（出貨模式不建立、總表當它不回報）——這兩頁 golden 都不記 Enter，只影響選項 C 的「打開時重查」。
- golden 22 個開窗鈕：**16 個模態**（表單開著時主畫面點不到）、**6 個非模態**（Contact、QA Mode、Bar Code、Ground Man、Offset、Vacuum Unit）。

### 3.4 golden 在「表單開著時換登入等級」怎麼做（判斷 C 要不要）

1. **登入鈕在主畫面**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28080-28106`（運轉中直接 return）。16 個模態表單開著時主畫面點不到，操作員按不到登入鈕（這是 VCL 模態視窗的一般行為，本次沒有在樹內另外驗證）；6 個非模態表單開著時可以按。
2. **表單開著時等級還是會變的路**：
   - 閒置自動登出（設定 [A01]）：主畫面計時器 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26022-26048`——時間到改成 Operator、重算主畫面按鈕、**只把 Offset 視窗關掉**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26043-26044`）、把工具／設定兩個選單藏起來；其他開著的設定表單不關、不重跑開頁程式。（計時器在模態表單開著時照樣跑，也是 VCL 一般行為。）
   - 遠端指令 HTSET 310／311 直接改成 Supervisor／Operator：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Command.cpp:13248-13262`。
   - 表單自己的密碼流程最後登出成 Operator：Setup `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4610-4661`、Configuration `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482-6528`。
   - 按 START 自動切 Operator（設定 [A01] press start）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:4667-4673`。
3. **等級變了 golden 做什麼**：只重算主畫面的按鈕與少數別的表單上的元件（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:12926-13191` ChangeLevelAttr，主畫面另一個計時器每拍都呼叫 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:21675`；會動到 Lot Info 的頁籤與上傳下載鈕、Temp Set 的 ATC 自我檢測鈕、Bin 的 MRT 勾選；登出成 Operator 時把工具／設定選單收起來 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13182-13189`）。**開著的設定表單不重跑開頁程式、也不重查開窗條件。**
4. **存檔時**：只有一個客戶選項 [A01_2]（「切成 Operator 後不可存設定」，RogerYang 20260305）在 25 個表單的存檔鈕開頭檢查「現在是不是 Operator」，是就跳「[A01_2]目前已切換到Operator權限，請重新登入再做設定!」並關窗。例：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14181-14192`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\QAMode.cpp:75-80`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:1270-1276`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\GroundMan\GroundMan.cpp:1420-1425`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\VacuumUnit\VacuumUnit.cpp:380-386`。選項沒開時，golden 讓已開著的表單照開頁時的等級存檔。
5. **移植樹現況**：沒有翻 golden 的閒置自動登出（在移植樹找 `iA01ChangeOpTime` 只有讀設定的地方：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp:3016-3021`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp:3190`，外加 SECS 的設定表）；網頁上等級會變的路是主畫面登入／登出（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp`）與按 START 切 Operator（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1279`）。

**判斷**：golden 不會在「表單開著時換登入」重查；移植樹每次存檔都重查開窗閘＋比對等級（3.2），**已經比 golden 嚴**。所以選項 C 不是「照 golden」才需要的。golden 真正有、網頁沒有的是「**每按一次開窗就重查一次、重跑一次開頁程式**」——這是選項 D 補的。

### 3.5 指令紀錄（20260927 21:0x～21:40，全部唯讀）

- 用 `git -C D:\HT9045 show c20bdec2:<檔>` 讀出 `D:\HT9045\web\background.html`、`D:\HT9045\web\page\ht9045_wire_engine.js`，另存後與工作樹同一個檔 `diff --strip-trailing-cr`：相同。
- 用同一個指令讀 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp`：全文或相關段落。
- `git -C D:\HT9045 diff` 看 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp` 今晚未 commit 的通用化（只讀）。
- 在 `D:\HT9045\web\page\` 找誰聽外框的「視窗開關」訊息（`git -C D:\HT9045 grep "HT_WIN" HEAD -- web/page`）：只有 `D:\HT9045\web\page\HW.MotorTest.html`、`D:\HT9045\web\page\ht9045_cleaning_c.js`；引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 0 筆。
- 在 `D:\HT9045\web\page\` 找宣告重讀鈕的接線檔（`reloadBtn :`）：只有 `D:\HT9045\web\page\ht9045_wire_hwhandlersys.js:257`（另加引擎裡的說明註解一筆）。
- 24 個 C 路頁（`D:\HT9045\web\page\` 底下，名單見 3.3）逐頁列出載入的 script：24 頁共同載入的只有 `D:\HT9045\web\page\theme.js`、`D:\HT9045\web\page\ht9045_recipe_client.js`、`D:\HT9045\web\page\ht9045_wire_engine.js`。
- golden：以 cp950 解碼的 Python 小工具（放在這次的暫存目錄，不留檔）列出 22 個開窗鈕的模態／非模態、各表單裡 AccessLevel／Insufficient 出現在哪個函式、`bA02DisableSaveParsWhenSwitchToOp` 的 25 個檢查點、`AccessLevel=` 的所有指定點與所在函式。
- 在移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 找閒置自動登出的時間設定（`git -C D:\HT9045 grep iA01ChangeOpTime HEAD`）：只有讀設定的地方（3.4 第 5 點）。

---

## 4. 選項

### 選項 A：維持現況

- **白話**：不改。「Enter」照舊在開站時記；權限與欄位照舊在開站、HMI 重新整理、存檔時由 C++ 決定。
- **改哪些檔**：無。
- **延遲與誤判**：wb_serve 啟動後第一次開站一次記最多 21 筆；操作員真的開窗不記。換登入後要整個 HMI 重新整理才看到對的頁面。
- **工作量**：0。
- **風險**：事件紀錄接上以後（R109）紀錄會誤導；換登入的操作不順（23 頁沒有重讀鈕，存檔被擋時「請先按重讀」沒有鈕可按）；Teach 開站清回原點旗標（模擬組態）、Contact 開頁換算在開站跑。**不會寫錯檔**（存檔前重查還在）。
- **例子**：操作員 08:00 開 HMI、10:00 開 Speed。BCB 版：10:00 記「Enter Speed」。A：08:00 已記、10:00 不記。

### 選項 B：看視窗總表的「打開」那一下記 Enter（St01 自己做）

- **白話**：C++ 每 0.5 秒看一次網頁送來的「哪些視窗開著」清單；某個設定頁的視窗從「沒開／關了」變成「開著」，就先照 golden 查一次能不能開，**能開才記「Enter …」**，不能開就在主控台印一行被擋的原因（golden 被擋的鈕也不記）。開站時不再記。
- **程式做法**：
  1. 開窗事件表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:866-892`，21 列）加一欄「golden 表單名」（＝`D:\HT9045\web\background.html` WINDOWS 表的 form，對照見 3.3）。
  2. 一支每拍檢查的函式，兩種掛法擇一：
     - (a) 掛今晚通用化的 hook（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 的 W906_WindowEdgeRegister），但它要先補三件事：**取樣改成「確定開著」**（見下面誤判 1）、**呼叫時帶表單名**（不然 21 頁要寫 21 支小函式）、**登記表 16 格不夠**（21 頁＋St02 的 Tester I/F）。
     - (b) 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 檔尾自己寫一支同規則的小迴圈，接在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行。
  3. 開頁指令那裡不再記：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:57`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:256`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp:353`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:493` 四處同一行改。
  4. ctest：打開／關掉邊緣、全部過期沿用上一拍、兩個分頁取聯集、沒建立的視窗不算開、被開窗閘擋的不記、21 個表單名在 `D:\HT9045\web\background.html` 都找得到（防止改名後靜默對不上）。
- **改哪些檔與主人**：全部是 St01 的段（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 檔尾兩段、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`／`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h`、四個呼叫點都是 St01 加的同一行）；視窗總表 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`（Jimmy／EastSun）只讀它的查詢函式、不改。⇒ **St01 自己做得完**，不用等別人。
- **延遲**：網頁狀態改變後 60 毫秒送清單（`D:\HT9045\web\background.html:847`）＋主迴圈最多 0.5 秒（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:2931`）⇒ 大約 0.6 秒內記下（由常數推得，未量測）。golden 是按鈕當下。
- **誤判與處理**：
  1. **沒建立的視窗會被當成開著**：總表的保守答案對「清單裡沒有這個表單」回答「開著」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:226-237`），那是給 START 閘的安全方向。Bar Code、Ground Man 在沒裝該功能的機台上不建立視窗（3.3），照保守答案取樣，第一份清單進來就會「打開」一次、**記一筆假的「Enter 2D Bar Code Form」**，之後永遠算開著。⇒ 記事件要用相反方向：只有「新鮮的清單明說 open 或 minimized」才算開著；不知道＝沒開；全部過期＝沿用上一拍（同 S122）。這一點也是 (a) 掛通用 hook 前必須先補的。
  2. **兩個 HMI 分頁（或兩台電腦）**：總表取聯集，甲分頁開著時乙分頁再開同一頁不記；golden 只有一個畫面，沒有這種情況。
  3. **HMI 重新整理（F5）時頁正開著**：舊連線的清單 15 秒內還算新鮮 ⇒ 這 15 秒內再開不記；15 秒後算關掉，之後再開會記。
  4. **瀏覽器被節流或整個關掉**：全部過期 ⇒ 沿用上一拍，不會亂記；下一個 HMI 連上後照新清單。
  5. **單獨打開的頁（沒有外框，只有開發時用）**：沒有清單 ⇒ 不記（偏離，寫明）。
  6. **最小化算開著**（視窗總表契約 §3）：縮小再叫回不記——golden 最小化也不跑開頁。
  7. **從別的頁開**：經 Teach 頁開 Bar Code、經 Contact 頁開 Temp Offset，golden 那兩顆鈕不記；網頁分不出從哪裡開，照主路記（跟現在一樣，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:845-847`「偏離 1」）。
- **工作量**：St01 約半天到一天（含 ctest）。
- **風險**：只影響「什麼時候記 Enter」；權限、欄位、存檔都不變。**R110 自然消失**：Configuration 存完視窗還開著、沒有「打開」那一下 ⇒ 不再多記一筆（golden 要再按一次開窗才記；網頁要關窗再開才記）。換登入後頁面仍是開站時的樣子（B 不解決這件事）。
- **例子**：08:00 開 HMI、10:00 開 Speed → 10:00 約 0.6 秒後記「Enter Speed」；若 10:00 登入的是 Operator 而這台 Speed 要 Engineer → 不記，主控台印一行「被擋：等級不足」。

### 選項 C：打開那一下 C++ 也重查開窗閘，再通知頁面「要重讀／不能開」

- **白話**：B 之外，C++ 在視窗打開那一下重查能不能開，並比對「頁面上的資料是用哪個等級開的」；不一樣就發一個訊號給頁面，頁面自己重讀或顯示「權限已變更」。
- **程式做法**：C++（St01）：同 B 的邊緣，把結果放進執行期資料串流（新增一個 tag，例如「某頁需要重讀」；要動 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp`，共用檔）。網頁：引擎 `D:\HT9045\web\page\ht9045_wire_engine.js`（Jimmy）要訂閱這個 tag，收到就重讀或顯示橫幅——23 頁沒有重讀鈕，實際上只能自動重讀。
- **改哪些檔與主人**：C++ 半邊 St01；頁面半邊 Jimmy ⇒ **St01 做不完**。
- **延遲**：打開後約 0.6 秒 C++ 看到＋下一次資料串流約 0.5 秒＋頁面重讀一趟 ⇒ 約 1～1.5 秒內畫面是舊資料。
- **誤判**：同 B；另外資料串流是全體廣播，兩個分頁時沒開那頁的分頁也會收到（無害，多一次重讀）。AOA、Handler System 看不到邊緣（3.3 小結）。
- **工作量**：C++ 半天＋頁面半天到一天＋一個新 tag。
- **風險**：機制比 D 多一層（C++ 推 → 頁面收 → 再要一次資料），最後的效果跟 D 一樣是「打開時重讀」；golden 本身不在表單開著時因換人登入重查（3.4），存檔前重查已比 golden 嚴 ⇒ 安全上沒有多得到什麼。
- **例子**：同 1.3 例 1——10:00 工程師開 Yield Monitoring，約 1 秒後頁面自動重讀、顯示可以改的欄位。

### 選項 D：網頁引擎改成「視窗打開才向 C++ 要資料」（Jimmy 的引擎）

- **白話**：設定頁開站時照樣先載好藏著（畫面框架不變），但**先不向 C++ 要資料**；操作員按開窗那一下才要，每次關掉再開就再要一次。這樣 golden「每按一次開窗就重查、重跑開頁程式」就對上了：當下的登入等級、運轉狀態、Contact 開頁換算、Teach 開頁清回原點旗標，都在開窗那一刻發生。
- **程式做法**（Jimmy 的引擎 `D:\HT9045\web\page\ht9045_wire_engine.js`，估約 20～40 行）：
  1. 引擎接上時（`D:\HT9045\web\page\ht9045_wire_engine.js:2108` `load();`）如果頁面嵌在外框裡，先不要資料；
  2. 聽外框**已經在送**的「你的視窗開了／關了」訊息（HT_WIN：`D:\HT9045\web\background.html:665-683` 每次狀態改變都送、`D:\HT9045\web\background.html:1200-1201` 頁面載好補送一次目前狀態）；從「沒開」變成「開著」就要資料；
  3. 幾秒內都沒收到這個訊息（舊版外框）就照現在的做法直接要。
  先例：Motor Test 頁已經這樣做（`D:\HT9045\web\page\HW.MotorTest.html:995-1019`）。**外框不用改。**
- **C++ 端**（St01）：開窗閘、欄位等級不用改（每次要資料本來就照當下等級重查）。「Enter」只會在第一次開時記——因為 C++「這頁開過了」旗標（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:27-31`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:59-60`）只在 golden Close() 才清（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:211`）。要每次開都記：搭 B；或在「關掉」那一下把旗標清掉（只能等 D 上線後做，否則重開時不重讀的頁，存檔會被要求重讀）。
- **改哪些檔與主人**：引擎是 Jimmy 登記的檔 ⇒ **St01 做不完，要請 Jimmy**。
- **St01 能自己做的替代（D′）**：新增一個 St01 的小 script，聽同一個開關訊息、打開時呼叫引擎現成的重讀（`window.HT9045Page.load`，`D:\HT9045\web\page\ht9045_wire_engine.js:2129`），24 個 C 路頁的 HTML 各加一行載入它。缺點：開站那一次照舊要資料（開站記 Enter、Teach 開站清旗標都還在），開窗時再多要一次；要動 24 個頁面檔。只建議在 Jimmy 排不出時間時當過渡。
- **延遲**：打開後一趟來回（本機 WebSocket＋golden 開頁程式）欄位才出現，通常不到 1 秒（未量測）；golden 也是開頁程式跑完才顯示。
- **誤判**：不靠 C++ 的視窗總表，兩個分頁各開各的、各要各的，不受 15 秒過期影響；最小化不算關（跟 golden 一樣）；AOA、Handler System 也適用（外框的開關訊息看的是視窗，不看表單名）。
- **工作量**：Jimmy 約半天（含自測）；St01 0～半天（看有沒有做 B）。網頁探針 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\` 裡若有假設「開站就要資料」的，要跟著改（**未查**）。
- **風險**：開頁程式的副作用改到開窗時發生——**都是照 golden，但行為會變**：Configuration 開頁會把執行中的設定重讀回 lastdata.dat 的值（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5200-5202` 的註解；golden 開頁亦同）；Contact 開頁換算（R84）在開窗時跑；Teach 開窗才清回原點旗標、開 HMI 不再清。開站變快（開站少跑 24 次 golden 開頁程式）。
- **例子**：同 1.3 例 1——10:00 工程師登入、開 Yield Monitoring，頁面當場向 C++ 要資料、以 Engineer 等級顯示可以改的欄位，跟 BCB 版一樣。

### 選項比較

| | A 維持 | B 看總表記 Enter | C C++ 通知頁面重讀 | D 打開才要資料 |
|---|---|---|---|---|
| Enter 記在開窗那一刻 | 否（開站） | **是** | 是（含 B） | 只有第一次；搭 B 才每次 |
| 換登入後打開，頁面是當下等級 | 否 | 否 | 是（約 1～1.5 秒後） | **是（當場）** |
| Teach／Contact 開頁副作用在開窗時 | 否 | 否 | 否（開站那次還在） | **是** |
| St01 自己做得完 | — | **是** | 否（引擎） | 否（引擎；D′ 可以但有缺點） |
| 工作量 | 0 | 半天～1 天 | 1～2 天 | 半天（Jimmy）＋0～半天 |
| 會不會寫錯檔 | 不會（存檔前重查） | 不會 | 不會 | 不會 |

---

## 5. 建議

1. **B＋D**：
   - St01 先做 **B**（St01 自己做得完、半天到一天）。不急，但要在事件紀錄真的開始寫檔（R109）之前做完，免得客戶紀錄裡出現開站時的假「Enter」。做法選 (a) 或 (b) 看今晚的通用 hook 能不能補上「確定開著」取樣、帶表單名、容量；補不上就用 (b)。
   - 同時請 Jimmy 排 **D**（根本解）：順便解掉「換登入要整個重新整理」、R84 開頁換算時機、Teach 開 HMI 就清回原點旗標。B 和 D 不衝突：D 上線後 B 照樣負責「什麼時候記 Enter」，不用重做。
2. **C 不做**：golden 在表單開著時換人登入不重查（3.4）；存檔前重查已比 golden 嚴；C 要動 Jimmy 的引擎，效果跟 D 一樣但多一層。
3. 連帶：R110 在 B 之後自然變成「不多記」；R89～R93 的開窗閘內容不用改，只是 D 之後「查的時間點」對了；R84 的做法不用改，D 之後時機自動對。

---

## 6. 要 Steven 決定的題目全文（decisions-pending 的 Q49）

### Q49. 設定頁的「開窗」要怎麼對準操作員真的按開窗的那一刻（R108 延伸；R89～R93、R84、R110 同一個前提）

**背景**：
- BCB 版每按一次開窗鈕，當下做三件事：查能不能開（運轉中、登入等級）、記一筆「Enter …」事件、依當下等級決定哪些欄位能改。
- 網頁版在開站（HMI 載入）時就把 24 個設定頁在背景載好、藏起來，每頁一載好就向 C++ 要資料，C++ 那時才做這三件事；之後操作員按開窗只是把頁顯示出來（`D:\HT9045\web\background.html:906-907`、`D:\HT9045\web\page\ht9045_wire_engine.js:2108`、`D:\HT9045\web\background.html:746-771`）。
- 結果：(1)「Enter …」記在開站（今天還沒寫進檔，見 R109）；(2) 換人登入後，頁面還是開站時那個等級的樣子——等級升高看到「讀取失敗」或灰掉的欄位；等級降低時欄位看似能改，按存檔才被 C++ 擋（不會寫錯）；24 頁只有 1 頁有重讀鈕，其他要整個 HMI 重新整理；(3) Contact 開頁換算、Teach 開頁清「要重新回原點」也在開站跑（模擬組態下每次開 HMI，下次 START 都會先全部回原點；由程式推得、未實測）。
- BCB 版在表單開著時換人登入也不會重查（閒置自動登出只關 Offset 視窗：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26022-26048`；換等級只重算主畫面按鈕：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:12926-13191`），只有一個客戶選項會在存檔時擋 Operator（例 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14181-14192`）。網頁版每次存檔都重查，已經比 BCB 版嚴。
- 評估全文：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`。

**選項**：
- **A** 維持現況。
- **B** C++ 看網頁送來的「哪些視窗開著」清單，某頁從「沒開」變「開著」的那一下才查權限、記「Enter」；開站不再記。只改 St01 的檔（主要是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`），半天到一天。只修「記事件的時間」，換登入的問題不變。
- **C** B 之外，C++ 在打開那一下重查權限，並通知頁面重讀。要改 Jimmy 的網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 和一個共用檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp`；畫面約 1～1.5 秒後才更新。
- **D** 請 Jimmy 把網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 改成「視窗打開才向 C++ 要資料」（Motor Test 頁 `D:\HT9045\web\page\HW.MotorTest.html` 已經這樣做）；每次開窗都照當下等級重查、重跑開頁程式，跟 BCB 版一樣。Jimmy 約半天。

**St01建議**：**B＋D**——St01 先做 B（事件紀錄開始寫檔之前做完就好）；請 Jimmy 排 D。C 不做（BCB 版也不在表單開著時重查；效果跟 D 一樣但多一層）。B 之後 R110「Configuration 存完多記一筆」會自然消失。

**例子**：出貨機台 08:00 開機（登入是 Operator），這台的權限表把 Yield Monitoring 設成 Engineer 以上；10:00 工程師登入、打開 Yield Monitoring。
- BCB 版：10:00 記「Enter Yield Monitoring」，畫面以 Engineer 等級打開、可以改。
- A：08:00 開站時沒過權限、沒記；10:00 也不記；畫面是 08:00 的「等級不足、讀取失敗」，要整個 HMI 重新整理。
- B：10:00 記；畫面仍是 08:00 的樣子。
- C：10:00 記；約 1 秒後頁面自動重讀、可以改。
- D＋B：10:00 記；頁面當場以 Engineer 等級打開、可以改——跟 BCB 版一樣。

**目前狀態**：A（現況，commit `342779cc`，主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）；等 Steven 決定。

---

## 7. 沒查證／推論的地方

- 「模態表單開著時主畫面點不到」「計時器在模態表單開著時照樣跑」是 VCL 的一般行為，本次沒有在樹內另外驗證。
- Teach 開站清「要重新回原點」旗標（模擬組態）是由程式碼推得，沒實際跑 HMI 驗證。
- 「開站一次記最多 21 筆」要看開站當下哪些頁的開窗閘有過（取決於開機等級與這台的權限表），沒實測。
- 延遲數字（0.6 秒、1～1.5 秒、不到 1 秒）是由程式裡的常數推得，沒量測。
- 今晚的通用 hook（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 工作樹）還沒 commit，登記表容量、callback 形狀、取樣規則以它 commit 後的樣子為準。
- 網頁探針 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\` 有沒有假設「開站就要資料」沒查。
- 機台上（kiosk、沒有實體鍵盤）操作員能不能按 F5 重新整理沒查。
- 各機台權限表（`D:\HT9045\system\levelset.dat`）的實際值沒查；例子裡「Yield Monitoring 設成 Engineer 以上」是假設。

## 8. 20260929 現況：已做成（Q49 由 Q51 涵蓋；B10a commit `ff497e5d`）

> 上面 §1～§7 是 20260927 的評估，選項已被 Q51（頁面狀態表）取代。現在的做法：

- **開窗邊緣**：頁面狀態表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`，commit `6273f82f`／`87a625f0`）知道網頁視窗什麼時候真的開；C 路設定頁在真的開窗時才跑 golden FormShow、記「Enter …」（X-4＝R108／R110，已由 `87a625f0` 做完；ctest OpenEnterLog）。
- **關窗邊緣**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 頁面表的邊緣掛勾先呼叫 `W906_EvB10A_WindowEdge(form, open)`，再清 C 路的「開過了」。表與純判斷在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\WindowEdgeTails.h`（kRows：fYieldMonitoring、fSetup、fOffSet、fContact、fConfiguration），分派在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` 檔尾。要加一個表單：表加一列、MainClick 的函式對照加一行、各結構檔尾寫 `FileRW_<結構>_WindowEdge(bool open)`；ctest EvB10A_Edges 會比對個數。
- **規則**：
  - 這一次開窗 golden FormShow 有跑過才跑關窗程式（C 路頁 `filerw::PageShownNow`；Offset／Configuration 用 `evb10a::OpenLatch`）。開窗閘拒絕（等級不足）⇒ 不跑。
  - `skipWhileRunning`：golden 模態視窗（Yield、Setup、Configuration）運轉中（SystemStart||SoftStart）看到的關窗不跑、只印一行（同 R86）。Offset、Contact 照跑。
  - F5 在頁面表裡＝全部視窗先關再開 ⇒ 關窗程式會跑。Contact 的 golden FormClose 會 SystemStart=false、CarlibrationTask!=1 時 fAllMotorHome=false（要重新回原點）——跟 BCB 關 Contact 視窗一樣。
  - BCB「不准關」（例：SetUp 的 Auto Shuttle Sensor 要重設）只能在頁面的 Exit 鈕做（form.event `sbtExit`，頁面收到 ack `closed:false` 就不關）；按視窗 ✕ 時網頁已經關了，只能跑 FormClose。
  - 每一個邊緣主控台印 `[EVB10A] <表單> opened|closed -> …`；golden 訊息／待辦逐行印（沒有 WS 回覆可以帶）。
- **B10b 已做（commit `f14484e1`）**：X-3 開頁／讀檔時程式設值照 VCL 觸發 golden OnClick（TrayAssignment、Configuration；R100／R118）、X-5 Temp_Set 切分頁（R130）、CC-L1 Configuration 切頁依 `D:\HT9045\config\Security_new.def` 整頁鎖（Q46）。**還沒做**：SA-2 SCK_ART（計時器＋TSV 伺服器是執行期流程，不是頁面事件）；R129（Offset 藏起來的分頁，沒有人讀那個旗標；D-013 `4b676a25` 也判定不做：Offset 切頁沒有 C++ 事件，改「藏起來的 ActivePage 算看得見」會動到每一個 C 路頁）；其他頁的 X-3（ArmSpeed、DeviceForm、HSys、Temperature 其餘、BarCode、Cleaning、SetUp…要逐頁看 BeforeApply／SaveFlow 會不會重跑）；其他 C 路頁按 ✕ 還不跑 golden FormClose（⛔ 20260929 B10c `6d0dfbb1` 已補 14 頁，見下一段）；主選單 sbSettingClick／sbConfigClick 的運轉中／權限／SECS EnterTool 閘。

- **B10c 已做（commit `6d0dfbb1`，20260929）**：kRows 第 6～19 列——(A) fSpeed、fLd_ULd、fTrayForm、fTrayAssignment、fTemp_Set（從 Contact 開是非模態 ⇒ 運轉中也跑）、fDIOFrom、fQAMode、fBarCode、fVacuumUnit、fBinSel、HandlerSystem（HS_FormClose 經 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\HSys.py` 新翻）；(B) fCounterSel（關窗本身就是存檔）、fStartCondition（golden ✕ 會存檔）、fCleaning（SetWorkParameter）。
  - 新的共用規則：`filerw::PageCloseEdgeRefused(tag)`——golden FormShow 這一次開窗有跑、而且 FormClose 還沒跑過才跑；「跑過」由 `closeRan` 記（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 標 "closed" 的地方與 CounterSel 的存檔包裝設，PageWindowClosed 清）。
  - StartCondition 守衛：這一次開窗期間有生產（SystemStart||SoftStart，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp:359` 每拍取樣）⇒ 關窗不跑 golden 存檔、頁面四個存檔鈕一律拒存（「生產中開著這一頁，計數已經變了；請關掉重開這一頁再存」），免得 LastSet.iContactCT 被寫回開窗時的值（golden 是模態，走不到這種情形）。
  - BinSelect 補進 kOwnEntryForms：以前關窗不重置，「Enter Bin」（MES2171）只在開機後第一次開記。
  - 沒做（給 Jimmy／別人）：fTeach（Jimmy 的 5(b) 走 W906_WindowEdgeRegister，a4581654＋6d16e728）、fShuttleMove（原生頁）、fGroundMan（RS232 重開）、fContactForce（ADAM EP）。待補：BarCode 的 Exit 額外動作（sbtExitClick 重設 2DID 檢查工作）要 form.event sbtExit。（⛔ 20260930 已補，見下一段）

- **B10c 後續：BarCode 的 Exit 鈕（20260930，St01 工程師做、ST01-E 核對後 commit）**：Setup.BarCode 的 Exit 鈕照 golden `TfBarCode::sbtExitClick`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:2408-2417`）跑，寫法跟 SetUp 的 SU-9、Temp_Set 的 TS-10 一樣。
  - golden 做的事：`Close()`（→ FormClose：bShow=false、tmr1 關、DoIniDataToForm）、bShow=false、`i2DIDCheckSH1Task=1`、`i2DIDCheckSH2Task=1`、兩顆「Cheack 2DID SH1／SH2」鈕（btStart2DIDCheckSh1／Sh2）重新可按。golden 這支沒有「不准關」的條件，所以一定關窗。
  - C++：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_BarCode.py`（methods 加 sbtExitClick、逐行 `_expect` 釘住 golden、Down 轉 `;`、Close() 轉「記 closed＋BC_FormClose」、events 加 sbtExit）→ `gen_editlist.py --only TestIF_File_BarCode` 重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.gen.inc`（等冪）；sbtExit 的替身在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.cpp` 的 BC_EvBootProxies 建（產生器不建：golden 處理器裡提到 sbtExit 的那一行已轉成 `;`，同 SU-9）。兩個全域是 golden `BarCode.cpp:46-47` 的，補在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\BarCode\BarCode.cpp` 檔尾；golden `BarCode.h:1008-1009` 的 extern 沒補進移植樹 `BarCode\BarCode.h`（它經 aHotPlateSubstrate.h 進 221 支 TU），產生檔自己前置宣告。
  - 頁面：`D:\HT9045\web\page\ht9045_barcode_ev.js` 的 Exit 段——捕獲階段攔下 `.exitbtn`，送 form.event `{"form":"TfBarCode","control":"sbtExit","event":"click"}`，回 `closed:true` 就關窗；伺服器沒有這個事件、回 reload page 或 running ⇒ 直接關（關窗邊緣照 golden 跑 FormClose）；其他失敗說原因、1.2 秒後照樣關。
  - 跟關窗邊緣的交接：Exit 的 golden Close() 記了 closed ⇒ `closeRan`，頁面關窗時 `FileRW_BarCode_WindowEdge` 經 `filerw::PageCloseEdgeRefused` 不跑第二次 FormClose；按 ✕（沒按 Exit）照舊只跑 FormClose（golden ✕ 不跑 sbtExitClick）。
  - 跟 golden 不同（寫明）：運轉中（SystemStart||SoftStart）form.event 一律回 running（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp:77`）⇒ 頁面直接關，只跑 FormClose，那四行（兩個全域、兩顆鈕）沒跑。今天沒有影響：兩個全域在 golden 只有 State Record 的 task 環讀（main.cpp:10412-10413），移植樹那兩列還閘著（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cStateRecord.cpp:1944-1949`，Jimmy 的 GATE(W906-TASKLIST)）；兩顆 2DID 檢查鈕的處理器（btStart2DIDCheckSh1Click、Do2DIDCheck）沒翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:31425` GATE）。
  - 給 Jimmy：cStateRecord.cpp 那兩列的閘理由「移植樹沒有這個變數」現在不成立，解閘時在 `BarCode\BarCode.h` 補 golden 的 extern、TaskListRegister 的列數 265 → 267。
  - 測試：ctest FormEvent_Position [18]（Exit 沒有擋關條件：closed:true、FormClose 一次、兩顆鈕在 changed 裡變回可按、之後關窗邊緣拒絕再跑、清掉後重開 ✕ 又會跑）、EvB10A_Edges [7]（原始碼棘輪：產生檔處理器照 golden 順序、事件表一列、替身與 Panel1 父層、兩個全域有定義、頁面攔 Exit 送 form.event；用壞版本對照跑過，4 項會紅）；頁面 Node 替身冒煙 17 項（scratchpad，不是正式測試）。沒有跑 wb_serve、沒有上機。
