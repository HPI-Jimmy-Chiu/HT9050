# HT9045 Web Bridge — HTML 端的 JSON 契約

> **//Steven 20260916** — 新檔。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md`
> 最後更新：20260927（Steven 團隊）—— §1.3 防連點補註對過 HEAD `99ec7b7b` 的 `WebCmdGuard.cpp`／`.h` 更正（`motor.access` 改 action 級、IO 鈕 `W906IoClickGuardScope`、行號），全文以 `wbserve-conventions.md` §4 為準。

## 這份文件是什麼

這是**瀏覽器 HMI（消費端）對資料的要求**，寫給實作 `wb_serve` 與底層 C++（producer）的人當介面依據。

- 「**現況**」段落記的是**已經在服務中**的形狀 —— 逐項對照過原始碼行號，**不要重做，也不要改形狀**。
- 「**要求**」段落是 HTML 端需要而目前沒有的。每一項都寫了**為什麼非要不可**；沒有理由的欄位一律不要求。

## 這份文件不是什麼

- **不是** `ht9045-html-json` skill 裡「四大類 JSON／開站載入順序／更新觸發與頻率分類」那套規格。
  那套是 **C# JSON Simulator 寫檔、HTML 讀檔**的路徑，使用者 2026-09-16 裁決**不再使用**。
  兩套的欄位名看起來像，但傳輸模型完全不同，混著讀會做錯。
- **不是** IO／馬達硬體後端的規格。那由另外的工程師負責，本文件只定義「值送到瀏覽器時長什麼樣」。

## 一個貫穿全文的原則

> **不可知就送 `null`，不要送 0、不要送空字串、不要送 DFM 的展示值。**

`WebBridge/TagSnapshot.h` section 4 rule 3 已經這樣定，HTML 端也照這個實作（`null` 一律顯示 `---`）。
理由很實際：`0` 是合法的溫度、合法的計數、合法的權限等級。把「還沒讀到」顯示成 `0`，
操作員會照著一個編造的數字做決定，而且畫面看起來完全正常。

---

# 第一部分：現況（已在服務中，不要重做）

## 1.1 傳輸

| 通道 | 用途 |
|---|---|
| HTTP `GET` | 讀檔案類資料（配方、機台設定檔、記錄檔）。**只有 `GET`／`HEAD`**，其他一律 405 |
| WebSocket `/ht9045` | 執行期狀態串流（伺服器→瀏覽器）＋ 指令（瀏覽器→伺服器） |

20260929 起一個瀏覽器分頁只開**一條** `/ht9045`：外框（`web\background.html`）的 hub 持有連線，iframe 經 MessageChannel 走它（`web\page\ht9045_link.js`）。規則、停止（只放開 jog）、自測見 [ws-link-hub.md](ws-link-hub.md)。

寫入**一律走 WebSocket，不走 HTTP**。原因記在 `ht9045_recipe_client.js` 檔頭：
伺服器的請求解析器在 header 結束處就停了，連線狀態機裡沒有任何 request body 緩衝，
POST 需要的是那套機制而不只是一條路由。

**單則 WS 訊息上限 64 KiB**（`WebBridgeServer.cpp:258` `kMaxWsMessage`）。
超過會**直接斷線，不是回錯誤**。producer 送大量 tag 時要注意這條。

## 1.2 伺服器 → 瀏覽器的訊框

```jsonc
{"type":"snapshot","data":{ "<tag>": <值|null>, ... }}   // 連線時送一次完整狀態
{"type":"patch",   "data":{ "<tag>": <值|null>, ... }}   // 之後只送異動
{"type":"ack",     "id":<指令 id>, "ok":<bool>, ...}      // 指令回應
{"type":"alarm",   "code":"<字串>", "text":"<字串>", "at":"<時間>"}
{"type":"modal",   "title":"<字串>", "text":"<字串>", "at":"<時間>"}
{"type":"query",   "qid":<數>, "code":"<字串>", "kcode":<bitmask>,
                   "options":["RETRY","SKIP","CLEAN_OUT"], "at":"<時間>"}
```

出處：`WebBridgeServer.cpp:1125`（snapshot）、`:1443`（patch）、`:1216`（ack）、
`:1534`（alarm）、`:1558`（modal）、`:1582`（query）。

**`patch` 的語義**（`WebBridgeServer.cpp:1438`）：只帶「新增或值不同」的 tag；
上一次送過、這次不在 snapshot 裡的 tag，會以 `"<tag>": null` 送出，代表**消失／現在不可知**。
HTML 端照這個實作：把 `null` 存起來而不是刪掉，訂閱者才分得出「掉回未知」與「從來沒出現過」。

