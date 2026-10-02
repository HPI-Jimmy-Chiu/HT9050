# 評估：只替「開著的畫面」送資料（C++ 依頁面表減少 JSON 通訊）

> 讀者：Steven（結論）、Jimmy（逐段看、逐項同意或不同意，§10）。撰寫：ST01-E 派的工程師，20260930 15:4x～16:4x。**只讀研究＋這一份文件：沒有改程式、沒有 build、沒有跑 ctest、沒有跑 wb_serve、沒有寫機台檔、沒有 commit。**
> Steven 20260930 15:3x 原話（ST01-M 轉）：「C++能根據這個表, 減少JSON的溝通資料嗎? 有開的才進行通訊」。同日追加（ST01-M 轉）：另寫一節「跟 C++ 原生畫面怎麼搭」（§5）；**任何程式都要先等 Jimmy 同意**。
> Jimmy 16:1x 已同意方向、附四個條件（§0.1 原話照錄），本檔每一個條件都有一個標了「條件 (a)～(d)」的獨立段落可以對。
> ⛔ **20260930 20:4x 起改由筆電實作**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260930.md` 第 12 條，main `1742fc1e`；Jimmy：「你能夠全部接手做嗎？後面通知st01按照我們做法」「有開的網頁才能更新資料」）：本檔改成筆電實作時的**現況調查參考**，§0、§6 的「誰改」已過期。St01 已寫的第 2 階段 E（`6ea7f1eb`）與第 1 階段（`ca661a18`，分支 `v906/st01-stream-s1`）在 review6 上 revert（`b56e1165`／`c43cee9e`），筆電自己重套 2E、參考第 1 階段（TO_STEVEN §4 20260930 21:0x）；第 2 階段 A 沒有寫。St01 之後不做新的串流／輪詢／發布改動，等筆電在 TO_STEVEN §4 寫做法再照做。2A 工程師唯讀查到的細節見 §12 最後的補記。
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6`，commit `b45d4482`。移植樹的檔一律用 `git -C D:\HT9045 show b45d4482:<路徑>` 讀（工作樹同時有別人在改 `tools\wb_serve.cpp` 等檔）；本文引用的網頁檔在工作樹與 `b45d4482` 相同（工作樹只有 `D:\HT9045\web\page\ht9045_agv_c.js` 是髒的，本文沒有引用它）。
> 三棵樹的寫法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6，cp950）；`D:\HT9045\web\` 開頭＝**網頁**。
> 數字的標法：「量到」＝有檔或紀錄可查；「推算」＝由程式形狀 × 頻率算出來、沒有接伺服器量；「沒查證」＝沒有證據，只是推論。

---

## 0. 給 Steven 的一段話

**可以做，Jimmy 也同意方向（附四個條件，本檔全部照做）。** 今天 C++ 每 0.2～0.5 秒把約 7,100～8,100 個畫面資料項（tag）整批重新整理一次、推給每個瀏覽器分頁，其中約四分之三（`pci1203.*` 的 5,886 個）只有平常關著的「1203 設定頁」在看，而這一段整理就跑在機台主執行緒上（Jimmy：點擊延遲最多 43.6 ms 花在這段）；另外 IO 頁（每 0.2 秒拿一次、每次約 11～16 萬位元組）和 Teach 頁（每 1 秒拿一次、約 4.5 萬位元組）就算視窗關著也一直在拿。做法分三段：**先量**（加計數與每 10 秒一行紀錄，不改任何行為）、**再改**（1203 設定頁沒人開就在「整理」那一步跳過那一大塊；IO 頁與 Teach 頁照 golden `Timer1Timer` 的「`fShow==false` 就 return」關窗不拿；Motion View 托盤由 St02 自己改）、**量完再說**的留到第三段。典型的生產狀態（主畫面＋一兩個設定頁）每個分頁的通訊**推算**從約 0.6～0.9 MB／秒、45 則／秒降到約 22～36 KB／秒、36～41 則／秒（位元組少約 95%；則數只少約 15%，因為告警信箱每 0.1 秒要看一次，那一條不能停），主執行緒每次整理 tag 的量少約 73～83%。**永遠不停的**：主畫面狀態、告警／訊息框、Exit 關站蓋層、登入等級、心跳與權杖、頁面表自己的開關回報，以及不看網頁的使用者（事件記錄分析、SECS SV、REST API、TCP 7016／7017）讀的一切（§3 逐項附檔案行號）；不確定是不是「只有那一頁在用」的，一律照送。**誰改**：照 Jimmy 的預設，程式由 St01 寫（含筆電的檔，先在 FROM_STEVEN §1 認領確切行號），筆電審認領並跑兩組態 gate 才合；St02 自己改托盤那約 5 行；動到機台端常設不碰的 `WebBridgeTags.cpp` 那一段與 IO 頁要知會 EastSun。**風險低**：只是少送，golden 邏輯沒有任何一處看「資料有沒有送出去」；最主要的風險是「剛打開那一下看到舊值或 "---"」，設計成開窗那一圈就補齊。原生畫面（`W906_NATIVE_FORMS`）不影響本案：原生視窗直接讀記憶體，不吃這些資料。

### 0.1 Jimmy 的四個條件（原話照錄）與本案怎麼照做

Jimmy 原話（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` §0 第 7 列，main `1d596436`，20260930 16:10）：

> **St01 提案：只送「有開著的頁面」的資料**（Steven 15:44 的信；原生頁面開著時對應的網頁串流也不送）｜動到筆電的串流檔（`wb_serve.cpp`、`WebBridgeTags.cpp`、JsonBridge）。評估：可行、值得做（現在每拍整理約 7,000 個 tag，點擊延遲最多 43.6 ms 花在這段）；條件四個：在「整理」那一步就跳過、不看網頁的使用者（事件記錄分析、SECS SV、REST API、TCP 7016／7017）列進不能停、不確定就照送、先量後改｜**①同意方向＋四個條件，等他的評估文件出來筆電逐段看過才寫程式；②St01 改、筆電審認領＋兩組態 gate 再合**。你沒回之前什麼都不改｜Steven 信 15:44；St01 評估文件寫好會貼在 FROM_STEVEN §3

| 條件 | 本案的做法 | 在哪一節 |
|---|---|---|
| **(a) 在「整理」那一步就跳過** | 找到整理的迴圈 `PublishHandlerTags`（`WebBridgeTags.cpp:669-2334`），1203 那一大塊在 `:1360-2314`；閘門放在那一段的入口，沒人開就**不整理**（不只是不送） | §2.6、§4.3 |
| **(b) 不看網頁的使用者列進不能停** | 四個使用者逐一查了「從哪裡讀」：都**不讀** tag 表；但 SECS SV 的 `secs.sv.*`、事件記錄的 `log.*`、REST API 讀的 IO／馬達 JSON 快取、REST 目錄公開的 `io.*`／`motor.*`／`prod.*` 一律照整理 | §2.7、§3 第 10～14 項 |
| **(c) 不確定就照送** | 只有「確定只有某一頁在用」才閘：`pci1203.*`（只有 1203 設定頁）與 `motionView.trays.*`（只有 Motion View，St02 確認）；`io.*`、`motor.*` 因為 REST 目錄對外公開，**照送**；查詢本身也是「不知道就回要」 | §4.1、§4.2 |
| **(d) 先量後改** | 第 1 階段只加計數與每 10 秒一行 `[STREAM]` 紀錄（整理幾個、花幾毫秒、送幾位元組），前後各量一次；本案現在不跑 wb_serve | §4.0、§9 T0 |

---

## 1. 背景（白話）

- **tag 串流**：C++ 把畫面要用的值整理成「名字→值」的一張表（tag），用 WebSocket 推給瀏覽器：連上時送整張（snapshot），之後只送有變的（patch）。整理這張表的是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp` 的 `PublishHandlerTags`，**跑在機台主迴圈那一條執行緒上**（golden 狀態機、1203 讀卡、輸出也都在這一條）。
- **HTTP 輪詢**：有些頁面自己定時用 HTTP 來拿一整份 JSON（IO 點狀態、馬達狀態、告警信箱）。
- **頁面表**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`（St01，設計 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md`）記住每個網頁視窗「開／縮小／關」，資料來自網頁外框 `D:\HT9045\web\background.html` 每次開關就送的「視窗總表」（`ui.windows.put`），收在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`（Jimmy）。
- **一個分頁一條 WebSocket**：20260929 起外框 `D:\HT9045\web\page\ht9045_link.js`（St01 ST01-E3，WSLINK hub）替整個分頁開唯一一條連線，iframe 裡的頁面都經過它（以前每個 iframe 自己開，0926 在機台量到 23 條連線，`ht9045_link.js:5-7`）。

---

## 2. 今天送了什麼（查證）

### 2.1 tag 串流：誰在讀哪一家

