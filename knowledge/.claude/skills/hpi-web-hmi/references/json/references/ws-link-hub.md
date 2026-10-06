> 保存來源：`.claude/skills/ht9045-html-json/references/ws-link-hub.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 一個瀏覽器分頁一條 WebSocket：外框 hub（WSLINK，20260929～30）

> 撰寫：St01 ST01-E3。程式：`web\page\ht9045_link.js`（檔頭 HUB RULES 是權威）；自測：
> `HT9011UC_Cpp_V3.33.906.0\tools\webprobe\ws_link_selftest.cjs`（ctest `WB_WsLink`，node、離線、不連 wb_serve、不讀寫機台檔）。

## 1. 為什麼

Jimmy 20260929：機台 9/26 實測每個 iframe 各開一條 WebSocket，23 條撞到伺服器 16 條上限，Motor Test 整頁沒反應。
改成**外框（`web\background.html`）持有唯一一條真的連線**，iframe 經 MessageChannel 走外框的 hub；伺服器端不用改。

## 2. 接線（都是同一行插入，不移動行號）

| 位置 | 做什麼 |
|---|---|
| `web\background.html:312` | 先載 `page/ht9045_link.js` 並 `HT9045Link.startHub(ws://…/ht9045)`，再載 recipe client |
| `web\background.html:668` `setWinState` | 視窗狀態不是 open（closed／minimized）⇒ `HT9045Link.windowHidden(那個視窗的 iframe.contentWindow, st)`（§4） |
| `web\background.html:669` | `pushRegistry(false)`：開／關／縮小的邊緣立刻送（原本 60 ms 防抖會吃掉「開了又關」） |
| `web\page\ht9045_recipe_client.js:134` | iframe 沒載 `ht9045_link.js` 就從**同資料夾**載一次（`document.currentScript.src`），`connect()` 等它最多 2 秒，載不到＝照舊直連 |
| `ht9045_recipe_client.js:144`／`:185` | `connect()` 先等上面的 `linkWait` |
| `ht9045_recipe_client.js:146` | `HT9045Link.open(wsUrl())`（沒有 HT9045Link 時照舊 `new WebSocket`） |
| `ht9045_recipe_client.js:165` | 收到 `{"type":"link.token","owner":false}` ⇒ 清掉 `haveToken`（下一個自動動作重新 acquire） |
| `ht9045_modal.js:271`、`js\transport\ws.js:59`、`pci1203.html:608` | 同樣改走 `HT9045Link.open` |
| `web\page\HW.teach.html:494` | 頁面自己的放開：`pagehide`、父視窗 `HT_WIN` 狀態不是 open ⇒ `HTMotorAccess.releaseHeld()`（同 `HW.MotorTest.html`） |

C# 模擬器（另一台伺服器、只在 debug 模擬模式）不走 hub；`background.html:392` 那條送完就關。

## 3. hub 的規則（`ht9045_link.js` 檔頭）

- 每個指令**只轉送一次**、hub 不重送（伺服器的 WebCmdGuard 400 ms 內重送會回 busy；重送留給各頁自己）。
- 有數字 `id` 的訊框換成 hub 全域 id，ack 換回原 id、**只回給發問的那一頁**；沒有 id 的（snapshot／patch／alarm／modal／query…）扇出給每一頁。
- tag 快取（snapshot＋patch，null 照存）：晚開的頁拿到合成 snapshot（帶最後的 seq）。
- 權杖（一條連線＝一個瀏覽器＝一個操作員）：
  - `control.acquire`（自動）：hub 拿著時就地回 ok；沒拿著時只送一個，其他等它的 ack。
  - `control.takeover`（操作員按的）**一律送伺服器**：要能從**另一個瀏覽器**拿回來。同一條連線接手時伺服器的 owner 不變
    （`tools\wb_serve.cpp` `W906_OwnerHeldSince`：同頁接手不算換人），所以同一個瀏覽器內換頁接手**不會觸發取消**（Jimmy 條件②）。
  - `control.release` 只有最後一個持有的頁放掉才送；`not-operator` ⇒ 每一頁收到 `link.token owner:false`。
  - `control.*` 在伺服器的 socket 執行緒就回了（`WebBridge\WebBridgeServer.cpp:1388`），**不經 WebCmdGuard**，不會 busy。
- `ui.windows.put` 不經 hub：外框是它唯一的送出者。

## 4. 停止：hub 只會放開 jog，**從來不送 STOP**

伺服器的 `motor.stop`（`WebMotorAccess.cpp` `DoStop`）：button 是 jog 鈕 ⇒ `DoJogRelease`（只停那一軸）；
其他 button（例 `btnStop`）＝ golden `StopAllMotor`＋`CancelAllJobs`＋每一軸 `Stop1203`，而且過 SystemStart 閘。

- **只有「按著的 jog」會讓 hub 出手**：`motor.access` 的 action 是 `jogP`／`jogN` 就記 `c.held[button]`；
  那一頁自己送同一顆的 `motor.stop` 就清掉；`btnStop` 這類非 jog 的停止清掉全部。
  formShow／formClose／Teach 的 `{query:true}`／移動／HOME／Loop **都不記**。
- 出手時機（都呼叫 `_releaseHeld`：每個按著的 jog 送一個 `motor.stop`，**button＝那顆 jog 自己的按鈕**）：
  - 頁面不見：pagehide 的 bye、port 關閉、ping 漏 3 次（reason `link.pageGone:<why>`）；
  - 視窗被藏起來：外框關／縮小只是 `display:none`，iframe 沒卸載、不送 bye、ping 照回，所以要外框呼叫
    `HT9045Link.windowHidden()`（reason `link.windowHidden:<state>`）。
- 頁面的 HOME／Loop 不動：golden `TfMotorTest::FormClose` 什麼都不停；關窗的語意是伺服器的頁面表關窗邊緣（Jimmy 的 (b)）。
- ⛔ **M1（20260930，St02-E2 審查 B）**：第一版 hub 對「送過任何 motor.access 的頁」在頁面不見時送 `button:'link.pageGone'`
  ＝伺服器的 DoStop＝整台 StopAllMotor——生產中重新整理 Motor Test／Teach 就停掉每一軸。修正 `54f9c8d0`。
  **以後任何「hub 代送停止」都只能是 jog 放開**（自測 [8] 與全程不變式會擋）。
- m11（`94b5c31e`）：hub 放開後，頁面自己又送同一顆的放開（`HT_WIN`→`releaseHeld`）：不再送第二次，
  等 hub 那次的 ack——成功就回頁面 ok（`linkDup`），被拒就把頁面那次照送當重試。重新按下、超過 2 秒（`DUP_RELEASE_MS`）、`btnStop` 一律照送。

## 5. iframe 找 hub（relay 探測）

- iframe 送 `{ht9045link:'hello'}`（附 MessageChannel port）給父視窗；hub 回 `welcome` 才走 relay。
- 父視窗回 `nohub`（有 link 但這個網址沒有 hub）⇒ **只記在那個網址**，那個網址之後直連。
- 1.5 秒（`HELLO_WAIT_MS`）沒回應 ⇒ **只有這一條**直連、console 警告、`status().relayMisses`++，下一次 `open()` 再問
  （m13，`94b5c31e`；原本一次逾時或別的網址的 nohub 就讓整個 iframe 之後都直連，16 條上限的問題會安靜地回來）。
- 最上層沒有 hub ⇒ 立刻直連；沒有 MessageChannel ⇒ 直連。

## 6. 自測（`ws_link_selftest.cjs`）

- node vm 開一個外框（跑 hub）和幾個 iframe；假伺服器照 WebBridgeServer 的權杖規則，另外照 WebCmdGuard 400 ms 防連點
  （`control.*` 免擋，同真的伺服器）；`server.slow` 延遲回覆、`server.refuseStop` 拒絕某顆按鈕的停止（[19] 用）。
- 子視窗各拿一個自己的父視窗代理，`ev.source` 才分得出是哪一個 iframe（`windowHidden(x._src)`）。
- 段落：[1]～[7] 連線數／id／扇出／快取／權杖；[8] M1；[9] ping 遺失；[10] 退路；[11] 真的 recipe client；[12] 重連；
  [13] takeover／換頁接手；[14] bye／ping 遺失放開 jog；[15] 同資料夾載入＋TK-2；[16] link.token；[17] windowHidden＋
  `setWinState` 接線（先去註解再比）；[18] `HW.teach.html` 頁面放開；[19] m11；[20] m13；最後全程不變式：hub 自己送的停止全是 jog 放開。
- 對照組（改了之後一定要跑一次舊版，確認新測試真的會紅）：`W906_WS_LINK=<舊的 ht9045_link.js>`、
  `W906_WS_BG=<舊的 background.html>`、`W906_WS_TEACH=<舊的 HW.teach.html>`。
- 一律用 `waitFor(條件)`，不要固定 `sleep`：共用 PC 忙時（CPU 100%）20 ms 的 sleep 會隨機紅。

## 7. 踩過的坑

- **同一行插入要在行尾 `//` 註解之前**：`3f95f0c8` 把 `windowHidden` 接在 `background.html:668` 的 `// AI(W906-MT-E3)` 後面＝死碼
  （ST01-E `fd4ecb82` 修）；原本的文字比對檢查照樣通過 ⇒ 來源檢查一律先去註解（引號裡的 `//` 不算）再比。
- Bash 工具送 heredoc 給 Python 時 `\\` 會被砍成 `\`（`\\t` 變 TAB、`\\n` 變換行）：含反斜線的腳本用檔案寫好再跑。
- `grep -c $'\r'` 在這個環境不可靠；數 CR 用 `tr -cd '\r' | wc -c`。

## 8. commit

`d010084a` v1 → `2f0d3be8` recipe client＋takeover 一律送伺服器 → `3f95f0c8`＋`fd4ecb82` windowHidden → `5e01d21f` 接線檢查看程式不看註解
→ `66cd6305` Teach 頁放開 → `54f9c8d0` M1 → `94b5c31e` m11／m13（review6 `0a96fa79`）。

<!-- preserved-content:end -->