**`at` 的格式**：`IsoLocalNow()`（`WebBridgeServer.cpp:310-324`）產生 `2026-09-16T16:45:12`，
**沒有時區位移**。見 §2.4 的要求。

## 1.3 瀏覽器 → 伺服器的指令

信封固定（`WebBridge/CommandQueue.h:61`）：

```jsonc
{"type":"cmd", "id":<遞增數>, "cmd":"<名稱>", "tag":"<選填>", "value":<選填>}
```

> ⚠ **`value` 必須是字串。** 指令通道只帶 null／bool／數字／字串，帶不了物件。
> 複雜的 payload 一律先 `JSON.stringify` 成字串塞進 `value`。
> ⚠ **`dryRun` 一定要放在 `value` 那個 JSON 裡面**，不能當成訊息的兄弟欄位 ——
> `WebCommand` 只認 `{id,cmd,tag,value}`，放外面會被**靜默丟棄而變成真寫**。

| 指令 | tag | value（JSON 字串） | ack 的 payload |
|---|---|---|---|
| `control.acquire` | — | — | 失敗時 `error:"control-held"` |
| `control.release` | — | — | 失敗時 `error:"not-operator"` |
| `recipe.doc.put` | 文件名 | `{sections:{<區段>:{<鍵>:{raw}}}, dryRun}` | `{changed, identical, notFound}` |
| `system.file.put` | 檔名 | ini：`{sections:…, dryRun}`／csv：`{rows:{<列鍵>:{<欄>:值}}, dryRun}` | `{changed, identical, notFound, backup}` |
| `system.csv.rows` | csv 檔名 | `{add:[{<欄>:值}], delete:["<列鍵>"], dryRun}` | `{added, deleted, notFound, backup}` |
| `system.levels.put` | `levelset` | `{values:{"<索引>":<整數>}, dryRun}` | `{changed, identical, notFound, backup}` |
| `modal.answer` | — | 回答內容 | — |