總數：0924 普查 7,082 個（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\CHANNEL_CENSUS_20260924.md` 表 A，筆電、沒有卡、閒置 25 秒）；0929 沙盒開機印 `published 8088 tags`（`D:\AI_TempFile\q50-run\20260929_210245\r\r1\console.txt:144`）。

| 家族 | 個數 | 在哪裡整理 | 誰在讀 | 本案 |
|---|---:|---|---|---|
| `pci1203.*` | 5,889（32 軸×約 110＋DI 320×5＋DO 192×3＋48 站＋卡片層；常數 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h:1686-1689`） | `WebBridgeTags.cpp:1360-2314`（1203MON 區塊；`pci1203.linked`／`enabled`／`phase` 三個在 `:1395-1402`，註解寫明「永遠發」） | **只有** 1203 設定頁 `D:\HT9045\web\pci1203.html`（`D:\HT9045\web\js\pci1203.js`、`D:\HT9045\web\js\pci1203\view.js`）；視窗 `pci1203` 是 lazy，關窗就卸載（`D:\HT9045\web\background.html:482`、`:783-797`）。`grep` `pci1203\.` 全 `D:\HT9045\web\`：其他命中只有註解與說明頁；C++ 端只有被動量測工具 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\channel_census.py` 會讀。不在 REST 目錄裡（`tools\wb_serve.cpp:2617-2640`） | **閘**（B，5,886 個；三個卡片層照發） |
| `io.di`／`io.do`／`*.valid`／`io.ver` 等 | 7 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanIo.cpp` `StageIo`，`wb_serve.cpp:2893` | 網頁只有 `D:\HT9045\web\js\pci1203.js`；但 REST 目錄 `/api/struct` 對外公開（「S9 io.di/io.do bit-packed base64; /schema only」，`wb_serve.cpp:2617-2640`） | **照送**（條件 c：不是確定只有頁面在用） |
| `motor.axes`／`count`／`ver` | 3 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMotor.cpp:40-92`，`wb_serve.cpp:2894` | 網頁沒有人讀（0924 普查表 A；本次 `grep` 0 處）；但 REST 目錄公開（「S9 motor.axes ordered array」，同上） | **照送**（條件 c） |
| `motionView.trays.*`＋`motionView.screenScale` | 34＋4（St02 回覆） | St02 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.cpp`，`wb_serve.cpp:2894` | 只有 Main.MotionView（`D:\HT9045\web\page\ht9045_mv_trays.js`）；St02 確認只做顯示 | **閘**（C，St02 自己改） |
| `secs.sv.*` | 741 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SecsTagPublish.cpp:105`（Jimmy P4），`wb_serve.cpp:2880` | 網頁今天沒有人讀（0924 普查表 A 第 7 項） | **永遠照整理**（條件 b：SECS SV） |
| `log.*` | 4 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\EventLog.cpp:173`，`wb_serve.cpp:2882` | 事件記錄游標 | **永遠照整理**（條件 b） |
| `temp.*`／`zone.*`、`def.*` | 217／8、61 | `JsonBridge\StageThermo.cpp`、`JsonBridge\ChanConfig.cpp` | 部分有人讀；多半 null 或開機後不變，不進 patch | 照送 |
| **主畫面群**：`machine.*`、`guard.*`、`tower.*`、`clock.text`、`pump.*`、`auth.level`、`user.level`、`control.owner`、`recipe.current`、`ui.pages`、`mainsb.*`、`lot.*`、`sort.*`、`tcat.*`、`bin.*`、`binsel.*`、`prod.*`、`alarm.*`、`cfg.ver`、`build.*` | 約 1,000～1,300 | `WebBridgeTags.cpp:669-1358`、`:2340-3497`、`wb_serve.cpp:2881-2895` | 主畫面 `D:\HT9045\web\page\main.html`、外框、開站就開著的旁邊視窗（Sort Count、Contact Counter、Lot Info、機況監視列、Show Message、Tester Category、BinSelect） | **永遠照整理**（§3） |

**多久整理一次**：主迴圈只在「有事」才整理並發布：500 ms 拍子（`kServeTickMs`，`wb_serve.cpp:2931`）、有卡時 200 ms 的 1203 讀取（`kIoTickMs`，`wb_serve.cpp:2940`；`ioPolled` `:4645`／`:4657`）、每一批網頁指令、每一次輸出（`wb_serve.cpp:5938-5948`）⇒ 有卡的機台約 **5～7 次／秒**，沒有卡的 SIM 約 2 次／秒（0924 普查：25 秒 50 個 patch，量到）。

**伺服器怎麼送**：每個世代算一次差量，同一個基準的連線共用同一份（WSFANOUT，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1498-1535`，整張表在 `:1511` 複製一份再比）；沒變就不送；消失的 tag 在下一個 patch 送成 null（`:1526-1527`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\TagSnapshot.h:155-159`）。新連線先收整張 snapshot（`WebBridgeServer.cpp:1145-1150`），再補發還沒回答的告警框（`:1157`）。

**有多大**（推算，腳本在暫存目錄，照 `WebBridgeTags.cpp` 的 tag 名形狀算）：
- snapshot 約 **200～230 KB**，其中 `pci1203.*` 約 170 KB（約 75～85%）。
- patch：沒有卡、閒置＝25 秒內 87 個 tag 變過（0924 普查，量到），每個 patch 幾百位元組。有卡：每次讀卡至少讀取計數與四段耗時等約 8 個 `pci1203.*` 會變，加上伺服開著的軸編碼器跳動、動作中的軸位置／速度／狀態；**推算每個分頁約 3～35 KB／秒，其中 `pci1203.*` 約 70～90%**。有卡的實測數字**沒有**（第 1 階段要量，§4.0）。
- 經過 hub 之後，C++ 每個瀏覽器分頁只送一份；hub 再用 MessageChannel 把每個 patch 複製給分頁裡**每一個** iframe（`ht9045_link.js:459-470`，含藏起來的），那是瀏覽器自己的 CPU。hub 也快取整張表，晚載入的頁面拿 hub 的快取，C++ 不必再送一次 snapshot（`ht9045_link.js:31-32`、`:459-467`）。

### 2.2 HTTP 輪詢

| 誰拿 | 拿什麼 | 多久 | 每次多大（推算） | 視窗關著還拿嗎 | 證據 |
|---|---|---:|---|---|---|
| IO 頁 `D:\HT9045\web\page\HW.IoSetView.html` | `GET /api/struct/io/runtime` | **200 ms** | 約 11～16 萬位元組：這台 `D:\HT9045\system\IO_Table.csv` 790 行、HT9050 的表 967 點（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\README.md` §3.2）×每點約 134～170 B（`D:\HT9045\web\JSON\IO-runtime.json` 同形狀樣本 643 點、壓成單行 86,762 B＝每點 134 B，量到；C++ 多送 `raw`、`source`、`updatedAt`） | **拿**：只看整個分頁有沒有被藏（`document.hidden`），不看視窗；開站就開始 | `HW.IoSetView.html:435-452`、`:513`；外框開站就載入所有非 lazy 視窗的 iframe、關窗只是 `display:none`（`background.html:916`、`:774-797`）；間隔照 C++ 報的 200（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanIoPoints.cpp:30`） |
| Teach 頁 `D:\HT9045\web\page\HW.teach.html` | `GET /api/struct/motor/runtime` | **1000 ms** | 約 45～50 KB（每軸約 72 個欄位；樣本 `D:\HT9045\web\JSON\Motor-runtime.json` 每軸 25 欄＝336 B，量到；這台 `D:\HT9045\system\Mot_Table.csv` 45 行、HT9050 48 軸） | **拿**：沒有任何視窗判斷；開站就開始 | `HW.teach.html:388-419`（`:400` 計時器、`:419` 開站呼叫） |
| Motor Test `D:\HT9045\web\page\HW.MotorTest.html` | 同上 | 500 ms | 同上 | **不拿**（照 golden `Timer1Timer` 的 `if(fShow==false) return;`） | `HW.MotorTest.html:848-851`、`:1286` |
| Motor View `D:\HT9045\web\page\Main.MotorView.html` | 同上 | 500 ms | 同上 | 不拿（看外框送的 `HT_WIN`） | `Main.MotorView.html:253-256`、`:286` |
| Motion View `D:\HT9045\web\page\ht9045_mv_motor.js` | 同上 | 500 ms | 同上 | 不拿 | `ht9045_mv_motor.js:32-35`、`:97-103` |
| IO 除錯頁 `D:\HT9045\web\page\IoLive.html`（不在外框裡，單獨開） | io runtime | 設定值 | 同 IO 頁 | 只有單獨開時 | `IoLive.html:173` |
| 外框的告警框橋 `D:\HT9045\web\page\dialog-bridge.js` | 信箱 `/JSON/Alarm-dialog-request.json`、`Message-dialog-request.json`、`Dialog-close-request.json`（C++ 寫在 `D:\HT9045\web\JSON\runtime\`） | **100 ms** | 三個檔 414～709 B（量到）⇒ 約 30 則／秒、20～25 KB／秒 | 拿（**永遠不停**，§3） | `dialog-bridge.js:5`、`:771-777` |
| 外框的生產資料 `D:\HT9045\web\background.html` | `JSON/Production-update.json` | 250 ms（上一個沒回來就跳過） | 見 2.5 | 拿 | `background.html:1362-1375`；`D:\HT9045\web\page\settings.js:32-44`、`:302-303` |
| Exit 關站蓋層 `D:\HT9045\web\page\ht9045_main_close.js` | WS `act.main.closeProgram {"op":"status"}` | 800 ms（只在關站中） | 小 | —（**永遠不停**） | `ht9045_main_close.js:1-37` |
| Contact Counter、Observer | WS 指令 | 1000 ms | 小 | 不拿（看 iframe 是不是被藏） | `D:\HT9045\web\page\ht9045_contactct_wire.js:219-231`、`D:\HT9045\web\page\ht9045_observer_wire.js:729-736` |
| Smart Diagnostic | WS `op timer` | 1000 ms（C++ 說計時器開著才跑） | 小 | 只看 `document.hidden`（沒查證開窗後何時停） | `D:\HT9045\web\page\ht9045_smartdiag_web.js:154` |
| Tray Edit、Tester Comm、Event Log | 各自的 HTTP／WS | — | — | 已經只在開著時拿（St02 16:02 回覆） | `D:\HT9045\web\page\ht9045_trayedit.js:34-36` |

### 2.3 其他 WebSocket 訊框

| 訊框 | 什麼時候 | 證據 |
|---|---|---|
| `alarm`／`modal`／`query`（告警、訊息框、要按鈕回答的框） | 事件發生時廣播；未回答的 query 在新連線補發 | `WebBridgeServer.h:252`、`:283`；`WebBridgeServer.cpp:1157`、`:1727`；`tools\wb_serve.cpp:440`（`ForwardShowErrorMessage`）、`:795` |
| ping／閒置斷線 | 每條連線 15 秒 ping、45 秒沒回應斷線 | `WebBridgeServer.cpp:387-388`、`:1576-1582` |
| 網頁→C++ 視窗總表 `ui.windows.put` | 每次開／關／縮小立刻送；連著時每 5 秒心跳；每秒檢查重連；離開頁面送「全部關」 | `background.html:665-670`、`:1120`、`:1132`、`:1141-1150`；每份約 4 KB（71 個視窗） |
| C++→網頁 `ui.pages`（頁面表整張） | 內容變了才進 patch | `WebBridgeTags.cpp:1263` |

### 2.4 C++ 自己在主執行緒上做、不管有沒有人看的

- **tag 整理**：見 2.6（條件 a）。
- **IO／馬達 JSON 快取**：每一拍（500 ms）與每一次讀卡（200 ms）之後都重做 `/api/struct/io/runtime` 與 `/api/struct/motor/runtime` 兩份（`wb_serve.cpp:6399-6425`，觸發在 `:4597`、`:4657`、`:6223`，執行在 `:5934`）⇒ 有卡時約 7 次／秒 × 約 16～21 萬位元組 ≈ **1.1～1.5 MB／秒的 JSON**（推算）。這兩份是 REST API（條件 b），本案**不動**，只在第 1 階段量它的成本。
- **伺服器（socket 執行緒）**每個世代把整張 tag 表複製一份再比（`WebBridgeServer.cpp:1511-1528`）；0926 機台量到這一段吃滿一顆核心、每個 HTTP 要等 0.7 秒（同處註解），WSFANOUT 之後改成每個基準比一次。

### 2.5 順便查到（跟開不開頁無關，但是 Steven01 這台目前最大的一條）

`D:\HT9045\web\JSON\Production-update.json` 在 Steven01 這台是 **8,649,139 B**（0914，JSON 模擬器時代留下，`D:\HT9045\.gitignore` 第 258 行排除，**C++ 沒有寫入者**：`git grep` 只有 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeRecipeDoc.h:75` 一行註解；`D:\HT9045\web\page\ht9045_home_refresh.js` 檔頭也寫「沒有任何 C++ 會寫那個檔」）。外框每 250 ms 拿一次（`background.html:1363`，Jerry 0929 加了「上一個沒回來就跳過」），每次加時間戳不用快取（`settings.js:35`），伺服器整份讀進記憶體再送（上限 64 MiB、`no-store`：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\HttpStatic.cpp:20`、`:324`、`:412`、`:440`）。⇒ 在 Steven01 這台理論上限約 34 MB／秒（實際被往返時間限制，**沒量**）。機台上有沒有這個檔**沒查證**；乾淨的機台是 404（每次約 100 B）。列在 §10 第 I 項給 Jimmy 知道。

### 2.6 條件 (a)：「整理」那一步在哪裡、花多少

- **整理的迴圈**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:669` `PublishHandlerTags` → `:675` `snap.beginPublish()`（清空暫存表，所以每一圈都要把想出現的 tag 全部重新放一次，`TagSnapshot.h:155-159`）→ 逐項 `stage*()` → `:2329-2330` 額外發布者（`wb_serve.cpp:2879-2897` 的 `PublishExtraTags`：SECS、設定、事件記錄、溫控、生產、IO、馬達、托盤、告警）→ `:2332-2334` `commitPublish` 並回傳整理了幾個。
- **1203 那一大塊**：`WebBridgeTags.cpp:1359` 讓出點之後、`:1360-2314`：卡片層（`:1388-1646`）、32 個軸槽每槽約 110 個（`:1648-1995`，每槽之後讓出一次）、DI 320 個 port 每個 5 個（`:2020-2078`）、DO 192 個 port 每個 3 個（`:2083-2112`）、48 個站（`:2169-2312`）。每一個都要 `snprintf` 組名字＋放進表。
- **誰在叫**：主迴圈 `wb_serve.cpp:5945`（每次有事）；開機 `:4265`；開卡過程中顯示進度 `:4422`。
- **花多少**：Jimmy「現在每拍整理約 7,000 個 tag，點擊延遲最多 43.6 ms 花在這段」；程式裡的出處是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\IoBtnPanelClick.cpp:177-180`（0925 14:19 點擊紀錄「其他」最多 43.6 ms，嫌疑是 7,084 個 tag 的整理），之後在 `:183` 加了「發布(publish)」欄、`wb_serve.cpp:5945` 前後打 `W906_IoTiming(7)`／`(8)`，又在整理中間加了讓出點先做輸出（`WebBridgeTags.h` `SetPublishYieldHook`、`wb_serve.cpp:6446-6464`）。**每一次整理實際幾毫秒，目前沒有一個平均／最大的數字**（點擊紀錄只在有點擊時記）⇒ 第 1 階段要補。
- ⇒ 本案 B 的閘門放在 **`:1360` 那一段的入口**（沒人開就整段不跑），不是在伺服器送出時才濾。

### 2.7 條件 (b)：不看網頁的使用者讀什麼

`git grep` 移植樹非測試碼：tag 表（`TagSnapshot`）只有伺服器在讀（`WebBridgeServer.cpp:1148`、`:1511`；`wb_serve.cpp:4343` `server.SetSnapshot(&snap)`），HTTP 路由不讀它；JSON 快取只有 HTTP 路由與操作紀錄在讀（`wb_serve.cpp:2553-2554`、`:6424`）。四個使用者逐一：

| 使用者 | 從哪裡讀 | 讀不讀本案整理的東西 | 本案怎麼處理 |
|---|---|---|---|
| **事件記錄分析（ELA）** | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp:289`（`Hub::RunOnce`）、`:646`（執行緒每圈 `RunOnce`＋`ScheduleTick`，golden 的定時報表／上傳）；輸入是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Interface\InterfaceSYS.cpp:554` `SendCommand_EventLog` 的掛勾與記錄檔；HTTP `/api/ela`（`EventLogAnalysis\ElaService.cpp:300`，`wb_serve.cpp:2868`） | 不讀 tag 表、不讀 JSON 快取 | 不碰；另外事件記錄環的 `log.*` tag（`JsonBridge\EventLog.cpp:173`，`wb_serve.cpp:2882`）照整理 |
| **SECS SV** | SECS 主機回答 SV 用 golden 自己的 SV 指標（`SecsTagPublish.h` 檔頭：「每一筆都直接指向存放值的全域」）；`secs.sv.*` tag 由 `SecsTagPublish.cpp:105` 整理 | 主機不讀 tag 表 | `secs.sv.*` **永遠照整理**（照 Jimmy 的條件，即使網頁今天沒人讀） |
| **REST API** | `wb_serve.cpp:2838-2870` `ApiRoute`（`/api/editlist`、`/api/form`、`/api/recipe`、`/api/system`、`/api/text`、`/api/struct`、Tester Comm、ELA）；其中 `/api/struct/io/{schema,runtime}`、`/api/struct/motor/{schema,runtime}` 讀主執行緒做好的快取（`:2553-2554` → `W906_ApiCacheGet` `:6434`，快取在 `:6399-6425` 重做）；`/api/struct` 目錄把 `prod`、`io`、`motor` 公開成「值在 tag 串流上」的通道（`:2617-2640`） | **讀 JSON 快取**；目錄公開的 tag 可能有網頁以外的人在收 | JSON 快取**照今天的頻率重做**（第 3 階段才看要不要改，§4.4）；`prod.*`、`io.*`、`motor.*` **照整理** |
| **TCP 7016／7017**（Handler 指令伺服器／結果伺服器） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Tcp\CmdServerPump.cpp:22-23`（`fMain->TCPCommandServer`／`TeraTCPResultServer`）、`:136-150`（`Pass`：golden `FormShow` 開伺服器、指令交給 golden 的處理）；主執行緒每圈 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp:151` `W906_CmdServerPumpTick`；埠號 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:13516-13517` | 不讀 tag 表；回答用 golden 的全域 | 不碰 |
| （另）**TCP 8046 tag 線**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\TcpTagLink.cpp`，BCB6 1203 監看客戶端用的） | 讀的就是同一張 tag 表；但**今天沒有任何建置的工具在發**：`tools\` 底下已經沒有 `wb_publish.cpp`／`wb_gateway.cpp`（`git ls-files` 0 筆），只剩 ctest `test_wb_tcplink` 與 `CMakeLists.txt:3276` 編進函式庫 | 會讀 | 閘門只由 `wb_serve` 安裝；日後若 `wb_serve` 接上它，那條線要算「有人要看」（照條件 c） |

---

## 3. 絕對不能停的（不管頁面開不開、瀏覽器開幾個）

| # | 什麼 | 為什麼不能停 | 檔案:行（移植樹／網頁） |
|---|---|---|---|
| 1 | 主畫面狀態：`machine.state`／`stateCode`／`stateSource`、`tower.*`、`clock.text`、`pump.*`、`mainsb.*` | 主畫面一直開著（`fMain` 是 locked 視窗） | `WebBridgeTags.cpp:1100`、`:1156`、`:1216`、`:1325`、`:3355`；`D:\HT9045\web\page\main.html` |
| 2 | 守衛訊號 `guard.systemStart`、`guard.softStop`、`guard.allMotorHome`、`guard.contactMode`、`guard.machine.*` | 外框用它們決定全螢幕互斥層與橫幅 | `WebBridgeTags.cpp:1246-1263`、`:1316-1320`；`background.html:1115-1116` |
| 3 | 告警／訊息框：信箱三個檔、WS 的 `alarm`／`modal`／`query`、新連線補發、`alarm.*` | 操作員必須看得到、按得掉 | `dialog-bridge.js:5`、`:771-777`；`D:\HT9045\web\page\ht9045_modal.js:52`、`:86`（單獨開頁時）；`wb_serve.cpp:440`、`:744`、`:757`、`:2895`；`WebBridgeServer.cpp:1157`、`:1727` |
| 4 | Exit 關站蓋層（Q44 停機狀態） | 關站中要看到還有什麼沒停 | `ht9045_main_close.js:1-37`、`:177`、`:210` |
| 5 | 登入／等級：`auth.level`、`user.level`、登入結果信箱 | 權限閘在網頁要顯示對的等級 | `WebBridgeTags.cpp:789`、`:797`；`dialog-bridge.js:523-537` |
| 6 | 心跳與操作員權杖：WS ping、`control.owner`、權杖續約 | 權杖與「連線消失就停」的死人開關靠它 | `WebBridgeServer.cpp:387-388`、`:1576-1582`；`WebBridgeTags.cpp:831`；`HW.MotorTest.html:1087`、`HW.teach.html:499`；`ht9045_link.js:289-296`（hub→iframe，瀏覽器內） |
| 7 | 頁面表自己：網頁→C++ `ui.windows.put`（含離開頁面的「全部關」）、C++→網頁 `ui.pages`（含 HOME ALL 自動開 Home Monitor 的 `want`） | fShow、S122、Motor Test／Teach 關窗停工作都看它；**它就是本案的依據** | `background.html:665-670`、`:1118`、`:1120`、`:1132`、`:1141-1150`；`WebBridgeTags.cpp:1263` |
| 8 | WebSocket 連線本身 | 「沒有畫面就不准 START、運轉中沒畫面 10 秒就 STOP」與「阻塞框等太久自動開瀏覽器」都看連線數 | `WebPageTable.cpp:377-381`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebModalWake.h:19-23`（只算 WebSocket） |
| 9 | 1203 讀卡、馬達死人開關、覆蓋掛鉤抄本、輸出優先服務 | 機台安全與 golden 流程；**本案只動「整理給網頁看」那一段，卡照讀** | `wb_serve.cpp:4645-4657`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1221-1247` |
| 10 | 事件記錄分析（條件 b） | golden 的定時報表／上傳不能跟著頁面停 | `EventLogAnalysis\ElaHub.cpp:289`、`:646`；`Interface\InterfaceSYS.cpp:554`；`log.*`：`JsonBridge\EventLog.cpp:173`、`wb_serve.cpp:2882` |
| 11 | SECS SV（條件 b） | 主機要的狀態 | `secs.sv.*`：`SecsTagPublish.cpp:105`、`wb_serve.cpp:2880` |
| 12 | REST API（條件 b） | 網頁以外的程式也會來拿 | `wb_serve.cpp:2838-2870`；JSON 快取 `:6399-6441`；REST 目錄公開的 `prod.*`／`io.*`／`motor.*`（`:2617-2640`、`:2892-2894`） |
| 13 | TCP 7016／7017（條件 b） | 測試機／主機的指令與結果 | `TesterComm\Tcp\CmdServerPump.cpp:22-23`、`:136-150`；`TesterComm\Handler\TesterCommWiring.cpp:151` |
| 14 | TCP 8046 tag 線（今天沒人發，§2.7） | 若日後接上，讀的就是 tag 表 | `WebBridge\TcpTagLink.cpp` |

⇒ 本案**只**閘 `pci1203.*`（三個卡片層除外）與 `motionView.trays.*`，並讓兩個頁面關窗不拿；上表 14 項一行都不碰。

---

## 4. 提案（三個階段）

### 4.0 第 1 階段：先量（條件 d）—— 不改任何行為

加計數，每 10 秒在 wb_serve 主控台印一行（有設 `W906_OPLOG_DIR` 時也寫進操作紀錄），**本案現在不跑 wb_serve**；量要等 Steven 同意在 SIM 沙盒跑（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md`），再上機台量一次。改之前量一次、第 2 階段改完同樣條件再量一次，兩行貼進本檔 §7。

範例（格式，不是量到的數字）：
```
[STREAM] 10.0s publishes=62 staged avg=8088 max=8090 pci1203=5889 (73%) publishMs avg=… max=…
         apiCache builds=70 ioBytes=… motorBytes=… buildMs avg=… max=…
         ws conns=1 snapshots=0 patches=58 patchBytes=… snapshotBytes=…
         http ioRuntime=50/… motorRuntime=10/… mailbox=300/… prodUpdate=40/…
```

| 計數 | 放在哪（同一行附加，行號不動） | 檔的主人 |
|---|---|---|
| 整理次數、每次整理幾個（`PublishHandlerTags` 的回傳值）、每次幾微秒 | `wb_serve.cpp:5945`（主迴圈那一次整理的前後；`:4265`、`:4422` 另計） | 筆電（Jimmy） |
| 各家族幾個（`pci1203.`、`secs.sv.`、其他）：每 10 秒一次 `snap.read()` 依前綴數（不動 `WebBridgeTags.cpp`） | `wb_serve.cpp` 印那一行的地方（主迴圈尾端同一行） | 筆電 |
| JSON 快取：重做幾次、每份幾位元組、幾微秒 | `wb_serve.cpp:6399-6425`（`W906_ApiCacheRefresh`） | 筆電（機台 IOWEB-P6／P25 寫的） |
| WS：snapshot／patch 則數（已有 `WebBridgeServer.h:146-147`）＋**新加**送出位元組 | `WebBridgeServer.cpp` `PumpSnapshot`（`:1529-1532`）與送 snapshot 那一處（`:1148-1149`）；統計結構 `WebBridgeServer.h` | 筆電 |
| HTTP：io／motor runtime、三個信箱、`Production-update.json` 的次數與位元組 | `wb_serve.cpp:2553-2554`、`:2842`（`/JSON/*` 經 `W906_JsonScrubRoute` 那一行） | 筆電 |
| 累計與印出本身（純函式，可以 ctest） | 新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStreamStats.cpp`／`.h`（St01） | St01 |
| 瀏覽器那一端：每個分頁收了幾則、幾位元組（`HT9045Link` 的 stats 已有則數，加位元組） | `D:\HT9045\web\page\ht9045_link.js:155`、`:469` | St01（ST01-E3） |

### 4.1 原則

1. **只是傳輸的省略，不是邏輯**：閘門只「讀」頁面表，不寫它；golden 的 fShow 答案（`WebPageTable.cpp` 規則 1～7）不變；卡照讀、golden 流程照跑。
2. **條件 (c) 不確定就照送**（跟 fShow 相反）：fShow 在「不知道」時 Steven 裁成「當關」（Q-P1）；傳輸這邊多送無害、少送才會讓畫面停在舊值。所以：
   - 只有「**確定只有某一頁在用**」的家族才閘（今天只有 `pci1203.*`、`motionView.trays.*`）；
   - 查詢本身「沒安裝（ctest）」「從沒收過總表」「不認得的視窗」「回報過期但還連著」一律回「要」；只有**每一條還連著的連線**都明確說那個視窗關著（closed／never／absent）時才回「不要」；一條 WebSocket 都沒有時也回「不要」（沒有收件人）。
3. **看狀態，不看邊緣**：每一次整理都當場問頁面表，不靠「開→關」事件 ⇒ 不會漏邊緣。
4. **縮小＝照送（照 BCB）**：golden 縮小不跑 FormClose，`fShow` 還是 true，表單的 `TTimer` 照跑（外框註解 `background.html:676`；VCL 行為是推論，沒在 golden 機台上量）；頁面表本來就把縮小算開（`WebPageTable.cpp:215`、`:231`），外框送給頁面的 `HT_WIN` 也是 `open:true`（`background.html:682`）。

### 4.2 第 2 階段 A：頁面表多一個「這一頁有人要看嗎」（前置）

- **St01**：`WebPageTable.cpp`／`.h` 檔尾加 `bool W906_PageStreamWanted(const char* webId)`，用 **WINDOWS 表的 id** 查（例 `"pci1203"`、`"motionview"`），照 4.1 第 2 條回答。
- **要動 Jimmy 的一行**：`pci1203`、`motionview` 這幾個視窗在 golden 沒有表單名（`form:null`），外框**有送**它們的狀態（`background.html:829-853` 每個視窗都列、form 送 null），但視窗總表收到就丟掉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:141` `if (!cJSON_IsString(jform)) continue;`），頁面表的網頁規則對它們永遠回 false（`WebPageTable.cpp:225-234` 的註解就寫了）。⇒ 在 `:141` 同一行、`continue` 之前多記一份「以視窗 id 為鍵」的狀態，檔尾加一支 `WebWindowRegistryQueryId(id)`。golden 表單名那一份（fShow 的 join key，契約 §4 事實 5）**完全不動**。Jimmy 0928 對同一個檔的答覆是「可以，請在 §1 先認領確切行號」（`TO_STEVEN.md` main 版第 216 行）。
- 例：Steven 開著 1203 設定頁、遠端電腦的瀏覽器沒開 ⇒ 聯集＝有人要看 ⇒ 照整理。