> ⛔ **補註（Steven 團隊 20260926，HEAD 8fad1522）**：上表是 20260916 的指令集，之後新增的指令不在這裡重列，
> 避免兩處維護：
> - **C 路**（golden 表單橋）三個：`editlist.get`（tag＝結構名，value 省略）、`editlist.save`
>   （value＝`{"widgets":{…},"answers":{…}}` 的 JSON 字串）、`form.save`（A 形狀，只剩 `Setup.HotPlate.html`）。
>   形狀、檢查順序、ack 欄位見 `route-c-golden-bridge.md` §3。
> - C 路接手的檔，`recipe.doc.put`／`system.file.put` 會回 409（`CRouteOwner`，`dryRun` 放行；同檔 §6）。
> - 對話框回答線上一律用 `dialog.response`，`modal.answer` 只是伺服端別名（skill `ht9045-json-bridge`
>   `references/decisions.md` Q30-8）。
> - `system.levels.put` 20260926 起改走 golden `TfSecurity::FormClose` 全流程（`WebLevelSet.cpp` 的
>   `W906_LevelSetPut`，commit `8c5ea501`，json-bridge `write-inventory.md` `LevelSet` 列）。
> - ⛔ **防連點（Steven 團隊 20260926～27 補，S107-3；`2ae40ffe`，`8314e3bd` 把 `observer.get` 改成 act 級）**：（⛔ 20260927 更正：這一條寫在 `f45b92f5`（`motor.access` 改 action 級）與 `b782b00b`（IO 鈕 scope）之前，過期的句子逐句標在原處，現況見本條最後一項與 `wbserve-conventions.md` §4）wb_serve 在分派迴圈頭
>   （`tools/wb_serve.cpp:4676` 的 `W906CmdGuardScope`；本體 `WebCmdGuard.cpp`／`.h`，只依賴 WebCommand／TagValue／cJSON，沒有機台碼）
>   對**每一條** WS 指令判：同一個 `cmd`＋`tag`＋`value`（型別＋文字）（⛔ 20260927 更正：兩個例外——`motor.access` 的 value 先正規化再算 key；`io.btnPanelClick` 不經主 guard、改用 `cmd`＋`tag`＋down，見本條最後一項）在上一條**完成後** 400 ms 內又到、或上一條還在排隊 → **不執行**，
>   ack `ok:false`、`error` 以 **`busy:`** 開頭（例 `busy: same command in progress or just done (towerlight.op, 120 ms ago)`）。
>   `busy:` 的意思是「第一下還在跑或剛做完」，**不是失敗**。`value` 算在判斷裡，所以兩段式確認（`confirmed:false`→`true`）、
>   `answers` 重送、checkbox 來回切都不會被擋；被擋的那一下不延長窗口。
>   - 白名單（不擋）：名稱級 `sys.ping`、`cfg.resync`、`log.event`、`ui.windows.put`、`stream.resync`、`modal.answer`、`dialog.response`、
>     `dialog.auth`、`motor.access`、`motor.stop`、`io.btnPanelClick`、`sim.di.set`、`pause.run`、`editlist.get`、`contactct.get`、
>     `counterclear.get`、`auth.mode`，前綴 `pci1203.`、`olp.`（`WebCmdGuard.cpp:61-77`）（⛔ 20260927 更正：`motor.access` 已移出名稱級、改看 `action`（`f45b92f5`）；`io.btnPanelClick` 仍在名稱級，但改由 IO 鈕 scope 擋（`b782b00b`）；行號現為 `:79-90`、前綴 `:91-94`）；op 級（看 value JSON 的 `op` 或 `act`，
>     只有列出的讀取類放行）見 `WebCmdGuard.cpp:22-37` 的說明表與 `:97-108` 的 `kOpRules`——例 `observer.get` 的 `yieldSite`／`yieldMax`／
>     `yieldMin`／`yieldClear` 要擋（`:107`）（⛔ 20260927 更正：`motor.access` 另看 `action`；行號現為說明表 `:26-54`、`kOpRules` `:117-129`、`observer.get` 那條 `:127`）。其他指令（含 `dryRun` 預覽、之後新加的指令）一律擋。
>   - 窗口：環境變數 `W906_CMDGUARD_MS`（預設 400，`0`＝關，上限 10000，`WebCmdGuard.h:82-83`；⛔ 20260927 更正：現為 `:90-91`，`0` 也同時關掉 IO 鈕 scope）。**探針要在 400 ms 內重送同一指令，
>     啟動 wb_serve 前設 `W906_CMDGUARD_MS=0`**（`2ae40ffe` 本文舉的例子：`status_towerlight_probe` 的 `click_led` 間隔 0.2 s）。
>     ctest `WebCmdGuard`（`tests/test_webcmdguard.cpp`）。
>   - 頁面第二道：`web/page/ht9045_busy_util.js` 的 `window.HT9045Busy`（`is(x)` 認 `busy:`、`coolMs()`＝400、`NOTE` 中性提示），
>     各頁在呼叫當下才取用（沒載入就照舊）；目前 12 頁載入（`main.html` 與 St01 的 Data／Status／Setup 頁，
>     `grep -l ht9045_busy_util.js web/page/*.html`，20260927）。**`ht9045_wire_engine.js`／`ht9045_recipe_client.js`（Jimmy 的引擎檔）
>     沒有處理 `busy:`**，走引擎存檔的 C 路頁面會把它當一般錯誤顯示（`62f064bf` 本文「Not done here」）。
>   - ⛔ **20260927 更正（現況；對過 HEAD `99ec7b7b` 的 `WebCmdGuard.cpp`／`WebCmdGuard.h`／`tools/wb_serve.cpp`。全文以同資料夾的 `wbserve-conventions.md` §4 為準：`D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md`）**
>     - **名稱級白名單**（整條不經主 guard；`WebCmdGuard.cpp:79-90`，前綴 `:91-94`）：`sys.ping`、`cfg.resync`、`log.event`、`ui.windows.put`、`stream.resync`；
>       `modal.answer`、`dialog.response`、`dialog.auth`；`motor.stop`、`io.btnPanelClick`（另有 IO 鈕 scope，見下）、`sim.di.set`；`pause.run`；
>       `editlist.get`、`contactct.get`、`counterclear.get`、`auth.mode`；前綴 `pci1203.`、`olp.`。**`motor.access` 已不在這張表**（`f45b92f5`）。
>     - **op 級白名單**（看 value JSON 的一個欄位，只有列出的值放行、其他值照擋；說明表 `:26-54`、`kOpRules` `:117-129`）：
>       看 `op`：`security.jam`、`security.passwd`、`lotinfo.op`、`towerlight.op`、`recipe.change`、`act.main.peModel`、`act.sortCT.clearCount`；
>       看 `act`：`smartdiag.op`、`builder.op`、**`observer.get`**（`8314e3bd`，S116Y：讀取類 `""`／`open`／`timer`／`tab`／`rowNo`／`form`／`year`／`month`／`file`／
>       `filter`／`query`／`ccKinds`／`ccKindsForm`／`ccHistory`／`ccHistoryForm` 放行；會改記憶體的 `yieldSite`／`yieldMax`／`yieldMin`／`yieldClear` 擋；
>       `kObserverActs` `:112-113`，規則 `:127`）。欄位缺時用本體的預設（例 `observer.get` 缺 `act`＝`open`）；value 不是 JSON、或欄位不是字串＝不在白名單（擋）。
>     - **`motor.access` 看 `action`**（RULINGS_20260927 §2 第 8 題 A，`f45b92f5`；`kMotorAccessActs` `:115-116`，規則 `:128`）：放行 `jogP`、`jogN`、`stop`、
>       `setSpeed`（捲軸拖動連續送），以及頁面專用、C++ 無動作的 `setPos1`、`setPos2`、`refreshParameter`、`setTeachFromCurrent`、`setTeachFromOffset`；
>       `home`／`loopMove` 帶 `params.start=false`（抬起＝停止方向）也放行（`MotorAccessReleaseDir` `:139`，`Exempt` 裡 `:318`）。其他全擋：缺 `action`、
>       `moveRelative`、`moveAbsolute`、`moveSoftLimitP／N`、`home`／`loopMove`（start=true）、`servoToggle`、`motorPowerToggle`、`teachSet`、`teachGo`、各種 setter…
>       **key 先正規化**（`KeyOf` `:362` → `MotorAccessKeyText` `:182`）：算 key 前拿掉 `seq`、`id`、`issuedAt`、`state`、`reason`、`params.currentPos`、
>       `params.speedEvent`，`moveRelative` 另拿 `params.targetPos`、`servoToggle` 另拿 `params.servoOn`（`kMotorAccessStrip` `:167-178`）——不拿掉的話每按一下
>       `seq` 都變、永遠擋不到。不同軸、不同距離、不同教導點仍是不同 key。
>     - **IO 面板輸出鈕 `W906IoClickGuardScope`**（RULINGS_20260927 §2 第 9 題 A＋Jimmy 08:3x 條件，`b782b00b`；`WebCmdGuard.h:193-246`、`WebCmdGuard.cpp:566` 起）：
>       `io.btnPanelClick` 走輸出優先 `W906_ServiceOutputs`（`tools/wb_serve.cpp:6303`，呼叫 `:6335`），不經過迴圈頭，所以擋在真正執行的 `W906_DispatchIoClick`
>       第一行（`:6215`）；它留在名稱級白名單，主分派那條（`:5791`）才不會被擋兩次。key＝`cmd`＋`tag`（Alias）＋正規化後的 down（0、1，其他＝-1；
>       `IoClickDownOf` `:376`、`ButtonKeyOf` `:400`）。**同一顆鈕、同一個 down** 在 W 內第二下 → busy；**down 跟上一次不同一律放行**（400 ms 內 開→關→開
>       三下都會執行，輸出＝最後一下）；不同 Alias 不互擋；沒有 tag 的不擋也不記。跟主 guard 共用同一個實例與 `W906_CMDGUARD_MS`。
>     - **busy 訊息原文**（ack `ok:false`，字首固定 `busy:`；`CheckKey` 組字串 `WebCmdGuard.cpp:444-445`）：主 guard
>       `busy: same command in progress or just done (towerlight.op, 120 ms ago)`；IO 鈕
>       `busy: same button and state within 400 ms (io.btnPanelClick C_Load_Up down=1, 120 ms ago)`（400＝目前的 W）。
>     - **`W906_CMDGUARD_MS`**：十進位毫秒，預設 400（從上一條完成起算）；`0`＝整個關掉（主 guard 與 IO 鈕都關）；上限 10000（超過夾到 10000）；不合法＝用預設並印一行。
>       第一次用到時讀一次（常數 `WebCmdGuard.h:90-91`、說明 `:42-46`；`WebCmdGuard.cpp:504-520` `InitialWindowMs`、`:524` `WebCmdGuardGlobal`），開機 log 印
>       `[cmdguard] double-click guard: window 400 ms …`。ctest `WebCmdGuard`＝`tests/test_webcmdguard.cpp`（`[1]`～`[18]`，`tests/CMakeLists.txt:3804-3817`）。