### 4.3 第 2 階段 B：1203 那一大塊在「整理」那一步就跳過（條件 a）

- `WebBridgeTags.cpp:1360-2314` 整段在入口問 `W906_PageStreamWanted("pci1203")`，回「不要」就整段不跑（5,886 個 tag 不組名字、不放進表）；`pci1203.linked`／`enabled`／`phase`（`:1395-1402`）三個卡片層移到入口之前、照舊永遠發。用函式指標掛（沒安裝＝照整理），理由同 `SetExtraTagPublisher`：這個檔被 `tests\test_wb_tags.cpp` 直接編，精確筆數的斷言不能被打壞（`WebBridgeTags.cpp:2318-2328`）。
  - ⚠ 這一段是**機台端（EastSun）常設不碰**（`TO_STEVEN.md` main 版第 61 行「`WebBridgeTags.cpp` 的 tag」）⇒ 照 Jimmy 的預設 St01 寫、筆電審，並在 GitHub 更新包知會 EastSun。
- 關窗時：下一次整理那 5,886 個 tag 不見了 ⇒ 伺服器送一次約 170 KB 的「全部 null」patch（`WebBridgeServer.cpp:1526-1527`，null＝不知道，畫面顯示 "---"）。開窗時：外框送 `ui.windows.put` ⇒ 主迴圈同一圈 drain 到它、同一圈就整理並發布（有指令就發布，`wb_serve.cpp:5938`）⇒ 約 180 KB 的值一次補齊。lazy 的 1203 頁本身載入要幾百毫秒以上，多半在它的腳本跑起來之前 hub 快取就已經是新值。
- `io.*`、`motor.*` **不閘**（條件 c：REST 目錄公開，`wb_serve.cpp:2617-2640`）。