**單一操作員權杖**：任何寫入前要先 `control.acquire`；第二個連線在權杖被持有時會被拒（`control-held`）。

**寫入閘門**：`system\` 底下的檔案除了 `--allow-cmd` 還要 `--allow-system-write`，預設兩者皆關。

**拒寫語義（重要，已定案）**：all-or-nothing 拒寫時 ack **必須** `ok:false` 且 `changed:0`。
不可以只回 `notFound>0` 卻讓 `ok` 維持 `true` —— 那會讓畫面顯示「寫入完成」而磁碟沒動。
`dryRun` 也套同一條：預覽會被拒，就該在預覽時說會被拒。

## 1.4 HTTP 讀取端點

```
GET /api/recipe/            {recipe, path, documents:[…]}
GET /api/recipe/<doc>       {path, available, sections:{<區段>:{<鍵>:{value,type,raw}}}}

GET /api/system/            {files:[{name,path,kind,available,bytes,…}]}
GET /api/system/<name>      ini  -> 與配方文件同形狀
                            csv  -> {columns:[…], keyColumn, rows:[{<欄>:值}]}
                            i32  -> {path,kind:"i32",available,bytes,count,min,max,values:[…]}

GET /api/text/              {roots:[{name,path,kind,available}]}
GET /api/text/<root>/…      目錄 -> {path,root,subPath,entries:[{name,dir,bytes,unreachable?}],truncated}
                            檔案 -> {path,bytes,text}
```

> ⚠ 每個欄位是 `{value, type, raw}` 三件組不是裸值。`settings.js:154` 只有在
> `value` 與 `raw` **同時存在**時才解包；只送 `{raw,type}` 會讓頁面拿到整個物件
> 並渲染成 `[object Object]`，而且不會報錯。**不要「簡化」這個 payload。**

---

# 第二部分：HTML 端的要求（目前沒有）

## 2.1 訊框層的 metadata，不要掛在每個 tag 上

`ht9045-html-json` skill 的舊規格要求每個 stream 帶
`available` / `updateClass` / `trigger` / `updatedAt` / `seq` 五個欄位。
**照字面搬到這條串流上是錯的做法**：117 個 tag × 5 個欄位 × 每 500ms，
光 metadata 就比資料本身大好幾倍，而且會撞上 64 KiB 上限。

要求的形狀是**兩層**：

```jsonc
{
  "type": "patch",
  "seq":  1234,                              // 新增，見 §2.2
  "at":   "2026-09-16T16:45:12+08:00",       // 新增，見 §2.4
  "trigger": "test-complete",                // 新增，見 §2.5
  "data": { "<tag>": <值|null> }             // 不變
}
```

五個欄位的歸屬：

| 舊欄位 | 歸屬 | 說明 |
|---|---|---|
| `seq` | **訊框層**，producer 產生 | 整條串流一個序號，不是每個 tag 一個 |
| `updatedAt` → `at` | **訊框層**，producer 產生 | HTML 只知道「我收到的時間」，那會把 500ms tick 與網路延遲算進去，不能拿來判斷資料新舊 |
| `trigger` | **訊框層**，producer 產生 | 只有 producer 知道是哪個事件造成這次變動 |
| `available` | **不需要這個欄位** | 現行協定已經用 `null` 表達「不可知」。除非要分辨「來源根本不存在」與「存在但這一刻讀不到」——若要分辨，見 §2.6 |
| `updateClass` | **HTML 端自己定，不上線傳** | 它是設計時就決定的靜態屬性，不隨每次訊框變。放在接線檔的對照表裡（見 §3.1） |

## 2.2 `seq` — 串流序號

**要求**：`snapshot` 與 `patch` 都帶 `seq`，單調遞增，同一條連線不重複、不回頭。

**為什麼非要不可**：沒有它，HTML 無法察覺自己漏了一幀。
現在的行為是「收到什麼就套用什麼」，中間掉一幀 patch 會讓畫面上某幾格永遠停在舊值，
而且**看起來完全正常**（沒有紅字、沒有 `---`）。這是會誤導操作員的那種壞法。

**HTML 端的反應**（我們這邊實作，不需要 producer 做）：

| 情況 | HTML 的行為 |
|---|---|
| `seq` == 上一次 + 1 | 正常套用 |
| `seq` 跳號 | 丟棄該幀，送 `{"type":"cmd","cmd":"stream.resync"}`，在收到新的 `snapshot` 前把畫面標成 stale |
| `seq` 比上一次小 | 視為伺服器重啟，等同 resync |
| `type=="snapshot"` | 無條件接受並把 `seq` 設為該幀的值 |

**需要 producer 配合的**：一個 `stream.resync` 指令，收到就重送完整 `snapshot`。

## 2.3 `snapshot` 要能主動重送

**要求**：指令 `stream.resync`，ack 之後（或同時）送一幀完整 `snapshot`。

**為什麼**：目前 `snapshot` **只在連線建立時送一次**（`WebBridgeServer.cpp:1125`）。
HTML 發現 seq 跳號時，唯一的補救方法是斷線重連 —— 那會連帶丟掉單一操作員權杖，
如果當下正在寫入流程中間，後果不可控。

## 2.4 `at` 要帶時區位移

**要求**：`IsoLocalNow()`（`WebBridgeServer.cpp:310`）目前輸出 `2026-09-16T16:45:12`，
**沒有時區**。請補成 `2026-09-16T16:45:12+08:00`。

**為什麼**：沒有位移的字串在 JavaScript 裡 `new Date("2026-09-16T16:45:12")` 會被當成**本地時間**解析
（ES2015 之後的行為），但同樣的字串帶 `Z` 會被當成 UTC。機台、瀏覽器、記錄檔如果不在同一個時區，
差值會是整數小時而且**不會有任何錯誤徵兆**。這個欄位已經在 `alarm`／`modal`／`query` 上使用中，
補位移對三者都有好處。

## 2.5 `trigger` — 這一幀為什麼發

**要求**：訊框層一個字串。建議的值域（對應 skill 的五個更新分類）：

| `trigger` | 什麼時候發 | 對應舊分類 |
|---|---|---|
| `"startup"` | 連線建立時的第一幀 snapshot | `startup-once` |
| `"resync"` | 回應 `stream.resync` | — |
| `"tick"` | 週期性重發（目前是 500ms） | `forced-poll` |
| `"operator"` | 操作員動作造成的變動（存檔、切換 Recipe、Lot Start/End） | `operator-event` |
| `"production"` | 生產事件（Test Complete、Bin 計數、連續 fail） | `production-event` |
| `"controller"` | 溫控器通訊完成 | `controller-poll` |

**為什麼**：HTML 端要據此決定**要不要重讀檔案類資料**。
例如 `trigger=="operator"` 且 `recipe.current` 變了，就要重新 `GET /api/recipe/`；
`trigger=="tick"` 則什麼都不用做。沒有這個欄位，HTML 只能每次都重讀（浪費）或都不重讀（顯示過期資料）。

> 值域可以增加，HTML 端對不認得的 `trigger` 一律當成 `"tick"` 處理（保守但安全）。

## 2.6 `available`：只在要分辨兩種「沒有值」時才需要

目前 `null` 同時表示「來源還沒載入」與「這台機器沒有這個硬體」。
畫面上兩者都顯示 `---`，對操作員來說夠了。

**但如果 producer 希望 HTML 分辨**（例如把「未安裝」畫成灰色、「讀取中」畫成閃爍），
請**不要**加 `available` 布林欄位 —— 那樣要嘛每個 tag 多一層物件（撞 64 KiB），
要嘛另開一張表。建議改成**訊框層的一張清單，只在 snapshot 帶**：

```jsonc
{"type":"snapshot","seq":1,"at":"…","trigger":"startup",
 "data":{…},
 "absent":["io.fix4.*","motor.sortArm.*"]}      // 這台機器沒有的，前綴或完整 tag 名