### 4.4 第 2 階段 C：Motion View 托盤（St02）

St02 已回覆會自己改約 5 行（`D:\HT9045\docs\handoff\FROM_STEVEN.md` v906/steven-handoff 版 §4 第 753 行，20260930 16:02）：頁面關著時 `W906_StageMotionViewTrays` 開頭就回，等 A 的查詢名稱 ⇒ `W906_PageStreamWanted("motionview")`。

### 4.5 第 2 階段 E：IO 頁與 Teach 頁照 golden，視窗關著不拿

- golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\iosetview.cpp:162`（`Tfiosetview::Timer1Timer`）與 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:1366`（`TfTeach::Timer1Timer`）第一件事都是 `if(fShow==false) return;`。Motor Test 已經照這樣做（`HW.MotorTest.html:848-851`，引 golden `uMotorTest.cpp:914`），Main.MotorView、Motion View 也是。
- 改法照 Motor Test：聽 `HT_WIN`、`pollAllowed()＝!winHosted || winShown`、開窗那一下立刻拿一次（`Main.MotorView.html:271-276` 的寫法）。從沒收過 `HT_WIN`（單獨開頁、舊外框）照舊一直拿。
  - `HW.IoSetView.html:435-452`（機台 EastSun IOWEB／Jimmy 的頁）。
  - `HW.teach.html:399-406`（Jimmy W5 的頁；St01 0929 動過 `:494`）。Teach 的計時器裡還有「HOME 做完把按鈕彈起」（`teachSyncHome`），golden `Timer1Timer` 關窗時也不跑，照 golden。
- 這一條不影響 REST API（別的程式照樣可以拿）。**省下最多位元組的是這一條**（每個分頁約 0.58～0.87 MB／秒、6 則／秒）。

### 4.6 第 3 階段：量完、Jimmy 同意才考慮（現在不做）

| 代號 | 內容 | 為什麼現在不做 |
|---|---|---|
| D | JSON 快取改成「有人開、或 2 秒內有人拿才全速重做，否則 1 秒一次」 | REST API 在不能停清單（條件 b）；先看第 1 階段量到它佔主執行緒多少，再問 Jimmy |
| B' | `io.*`、`motor.*` 也閘 | REST 目錄公開，不確定網頁以外有沒有人收（條件 c） |
| F | hub 只把 patch 轉給用得到的 iframe（`ht9045_link.js`，St01） | 省的是瀏覽器 CPU，不是 C++ |
| G | 伺服器每條連線只送它那個分頁開著的頁面（`WebBridgeServer.cpp`，Jimmy） | 閘在整理那一步已經拿到大部分；多分頁才有差，而且會打散 WSFANOUT 的共用 |

### 4.7 各種情況怎麼走

| 情況 | 會發生什麼 |
|---|---|
| 關→開 | 外框立刻送 `ui.windows.put`（`background.html:665-670`，0929 起不再等 60 ms）；C++ 同一圈 drain、同一圈整理並發布 ⇒ tag 在 ≤50 ms 內補齊。頁面自己在 `HT_WIN` 開的時候也立刻拿一次（已經有的 H4 做法：C 路頁開窗才向 C++ 要資料，`D:\HT9045\web\page\ht9045_wire_engine.js:2046-2066`、`:2129-2135`）。JSON 快取本來就照今天的頻率在做，第一次拿到的最多是 200～500 ms 前的。 |
| 開→關 | 下一次整理就跳過；tag 送一次 null。 |
| F5 | 舊頁離開前送「全部關」（`background.html:1141-1150`）⇒ 閘關；新頁連上拿到的 snapshot 沒有 `pci1203.*`；F5 之後設定頁本來就是關的（WINDOWS 表 `hidden:true`），打開哪一頁就補哪一頁。 |
| 斷線重連（沒有 F5） | hub 重連、伺服器送新的 snapshot；外框每秒檢查、重連就重送總表（`background.html:1120-1124`）；舊連線的回報過期但還有連線 ⇒ 照最後一次（照送），不會空窗。 |
| 瀏覽器全關（R122 的 10 秒寬限） | 沒有連線 ⇒ 閘回「不要」（沒有收件人）；10 秒寬限與「沒畫面就 STOP」照舊由頁面表判斷（`WebPageTable.cpp` 檔頭），本案不碰。 |
| 兩個分頁／兩台電腦 | 聯集：任何一個開著就整理；每條連線都收得到（G 才會分開）。每個分頁一條連線（hub），兩個分頁＝C++ 送兩份。 |
| ctest | 沒有安裝閘門 ⇒ 跟今天一模一樣。 |

### 4.8 hub 有沒有改變什麼

- C++ 那邊「每個分頁一條連線」讓閘門的效果直接等於「每個分頁少送多少」；沒有 hub 時（0926，23 條連線）同樣的閘門可以省 23 倍。
- hub 的快取會留著 null（`ht9045_link.js:31-32`、`:459-467`），晚載入的頁面拿到的是 "---"，接著是新值，不會拿到過時的真值。
- hub 不送 `ui.windows.put`（`ht9045_link.js:55`），外框仍是唯一的回報者 ⇒ 頁面表的依據不變。
- hub 把每個 patch 轉給分頁裡每個 iframe（含藏起來的）：閘門之後那些 patch 本身就小了，F 可以之後再說。

---

## 5. 跟 C++ 原生畫面怎麼搭（Steven 20260930 追加）