```

`absent` 只在機台組態變動時才會變，不需要每 500ms 送。

## 2.7 tag 命名規則（給新增 tag 的人）

現行 117 個 tag 已經遵守，請延續：

- 小寫、點分層，`<子系統>.<物件>.<欄位>`，例如 `temp.sv`、`lot.auto1.trayCount`。
- 陣列用數字段：`zone.hotplate.1`、`zone.hotplate.2`（**從 1 開始**，跟畫面上的編號一致，不是 C 陣列下標）。
- **不要在 tag 名裡編碼單位或型別**（不要 `temp.sv.degC`）。單位屬於畫面，型別看值。
- 名字一旦上線就是契約。要改名請**同時送新舊兩個名字至少一個版本**，
  否則 HTML 的對照表會變成 ghost（對照表有、伺服器沒送），表⑥ 會標出來但畫面上那一格就是空的。

## 2.9 值域（min／max）—— 兩邊都要有，不是二選一

> 使用者裁決 20260916：「這題應該是兩邊都要有，因為手動輸入的時候會需要知道上下限，
> 讀檔的時候也需要做上下限的保護。」

這一條**與「變動門檻（deadband）」是兩件不同的事**，本文件把它們分開：
值域是「這個參數合法的範圍」，門檻是「變化多少才值得送一幀」。前者是安全問題，後者是頻寬問題。
先解決前者。

### 現況：一半已經有了，另一半完全沒有

| 用途 | 現況 |
|---|---|
| **手動輸入的上下限** | ✅ **已有**。接線檔的 `kb` 區塊帶 `[FLAG, dp, checkRange, min, max]`，來源是 golden 的 `ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)`，QWERTY 小鍵盤照它擋 |
| **寫入時的值域保護** | ⚠ **只有 `levelset` 有**（`system.levels.put` 收 0..4，超出整批拒寫）。`recipe.doc.put`／`system.file.put`／`system.csv.rows` **完全沒有值域檢查**，任何字串都寫得進去 |
| **讀取時的值域保護** | ❌ **沒有**。檔案裡的值超出合法範圍時照原樣送上來 |

### 為什麼瀏覽器端擋了還要伺服器端再擋

這不是重複，是**必要的雙重檢查**，而且本專案今天早上已經為同一個問題做過同樣的決定：

> `motTable` 的新鍵限制 `M##`（伺服器端＋瀏覽器端雙重檢查）……
> **WS 直連可繞過瀏覽器檢查，所以伺服器端也要擋。**
> —— `CHANGES_20260916_Steven.md` §7c 關鍵決策

小鍵盤的 min/max 只在「操作員用畫面上的小鍵盤打字」時生效。
任何人開一條 WebSocket 直接送 `system.file.put`，那道檢查等於不存在。
伺服器是唯一擋得住的地方；瀏覽器那道的價值是**即時回饋**（打到超出範圍當下就知道，
不必送出去等 ack），不是安全。

### 要求

**(1) 讀取端點回傳值域。** ini／csv 的欄位投影目前是 `{value, type, raw}`，
請在**有值域可講的欄位**補上 `min`／`max`（沒有就不要放，不要填 0 當佔位）：

```jsonc
"Soak Time": {"value": 120, "type": "int", "raw": "120", "min": 5, "max": 10000}
```

值域的來源應該是 golden 自己的 `CheckRange()` 呼叫與 `ShowQwertyKey` 的參數，
**不要另外發明一套**。`/api/system/levelset` 已經是這個形狀（`count`／`min`／`max` 在檔案層）。

**(2) 寫入時伺服器端檢查，超出就整批拒寫。** 語義與 `system.levels.put` 一致：
`ok:false`、`changed:0`、`error` 說明是哪個鍵超出什麼範圍。
**不要夾回合法範圍**（clamp）—— 操作員以為自己存了 15000，實際存進去 10000，
畫面重讀後才發現，那比直接拒絕更糟。

**(3) 讀到超出值域的既有值時，照實送、不要修。**
檔案裡本來就有的壞值是**現場的事實**，HTML 會把它顯示出來並標記異常。
伺服器擅自夾回去會讓這個問題永遠不被發現。
（HTML 端已有對應機制：`Status.Security` 讀到超出畫面選項數的值時記入 `UNFILLABLE`
並讓整頁拒寫，而不是靜靜挑一個選項。）

### 變動門檻（deadband）—— 暫時不做

浮點數最後一位的跳動會讓 patch 一直有內容。但在補上 `seq` 與量到實際的 patch 頻率之前，
這是**還沒發生的問題**（現行實測 6 秒 0 幀）。
先不要做 —— 加了門檻就等於「畫面上的值與機台的值容許有落差」，那是要付代價的決定，
應該等有實測數據證明頻寬真的是問題時再談。

## 2.8 值的型別

| 型別 | 送什麼 | 不要送什麼 |
|---|---|---|
| 數值 | JSON 數字 `85` 或 `85.0` | 字串 `"85"`（HTML 要做 `toFixed` 會變成字串相加） |
| 布林 | `true` / `false` | `0` / `1` / `"ON"` |
| 列舉 | **字串**，例如 `"SIM RUN"` | 裸整數 —— 意義只有 C++ 那邊知道，HTML 會被迫複製一張對照表 |
| 不可知 | `null` | `0`／`""`／`"---"`／`"N/A"` |

---

# 第三部分：HTML 端自己負責的（producer 不用管）

## 3.1 `updateClass` 對照表

放在各頁的接線檔 `ht9045_wire_<slug>.js` 的 `tags:` 區塊，不上線傳：

```js
tags: {
  'temp.sv': ['edWorkTemperBase', 'text', 1],   // 元素 id、形狀、小數位
}
```

`updateClass` 是「這個 tag 預期怎麼更新」的靜態註記，HTML 用它決定 stale 門檻
（例如標記為 `production` 的 tag 十分鐘沒動很正常，標記為 `tick` 的十秒沒動就該警告）。

## 3.2 tag → 畫面元素的對照

由 HTML 端維護並負責查證。**只收有依據的對應** —— 一個接錯的 tag 會讓畫面上一個
看起來正常的數字其實來自別的東西，比空著更危險。

現況：117 個 tag 只接了 3 個，其餘 114 個在現行 HTML 上沒有可驗證的目標元素。
盤點見 `page/ScreenShots.html` 表⑥（由 `scratchpad/gen_tag_status.py` 從執行中的伺服器量出來）。

## 3.3 stale 與斷線的顯示

HTML 端負責，producer 不用送「我還活著」：WebSocket 斷線本身就是信號。

---

# 第四部分：三個設計問題的建議（待使用者確認）

## 4.1 `stream.resync` 的回應方式 → **建議：ack 之後另送一幀 snapshot，只送給提出要求的那條連線**

| 做法 | 評估 |
|---|---|
| **(建議) ack 後另送 snapshot** | snapshot 這條路徑**兩邊都已經存在且已測過** —— 伺服器 `SendJson(c, …)` 是逐連線的（`WebBridgeServer.cpp:1125`），瀏覽器的 snapshot 處理也已實作。resync 與連線建立**收斂到同一段程式**，要測的狀態少一組 |
| ack 本身帶 snapshot | ack 的 payload 目前是 JSON **字串**，把 117 個 tag 塞進去等於**雙重編碼**（引號全部要跳脫），長度會膨脹一到兩成，更容易撞 64 KiB 上限。而且它會製造第二條「如何套用完整狀態」的程式路徑 |