背景：六頁 Main.MotorView、HW.IoSetView、HW.MotorTest、HW.teach、HW.home、HW.ShuttleMove 有原生 Win32 視窗版（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\`，ST01-E3／St02／筆電合過），建置時開關 `W906_NATIVE_FORMS`（預設 OFF；Q54：只有這六頁、只有建置時開關，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\README.md` §10）；skill `D:\HT9045\.claude\skills\ht9045-cpp-generated-pages\`。

### 5.1 (a) 原生視窗今天怎麼記開關、有沒有叫頁面表

- **沒有**：`git grep` `W906_Page`／`PageTable` 在 `ui\native\` 只有 README 一句「頁面表沒有登記原生視窗；網頁那頁與原生視窗互不知道對方開著（唯讀所以今晚沒關係，1b 輸出前一定要做）」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\README.md:226`）。
- 原生視窗自己知道自己開著：只在開著時更新畫面（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeFormsWbServe.cpp:823-858`，`IoViewIsOpen()`、`MotorViewIsOpen()`…）；資料**直接讀記憶體**（`HSys.IOTable`、1203 監看器樣本、`MOT[]`、EastSun 的覆蓋掛鉤），不經 JSON、不經 tag（同檔 `:1-40`）；ON 的建置開機時六個全部自己打開（`:889-923`），關掉可以用 Ctrl+Alt+I／M／T／S／H／K 重開（`:873-878`）。
- **ST01-M 建議的「走 `W906_PageProgramSet`、像 C++ 自己開的對話框那樣」要更正**，三個原因：
  1. 這六個表單在頁面表都是網頁列：`fiosetview`、`fTeach`、`fMotorTest`、`fShuttleMove` 是 `kPgWeb`，程式寫入的狀態**不會被採用**（`WebPageTable.cpp:247-257` 的 `kPgWeb` 分支只看網頁），寫了等於沒寫；`fHome` 是 `kPgBoth`，寫入會設 `want=open`，**每個瀏覽器都會跳出網頁的 Home Monitor**（`:341-356`）；Motor View 沒有 golden 表單名，查不到列，只會印一行 unknown form（`:236-245`）。
  2. 就算改成會採用，原生視窗一旦算「fShow＝開」，golden 邏輯就會變：Teach／Motor Test 開著 ⇒ 主流程暫停（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30493-30494`）、START 診斷閘、S122 清回原點旗標（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`）。而 ON 的建置**開機就把六個都打開** ⇒ 機台一開就被當成「Teach 開著」。原生 v1 只是顯示，不是 golden 的那張表單，這樣算是錯的。
  3. **本案根本不需要原生視窗登記**：原生視窗不吃任何 tag 或 JSON，閘門只要看「網頁那一頁有沒有人開」。
  ⇒ 原生視窗要不要算進 fShow 是 README 說的「1b（原生可以按輸出）之前」的另一題，到時候在頁面表加一個**獨立的「原生」欄**（不是 `kPgProgram`），跟串流閘門、fShow 分開決定（§11）。

### 5.2 (b) 開關 ON：原生開著、網頁那頁沒開

閘門只看網頁視窗 ⇒ 1203 設定頁沒開就不整理 `pci1203.*`；網頁的 IO 頁、Teach 頁關著就不拿 runtime JSON（第 2 階段 E）；原生視窗照樣每 20 ms 比一次（實際約 50 ms，README §8 第 9 條）直接讀記憶體，不受影響（覆蓋掛鉤抄本在 `WebMotorAccessLive.cpp:1221-1247`，每拍與每次讀卡都做，不在閘門裡）。JSON 快取照今天的頻率在做（REST API，條件 b），只是沒有網頁來拿。**§3 的不能停清單不變。**

### 5.3 (c) 開關 OFF（今天出貨的建置）

原生程式一行都不進 exe（`ui\native\README.md` §11.2：五個接縫是空函式），本案就是單純的「只替開著的網頁送」。§7 的數字就是 OFF 的數字。

### 5.4 (d) 關原生→開網頁，以及反過來

- 關原生、開網頁：原生關掉對 C++ 串流沒有任何影響；網頁開 ⇒ §4.7「關→開」那一列（tag ≤50 ms；網頁頁面開窗立刻拿一次）。
- 關網頁、開原生：網頁關 ⇒ 下一次整理跳過、頁面不再拿；原生重開時自己馬上用記憶體重畫一次（`NativeFormsWbServe.cpp:873-878` `RefreshIfDue(true)`），不依賴網頁的任何資料。
- 兩邊同時開：網頁那一頁照今天的頻率收；原生照自己的頻率讀。

### 5.5 (e) 省多少、要動誰的檔

| 建置 | 情況 | 通訊（每個分頁，推算） | 主執行緒 |
|---|---|---|---|
| OFF | 主畫面＋一兩個設定頁 | §7 情況一：約 0.6～0.93 MB／秒 → 約 22～36 KB／秒 | tag 整理少 73～83% |
| ON | 原生 IO＋原生 MotorView 開著、網頁那兩頁關著 | 同上（網頁的 runtime JSON 沒人拿） | 同上；原生自己畫面的成本另計、本案不改（README §3.2 展示量到 IO 967 點每次 1.43 ms、MotorView 48 軸每次 2.82 ms） |
| ON | 原生與網頁同一頁都開著 | 網頁那一頁照今天 | 網頁那份照今天 |

要動的檔：**本案不動 `ui\native\` 任何一個檔**，也不動 `wb_serve.cpp` 的原生接縫（筆電的檔、St01 同一行插入的 `:4536`、`:4538`、`:4575`、`:5967`、`:7622`）。以後若要讓原生視窗算進頁面表（§11），才會動：St01 `WebPageTable.cpp`（新欄）＋`NativeFormsWbServe.cpp`（St01 的 IoView／MotorView 開關兩處；St02 的 MotorTest／Home／Teach 各一處）＋St02 `ui\native\NativeShuttleMoveGlue.cpp`，屆時各自認領。

---

## 6. 誰改哪個檔、要誰同意

**照 Jimmy 的預設：程式一律 St01 寫**（St02 的托盤除外），動筆電的檔先在 `FROM_STEVEN.md` §1 認領確切行號；**筆電審認領＋跑兩組態 gate 才合**；動到機台端的檔另外在 GitHub 更新包知會 EastSun。依據：`git -C D:\HT9045 log b45d4482 --format='%an %s' -5 -- <檔>`；`git show origin/main:docs/handoff/TO_STEVEN.md`（main `1b70caba`）§1；`git show origin/v906/steven-handoff:docs/handoff/FROM_STEVEN.md`（`2d208929`）§1、§4。

| 階段／部分 | 檔 | 原主人（最近 commit） | 誰寫／誰審 | 另外要 |
|---|---|---|---|---|
| 1 量 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5945`、`:6399-6425`、`:2553-2554`、`:2842`、主迴圈尾端印一行 | 筆電、機台（IOWEB）、St01（各處同一行插入） | St01 寫／筆電審 | 認領確切行號 |
| 1 量 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1148-1149`、`:1529-1532`；`WebBridge\WebBridgeServer.h` 統計結構 | jimmychiu、機台（WSFANOUT） | St01 寫／筆電審 | 認領 |
| 1 量 | 新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStreamStats.cpp`／`.h`＋ctest | St01 | St01／筆電審 | — |
| 1 量 | `D:\HT9045\web\page\ht9045_link.js:155`、`:469` | St01（ST01-E3） | St01 | — |
| 2A 查詢 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`／`WebPageTable.h` 檔尾；`tests\test_pagetable.cpp`、`tests\CMakeLists.txt` 檔尾 | St01 | St01／筆電審 | — |
| 2A 總表 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:141` 同一行＋檔尾一支；`WebWindowRegistry.h` 檔尾宣告 | jimmychiu（0920／0921）、機台 MT-FIX1、St01 S-10 | St01 寫／筆電審 | Jimmy 看契約 §4 事實 5 這一格（fShow 那份不動） |
| 2B 整理跳過 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:1360-2314`（入口一行、三個卡片層移到入口前）＋安裝座一行 | 機台端常設不碰（`TO_STEVEN.md` §1 第 61 行）；最近 jimmychiu、Steven | St01 寫／筆電審 | **知會 EastSun** |
| 2B 安裝 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389`（頁面表安裝的共用行，同一行附加） | 共用行 | St01／筆電審 | — |
| 2C 托盤 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.cpp` | St02 | St02 自己寫／筆電審 | St02 已答應 |
| 2E IO 頁 | `D:\HT9045\web\page\HW.IoSetView.html:435-452` | 機台（EastSun IOWEB）、jimmychiu | St01 寫／筆電審 | **知會 EastSun** |
| 2E Teach 頁 | `D:\HT9045\web\page\HW.teach.html:399-406` | jimmychiu（W5）、St01 `:494` | St01 寫／筆電審 | — |
| 不動 | `D:\HT9045\web\background.html`（已經送 form:null 視窗的狀態）、`D:\HT9045\web\page\ht9045_recipe_client.js`、`D:\HT9045\web\page\ht9045_wire_engine.js`（筆電 `TO_STEVEN.md` §1 第 26 行第 13 條還在認領中）、`ui\native\*`、`JsonBridge\ChanIo.cpp`、`JsonBridge\ChanMotor.cpp`、`SecsTagPublish.cpp`、`JsonBridge\EventLog.cpp`、ELA、TesterComm | — | — | — |
| 提案紀錄 | FROM_STEVEN v906/steven-handoff 版 §1 第 456 行（20260930 15:46，ST01-M，「No code until you agree」）；Jimmy 要求文件寫好貼在 FROM_STEVEN §3 | St01 | ST01-E 推上本檔後把 hash 補在那兩處 | — |

---

## 7. 預期省多少（每個瀏覽器分頁，有 1203 卡的機台，全部是推算；第 1 階段量到後改寫本節）

### 情況一：生產中，主畫面＋一兩個設定頁（例：Contact、Set Up），IO／Teach／Motor／1203 頁都關著

| 項目 | 今天 | 第 2 階段做完 | 差 |
|---|---|---|---|
| WS patch | 5～7 則／秒，約 3～35 KB／秒 | 2～7 則／秒，約 1～10 KB／秒（`io.*`、`motor.axes`、主畫面群照送） | 約 −2～25 KB／秒 |
| io runtime（IO 頁關著照拿） | 5 則／秒，約 0.53～0.82 MB／秒 | 0 | −0.53～0.82 MB／秒 |
| motor runtime（Teach 關著照拿） | 1 則／秒，約 45～50 KB／秒 | 0 | −45～50 KB／秒 |
| 告警信箱（不能停） | 30 則／秒，約 20～25 KB／秒 | 同 | 0 |
| 生產資料輪詢（乾淨機台 404） | 4 則／秒，約 0.4 KB／秒 | 同 | 0 |
| 總表心跳（上行） | 0.2 則／秒，約 0.8 KB／秒 | 同 | 0 |
| **合計** | **約 45～47 則／秒、0.6～0.93 MB／秒** | **約 36～41 則／秒、22～36 KB／秒** | **位元組約 −95%；則數約 −15%** |
| 主執行緒：tag 整理 | 每次約 7,100～8,100 個 × 5～7 次／秒 | 每次約 1,200～2,200 個 | −73～83%（5,886／8,088＝73%；0924 的 7,082 時是 83%） |
| 主執行緒：JSON 快取 | 約 7 次／秒 × 約 16～21 萬位元組 | 同（REST API，條件 b） | 0（第 3 階段 D 才看） |
| 伺服器：每個世代複製比對整張表 | 約 7,100～8,100 筆 | 約 1,200～2,200 筆 | −73～83% |

### 情況二：保養中，IO 頁＋Motor Test 開著（1203 頁、Teach 關著）

io runtime 與 Motor Test 的 motor runtime 照今天（那是要看的）；省 Teach 那份（約 45～50 KB／秒）＋ `pci1203.*` 那一塊（約 2～25 KB／秒）＋主執行緒 tag 整理 73～83%。位元組約 −7～9%。

### 情況三：開關 ON，用原生 IO／MotorView 代替網頁

同情況一（網頁那幾頁關著）。

⚠ 則數省得不多：每秒 30 則的告警信箱輪詢是「不能停」那一類，而且它是頁面在拉、不是 C++ 在推；要省它得改成「WS 通知才去拿」（`ht9045_modal.js` 檔頭說 WS 的 `modal` 訊框目前只當「馬上去讀」的觸發），那是告警通道的另一題，不在本案（§11）。

---

## 8. 風險

| # | 風險 | 會怎樣 | 怎麼擋 |
|---|---|---|---|
| 1 | 剛打開 1203 頁看到 "---" | 最多到下一次整理（≤50 ms，指令那一圈就整理） | 同一圈補齊（§4.3）；§9 T6 量 |
| 2 | 剛打開 IO 頁按輸出鈕 | JSON 快取沒有改（照今天的頻率在做），所以跟今天一樣；IO 頁多了「開窗立刻拿一次」 | 無新增風險；§9 T7 上機仍看一次 |
| 3 | 漏掉邊緣 | — | 閘門看狀態不看邊緣（§4.1 第 3 條）；`ui.windows.put` 每 5 秒心跳重送 |
| 4 | 安全值不更新 | — | §3 十四項一行不動；閘門只讀頁面表；卡照讀；C++ 沒有邏輯讀 tag 表 |
| 5 | 有別人在讀 `pci1203.*` 而本案的普查沒找到 | 那個讀者在 1203 頁關著時看到 null | 條件 (c)；§9 T5 加「誰讀 `pci1203.`／`motionView.trays`」的普查棘輪，新增的讀者一出現就紅；TCP 8046 若接回來，閘門要把它算成有人要看（§2.7） |
| 6 | 被動量測工具（`channel_census.py`、`snap_dump.py`）看到的 tag 變少 | 量測數字跟以前不同 | 第 1 階段先在改之前量；普查報告註明當下 1203 頁開不開 |
| 7 | 契約改動 | 總表多記 form:null 視窗 | 另一張表、另一支查詢；fShow 那一份不動；筆電審 |
| 8 | 關 1203 頁送一次約 170 KB 的 null、開一次送約 180 KB | 一次性的尖峰，等於一次 snapshot | 可接受；T0／T6 量 |
| 9 | 多分頁省不到別的分頁 | 一個分頁開 1203 頁，其他分頁也收 | G 才能分開；機台通常一個分頁 |
| 10 | 1203 頁的 DO 按鈕在補齊前是 null | 按鈕狀態不明的那一瞬間 | lazy 頁載入本身就比補齊慢；**沒查證** `D:\HT9045\web\js\pci1203.js` 對 null 的按鈕怎麼處理，T6 要看 |
| 11 | 計數本身吃 CPU | 每 10 秒一次 `snap.read()`（複製約 8,000 筆） | 只有每 10 秒一次；T0 自己量得到 |

---

## 9. 怎麼測

| 代號 | 內容 | 在哪跑 | 需要誰 |
|---|---|---|---|
| T0 | **第 1 階段量測**：`WebStreamStats` 的累計與印出（ctest 純函式）；上線後 SIM 沙盒跑「開站閒置 60 秒／開 1203 頁 60 秒／開 IO 頁 60 秒／開 Teach 60 秒／兩個分頁」各一段，記 `[STREAM]` 行；同一份劇本在第 2 階段後再跑一次；再上機台跑一次 | 本機 ctest＋Steven01 沙盒（**要 Steven 同意**）＋機台 | Steven 同意；機台由 Jimmy／EastSun 或 St02 |
| T1 | `test_pagetable` 加 `W906_PageStreamWanted`：沒安裝＝要、從沒收過總表＝要、每條連線都說關＝不要、兩個分頁一開一關＝要、過期但還連著＝照最後、沒有連線＝不要、不認得的 id＝要＋印一行、縮小＝要、form:null 的 `pci1203` 查得到 | 本機 ctest（秒級，只連頁面表＋總表＋cJSON） | St01 |
| T2 | 總表：form:null 視窗以 id 記得住；fShow 的 golden 表單名查詢結果跟今天逐一相同（`test_winregistry` 不動，另在 T1 用公開函式驗） | 本機 ctest | St01 |
| T3 | 假的 tag 收件端：`test_wb_tags` 加一格——裝一個對 `"pci1203"` 回「不要」的閘 ⇒ 整理的筆數正好少 5,886、三個卡片層還在；沒裝 ⇒ 筆數跟今天一樣（現有斷言不動）；再用兩個世代的 `TagSnapshot` 驗「關＝那些 tag 送 null、開＝全部值回來」 | 本機 ctest | St01 |
| T5 | 消費者普查棘輪：掃 `D:\HT9045\web\` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\` 找讀 `pci1203.`、`motionView.trays` 的檔，必須都在允許清單（1203 頁、Motion View、被動量測工具）裡 | 本機 ctest（Python，照 `tools\fshow_audit.py` 的樣子） | St01 |
| T6 | SIM wb_serve＋瀏覽器：開關 1203 頁／IO 頁／Teach／Motor Test、F5、兩個分頁、全部關掉看 10 秒寬限照舊、開操作紀錄；對照 T0 的 `[STREAM]` 行與瀏覽器 DevTools Network | Steven01 沙盒（**要 Steven 同意**） | Steven 同意 |
| T7 | 上機（HT9050，有卡）：同 T6，另看 IO 燈號在開窗後多久是新的、開窗後第一下按輸出、1203 頁開關、點擊延遲紀錄「發布」欄前後、原生 ON 的建置開原生不開網頁 | 機台 | Jimmy／EastSun 或 St02 |

每一步照慣例在全新 build 目錄跑兩組態 gate，比對失敗清單；合併前由筆電再跑一次兩組態 gate。

---

## 10. 給 Jimmy 逐段看（Steven：Jimmy 逐段看過才寫程式）

| 段落 | 要 Jimmy 看的 | 回答 |
|---|---|---|
| §0.1 | 四個條件的對照有沒有照原意 | 對／不對 |
| §2.6（條件 a） | 整理迴圈與 1203 區塊的行號、閘門放在 `:1360` 入口 | 同意／不同意 |
| §2.7、§3 第 10～14 項（條件 b） | 四個使用者讀什麼、`secs.sv.*`／`log.*`／JSON 快取／`prod.*`・`io.*`・`motor.*` 照整理 | 同意／漏了誰 |
| §4.1（條件 c） | 只閘 `pci1203.*` 與 `motionView.trays.*`；查詢「不知道就要」 | 同意／不同意 |
| §4.0（條件 d） | 計數放的位置、`[STREAM]` 那一行的欄位；St01 寫、筆電審 | 同意／要改哪幾欄 |
| §4.2 A | `WebWindowRegistry.cpp:141` 同一行多記 form:null 視窗、檔尾加以 id 查的函式 | 同意／不同意 |
| §4.3 B | `WebBridgeTags.cpp:1360-2314` 入口跳過、三個卡片層照發；知會 EastSun 由筆電在更新包寫 | 同意／不同意 |
| §4.4 C | St02 自己改托盤 | 知道 |
| §4.5 E | `HW.IoSetView.html`、`HW.teach.html` 照 golden 關窗不拿；知會 EastSun | 同意／不同意 |
| §4.6 | D、B'、F、G 第 3 階段才看 | 知道 |
| §5 | 原生不需要登記、ST01-M 那個建議的更正 | 同意／不同意 |
| §10 I | Steven01 這台 `D:\HT9045\web\JSON\Production-update.json` 是 8.6 MB 的舊檔、外框每 250 ms 拿一次（§2.5）；機台上有沒有這個檔請順便看一下 | 知道 |

建議順序：第 1 階段量（先做、不改行為）→ 第 2 階段 A → B／C／E（E 跟其他無關，隨時可以做，位元組省最多）→ 再量一次 → 第 3 階段看數字再說。

---

## 11. 之後要決定（不是這一案）

- **原生視窗要不要算進 fShow**（`ui\native\README.md:226`「1b 輸出前一定要做」）：算進去，golden 的「Teach／Motor Test 開著就暫停主流程」等會對原生視窗生效；而 ON 的建置開機就把六個都打開。這是 Steven／Jimmy 在做原生 1b 時要決定的，跟串流閘門分開（§5.1）。
- **告警信箱改成 WS 通知才拿**（§7 最後）：可以把每秒 30 則降到接近 0，但那是告警通道（Steven 0922 的對話框橋接契約）的設計，要另外評估。
- **第 3 階段 D／B'**：看第 1 階段量到的 JSON 快取與 `io.*`／`motor.*` 成本，再問 Jimmy。

---

## 12. 沒查證／推論的地方

- 有卡的機台上每個 patch 幾個 tag、幾位元組：**沒量**；3～35 KB／秒是依 tag 名形狀與讀卡欄位推的（第 1 階段量）。
- snapshot 200～230 KB、`pci1203.*` 170 KB：腳本依 `WebBridgeTags.cpp` 的名字形狀推的（每個軸約 110 個 tag、平均尾碼 10.5 字元），不是從線上抓的。
- io runtime 每點 134～170 B、motor runtime 每軸約 1 KB：用 `D:\HT9045\web\JSON\` 的舊樣本換算，C++ 實際送的沒抓。
- 整理一次、JSON 快取一次各幾毫秒：沒量（第 1 階段量）；43.6 ms 是 0925 點擊紀錄「其他」的最大值，不是整理本身的量測。
- `display:none` 的 iframe 裡 `document.hidden` 仍是 false：照 HTML 規格（iframe 跟著最上層分頁）以及 `ht9045_contactct_wire.js:219-231`、`ht9045_trayedit.js:34` 另外用 `frameElement` 判斷的做法推論，**沒在機台 Edge 上實測**。
- golden 縮小時 `TTimer` 照跑：VCL 一般行為，沒在 golden 機台上量。
- REST 目錄公開的 `io.*`／`motor.*`／`prod.*` 今天有沒有網頁以外的人在收：**沒查到有**，但照條件 (c) 當成有。
- `pci1203.js` 對 null 的 DO 按鈕怎麼畫：沒查。
- Smart Diagnostic 的 1 秒計時器在關窗後何時停：沒查。
- 機台上有沒有 `Production-update.json` 的舊檔：沒查。
- `secs.sv.*` 生產中每秒變幾個：沒量。

### 12.1 補記：第 2 階段 A 唯讀查證（ST01-E 派的工程師，20260930 21:3x，沒有改任何檔；給筆電實作參考）

- **「連著」只有數目**：頁面表的 `g_host.liveWs` 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 裝成 `g_modalServer->LiveWebSocketCount()`（`WebBridge\WebBridgeServer.h:247`，升級 +1、CloseConn −1，死連線最多留 45 秒）；用在 `WebPageTable.cpp:218-222`（`WsLive()`）與 `:377-381`（`PageScreenPresent`）。伺服器沒有「哪一條還活著」的 API（`WebWindowRegistry.h:38-48` 自己也寫了）⇒ §4.1 第 2 條「每一條還連著的連線都說關」只能近似；要精確得在 WebBridgeServer 加每條連線的存活查詢。
- **「過期」**：總表每條連線的 `recvMs`（`time(0)*1000`，1 秒解析度，`WebWindowRegistry.cpp:59-62`、`:96`）對 `WebWindowRegistryStaleMs()` 15 秒（`:16`）；新的回報蓋過過期的（MT-FIX1，`:179-222`）。總表的 map 只增不減（`:50-57`），重連後死掉的那筆可能蓋住一條從不回報的活連線。
- **鎖**：`WebWindowRegistryPut` 全部在主迴圈呼叫（`wb_serve.cpp:638`、`:847`、`:5828`、`:6791`），總表沒有鎖、`WebPageTable.h:32-33` 寫明「主執行緒、不加鎖」⇒ 以 id 記的第二份只要讀者也在主執行緒（`PublishHandlerTags`、經 `PublishExtraTags` 的 `ChanMvTrays`）就不用鎖。
- **陷阱：沒有視窗的總表**：`windows:{}` 或全部無效的一份，照樣整份取代那條連線的 golden 表（`Store()[connId]=f`，`:161`），但走不到 `:141` ⇒ 只在 `:141` 記的 id 表會留著上一份的狀態。要在一份總表的第一個有效視窗時重新起算，查詢時對 `Store()[conn]` 的 seq／at 比對，對不上就當那條連線沒提到（＝要）。`WebWindowRegistryResetForTest`（`:464-470`）不在允許的行內 ⇒ 測試的重設另想辦法。
- **外框送法**：`background.html` 每一份都列出 WINDOWS 表全部的列（含 absent，`:829-853`），`seq` 每份 +1（`:864`、`:1146`），離開頁面那一份全部標關（`:1141-1150`）。
- **§2.1／§8 風險 5 漏掉的讀者**：`D:\HT9045\web\pci1203.html` 可以單獨開，沒有 hub 時自己開 WebSocket（`D:\HT9045\web\js\transport\ws.js:59`），而且**不送** `ui.windows.put` ⇒ 閘門看不到它；照條件 (c) 要把「有沒有這種讀者」算進去。其他直接開 WebSocket 的只有 `background.html:390` 與 `simulator-bridge.js`（都是 9045 模擬器）。
- **§4.1 第 2 條與 §4.7／§9 T1 不一致**：第 2 條把「回報過期但還連著」列在一律回「要」，§4.7 與 T1 寫「照最後一次」；頁面表現有規則 6 是照最後一次。
- **其他**：`tools\fshow_audit.py:69` 把 `WebPageTable.cpp` 裡符合 `^\s*\{\s*"Ident"\s*,\s*"` 的行當表單名讀 ⇒ 新碼避開那種形狀；編譯器是 MinGW.org GCC 6.3.0，沒有 `std::filesystem`（「還沒有呼叫者」的棘輪要用 FindFirstFileA 或 `git ls-files`）。

## 13. 指令紀錄（20260930，全部唯讀）

- `git -C D:\HT9045 log -5`（HEAD `b45d4482`，分支 `v906/steven-cbridge-review6`）；`git status --short`（只看、不碰別人的髒檔）；`git fetch`。
- `git show b45d4482:<檔>` 讀：`WebBridgeTags.cpp`／`.h`、`tools\wb_serve.cpp`、`WebPageTable.cpp`／`.h`、`WebWindowRegistry.cpp`、`WebBridge\WebBridgeServer.cpp`／`.h`、`WebBridge\TagSnapshot.h`、`WebBridge\HttpStatic.cpp`、`JsonBridge\ChanIo.h`、`ChanIoPoints.cpp`、`ChanMotor.cpp`、`ChanMotorPoints.cpp`、`WebMotorAccessLive.cpp`、`SecsTagPublish.h`／`.cpp`、`WebModalWake.h`、`JsonBridge\IoBtnPanelClick.cpp`、`ui\native\NativeFormsWbServe.cpp`、`ui\native\README.md`、`docs\CHANNEL_CENSUS_20260924.md`、`tools\webprobe\channel_census.py`、`tools\ioweb_probe.cpp`、`TesterComm\Tcp\CmdServerPump.cpp`、`EventLogAnalysis\ElaService.cpp`、`csystem.cpp`（fShow 那幾行）、`WebStart.cpp:1148`；`git grep` 找 `7016`／`7017`、`REST`、`TcpTagLink`、`TagSnapshot`／`ApiCacheGet` 的讀者、`PublishEventLogTags`、`SendCommand_EventLog`；`git ls-files` 確認 `tools\wb_publish.cpp`／`wb_gateway.cpp` 不存在。
- 網頁（工作樹，跟 `b45d4482` 相同）：`background.html`、`page\ht9045_link.js`、`page\ht9045_recipe_client.js`、`page\HW.IoSetView.html`、`page\HW.teach.html`、`page\HW.MotorTest.html`、`page\Main.MotorView.html`、`page\ht9045_mv_motor.js`、`page\dialog-bridge.js`、`page\ht9045_modal.js`、`page\settings.js`、`page\ht9045_main_close.js`、`page\ht9045_contactct_wire.js`、`page\ht9045_observer_wire.js`、`page\ht9045_smartdiag_web.js`、`page\ht9045_trayedit.js`、`page\ht9045_testcategory_wire.js`、`page\ht9045_home_refresh.js`、`page\shot_windows.js`；`grep -r` 限定 `D:\HT9045\web` 找 `setInterval`、`pci1203\.`、`io.di`、`motor.axes`、`motionView.trays`、`api/struct/(io|motor)`、`new WebSocket`／`HT9045Link.open`。
- golden V912（cp950 → UTF-8）：`iosetview.cpp:155-170`、`uteach.cpp:1363-1372`、`uMotorTest.cpp:912-914`。
- 交接檔：`git show origin/v906/steven-handoff:docs/handoff/FROM_STEVEN.md`（`2d208929`，§1 第 456 行、§4 第 753 行）、`git show origin/main:docs/handoff/TO_STEVEN.md`（`1b70caba`，§1 第 24／26／29／61 行、§4 第 131／172／216 行）、`git show origin/main:HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`（`1d596436`，§0 第 7 列）。
- 檔案大小（只讀）：`D:\HT9045\web\JSON\Production-update.json`、`IO-runtime.json`、`Motor-runtime.json`、`D:\HT9045\web\JSON\runtime\*.json`；行數：`D:\HT9045\system\IO_Table.csv`、`D:\HT9045\system\Mot_Table.csv`；`D:\AI_TempFile\q50-run\20260929_210245\r\r1\console.txt`（`published 8088 tags`）。
- 暫存目錄兩支 Python（不留在 repo）：把 `IO-runtime.json`／`Motor-runtime.json` 壓成單行量大小；依 `WebBridgeTags.cpp` 的 tag 名形狀推算 snapshot 大小。