⚠ **必須逐連線送，不可廣播**：一條連線漏幀不代表其他連線也漏了，廣播會讓沒問題的連線也重置。

---

## 4.2 `production` 事件要不要重建 delta ＋ checkpoint → **建議：不要**

舊規格（A 路）有 `test-complete-delta`（只送受影響的 Site ＋ Arm counters）與
「每 100 delta 或 5 分鐘一次 checkpoint」。三個理由不要重建：

1. **`patch` 本身就已經是 delta，而且是逐連線算的。**
   `WebBridgeServer.cpp:1438` 拿 `c.lastSent` 跟當下狀態逐 tag 比對，只送有差的。
   它比舊規格的 delta 更精確 —— 舊的是「這個事件影響哪些東西」（要人工維護清單），
   新的是「這條連線實際還沒收到什麼」（機械算出來的，不會漏也不會多）。
2. **`seq` ＋ `stream.resync` 取代了 checkpoint。**
   checkpoint 的作用是「萬一漏了能回到已知狀態」，但它是**用時間猜**什麼時候需要復原。
   seq 是**偵測到真的漏了**才復原，更即時也更省。
3. **語義分組是從 tag 名推得出來的。**
   實測 tag 命名是 `site.arm1.s5`／`sort.auto1.count` 這種形狀，
   patch 裡出現哪些名字，就知道是哪個 Site、哪條 Arm。不需要另外包一層。

### ⚠ 但有一個真的缺口，請一起評估

500ms 的 tick 會把**兩個相鄰的生產事件合併成一幀 patch**。對「顯示目前狀態」沒差
（本來就只看最新值），但對「每一個事件都不能漏」的用途會丟掉中間那次。

現行 HMI 顯示的是**狀態**不是事件流，所以不受影響。
如果之後需要事件流（例如逐筆測試結果），**建議另開一條像 `alarm` 那樣的事件訊框**，
不要去改 tag 串流的語義 —— tag 串流的價值正在於「最新狀態」這個簡單模型。

---

## 4.3 `absent` 清單要不要做 → **建議：不要放進串流，HTML 自己從 `/api/system/gerneral` 算**

「這台機器有沒有裝這個硬體」的權威來源是 `Gerneral.ini`，
實測它有 623 個鍵，其中就有 `INSTALL_OCR`／`INSTALL_HEAT_GUN`／`USE_COLOR_TRAY_SENSOR`
這類安裝旗標 —— **而且 `/api/system/gerneral` 已經在服務中**。

| 做法 | 評估 |
|---|---|
| **(建議) HTML 從 `/api/system/gerneral` 自己算** | 不動協定、不增加線上位元組。而且這正好把 skill 裡那個一直缺的 **`View-rules` 衍生層**補起來（「依 General-config ＋ Config 的值判斷各視窗/頁內區塊 hide／disable／dim」），一舉兩得 |
| 串流加 `absent` 清單 | 把一個**開機後就不會變**的東西放進每次連線的訊框。而且會出現兩個真相來源：`Gerneral.ini` 說沒裝、`absent` 沒列到，該信哪個？ |

也就是說：**這一題的答案不是改協定，是補 HTML 端的 View-rules 層。**
補完之後「未安裝」與「讀取中」在畫面上自然就分得開 —— 前者由組態決定（灰掉），
後者是 `null`（顯示 `---`）。

---

## 4.4 值域 → 見 §2.9（使用者已裁決：兩邊都要有）

# 附錄：現況數字（2026-09-16 實測）

| 項目 | 數字 | 怎麼量的 |
|---|---|---|
| 線上 tag 數 | **117** | `snap.stagedTagCount()`／連線抓 snapshot |
| 其中有值（非 null） | 9 | 同上 |
| HTML 已接 | 3 | `scratchpad/gen_tag_status.py` |
| 6 秒內的 patch 幀數 | **0** | 原始 socket 觀察 |

> 最後一項不是壞掉：能變動的 tag 幾乎都被 `CustomerCodeLoaded()` 的 liveness 閘門擋成 null，
> 根因是 `database.cpp:333` 的 early return（`D:\GPIB9045\system\general.ini` 的
> `Model=9050GPIB` 不在接受清單裡，導致 `CUSTOMER_CODE` 那一行永遠讀不到）。
> **不是移植 bug** —— golden V899 `database.cpp:308-321` 一模一樣。
>
> ⚠ 引用 tag 數時**不要引註解**：`WebBridgeTags.h` 曾寫 61、`.cpp` 曾寫 100，都過期。
> 要現況請跑 `scratchpad/gen_tag_status.py`。
